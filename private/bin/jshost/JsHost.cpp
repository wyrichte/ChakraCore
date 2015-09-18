//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "proxystub.h"
#include "guids.h"
#include "DbgHelp.h"
#include "WScriptJsrt.h"
#include "Shellapi.h"
#ifdef EDIT_AND_CONTINUE
#include "ChakraInternalInterface.h"
#endif
#include "MemProtectHeap.h"
#include "TestLargeAddress.h"
#include "hostsysinfo.h"

HINSTANCE jscriptLibrary = NULL;
IGlobalInterfaceTable * git = NULL;
HANDLE shutdownEvent = NULL;
LPWSTR dbgBaselineFilename = NULL;
bool IsRunningUnderJdtest = 0;
LPCWSTR alternateDllName = NULL;

HINSTANCE nativeTestDll = NULL;
NativeTestEntryPoint pfNativeTestEntryPoint = NULL;
int nativeTestargc = 0;
LPWSTR* nativeTestargv = NULL;
HANDLE jsEtwConsoleEvent = nullptr;

///
/// JsRT test harness
///

JsValueRef attachFunction = JS_INVALID_REFERENCE;
JsRuntimeAttributes jsrtAttributes = JsRuntimeAttributeAllowScriptInterrupt;

LPCWSTR JsErrorCodeToString(JsErrorCode jsErrorCode)
{
    switch (jsErrorCode)
    {
    case JsNoError: // not really an error, but still worth stringifying properly
        return L"JsNoError";
        break;

    case JsErrorInvalidArgument:
        return L"JsErrorInvalidArgument";
        break;

    case JsErrorNullArgument:
        return L"JsErrorNullArgument";
        break;

    case JsErrorNoCurrentContext:
        return L"JsErrorNoCurrentContext";
        break;

    case JsErrorInExceptionState:
        return L"JsErrorInExceptionState";
        break;

    case JsErrorNotImplemented:
        return L"JsErrorNotImplemented";
        break;

    case JsErrorWrongThread:
        return L"JsErrorWrongThread";
        break;

    case JsErrorRuntimeInUse:
        return L"JsErrorRuntimeInUse";
        break;

    case JsErrorBadSerializedScript:
        return L"JsErrorBadSerializedScript";
        break;

    case JsErrorInDisabledState:
        return L"JsErrorInDisabledState";
        break;

    case JsErrorCannotDisableExecution:
        return L"JsErrorCannotDisableExecution";
        break;

    case JsErrorHeapEnumInProgress:
        return L"JsErrorHeapEnumInProgress";
        break;

    case JsErrorOutOfMemory:
        return L"JsErrorOutOfMemory";
        break;

    case JsErrorScriptException:
        return L"JsErrorScriptException";
        break;

    case JsErrorScriptCompile:
        return L"JsErrorScriptCompile";
        break;

    case JsErrorScriptTerminated:
        return L"JsErrorScriptTerminated";
        break;

    case JsErrorFatal:
        return L"JsErrorFatal";
        break;

    default:
        return L"<unknown>";
        break;
    }
}


size_t const TestBufferSize = 100;
size_t const TestObjectSize = 128;

HRESULT MemProtectHeapTestInit(void * heap, void *** pobjects, void *** psaved_ptr)
{
    void ** saved_ptr = (void **)malloc(sizeof(void *) * TestBufferSize);
    *psaved_ptr = saved_ptr;

    void ** objects = (void **)JScript9Interface::MemProtectHeapRootAlloc(heap, sizeof(void *) * TestBufferSize);
    if (objects == nullptr)
    {
        fprintf(stderr, "ERROR: MemProtectHeapRootAlloc failed: Out of Memory\n");
        return E_OUTOFMEMORY;
    }
    for (int i = 0; i < TestBufferSize; i++)
    {
        objects[i] = JScript9Interface::MemProtectHeapRootAlloc(heap, TestObjectSize);
        if (objects[i] == nullptr)
        {
            fprintf(stderr, "ERROR: MemProtectHeapRootAlloc failed: Out of Memory\n");
            return E_OUTOFMEMORY;
        }
        saved_ptr[i] = objects[i];
    }

    for (int i = 0; i < TestBufferSize; i += 2)
    {
        HRESULT hr = JScript9Interface::MemProtectHeapUnrootAndZero(heap, objects[i]);
        if (FAILED(hr))
        {
            fprintf(stderr, "ERROR: MemProtectHeapUnrootAndZero failed for %p HRESULT=%x\n", objects[i], hr);
            return hr;
        }
    }
    
    *pobjects = objects;
    return S_OK;
}

HRESULT MemProtectTestAlive(void * heap, void ** objects)
{
    // Everything should still be alive    
    for (int i = 0; i < TestBufferSize; i++)
    {
        size_t size;
        HRESULT hr = JScript9Interface::MemProtectHeapMemSize(heap, objects[i], &size);
        if (FAILED(hr))
        {
            fprintf(stderr, "ERROR: MemProtectHeapMemSize failed for %p HRESULT=%x\n", objects[i], hr);
            return hr;
        }

        if (size != TestObjectSize)
        {
            fprintf(stderr, "ERROR: MemProtectHeapMemSize get incorrect size for %p: got size %Iu\n", objects[i], size);            
            return E_FAIL;
        }
    }
    return S_OK;
}

void MemProtectTestFreeHalf(void * heap, void ** objects)
{
    for (int i = 0; i < TestBufferSize; i += 2)
    {
        objects[i] = nullptr;
    }
}

HRESULT MemProtectTestHalfAlive(void * heap, void ** objects, void **saved_ptr)
{
    // odd one should still be alive
    for (int i = 1; i < TestBufferSize; i += 2)
    {
        size_t size;
        HRESULT hr = JScript9Interface::MemProtectHeapMemSize(heap, objects[i], &size);
        if (FAILED(hr))
        {
            fprintf(stderr, "ERROR: MemProtectHeapMemSize failed for %p HRESULT=%x\n", objects[i], hr);
            return hr;
        }

        if (size != TestObjectSize)
        {
            fprintf(stderr, "ERROR: MemProtectHeapMemSize get incorrect size for %p: got size %Iu\n", objects[i], size);
            return E_FAIL;
        }
    }

    // even one should still be dead (if there are no false reference)
    for (int i = 0; i < TestBufferSize; i += 2)
    {
        size_t size;
        HRESULT hr = JScript9Interface::MemProtectHeapMemSize(heap, saved_ptr[i], &size);
        if (SUCCEEDED(hr))
        {
            fprintf(stderr, "ERROR: MemProtectHeapMemSize succeed for %p: get size %Iu\n", saved_ptr[i], size);
        }
    }
    return S_OK;
}

HRESULT MemProtectHeapTest()
{
    void * heap;
    HRESULT hr = JScript9Interface::MemProtectHeapCreate(&heap, MemProtectHeapCreateFlags_ProtectCurrentStack);
    if (FAILED(hr))
    {
        fprintf(stderr, "ERROR: MemProtectHeapCreate failed HRESULT=%x\n", hr);
        return hr;
    }

    void ** saved_ptr;
    void ** objects;
    hr = MemProtectHeapTestInit(heap, &objects, &saved_ptr);
    if (FAILED(hr))
    {
        goto Exit;
    }
       
    JScript9Interface::MemProtectHeapCollect(heap, MemProtectHeap_CollectForce);

    hr = MemProtectTestAlive(heap, objects);
    if (FAILED(hr))
    {
        goto Exit;
    }

    MemProtectTestFreeHalf(heap, objects);

    JScript9Interface::MemProtectHeapCollect(heap, MemProtectHeap_CollectForce);

    hr = MemProtectTestHalfAlive(heap, objects, saved_ptr);
Exit:
    JScript9Interface::MemProtectHeapDestroy(heap);
    free(saved_ptr);
    return hr;
}

#define IfJsrtErrorFail(expr) do { JsErrorCode jsErrorCode = expr; if ((jsErrorCode) != JsNoError) { fwprintf(stderr, L"ERROR: " TEXT(#expr) L" failed. JsErrorCode=0x%x (%s)\n", jsErrorCode, JsErrorCodeToString(jsErrorCode)); fflush(stderr); goto Error; } } while (0)

