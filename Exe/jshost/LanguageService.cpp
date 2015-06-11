/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "ieisos.h"

#ifdef LANGUAGE_SERVICE_TEST

bool stress = false;
bool perf = false;
bool verbose = false;
wchar_t *fileName = NULL;


HRESULT ProcessArguments(int argc, __in_ecount(argc) LPWSTR argv[])
{
    for(int i = 1; i < argc; ++i)
    {
        if(!_wcsicmp(argv[i], L"-stress"))
        {
            stress = true;
        }
        else if (!_wcsicmp(argv[i], L"-perf"))
        {
            perf = true;
        }
        else if(!_wcsicmp(argv[i], L"-verbose"))
        {
            verbose = true;
        }
        else if(argv[i][0] == L'-' || argv[i][0] == L'/')
        {
            // ignore switches for Unit tests
            wchar_t *IgnoreSwitches[] = {
                L"DumpOnCrash",
                L"MaxInterpretCount",
                L"MaxSimpleJitRunCount",
                L"BGJit",
                L"dynamicProfileCache"
            };
            bool ignore = false;
            for(int j = 0; j < _countof(IgnoreSwitches) && !ignore; ++j)
            {
                if(!_wcsnicmp(argv[i]+1, IgnoreSwitches[j], wcslen(IgnoreSwitches[j])))
                    ignore = true;
            }
        }
        else
        {
            if(fileName != NULL)
            {
                printf("Error: only one file is supported.\n");
                return E_FAIL;
            }
            else
            {
                fileName = argv[i];
            }
        }
    }

    if(fileName == NULL)
    {
        printf("Error: no file specified.\n");
        return E_FAIL;
    }

    return S_OK;
}


