//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    //
    // Internal representation of stack frame used between RemoteStackWalker and RemoteStackFrameEnumerator.
    //
    class InternalStackFrame
    {
        friend class FrameChainBasedStackFrameEnumerator;
    public:
        ULONG FrameId;
        void* InstructionPointer;
        void* ReturnAddress;
        void* EffectiveFrameBase;         // [x86/ARM] EBP/r11; [amd64] Parent SP. Points right to the arguments.
        void* FrameBase;                  // Actual frame base: EBP/r11/RBP. Used for walking inline frames, important to be RBP on amd64.
        void* StackPointer;
    private:
        IVirtualReader* m_reader;
        ThreadContext* m_threadContext;

    public:
        InternalStackFrame(IVirtualReader* reader, ThreadContext* threadContext = NULL); // NULL threadContext is only used by FrameChainBasedStackFrameEnumerator.
        void AssignFrameDataFrom(const InternalStackFrame& other);   // Assign frame data but don't touch m_reader and ThreadContext.
        bool IsInStackCheckCode(void* entry);
        size_t GetStackCheckCodeHeight();
        void* GetAdvanceToAddr();

    private:
        // Disable to avoid misuse overwriting m_reader and ThreadContext(use AssignFrom instead).
        InternalStackFrame& operator=(const InternalStackFrame&);
    };
} // namespace JsDiag.
