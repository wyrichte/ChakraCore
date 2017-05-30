#pragma once

#include "ScriptContextTelemetry.h"

class ScriptContextTelemetry;

__interface IScriptContextTelemetryProvider
{
public:
    
    /// <remarks>This isn't const as to allow implementations to self-mutate.</remarks>
#ifdef TELEMETRY_OUTPUTPRINT
    virtual void OutputPrint();
#endif
};