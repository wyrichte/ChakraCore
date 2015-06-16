//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

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

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