HRESULT DoOneJsrtIteration(BSTR filename)
{
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();

    MessageQueue *messageQueue = NULL;

    if (pfNativeTestEntryPoint)
    {
        fwprintf(stderr, L"Cannot use native tests with -jsrt flag.\n");
        fflush(stderr);
        return E_FAIL;
    }

    LPCWSTR fileContents = NULL;
    HRESULT hr = JsHostLoadScriptFromFile(filename, fileContents);
    if (FAILED(hr))
    {
        return hr;
    }

    JsRuntimeHandle runtime = JS_INVALID_RUNTIME_HANDLE;

    IfJsrtErrorFail(JScript9Interface::JsrtCreateRuntime(jsrtAttributes, NULL, &runtime));
    JsValueRef context = JS_INVALID_REFERENCE;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateContext(runtime, &context));
    IfJsrtErrorFail(JScript9Interface::JsrtSetCurrentContext(context));

    if (HostConfigFlags::flags.EnableDebug || HostConfigFlags::flags.DebugLaunch)
    {
        IfFailGo(CoInitializeEx(NULL, COINIT_MULTITHREADED));

        hr = diagnosticsHelper->InitializeDebugging(/*canSetBreakpoints*/true);
        if (hr != S_OK)
        {
            wprintf(L"[FAILED] to load the PDM\n");
            goto Error;
        }
        IfJsrtErrorFail(JScript9Interface::JsrtStartDebugging());
    }

    Assert(attachFunction == JS_INVALID_REFERENCE);    
    if (!WScriptJsrt::Initialize([] (JsValueRef function) 
        { 
        if (JsNoError != JScript9Interface::JsrtAddRef(function, NULL))
            {
                fwprintf(stderr, L"FATAL ERROR: JsAddRef failed\n");
                exit(-1);
            }
            attachFunction = function; 
        }))
    {
        IfFailGo(E_FAIL);
    }

    messageQueue = new MessageQueue();
    WScriptJsrt::AddMessageQueue(messageQueue);

    wchar_t fullPath[_MAX_PATH];

    if (_wfullpath(fullPath, filename, _MAX_PATH) == nullptr)
    {
        IfFailGo(E_FAIL);
    }

    // canonicalize that path name to lower case for the profile storage
    size_t len = wcslen(fullPath);
    for (size_t i = 0; i < len; i ++)
    {
        fullPath[i] = towlower(fullPath[i]);
    }

#ifdef FAULT_INJECTION
    // FaultInjection check point: engine initialized
    int faultInjection;
    if (JScript9Interface::GetFaultInjectionFlag(&faultInjection) == S_OK
        && faultInjection == 0)
    {
        fwprintf(stderr, L"FaultInjection - Checkpoint EngineLoaded Allocation Count:%d\n", JScript9Interface::GetCurrentFaultInjectionCount());
        fflush(stderr);
    }
#endif

    IfJsrtErrorFail(JScript9Interface::JsrtRunScript(fileContents, 0, fullPath, NULL));

    // Repeatedly flush the message queue until it's empty.  It is necessary to loop on this
    // because WScript.Attach may add additional callbacks to the queue.
    do {
        IfFailGo(messageQueue->ProcessAll());

        if (attachFunction != JS_INVALID_REFERENCE && diagnosticsHelper->m_debugger == nullptr)
        {
            if (!HostConfigFlags::flags.Targeted)
            {
                // Without this, the automatic setting breakpoint will not work.
                HostConfigFlags::flags.Auto = true;
            }

            // Time to attach the debugger to the host dynamically.
            HostConfigFlags::flags.DebugLaunch = SysAllocString(L"");

            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
            if (hr != S_OK)
            {
                goto Error;
            }

            hr = diagnosticsHelper->InitializeDebugging(/*canSetBreakpoints*/true);
            if (hr != S_OK)
            {
                wprintf(L"[FAILED] to load the PDM\n");
                goto Error;
            }

            IfJsrtErrorFail(JScript9Interface::JsrtStartDebugging());

            Assert(SUCCEEDED(hr));

            if (hr == S_OK)
            {
                // Now execute the function
                JsValueRef arguments[1];
                IfJsrtErrorFail(JScript9Interface::JsrtGetUndefinedValue(&arguments[0]));

                JsValueRef result;
                IfJsrtErrorFail(JScript9Interface::JsrtCallFunction(attachFunction, arguments, 1, &result));
            }
        }

    } while(!messageQueue->IsEmpty());

Error:
    if (attachFunction != JS_INVALID_REFERENCE)
    {
        JScript9Interface::JsrtRelease(attachFunction, NULL);
        attachFunction = JS_INVALID_REFERENCE;
    }
    JScript9Interface::JsrtSetCurrentContext(NULL);

    if(messageQueue != NULL)
    {
        delete messageQueue;
    }

    if (runtime != JS_INVALID_RUNTIME_HANDLE)
    {
        JScript9Interface::JsrtDisposeRuntime(runtime);
    }

    DiagnosticsHelper::DisposeHelper(true /*nullify the debug app*/);

    _flushall();
    
    return S_OK;
}

int _cdecl ExecuteJsrtTests(int argc, __in_ecount(argc) LPWSTR argv[])
{
    return ExecuteTests(argc, argv, DoOneJsrtIteration);
}

///
/// IActiveScript test harness
///

std::multimap<DWORD, JsHostActiveScriptSite*> hostThreadMap;
CRITICAL_SECTION hostThreadMapCs;

template <class Func>
void ForEachActiveHostThread(const Func& func)
{
    AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);
    std::for_each(hostThreadMap.begin(), hostThreadMap.end(), [&](const std::pair<DWORD, JsHostActiveScriptSite*>& item)
    {
        if (item.second) // nullptr if the script engine is already shutdown
        {
            func(item.second);
        }
    });
}

// Registers the factory for the IJsHostScriptSite proxy/stub in the current thread
HRESULT RegisterPSObject(DWORD *cookie)
{
    HRESULT hr;

    IUnknown *punk;
    hr = JsHostScriptSitePrxDllGetClassObject(IID_IJsHostScriptSite, IID_IUnknown, (void **)&punk);
    if(SUCCEEDED(hr))
    {
        hr = CoRegisterClassObject(IID_IJsHostScriptSite, punk, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, cookie);
        punk->Release();
    }

    return hr;
}

HRESULT UnregisterPSObject(DWORD cookie)
{
    return CoRevokeClassObject(cookie);
}

