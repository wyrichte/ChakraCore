//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "TelemetryPch.h"
#include <strsafe.h>
#include "ChakraVersion.h"
#include "ieconfig.h"
#include "globalthreadstate.h"
#include <telemetry\MicrosoftTelemetry.h>
#include "Base\ThreadContextTLSEntry.h"
#include "Base\ThreadBoundThreadContextManager.h"

#ifdef ENABLE_EXPERIMENTAL_FLAGS
#include <iesettings.h>
#endif

// GUID for "ChakraProvider_V0.1": {FC7BA620-EB50-483D-97A0-72D8268A14B5}

TRACELOGGING_DEFINE_PROVIDER(g_hTraceLoggingProv,
    "Microsoft.Web.Platform.Chakra",
    (0xfc7ba620, 0xeb50, 0x483d, 0x97, 0xa0, 0x72, 0xd8, 0x26, 0x8a, 0x14, 0xb5),
    TraceLoggingOptionMicrosoftTelemetry());


#include "TelemetryMacros.h"

WCHAR *g_ProcessExclusionList[] = {
    _u("jshost"),
    _u("jc"),
    _u("slate"),
    _u("mshtmpad"),
    _u("te.processhost"),
    _u("jdtest"),
    _u("jsglass"),
    _u("loader42")
};


// Creating wrapper for atExit Scenario as we want to tackle OOM and other exceptions.
void __cdecl firePackageTelemetryAtExit() 
{
  if (g_TraceLoggingClient != nullptr && !(g_TraceLoggingClient->GetNodeTelemetryProvider()->IsPackageTelemetryFired()))
  {
    HRESULT hr = NOERROR;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
      g_TraceLoggingClient->GetNodeTelemetryProvider()->FirePackageTelemetryHelper();
    END_TRANSLATE_OOM_TO_HRESULT(hr);
  }
}

DWORD Telemetry::initialized = FALSE;
void Telemetry::EnsureInitializeForJSRT()
{
    if (::InterlockedExchange(&initialized, TRUE) == FALSE)
    {
        atexit(firePackageTelemetryAtExit);
    }
}

void Telemetry::OnJSRTThreadContextClose()
{
    if (g_TraceLoggingClient != nullptr && !(g_TraceLoggingClient->GetNodeTelemetryProvider()->IsPackageTelemetryFired()))
    {
        g_TraceLoggingClient->GetNodeTelemetryProvider()->FirePackageTelemetryHelper();
    }
}

TraceLoggingClient *g_TraceLoggingClient = NULL;

TraceLoggingClient::TraceLoggingClient() : shouldLogTelemetry(true), telemetryThrottledByChance(false)
{
    // Check if we're running in a process from which telemetry should
    // not be logged.  We'll default to logging telemetry if the process
    // name cannot be determined for some reason.
    WCHAR fullPath[MAX_PATH];
    WCHAR fname[MAX_PATH];
    DWORD dwResult = GetModuleFileName(NULL, fullPath, _countof(fullPath));
    if (dwResult != 0)
    {
        if (_wsplitpath_s(fullPath, NULL, 0, NULL, 0, fname, _countof(fname), NULL, 0) == 0)
        {
            for (int i = 0; i < _countof(g_ProcessExclusionList); ++i)
            {
                if (_wcsicmp(fname, g_ProcessExclusionList[i]) == 0)
                {
                    shouldLogTelemetry = false;
                    break;
                }
            }
        }
    }

    // Throttle to 1/4 of the telemetry events actually being active.
    if (shouldLogTelemetry)
    {
        shouldLogTelemetry = GetCurrentTime() % 4 == 1;
        telemetryThrottledByChance = !shouldLogTelemetry;
    }

    TraceLoggingRegister(g_hTraceLoggingProv);
}

TraceLoggingClient::~TraceLoggingClient()
{
    TraceLoggingUnregister(g_hTraceLoggingProv);
}

NodeTelemetryProvider* TraceLoggingClient::GetNodeTelemetryProvider()
{
    return &node;
}

void TraceLoggingClient::ResetTelemetryStats(ThreadContext* threadContext)
{
    if (threadContext != NULL)
    {
        threadContext->ResetLangStats();
#ifdef ENABLE_DIRECTCALL_TELEMETRY
        threadContext->directCallTelemetry.Reset();
#endif
    }
}

bool TraceLoggingClient::isExperimentalFlagEnabled()
{
#ifdef ENABLE_EXPERIMENTAL_FLAGS
    BOOL regValue;
    if (SUCCEEDED(SettingStore::GetBOOL(SettingStore::IEVALUE_ExperimentalFeatures_ExperimentalJS, &regValue)))
    {
        return regValue != FALSE;
    }
#endif
    return false;
}

void TraceLoggingClient::FireChakraInitTelemetry(DWORD host, bool isJSRT)
{
    if (!this->throttle.isThrottled())
    {
        TraceLogChakra(
            TL_CHAKRAINIT,
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingBool(this->isExperimentalFlagEnabled(), "experimentalFlagEnabled")
            );
    }
}

