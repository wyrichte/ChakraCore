//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "JsrtChakraPch.h"
#include "JsrtRuntime.h"
#include "JsrtComException.h"
#include "JsrtExternalObject.h"
#include "rterrors.h"
#include "jsrtprivate.h"
#include "JsrtInternal.h"
#include "JsrtDelegateWrapper.h"
#include "JsrtContextChakra.h"

#include "hostdispatch.h"
#include "dispmemberproxy.h"
#include "DispatchHelper.h"
#include "ExternalObject.h"
#include "Library\ES5Array.h"
#include "ActiveScriptProfilerHeapEnum.h"

STDAPI_(JsErrorCode) JsCreateWeakContainer(JsRef ref, JsWeakContainerRef *weakContainerRef)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_JSREF(ref);
        PARAM_NOT_NULL(weakContainerRef);
        *weakContainerRef = JS_INVALID_REFERENCE;

        Recycler *recycler = scriptContext->GetRecycler();
        recycler->FindOrCreateWeakReferenceHandle((void *)ref, (RecyclerWeakReference<void> **)weakContainerRef);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsIsReferenceValid(JsWeakContainerRef weakContainerRef, bool *isValid)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(weakContainerRef);
        PARAM_NOT_NULL(isValid);
        *isValid = false;

        RecyclerWeakReference<void> *weakRef = (RecyclerWeakReference<void> *)weakContainerRef;
        *isValid = (weakRef->Get() != nullptr);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetReference(JsWeakContainerRef weakContainerRef, JsRef *ref)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(weakContainerRef);
        PARAM_NOT_NULL(ref);
        *ref = JS_INVALID_REFERENCE;

        RecyclerWeakReference<void> *weakRef = (RecyclerWeakReference<void> *)weakContainerRef;
        *ref = weakRef->Get();
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetArrayLength(JsValueRef value, size_t *length)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(value, scriptContext);
        PARAM_NOT_NULL(length);
        *length = 0;

        if (!Js::JavascriptArray::Is(value))
        {
            return JsErrorInvalidArgument;
        }

        *length = Js::JavascriptArray::FromVar(value)->GetLength();
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsVariantToValue(VARIANT * variant, JsValueRef * value)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode { 
        PARAM_NOT_NULL(variant);
        PARAM_NOT_NULL(value);
        *value = nullptr;

        Js::Var var;
        JsrtComException::ThrowIfFailed(DispatchHelper::MarshalVariantToJsVar(variant, &var, scriptContext));
        *value = var;

        return JsNoError;
    });    
}

STDAPI_(JsErrorCode) JsValueToVariant(JsValueRef object, VARIANT * variant)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(object, scriptContext);
        PARAM_NOT_NULL(variant);
        ZeroMemory(variant, sizeof(VARIANT));

        JsrtComException::ThrowIfFailed(DispatchHelper::MarshalJsVarToVariant(object, variant));

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsStartProfiling(IActiveScriptProfilerCallback *callback, PROFILER_EVENT_MASK eventMask, unsigned long contextValue)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode { 
        PARAM_NOT_NULL(callback);

        if (!scriptContext->IsProfiling())
        {
            HRESULT hr = scriptContext->RegisterProfileProbe(callback, eventMask, contextValue, nullptr, DispMemberProxy::ProfileInvoke);
            return JsrtComException::JsErrorFromHResult(hr);
        }

        return JsErrorAlreadyProfilingContext;
    });    
}

STDAPI_(JsErrorCode) JsStopProfiling(HRESULT reason)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode { 
        JsErrorCode result = JsNoError;

        if (scriptContext->IsProfiling())
        {
            HRESULT hr = scriptContext->DeRegisterProfileProbe(reason, DispMemberProxy::DefaultInvoke);
            result = JsrtComException::JsErrorFromHResult(hr);
        }

        return result;
    });    
}

STDAPI_(JsErrorCode) JsEnumerateHeap(IActiveScriptProfilerHeapEnum **ppEnum)
{
    PARAM_NOT_NULL(ppEnum);
    *ppEnum = nullptr;

    // Heap enumeration requires us to not be in script already, so no wrapper.
    JsrtContextChakra *context = (JsrtContextChakra *)JsrtContext::GetCurrent();
    JsErrorCode error = CheckContext(context, true);

    if (error != JsNoError)
    {
        return error;
    }

    ActiveScriptProfilerHeapEnum *pHeapEnum = NULL;
    HRESULT hr = context->GetScriptEngine()->CreateHeapEnum(&pHeapEnum, /* preEnumHeap2 */ false, PROFILER_HEAP_ENUM_FLAGS::PROFILER_HEAP_ENUM_FLAGS_RELATIONSHIP_SUBSTRINGS);
    error = JsrtComException::JsErrorFromHResult(hr);
    if (SUCCEEDED(hr))
    {
        *ppEnum = pHeapEnum;
    }

    return error;
}

STDAPI_(JsErrorCode) JsIsEnumeratingHeap(bool *isEnumeratingHeap)
{
    PARAM_NOT_NULL(isEnumeratingHeap);
    *isEnumeratingHeap = false;

    JsrtContext *currentContext = JsrtContext::GetCurrent();

    if (currentContext == nullptr)
    {
        return JsErrorNoCurrentContext;
    }   

    *isEnumeratingHeap = currentContext->GetScriptContext()->GetRecycler()->IsHeapEnumInProgress();
    return JsNoError;
}

