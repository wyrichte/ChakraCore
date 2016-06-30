#include "stdafx.h"

ScriptSite::ScriptSite() :
    m_refCount(0),
    m_dwNextSourceCookie(0),
    m_pActiveScript(NULL),
    m_pGlobalObject(NULL)
{
    this->AddRef();

    DEBUGPRINT("Constructed a ScriptSite (%x)\n", this);
}

ScriptSite::~ScriptSite()
{
    DEBUGPRINT("Destroyed a ScriptSite (%x)\n", this);
}

STDMETHODIMP ScriptSite::QueryInterface(REFIID riid, void ** ppvObj)
{
    QI_IMPL(IID_IUnknown, IActiveScriptSite);
    QI_IMPL(IID_IActiveScriptSite, IActiveScriptSite);
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ScriptSite::AddRef(void)
{
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) ScriptSite::Release(void)
{
    LONG res = InterlockedDecrement(&m_refCount);

    if (res == 0)
    {
        delete this;
    }

    return res;
}

STDMETHODIMP ScriptSite::GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti)
{
    HRESULT hr = S_OK;

    if (ppti)
    {
        *ppti = NULL;
    }

    //if (wcscmp(pstrName, _u("WScript")) == 0)
    //{
    //    if (!wscriptDispatch)
    //    {
    //        IActiveScript* activeScript;
    //        hr = GetActiveScript(&activeScript);
    //        IfFailedReturn(hr);
    //        wscriptDispatch = new WScriptDispatch(activeScript, ScriptSite::GetActiveScript);

    //        // WScriptDispatch takes its own ref count. Release this one.
    //        activeScript->Release();
    //    }
    //    hr = wscriptDispatch->QueryInterface(IID_IUnknown, (void**)ppiunkItem);
    //}
    //else
    //{
    //    hr = TYPE_E_ELEMENTNOTFOUND;
    //}

    return hr;
}

STDMETHODIMP ScriptSite::OnScriptError(IActiveScriptError * error)
{
    HRESULT hr = S_OK;
    DWORD sourceContext;
    ULONG lineNumber;
    LONG charPosition;
    EXCEPINFO exceptionInfo;
    hr = error->GetSourcePosition(&sourceContext, &lineNumber, &charPosition);
    if (SUCCEEDED(hr))
    {
        hr = error->GetExceptionInfo(&exceptionInfo);
    }
    if (FAILED(hr))
    {
        fwprintf(stderr, _u("An unknown error occured."));
        return hr;
    }
    fwprintf(stderr, _u("%s: %s\n"), exceptionInfo.bstrSource, exceptionInfo.bstrDescription);

    return S_OK;
}

HRESULT ScriptSite::Create(ScriptSite ** scriptSiteOut)
{
    HRESULT hr = S_OK;

    ScriptSite * scriptSite = new ScriptSite();

    // Create the script engine
    hr = scriptSite->CreateScriptEngine();
    
    if (SUCCEEDED(hr))
    {
        *scriptSiteOut = scriptSite;
    }
    else
    {
        if (scriptSite)
        {
            Shutdown(scriptSite);
        }
    }

    return hr;
}

HRESULT ScriptSite::Shutdown(ScriptSite * scriptSite)
{
    HRESULT hr = S_OK;

    scriptSite->AddRef();
    scriptSite->StopScriptEngine();
    scriptSite->Release();

    return hr;
}

HRESULT ScriptSite::StopScriptEngine()
{
    HRESULT hr = S_OK;

    if(m_pGlobalObject)
    {
        m_pGlobalObject->Release();
        m_pGlobalObject = NULL;
    }

    SCRIPTSTATE ss;
    hr = m_pActiveScript->GetScriptState(&ss);
    if (SUCCEEDED(hr) && ss != SCRIPTSTATE_CLOSED)
    {
        hr = m_pActiveScript->SetScriptState(SCRIPTSTATE_CLOSED);
    }
    m_pActiveScript->Release();
    m_pActiveScript = NULL;

    return hr;
}

