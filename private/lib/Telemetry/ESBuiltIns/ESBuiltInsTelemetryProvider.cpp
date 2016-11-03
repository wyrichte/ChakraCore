#include "TelemetryPch.h"
#include "ESBuiltInsTelemetryProvider.h"

#ifdef TELEMETRY_ESB

#include "..\TelemetryMacros.h"

using namespace Js;

ESBuiltInsTelemetryProvider::ESBuiltInsTelemetryProvider( ScriptContextTelemetry& scriptContextTelemetry ) :
    scriptContextTelemetry( scriptContextTelemetry ),
    opcodeTelemetry( *this ),
    usageMap( ESBuiltInsDatabase::CreateUsageMap( *scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) )
#ifdef TELEMETRY_ESB_STRINGS
    ,
    nameMath( nullptr ), // JavascriptString::NewCopySzFromArena(_u("Math"), &scriptContextTelemetry.GetScriptContext(), scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) ),
    nameJson( nullptr ) // JavascriptString::NewCopySzFromArena(_u("JSON"), &scriptContextTelemetry.GetScriptContext(), scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) )
#endif
{
    // Register the opcodeTelemetry callback handler:
    scriptContextTelemetry.GetOpcodeTelemetry().esBuiltInsOpcodeTelemetry = &this->opcodeTelemetry;
}


ESBuiltInsTelemetryProvider::~ESBuiltInsTelemetryProvider()
{
    ESBuiltInsDatabase::FreeUsageMap( *scriptContextTelemetry.GetScriptContext().TelemetryAllocator(), this->usageMap );
}

void ESBuiltInsTelemetryProvider::OnPropertyEncountered( const Var instance, const PropertyId propertyId, const bool successful )
{
    count_OnPropertyEncountered++;
    
    // 0. If the propertyId is beyond the list of known properties, then abort, we're not interested!
    // 1. Get the ESBuiltInTypeId of `instance`.
    // 1.1. If the Js::TypeId is immediately mappable, get that.
    // 1.2. Otherwise, look up the type name as a string.
    // 1.3. If type name cannot be processed (object has no type name, name not found in database, etc), then abort and return.
    // 2. Get the ESBuiltInPropertyId
    // 2.1. Combine ESBuiltInTypeId and PropertyId together to form the ESBuiltInPropertyIdKey
    // 2.2. Get the ESBuiltInPropertyId from ESBuiltInPropertyIdKey
    // 3. Increment the usage of that member.

    if( propertyId >= Js::PropertyIds::_countJSOnlyProperty ) return;

    ESBuiltInPropertyId esbiPropertyId = ESBuiltInPropertyId::_None;
    bool isConstructorProperty;
    ESBuiltInTypeNameId esbiTypeNameId = this->GetESBuiltInTypeNameId( instance, propertyId, isConstructorProperty, esbiPropertyId );
    
    if( esbiTypeNameId == ESBuiltInTypeNameId::_None )
        return;

    if( esbiPropertyId == ESBuiltInPropertyId::_None ) // `esbiPropertyId` can be found via shortcut.
        esbiPropertyId = ESBuiltInsDatabase::GetESBuiltInPropertyId( esbiTypeNameId, isConstructorProperty, propertyId );

    if( esbiPropertyId != ESBuiltInPropertyId::_None )
        this->IncrementUseCount( esbiPropertyId );
}

void ESBuiltInsTelemetryProvider::OnConstructorCalled( const Js::Var constructorFunction )
{
    count_OnConstructorCalled++;
    
    // Ensure the constructor function is a valid function.
    // Get the name of the function, and get the ESBuiltInPropertyId for its constructor, and increment it.

    if( JavascriptFunction::Is( constructorFunction ) )
    {
        JavascriptFunction* function = JavascriptFunction::FromVar( constructorFunction );

        ESBuiltInTypeNameId esbiTypeNameId;
#ifdef TELEMETRY_ESB_STRINGS
        {
            JavascriptString* functionName = function->GetDisplayName( true );

            esbiTypeNameId = ESBuiltInsDatabase::GetESBuiltInTypeNameId( functionName );
        }
#else
        {
            esbiTypeNameId = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( this->scriptContextTelemetry.GetScriptContext(), function );
        }
#endif

        if( esbiTypeNameId != ESBuiltInTypeNameId::_None )
        {
            ESBuiltInPropertyId ctorInvocation = ESBuiltInsDatabase::GetESBuiltInPropertyId( esbiTypeNameId, /*isConstructorProperty:*/ true, PropertyIds::__constructor );
            if( ctorInvocation != ESBuiltInPropertyId::_None )
            {
                this->IncrementUseCount( ctorInvocation );
            }
        }
    }
    
}

