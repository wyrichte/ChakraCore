//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
// COM object for thread specific service communciated between host and script engine
// like garbage collection interfaces and timer services.

#pragma once

interface ITracker: public IDispatchEx
{
    STDMETHOD(EnumerateTrackedObjects) (void *pgc)    = 0;
    STDMETHOD(SetTrackingAlias)   (VARIANT * pvar)   = 0;
    STDMETHOD(GetTrackingAlias)   (VARIANT ** ppvar) = 0;
};

class JavascriptThreadService;

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

class ThreadServiceWrapperBase : public ThreadServiceWrapper
{
public:
    bool ScheduleNextCollectOnExit() override sealed;
    void ScheduleFinishConcurrent() override sealed;
    void SetForceOneIdleCollection() override;

protected:
    enum FinishReason
    {
        FinishReasonNormal,
        FinishReasonIdleTimerSetupFailed,
        FinishReasonTaskComplete
    };

    ThreadServiceWrapperBase();

    bool Initialize(ThreadContext *newThreadContext);
    void Shutdown();

    bool IdleCollect();
    void FinishIdleCollect(FinishReason reason);
    void ClearForceOneIdleCollection();

    virtual bool CanScheduleIdleCollect() = 0;
    virtual bool OnScheduleIdleCollect(uint delta, bool scheduleAsTask) = 0;
    virtual void OnFinishIdleCollect() = 0;
    virtual bool ShouldFinishConcurrentCollectOnIdleCallback() = 0;

    ThreadContext *GetThreadContext() { return threadContext; }

private:
    static const unsigned int IdleTicks = 1000; // 1 second
    static const unsigned int IdleFinishTicks = 100; // 100 ms;

    bool ScheduleIdleCollect(uint ticks, bool scheduleAsTask);

    ThreadContext* threadContext;
    bool inIdleCollect;
    bool needIdleCollect;
    bool forceIdleCollectOnce;
    unsigned int tickCountNextIdleCollection;
    bool hasScheduledIdleCollect;
    bool shouldScheduleIdleCollectOnExitIdle;
};

class JavascriptThreadService sealed : public IActiveScriptGarbageCollector,
                   public IActiveScriptDirectGarbageCollector,
                   public ITrackingService,
                   public IJavascriptThreadProperty,
                   public IActiveScriptLifecycleEventSink,
                   public ThreadServiceWrapperBase
{
    enum IdleTaskState
    {
        NotScheduled,
        NeedToSchedule,
        Scheduled,
        Completed
    };

public:
    JavascriptThreadService ();
    ~JavascriptThreadService ();

    BOOL Initialize(void);

    HRESULT OnFinishMarkTaskComplete();

    // *** IUnknown ***
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // *** IActiveScriptGarbageCollector ***
    STDMETHOD(CollectGarbage)(SCRIPTGCTYPE scriptgctype) override;

    // *** IActiveScriptDirectGarbageCollector ***
    STDMETHOD(AddCallBack)(CollectGarbageCallBack gccallback, void * context, CollectGarbageCallBackHandle* callBackHandle) override;
    STDMETHOD(RemoveCallBack)(CollectGarbageCallBackHandle callBackHandle) override;

    // *** ITrackingService
    STDMETHOD(RegisterTrackingClient) (ITracker* pTracker) override;
    STDMETHOD(UnregisterTrackingClient) (ITracker* pTracker) override;
    STDMETHOD(EnumerateTrackingClient) (void* pv, IUnknown* punk, BOOL fTracker) override;
    STDMETHOD(IsTrackedObject) (IUnknown* pUnk, IDispatchEx** ppDispTracked, BOOL* pfTracker) override;

    // IJavascriptThreadProperty methods
    STDMETHOD(SetTimerProvider)(ITimerCallbackProvider* timerProvider) override;
    STDMETHOD(SetIdleTaskHost)(IUnknown* idleTaskHost) override;

    // IActiveScriptLifecycleEventSink
    STDMETHOD(OnEvent)(_In_ EventId eventId, _In_ VARIANT* pvarArgs);

    static HRESULT CollectGarbage(Recycler* recycler, SCRIPTGCTYPE scriptgctype);

private:
    ulong refCount;
    DWORD dwBaseThread;

    // Flag indicating that we need to schedule an idle task the next time 
    // we get a timer callback
    IdleTaskState idleTaskState;
    __declspec(thread) static bool wasInitialize;

    HRESULT ValidateBaseThread(void);
    // Telemetry
    bool SetTelemetryBlock(ActiveScriptTelemetryBlock * telemetryBlock);

    ActiveScriptTelemetryBlock * externalTelemetryBlock;
    struct WatsonTelemetryBlock
    {
        FILETIME suspendStartTime;
        FILETIME suspendEndTime;
    };
    WatsonTelemetryBlock localTelemetryBlock;
    WatsonTelemetryBlock * telemetryBlock;

    // Tracker
    ITracker* rootTracker;

    // Idle Collection

    BOOL EnsureTimerProvider();

    ITimerCallbackProvider* timerProvider;
    IIdleTaskHost* idleTaskHost;
    RecyclerFinishConcurrentIdleTask* finishConcurrentTask;

#ifdef RECYCLER_TRACE
    bool hasTimerScheduled;
#endif

#if DBG
    bool isIdleTaskScheduled;
#endif

    // ThreadServiceWrapper
    // There should be one JavascriptThreadService per ThreadContext. ThreadService would work from ThreadContext
    // Let's use ThreadContext to hold the JavascriptThreadService.
    // As the contract between trident & jscript, let's use root tracker to hold on to timerservice. ie., we'll QI
    // roottracker to get timer service.
    bool CanScheduleIdleCollect() override;
    bool OnScheduleIdleCollect(uint ticks, bool scheduleAsTask) override;
    void OnFinishIdleCollect() override;
    bool ShouldFinishConcurrentCollectOnIdleCallback() override;

    void SetIdleTaskState(IdleTaskState newState);

    static void __cdecl IdleCollectCallback(void* context);
    void SetRootTracker(ITracker* rootTracker) ;
    ITracker* GetRootTracker();

    static void __cdecl  RootTrackerMarker(void * context);
    void RootTrackerMarker();
};

class JsrtThreadService : public ThreadServiceWrapperBase
{
public:
    JsrtThreadService();
    ~JsrtThreadService();

    bool Initialize(ThreadContext *threadContext);
    unsigned int Idle();

    // Does nothing, we don't force idle collection for JSRT
    void SetForceOneIdleCollection() override {}

private:
    bool CanScheduleIdleCollect() override { return true; }
    bool OnScheduleIdleCollect(uint ticks, bool scheduleAsTask) override;
    void OnFinishIdleCollect() override;
    bool ShouldFinishConcurrentCollectOnIdleCallback() override;

    unsigned int nextIdleTick;
};
