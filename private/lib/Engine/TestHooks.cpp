//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"
#ifdef ENABLE_PROJECTION
#include "ProjectionWriter.h"
#endif
#include "TestHooks.h"
#include "JavascriptErrorDebug.h"

#ifndef ENABLE_TEST_HOOKS
HRESULT OnJScript9Loaded()
{
    return S_OK;
}

#else

#include "Library\ES5Array.h"
#include "ActiveScriptProfilerHeapEnum.h"

class CClassFactory;
extern CClassFactory* (*pfCreateJscript9ClassFactory)(void);

HRESULT __stdcall SetConfigFlags(__in int argc, __in_ecount(argc) LPWSTR argv[], ICustomConfigFlags * pCustomConfigFlags)
{
    CmdLineArgsParser parser(pCustomConfigFlags);
    if (parser.Parse(argc, argv) != 0)
    {
        return E_FAIL;
    }

    if(!Js::Configuration::Global.flags.IsEnabled(Js::HostTypeFlag))
    {
        Js::Configuration::Global.flags.HostType = Js::HostTypeBrowser;
    }

    return S_OK;
}

HRESULT __stdcall PrintConfigFlagsUsageString()
{
    Js::ConfigFlagsTable::PrintUsageString();
    return S_OK;
}

HRESULT __stdcall GenerateValidPointersMapHeader(LPCWSTR vpmFullPath)
{
    HRESULT hr = S_OK;
    ThreadContext * threadContext = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        threadContext = ThreadBoundThreadContextManager::EnsureContextForCurrentThread();
    } END_TRANSLATE_OOM_TO_HRESULT(hr);
    if (!threadContext)
    {
        return E_FAIL;
    }

    Assert(!FAILED(hr));
    Recycler * recycler = threadContext->GetRecycler();
    return recycler->GetAutoHeap()->GenerateValidPointersMapHeader(vpmFullPath);
}

HRESULT __stdcall GetRestrictedString(Var error, BSTR * string)
{
    *string = nullptr;
    if (Js::JavascriptErrorDebug::Is(error))
    {
        *string = Js::JavascriptErrorDebug::FromVar(error)->GetRestrictedErrorString();
    }
    return S_OK;
}

HRESULT __stdcall GetCapability(Var error, BSTR * string)
{
    *string = nullptr;
    if (Js::JavascriptErrorDebug::Is(error))
    {
        *string = Js::JavascriptErrorDebug::FromVar(error)->GetCapabilitySid();
    }
    return S_OK;
}

LPWSTR __stdcall GetSystemStringFromHr(HRESULT hr)
{
    LPWSTR returnStr = NULL;

    DWORD_PTR pArgs[] = { (DWORD_PTR)_u(""), (DWORD_PTR)_u(""), (DWORD_PTR)_u(""), (DWORD_PTR)_u(""), (DWORD_PTR)_u(""), 
        (DWORD_PTR)_u(""), (DWORD_PTR)_u(""), (DWORD_PTR)_u(""), (DWORD_PTR)_u(""), (DWORD_PTR)_u("") };

    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_ARGUMENT_ARRAY | 
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        hr,
        0,
        (LPWSTR)&returnStr,
        0,
        (va_list*)pArgs))
    {
        return returnStr;
    }
    return nullptr;
}

BOOL __stdcall GetMemoryFootprintOfRC(IActiveScriptDirect * scriptDirect, LPCWSTR fullTypeName, INT32 * gcPressure) 
{
    auto scriptEngine = (ScriptEngine*)scriptDirect;
    Projection::ProjectionContext * projectionContext = scriptEngine->GetProjectionContext();
    Projection::ProjectionWriter * writer = projectionContext->GetProjectionWriter();
    HTYPE htype;
    Projection::RuntimeClassTypeInformation * typeInformation;
    bool result = writer->GetRuntimeClassTypeInformation(projectionContext->IdOfString(fullTypeName), &htype, &typeInformation);
    if (result && gcPressure)
    {
        *gcPressure = typeInformation->GCPressure();
    }
    return result;
}

void __stdcall DoNotSupportWeakDelegate(IActiveScriptDirect * scriptDirect) 
{
    auto scriptEngine = (ScriptEngine*)scriptDirect;
    Projection::ProjectionContext * projectionContext = scriptEngine->GetProjectionContext();
    projectionContext->DoNotSupportWeakDelegate();
}

