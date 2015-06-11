//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"

#define MAX_PROGID_LENGTH 39

Js::FunctionInfo AsyncDebug::EntryInfo::BeginAsyncOperation(AsyncDebug::BeginAsyncOperation, Js::FunctionInfo::None);
Js::FunctionInfo AsyncDebug::EntryInfo::BeginAsyncCallback(AsyncDebug::BeginAsyncCallback, Js::FunctionInfo::None);
Js::FunctionInfo AsyncDebug::EntryInfo::CompleteAsyncCallback(AsyncDebug::CompleteAsyncCallback, Js::FunctionInfo::None);
Js::FunctionInfo AsyncDebug::EntryInfo::CompleteAsyncOperation(AsyncDebug::CompleteAsyncOperation, Js::FunctionInfo::None);
Js::FunctionInfo AsyncDebug::EntryInfo::UpdateAsyncCallbackStatus(AsyncDebug::UpdateAsyncCallbackStatus, Js::FunctionInfo::None);

const GUID AsyncDebug::ChakraPlatformGUID = { 0x3ceff62f, 0xe251, 0x4588, { 0xae, 0x37, 0x9d, 0x6d, 0x74, 0x8a, 0xcd, 0x49 } };
const AsyncDebug::AsyncSource AsyncDebug::ChakraAsyncSource = AsyncDebug::AsyncSource_Library;
const AsyncDebug::AsyncOperationId AsyncDebug::InvalidAsyncOperationId = 0;
AsyncDebug::AsyncOperationId AsyncDebug::nextAsyncOperationId = 0;
char AsyncDebug::isDownlevel = -1;

// Forward-declare downlevel causaily API defined in RoCausality.cpp.
HRESULT RoCausalityTraceAsyncOperationCreation(
    _In_     CausalityTraceLevel     traceLevel,
    _In_     CausalitySource         source,
    _In_     GUID                    platformId,
    _In_     UINT64                  operationId, 
    _In_opt_ PCWSTR                  operationName,
    _In_opt_ UINT64                  relatedId);

HRESULT RoCausalityTraceAsyncOperationCompletion(
    _In_     CausalityTraceLevel       traceLevel,
    _In_     CausalitySource           source,
    _In_     GUID                      platformId,
    _In_     UINT64                    operationId,
    _In_     AsyncStatus               completionStatus);

HRESULT RoCausalityTraceSynchronousWorkItemStart(
    _In_     CausalityTraceLevel      traceLevel,
    _In_     CausalitySource          source,
    _In_     GUID                     platformId,
    _In_     UINT64                   operationId,
    _In_     CausalitySynchronousWork work);

HRESULT RoCausalityTraceSynchronousWorkItemCompletion(
    _In_     CausalityTraceLevel      traceLevel,
    _In_     CausalitySource          source,
    _In_     CausalitySynchronousWork work);

HRESULT RoCausalityTraceAsyncOperationRelation(
    _In_     CausalityTraceLevel     traceLevel,
    _In_     CausalitySource         source,
    _In_     GUID                    platformId,
    _In_     UINT64                  operationId,
    _In_     CausalityRelation       relation);

AsyncDebug::AsyncOperationId AsyncDebug::GetNextAsyncOperationId()
{
    // TODO: For now just use a simple counter in AsyncDebug. 
    //       We would ideally use the address of the IAsyncInfo associated with the promise 
    //       but we have to have the id before we make the WinRT call, before we read out 
    //       the out parameters. There's also the Javascript-side API to support which doesn't 
    //       have an IAsyncInfo or promise object.
    AsyncDebug::AsyncOperationId val = InterlockedIncrement(&nextAsyncOperationId);

    // Skip InvalidAsyncOperationId in order to keep one special-case value.
    if (val == AsyncDebug::InvalidAsyncOperationId)
    {
        return GetNextAsyncOperationId();
    }

    return val;
}

bool AsyncDebug::IsAsyncDebuggingEnabled(Js::ScriptContext* scriptContext)
{
    return CONFIG_FLAG(AsyncDebugging);
}

