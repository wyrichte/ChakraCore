#include "stdafx.h"
#include "ScriptEngineTelemetry.h"

// Include headers for each long-lasting component:
#include "ESBuiltIns\ESBuiltInsDatabase.h"

#ifdef TELEMETRY

bool ScriptEngineTelemetry::isInitialized = false;

void ScriptEngineTelemetry::Initialize()
{
    // This guard is this method might be called multiple times if ScriptEngine instances are created multiple times in the same process.
    if( ScriptEngineTelemetry::isInitialized ) return;
    
    // TODO: Find out if this needs to be thread-safe.

    // The HeapAllocator cannot be used from ScriptEngine --> ScriptEngineTelemetry::Initialize because CanHandleOutOfMemoryError() is false at this point during execution.

    ScriptEngineTelemetry::isInitialized = true;
}

void ScriptEngineTelemetry::Cleanup()
{
#ifdef TELEMETRY_ESB
    ESBuiltInsDatabase::Cleanup();
#endif
}
#endif