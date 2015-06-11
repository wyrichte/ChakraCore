//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class CJavascriptOperations sealed : public IJavascriptOperations {
    unsigned long refCount;
    Js::ScriptContext* scriptContext;
public:
    CJavascriptOperations(Js::ScriptContext* scriptContext) : scriptContext(scriptContext), refCount(0) {

    }
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject) ;


    ULONG STDMETHODCALLTYPE AddRef( void);

    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE HasProperty(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in PropertyId propertyId, __out BOOL* result);
    HRESULT STDMETHODCALLTYPE GetProperty(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in PropertyId propertyId, __out Var* value);
    HRESULT STDMETHODCALLTYPE SetProperty(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in PropertyId propertyId, __in Var value);
    HRESULT STDMETHODCALLTYPE DeleteProperty(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in PropertyId propertyId);
    HRESULT STDMETHODCALLTYPE HasItem(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in Var index, __out BOOL* result);
    HRESULT STDMETHODCALLTYPE GetItem(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in Var index, __out Var* value);
    HRESULT STDMETHODCALLTYPE SetItem(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in Var index, __in Var value);
    HRESULT STDMETHODCALLTYPE DeleteItem(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in Var index);
    HRESULT STDMETHODCALLTYPE Equals(__in IActiveScriptDirect* scriptDirect,__in Var a,__in Var b, __out BOOL* result);
    HRESULT STDMETHODCALLTYPE StrictEquals(__in IActiveScriptDirect* scriptDirect,__in Var a, __in Var b, __out BOOL* result);
    HRESULT STDMETHODCALLTYPE ThrowException(__in IActiveScriptDirect* scriptDirect,__in Var exceptionObject, __in BOOL release);
    HRESULT STDMETHODCALLTYPE QueryObjectInterface(__in IActiveScriptDirect* scriptDirect,__in Var instance, __in REFIID riid, __out void **ppvObj);
};
