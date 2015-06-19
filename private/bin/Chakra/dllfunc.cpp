//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Description:top level functions for dllMain and COM server 
//TBD to restore the TLS support

#include "StdAfx.h"
#include "guids.h"
#include "proxystub.h"

extern CClassFactory* CreateJScript9DACClassFactory(void);
extern CClassFactory* CreateDiagHookClassFactory(void);
extern CClassFactory* CreateJScript9ThreadServiceClassFactory();
CClassFactory* CreateJscript9ClassFactory(void);

CClassFactory* (*pfCreateJscript9ClassFactory)(void) = CreateJscript9ClassFactory;
typedef CClassFactory* (PFNCreateClassFactory)(void);
PFNCreateClassFactory* const classFactories[] = {CreateJscript9ClassFactory, CreateJScript9ThreadServiceClassFactory};

const WCHAR * threadModelBoth        = L"Both";
const WCHAR * threadModelApartment   = L"Apartment";
ATOM  lockedDll = 0;
const WCHAR * g_pstrDLLName = L"";

static long s_cLibRef   = 0;
static CriticalSection s_csDllCanUnloadNow;

//#include <initguid.h>

#include "muiload.h"

//IE MSHTML responsiveness events
#ifdef F_JSETW
#include <IERESP_mshtml.h>
#endif
extern HANDLE g_hInstance;
#if DEBUG
extern void ValidateNameTableTerm(void);
#endif

static BOOL AttachProcess(HANDLE hmod)
{    
    if (!ThreadContextTLSEntry::InitializeProcess()
#if !defined(LANGUAGE_SERVICE)
      || !JsrtContext::Initialize()
#endif
      )
    {
        return FALSE;
    }
    
    Assert(GetVersion() < 0x80000000);
    g_hInstance = hmod;
    AutoSystemInfo::SaveModuleFileName(hmod);

#if defined(_M_IX86)
    // Enable SSE2 math functions in CRT if SSE2 is available
    _set_SSE2_enable(TRUE);
#endif
    if (FAILED(OnJScript9Loaded()))
    {
        return FALSE;
    }
#if DOMEnabled
    InitializeAdditionalProperties = Js::DOMProperties::InitializeDOMProperties;
    TotalNumberOfBuiltInProperties = Js::DOMPropertyIds::_count;
#endif
    {
        CmdLineArgsParser parser;

        ConfigParser::ParseOnModuleLoad(parser, hmod);
    }

    EtwTrace::Register();
    ValueType::Initialize();
    if (!BinaryFeatureControl::LanguageService())
    {
        wchar_t *engine = szChakraLock;
#if DEBUG


        if (Js::Configuration::Global.flags.ForceLegacyEngine)
        {
            engine = szJScript9Lock;
            if (::FindAtom(szChakraLock) != 0)
            {
                AssertMsg(FALSE, "Expecting to Load jscrip9.dll but process already loaded chakra.dll");
                Binary_Inconsistency_fatal_error();
            }
        }
        else
#endif
        {
            if (::FindAtom(szJScript9Lock) != 0)
            {
                AssertMsg(FALSE, "Expecting to Load chakra.dll but process already loaded jscrip9.dll");
                Binary_Inconsistency_fatal_error();
            }
        }
        lockedDll = ::AddAtom(engine);
        AssertMsg(lockedDll, "failed to lock the dll");
    }
    ThreadContext::GlobalInitialize();

    g_TraceLoggingClient = NoCheckHeapNewStruct(TraceLoggingClient);

#ifdef DYNAMIC_PROFILE_STORAGE
    return DynamicProfileStorage::Initialize();
#else
    return TRUE;
#endif    
}

static void CleanupResDLLMap(void)
{
    if (NULL != g_pgllmap)
    {
        LMAP lmap;
        while(g_pgllmap->FPop(&lmap))
        {
            if (lmap.fUnload && lmap.hlib)
            {
                //#ifdef SCRIPT_MUI
                if (lmap.fMui)
                    FreeMUILibrary((HMODULE)lmap.hlib);
                else
                    //#endif // SCRIPT_MUI
                    FreeLibrary((HINSTANCE)lmap.hlib);
            }
        }
        g_pgllmap->Release();
        g_pgllmap = NULL;
    }
}

/****************************** Public Functions *****************************/

#if _WIN32 || _WIN64
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hmod, DWORD dwReason, PVOID pvReserved)
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
        if (!BinaryFeatureControl::LanguageService())
        {
            lockedDll = ::DeleteAtom(lockedDll); // If the function succeeds, the return value is zero.
            AssertMsg(lockedDll == 0, "Failed to release the lock");
        }

#ifdef DYNAMIC_PROFILE_STORAGE    
        DynamicProfileStorage::Uninitialize();
#endif
        // Do this before DetachProcess() so that we won't have ETW rundown callbacks while destroying threadContexts.
        EtwTrace::UnRegister();

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

void DetachProcess()
{    
    if (g_hInstance == NULL)
    {
        return;
    }

    if (g_TraceLoggingClient != nullptr)
    {
        NoCheckHeapDelete(g_TraceLoggingClient);
    }

    ThreadBoundThreadContextManager::DestroyAllContextsAndEntries();

    // In JScript, we never unload except for when the app shuts down
    // because DllCanUnloadNow always returns S_FALSE. As a result
    // its okay that we never try to cleanup. Attempting to cleanup on
    // shutdown is bad because we shouldn't free objects built into
    // other dlls.

#if !defined(LANGUAGE_SERVICE)
    JsrtRuntime::Uninitialize();
    JsrtContext::Uninitialize();
#endif

    ThreadContextTLSEntry::CleanupProcess();
    
    CleanupResDLLMap();

#if defined(ENABLE_DEBUG_CONFIG_OPTIONS)
    if (Js::Configuration::Global.flags.Console && Js::Configuration::Global.flags.ConsoleExitPause)
    {
        HANDLE handle = GetStdHandle( STD_INPUT_HANDLE );

        FlushConsoleInputBuffer(handle);

        Output::Print(L"Press any key to exit...\n");
        Output::Flush();

        WaitForSingleObject(handle, INFINITE);

    }
#endif

#if DEBUG
    //ValidateNameTableTerm(); //TBD - replace this with an equivalent mechanism. When dettaching is good to verify no objects leak
#endif
#if PROFILE_DICTIONARY
    DictionaryStats::OutputStats();
#endif
    g_hInstance = NULL;
}

//
// declare class factories
//

STDAPIEXPORT DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    CClassFactory * pCF = NULL;

    // Don’t check for null as per Win8 495205. This will never validly be null. When it is, an AV is appropriate

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
    else if (rclsid == CLSID_DiagHook)
    {
        pCF = CreateDiagHookClassFactory();
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
    return (LONG)(s_cLibRef + g_cLibLocks);
}
