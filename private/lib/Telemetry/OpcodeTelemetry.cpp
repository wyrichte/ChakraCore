#include "TelemetryPch.h"
#include "OpcodeTelemetry.h"

#include "ScriptContextTelemetry.h"

#ifdef ENABLE_BASIC_TELEMETRY

using namespace Js;

OpcodeTelemetry::OpcodeTelemetry(ScriptContextTelemetry& telemetry) :
    scriptContextTelemetry( telemetry )
{
}

OpcodeTelemetry::~OpcodeTelemetry()
{
}

ScriptContextTelemetry& OpcodeTelemetry::GetScriptContextTelemetry()
{
    return this->scriptContextTelemetry;
}

const Js::ScriptContext& OpcodeTelemetry::GetScriptContext()
{
    return this->scriptContextTelemetry.GetScriptContext();
}

void OpcodeTelemetry::ProgramLocationFunctionId( const int functionId ) { this->programLocationFunctionId = functionId; }
int  OpcodeTelemetry::ProgramLocationFunctionId() const                 { return this->programLocationFunctionId; }

void OpcodeTelemetry::ProgramLocationBytecodeOffset( const int bytecodeOffset ) { this->programLocationBytecodeOffset = bytecodeOffset; }
int  OpcodeTelemetry::ProgramLocationBytecodeOffset() const                     { return this->programLocationBytecodeOffset; }

void OpcodeTelemetry::GetMethodProperty( const Var& instance, const PropertyId& propertyId, const Var& value, const bool successful )
{
    this->esBuiltInsOpcodeTelemetry->GetMethodProperty( instance, propertyId, value, successful );
}

void OpcodeTelemetry::GetProperty( const Var& instance, const PropertyId& propertyId, const Var& value, const bool successful )
{
    this->esBuiltInsOpcodeTelemetry->GetProperty( instance, propertyId, value, successful );
}

void OpcodeTelemetry::IsInstanceOf( const Var& instance, const Var& constructorFunction, const Var& result )
{
    this->esBuiltInsOpcodeTelemetry->IsInstanceOf( instance, constructorFunction, result );
}

void OpcodeTelemetry::TypeOfProperty( const Var& instance, const PropertyId& propertyId, const Var& value, const Var& result, const bool successful )
{
    this->esBuiltInsOpcodeTelemetry->TypeOfProperty( instance, propertyId, value, result, successful );
}

void OpcodeTelemetry::IsIn( const Var& instance, const PropertyId& propertyId, const bool successful )
{
    this->esBuiltInsOpcodeTelemetry->IsIn( instance, propertyId, successful );
}

void OpcodeTelemetry::NewScriptObject( const Var& constructorFunction, const Arguments& arguments, const Var& constructedInstance )
{
    this->esBuiltInsOpcodeTelemetry->NewScriptObject( constructorFunction, arguments, constructedInstance );
}

#endif