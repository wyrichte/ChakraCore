#include "stdafx.h"
#include "TestEtwEventSinkImpl.h"

TestEtwEventSink* TestEtwEventSinkImpl::Instance = NULL;

const std::wstring MethodEventNames::Load = L"EventWriteMethodLoad";
const std::wstring MethodEventNames::Unload = L"EventWriteMethodUnload";
const std::wstring MethodEventNames::DCStart = L"EventWriteMethodDCStart";
const std::wstring MethodEventNames::DCEnd = L"EventWriteMethodDCEnd";

const std::wstring SourceEventNames::Load = L"EventWriteSourceLoad";
const std::wstring SourceEventNames::Unload = L"EventWriteSourceUnload";
const std::wstring SourceEventNames::DCStart = L"EventWriteSourceDCStart";
const std::wstring SourceEventNames::DCEnd = L"EventWriteSourceDCEnd";

#define ETWEVENTSINK_TRACE0(sink, msg, ...) if (sink->IsTraceOn()) { wprintf(msg, __VA_ARGS__); }
#define ETWEVENTSINK_TRACE(msg, ...) ETWEVENTSINK_TRACE0(this, msg, __VA_ARGS__)

TestEtwEventSink* __stdcall CREATE_EVENTSINK_PROC_NAME(TestEtwEventSink::RundownFunc rundown, bool trace)
{
    return !rundown ? TestEtwEventSinkImpl::GetInstance(trace) : MockEtwController::GetInstance(rundown, trace);
}

void TestEtwEventSinkImpl::WriteMethodEvent(const wchar_t* eventName, 
        void* scriptContextId,
        void* methodStartAddress, 
        uint64 methodSize, 
        uint methodID, 
        uint16 methodFlags, 
        uint16 methodAddressRangeID, 
        DWORD_PTR sourceID, 
        uint line, 
        uint column, 
        const wchar_t* methodName)
{
    ETWEVENTSINK_TRACE(L"%s, 0x%p, 0x%p, %4I64u, %4u, %4hu, %4hu, %4Iu, %4u, %4u, %s\n", eventName, scriptContextId, methodStartAddress, 
            methodSize, methodID, methodFlags, methodAddressRangeID, sourceID, line, column, methodName);

    if(eventName == MethodEventNames::Load)
    {
        AutoCriticalSection(&this->csMethodMap);
        auto inserted = this->methodEventMap->insert(MethodEventMap::value_type(methodStartAddress, 
            new MethodInfo(scriptContextId, methodStartAddress, methodSize, methodID, methodFlags, methodAddressRangeID, sourceID, line, column, methodName)));
        IfFalsePrintAssertReturn(inserted.second, "Duplicate method load event for the same address:0x%p", methodStartAddress);
    }
    else if (eventName == MethodEventNames::Unload)
    {
        MethodInfo* methodInfo;
        {
            AutoCriticalSection(&this->csMethodMap);
            auto methodInfoIt = this->methodEventMap->find(methodStartAddress);
            IfFalsePrintAssertReturn(methodInfoIt != methodEventMap->end(), "Load event not found for method:%s", methodName);
            methodInfo = methodInfoIt->second;
            this->methodEventMap->erase(methodInfoIt);
        }
        IfFalseAssertReturn(methodInfo->scriptContextId == scriptContextId);
        IfFalseAssertReturn(methodInfo->methodStartAddress == methodStartAddress);
        IfFalseAssertReturn(methodInfo->methodSize == methodSize);
        IfFalseAssertReturn(methodInfo->methodID == methodID);
        IfFalseAssertReturn(methodInfo->methodFlags == methodFlags);
        IfFalseAssertReturn(methodInfo->methodAddressRangeID == methodAddressRangeID);
        IfFalseAssertReturn(methodInfo->sourceID == sourceID);
        IfFalseAssertReturn(methodInfo->line == line);
        IfFalseAssertReturn(methodInfo->column == column);
        IfFalseAssertReturn(methodInfo->name == methodName);
        delete methodInfo;
    }
    else if(eventName == MethodEventNames::DCStart || eventName == MethodEventNames::DCEnd)
    {
        // Do some validation
    }
    else
    {
        UnconditionallyFail(L"Unexpected method event type");
    }
}


