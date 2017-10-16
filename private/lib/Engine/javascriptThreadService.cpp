//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"
#include "share.h"
/// 
/// Idle Task that is used to do finish a GC on idle
/// This is scheduled by the timer-based Idle GC callback when the idle callback
/// is called and a collection is in progress. The idle task can finish the GC
/// by safely skipping the stack scan during FinishMark
///
class RecyclerFinishConcurrentIdleTask sealed : public IIdleTask
{
public:
    RecyclerFinishConcurrentIdleTask(ThreadContext* threadContext, JavascriptThreadService* service);

    // *** IUnknown ***
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // *** IActiveScriptGarbageCollector ***
    STDMETHOD_(void, RunIdleTask)() override;

    void OnThreadServiceDestroyed(JavascriptThreadService* threadService);

#if DBG
    void OnTaskScheduled() { this->isIdleTaskComplete = false; }
#endif

private:
    ulong refCount;
    ThreadContext* threadContext;
    JavascriptThreadService* threadService;
#if DBG
    bool isIdleTaskComplete;
#endif
};


//
// JavascriptThreadService
//

bool JavascriptThreadService::wasInitialize = false;

JavascriptThreadService::JavascriptThreadService() :
    ThreadServiceWrapperBase(),
    refCount(0),
    rootTracker(NULL),
    timerProvider(NULL),
    idleTaskHost(NULL),
    finishConcurrentTask(NULL),
    idleTaskState(IdleTaskState::NotScheduled),
#ifdef RECYCLER_TRACE
    hasTimerScheduled(false),
#endif
#ifdef DBG
    isIdleTaskScheduled(false),
#endif
    externalTelemetryBlock(NULL),
    telemetryBlock(&localTelemetryBlock)
{
    Assert(wasInitialize == false);
    dwBaseThread = GetCurrentThreadId();
    wasInitialize = true;
    DLLAddRef(); // One DLL reference for each existing script engine

    memset(&localTelemetryBlock, 0, sizeof(localTelemetryBlock));
}

JavascriptThreadService::~JavascriptThreadService()
{
    if (GetThreadContext() != nullptr)
    {
        GetThreadContext()->SetThreadServiceWrapper(nullptr);
    }

    if (timerProvider)
    {
        Shutdown();
        timerProvider->Release();
        timerProvider = NULL;
    }

    if (idleTaskHost)
    {
        // ShutdownIdleTask();
        idleTaskHost->Release();
        idleTaskHost = NULL;
    }

    if (rootTracker)
    {
        rootTracker->Release();
        rootTracker = NULL;
    }

    if (finishConcurrentTask)
    {
        finishConcurrentTask->OnThreadServiceDestroyed(this);
        finishConcurrentTask->Release();
        finishConcurrentTask = NULL;
    }

    wasInitialize = false;
    DLLRelease();
}

BOOL JavascriptThreadService::Initialize(void)
{
    HRESULT hr = S_OK;

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        // Ensure thread context so the page allocator is initialized
        bool returnValue = ThreadServiceWrapperBase::Initialize(ThreadBoundThreadContextManager::EnsureContextForCurrentThread());
        if (returnValue) 
        {
            GetThreadContext()->EnsureRecycler();
            Assert(GetThreadContext()->GetRecycler() != nullptr);
        }
        return returnValue;
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr == S_OK;
}

HRESULT JavascriptThreadService::OnFinishMarkTaskComplete(void)
{
    Assert(this->idleTaskHost != nullptr);
    Assert(this->finishConcurrentTask != nullptr);
    Assert(this->isIdleTaskScheduled == true); // We reuse the timer flag to indicate that the idle task has been scheduled

    this->SetIdleTaskState(IdleTaskState::Completed);
    this->ClearForceOneIdleCollection();
    this->FinishIdleCollect(FinishReasonTaskComplete);

    return S_OK;
}

