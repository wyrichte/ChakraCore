//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"

namespace JsStaticAPI
{
    inline LONG FatalExceptionFilter(__in LPEXCEPTION_POINTERS lpep)
    {
        LONG rc = UnhandledExceptionFilter(lpep);

        // re == EXCEPTION_EXECUTE_HANDLER means there is no debugger attached, let's terminate
        // the process. Otherwise give control to the debugger.
        // Note: in case when postmortem debugger is registered but no actual debugger attached,
        //       rc will be 0 (and EXCEPTION_EXECUTE_HANDLER is 1), so it acts as if there is debugger attached.
        if (rc == EXCEPTION_EXECUTE_HANDLER)
        {
            TerminateProcess(GetCurrentProcess(), (UINT)DBG_TERMINATE_PROCESS);
        }
        else
        {
            // However, if debugger was not attached for some reason, terminate the process.
            if (!IsDebuggerPresent())
            {
                TerminateProcess(GetCurrentProcess(), (UINT)DBG_TERMINATE_PROCESS);
            }
            DebugBreak();
        }

        return EXCEPTION_CONTINUE_SEARCH;
    }

    void ReportFatalException(__in HRESULT exceptionCode, __in ErrorReason reasonCode)
    {
        if (IsDebuggerPresent())
        {
            DebugBreak();
        }

        __try
        {
            ULONG_PTR ExceptionInformation[2];
            ExceptionInformation[0] = (ULONG_PTR)reasonCode;
            ExceptionInformation[1] = NULL;
            RaiseException(exceptionCode, EXCEPTION_NONCONTINUABLE, 2, (ULONG_PTR*)ExceptionInformation);
        }
        __except (JsStaticAPI::FatalExceptionFilter(GetExceptionInformation()))
        {
        }
    }

#define FATAL_ON_FAILED_API_RESULT(hr) if (FAILED(hr)) { JsStaticAPI::ReportFatalException(hr, Fatal_Failed_API_Result); }

    //============================================================================================================
    // Helpers
    //============================================================================================================

    template <class Fn>
    Var GetLibraryObject(IActiveScriptDirect* activeScriptDirect, Fn fn)
    {
        ScriptEngineBase* scriptEngine = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngine->GetScriptContext()->GetScriptContextBase();
        return fn(scriptContextBase);
    }

    //============================================================================================================
    // Static APIs
    //============================================================================================================

    Var JavascriptLibrary::GetUndefined(IActiveScriptDirect* activeScriptDirect)
    {
        return GetLibraryObject(activeScriptDirect,
            [&](const Js::ScriptContextBase*
            scriptContext)-> Var {
                // ScriptContext can be NULL if we are called from a closed engine. In most cases, that
                // won't happen as we already reject the call before jscript9 calls into mshtml. However,
                // we can call GetUndefined() at the end of navigation methods like window.location = ...
                // In that case we will just return NULL. ExternalFunctionThunk translates NULL to undefined
                // as well.
                if (NULL == scriptContext)
                {
                    return NULL;
                }
                return scriptContext->GetLibrary()->GetUndefined();
        });
    }

    Var JavascriptLibrary::GetNull(IActiveScriptDirect* activeScriptDirect)
    {
        return GetLibraryObject(activeScriptDirect,
            [&](const Js::ScriptContextBase* ScriptContextBase)-> Var {
                return ScriptContextBase->GetLibrary()->GetNull();
        });
    }

    Var JavascriptLibrary::GetTrue(IActiveScriptDirect* activeScriptDirect)
    {
        return GetLibraryObject(activeScriptDirect,
            [&](const Js::ScriptContextBase* ScriptContextBase)-> Var {
                return ScriptContextBase->GetLibrary()->GetTrue();
        });
    }

    Var JavascriptLibrary::GetFalse(IActiveScriptDirect* activeScriptDirect)
    {
        return GetLibraryObject(activeScriptDirect,
            [&](const Js::ScriptContextBase* ScriptContextBase)-> Var {
                return ScriptContextBase->GetLibrary()->GetFalse();
        });
    }

    Var JavascriptLibrary::GetGlobalObject(IActiveScriptDirect* activeScriptDirect)
    {
        return GetLibraryObject(activeScriptDirect,
            [&](const Js::ScriptContextBase* ScriptContextBase)-> Var {
                return ScriptContextBase->GetGlobalObject();
        });
    }

    Var JavascriptLibrary::GetPromiseConstructor(IActiveScriptDirect* activeScriptDirect)
    {
        return GetLibraryObject(activeScriptDirect,
            [&](const Js::ScriptContextBase* ScriptContextBase)-> Var {
                return ScriptContextBase->GetLibrary()->GetPromiseConstructor();
        });
    }

