//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#ifndef USE_EDGEMODE_JSRT
#define USE_EDGEMODE_JSRT
#endif // USE_EDGEMODE_JSRT
#include "jsrt.h"

// ToDo: Move this into TestHooks and initialize in OnJscript9Loaded once we've merged the DLLs
struct JsrtTestHooks
{
    // Removed JsRuntimeVersion from parameters because of its removal in chakra.dll
    typedef JsErrorCode (WINAPI *JsrtCreateRuntimePtr)(JsRuntimeAttributes attributes, JsThreadServiceCallback threadService, JsRuntimeHandle *runtime);
    typedef JsErrorCode (WINAPI *JsrtCreateContextPtr)(JsRuntimeHandle runtime, JsContextRef *newContext);
    typedef JsErrorCode (WINAPI *JsrtSetCurrentContextPtr)(JsContextRef context);
    typedef JsErrorCode (WINAPI *JsrtGetCurrentContextPtr)(JsContextRef* context);
    typedef JsErrorCode (WINAPI *JsrtDisposeRuntimePtr)(JsRuntimeHandle runtime);
    typedef JsErrorCode (WINAPI *JsrtCreateObjectPtr)(JsValueRef *object);
    typedef JsErrorCode (WINAPI *JsrtCreateExternalObjectPtr)(void* data, JsFinalizeCallback callback, JsValueRef *object);
    typedef JsErrorCode (WINAPI *JsrtCreateFunctionPtr)(JsNativeFunction nativeFunction, void *callbackState, JsValueRef *function);
    typedef JsErrorCode (WINAPI *JsrtPointerToStringPtr)(const char16 *stringValue, size_t length, JsValueRef *value);
    typedef JsErrorCode (WINAPI *JsrtSetPropertyPtr)(JsValueRef object, JsPropertyIdRef property, JsValueRef value, bool useStrictRules);
    typedef JsErrorCode (WINAPI *JsrtGetGlobalObjectPtr)(JsValueRef *globalObject);
    typedef JsErrorCode (WINAPI *JsrtGetUndefinedValuePtr)(JsValueRef *globalObject);
    typedef JsErrorCode (WINAPI *JsrtConvertValueToStringPtr)(JsValueRef value, JsValueRef *stringValue);
    typedef JsErrorCode (WINAPI *JsrtConvertValueToNumberPtr)(JsValueRef value, JsValueRef *numberValue);
    typedef JsErrorCode (WINAPI *JsrtConvertValueToBooleanPtr)(JsValueRef value, JsValueRef *booleanValue);
    typedef JsErrorCode (WINAPI *JsrtStringToPointerPtr)(JsValueRef value, const char16 **stringValue, size_t *length);
    typedef JsErrorCode (WINAPI *JsrtBooleanToBoolPtr)(JsValueRef value, bool *boolValue);
    typedef JsErrorCode (WINAPI *JsrtGetPropertyIdFromNamePtr)(const char16 *name, JsPropertyIdRef *propertyId);
    typedef JsErrorCode (WINAPI *JsrtGetPropertyPtr)(JsValueRef object, JsPropertyIdRef property, JsValueRef* value);
    typedef JsErrorCode (WINAPI *JsrtHasPropertyPtr)(JsValueRef object, JsPropertyIdRef property, bool *hasProperty);
    typedef JsErrorCode (WINAPI *JsrtRunScriptPtr)(const char16 *script, DWORD_PTR sourceContext, const char16 *sourceUrl, JsValueRef* result);
    typedef JsErrorCode (WINAPI *JsrtCallFunctionPtr)(JsValueRef function, JsValueRef* arguments, unsigned short argumentCount, JsValueRef *result);
    typedef JsErrorCode (WINAPI *JsrtNumberToDoublePtr)(JsValueRef value, double *doubleValue);
    typedef JsErrorCode (WINAPI *JsrtDoubleToNumberPtr)(double doubleValue, JsValueRef* value);
    typedef JsErrorCode (WINAPI *JsrtGetExternalDataPtr)(JsValueRef object, void **data);
    typedef JsErrorCode (WINAPI *JsrtCreateArrayPtr)(unsigned int length, JsValueRef *result);
    typedef JsErrorCode (WINAPI *JsrtCreateArrayBufferPtr)(unsigned int byteLength, JsValueRef *result);
    typedef JsErrorCode (WINAPI *JsrtGetArrayBufferStoragePtr)(JsValueRef instance, BYTE **buffer, unsigned int *bufferLength);
    typedef JsErrorCode (WINAPI *JsrtSetIndexedPropertyPtr)(JsValueRef object, JsValueRef index, JsValueRef value);
    typedef JsErrorCode (WINAPI *JsrtCreateErrorPtr)(JsValueRef message, JsValueRef *error);
    typedef JsErrorCode (WINAPI *JsrtSetExceptionPtr)(JsValueRef exception);
    typedef JsErrorCode (WINAPI *JsrtGetAndClearExceptiopnPtr)(JsValueRef* exception);
    typedef JsErrorCode (WINAPI *JsrtGetRuntimePtr)(JsContextRef context, JsRuntimeHandle *runtime);
    typedef JsErrorCode (WINAPI *JsrtCollectGarbagePtr)(JsRuntimeHandle runtime);
    typedef JsErrorCode (WINAPI *JsrtStartDebuggingPtr)();
    typedef JsErrorCode (WINAPI *JsrtReleasePtr)(JsRef ref, unsigned int* count);
    typedef JsErrorCode (WINAPI *JsrtAddRefPtr)(JsRef ref, unsigned int* count);
    typedef JsErrorCode (WINAPI *JsrtGetValueType)(JsValueRef value, JsValueType *type);
    typedef JsErrorCode (WINAPI *JsrtParseScriptWithAttributes)(const char16 *script, JsSourceContext sourceContext, const char16 *sourceUrl, JsParseScriptAttributes parseAttributes, JsValueRef *result);

