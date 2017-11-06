
//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteFunctionBody;
class RemoteFunctionInfo
{
public:
    RemoteFunctionInfo() {}
    RemoteFunctionInfo(ExtRemoteTyped const& functionInfo) : functionInfo(functionInfo) {};
    bool HasBody();    
    RemoteFunctionBody GetFunctionBody();
    ULONG64 GetOriginalEntryPoint();
private:
    ExtRemoteTyped functionInfo;
};
