#pragma once

namespace JsDiag
{
    //
    // Interface for enumerating stack frames.
    //
    struct RemoteStackFrameEnumerator
    {
        // Arbitrary maximum number of stack frames to walk. Consider script stack overflow case.
        // Use this limit to break from infinite loop when data is corrupted.
        static const ULONG MAX_STACKFRAMES = 20000 * 20;

        virtual bool Next() = 0;
        virtual void Current(InternalStackFrame* frame) = 0;
        virtual bool AdvanceToFrame(const void* advanceToAddr);
        virtual ~RemoteStackFrameEnumerator() {}
    };

    //
    // dbgeng-like Stack frame enumerator (IDebugControl::GetStackTrace).
    //
    class DbgEngStackFrameEnumerator : public RemoteStackFrameEnumerator
    {
        CComPtr<IStackProviderDataTarget> m_dataTarget;
        STACKFRAME64 m_frames[2];    // Current frame and frame below it. We keep the frame below for EffectiveFrameBase on amd64.
        CONTEXT m_context;
        ULONG m_threadId;
        ULONG m_currentFrameId;
        bool m_noMoreFrames;

    public:
        DbgEngStackFrameEnumerator(IStackProviderDataTarget* dataTarget, ULONG growDelta, ULONG threadId);
        virtual ~DbgEngStackFrameEnumerator() override;

        virtual bool Next() override;
        virtual void Current(InternalStackFrame* frame) override;

    private:
        void AdvanceToNextFrame();
    };

    // Stack frame enumerator based on stack unwinder provided by VS.
    class VSStackFrameEnumerator : public RemoteStackFrameEnumerator
    {
        CComPtr<IJsDebugDataTarget> m_dataTarget;
        CComPtr<IEnumJsStackFrames> m_enumerator;
        JS_NATIVE_FRAME m_frames[2];    // Current frame and frame below it. We keep the frame below for EffectiveFrameBase on amd64.
        ULONG m_currentFrameId;
        bool m_canRequestProvider;      // Whether it's OK to ask provider for more frames. If false, m_frames[1] is N/A but current frame is valid.
        bool m_noMoreFrames;

    public:
        VSStackFrameEnumerator(ULONG threadId, IJsDebugDataTarget* dataTarget);
        virtual ~VSStackFrameEnumerator() override;

        virtual bool Next() override;
        virtual void Current(InternalStackFrame* frame) override;

    private:
        void AdvanceToNextFrame();
    };

    //
    // Stack frame enumerator based on dbgeng.dll (IDebugControl::GetStackTrace).
    //
    class FrameChainBasedStackFrameEnumerator : public RemoteStackFrameEnumerator
    {
        AutoPtr<InternalStackFrame> m_currentFrame;
        IVirtualReader* m_reader;

    public:
        FrameChainBasedStackFrameEnumerator(void* frameAddress, void* stackAddress, void* instructionPointer, IVirtualReader* reader);
        ~FrameChainBasedStackFrameEnumerator();

        virtual bool Next() override;
        virtual void Current(InternalStackFrame* frame) override;
        virtual bool AdvanceToFrame(const void* advanceToAddr) override;

    private:

        static void* GetChildFrameCallSiteFromFrameBase(void* frameBase);
        void* GetRetAddrFromFrameBase(void* frameBase);
    };

} // namespace JsDiag.
