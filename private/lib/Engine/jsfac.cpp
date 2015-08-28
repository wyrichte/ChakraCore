//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

// Description: defs for the jscript class factory

#include "StdAfx.h"
#pragma hdrstop

#include "guids.h"

OLECHAR szLangName[] = OLESTR("JScript");
LPOLESTR g_pszLangName = szLangName;
static const WCHAR* s_rgpszLang[] =
{
    L"JScript",
    L"LiveScript",
    L"JavaScript",
    L"JavaScript1.1",
    L"JavaScript1.2",
    L"JavaScript1.3",
    L"ECMAScript"
};

CJScript9ClassFactory::CJScript9ClassFactory(void)
        : CClassFactory(
        CLSID_Chakra, szCLSID_Chakra, L"JavaScript Language", NULL,
            L"Chakra.dll",
            NULL, 0, //Do not add ProgId for chakra.dll
            inCatActiveScript | inCatActiveScriptParse
          )
{
}

STDMETHODIMP CJScript9ClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvObj)
{
    HRESULT       hr = S_OK;
    ScriptEngine *  pos;

    CHECK_POINTER(ppvObj);
    *ppvObj = NULL;

    if (NULL != punkOuter)
        return CLASS_E_NOAGGREGATION;

    pos = AllocateEngine(szLangName);
    IFNULLMEMRET(pos);
    IfFailGo(pos->InitializeThreadBound());

    hr = pos->QueryInterface(riid, ppvObj);

Error:
    pos->Release();
    return hr;
}

ScriptEngine* CJScript9ClassFactory::AllocateEngine(LPOLESTR szLangName)
{
    return HeapNewNoThrow(ScriptEngine, GetTypeId(), szLangName);
}

CClassFactory * CreateJscript9ClassFactory(void)
{
    return HeapNewNoThrow(CJScript9ClassFactory);
}

CClassFactory* CreateJScript9ThreadServiceClassFactory()
{
    return HeapNewNoThrow(CJScript9ThreadServiceClassFactory);
}

STDMETHODIMP CJScript9ThreadServiceClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvObj)
{
    HRESULT       hr = S_OK;

    CHECK_POINTER(ppvObj);
    *ppvObj = NULL;

    if (NULL != punkOuter)
        return CLASS_E_NOAGGREGATION;

    JavascriptThreadService* threadService = HeapNewNoThrow(JavascriptThreadService);

    if (threadService == NULL)
    {
        return E_OUTOFMEMORY;
    }
    if (!threadService->Initialize())
    {
        HeapDelete(threadService);
        return E_OUTOFMEMORY;
    }

    hr = threadService->QueryInterface(riid, ppvObj);
    return hr;
}

CJScript9ThreadServiceClassFactory::CJScript9ThreadServiceClassFactory(void)
        : CClassFactory(
            CLSID_ChakraThreadService, szCLSID_ChakraThreadService, L"JavaScript Language", NULL,
            L"Chakra.dll",
            NULL, 0, 
            0
          )
{
}

class CJScript9DACClassFactory : public CClassFactory
{
public:
    CJScript9DACClassFactory() :
      CClassFactory(CLSID_JScript9DAC, NULL, NULL, NULL, NULL, NULL, 0, 0)
    {
    }

    REFIID GetTypeId(void) { return CLSID_JScript9DAC; }

    STDMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvObj)
    {
        CHECK_POINTER(ppvObj);
        *ppvObj = NULL;

        if (punkOuter != NULL)
        {
            return CLASS_E_NOAGGREGATION;
        }

        HRESULT hr = NOERROR;
        CComPtr<Js::ScriptDAC> pDAC;

        IFFAILRET(Js::ScriptDAC::CreateInstance(&pDAC));
        return pDAC->QueryInterface(riid, ppvObj);
    }
};

class CDiagHookClassFactory : public CClassFactory
{
public:
    CDiagHookClassFactory() :
      CClassFactory(CLSID_DiagHook, NULL, NULL, NULL, NULL, NULL, 0, 0)
    {
    }

    REFIID GetTypeId(void) { return CLSID_DiagHook; }

    STDMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvObj)
    {
        CHECK_POINTER(ppvObj);
        *ppvObj = NULL;

        if (punkOuter != NULL)
        {
            return CLASS_E_NOAGGREGATION;
        }

        HRESULT hr = NOERROR;
        CComPtr<JsDiag::DiagHook> pDiagHook;

        IFFAILRET(JsDiag::DiagHook::CreateInstance(&pDiagHook));
        return pDiagHook->QueryInterface(riid, ppvObj);
    }
};

CClassFactory * CreateJScript9DACClassFactory(void)
{
    return new CJScript9DACClassFactory();
}

CClassFactory * CreateDiagHookClassFactory(void)
{
    return new CDiagHookClassFactory();
}