// *** IUnknown Methods ***
STDMETHODIMP JavascriptThreadService::QueryInterface(
    /* [in]  */ REFIID riid,
    /* [out] */ void **ppvObj)
{
    VALIDATE_WRITE_POINTER(ppvObj, void *);
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = this;
        AddRef();
        return NOERROR;
    }
    QI_IMPL(__uuidof(IJavascriptThreadService), IJavascriptThreadService);
    QI_IMPL(__uuidof(IActiveScriptGarbageCollector), IActiveScriptGarbageCollector);
    QI_IMPL(__uuidof(IActiveScriptDirectGarbageCollector), IActiveScriptDirectGarbageCollector);
    QI_IMPL(__uuidof(IJavascriptThreadProperty), IJavascriptThreadProperty);
    QI_IMPL(__uuidof(IActiveScriptLifecycleEventSink), IActiveScriptLifecycleEventSink);
    QI_IMPL(IID_ITrackingService, ITrackingService);
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) JavascriptThreadService::AddRef(void)
{
    return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) JavascriptThreadService::Release(void)
{
    long lw = InterlockedDecrement(&refCount);
    if (0 == lw)
    {
        delete this;
    }
    return lw;
}

//============================================================================================================
// IActiveScriptDirectGarbageCollector Implementation
//============================================================================================================
STDMETHODIMP JavascriptThreadService::CollectGarbage(SCRIPTGCTYPE scriptgctype)
{
    HRESULT hr;

    // Pad CoCreates a bare script engine to call CollectGarbage to avoid stack pinning. Rather than write another
    // GC collection pass which ignores stack pins, which would sometimes be wrong, we instead simply call CollectGarbage
    // on this temporary engine. Since we don't call SetScriptSite our base thread is NOBASETHREAD. Allow this call to go
    // through since we'll follow up with getting the current thread context and won't use our JavascriptThreadService.
    if (dwBaseThread != NOBASETHREAD)
    {
        IFFAILRET(ValidateBaseThread());
    }

    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    // This can be called from an engine that hasn't called SetScriptSite yet. 
    if (!threadContext->IsStackAvailableNoThrow())
    {
        return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
    }
    Recycler* recycler = threadContext->GetRecycler();
    return JavascriptThreadService::CollectGarbage(recycler, scriptgctype);
}

HRESULT JavascriptThreadService::CollectGarbage(Recycler* recycler, SCRIPTGCTYPE scriptgctype)
{
    if (recycler)
    {
#ifdef RECYCLER_TRACE
        recycler->SetDomCollect(true);
#endif
        switch((DWORD)scriptgctype)
        {
        case SCRIPTGCTYPE_CONCURRENT:
            recycler->CollectNow<CollectNowConcurrent>();
            break;
        case SCRIPTGCTYPE_PARTIAL:
            recycler->CollectNow<CollectNowPartial>();
            break;
        case SCRIPTGCTYPE_CONCURRENTPARTIAL:
            recycler->CollectNow<CollectNowConcurrentPartial>();
            break;
        case SCRIPTGCTYPE_NORMAL:
            recycler->CollectNow<CollectNowDefault>();
            break;
        case SCRIPTGCTYPE_EXHAUSTIVE:
            recycler->CollectNow<CollectNowExhaustive>();
            break;
        case SCRIPTGCTYPE_FORCEINTHREAD:
            recycler->CollectNow<CollectNowForceInThreadExternal>();
            break;
        case SCRIPTGCTYPE_FORCEINTHREAD_NOSTACK:
            {
                Recycler::AutoEnterExternalStackSkippingGCMode autoGC(recycler);
                recycler->CollectNow<CollectNowForceInThreadExternalNoStack>();
            }
            break;
        case SCRIPTGCTYPE_CONCURRENT_FINISHONLY:
            recycler->FinishConcurrent<FinishConcurrentDefault>();
            break;
        }
#ifdef RECYCLER_TRACE
        recycler->SetDomCollect(false);
#endif

        recycler->FinishDisposeObjectsNow<FinishDispose>();
    }

    return S_OK;
}


