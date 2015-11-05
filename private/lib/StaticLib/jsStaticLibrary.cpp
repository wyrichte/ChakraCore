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

    HRESULT JavascriptLibrary::SetNoScriptScope(/* in */ IUnknown *threadService, bool noScriptScope)
    {
        if (threadService == nullptr)
        {
            AssertMsg(false, "threadService was nullptr; the parameter was incorrect");

            // If passed nullptr and we skipped the Assert (in Release build), return an error code.
            return E_INVALIDARG;
        }

        IJavascriptThreadProperty *pThreadProperty = nullptr;

        HRESULT hresult = threadService->QueryInterface(__uuidof(IJavascriptThreadProperty), /* out */ reinterpret_cast<void **>(&pThreadProperty));
        AssertMsg(S_OK == hresult, "threadService did not QI to IJavascriptThreadProperty; the parameter was incorrect");

        if (S_OK == hresult)
        {
            JavascriptThreadService *jsThreadService = static_cast<JavascriptThreadService *>(pThreadProperty);
            ThreadContext *threadContext = jsThreadService->GetThreadContext();

            pThreadProperty->Release();

            threadContext->SetNoScriptScope(noScriptScope);

            return S_OK;
        }
        else
        {
            // If QI failed and we skipped the Assert (in Release build), return an error code.
            return E_INVALIDARG;
        }
    }

    HRESULT JavascriptLibrary::IsNoScriptScope(/* in */ IUnknown *threadService, /* out */ bool *isNoScriptScope)
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

        IJavascriptThreadProperty *pThreadProperty = nullptr;

        HRESULT hresult = threadService->QueryInterface(__uuidof(IJavascriptThreadProperty), /* out */ reinterpret_cast<void **>(&pThreadProperty));
        AssertMsg(S_OK == hresult, "threadService did not QI to IJavascriptThreadProperty; the parameter was incorrect");

        if (S_OK == hresult)
        {
            JavascriptThreadService *jsThreadService = static_cast<JavascriptThreadService *>(pThreadProperty);
            ThreadContext *threadContext = jsThreadService->GetThreadContext();

            pThreadProperty->Release();

            *isNoScriptScope = threadContext->IsNoScriptScope();
            return S_OK;
        }
        else
        {
            // If the QI failed and we skipped the Assert (in Release build):
            // - Fail by returning an error code.
            // - If error is not handled, we should behave as if this is a NoScriptScope and fail fast on script entry
            //   i.e. the case where IsNoScriptScope() == true.
            *isNoScriptScope = true;
            return E_INVALIDARG;
        }
    }
}
