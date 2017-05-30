#pragma once

#ifdef ENABLE_BASIC_TELEMETRY

#include "IScriptContextTelemetryProvider.h"

#include "ESBuiltins/OpcodeTelemetry.h"

typedef JsUtil::List<IScriptContextTelemetryProvider*,ArenaAllocator> TelemetryProviderList;

/// <summary>Provides access to telemetry objects associated with a ScriptContext.</summary>
class ScriptContextTelemetry final
{
private:
    Js::ScriptContext& scriptContext;
    TelemetryProviderList telemetryProviders;
    Throttle throttle;

    // Keep local references to the telemetry providers for their use later
#define SCT_STATE -3
#include "ScriptContextTelemetryModules.h"
#undef SCT_STATE
    
    // Telemetry aspects:
    OpcodeTelemetry opcodeTelemetry; // Telemetry on specific interpreter opcodes - requires callbacks added to each opcode handler in InterpreterStackFrame
    
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

    void OutputPrint();
	void OutputTraceLogging(GUID activityId, DWORD hostType, bool isJSRT);
};

#endif