HRESULT __stdcall StartScriptProfiling(IActiveScriptDirect * scriptDirect, IActiveScriptProfilerCallback *profilerObject, DWORD eventMask, DWORD context) 
{
    auto scriptEngine = (ScriptEngine*)scriptDirect;
    return scriptEngine->StartScriptProfiling(profilerObject, eventMask, context);
}

HRESULT __stdcall StopScriptProfiling(IActiveScriptDirect * scriptDirect) 
{
    auto scriptEngine = (ScriptEngine*)scriptDirect;
    return scriptEngine->StopProfiling(S_OK);
}

void __stdcall DisplayMemStats() 
{
    ThreadContext * threadContext = ThreadContext::GetContextForCurrentThread();
    if (!threadContext)
    {
        return;
    }
    Recycler * recycler = threadContext->GetRecycler();  
    // Recycler might not have initialized, check if it is null
    if (recycler)
    {
        recycler->DisplayMemStats();
    }
}

void __stdcall GetContentOfSharedArrayBuffer(Var instance, void** content)
{
    *content = Js::SharedArrayBuffer::FromVar(instance)->GetSharedContents();
}

void __stdcall CreateSharedArrayBufferFromContent(IActiveScriptDirect * scriptDirect, void* content, Var* instance)
{
    auto scriptEngine = (ScriptEngine*)scriptDirect;
    auto scriptContext = scriptEngine->GetScriptContext();

    HRESULT hr;
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        *instance = scriptContext->GetLibrary()->CreateSharedArrayBuffer((Js::SharedContents*)content);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

}


#ifdef ENABLE_INTL_OBJECT
void __stdcall ResetTimeZoneFactoryObjects()
{
    ThreadContext * threadContext = ThreadContext::GetContextForCurrentThread();
    if (!threadContext)
    {
        return;
    }
    threadContext->GetWindowsGlobalizationAdapter()->ResetTimeZoneFactoryObjects();
}
#endif

BOOL __stdcall SupportsWeakDelegate(IActiveScriptDirect * scriptDirect) 
{
    auto scriptEngine = (ScriptEngine*)scriptDirect;
    Projection::ProjectionContext * projectionContext = scriptEngine->GetProjectionContext();
    return projectionContext->SupportsWeakDelegate() == true ? TRUE : FALSE;
}

void PreInitializePropertyIds(IActiveScriptDirect* scriptDirect)
{
    static const LPWSTR WinRTHeapEnumPropertyNames[] = {
        _u("size"),
        _u("new"),
        _u("pinned"),
        _u("object"),
        _u("type"),
        _u("gcPressure"),
        _u("winrtType"),
        _u("Instance"),
        _u("RuntimeClass"),
        _u("nativePtr"),
        _u("eventCount"),
        _u("eventNames"),
        _u("eventHandlers") };
    PropertyId propertyId;
    HRESULT hr = NOERROR;
    for (uint i = 0; i < _countof(WinRTHeapEnumPropertyNames) && SUCCEEDED(hr); i++)
    {
        scriptDirect->GetOrAddPropertyId(WinRTHeapEnumPropertyNames[i], &propertyId);
    }
    Assert(SUCCEEDED(hr));
}

