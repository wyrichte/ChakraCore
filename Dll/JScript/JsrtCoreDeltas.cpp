//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include <stdafx.h>
#include "jsrtprivate.h"
#include "jsrtInternal.h"
#include "GenerateByteCodeConfig.h"
#include "JsrtExternalObject.h"
#include "JsrtComException.h"

JsErrorCode RunScriptCore(const wchar_t *script, JsSourceContext sourceContext, const wchar_t *sourceUrl, bool parseOnly, JsValueRef *result)
{    
    Js::JavascriptFunction *scriptFunction;
    CompileScriptException se;

    JsErrorCode errorCode = ContextAPINoScriptWrapper(
        [&] (Js::ScriptContext * scriptContext) -> JsErrorCode {   
            PARAM_NOT_NULL(script);
            PARAM_NOT_NULL(sourceUrl);

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

JsErrorCode JsParseScript(const wchar_t * script, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunScriptCore(script, sourceContext, sourceUrl, true, result);
}

JsErrorCode JsRunScript(const wchar_t * script, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunScriptCore(script, sourceContext, sourceUrl, false, result);
}

JsErrorCode JsSerializeScriptCore(const wchar_t *script, unsigned char *buffer, unsigned long *bufferSize)
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
        function = scriptContext->LoadScript(script, &si, &se, true, true, false, &sourceInfo, Js::Constants::GlobalCode);
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
        HRESULT hr = Js::ByteCodeSerializer::SerializeToBuffer(scriptContext, tempAllocator, dwSourceCodeLength, utf8Code, 0, nullptr, functionBody, functionBody->GetHostSrcInfo(), false, &buffer, bufferSize);
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

JsErrorCode JsSerializeScript(const wchar_t *script, unsigned char *buffer, unsigned long *bufferSize)
{
    return JsSerializeScriptCore(script, buffer, bufferSize);
}

JsErrorCode RunSerializedScriptCore(const wchar_t *script, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, bool parseOnly, JsValueRef *result)
{
    Js::JavascriptFunction *function;
    JsErrorCode errorCode = ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
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

JsErrorCode JsParseSerializedScript(const wchar_t * script, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunSerializedScriptCore(script, buffer, sourceContext, sourceUrl, true, result);
}

JsErrorCode JsRunSerializedScript(const wchar_t * script, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunSerializedScriptCore(script, buffer, sourceContext, sourceUrl, false, result);
}