void TestEtwEventSinkImpl::WriteSourceEvent(const wchar_t* eventName, uint64 sourceContext, void* scriptContextId, uint sourceFlags, const wchar_t* url )
{
    ETWEVENTSINK_TRACE(L"%s, %4I64u, 0x%p, %4u, %s\n", eventName, sourceContext, scriptContextId, sourceFlags, url);

    if(eventName == SourceEventNames::Load)
    {
        auto inserted = this->sourceEventMap->insert(SourceEventMap::value_type(sourceContext, 
            new SourceInfo(sourceContext, scriptContextId, sourceFlags, url)));
        IfFalsePrintAssertReturn(inserted.second, "Duplicate method load event for the same source cookie:0x%p", sourceContext);
    }
    else if (eventName == SourceEventNames::Unload)
    {
        SourceInfo* sourceInfo;
        auto it = this->sourceEventMap->find(sourceContext);
        IfFalsePrintAssertReturn(it != sourceEventMap->end(), "Load event not found for url:%s", url);
        sourceInfo = it->second;
        this->sourceEventMap->erase(it);
        
        IfFalseAssertReturn(sourceInfo->scriptContextId == scriptContextId);
        IfFalseAssertReturn(sourceInfo->sourceContext == sourceContext);
        IfFalseAssertReturn(sourceInfo->sourceFlags == sourceFlags);
        IfFalseAssertReturn(sourceInfo->url == url);

        delete sourceInfo;
    }
    else if(eventName == SourceEventNames::DCStart || eventName == SourceEventNames::DCEnd)
    {
        // Do some validation
    }
    else
    {
        UnconditionallyFail(L"Unexpected source event type");
    }
}


void TestEtwEventSinkImpl::UnloadInstance()
{
    VerifyEventMap(this->methodEventMap);
    VerifyEventMap(this->sourceEventMap);

    DeleteCriticalSection(&csMethodMap);

    if(m_fLastTestCaseFailed)
    {
        wprintf(L"\n ***IMPORTANT*** To debug test failures use -trace:etw\n");
    }

    assert(TestEtwEventSinkImpl::Instance == this);
    delete (TestEtwEventSinkImpl::Instance);
    TestEtwEventSinkImpl::Instance = NULL;
}

TestEtwEventSink* TestEtwEventSinkImpl::GetInstance(bool trace)
{
    assert(TestEtwEventSinkImpl::Instance == NULL);
    TestEtwEventSinkImpl::Instance = new TestEtwEventSinkImpl(trace);
    return TestEtwEventSinkImpl::Instance;
}

TestEtwEventSinkImpl::TestEtwEventSinkImpl(bool trace) : trace(trace), m_fLastTestCaseFailed(false)
{
    methodEventMap = new MethodEventMap();
    sourceEventMap = new SourceEventMap();
    InitializeCriticalSection(&csMethodMap);
}

//
// Global critical section to synchronize with etw thread, independent of MockEtwController life time.
//
CRITICAL_SECTION MockEtwController::s_csController;

//
// Global s_terminateRequestEvent to flag that the etw thread should terminate, independent of MockEtwController life time.
//
HANDLE MockEtwController::s_terminateRequestEvent = NULL;

MockEtwController::MockEtwController(RundownFunc rundown, bool trace)
    : TestEtwEventSinkImpl(trace),
    rundown(rundown), etwThread(NULL), etwThreadId(0), etwThreadStarted(false), terminatedEvent(NULL)
{
    InitializeCriticalSection(&s_csController);

    // Start etw thread upon construct
    StartEtwThread();
}

