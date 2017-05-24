//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteInterpreterStackFrame
{
public:
    RemoteInterpreterStackFrame();
    RemoteInterpreterStackFrame(ExtRemoteTyped const& interpreterStackFrame);

    RemoteInterpreterStackFrame GetPreviousFrame();
    ULONG64 GetReturnAddress();
    RemoteFunctionBody GetFunctionBody();
    RemoteScriptFunction GetScriptFunction();
    bool IsFromBailout();
    bool IsNull();
    ULONG64 GetAddress();
    ULONG GetCurrentLoopNum();
private:
    ExtRemoteTyped interpreterStackFrame;
};
