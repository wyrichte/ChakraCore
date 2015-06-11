//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "share.h"
#pragma hdrstop

//
// ThreadServiceWrapperBase
///

#ifdef RECYCLER_TRACE
#define IDLE_COLLECT_VERBOSE_TRACE(msg, ...) \
    if (Js::Configuration::Global.flags.Verbose) \
    { \
        IDLE_COLLECT_TRACE(msg, __VA_ARGS__); \
    }

#define IDLE_COLLECT_TRACE(msg, ...) \
    if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::IdleCollectPhase)) \
    {\
        Output::Print(L"%04X> " ## msg, ::GetCurrentThreadId(), __VA_ARGS__); \
        Output::Flush(); \
    } 
#else
#define IDLE_COLLECT_TRACE(...)
#define IDLE_COLLECT_VERBOSE_TRACE(...)
#endif

ThreadServiceWrapperBase::ThreadServiceWrapperBase() :
    threadContext(nullptr),
    needIdleCollect(false),
    inIdleCollect(false),
    hasScheduledIdleCollect(false),
    shouldScheduleIdleCollectOnExitIdle(false),
    forceIdleCollectOnce(false)
{
}

bool ThreadServiceWrapperBase::Initialize(ThreadContext *newThreadContext)
{
    if (newThreadContext == nullptr)
    {
        return false;
    }
    threadContext = newThreadContext;
    threadContext->SetThreadServiceWrapper(this);
    return true;
}

void ThreadServiceWrapperBase::Shutdown()
{
    if (hasScheduledIdleCollect)
    {
#if DBG
        // Fake the inIdleCollect to get pass asserts in FinishIdleCollect
        inIdleCollect = true;
#endif
        FinishIdleCollect(FinishReason::FinishReasonNormal);
    }
}

bool ThreadServiceWrapperBase::ScheduleIdleCollect(uint ticks, bool scheduleAsTask)
{
    Assert(!threadContext->IsInScript());

    // We should schedule have called this in one of two cases:
    //  1) Either needIdleCollect is true- in which case, we should schedule one
    //  2) Or ScheduleNextCollectionOnExit was called when needIdleCollect was true, but we didn't schedule 
    //      one because we were at the time in one. Later, as we unwound, we might have set needIdleCollect to false
    //      but because we had noted that we needed to schedule a collect, we would end up coming into this function
    //      so allow for that
    Assert(needIdleCollect || shouldScheduleIdleCollectOnExitIdle|| threadContext->GetRecycler()->CollectionInProgress());

    if (!CanScheduleIdleCollect())
    {
        return false;
    }

    if (hasScheduledIdleCollect)
    {
        return true;
    }

    if (OnScheduleIdleCollect(ticks, scheduleAsTask))
    {
        JSETW(EventWriteJSCRIPT_GC_IDLE_START(this));
        IDLE_COLLECT_VERBOSE_TRACE(L"ScheduledIdleCollect- Set hasScheduledIdleCollect\n");

        hasScheduledIdleCollect = true;
        return true;
    }
    else
    {
        IDLE_COLLECT_TRACE(L"Idle timer setup failed\n");
        FinishIdleCollect(FinishReason::FinishReasonIdleTimerSetupFailed);
        return false;
    }
}

bool ThreadServiceWrapperBase::IdleCollect()
{
    Assert(hasScheduledIdleCollect);
    IDLE_COLLECT_VERBOSE_TRACE(L"IdleCollect- reset hasScheduledIdleCollect\n");
    hasScheduledIdleCollect = false;

    // Don't do anything and kill the timer if we are called recursively or if we are in script
    if (inIdleCollect || threadContext->IsInScript())
    {
        FinishIdleCollect(FinishReason::FinishReasonNormal);
        return hasScheduledIdleCollect;
    }

    // If during idle collect we determine that we need to schedule another
    // idle collect, this gets flipped to true
    shouldScheduleIdleCollectOnExitIdle = false;

    AutoBooleanToggle autoInIdleCollect(&inIdleCollect);
    Recycler* recycler = threadContext->GetRecycler();
    // Finish concurrent on timer heart beat if needed
    // We wouldn't try to finish if we need to schedule 
    // an idle task to finish the collection
    if (this->ShouldFinishConcurrentCollectOnIdleCallback() && recycler->FinishConcurrent<FinishConcurrentOnIdle>())
    {
        IDLE_COLLECT_TRACE(L"Idle callback: finish concurrent\n");
        JSETW(EventWriteJSCRIPT_GC_IDLE_CALLBACK_FINISH(this));
    }

    while (true)
    {
        // If a GC is still happening, just wait for the next heart beat
        if (recycler->CollectionInProgress())
        {
            ScheduleIdleCollect(IdleTicks, true /* schedule as task */);
            break;
        }

        // If there no more need of idle collect, then cancel the timer
        if (!needIdleCollect)
        {
            FinishIdleCollect(FinishReason::FinishReasonNormal);
            break;
        }

        int timeDiff = tickCountNextIdleCollection - GetTickCount();

        // See if we pass the time for the next scheduled Idle GC
        if (timeDiff > 0)
        {
            // IDLEGC-TODO:  since we might have activated another GC outside of script,
            // We should recheck the GC heuristic to make sure we will still activate a GC
            // when we do activate the idle GC and kill the timer if not.

            // Not time yet, wait for the next heart beat
            // IDLEGC-CONSIDER: may be we want to schedule it to call on when we want the idle GC (timeDiff)
            // instead of a heart beat using full timeout amount?
            ScheduleIdleCollect(IdleTicks, false /* not schedule as task */);

            IDLE_COLLECT_TRACE(L"Idle callback: nop until next collection: %d\n",  timeDiff);
            break;
        }

        // activate an idle collection
        IDLE_COLLECT_TRACE(L"Idle callback: collection: %d\n", timeDiff);
        JSETW(EventWriteJSCRIPT_GC_IDLE_CALLBACK_NEWCOLLECT(this));

        needIdleCollect = false;
        recycler->CollectNow<CollectOnScriptIdle>();
    }

    if (shouldScheduleIdleCollectOnExitIdle)
    {
        ScheduleIdleCollect(IdleTicks, false /* not schedule as task */);
    }

    return hasScheduledIdleCollect;
}

void ThreadServiceWrapperBase::FinishIdleCollect(ThreadServiceWrapperBase::FinishReason reason)
{
    Assert(reason == FinishReason::FinishReasonIdleTimerSetupFailed || 
        reason == FinishReason::FinishReasonTaskComplete ||
        inIdleCollect || threadContext->IsInScript() || !threadContext->GetRecycler()->CollectionInProgress());

    IDLE_COLLECT_VERBOSE_TRACE(L"FinishIdleCollect- Reset hasScheduledIdleCollect\n");
    hasScheduledIdleCollect = false;
    needIdleCollect = false;

    OnFinishIdleCollect();

    IDLE_COLLECT_TRACE(L"Idle timer finished\n");
    JSETW(EventWriteJSCRIPT_GC_IDLE_FINISHED(this));
}

bool ThreadServiceWrapperBase::ScheduleNextCollectOnExit()
{
    Assert(!threadContext->IsInScript());
    Assert(!needIdleCollect || hasScheduledIdleCollect);

    Recycler* recycler = threadContext->GetRecycler();
    recycler->FinishConcurrent<FinishConcurrentOnExitScript>();

#ifdef RECYCLER_TRACE
    bool oldNeedIdleCollect = needIdleCollect;

    if (forceIdleCollectOnce)
    {
        IDLE_COLLECT_VERBOSE_TRACE(L"Need to force one idle collection\n");
    }
#endif

    needIdleCollect = forceIdleCollectOnce || recycler->ShouldIdleCollectOnExit();

    if (needIdleCollect)
    {
        // Set up when we will do the idle decommit
        tickCountNextIdleCollection = GetTickCount() + IdleTicks;

        IDLE_COLLECT_VERBOSE_TRACE(L"Idle on exit collect %s: %d\n", (oldNeedIdleCollect? L"rescheduled" : L"scheduled"), 
            tickCountNextIdleCollection - GetTickCount());

        JSETW(EventWriteJSCRIPT_GC_IDLE_SCHEDULED(this));
    }
    else
    {
        IDLE_COLLECT_VERBOSE_TRACE(L"Idle on exit collect %s\n", oldNeedIdleCollect? L"cancelled" : L"not scheduled");
        if (!recycler->CollectionInProgress())
        {            
            // We collected and finished, no need to ensure the idle collect call back.
            return true;
        }

        IDLE_COLLECT_VERBOSE_TRACE(L"Idle on exit collect %s\n", hasScheduledIdleCollect || oldNeedIdleCollect? L"reschedule finish" : L"schedule finish");
    }

    // Don't schedule the call back if we are already in idle call back, as we don't do anything on recursive call anyways
    // IdleCollect will schedule one if necessary
    if (inIdleCollect)
    {
        shouldScheduleIdleCollectOnExitIdle = true;
        return true;
    }
    else
    {
        return ScheduleIdleCollect(IdleTicks, false /* not schedule as task */);
    }
}

void ThreadServiceWrapperBase::ClearForceOneIdleCollection()
{
    IDLE_COLLECT_VERBOSE_TRACE(L"Clearing force idle collect flag\n");

    this->forceIdleCollectOnce = false;
}

void ThreadServiceWrapperBase::SetForceOneIdleCollection()
{
    IDLE_COLLECT_VERBOSE_TRACE(L"Setting force idle collect flag\n");

    this->forceIdleCollectOnce = true;
}

void ThreadServiceWrapperBase::ScheduleFinishConcurrent()
{
    Assert(!threadContext->IsInScript());
    Assert(threadContext->GetRecycler()->CollectionInProgress());

    if (!this->inIdleCollect)
    {
        IDLE_COLLECT_VERBOSE_TRACE(L"Idle collect %s\n", needIdleCollect? L"reschedule finish" : L"scheduled finish"); 
        this->needIdleCollect = false;
        ScheduleIdleCollect(IdleFinishTicks, true /* schedule as task */);
    }
}

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
    Assert(this->idleTaskHost != null);
    Assert(this->finishConcurrentTask != null);
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
        *ppvObj = static_cast<IUnknown*>(static_cast<ITrackingService*>(this));
        AddRef();
        return NOERROR;
    }
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
                    if (SUCCEEDED(pUnk->QueryInterface(IID_IDispatch, (void**)&dispatch)))
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
            hr = javascriptDispatch->QueryInterface(IID_IDispatchEx, (void**)ppDispTracked);
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

    IDLE_COLLECT_TRACE(L"Running FinishConcurrent during Idle Task\n");

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
        IDLE_COLLECT_TRACE(L"FinishConcurrent succeeded, completing task\n");

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
    if (externalTelemetryBlock != null)
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
    if (timerProvider != NULL)
    {
        return TRUE;
    }
    if (NULL == rootTracker)
    {
        return FALSE;
    }
    HRESULT hr = rootTracker->QueryInterface(__uuidof(ITimerCallbackProvider), (void**)&timerProvider);
    Assert(SUCCEEDED(hr) && timerProvider != NULL);
    if (FAILED(hr))
    {
        timerProvider = NULL;
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
    IDLE_COLLECT_VERBOSE_TRACE(L"Changing idle task state. Old: %d, New: %d\n", oldState, newState);
    this->idleTaskState = newState;
}