/// <remarks>Even if <param ref="esBuiltInPropertyId" /> is `_None` it should be logged to ensure the telemetry code is operational.</remarks>
void ESBuiltInsTelemetryProvider::IncrementUseCount(const ESBuiltInPropertyId esBuiltInPropertyId)
{
    size_t idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( esBuiltInPropertyId );
    if( idx == SIZE_MAX ) return;
    byte* countPtr = &this->usageMap[ idx ];
    byte  count    = *countPtr;
    if( count++ == BYTE_MAX ) return;
    *countPtr = count;
}

bool ESBuiltInsTelemetryProvider::IsBuiltInMath(const Var& instance) const
{
    DynamicObject* math = this->scriptContextTelemetry.GetScriptContext().GetLibrary()->GetMathObject();

    return instance == math;
}

bool ESBuiltInsTelemetryProvider::IsBuiltInJson(const Var& instance) const
{
    DynamicObject* json = this->scriptContextTelemetry.GetScriptContext().GetLibrary()->GetJSONObject();

    return instance == json;
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId_Function( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    count_GetBuiltInTypeNameFunction++;
    
    // BUG: the property-accession `String.prototype` causes this method to return `(isConstructor: false, typeId: ESBuiltInTypeNameId::Function)` because `prototype` is a member of Function.
    // However it should return `(isConstructor: true, typeId: ESBuiltInTypeNameId::String)`

//#ifdef NEVER
    // `instance` could be accessing a property of Function (e.g. Function.bind), or it could be accessing a Constructor Property (e.g. Array.isArray).
    // So see if the PropertyId belongs to Function, and if so, return immediately before doing a name-check.

    ESBuiltInPropertyId definedOnFunction;
    definedOnFunction = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Function, /*isConstructorProperty:*/ false, propertyId );
    if( definedOnFunction != ESBuiltInPropertyId::_None )
    {
        isConstructorProperty = false;
        shortcutPropertyFound = definedOnFunction;
        return ESBuiltInTypeNameId::Function;
    }

    definedOnFunction = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Function, /*isConstructorProperty:*/ true, propertyId );
    if( definedOnFunction != ESBuiltInPropertyId::_None ) {
        // There is a bug here in that `Function_Constructor_length` or `Function_Constructor_prototype` will never be recorded as a Constructor property, as the names are also used by instances, and this code has no way of knowing if this is *a* function, or *the* Function function.
        isConstructorProperty = true;
        shortcutPropertyFound = definedOnFunction;
        return ESBuiltInTypeNameId::Function;
    }
//#endif

    // PropertyId does not belong to Function, therefore this is a Constructor property. But which Constructor is it?
    // There are two ways to solve this:
    // 1. Look up the TypeNameId by string (i.e. the name of the function), "Array.isArray" -> "Array" -> the global Array constructor function.
    // 2. Compare the pointer to the function to known pointers, `if( function == library.ArrayConstructor )`

    // Method 1 has the advantage of working to detect polyfills, but is slow as string comparisons, even when using a Trie, require lots of steps, repeatedly.
    // Method 2 only detects usage of implemented and non-overrriden built-ins, but is a lot faster.

    const JavascriptFunction* instanceAsFunction = JavascriptFunction::FromVar( instance );

    isConstructorProperty = true;

    ESBuiltInTypeNameId ret = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( this->scriptContextTelemetry.GetScriptContext(), instanceAsFunction );
    return ret;
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId_Object( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    count_GetBuiltInTypeNameObject++;
    
    if( this->IsBuiltInMath( instance ) ) return ESBuiltInTypeNameId::Math;
    if( this->IsBuiltInJson( instance ) ) return ESBuiltInTypeNameId::JSON;
    
    // `instance` could be a Polyfill Object. Inspect the name of its Constructor to see if it matches (e.g. `function DataView() { this.someDataViewProperty = 'abc'; };` ).
    // However, an optimization: don't name-check if the propertyId already belongs to Object (e.g. hasOwnProperty).

    ESBuiltInPropertyId definedOnObject = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Object, /*isConstructorProperty:*/ false, propertyId ); // `isConstructorProperty` will always be false because `instance` is not a function and so cannot be a constructor.
    if( definedOnObject != ESBuiltInPropertyId::_None )
    {
        isConstructorProperty = false;
        shortcutPropertyFound = definedOnObject;
        return ESBuiltInTypeNameId::Object;
    }

