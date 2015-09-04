/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "HostCommonPch.h"
#include "Wscript.h"
#include <mshtmcid.h>

Dispatch::Dispatch()
    : refCount(1)
{
}

STDMETHODIMP Dispatch::QueryInterface(REFIID riid, void ** ppvObj)
{
    QI_IMPL(IID_IUnknown, IDispatchEx);
    QI_IMPL(__uuidof(IDispatch), IDispatchEx);
    QI_IMPL(__uuidof(IDispatchEx), IDispatchEx);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) Dispatch::AddRef() 
{ 
    return InterlockedIncrement(&refCount); 
}

STDMETHODIMP_(ULONG) Dispatch::Release()
{
    LONG res = InterlockedDecrement(&refCount);

    if (res == 0)
    {
        delete this;
    }

    return res;
}

STDMETHODIMP Dispatch::GetIDsOfNames(__in REFIID /*riid*/, __in_ecount(cpsz) LPOLESTR * prgpsz, __in UINT cpsz, __in LCID /*lcid*/, __inout_ecount(cpsz) DISPID * prgid)
{
    for (unsigned int i = 0; i < cpsz; i++)
    {
        prgid[i] = DISPID_UNKNOWN;
    }

    return GetDispID(prgpsz[0], fdexNameCaseInsensitive, &prgid[0]);
}

STDMETHODIMP Dispatch::Invoke(DISPID id, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei, UINT * /*puArgErr*/)
{
    if (IID_NULL != riid)
    {
        if (NULL != pvarRes)
        {
            VariantInit(pvarRes);
        }
        if (NULL != pei)
        {
            memset(pei, 0, sizeof(*pei));
        }
        return DISP_E_UNKNOWNINTERFACE;
    }

    return InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, NULL);
}

WScriptDispatch::WScriptDispatch(IActiveScript* activeScript, GetActiveScriptCallback pfGetActiveScriptCallback, NotifyCallback pfSetKeepAliveCallback, NotifyCallback pfQuitCallback, AttachDetachCallback pfAttachDetachCallback)
    : Dispatch(), 
    m_activeScript(activeScript),
    m_pfGetActiveScriptCallback(pfGetActiveScriptCallback),
    m_pfSetKeepAliveCallback(pfSetKeepAliveCallback),
    m_pfQuitCallback(pfQuitCallback),
    m_pfAttachDetachCallback(pfAttachDetachCallback),
    m_DiagnosticsHelperSite(NULL)
{
    if (m_activeScript)
    {
        m_activeScript->AddRef();
    }
}

WScriptDispatch::~WScriptDispatch() 
{
    if (m_activeScript)
    {
        m_activeScript->Release();
        m_activeScript = NULL;
    }
}

STDMETHODIMP WScriptDispatch::GetDispID(BSTR bstrName, DWORD grfdex, DISPID * pid)
{
    HRESULT hr = S_OK;

    if(((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"Echo") == 0) || 
       ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"Echo") == 0))
    {
        *pid = DISPID_Echo;
    }
    else if(((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"Quit") == 0) || 
            ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"Quit") == 0))
    {
        *pid = DISPID_Quit;
    }
    else if(((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"LoadScriptFile") == 0) || 
            ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"LoadScriptFile") == 0))
    {
        *pid = DISPID_LoadScriptFile;
    }
    else if(((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"GetWorkingSet") == 0) || 
            ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"GetWorkingSet") == 0))
    {
        *pid = DISPID_GetWorkingSet;
    }
    else if(((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"SetKeepAlive") == 0) || 
            ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"SetKeepAlive") == 0))
    {
        *pid = DISPID_SetKeepAlive;
    }
    else if(((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"Attach") == 0) || 
            ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"Attach") == 0))
    {
        *pid = DISPID_Attach;
    }
    else if(((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"Detach") == 0) || 
            ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"Detach") == 0))
    {
        *pid = DISPID_Detach;
    }
    else if(((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"StartProfiling") == 0) || 
            ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"StartProfiling") == 0))
    {
        *pid = DISPID_StartProfiling;
    }
    else if (((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"StopProfiling") == 0) ||
        ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"StopProfiling") == 0))
    {
        *pid = DISPID_StopProfiling;
    }
    else if (((grfdex & fdexNameCaseInsensitive) && _wcsicmp(bstrName, L"PerformSourceRundown") == 0) ||
        ((grfdex & fdexNameCaseSensitive) && wcscmp(bstrName, L"PerformSourceRundown") == 0))
    {
        *pid = DISPID_PerformSourceRundown;
    }
    else
    {
        *pid = DISPID_UNKNOWN;
        hr = DISP_E_UNKNOWNNAME;
    }

    return hr;
}

