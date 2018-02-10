//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace JsDiag
{
// Javascript stack frame layout (all architechtures):
//   (low addr)
//   EBP = prev frame addr  <-- frame->FrameOffset points here (not applicable for amd64)
//   RET = return addr of current frame
//   argv[0] = function
//   argv[1] = callInfo
//   argv[2] = values[0] = this
//   argv[3] = values[1]
//   argv[4] = values[2]
//   ...
//   (high addr)

RemoteStackWalker::RemoteStackWalker(DebugClient* debugClient, ULONG threadId, ThreadContext* threadContextAddr) :
    m_frameEnumerator(nullptr),
    m_threadId(threadId),
    m_currentJavascriptFrameIndex(0),
    m_scriptEntryExitRecord(nullptr),
    m_prevInterpreterFrame(nullptr),
    m_interpreterFrame(nullptr),
    m_currentFrame(nullptr),
    m_inlineWalker(nullptr),
    m_scriptExitFrameAddr(nullptr),
    m_scriptEntryFrameBase(nullptr),
    m_scriptEntryReturnAddress(nullptr),
    m_isJavascriptFrame(false),
    m_checkedFirstInterpreterFrame(false)
{
    // Set up the Debug Client.
    Assert(debugClient);
    m_debugClient = debugClient;

    CreateComObject(static_cast<RemoteScriptContext*>(nullptr), &m_scriptContext); // Initialize m_scriptContext to nullptr.

    m_reader = m_debugClient->GetReader();

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    // Needed to get DIAG_PHASE_OFF working for inlining
    RemoteConfiguration::EnsureInstance(m_reader, m_debugClient->GetGlobalPointer<const Configuration>(Globals_Configuration)); 
#endif

    Assert(threadContextAddr);
    RemoteThreadContext threadContext(m_reader, threadContextAddr);
    if(threadContext->entryExitRecord)
    {
        // All conditions are valid, there is JS stack in the thread.
        this->SetScriptEntryExitRecord(threadContext->entryExitRecord);

        // Frame addr for x86/arm, return addr for amd64 (see GET_CURRENT_FRAME_ID, common.h).
        void* frameAddrOfScriptExitFunction = m_scriptEntryExitRecord->ToTargetPtr()->frameIdOfScriptExitFunction;

        // If we are outside the script, grab the frame from which we left script.
        // Otherwise, just start from top of stack (do nothing, that's the default for the enumerator).
        if (frameAddrOfScriptExitFunction != nullptr)
        {
            this->SetScriptContext(m_scriptEntryExitRecord->ToTargetPtr()->scriptContext);
        }
       
        // Make sure that we have initial ScriptContext set.
        if (m_scriptContext->Get() == nullptr)
        {
            // Get top one from ThreadContext::ScriptContext list.
            this->SetScriptContext(threadContext->scriptContextList);
        }

        Assert(m_scriptContext->Get());
        if (m_scriptContext->Get()) // Defense in depth: Only create m_frameEnumerator to enumerate if we have a scriptContext
        {
            // Now we can create the enumerator.
            m_frameEnumerator = m_debugClient->CreateStackFrameEnumerator(m_threadId, frameAddrOfScriptExitFunction);

            m_prevInterpreterFrame = threadContext->leafInterpreterFrame ?
                new(oomthrow) RemoteInterpreterStackFrame(m_reader, threadContext->leafInterpreterFrame) :
                nullptr;
        }
    }
}

RemoteStackWalker::~RemoteStackWalker()
{
}

// Returns true when found next JS frame.
// Returns false if walked to the very end and couldn't find a JS frame.
bool RemoteStackWalker::WalkToNextJavascriptFrame()
{
    while (this->WalkOneFrame())
    {
        if (this->IsJavascriptFrame())
        {
            return true;
        }
    }

    return false;
}

void RemoteStackWalker::GetCurrentJavascriptFrame(RemoteStackFrame** remoteFrame)
{
    if (m_currentFrame == nullptr)
    {
        // Either WalkToNextJavascriptFrame() wasn't called OR no more frames available.
        DiagException::Throw(E_UNEXPECTED, DiagErrorCode::STACKFRAMEENUMERATOR_NO_MORE_FRAME);
    }
    else
    {
        Assert(remoteFrame);
        Js::JavascriptFunction* functionAddr = this->GetCurrentFunction();

#ifdef _M_AMD64
        if (functionAddr == m_debugClient->GetGlobalPointer<void>(Globals_Amd64FakeEHFrameProcOffset))
        {
            Assert(m_lastInternalFrame.GetFrameAddress() && m_lastInternalFrame.GetFrameType() == InternalFrameType::IFT_EHFrame);
            Assert(!m_interpreterFrame);

            RefCounted<RemoteFunctionBody> nullBody;
            CreateComObject(nullptr, &nullBody); // Create a ref counted object which contains null RemoteFunctionBody
             
            // Create a frame with null functionbody
            CreateComObject(nullBody, m_scriptContext, functionAddr, 
                this->GetCurrentArgvAddress(), this->GetCurrentCallInfo(), m_currentFrame, remoteFrame);
            return;
        }
#endif
        
        RemoteJavascriptFunction function(m_reader, functionAddr);

        uint loopNumber = LoopHeader::NoLoop;
        int byteCodeOffset = this->GetByteCodeOffset(&loopNumber); // Make sure we call this before detaching the interpreter frame.

        RefCounted<RemoteFunctionBody> functionBody;
        GetRefCountedRemoteFunctionBody(function.GetFunction(), &functionBody);

        CComPtr<RemoteStackFrame> frame;
        CreateComObject(functionBody, m_scriptContext, functionAddr, 
            this->GetCurrentArgvAddress(), this->GetCurrentCallInfo(), m_currentFrame, &frame);

        // TODO: consider moving these into constructor.
        frame->SetInterpreterFrame(m_interpreterFrame.Detach()); // WARNING: This detaches and passes ownership of interpreterFrame
        frame->SetByteCodeOffset(byteCodeOffset);
        frame->SetIsInlineFrame(m_inlineWalker != nullptr);
        frame->SetLoopBodyNumber(loopNumber);
        frame->SetFrameId(this->m_currentJavascriptFrameIndex);

        *remoteFrame = frame.Detach();
    }
}

// Walk to next frame and update m_currentFrame.
// Returns false when there are no more frames available.
bool RemoteStackWalker::WalkOneFrame()
{
    m_interpreterFrame = nullptr;
    if (m_lastInternalFrame.IsFrameConsumed())
    {
        m_lastInternalFrame.Reset();
    }

    if (m_inlineWalker)
    {
        bool isNextInlineFrameAvailable = m_inlineWalker->Next();
        if (!isNextInlineFrameAvailable)
        {
            m_inlineWalker = nullptr;
            // Next frame now is the native frame we had when inline walk was initiated.
        }

        return true;
    }

    if (m_frameEnumerator && m_frameEnumerator->Next())
    {
        bool isFirstFrame = m_currentFrame == nullptr;
        m_currentFrame = new(oomthrow) InternalStackFrame(m_reader, m_scriptContext->Get()->ToTargetPtr()->threadContext);
        m_frameEnumerator->Current(m_currentFrame);

        void* ip = reinterpret_cast<void*>(m_currentFrame->InstructionPointer);
        if(isFirstFrame)
        {
            // For Hybrid Debugging we need to provide correct 'end frame' for stack range.
            // As there is no uppper frame, use StackPointer as it marks the end of the frame & must be available for 1st jitted frame.
            this->m_scriptExitFrameAddr = m_currentFrame->StackPointer ? 
                m_currentFrame->StackPointer :
                m_currentFrame->EffectiveFrameBase;
        }
        // If we're at the entry from a host frame, hop to the frame from which we left the script.
        if (m_scriptEntryExitRecord && ip == m_scriptEntryExitRecord->ToTargetPtr()->returnAddrOfScriptEntryFunction)
        {
            m_scriptEntryFrameBase = m_currentFrame->EffectiveFrameBase;
            m_scriptEntryReturnAddress = m_currentFrame->ReturnAddress;

            bool advanced = false;

            // Get next script exit record. Note: WER might be calling us after AV which has corrupted our internal state. Stop if entryExitRecord list looks wrong.
            const Js::ScriptEntryExitRecord* next = m_scriptEntryExitRecord->ToTargetPtr()->next;
            if (next > m_scriptEntryExitRecord->GetRemoteAddr()) // Valid next must be below current
            {
                this->SetScriptEntryExitRecord(next);
                this->SetScriptContext(m_scriptEntryExitRecord->ToTargetPtr()->scriptContext);

                this->m_scriptExitFrameAddr = this->AdvanceToFrame(m_scriptEntryExitRecord->ToTargetPtr()->frameIdOfScriptExitFunction);
                if (this->m_scriptExitFrameAddr >= m_scriptEntryFrameBase) // Note "==" is valid: m_scriptEntryFrameBase is actually caller of real entry frame. It could also be exit frame.
                {
                    advanced = true; // successfully advanced to next script frame
                }
            }

            if (!advanced)
            {
                m_isJavascriptFrame = false;
                return false; // No more script frames.
            }
        }

        this->UpdateFrame();
        return true;
    }

    InternalStackFrame* oldValue = m_currentFrame.Detach();
    delete oldValue;
    Assert(m_currentFrame == nullptr);
    return false;
}

void RemoteStackWalker::UpdateFrame()
{
    m_isJavascriptFrame = this->CheckJavascriptFrame();

#if defined(DUMP_FRAMES_LEVEL) && DUMP_FRAMES_LEVEL > 1
     this->DumpFrame();
#endif
    if (this->IsJavascriptFrame())
    {
        m_currentJavascriptFrameIndex++;
        
        // Update script context by one from current frame.
        Js::JavascriptFunction* functionAddr = this->GetCurrentFunction();

#if defined(_M_AMD64)
        if (functionAddr == m_debugClient->GetGlobalPointer<void>(Globals_Amd64FakeEHFrameProcOffset))
        {
            Assert(m_lastInternalFrame.GetFrameAddress() && m_lastInternalFrame.GetFrameType() == InternalFrameType::IFT_EHFrame);
            return;
        }
#endif

        // We might've bailed out of an inlinee, so check if there were any inlinees.
        if (m_interpreterFrame && (m_interpreterFrame->ToTargetPtr()->m_flags & InterpreterStackFrameFlags_FromBailOut))
        {
            Assert(!m_inlineWalker);

            m_inlineWalker = RemoteInlineFrameWalker::FromPhysicalFrame(m_reader, m_currentFrame, 
                reinterpret_cast<ScriptFunction *>(functionAddr));

            if (m_inlineWalker)
            {
                bool areInlineFramesAvailable = m_inlineWalker->Next();
                Assert(areInlineFramesAvailable);
                Assert(m_interpreterFrame->ToTargetPtr()->function == m_inlineWalker->GetCurrentFunction());

                // We're now back in the state where currentFrame == physical frame of the inliner, but
                // since interpreterFrame != null, we'll pick values from the interpreterFrame (the bailout
                // frame of the inliner). Also, the stackwalker will continue from the
                // inlinee frames on the stack when Walk() is called next time.
            }
        }
        else
        {
            Assert(m_interpreterFrame == nullptr || m_interpreterFrame->ToTargetPtr()->function == functionAddr);
        }

        if (functionAddr)
        {
            RemoteRecyclableObject obj(m_reader, functionAddr);
            this->SetScriptContext(obj.GetScriptContext());
        }
    }
}

bool RemoteStackWalker::CheckJavascriptFrame()
{
    void* ip = m_currentFrame->InstructionPointer;
    if(ip == nullptr || m_debugClient->IsJsModuleAddress(ip))
    {
        return false;
    }
    if (this->IsInterpreterFrame(ip))
    {
        // On stack top we may run into a partially initialized interperter stack frame.
        if (!m_checkedFirstInterpreterFrame)
        {
            m_checkedFirstInterpreterFrame = true;

            // A partially initialized interpreter frame may match returnAddress of leaf caller interpreter frame (recursive call).
            // When this happens, its frame base is above (<) the leaf interpreter caller frame.
            if (m_currentFrame->EffectiveFrameBase < m_prevInterpreterFrame->ToTargetPtr()->addressOfReturnAddress)
            {
                return false; // Skip it
            }
        }

        // We've got an interpterer frame.
        m_interpreterFrame = m_prevInterpreterFrame.Detach();
        Js::InterpreterStackFrame* interpreterFrame = m_interpreterFrame->ToTargetPtr()->previousInterpreterFrame;
        m_prevInterpreterFrame = interpreterFrame ?
            new(oomthrow) RemoteInterpreterStackFrame(m_reader, interpreterFrame) :
            nullptr;

        if (m_interpreterFrame->IsCurrentLoopNativeAddr(m_lastInternalFrame.GetFrameAddress()))
        {
            // Found interpreter frame containing last internal frame.
            m_lastInternalFrame.SetIsFrameConsumed();
        }
        else
        {
            // Current frame is interpreter frame, and last internal frame does not belong to it.
            // Just in case, make sure we don't have last internal frame.
            m_lastInternalFrame.Reset();
        }

        return true;
    }
    else if (this->IsNativeFrame(ip))
    {
#if defined(_M_AMD64)
        if (reinterpret_cast<void*>(this->GetCurrentFunction()) == m_debugClient->GetGlobalPointer<void>(Globals_Amd64FakeEHFrameProcOffset))
        {
            // There could be nested internal frames in the case of try...catch..finally
            // let's not set the last internal frame address if it has already been set.
            // Note: this is for just native frames (it doesn't have to be interpreter frame owning internal EH frame).
            if (m_lastInternalFrame.GetFrameAddress() == nullptr)
            {
                m_lastInternalFrame.SetFrame(ip, InternalFrameType::IFT_EHFrame);
            }
            Assert(m_lastInternalFrame.GetFrameAddress() && m_lastInternalFrame.GetFrameType() == InternalFrameType::IFT_EHFrame);
            return true;
        }
#endif

        if (this->GetCurrentCallInfo().Flags & CallFlags_InternalFrame)
        {
            m_lastInternalFrame.SetFrame(ip, InternalFrameType::IFT_LoopBody);
            return true;
        }

        // Current frame is native; if there was internal EH frame, we should keep it for e.g. GetByteCodeOffset,
        // so the line/col number corresponds to the internal frame where the IP actually is.
        // Mark it as consumed, as it's valid only within context of current native frame.
        if (m_lastInternalFrame.GetFrameAddress() != nullptr)
        {
            AssertMsg(m_lastInternalFrame.GetFrameType() == InternalFrameType::IFT_EHFrame, "Got unexpected internal frame!");
            m_lastInternalFrame.SetIsFrameConsumed();
        }

        AssertMsg(!m_inlineWalker, "When we get here, m_inlineWalker must be nullptr!");
        m_inlineWalker = RemoteInlineFrameWalker::FromPhysicalFrame(m_reader, m_currentFrame, 
            reinterpret_cast<ScriptFunction *>(this->GetCurrentFunction()));
        if (m_inlineWalker)
        {
            bool isNextInlineFrameAvailable = m_inlineWalker->Next();
            Assert(isNextInlineFrameAvailable);
            // From now on we'll walk inline frames. When they are over, we'll continue with current frame being same m_currentFrame.
        }

        return true;
    }
    else
    {
        return false;
    }
}

// Bytecode offset for current frame.
uint32 RemoteStackWalker::GetByteCodeOffset(uint* loopNumber)
{
    if (this->IsJavascriptFrame())
    {
        if (m_interpreterFrame && m_lastInternalFrame.GetFrameAddress() == nullptr) // Pure interpreter frame (no loop body).
        {
            RemoteByteCodeReader reader(m_reader, m_interpreterFrame->GetReader());
            uint32 offset = reader.GetCurrentOffset();
            
            if(this->m_currentJavascriptFrameIndex == 1)
            {
                // On the topmost frame, if we are on a breakpoint or stepping then we already have the correct
                // bytecode offset as the instruction has not yet been executed
                RemoteThreadContext threadContext(m_reader, m_scriptContext->Get()->GetThreadContext());
                DebugManager* debugManager = threadContext.GetDebugManager();
                if (debugManager)
                {
                    RemoteDebugManager remoteDebugManager(m_reader, debugManager);
                    if (remoteDebugManager.GetIsAtDispatchHalt())
                    {
                        // This relies on ProbeContainer::DispatchXXX to adjust bytecode offset on top-most frame, if required
                        // (only ProbeContainer::DispatchExceptionBreakpoint does the adjustment, see setting pHaltState->SetCurrentOffset),
                        // As library frame support is not enabled yet in hybrid and enabled in inproc, inproc side wouldn't see library frames 
                        // (while hybrid currently sees them all) and DispatchExceptionBreakpoint on the inproc side wouldn't adjust offset 
                        // for top-most frame. So, adjust it here (just fall through for the exception case).
                        // TODO: remove special case (make 'if (!needAdjustment)' block unconditional) when library frame support is added on hybrid side.
                        Assert(remoteDebugManager.GetInterpreterHaltState());
                        RemoteInterpreterHaltState remoteHaltState(m_reader, remoteDebugManager.GetInterpreterHaltState());
                        bool needAdjustment = false; // needAdjustment <=> top-most frame && exception && library code.
                        if (remoteHaltState->stopType == StopType::STOP_EXCEPTIONTHROW)
                        {
                            RemoteFunctionBody remoteBody(m_reader, m_interpreterFrame->ToTargetPtr()->m_functionBody);
                            if (remoteBody->m_utf8SourceInfo)
                            {
                                RemoteUtf8SourceInfo remoteUtf8SourceInfo(m_reader, remoteBody->m_utf8SourceInfo);
                                needAdjustment = remoteUtf8SourceInfo->m_isLibraryCode;
                            }
                        }
                        if (!needAdjustment)
                        {
                            return offset;
                        }
                    }
                }
            }

            if (offset == 0)
            {
                // This will be the case when we are broken on the debugger on very first statement (due to async break).
                // Or the interpreter loop can throw OOS on entrance before executing bytecode.
                return offset;
            }
            else
            {
                // Make sure current offset belongs to current executing opcode, 
                // as normally current offset of the reader corresponds to next opcode.
                return offset - 1;
            }
        }
        else // The frame is one of: { interpreter + jit loop body, pure native, native + inline }.
        {
            DWORD_PTR codeAddr;
            this->GetCodeAddrAndLoopNumberFromCurrentFrame(&codeAddr, loopNumber);

            // If there is interpreter frame, get the function from the interpreterFrame -- 
            // exactly same as this->GetCurrentFunction(false) if the interperter frame is not called from bailout path.
            JavascriptFunction* functionAddr = m_interpreterFrame ? 
                m_interpreterFrame->ToTargetPtr()->function :   // TODO: maybe just call this->GetCurrentFunction() [adjust it for interpreter frame]?
                this->GetCurrentFunction(false);
            RemoteJavascriptFunction function(m_reader, functionAddr);
            FunctionBody* inlinee = m_inlineWalker ? 
                RemoteJavascriptFunction(m_reader, m_inlineWalker->GetCurrentFunction()).GetFunction() :
                nullptr;
            AssertMsg(!m_interpreterFrame || m_lastInternalFrame.GetFrameAddress(), "Interpreter frame must be jit loop body here.");
            // Note: inlining is disabled in jit loop body. Don't attempt to get the statement map from the inlined frame for jit loop body. 

            uint32 inlineeOffset = 0;
            if (!this->m_interpreterFrame && !m_lastInternalFrame.GetFrameAddress()) // Otherwise the frame is interpreter + jit loop body.
            {
                if (m_inlineWalker)
                {
                    inlineeOffset = m_inlineWalker->GetCurrentInlineeOffset();
                    // Note: the IP of top-most inline frame would be the IP of physical frame.
                }
                else
                {
                    // If we are not currently walking inline frames, but they are available,
                    // the offset of physical frame would be the offset of the bottom inline frame.
                    AutoPtr<RemoteInlineFrameWalker> tmpInlineWalker = RemoteInlineFrameWalker::FromPhysicalFrame(
                        m_reader, m_currentFrame, reinterpret_cast<ScriptFunction *>(functionAddr));
                    inlineeOffset = tmpInlineWalker ? tmpInlineWalker->GetBottomMostInlineeOffset() : 0;
                }
            }

            StatementData statementMap;
            RemoteFunctionBody functionBody(m_reader, function.GetFunction());
            if (functionBody.GetMatchingStatementMapFromNativeOffset(&statementMap, codeAddr, *loopNumber, inlineeOffset, inlinee))
            {
                return statementMap.bytecodeBegin;
            }
        }
    } // if (this->IsJavascriptFrame()).

    return 0;
}

void RemoteStackWalker::GetCodeAddrAndLoopNumberFromCurrentFrame(DWORD_PTR* pCodeAddr, uint* pLoopNumber)
{
    Assert(pCodeAddr);
    Assert(pLoopNumber);
    Assert(this->IsJavascriptFrame());

    uint loopNumber = LoopHeader::NoLoop;
    DWORD_PTR codeAddr;

    if (m_lastInternalFrame.GetFrameAddress())
    {
        if (m_lastInternalFrame.GetFrameType() == IFT_LoopBody)
        {
            if (m_interpreterFrame) // We are at the interpreter frame that calls jit loop body
            {
                loopNumber = m_interpreterFrame->ToTargetPtr()->currentLoopNum;
            }
            else if (m_prevInterpreterFrame) // We are at the jit loop body frame
            {
                loopNumber = m_prevInterpreterFrame->ToTargetPtr()->currentLoopNum;
            }
        }

        codeAddr = reinterpret_cast<DWORD_PTR>(m_lastInternalFrame.GetFrameAddress());
    }
    else
    {
        codeAddr = reinterpret_cast<DWORD_PTR>(m_currentFrame->InstructionPointer);
    }
    Assert(codeAddr);

    if (codeAddr)
    {
        // Note: decrement is by 1 byte, as DWORD_PTR is typedef for unsigned long/__int64.
#ifdef _M_ARM
        // For ARM the 'return address' is always odd and is 'next instr addr' + 1 byte, so to get to the BLX instr, we need to subtract 2 bytes from it.
        AssertMsg(codeAddr % 2 == 1, "Got even number for codeAddr! It's expected to be return address, which should be odd.");
        codeAddr--;
#endif
        codeAddr--;
    }

    *pCodeAddr = codeAddr;
    *pLoopNumber = loopNumber;
}

bool RemoteStackWalker::IsInterpreterFrame(void* instructionPointer)
{
    return
        m_prevInterpreterFrame != nullptr &&
        instructionPointer == m_prevInterpreterFrame->ToTargetPtr()->returnAddress;
}

bool RemoteStackWalker::IsNativeFrame(void* instructionPointer)
{
    return (m_scriptContext->Get() != nullptr ? m_scriptContext->Get()->IsNativeAddress(instructionPointer) : false);
}

bool RemoteStackWalker::IsJavascriptFrame()
{
    return m_isJavascriptFrame;
}

//
// Advance enumerator and m_currentFrame to specific frame below.
// If we can't find it, leave the enumerator at last frame and m_currentFrame unchanged.
// Returns stack pointer of frame that was advanced to. null if the frame was not found.
//
void* RemoteStackWalker::AdvanceToFrame(const void* advanceToAddr)
{
    // TODO: this needs to be moved to enumerator, and frame chain enumerator should just start from this addr
    //       discarding all frames on top (we can rely on us compiled with EBP-frames but not on 3rd parties.
    //       And we could do nicer than #ifdef for multi-platform support.

    Assert(m_currentFrame);
    if (m_currentFrame->GetAdvanceToAddr() == advanceToAddr)
    {
        return m_currentFrame->EffectiveFrameBase;
    }

    Assert(m_frameEnumerator);
    if (m_frameEnumerator->AdvanceToFrame(advanceToAddr))
    {
        m_frameEnumerator->Current(m_currentFrame);
        // Assert(m_currentFrame->GetAdvanceToAddr() == advanceToAddr); // Note that for chain based enu, we will return the frame below.

        return m_currentFrame->EffectiveFrameBase;
    }

    return nullptr;
}

void** RemoteStackWalker::GetCurrentArgvAddress(bool includeInlineFrames /* = true */)
{
    // Note: for interpreter original walker is using interpreterFrame->GetArgumentsObject(),
    // but here current one should be fine.
    if (includeInlineFrames && m_inlineWalker)
    {
        return m_inlineWalker->GetCurrentArgvAddress();
    }

    size_t platformAdjustment =
#ifdef _M_X64
        0;
#else
        8;
#endif
    return reinterpret_cast<void**>(reinterpret_cast<BYTE*>(m_currentFrame->EffectiveFrameBase) + platformAdjustment);
}

Js::JavascriptFunction* RemoteStackWalker::GetCurrentFunction(bool includeInlineFrames /* = true */)
{
    if (includeInlineFrames && m_inlineWalker)
    {
        return m_inlineWalker->GetCurrentFunction();
    }

    // Note: this can be called from inside CheckJavascriptFrame for a frame that we haven't set m_isJavascriptFrame yet.
    const void* addr = reinterpret_cast<BYTE*>(
        this->GetCurrentArgvAddress(includeInlineFrames)) + Js::JavascriptFunctionArgIndex_Function * sizeof(void*);
    return VirtualReader::ReadVirtual<Js::JavascriptFunction*>(this->m_reader, addr);
}

Js::CallInfo RemoteStackWalker::GetCurrentCallInfo(bool includeInlineFrames /* = true */)
{
    if (includeInlineFrames && m_inlineWalker)
    {
        return m_inlineWalker->GetCurrentCallInfo();
    }

    // Note: this can be called from inside CheckJavascriptFrame for a frame that we haven't set m_isJavascriptFrame yet.
    void* addr = reinterpret_cast<BYTE*>(
        this->GetCurrentArgvAddress(includeInlineFrames)) + Js::JavascriptFunctionArgIndex_CallInfo * sizeof(void*);
    return VirtualReader::ReadVirtual<Js::CallInfo>(this->m_reader, addr);
}

void RemoteStackWalker::SetScriptContext(const Js::ScriptContext* newScriptContextAddr)
{
    // Note: even if newScriptContextAddr is null, we still wrap it with CComPtr<CComObject> for consistency.
    RefCounted<RemoteScriptContext> scriptContext;
    this->GetRefCountedRemoteScriptContext(newScriptContextAddr, &scriptContext);
    m_scriptContext = scriptContext;
}

void RemoteStackWalker::SetScriptEntryExitRecord(const Js::ScriptEntryExitRecord* newRecord)
{
    m_scriptEntryExitRecord = newRecord ? new(oomthrow) RemoteScriptEntryExitRecord(m_reader, newRecord) : nullptr;
}

void* RemoteStackWalker::GetCurrentScriptExitFrameBase()
{
    return m_scriptExitFrameAddr;
}

void* RemoteStackWalker::GetCurrentScriptEntryFrameBase()
{
    return m_scriptEntryFrameBase;
}

void* RemoteStackWalker::GetCurrentScriptEntryReturnAddress()
{
    return m_scriptEntryReturnAddress;
}

#if defined(DBG) || defined(ENABLE_DEBUG_CONFIG_OPTIONS)
void RemoteStackWalker::DumpFrame()
{
    WCHAR buf[256];
#ifdef _M_X64
    swprintf_s(buf, _countof(buf), _u("\t#%02d Base=%08llp IP=%08llp Ret=%08llp %s\n"),
        m_currentFrame->FrameId, m_currentFrame->EffectiveFrameBase, m_currentFrame->InstructionPointer, m_currentFrame->ReturnAddress, m_isJavascriptFrame ? _u("JS") : _u(""));
#else
    swprintf_s(buf, _countof(buf), _u("\t#%02d Base=%08p IP=%08p Ret=%08p %s\n"),
        m_currentFrame->FrameId, m_currentFrame->EffectiveFrameBase, m_currentFrame->InstructionPointer, m_currentFrame->ReturnAddress, m_isJavascriptFrame ? _u("JS") : _u(""));
#endif
    OutputDebugString(buf);
    wprintf_s(_u("%s"), buf);

    // Read some data from the stack.
    WCHAR* start = buf;
    for (int j = 0; j < 4; ++j)
    {
        int charsWritten = 0;
        RemoteData<long*> data(m_reader, (const PLONG*)((BYTE*)m_currentFrame->EffectiveFrameBase + j * sizeof(long*)));
#ifdef _M_X64
        charsWritten = swprintf_s(start, _countof(buf) - (start - buf), _u("\t\t%08llp"), *data.ToTargetPtr());
#else
        charsWritten = swprintf_s(start, _countof(buf) - (start - buf), _u("\t\t%08p"), *data.ToTargetPtr());
#endif
        if(charsWritten == -1)
        {
            break;
        }
        start += charsWritten;
    }
    OutputDebugString(buf);
    OutputDebugString(_u("\n"));
    wprintf_s(_u("%s\n"), buf);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// InternalFrameTracker

RemoteStackWalker::InternalFrameTracker::InternalFrameTracker() :
    m_state(InternalFrameState::IFS_NoFrame),
    m_frameAddress(nullptr),
    m_frameType(InternalFrameType::IFT_None)
{
}

void RemoteStackWalker::InternalFrameTracker::Reset()
{
    this->SetState(InternalFrameState::IFS_NoFrame);
    m_frameAddress = nullptr;
    m_frameType = InternalFrameType::IFT_None;
}

void RemoteStackWalker::InternalFrameTracker::SetFrame(void* frameAddr, InternalFrameType frameType)
{
    AssertMsg(m_frameAddress == nullptr || m_frameAddress == frameAddr, "It is not valid to redefine internal frame.");
    Assert(m_frameType == InternalFrameType::IFT_None || m_frameType == frameType);

    this->SetState(InternalFrameState::IFS_FrameDetected);
    m_frameAddress = frameAddr;
    m_frameType = frameType;
}

void* RemoteStackWalker::InternalFrameTracker::GetFrameAddress()
{
    return m_frameAddress;
}

RemoteStackWalker::InternalFrameType RemoteStackWalker::InternalFrameTracker::GetFrameType()
{
    return m_frameType;
}

bool RemoteStackWalker::InternalFrameTracker::IsFrameConsumed()
{
    return m_state == InternalFrameState::IFS_FrameConsumed;
}

void RemoteStackWalker::InternalFrameTracker::SetIsFrameConsumed()
{
    this->SetState(InternalFrameState::IFS_FrameConsumed);
}

void RemoteStackWalker::InternalFrameTracker::SetState(InternalFrameState state)
{
    AssertMsg(this->IsValidStateTransition(state), "Invalid state transition.");
    m_state = state;
}

#ifdef DBG
bool RemoteStackWalker::InternalFrameTracker::IsValidStateTransition(InternalFrameState newState)
{
    if (newState == m_state)
    {
        return true;
    }

    if (m_state == InternalFrameState::IFS_NoFrame)
    {
        return newState == InternalFrameState::IFS_FrameDetected;
    }
    else if (m_state == InternalFrameState::IFS_FrameDetected)
    {
        return newState == InternalFrameState::IFS_FrameConsumed;
    }
    else if (m_state == InternalFrameState::IFS_FrameConsumed)
    {
        return newState == InternalFrameState::IFS_NoFrame;
    }
    else
    {
        AssertMsg(FALSE, "Unknown state in m_state!");
        return false;
    }
}
#endif DBG

void RemoteStackWalker::GetRefCountedRemoteFunctionBody(const FunctionBody* addr, _Out_ RefCountedObject<RemoteFunctionBody>** ppRefCountedRemoteFunctionBody)
{
    RefCounted<RemoteFunctionBody> body;
    if (!m_functionBodyMap.Lookup(addr, body))
    {
        AutoPtr<RemoteFunctionBody> functionBody = new(oomthrow) RemoteFunctionBody(m_reader, addr);
        CreateComObject(functionBody, &body);
        functionBody.Detach(); // ownership passed

        m_functionBodyMap[addr] = body;
    }

    CheckHR(body.CopyTo(ppRefCountedRemoteFunctionBody));
}

void RemoteStackWalker::GetRefCountedRemoteScriptContext(const ScriptContext* addr, _Out_ RefCountedObject<RemoteScriptContext>** ppRefCountedRemoteScriptContext)
{
    RefCounted<RemoteScriptContext> scriptContext;
    if (!m_scriptContextMap.Lookup(addr, scriptContext))
    {
        AutoPtr<RemoteScriptContext> tempContext = new(oomthrow) RemoteScriptContext(m_reader, addr);
        CreateComObject(tempContext, &scriptContext);
        tempContext.Detach(); // ownership passed

        m_scriptContextMap[addr] = scriptContext;
    }

    CheckHR(scriptContext.CopyTo(ppRefCountedRemoteScriptContext));
}

} // namespace JsDiag.

  // shared static data
#include "DateImplementationData.h"

  // stubs
namespace Js
{
#if DBG
    bool Throw::ReportAssert(LPCSTR fileName, uint lineNumber, LPCSTR error, LPCSTR message)
    {
        AssertMsg(false, "Runtime assertion");
        return false;
    }

    void Throw::LogAssert()
    {
        AssertMsg(false, "Runtime assertion");
    }
#endif

    void Throw::FatalInternalError()
    {
        AssertMsg(false, "Runtime fatal error");
    }

#if defined(PROFILE_RECYCLER_ALLOC) && defined(RECYCLER_DUMP_OBJECT_GRAPH)
    bool RecyclableObject::DumpObjectFunction(type_info const * typeinfo, bool isArray, void * objectAddress) { return true; }
    bool Type::DumpObjectFunction(type_info const * typeinfo, bool isArray, void * objectAddress) { return true; }
#endif
}

#if defined(PROFILE_RECYCLER_ALLOC) && defined(RECYCLER_DUMP_OBJECT_GRAPH)
void RecyclerObjectDumper::RegisterDumper(type_info const * typeinfo, DumpFunction dumperFunction) {}
#endif
