#include "TelemetryPch.h"
#include "KnownMethodTelemetry.h"

#ifdef ENABLE_BASIC_TELEMETRY

using namespace Js;

KnownMethodTelemetry::KnownMethodTelemetry( ScriptContextTelemetry& telemetry ) :
    scriptContextTelemetry( telemetry )
{
}


KnownMethodTelemetry::~KnownMethodTelemetry()
{
}

#endif
