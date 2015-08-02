//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "TelemetryPch.h"
#include "Runtime.h"

#ifdef ENABLE_DIRECTCALL_TELEMETRY

DirectCallTelemetry::DirectCallTelemetry(ThreadContext *threadContext)
    : m_threadContext(threadContext)
{
    QueryPerformanceFrequency(&m_frequency);
    m_directCallTimes = AnewArray(threadContext->GetThreadAlloc(), DirectCallTime, BufferCount);
    Reset();
}

bool DirectCallTelemetry::ShouldLogDirectCall()
{
    if (++m_iterationCount >= LogPeriod)
    {
        m_iterationCount = 0;
        return true;
    }
    else
    {
        return false;
    }
}


void DirectCallTelemetry::LogStart(void *methodPtr)
{
    m_currentCall = methodPtr;
    QueryPerformanceCounter(&m_startTime);
}

void DirectCallTelemetry::LogFinish(void *methodPtr, Js::Arguments *args)
{
    // Check for a matched start/end.
    if (m_currentCall == methodPtr)
    {
        LARGE_INTEGER endTime;
        QueryPerformanceCounter(&endTime);
        Assert(m_bufferIndex < BufferCount);

        uint typeId = 0;
        if (args->Info.Count > 0)
        {
            Js::Var &thisVar = args->Values[0];
            typeId = static_cast<uint>(Js::JavascriptOperators::GetTypeId(thisVar));
        }
        m_directCallTimes[m_bufferIndex].m_typeId = typeId;
        m_directCallTimes[m_bufferIndex].m_directCallAddress = m_currentCall;
        m_directCallTimes[m_bufferIndex].m_elapsedTime = endTime.QuadPart - m_startTime.QuadPart;

        if (++m_bufferIndex >= BufferCount)
        {
            m_bufferIndex = 0;
            m_bufferWrapped = true;
        }

#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
        LARGE_INTEGER logEndTime, tmpTime;
        QueryPerformanceCounter(&logEndTime);
        tmpTime.QuadPart = logEndTime.QuadPart - endTime.QuadPart;
        if (tmpTime.QuadPart > m_maxLogTicks.QuadPart)
        {
            m_maxLogTicks.QuadPart = tmpTime.QuadPart;
        }

#endif

        if (ShouldDoPeriodicLogging(endTime))
        {
#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
            LARGE_INTEGER logStart = { 0 }, logEnd = { 0 };
            if(CONFIG_FLAG(DirectCallTelemetryStats))
            {
                QueryPerformanceCounter(&logStart);
            }
#endif

            ++m_countLogs;
            g_TraceLoggingClient->FirePeriodicDomTelemetry(m_threadContext->activityId);
            m_bufferIndex = 0;
            m_bufferWrapped = false;
            m_lastLogTime = endTime;

#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
            if(CONFIG_FLAG(DirectCallTelemetryStats))
            {
                QueryPerformanceCounter(&logEnd);
                double tracelogTimeMs = (logEnd.QuadPart - logStart.QuadPart) * 1000.0 / GetFrequency();
                double maxLogTimeMs = m_maxLogTicks.QuadPart * 1000.0 / GetFrequency();
                g_TraceLoggingClient->FireDomTelemetryStats(tracelogTimeMs, maxLogTimeMs);
            }
#endif
        }
    }
}

void DirectCallTelemetry::GetBinaryData(void **data, uint16* size)
{
    *data = m_directCallTimes;
    size_t tmp = sizeof(DirectCallTime) * (m_bufferWrapped ? BufferCount : m_bufferIndex);
    *size = static_cast<uint16>(tmp);
}

void DirectCallTelemetry::Reset()
{
    m_bufferIndex = 0;
    m_bufferWrapped = false;
    m_countLogs = 0;
    m_currentCall = nullptr;

    // Initialize the iteration counter to a random value so we have a chance of sampling
    // calls early in the page execution.
    m_iterationCount = m_threadContext->GetRandomNumber() % LogPeriod;

    QueryPerformanceCounter(&m_lastLogTime);
    memset(m_directCallTimes, 0, sizeof(m_directCallTimes));
#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
    m_maxLogTicks.QuadPart = 0;
    m_countUnknownCalls = 0;
#endif
}

uint64 DirectCallTelemetry::GetFrequency()
{
    return m_frequency.QuadPart;
}

bool DirectCallTelemetry::ShouldDoPeriodicLogging(LARGE_INTEGER currentTime)
{
    uint64 ticks = currentTime.QuadPart - m_lastLogTime.QuadPart;
    double seconds = ticks * 1.0 / m_frequency.QuadPart;

    // Log if all of the following are true:
    //  * we've filled the data buffer
    //  * we've logged less than MaxNumberLogs (10) times
    //  * it's been LogDelaySeconds (5 min) since the last periodic log
    return m_bufferWrapped && m_countLogs < MaxNumberLogs && seconds >= LogDelaySeconds;
}

DirectCallTelemetry::AutoLogger::AutoLogger(Js::ScriptContext *scriptContext, Js::JavascriptExternalFunction *func, Js::Arguments *args)
    : m_methodPtr(nullptr), m_args(args)
{
    m_directCallTelemetry = &scriptContext->GetThreadContext()->directCallTelemetry;
    m_shouldLog = m_directCallTelemetry->ShouldLogDirectCall();
    m_methodPtr = func->GetNativeMethod();
    if (m_shouldLog)
    {
        m_directCallTelemetry->LogStart(m_methodPtr);
    }
}

DirectCallTelemetry::AutoLogger::~AutoLogger()
{
    if (m_shouldLog)
    {
        m_directCallTelemetry->LogFinish(m_methodPtr, m_args);
    }
}

#endif