    JsrtCreateRuntimePtr pfJsrtCreateRuntime;
    JsrtCreateContextPtr pfJsrtCreateContext;
    JsrtSetCurrentContextPtr pfJsrtSetCurrentContext;
    JsrtGetCurrentContextPtr pfJsrtGetCurrentContext;
    JsrtDisposeRuntimePtr pfJsrtDisposeRuntime;
    JsrtCreateObjectPtr pfJsrtCreateObject;
    JsrtCreateExternalObjectPtr pfJsrtCreateExternalObject;
    JsrtCreateFunctionPtr pfJsrtCreateFunction;
    JsrtPointerToStringPtr pfJsrtPointerToString;
    JsrtSetPropertyPtr pfJsrtSetProperty;
    JsrtGetGlobalObjectPtr pfJsrtGetGlobalObject;
    JsrtGetUndefinedValuePtr pfJsrtGetUndefinedValue;
    JsrtGetUndefinedValuePtr pfJsrtGetTrueValue;
    JsrtGetUndefinedValuePtr pfJsrtGetFalseValue;
    JsrtConvertValueToStringPtr pfJsrtConvertValueToString;
    JsrtConvertValueToNumberPtr pfJsrtConvertValueToNumber;
    JsrtConvertValueToBooleanPtr pfJsrtConvertValueToBoolean;
    JsrtStringToPointerPtr pfJsrtStringToPointer;
    JsrtBooleanToBoolPtr pfJsrtBooleanToBool;
    JsrtGetPropertyIdFromNamePtr pfJsrtGetPropertyIdFromName;
    JsrtGetPropertyPtr pfJsrtGetProperty;
    JsrtHasPropertyPtr pfJsrtHasProperty;
    JsrtRunScriptPtr pfJsrtRunScript;
    JsrtCallFunctionPtr pfJsrtCallFunction;
    JsrtNumberToDoublePtr pfJsrtNumbertoDouble;
    JsrtDoubleToNumberPtr pfJsrtDoubleToNumber;
    JsrtGetExternalDataPtr pfJsrtGetExternalData;
    JsrtCreateArrayPtr pfJsrtCreateArray;
    JsrtCreateArrayBufferPtr pfJsrtCreateArrayBuffer;
    JsrtGetArrayBufferStoragePtr pfJsrtGetArrayBufferStorage;
    JsrtSetIndexedPropertyPtr pfJsrtSetIndexedProperty;
    JsrtCreateErrorPtr pfJsrtCreateError;
    JsrtSetExceptionPtr pfJsrtSetException;
    JsrtGetAndClearExceptiopnPtr pfJsrtGetAndClearException;
    JsrtGetRuntimePtr pfJsrtGetRuntime;
    JsrtCollectGarbagePtr pfJsrtCollectGarbage;
    JsrtStartDebuggingPtr pfJsrtStartDebugging;
    JsrtReleasePtr pfJsrtRelease;
    JsrtAddRefPtr pfJsrtAddRef;
    JsrtGetValueType pfJsrtGetValueType;
    JsrtParseScriptWithAttributes pfJsrtParseScriptWithAttributes;
};

