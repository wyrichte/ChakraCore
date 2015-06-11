#pragma once

#include "IScriptContextTelemetryProvider.h"

#include "OpcodeTelemetry.h"

typedef JsUtil::List<IScriptContextTelemetryProvider*,ArenaAllocator> TelemetryProviderList;

/// <summary>Provides access to telemetry objects associated with a ScriptContext.</summary>
class ScriptContextTelemetry final
{
private:
    Js::ScriptContext& scriptContext;
    TelemetryProviderList telemetryProviders;
    
    // Telemetry aspects: (so far, only OpcodeTelemetry is implemented)
    OpcodeTelemetry opcodeTelemetry;
    
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

    void Initialize();

    void OutputTelemetry();
};

