//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

HostDispatchEnumerator::HostDispatchEnumerator(HostDispatch* inHostDispatch) :
    JavascriptEnumerator(inHostDispatch->GetScriptContext()),
    idCurrent(DISPID_STARTENUM)
{
    Assert(inHostDispatch->CanSupportIDispatchEx());
    this->hostDispatch = inHostDispatch;
}


BOOL HostDispatchEnumerator::MoveNext(Js::PropertyAttributes* attributes)
{
    HRESULT hr;
    IDispatchEx* dispatch = (IDispatchEx*)hostDispatch->GetDispatchNoRef();
    if (dispatch  == NULL)
    {
        return FALSE;
    }
    ScriptSite* scriptSite = hostDispatch->GetScriptSite();
    ScriptEngine* scriptEngine;
    if (scriptSite == NULL || (scriptEngine = scriptSite->GetScriptEngine()) == NULL)
    {
        return FALSE;
    }
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();

    /* REVIEW: Do we need to handle exception here? */
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = dispatch->GetNextDispID(fdexEnumAll | scriptEngine->GetInvokeVersion() << 28 , idCurrent, &idCurrent);
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (attributes != nullptr)
    {
        *attributes = PropertyEnumerable;
    }

    return hr == S_OK;
}

Js::Var HostDispatchEnumerator::GetCurrentIndex()
{
    BSTR name;

    IDispatchEx* dispatch = (IDispatchEx*)hostDispatch->GetDispatchNoRef();
    ScriptSite* scriptSite = hostDispatch->GetScriptSite();
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    if (dispatch  == NULL)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    HRESULT hr;
    /* REVIEW: Do we need to handle exception here? */
    BEGIN_LEAVE_SCRIPT(scriptContext)
    { 
        hr = dispatch->GetMemberName(idCurrent, &name);
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (FAILED(hr) || name == NULL)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // Copy the host's bstr into a recyclable buffer and free the bstr.
    UINT len = SysStringLen(name);
    Js::Var str = Js::JavascriptString::NewCopyBuffer(name, len, scriptContext);
    SysFreeString(name);
    return str;
}

void HostDispatchEnumerator::Reset()
{
    idCurrent = DISPID_STARTENUM;
}