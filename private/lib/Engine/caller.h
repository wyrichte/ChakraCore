/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

class ScriptSite;
class DispatchExCaller sealed : public IServiceProvider, public IProvideRuntimeContext, public ICanHandleException
    {
public:
    static HRESULT Create(
        __in ScriptSite *session,
        __in IUnknown *punkCaller,
        __out DispatchExCaller **dispatchCaller);

    DispatchExCaller(__in DispatchExCaller*);

    /****************************************
        IUnknown methods
    ****************************************/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    /****************************************
        IServiceProvider methods
    ****************************************/
    virtual HRESULT STDMETHODCALLTYPE QueryService(REFGUID guidService,
        REFIID riid, void **ppvObj);

    HRESULT STDMETHODCALLTYPE QSCaller(REFGUID guidService,
        REFIID riid, void ** ppvObj);
    HRESULT STDMETHODCALLTYPE QSSite(REFGUID guidService,
        REFIID riid, void ** ppvObj);


    /****************************************
    IProvideRuntimeContext methods
    ****************************************/    
    virtual HRESULT STDMETHODCALLTYPE GetCurrentSourceContext(DWORD_PTR* pdwContext, VARIANT_BOOL* pfExecutingGlobalCode);

    virtual HRESULT STDMETHODCALLTYPE CanHandleException(
        /* [in] */ __RPC__in EXCEPINFO *pExcepInfo,
        /* [in] */ __RPC__in VARIANT *pvar);

    /****************************************
    DispatchExCaller methods
    ****************************************/ 
    HRESULT Close(void);

    ScriptSite * GetScriptSite() const { return scriptSite; }
protected:
    long refCount;
    ScriptSite *scriptSite;
    IUnknown *m_punkCaller;
    IUnknown *m_punkSite;

    // Cache ServiceProviders.
    bool m_fQueriedCaller : 1;
    bool m_fQueriedSite : 1;
    IServiceProvider *m_pspCaller;
    IServiceProvider *m_pspSite;

    DispatchExCaller(void);
    ~DispatchExCaller(void);
    };