int ExecuteLSTests(int argc, __in_ecount(argc) LPWSTR argv[])
{
    HRESULT hr;

    std::wstring jslspath = FindJScript9LS();
    if (jslspath.size() == 0)
    {
        printf("Can't find chakralstest.dll\n");
        return 1;
    }

    JScript9Interface::ArgInfo argInfo = { argc, argv, ::PrintUsage, NULL  };        // Call the real entrypoint.
    JScript9Interface::SetArgInfo(argInfo);
    LanguageServiceTestDriver testDriver(jslspath.c_str());
    IfFailGo(testDriver.InitializeHost());

    IfFailGo(ProcessArguments(argc, argv));

    if(verbose)
    {
        testDriver.SetVerbose();
    }
       
    IfFailGo(testDriver.AddFile(fileName));


    if (stress)
    {
        IfFailGo(testDriver.RunStress());
    }
    else if (perf)
    {
        IfFailGo(testDriver.RunPerf());
    }
    else 
    {
        IfFailGo(testDriver.ExecuteTests());
    }
    

Error:
    if (!SUCCEEDED(hr))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



CAuthoringHost::CAuthoringHost()
: refCount(1),
_message(this, GetCurrentThreadId()),
_timer(NULL),
_callback(NULL)
{
    _threadId = GetCurrentThreadId();
}

CAuthoringHost::~CAuthoringHost()
{
}

STDMETHODIMP CAuthoringHost::QueryInterface(REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (riid == _uuidof(IUnknown))
    {
        *ppvObject = static_cast<IUnknown*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG CAuthoringHost::AddRef()
{
    return InterlockedIncrement(&refCount);
}

ULONG CAuthoringHost::Release()
{
    long currentCount = InterlockedDecrement(&refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

static CAuthoringHost *gHost = NULL;

VOID CALLBACK CAuthoringHost::TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (gHost)
    {
        gHost->Timer();
    }
}

void CAuthoringHost::Timer()
{
    //printf("Calling timer!\n");
    void *tmpCallback = _callback;
    void *tmpData = _callbackData;
    if (tmpCallback && tmpData)
    {
        KillTimer(NULL, _timer);
        void(*callback)(void *) = (void(*)(void*))tmpCallback;
        void *data = tmpData;
        callback(data);
        _timer = SetTimer(NULL, NULL, (DWORD)HostConfigFlags::flags.Hurry, TimerCallback);
    }
}

HRESULT CAuthoringHost::SetCallbackTimer(DWORD dwTime)
{
    Message<CAuthoringHost>::FunctionCallResult result;
    if (S_OK == _message.AsyncCall(&CAuthoringHost::SetCallbackTimer, dwTime, &result))
    {
        // We need to make this asynchronous so that the runtime can do some work by that time.
        return S_OK;
    }
    gHost = this;
    _timer = SetTimer(NULL, NULL, dwTime, TimerCallback);
    return S_OK;
}

DWORD __stdcall CAuthoringHost::TimerLoop(LPVOID arg)
{
    CoInitializeEx(NULL, IsOs_OneCoreUAP() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED); 

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_QUIT)
        {
            break;
        }
        if (Message<CAuthoringHost>::TryDispatch(msg))
        {
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    CoUninitialize();
    return 0;
}

void CAuthoringHost::CallAfter(DWORD dwTime, void(*callback)(void *), void *data)
{
    if (_callback == NULL) // supporting on hurry at a time.
    {
        _callback = callback;
        _callbackData = data;
        SetCallbackTimer(dwTime);
    }
}

void CAuthoringHost::KillCallback()
{
    _callback = NULL;
    if (_timer)
    {
        KillTimer(NULL, _timer);
    }
    _timer = NULL;
}

unsigned int CAuthoringHost::HostLoop(void* args)
{
    HRESULT hr = S_OK;
    CAuthoringHost* host;
    HostLoopArgs* loopArgs = (HostLoopArgs*)args;

    CoInitializeEx(NULL, IsOs_OneCoreUAP() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED); 

    host = new CAuthoringHost();

    IfFailGo(host->Initialize(loopArgs->jscriptPath));

    // TODO this is a hack for getting commands from the main thread
    // to the scriptHost thread.
    // I need to consider how to make this affordable for expanding the command set
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_QUIT)
        {
            break;
        }
        if (Message<CAuthoringHost>::TryDispatch(msg))
        {
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

Error:
    CoUninitialize();
    //_endthreadex(0);
    return 0;
}

HRESULT CAuthoringHost::Initialize(LPCWSTR targetdll)
{
    if (_threadId != GetCurrentThreadId())
    {
        AssertMsg(TRUE, "Called host initialize on the wrong thread");
        return E_FAIL;
    }

    hThread = CreateThread(NULL, 0, &CAuthoringHost::TimerLoop, this, 0, (DWORD*)&_threadId);
    _message.SetThreadId(_threadId);

    HRESULT hr = S_OK;

    const CLSID CLSID_JScript9Ls = { 0xf13098a9, 0xcec8, 0x471e, 0x8e, 0x43, 0xd0, 0xbd, 0x93, 0x12, 0x62, 0x3 };
    IfFailGo(PrivateCoCreate(targetdll, CLSID_JScript9Ls, NULL, CLSCTX_INPROC_SERVER, _uuidof(IAuthorServices), (LPVOID*)&_authoringServices));
Error:
    return hr;
}

std::wstring FindJScript9LS()
{
    // Attempt to find a loadable version of chakralstest.dll.
    //   1. Check the local path
    //   2. Check %_NTTREE%
    WCHAR buf[_MAX_PATH];
    WCHAR drive[_MAX_PATH];
    WCHAR dir[_MAX_PATH];
    WCHAR fname[_MAX_PATH];
    WCHAR ext[_MAX_PATH];

    GetModuleFileName(NULL, buf, _MAX_PATH);
    _wsplitpath_s(buf, drive, dir, fname, ext);

    std::wstring jslsdir;
    jslsdir += drive;
    jslsdir += dir;
    jslsdir += L"\\chakralstest.dll";

    if (GetFileAttributes(jslsdir.c_str()) != INVALID_FILE_ATTRIBUTES)
        return jslsdir;

    // Check %_NTTREE%
    if (GetEnvironmentVariable(L"_NTTREE", buf, _MAX_PATH) == 0)
        return L"";

    jslsdir = buf;
    jslsdir += L"\\chakralstest.dll";

    if (GetFileAttributes(jslsdir.c_str()) != INVALID_FILE_ATTRIBUTES)
        return jslsdir;

    return L"";
}

#endif // LANGUAGE_SERVICE_TEST