// Fired whenever we close a page by navigating away
void TraceLoggingClient::FireSiteNavigation(const char16 *url, GUID activityId, DWORD host, bool isJSRT)
{
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();

    // TODO: use current threadContext to retrieve thread-local data, turn it into
    // a specific schema, then log the event.


    // Workflow of Logging Telemetry
    // 1. Get the Telemetry point either as a value or as a pointer/reference if the structure is too big
    // 2. Report the telemetry 
    // 3. RESET the VALUE to INITIAL Values in the ResetMethod Below. If the value being reported is single scalar then use GetAndReset<Telemetry>()
    //    else do resetting in the Tracelogging Telemetry Stats API.

    //******** Treat this AS things to think about while adding NEW Telemetry****************
    // 1. Update the Reset count of your telemetry, if applicable
    // 2. Is it a perf metric, does it makes sense to report data when ScriptContext is in debug mode?
    // 3. If so then using isAnyScriptContextInDebugMode (variable defined below) would suffice?


    // printing GC Pause stats
    if (threadContext != NULL)
    {
        if (url != NULL && (CONFIG_ISENABLED(Js::ES6LangTelFlag) || CONFIG_ISENABLED(Js::ES5LangTelFlag)))
        {
            Output::Print(_u("Navigated from site: %s\n"), url);
        }

        if (!this->throttle.isThrottled())
        {

#ifdef ENABLE_DIRECTCALL_TELEMETRY
            // This is called inside a block that just checked whether we should
            // throttle or not - no need to check again in the callee so call the function
            // that just directly logs instead.

            // Note: the IE perf team uses this heavily, so don't touch it
            FireDomTelemetry(activityId);
#endif

            ResetTelemetryStats(threadContext);
        }
    }
}

#ifdef ENABLE_DIRECTCALL_TELEMETRY
void TraceLoggingClient::FirePeriodicDomTelemetry(GUID activityId)
{
    if (!this->throttle.isThrottled())
    {
        FireDomTelemetry(activityId);
    }
}

void TraceLoggingClient::FireDomTelemetry(GUID activityId)
{
    void *data;
    uint16 dataSize;
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    threadContext->directCallTelemetry.GetBinaryData(&data, &dataSize);

    TraceLogChakraNoProbThrottle(
        TL_DIRECTCALLRAW,
        TraceLoggingGuid(activityId, "activityId"),
        TraceLoggingUInt64(threadContext->directCallTelemetry.GetFrequency(), "Frequency"),
        TraceLoggingUInt64(reinterpret_cast<uint64>(threadContext->GetTridentLoadAddress()), "TridentLoadAddress"),
        TraceLoggingBinary(data, dataSize, "Data")
        );
}
#endif

#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
void TraceLoggingClient::FireDomTelemetryStats(double tracelogTimeMs, double logTimeMs)
{
    TraceLogChakra(
        TL_DIRECTCALLTIME,
        TraceLoggingFloat64(tracelogTimeMs, "TraceLogTimeInMs"),
        TraceLoggingFloat64(logTimeMs, "MaxLogTimeInMs")
        );
}
#endif


CEventTraceProperties::CEventTraceProperties()
    : m_pEventTraceProperties(reinterpret_cast<EVENT_TRACE_PROPERTIES*>(m_rgData))
{
    memset(m_rgData, 0, sizeof(m_rgData));

    // Configure core structure
    m_pEventTraceProperties->Wnode.BufferSize = sizeof(m_rgData);
    m_pEventTraceProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;

    m_pEventTraceProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    m_pEventTraceProperties->LoggerNameOffset = m_pEventTraceProperties->LogFileNameOffset + s_cbLogFileName;

    // Set up defaults
    m_pEventTraceProperties->Wnode.ClientContext = 1; // EVENT_TRACE_CLOCK_PERFCOUNTER;
    m_pEventTraceProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
}


CEventTraceProperties::operator EVENT_TRACE_PROPERTIES*()
{
    return m_pEventTraceProperties;
}

EVENT_TRACE_PROPERTIES& CEventTraceProperties::Properties()
{
    return *m_pEventTraceProperties;
}

HRESULT CEventTraceProperties::SetLogFileName(_In_ LPCWSTR wszLogFileName)
{
    WCHAR* pDestination = reinterpret_cast<WCHAR*>(m_rgData + m_pEventTraceProperties->LogFileNameOffset);
    return StringCbCopy(pDestination, s_cbLogFileName, wszLogFileName);
}

HRESULT CEventTraceProperties::SetLoggerName(_In_ LPCWSTR wszLoggerName)
{
    WCHAR* pDestination = reinterpret_cast<WCHAR*>(m_rgData + m_pEventTraceProperties->LoggerNameOffset);
    return StringCbCopy(pDestination, s_cbLoggerName, wszLoggerName);
}

CEtwSession::CEtwSession(_In_ LPCWSTR wszLoggerName, _In_ LPCWSTR wszLogFileName, _In_ SessionScope sessionScope)
    : m_rgData(),
    m_SessionHandle()
{
    m_rgData.SetLoggerName(wszLoggerName);
    m_rgData.SetLogFileName(wszLogFileName);

    if (sessionScope == SessionScope_InProcess)
    {
        m_rgData.Properties().LogFileMode |= EVENT_TRACE_PRIVATE_LOGGER_MODE | EVENT_TRACE_PRIVATE_IN_PROC;
    }
    else
    {
        m_rgData.Properties().LogFileMode |= EVENT_TRACE_INDEPENDENT_SESSION_MODE;
    }

    {
        ULONG status = StartTrace(&m_SessionHandle, wszLoggerName, m_rgData);

        if (status != ERROR_SUCCESS)
        {
            throw "Error!";
        }
    }
}

CEtwSession::~CEtwSession()
{
    if (m_SessionHandle)
    {
        (void)ControlTrace(m_SessionHandle, NULL, m_rgData, EVENT_TRACE_CONTROL_STOP);

        m_SessionHandle = 0;
    }
}

HRESULT CEtwSession::EnableProvider(_In_ GUID const& ProviderId)
{
    if (!m_SessionHandle)
    {
        return E_UNEXPECTED;
    }

    ULONG status = EnableTraceEx2(m_SessionHandle, &ProviderId, EVENT_CONTROL_CODE_ENABLE_PROVIDER, 0, 0, 0, INFINITE, NULL);

    if (status != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    return S_OK;
}
