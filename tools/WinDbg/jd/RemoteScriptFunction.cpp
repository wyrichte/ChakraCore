//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

RemoteScriptFunction::RemoteScriptFunction()
{}

RemoteScriptFunction::RemoteScriptFunction(ExtRemoteTyped const& o) :
    RemoteJavascriptFunction(o)
{
}

RemoteScriptFunction::RemoteScriptFunction(ULONG64 ptr) :
    RemoteScriptFunction(ExtRemoteTyped("(Js::ScriptFunction *)@$extin", ptr))
{
}

RemoteFunctionBody
RemoteScriptFunction::GetFunctionBody()
{
    RemoteFunctionInfo functionInfo(object.Field("functionInfo"));
    return functionInfo.GetFunctionBody();
}

void
RemoteScriptFunction::PrintNameAndNumberWithLink()
{
    RemoteFunctionBody functionBody = this->GetFunctionBody();
    ExtBuffer<WCHAR> displayNameBuffer;
    if (GetExtension()->PreferDML())
    {
        GetExtension()->Dml(_u("<link cmd=\"!jd.var 0x%p\">%s</link> (#%d.%d, #%d)"), object.GetPtr(),
            functionBody.GetDisplayName(&displayNameBuffer),
            functionBody.GetSourceContextId(),
            functionBody.GetLocalFunctionId(),
            functionBody.GetFunctionNumber());
    }
    else
    {
        GetExtension()->Out(_u("%s (#%d.%d, #%d) /*\"!jd.var 0x%p\" to display*/"),
            functionBody.GetDisplayName(&displayNameBuffer),
            functionBody.GetSourceContextId(),
            functionBody.GetLocalFunctionId(),
            functionBody.GetFunctionNumber(),
            object.GetPtr());
    }
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
