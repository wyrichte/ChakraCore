/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"
#pragma hdrstop


DispatchExCaller::DispatchExCaller(void)
    :refCount(1),
    scriptSite(NULL),
    m_punkCaller(NULL),
    m_punkSite(NULL),
    m_fQueriedCaller(false),
    m_fQueriedSite(false),
    m_pspCaller(NULL),
    m_pspSite(NULL)
{
}


DispatchExCaller::~DispatchExCaller(void)
{
    Close();
}


HRESULT DispatchExCaller::Close(void)
{
    if (NULL != scriptSite)
        {
        scriptSite->Release();
        scriptSite = NULL;
        }
    if (NULL != m_punkCaller)
        {
        m_punkCaller->Release();
        m_punkCaller = NULL;
        }
    if (NULL != m_punkSite)
        {
        m_punkSite->Release();
        m_punkSite = NULL;
        }
    if (NULL != m_pspCaller)
        {
        m_pspCaller->Release();
        m_pspCaller = NULL;
        }
    if (NULL != m_pspSite)
        {
        m_pspSite->Release();
        m_pspSite = NULL;
        }
    return NOERROR;
}

DispatchExCaller::DispatchExCaller(__in DispatchExCaller* other)
    :refCount(1),
    m_fQueriedCaller(FALSE),
    m_fQueriedSite(FALSE),
    m_pspCaller(FALSE),
    m_pspSite(NULL)
{
    scriptSite = other->scriptSite;
    scriptSite->AddRef();

    m_punkCaller = other->m_punkCaller;
    if (m_punkCaller)
    {
        m_punkCaller->AddRef();
    }

    m_punkSite = other->m_punkSite;
    if (m_punkSite)
    {
        m_punkSite->AddRef();
    }


}

HRESULT DispatchExCaller::Create(
        __in ScriptSite *scriptSite,
        __in IUnknown *punkCaller,
        __out DispatchExCaller **dispatchCaller)

{
    IfNullReturnError(dispatchCaller, E_POINTER);
    *dispatchCaller = nullptr;

    ScriptEngine *oleScript;

    if (NULL == (*dispatchCaller = HeapNewNoThrow(DispatchExCaller)))
    {
        return E_OUTOFMEMORY;
    }

    (*dispatchCaller)->scriptSite = scriptSite;
    scriptSite->AddRef();

    if (NULL != punkCaller)
        {
        (*dispatchCaller)->m_punkCaller = punkCaller;
        punkCaller->AddRef();
        }
    if (NULL == (oleScript = scriptSite->GetScriptEngine()) ||
        FAILED(oleScript->GetScriptSite(IID_IUnknown, (void **)&(*dispatchCaller)->m_punkSite)))
        {
        (*dispatchCaller)->m_punkSite = NULL;
        }

    return NOERROR;
}


HRESULT DispatchExCaller::QueryInterface(REFIID riid, void **ppvObj)
{
    AssertMem(ppvObj);

    CHECK_POINTER(ppvObj);
    if (IID_IUnknown == riid ||
        IID_IJavascriptLocalProxy == riid)
        *ppvObj = (IServiceProvider *)this;
    else if (__uuidof(IServiceProvider) == riid)
        *ppvObj = (IServiceProvider *)this;
    else if (IID_ICanHandleException == riid)
        *ppvObj = (ICanHandleException *)this;
    else if (IID_IProvideRuntimeContext == riid)
        *ppvObj = (IProvideRuntimeContext *)this;
    else
    {
        *ppvObj = NULL;
        return HR(E_NOINTERFACE);
    }

    AddRef();
    return NOERROR;
}

ULONG DispatchExCaller::AddRef(void)
{
    return InterlockedIncrement(&refCount);
}


ULONG DispatchExCaller::Release(void)
{
    long currentCount;

    if (0 == (currentCount = InterlockedDecrement(&refCount)))
        {
        delete this;
        }
    return currentCount;
}


HRESULT DispatchExCaller::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    ScriptEngine *pos;
    HRESULT hr;

    CHECK_POINTER(ppvObj);
    AssertMem(ppvObj);
    *ppvObj = NULL;

    if (NULL == scriptSite)
        return HR(E_UNEXPECTED);

    if (guidService == SID_GetCaller)
        {
        if (NULL == m_punkCaller)
            return NOERROR;
        return m_punkCaller->QueryInterface(riid, ppvObj);
        }
    if (guidService == SID_GetScriptSite)
        {
        if (NULL == m_punkSite)
            return NOERROR;
        return m_punkSite->QueryInterface(riid, ppvObj);
        }

    if (guidService == SID_VariantConversion)
        {
        if (NULL == (pos = scriptSite->GetScriptEngine()))
            return HR(E_UNEXPECTED);
        return pos->QueryInterface(riid, ppvObj);
        }
       if (guidService == SID_ProvideRuntimeContext)
       {
            return QueryInterface(riid, ppvObj);
       }

     //Try to delegate to the caller, then the site.
     //Note that if the caller does support QueryService
     //we do not delegate to the site even if the caller
     //does not support the service.
    
    hr = QSCaller(guidService, riid, ppvObj);
    if (NULL != m_pspCaller)
        return hr;
    else 
        return QSSite(guidService, riid, ppvObj);
}

