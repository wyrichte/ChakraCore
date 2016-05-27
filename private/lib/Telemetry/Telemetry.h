//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <wincrypt.h>
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
#define TL_CHAKRAINIT "ChakraInit"
#define TL_DIRECTCALLRAW "DirectCallRaw.PerfStats"
#define TL_DIRECTCALLTIME "DirectCallRaw.LogTime"
#define TL_CHAKRANODEPACK "ChakraNodePackage"


#ifdef DBG
#define TL_BINARYFLAVOR "CHK"
#else
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#define TL_BINARYFLAVOR "FRETEST"
#else
#define TL_BINARYFLAVOR "FRE"
#endif
#endif

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
    //Code for node telemetry purposes
    typedef JsUtil::BaseHashSet<const char16*, Recycler, PrimeSizePolicy> NodePackageSet;
    NodePackageSet *NodePackageIncludeList;
    bool hasNodeModules;
    bool isPackageTelemetryFired;
    LARGE_INTEGER freq;
    HCRYPTPROV hProv;

    Throttle throttle;

public:
    TraceLoggingClient();
    ~TraceLoggingClient();
    bool GetShouldLogTelemetry() { return shouldLogTelemetry;  }
    void SetIsHighResPerfCounterAvailable();
    void FireSiteNavigation(const char16 *url, GUID activityId, DWORD host, bool isJSRT);
    void FireChakraInitTelemetry(DWORD host, bool isJSRT);

#ifdef ENABLE_DIRECTCALL_TELEMETRY
    void FirePeriodicDomTelemetry(GUID activityId);
    void FireFinalDomTelemetry(GUID activityId);
#endif

#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
    void FireDomTelemetryStats(double tracelogTimeMs, double logTimeMs);
#endif
    void ResetTelemetryStats(ThreadContext* threadContext);
    void CreateHashAndFirePackageTelemetry();
    void InitializeNodePackageList();
    void ReleaseNodePackageList();
    void AddPackageName(const char16* packageName);
    bool IsPackageTelemetryFired(){ return isPackageTelemetryFired; }
    void SetIsPackageTelemetryFired(bool value){ isPackageTelemetryFired = value; }
    // For node telemetry purposes
    void TryLogNodePackage(Recycler*, const char16*url);
    HCRYPTPROV EnsureCryptoContext();
    void FirePackageTelemetryHelper();
};

extern TraceLoggingClient *g_TraceLoggingClient;