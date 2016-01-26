//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

/****************************************************************************
 * WWAHostJSCounterProvider
 *
 * NOTE: This works with any AppContainer process hosting jscript9.dll
 * Not just WWAHost.exe
 *
 * WWAHost runs in AppContainer, and it has no access to the v2 perf counter
 * driver in Win8.  Basically, v2 perf counter is not supported in Win8 
 * AppContainer.
 * 
 * In order to work around that, JScript9 running under AppContainer (or 
 * any reason perf counter failed to initialize) will create a shared 
 * memory named object for the counters.
 * 
 * WWAHostJSCounterProvider will be running in full trust. It registers
 * as a provider of the jscript9 perf counters.  When perflib (perfmon)
 * start enumerating instances of perf counters, we will scan all the process
 * and see if they are running inside an AppContainer and have the shared memory
 * named object (under the \\Session\<d>\AppContainerNamedObject\<SID>)
 *
 * If the process has those shared memory, it will create per counter instance
 * for them.  For WWAHost, (or any package process), the instance
 * will be named after the package name, e.g. WinStore).  For others, 
 * it will be the module name (e.g. iexplore.exe for MoBro)
 *
 * WWAHostJSCounterProvider will monitor the AppContainer  process it has create
 * counters for until it quits (via RegisterWaitForSingleObject to wait for 
 * the process to signal when it exits), at which point, it will destroy
 * the perf counter instance.
 *
 ****************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <psapi.h>
#include <appmodel.h>

#pragma warning(disable: 4512)  // private operator= are good to have

/*================================================
 * Include shared perf counter implementation
 *================================================*/
#define Assert(x)
class BinaryFeatureControl
{
public:
    static bool PerfCounter() { return true; }
};

#ifndef PERF_COUNTERS
#define PERF_COUNTERS
#endif
#ifndef HEAP_PERF_COUNTERS
#define HEAP_PERF_COUNTERS
#endif
#ifndef RECYCLER_PERF_COUNTERS
#define RECYCLER_PERF_COUNTERS
#endif
#ifndef PROFILE_RECYCLER_ALLOC
#define PROFILE_RECYCLER_ALLOC
#endif
#define ENABLE_COUNTER_NOTIFICATION_CALLBACK
typedef unsigned int uint;
#include "perfcounter.h"
#include "..\Memory\PageAllocatorDefines.h"
#include "perfcounterImpl.cpp"

/*=====================================================
 * Function to get the AppContain object name path
 * Copied and slightly modified from inetcore/lib/il/lowbox.cxx 
 *=====================================================*/