#define FLAG(type, name, description, defaultValue, ...) FLAG_##type##(name)
#define FLAG_String(name) \
    bool IsEnabled##name##Flag() \
    { \
        return Js::Configuration::Global.flags.IsEnabled(Js::##name##Flag); \
    } \
    HRESULT __stdcall Get##name##Flag(BSTR *flag) \
    { \
        *flag = SysAllocString(Js::Configuration::Global.flags.##name##); \
        return (*flag == NULL ? E_OUTOFMEMORY : S_OK); \
    } \
    HRESULT __stdcall Set##name##Flag(BSTR flag) \
    { \
        Js::Configuration::Global.flags.##name = flag; \
        return S_OK; \
    }
#define FLAG_Boolean(name) \
    bool IsEnabled##name##Flag() \
    { \
        return Js::Configuration::Global.flags.IsEnabled(Js::##name##Flag); \
    } \
    HRESULT __stdcall Get##name##Flag(bool *flag) \
    { \
        *flag = Js::Configuration::Global.flags.##name##; \
        return S_OK; \
    } \
    HRESULT __stdcall Set##name##Flag(bool flag) \
    { \
        Js::Configuration::Global.flags.##name = flag; \
        return S_OK; \
    }
#define FLAG_Number(name) \
    bool IsEnabled##name##Flag() \
    { \
        return Js::Configuration::Global.flags.IsEnabled(Js::##name##Flag); \
    } \
    HRESULT __stdcall Get##name##Flag(int *flag) \
    { \
        *flag = Js::Configuration::Global.flags.##name##; \
        return S_OK; \
    } \
    HRESULT __stdcall Set##name##Flag(int flag) \
    { \
        Js::Configuration::Global.flags.##name = flag; \
        return S_OK; \
    }
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

typedef BOOL (__stdcall * NotifyOnScriptStateChangedCallBackFuncPtr)(IActiveScriptDirect* scriptDirectRef, SCRIPTSTATE ss, void** engineSpecificStorage);
NotifyOnScriptStateChangedCallBackFuncPtr pfNotifyOnScriptStateChangedCallBackFuncPtr = NULL;
class TestScriptEngine : public ScriptEngine
{
public:
    TestScriptEngine (REFIID riidLanguage, LPCOLESTR pszLanguageName) :
        ScriptEngine(riidLanguage, pszLanguageName), m_engineSpecificStorage(NULL)
        { 
        }

protected:
    void NotifyScriptStateChange(SCRIPTSTATE ss)
    {
        __super::NotifyScriptStateChange(ss);
        if (pfNotifyOnScriptStateChangedCallBackFuncPtr)
        {
            pfNotifyOnScriptStateChangedCallBackFuncPtr(this, ss, &m_engineSpecificStorage);
        }
    }
private:
    void* m_engineSpecificStorage;
};

class TestCJScript9ClassFactory : public CJScript9ClassFactory
{
protected:
    ScriptEngine *AllocateEngine(__in LPOLESTR szLangName)
    {
        return HeapNewNoThrow(TestScriptEngine, GetTypeId(), szLangName);
    }
};

CClassFactory * CreateTestJscript9ClassFactory(void)
{
    return new TestCJScript9ClassFactory();
}

HRESULT __stdcall NotifyOnScriptStateChanged(NotifyOnScriptStateChangedCallBackFuncPtr pfCallBack)
{
    pfCreateJscript9ClassFactory = CreateTestJscript9ClassFactory;
    pfNotifyOnScriptStateChangedCallBackFuncPtr = pfCallBack;
    return S_OK;
}

HRESULT __stdcall ClearAllProjectionCaches(IActiveScriptDirect * scriptDirect) 
{
    auto scriptEngine = (ScriptEngine*)scriptDirect;
    scriptEngine->ResetProjectionContext();
    return S_OK;
}

HRESULT __stdcall SetAssertToConsoleFlag(bool flag)
{
#ifdef DBG
    AssertsToConsole = flag;
#endif
    return S_OK;
}

HRESULT __stdcall FinalGC()
{
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT) || defined(INTERNAL_MEM_PROTECT_HEAP_ALLOC)
    bool doFinalGC = false;

#if defined(LEAK_REPORT)
    if (Js::Configuration::Global.flags.IsEnabled(Js::LeakReportFlag))
    {
        doFinalGC = true;
    }
#endif

#if defined(CHECK_MEMORY_LEAK)
    if (Js::Configuration::Global.flags.CheckMemoryLeak)
    {
        doFinalGC = true;
    }
#endif

#if defined(INTERNAL_MEM_PROTECT_HEAP_ALLOC)
    if (Js::Configuration::Global.flags.MemProtectHeap)
    {
        doFinalGC = true;
    }
#endif

    if (!doFinalGC)
    {
        return E_FAIL;
    }


    ThreadContext * threadContext = ThreadContext::GetContextForCurrentThread();

    // Don't run the final GC if no script context was ever registered with the thread
    // context- there wouldn't be anything interesting to collect anyway
    if (!threadContext || threadContext->WasAnyScriptContextEverRegistered() == false)
    {
        return E_FAIL;
    }
    Recycler * recycler = threadContext->GetRecycler();  
    // Recycler might not have initialized, check if it is null
    if (recycler)
    {
        recycler->EnsureNotCollecting();    
        recycler->CollectNow<CollectNowFinalGC>();    
        Assert(!recycler->CollectionInProgress());
    }
#endif
    return S_OK;
}

HRESULT __stdcall GetThreadService(IActiveScriptGarbageCollector** threadService)
{
    HRESULT hr = E_FAIL;
    ThreadContext * threadContext = ThreadContext::GetContextForCurrentThread();
    if (threadContext)
    {
        ThreadServiceWrapper* threadServiceWrapper = threadContext->GetThreadServiceWrapper();
        if (threadServiceWrapper)
        {
            JavascriptThreadService* javascriptThreadService = static_cast<JavascriptThreadService*>(threadServiceWrapper);
            if (javascriptThreadService)
            {
                hr = javascriptThreadService->QueryInterface(__uuidof(IActiveScriptGarbageCollector), (void**)threadService);
            }
        }
    }
    return hr;
}

HRESULT __stdcall SetEnableCheckMemoryLeakOutput(bool flag)
{
#if defined(CHECK_MEMORY_LEAK)
    MemoryLeakCheck::SetEnableOutput(flag);
#endif
    return S_OK;
}

void __stdcall NotifyUnhandledException(PEXCEPTION_POINTERS exceptionInfo)
{
#ifdef GENERATE_DUMP
    // We already reported assert at the assert point, don't do it here again
    if (exceptionInfo->ExceptionRecord->ExceptionCode != STATUS_ASSERTION_FAILURE)
    {
        if (Js::Configuration::Global.flags.IsEnabled(Js::DumpOnCrashFlag))
        {
            Js::Throw::GenerateDump(exceptionInfo, Js::Configuration::Global.flags.DumpOnCrash);
        }
    }
#endif
}
HRESULT __stdcall FlushOutput()
{
    Output::Flush();
    return S_OK;
}
#ifdef FAULT_INJECTION
unsigned int __stdcall GetCurrentFaultInjectionCount()
{
    return Js::FaultInjection::Global.countOfInjectionPoints;
}
#endif

HRESULT OnJScript9Loaded()
{
    typedef HRESULT (__stdcall *OnJScript9LoadedPtr)(TestHooks &testHooks);
    OnJScript9LoadedPtr pfJScript9Loaded = (OnJScript9LoadedPtr)GetProcAddress(GetModuleHandle(NULL), "OnJScript9LoadedEntry");
    if (! pfJScript9Loaded)
    {
        return S_OK;
    }

    TestHooks testHooks = 
    {
        DllGetClassObject,
        JsVarToScriptDirect,
        JsVarAddRef,
        JsVarRelease,
        JsVarToExtension,
        SetConfigFlags,
        GenerateValidPointersMapHeader,
        PrintConfigFlagsUsageString,        
        GetRestrictedString,
        GetCapability,
        GetSystemStringFromHr,
        GetMemoryFootprintOfRC,
        DoNotSupportWeakDelegate,
        SupportsWeakDelegate,
        NotifyOnScriptStateChanged,        
        ClearAllProjectionCaches,
        SetAssertToConsoleFlag,
        SetEnableCheckMemoryLeakOutput,
        FinalGC,
        ActiveScriptProfilerHeapEnum::SetGetHeapObjectInfoPtr,
        GetThreadService,
        StartScriptProfiling,
        StopScriptProfiling,
        DisplayMemStats,
        FlushOutput,
        GetContentOfSharedArrayBuffer,
        CreateSharedArrayBufferFromContent,
#ifdef ENABLE_INTL_OBJECT
        ResetTimeZoneFactoryObjects,
#endif
#ifdef FAULT_INJECTION
        GetCurrentFaultInjectionCount,
#endif

#define FLAG(type, name, description, defaultValue, ...) FLAG_##type##(name)
#define FLAGINCLUDE(name) \
    IsEnabled##name##Flag, \
    Get##name##Flag, \
    Set##name##Flag,
#define FLAG_String(name) FLAGINCLUDE(name)
#define FLAG_Boolean(name) FLAGINCLUDE(name)
#define FLAG_Number(name) FLAGINCLUDE(name)
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

        NotifyUnhandledException
    };

    return pfJScript9Loaded(testHooks);
}

#endif // ENABLE_TEST_HOOKS

