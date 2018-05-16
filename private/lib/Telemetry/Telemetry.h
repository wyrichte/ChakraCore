//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <TraceLoggingProvider.h>

#define TL_CHAKRAINIT "ChakraInit"
#define TL_DIRECTCALLRAW "DirectCallRaw.PerfStats"
#define TL_DIRECTCALLTIME "DirectCallRaw.LogTime"

class Throttle
{
    const WORD MIN_GAP = 2; // minimum gap between two telemetry logging from the same code path
    WORD lastEvent;
public:
    Throttle() { lastEvent = 0;  }
    bool isThrottled()
    {
        SYSTEMTIME st;
        GetSystemTime(&st);
        WORD thisEvent = st.wSecond / MIN_GAP;

        if (thisEvent != lastEvent)
        {
            lastEvent = thisEvent;
            return false;
        }
        else
        {
            return true;
        }
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
public:
    TraceLoggingClient();
    ~TraceLoggingClient();
    bool GetShouldLogTelemetry() { return shouldLogTelemetry;  }
    void FireSiteNavigation(const char16 *url, GUID activityId, DWORD host, bool isJSRT);
    void FireChakraInitTelemetry(DWORD host, bool isJSRT);
    bool IsThrottled() { return this->throttle.isThrottled(); }
    bool IsProviderEnabled() const;

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
