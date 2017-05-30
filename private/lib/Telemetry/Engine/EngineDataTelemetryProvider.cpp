#include "TelemetryPch.h"
#include "EngineDataTelemetryProvider.h"

#if defined(REJIT_STATS) && defined(ENABLE_BASIC_TELEMETRY)

EngineDataTelemetryProvider::EngineDataTelemetryProvider(ScriptContextTelemetry & sc)
{
}

#ifdef TELEMETRY_OUTPUTPRINT
void EngineDataTelemetryProvider::OutputPrint()
{
    return;
}
#endif

#endif // defined(REJIT_STATS) && defined(ENABLE_BASIC_TELEMETRY)