//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteScriptFunction;
class RemoteJavascriptFunction : public RemoteRecyclableObject
{
public:
    RemoteJavascriptFunction();
    RemoteJavascriptFunction(ULONG64 ptr);
    RemoteJavascriptFunction(ExtRemoteTyped const& o);
    bool IsScriptFunction();
    bool IsBoundFunction();
    RemoteScriptFunction AsScriptFunction();

    RemoteFunctionInfo GetFunctionInfo();
    RemoteRecyclableObject GetTargetFunction();

    void Print();
};