STDMETHODIMP JavascriptThreadService::AddCallBack(CollectGarbageCallBack gccallback, void * context, CollectGarbageCallBackHandle* callBackHandle)
{
    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        *callBackHandle = ThreadContext::GetContextForCurrentThread()->AddRecyclerCollectCallBack((RecyclerCollectCallBackFunction)gccallback, context);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

STDMETHODIMP JavascriptThreadService::RemoveCallBack(CollectGarbageCallBackHandle callBackHandle)
{
    ThreadContext::GetContextForCurrentThread()->RemoveRecyclerCollectCallBack((ThreadContext::CollectCallBack *)callBackHandle);
    return S_OK;
}

//============================================================================================================
// ITrackingService Implementation
//============================================================================================================
STDMETHODIMP JavascriptThreadService::RegisterTrackingClient(ITracker * pTracker)
{
    // This is call at thread creation, and UnregisterTrackingClient is called at
    // thread termination. I don't see much point implementing the TrackerClientRootList.
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    pTracker->AddRef();
    SetRootTracker(pTracker);
    threadContext->GetRecycler()->SetExternalRootMarker(JavascriptThreadService::RootTrackerMarker, threadContext);
    return S_OK;
}

STDMETHODIMP JavascriptThreadService::UnregisterTrackingClient(ITracker * pTracker)
{
    // See comment above.
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    pTracker->Release();
    SetRootTracker(NULL);

    threadContext->GetRecycler()->SetExternalRootMarker(NULL, NULL);
    return S_OK;
}

STDMETHODIMP JavascriptThreadService::EnumerateTrackingClient(void * pv, IUnknown * punk, BOOL fTracker)
{
    HRESULT hr = NOERROR;
    ITracker * pTracker = NULL;
    HostVariant* hostVariant;
    Recycler* recycler = (Recycler*)pv;
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    if (threadContext->GetRecycler() != recycler)
    {
        return E_INVALIDARG;
    }

#if DBG
    recycler->CheckAllocExternalMark();
#endif

    if (fTracker)
    {
        pTracker = (ITracker*)punk;
        VARIANT* varDispatch;
        IfFailedReturn(pTracker->GetTrackingAlias(&varDispatch));
        if (varDispatch != nullptr)
        {
            hostVariant = CONTAINING_RECORD(varDispatch, HostVariant, varDispatch);
            Assert(hostVariant->isTracked);
            recycler->TryMarkNonInterior(hostVariant);
            return NOERROR;
        }
        return S_FALSE;
    }

    recycler->TryMarkInterior(punk);
    return NOERROR;
}

STDMETHODIMP JavascriptThreadService::IsTrackedObject(IUnknown * pUnk, IDispatchEx **ppDispTracked, BOOL *pfTracker)
{
    AssertNotInScript();
    HRESULT hr = NOERROR;

    ITracker* tracker;
    JavascriptDispatch* javascriptDispatch;
    IJavascriptDispatchLocalProxy* jsProxy;

    if (pUnk)
    {
        if (SUCCEEDED(pUnk->QueryInterface(IID_ITrackerJS9, (void**)&tracker)))
        {
            VARIANT* varDispatch;
            hr = tracker->GetTrackingAlias(&varDispatch);
            Assert(SUCCEEDED(hr));
            if (varDispatch != NULL)
            {
                HostVariant* hostVariant = CONTAINING_RECORD(varDispatch, HostVariant, varDispatch);
                *ppDispTracked = (IDispatchEx*)hostVariant->GetDispatchNoRef();
                *pfTracker = TRUE;
            }
            else
            {
                Recycler * recycler = GetThreadContext()->GetRecycler();
                BEGIN_TRANSLATE_OOM_TO_HRESULT
                {
                    HostVariant * hostVariant = NULL;
                    IDispatch* dispatch;
                    if (SUCCEEDED(pUnk->QueryInterface(__uuidof(IDispatch), (void**)&dispatch)))
                    {
                        hostVariant = RecyclerNewTrackedLeaf(recycler, HostVariant, dispatch);
                        hostVariant->SetupTracker(tracker);
                        dispatch->Release();
                    }
                    else
                    {
                        hostVariant = RecyclerNewTrackedLeaf(recycler, HostVariant, dispatch);
                    }
                    *ppDispTracked = (IDispatchEx*)hostVariant->GetDispatchNoRef();
                    *pfTracker = TRUE;
                }
                END_TRANSLATE_OOM_TO_HRESULT(hr);
            }
            tracker->Release();
            return hr;
        }
        else if (SUCCEEDED(pUnk->QueryInterface(__uuidof(IJavascriptDispatchLocalProxy), (void**)&jsProxy)))
        {
            javascriptDispatch = static_cast<JavascriptDispatch*>(jsProxy);
            javascriptDispatch->SetAsGCTracked();
            hr = javascriptDispatch->QueryInterface(__uuidof(IDispatchEx), (void**)ppDispTracked);
            jsProxy->Release();
            (*ppDispTracked)->Release();
            *pfTracker = FALSE;
            return S_OK;
        }
    }
    return E_INVALIDARG;
}

// Root tracker enumeration
void __cdecl JavascriptThreadService::RootTrackerMarker(void * context)
{
    ThreadContext* threadContext = (ThreadContext *)context;
    ((JavascriptThreadService *)threadContext->GetThreadServiceWrapper())->RootTrackerMarker();
}

void JavascriptThreadService::RootTrackerMarker()
{
    Recycler* recycler = GetThreadContext()->GetRecycler();
    ITracker* tracker = GetRootTracker();
    if (tracker != NULL)
    {
        tracker->EnumerateTrackedObjects(recycler);
    }
}

void JavascriptThreadService::SetRootTracker(ITracker* inRootTracker)
{
    Assert(inRootTracker == NULL || rootTracker == inRootTracker || rootTracker == NULL);
    rootTracker = inRootTracker;
}

ITracker* JavascriptThreadService::GetRootTracker()
{
    Assert(rootTracker != NULL);
    return rootTracker;
}

//============================================================================================================
// RecyclerFinishConcurrentIdleTask Implementation
//============================================================================================================

// *** IUnknown Methods ***
STDMETHODIMP RecyclerFinishConcurrentIdleTask::QueryInterface(
    /* [in]  */ REFIID riid,
    /* [out] */ void **ppvObj)
{
    VALIDATE_WRITE_POINTER(ppvObj, void *);

    QI_IMPL(IID_IUnknown, IUnknown);
    QI_IMPL(__uuidof(IIdleTask), IIdleTask);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) RecyclerFinishConcurrentIdleTask::AddRef(void)
{
    return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) RecyclerFinishConcurrentIdleTask::Release(void)
{
    long lw = InterlockedDecrement(&refCount);
    if (0 == lw)
    {
        delete this;
    }
    return lw;
}

void RecyclerFinishConcurrentIdleTask::OnThreadServiceDestroyed(JavascriptThreadService* service)
{
    Assert(this->threadService == service);
    this->threadService = nullptr;
    this->threadContext = nullptr;
}

RecyclerFinishConcurrentIdleTask::RecyclerFinishConcurrentIdleTask(ThreadContext* threadContext, JavascriptThreadService* service):
    refCount(0)
#if DBG
    , isIdleTaskComplete(false)
#endif
{
    this->threadContext = threadContext;
    this->threadService = service;
}

STDMETHODIMP_(void) RecyclerFinishConcurrentIdleTask::RunIdleTask(void)
{
    if (this->threadContext == nullptr)
    {
#if DBG
        this->isIdleTaskComplete = true;
#endif
        return;
    }

    Recycler* recycler = this->threadContext->GetRecycler();

    IDLE_COLLECT_TRACE(_u("Running FinishConcurrent during Idle Task\n"));

    // At this point, we are in one of three states:
    //  1) A collection is not in progress
    //  2) The GC is sweeping in the background thread
    //  3) Neither 1 nor 2 is true, so the GC must either be marking or waiting for an in-thread mark
    //
    // The idle task has something to do in the 3rd case. In the 1st and 2nd case, it's enough for it
    // to simply consider the task to be completed and remove the task
    //
    // In the 3rd case we call FinishConcurrent to try and finish marking. This will check to see if the GC is 
    // still marking in the background thread or if it's done. If it's done, it'll synchronously do an in-thread mark.
    // If the GC is still marking, FinishConcurrent will return false. For now, we'll leave the task scheduled here
    // so that as long as we're idle, we can keep checking on the background thread. Parallel mark is being implemented 
    // as we speak, and when that's done, FinishConcurrent can be changed to help with the background marking process.
    if (!recycler->CollectionInProgress() || recycler->IsSweeping() || recycler->FinishConcurrent<FinishConcurrentOnIdleAtRoot>())
    {
        IDLE_COLLECT_TRACE(_u("FinishConcurrent succeeded, completing task\n"));

        // TODO: If we have to do this multiple times, move this assert to an if-statement
        Assert(this->isIdleTaskComplete == false);
        HRESULT hr = this->threadService->OnFinishMarkTaskComplete();

#if DBG
        this->isIdleTaskComplete = true;
#endif

        Assert(SUCCEEDED(hr));
    }
}

//============================================================================================================
// IJavascriptThreadProperty Implementation
//============================================================================================================
HRESULT JavascriptThreadService::SetTimerProvider(ITimerCallbackProvider* inTimerProvider)
{
    HRESULT hr = NOERROR;
    IfFailedReturn(ValidateBaseThread());
    if (NULL == inTimerProvider)
    {
        return E_INVALIDARG;
    }
    Assert(NULL == timerProvider);
    if (NULL != timerProvider)
    {
        // REVIEW: different policy here? 
        return E_INVALIDARG;
    }
    timerProvider = inTimerProvider;
    timerProvider->AddRef();
    return NOERROR;
}

// IJavascriptThreadProperty Implementation
//============================================================================================================
HRESULT JavascriptThreadService::SetIdleTaskHost(IUnknown* inIdleTaskHost)
{
    HRESULT hr = NOERROR;
    IfFailedReturn(ValidateBaseThread());
    if (NULL == inIdleTaskHost)
    {
        return E_INVALIDARG;
    }

    Assert(NULL == this->idleTaskHost);
    if (NULL != this->idleTaskHost)
    {
        // REVIEW: different policy here? 
        return E_UNEXPECTED;
    }

    hr = inIdleTaskHost->QueryInterface(_uuidof(IIdleTaskHost), (void**)&idleTaskHost);

    if (SUCCEEDED(hr))
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            this->finishConcurrentTask = HeapNew(RecyclerFinishConcurrentIdleTask, GetThreadContext(), this);
            this->finishConcurrentTask->AddRef();
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr)
    }

    return hr;
}

