#pragma once

#ifdef TELEMETRY_ESB

#include "..\IScriptContextTelemetryProvider.h"

#include "ESBuiltInsOpcodeTelemetry.h"
#include "ESBuiltInsDatabase.h"

class ESBuiltInsTelemetryProvider :
    public IScriptContextTelemetryProvider
{
private:
    ESBuiltInsOpcodeTelemetry opcodeTelemetry;
    byte*                     usageMap;

    void IncrementUseCount(const ESBuiltInPropertyId esBuiltInPropertyId);

    size_t count_OnPropertyEncountered;
    size_t count_OnConstructorCalled;
    size_t count_GetBuiltInTypeNameFunction;
    size_t count_GetBuiltInTypeNameObject;
    size_t count_GetBuiltInTypeNameOther;

#ifdef TELEMETRY_ESB_STRINGS
    ESBuiltInPropertyId GetESBuiltInPropertyId(const Js::JavascriptString* typeName, const Js::PropertyId propertyId);

    Js::JavascriptString* nameMath;
    Js::JavascriptString* nameJson;
#endif

public:
    ESBuiltInsTelemetryProvider(ScriptContextTelemetry& scriptContextTelemetry);
    ~ESBuiltInsTelemetryProvider();

    const wchar_t* const GetDisplayName() const override;

    void Output() override;

    // Built-ins specific telemetry methods below:

    void OnPropertyEncountered( const Js::Var instance, const Js::PropertyId propertyId, const bool successful );
    void OnConstructorCalled( const Js::Var constructorFunction );

    bool IsBuiltInMath(const Js::Var& instance) const;
    bool IsBuiltInJson(const Js::Var& instance) const;
    
    ESBuiltInTypeNameId GetESBuiltInTypeNameId( const Js::Var instance, const Js::PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound );

    ESBuiltInTypeNameId GetESBuiltInTypeNameId_Function( const Js::Var instance, const Js::PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound );
    ESBuiltInTypeNameId GetESBuiltInTypeNameId_Object  ( const Js::Var instance, const Js::PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound );
    ESBuiltInTypeNameId GetESBuiltInTypeNameId_Other   ( const Js::Var instance, const Js::PropertyId propertyId, _Out_ bool& isConstructorProperty, Js::TypeId typeId, _Out_ ESBuiltInPropertyId& shortcutPropertyFound );

//    ESBuiltInTypeNameId GetESBuiltInTypeNameIdForConstructorFunction( const Js::JavascriptFunction* constructor );
};

#endif