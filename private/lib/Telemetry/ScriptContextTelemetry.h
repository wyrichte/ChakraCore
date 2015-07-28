#pragma once

#ifdef ENABLE_BASIC_TELEMETRY

#include "IScriptContextTelemetryProvider.h"

#include "OpcodeTelemetry.h"
#include "KnownMethodTelemetry.h"

typedef JsUtil::List<IScriptContextTelemetryProvider*,ArenaAllocator> TelemetryProviderList;

/// <summary>Provides access to telemetry objects associated with a ScriptContext.</summary>
class ScriptContextTelemetry final
{
private:
    Js::ScriptContext& scriptContext;
    TelemetryProviderList telemetryProviders;
    
    // Telemetry aspects:
    OpcodeTelemetry opcodeTelemetry; // Telemetry on specific interpreter opcodes - requires callbacks added to each opcode handler in InterpreterStackFrame
    KnownMethodTelemetry knownMethodTelemetry; // Telemetry on specific known method calls - requires callbacks added to each desired known method
    
    template<typename Func>
    void ForEachTelemetryProvider( Func callback )
    {
        int count = this->telemetryProviders.Count();
        for( int i = 0; i < count; i++ )
        {
            IScriptContextTelemetryProvider* telemetryProvider = this->telemetryProviders.Item(i);
            callback( telemetryProvider );
        }
    }

public:
    ScriptContextTelemetry(Js::ScriptContext& scriptContext);
    ~ScriptContextTelemetry();

    Js::ScriptContext& GetScriptContext() const;
    
    OpcodeTelemetry& GetOpcodeTelemetry();
    KnownMethodTelemetry& GetKnownMethodTelemetry();

    void Initialize();

    void OutputTelemetry();
};

#endif