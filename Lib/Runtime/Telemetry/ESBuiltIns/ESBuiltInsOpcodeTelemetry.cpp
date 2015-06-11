#include "stdafx.h"

#include "ESBuiltInsOpcodeTelemetry.h"

#ifdef TELEMETRY_ESB

using namespace Js;

ESBuiltInsOpcodeTelemetry::ESBuiltInsOpcodeTelemetry( ESBuiltInsTelemetryProvider& parent ) :
    parentProvider( parent )
{
}


ESBuiltInsOpcodeTelemetry::~ESBuiltInsOpcodeTelemetry()
{
}

void ESBuiltInsOpcodeTelemetry::GetMethodProperty( const Var& instance, const PropertyId& propertyId, const Var& value, const bool successful )
{
    this->parentProvider.OnPropertyEncountered( instance, propertyId, successful );
}

void ESBuiltInsOpcodeTelemetry::GetProperty( const Var& instance, const PropertyId& propertyId, const Var& value, const bool successful )
{
    this->parentProvider.OnPropertyEncountered( instance, propertyId, successful );
}

void ESBuiltInsOpcodeTelemetry::IsInstanceOf( const Var& instance, const Var& constructorFunction, const Var& result )
{
    // TODO: Investigate the use of `instanceof` for feature detection, and if commonly used, add telemetry here.
}

void ESBuiltInsOpcodeTelemetry::TypeOfProperty( const Var& instance, const PropertyId& propertyId, const Var& value, const Var& result, const bool successful )
{
    this->parentProvider.OnPropertyEncountered( instance, propertyId, successful );
}

void ESBuiltInsOpcodeTelemetry::IsIn( const Js::Var& instance, const Js::PropertyId& propertyId, const bool successful )
{
    this->parentProvider.OnPropertyEncountered( instance, propertyId, successful );
}

void ESBuiltInsOpcodeTelemetry::NewScriptObject( const Var& constructorFunction, const Arguments& arguments, const Var& constructedInstance )
{
    this->parentProvider.OnConstructorCalled( constructorFunction );
}

#endif