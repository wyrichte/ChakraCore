//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace JsDiag
{
    ////////////////////////////////////// RemoteDiagInterpreterFrame

    RemoteStackFrame::RemoteDiagInterpreterFrame::RemoteDiagInterpreterFrame(RemoteStackFrame* actualFrame) :
        m_actualFrame(actualFrame)
    {
        Assert(m_actualFrame);
    }

    ScriptContext* RemoteStackFrame::RemoteDiagInterpreterFrame::GetScriptContext()
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->ToTargetPtr()->GetScriptContext();
    }

    FrameDisplay* RemoteStackFrame::RemoteDiagInterpreterFrame::GetFrameDisplay(RegSlot frameDisplayRegister)
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->GetFrameDisplay(frameDisplayRegister);
    }

    Js::Var RemoteStackFrame::RemoteDiagInterpreterFrame::GetInnerScope(RegSlot scopeLocation)
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->GetInnerScope(scopeLocation);
    }

    Js::Var RemoteStackFrame::RemoteDiagInterpreterFrame::GetRootObject()
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->GetRootObject();
    }

    Js::Var RemoteStackFrame::RemoteDiagInterpreterFrame::GetArgumentsObject()
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->ToTargetPtr()->GetArgumentsObject();
    }

    Js::Var RemoteStackFrame::RemoteDiagInterpreterFrame::GetReg(RegSlot reg)
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->GetReg(reg);
    }

    Js::Var* RemoteStackFrame::RemoteDiagInterpreterFrame::GetInParams()
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->ToTargetPtr()->m_inParams;
    }

    int RemoteStackFrame::RemoteDiagInterpreterFrame::GetInParamCount()
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->ToTargetPtr()->m_inSlotsCount;
    }

    RemoteFunctionBody* RemoteStackFrame::RemoteDiagInterpreterFrame::GetRemoteFunctionBody()
    {
        return m_actualFrame->GetRemoteFunctionBody();
    }

    ScriptFunction* RemoteStackFrame::RemoteDiagInterpreterFrame::GetFunction()
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->ToTargetPtr()->function;
    }

    bool RemoteStackFrame::RemoteDiagInterpreterFrame::IsInterpreterFrame()
    {
        return true;
    }

    UINT16 RemoteStackFrame::RemoteDiagInterpreterFrame::GetFlags()
    {
        return m_actualFrame->GetRemoteInterpreterFrame()->ToTargetPtr()->m_flags;
    }

    ////////////////////////////////////// RemoteDiagNativeFrame

    RemoteStackFrame::RemoteDiagNativeFrame::RemoteDiagNativeFrame(RemoteStackFrame* actualFrame) :
        m_actualFrame(actualFrame),
        m_localVarSlotsOffset(Constants::InvalidOffset)
    {
        void** stackAddr = m_actualFrame->GetArgvAddr();

        DWORD_PTR codeAddr = (DWORD_PTR)m_actualFrame->GetInstructionPointer();
        FunctionEntryPointInfo* entryPointInfoAddr = this->GetRemoteFunctionBody()->GetEntryPointFromNativeAddress(codeAddr);
        if (entryPointInfoAddr)
        {
            RemoteEntryPointInfo<FunctionEntryPointInfo> entryPointInfo(m_actualFrame->GetInspectionContext()->GetReader(), entryPointInfoAddr);
            m_localVarSlotsOffset = entryPointInfo->localVarSlotsOffset;
        }
    }

    ScriptContext* RemoteStackFrame::RemoteDiagNativeFrame::GetScriptContext()
    {
        RemoteScriptContext* scriptContext = m_actualFrame->GetRemoteScriptContext();
        Assert(scriptContext);
        return scriptContext->GetRemoteAddr();
    }

    FrameDisplay* RemoteStackFrame::RemoteDiagNativeFrame::GetFrameDisplay(RegSlot frameDisplayRegister)
    {
        FrameDisplay* displayAddr = nullptr;

        Assert(this->GetFunction() != nullptr);
        RegSlot frameDisplayReg = this->GetRemoteFunctionBody()->GetFrameDisplayRegister();

        if (frameDisplayReg != Js::Constants::NoRegister && frameDisplayReg != 0)
        {
            displayAddr = (FrameDisplay*)this->GetReg(frameDisplayReg);
        }
        else
        {
            ScriptFunction* funcAddr = this->GetFunction();
            RemoteScriptFunction func(m_actualFrame->GetInspectionContext()->GetReader(), funcAddr);
            displayAddr = func->environment;
        }

        return displayAddr;
    }

    Js::Var RemoteStackFrame::RemoteDiagNativeFrame::GetInnerScope(RegSlot scopeLocation)
    {
        return GetReg(scopeLocation);
    }

    Js::Var RemoteStackFrame::RemoteDiagNativeFrame::GetRootObject()
    {
        return this->GetRemoteFunctionBody()->GetRootObject();
    }

    Js::Var RemoteStackFrame::RemoteDiagNativeFrame::GetArgumentsObject()
    {
        Js::Var* addr = (Js::Var*)this->m_actualFrame->GetArgvAddr() + JavascriptFunctionArgIndex_ArgumentsObject;
        return VirtualReader::ReadVirtual<Js::Var>(this->m_actualFrame->GetInspectionContext()->GetReader(), addr);
    }

    Js::Var RemoteStackFrame::RemoteDiagNativeFrame::GetReg(RegSlot slotId)
    {
        Js::Var* varAddr = this->GetNonTempSlotOffsetLocation(slotId);
        return varAddr != nullptr ?
            VirtualReader::ReadVirtual<Js::Var>(this->m_actualFrame->GetInspectionContext()->GetReader(), varAddr) :
            nullptr;
    }

    Js::Var* RemoteStackFrame::RemoteDiagNativeFrame::GetInParams()
    {
        Js::Var* addr = (Js::Var*)this->m_actualFrame->GetArgvAddr() + JavascriptFunctionArgIndex_This;
        return (Js::Var*)addr;
    }

    int RemoteStackFrame::RemoteDiagNativeFrame::GetInParamCount()
    {
        return m_actualFrame->GetCallInfo().Count;
    }

    RemoteFunctionBody* RemoteStackFrame::RemoteDiagNativeFrame::GetRemoteFunctionBody()
    {
        return m_actualFrame->GetRemoteFunctionBody();
    }

    ScriptFunction* RemoteStackFrame::RemoteDiagNativeFrame::GetFunction()
    {
        JavascriptFunction* function = m_actualFrame->GetFunction();
        return static_cast<ScriptFunction*>(function);
    }

    bool RemoteStackFrame::RemoteDiagNativeFrame::IsInterpreterFrame()
    {
        return false;
    }

    UINT16 RemoteStackFrame::RemoteDiagNativeFrame::GetFlags()
    {
        // See CDebugStackFrame::GetDebugPropertyCore: the flags are only used for interpreter scenario (for just my code?).
        return 0;
    }

    // Returns an address that belongs to target address space.
    Js::Var* RemoteStackFrame::RemoteDiagNativeFrame::GetNonTempSlotOffsetLocation(RegSlot slotId)
    {
        RemoteFunctionBody* body = this->GetRemoteFunctionBody();
        Assert(body);

        int32 slotOffset;
        if (body->GetNonTempSlotOffset(slotId, &slotOffset))
        {
            Assert(m_localVarSlotsOffset != Constants::InvalidOffset);
            slotOffset = m_localVarSlotsOffset + slotOffset;
            
            void** stackAddr = m_actualFrame->GetArgvAddr();
            return (Js::Var *)(((char *)stackAddr) + slotOffset);
        }

        Assert(false);
        return nullptr;
    }

} // namespace JsDiag.
