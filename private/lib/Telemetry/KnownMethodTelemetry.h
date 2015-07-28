#pragma once

#ifdef ENABLE_BASIC_TELEMETRY

#include "IOpcodeTelemetry.h"

class ScriptContextTelemetry;

#include "TelemetryList.includes.h"

/// <summary>Responsible for invoking telemetry providers from known methods, e.g. `Date.parse`.</summary>
/// <remarks>This exists to hide implementation details of telemetry providers from product code.</remarks>
class KnownMethodTelemetry
{
private:
    ScriptContextTelemetry& scriptContextTelemetry;

public:
    
    /////////////////////
    // Begin list of KnownMethodTelemetry instances (this is not a List<IOpcodeTelemetry> for performance reasons, though this does complicate class definitions and #include order).

#ifdef TELEMETRY_DateParse
    DateParseTelemetryProvider* dateParseTelemetryProvider;
#endif

    // End list of KnownMethodTelemetry instances
    /////////////////////

    KnownMethodTelemetry( ScriptContextTelemetry& telemetry );
    KnownMethodTelemetry(const ScriptContextTelemetry& telemetry ) = delete; // `= delete` disables the copy-constructor.
    ~KnownMethodTelemetry();

    void JavascriptDate_ParseHelper(Js::ScriptContext* scriptContext, Js::JavascriptString* str, double returnValue, bool exceptionRaised);
};

#endif