void MockEtwController::StartEtwThread()
{
    s_terminateRequestEvent = CreateEvent(NULL, /*bManualReset*/TRUE, /*bInitialState*/FALSE, NULL);
    if (!s_terminateRequestEvent)
    {
        UnconditionallyFail(L"Failed to create terminate request event");
    }

    terminatedEvent = CreateEvent(NULL, /*bManualReset*/TRUE, /*bInitialState*/FALSE, NULL);
    if (!terminatedEvent)
    {
        UnconditionallyFail(L"Failed to create terminated event");
    }

    etwThread = CreateThread(NULL, 0, &EtwThreadProc, this, 0, &etwThreadId);
    if (!etwThread)
    {
        UnconditionallyFail(L"Failed to create etw callback thread");
    }

    // NOTE: We can't deterministically wait for EtwThreadProc to start here. This function is called
    // through EtwTrace::Register(), which is called in process attach. In process attach Windows has
    // entered certain critical section which is also needed to actually start new thread.
}

void MockEtwController::StopEtwThread()
{
    if (!etwThread)
    {
        return; // unit test failed already
    }
    
    // Critical section scope
    {
        // This critical section synchronizes access to 2 things: (1) The terminate request, and (2) this->etwThreadStarted.
        // If we enter here first, we signal s_terminateRequestEvent, so EtwThreadProc won't run.
        // If EtwThreadProc enter first, this->etwThreadStarted will be true, and we'll wait for etw thread to shutdown.
        AutoCriticalSection autoControllerCS(&s_csController);

        // Always signal terminate request
        if (!SetEvent(s_terminateRequestEvent))
        {
            UnconditionallyFail(L"Failed to signal terminate request event");
        }

        // Only proceed if ETW thread really started. There is a small possibility that the thread
        // was queued to initialize and we request to terminate it here.
        if (!this->etwThreadStarted)
        {
            ETWEVENTSINK_TRACE(L"ETW callback thread is requested to terminate before actual start\n");
            return; // Done, we've signalled and the etw thread won't Run().
        }
    }

    // If ETW rundown thread was running alive, we should receive terminatedEvent signal then the thread dies.
    // However the thread could be dead already due to ExitProcess. In that case we won't get terminatedEvent signal.
    //
    // WARNING: ExitProcess may terminate ETW callback thread while it holds certain ETW rundown locks. If
    // subsequent DetachProcess procedure asks for the same locks, we will deadlock. If that happens, consider
    // invoking EtwTrace::Unregister() early in DllCanUnloadNow().
    //
    HANDLE handles[] = {terminatedEvent, etwThread};
    DWORD dwWaitResult = WaitForMultipleObjectsEx(_countof(handles), handles, /*bWaitAll*/FALSE, INFINITE, FALSE);

    switch (dwWaitResult)
    {
    case WAIT_OBJECT_0: // terminatedEvent signaled
        ETWEVENTSINK_TRACE(L"ETW callback thread terminated\n");
        break; // success

    case WAIT_OBJECT_0 + 1: // etw thread dead
        ETWEVENTSINK_TRACE(L"ETW callback thread was already terminated\n");
        break; // maybe terminated by ExitProcess

    default:
        wprintf(L"Wait for terminate result: %d, last error: %d\n", (int)dwWaitResult, (int)GetLastError());
        UnconditionallyFail(L"Failed to wait for terminate event\n");
        break; // failed
    }

    CloseHandle(terminatedEvent);
    CloseHandle(etwThread);
}

bool MockEtwController::IsEtwThread() const
{
    return GetCurrentThreadId() == etwThreadId;
}

