#include "stdafx.h"
#include "ScriptContextTelemetry.h"

#include "ESBuiltIns\ESBuiltInsTelemetryProvider.h"

#ifdef TELEMETRY

using namespace Js;

ScriptContextTelemetry::ScriptContextTelemetry( ScriptContext& scriptContext ) :
    scriptContext( scriptContext ),
    telemetryProviders( scriptContext.TelemetryAllocator() ),

    opcodeTelemetry( *this )
{
    // Register each Telemetry provider here.
#ifdef TELEMETRY_ESB
    {
        this->telemetryProviders.Add( Anew( scriptContext.TelemetryAllocator(), ESBuiltInsTelemetryProvider, *this ) );
    }
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

void ScriptContextTelemetry::OutputTelemetry()
{
    this->ForEachTelemetryProvider(
        [&]( IScriptContextTelemetryProvider* tp )
        {
#ifndef TELEMETRY_TRACELOGGING
            {
                Output::Print(L"\r\n");
                Output::Print( tp->GetDisplayName() );
                Output::Print(L"\r\n");
            }
#endif
            tp->Output();
        }
    );
}

#endif