#include "stdafx.h"
#include "RemoteStackFrameEnumerator.h"

namespace JsDiag
{
///////////////////////////////////////////////////////////////////////////////////////////////////
// RemoteStackFrameEnumerator.
    bool RemoteStackFrameEnumerator::AdvanceToFrame(const void* advanceToAddr)
    {
        InternalStackFrame tempFrame(nullptr, nullptr);
        while (this->Next())
        {
            this->Current(&tempFrame);
            if (advanceToAddr == tempFrame.GetAdvanceToAddr())
            {
                return true;
            }
        }

        return false;
    }


///////////////////////////////////////////////////////////////////////////////////////////////////
// DbgEngStackFrameEnumerator.
    const ULONG c_initialFrameId = static_cast<ULONG>(-1);

    DbgEngStackFrameEnumerator::DbgEngStackFrameEnumerator(IStackProviderDataTarget* dataTarget, ULONG growDelta, ULONG threadId) :
        m_dataTarget(dataTarget), m_threadId(threadId), m_currentFrameId(c_initialFrameId), m_noMoreFrames(false)
    {
        Assert(dataTarget);

        SecureZeroMemory(m_frames, sizeof(m_frames));
        SecureZeroMemory(&m_context, sizeof(m_context));

        HRESULT hr = m_dataTarget->GetThreadContext(threadId, CONTEXT_ALL, sizeof(m_context), (PBYTE)&m_context);
        CheckHR(hr, DiagErrorCode::DIAGPROVIDER_GETTHREADCONTEXT);

        hr = m_dataTarget->GetStackTrace(m_threadId, &m_frames[1], &m_context);
        CheckHR(hr, DiagErrorCode::DIAGPROVIDER_GETSTACKTRACE);

        this->AdvanceToNextFrame();
    }

    DbgEngStackFrameEnumerator::~DbgEngStackFrameEnumerator()
    {
    }

    // Advances to next frame.
    bool DbgEngStackFrameEnumerator::Next()
    {
        this->AdvanceToNextFrame();
        if (m_noMoreFrames)
        {
            return false;
        }

        return ++m_currentFrameId < MAX_STACKFRAMES;
    }

    // Returns current frame, or throws.
    void DbgEngStackFrameEnumerator::Current(InternalStackFrame* frame)
    {
        if (m_noMoreFrames || m_currentFrameId == c_initialFrameId)
        {
            DiagException::Throw(E_INVALIDARG, DiagErrorCode::STACKFRAMEENUMERATOR_NO_MORE_FRAME);
        }

        const STACKFRAME64& currentFrame = m_frames[0];
#ifdef _M_X64
        // We always have one extra frame beyond 'last' cached frame.
        frame->EffectiveFrameBase = reinterpret_cast<void*>(m_frames[1].AddrStack.Offset);
#elif defined (_M_ARM)
        // The FrameBase that underlying enumerator returns is exactly parent callsite SP/start of argv.
        // Adjust it to our representation (subtract space for R11 and RET).
        frame->EffectiveFrameBase = reinterpret_cast<void*>(m_frames[1].AddrStack.Offset - 2 * sizeof(void*));
#else
        frame->EffectiveFrameBase = reinterpret_cast<void*>(currentFrame.AddrFrame.Offset);
#endif
        frame->FrameBase = reinterpret_cast<void*>(currentFrame.AddrFrame.Offset);
        frame->InstructionPointer = reinterpret_cast<void*>(currentFrame.AddrPC.Offset);
        frame->ReturnAddress = reinterpret_cast<void*>(currentFrame.AddrReturn.Offset);
        frame->StackPointer = reinterpret_cast<void*>(currentFrame.AddrStack.Offset);
        frame->FrameId = this->m_currentFrameId;
    }

    void DbgEngStackFrameEnumerator::AdvanceToNextFrame()
    {
        if (!m_noMoreFrames)
        {
            // Shift m_frames.
            m_frames[0] = m_frames[1];

            HRESULT hr = m_dataTarget->GetStackTrace(m_threadId, &m_frames[1], &m_context);
            CheckHR(hr, DiagErrorCode::DIAGPROVIDER_GETSTACKTRACE);

            if (m_frames[1].AddrReturn.Offset == 0)
            {
                m_noMoreFrames = true;
            }
        }
    }

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSStackFrameEnumerator.
        