struct MemProtectTestHooks
{
    typedef HRESULT(WINAPI *MemProtectHeapCreatePtr)(void ** heapHandle, int flags);
    typedef void *(WINAPI *MemProtectHeapRootAllocPtr)(void * heapHandle, size_t size);
    typedef void *(WINAPI *MemProtectHeapRootAllocLeafPtr)(void * heapHandle, size_t size);
    typedef HRESULT(WINAPI *MemProtectHeapUnrootAndZeroPtr)(void * heapHandle, void * memory);
    typedef HRESULT(WINAPI *MemProtectHeapMemSizePtr)(void * heapHandle, void * memory, size_t * outSize);
    typedef HRESULT(WINAPI *MemProtectHeapDestroyPtr)(void *heapHandle);
    typedef HRESULT(WINAPI *MemProtectHeapCollectPtr)(void * heapHandle, int flags);
    typedef HRESULT(WINAPI *MemProtectHeapProtectCurrentThreadPtr)(void * heapHandle, void (__stdcall* threadWake)(void* threadWakeArgument), void* threadWakeArgument);
    typedef HRESULT(WINAPI *MemProtectHeapUnprotectCurrentThreadPtr)(void * heapHandle);
    typedef HRESULT(WINAPI *MemProtectHeapSynchronizeWithCollectorPtr)(void * heapHandle);

    MemProtectHeapCreatePtr pfMemProtectHeapCreate;
    MemProtectHeapRootAllocPtr pfMemProtectHeapRootAlloc;
    MemProtectHeapRootAllocLeafPtr pfMemProtectHeapRootAllocLeaf;
    MemProtectHeapUnrootAndZeroPtr pfMemProtectHeapUnrootAndZero;
    MemProtectHeapMemSizePtr pfMemProtectHeapMemSize;
    MemProtectHeapDestroyPtr pfMemProtectHeapDestroy;
    MemProtectHeapCollectPtr pfMemProtectHeapCollect;
    MemProtectHeapProtectCurrentThreadPtr pfMemProtectHeapProtectCurrentThread;
    MemProtectHeapUnprotectCurrentThreadPtr pfMemProtectHeapUnprotectCurrentThread;
    MemProtectHeapSynchronizeWithCollectorPtr pfMemProtectHeapSynchronizeWithCollector;
};
class JScript9Interface
{
public:
    typedef void (__stdcall * HostPrintUsageFuncPtr)();

