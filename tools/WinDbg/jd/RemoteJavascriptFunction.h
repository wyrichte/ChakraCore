//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

class RemoteScriptFunction;
class RemoteJavascriptFunction : public RemoteRecyclableObject
{
public:
    RemoteJavascriptFunction();
    RemoteJavascriptFunction(ULONG64 ptr);
    RemoteJavascriptFunction(ExtRemoteTyped const& o);
    bool IsScriptFunction();
    RemoteScriptFunction AsScriptFunction();

    RemoteFunctionInfo GetFunctionInfo();
};

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
