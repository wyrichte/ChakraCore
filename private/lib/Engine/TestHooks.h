//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

HRESULT OnJScript9Loaded();
struct _PROFILER_HEAP_OBJECT;
typedef HRESULT (__stdcall *GetHeapObjectInfoPtr)(Var instance, HostProfilerHeapObject** heapObjOut, HeapObjectInfoReturnResult& returnResult);

#ifdef ENABLE_TEST_HOOKS

interface ICustomConfigFlags;

struct TestHooks
{
    typedef HRESULT (__stdcall *DllGetClassObjectPtr)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
    typedef HRESULT (__stdcall *JsVarToScriptDirectPtr)(Var instance, IActiveScriptDirect** scriptDirectRef);
    typedef HRESULT (__stdcall *JsVarAddRefPtr)(Var instance);
    typedef HRESULT (__stdcall *JsVarReleasePtr)(Var instance);
    typedef HRESULT (__stdcall *JsVarToExtensionPtr)(Var instance, void** extensionRef);

    typedef BOOL (__stdcall * HostCallBack)(LPCWSTR flag, BOOL state);
    typedef void (__stdcall * HostPrintUsage)();
    typedef HRESULT (__stdcall *SetConfigFlagsPtr)(int argc, LPWSTR argv[], ICustomConfigFlags * pCustomConfigFlags);

    typedef HRESULT (__stdcall *PrintConfigFlagsUsageStringPtr)(void);   
    typedef HRESULT (__stdcall *GetRestrictedStringPtr)(Var error, BSTR * string);
    typedef HRESULT (__stdcall *GetCapabilitySidPtr)(Var error, BSTR * string);
    typedef LPTSTR (__stdcall *GetSystemStringFromHrPtr)(HRESULT hr);
    typedef BOOL (__stdcall *GetMemoryFootprintOfRCPtr)(IActiveScriptDirect * scriptDirect, LPCWSTR fullTypeName, INT32 * gcPressure);
    typedef HRESULT (__stdcall *GenerateValidPointersMapHeaderPtr)(LPCWSTR vpmFullPath);
    typedef void (__stdcall *DoNotSupportWeakDelegatePtr)(IActiveScriptDirect * scriptDirect);
    typedef BOOL (__stdcall *SupportsWeakDelegatePtr)(IActiveScriptDirect * scriptDirect);
    typedef BOOL (__stdcall * NotifyOnScriptStateChangedCallBackFuncPtr)(IActiveScriptDirect* scriptDirectRef, SCRIPTSTATE ss, void** engineSpecificStorage);
    typedef HRESULT (__stdcall *NotifyOnScriptStateChanged)(NotifyOnScriptStateChangedCallBackFuncPtr pfCallBack);    
    typedef HRESULT (__stdcall *ClearAllProjectionCachesPtr)(IActiveScriptDirect * scriptDirect);
    typedef HRESULT (__stdcall *SetAssertToConsoleFlagPtr)(bool flag);    
    typedef HRESULT (__stdcall *SetEnableCheckMemoryLeakOutputPtr)(bool flag);
    typedef HRESULT(__stdcall *FlushOutputPtr)();
    typedef HRESULT (__stdcall * FinalGCPtr)();
    typedef void (__stdcall *SetGetHeapObjectInfoPtr)(GetHeapObjectInfoPtr);
    typedef HRESULT (__stdcall *GetThreadServicePtr)(IActiveScriptGarbageCollector** threadService);
    typedef void (__stdcall * NotifyUnhandledExceptionPtr)(PEXCEPTION_POINTERS exceptionInfo);