inline HRESULT JavascriptThreadService::ValidateBaseThread(void)
{
    if (GetCurrentThreadId() != dwBaseThread)
    {
        AssertMsg(FALSE, "Not Base Thread");
        return E_UNEXPECTED;
    }
    else
        return NOERROR;
}

//============================================================================================================
// IActiveScriptLifecycleEventSink Implementation
//============================================================================================================
STDMETHODIMP JavascriptThreadService::OnEvent(_In_ EventId eventId, _In_ VARIANT* pvarArgs)
{
    switch (eventId)
    {
    case EventId_StartupComplete:    
        return E_INVALIDARG;
    case EventId_SuspendCleanupStart:
        RECORD_TIMESTAMP(suspendStartTime);

        this->GetThreadContext()->GetRecycler()->CollectNow<CollectOnSuspendCleanup>();
        return this->GetThreadContext()->GetRecycler()->CollectionInProgress()? S_OK : S_FALSE;
    case EventId_SuspendCleanupEnd:
        RECORD_TIMESTAMP(suspendEndTime);

        while (this->GetThreadContext()->GetRecycler()->CollectionInProgress())
        {
            this->GetThreadContext()->GetRecycler()->FinishConcurrent<ForceFinishCollection>();
        }
        return S_OK;
    case EventId_CollectTelemetry:
        Assert(pvarArgs);
        Assert(pvarArgs->vt == VT_BYREF);

        bool success = this->SetTelemetryBlock((ActiveScriptTelemetryBlock *)pvarArgs->byref);
        AssertMsg(success, "Host should not pass in multiple different pointers");
        return (success ? S_OK : E_FAIL);
    };
    Assert(false);
    return E_NOTIMPL;
}

