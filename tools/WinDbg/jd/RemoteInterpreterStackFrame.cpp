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
#endif