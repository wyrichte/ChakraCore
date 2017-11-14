//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

HRESULT WINAPI GetDumpStreams(
    _In_ IStackProviderDataTarget* dataTarget,
    _In_ MINIDUMP_TYPE DumpType,
    _COM_Outptr_ PMINIDUMP_USER_STREAM_INFORMATION *ppUserStream)
{
    if (!dataTarget || (DumpType & MINIDUMP_TYPE::MiniDumpWithoutAuxiliaryState))
    {
        return E_INVALIDARG;
    }

    if (ppUserStream == nullptr)
    {
        return E_POINTER;
    }

    *ppUserStream = nullptr;

    return JsDiag::JsDebugApiWrapper([=]
    {
        JsDiag::ScriptState::SaveState(dataTarget, ppUserStream, DumpType);
        return S_OK;
    });
}

HRESULT WINAPI FreeDumpStreams(
    _In_ PMINIDUMP_USER_STREAM_INFORMATION pUserStream)
{
    if (pUserStream == nullptr || !JsDiag::ScriptDumpInfo::Is(pUserStream))
    {
        return E_INVALIDARG;
    }

    return JsDiag::JsDebugApiWrapper([=]
    {
        delete JsDiag::ScriptDumpInfo::From(pUserStream);
        return S_OK;
    });
}

namespace JsDiag
{
    //
    // Construct a ScriptDumpStream and set the Type to JavaScriptDataStream.
    //
    ScriptDumpStream::ScriptDumpStream(LPVOID buffer, ULONG bufferSize)
    {
        this->Type = JavaScriptDataStream;
        this->Buffer = buffer;
        this->BufferSize = bufferSize;
    }

    ScriptDumpStream::~ScriptDumpStream()
    {
        delete[] this->Buffer;
    }

    //
    // Check if a given MINIDUMP_USER_STREAM is a valid JavaScriptDataStream. Used to validate in FreeDumpStreams.
    //
    bool ScriptDumpStream::Is(_In_ PMINIDUMP_USER_STREAM pUserStream)
    {
        return pUserStream->Type == JavaScriptDataStream
            && pUserStream->BufferSize != 0
            && pUserStream->Buffer != nullptr;
    }

    //
    // Convert a PMINIDUMP_USER_STREAM to ScriptDumpStream* type. The given stream must originally be a ScriptDumpStream instance.
    //
    ScriptDumpStream* ScriptDumpStream::From(_In_ PMINIDUMP_USER_STREAM pUserStream)
    {
        Assert(Is(pUserStream));
        return static_cast<ScriptDumpStream*>(pUserStream);
    }

    //
    // Construct a ScriptDumpInfo to contain given stream content. We use only 1 user stream.
    //
    ScriptDumpInfo::ScriptDumpInfo(LPVOID buffer, ULONG bufferSize):
        m_scriptDumpStream(buffer, bufferSize)
    {
        this->UserStreamCount = 1;
        this->UserStreamArray = &m_scriptDumpStream;
    }

    //
    // Check if a given PMINIDUMP_USER_STREAM_INFORMATION is a ScriptDumpInfo instance. Used to validate in FreeDumpStreams.
    //
    bool ScriptDumpInfo::Is(_In_ PMINIDUMP_USER_STREAM_INFORMATION pUserStreamInfo)
    {
        return pUserStreamInfo->UserStreamCount == 1
            && (LPBYTE)pUserStreamInfo->UserStreamArray == (LPBYTE)pUserStreamInfo + offsetof(ScriptDumpInfo, m_scriptDumpStream)
            && ScriptDumpStream::Is(pUserStreamInfo->UserStreamArray);
    }

    //
    // Convert a PMINIDUMP_USER_STREAM_INFORMATION to ScriptDumpInfo*. The given stream info must originally be a ScriptDumpInfo instance.
    //
    ScriptDumpInfo* ScriptDumpInfo::From(_In_ PMINIDUMP_USER_STREAM_INFORMATION pUserStreamInfo)
    {
        Assert(Is(pUserStreamInfo));
        return static_cast<ScriptDumpInfo*>(pUserStreamInfo);
    }

    MemoryWriteStream::MemoryWriteStream()
    {
        m_stream.Attach(::SHCreateMemStream(nullptr, 0));
        if (!m_stream)
        {
            DiagException::ThrowOOM();
        }
    }

