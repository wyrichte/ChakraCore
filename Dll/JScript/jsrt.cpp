//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "JsrtRuntime.h"
#include "JsrtComException.h"
#include "JsrtExternalObject.h"
#include "rterrors.h"
#include "jsrtprivate.h"
#include "GenerateByteCodeConfig.h"
#include "JsrtInternal.h"
#include "JsrtDelegateWrapper.h"

template <class Fn, class T>
T CallbackWrapper(Fn fn, T default)
{
    T result = default;
    try
    {
        AUTO_NESTED_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

        result = fn();
    }
    catch (Js::OutOfMemoryException)
    {
    }
    catch (Js::StackOverflowException)
    {
    }
    catch (Js::ExceptionBase)
    {
        AssertMsg(false, "Unexpected engine exception.");
    }
    catch (...)
    {
        AssertMsg(false, "Unexpected non-engine exception.");
    }

    return result;
}

template <class Fn>
bool CallbackWrapper(Fn fn)
{
    return CallbackWrapper(fn, false);
}

JsErrorCode RunScriptCore(const wchar_t *script, JsSourceContext sourceContext, const wchar_t *sourceUrl, bool parseOnly, JsValueRef *result)
{    
    Js::JavascriptFunction *scriptFunction;
    CompileScriptException se;

    JsErrorCode errorCode = ContextAPINoScriptWrapper(
        [&] (Js::ScriptContext * scriptContext) -> JsErrorCode {   
            PARAM_NOT_NULL(script);
            PARAM_NOT_NULL(sourceUrl);

            JsrtContext * context = JsrtContext::GetCurrent();
            SourceContextInfo * sourceContextInfo = scriptContext->GetSourceContextInfo(sourceContext, NULL);

            if (sourceContextInfo == NULL)
            {            
                sourceContextInfo = scriptContext->CreateSourceContextInfo(sourceContext, sourceUrl, wcslen(sourceUrl), NULL);            
            }        

            SRCINFO si = {
                /* sourceContextInfo   */ sourceContextInfo,
                /* dlnHost             */ 0,
                /* ulColumnHost        */ 0,
                /* lnMinHost           */ 0,
                /* ichMinHost          */ 0,
                /* ichLimHost          */ wcslen(script),
                /* ulCharOffset        */ 0,
                /* mod                 */ kmodGlobal,
                /* grfsi               */ 0
            };

            Js::Utf8SourceInfo* utf8SourceInfo;
            scriptFunction = scriptContext->LoadScript(script, &si, &se, result != NULL, false, false, &utf8SourceInfo, Js::Constants::GlobalCode);

            if (scriptFunction != NULL)
            {
                if (context->GetScriptEngine()->CanRegisterDebugSources())
                {   
                    ScriptEngine * scriptEngine = context->GetScriptEngine();
                    // It is safer to ensure it is deserialized at this point before createing a cscriptbody.
                    if(CONFIG_FLAG(ForceSerialized) && scriptFunction->GetFunctionProxy() != null) {
                        scriptFunction->GetFunctionProxy()->EnsureDeserialized();
                    }
                    CScriptBody* pbody = HeapNew(CScriptBody, scriptFunction->GetFunctionBody(), scriptEngine, utf8SourceInfo);            
                    JsrtComException::ThrowIfFailed(scriptEngine->DbgRegisterScriptBlock(pbody));    
                    pbody->Release();
                }
            }
            return JsNoError;
    });

    if (errorCode != JsNoError)
    {
        return errorCode;
    }

    return ContextAPIWrapper<false>([&](Js::ScriptContext* scriptContext) -> JsErrorCode {
        if (scriptFunction == NULL)
        {
            HandleScriptCompileError(scriptContext, &se);
            return JsErrorScriptCompile;
        }             

        if (parseOnly)
        {
            PARAM_NOT_NULL(result);
            *result = scriptFunction;
        }
        else
        {
            Js::Arguments args(0, NULL);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            Js::Var varThis;
            if (PHASE_FORCE1(Js::EvalCompilePhase))
            {
                varThis = Js::JavascriptOperators::OP_GetThis(scriptContext->GetLibrary()->GetUndefined(), kmodGlobal, scriptContext);
                args.Info.Flags = (Js::CallFlags)CallFlags_Eval;
                args.Info.Count = 1;
                args.Values = &varThis;
            }
#endif
            Js::Var varResult = Js::JavascriptFunction::CallFunction<true>(scriptFunction,  scriptFunction->GetEntryPoint(), args);
            if (result != NULL)
            {
                *result = varResult;
            }
        }
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsParseScript(const wchar_t * script, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunScriptCore(script, sourceContext, sourceUrl, true, result);
}

STDAPI_(JsErrorCode) JsRunScript(const wchar_t * script, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunScriptCore(script, sourceContext, sourceUrl, false, result);
}

JsErrorCode JsSerializeScriptCore(const wchar_t *script, BYTE *functionTable, int functionTableSize, unsigned char *buffer, unsigned long *bufferSize, bool serializeNative)
{
    Js::JavascriptFunction *function;
    CompileScriptException se;

    JsErrorCode errorCode = ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(script);
        PARAM_NOT_NULL(bufferSize);

        if (*bufferSize > 0)
        {
            PARAM_NOT_NULL(buffer);
            ZeroMemory(buffer, *bufferSize);
        }

        if (scriptContext->IsInDebugMode())
        {
            return JsErrorCannotSerializeDebugScript;
        }

        SourceContextInfo * sourceContextInfo = scriptContext->GetSourceContextInfo(JS_SOURCE_CONTEXT_NONE, NULL);
        Assert(sourceContextInfo != nullptr);

        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ 0,
            /* ulColumnHost        */ 0,
            /* lnMinHost           */ 0,
            /* ichMinHost          */ 0,
            /* ichLimHost          */ wcslen(script),
            /* ulCharOffset        */ 0,
            /* mod                 */ kmodGlobal,
            /* grfsi               */ 0
        };

        Js::Utf8SourceInfo* sourceInfo;
        function = scriptContext->LoadScript(script, &si, &se, true, true, serializeNative, &sourceInfo, Js::Constants::GlobalCode);
        return JsNoError;
    });

    if (errorCode != JsNoError)
    {
        return errorCode;
    }

    return ContextAPIWrapper<false>([&](Js::ScriptContext* scriptContext) -> JsErrorCode {
        if (function == NULL)
        {
            HandleScriptCompileError(scriptContext, &se);
            return JsErrorScriptCompile;
        }
        // Could we have a deserialized function in this case?
        // If we are going to seralize it, a check isn't to expensive
        if (CONFIG_FLAG(ForceSerialized) && function->GetFunctionProxy() != null) {
            function->GetFunctionProxy()->EnsureDeserialized();
        }
        Js::FunctionBody *functionBody = function->GetFunctionBody();
        const Js::Utf8SourceInfo *sourceInfo = functionBody->GetUtf8SourceInfo();
        DWORD dwSourceCodeLength = sourceInfo->GetCbLength(L"JsSerializeScript");
        LPCUTF8 utf8Code = sourceInfo->GetSource(L"JsSerializeScript");

        BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"ByteCodeSerializer");
        HRESULT hr = S_OK;
        if (serializeNative)
        {
            BYTE *nativeCode = nullptr;
            DWORD dwNativeCodeSize = 0;
            BYTE *byteCodeBuffer = nullptr;
            DWORD dwByteCodeSize = 0;
            hr = Js::ByteCodeSerializer::SerializeToBuffer(scriptContext, tempAllocator, dwSourceCodeLength, utf8Code, 0, nullptr, functionBody, functionBody->GetHostSrcInfo(), true, &byteCodeBuffer, &dwByteCodeSize, GENERATE_BYTE_CODE_FOR_NATIVE);
            if (SUCCEEDED(hr))
            {
                GenerateAllFunctionsForSerialization(scriptContext->GetNativeCodeGenerator(), functionBody, nullptr, 0, byteCodeBuffer, dwByteCodeSize, functionTableSize, functionTable, &nativeCode, &dwNativeCodeSize);

                if (*bufferSize >= dwNativeCodeSize)
                {
                    memcpy_s(buffer, *bufferSize, nativeCode, dwNativeCodeSize);
                }
                else if (buffer != nullptr)
                {
                    return JsErrorInvalidArgument;
                }

                *bufferSize = dwNativeCodeSize;
                CoTaskMemFree(nativeCode);
                CoTaskMemFree(byteCodeBuffer);
            }
        }
        else
        {
            hr = Js::ByteCodeSerializer::SerializeToBuffer(scriptContext, tempAllocator, dwSourceCodeLength, utf8Code, 0, nullptr, functionBody, functionBody->GetHostSrcInfo(), false, &buffer, bufferSize);
        }
        END_TEMP_ALLOCATOR(tempAllocator, scriptContext);

        if (SUCCEEDED(hr))
        {
            return JsNoError;
        }
        else
        {
            return JsErrorScriptCompile;
        }
    });

}

