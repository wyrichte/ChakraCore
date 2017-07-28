/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include <EnginePch.h>

ULONG STDMETHODCALLTYPE CJavascriptOperations:: AddRef( void)
{
    return InterlockedIncrement(&refCount);
}

ULONG STDMETHODCALLTYPE CJavascriptOperations:: Release()
{
    unsigned long currentCount = InterlockedDecrement(&refCount);

    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
    IfNullReturnError(ppvObject, E_POINTER);
    *ppvObject = nullptr;

    if (riid == IID_IUnknown ||
        riid == _uuidof(IJavascriptOperations))
    {
        *ppvObject = (IJavascriptOperations*)this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::HasProperty(__in IActiveScriptDirect* scriptDirect, __in Var instance, __in PropertyId propertyId, __out BOOL* result)
{
    IfNullReturnError(result, E_POINTER);
    *result = FALSE;

    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        *result = Js::JavascriptOperators::OP_HasProperty(instance, propertyId, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);

    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::GetProperty(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in PropertyId propertyId, __out Var* value)
{
    IfNullReturnError(value, E_POINTER);
    *value = nullptr;

    HRESULT  hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        *value = Js::JavascriptOperators::OP_GetProperty(instance, propertyId, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);

    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::SetProperty(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in PropertyId propertyId, __in Var value)
{
    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        Js::JavascriptOperators::OP_SetProperty(instance, propertyId, value, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);

    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::DeleteProperty(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in PropertyId propertyId)
{
    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        Js::JavascriptOperators::OP_DeleteProperty(instance, propertyId, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);
    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::HasItem(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in Var index, __out BOOL* result)
{
    IfNullReturnError(result, E_POINTER);
    *result = FALSE;

    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        *result = Js::JavascriptOperators::OP_HasItem(instance, index, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);
    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::GetItem(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in Var index, __out Var* value)
{
    IfNullReturnError(value, E_POINTER);
    *value = nullptr;

    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        *value = Js::JavascriptOperators::OP_GetElementI(instance, index, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);
    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::SetItem(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in Var index, __in Var value)
{
    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        Js::JavascriptOperators::OP_SetElementI(instance, index, value, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);
    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::DeleteItem(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in Var index)
{
    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        Js::JavascriptOperators::OP_DeleteElementI(instance, index, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);
    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::Equals(__in IActiveScriptDirect* scriptDirect,__in Var a, __in Var b, __out BOOL* result)
{
    IfNullReturnError(result, E_POINTER);
    *result = FALSE;

    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        *result = Js::JavascriptOperators::Equal_Full(a, b, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);
    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::StrictEquals(__in IActiveScriptDirect* scriptDirect,__in Var a, __in Var b, __out BOOL* result)
{
    IfNullReturnError(result, E_POINTER);
    *result = FALSE;

    HRESULT hr = NOERROR;
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(requestContext, false)
    {
        *result = Js::JavascriptOperators::StrictEqual(a, b, requestContext);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, requestContext);
    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::ThrowException(__in IActiveScriptDirect* scriptDirect,__in Var exceptionObject, __in BOOL release)
{
    if (release)
    {
        Release();
    }
    ScriptSite* requestSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* requestContext = requestSite->GetScriptSiteContext();
    HostScriptContext* hostScriptContext = nullptr;
    Js::ScriptContext* srcScriptContext;
    if (SUCCEEDED(requestSite->GetPreviousHostScriptContext(&hostScriptContext)) &&
        hostScriptContext != NULL)
    {
        srcScriptContext = hostScriptContext->GetScriptContext();
    }
    else
    {
        // we are the only one on the stack.
        srcScriptContext = scriptContext;
    }

    HRESULT hr = NOERROR;

    // We need to enter script now as the throw code path can possibly call a getter defined on Error.stackTraceLimit.
    // Also assume hasCaller, as there must be caller in the stack to catch this exception.
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_OOM_TO_HRESULT(requestContext, /*doCleanup*/false, /*hasCaller*/true)
    {
        if (exceptionObject == (Var)requestContext->GetThreadContext()->GetPendingOOMErrorObject())
        {
            // This is actually the OOM exception object masquerading as a Var. CreateErrorObject passed it
            // to the DOM because it was unable to create an error object.
            Js::JavascriptExceptionOperators::ThrowExceptionObject((Js::JavascriptExceptionObject*)exceptionObject, requestContext);
        }
        else
        {
            Js::JavascriptExceptionOperators::Throw(exceptionObject, srcScriptContext);
        }
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_OOM_TO_HRESULT(hr);
    VERIFYHRESULTBEFORERETURN(hr, requestContext);

    return hr;
}

HRESULT STDMETHODCALLTYPE CJavascriptOperations::QueryObjectInterface(__in IActiveScriptDirect* scriptDirect, __in Var instance, __in REFIID riid, __out void **ppvObj)
{
    IfNullReturnError(ppvObj, E_POINTER);
    *ppvObj = nullptr;

    if (Js::TaggedNumber::Is(instance))
    {
        return E_INVALIDARG;
    }
    
    Js::RecyclableObject* object = Js::RecyclableObject::FromVar(instance);

    HRESULT hr =  object->QueryObjectInterface(riid, ppvObj);

    return hr;
}