    struct ArgInfo
    {
        int argc;
        LPWSTR* argv;
        HostPrintUsageFuncPtr hostPrintUsage;
        BSTR* filename;
    };
#define CHECKED_CALL_RETURN(func, retVal, ...) ((m_testHooksSetup||m_freeChakraLoaded) && m_testHooks.pf##func? m_testHooks.pf##func(__VA_ARGS__) : retVal)
#define CHECKED_CALL(func, ...) ((m_testHooksSetup||m_freeChakraLoaded) && m_testHooks.pf##func? m_testHooks.pf##func(__VA_ARGS__) : E_NOTIMPL)

private:
    static boolean m_testHooksSetup;
    static boolean m_testHooksInitialized;
    static boolean m_usageStringPrinted;
    static boolean m_freeChakraLoaded;
    static ArgInfo m_argInfo;
    static TestHooks m_testHooks;
    static JsrtTestHooks m_jsrtTestHooks;
    static MemProtectTestHooks m_memProtectTestHooks;
    static HRESULT JScript9Interface::ParseConfigFlags();

public:
    static HINSTANCE LoadDll(bool useRetailDllName, LPCWSTR alternateDllName, ArgInfo& argInfo);
    static void SetArgInfo(ArgInfo& args);
    static HRESULT FinalGC();
    static void DisplayRecyclerStats();
#ifdef ENABLE_INTL_OBJECT
    static void ResetTimeZoneFactoryObjects();
#endif

    static void UnloadDll(HINSTANCE jscriptLibrary);
    static HRESULT OnJScript9Loaded(TestHooks& testHooks);
    static HRESULT GetThreadService(IActiveScriptGarbageCollector** threadService);

    static HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) { return CHECKED_CALL(DllGetClassObject, rclsid, riid, ppv); }
    static HRESULT JsVarToScriptDirect(Var instance, IActiveScriptDirect** scriptDirectRef) { return CHECKED_CALL(JsVarToScriptDirect, instance, scriptDirectRef); }
    static HRESULT JsVarAddRef(Var instance) { return CHECKED_CALL(JsVarAddRef,instance); }
    static HRESULT JsVarRelease(Var instance) { return CHECKED_CALL(JsVarRelease, instance); }
    static HRESULT JsVarToExtension(Var instance, void** extensionRef) { return CHECKED_CALL(JsVarToExtension, instance, extensionRef); }
    static HRESULT SetConfigFlags(__in int argc, __in_ecount(argc) LPWSTR argv[], ICustomConfigFlags * pCustomConfigFlags) { return CHECKED_CALL(SetConfigFlags, argc, argv, pCustomConfigFlags); }
    static HRESULT PrintConfigFlagsUsageString() { m_usageStringPrinted = true; return CHECKED_CALL(PrintConfigFlagsUsageString); }
    static HRESULT GenerateValidPointersMapHeader(LPCWSTR vpmFullPath) { return CHECKED_CALL(GenerateValidPointersMapHeader, vpmFullPath); }
    static HRESULT GetFileNameFlag(BSTR * filename) { return CHECKED_CALL(GetFilenameFlag, filename); }
    static HRESULT FlushOutput() { return CHECKED_CALL(FlushOutput); }
    static void GetContentOfSharedArrayBuffer(Var instance, void** content) { CHECKED_CALL(GetContentOfSharedArrayBuffer, instance, content); }
    static void CreateSharedArrayBufferFromContent(IActiveScriptDirect * scriptDirect, void* content, Var* instance) { CHECKED_CALL(CreateSharedArrayBufferFromContent, scriptDirect, content, instance); }

    static HRESULT GetCrashOnExceptionFlag(bool * flag)
    {
#ifdef SECURITY_TESTING
        return CHECKED_CALL(GetCrashOnExceptionFlag, flag);
#else
        return E_UNEXPECTED;
#endif
    }

#ifdef FAULT_INJECTION
    static HRESULT GetFaultInjectionFlag(int * flag)
    {
        return CHECKED_CALL(GetFaultInjectionFlag, flag);
    }
    
    static unsigned int GetCurrentFaultInjectionCount()
    {
        return m_testHooks.pfGetCurrentFaultInjectionCount == nullptr ?
            0 : m_testHooks.pfGetCurrentFaultInjectionCount();
    }