STDAPI_(JsErrorCode) JsSerializeScript(const wchar_t *script, unsigned char *buffer, unsigned long *bufferSize)
{
    return JsSerializeScriptCore(script, nullptr, 0, buffer, bufferSize, Js::Configuration::Global.flags.IncludeNativeCodeWithSerializedByteCodes);
}

STDAPI_(JsErrorCode) JsSerializeNativeScript(const wchar_t *script, BYTE *functionTable, int functionTableSize, unsigned char *buffer, unsigned long *bufferSize)
{
    return JsSerializeScriptCore(script, functionTable, functionTableSize, buffer, bufferSize, true);
}


JsErrorCode RunSerializedScriptCore(const wchar_t *script, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, bool parseOnly, JsValueRef *result)
{
    Js::JavascriptFunction *function;
    JsErrorCode errorCode = ContextAPINoScriptWrapper([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(script);
        PARAM_NOT_NULL(buffer);
        PARAM_NOT_NULL(sourceUrl);

        size_t length = wcslen(script);
        if (length > UINT_MAX)
        {
            return JsErrorOutOfMemory;
        }

        size_t cbUtf8Buffer = (length + 1) * 3;
        if (cbUtf8Buffer > UINT_MAX)
        {
            return JsErrorOutOfMemory;
        }

        LPUTF8 utf8Script = RecyclerNewArrayLeaf(scriptContext->GetRecycler(), utf8char_t, cbUtf8Buffer);

        Assert(length < MAXLONG);
        utf8::EncodeIntoAndNullTerminate(utf8Script, script, static_cast<charcount_t>(length));

        JsrtContext * context = JsrtContext::GetCurrent();
        SourceContextInfo *sourceContextInfo;
        SRCINFO *hsi;
        Js::FunctionBody *functionBody = NULL;

        HRESULT hr;

        sourceContextInfo = scriptContext->GetSourceContextInfo(sourceContext, NULL);

        if (sourceContextInfo == NULL)
        {
            sourceContextInfo = scriptContext->CreateSourceContextInfo(sourceContext, sourceUrl, wcslen(sourceUrl), NULL);
        }

        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ 0,
            /* ulColumnHost        */ 0,
            /* lnMinHost           */ 0,
            /* ichMinHost          */ 0,
            /* ichLimHost          */ wcslen(script),
            /* ulCharOffset        */ 0,
            /* mod                 */ kmodGlobal,
            /* grfsi               */ 0
        };

        ulong flags = 0;

        if (CONFIG_FLAG(CreateFunctionProxy) && !scriptContext->IsProfiling())
        {
            flags = fscrAllowFunctionProxy;
        }

        hsi = scriptContext->AddHostSrcInfo(&si);
        hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, flags, utf8Script, hsi, buffer, nullptr, &functionBody);

        if (FAILED(hr))
        {
            return JsErrorBadSerializedScript;
        }

        function = scriptContext->GetLibrary()->CreateScriptFunction(functionBody);

        if (context->GetScriptEngine()->CanRegisterDebugSources() || scriptContext->IsProfiling())
        {   
            ScriptEngine * scriptEngine = context->GetScriptEngine();
            CScriptBody* pbody = HeapNew(CScriptBody, functionBody, scriptEngine, functionBody->GetUtf8SourceInfo());

            if (context->GetScriptEngine()->CanRegisterDebugSources())
            {
                HRESULT hr = scriptEngine->DbgRegisterScriptBlock(pbody);
                if (FAILED(hr))
                {
                    pbody->Release();
                    JsrtComException::ThrowIfFailed(hr);
                }
            }

            if (scriptContext->IsProfiling())
            {
                scriptContext->RegisterScript(pbody->GetRootFunction());
            }

            pbody->Release();
        }

        return JsNoError;
    });

    if (errorCode != JsNoError)
    {
        return errorCode;
    }

    return ContextAPIWrapper<false>([&](Js::ScriptContext* scriptContext) -> JsErrorCode {
        if (parseOnly)
        {
            PARAM_NOT_NULL(result);
            *result = function;
        }
        else
        {
            Js::Var varResult = Js::JavascriptFunction::CallFunction<true>(function, function->GetEntryPoint(), Js::Arguments(0, NULL));
            if (result != NULL)
            {
                *result = varResult;
            }
        }
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsParseSerializedScript(const wchar_t * script, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunSerializedScriptCore(script, buffer, sourceContext, sourceUrl, true, result);
}

STDAPI_(JsErrorCode) JsRunSerializedScript(const wchar_t * script, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunSerializedScriptCore(script, buffer, sourceContext, sourceUrl, false, result);
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
        PARAM_NOT_NULL(object);
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
    JsrtContext *context = JsrtContext::GetCurrent();
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

            JsrtContext * context = JsrtContext::GetCurrent();

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
            JsrtContext * context = JsrtContext::GetCurrent();

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

        JsrtContext* context = JsrtContext::GetCurrent();
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
        JsrtContext* context = JsrtContext::GetCurrent();
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
        Assert(S_OK == inspectable->QueryInterface(IID_IInspectable, (void**)&dbgInspectable));
        Assert(FAILED(inspectable->QueryInterface(IID_IDispatchEx, (void**)&dbgDispatch)));
        END_LEAVE_SCRIPT(scriptContext)
#endif
        JsrtContext* context = JsrtContext::GetCurrent();
        HRESULT hr = NOERROR;
        ScriptEngine* scriptEngine = context->GetScriptEngine();
        if (scriptEngine->GetProjectionContext() == nullptr)
        {
            return JsErrorCannotStartProjection;
        }
        BEGIN_LEAVE_SCRIPT(scriptContext)
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
        PARAM_NOT_NULL(value);
        PARAM_NOT_NULL(inspectable);
        *inspectable = nullptr;
        // Check if projection is enabled, if not return error
        JsrtContext* context = JsrtContext::GetCurrent();
        ScriptEngine* scriptEngine = context->GetScriptEngine();
        if (scriptEngine->GetProjectionContext() == nullptr)
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
            hr = externalObject->QueryObjectInterface(IID_IInspectable, (void**)inspectable);
        END_LEAVE_SCRIPT(scriptContext)
            if (FAILED(hr))
            {
            return JsErrorObjectNotInspectable;
            }
        return JsNoError;
    });
}