    Var JavascriptLibrary::GetPromiseResolve(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();

        Var resolveFunc = scriptContextBase->GetLibrary()->GetPromiseResolve();
        if (NULL == resolveFunc)
        {
            HRESULT hr = scriptEngineBase->EnsurePromiseResolveFunction(&resolveFunc);
            if (hr != S_OK)
            {
                AssertMsg(false, "Failed to get the Promise.resolve function");
                resolveFunc = NULL;
            }
        }
        return resolveFunc;
    }

    Var JavascriptLibrary::GetPromiseThen(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();

        Var thenFunc = scriptContextBase->GetLibrary()->GetPromiseThen();
        if (NULL == thenFunc)
        {
            HRESULT hr = scriptEngineBase->EnsurePromiseThenFunction(&thenFunc);
            if (hr != S_OK)
            {
                AssertMsg(false, "Failed to get the Promise.prototype.then function");
                thenFunc = NULL;
            }
        }
        return thenFunc;
    }

    Var JavascriptLibrary::GetJSONStringify(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();

        Var jsonStringifyFunc = scriptContextBase->GetLibrary()->GetJSONStringify();
        if (NULL == jsonStringifyFunc)
        {
            HRESULT hr = scriptEngineBase->EnsureJSONStringifyFunction(&jsonStringifyFunc);
            if (hr != S_OK)
            {
                AssertMsg(false, "Failed to get the JSON.stringify function");
                jsonStringifyFunc = NULL;
            }
        }
        return jsonStringifyFunc;
    }

    Var JavascriptLibrary::GetObjectFreeze(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();

        Var objFreezeFunc = scriptContextBase->GetLibrary()->GetObjectFreeze();
        if (NULL == objFreezeFunc)
        {
            HRESULT hr = scriptEngineBase->EnsureObjectFreezeFunction(&objFreezeFunc);
            if (hr != S_OK)
            {
                AssertMsg(false, "Failed to get the Object.freeze function");
                objFreezeFunc = NULL;
            }
        }
        return objFreezeFunc;
    }

    HRESULT JavascriptLibrary::SetNoScriptScope(/* in */ ITrackingService *threadService, bool noScriptScope)
    {
        if (threadService == nullptr)
        {
            AssertMsg(false, "threadService was nullptr; the parameter was incorrect");

            // If passed nullptr and we skipped the Assert (in Release build), return an error code.
            return E_INVALIDARG;
        }

        JavascriptThreadService *jsThreadService = static_cast<JavascriptThreadService *>(threadService);
        jsThreadService->GetThreadContext()->SetNoScriptScope(noScriptScope);
        return S_OK;
    }

    HRESULT JavascriptLibrary::IsNoScriptScope(/* in */ ITrackingService *threadService, /* out */ bool *isNoScriptScope)
    {
        if (threadService == nullptr)
        {
            AssertMsg(false, "threadService was nullptr; the parameter was incorrect");

            // If passed nullptr and we skipped the Assert (in Release build):
            // - Fail by returning an error code.
            // - If error is not handled, we should behave as if this is a NoScriptScope and fail fast on script entry
            //   i.e. the case where IsNoScriptScope() == true.
            *isNoScriptScope = true;
            return E_INVALIDARG;
        }

        JavascriptThreadService *jsThreadService = static_cast<JavascriptThreadService *>(threadService);
        *isNoScriptScope = jsThreadService->GetThreadContext()->IsNoScriptScope();
        return S_OK;
    }

    Var JavascriptLibrary::GetArrayForEachFunction(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();

        Var func = scriptContextBase->GetLibrary()->GetArrayPrototypeForEachFunction();
        if (func == nullptr)
        {
            HRESULT hr = scriptEngineBase->EnsureArrayPrototypeForEachFunction(&func);
            if (hr != S_OK)
            {
                func = nullptr;
            }
        }
        return func;
    }

    Var JavascriptLibrary::GetArrayKeysFunction(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();

        Var func = scriptContextBase->GetLibrary()->GetArrayPrototypeKeysFunction();
        if (func == nullptr)
        {
            HRESULT hr = scriptEngineBase->EnsureArrayPrototypeKeysFunction(&func);
            if (hr != S_OK)
            {
                func = nullptr;
            }
        }
        return func;
    }

    Var JavascriptLibrary::GetArrayValuesFunction(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();

        Var func = scriptContextBase->GetLibrary()->GetArrayPrototypeValuesFunction();
        if (func == nullptr)
        {
            HRESULT hr = scriptEngineBase->EnsureArrayPrototypeValuesFunction(&func);
            if (hr != S_OK)
            {
                func = nullptr;
            }
        }
        return func;
    }