bool AsyncDebug::IsDownlevel(Js::ScriptContext* scriptContext)
{
    // isDownlevel is a tri-state. 
    //   -1 = not calculated. 
    //    1 = downlevel. 
    //    0 = not downlevel.
    if (AsyncDebug::isDownlevel == -1)
    {
        // If foundation object isn't available (or disabled), we are running on Win7.
#ifdef ENABLE_FOUNDATION_OBJECT
        AsyncDebug::isDownlevel = scriptContext->GetThreadContext()->GetWinRtFoundationLibrary()->IsAvailable() ? 0 : 1;
#else
        AsyncDebug::isDownlevel = 1;
#endif

        // We can force the use of the downlevel causality API via a flag.
        if (CONFIG_FLAG(ForceDownlevelAsyncDebug))
        {
            AsyncDebug::isDownlevel = 1;
        }

        // Emit a one-time message that we will be using the downlevel causality API.
        if (CONFIG_FLAG(TraceAsyncDebugCalls) && AsyncDebug::isDownlevel == 1)
        {
            Output::Print(L"Using downlevel causality API (RoCausality) instead of WinRT-based API.\n");
            Output::Flush();
        }
    }

    return AsyncDebug::isDownlevel == 1;
}

HRESULT AsyncDebug::HostWrapperForTraceOperationCreation(Js::ScriptContext* scriptContext, LogLevel logLevel, AsyncDebug::AsyncOperationId operationId, LPCWSTR operationName)
{
    return HostWrapperForTraceOperationCreation(scriptContext, logLevel, ChakraPlatformGUID, operationId, operationName);
}

HRESULT AsyncDebug::HostWrapperForTraceOperationCreation(Js::ScriptContext* scriptContext, LogLevel logLevel, GUID platformId, AsyncDebug::AsyncOperationId operationId, LPCWSTR operationName)
{
    HRESULT hr = S_OK;

    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        hr = WrapperForTraceOperationCreation(scriptContext, logLevel, platformId, operationId, operationName);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    return hr;
}

