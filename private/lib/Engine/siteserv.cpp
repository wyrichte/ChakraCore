//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#include "EnginePch.h"
#pragma hdrstop

/*
SiteService object is an IServiceProvider that does not directly
provide any services but does allow contained objects access to 
services provided by the script site. When an externl object is 
created via CreateObject/ActiveXObject/etc and that object supports
IObjectWithSite, then a SiteService object is created and a pointer 
passed down to the newly created external object.
*/

SiteService::SiteService(void)
{
    m_refCount = 1;
    m_psess = NULL;
}


SiteService::~SiteService(void)
{
    Close();
}


HRESULT SiteService::Close(void)
{
    if (NULL != m_psess)
    {
        m_psess->Release();
        m_psess = NULL;
    }
    return NOERROR;
}


HRESULT SiteService::Create(SiteService **ppdc, ScriptSite *psess)
{
    AssertMem(ppdc);
    AssertMem(psess);

    if (NULL == (*ppdc = new SiteService))
    {
        return HR(E_OUTOFMEMORY);
    }

    (*ppdc)->m_psess = psess;
    psess->AddRef();

    return NOERROR;
}


HRESULT SiteService::QueryInterface(REFIID riid, void **ppvObj)
{
    AssertMem(ppvObj);
    CHECK_POINTER(ppvObj);
    if (IID_IUnknown == riid)
    {
        *ppvObj = (IUnknown *)this;
    }
    else if (IID_IServiceProvider == riid)
    {
        *ppvObj = (IServiceProvider *)this;
    }
    else
    {
        *ppvObj = NULL;
        return HR(E_NOINTERFACE);
    }

    AddRef();
    return NOERROR;
}


ULONG SiteService::AddRef(void)
{
    return InterlockedIncrement(&m_refCount);
}


ULONG SiteService::Release(void)
{
    long lw;

    if (0 == (lw = InterlockedDecrement(&m_refCount)))
    {
        delete this;
        return 0;
    }
    return lw;
}


HRESULT SiteService::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    ScriptEngine *pos;
    HRESULT hr = HR(E_NOINTERFACE);
    IUnknown *punkSite = NULL;
    IServiceProvider *pspSite = NULL;

    CHECK_POINTER(ppvObj);
    AssertMem(ppvObj);
    *ppvObj = NULL;

    // Only hold on to ScriptSite pointer
    if (NULL == m_psess || 
        NULL == (pos = m_psess->GetScriptEngine()) ||
        // PREFIX: dereferencing NULL pointer 'punkSite'  Use S_OK instead of FAILED() since
        // GetScriptSite may return S_FALSE and punkSite == NULL
        (S_OK != (hr = pos->GetScriptSite(IID_IUnknown, (void **)&punkSite))) ||
        FAILED(hr = punkSite->QueryInterface(IID_IServiceProvider, (void **)&pspSite))
        )
    {
        goto done;
    }

    hr = pspSite->QueryService(guidService, riid, ppvObj);

done:
    if (punkSite)
    {
        punkSite->Release();
    }
    if (pspSite)
    {
        pspSite->Release();
    }

    return hr;
}
