//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Description:top level functions for dllMain and COM server 
//TBD to restore the TLS support

#include "EnginePch.h"
#include "core\AtomLockGuids.h"
#include "proxystub.h"
#include "core\ConfigParser.h"
#ifdef DYNAMIC_PROFILE_STORAGE
#include "Language\DynamicProfileStorage.h"
#endif
#include "JsrtContext.h"
#include "TestHooks.h"

#ifdef ENABLE_BASIC_TELEMETRY
#include "..\Telemetry\Telemetry.h"
#endif

extern CClassFactory* CreateJScript9DACClassFactory(void);
extern CClassFactory* CreateJScript9ThreadServiceClassFactory();
CClassFactory* CreateJscript9ClassFactory(void);

CClassFactory* (*pfCreateJscript9ClassFactory)(void) = CreateJscript9ClassFactory;
typedef CClassFactory* (PFNCreateClassFactory)(void);
PFNCreateClassFactory* const classFactories[] = {CreateJscript9ClassFactory, CreateJScript9ThreadServiceClassFactory};

static const WCHAR * threadModelBoth        = _u("Both");
static const WCHAR * threadModelApartment   = _u("Apartment");
static ATOM  lockedDll = 0;
static long s_cLibRef   = 0;
static CriticalSection s_csDllCanUnloadNow;

extern HANDLE g_hInstance;

static BOOL AttachProcess(HANDLE hmod)
{    
    if (!ThreadContextTLSEntry::InitializeProcess())
    {
        return FALSE;
    }
    
    Assert(GetVersion() < 0x80000000);
    g_hInstance = hmod;
    AutoSystemInfo::SaveModuleFileName(hmod);

#if defined(_M_IX86) && !defined(_M_HYBRID_X86_ARM64)
    // Enable SSE2 math functions in CRT if SSE2 is available
    _set_SSE2_enable(TRUE);
#endif
    if (FAILED(OnJScript9Loaded()))
    {
        return FALSE;
    }
#if DOMEnabled
    InitializeAdditionalProperties = Js::DOMProperties::InitializeDOMProperties;
    Js::TotalNumberOfBuiltInProperties = Js::DOMPropertyIds::_count;
#endif
    {
        CmdLineArgsParser parser;

        ConfigParser::ParseOnModuleLoad(parser, hmod);
    }

    JS_ETW(EtwTrace::Register());
#ifdef VTUNE_PROFILING
    VTuneChakraProfile::Register();
#endif

    ValueType::Initialize();

    char16 *engine = szChakraLock;

#if DEBUG

    if (Js::Configuration::Global.flags.ForceLegacyEngine)
    {
        engine = szJScript9Lock;
        if (::FindAtom(szChakraLock) != 0)
        {
            AssertMsg(FALSE, "Expecting to load jscrip9.dll but process already loaded chakra.dll");
            Binary_Inconsistency_fatal_error();
        }
        if (::FindAtom(szChakraCoreLock) != 0)
        {
            AssertMsg(FALSE, "Expecting to load jscrip9.dll but process already loaded chakracore.dll");
            Binary_Inconsistency_fatal_error();
        }
    }
    else
#endif
    {
        if (::FindAtom(szJScript9Lock) != 0)
        {
            AssertMsg(FALSE, "Expecting to load chakra.dll but process already loaded jscrip9.dll");
            Binary_Inconsistency_fatal_error();
        }
        if (::FindAtom(szChakraCoreLock) != 0)
        {
            AssertMsg(FALSE, "Expecting to load chakra.dll but process already loaded chakracore.dll");
            Binary_Inconsistency_fatal_error();
        }
    }
    lockedDll = ::AddAtom(engine);
    AssertMsg(lockedDll, "failed to lock the dll");

    ThreadContext::GlobalInitialize();

#ifdef ENABLE_BASIC_TELEMETRY
    g_TraceLoggingClient = NoCheckHeapNewStruct(TraceLoggingClient);
#endif

#ifdef DYNAMIC_PROFILE_STORAGE
    return DynamicProfileStorage::Initialize();
#else
    return TRUE;
#endif    
}

