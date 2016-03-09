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
    return object.Field("functionInfo").GetPtr();
}

void
RemoteScriptFunction::PrintNameAndNumberWithLink(EXT_CLASS_BASE * ext)
{
    RemoteFunctionBody functionBody = this->GetFunctionBody();
    ExtBuffer<WCHAR> displayNameBuffer;
    ext->Dml(_u("<link cmd=\"!jd.var 0x%p\">%s</link> (#%d.%d, #%d)"), object.GetPtr(),
        functionBody.GetDisplayName(&displayNameBuffer), 
        functionBody.GetSourceContextId(), 
        functionBody.GetLocalFunctionId(), 
        functionBody.GetFunctionNumber());
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------