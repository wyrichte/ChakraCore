#pragma once

#include "ScriptContextTelemetry.h"

class ScriptContextTelemetry;

class IScriptContextTelemetryProvider
{
protected:
    ScriptContextTelemetry& scriptContextTelemetry;

public:
    
    /// <summary>Initializes the telemetry provider. When implementing this, the provider will add its telemetry watchers to the ScriptContextTelemetry object for callbacks during script execution.</summary>
    IScriptContextTelemetryProvider(ScriptContextTelemetry& scriptContextTelemetry);

    virtual ~IScriptContextTelemetryProvider() = 0;

    virtual const wchar_t* const GetDisplayName() const = 0;

    /// <remarks>This isn't const as to allow implementations to self-mutate.</remarks>
    virtual void Output() = 0;
};