#else
    static HRESULT GetFaultInjectionFlag(int * /*flag*/)
    {
        return E_UNEXPECTED;
    }
    static unsigned int GetCurrentFaultInjectionCount()
    {
        return 0;
    }
#endif
    
    static HRESULT GetHostTypeFlag(int * flag ) { return CHECKED_CALL(GetHostTypeFlag,flag); }    
    static HRESULT GetRestrictedString(Var error, BSTR * string) { return CHECKED_CALL(GetRestrictedString,error, string); }
    static HRESULT GetCapabilitySid(Var error, BSTR * string) { return CHECKED_CALL(GetCapabilitySid,error, string); }
    static BOOL GetMemoryFootprintOfRC(IActiveScriptDirect * scriptDirect, LPCWSTR fullTypeName, INT32 * gcPressure) { return CHECKED_CALL_RETURN(GetMemoryFootprintOfRC, FALSE, scriptDirect, fullTypeName, gcPressure); }
    static void DoNotSupportWeakDelegate(IActiveScriptDirect * scriptDirect) { CHECKED_CALL(DoNotSupportWeakDelegate,scriptDirect); }
    static BOOL SupportsWeakDelegate(IActiveScriptDirect * scriptDirect) { return CHECKED_CALL_RETURN(SupportsWeakDelegate, FALSE, scriptDirect); }
    static boolean IsUsageStringPrinted() { return m_usageStringPrinted; }
    static HRESULT GetLoopFlag(int * flag)
    {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        return CHECKED_CALL(GetLoopFlag, flag);
#else
        *flag = DEFAULT_CONFIG_Loop;
        return S_OK;
#endif
    }    

    static LPTSTR GetSystemStringFromHr(HRESULT hr) { return CHECKED_CALL_RETURN(GetSystemStringFromHr, _u(""), hr); }
    static HRESULT NotifyOnScriptStateChanged(TestHooks::NotifyOnScriptStateChangedCallBackFuncPtr pfCallBack) { return CHECKED_CALL(NotifyOnScriptStateChanged,pfCallBack); }    
    static HRESULT ClearAllProjectionCaches(IActiveScriptDirect * scriptDirect) { return CHECKED_CALL(ClearAllProjectionCaches,scriptDirect); }
    static HRESULT SetAssertToConsoleFlag(bool flag) { return CHECKED_CALL(SetAssertToConsoleFlag,flag); }
#ifdef CHECK_MEMORY_LEAK
    static bool IsEnabledCheckMemoryFlag() { return CHECKED_CALL_RETURN(IsEnabledCheckMemoryLeakFlag, FALSE); }
    static HRESULT SetCheckMemoryLeakFlag(bool flag) { return CHECKED_CALL(SetCheckMemoryLeakFlag,flag); }
    static HRESULT SetEnableCheckMemoryLeakOutput(bool flag) { return CHECKED_CALL(SetEnableCheckMemoryLeakOutput, flag); }
#endif
#ifdef DBG
    static HRESULT SetCheckOpHelpersFlag(bool flag) { return CHECKED_CALL(SetCheckOpHelpersFlag,flag); }