//============================================================================================================
// Telemetry block
//============================================================================================================
bool 
JavascriptThreadService::SetTelemetryBlock(ActiveScriptTelemetryBlock * block)
{
    if (externalTelemetryBlock != nullptr)
    {
        return externalTelemetryBlock == block;
    }
    externalTelemetryBlock = block;
    this->GetThreadContext()->GetRecycler()->SetTelemetryBlock((RecyclerWatsonTelemetryBlock *)block);
    this->GetThreadContext()->SetTelemetryBlock((ThreadContextWatsonTelemetryBlock *)&block->lastScriptStartTime);
    this->telemetryBlock = (WatsonTelemetryBlock *)&block->suspendStartTime;
    return true;
}

//============================================================================================================
// Idle GC
//============================================================================================================
BOOL JavascriptThreadService::EnsureTimerProvider()
{
    if (timerProvider != nullptr)
    {
        return TRUE;
    }
    if (nullptr == rootTracker)
    {
        return FALSE;
    }
    HRESULT hr = rootTracker->QueryInterface(__uuidof(ITimerCallbackProvider), (void**)&timerProvider);
    Assert(SUCCEEDED(hr) && timerProvider != nullptr);
    if (FAILED(hr))
    {
        timerProvider = nullptr;
        return FALSE;
    }
    return TRUE;
}

