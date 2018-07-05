//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <winbase.h>
#include <TraceLoggingProvider.h>

#define TL_CHAKRAINIT "ChakraInit"
#define TL_DIRECTCALLRAW "DirectCallRaw.PerfStats"
#define TL_DIRECTCALLTIME "DirectCallRaw.LogTime"


class CriticalSectionWrapper
{
private:
    CRITICAL_SECTION cs;
public:
    CriticalSectionWrapper()
    {
       InitializeCriticalSection(&cs);
    }

    ~CriticalSectionWrapper()
    {
        DeleteCriticalSection(&cs);
    }

    void Enter()
    {
        EnterCriticalSection(&cs);
    }

    void Leave()
    {
        LeaveCriticalSection(&cs);
    }
};

class Throttle
{
    // local period length of 1 second
    static const ULONGLONG LOCAL_PERIOD_LENGTH = 1000;
    static const uint32 LOCAL_MAX_EVENTS_PER_INTERVAL = 3;
    uint32 localEventCount = 0;
    ULONGLONG localLastEvent = 0;

    // global period length of 60 seconds
    static const ULONGLONG GLOBAL_PERIOD_LENGTH = 60000;
    static const uint32 GLOBAL_MAX_EVENTS_PER_INTERVAL = 180;
    static uint32 globalEventCount;
    static ULONGLONG globalLastEvent;
    static  CriticalSectionWrapper criticalSectionWrapper;

public:
    Throttle() { localLastEvent = 0; }

    bool isThrottled()
    {
        // number of milliseconds since system started
        ULONGLONG millis = GetTickCount64();
        
        if (localThrottleUpdate(millis))
        {
            return true;
        }

        // global throttle
        if (globalThrottleUpdate(millis))
        {
            return true;
        }

        return false;
    }

    /**
     *  Update current Throttle counters.  Return true if throttled, false otherwise.
     */
    bool localThrottleUpdate(_In_ ULONGLONG millis)
    {
        bool isThrottled = false;
        ULONGLONG thisEvent = millis / LOCAL_PERIOD_LENGTH;
        if (thisEvent == localLastEvent)
        {
            ++localEventCount;
            isThrottled = localEventCount > LOCAL_MAX_EVENTS_PER_INTERVAL;
        }
        else
        {
            localLastEvent = thisEvent;
            localEventCount = 1;
        }

        return isThrottled;
    }

    /**
     *  Update global Throttle counters.  Return true if throttled, false otherwise.
     */
    bool globalThrottleUpdate(_In_ ULONGLONG millis)
    {
        bool isThrottled = false;
        criticalSectionWrapper.Enter();
        ULONGLONG globalCurrentEvent = millis / GLOBAL_PERIOD_LENGTH;
        if (globalLastEvent == globalCurrentEvent)
        {
            ++globalEventCount;
            isThrottled = globalEventCount > GLOBAL_MAX_EVENTS_PER_INTERVAL;
        }
        else
        {
            // reset time & timers
            globalLastEvent = globalCurrentEvent;
            globalEventCount = 1;
        }
        criticalSectionWrapper.Leave();
        return isThrottled;
    }

};

class Telemetry
{
public:
    static void EnsureInitializeForJSRT();
    static void OnJSRTThreadContextClose();
private:
    static DWORD initialized;
};

extern struct _TlgProvider_t const * const g_hTraceLoggingProv;

class TraceLoggingClient
{
    bool shouldLogTelemetry;

    Throttle throttle;

    bool isExperimentalFlagEnabled();
    GUID chakraInstanceID;

public:
    TraceLoggingClient();
    ~TraceLoggingClient();
    bool GetShouldLogTelemetry() { return shouldLogTelemetry;  }
    void FireSiteNavigation(const char16 *url, GUID activityId, DWORD host, bool isJSRT);
    void FireChakraInitTelemetry(DWORD host, bool isJSRT);
    bool IsThrottled() { return this->throttle.isThrottled(); }
    bool IsProviderEnabled() const;
    const GUID& GetChakraInstanceID() const { return  this->chakraInstanceID; }

#ifdef ENABLE_DIRECTCALL_TELEMETRY
    void FirePeriodicDomTelemetry(GUID activityId);
    void FireDomTelemetry(GUID activityId);
#endif

#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
    void FireDomTelemetryStats(double tracelogTimeMs, double logTimeMs);
#endif
    void ResetTelemetryStats(ThreadContext* threadContext);
};

extern TraceLoggingClient *g_TraceLoggingClient;
