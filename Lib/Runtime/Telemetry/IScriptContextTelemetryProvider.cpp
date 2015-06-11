#include "stdafx.h"
#include "IScriptContextTelemetryProvider.h"

#ifdef TELEMETRY

IScriptContextTelemetryProvider::IScriptContextTelemetryProvider( ScriptContextTelemetry& scriptContextTelemetry ) :
    scriptContextTelemetry( scriptContextTelemetry )
{
}

/// <remarks>Despite this being a pure virtual (abstract) destructor, an implementation is still required by the linker.</remarks>
IScriptContextTelemetryProvider::~IScriptContextTelemetryProvider()
{
}

#endif