HRESULT AsyncDebug::ScriptWrapperForTraceOperationCreation(Js::ScriptContext* scriptContext, LogLevel logLevel, AsyncOperationId operationId, LPCWSTR operationName)
{
    HRESULT hr = S_OK;

    try
    {
        hr = WrapperForTraceOperationCreation(scriptContext, logLevel, operationId, operationName);
    }
    catch (Js::OutOfMemoryException)
    {
        hr = E_OUTOFMEMORY;
    }
    catch (Js::StackOverflowException)
    {
        hr = VBSERR_OutOfStack;
    }
    catch (...)
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT AsyncDebug::WrapperForTraceOperationCreation(Js::ScriptContext* scriptContext, LogLevel logLevel, AsyncDebug::AsyncOperationId operationId, LPCWSTR operationName)
{
    return WrapperForTraceOperationCreation(scriptContext, logLevel, ChakraPlatformGUID, operationId, operationName);
}

HRESULT AsyncDebug::WrapperForTraceOperationCreation(Js::ScriptContext* scriptContext, LogLevel logLevel, GUID platformId, AsyncDebug::AsyncOperationId operationId, LPCWSTR operationName)
{
    HRESULT hr = S_OK;

    // Current implementation passes nullptr in relatedContextId field. Previously we passed a pointer which 
    // causality API would QI for IActiveScriptStackWalkerProvider in order to get a StackWalker.
    UINT64 relatedContext = 0;
    AsyncSource source = ChakraAsyncSource;

    Assert(scriptContext);

    if (logLevel < LogLevel_Required || logLevel > LogLevel_Last)
    {
        logLevel = LogLevel_Required;
    }

    if (!operationName)
    {
        operationName = L"";
    }

#if F_JSETW
    // Walk the stack if debugger is attached and the listener is active for the ETW event or if we are tracing async calls.
    // Tracing should only be enabled for unit tests or manually troubleshooting and is easier to configure than an ETW consumer.
    if (scriptContext->IsInDebugMode() && (EventEnabledJSCRIPT_ASYNCCAUSALITY_STACKTRACE() || CONFIG_FLAG(TraceAsyncDebugCalls)))
    {
        EmitStackWalk(scriptContext, operationId);
    }

    if (EventEnabledJSCRIPT_ASYNCCAUSALITY_STACKTRACE_V2() || PHASE_TRACE1(Js::StackFramesEventPhase))
    {
        scriptContext->EmitStackTraceEvent(operationId, (ushort)Js::JavascriptExceptionOperators::DefaultStackTraceLimit, true /*emitV2AsyncStackEvent*/);
    }
#endif

    if (CONFIG_FLAG(TraceAsyncDebugCalls))
    {
        WCHAR guidStr[MAX_PROGID_LENGTH];

        StringFromGUID2(platformId, guidStr, ARRAYSIZE(guidStr));

        Output::Print(L"Calling AsyncCausalityTracer.TraceOperationCreation(traceLevel=%d, source=%d, platformId=%s, operationId=%llu, operationName=%s, relatedContext=%llu)\n", logLevel, source, guidStr, operationId, operationName, relatedContext);
        Output::Flush();
    }

    if (IsDownlevel(scriptContext))
    {
        hr = RoCausalityTraceAsyncOperationCreation((CausalityTraceLevel)logLevel, (CausalitySource)source, platformId, operationId, operationName, relatedContext);
    }
    else
    {
#ifdef ENABLE_FOUNDATION_OBJECT
        hr = scriptContext->GetThreadContext()->GetWindowsFoundationAdapter()->TraceOperationCreation(scriptContext, logLevel, source, platformId, operationId, operationName, relatedContext);
#endif
    }

    return hr;
}

HRESULT AsyncDebug::WrapperForTraceOperationCompletion(Js::ScriptContext* scriptContext, LogLevel logLevel, AsyncDebug::AsyncOperationId operationId, AsyncOperationStatus status) 
{
    return WrapperForTraceOperationCompletion(scriptContext, logLevel, ChakraPlatformGUID, operationId, status);
}

HRESULT AsyncDebug::WrapperForTraceOperationCompletion(Js::ScriptContext* scriptContext, LogLevel logLevel, GUID platformId, AsyncDebug::AsyncOperationId operationId, AsyncOperationStatus status) 
{
    HRESULT hr = S_OK;
    AsyncSource source = ChakraAsyncSource;

    Assert(scriptContext);

    if (logLevel < LogLevel_Required || logLevel > LogLevel_Last)
    {
        logLevel = LogLevel_Required;
    }

    if (CONFIG_FLAG(TraceAsyncDebugCalls))
    {
        WCHAR guidStr[MAX_PROGID_LENGTH];
        
        StringFromGUID2(platformId, guidStr, ARRAYSIZE(guidStr));

        Output::Print(L"Calling AsyncCausalityTracer.TraceOperationCompletion(traceLevel=%d, source=%d, platformId=%s, operationId=%llu, status=%d)\n", logLevel, source, guidStr, operationId, status);
        Output::Flush();
    }

    if (IsDownlevel(scriptContext))
    {
        hr = RoCausalityTraceAsyncOperationCompletion((CausalityTraceLevel)logLevel, (CausalitySource)source, platformId, operationId, (AsyncStatus)status);
    }
    else
    {
#ifdef ENABLE_FOUNDATION_OBJECT
        hr = scriptContext->GetThreadContext()->GetWindowsFoundationAdapter()->TraceOperationCompletion(scriptContext, logLevel, source, platformId, operationId, status);
#endif
    }

    return hr;
}

HRESULT AsyncDebug::WrapperForTraceSynchronousWorkStart(Js::ScriptContext* scriptContext, LogLevel logLevel, AsyncDebug::AsyncOperationId operationId, AsyncCallbackType workType)
{
    return WrapperForTraceSynchronousWorkStart(scriptContext, logLevel, ChakraPlatformGUID, operationId, workType);
}

HRESULT AsyncDebug::WrapperForTraceSynchronousWorkStart(Js::ScriptContext* scriptContext, LogLevel logLevel, GUID platformId, AsyncDebug::AsyncOperationId operationId, AsyncCallbackType workType)
{
    HRESULT hr = S_OK;
    AsyncSource source = ChakraAsyncSource;

    Assert(scriptContext);

    if (logLevel < LogLevel_Required || logLevel > LogLevel_Last)
    {
        logLevel = LogLevel_Required;
    }

    if (workType < AsyncCallbackType_Completion || workType > AsyncCallbackType_Last)
    {
        workType = AsyncCallbackType_Completion;
    }

    if (CONFIG_FLAG(TraceAsyncDebugCalls))
    {
        WCHAR guidStr[MAX_PROGID_LENGTH];

        StringFromGUID2(platformId, guidStr, ARRAYSIZE(guidStr));

        Output::Print(L"Calling AsyncCausalityTracer.TraceSynchronousWorkStart(traceLevel=%d, source=%d, platformId=%s, operationId=%llu, work=%d)\n", logLevel, source, guidStr, operationId, workType);
        Output::Flush();
    }

    if (IsDownlevel(scriptContext))
    {
        hr = RoCausalityTraceSynchronousWorkItemStart((CausalityTraceLevel)logLevel, (CausalitySource)source, platformId, operationId, (CausalitySynchronousWork)workType);
    }
    else
    {
#ifdef ENABLE_FOUNDATION_OBJECT
        hr = scriptContext->GetThreadContext()->GetWindowsFoundationAdapter()->TraceSynchronousWorkStart(scriptContext, logLevel, source, platformId, operationId, workType);
#endif
    }

    return hr;
}

HRESULT AsyncDebug::WrapperForTraceSynchronousWorkCompletion(Js::ScriptContext* scriptContext, LogLevel logLevel)
{
    HRESULT hr = S_OK;
    AsyncSource source = ChakraAsyncSource;
    // TODO: Update Chakra functional spec to add work argument to msCompleteAsyncCallback.
    AsyncCallbackType workType = AsyncCallbackType_Completion;

    Assert(scriptContext);

    if (logLevel < LogLevel_Required || logLevel > LogLevel_Last)
    {
        logLevel = LogLevel_Required;
    }

    if (workType < AsyncCallbackType_Completion || workType > AsyncCallbackType_Last)
    {
        workType = AsyncCallbackType_Completion;
    }

    if (CONFIG_FLAG(TraceAsyncDebugCalls))
    {
        Output::Print(L"Calling AsyncCausalityTracer.TraceSynchronousWorkCompletion(traceLevel=%d, source=%d, work=%d)\n", logLevel, source, workType);
        Output::Flush();
    }

    if (IsDownlevel(scriptContext))
    {
        hr = RoCausalityTraceSynchronousWorkItemCompletion((CausalityTraceLevel)logLevel, (CausalitySource)source, (CausalitySynchronousWork)workType);
    }
    else
    {
#ifdef ENABLE_FOUNDATION_OBJECT
        hr = scriptContext->GetThreadContext()->GetWindowsFoundationAdapter()->TraceSynchronousWorkCompletion(scriptContext, logLevel, source, workType);
#endif
    }

    return hr;
}

HRESULT AsyncDebug::WrapperForTraceOperationRelation(Js::ScriptContext* scriptContext, LogLevel logLevel, AsyncDebug::AsyncOperationId operationId, AsyncCallbackStatus relation)
{
    return WrapperForTraceOperationRelation(scriptContext, logLevel, ChakraPlatformGUID, operationId, relation);
}

HRESULT AsyncDebug::WrapperForTraceOperationRelation(Js::ScriptContext* scriptContext, LogLevel logLevel, GUID platformId, AsyncDebug::AsyncOperationId operationId, AsyncCallbackStatus relation)
{
    HRESULT hr = S_OK;
    AsyncSource source = ChakraAsyncSource;

    Assert(scriptContext);

    if (logLevel < LogLevel_Required || logLevel > LogLevel_Last)
    {
        logLevel = LogLevel_Required;
    }

    if (CONFIG_FLAG(TraceAsyncDebugCalls))
    {
        WCHAR guidStr[MAX_PROGID_LENGTH];

        StringFromGUID2(platformId, guidStr, ARRAYSIZE(guidStr));

        Output::Print(L"Calling AsyncCausalityTracer.TraceOperationRelation(traceLevel=%d, source=%d, platformId=%s, operationId=%llu, relation=%d)\n", logLevel, source, guidStr, operationId, relation);
        Output::Flush();
    }
    
    if (IsDownlevel(scriptContext))
    {
        hr = RoCausalityTraceAsyncOperationRelation((CausalityTraceLevel)logLevel, (CausalitySource)source, platformId, operationId, (CausalityRelation)relation);
    }
    else
    {
#ifdef ENABLE_FOUNDATION_OBJECT
        hr = scriptContext->GetThreadContext()->GetWindowsFoundationAdapter()->TraceOperationRelation(scriptContext, logLevel, source, platformId, operationId, relation);
#endif
    }

    return hr;
}

Js::Var AsyncDebug::BeginAsyncOperation(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);

    auto scriptContext = function->GetScriptContext();
    
    if (!AsyncDebug::IsAsyncDebuggingEnabled(scriptContext))
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
    Assert(scriptContext->GetThreadContext()->IsScriptActive());

    // operationName
    Js::JavascriptString* operationName = nullptr;
    LPCWSTR operationNameStr = nullptr;

    if (args.Info.Count >= 2 && args[1] != nullptr)
    {
        operationName = Js::JavascriptConversion::ToString(args[1], scriptContext);
    }
    else
    {
        operationName = function->GetLibrary()->GetEmptyString();
    }
        
    if (operationName != nullptr)
    {
        operationNameStr = operationName->GetSz();
    }

    // logLevel
    int logLevel = LogLevel_Required;

    if (args.Info.Count >= 3 && args[2] != nullptr)
    {
        logLevel = Js::JavascriptConversion::ToInt32(args[2], scriptContext);

        if (logLevel > LogLevel_Last || logLevel < LogLevel_Required)
        {
            logLevel = LogLevel_Required;
        }
    }
    
    // Get unique operationId.
    AsyncOperationId operationId = AsyncDebug::GetNextAsyncOperationId();

    // Call the causality wrapper. The caller is JS, so we don't want to return the hr from the causality API.
    ScriptWrapperForTraceOperationCreation(scriptContext, (LogLevel)logLevel, operationId, operationNameStr);

    return Js::JavascriptNumber::ToVar(operationId, scriptContext);
}

