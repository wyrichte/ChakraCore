/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
// ScpNode.cpp : Implementation of CProcDMApp and DLL registration.

#include "EnginePch.h"
#pragma hdrstop

/////////////////////////////////////////////////////////////////////////////
//
CComEngineModule _Module;

CStandardScriptSourceNode::CStandardScriptSourceNode(void)
{
    _Module.Lock();
    m_refCount = 1;
    m_pdan = NULL;
    m_pszShort = NULL;
    m_pszLong = NULL;
    m_bstrUrl = NULL;
}


HRESULT CStandardScriptSourceNode::Close(void)
{
    if (NULL != m_pdan)
    {
        IDebugApplicationNode *pdan = m_pdan;
        m_pdan = NULL;
        pdan->Close();
        pdan->Release();
    }
    if (NULL != m_pszShort)
    {
        free(m_pszShort);
        m_pszShort = NULL;
    }
    if (NULL != m_pszLong)
    {
        free(m_pszLong);
        m_pszLong = NULL;
    }
    if (NULL != m_bstrUrl)
    {
        ::SysFreeString(m_bstrUrl);
        m_bstrUrl = NULL;
    }
    return NOERROR;
}


CStandardScriptSourceNode::~CStandardScriptSourceNode(void)
{
    Close();
    _Module.Unlock();
}


HRESULT CStandardScriptSourceNode::SetNode(IDebugDocumentProvider *pddp,
    IDebugApplicationNode *pdan)
{
    if (NULL != pdan)
        pdan->AddRef();
    if (m_pdan)
    {
        m_pdan->Close();
        m_pdan->Release();
    }
    m_pdan = pdan;

    if (NULL != m_pdan)
        m_pdan->SetDocumentProvider(pddp);
    return NOERROR;
}


HRESULT CStandardScriptSourceNode::GetNode(IDebugApplicationNode **ppdan)
{
    CHECK_POINTER(ppdan);
    *ppdan = m_pdan;
    if (NULL == m_pdan)
        return HR(E_NOINTERFACE);
    m_pdan->AddRef();
    return NOERROR;
}


HRESULT CStandardScriptSourceNode::QueryInterface(REFIID iid, void ** ppvObject)
{
    CHECK_POINTER(ppvObject);

    if (InlineIsEqualGUID(iid, IID_IUnknown))
        *ppvObject = (IUnknown *)this;
    else
    {
        *ppvObject = NULL;
        return HR(E_NOINTERFACE);
    }

    AddRef();
    return NOERROR;
}


HRESULT CStandardScriptSourceNode::GetName(DOCUMENTNAMETYPE dnt, BSTR *pbstr)
{
    CHECK_POINTER(pbstr);

    *pbstr = NULL;

    switch (dnt) {
        case DOCUMENTNAMETYPE_APPNODE:
        case DOCUMENTNAMETYPE_FILE_TAIL:
            *pbstr = SysAllocString(m_pszShort);
            break;
        case DOCUMENTNAMETYPE_TITLE:
            *pbstr = SysAllocString(m_pszShort);
            break;
        case DOCUMENTNAMETYPE_UNIQUE_TITLE:
            *pbstr = SysAllocString(m_pszLong);
            break;
        case DOCUMENTNAMETYPE_URL:
            if (m_bstrUrl)
            {
                *pbstr = SysAllocString(m_bstrUrl);
                break;
            }
            // fall through and return error
        default:
            return E_FAIL;
    }

    if (NULL == *pbstr)
        return HR(E_OUTOFMEMORY);
    return NOERROR;
}


HRESULT CStandardScriptSourceNode::SetLongName(LPCOLESTR psz)
{
    if (NULL != m_pszLong)
    {
        free(m_pszLong);
        m_pszLong = NULL;
    }
    if (NULL == psz)
        return NOERROR;
    if (NULL == (m_pszLong = ostrdup(psz)))
        return HR(E_OUTOFMEMORY);
    return NOERROR;
}


HRESULT CStandardScriptSourceNode::SetShortName(LPCOLESTR psz)
{
    if (NULL != m_pszShort)
    {
        free(m_pszShort);
        m_pszShort = NULL;
    }
    if (NULL == psz)
        return NOERROR;
    if (NULL == (m_pszShort = ostrdup(psz)))
        return HR(E_OUTOFMEMORY);
    return NOERROR;
}