// Thread proc for the engine thread
DWORD WINAPI EngineThreadProcImpl(LPVOID param)
{
    HANDLE * waitHandles = (HANDLE*)param;
    HANDLE readyEvent = waitHandles[0];
    Assert(readyEvent);
    HANDLE terminateThreadEvent = waitHandles[1];

    HRESULT hr = S_OK;
  
    hr = CoInitializeEx(NULL, HostSystemInfo::SupportsOnlyMultiThreadedCOM() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);    
    if (FAILED(hr))
    {
        return hr;
    }

    // Register the proxy/stub for the current thread
    DWORD classFactoryCookie;
    hr = RegisterPSObject(&classFactoryCookie);
    if (SUCCEEDED(hr))
    {
        HANDLE waitFor[] = {shutdownEvent, terminateThreadEvent};
        int waitForCount = terminateThreadEvent ? 2 : 1;

        // Signal that the thread is ready
        if (!SetEvent(readyEvent))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        if (SUCCEEDED(hr))
        {
            BOOL continueRunning = TRUE;
            while(SUCCEEDED(hr) && continueRunning)
            {
                auto result = MsgWaitForMultipleObjectsEx(waitForCount, waitFor, INFINITE, QS_ALLEVENTS, MWMO_ALERTABLE);

                switch(result)
                {
                case WAIT_OBJECT_0:
                    continueRunning = FALSE;
                case WAIT_OBJECT_0 + 1:
                    if (waitForCount==2)
                    {
                        continueRunning = FALSE;
                    }
                    // Fall through
                case WAIT_OBJECT_0 + 2:
                    {
                        // Pump messages
                        MSG msg;
                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                    break;

                case WAIT_IO_COMPLETION:
                    break;

                case WAIT_FAILED:
                    hr = HRESULT_FROM_WIN32(GetLastError());
                default:
                    break;
                }
            }
        }
        UnregisterPSObject(classFactoryCookie);
    }

    // Force one final GC before we CoUninitialize
    JScript9Interface::FinalGC();

    CoUninitialize();

    return hr;
}

int JcExceptionFilter(int exceptionCode, _EXCEPTION_POINTERS *ep);
DWORD WINAPI EngineThreadProc(LPVOID param)
{
    __try
    {
        return EngineThreadProcImpl(param);
    }
    __except (JcExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
    {
        // Flush all I/O buffers
        _flushall();

        // Exception happened, so we probably didn't clean up properly, 
        // Don't exit normally, just terminate
        TerminateProcess(::GetCurrentProcess(), GetExceptionCode());
    }

    return (DWORD)-1;
}

// Creates an engine thread
HRESULT CreateEngineThread(HANDLE * thread, HANDLE * terminateThreadEvent)
{
    HRESULT hr = S_OK;

    HANDLE readyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!readyEvent)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HANDLE terminateHandle = 0;   
    if (terminateThreadEvent) 
    {
        *terminateThreadEvent = terminateHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!terminateHandle)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    HANDLE threadProcEvents[] = {readyEvent, terminateHandle};

    *thread = CreateThread(NULL, 0, EngineThreadProc, (LPVOID) threadProcEvents, 0, NULL);
    if (!thread)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    HANDLE waitHandles[] = {readyEvent, *thread};
    DWORD waitResult = WaitForMultipleObjectsEx(2, waitHandles, FALSE, INFINITE, false);
    switch(waitResult)
    {
    case WAIT_OBJECT_0:
        break;

    case WAIT_OBJECT_0 + 1:
        {
            if (!GetExitCodeThread(*thread, (LPDWORD)&hr))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

            if (SUCCEEDED(hr))
            {
                hr = E_FAIL;
            }
        }
        break;

    case WAIT_FAILED:
        hr = HRESULT_FROM_WIN32(GetLastError());
        break;
    }

    return hr;
}

// Creates a new javascript engine on the current thread
HRESULT CreateNewEngine(JsHostActiveScriptSite ** scriptSite, bool freeAtShutdown, bool actAsDiagnosticsHost, bool isPrimary, WORD domainId)
{
    HRESULT hr = JsHostActiveScriptSite::Create(scriptSite, actAsDiagnosticsHost, isPrimary, domainId);
    if (SUCCEEDED(hr) && freeAtShutdown)
    {
        AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);
        try
        {
            hostThreadMap.insert(std::pair<DWORD, JsHostActiveScriptSite*>(GetCurrentThreadId(), *scriptSite));
            (*scriptSite)->AddRef(); // for the reference in hostThreadMap
        }
        catch(const exception & e)
        {
            Unused(e); // Good to leave around for debugging
            hr = E_FAIL;
        }
    }

    return hr;
}

HRESULT ShutdownEngine(JsHostActiveScriptSite* scriptSite)
{
    HRESULT hr = JsHostActiveScriptSite::Shutdown(scriptSite);
    if (SUCCEEDED(hr))
    {
        AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);
        auto iter = std::find_if(hostThreadMap.begin(), hostThreadMap.end(), [=](const std::pair<DWORD, JsHostActiveScriptSite*>& item) -> bool {
            return item.second == scriptSite;
        });

        if (iter != hostThreadMap.end())
        {
            // Release the value reference but don't erase the map entry, as we need the threadId key to wait at final shutdown.
            iter->second->Release();
            iter->second = nullptr;
        }
    }

    return hr;
}

struct CreateEngineApcProcParam
{
    JsHostActiveScriptSite * scriptSite;
    bool freeAtShutdown;
    HANDLE completeEvent;
    WORD domainId;
    HRESULT hr;
};

// The APC proc for creating a javascript engine in another thread
VOID CALLBACK CreateEngineApcProc(ULONG_PTR param)
{
    CreateEngineApcProcParam * parameters = (CreateEngineApcProcParam*)param;

    parameters->hr = CreateNewEngine(&parameters->scriptSite, parameters->freeAtShutdown, HostConfigFlags::flags.DiagnosticsEngine /*actAsDiagnosticsHost*/, false /* APC proc so always on bg thread */, parameters->domainId);
   
    if (parameters->completeEvent)
    {
        SetEvent(parameters->completeEvent);
    }
}

// Creates a javascript engine in the specified thread
HRESULT CreateNewEngine(HANDLE thread, JsHostActiveScriptSite ** scriptSite, bool freeAtShutdown, bool actAsDiagnosticsHost, bool isPrimary, WORD domainId)
{
    HRESULT hr = S_OK;

    DWORD threadId = GetThreadId(thread);
    if (!threadId)
    {
        hr = E_FAIL;
    }
    if (SUCCEEDED(hr))
    {
        if (threadId == GetCurrentThreadId())
        {
            // If we're running in the correct thread create the engine directly
            hr = CreateNewEngine(scriptSite, freeAtShutdown, actAsDiagnosticsHost, isPrimary, domainId);
        }
        else
        {
            CreateEngineApcProcParam param = { 0 };
            param.freeAtShutdown = freeAtShutdown;
            param.domainId = domainId;
            param.hr = S_OK;
            // Create the event that will be signalled when the APC is done
            param.completeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (!param.completeEvent)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
            {
                // Queue an APC to the target thread to create the engine
                if (!QueueUserAPC(CreateEngineApcProc, thread, (ULONG_PTR)&param))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
                else
                {
                    switch (WaitForSingleObject(param.completeEvent, INFINITE))
                    {
                    case WAIT_OBJECT_0:
                        hr = param.hr;
                        break;

                    case WAIT_FAILED:
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        break;
                    }
                }

                CloseHandle(param.completeEvent);                
            }

            if (SUCCEEDED(hr))
            {
                *scriptSite = param.scriptSite;
            }
        }
    }

    return hr;
}

HRESULT __stdcall JsHostLoadScriptFile(void* jsHostScriptSite, LPCWSTR filename)
{
    return ((IJsHostScriptSite*)jsHostScriptSite)->LoadScriptFile(filename);
}

HRESULT __stdcall JsHostLoadScript(void* jsHostScriptSite, LPCWSTR script)
{
    return ((IJsHostScriptSite*)jsHostScriptSite)->LoadScript(script);
}

HRESULT __stdcall JsHostCreateNewEngine(IActiveScriptDirect** scriptDirect, void** jsHostScriptSiteOut, bool freeAtShutdown)
{
    JsHostActiveScriptSite* jsHostScriptSite = nullptr;
    HRESULT hr = CreateNewEngine(&jsHostScriptSite, freeAtShutdown, HostConfigFlags::flags.DiagnosticsEngine /*actAsDiagnosticsHost*/, true /* always primary */, 0);
    CComPtr<IActiveScript> activeScript;
    if (SUCCEEDED(hr))
    {
        hr = jsHostScriptSite->GetActiveScript(&activeScript);
    }
    if (SUCCEEDED(hr))
    {
        hr = activeScript->QueryInterface(__uuidof(IActiveScriptDirect), (LPVOID*)scriptDirect);
    }
    if (SUCCEEDED(hr))
    {
        *jsHostScriptSiteOut = (IJsHostScriptSite*)jsHostScriptSite;
    }
return hr;
}

HRESULT __stdcall JsHostShutdownScriptSite(void* jsHostScriptSite)
{
    // Need to cast it to IJsHostScriptSite first before JsHostActiveScriptSite so that we get the interface
    // adjustment right.
    JsHostActiveScriptSite * scriptSite = (JsHostActiveScriptSite*)(IJsHostScriptSite*)jsHostScriptSite;
    HRESULT hr = JsHostActiveScriptSite::Shutdown(scriptSite);
    if (SUCCEEDED(hr))
    {
        // Release this reference
        scriptSite->Release();
    }
    return hr;
}

HRESULT __stdcall JsHostCreateThreadService(IActiveScriptGarbageCollector** threadService)
{
    HRESULT hr;
    hr = JScript9Interface::GetThreadService(threadService);
    if (SUCCEEDED(hr))
    {
        return NOERROR;
    }
    hr = CoCreateInstance(CLSID_ChakraThreadService, NULL, CLSCTX_INPROC_SERVER, _uuidof(IActiveScriptGarbageCollector), (LPVOID*)&threadService);
    return hr;
}