Js::Var AsyncDebug::BeginAsyncCallback(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);

    auto scriptContext = function->GetScriptContext();
    
    if (!AsyncDebug::IsAsyncDebuggingEnabled(scriptContext))
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
    Assert(scriptContext->GetThreadContext()->IsScriptActive());
        
    // asyncOperationId
    AsyncOperationId asyncOperationId = (AsyncOperationId)-1;

    if (args.Info.Count >= 2 && args[1] != nullptr)
    {
        asyncOperationId = Js::JavascriptConversion::ToUInt64(args[1], scriptContext);
    }

    if (AsyncDebug::InvalidAsyncOperationId == asyncOperationId)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // workType
    int workType = AsyncCallbackType_Completion;

    if (args.Info.Count >= 3 && args[2] != nullptr)
    {
        workType = Js::JavascriptConversion::ToInt32(args[2], scriptContext);

        if (workType > AsyncCallbackType_Last || workType < AsyncCallbackType_Completion)
        {
            workType = AsyncCallbackType_Completion;
        }
    }

    // logLevel
    int logLevel = LogLevel_Required;

    if (args.Info.Count >= 4 && args[3] != nullptr)
    {
        logLevel = Js::JavascriptConversion::ToInt32(args[3], scriptContext);

        if (logLevel > LogLevel_Last || logLevel < LogLevel_Required)
        {
            logLevel = LogLevel_Required;
        }
    }

    // Call the causality wrapper. The caller is JS, so we don't want to return the hr from the causality API.
    WrapperForTraceSynchronousWorkStart(scriptContext, (LogLevel)logLevel, asyncOperationId, (AsyncCallbackType)workType);

    return scriptContext->GetLibrary()->GetUndefined();
}

