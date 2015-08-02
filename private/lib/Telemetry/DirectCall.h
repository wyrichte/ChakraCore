//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#ifdef ENABLE_DIRECTCALL_TELEMETRY

// One-byte packing because these structures are bit-by-bit copied into telemetry data.
#pragma pack(push, 1)
struct DirectCallTime
{
    uint64 m_elapsedTime;
    void *m_directCallAddress;
    uint m_typeId;
};
#pragma pack(pop)

class DirectCallTelemetry
{
    ThreadContext *m_threadContext;
    LARGE_INTEGER m_frequency;

    // Data buffer parameters.
    static const int MaxDataBufferSize = 8192;
    static const int BufferCount = MaxDataBufferSize / sizeof(DirectCallTime);
    DirectCallTime *m_directCallTimes;

    // Current index into the call time array
    int m_bufferIndex;
    bool m_bufferWrapped;

    // Handles the frequency with which logging is done
    static const int LogPeriod = 1000;
    int m_iterationCount;


    // TraceLog call rate limiting
    static const int MaxNumberLogs = 10;
    static const int LogDelaySeconds = 300;
    LARGE_INTEGER m_lastLogTime;
    int m_countLogs;
    bool ShouldDoPeriodicLogging(LARGE_INTEGER currentTime);

    // Data about the call currently being logged.
    LARGE_INTEGER m_startTime;
    void * m_currentCall;


#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
    uint m_countUnknownCalls;
    LARGE_INTEGER m_maxLogTicks;
#endif

    void LogStart(void *methodPtr);
    void LogFinish(void *methodPtr, Js::Arguments *args);
    bool ShouldLogDirectCall();
public:

    class AutoLogger
    {
        bool m_shouldLog;
        void *m_methodPtr;
        Js::Arguments *m_args;
        DirectCallTelemetry *m_directCallTelemetry;
    public:
        AutoLogger(Js::ScriptContext *scriptContext, Js::JavascriptExternalFunction *func, Js::Arguments *args);
        ~AutoLogger();
    };

    DirectCallTelemetry(ThreadContext*);
    void GetBinaryData(void ** data, uint16* size);
    void Reset();

    uint64 GetFrequency();
};

#endif