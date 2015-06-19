//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace JsDiag
{
    //
    // High-level abstraction of stack frame to unify interpreter and native frames.
    //
    class RemoteDiagFrame
    {
    public:
        virtual ScriptContext* GetScriptContext() = 0;
        virtual FrameDisplay* GetFrameDisplay(RegSlot frameDisplayRegister) = 0;
        virtual Js::Var GetRootObject() = 0;
        virtual Js::Var GetArgumentsObject() = 0;
        virtual Js::Var GetReg(RegSlot reg) = 0;
        virtual Js::Var* GetInParams() = 0;
        virtual int GetInParamCount() = 0;
        virtual RemoteFunctionBody* GetRemoteFunctionBody() = 0;
        virtual ScriptFunction* GetFunction() = 0;
        virtual bool IsInterpreterFrame() = 0;
        virtual UINT16 GetFlags() = 0;
        virtual ~RemoteDiagFrame() {}
    };

} // namespace JsDiag.