Js::Var AsyncDebug::CompleteAsyncCallback(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);

    auto scriptContext = function->GetScriptContext();
    
    if (!AsyncDebug::IsAsyncDebuggingEnabled(scriptContext))
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
    Assert(scriptContext->GetThreadContext()->IsScriptActive());

    // logLevel
    int logLevel = LogLevel_Required;

    if (args.Info.Count >= 2 && args[1] != nullptr)
    {
        logLevel = Js::JavascriptConversion::ToInt32(args[1], scriptContext);

        if (logLevel > LogLevel_Last || logLevel < LogLevel_Required)
        {
            logLevel = LogLevel_Required;
        }
    }

    // asyncOperationId (optional)
    // If user specifies a second parameter, assume it is an AsyncOperationId.
    // The REX API for AsyncCallbackCompletion does not take an AsyncOperationId so 
    // the only thing to do with this value is to filter out invalid ids.
    if (args.Info.Count >= 3 && args[2] != nullptr)
    {
        AsyncOperationId asyncOperationId = Js::JavascriptConversion::ToUInt64(args[2], scriptContext);

        if (AsyncDebug::InvalidAsyncOperationId == asyncOperationId)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
    }

    // Call the causality wrapper. The caller is JS, so we don't want to return the hr from the causality API.
    WrapperForTraceSynchronousWorkCompletion(scriptContext, (LogLevel)logLevel);

    return scriptContext->GetLibrary()->GetUndefined();
}

