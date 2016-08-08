//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"
#include "hostdispatchenumerator.h"

HostDispatchEnumerator::HostDispatchEnumerator(HostDispatch* inHostDispatch) :
    JavascriptEnumerator(inHostDispatch->GetScriptContext()),
    idCurrent(DISPID_STARTENUM)
{
    Assert(inHostDispatch->CanSupportIDispatchEx());
    this->hostDispatch = inHostDispatch;
}


Var HostDispatchEnumerator::MoveAndGetNext(Js::PropertyId& propertyId, Js::PropertyAttributes* attributes)
{
    propertyId = Js::Constants::NoProperty;
    HRESULT hr;
    IDispatchEx* dispatch = (IDispatchEx*)hostDispatch->GetDispatchNoRef();
    if (dispatch  == NULL)
    {
        return nullptr;
    }
    ScriptSite* scriptSite = hostDispatch->GetScriptSite();
    ScriptEngine* scriptEngine;
    if (scriptSite == NULL || (scriptEngine = scriptSite->GetScriptEngine()) == NULL)
    {
        return nullptr;
    }
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    BSTR name = nullptr;

    /* REVIEW: Do we need to handle exception here? */
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = dispatch->GetNextDispID(fdexEnumAll | scriptEngine->GetInvokeVersion() << 28 , idCurrent, &idCurrent);
        if (SUCCEEDED(hr))
        {
            hr = dispatch->GetMemberName(idCurrent, &name);
        }
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (FAILED(hr) || name == NULL)
    {
        return nullptr;
    }

    if (attributes != nullptr)
    {
        *attributes = PropertyEnumerable;
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