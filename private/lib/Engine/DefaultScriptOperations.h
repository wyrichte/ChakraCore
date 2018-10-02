//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
// We cannot put this in Js namespace as the Var/PropertyId etc. definitions are
// from the IDL and there is no support for namespace in idl.

// We made a conscious decision that we will allow SEH to flow between
// the interface boundary. We shouldn't let C++ exception flow through the interface
// boundaries though.
namespace Js
{
#define END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_CURRENT_CALLER(hr, scriptContext, hostScriptContext) \
    END_JS_RUNTIME_CALL(scriptContext); \
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
    catch (const Js::JavascriptException& err) \
    { \
        Js::JavascriptExceptionObject * pError = err.GetAndClear(); \
        DispatchExCaller * pdc = nullptr; \
        if (hostScriptContext) \
        { \
            hr = hostScriptContext->GetDispatchExCaller((void **)&pdc); \
            if (SUCCEEDED(hr)) \
            { \
                hr = ScriptSite::HandleJavascriptException(pError, scriptContext, pdc); \
            } \
        } \
        else \
        { \
            hr = ScriptSite::HandleJavascriptException(pError, scriptContext, nullptr); \
        } \
    } \
    CATCH_UNHANDLED_EXCEPTION(hr)

    class CVarNullEnumerator sealed: public IVarEnumerator2
    {
    private:
        CVarNullEnumerator() {};

    public:
        static CVarNullEnumerator Instance;

        STDMETHODIMP QueryInterface(REFIID riid,void **ppv)
        {
            if (ppv == nullptr)
                return E_POINTER;
            if (IsEqualGUID(riid,__uuidof(IVarEnumerator))) {
                *ppv = static_cast<IVarEnumerator*>(this);
            }
            else if (IsEqualGUID(riid,__uuidof(IVarEnumerator2))) {
                *ppv  =static_cast<IVarEnumerator2*>(this);
            }
            else if (IsEqualGUID(riid,IID_IUnknown)) {
                *ppv = static_cast<IVarEnumerator*>(this);
            }
            else {
                *ppv = nullptr;
                return E_NOINTERFACE;
            }
            reinterpret_cast<IUnknown*>(*ppv)->AddRef();
            return S_OK;
        }
        STDMETHODIMP_(ULONG) AddRef(void) { return 1; }

        STDMETHODIMP_(ULONG) Release(void)
        {
            return 1;
        }

        STDMETHODIMP MoveNext(/*[out]*/ BOOL* itemsAvailable, /*[out,optional]*/ ::PropertyAttributes* attributes)
        {
            *itemsAvailable = false;
            return NOERROR;
        }

        STDMETHODIMP GetCurrentName(/*[out]*/ Var* item)
        {
            return E_FAIL;
        }

        STDMETHODIMP GetJavascriptEnumerator(/*[out*/ Var * enumerator)
        {
            *enumerator = nullptr;
            return NOERROR;
        }
    };

    class JavascriptEnumeratorWrapper;
    class CVarEnumerator sealed : public IVarEnumerator2
    {
        unsigned long refCount;
        Js::ScriptContext* scriptContext;
        RecyclerRootPtr<Js::JavascriptEnumeratorWrapper> internalEnum;
    public:
        CVarEnumerator(Js::JavascriptEnumeratorWrapper * internalEnum, Js::ScriptContext* scriptContext);
        ~CVarEnumerator();

        STDMETHODIMP QueryInterface(REFIID riid,void **ppv)
        {
            if (ppv== nullptr)
                return E_POINTER;
            if (IsEqualGUID(riid,__uuidof(IVarEnumerator))) {
                *ppv=static_cast<IVarEnumerator*>(this);
            }
            else if (IsEqualGUID(riid,__uuidof(IVarEnumerator2))) {
                *ppv=static_cast<IVarEnumerator2*>(this);
            }
            else if (IsEqualGUID(riid,IID_IUnknown)) {
                *ppv=static_cast<IVarEnumerator*>(this);
            }
            else {
                *ppv= nullptr;
                return E_NOINTERFACE;
            }
            reinterpret_cast<IUnknown*>(*ppv)->AddRef();
            return S_OK;
        }

        STDMETHODIMP_(ULONG) AddRef(void) { return InterlockedIncrement(&refCount); }

        STDMETHODIMP_(ULONG) Release(void)
        {
            LONG res = InterlockedDecrement(&refCount);

            if (res == 0)
                HeapDelete(this);
            return res;
        }

        STDMETHODIMP MoveNext(/*[out]*/ BOOL* itemsAvailable, /*[out,optional]*/ ::PropertyAttributes* attributes);

        STDMETHODIMP GetCurrentName(/*[out]*/ Var* item);

