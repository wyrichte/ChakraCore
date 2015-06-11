#include "stdafx.h"
#include "OpcodeTelemetry.h"

#include "ScriptContextTelemetry.h"

#ifdef TELEMETRY

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
#define IOPCODETELEMETRY(typeName,field) this->##field##->GetMethodProperty(instance, propertyId, value, successful);
#include "TelemetryList.inc"
#undef IOPCODETELEMETRY
}

void OpcodeTelemetry::GetProperty( const Var& instance, const PropertyId& propertyId, const Var& value, const bool successful )
{
#define IOPCODETELEMETRY(typeName,field) this->##field##->GetProperty(instance, propertyId, value, successful);
#include "TelemetryList.inc"
#undef IOPCODETELEMETRY
}

void OpcodeTelemetry::IsInstanceOf( const Var& instance, const Var& constructorFunction, const Var& result )
{
#define IOPCODETELEMETRY(typeName,field) this->##field##->IsInstanceOf(instance, constructorFunction, result);
#include "TelemetryList.inc"
#undef IOPCODETELEMETRY
}

void OpcodeTelemetry::TypeOfProperty( const Var& instance, const PropertyId& propertyId, const Var& value, const Var& result, const bool successful )
{
#define IOPCODETELEMETRY(typeName,field) this->##field##->TypeOfProperty(instance, propertyId, value, result, successful);
#include "TelemetryList.inc"
#undef IOPCODETELEMETRY
}

void OpcodeTelemetry::IsIn( const Var& instance, const PropertyId& propertyId, const bool successful )
{
#define IOPCODETELEMETRY(typeName,field) this->##field##->IsIn(instance, propertyId, successful);
#include "TelemetryList.inc"
#undef IOPCODETELEMETRY
}

void OpcodeTelemetry::NewScriptObject( const Var& constructorFunction, const Arguments& arguments, const Var& constructedInstance )
{
#define IOPCODETELEMETRY(typeName,field) this->##field##->NewScriptObject(constructorFunction, arguments, constructedInstance);
#include "TelemetryList.inc"
#undef IOPCODETELEMETRY
}

#endif