void WaitForThreadsToFinish(std::set<HANDLE> threads)
{
    // Copy thread handles to std::vector from which we can get raw array of handles.
    std::vector<HANDLE> handles;
    for (std::set<HANDLE>::iterator iter = threads.begin(); iter != threads.end(); iter++)
    {
        HANDLE handle = *iter;
        handles.push_back(handle);
    }

    // Wait for all threads to finish, meanwhile pumping COM messages (but not all windows messages).
    // Note: on STA thread this will pump COM messages, on MTA thread it will not, but for MTA it's also OK, as we don't need to pump COM messages.
    // Note: COWAIT_WAITALL cannot be used as it does not process messages in this mode.
    while (true)
    {
        int handleCount = handles.size();
        if (handleCount == 0)
        {
            // Either all signaled, or the set was empty, anyhow, we are done.
            break;
        }

        HANDLE* handlesRaw = &handles[0]; // We can do that as std::vector is guaranteed to be contiguous.
        DWORD waitResult;
        HRESULT hr = CoWaitForMultipleHandles(0, INFINITE, handleCount, handlesRaw, &waitResult);
        if (!SUCCEEDED(hr) )
        {
            if (hr != RPC_S_CALLPENDING)
            {
                fwprintf(stderr, L"ERROR: CoWaitForMultipleHandles on engine threads failed. hr=0x%x, GLE=0x%x\n", hr, GetLastError());
                fflush(stderr);
            }
            break;
        }
        Assert(waitResult < WAIT_OBJECT_0 + handleCount);

        // Remove signaled handle.
        int signaledIndex = waitResult - WAIT_OBJECT_0;
        handles.erase(handles.begin() + signaledIndex);
    }
    for (unsigned int i = 0; i < handles.size(); i++)
    {
        CloseHandle(handles[i]);
    }
}

//
// Common debug operation shared by SourceRundown, DebugAttach/Detach, Start/StopProfiling:
//
//  Run SiteFunc on each scriptSite, then run the target function in the message.
//
template <class Message, class SiteFunc>
HRESULT PerformDebugOperation(Message& msg, PCWSTR operationName, const SiteFunc& siteFunc)
{
    HRESULT hr = S_OK;

    ForEachActiveHostThread([&](JsHostActiveScriptSite* scriptSite)
    {
        hr = siteFunc(scriptSite);
        if (FAILED(hr))
        {
            fwprintf(stderr, L"ERROR: %s failed, hr=0x%x\n", operationName, hr);
            fflush(stderr);
        }
    });

    // Call the script function in message.
    hr = msg.CallJavascriptFunction();

    return hr;
}

template <class Message, class DebugOperationFunc>
void QueueDebugOperation(typename Message::CustomArgType function, const DebugOperationFunc& operation)
{
    WScriptFastDom::PushMessage(Message::Create(function, operation));
}

template <class Message, class SiteFunc>
void QueueDefaultDebugOperation(typename Message::CustomArgType function, PCWSTR operationName, const SiteFunc& siteFunc)
{
    QueueDebugOperation<Message>(function, [=](Message& msg)
    {
        return PerformDebugOperation(msg, operationName, siteFunc);
    });
}

// Queue a source rundown message
void PerformSourceRundown()
{
    typedef WScriptFastDom::CallbackMessage Message;
    QueueDebugOperation<Message>(/*function*/nullptr, [=](Message& msg)
    {
        DiagnosticsHelper * diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
        if (!diagnosticsHelper->m_shouldPerformSourceRundown)
        {
            return S_OK;
        }

        PerformDebugOperation(msg, L"SourceRundown", [](JsHostActiveScriptSite* scriptSite)
        {
            return scriptSite->CheckPerformSourceRundown();
        });

        // Done with source rundown.
        diagnosticsHelper->m_shouldPerformSourceRundown = false;
        return S_OK;
    });
}

void CheckAutomaticSourceRundown()
{
    if (HostConfigFlags::flags.AutomaticSourceRundown)
    {
        // Perform source rundown if it's specified as a config flag.  This allows for
        // running existing UTs that utilize attach/detach without needing to explicitly
        // call WScript.PerformSourceRundown() in each test.
        DiagnosticsHelper::GetDiagnosticsHelper()->m_shouldPerformSourceRundown = true;

        PerformSourceRundown(); // Queue a source rundown message
    }
}

template <class Message, bool attach>
void DebugAttachOrDetach(typename Message::CustomArgType function)
{
    QueueDefaultDebugOperation<Message>(function, attach ? L"DebugAttach" : L"DebugDetach", [=](JsHostActiveScriptSite* scriptSite)
    {
        return attach ? scriptSite->CheckDynamicAttach() : scriptSite->CheckDynamicDetach();
    });
}

void FastDomDebugAttach(Var function)           { DebugAttachOrDetach<WScriptFastDom::CallbackMessage, true>(function); }
void FastDomDebugDetach(Var function)           { DebugAttachOrDetach<WScriptFastDom::CallbackMessage, false>(function); }
void DispatchDebugAttach(IDispatch* function)   { DebugAttachOrDetach<WScriptDispatchCallbackMessage, true>(function); }
void DispatchDebugDetach(IDispatch* function)   { DebugAttachOrDetach<WScriptDispatchCallbackMessage, false>(function); }

void FastDomEdit(Var function, IDebugDocumentText* debugDocumentText, ULONG startOffset, ULONG length, PCWSTR editContent, ULONG newLength)
{
    QueueDebugOperation<WScriptFastDom::CallbackMessage>(function, [=](WScriptFastDom::CallbackMessage& msg)
    {
        HRESULT hr;

#ifdef EDIT_AND_CONTINUE
        ScriptDirect scriptDirect;
        CComPtr<IActiveScriptEdit> pActiveScriptEdit;
        CComPtr<IScriptEditQuery> pScriptEditQuery;

        IfFailGo(scriptDirect.From(function));
        IfFailGo(scriptDirect->QueryInterface(&pActiveScriptEdit));

        {
            ScriptEditRequest request = { debugDocumentText, { startOffset, length }, editContent, newLength };

            // Query single edit
            IfFailGo(pActiveScriptEdit->QueryEdit(&request, 1, &pScriptEditQuery));
        }

        BOOL canApply;
        IfFailGo(pScriptEditQuery->CanApply(&canApply));
        if (canApply)
        {
            IfFailGo(pScriptEditQuery->CommitEdit());
        }
        else
        {
            wprintf(L"EnC: Cannot apply edit\n");
            ULONG resultCount;
            IfFailGo(pScriptEditQuery->GetResultCount(&resultCount));
            for (ULONG i = 0; i < resultCount; i++)
            {
                AutoScriptEditResult result;
                IfFailGo(pScriptEditQuery->GetResult(i, &result));
                wprintf(L"EnC: Compile error: %ls, line %d, column %d\n", result.message, result.line, result.column);
            }
            
            //TODO: even more info
        }
#endif

        // Call the script function in message.
        IfFailGo(msg.CallJavascriptFunction());

Error:
        return hr;
    });
}

template <class Message, bool start>
void StartOrStopProfiler(typename Message::CustomArgType function)
{
    QueueDebugOperation<Message>(function, [=](Message& msg)
    {
        return PerformDebugOperation(msg, start ? L"StartProfiler" : L"StopProfiler", [=](JsHostActiveScriptSite* scriptSite)
        {
            return start ? scriptSite->StartScriptProfiler() : scriptSite->StopScriptProfiler();
        });
    });
}

void FastDomStartProfiler(Var function)         { return StartOrStopProfiler<WScriptFastDom::CallbackMessage, true>(function); }
void FastDomStopProfiler(Var function)          { return StartOrStopProfiler<WScriptFastDom::CallbackMessage, false>(function); }
void DispatchStartProfiler(IDispatch* function) { return StartOrStopProfiler<WScriptDispatchCallbackMessage, true>(function); }
void DispatchStopProfiler(IDispatch* function)  { return StartOrStopProfiler<WScriptDispatchCallbackMessage, false>(function); }

