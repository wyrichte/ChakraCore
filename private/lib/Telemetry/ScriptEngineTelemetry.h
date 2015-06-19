#pragma once

/// <summary>Provides static methods that last for the life of a ScriptEngine instance, granting inter-ScriptContext telemetry logic.</summary>
class ScriptEngineTelemetry
{
private:
    // Delete constructors, this is a static class.
    ScriptEngineTelemetry() = delete;
    ScriptEngineTelemetry(const ScriptEngineTelemetry& copy) = delete;

    static bool isInitialized;

public:
    static void Initialize();

    static void Cleanup();
};