    void MemoryWriteStream::GetContent(PVOID* ppBuf, ULONG* pSize)
    {
        ULONG streamLen;
        {
            const LARGE_INTEGER li = {0};
            ULARGE_INTEGER uli = {0};

            // Get stream length (only supports maximum ULONG)
            CheckHR(m_stream->Seek(li, SEEK_CUR, &uli));

            if (uli.HighPart != 0)
            {
                DiagException::ThrowOOM();
            }
            streamLen = uli.LowPart;

            // Rewind stream to the beginning
            CheckHR(m_stream->Seek(li, SEEK_SET, /*plibNewPosition*/nullptr));
        }

        AutoArrayPtr<BYTE> pb = new(oomthrow) BYTE[streamLen];
        ULONG readBytes;
        CheckHR(m_stream->Read(pb, streamLen, &readBytes), DiagErrorCode::MEMORY_STREAM_READ);
        if (readBytes != streamLen)
        {
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::MEMORY_STREAM_READ_MISMATCH);
        }

        *ppBuf = pb.Detach();
        *pSize = streamLen;
    }

    void MemoryWriteStream::Write(const void* buf, size_t byteCount)
    {
        ULONG writtenBytes;
        HRESULT hr = m_stream->Write(buf, static_cast<ULONG>(byteCount), &writtenBytes);
        CheckHR(hr, DiagErrorCode::MEMORY_STREAM_WRITE);

        if (static_cast<ULONG>(byteCount) != writtenBytes)
        {
            DiagException::Throw(E_UNEXPECTED, DiagErrorCode::MEMORY_STREAM_WRITE_MISMATCH);
        }
    }

    void MemoryWriteStream::Read(void* buf, size_t byteCount)
    {
        Assert(false); // should never be called
        DiagException::Throw(E_UNEXPECTED);
    }

    bool MemoryWriteStream::IsEOF()
    {
        Assert(false); // should never be called
        DiagException::Throw(E_UNEXPECTED);
    }

    void ScriptState::SaveState(IStackProviderDataTarget* dataTarget, AutoList<WerStack>& werStacks, MINIDUMP_TYPE dumpType)
    {
        DbgEngDiagProvider diagProvider(dataTarget);
        DebugClient debugClient(&diagProvider);

        ThreadContext* pThreadContext = debugClient.GetGlobalValue<ThreadContext*>(Globals_ThreadContextList);
        while (pThreadContext != nullptr)
        {
            RemoteThreadContext threadContext(debugClient.GetReader(), pThreadContext);
            ULONG threadId = threadContext->GetCurrentThreadId();

            // Collect all frames of this thread stack
            CAtlArray<CComPtr<RemoteStackFrame>> frames;
            RemoteStackWalker stackWalker(&debugClient, threadId, pThreadContext);
            while (stackWalker.WalkToNextJavascriptFrame())
            {
                CComPtr<RemoteStackFrame> remoteFrame;
                stackWalker.GetCurrentJavascriptFrame(&remoteFrame);
                frames.Add(remoteFrame);
            }

            // Transform frames into WerStack
            if (!frames.IsEmpty())
            {
                AutoArrayPtr<WerFrame> werFrames = new(oomthrow) WerFrame[frames.GetCount()];
                for (size_t i = 0; i < frames.GetCount(); i++)
                {
                    RemoteStackFrame* frame = frames[i];
                    werFrames[i].Initialize(
                        reinterpret_cast<UINT64>(frame->GetFrameBase()),
                        reinterpret_cast<UINT64>(frame->GetReturnAddress()),
                        reinterpret_cast<UINT64>(frame->GetInstructionPointer()),
                        reinterpret_cast<UINT64>(frame->GetStackPointer()),
                        frame->IsInlineFrame(),
                        frame->GetUri(),
                        frame->GetFunctionName(),
                        frame->GetRow(),
                        frame->GetColumn());

                    // Hack: Only claim JS frame InstructionPointers as pointing to known memory regions to avoid addresses being
                    // poisoned in triage dump. Using size 1 because we don't know or care about the real size of the code blocks.
                    // We could alternatively walk and enumerate real JS dynamic code memory regions, which might be a bit costly.
                    CheckHR(dataTarget->EnumMemoryRegion(werFrames[i].InstructionPointer, 1));
                }

                WerStack* werStack = new(oomthrow) WerStack(threadId, frames.GetCount(), werFrames);
                werFrames.Detach(); // ownership transferred

                werStacks.Add(werStack); // werStacks owns pointers
            }

            if ((dumpType & MiniDumpFilterMemory) == 0)
            {
                // enumerate OOP JIT frames (which are MEM_MAPPED and as such will not automatically be included in dumps)
                uintptr_t preReservedRegionStartAddr = threadContext->m_prereservedRegionAddr;

                if (preReservedRegionStartAddr != 0)
                {
                    uintptr_t preReservedRegionEndAddr = (uintptr_t)RemotePreReservedVirtualAllocWrapper::GetPreReservedEndAddress((void*)preReservedRegionStartAddr);
                    CheckHR(dataTarget->EnumMemoryRegion(preReservedRegionStartAddr, (ULONG)(preReservedRegionEndAddr - preReservedRegionStartAddr)));
                }

                ScriptContext * scriptContextList = threadContext->scriptContextList;
                while (scriptContextList != nullptr)
                {
                    RemoteScriptContext scriptContext(debugClient.GetReader(), scriptContextList);

                    if (scriptContext->jitFuncRangeCache != nullptr)
                    {
                        RemoteJITPageAddrToFuncRangeCacheWrapper jitFuncRangeCache(debugClient.GetReader(), scriptContext->jitFuncRangeCache);

                        if (jitFuncRangeCache->jitPageAddrToFuncRangeMap != nullptr)
                        {
                            RemoteDictionary<JITPageAddrToFuncRangeCache::JITPageAddrToFuncRangeMap> jitPageAddrToFuncRangeMap(debugClient.GetReader(), jitFuncRangeCache->jitPageAddrToFuncRangeMap);

                            jitPageAddrToFuncRangeMap.Map([dataTarget, debugClient](void * pageAddr, JITPageAddrToFuncRangeCache::RangeMap * pRangeMap)
                            {
                                if (pRangeMap != nullptr)
                                {
                                    RemoteDictionary<JITPageAddrToFuncRangeCache::RangeMap> rangeMap(debugClient.GetReader(), pRangeMap);

                                    rangeMap.Map([dataTarget](void * address, uint bytes)
                                    {
                                        CheckHR(dataTarget->EnumMemoryRegion((uintptr_t)address, bytes));
                                    });
                                }
                            });
                        }

                        if (jitFuncRangeCache->largeJitFuncToSizeMap != nullptr)
                        {
                            RemoteDictionary<JITPageAddrToFuncRangeCache::LargeJITFuncAddrToSizeMap> largeFuncMap(debugClient.GetReader(), jitFuncRangeCache->largeJitFuncToSizeMap);
                            largeFuncMap.Map([dataTarget](void * pageAddr, uint bytes)
                            {
                                CheckHR(dataTarget->EnumMemoryRegion((uintptr_t)pageAddr, bytes));
                            });
                        }
                    }

                    scriptContextList = scriptContext->next;
                }
            }

            // Advance to next ThreadContext
            pThreadContext = static_cast<ThreadContext*>(threadContext->next);
        }
    }

    void ScriptState::SaveState(IStackProviderDataTarget* dataTarget, PMINIDUMP_USER_STREAM_INFORMATION *ppUserStream, MINIDUMP_TYPE dumpType)
    {
        LPVOID pb = nullptr;
        ULONG size = 0;
        DiagErrorCode errorCode = DiagErrorCode::NONE;

        // Try to capture inner HR and save the failure code in dump stream
        HRESULT hr = JsDiag::JsDebugApiWrapper([&]
        {
            try
            {
                // Collect all WER stacks
                AutoList<WerStack> werStacks;
                SaveState(dataTarget, werStacks, dumpType);

                // Transform collected WER stacks (even if none) into dump stream
                ULONG stacksCount = static_cast<ULONG>(werStacks.GetCount());
                AutoArrayPtr<WerStack> stacks = new(oomthrow) WerStack[stacksCount];
                werStacks.Map([&](size_t i, WerStack* stack)
                {
                    stacks[i] = *stack; // NOTE: WerStack transfers ownership of frames
                });
                werStacks.Clear(); // No longer needed, release memory

                WerMessage message(stacksCount, /*ownership transfer*/stacks.Detach());
                GetStream(&message, &pb, &size);

                return S_OK;
            }
            catch (const DiagException& diagEx)
            {
                errorCode = diagEx.errorCode; // Save internal error code
                throw diagEx;
            }
        });

        if (FAILED(hr))
        {
            WerMessage message(hr, errorCode);
            GetStream(&message, &pb, &size);
        }

        Assert(pb && size > 0);
        AutoArrayPtr<BYTE> pbRef = reinterpret_cast<BYTE*>(pb); // ownership track
        *ppUserStream = new(oomthrow) ScriptDumpInfo(pb, size);
        pbRef.Detach(); // ownership transferred
    }

    void ScriptState::GetStream(WerMessage* message, PVOID* ppBuf, ULONG* pSize)
    {
        MemoryWriteStream stream;
        Serializer::Serialize(message, &stream, nullptr);
        stream.GetContent(ppBuf, pSize);
    }
}
