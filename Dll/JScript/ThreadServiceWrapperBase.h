//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

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