    VSStackFrameEnumerator::VSStackFrameEnumerator(ULONG threadId, IJsDebugDataTarget* dataTarget) : 
        m_dataTarget(dataTarget), m_currentFrameId(c_initialFrameId), m_canRequestProvider(true), m_noMoreFrames(false)
    {
        HRESULT hr = m_dataTarget->CreateStackFrameEnumerator(threadId, &m_enumerator);
        CheckHR(hr, DiagErrorCode::STACKFRAMEENUMERATOR_CREATE);

        // Note: according to contract with VS, the enumerator doesn't need to be reset.
        //       It's fine to call Reset but that would just re-initialize to the same pos consuming perf.

        SecureZeroMemory(m_frames + 1, sizeof(JS_NATIVE_FRAME));
        this->AdvanceToNextFrame();
    }

    VSStackFrameEnumerator::~VSStackFrameEnumerator()
    {
    }

    bool VSStackFrameEnumerator::Next()
    {
        this->AdvanceToNextFrame();
        if (m_noMoreFrames)
        {
            return false;
        }

        return ++m_currentFrameId < MAX_STACKFRAMES;
    }

    void VSStackFrameEnumerator::Current(InternalStackFrame* frame)
    {
        if (m_noMoreFrames || m_currentFrameId == c_initialFrameId)
        {
            DiagException::Throw(E_INVALIDARG, DiagErrorCode::STACKFRAMEENUMERATOR_NO_MORE_FRAME);
        }

        JS_NATIVE_FRAME* currentFrame = &m_frames[0];
#ifdef _M_X64
        // We always have one extra frame beyond current frame.
        frame->EffectiveFrameBase = reinterpret_cast<void*>(m_frames[1].StackOffset);
#elif defined (_M_ARM)
        // The FrameBase that underlying enumerator returns is exactly parent callsite SP/start of argv.
        // Adjust it to our representation (subtract space for R11 and RET).
        frame->EffectiveFrameBase = reinterpret_cast<void*>(m_frames[1].StackOffset - 2 * sizeof(void*));
#else
        frame->EffectiveFrameBase = reinterpret_cast<void*>(currentFrame->FrameOffset);
#endif
        frame->FrameBase = reinterpret_cast<void*>(currentFrame->FrameOffset);
        frame->InstructionPointer = reinterpret_cast<void*>(currentFrame->InstructionOffset);
        frame->ReturnAddress = reinterpret_cast<void*>(currentFrame->ReturnOffset);
        frame->StackPointer = reinterpret_cast<void*>(currentFrame->StackOffset);
        frame->FrameId = m_currentFrameId;
    }

    void VSStackFrameEnumerator::AdvanceToNextFrame()
    {
        if (!m_noMoreFrames)
        {
            if (!m_canRequestProvider)
            {
                m_noMoreFrames = true;
            }
            else
            {
                // Shift m_frames.
                *m_frames = *(m_frames + 1);

                ULONG filledCount;
                HRESULT hr = m_enumerator->Next(1, m_frames + 1, &filledCount);
                CheckHR(hr, DiagErrorCode::STACKFRAMEENUMERATOR_NEXT);
                AssertMsg(hr == S_FALSE && filledCount == 0 || hr == S_OK && filledCount == 1, "Wrong filledCount.");

                if (hr == S_FALSE)
                {
                    SecureZeroMemory(m_frames + 1, sizeof(JS_NATIVE_FRAME));
                    m_canRequestProvider = false;
                }
            }
        }
    }

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameChainBasedStackFrameEnumerator.
    
