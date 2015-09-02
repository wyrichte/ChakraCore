#pragma once

#include "ScriptContextTelemetry.h"

class ScriptContextTelemetry;

__interface IScriptContextTelemetryProvider
{
public:
    
    /// <remarks>This isn't const as to allow implementations to self-mutate.</remarks>
    void OutputPrint();
    
    void OutputTraceLogging(GUID activityId, DWORD hostType, bool isJSRT);
};