HRESULT DoOneIASIteration(BSTR filename)
{
    HRESULT hr = S_OK;

    // Create the main engine thread
    HANDLE mainEngineThread = GetCurrentThread();

    MessageQueue *messageQueue = new MessageQueue();
    WScriptFastDom::AddMessageQueue(messageQueue);

    // Create the main javascript engine
    JsHostActiveScriptSite * mainScriptSite = NULL;
    hr = CreateNewEngine(mainEngineThread, &mainScriptSite, true, HostConfigFlags::flags.DiagnosticsEngine /*actAsDiagnosticsHost*/, true /* primary engine */, 0);
    if (SUCCEEDED(hr))
    {
#ifdef FAULT_INJECTION
        // FaultInjection check point: engine initialized
        int faultInjection;
        if (JScript9Interface::GetFaultInjectionFlag(&faultInjection) == S_OK
            && faultInjection == 0)
        {
            fwprintf(stderr, L"FaultInjection - Checkpoint EngineLoaded Allocation Count:%d\n", JScript9Interface::GetCurrentFaultInjectionCount());
            fflush(stderr);
        }
#endif

        CheckAutomaticSourceRundown(); // Enqueue automatic source rundown request early, before any requests made from script.

        WScriptFastDom::SetMainScriptSite(mainScriptSite);
        if (filename)
        {
            // Load the main script
            hr = mainScriptSite->LoadScriptFile(filename);            
        }
        else if (pfNativeTestEntryPoint)
        {
            // Call the native test entry point
            IActiveScript* activeScript = NULL;
            hr = mainScriptSite->GetActiveScript(&activeScript);
            if (FAILED(hr))
            {
                fwprintf(stderr, L"FATAL ERROR: Unable to load QI for IActiveScript. HR=0x%x\n", hr);
                fflush(stderr);
            }
            else
            {
                JsHostNativeTestArguments nativeTestArgs = 
                { 
                    activeScript, 
                    (IJsHostScriptSite*)mainScriptSite, 
                    jscriptLibrary, 
                    mainScriptSite->GetOnScriptErrorHelper(), 
                    JsHostLoadScriptFile, 
                    JsHostCreateNewEngine, 
                    JsHostShutdownScriptSite, 
                    JsHostCreateThreadService,
                    nativeTestargc, 
                    nativeTestargv
                };
                hr = pfNativeTestEntryPoint(&nativeTestArgs);
                activeScript->Release();
            }
        }
        else
        {
            fwprintf(stderr, L"ERROR: No file or native test entry point provided\n");
            fflush(stderr);
        }

        // Process all queued callbacks.
        messageQueue->ProcessAll();

        WScriptFastDom::ClearMainScriptSite();
        mainScriptSite->Release();
    }

    CloseHandle(mainEngineThread);
    
    delete messageQueue;

    std::set<HANDLE> threads;
    {
        AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);
        try
        {
            for (std::multimap<DWORD, JsHostActiveScriptSite*>::iterator iter = hostThreadMap.begin(); iter != hostThreadMap.end(); iter++)
            {
                if (iter->second) // nullptr if the script engine is already shutdown
                {
                    JsHostActiveScriptSite::Shutdown(iter->second);
                    iter->second->Release(); // release reference as we'll clear the map after iterating through it
                }
            }
        }
        catch (const exception & e)
        {
            Unused(e); // Good to leave around for debugging
        }

        try
        {
            for (std::multimap<DWORD, JsHostActiveScriptSite*>::iterator iter = hostThreadMap.begin(); iter != hostThreadMap.end(); iter++)
            {
                if (iter->first != GetCurrentThreadId())
                {
                    HANDLE thread = OpenThread(SYNCHRONIZE, FALSE, iter->first);
                    if (thread)
                    {
                        threads.insert(thread);
                    }
                }
            }
        }
        catch (const exception & e)
        {
            Unused(e); // Good to leave around for debugging
        }
        hostThreadMap.clear();
    }

    DiagnosticsHelper::DisposeHelper();

    if (SetEvent(shutdownEvent))
    {
        try
        {
            if (threads.size() > 0)
            {
                // The code was extracted into separate function to walk around what seems to be compiler/optimizer bug:
                // (optimized build only) in epilog stack gets mismatched by 4 bytes and wmain2's ret which is in EBX gets wrong value.
                // This causes NativeUnitTests to fail when run under rl.exe.
                WaitForThreadsToFinish(threads);
            }
        }
        catch(const exception & e)
        {
            Unused(e); // Good to leave around for debugging
        }
    }

    if (!ResetEvent(shutdownEvent))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    JsHostActiveScriptSite::Terminate();
    return hr;
}

int _cdecl ExecuteIASTests(int argc, __in_ecount(argc) LPWSTR argv[])
{
    int ret = 1;
    HRESULT hr = S_OK;
 
    hr = CoInitializeEx(NULL, HostSystemInfo::SupportsOnlyMultiThreadedCOM() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);   
    if (FAILED(hr))
    {
        wprintf(L"FATAL ERROR: CoInitializeEx() failed. hr=0x%x\n", hr);
        exit(1);
    }

    // Register the proxy/stub object for IJsHostScriptSite in the main thread
    DWORD classFactoryCookie;
    hr = RegisterPSObject(&classFactoryCookie);
    if (FAILED(hr))
    {
        wprintf(L"FATAL ERROR: CoRegisterClassObject() failed. hr=0x%x\n", hr);
        exit(1);
    }

    // Associate the proxy/stub object with the interface
    hr = CoRegisterPSClsid(IID_IJsHostScriptSite, IID_IJsHostScriptSite);
    if (FAILED(hr))
    {
        wprintf(L"FATAL ERROR: CoRegisterPSClsid() failed. hr=0x%x\n", hr);
        exit(1);
    }

    // Initialize GIT
    hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, __uuidof(IGlobalInterfaceTable), (void **)&git);
    if (FAILED(hr))
    {
        wprintf(L"FATAL ERROR: CoCreateInstance(CLSID_StdGlobalInterfaceTable) failed. hr=0x%x\n", hr);
        exit(1);
    }

    // Create the shutdown event
    shutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!shutdownEvent)
    {
        ret = GetLastError();
        fwprintf(stderr, L"FATAL ERROR: CreateEvent(shutdownEvent) failed. GLE=0x%x\n", ret);
        fflush(stderr);
        return ret;
    }

    InitializeCriticalSection(&hostThreadMapCs);

    ret = ExecuteTests(argc, argv, DoOneIASIteration);

    UnregisterPSObject(classFactoryCookie);

    // Force one final GC before we CoUninitialize
    JScript9Interface::FinalGC();
    
    if (HostConfigFlags::flags.DumpRecyclerStats)
    {
        PROCESS_MEMORY_COUNTERS_EX memCounters;

        memCounters.cb=sizeof(memCounters);
        GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS*)&memCounters,memCounters.cb);
        wprintf(L"Peak working set %Iu\n",memCounters.PeakWorkingSetSize);
        wprintf(L"Working set      %Iu\n",memCounters.WorkingSetSize);
        wprintf(L"Private memory   %Iu\n",memCounters.PrivateUsage);

        JScript9Interface::DisplayRecyclerStats();
    }

    if (UTF8BoundaryTestBuffer != nullptr)
    {
        VirtualFree(UTF8BoundaryTestBuffer, 0, MEM_RELEASE);
    }
    if (UTF8SourceMapper != nullptr)
    {
        AssertMsg (UTF8SourceMapper->GetRefCount() == 0, "Not all the references were released of the IActiveScriptByteCodeSource passed in to ExecuteByteCodeBuffer");
        delete UTF8SourceMapper;
        UTF8SourceMapper = nullptr;
    }

#ifdef ENABLE_INTL_OBJECT
    JScript9Interface::ClearTimeZoneCalendars();
#endif

    CoUninitialize();

    return ret;
}

