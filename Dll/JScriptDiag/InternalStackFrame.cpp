#include "stdafx.h"

namespace JsDiag
{
    InternalStackFrame::InternalStackFrame(IVirtualReader* reader, ThreadContext* threadContext) :
        m_reader(reader),
        m_threadContext(threadContext),
        FrameId(0),
        InstructionPointer(NULL),
        ReturnAddress(NULL),
        EffectiveFrameBase(NULL),
        FrameBase(NULL),
        StackPointer(NULL)
    {
    }

    // Assign frame data but don't touch m_reader and ThreadContext.
    void InternalStackFrame::AssignFrameDataFrom(const InternalStackFrame& other)
    {
        this->FrameId = other.FrameId;
        this->InstructionPointer = other.InstructionPointer;
        this->ReturnAddress = other.ReturnAddress;
        this->EffectiveFrameBase = other.EffectiveFrameBase;
        this->FrameBase = other.FrameBase;
        this->StackPointer = other.StackPointer;
    }

    //
    // Returns true <=> given entry is in stack range of this frame.
    // Parameters:
    // - entry: function entry for this frame (IP of beginning of the function).
    //
    bool InternalStackFrame::IsInStackCheckCode(void* entry)
    {
        void *const codeAddr = this->InstructionPointer;
        return size_t(codeAddr) - size_t(entry) <= this->GetStackCheckCodeHeight();
    }

    size_t InternalStackFrame::GetStackCheckCodeHeight()
    {
        Assert(m_threadContext);
        RemoteThreadContext threadContext(m_reader, m_threadContext);
        size_t stackCheckCodeHeight = 
            threadContext.DoInterruptProbe() ? StackFrameConstants::StackCheckCodeHeightWithInterruptProbe
            : threadContext.GetIsThreadBound() ? StackFrameConstants::StackCheckCodeHeightThreadBound 
            : StackFrameConstants::StackCheckCodeHeightNotThreadBound;
        return stackCheckCodeHeight;
    }

    // Get address to use with RemoteStackFrameEnumerator::AdvanceTo.
    void* InternalStackFrame::GetAdvanceToAddr()
    {
        // advanceToAddr = frame addr for x86/arm, return addr for amd64 (see GET_CURRENT_FRAME_ID, common.h).
        return
#ifdef _M_X64
            this->ReturnAddress;
#else
            this->EffectiveFrameBase;
#endif
    }
}
