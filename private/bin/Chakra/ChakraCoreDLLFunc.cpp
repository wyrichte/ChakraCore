#include <stdafx.h>

static BOOL AttachProcess(HANDLE hmod)
{
    if (!ThreadContextTLSEntry::InitializeProcess() || !JsrtContext::Initialize())
    {
        return FALSE;
    }

    AutoSystemInfo::SaveModuleFileName(hmod);

#if defined(_M_IX86)
    // Enable SSE2 math functions in CRT if SSE2 is available
    _set_SSE2_enable(TRUE);
#endif
    {
        CmdLineArgsParser parser;

        ConfigParser::ParseOnModuleLoad(parser, hmod);
    }

    EtwTrace::Register();
    ValueType::Initialize();
    ThreadContext::GlobalInitialize();

    g_TraceLoggingClient = NoCheckHeapNewStruct(TraceLoggingClient);

#ifdef DYNAMIC_PROFILE_STORAGE
    return DynamicProfileStorage::Initialize();
#else
    return TRUE;
#endif    
}

static void DetachProcess()
{
    ThreadBoundThreadContextManager::DestroyAllContextsAndEntries();

    // In JScript, we never unload except for when the app shuts down
    // because DllCanUnloadNow always returns S_FALSE. As a result
    // its okay that we never try to cleanup. Attempting to cleanup on
    // shutdown is bad because we shouldn't free objects built into
    // other dlls.
    JsrtRuntime::Uninitialize();
    JsrtContext::Uninitialize();

    ThreadContextTLSEntry::CleanupProcess();

#if PROFILE_DICTIONARY
    DictionaryStats::OutputStats();
#endif
}

/****************************** Public Functions *****************************/

#if _WIN32 || _WIN64
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hmod, DWORD dwReason, PVOID pvReserved)
{
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
        ThreadBoundThreadContextManager::DestroyContextAndEntryForCurrentThread();
        return TRUE;

    case DLL_PROCESS_DETACH:
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
