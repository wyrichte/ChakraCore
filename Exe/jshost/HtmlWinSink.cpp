//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

//
//  - CWinSink implements a simple object that traps MSHTML window events such as window::onload.
//    DHTML authors simply do the following in an HTML page:
//        function window_load()
//        {
//           alert("window::onload fired");
//        }
//
//        window.onload = window_load;
//
//    C++ developers do the following:
//        Instantiate a host object that implements IDispatch
//        QI an interface exposed by MSHTML for IConnectionPointContainer (ICPC)
//        Request a specific connection point via ICPC
//        Call Advise on the connect point, passing a reference to the host object
//
//        MSHTML will call the host's IDispatch::Invoke implementation when an event occurs.
//
//  - For a complete list of window events see dispinterface HTMLWindowEvents2 in mshtml.idl.
//


#include "StdAfx.h"

HRESULT CWinSink::Init(IHTMLWindow2* pWin)
{
  HRESULT hr = NOERROR;
  LPCONNECTIONPOINTCONTAINER pCPC = NULL;

  if (m_pWin)
  {
    m_pWin->Release();
  }
  m_pWin = pWin;
  m_pWin->AddRef();

  if (FAILED(hr = pWin->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&pCPC)))
  {
    goto Error;
  }

  if (FAILED(hr = pCPC->FindConnectionPoint(DIID_HTMLWindowEvents2, &m_pCP)))
  {
    goto Error;
  }

  m_hrConnected = m_pCP->Advise((LPUNKNOWN)this, &m_dwWindowEventCookie);

Error:
  if (pCPC) pCPC->Release();
  return hr;
}

STDMETHODIMP CWinSink::QueryInterface(REFIID riid, LPVOID* ppv)
{
  *ppv = NULL;

  if (IID_IUnknown == riid)
  {
    *ppv = (LPUNKNOWN)this;
    AddRef();
    return NOERROR;
  }
  else if (IID_IDispatch == riid)
  {
    *ppv = (IDispatch*)this;
    AddRef();
    return NOERROR;
  }
  else
  {
    OLECHAR wszBuff[39];
    int i = StringFromGUID2(riid, wszBuff, 39);
    Assert(i != 0);
    ODS(L"CWinSink QI: "); ODS(wszBuff); ODS(L"\n");
    return E_NOTIMPL;
  }
}

STDMETHODIMP_(ULONG) CWinSink::AddRef()
{
  WCHAR szBuff[255];
  swprintf_s(szBuff, 255, L"CWinSink refcount increased to %d\n", m_dwRef+1);
  ODS(szBuff);
  return ++m_dwRef;
}

STDMETHODIMP_(ULONG) CWinSink::Release()
{
  WCHAR szBuff[255];

  if (--m_dwRef == 0)
  {
    ODS(L"Deleting CWinSink\n");
    delete this;
    return 0;
  }

  swprintf_s(szBuff, 255, L"CWinSink refcount reduced to %d\n", m_dwRef);
  ODS(szBuff);
  return m_dwRef;
}

// OA events are fired through the IDispatch::Invoke of the sink object
STDMETHODIMP CWinSink::Invoke(DISPID dispIdMember,
            REFIID riid,
            LCID lcid,
            WORD wFlags,
            DISPPARAMS __RPC_FAR *pDispParams,
            VARIANT __RPC_FAR *pVarResult,
            EXCEPINFO __RPC_FAR *pExcepInfo,
            UINT __RPC_FAR *puArgErr)
{
  if (!pVarResult)
  {
    return E_POINTER;
  }

  int filenameIndex = 1;
  int lineNumberIndex = 0;
  int errorDescriptionIndex = 2;

  switch (dispIdMember)
  {
  case DISPID_HTMLSCRIPTEVENTS2_ONERROR:
      ODS(L"HTMLScriptEvents2::onerror fired\n");

      // In IE mode 10+ we are returned 4 arguments, shifting everything right by one.
      // pDispParams->rgvarg[0] is now the column number for the script error.
      if (4 == pDispParams->cArgs)
      {
          filenameIndex++;
          lineNumberIndex++;
          errorDescriptionIndex++;
      }
      // In IE 11 timeframe we are now returned 5 arguments, shifting everything to the right again.
      // pDispParams->rgvarg[0] is now VT_NULL.
      else if (5 == pDispParams->cArgs)
      {
          filenameIndex += 2;
          lineNumberIndex += 2;
          errorDescriptionIndex += 2;
      }

      // TODO: Add the column number for the error to the output here.
      fwprintf(stderr, L"Script error at %s(%d): %s\n",
          PathFindFileName(pDispParams->rgvarg[filenameIndex].bstrVal),
          pDispParams->rgvarg[lineNumberIndex].lVal,
          pDispParams->rgvarg[errorDescriptionIndex].bstrVal);
      return E_FAIL;

  default:
      return DISP_E_MEMBERNOTFOUND;
  }

  return NOERROR;
}
