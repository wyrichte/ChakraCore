//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"

namespace JsStaticAPI
{
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

}