    FrameChainBasedStackFrameEnumerator::FrameChainBasedStackFrameEnumerator(void* frameAddress, void* stackAddress, void* instructionPointer, IVirtualReader* reader)
        : m_reader(reader)
    {
#ifdef _M_X64
        AssertMsg(FALSE, "FrameChainBasedStackFrameEnumerator is not supported for AMD64 (frame chains in general are not supported)");
        DiagException::Throw(E_INVALIDARG);
#endif
        // Set up current frame so that when we call Next 1st time it will return this frame.
        m_currentFrame = new(oomthrow) InternalStackFrame(m_reader);

        m_currentFrame->EffectiveFrameBase = frameAddress;
        m_currentFrame->FrameBase = m_currentFrame->EffectiveFrameBase;
        m_currentFrame->InstructionPointer = instructionPointer;
        m_currentFrame->ReturnAddress = this->GetRetAddrFromFrameBase(m_currentFrame->EffectiveFrameBase);
        m_currentFrame->StackPointer = stackAddress;
        m_currentFrame->FrameId = (ULONG)-1;
    }

    FrameChainBasedStackFrameEnumerator::~FrameChainBasedStackFrameEnumerator()
    {
    }

    bool FrameChainBasedStackFrameEnumerator::Next()
    {
        if (m_currentFrame == nullptr)
        {
            return false;
        }
        else if (m_currentFrame->EffectiveFrameBase == nullptr)
        {
            // No more frames.
            m_currentFrame = nullptr;
            return false;
        }

        // Very 1st frame is what's passed to our ctor, check if we need to return it, otherwise advance to next frame.
        if (m_currentFrame->FrameId != (ULONG)-1)
        {
            void* newFrameBase = VirtualReader::ReadVirtual<void*>(m_reader, m_currentFrame->EffectiveFrameBase);
            if (newFrameBase == nullptr || newFrameBase <= m_currentFrame->EffectiveFrameBase)
            {
                m_currentFrame = nullptr;
                return false;
            }

            m_currentFrame->EffectiveFrameBase = newFrameBase;
            m_currentFrame->FrameBase = m_currentFrame->EffectiveFrameBase;
            m_currentFrame->StackPointer = m_currentFrame->StackPointer;
            m_currentFrame->InstructionPointer = m_currentFrame->ReturnAddress;
            m_currentFrame->ReturnAddress = this->GetRetAddrFromFrameBase(m_currentFrame->EffectiveFrameBase);
        }

        return ++m_currentFrame->FrameId < MAX_STACKFRAMES;
    }

    void FrameChainBasedStackFrameEnumerator::Current(InternalStackFrame* frame)
    {
        Assert(frame);

        if (m_currentFrame == nullptr)
        {
            DiagException::Throw(E_INVALIDARG, DiagErrorCode::STACKFRAMEENUMERATOR_NO_MORE_FRAME);
        }

        Assert(frame);

        frame->AssignFrameDataFrom(*m_currentFrame);
    }

    bool FrameChainBasedStackFrameEnumerator::AdvanceToFrame(const void* advanceToAddr)
    {
        // Just hop over all the frames.
        // Note: same idea as in JavascriptStackWalker, based on the observation that we never leave script on actual jitted/int.thunk frame.
        m_currentFrame->EffectiveFrameBase = const_cast<void*>(advanceToAddr);

        // Keep "ReturnAddress" in sync with "EffectiveFrameBase". It will become "InstructionPointer" for next frame.
        if (m_currentFrame->EffectiveFrameBase)
        {
            m_currentFrame->ReturnAddress = this->GetRetAddrFromFrameBase(m_currentFrame->EffectiveFrameBase);
        }

        return this->Next();

        // Note: now FrameId is not compatible with debugger frame numbering.
    }

    // static
    void* FrameChainBasedStackFrameEnumerator::GetChildFrameCallSiteFromFrameBase(void* frameBase)
    {
        return reinterpret_cast<BYTE*>(frameBase) + 2 * sizeof(void*);
    }

    void* FrameChainBasedStackFrameEnumerator::GetRetAddrFromFrameBase(void* frameBase)
    {
        return VirtualReader::ReadVirtual<void*>(m_reader, (BYTE*)frameBase + sizeof(void*));
    }

} // namespace JsDiag.