void DetachProcess()
{
    if (g_hInstance == NULL)
    {
        return;
    }

#ifdef ENABLE_BASIC_TELEMETRY
    if (g_TraceLoggingClient != nullptr)
    {
        NoCheckHeapDelete(g_TraceLoggingClient);
        g_TraceLoggingClient = nullptr;
    }
#endif

    ThreadBoundThreadContextManager::DestroyAllContextsAndEntries();

    // In JScript, we never unload except for when the app shuts down
    // because DllCanUnloadNow always returns S_FALSE. As a result
    // its okay that we never try to cleanup. Attempting to cleanup on
    // shutdown is bad because we shouldn't free objects built into
    // other dlls.

    JsrtRuntime::Uninitialize();

    // threadbound entrypoint should be able to get cleanup correctly, however tlsentry
    // for current thread might be left behind if this thread was initialized.
    ThreadContextTLSEntry::CleanupThread();

    ThreadContextTLSEntry::CleanupProcess();

#if defined(ENABLE_DEBUG_CONFIG_OPTIONS)
    if (Js::Configuration::Global.flags.Console && Js::Configuration::Global.flags.ConsoleExitPause)
    {
        HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);

        FlushConsoleInputBuffer(handle);

        Output::Print(_u("Press any key to exit...\n"));
        Output::Flush();

        WaitForSingleObject(handle, INFINITE);

    }
#endif

#if PROFILE_DICTIONARY
    DictionaryStats::OutputStats();
#endif
    g_hInstance = NULL;
}

/****************************** Public Functions *****************************/

#if _WIN32 || _WIN64
EXTERN_C BOOL WINAPI ChakraDllMain(HINSTANCE hmod, DWORD dwReason, PVOID pvReserved)
{
    // Call DllMain implementation for the the proxy/stub objects for JscriptInfo.idl
    if (!JscriptInfoPrxDllMain(hmod, dwReason, pvReserved))
    {
        return FALSE;
    }

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        return AttachProcess(hmod);
    }
    case DLL_THREAD_ATTACH:
        ThreadContextTLSEntry::InitializeThread();
#ifdef HEAP_TRACK_ALLOC
        HeapAllocator::InitializeThread();
#endif
        return TRUE;

    case DLL_THREAD_DETACH:  
        // If we are not doing DllCanUnloadNow, so we should clean up. Otherwise, DllCanUnloadNow is already running, 
        // so the ThreadContext global lock is already taken.  If we try to clean up, we will block on the ThreadContext 
        // global lock while holding the loader lock, which DllCanUnloadNow may block on waiting for thread temination
        // which requires the loader lock. DllCanUnloadNow will clean up for us anyway, so we can just skip the whole thing.        
        if (s_csDllCanUnloadNow.TryEnter())
        {            
            ThreadBoundThreadContextManager::DestroyContextAndEntryForCurrentThread();    
            s_csDllCanUnloadNow.Leave();
        }
        return TRUE;

    case DLL_PROCESS_DETACH:

        lockedDll = ::DeleteAtom(lockedDll); // If the function succeeds, the return value is zero.
        AssertMsg(lockedDll == 0, "Failed to release the lock for chakra.dll");


#ifdef DYNAMIC_PROFILE_STORAGE    
        DynamicProfileStorage::Uninitialize();
#endif
        // Do this before DetachProcess() so that we won't have ETW rundown callbacks while destroying threadContexts.
        JS_ETW(EtwTrace::UnRegister());
#ifdef VTUNE_PROFILING
        VTuneChakraProfile::UnRegister();
#endif

        // don't do anything if we are in forceful shutdown
        // try to clean up handles in graceful shutdown
        if (pvReserved == NULL)
        {
            DetachProcess();
        }
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
        else 
        {            
            ThreadContext::ReportAndCheckLeaksOnProcessDetach();
        }
#endif
        return TRUE;

    default:
        AssertMsg(FALSE, "DllMain() called with unrecognized dwReason.");
        return FALSE;
    }
}
#endif // _WIN32 || _WIN64