#include "..\..\..\core\lib\common\exceptions\reporterror.h"
LPTOP_LEVEL_EXCEPTION_FILTER originalUnhandledExceptionFilter;
LONG WINAPI JsHostUnhandledExceptionFilter(LPEXCEPTION_POINTERS lpep)
{
    DWORD exceptionCode = lpep->ExceptionRecord->ExceptionCode;
    if ((exceptionCode == E_OUTOFMEMORY || exceptionCode == E_UNEXPECTED) &&
        (lpep->ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE) &&
        lpep->ExceptionRecord->NumberParameters == 2)
    {
        JScript9Interface::NotifyUnhandledException(lpep);

        bool crashOnException = false;
        JScript9Interface::GetCrashOnExceptionFlag(&crashOnException);

        if (!crashOnException)
        {

            ErrorReason reasonCode = (ErrorReason)lpep->ExceptionRecord->ExceptionInformation[0];
            wchar_t const * reason = L"Unknown Reason";
            switch (reasonCode)
            {
            case JavascriptDispatch_OUTOFMEMORY:
                reason = L"JavascriptDispatch out of memory";
                break;
            case Fatal_Internal_Error:
                reason = L"internal error";
                break;
            case Fatal_Debug_Heap_OUTOFMEMORY:
                reason = L"debug heap out of memory";
                break;
            case Fatal_Amd64StackWalkerOutOfContexts:
                reason = L"amd64 stack walker out of contexts";
                break;
            case Fatal_Binary_Inconsistency:
                reason = L"binary inconsistency";
                break;
            case WriteBarrier_OUTOFMEMORY:
                reason = L"write Barrier out of memory";
                break;
            case CustomHeap_MEMORYCORRUPTION:
                reason = L"customHeap memory corruption";
                break;
            case LargeHeapBlock_Metadata_Corrupt:
                reason = L"large heap block metadata corruption";
                break;
            case Fatal_Version_Inconsistency:
                reason = L"version inconsistency";
                break;
            case MarkStack_OUTOFMEMORY:
                reason = L"mark stack out of memory";
                break;
            };
            fwprintf(stderr, L"NON-CONTINUABLE FATAL ERROR: jshost.exe failed due to exception code %x, %s (%d)\n", exceptionCode, reason, reasonCode);
            _flushall();

            TerminateProcess(::GetCurrentProcess(), exceptionCode);
        }
    }
    return originalUnhandledExceptionFilter(lpep);
}

void SetupUnhandledExceptionFilter()
{
    originalUnhandledExceptionFilter = SetUnhandledExceptionFilter(JsHostUnhandledExceptionFilter);
}

///
/// JsHost main methods
///

// The SEH exception filter
int JcExceptionFilter(int exceptionCode, _EXCEPTION_POINTERS *ep)
{
    JScript9Interface::NotifyUnhandledException(ep);

    bool crashOnException = false;
    JScript9Interface::GetCrashOnExceptionFlag(&crashOnException);

    if (exceptionCode == EXCEPTION_BREAKPOINT || (crashOnException && exceptionCode != 0xE06D7363))
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    fwprintf(stderr, L"FATAL ERROR: jshost.exe failed due to exception code %x\n", exceptionCode);
    fflush(stderr);

    return EXCEPTION_EXECUTE_HANDLER;
}

void LaunchAndAttachDebugger()
{
    WCHAR buf[64];
    WCHAR cmdLineBuf[1024] = { 0 };
    std::wstring eventName = L"jdtest";
    std::wstring cmdLine;

    // event name: jdtest1234
    _itow_s(GetCurrentProcessId(), buf, 10);
    eventName += buf;

    // command line: ... -event:jdtest1234
    cmdLine = L"jdtest.exe ";
    cmdLine += HostConfigFlags::jdtestCmdLine;
    cmdLine += L" -event:";
    cmdLine += eventName;
    wcscpy_s(cmdLineBuf, cmdLine.c_str());

    HANDLE hDebuggerEvt;
    hDebuggerEvt = CreateEvent(NULL, TRUE, FALSE, eventName.c_str());

    STARTUPINFO startupInfo = {0};
    PROCESS_INFORMATION procInfo = {0};
    startupInfo.cb = sizeof(STARTUPINFO);
    if(CreateProcess(NULL, cmdLineBuf, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &procInfo) == FALSE)
    {
        wprintf(L"ERROR: failed to attach debugger\n");
        exit(1);
    }

    CloseHandle(procInfo.hThread);
    CloseHandle(procInfo.hProcess);

    if(WaitForSingleObject(hDebuggerEvt, INFINITE) != WAIT_OBJECT_0)
    {
        wprintf(L"ERROR: failed to attach debugger\n");
        exit(1);
    }

    if(!IsDebuggerPresent())
    {
        wprintf(L"ERROR: failed to attach debugger\n");
        exit(1);
    }

    // out of proc debugger now attached
}

// Launch JsEtwConsole, wait for it to begin processing ETW events, then return.
// This method opens JsEtwConsole via ShellExecute which will attempt to elevate if needed.
// We wait for JsEtwConsole to indicate readiness via a named event returned from CreateEvent.
void LaunchEtwListenerAndWaitForReady()
{
    WCHAR buf[MAX_PATH];
    WCHAR cmdLineBuf[1024] = {0};
    std::wstring cmdLine;
    std::wstring etwBaseFilename = L"JsEtwConsole";
    std::wstring autoExitFilename;

    // etwBaseFilename is the name of the event we will pass to JsEtwConsole
    _itow_s(GetCurrentProcessId(), buf, 10);
    etwBaseFilename += L".";
    etwBaseFilename += buf;

    // autoExitFilename is the name of a file which JsEtwConsole will watch for and exit when it exists.
    // This is how we will signal for JsEtwConsole to quit.
    autoExitFilename = L"";
    autoExitFilename += etwBaseFilename;
    autoExitFilename += L".autoexit";

    // Build command line used to execute JsEtwConsole.exe
    //   -noetwcleanup: we don't want to clean-up any in-progress ETW sessions from other instances
    //   -filenonverbose: this removes timestamps, guids, etc from the file output so it can be used as baseline
    //   -processid: used to indicate that JsEtwConsole.exe should only listen for events from this process id
    //   -file: this is the file which JsEtwConsole.exe should write events into (we'll parse it later)
    //   -exitwhenexists: this is the file which we will create to indicate JsEtwConsole.exe should exit
    //   -eventname: name of the event which JsEtwConsole should set when it have finished startup/shutdown
    //   Also append any additional flags specified via the -JsEtwConsole switch.
    cmdLine = L" -noetwcleanup -filenonverbose -processid ";
    cmdLine += buf;
    cmdLine += L" ";
    cmdLine += HostConfigFlags::jsEtwConsoleCmdLine;
    cmdLine += L" -file "; 
    cmdLine += etwBaseFilename;
    cmdLine += L".etwlog -eventname ";
    cmdLine += etwBaseFilename;
    cmdLine += L" -exitwhenexists ";
    cmdLine += autoExitFilename;

    wcscpy_s(cmdLineBuf, cmdLine.c_str());
    wcscpy_s(buf, autoExitFilename.c_str());

    // Delete the auto-exit file if it already exists
    DeleteFile(buf);

    // Create an event which JsEtwConsole should use to communicate with us
    jsEtwConsoleEvent = CreateEvent(NULL, TRUE, FALSE, etwBaseFilename.c_str());
    if (jsEtwConsoleEvent == NULL)
    {
        DWORD hr = GetLastError();
        wprintf(L"ERROR: failed to create event for JsEtwConsole with hr=%X\n", hr);
        exit(1);
    }

    // Construct the full path for JsEtwConsole, assume it is next to JsHost.exe
    wchar_t modulePath[_MAX_PATH];
    if (GetModuleFileName(NULL, modulePath, _MAX_PATH) == _MAX_PATH)
    {
        wprintf(L"ERROR: failed to module path with gle=%X\n", GetLastError());
        exit(1);
    }
    std::wstring JsEtwConsoleExe = modulePath;
    wchar_t wszDrive[_MAX_DRIVE];
    wchar_t wszDir[_MAX_DIR];
    errno_t err;
    if ((err = _wsplitpath_s(modulePath, wszDrive, _MAX_DRIVE, wszDir, _MAX_DIR, NULL, 0, NULL, 0)) != 0)
    {
        wprintf(L"ERROR: failed to split module path errno=%X\n", err);
        exit(1);
    }

    if ((err = _wmakepath_s(modulePath, wszDrive, wszDir, L"JsEtwConsole", L".exe")) != 0)
    {
        wprintf(L"ERROR: failed to make module path errno=%X\n", err);
        exit(1);
    }

    // Launch JsEtwConsole and elevate if needed (but run as minimized)
    int success = (int)ShellExecute(NULL, L"runas", modulePath, cmdLineBuf, NULL, SW_SHOWMINNOACTIVE);
    if (success <= 32)
    {
        wprintf(L"ERROR: failed to launch JsEtwConsole with code=%d\n", success);
        exit(1);
    }
    
    // Wait for JsEtwConsole to set the event indicating it has finished starting up
    if (WaitForSingleObject(jsEtwConsoleEvent, INFINITE) != WAIT_OBJECT_0)
    {
        wprintf(L"ERROR: failed to launch JsEtwConsole (failed to receive signal from JsEtwConsole)\n");
        exit(1);
    }

    // Reset the event so JsEtwConsole can set it again when it has finished shutting down
    ResetEvent(jsEtwConsoleEvent);
}

