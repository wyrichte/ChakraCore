//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once

class SiteService sealed : public IServiceProvider
{
public:
    static HRESULT Create(SiteService **ppdc, ScriptSite *psess);

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

    HRESULT Close(void);

protected:
    long m_refCount;
    ScriptSite *m_psess;

    SiteService(void);
    ~SiteService(void);
};



