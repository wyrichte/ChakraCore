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

void KnownMethodTelemetry::JavascriptDate_ParseHelper(ScriptContext* scriptContext, JavascriptString* str, double returnValue, bool exceptionRaised)
{
#ifdef TELEMETRY_DateParse
    dateParseTelemetryProvider->JavascriptDate_ParseHelper( scriptContext, str, returnValue, exceptionRaised );
#endif
}

#endif