Js::Var AsyncDebug::UpdateAsyncCallbackStatus(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);

    auto scriptContext = function->GetScriptContext();
    
    if (!AsyncDebug::IsAsyncDebuggingEnabled(scriptContext))
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
    Assert(scriptContext->GetThreadContext()->IsScriptActive());

    // relatedAsyncOperationID
    AsyncOperationId relatedAsyncOperationID = (AsyncOperationId)-1;

    if (args.Info.Count >= 2 && args[1] != nullptr)
    {
        relatedAsyncOperationID = Js::JavascriptConversion::ToUInt64(args[1], scriptContext);
    }

    if (AsyncDebug::InvalidAsyncOperationId == relatedAsyncOperationID)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // relationType
    int relationType = AsyncCallbackStatus_Invalid;

    if (args.Info.Count >= 3 && args[2] != nullptr)
    {
        relationType = Js::JavascriptConversion::ToInt32(args[2], scriptContext);

        if (relationType > AsyncCallbackStatus_Last || relationType < AsyncCallbackStatus_AssignDelegate)
        {
            relationType = AsyncCallbackStatus_Invalid;
        }
    }

    // If relationType is undefined or invalid, silently fail.
    if (relationType == AsyncCallbackStatus_Invalid)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // logLevel
    int logLevel = LogLevel_Required;

    if (args.Info.Count >= 4 && args[3] != nullptr)
    {
        logLevel = Js::JavascriptConversion::ToInt32(args[3], scriptContext);

        if (logLevel > LogLevel_Last || logLevel < LogLevel_Required)
        {
            logLevel = LogLevel_Required;
        }
    }

    // Call the causality wrapper. The caller is JS, so we don't want to return the hr from the causality API.
    WrapperForTraceOperationRelation(scriptContext, (LogLevel)logLevel, relatedAsyncOperationID, (AsyncCallbackStatus)relationType);

    return scriptContext->GetLibrary()->GetUndefined();
}

Js::Var AsyncDebug::CompleteAsyncOperation(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);

    auto scriptContext = function->GetScriptContext();
    
    if (!AsyncDebug::IsAsyncDebuggingEnabled(scriptContext))
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
    Assert(scriptContext->GetThreadContext()->IsScriptActive());

    // asyncOperationID
    AsyncOperationId asyncOperationID = (AsyncOperationId)-1;

    if (args.Info.Count >= 2 && args[1] != nullptr)
    {
        asyncOperationID = Js::JavascriptConversion::ToUInt64(args[1], scriptContext);
    }

    if (AsyncDebug::InvalidAsyncOperationId == asyncOperationID)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // status
    int status = AsyncOperationStatus_Completed;

    if (args.Info.Count >= 3 && args[2] != nullptr)
    {
        status = Js::JavascriptConversion::ToInt32(args[2], scriptContext);

        if (status > AsyncOperationStatus_Last || status < AsyncOperationStatus_Started)
        {
            status = AsyncOperationStatus_Completed;
        }
    }

    // logLevel
    int logLevel = LogLevel_Required;

    if (args.Info.Count >= 4 && args[3] != nullptr)
    {
        logLevel = Js::JavascriptConversion::ToInt32(args[3], scriptContext);

        if (logLevel > LogLevel_Last || logLevel < LogLevel_Required)
        {
            logLevel = LogLevel_Required;
        }
    }

    // Call the causality wrapper. The caller is JS, so we don't want to return the hr from the causality API.
    WrapperForTraceOperationCompletion(scriptContext, (LogLevel)logLevel, asyncOperationID, (AsyncOperationStatus)status);

    return scriptContext->GetLibrary()->GetUndefined();
}