    Var JavascriptLibrary::GetArrayEntriesFunction(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();

        Var func = scriptContextBase->GetLibrary()->GetArrayPrototypeEntriesFunction();
        if (func == nullptr)
        {
            HRESULT hr = scriptEngineBase->EnsureArrayPrototypeEntriesFunction(&func);
            if (hr != S_OK)
            {
                func = nullptr;
            }
        }
        return func;
    }

    PropertyId JavascriptLibrary::GetPropertyIdSymbolIterator(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();
        return scriptContextBase->GetLibrary()->GetPropertyIdSymbolIterator();
    }

    Var  JavascriptLibrary::CreateWeakMap(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        Var mapInstance = nullptr;
        if (scriptEngineBase->CreateWeakMap(&mapInstance) != S_OK)
        {
            mapInstance = nullptr;
        }
        return mapInstance;
    }

    HRESULT JavascriptLibrary::WeakMapHas(IActiveScriptDirect* activeScriptDirect, Var instance, Var key, __out bool* has)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        return scriptEngineBase->WeakMapHas(instance, key, has);
    }

    HRESULT JavascriptLibrary::WeakMapSet(IActiveScriptDirect* activeScriptDirect, Var instance, Var key, Var value)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        return scriptEngineBase->WeakMapSet(instance, key, value);
    }

    HRESULT JavascriptLibrary::WeakMapGet(IActiveScriptDirect* activeScriptDirect, Var instance, Var key, __out Var *value, __out bool* result)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        return scriptEngineBase->WeakMapGet(instance, key, value, result);
    }

    HRESULT JavascriptLibrary::WeakMapDelete(IActiveScriptDirect* activeScriptDirect, Var instance, Var key, __out bool* result)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        return scriptEngineBase->WeakMapDelete(instance, key, result);
    }
	
    PropertyId JavascriptLibrary::GetPropertyIdSymbolToStringTag(IActiveScriptDirect* activeScriptDirect)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        const Js::ScriptContextBase* scriptContextBase = scriptEngineBase->GetScriptContext()->GetScriptContextBase();
        return scriptContextBase->GetLibrary()->GetPropertyIdSymbolToStringTag();
    }

    Var JavascriptLibrary::CreateExternalEntriesFunction(IActiveScriptDirect* activeScriptDirect,
        JavascriptTypeId type, uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        Var func = nullptr;
        HRESULT hr = scriptEngineBase->CreateIteratorEntriesFunction(type, byteCount, prototypeForIterator, initFunction, nextFunction, &func);
        FATAL_ON_FAILED_API_RESULT(hr)

        return func;
    }

    Var JavascriptLibrary::CreateExternalKeysFunction(IActiveScriptDirect* activeScriptDirect,
        JavascriptTypeId type, uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        Var func = nullptr;
        HRESULT hr = scriptEngineBase->CreateIteratorKeysFunction(type, byteCount, prototypeForIterator, initFunction, nextFunction, &func);
        FATAL_ON_FAILED_API_RESULT(hr)

        return func;
    }

    Var JavascriptLibrary::CreateExternalValuesFunction(IActiveScriptDirect* activeScriptDirect,
        JavascriptTypeId type, uint byteCount, Var prototypeForIterator, InitIteratorFunction initFunction, NextFunction nextFunction)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        Var func = nullptr;
        HRESULT hr = scriptEngineBase->CreateIteratorValuesFunction(type, byteCount, prototypeForIterator, initFunction, nextFunction, &func);
        FATAL_ON_FAILED_API_RESULT(hr)

        return func;
    }

    Var JavascriptLibrary::CreateIteratorNextFunction(IActiveScriptDirect* activeScriptDirect, JavascriptTypeId type)
    {
        ScriptEngineBase* scriptEngineBase = ScriptEngineBase::FromIActiveScriptDirect(activeScriptDirect);
        Var func = nullptr;
        HRESULT hr = scriptEngineBase->CreateIteratorNextFunction(type, &func);
        FATAL_ON_FAILED_API_RESULT(hr)

        return func;
    }

    void * JavascriptLibrary::CustomIteratorToExtension(Var iterator)
    {
        Assert(iterator != nullptr);
        return (void*)(((char*)iterator) + sizeof(Js::CustomExternalIterator));
    }

    Var JavascriptLibrary::GetIteratorPrototype(IActiveScriptDirect* activeScriptDirect)
    {
        return GetLibraryObject(activeScriptDirect,
            [&](const Js::ScriptContextBase* ScriptContextBase)-> Var {
            Var object = ScriptContextBase->GetLibrary()->GetIteratorPrototype();
            if (object == nullptr)
            {
                FATAL_ON_FAILED_API_RESULT(E_FAIL);
            }
            return object;
        });
    }

}