#endif
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    static HRESULT SetOOPCFGRegistrationFlag(bool flag) { return CHECKED_CALL(SetOOPCFGRegistrationFlag, flag); }
#endif

    static boolean SupportsDllGetClassObjectCallback() {return m_testHooks.pfDllGetClassObject != NULL; }
    static boolean SupportsPrintConfigFlagsUsageString() { return m_testHooksSetup && m_testHooks.pfPrintConfigFlagsUsageString != NULL; }

    static HRESULT StartScriptProfiling(IActiveScriptDirect * scriptDirect, IActiveScriptProfilerCallback *profilerObject, DWORD eventMask, DWORD context) { return CHECKED_CALL(StartScriptProfiling,scriptDirect, profilerObject, eventMask, context); }
    static HRESULT StopScriptProfiling(IActiveScriptDirect * scriptDirect) { return CHECKED_CALL(StopScriptProfiling, scriptDirect); }

    static boolean SupportsGetFaultInjectionFlag()
    {
#ifdef FAULT_INJECTION
        return m_testHooksSetup && m_testHooks.pfGetFaultInjectionFlag != NULL;
#else
        return false;
#endif
    }

    static boolean SupportsNotifyOnScriptStateChanged() { return m_testHooksSetup && m_testHooks.pfNotifyOnScriptStateChanged != NULL; }

    static void NotifyUnhandledException(PEXCEPTION_POINTERS exceptionInfo)
    { 
        if (m_testHooksSetup && m_testHooks.pfnNotifyUnhandledException != NULL)
        {
            m_testHooks.pfnNotifyUnhandledException(exceptionInfo);
        }
    }

    static JsErrorCode WINAPI JsrtCreateRuntime(JsRuntimeAttributes attributes, JsThreadServiceCallback threadService, JsRuntimeHandle *runtime) { return m_jsrtTestHooks.pfJsrtCreateRuntime(attributes, threadService, runtime); }
    static JsErrorCode WINAPI JsrtCreateContext(JsRuntimeHandle runtime, JsContextRef *newContext) { return m_jsrtTestHooks.pfJsrtCreateContext(runtime, newContext); }
    static JsErrorCode WINAPI JsrtSetCurrentContext(JsContextRef context) { return m_jsrtTestHooks.pfJsrtSetCurrentContext(context); }
    static JsErrorCode WINAPI JsrtGetCurrentContext(JsContextRef* context) { return m_jsrtTestHooks.pfJsrtGetCurrentContext(context); }
    static JsErrorCode WINAPI JsrtDisposeRuntime(JsRuntimeHandle runtime) { return m_jsrtTestHooks.pfJsrtDisposeRuntime(runtime); }
    static JsErrorCode WINAPI JsrtCreateObject(JsValueRef *object) { return m_jsrtTestHooks.pfJsrtCreateObject(object); }
    static JsErrorCode WINAPI JsrtCreateExternalObject(void *data, JsFinalizeCallback callback, JsValueRef *object) { return m_jsrtTestHooks.pfJsrtCreateExternalObject(data, callback, object); }
    static JsErrorCode WINAPI JsrtCreateFunction(JsNativeFunction nativeFunction, void *callbackState, JsValueRef *function) { return m_jsrtTestHooks.pfJsrtCreateFunction(nativeFunction, callbackState, function); }
    static JsErrorCode WINAPI JsrtPointerToString(const char16 *stringValue, size_t length, JsValueRef *value) { return m_jsrtTestHooks.pfJsrtPointerToString(stringValue, length, value); }
    static JsErrorCode WINAPI JsrtSetProperty(JsValueRef object, JsPropertyIdRef property, JsValueRef value, bool useStrictRules) { return m_jsrtTestHooks.pfJsrtSetProperty(object, property, value, useStrictRules); }
    static JsErrorCode WINAPI JsrtGetGlobalObject(JsValueRef *globalObject) { return m_jsrtTestHooks.pfJsrtGetGlobalObject(globalObject); }
    static JsErrorCode WINAPI JsrtGetUndefinedValue(JsValueRef *globalObject) { return m_jsrtTestHooks.pfJsrtGetUndefinedValue(globalObject); }
    static JsErrorCode WINAPI JsrtGetTrueValue(JsValueRef *globalObject) { return m_jsrtTestHooks.pfJsrtGetTrueValue(globalObject); }
    static JsErrorCode WINAPI JsrtGetFalseValue(JsValueRef *globalObject) { return m_jsrtTestHooks.pfJsrtGetFalseValue(globalObject); }
    static JsErrorCode WINAPI JsrtConvertValueToString(JsValueRef value, JsValueRef *stringValue) { return m_jsrtTestHooks.pfJsrtConvertValueToString(value, stringValue); }
    static JsErrorCode WINAPI JsrtConvertValueToNumber(JsValueRef value, JsValueRef *numberValue) { return m_jsrtTestHooks.pfJsrtConvertValueToNumber(value, numberValue); }
    static JsErrorCode WINAPI JsrtConvertValueToBoolean(JsValueRef value, JsValueRef *booleanValue) { return m_jsrtTestHooks.pfJsrtConvertValueToBoolean(value, booleanValue); }
    static JsErrorCode WINAPI JsrtStringToPointer(JsValueRef value, const char16 **stringValue, size_t *length) { return m_jsrtTestHooks.pfJsrtStringToPointer(value, stringValue, length); }
    static JsErrorCode WINAPI JsrtBooleanToBool(JsValueRef value, bool* boolValue) { return m_jsrtTestHooks.pfJsrtBooleanToBool(value, boolValue); }
    static JsErrorCode WINAPI JsrtGetPropertyIdFromName(const char16 *name, JsPropertyIdRef *propertyId) { return m_jsrtTestHooks.pfJsrtGetPropertyIdFromName(name, propertyId); }
    static JsErrorCode WINAPI JsrtGetProperty(JsValueRef object, JsPropertyIdRef property, JsValueRef* value) { return m_jsrtTestHooks.pfJsrtGetProperty(object, property, value); }
    static JsErrorCode WINAPI JsrtHasProperty(JsValueRef object, JsPropertyIdRef property, bool *hasProperty) { return m_jsrtTestHooks.pfJsrtHasProperty(object, property, hasProperty); }
    static JsErrorCode WINAPI JsrtRunScript(const char16 *script, DWORD_PTR sourceContext, const char16 *sourceUrl, JsValueRef* result) { return m_jsrtTestHooks.pfJsrtRunScript(script, sourceContext, sourceUrl, result); }
    static JsErrorCode WINAPI JsrtCallFunction(JsValueRef function, JsValueRef* arguments, unsigned short argumentCount, JsValueRef *result) { return m_jsrtTestHooks.pfJsrtCallFunction(function, arguments, argumentCount, result); }
    static JsErrorCode WINAPI JsrtNumberToDouble(JsValueRef value, double* doubleValue) { return m_jsrtTestHooks.pfJsrtNumbertoDouble(value, doubleValue); }
    static JsErrorCode WINAPI JsrtDoubleToNumber(double doubleValue, JsValueRef* value) { return m_jsrtTestHooks.pfJsrtDoubleToNumber(doubleValue, value); }
    static JsErrorCode WINAPI JsrtGetExternalData(JsValueRef object, void **data) { return m_jsrtTestHooks.pfJsrtGetExternalData(object, data); }
    static JsErrorCode WINAPI JsrtCreateArray(unsigned int length, JsValueRef *result) { return m_jsrtTestHooks.pfJsrtCreateArray(length, result); }
    static JsErrorCode WINAPI JsrtCreateArrayBuffer(unsigned int byteLength, JsValueRef *result) { return m_jsrtTestHooks.pfJsrtCreateArrayBuffer(byteLength, result); }
    static JsErrorCode WINAPI JsrtGetArrayBufferStorage(JsValueRef instance, BYTE **buffer, unsigned int *bufferLength) { return m_jsrtTestHooks.pfJsrtGetArrayBufferStorage(instance, buffer, bufferLength); }
    static JsErrorCode WINAPI JsrtSetIndexedProperty(JsValueRef object, JsValueRef index, JsValueRef value) { return m_jsrtTestHooks.pfJsrtSetIndexedProperty(object, index, value); }
    static JsErrorCode WINAPI JsrtCreateError(JsValueRef message, JsValueRef *error) { return m_jsrtTestHooks.pfJsrtCreateError(message, error); }
    static JsErrorCode WINAPI JsrtSetException(JsValueRef exception) { return m_jsrtTestHooks.pfJsrtSetException(exception); }
    static JsErrorCode WINAPI JsrtGetAndClearException(JsValueRef *exception) { return m_jsrtTestHooks.pfJsrtGetAndClearException(exception); }
    static JsErrorCode WINAPI JsrtGetRuntime(JsContextRef context, JsRuntimeHandle *runtime) { return m_jsrtTestHooks.pfJsrtGetRuntime(context, runtime); }
    static JsErrorCode WINAPI JsrtCollectGarbage(JsRuntimeHandle runtime) { return m_jsrtTestHooks.pfJsrtCollectGarbage(runtime); }
    static JsErrorCode WINAPI JsrtStartDebugging() { return m_jsrtTestHooks.pfJsrtStartDebugging(); }
    static JsErrorCode WINAPI JsrtRelease(JsRef ref, unsigned int* count) { return m_jsrtTestHooks.pfJsrtRelease(ref, count); }
    static JsErrorCode WINAPI JsrtAddRef(JsRef ref, unsigned int* count) { return m_jsrtTestHooks.pfJsrtAddRef(ref, count); }
    static JsErrorCode WINAPI JsrtGetValueType(JsValueRef value, JsValueType *type) { return m_jsrtTestHooks.pfJsrtGetValueType(value, type); }
    static JsErrorCode WINAPI JsrtParseScriptWithAttributes(const char16 *script, JsSourceContext sourceContext, const char16 *sourceUrl, JsParseScriptAttributes parseAttributes, JsValueRef *result) { return m_jsrtTestHooks.pfJsrtParseScriptWithAttributes(script, sourceContext, sourceUrl, parseAttributes, result); }


    static HRESULT MemProtectHeapCreate(void ** heapHandle, int flags) { return m_memProtectTestHooks.pfMemProtectHeapCreate(heapHandle, flags); }
    static void * MemProtectHeapRootAlloc(void * heapHandle, size_t size) { return m_memProtectTestHooks.pfMemProtectHeapRootAlloc(heapHandle, size); }
    static void * MemProtectHeapRootAllocLeaf(void * heapHandle, size_t size) { return m_memProtectTestHooks.pfMemProtectHeapRootAllocLeaf(heapHandle, size); }
    static HRESULT MemProtectHeapUnrootAndZero(void * heapHandle, void * memory) { return m_memProtectTestHooks.pfMemProtectHeapUnrootAndZero(heapHandle, memory); }
    static HRESULT MemProtectHeapMemSize(void * heapHandle, void * memory, size_t * outSize) { return m_memProtectTestHooks.pfMemProtectHeapMemSize(heapHandle, memory, outSize); }
    static HRESULT MemProtectHeapDestroy(void * heapHandle) { return m_memProtectTestHooks.pfMemProtectHeapDestroy(heapHandle); }
    static HRESULT MemProtectHeapCollect(void * heapHandle, int flags) { return m_memProtectTestHooks.pfMemProtectHeapCollect(heapHandle, flags); }
    static HRESULT MemProtectHeapProtectCurrentThread(void * heapHandle, void (__stdcall* threadWake)(void* threadWakeArgument), void* threadWakeArgument) { return m_memProtectTestHooks.pfMemProtectHeapProtectCurrentThread(heapHandle, threadWake, threadWakeArgument); }
    static HRESULT MemProtectHeapUnprotectCurrentThread(void * heapHandle) { return m_memProtectTestHooks.pfMemProtectHeapUnprotectCurrentThread(heapHandle); }
    static HRESULT MemProtectHeapSynchronizeWithCollector(void * heapHandle) { return m_memProtectTestHooks.pfMemProtectHeapSynchronizeWithCollector(heapHandle); }
};