HRESULT ScriptSite::CreateScriptEngine()
{
    IActiveScriptParse *activeScriptParse = NULL;
    IClassFactory * jscriptClassFactory = NULL;
    IDispatch * globalObjectDispatch = NULL;
    IDispatchEx * globalObjectDispatchEx = NULL;
    IActiveScriptProperty *activeScriptProperty = NULL;

    HRESULT hr = NOERROR;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        return hr;
    }

    if (!g_jscript9dll)
    {
        hr = CoCreateInstance(CLSID_Chakra, NULL, CLSCTX_INPROC_SERVER, _uuidof(IActiveScript), (LPVOID*)&m_pActiveScript);
        IfFailedGo(hr);
    }
    else
    {
        hr = pfDllGetClassObject(CLSID_Chakra, IID_IClassFactory, (LPVOID*)&jscriptClassFactory);
        IfFailedGo(hr);

        hr = jscriptClassFactory->CreateInstance(NULL, _uuidof(IActiveScript), (LPVOID*)&m_pActiveScript);        
        IfFailedGo(hr);
    }

    hr = m_pActiveScript->QueryInterface(__uuidof(IActiveScriptProperty), (LPVOID*)&activeScriptProperty);
    IfFailedGo(hr);

    VARIANT prop;
    VariantInit(&prop);

    prop.vt = VT_I4;
    prop.lVal = SCRIPTLANGUAGEVERSION_5_10;
    hr = activeScriptProperty->SetProperty(SCRIPTPROP_INVOKEVERSIONING, 0, &prop);
    IfFailedGo(hr);


    prop.vt = VT_I4;
    prop.lVal = SCRIPTHOSTTYPE_APPLICATION;
    hr = activeScriptProperty->SetProperty(SCRIPTPROP_HOSTTYPE, 0, &prop);
    IfFailedGo(hr);

    hr = m_pActiveScript->SetScriptSite(this);
    IfFailedGo(hr);

    hr = m_pActiveScript->QueryInterface(__uuidof(IActiveScriptParse), (LPVOID*)&activeScriptParse);
    IfFailedGo(hr);

    hr = activeScriptParse->InitNew();
    IfFailedGo(hr);

    hr = m_pActiveScript->SetScriptState(SCRIPTSTATE_STARTED);
    IfFailedGo(hr);

    hr = m_pActiveScript->GetScriptDispatch(NULL, &m_pGlobalObject);
    IfFailedGo(hr);

    //hr = globalObjectDispatch->QueryInterface(IID_IDispatchEx, (void**)&globalObjectDispatchEx);
    //IfFailedGo(hr);
   
LReturn:
    if (FAILED(hr))
    {
        if (m_pGlobalObject)
        {
            m_pGlobalObject->Release();
        }
        if (m_pActiveScript)
        {
            m_pActiveScript->SetScriptState(SCRIPTSTATE_CLOSED);
            m_pActiveScript->Release();
        }
    }

    //if (globalObjectDispatchEx)
    //{
    //    globalObjectDispatchEx->Release();
    //}

    //if (globalObjectDispatch)
    //{
    //    globalObjectDispatch->Release();
    //}

    if(activeScriptProperty)
    {
        activeScriptProperty->Release();
    }

    if (activeScriptParse)
    {
        activeScriptParse->Release();
    }

    if (jscriptClassFactory)
    {
        jscriptClassFactory->Release();
    }

    CoUninitialize();

    return hr;
}

HRESULT ScriptSite::GetActiveScript(IActiveScript ** activeScript)
{
    *activeScript = m_pActiveScript;
    return S_OK;
}

HRESULT ScriptSite::ParseScript(LPCOLESTR contents)
{
    IActiveScriptParse * activeScriptParse = NULL;
    HRESULT hr = m_pActiveScript->QueryInterface(__uuidof(IActiveScriptParse), (LPVOID*)&activeScriptParse);
    if (SUCCEEDED(hr))
    {
        DWORD dwSourceCookie = m_dwNextSourceCookie++;
        EXCEPINFO excepinfo;
        hr = activeScriptParse->ParseScriptText(contents, NULL, NULL, NULL, dwSourceCookie, 0, SCRIPTTEXT_HOSTMANAGESSOURCE, NULL, &excepinfo);
        activeScriptParse->Release();
    }
    return hr;
}

HRESULT ScriptSite::GetGlobalValue(LPCOLESTR pszStr, int *val)
{
    HRESULT hr = S_OK;
    
    DISPPARAMS params = {0};
    VARIANT varOut;
    EXCEPINFO ei;

    DISPID dispid;

    IfFailGo(GetDispID(pszStr, &dispid));

    VariantInit(&varOut);

    IfFailGo(m_pGlobalObject->Invoke(
        dispid,
        IID_NULL,
        1033,
        DISPATCH_PROPERTYGET,
        &params,
        &varOut,
        &ei,
        NULL));

    // HACK
    *val = varOut.intVal;

Error:
    return hr;
}

HRESULT ScriptSite::InvokeGlobalFunction(LPCOLESTR pszStr)
{
    HRESULT hr = S_OK;
    DISPPARAMS params = {0};
    EXCEPINFO ei;
    DISPID dispid;

    IfFailGo(GetDispID(pszStr, &dispid));

    IfFailGo(m_pGlobalObject->Invoke(
        dispid,
        IID_NULL,
        1033,
        DISPATCH_METHOD,
        &params,
        NULL,
        &ei,
        NULL));

Error:
    return hr;
}

HRESULT ScriptSite::GetDispID(LPCOLESTR pszStr, DISPID *dispid)
{
    HRESULT hr = S_OK;
    if(m_DISPIDs.find(pszStr) != m_DISPIDs.end())
    {
        *dispid = m_DISPIDs[pszStr];
    }
    else
    {
        *dispid = 0;
        hr = m_pGlobalObject->GetIDsOfNames(
            IID_NULL,
            (LPOLESTR*)&pszStr,
            1,
            1033,
            dispid);

        if(SUCCEEDED(hr))
        {
            m_DISPIDs[pszStr] = *dispid;
        }
    }
    return hr;
}