bool JavascriptThreadService::CanScheduleIdleCollect()
{
    if (!EnsureTimerProvider())
    {
        Assert(!this->hasTimerScheduled);
        return false;
    }

    return true;
}

void JavascriptThreadService::SetIdleTaskState(IdleTaskState newState)
{
    IdleTaskState oldState = this->idleTaskState;
    IDLE_COLLECT_VERBOSE_TRACE(_u("Changing idle task state. Old: %d, New: %d\n"), oldState, newState);
    this->idleTaskState = newState;
}

bool JavascriptThreadService::OnScheduleIdleCollect(uint ticks, bool canScheduleAsTask)
{
    HRESULT hr;

    if (canScheduleAsTask && this->idleTaskHost != nullptr)
    {
        // At this point, one of the following is true:
        // - Either we need to schedule an idle task because a timer isn't scheduled (idleTaskState is NeedToSchedule, isIdleTaskScheduled is false)
        // - Or we need to post a timer, to schedule the idle task in the next callback (both variables are false)

        if (this->idleTaskState == IdleTaskState::NeedToSchedule)
        {
            Assert(this->isIdleTaskScheduled == false);
            Assert(this->finishConcurrentTask != nullptr);

            // If scheduling the idle task fails, fallback to the timer
            if (SUCCEEDED(this->idleTaskHost->AddIdleTask(this->finishConcurrentTask)))
            {
                this->SetIdleTaskState(IdleTaskState::Scheduled);

#if DBG
                this->isIdleTaskScheduled = true;
                this->finishConcurrentTask->OnTaskScheduled();
                this->hasTimerScheduled = true; // pretend that this is the timer
#endif
                return true;
            }
        }
        else
        {
            IDLE_COLLECT_VERBOSE_TRACE(_u("Set needScheduleIdleTask to true\n"));
            this->SetIdleTaskState(IdleTaskState::NeedToSchedule);
        }
    }

    hr = timerProvider->SetTimer(ticks, JavascriptThreadService::IdleCollectCallback, this);

    if (SUCCEEDED(hr))
    {
#ifdef RECYCLER_TRACE
        if (hasTimerScheduled)
        {
            IDLE_COLLECT_TRACE(_u("Idle timer restarted: %d\n"), ticks);
        }
        else
        {
            IDLE_COLLECT_TRACE(_u("Idle timer started  : %d\n"), ticks);
        }
        hasTimerScheduled = true;
#endif

        return true;
    }

    return false;
}

