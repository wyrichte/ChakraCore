#include "TelemetryPch.h"
#include "ScriptContextTelemetry.h"

#include "ESBuiltIns\ESBuiltInsTelemetryProvider.h"
#include "DateParse\DateParseTelemetryProvider.h"

#ifdef ENABLE_BASIC_TELEMETRY

using namespace Js;

ScriptContextTelemetry::ScriptContextTelemetry( ScriptContext& scriptContext ) :
    scriptContext( scriptContext ),
    telemetryProviders( scriptContext.TelemetryAllocator() ),

    opcodeTelemetry( *this ),
    knownMethodTelemetry( *this )
{
    // Register each Telemetry provider here.
#ifdef TELEMETRY_ESB
       this->telemetryProviders.Add( Anew( scriptContext.TelemetryAllocator(), ESBuiltInsTelemetryProvider, *this ) );
#endif
#ifdef TELEMETRY_DateParse
       this->telemetryProviders.Add( Anew( scriptContext.TelemetryAllocator(), DateParseTelemetryProvider, *this ) );
#endif
}

ScriptContextTelemetry::~ScriptContextTelemetry()
{
}

ScriptContext& ScriptContextTelemetry::GetScriptContext() const
{
    return this->scriptContext;
}

OpcodeTelemetry& ScriptContextTelemetry::GetOpcodeTelemetry()
{
    return this->opcodeTelemetry;
}

KnownMethodTelemetry& ScriptContextTelemetry::GetKnownMethodTelemetry()
{
    return this->knownMethodTelemetry;
}

void ScriptContextTelemetry::OutputPrint()
{
#ifdef TELEMETRY_OUTPUTPRINT
    this->ForEachTelemetryProvider(
        [&]( IScriptContextTelemetryProvider* tp )
        {
            tp->OutputPrint();
        }
    );
#endif
}

void ScriptContextTelemetry::OutputTraceLogging(GUID activityId, DWORD hostType, bool isJSRT)
{
#ifdef TELEMETRY_TRACELOGGING
    this->ForEachTelemetryProvider(
        [&]( IScriptContextTelemetryProvider* tp )
        {
            tp->OutputTraceLogging( activityId, hostType, isJSRT );
        }
    );
#endif
}

#endif