// Info:        Helper method for WinRT async method calls.
//              Generates a new async operation id and calls the wrapper to begin tracing the async operation.
// Parameters:  runtimeClassName - Name of the runtime class which the method belongs to.
//              methodName - Name of the WinRT method we are calling.
//              scriptContext - ScriptContext making the call.
// Returns:     The new async operation id generated for this method call.
AsyncDebug::AsyncOperationId AsyncDebug::BeginAsyncOperationForWinRTMethodCall(LPCWSTR runtimeClassName, LPCWSTR methodName, Js::ScriptContext* scriptContext)
{
    auto asyncOperationId = AsyncDebug::GetNextAsyncOperationId();
    
    try
    {
        BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"AsyncDebug");
        {
            Js::StringBuilder<ArenaAllocator> methodNameBuffer(tempAllocator);

            methodNameBuffer.Reset();
            
            if (runtimeClassName)
            {
                methodNameBuffer.AppendSz(runtimeClassName);
                methodNameBuffer.Append('.');
            }

            if (methodName)
            {
                methodNameBuffer.AppendSz(methodName);
            }

            auto methodNameString = methodNameBuffer.Detach();

            AsyncDebug::ScriptWrapperForTraceOperationCreation(scriptContext, AsyncDebug::LogLevel_Required, asyncOperationId, methodNameString);
        }
        END_TEMP_ALLOCATOR(tempAllocator, scriptContext);
    }
    catch (Js::OutOfMemoryException)
    {
        // We don't want to let failures in this debugging feature kill the the app.
        return AsyncDebug::InvalidAsyncOperationId;
    }
    
    return asyncOperationId;
}

// Info:        Append sourceString to stringBuilder after escaping charToEscape with escapeChar.
//              "SomeBadly\0Formed\0String" => "SomeBadly\\\0Formed\\\0String"
// Parameters:  stringBuilder - The Js::StringBuilder to which we should append sourceString.
//              sourceString - The string we want to escape and append to stringBuilder.
//              sourceStringLen - Length of sourceString.
//              escapeChar - Char to use for escaping.
//              charToEscape - The char which we should escape with escapeChar.
// Returns:     Count of chars written to stringBuilder.
charcount_t AsyncDebug::AppendWithEscapeCharacters(Js::StringBuilder<ArenaAllocator>* stringBuilder, const WCHAR* sourceString, charcount_t sourceStringLen, WCHAR escapeChar, WCHAR charToEscape)
{
    const WCHAR* charToEscapePtr = wcschr(sourceString, charToEscape);
    charcount_t charsPadding = 0;

    // Only escape characters if sourceString contains one.
    if (charToEscapePtr)
    {
        charcount_t charsWritten = 0;
        charcount_t charsToAppend = 0;

        while (charToEscapePtr)
        {
            charsToAppend = static_cast<charcount_t>(charToEscapePtr - sourceString) - charsWritten;

            stringBuilder->Append(sourceString + charsWritten, charsToAppend);
            stringBuilder->Append(escapeChar);
            stringBuilder->Append(charToEscape);

            // Keep track of this extra escapeChar character so we can update the buffer length correctly below.
            charsPadding++;

            // charsWritten is a count of the chars from sourceString which have been written - not count of chars Appended to stringBuilder.
            charsWritten += charsToAppend + 1;

            // Find next charToEscape.
            charToEscapePtr++;
            charToEscapePtr = wcschr(charToEscapePtr, charToEscape);
        }

        // Append the final part of the string if there is any left after the final charToEscape.
        if (charsWritten != sourceStringLen)
        {
            charsToAppend = sourceStringLen - charsWritten;
            stringBuilder->Append(sourceString + charsWritten, charsToAppend);
        }
    }
    else
    {
        stringBuilder->AppendSz(sourceString);
    }

    return sourceStringLen + charsPadding;
}