// Tell JsEtwConsole to exit and wait for it to finish shutting down
void StopEtwListener()
{
    WCHAR buf[MAX_PATH];
    std::wstring autoExitFilename = L"JsEtwConsole";

    _itow_s(GetCurrentProcessId(), buf, 10);
    autoExitFilename += L".";
    autoExitFilename += buf;
    autoExitFilename += L".autoexit";

    wcscpy_s(buf, autoExitFilename.c_str());

    // We indicate to JsEtwConsole that it should exit by writing a file
    FILE* f = _wfopen(buf, L"w");
    fclose(f);

    // Wait for JsEtwConsole to set the event indicating it has finished shutting down
    if (WaitForSingleObject(jsEtwConsoleEvent, INFINITE) != WAIT_OBJECT_0)
    {
        wprintf(L"ERROR: failed to stop JsEtwConsole\n");
        exit(1);
    }

    // Delete the auto-exit file to avoid polluting the unittest folder
    DeleteFile(buf);
}

// Read the ETW log file created by JsEtwConsole.exe and write it via printf
void WriteEtwLog()
{
    bool success = false;
    WCHAR buf[2048];
    std::wstring etwLogFilename = L"JsEtwConsole";

    _itow_s(GetCurrentProcessId(), buf, 10);
    etwLogFilename += L".";
    etwLogFilename += buf;
    etwLogFilename += L".etwlog";
    
    wcscpy_s(buf, etwLogFilename.c_str());

    FILE* f = _wfopen(buf, L"r");
    if (f == nullptr)
    {
        wprintf(L"ERROR: failed to open etw log file\n");
        exit(1);
    }

    char* utf8buf = nullptr;
    WCHAR* utf16buf = nullptr;
    const long MAX_FILE_SIZE = 1024 * 1024 * 1;
    fseek(f , 0, SEEK_END);
    long fileSize = ftell(f);
    rewind(f);

    if (fileSize <= MAX_FILE_SIZE)
    {
        utf8buf = (char*) malloc(sizeof(char) * fileSize);

        if (utf8buf == nullptr)
        {
            wprintf(L"ERROR: failed to allocate buffer to read etw log file\n");
            goto LReturn;
        }

        size_t bytesRead = fread(utf8buf, sizeof(char), fileSize, f);

        utf8buf[bytesRead] = '\0';

        utf16buf = (WCHAR*) malloc(sizeof(WCHAR) * fileSize);

        if (utf16buf == nullptr)
        {
            wprintf(L"ERROR: failed to allocate buffer to read etw log file\n");
            goto LReturn;
        }

        size_t returnValue;
        errno_t result = mbstowcs_s(&returnValue, utf16buf, fileSize, utf8buf, fileSize);

        if (result != 0)
        {
            wprintf(L"ERROR: failed to convert character buffer from utf8 to utf16\n");
            goto LReturn;
        }

        wprintf(L"ETW Log Begin\n");
        wprintf(L"=============================================================\n");
        wprintf(utf16buf);
        wprintf(L"=============================================================\n");
        wprintf(L"ETW Log End\n");
    }
    else
    {
        wprintf(L"ERROR: etw log file is too large\n");
        goto LReturn;
    }

    success = true;

LReturn:

    // Delete the ETW log file when we are done with it
    DeleteFile(buf);

    if (utf8buf != nullptr)
    {
        free(utf8buf);
    }

    if (utf16buf != nullptr)
    {
        free(utf16buf);
    }

    if (f != nullptr)
    {
        fclose(f);
    }

    if (success == false)
    {
        exit(1);
    }
}