bool JavascriptThreadService::OnScheduleIdleCollect(uint ticks, bool canScheduleAsTask)
{
    HRESULT hr;

    if (canScheduleAsTask && this->idleTaskHost != null)
    {
        // At this point, one of the following is true:
        // - Either we need to schedule an idle task because a timer isn't scheduled (idleTaskState is NeedToSchedule, isIdleTaskScheduled is false)
        // - Or we need to post a timer, to schedule the idle task in the next callback (both variables are false)

        if (this->idleTaskState == IdleTaskState::NeedToSchedule)
        {
            Assert(this->isIdleTaskScheduled == false);
            Assert(this->finishConcurrentTask != null);

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
            IDLE_COLLECT_VERBOSE_TRACE(L"Set needScheduleIdleTask to true\n");
            this->SetIdleTaskState(IdleTaskState::NeedToSchedule);
        }
    }

    hr = timerProvider->SetTimer(ticks, JavascriptThreadService::IdleCollectCallback, this);

    if (SUCCEEDED(hr))
    {
#ifdef RECYCLER_TRACE
        if (hasTimerScheduled)
        {
            IDLE_COLLECT_TRACE(L"Idle timer restarted: %d\n", ticks);
        }
        else
        {
            IDLE_COLLECT_TRACE(L"Idle timer started  : %d\n", ticks);
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

            IDLE_COLLECT_TRACE(L"Killing the idle task\n");
            HRESULT hr = this->idleTaskHost->RemoveIdleTask(this->finishConcurrentTask);
            Assert(SUCCEEDED(hr));
            DebugOnly(this->isIdleTaskScheduled = false);
        }
        else
        {
            IDLE_COLLECT_TRACE(L"Killing the timer\n");
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
    IDLE_COLLECT_TRACE(L"Idle collect callback called\n");
    JavascriptThreadService* threadService = static_cast<JavascriptThreadService*>(context);
    Assert(!threadService->isIdleTaskScheduled);

    AssertMsg(threadService->GetThreadContext() == ThreadBoundThreadContextManager::EnsureContextForCurrentThread(), "invalid calling function");

    bool hasScheduledIdleCollect = threadService->IdleCollect();
    IDLE_COLLECT_VERBOSE_TRACE(L"IdleCollectCallback- Set hasScheduledIdleCollect to %d\n", hasScheduledIdleCollect);

    // Make sure we have register the callback or kill the timer
    Assert(hasScheduledIdleCollect || !threadService->hasTimerScheduled);
}

//
// JsrtThreadService
//

JsrtThreadService::JsrtThreadService() :
    ThreadServiceWrapperBase(),
    nextIdleTick(DWORD_MAX)
{
}
 
JsrtThreadService::~JsrtThreadService()
{
    Shutdown();
}

bool JsrtThreadService::Initialize(ThreadContext *threadContext)
{
    return ThreadServiceWrapperBase::Initialize(threadContext);
}

unsigned int JsrtThreadService::Idle()
{
    unsigned int currentTicks = GetTickCount();

    if (currentTicks >= nextIdleTick)
    {
        IdleCollect();
    }

    return nextIdleTick;
}

bool JsrtThreadService::OnScheduleIdleCollect(uint ticks, bool /* canScheduleAsTask */)
{
    nextIdleTick = GetTickCount() + ticks;
    return true;
}

bool JsrtThreadService::ShouldFinishConcurrentCollectOnIdleCallback()
{
    // For the JsrtThreadService, there is no idle task host
    // so we should always try to finish concurrent on entering 
    // the idle callback
    return true;
}

void JsrtThreadService::OnFinishIdleCollect()
{
    nextIdleTick = DWORD_MAX;
}