#ifdef TELEMETRY_ESB_GetConstructorPropertyPolyfillDetection
    {
        // PropertyId does not belong to Object, so get the object's constructor name and see if it's a built-in. This happens in the case of polyfilled constructors.

        // Get the Constructor property, as per DiagObjectModel.cpp, RecyclableObjectDisplay::Type() (line 1669)
        RecyclableObject*  instanceAsPropertyObject = RecyclableObject::FromVar( instance ); // I'm not sure why this is necessary.
        Var                value;
        PropertyValueInfo* info = nullptr;

        ScriptContext* scriptContextPtr = const_cast<ScriptContext*>( &this->scriptContextTelemetry.GetScriptContext() );

        bool ok = JavascriptOperators::GetProperty( instance, instanceAsPropertyObject, PropertyIds::constructor, &value, scriptContextPtr, info ) == TRUE;
        if( ok && JavascriptFunction::Is( value ) )
        {
            JavascriptFunction* constructorFunction = JavascriptFunction::FromVar( value );
            {
                ESBuiltInTypeNameId ret = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( this->scriptContextTelemetry.GetScriptContext(), constructorFunction );
                return ret;
            }
        }
        else
        {
            return ESBuiltInTypeNameId::_None;
        }
    }
#else
    {
        // Skip checking for polyfills, abort further processing.
        return ESBuiltInTypeNameId::_None;
    }
#endif
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId_Other( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, TypeId typeId, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    count_GetBuiltInTypeNameOther++;
    
    ESBuiltInTypeNameId esbiTypeNameId = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByTypeId( typeId );
    if( esbiTypeNameId == ESBuiltInTypeNameId::_None ) return esbiTypeNameId;

    // See if the specified property is defined on the identified type, if not, see if it's defined for Object (and so, is inherited):
    ESBuiltInPropertyId definedOnType = ESBuiltInsDatabase::GetESBuiltInPropertyId( esbiTypeNameId, /*isConstructorProperty:*/ false, propertyId );
    if( definedOnType == ESBuiltInPropertyId::_None )
    {
        ESBuiltInPropertyId definedOnObject = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Object, /*isConstructorProperty:*/ false, propertyId ); // `isConstructorProperty` will always be false because `instance` is not a function and so cannot be a constructor.
        if( definedOnObject != ESBuiltInPropertyId::_None )
        {
            shortcutPropertyFound = definedOnObject;
            return ESBuiltInTypeNameId::Object;
        }
    }
    else
    {
        shortcutPropertyFound = definedOnType;
    }

    return esbiTypeNameId;
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    isConstructorProperty = false;
    
    if( instance == nullptr) return ESBuiltInTypeNameId::_None;

    Js::TypeId typeId = JavascriptOperators::GetTypeId( instance );
    
    // Special-cases are needed for Functions and Objects because we don't know what their real type is.
    if( typeId == TypeId::TypeIds_Function )
    {
        return this->GetESBuiltInTypeNameId_Function( instance, propertyId, isConstructorProperty, shortcutPropertyFound );
    }
    else if( typeId == TypeId::TypeIds_Object )
    {
        return this->GetESBuiltInTypeNameId_Object( instance, propertyId, isConstructorProperty, shortcutPropertyFound );
    }
    else
    {
        return this->GetESBuiltInTypeNameId_Other( instance, propertyId, isConstructorProperty, typeId, shortcutPropertyFound );
    }
}

