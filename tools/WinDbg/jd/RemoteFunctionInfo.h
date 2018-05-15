
//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteFunctionBody;
class RemoteFunctionInfo
{
public:
    RemoteFunctionInfo() {}
    RemoteFunctionInfo(JDRemoteTyped functionInfo) : functionInfo(functionInfo.GetExtRemoteTyped()) {};
    bool HasBody();    
    RemoteFunctionBody GetFunctionBody();
    ULONG64 GetOriginalEntryPoint();
private:
    ExtRemoteTyped functionInfo;
};
