//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
class ChakraHostScriptContext sealed : public HostScriptContext
{
public:
    ChakraHostScriptContext(ScriptSite * scriptSite)
        : HostScriptContext(scriptSite->GetScriptSiteContext())
    {
        this->scriptSite = scriptSite;
        scriptSite->AddRef();
    }
    ~ChakraHostScriptContext()
    {
        scriptSite->Release();
    }
    virtual void Delete()
    {
        HeapDelete(this);
    }

    HRESULT GetPreviousHostScriptContext(__deref_out HostScriptContext** previousScriptSite)
    {
        return scriptSite->GetPreviousHostScriptContext(previousScriptSite);
    }

    HRESULT SetCaller(IUnknown *punkNew, IUnknown **ppunkPrev)
    {
        scriptSite->SetCaller(punkNew, ppunkPrev);
        return NOERROR;
    }

    BOOL HasCaller()
    {
        return scriptSite->GetCurrentCallerNoRef() != nullptr;
    }

    HRESULT PushHostScriptContext()
    {
        return scriptSite->PushHostScriptContext(this);
    }

    void PopHostScriptContext()
    {
        scriptSite->PopHostScriptContext();
    }

    HRESULT GetDispatchExCaller(__deref_out void** dispatchExCaller)
    {
        return scriptSite->GetDispatchExCaller((DispatchExCaller**)dispatchExCaller);
    }

    void ReleaseDispatchExCaller(__in void* dispatchExCaller)
    {
        scriptSite->ReleaseDispatchExCaller((DispatchExCaller*)dispatchExCaller);
    }

    Js::ModuleRoot * GetModuleRoot(int moduleID)
    {
        Assert(moduleID != kmodGlobal);
        return scriptSite->GetModuleRoot(moduleID);
    }

    HRESULT CheckCrossDomainScriptContext(__in Js::ScriptContext* scriptContext) override
    {
        return scriptSite->CheckCrossDomainScriptContext(scriptContext);
    }

    HRESULT GetHostContextUrl(__in DWORD_PTR hostSourceContext, __out BSTR& pUrl) override
    {
        return scriptSite->GetScriptEngine()->GetHostContextUrl(hostSourceContext, &pUrl);
    }

    void CleanDynamicCodeCache() override
    {
        if (!scriptSite->IsClosed()) {
            scriptSite->GetScriptEngine()->CleanScriptBodyMap();
        }
    }

    HRESULT VerifyDOMSecurity(Js::ScriptContext* targetContext, Js::Var obj) override
    {
        return scriptSite->VerifyDOMSecurity(targetContext, obj);
    }

    Js::JavascriptMethod GetSimpleSlotAccessCrossSiteThunk() override
    {
        return DOMFastPathInfo::CrossSiteSimpleSlotAccessorThunk;
    }

    HRESULT CheckEvalRestriction() override
    {
        return scriptSite->CheckEvalRestriction();
    }

    HRESULT HostExceptionFromHRESULT(HRESULT hr, Js::Var* outError) override
    {
        return scriptSite->HostExceptionFromHRESULT(hr, outError);
    }

    HRESULT GetExternalJitData(ExternalJitData id, void *data) override
    {
        return scriptSite->GetExternalJitData(id, data);
    }

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
    void EnsureParentInfo(Js::ScriptContext* scriptContext = nullptr) override
    {
        return scriptSite->EnsureParentInfo(scriptContext);
    }
#endif

    ScriptSite * GetScriptSite() { return scriptSite; }
private:
    ScriptSite * scriptSite;
};