// Info:        Walk the JavaScript stack and emit it via an ETW event.
// Parameters:  scriptContext - The ScriptContext of the executing script.
//              operationId - The async operation id used to correlate the stack to an outstanding operation.
void AsyncDebug::EmitStackWalk(Js::ScriptContext* scriptContext, AsyncDebug::AsyncOperationId operationId)
{
    // If call root level is zero, there is no EntryExitRecord and the stack walk will fail.
    if (scriptContext->GetThreadContext()->GetCallRootLevel() == 0)
    {
        return;
    }

    BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"AsyncDebug")
    {
        const ushort stackTraceLimit = (ushort)Js::JavascriptExceptionOperators::DefaultStackTraceLimit;
        unsigned short nameBufferLength = 0;
        Js::StringBuilder<ArenaAllocator> nameBuffer(tempAllocator);
        AsyncDebug::ETWStackFrame frames[stackTraceLimit];
        Js::JavascriptStackWalker walker(scriptContext);
                
        nameBuffer.Reset();

        if (CONFIG_FLAG(TraceAsyncDebugCalls))
        {
            Output::Print(L"Posting stack trace via ETW:\n");
        }

        ushort frameCount = walker.WalkUntil(stackTraceLimit, [&](Js::JavascriptFunction* function, ushort frameIndex) -> bool
        {
            const WCHAR* name = nullptr;
            frames[frameIndex].sourceLocationStartIndex = 0;
            frames[frameIndex].sourceLocationLength = 0;

            if (function->IsScriptFunction())
            {
                Js::FunctionBody * functionBody = function->GetFunctionBody();

                if (!function->IsLibraryCode()) // For library code we are not getting the statement span
                {
                    if (!functionBody->GetStatementIndexAndLengthAt(walker.GetByteCodeOffset(),
                        &frames[frameIndex].sourceLocationStartIndex,
                        &frames[frameIndex].sourceLocationLength))
                    {
                        // Faild to get the statement.
                        return false;
                    }
                }

                name = functionBody->GetExternalDisplayName();
                auto utf8SourceInfo = functionBody->GetUtf8SourceInfo();
                Assert(utf8SourceInfo);

                frames[frameIndex].documentId = utf8SourceInfo->HasDocumentText() ? (UINT64)utf8SourceInfo->GetDocumentText() : 0;
            }
            else
            {
                frames[frameIndex].documentId = 0;
                name = walker.GetCurrentNativeLibraryEntryName();
            }
            ushort nameLen = AsyncDebug::ProcessNameAndGetLength(&nameBuffer, name);

            frames[frameIndex].nameIndex = nameBufferLength;

            // Keep track of the current length of the buffer. The next nameIndex will be at this position (+1 for each '\\', '\"', and ';' character added above).
            nameBufferLength += nameLen;

            // An overflow occured.
            Assert(nameBufferLength > frames[frameIndex].nameIndex);

            if (CONFIG_FLAG(TraceAsyncDebugCalls))
            {
                Output::Print(L"\tFrame %hu: Name=%hu (%s), SourceLocationLength=%u, SourceLocationStartIndex=%u\n", 
                    frameIndex, frames[frameIndex].nameIndex, name, frames[frameIndex].sourceLocationLength, frames[frameIndex].sourceLocationStartIndex);
            }

            return false;
        }, true/*onlyOnDebugMode*/);

        auto nameBufferString = nameBuffer.Detach();

        if (CONFIG_FLAG(TraceAsyncDebugCalls))
        {
            Output::Print(L"NameBuffer = %s\n", nameBufferString);
        }

        // Account for the terminating null character.
        nameBufferLength++;
        
        JSETW(EventWriteJSCRIPT_ASYNCCAUSALITY_STACKTRACE(operationId, frameCount, nameBufferLength, sizeof(AsyncDebug::ETWStackFrame), frames, nameBufferString));
    }
    END_TEMP_ALLOCATOR(tempAllocator, scriptContext);
}

/*static*/
ushort AsyncDebug::ProcessNameAndGetLength(Js::StringBuilder<ArenaAllocator>* nameBuffer, const WCHAR* name)
{
    Assert(nameBuffer);
    Assert(name);

    ushort nameLen = (ushort)wcslen(name);

    // Surround each function name with quotes and escape any quote characters in the function name itself with '\\'.
    nameBuffer->Append('\"');

    // Adjust nameLen based on any escape characters we added to escape the '\"' in name.
    nameLen = (unsigned short)AppendWithEscapeCharacters(nameBuffer, name, nameLen, '\\', '\"');

    nameBuffer->AppendCppLiteral(L"\";");

    // Add 3 padding characters here - one for initial '\"' character, too.
    nameLen += 3;

    return nameLen;
}

