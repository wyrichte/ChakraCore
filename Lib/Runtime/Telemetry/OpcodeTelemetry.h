#pragma once

#include "IOpcodeTelemetry.h"

class ScriptContextTelemetry;

#include "TelemetryList.includes.h"

/// <summary>Responsible for invoking OpcodeTelemetry implementations and providing Opcode-level telemetry data. A new
/// OpcodeTelemetry is instantiated for each ScriptContext instance.</summary>
class OpcodeTelemetry : public IOpcodeTelemetry
{
private:
    
    ScriptContextTelemetry& scriptContextTelemetry;

    // Program location
    int programLocationFunctionId;
    int programLocationBytecodeOffset;

public:
    
#define IOPCODETELEMETRY(typeName,field) typeName##* field##;
#include "TelemetryList.inc"
#undef IOPCODETELEMETRY

    OpcodeTelemetry(ScriptContextTelemetry& telemetry);
    OpcodeTelemetry(const OpcodeTelemetry& copy) = delete; // `= delete` disables the copy-constructor.
    ~OpcodeTelemetry();

    /// <summary>Gets a reference to the parent ScriptContextTelemetry object which owns this OpcodeTelemetry object.</summary>
    ScriptContextTelemetry& GetScriptContextTelemetry();
    /// <summary>Shortcut method to get a reference to the ScriptContext that owns the parent ScriptContextTelemetry object (which owns this OpcodeTelemetry object).</summary>
    const Js::ScriptContext&      GetScriptContext();

    /// <summary>Informs the OpcodeTelemetry of the current opcode's parent function ID and bytecode offset within the function - together, these uniquely identify a point in a script within a script context.</summary>
    void ProgramLocationFunctionId(const int functionId); // setter
    int  ProgramLocationFunctionId() const; // getter

    void ProgramLocationBytecodeOffset(const int bytecodeOffset); // setter
    int  ProgramLocationBytecodeOffset() const; // getter

    void GetMethodProperty(const Js::Var& instance, const Js::PropertyId& propertyId, const Js::Var& value, const bool successful) override;
    void GetProperty      (const Js::Var& instance, const Js::PropertyId& propertyId, const Js::Var& value, const bool successful) override;
    void IsInstanceOf     (const Js::Var& instance, const Js::Var& constructorFunction, const Js::Var& result) override;
    void TypeOfProperty   (const Js::Var& instance, const Js::PropertyId& propertyId, const Js::Var& value, const Js::Var& result, const bool successful) override;
    void IsIn             (const Js::Var& instance, const Js::PropertyId& propertyId, const bool successful) override;
    void NewScriptObject  (const Js::Var& constructorFunction, const Js::Arguments& arguments, const Js::Var& constructedInstance) override;
};