//
// Simulates ETW thread callback
//
DWORD WINAPI MockEtwController::EtwThreadProc(_In_ LPVOID lpParameter)
{
    MockEtwController* controller = (MockEtwController*)lpParameter;

    // Critical section scope
    {
        // See comments in StopEtwThread() for this critical section.
        AutoCriticalSection autoControllerCS(&s_csController);

        // Check if requested to terminate already. If yes, we should just quit. "controller" could
        // have been destroyed already.
        //
        // NOTE: Use "0" timeout to test if the event was set. This doesn't block.
        //
        DWORD dwWaitResult = WaitForSingleObject(s_terminateRequestEvent, /*dwTimeout*/0);
        switch (dwWaitResult)
        {
        case WAIT_OBJECT_0:
            return 0; // Request to terminate, done

        case WAIT_TIMEOUT:
            break;  // Not signaled, we can proceed.

        default:
            wprintf(L"Failed to wait for terminate request. Wait result: %d, last error: %d\n", (int)dwWaitResult, (int)GetLastError());
            return (DWORD)(-1); // Failed
        }

        // Mark that etw thread is started. This prevents StopEtwThread() from destroying "controller" before we shutdown.
        controller->etwThreadStarted = true;
    }

    ETWEVENTSINK_TRACE0(controller, L"Etw callback thread started\n");
    DWORD dwResult = controller->Run();
    ETWEVENTSINK_TRACE0(controller, L"Etw callback thread completed: exit %d\n", dwResult);

    SetEvent(controller->terminatedEvent); // Signal terminated
    return dwResult;
}

DWORD MockEtwController::Run()
{
    const DWORD SLEEP_MILISECONDS = 300; // sleep between rundown

    bool start = false;
    for(;;)
    {
        start = !start; // start/stop means nothing for us, just toggle it
        rundown(start); // Invoke rundown callback

        DWORD dwWaitResult = WaitForSingleObject(s_terminateRequestEvent, SLEEP_MILISECONDS);
        switch (dwWaitResult)
        {
        case WAIT_OBJECT_0:
            return 0; // Done, request to terminate

        case WAIT_TIMEOUT:
            break;

        default:
            wprintf(L"Failed to wait for terminate request. Wait result: %d, last error: %d\n", (int)dwWaitResult, (int)GetLastError());
            return (DWORD)(-1); // Failed
        }
    }
}

void MockEtwController::WriteMethodEvent(const wchar_t* eventName, 
        void* scriptContextId,
        void* methodStartAddress, 
        uint64 methodSize, 
        uint methodID, 
        uint16 methodFlags, 
        uint16 methodAddressRangeID, 
        DWORD_PTR sourceID, 
        uint line, 
        uint column, 
        const wchar_t* methodName)
{
    // DC events should only be from etw thread. Other events (load/unload) should only be from non-etw thread.
    bool isDCEvent = (eventName == MethodEventNames::DCStart || eventName == MethodEventNames::DCEnd);
    IfFalseAssertReturn(IsEtwThread() == isDCEvent);

    // Call base sink implementation
    __super::WriteMethodEvent(eventName, scriptContextId, methodStartAddress, methodSize, methodID, methodFlags, methodAddressRangeID, sourceID, line, column, methodName);
}

void MockEtwController::WriteSourceEvent(const wchar_t* eventName, uint64 sourceContext, void* scriptContextId, uint sourceFlags, const wchar_t* url)
{
    // DC events should only be from etw thread. Other events (load/unload) should only be from non-etw thread.
    bool isDCEvent = (eventName == SourceEventNames::DCStart || eventName == SourceEventNames::DCEnd);
    IfFalseAssertReturn(IsEtwThread() == isDCEvent);

    // Call base sink implementation
    __super::WriteSourceEvent(eventName, sourceContext, scriptContextId, sourceFlags, url);
}

void MockEtwController::UnloadInstance()
{
    StopEtwThread();
    __super::UnloadInstance();
}

TestEtwEventSink* MockEtwController::GetInstance(RundownFunc rundown, bool trace)
{
    assert(TestEtwEventSinkImpl::Instance == NULL);
    TestEtwEventSinkImpl::Instance = new MockEtwController(rundown, trace);
    return TestEtwEventSinkImpl::Instance;
}
