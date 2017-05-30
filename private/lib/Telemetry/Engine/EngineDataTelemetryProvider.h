#pragma once
#if defined(REJIT_STATS) && defined(ENABLE_BASIC_TELEMETRY)

#include "..\IScriptContextTelemetryProvider.h"
#include "telemetry.h"

class EngineDataTelemetryProvider : public IScriptContextTelemetryProvider
{
public:
    EngineDataTelemetryProvider(ScriptContextTelemetry& sc);
#ifdef TELEMETRY_OUTPUTPRINT
    virtual void OutputPrint() override;
#endif
};

#endif