void DiagHookDumpGlobals();

void CALLBACK
DumpDiagInfo(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
    DiagHookDumpGlobals();
}


//
// declare class factories
//

STDAPIEXPORT DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    CClassFactory * pCF = NULL;

    // Don�t check for null as per Win8 495205. This will never validly be null. When it is, an AV is appropriate

    // Call DllGetClassObject implementation for the the proxy/stub objects for JscriptInfo.idl
    if ((hr = JscriptInfoPrxDllGetClassObject(rclsid, riid, ppv)) == S_OK)
    {
        return S_OK;
    }

    if (CLASS_E_CLASSNOTAVAILABLE != hr)
    {
        return hr;
    }

    if (rclsid == CLSID_Chakra)
    {
        pCF = pfCreateJscript9ClassFactory();
    }
    else if (rclsid == CLSID_JScript9DAC)
    {
        pCF = CreateJScript9DACClassFactory();
    }
    else if (rclsid == CLSID_ChakraThreadService)
    {
        pCF = CreateJScript9ThreadServiceClassFactory();
    }
    else
    {
        return  CLASS_E_CLASSNOTAVAILABLE;
    }

    if (NULL == pCF)
    {
        *ppv = NULL;
        return E_OUTOFMEMORY;
    }

    Assert(rclsid == pCF->GetTypeId());

    hr = pCF->QueryInterface(riid, ppv);
    pCF->Release();
    return hr;
}


STDAPIEXPORT DllRegisterServer(void)
{
    HRESULT hr;
    CClassFactory * pCF;

    for (int i = 0; i < _countof(classFactories); i++)
    {
        pCF = classFactories[i]();
        if (!pCF)
            return E_UNEXPECTED;
        hr = pCF->RegisterServer(threadModelBoth);
        pCF->Release();
        IFFAILRET(hr);
    }
    // Call DllRegisterServer implementation for the the proxy/stub objects for JscriptInfo.idl
    hr = JscriptInfoPrxDllRegisterServer();

    return hr;
}


STDAPIEXPORT DllUnregisterServer(void)
{
    HRESULT hr;
    CClassFactory * pCF;

    for (int i = 0 ; i < _countof(classFactories); i++)
    {
        pCF = classFactories[i]();
        if (!pCF)
            hr = E_UNEXPECTED;
        else
        {
            hr = pCF->UnregisterServer();
            pCF->Release();
        }
        IFFAILRET(hr);
    }

    // Call DllUnregisterServer implementation for the the proxy/stub objects for JscriptInfo.idl
    hr = JscriptInfoPrxDllUnregisterServer();

    return hr;
}


//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
class CComEngineModule :
    public CComModule
{
public:
    LONG Lock();
    LONG Unlock();
    LONG GetLockCount();
};

CComEngineModule _Module;

void DLLAddRef(void)
{
    _Module.Lock();    
    Assert(_Module.GetLockCount() >= 0);
}

void DLLRelease(void)
{
    _Module.Unlock();    
    Assert(_Module.GetLockCount() >= 0);
}

STDAPIEXPORT DllCanUnloadNow(void)
{
    // Call DllCanUnloadNow implementation for the the proxy/stub objects for JscriptInfo.idl
    HRESULT hr = JscriptInfoPrxDllCanUnloadNow();
    if (hr != S_OK)
    {
        return hr;
    }
    if (_Module.GetLockCount() == 0)
    {
        // Since DestoryAllContexts will wait for thread finish, we can't have THREAD_DETACH
        // block on waiting the ThreadContext global lock while holding the loader lock
        // Take the s_csDllcanUnloadNow lock so that THREAD_DETACH will not try to take the ThreadContext
        // global lock when we are processing DllCanUnloadNow.
        AutoCriticalSection autocs1(&s_csDllCanUnloadNow);
        AutoCriticalSection autocs2(ThreadContext::GetCriticalSection());
        if (_Module.GetLockCount() == 0)
        {
            ThreadBoundThreadContextManager::DestroyAllContexts();
#ifdef INTERNAL_MEM_PROTECT_HEAP_ALLOC
            HeapAllocator::Instance.FinishMemProtectHeapCollect();
#endif
            // TODO: Return S_OK when we wait for the working thread to finish before returning
            return NOERROR;
        }        
    }
    return S_FALSE;

}