int ExecuteTests(int argc, __in_ecount(argc) LPWSTR argv[], DoOneIterationPtr pfDoOneIteration, bool useChakra /*= false*/)
{
    int ret = 0;
    HRESULT hr = S_OK;
    BSTR filename = NULL;
    JScript9Interface::ArgInfo argInfo = { argc, argv, ::PrintUsage, pfNativeTestEntryPoint ? NULL : &filename };        // Call the real entrypoint.

    // Spin-up the ETW console listener, if needed.
    if (HostConfigFlags::jsEtwConsoleCmdLine != nullptr)
    {
        LaunchEtwListenerAndWaitForReady();
    }

    // Use an alternate dll name, if requested on the command line.  This is required for running multiple Magellan based code coverage
    // instances in the same user session.  Magellan does not distinguish between two processes writing code coverage data to the same
    // dll's database, so simultaneous yet distinct traces require different dll names to be used.
    jscriptLibrary = JScript9Interface::LoadDll(useChakra, alternateDllName, argInfo);

    if (useChakra && !JScript9Interface::SupportsNotifyOnScriptStateChanged())
    {
        fwprintf(stderr, L"FATAL ERROR: Loaded chakra dll engine isn't the test version\n");
        return E_FAIL;
    }

    if (HostConfigFlags::flags.GenerateValidPointersMapHeaderIsEnabled)
    {
        return JScript9Interface::GenerateValidPointersMapHeader(HostConfigFlags::flags.GenerateValidPointersMapHeader);
    }

    if (HostConfigFlags::flags.MemProtectHeapTest)
    {
        return MemProtectHeapTest();        
    }

    // Check if running in the out of proc debugger was requested
    if (HostConfigFlags::jdtestCmdLine != nullptr)
    {
        if (HostConfigFlags::flags.DebugLaunch)
        {
            SysFreeString(HostConfigFlags::flags.DebugLaunch);
        }
        HostConfigFlags::flags.DebugLaunch = SysAllocString(L"hybrid");
        LaunchAndAttachDebugger();
    }

#ifdef CHECK_MEMORY_LEAK
    // Always check memory leak in jshost.exe, unless user specfied the flag already
    if (!JScript9Interface::IsEnabledCheckMemoryFlag())
    {
        JScript9Interface::SetCheckMemoryLeakFlag(true);
    }

    // BUGBUG: disable memory leak check for now till I figure out how to 
    // clean up edgehtml side's memprotectheap leak.
    if (useChakra)
    {
        JScript9Interface::SetCheckMemoryLeakFlag(false);
    }

    // Disable the output in case an unhandled exception happens
    // We will reenable it if there is no unhandled exceptions
    JScript9Interface::SetEnableCheckMemoryLeakOutput(false);
#endif
#ifdef DBG
    // Always enable this in console CHK builds
    JScript9Interface::SetCheckOpHelpersFlag(true);
#endif

    __try
    {
        int loopCount;
        hr = JScript9Interface::GetLoopFlag(&loopCount);
        if (SUCCEEDED(hr))
        {
            for (int i = 0; i < loopCount; ++i)
            {
                // Each iteration will reuse the same thread context/recycler
                hr = pfDoOneIteration(filename);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }
        else if (hr == E_NOTIMPL)
        {
            if (argc > 1 && _waccess(argv[1], 0) != -1) {
                hr = pfDoOneIteration(argv[1]);
            }
            else {
                wprintf(L"Jshost accept only filename as parameter when running with free chakra.dll\n");
            }
        }
    }
    __except(JcExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
    {
        // Flush all I/O buffers
        _flushall();

        // Exception happened, so we probably didn't clean up properly, 
        // Don't exit normally, just terminate
        TerminateProcess(::GetCurrentProcess(), GetExceptionCode());
    }

    if (argInfo.filename)
    {
        SysFreeString(*argInfo.filename);
    }

    if (FAILED(hr))
    {
        ret = hr;
    }

    // Flush all I/O buffers
    _flushall();


#ifdef CHECK_MEMORY_LEAK
    // Only check memory leak output if there is no unhandled exceptions
    //if (!HostConfigFlags::flags.DebugLaunch)  // Uncomment this to disable memory check for -debuglaunch (if we see random memory leak again)
    {
        JScript9Interface::SetEnableCheckMemoryLeakOutput(true);
    }
#endif

    if (HostConfigFlags::jsEtwConsoleCmdLine != nullptr)
    {
        // wait a little to flush things
        Sleep(500);

        StopEtwListener();
        WriteEtwLog();
    }

    return ret;
}

void __stdcall PrintUsage()
{
    wprintf(L"\n\nUsage: jshost.exe [-ls] [-jsrt[:JsRuntimeAttributes]] [-html] [flaglist] [filename]|[-nativetest:testdll [nativetestargs]]\n");
    HostConfigFlags::PrintUsageString();
    if (JScript9Interface::SupportsPrintConfigFlagsUsageString())
    {
        JScript9Interface::PrintConfigFlagsUsageString();
    }
}

int HandleNativeTestFlag(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[])
{
    LPCWSTR nativeTestFlag = L"-nativetest:";
    size_t nativeTestFlagLen = wcslen(nativeTestFlag);
    int i = HostConfigFlags::FindArg(argc, argv, nativeTestFlag, nativeTestFlagLen);
    if (i < 0)
    {
        return S_OK;
    }

    // we have the a -nativetest flag
    if (wcslen(argv[i]) == nativeTestFlagLen)
    {
        return E_FAIL;
    }
    LPWSTR nativeDllName = argv[i] + nativeTestFlagLen;
    nativeTestDll = LoadLibraryEx(nativeDllName, nullptr, 0);
    int ret = 0;
    if (! nativeTestDll)
    {
        ret = GetLastError();
        fwprintf(stderr, L"FATAL ERROR: Unable to load %s. GLE=0x%x\n", nativeDllName, ret);
        fflush(stderr);
        return ret;
    }
    pfNativeTestEntryPoint = (NativeTestEntryPoint)GetProcAddress(nativeTestDll, nativeTestEntryPointName);
    if (!pfNativeTestEntryPoint)
    {
        ret = GetLastError();
        fwprintf(stderr, L"FATAL ERROR: Unable to get address of %S in %s. GLE=0x%x\n", nativeTestEntryPointName, nativeDllName, ret);
        fflush(stderr);
        return ret;
    }

    nativeTestargc = argc - (i + 1);
    if (nativeTestargc > 0)
    {
        nativeTestargv = argv + i + 1;
    }
    // lop off all the flags from -nativetest and beyond
    argc = i;
    return 0;
}

int HandleDebuggerBaselineFlag(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[])
{
    LPCWSTR dbgBaselineFlag = L"-dbgbaseline:";
    LPCWSTR dbgBaselineFlagWithoutColon = L"-dbgbaseline";
    int dbgBaselineFlagLen = wcslen(dbgBaselineFlag);

    int i = 0;
    for(i = 1; i < argc; ++i)
    {
        if(!_wcsicmp(argv[i], dbgBaselineFlagWithoutColon)) 
        {
            dbgBaselineFilename = L"";
            break;
        } 
        else if(!_wcsnicmp(argv[i], dbgBaselineFlag, dbgBaselineFlagLen))
        {
            dbgBaselineFilename = argv[i] + dbgBaselineFlagLen;
            if(wcslen(dbgBaselineFilename) == 0)
            {
                fwprintf(stdout, L"[FAILED]: must pass a filename to -dbgbaseline:\n");
                return 1;
            }
            else
            {
                break;
            }
        }
    }

    if(i == argc)
        return 1;

    // remove this flag now
    HostConfigFlags::RemoveArg(argc, argv, i);

    return 0;
}

bool HandleJsrtTestFlag(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[])
{
    LPCWSTR jsrtTestFlag = L"-jsrt";    
    size_t jsrtTestFlagLen = wcslen(jsrtTestFlag);
    
    int i = HostConfigFlags::FindArg(argc, argv, jsrtTestFlag, jsrtTestFlagLen);
    if (i < 0)
    {
        return false;
    }
    
    // we have the a -jsrt flag
    if (wcslen(argv[i]) != jsrtTestFlagLen)
    {
        LPWSTR jsrtAttributeSupplied = argv[i] + jsrtTestFlagLen + 1;
        if (! swscanf_s(jsrtAttributeSupplied, L"%x", &jsrtAttributes))
        {
            return false;
        }
    }
    // remove the -jsrt flag
    HostConfigFlags::RemoveArg(argc, argv, i);
    
    return true;
}

bool HandleHtmlTestFlag(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[])
{
    bool isHtmlTest = false; // If the test is like http://..., https://..., *.htm, *.html

    // Search for explict -html switch (check http://..., https://..., *.htm, *.html along the way)
    int htmlSwitchIndex = HostConfigFlags::FindArg(argc, argv, [&](PCWSTR arg) -> bool
    {
        if (!isHtmlTest)
        {
            PCWCHAR ext = wcsrchr(arg, L'.');
            isHtmlTest = _wcsnicmp(arg, L"http://", 7) == 0
                || _wcsnicmp(arg, L"https://", 8) == 0
                || (ext && _wcsicmp(ext, L".htm") == 0)
                || (ext && _wcsicmp(ext, L".html") == 0);
        }

        // Even if isHtmlTest, we still need to finish searching for "-html" because we need to remove it.
        return _wcsicmp(arg, L"-html") == 0;
    });

    if (htmlSwitchIndex >= 0)
    {
        HostConfigFlags::RemoveArg(argc, argv, htmlSwitchIndex); // Consume -html
    }

    return htmlSwitchIndex >= 0 || isHtmlTest;
}

bool HandleAlternateDllFlag(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[])
{
    alternateDllName = HostConfigFlags::ExtractSwitch(argc, argv, L"-alternateDll:");
    return alternateDllName != nullptr;
}

void PeekRuntimeFlag(int argc, _In_reads_(argc) LPWSTR argv[])
{
    if (HostConfigFlags::FindArg(argc, argv, L"-EditTest") >= 0)
    {
        WScriptFastDom::EnableEditTests();
    }
}

int _cdecl wmain1(int argc, __in_ecount(argc) LPWSTR argv[])
{
    HandleDebuggerBaselineFlag(argc, argv);
    bool useJsrt = HandleJsrtTestFlag(argc, argv);
    bool useHtml = HandleHtmlTestFlag(argc, argv);
    HandleAlternateDllFlag(argc, argv);
    
    PeekRuntimeFlag(argc, argv);

    if (argc < 2)
    {
        PrintUsage();
        return EXIT_FAILURE;
    }

    HostConfigFlags::HandleArgsFlag(argc, argv);
    HostConfigFlags::HandleJdTestFlag(argc, argv);
    HostConfigFlags::HandleJsEtwConsoleFlag(argc, argv);

    if (int ret = HandleNativeTestFlag(argc, argv) != 0)
    {
        return ret;
    }

    if (HostConfigFlags::flags.Auto && !HostConfigFlags::flags.DebugLaunch)
    {
        wprintf(L"ERROR: -auto must be used with -debuglaunch\n");
        return EXIT_FAILURE;
    }

    int ret = 0;

    if (useJsrt)
    {
        ret = ExecuteJsrtTests(argc, argv);
    }
    else if (useHtml)
    {
        ret = ExecuteHtmlTests(argc, argv);
    }
    else
    {
        if (HostConfigFlags::FindArg(argc, argv, L"-WERExceptionSupport", wcslen(L"WERExceptionSupport")) >= 0)
        {
            // Projection test, use the old comment style
            HostConfigFlags::flags.EnableExtendedErrorMessages = false;
        }
        else
        {
            // otherwise use the new comment format
            HostConfigFlags::flags.EnableExtendedErrorMessages = true;
            HostConfigFlags::AddSwitch(argc, argv, L"-WERExceptionSupport");
        }
        ret = ExecuteIASTests(argc, argv);
    }

    // We need to unload the jscriptLibrary after CoUnitialize, so that DllCanUnloadNow
    // is called after all the COM stuff is released.
    if (jscriptLibrary)
    {
        JScript9Interface::UnloadDll(jscriptLibrary);
    }

    // Free up the memmory used because of extended err msg
    if (HostConfigFlags::flags.EnableExtendedErrorMessages)
    {
        delete[] argv;
    }

    return ret;
}

int _cdecl wmain(int argc, __in_ecount(argc) LPWSTR argv[])
{
    SetupUnhandledExceptionFilter();

    HostConfigFlags::pfnPrintUsage = PrintUsage;
    
    char *jdtestVar = getenv("JDTEST");
    if(jdtestVar && !strcmp(jdtestVar, "1"))
    {
        IsRunningUnderJdtest = true;
    }
    ATOM lock = ::AddAtom(szChakraLock);
    AssertMsg(lock, "failed to lock dll");
    int ret = 0;
    if (UseLargeAddresses(argc, argv))
    {
         ret = TestLargeAddress(argc, argv, wmain1);
    }
    else
    {
        ret = wmain1(argc, argv);
    }

    return ret;
}