    typedef HRESULT (__stdcall * StartScriptProfilingPtr)(IActiveScriptDirect * scriptDirect, IActiveScriptProfilerCallback *profilerObject, DWORD eventMask, DWORD context);
    typedef HRESULT (__stdcall * StopScriptProfilingPtr)(IActiveScriptDirect * scriptDirect);
    typedef void(__stdcall * DisplayMemStatsPtr)();
    typedef void (__stdcall *GetContentOfSharedArrayBufferPtr)(Var instance, void** content);
    typedef void (__stdcall *CreateSharedArrayBufferFromContentPtr)(IActiveScriptDirect * scriptDirect, void* content, Var* instance);
#ifdef ENABLE_INTL_OBJECT
    typedef void(__stdcall * ClearTimeZoneCalendarsPtr)();
#endif
#ifdef FAULT_INJECTION
    typedef unsigned int(__stdcall *GetCurrentFaultInjectionCountPtr)();
#endif
    DllGetClassObjectPtr pfDllGetClassObject;
    JsVarToScriptDirectPtr pfJsVarToScriptDirect;
    JsVarAddRefPtr pfJsVarAddRef;
    JsVarReleasePtr pfJsVarRelease;
    JsVarToExtensionPtr pfJsVarToExtension;
    SetConfigFlagsPtr pfSetConfigFlags;
    GenerateValidPointersMapHeaderPtr pfGenerateValidPointersMapHeader;
    PrintConfigFlagsUsageStringPtr pfPrintConfigFlagsUsageString; 
    GetRestrictedStringPtr pfGetRestrictedString;
    GetCapabilitySidPtr pfGetCapabilitySid;
    GetSystemStringFromHrPtr pfGetSystemStringFromHr;
    GetMemoryFootprintOfRCPtr pfGetMemoryFootprintOfRC;
    DoNotSupportWeakDelegatePtr pfDoNotSupportWeakDelegate;
    SupportsWeakDelegatePtr pfSupportsWeakDelegate;
    NotifyOnScriptStateChanged pfNotifyOnScriptStateChanged;    
    ClearAllProjectionCachesPtr pfClearAllProjectionCaches;
    SetAssertToConsoleFlagPtr pfSetAssertToConsoleFlag;
    SetEnableCheckMemoryLeakOutputPtr pfSetEnableCheckMemoryLeakOutput;
    FinalGCPtr pfFinalGC;
    SetGetHeapObjectInfoPtr pfSetGetHeapObjectInfoPtr;
    GetThreadServicePtr pfnGetThreadService;    
    StartScriptProfilingPtr pfStartScriptProfiling;
    StopScriptProfilingPtr pfStopScriptProfiling;
    DisplayMemStatsPtr pfDisplayMemStats;
    FlushOutputPtr pfFlushOutput;
    GetContentOfSharedArrayBufferPtr pfGetContentOfSharedArrayBuffer;
    CreateSharedArrayBufferFromContentPtr pfCreateSharedArrayBufferFromContent;
#ifdef ENABLE_INTL_OBJECT
    ClearTimeZoneCalendarsPtr pfResetTimeZoneFactoryObjects;
#endif
#ifdef FAULT_INJECTION
    GetCurrentFaultInjectionCountPtr pfGetCurrentFaultInjectionCount;
#endif
#define FLAG(type, name, description, defaultValue, ...) FLAG_##type##(name)
#define FLAG_String(name) \
    bool (__stdcall *pfIsEnabled##name##Flag)(); \
    HRESULT (__stdcall *pfGet##name##Flag)(BSTR *flag); \
    HRESULT (__stdcall *pfSet##name##Flag)(BSTR flag);
#define FLAG_Boolean(name) \
    bool (__stdcall *pfIsEnabled##name##Flag)(); \
    HRESULT (__stdcall *pfGet##name##Flag)(bool *flag); \
    HRESULT (__stdcall *pfSet##name##Flag)(bool flag);
#define FLAG_Number(name) \
    bool (__stdcall *pfIsEnabled##name##Flag)(); \
    HRESULT (__stdcall *pfGet##name##Flag)(int *flag); \
    HRESULT (__stdcall *pfSet##name##Flag)(int flag);
    // skip other types for now
#define FLAG_Phases(name)
#define FLAG_NumberSet(name)
#define FLAG_NumberPairSet(name)
#define FLAG_NumberTrioSet(name)
#define FLAG_NumberRange(name)
#include "ConfigFlagsList.h"
#undef FLAG
#undef FLAG_String
#undef FLAG_Boolean
#undef FLAG_Number
#undef FLAG_Phases
#undef FLAG_NumberSet
#undef FLAG_NumberPairSet
#undef FLAG_NumberTrioSet
#undef FLAG_NumberRange


    NotifyUnhandledExceptionPtr pfnNotifyUnhandledException;
};

#endif