STDMETHODIMP WScriptDispatch::GetMemberName(DISPID id, BSTR * pbstrName)
{
    HRESULT hr = S_OK;

    switch(id)
    {
    case DISPID_Echo:
        *pbstrName = SysAllocString(L"Echo");
        break;

    case DISPID_Quit:
        *pbstrName = SysAllocString(L"Quit");
        break;

    case DISPID_LoadScriptFile:
        *pbstrName = SysAllocString(L"LoadScriptFile");
        break;

    case DISPID_GetWorkingSet:
        *pbstrName = SysAllocString(L"GetWorkingSet");
        break;

    case DISPID_SetKeepAlive:
        *pbstrName = SysAllocString(L"SetKeepAlive");
        break;

    case DISPID_Attach:
        *pbstrName = SysAllocString(L"Attach");
        break;

    case DISPID_Detach:
        *pbstrName = SysAllocString(L"Detach");
        break;

    case DISPID_PerformSourceRundown:
        *pbstrName = SysAllocString(L"PerformSourceRundown");
        break;

    case DISPID_StartProfiling:
        *pbstrName = SysAllocString(L"StartProfiling");
        break;

    case DISPID_StopProfiling:
        *pbstrName = SysAllocString(L"StopProfiling");
        break;

    default:
        *pbstrName = NULL;
        hr = DISP_E_UNKNOWNNAME;
    }

    return hr;
}

STDMETHODIMP WScriptDispatch::GetNextDispID(DWORD /*grfdex*/, DISPID id, DISPID * pid)
{
    HRESULT hr = S_OK;

    if (id == DISPID_MaxValue)
    {
        hr = S_FALSE;
    }
    else
    {
        *pid = id + 1;
    }

    return hr;
}