void ESBuiltInsTelemetryProvider::OutputTraceLogging(GUID activityId, DWORD hostType, bool isJSRT)
{

    if (!this->throttle.isThrottled())
    {
        size_t idx;

#define Get(propertyId) \
    ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->usageMap[ idx ] : 0 )

        TraceLogChakra("ESBuiltIns" /* #0 */,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingUInt32(hostType, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Array_Prototype_contains), "Array_Prototype_contains"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Array_Prototype_includes), "Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Array_Constructor_observe), "Array_Constructor_observe"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Array_Constructor_unobserve), "Array_Constructor_unobserve"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::ArrayBuffer_Constructor_transfer), "ArrayBuffer_Constructor_transfer"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Float32Array_Prototype_includes), "Float32Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Float64Array_Prototype_includes), "Float64Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Int16Array_Prototype_includes), "Int16Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Int32Array_Prototype_includes), "Int32Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Int8Array_Prototype_includes), "Int8Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Map_Prototype_toJSON), "Map_Prototype_toJSON"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Object_Constructor_getOwnPropertyDescriptors), "Object_Constructor_getOwnPropertyDescriptors"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Object_Constructor_observe), "Object_Constructor_observe"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Object_Constructor_unobserve), "Object_Constructor_unobserve"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::RegExp_Constructor_escape), "RegExp_Constructor_escape"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Set_Prototype_toJSON), "Set_Prototype_toJSON"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_at), "String_Prototype_at"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_leftPad), "String_Prototype_leftPad"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_lPad), "String_Prototype_lPad"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_padLeft), "String_Prototype_padLeft"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_padRight), "String_Prototype_padRight"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_rightPad), "String_Prototype_rightPad"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_rPad), "String_Prototype_rPad"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_substr), "String_Prototype_substr"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_trimLeft), "String_Prototype_trimLeft"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::String_Prototype_trimRight), "String_Prototype_trimRight"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Uint16Array_Prototype_includes), "Uint16Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Uint32Array_Prototype_includes), "Uint32Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Uint8Array_Prototype_includes), "Uint8Array_Prototype_includes"),
            TraceLoggingInt32(Get(ESBuiltInPropertyId::Uint8ClampedArray_Prototype_includes), "Uint8ClampedArray_Prototype_includes")
        );

#undef Get
    }
}

void ESBuiltInsTelemetryProvider::OutputPrint()
{
#ifdef TELEMETRY_OUTPUTPRINT
    if( CONFIG_ISENABLED(Js::ESBLangTelFlag) )
    {
        Output::Print( _u("----------\r\n") );
        Output::Print( _u("-- ECMAScript 5-7 Built-Ins Telemetry.\r\n") );
        Output::Print( _u("Date.parse Telemetry.\r\n"));
        Output::Print( _u("OnPropertyEncountered     : %d \r\n"), count_OnPropertyEncountered      );
        Output::Print( _u("OnConstructorCalled       : %d \r\n"), count_OnConstructorCalled        );
        Output::Print( _u("GetBuiltInTypeNameFunction: %d \r\n"), count_GetBuiltInTypeNameFunction );
        Output::Print( _u("GetBuiltInTypeNameObject  : %d \r\n"), count_GetBuiltInTypeNameObject   );
        Output::Print( _u("GetBuiltInTypeNameOther   : %d \r\n"), count_GetBuiltInTypeNameOther    );

        Output::Print( _u("ESBuiltInPropertyId \tObject               \tFunction            \tCallCount \r\n") );

        bool atLeast1 = false;
        // For each ESBuiltInProperty in ESBuiltInsDatabase::ESBuiltInPropertyList
        //  Get ESBuiltInPropertyId, convert to ESBuiltInPropertyIdIdx
        //      Get count from usageMap

        ESBuiltInPropertyList& properties = ESBuiltInsDatabase::GetESBuiltInPropertyIdIdxList();
        for( int i = 0; i < properties.Count(); i++ )
        {
            ESBuiltInProperty& prop = properties.Item( i );

            AssertMsg( i < ESBuiltInsDatabase::ESBuiltInPropertyIdCount, "i is not out of bounds." );

            byte count = this->usageMap[ i ];
            if( count == BYTE_MAX )
            {
                Output::Print( _u("%20d\t%-20s\t%-20s\t255+\r\n"), prop.esbiPropertyId, prop.constructorName, prop.propertyName );
                atLeast1 = true;
            }
            else if( count > 0 )
            {
                Output::Print( _u("%20d\t%-20s\t%-20s\t%9d\r\n"), prop.esbiPropertyId, prop.constructorName, prop.propertyName, count );
                atLeast1 = true;
            }
        }

        if( !atLeast1 )
        {
            Output::Print( _u("No built-ins called.\r\n") );
        }
        Output::Print( _u("----------\r\n"));
    }
#endif
}

#endif