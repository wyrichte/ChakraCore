//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

class RemoteInterpreterStackFrame
{
public:
    RemoteInterpreterStackFrame();
    RemoteInterpreterStackFrame(ExtRemoteTyped const& interpreterStackFrame);

    RemoteInterpreterStackFrame GetPreviousFrame();
    ULONG64 GetReturnAddress();
    RemoteFunctionBody GetFunctionBody();
    RemoteScriptFunction GetScriptFunction();
    bool IsNull();
private:
    ExtRemoteTyped interpreterStackFrame;
};

#endif