STDMETHODIMP WScriptDispatch::InvokeEx(DISPID id, LCID /*lcid*/, WORD wFlags, DISPPARAMS * pdp, VARIANT * pVarRes, EXCEPINFO * /*pei*/, IServiceProvider * pspCaller)
{
    HRESULT hr = S_OK;

    switch(id)
    {
    case DISPID_Echo:
        
        if (wFlags & DISPATCH_METHOD)
        {
            hr = WScriptDispatch::Echo(stdout, pdp, pspCaller);
        }
        else if (wFlags & DISPATCH_PROPERTYGET)
        {
            EchoDispatch * echoDispatch = new EchoDispatch(this);
            VariantInit(pVarRes);
            pVarRes->vt = VT_DISPATCH;
            pVarRes->pdispVal = echoDispatch;
        }
        break;
    case DISPID_GetWorkingSet:
        if ((wFlags & (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) != (DISPATCH_METHOD | DISPATCH_PROPERTYGET))
        {
            return E_INVALIDARG;
        }
        hr = GetWorkingSetFromActiveScript<IActiveScript>(m_activeScript, pVarRes);
        break;
    case DISPID_Quit:
        if (((wFlags & DISPATCH_METHOD) != DISPATCH_METHOD) || this->m_pfQuitCallback == nullptr)
        {
            return E_INVALIDARG;
        }
        this->m_pfQuitCallback();
        hr = S_OK;
        break;
    case DISPID_SetKeepAlive:
        if (((wFlags & DISPATCH_METHOD) != DISPATCH_METHOD) || this->m_pfSetKeepAliveCallback == nullptr)
        {
            return E_INVALIDARG;
        }
        this->m_pfSetKeepAliveCallback();
        hr = S_OK;
        break;

    case DISPID_Attach:
        if (((wFlags & DISPATCH_METHOD) != DISPATCH_METHOD))
        {
            return E_INVALIDARG;
        }
        hr = DebugAttach(pdp);
        break;

    case DISPID_Detach:
        if (((wFlags & DISPATCH_METHOD) != DISPATCH_METHOD))
        {
            return E_INVALIDARG;
        }
        hr = DebugDetach(pdp);
        break;

    case DISPID_PerformSourceRundown:
        if (((wFlags & DISPATCH_METHOD) != DISPATCH_METHOD))
        {
            return E_INVALIDARG;
        }
        hr = DebugSourceRundown();
        break;

    case DISPID_StartProfiling:
        if (((wFlags & DISPATCH_METHOD) != DISPATCH_METHOD))
        {
            return E_INVALIDARG;
        }
        hr = ScriptProfilerStart(pdp);
        break;

    case DISPID_StopProfiling:
        if (((wFlags & DISPATCH_METHOD) != DISPATCH_METHOD))
        {
            return E_INVALIDARG;
        }
        hr = ScriptProfilerStop(pdp);
        break;

    default:
        hr = DISP_E_MEMBERNOTFOUND;
    }

    return hr;
}

HRESULT WScriptDispatch::ValidCall(DISPPARAMS * pdp, IDispatch** ppArgNoRef, bool optional/* = false*/)
{
    return (m_DiagnosticsHelperSite != nullptr) ? ValidateDispatch(pdp, ppArgNoRef, optional) : E_FAIL;
}

HRESULT WScriptDispatch::ValidateDispatch(DISPPARAMS * pdp, IDispatch** ppArgNoRef, bool optional)
{
    Assert(pdp);
    if ((pdp->cArgs == 1) && (pdp->rgvarg[0].vt == VT_DISPATCH))
    {
        *ppArgNoRef = pdp->rgvarg[0].pdispVal;
        return S_OK;
    }
    else if (optional && pdp->cArgs == 0)
    {
        *ppArgNoRef = nullptr;
        return S_OK;
    }
    Assert(false);
    return E_FAIL;
}

HRESULT WScriptDispatch::DebugAttach(DISPPARAMS * pdp)
{
    IDispatch* pDispNoRef = nullptr;
    if (m_pfAttachDetachCallback != nullptr
        && ValidateDispatch(pdp, &pDispNoRef) == S_OK)
    {
        m_pfAttachDetachCallback(IDM_DEBUGGERDYNAMICATTACH, pDispNoRef);
        return S_OK;
    }

    if (ValidCall(pdp, &pDispNoRef) == S_OK)
    {
        return m_DiagnosticsHelperSite->SetDebugAttachFunc(pDispNoRef);
    }
    return E_FAIL;
}

HRESULT WScriptDispatch::DebugDetach(DISPPARAMS * pdp)
{
    IDispatch* pDispNoRef = nullptr;
    if (m_pfAttachDetachCallback != nullptr
        && ValidateDispatch(pdp, &pDispNoRef) == S_OK)
    {
        m_pfAttachDetachCallback(IDM_DEBUGGERDYNAMICDETACH, pDispNoRef);
        return S_OK;
    }

    if (ValidCall(pdp, &pDispNoRef) == S_OK)
    {
        return m_DiagnosticsHelperSite->SetDebugDetachFunc(pDispNoRef);
    }
    return E_FAIL;
}

HRESULT WScriptDispatch::DebugSourceRundown()
{
    if (m_pfAttachDetachCallback != nullptr)
    {
        m_pfAttachDetachCallback(IDM_DEBUGGERDYNAMICATTACHSOURCERUNDOWN, NULL/*no function to call*/);
    }

    return S_OK;
}

HRESULT WScriptDispatch::ScriptProfilerStart(DISPPARAMS * pdp)
{
    IDispatch* pDispNoRef = nullptr;
    if (ValidCall(pdp, &pDispNoRef) == S_OK)
    {
        return m_DiagnosticsHelperSite->SetProfilerStartFunc(pDispNoRef);
    }
    return E_FAIL;
}

HRESULT WScriptDispatch::ScriptProfilerStop(DISPPARAMS * pdp)
{
    IDispatch* pDispNoRef = nullptr;
    if (ValidCall(pdp, &pDispNoRef, /*optional*/true) == S_OK)
    {
        return m_DiagnosticsHelperSite->SetProfilerStopFunc(pDispNoRef);
    }
    return E_FAIL;
}

HRESULT WScriptDispatch::Echo(FILE * stream, DISPPARAMS * pdp, IServiceProvider * pspCaller)
{
    HRESULT hr = S_OK;

    IActiveScript * activeScript = NULL;
    if (m_pfGetActiveScriptCallback)
    {
        hr = m_pfGetActiveScriptCallback(pspCaller, &activeScript);
    }
    else
    {
        activeScript = m_activeScript;
        m_activeScript->AddRef();
    }
    Assert(activeScript);
    if (SUCCEEDED(hr))
    {
        IVariantChangeType * variantChangeType = NULL;
        hr = activeScript->QueryInterface(__uuidof(IVariantChangeType), (void**)&variantChangeType);
        if (SUCCEEDED(hr))
        {
            for (unsigned int i = pdp->cNamedArgs; i < pdp->cArgs; i++)
            {
                VARIANT stringValue;
                VariantInit(&stringValue);
                hr = variantChangeType->ChangeType(&stringValue, &pdp->rgvarg[pdp->cArgs - i - 1], GetThreadLocale(), VT_BSTR);
                if (SUCCEEDED(hr))
                {
                    if (i > 0)
                    {
                        fwprintf(stream, L" ");
                    }
                    fwprintf(stream, L"%ls", stringValue.bstrVal);
                    VariantClear(&stringValue);
                }
            }
            fwprintf(stream, L"\n");
            fflush(stream);
            variantChangeType->Release();
        }
        activeScript->Release();
    }
    return hr;
}

HRESULT WScriptDispatch::Initialize(IActiveScript * activeScript)
{
    HRESULT hr = S_OK;

    hr = activeScript->AddNamedItem(L"WScript", SCRIPTITEM_ISVISIBLE);

    return hr;
}

STDMETHODIMP WScriptDispatch::EchoDispatch::InvokeEx(DISPID id, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS * pdp, VARIANT * /*pVarRes*/, EXCEPINFO * /*pei*/, IServiceProvider * pspCaller)
{
    HRESULT hr = S_OK;

    if (id != DISPID_VALUE)
    {
        hr = DISP_E_MEMBERNOTFOUND;
    }

    m_container->Echo(stdout, pdp, pspCaller);

    return hr;
}

JsValueRef __stdcall EchoCallback(JsValueRef /*callee*/, bool /*isConstructCall*/, JsValueRef *arguments, unsigned short argumentCount, void * /*callbackState*/)
{
    for (unsigned int i = 1; i < argumentCount; i++)
    {          
        if (i > 1)
        {
            wprintf(L" ");
        }
        JsValueRef strValue;
        if (JScript9Interface::JsrtConvertValueToString(arguments[i], &strValue) == JsNoError) 
        {
            LPCWSTR str = NULL;
            size_t length;
            if (JScript9Interface::JsrtStringToPointer(strValue, &str, &length) == JsNoError) 
            {
                wprintf(L"%s", str);
            }
        }
    }

    wprintf(L"\n");        

    JsValueRef undefinedValue;
    JScript9Interface::JsrtGetUndefinedValue(&undefinedValue);
    return undefinedValue;
}
