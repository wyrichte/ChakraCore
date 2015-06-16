//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <winmeta.h>
#include <evntrace.h>
#include <TraceLoggingProvider.h>

// List of all telemetry data points.
#define TL_GCPAUSESTATS "GCPauseStats.V2"
#define TL_ES5BUILTINS "ES5Builtins"
#define TL_ES6BUILTINS "ES6Builtins"
#define TL_TABUILTINS "TABuiltins"
#define TL_ES6CTORS "ES6Ctors"
#define TL_JITTIMESTATS "JITTime.V2"
#define TL_ES6LANGFEATURES "ES6LanguageFeatures"
#define TL_GLOBALSTATS "GlobalStats.V2"
#define TL_MEMSTATS "MemStats.V2"
#define TL_PARSERSTATS "Parser"

#ifdef DBG
#define TL_BINARYFLAVOR "CHK"
#else
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#define TL_BINARYFLAVOR "FRETEST"
#else
#define TL_BINARYFLAVOR "FRE"
#endif
#endif

extern struct _TlgProvider_t const * const g_hTraceLoggingProv;

class CEventTraceProperties
{
public:

    CEventTraceProperties();

    operator EVENT_TRACE_PROPERTIES* ();
    EVENT_TRACE_PROPERTIES& Properties();
    HRESULT SetLogFileName(_In_ LPCWSTR wszLogFileName);
    HRESULT SetLoggerName(_In_ LPCWSTR wszLoggerName);

private:
    static SIZE_T const s_cbLogFileName = MAX_PATH * sizeof(WCHAR);
    static SIZE_T const s_cbLoggerName = MAX_PATH * sizeof(WCHAR);
    static SIZE_T const s_cbEventTraceData = sizeof(EVENT_TRACE_PROPERTIES) + s_cbLogFileName + s_cbLoggerName;

    BYTE m_rgData[s_cbEventTraceData];
    EVENT_TRACE_PROPERTIES* m_pEventTraceProperties;
};

class CEtwSession
{
public:
    enum SessionScope
    {
        SessionScope_SystemWide,
        SessionScope_InProcess
    };

public:
    CEtwSession(_In_ LPCWSTR wszLoggerName, _In_ LPCWSTR wszLogFileName, _In_ SessionScope sessionScope = SessionScope_SystemWide);
    ~CEtwSession();

    HRESULT EnableProvider(_In_ GUID const& ProviderId);

private:
    CEventTraceProperties m_rgData;
    TRACEHANDLE m_SessionHandle;
};

class TraceLoggingClient
{
    CEtwSession *session;
    bool shouldLogTelemetry;	
    bool isHighResAvail;
public:
    TraceLoggingClient();
    ~TraceLoggingClient();
    bool GetShouldLogTelemetry() { return shouldLogTelemetry;  }
    void SetIsHighResPerfCounterAvailable();
    void FireSiteNavigation(const wchar_t *url, GUID activityId, DWORD host, bool isJSRT);
    void ResetTelemetryStats(ThreadContext* threadContext);

};

extern TraceLoggingClient *g_TraceLoggingClient;