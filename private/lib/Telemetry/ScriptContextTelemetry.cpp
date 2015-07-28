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

void ScriptContextTelemetry::OutputTelemetry()
{
    this->ForEachTelemetryProvider(
        [&]( IScriptContextTelemetryProvider* tp )
        {
#ifdef TELEMETRY_OUTPUTPRINT
            tp->OutputPrint();
#endif
#ifdef TELEMETRY_TRACELOGGING
            tp->OutputTraceLogging();
#endif
        }
    );
}

#endif