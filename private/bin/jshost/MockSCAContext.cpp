/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

HRESULT MockSCAContext::Init(SCAContextType context, IUnknown* target)
{
    m_context = context;
    m_target = target;
    return S_OK;
}

STDMETHODIMP MockSCAContext::QueryInterface(REFIID riid, void** ppvObj)
{
    IfNullReturnError(ppvObj, E_POINTER);

    QI_IMPL_INTERFACE(IUnknown);
    QI_IMPL_INTERFACE(ISCAContext);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP MockSCAContext::GetContext(SCAContextType* pContext)
{
    IfNullReturnError(pContext, E_POINTER);

    *pContext = m_context;
    return S_OK;
}

STDMETHODIMP MockSCAContext::GetTarget(IUnknown** ppTarget)
{
    IfNullReturnError(ppTarget, E_POINTER);

    if (m_target)
    {
        return m_target->QueryInterface(ppTarget);
    }

    *ppTarget = NULL;
    return S_OK;
}

STDMETHODIMP MockSCAContext::SetDependentObject(__in IUnknown *obj)
{
    if (m_dependentObject != nullptr)
    {
        return E_FAIL;
    }

    m_dependentObject = obj;
    return S_OK;
}

STDMETHODIMP MockSCAContext::GetDependentObject(__out IUnknown **obj)
{
    if (m_dependentObject != nullptr)
    {
        // Quick work around because m_dependentObject.CopyTo was giving build errors
        *obj = m_dependentObject;
        (*obj)->AddRef();
    }
    
    return S_OK;
}
