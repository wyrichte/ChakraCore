//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
// COM object for thread specific service communciated between host and script engine
// like garbage collection interfaces and timer services.

#pragma once

#include "Base\ThreadServiceWrapperBase.h"

interface ITracker: public IDispatchEx
{
    STDMETHOD(EnumerateTrackedObjects) (void *pgc)    = 0;
    STDMETHOD(SetTrackingAlias)   (VARIANT * pvar)   = 0;
    STDMETHOD(GetTrackingAlias)   (VARIANT ** ppvar) = 0;
};

// 0x8f88fd19, 0x5d42, 0x477b, {0xbd, 0x45, 0xf6, 0xa4, 0xa9, 0x77, 0xed, 0x05}
interface ITrackingService : public IUnknown
{
public:
    STDMETHOD(RegisterTrackingClient) (ITracker* pTracker) = 0;
    STDMETHOD(UnregisterTrackingClient) (ITracker* pTracker) = 0;
    STDMETHOD(EnumerateTrackingClient) (void* pv, IUnknown* punk, BOOL fTracker) = 0;
    STDMETHOD(IsTrackedObject) (IUnknown* pUnk, IDispatchEx** ppDispTracked, BOOL* pfTracker) = 0;
};

class JavascriptThreadService sealed : public IJavascriptThreadService,
                   public IActiveScriptGarbageCollector,
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

    // *** IJavascriptThreadService ***
    STDMETHOD(GetActiveScriptGarbageCollector)(IActiveScriptGarbageCollector** activeScriptGarbageCollector);
    STDMETHOD(GetActiveScriptDirectGarbageCollector)(IActiveScriptDirectGarbageCollector** activeScriptDirectGarbageCollector);
    STDMETHOD(GetTrackingService)(ITrackingService** trackingService);
    STDMETHOD(GetJavascriptThreadProperty)(IJavascriptThreadProperty** javascriptThreadProperty);
    STDMETHOD(GetActiveScriptLifecycleEventSink)(IActiveScriptLifecycleEventSink** activeScriptLifecycleEventSink);

    STDMETHOD(GetRecyclerNativeHeapHandle)(RecyclerNativeHeapHandle* recyclerNativeHeapHandle);

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
    class RecyclerFinishConcurrentIdleTask* finishConcurrentTask;

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
