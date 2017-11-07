//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef JD_PRIVATE

RemoteInterpreterStackFrame::RemoteInterpreterStackFrame()
{
}

RemoteInterpreterStackFrame::RemoteInterpreterStackFrame(ExtRemoteTyped const& interpreterStackFrame) :
    interpreterStackFrame(interpreterStackFrame)
{}

bool
RemoteInterpreterStackFrame::IsNull()
{
    return interpreterStackFrame.GetPtr() == 0;
}

ULONG64 RemoteInterpreterStackFrame::GetAddress()
{
    return interpreterStackFrame.GetPtr();
}

ULONG RemoteInterpreterStackFrame::GetCurrentLoopNum()
{
    return interpreterStackFrame.Field("currentLoopNum").GetUlong();
}

RemoteInterpreterStackFrame 
RemoteInterpreterStackFrame::GetPreviousFrame()
{
    return interpreterStackFrame.Field("previousInterpreterFrame");
}

ULONG64
RemoteInterpreterStackFrame::GetReturnAddress()
{
    return interpreterStackFrame.Field("returnAddress").GetPtr();
}

RemoteFunctionBody
RemoteInterpreterStackFrame::GetFunctionBody()
{
    return interpreterStackFrame.Field("m_functionBody");
}


RemoteScriptFunction
RemoteInterpreterStackFrame::GetScriptFunction()
{
    return interpreterStackFrame.Field("function");
}

bool 
RemoteInterpreterStackFrame::IsFromBailout()
{
    ULONG64 Ret;
    // legacy jscript implementation does not have this flag
    if (FAILED(GetExtension()->CppEvalExprU64NoThrow(GetExtension()->FillModule("%s!Js::InterpreterStackFrameFlags_FromBailOut"), Ret)))
        return false;
    return interpreterStackFrame.Field("m_flags").GetShort() & Ret;
}
#endif