bool JavascriptThreadService::ShouldFinishConcurrentCollectOnIdleCallback()
{
    // If we need to schedule an idle task, we shouldn't try to finish 
    // a concurrent collection. Instead, we schedule an idle task to 
    // do that- since that will do so from the root, it'll be able to 
    // run a FinishCollection skipping the stack
    return this->idleTaskState != IdleTaskState::NeedToSchedule;
}

void JavascriptThreadService::OnFinishIdleCollect()
{
    if (this->timerProvider)
    {
        // If the state is that the idle task has been completed, 
        // then remove the task. Otherwise, the idle task is not scheduled
        // and any pending timers need to be killed
        if (this->idleTaskState == IdleTaskState::Completed)
        {
            Assert(this->idleTaskHost);
            Assert(this->idleTaskState == IdleTaskState::Completed);

            IDLE_COLLECT_TRACE(_u("Killing the idle task\n"));
            HRESULT hr = this->idleTaskHost->RemoveIdleTask(this->finishConcurrentTask);
            Assert(SUCCEEDED(hr));
            DebugOnly(this->isIdleTaskScheduled = false);
        }
        else
        {
            IDLE_COLLECT_TRACE(_u("Killing the timer\n"));
            timerProvider->KillPendingTimer();
        }

#ifdef RECYCLER_TRACE
        this->hasTimerScheduled = false;
        this->SetIdleTaskState(IdleTaskState::NotScheduled);
#endif
    }
}

void __cdecl 
JavascriptThreadService::IdleCollectCallback(void* context)
{
    IDLE_COLLECT_TRACE(_u("Idle collect callback called\n"));
    JavascriptThreadService* threadService = static_cast<JavascriptThreadService*>(context);
    Assert(!threadService->isIdleTaskScheduled);

    AssertMsg(threadService->GetThreadContext() == ThreadBoundThreadContextManager::EnsureContextForCurrentThread(), "invalid calling function");

    bool hasScheduledIdleCollect = threadService->IdleCollect();
    IDLE_COLLECT_VERBOSE_TRACE(_u("IdleCollectCallback- Set hasScheduledIdleCollect to %d\n"), hasScheduledIdleCollect);

    // Make sure we have register the callback or kill the timer
    Assert(hasScheduledIdleCollect || !threadService->hasTimerScheduled);
}


STDMETHODIMP JavascriptThreadService::GetActiveScriptGarbageCollector(IActiveScriptGarbageCollector** activeScriptGarbageCollector)
{
    *activeScriptGarbageCollector = this;
    this->AddRef();
    return S_OK;
}

STDMETHODIMP JavascriptThreadService::GetActiveScriptDirectGarbageCollector(IActiveScriptDirectGarbageCollector** activeScriptDirectGarbageCollector)
{
    *activeScriptDirectGarbageCollector = this;
    this->AddRef();
    return S_OK;
}

STDMETHODIMP JavascriptThreadService::GetTrackingService(ITrackingService** trackingService)
{
    *trackingService = this;
    this->AddRef();
    return S_OK;
}

STDMETHODIMP JavascriptThreadService::GetJavascriptThreadProperty(IJavascriptThreadProperty** javascriptThreadProperty)
{
    *javascriptThreadProperty = this;
    this->AddRef();
    return S_OK;
}

STDMETHODIMP JavascriptThreadService::GetActiveScriptLifecycleEventSink(IActiveScriptLifecycleEventSink** activeScriptLifecycleEventSink)
{
    *activeScriptLifecycleEventSink = this;
    this->AddRef();
    return S_OK;
}

STDMETHODIMP JavascriptThreadService::GetRecyclerNativeHeapHandle(RecyclerNativeHeapHandle* recyclerNativeHeapHandle)
{
    *recyclerNativeHeapHandle = ThreadContext::GetContextForCurrentThread()->GetRecycler();
    return S_OK;
}