        STDMETHODIMP GetJavascriptEnumerator(/*[out]*/ Var * enumerator);

    };

    class DefaultScriptOperations : public ITypeOperations
    {
    private:
        static OperationUsage defaultUsage;

    public:
        DefaultScriptOperations();
        ~DefaultScriptOperations();

        static DefaultScriptOperations s_DefaultScriptOperations;

        HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject) ;

        ULONG STDMETHODCALLTYPE AddRef( void);

        ULONG STDMETHODCALLTYPE Release();
        HRESULT STDMETHODCALLTYPE GetOperationUsage(
            /* [out] */ OperationUsage *flags);

        HRESULT STDMETHODCALLTYPE HasOwnProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE GetOwnProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var *value,
            /* [out] */ BOOL *propertyPresent);

        HRESULT STDMETHODCALLTYPE GetPropertyReference(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var *value,
            /* [out] */ BOOL *propertyPresent);

        HRESULT STDMETHODCALLTYPE SetProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE SetPropertyWithAttributes(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [in] */ ::PropertyAttributes attributes,
            /* [in] */ ::SideEffects effects,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE DeleteProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE HasOwnItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE GetOwnItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [out] */ Var *value,
            /* [out] */ BOOL *itemPresent);

        HRESULT STDMETHODCALLTYPE SetItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [in] */ Var value,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE DeleteItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE GetEnumerator(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ BOOL enumNonEnumerable,
            /* [in] */ BOOL enumSymbols,
            /* [out] */ IVarEnumerator **enumerator);

        HRESULT STDMETHODCALLTYPE IsEnumerable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE IsWritable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE IsConfigurable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE SetEnumerable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value);

        HRESULT STDMETHODCALLTYPE SetWritable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value);

        HRESULT STDMETHODCALLTYPE SetConfigurable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value);

        HRESULT STDMETHODCALLTYPE SetAccessors(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var getter,
            /* [in] */ Var setter);

        HRESULT STDMETHODCALLTYPE GetAccessors(
             /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
           /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var* getter,
            /* [out] */ Var* setter,
            /* [out] */ BOOL* result);

        HRESULT STDMETHODCALLTYPE GetSetter(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags);

        HRESULT STDMETHODCALLTYPE GetItemSetter(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags);

        HRESULT STDMETHODCALLTYPE Equals(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var other,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE StrictEquals(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var other,
            /* [out] */ BOOL *result);

        HRESULT STDMETHODCALLTYPE QueryObjectInterface(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ REFIID riid,
            /* [out] */ void **ppvObj);

        HRESULT STDMETHODCALLTYPE GetInitializer(
            /* [out] */ InitializeMethod * initializer,
            /* [out] */ int * initSlotCapacity,
            /* [out] */ BOOL * hasAccessors);

        HRESULT STDMETHODCALLTYPE GetFinalizer(
            /* [out] */ FinalizeMethod * finalizer);

        HRESULT STDMETHODCALLTYPE HasInstance(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var constructor,
            /* [in] */ Var instance,
            /* [out] */ BOOL* result);

        HRESULT STDMETHODCALLTYPE GetNamespaceParent(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [out] */ Var* namespaceParent);

        HRESULT STDMETHODCALLTYPE CrossDomainCheck(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [out] */ BOOL* result);

        HRESULT STDMETHODCALLTYPE GetHeapObjectInfo(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ ProfilerHeapObjectInfoFlags flags,
            /* [out] */ HostProfilerHeapObject** result,
            /* [out] */ HeapObjectInfoReturnResult* returnResult);

    private:
        static Js::ScriptContext * GetCurrentScriptContext(IActiveScriptDirect* scriptDirect);

        template <class Fn>
        HRESULT DefaultOperationsWrapper(IActiveScriptDirect* scriptDirect,
            Var instance,
            Fn fn,
            HRESULT taggedIntErrorCode)
        {
            HRESULT hr = NOERROR;
            IfNullReturnError(instance, E_INVALIDARG);
            if (TaggedNumber::Is(instance))
            {
                return taggedIntErrorCode;
            }
            RecyclableObject* objInstance = VarTo<RecyclableObject>(instance);
            Js::ScriptContext * scriptContext = GetCurrentScriptContext(scriptDirect);
            if (nullptr == scriptContext)
            {
                // trident might call this with the site closed during navigation; we don't need to do anything
                // here if we are closed already.
                return E_ACCESSDENIED;
            }
            HostScriptContext * hostScriptContext = nullptr;
            BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
            {
                hr = fn(objInstance, scriptContext);
            }
            END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_CURRENT_CALLER(hr, scriptContext, hostScriptContext);
            VERIFYHRESULTBEFORERETURN(hr, objInstance->GetType()->GetScriptContext());
            return hr;
        };


    };
}