HRESULT DispatchExCaller::QSCaller(REFGUID guidService, REFIID riid, void **ppvObj)
{
    CHECK_POINTER(ppvObj);
    if (!m_fQueriedCaller)
        {
        Assert(NULL == m_pspCaller);
        if (NULL != m_punkCaller &&
            FAILED(m_punkCaller->QueryInterface(__uuidof(IServiceProvider),
                (void **)&m_pspCaller)))
            {
            m_pspCaller = NULL;
            }
        m_fQueriedCaller = true;
        }

    if (NULL != m_pspCaller)
        return m_pspCaller->QueryService(guidService, riid, ppvObj);
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

HRESULT DispatchExCaller::QSSite(REFGUID guidService, REFIID riid, void **ppvObj)
{
    CHECK_POINTER(ppvObj);
    if (!m_fQueriedSite)
        {
        Assert(NULL == m_pspSite);
        if (NULL != m_punkSite &&
            FAILED(m_punkSite->QueryInterface(__uuidof(IServiceProvider),
                (void **)&m_pspSite)))
            {
            m_pspSite = NULL;
            }
        m_fQueriedSite = true;
        }

    if (NULL != m_pspSite)
        return m_pspSite->QueryService(guidService, riid, ppvObj);
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

    /****************************************
        ICanHandleException method
    ****************************************/

// In current engine we have a top level try_catch block for all JS exceptions raised from CallRootFunction.
// these "leaked" exceptions are caught and translate to JSErr_UncaughtException. So now
// we don't really need this flag any more as we can populate the exception between the boundaries between
// DOM and JS engine.
// However we need this for cross thread/cross process calls. 
// In this code path, we are likely in a HostDispatch call to other engine or remote process call. We should 
// record the exception here, and let the call return back to us later with SCRIPT_E_PROPAGATE error, and we'll
// retrieve and throw the exception at that time. We shouldn't throw here as we are likely being called from 
// different thread/different engine. 
HRESULT STDMETHODCALLTYPE DispatchExCaller::CanHandleException(
    /* [in] */ __RPC__in EXCEPINFO *pExcepInfo,
    /* [in] */ __RPC__in VARIANT *pvar) 
{
    HRESULT hr = NOERROR;
    if (NULL == pExcepInfo)
    {
        return E_INVALIDARG;
    }
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    if (!scriptContext->GetThreadContext()->IsStackAvailableNoThrow())
    {
        return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
    }
    Js::JavascriptExceptionObject* exceptionObject = NULL;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if (pvar != NULL)
        {
            // assuming this is the exception object, and we need to create hostdispatch wrapper around it. 
            Js::Var errorObject;
            hr = DispatchHelper::MarshalVariantToJsVar(pvar, &errorObject, scriptContext);
            if (SUCCEEDED(hr))
            {
                exceptionObject = RecyclerNew(scriptContext->GetRecycler(), Js::JavascriptExceptionObject, errorObject, scriptContext, NULL);
            }
        }
        else
        {
            char16 * allocatedString = NULL;
            if (pExcepInfo->bstrDescription != NULL)
            {
                uint32 len = SysStringLen(pExcepInfo->bstrDescription) + 1;
                allocatedString = RecyclerNewArrayLeaf(scriptContext->GetRecycler(), char16, len);
                wcscpy_s(allocatedString, len, pExcepInfo->bstrDescription);
            }

            HRESULT hCode = pExcepInfo->scode;
            Js::JavascriptError *errorObject = scriptContext->GetLibrary()->CreateError();
            Js::JavascriptError::SetErrorMessageProperties(errorObject, hCode, allocatedString, scriptContext);
            exceptionObject = RecyclerNew(scriptContext->GetRecycler(), Js::JavascriptExceptionObject, errorObject, scriptContext, NULL);

        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    if (SUCCEEDED(hr))
    {
        scriptContext->RecordException(exceptionObject);
    }
    return hr;
}


// We need to put the functionBody in scriptenter, and retrieve the information here.It doesn't seems to be used in 
// most cases though. 
HRESULT DispatchExCaller::GetCurrentSourceContext(DWORD_PTR* pdwContext, VARIANT_BOOL* pfExecutingGlobalCode)
{
    HRESULT hr = NOERROR;
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    Js::JavascriptStackWalker walker(scriptContext);
    Js::JavascriptFunction* func = nullptr;
    if (walker.GetCaller(&func))
    {
        if (Js::JavascriptFunction::Is(func))
        {
            Js::FunctionBody* functionBody = func->GetFunctionBody();
            // Return the primrary host context, should never return our own secondary host context
            *pdwContext = functionBody->GetHostSourceContext();      
            *pfExecutingGlobalCode = walker.IsCallerGlobalFunction()? VARIANT_TRUE: VARIANT_FALSE;
        }
    }
    
    return hr;
}


