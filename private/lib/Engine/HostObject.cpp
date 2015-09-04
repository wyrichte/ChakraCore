/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"

HostObject::HostObject(Js::ScriptContext * scriptContext, IDispatch* pDispatch, Js::DynamicType * type):
    Js::HostObjectBase(type), needToCheckOtherItem(FALSE)
{    
    this->hostDispatch = HostDispatch::Create(scriptContext, pDispatch);    
    // Proactively allocate the DISPID cache.
    dispIdCache = RecyclerNew(scriptContext->GetRecycler(), DispIdCacheDictionaryType, scriptContext->GetRecycler(), 8);       
}

Js::ModuleRoot * HostObject::GetModuleRoot(
    Js::ModuleID moduleID)
{
    Assert(moduleID != kmodGlobal);
    ScriptSite* scriptSite = hostDispatch->GetScriptSite();
    return scriptSite->GetModuleRoot(moduleID);
}

Var HostObject::GetNamespaceParent(
    Js::Var childDispatch)
{
    HostDispatch       *pExternal;
    IDispatchEx        *pDispEx;
    IDispatch          *pDisp;
    IUnknown           *pUnk;
    HRESULT            hr;
    ScriptSite           *scriptSite;
    AutoReleasePtr<IDispatch> namespaceDispatch;

    // Do we have an object?

    if (!Js::RecyclableObject::Is(childDispatch))
    {
        return null;       
    }

    // Do we have an external object?
    if(Js::JavascriptOperators::GetTypeId(childDispatch) != Js::TypeIds_HostDispatch)
    {
        return null;
    }

    pExternal = static_cast<HostDispatch*>(childDispatch);
    scriptSite = pExternal->GetScriptSite();
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();

    // Do we have an IDispatchEx?

    pDisp = pExternal->GetDispatchNoRef();
    if (NULL == pDisp)
    {
        return null;
    }

    AssertInScript();
    /* REVIEW: Do we need to handle exception here? */
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = pDisp->QueryInterface(__uuidof(IDispatchEx), (void**)&pDispEx);        
        if (SUCCEEDED(hr))
        {
            // Do we have a parent?

            hr = pDispEx->GetNameSpaceParent(&pUnk);
            pDispEx->Release();
            if (SUCCEEDED(hr) && !pUnk)
            {
                hr = E_INVALIDARG;
            }
            if (SUCCEEDED(hr))
            {
                // Is the parent an IDispatch?
                
                hr = pUnk->QueryInterface(__uuidof(IDispatch), (void**)&namespaceDispatch);
                pUnk->Release();
            }
        }
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (FAILED(hr))
    {
        return null;
    }

    // Yes to all of the above: make a proxy for the parent's IDispatch.

    return HostDispatch::Create(scriptContext, namespaceDispatch);       
}

BOOL HostObject::TryGetDispId(Js::PropertyId propertyId, DISPID *pDispId)
{
    return this->dispIdCache->TryGetValue(propertyId, (int32*)pDispId);
}

void HostObject::CacheDispId(Js::PropertyId propertyId, DISPID dispId)
{
    this->dispIdCache->Add(propertyId, (int32)dispId);

    // Keep cached propertyId alive
    this->GetScriptContext()->TrackPid(propertyId);
}