static
HRESULT _GetProcessToken(__in DWORD dwDesiredAccess, __in DWORD dwProcessId, __out HANDLE *phProcessToken)
{
    HRESULT hr;
    HANDLE hProcess = OpenProcess(dwDesiredAccess, FALSE, dwProcessId);
    if (hProcess)
    {
        if (OpenProcessToken(hProcess, TOKEN_QUERY, phProcessToken))
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        CloseHandle(hProcess);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

typedef BOOL (pfnGetAppContainerNamedObjectPath) (
    __in_opt HANDLE Token,
    __in_opt PSID AppContainerSid,
    __inout ULONG ObjectPathLength,
    __out_ecount_opt(ObjectPathLength) LPWSTR ObjectPath,
    __out PULONG ReturnLength
    );

static
HRESULT
GetProcessAppContainerNamedObjectPath(
    __in DWORD dwProcessId,
    __out __ecount(cPath) PWCHAR pszPath,
    __in DWORD cPath
    )
{
    HRESULT hr = E_FAIL;

    if (cPath > 0)
    {
        *pszPath = 0;
        HANDLE hToken = NULL; // default to current process
        if (dwProcessId)
        {
            hr = _GetProcessToken(PROCESS_QUERY_INFORMATION, dwProcessId, &hToken);
        }
        else
        {
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
            DWORD cPathRet = 0;
            if (!GetAppContainerNamedObjectPath(hToken, NULL, cPath, pszPath, &cPathRet))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        if (hToken)
        {
            CloseHandle(hToken);
        }
    }

    return hr;
}

/*=============================================
 * Perf counter instance implementation
 *=============================================*/
using namespace PerfCounter;

template <typename TCounter>
class ProxyCounterSetInstance : public InstanceBase
{
public:
    ProxyCounterSetInstance() : InstanceBase(TCounter::GetProvider(), TCounter::GetGuid()), data(NULL)
    {
    }
    ~ProxyCounterSetInstance()
    {
        if (data != NULL)
        {
            __super::UninitializeSharedMemory(data, handle);
        }
    }
    bool IsEnabled() { return data != NULL; }

    bool Initialize(wchar_t const * wszInstanceName, DWORD processId)
    {
        if (IsProviderInitialized())
        {
            /* Get the app continaers name object path */
            wchar_t szAppContainerNameObjectPath[MAX_OBJECT_NAME_PREFIX];
            HRESULT hr = GetProcessAppContainerNamedObjectPath(processId, szAppContainerNameObjectPath, _countof(szAppContainerNameObjectPath));
            if (FAILED(hr))
            {
                return false;
            }

            /* Open the shared memory */
            data = __super::OpenSharedMemory(szAppContainerNameObjectPath, processId, TCounter::MaxCounter, handle);
            if (data == NULL)
            {
                return false;
            }

            /* initialize the perf counter instance */
            if (!__super::Initialize(wszInstanceName, processId))
            {
                return false;
            }

            for (DWORD i = 0; i < TCounter::MaxCounter; i++)
            {
                counters[i].Initialize(*this, i, &data[i]);
            }

            return true;
        }
        return false;
    }
private:
    HANDLE handle;
    DWORD * data;
    Counter counters[TCounter::MaxCounter];
};

/*=========================================================
 * Instance of per process counters 
 *=========================================================*/

class Counters
{
public:
    Counters() : wszInstanceName(NULL), hProcess(NULL) {}
    ~Counters()
    {
        if (wszInstanceName != NULL)
        {
            delete [] wszInstanceName;
        }
        if (hProcess != NULL)
        {
            CloseHandle(hProcess);
        }
    }
    bool Initialize(DWORD processId)
    {
        hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, FALSE, processId);
        if (hProcess == NULL)
        {
            return false;
        }

        // Get the package name to use as instance name
        wszInstanceName = GetPackageName(hProcess);
        if (!wszInstanceName)
        {
            // Get the module name to use as instance name instead
            wchar_t wszModuleName[_MAX_PATH];

            if (!GetModuleFileNameEx(hProcess, NULL, wszModuleName, _MAX_PATH))
            {
                return false;
            }

            wchar_t wszFilename[_MAX_FNAME];
            _wsplitpath_s(wszModuleName, NULL, 0, NULL, 0, wszFilename, _MAX_FNAME, NULL, 0);
            wszInstanceName = _wcsdup(wszFilename);
            if (wszInstanceName == NULL)
            {
                return false;
            }
        }

        // heap/recycler counter may not exist
        heapCounter.Initialize(wszInstanceName, processId);
        recyclerCounter.Initialize(wszInstanceName, processId);
        recyclerTrackerCounter.Initialize(wszInstanceName, processId);

        return pageAllocatorCounter.Initialize(wszInstanceName, processId)
            && basicCounter.Initialize(wszInstanceName, processId)
            && codeCounter.Initialize(wszInstanceName, processId);
    }

    HANDLE GetProcessHandle()
    {
        return hProcess;
    }
    wchar_t const * GetInstanceName()
    {
        return wszInstanceName;
    }
private:
    static wchar_t const * GetPackageName(HANDLE hProcess)
    {
        // Get the package name to use as instance name
        UINT32 len = 0;
        if (ERROR_INSUFFICIENT_BUFFER != GetPackageId(hProcess, &len, NULL))
        {
            return NULL;
        }

        BYTE * buffer = new BYTE[len];
        if (buffer == NULL)
        {
            return NULL;
        }

        wchar_t const * ret = NULL;
        if (ERROR_SUCCESS == GetPackageId(hProcess, &len, buffer))
        {
            ret = _wcsdup(((PACKAGE_ID *)buffer)->name);
        }
        delete [] buffer;
        return ret;
    }

    wchar_t const * wszInstanceName;
    HANDLE hProcess;
    ProxyCounterSetInstance<PageAllocatorCounterSetDefinition> pageAllocatorCounter;
    ProxyCounterSetInstance<BasicCounterSetDefinition> basicCounter;
    ProxyCounterSetInstance<CodeCounterSetDefinition> codeCounter;
    ProxyCounterSetInstance<HeapCounterSetDefinition> heapCounter;
    ProxyCounterSetInstance<RecyclerCounterSetDefinition> recyclerCounter;
    ProxyCounterSetInstance<RecyclerTrackerCounterSetDefinition> recyclerTrackerCounter;
};

struct CounterRecord
{
    Counters * counters;
    HANDLE waitHandle;
};


// Map to track current acctive counters
#include <map>
static std::map<DWORD, CounterRecord> s_counterMap;

// Syncronizing between the main thread and the notification thread, and on process exit worker thread
CRITICAL_SECTION s_cs;

// Event to notify the main thread to clean up process records
HANDLE s_cleanupEvent = NULL;

/* ===================================================================
 * Call back function to clean up the counters when the process exit 
 * ===================================================================*/
VOID CALLBACK OnProcessExitCallBack(PVOID lpParameter, BOOLEAN)
{
    ::EnterCriticalSection(&s_cs);
    DWORD pid = (DWORD)lpParameter;

    wprintf(L"STATUS:     Exit: %s (PID: %d)\n", s_counterMap[pid].counters->GetInstanceName(), pid);
    delete s_counterMap[pid].counters;
    s_counterMap[pid].counters = NULL;
    ::SetEvent(s_cleanupEvent);
    ::LeaveCriticalSection(&s_cs);
}

void UpdateInstances(bool printStatus = true)
{
    /* TODO: support only max 1024 processes? */
    DWORD rgProcessIds[1024];
    ULONG cbNeeded = 0;
    if (!EnumProcesses(rgProcessIds, sizeof(rgProcessIds), &cbNeeded))
    {
        // Why?
        wprintf(L"ERROR : Unable to enumerate process\n");
        return;
    }

    ::EnterCriticalSection(&s_cs);

    DWORD cProcesses = cbNeeded / sizeof(DWORD);
    size_t added = 0;
    for (size_t i = 0; i < cProcesses; i++)
    {
        if (s_counterMap.find(rgProcessIds[i]) != s_counterMap.end())
        {
            // Already created counter for the process
            continue;
        }

        Counters * counters = new Counters();
        if (counters == NULL)
        {
            wprintf(L"ERROR : Out of memory updating instances.\n");
            break;
        }

        if (!counters->Initialize(rgProcessIds[i]))
        {
            delete counters;
            continue;
        }

        HANDLE waitHandle;
        // Queue up a callback to clean when the process quits
        if (!RegisterWaitForSingleObject(&waitHandle, counters->GetProcessHandle(), 
            &OnProcessExitCallBack, (PVOID)rgProcessIds[i], INFINITE, WT_EXECUTEONLYONCE))
        {
            // Why?
            wprintf(L"ERROR : RegisterWaitForSingleObject failed.\n");
            delete counters;
            break;
        }

        CounterRecord record = { counters, waitHandle };
        s_counterMap[rgProcessIds[i]] = record;
        wprintf(L"STATUS:     Add: %s (PID: %d)\n", counters->GetInstanceName(), rgProcessIds[i]);
        added++;
    }
    if (printStatus)
    {
        wprintf(L"STATUS: Finish update instances. %d added. Total %d\n", added, s_counterMap.size());
    }
    else if (added != 0)
    {
        wprintf(L"STATUS: Scanning instance found %d added. Total %d\n", added, s_counterMap.size());
    }
    ::LeaveCriticalSection(&s_cs);
}

void CleanupInstances()
{
    ::EnterCriticalSection(&s_cs);

    // Clean up exited process entries
    for (auto i = s_counterMap.begin(); i != s_counterMap.end();)
    {
        if ((*i).second.counters == NULL)
        {
            /* Unregister the thread pool wait */
            UnregisterWait((*i).second.waitHandle);
            wprintf(L"STATUS:     Remove (PID: %d)\n", (*i).first);
            s_counterMap.erase(i++);

        }
        else
        {
            i++;
        }
    }
    ::ResetEvent(s_cleanupEvent);
    ::LeaveCriticalSection(&s_cs);
}

/* ===================================================================
 * Call back from perf lib when user is enumerating instance 
 * We will enumerate the processes in the system and find new 
 * AppContainer with jscript9 perf counter shared memory
 * to host the perf counters
 * ===================================================================*/
ULONG WINAPI
NotificationCallBack(ULONG RequestCode, PVOID Buffer, ULONG BufferSize)
{
    if (RequestCode != PERF_ENUM_INSTANCES)
    {
        return ERROR_SUCCESS;
    }

    UpdateInstances();
    return ERROR_SUCCESS;
}

unsigned int updateIntervalInMilliSeconds = INFINITE;

bool ParseCommandLine(int argc, wchar_t const ** argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (_wcsnicmp(argv[i], L"-updateinterval:", _countof( L"-updateinterval:") - 1) == 0)
        {
            wchar_t * endptr;
            updateIntervalInMilliSeconds = wcstol(argv[i] + _countof( L"-updateinterval:") - 1, &endptr, 10) * 1000;
            if ((size_t)(endptr - argv[i]) != wcslen(argv[i]))
            {
                wprintf(L"ERROR: Invalid number in -updateinterval '%s'\n", argv[i] + _countof( L"-updateinterval:") - 1);
                return false;
            }
            continue;
        }

        wprintf(L"ERROR: Invalid paramater '%s'\n", argv[i]);
        return false;
    }
    return true;
}

void PrintUsage()
{
    wprintf(L"WWAHostJSCounterProvider.exe <options>\n");
    wprintf(L"Options:\n");
    wprintf(L"    -updateinterval:<seconds> - time between updating the instances (default: infinite))n");
}

int __cdecl wmain(int argc, wchar_t const ** argv)
{
    if (!ParseCommandLine(argc, argv))
    {
        PrintUsage();
        return -1;
    }

    ::InitializeCriticalSection(&s_cs);
    s_cleanupEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    if (s_cleanupEvent == NULL)
    {
        wprintf(L"ERROR : Unable to create event\n");
        return -1;
    }

    PerfCounter::Provider::InternalCounter.SetNotificationCallBack(&NotificationCallBack);

    /* Notification is done on a separate thread. Use the main thread for clean up, 
       unless we are asked too look for instances periodically */
    while (true)
    {
        DWORD status = WaitForSingleObject(s_cleanupEvent, updateIntervalInMilliSeconds);
        if (status == WAIT_TIMEOUT)
        {
            UpdateInstances(false);
            continue;
        }
        if (status == WAIT_OBJECT_0)
        {
            CleanupInstances();
            continue;
        }
        break;
        
    }

    // No exit.  Rely on Ctrl-C or command line kill
}
