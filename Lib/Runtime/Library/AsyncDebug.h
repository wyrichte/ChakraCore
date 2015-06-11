//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class AsyncDebug
{
public:
    typedef UINT64 AsyncOperationId;

    // This enum should map to Windows::Foundation::Diagnostics::CausalityTraceLevel.
    // We do not use that enum here to avoid taking a dependency on WinRT.
    enum LogLevel
    {
        LogLevel_Required                   = 0,
        LogLevel_Important                  = 1,
        LogLevel_Verbose                    = 2,
        LogLevel_Last                       = LogLevel_Verbose,
        LogLevel_Invalid                    = LogLevel_Last+1
    };

    // This enum should map to Windows::Foundation::Diagnostics::CausalityRelation.
    // We do not use that enum here to avoid taking a dependency on WinRT.
    enum AsyncCallbackStatus
    {
        AsyncCallbackStatus_AssignDelegate  = 0,
        AsyncCallbackStatus_Join            = 1,
        AsyncCallbackStatus_ChooseAny       = 2,
        AsyncCallbackStatus_Cancel          = 3,
        AsyncCallbackStatus_Error           = 4,
        AsyncCallbackStatus_Last            = AsyncCallbackStatus_Error,
        AsyncCallbackStatus_Invalid         = AsyncCallbackStatus_Last+1
    };
    
    // This enum should map to Windows::Foundation::Diagnostics::CausalitySynchronousWork.
    // We do not use that enum here to avoid taking a dependency on WinRT.
    enum AsyncCallbackType
    {
        AsyncCallbackType_Completion         = 0,
        AsyncCallbackType_Progress           = 1,
        AsyncCallbackType_Execution          = 2,
        AsyncCallbackType_Last               = AsyncCallbackType_Execution,
        AsyncCallbackType_Invalid            = AsyncCallbackType_Last+1
    };
    
    // This enum should map to Windows::Foundation::AsyncStatus.
    // We do not use that enum here to avoid taking a dependency on WinRT.
    enum AsyncOperationStatus
    {
        AsyncOperationStatus_Started        = 0,
        AsyncOperationStatus_Completed      = 1,
        AsyncOperationStatus_Canceled       = 2,
        AsyncOperationStatus_Error          = 3,
        AsyncOperationStatus_Last           = AsyncOperationStatus_Error,
        AsyncOperationStatus_Invalid        = AsyncOperationStatus_Last+1
    };
    
    // This enum should map to Windows::Foundation::Diagnostics::CausalitySource.
    // We do not use that enum here to avoid taking a dependency on WinRT.
    enum AsyncSource
    {
        AsyncSource_Application             = 0,
        AsyncSource_Library                 = 1,
        AsyncSource_System                  = 2,
        AsyncSource_Last                    = AsyncSource_System,
        AsyncSource_Invalid                 = AsyncSource_Last+1
    };

    struct EntryInfo
    {
        static Js::FunctionInfo BeginAsyncOperation;
        static Js::FunctionInfo BeginAsyncCallback;
        static Js::FunctionInfo CompleteAsyncCallback;
        static Js::FunctionInfo UpdateAsyncCallbackStatus;
        static Js::FunctionInfo CompleteAsyncOperation;
    };

    static const GUID ChakraPlatformGUID;
    static const AsyncSource ChakraAsyncSource;
    static const AsyncOperationId InvalidAsyncOperationId;

    // The set of JavaScript-side wrappers around the causality API.
    static Js::Var BeginAsyncOperation(__in Js::RecyclableObject* function, __in Js::CallInfo callInfo, ...);
    static Js::Var BeginAsyncCallback(__in Js::RecyclableObject* function, __in Js::CallInfo callInfo, ...);
    static Js::Var CompleteAsyncCallback(__in Js::RecyclableObject* function, __in Js::CallInfo callInfo, ...);
    static Js::Var UpdateAsyncCallbackStatus(__in Js::RecyclableObject* function, __in Js::CallInfo callInfo, ...);
    static Js::Var CompleteAsyncOperation(__in Js::RecyclableObject* function, __in Js::CallInfo callInfo, ...);

    // The set of engine-side wrappers around the causality API.
    static HRESULT WrapperForTraceOperationCreation(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in AsyncOperationId operationId, __in_z_opt LPCWSTR operationName);
    static HRESULT WrapperForTraceOperationCreation(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in GUID platformId, __in AsyncOperationId operationId, __in_z_opt LPCWSTR operationName);
    static HRESULT WrapperForTraceOperationCompletion(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in AsyncOperationId operationId, __in AsyncOperationStatus status);
    static HRESULT WrapperForTraceOperationCompletion(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in GUID platformId, __in AsyncOperationId operationId, __in AsyncOperationStatus status);
    static HRESULT WrapperForTraceSynchronousWorkStart(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in AsyncOperationId operationId, __in AsyncCallbackType workType);
    static HRESULT WrapperForTraceSynchronousWorkStart(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in GUID platformId, __in AsyncOperationId operationId, __in AsyncCallbackType workType);
    static HRESULT WrapperForTraceSynchronousWorkCompletion(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel);
    static HRESULT WrapperForTraceOperationRelation(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in AsyncOperationId operationId, __in AsyncCallbackStatus relation);
    static HRESULT WrapperForTraceOperationRelation(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in GUID platformId, __in AsyncOperationId operationId, __in AsyncCallbackStatus relation);

    // Wrapper which is called by ScriptEngine. Assumes script is not active.
    static HRESULT HostWrapperForTraceOperationCreation(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in GUID platformId, __in AsyncOperationId operationId, __in_z_opt LPCWSTR operationName);
    static HRESULT HostWrapperForTraceOperationCreation(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in AsyncOperationId operationId, __in_z_opt LPCWSTR operationName);

    // Wrapper which is called from script. Assumes script is active.
    static HRESULT ScriptWrapperForTraceOperationCreation(__in Js::ScriptContext* scriptContext, __in LogLevel logLevel, __in AsyncOperationId operationId, __in_z_opt LPCWSTR operationName);

    static AsyncOperationId BeginAsyncOperationForWinRTMethodCall(LPCWSTR runtimeClassName, LPCWSTR methodName, Js::ScriptContext* scriptContext);

    static AsyncOperationId GetNextAsyncOperationId();
    static bool IsAsyncDebuggingEnabled(Js::ScriptContext* scriptContext);

    static ushort ProcessNameAndGetLength(Js::StringBuilder<ArenaAllocator>* nameBuffer, const WCHAR* name);

protected:

#ifdef F_JSETW
// Make this struct fully packed because ETW treats the buffer of frames as a single un-padded byte buffer.
#pragma pack(push, 1)
    struct ETWStackFrame
    {
        UINT64 documentId;
        UINT32 sourceLocationStartIndex;
        UINT32 sourceLocationLength;
        UINT16 nameIndex;
    };
#pragma pack(pop)
#endif

    static AsyncOperationId nextAsyncOperationId;
    static char isDownlevel;

    static charcount_t AppendWithEscapeCharacters(Js::StringBuilder<ArenaAllocator>* stringBuilder, const WCHAR* sourceString, charcount_t sourceStringLen, WCHAR escapeChar, WCHAR charToEscape);
    static void EmitStackWalk(Js::ScriptContext* scriptContext, AsyncDebug::AsyncOperationId operationId);

    // Look to see if we need to use the downlevel async debug API or the WinRT interface for win8+.
    static bool IsDownlevel(Js::ScriptContext* scriptContext);
};
