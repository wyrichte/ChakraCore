//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

class RemoteScriptFunction : public RemoteJavascriptFunction
{
public:
    RemoteScriptFunction();
    RemoteScriptFunction(ULONG64 ptr);
    RemoteScriptFunction(ExtRemoteTyped const& scriptFunction);
        
    RemoteFunctionBody GetFunctionBody();
    void PrintNameAndNumberWithLink();
};

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------