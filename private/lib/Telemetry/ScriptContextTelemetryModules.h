/*
 * This file controls which IScriptContextTelemetry modules get loaded.
 * Imported numerous times from ScriptContextTelemetry.cpp
 */


#ifdef TELEMETRY_ESB
#include "ESBuiltIns/ESBuiltinsSCT.h"
#endif
#if defined(REJIT_STATS) && defined(ENABLE_BASIC_TELEMETRY)
#include "Engine/EngineDataSCT.h"
#endif