LONG CComEngineModule::Lock()
{
    // Ignore Assert for IE9. In IE10 will address usage of this flag

    // Assert(!g_fContextsDestroyed); 
    return InterlockedIncrement((LONG *)&s_cLibRef);
}

LONG CComEngineModule::Unlock() 
{
    return InterlockedDecrement((LONG *)&s_cLibRef);
}

LONG CComEngineModule::GetLockCount() 
{
    return (LONG)s_cLibRef;
}

static bool GetDeviceFamily(_Out_opt_ ULONG* pulDeviceFamily)
{
    bool deviceInfoRetrieved = false;

    HMODULE hModNtDll = GetModuleHandle(_u("ntdll.dll"));
    if (hModNtDll == nullptr)
    {
        RaiseException(0, EXCEPTION_NONCONTINUABLE, 0, 0);
    }
    typedef void(*PFNRTLGETDEVICEFAMILYINFOENUM)(ULONGLONG*, ULONG*, ULONG*);
    PFNRTLGETDEVICEFAMILYINFOENUM pfnRtlGetDeviceFamilyInfoEnum =
        reinterpret_cast<PFNRTLGETDEVICEFAMILYINFOENUM>(GetProcAddress(hModNtDll, "RtlGetDeviceFamilyInfoEnum"));

    if (pfnRtlGetDeviceFamilyInfoEnum)
    {
        ULONGLONG UAPInfo;
        ULONG DeviceForm;
        pfnRtlGetDeviceFamilyInfoEnum(&UAPInfo, pulDeviceFamily, &DeviceForm);
        deviceInfoRetrieved = true;
    }

    return deviceInfoRetrieved;
}

#if DBG
// __DATE__ and __TIME__ are only available in debug for chakra.dll
#define USE_DATE_TIME_MACRO
#else
#include <rtlfilever.c> // For RtlGetVersionResourceFromSelf in release builds
#endif

void ChakraBinaryAutoSystemInfoInit(AutoSystemInfo * autoSystemInfo)
{
    ULONG DeviceFamily;
    if (GetDeviceFamily(&DeviceFamily))
    {
        bool isMobile = (DeviceFamily == 0x00000004 /*DEVICEFAMILYINFOENUM_MOBILE*/);
        autoSystemInfo->shouldQCMoreFrequently = isMobile;
        autoSystemInfo->supportsOnlyMultiThreadedCOM = isMobile;  //TODO: pick some other platform to the list
        autoSystemInfo->isLowMemoryDevice = isMobile;  //TODO: pick some other platform to the list
    }

#ifdef USE_DATE_TIME_MACRO
    autoSystemInfo->buildDateHash = JsUtil::CharacterBuffer<char>::StaticGetHashCode(__DATE__, _countof(__DATE__));
    autoSystemInfo->buildTimeHash = JsUtil::CharacterBuffer<char>::StaticGetHashCode(__TIME__, _countof(__TIME__));
#else
    // Release builds cannot use __DATE__ or __TIME__
    RTL_VERSION_RESOURCE buildTimestamp;
    RtlGetVersionResourceFromSelf(&buildTimestamp);

    if (buildTimestamp.Success)
    {
        autoSystemInfo->buildDateHash = JsUtil::CharacterBuffer<char16>::StaticGetHashCode(
            buildTimestamp.StringFileInfo.FileVersion.__Date__,
            _countof(buildTimestamp.StringFileInfo.FileVersion.__Date__));
        autoSystemInfo->buildTimeHash = JsUtil::CharacterBuffer<char16>::StaticGetHashCode(
            buildTimestamp.StringFileInfo.FileVersion.__Time__,
            _countof(buildTimestamp.StringFileInfo.FileVersion.__Time__));
    }
#endif
}