STDAPI_(JsErrorCode) JsStartDebugging()
{
    return ContextAPINoScriptWrapper(
        [&] (Js::ScriptContext * scriptContext) -> JsErrorCode {   

            JsrtContextChakra* context = (JsrtContextChakra *)JsrtContext::GetCurrent();

            if (context->GetRuntime()->GetThreadContext()->IsInScript())
            {
                return JsErrorRuntimeInUse;
            }

            if (context->GetScriptContext()->IsInDebugMode())
            {
                return JsErrorAlreadyDebuggingContext;
            }

            ScriptEngine* scriptEngine = context->GetScriptEngine();
            Assert(scriptEngine);

            IDebugApplication *debugApplication;

            if (FAILED(scriptEngine->GetDefaultDebugApplication(&debugApplication)))
            {
                return JsErrorFatal;
            }

            if (!context->SetDebugApplication(debugApplication))
            {
                return JsErrorFatal;
            }

            // Put the engine into debug mode.
            if (FAILED(scriptEngine->OnDebuggerAttached(0, nullptr)))
            {
                return JsErrorFatal;
            }

            return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsStopDebugging()
{
    return ContextAPINoScriptWrapper(
        [&] (Js::ScriptContext * scriptContext) -> JsErrorCode {   
            JsrtContextChakra * context = (JsrtContextChakra *)JsrtContext::GetCurrent();

            if (context->GetRuntime()->GetThreadContext()->IsInScript())
            {
                return JsErrorRuntimeInUse;
            }

            if (!context->GetScriptContext()->IsInDebugMode())
            {
                return JsErrorNotDebuggingContext;
            }

            ScriptEngine* scriptEngine = context->GetScriptEngine();
            Assert(scriptEngine);

            // Take the engine out of debug mode.
            if (FAILED(scriptEngine->OnDebuggerDetached()))
            {
                return JsErrorFatal;
            }

            context->ReleaseDebugApplication();

            return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsSetProjectionEnqueueCallback(_In_ JsProjectionEnqueueCallback projectionEnqueueCallback, _In_opt_ void *projectionEnqueueContext)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        HRESULT hr = NOERROR;

        if (projectionEnqueueCallback == nullptr)
        {
            return JsErrorNullArgument;
        }

        ComPtr<DelegateWrapper> delegateWrapper;
        hr = DelegateWrapper::CreateInstance(projectionEnqueueCallback, projectionEnqueueContext, &delegateWrapper);
        if (FAILED(hr))
        {
            return JsCannotSetProjectionEnqueueCallback;
        }

        JsrtContextChakra* context = (JsrtContextChakra *)JsrtContext::GetCurrent();
        if (context->SetProjectionDelegateWrapper(delegateWrapper.Get()) != JsNoError)
        {
            return JsCannotSetProjectionEnqueueCallback;
        }

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsProjectWinRTNamespace(_In_z_ const wchar_t *nameSpace)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode { 
        JsErrorCode errorCode;
        JsrtContextChakra * context = (JsrtContextChakra *)JsrtContext::GetCurrent();
        errorCode = context->ReserveWinRTNamespace(nameSpace);
        return errorCode;
    });
}

STDAPI_(JsErrorCode) JsInspectableToObject(_In_ IInspectable  *inspectable, _Out_ JsValueRef *value)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode { 
        PARAM_NOT_NULL(inspectable);
        PARAM_NOT_NULL(value);
#if DBG
        CComPtr<IInspectable> dbgInspectable = nullptr;
        CComPtr<IDispatchEx> dbgDispatch = nullptr;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        Assert(S_OK == inspectable->QueryInterface(__uuidof(IInspectable), (void**)&dbgInspectable));
        Assert(FAILED(inspectable->QueryInterface(__uuidof(IDispatchEx), (void**)&dbgDispatch)));
        END_LEAVE_SCRIPT(scriptContext)
#endif
        JsrtContextChakra * context = (JsrtContextChakra *)JsrtContext::GetCurrent();
        HRESULT hr = NOERROR;
        if (FAILED(context->EnsureProjectionHost()))
        {
            return JsErrorCannotStartProjection;
        }
        BEGIN_LEAVE_SCRIPT(scriptContext)

        ScriptEngine* scriptEngine = context->GetScriptEngine();
        hr = scriptEngine->InspectableUnknownToVarInternal(inspectable, value);
        END_LEAVE_SCRIPT(scriptContext)
        if (FAILED(hr))
        {
            JsrtComException::ThrowIfFailed(hr);
        }
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsObjectToInspectable(_In_ JsValueRef value, _Out_ IInspectable **inspectable)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_JSREF(value);
        PARAM_NOT_NULL(inspectable);
        *inspectable = nullptr;
        // Check if projection is enabled, if not return error
        JsrtContextChakra * context = (JsrtContextChakra *)JsrtContext::GetCurrent();
        if (FAILED(context->EnsureProjectionHost()))
        {
            return JsErrorCannotStartProjection;
        }

        // JsValueRef should be a Custom External Object for it to be IInspectable
        if (!Js::ExternalObject::Is(value))
        {
            return JsErrorObjectNotInspectable;
        }

        Js::ExternalObject* externalObject = Js::ExternalObject::FromVar(value);
        if (!externalObject->IsCustomExternalObject())
        {
            return JsErrorObjectNotInspectable;
        }

        // Found a CEO, QI for IInspectable and if succeed return that
        HRESULT hr = NOERROR;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = externalObject->QueryObjectInterface(__uuidof(IInspectable), (void**)inspectable);
        }
        END_LEAVE_SCRIPT(scriptContext)
            if (FAILED(hr))
            {
                return JsErrorObjectNotInspectable;
            }
        return JsNoError;
    });
}
