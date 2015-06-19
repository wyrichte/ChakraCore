#pragma once

#ifdef TELEMETRY_ESB

#include "..\OpcodeTelemetry.h"

class ESBuiltInsTelemetryProvider;

class ESBuiltInsOpcodeTelemetry :
    public IOpcodeTelemetry
{
private:
    ESBuiltInsTelemetryProvider& parentProvider;

public:
    ESBuiltInsOpcodeTelemetry(ESBuiltInsTelemetryProvider& parent);
    ~ESBuiltInsOpcodeTelemetry();

    void GetMethodProperty(const Js::Var& instance, const Js::PropertyId& propertyId  , const Js::Var& value,                        const bool successful) override;
    void GetProperty      (const Js::Var& instance, const Js::PropertyId& propertyId  , const Js::Var& value,                        const bool successful) override;
    void IsInstanceOf     (const Js::Var& instance, const Js::Var& constructorFunction,                       const Js::Var& result                       ) override;
    void TypeOfProperty   (const Js::Var& instance, const Js::PropertyId& propertyId  , const Js::Var& value, const Js::Var& result, const bool successful) override;
    void IsIn             (const Js::Var& instance, const Js::PropertyId& propertyId  ,                                              const bool successful) override;
    void NewScriptObject  (const Js::Var& constructorFunction, const Js::Arguments& arguments, const Js::Var& constructedInstance) override;
};

#include "ESBuiltInsTelemetryProvider.h"

#endif