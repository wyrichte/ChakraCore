//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

using namespace std;

class CWinSink : public IDispatch
{
public:
    CWinSink() : m_pWin(NULL), m_dwRef(1),
            m_hrConnected(CONNECT_E_CANNOTCONNECT),
            m_dwWindowEventCookie(0), m_pCP(NULL)
    {
    }

    ~CWinSink()
    {

    }

    HRESULT Init(IHTMLWindow2* pWin);
    HRESULT Passivate()
    {
        HRESULT hr = NOERROR;
        if (m_pCP)
        {
            if (m_dwWindowEventCookie)
            {
                hr = m_pCP->Unadvise(m_dwWindowEventCookie);
                m_dwWindowEventCookie = 0;
            }

            m_pCP->Release();
            m_pCP = NULL;
        }

        if (m_pWin)
        {
            m_pWin->Release();
            m_pWin = NULL;
        }

        return NOERROR;
    }

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)(); 

    // IDispatch method
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
            { ODS(_u("GetTypeInfoCount\n")); return E_NOTIMPL; }

    STDMETHOD(GetTypeInfo)(UINT iTInfo,
            LCID lcid,
            ITypeInfo** ppTInfo)
            { ODS(_u("GetTypeInfo\n")); return E_NOTIMPL; }

    STDMETHOD(GetIDsOfNames)(REFIID riid, 
            __in_ecount(cNames) LPOLESTR * rgszNames,
            UINT cNames,
            LCID lcid,
            DISPID* rgDispId)
            { ODS(_u("GetIDsOfNames\n")); return E_NOTIMPL; }
        
    STDMETHOD(Invoke)(DISPID dispIdMember,
            REFIID riid,
            LCID lcid,
            WORD wFlags,
            DISPPARAMS *pDispParams,
            VARIANT *pVarResult,
            EXCEPINFO *pExcepInfo,
            UINT *puArgErr);

private:
    IHTMLWindow2* m_pWin;
    DWORD m_dwRef;

    LPCONNECTIONPOINT m_pCP;
    HRESULT m_hrConnected;
    DWORD m_dwWindowEventCookie;
};