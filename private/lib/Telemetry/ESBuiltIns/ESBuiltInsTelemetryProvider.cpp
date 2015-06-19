#include "stdafx.h"
#include "ESBuiltInsTelemetryProvider.h"

#ifdef TELEMETRY_ESB

// Copy of defines from Telemetry.cpp, which has dependencies on MicrosoftTelemetry.h which cannot be included in a common header file.
#include <telemetry\MicrosoftTelemetry.h>

#if defined(_M_IX86)
#define TL_BINARYARCH "x86"
#elif defined(_M_X64)
#define TL_BINARYARCH "amd64"
#elif defined(_M_ARM)
#define TL_BINARYARCH "arm32"
#elif defined(_M_ARM64)
#define TL_BINARYARCH "arm64"
#else
#error Unknown architecture
#endif

#define TraceLogChakra(name, ...) \
    if (g_TraceLoggingClient->GetShouldLogTelemetry() == true) { \
        TraceLoggingWrite( \
        g_hTraceLoggingProv, \
        name, \
        TraceLoggingKeyword(MICROSOFT_KEYWORD_TELEMETRY), \
        TraceLoggingString(VER_IEVERSION_STR, "binaryVersion"), \
        TraceLoggingString(TL_BINARYFLAVOR, "binaryFlavor"), \
        TraceLoggingString(TL_BINARYARCH, "binaryArch"), \
        __VA_ARGS__ \
        ); \
    }

using namespace Js;

ESBuiltInsTelemetryProvider::ESBuiltInsTelemetryProvider( ScriptContextTelemetry& scriptContextTelemetry ) :
    IScriptContextTelemetryProvider( scriptContextTelemetry ),
    opcodeTelemetry( *this ),
    usageMap( ESBuiltInsDatabase::CreateUsageMap( *scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) )
#ifdef TELEMETRY_ESB_STRINGS
    ,
    nameMath( nullptr ), // JavascriptString::NewCopySzFromArena(L"Math", &scriptContextTelemetry.GetScriptContext(), scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) ),
    nameJson( nullptr ) // JavascriptString::NewCopySzFromArena(L"JSON", &scriptContextTelemetry.GetScriptContext(), scriptContextTelemetry.GetScriptContext().TelemetryAllocator() ) )
#endif
{
    // Register the opcodeTelemetry callback handler:
    scriptContextTelemetry.GetOpcodeTelemetry().esBuiltInsOpcodeTelemetry = &this->opcodeTelemetry;
}


ESBuiltInsTelemetryProvider::~ESBuiltInsTelemetryProvider()
{
    ESBuiltInsDatabase::FreeUsageMap( *scriptContextTelemetry.GetScriptContext().TelemetryAllocator(), this->usageMap );
}

const wchar_t* const ESBuiltInsTelemetryProvider::GetDisplayName() const
{
    return L"ECMAScript 5-7 Built-Ins Telemetry.";
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
    ESBuiltInTypeNameId esbiTypeNameId = this->GetESBuiltInTypeNameId( instance, propertyId, _Out_ isConstructorProperty, _Out_ esbiPropertyId );
    
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
    
    // `instance` could be accessing a property of Function (e.g. Function.bind), or it could be accessing a Constructor Property (e.g. Array.isArray).
    // So see if the PropertyId belongs to Function, and if so, return immediately before doing a name-check.

    ESBuiltInPropertyId belongsToFunction;
    belongsToFunction = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Function, /*isConstructorProperty:*/ false, propertyId );
    if( belongsToFunction != ESBuiltInPropertyId::_None )
    {
        shortcutPropertyFound = belongsToFunction;
        return ESBuiltInTypeNameId::Function;
    }

    belongsToFunction = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Function, /*isConstructorProperty:*/ true, propertyId );
    if( belongsToFunction != ESBuiltInPropertyId::_None ) {
        // There is a bug here in that `Function_Constructor_length` or `Function_Constructor_prototype` will never be recorded as a Constructor property, as the names are also used by instances, and this code has no way of knowing if this is *a* function, or *the* Function function.
        isConstructorProperty = true;
        shortcutPropertyFound = belongsToFunction;
        return ESBuiltInTypeNameId::Function;
    }

    // PropertyId does not belong to Function, therefore this is a Constructor property. But which Constructor is it?
    // There are two ways to solve this:
    // 1. Look up the TypeNameId by string (i.e. the name of the function), "Array.isArray" -> "Array" -> the global Array constructor function.
    // 2. Compare the pointer to the function to known pointers, `if( function == library.ArrayConstructor )`

    // Method 1 has the advantage of working to detect polyfills, but is slow as string comparisons, even when using a Trie, require lots of steps, repeatedly.
    // Method 2 only detects usage of implemented and non-overrriden built-ins, but is a lot faster.

    const JavascriptFunction* instanceAsFunction = JavascriptFunction::FromVar( instance );

    isConstructorProperty = true;

#ifdef TELEMETRY_ESB_STRINGS
    {
        JavascriptString* functionName = instanceAsFunction->GetDisplayName( true );

        ESBuiltInTypeNameId ret = ESBuiltInsDatabase::GetESBuiltInTypeNameId( functionName );
        return ret;
    }
#else
    {
        ESBuiltInTypeNameId ret = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( this->scriptContextTelemetry.GetScriptContext(), instanceAsFunction );
        return ret;
    }
#endif
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId_Object( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    count_GetBuiltInTypeNameObject++;
    
    if( this->IsBuiltInMath( instance ) ) return ESBuiltInTypeNameId::Math;
    if( this->IsBuiltInJson( instance ) ) return ESBuiltInTypeNameId::JSON;
    
    // `instance` could be a Polyfill Object. Inspect the name of its Constructor to see if it matches (e.g. `function DataView() { this.someDataViewProperty = 'abc'; };` ).
    // However, an optimization: don't name-check if the propertyId already belongs to Object (e.g. hasOwnProperty).

    ESBuiltInPropertyId belongsToObject = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Object, /*isConstructorProperty:*/ false, propertyId ); // `isConstructorProperty` will always be false because `instance` is not a function and so cannot be a constructor.
    if( belongsToObject != ESBuiltInPropertyId::_None )
    {
        shortcutPropertyFound = belongsToObject;
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

#   ifdef TELEMETRY_ESB_STRINGS
            {
                JavascriptString* constructorName;
                FunctionProxy* constructorFunctionProxy = constructorFunction->GetFunctionProxy();
                if( constructorFunctionProxy != nullptr )
                {
                    const wchar_t* constructorNameCStr = constructorFunctionProxy->GetDisplayName();
                    constructorName = JavascriptString::NewCopySz( constructorNameCStr, scriptContextPtr );
                }
                else
                {
                    constructorName = constructorFunction->GetDisplayName( true );
                }

                if( constructorName == nullptr ) return ESBuiltInTypeNameId::_None;
        
                // Look up the constructor name in the type list.
                ESBuiltInTypeNameId ret = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByString( constructorName );
                return ret;
            }
#   else
            {
                ESBuiltInTypeNameId ret = ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( this->scriptContextTelemetry.GetScriptContext(), constructorFunction );
                return ret;
            }
#   endif
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
    Assert( esbiTypeNameId != ESBuiltInTypeNameId::_None );

    // See if the specified property is defined on the identified type, if not, see if it's defined for Object (and so, is inherited):
    ESBuiltInPropertyId definedOnType = ESBuiltInsDatabase::GetESBuiltInPropertyId( esbiTypeNameId, /*isConstructorProperty:*/ false, propertyId );
    if( definedOnType == ESBuiltInPropertyId::_None )
    {
        ESBuiltInPropertyId belongsToObject = ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId::Object, /*isConstructorProperty:*/ false, propertyId ); // `isConstructorProperty` will always be false because `instance` is not a function and so cannot be a constructor.
        if( belongsToObject != ESBuiltInPropertyId::_None ) return ESBuiltInTypeNameId::Object;
    }

    return esbiTypeNameId;
}

ESBuiltInTypeNameId ESBuiltInsTelemetryProvider::GetESBuiltInTypeNameId( const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound )
{
    isConstructorProperty = false;
    
    if( instance == null ) return ESBuiltInTypeNameId::_None;

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

void ESBuiltInsTelemetryProvider::Output()
{
#ifdef TELEMETRY_TRACELOGGING
    {
        // Unfortunately we cannot #include a file directly within a macro usage.
        // i.e. this doesn't work:
        // 
        // #define BUILTIN(yada) TraceLoggingInt32( yada, #yada )
        // TraceLogChakra( /*snip*/
        // #include "ESBuiltInsDatabase.inc"
        // );
        //
        // So the tracelogging call is instead populated by a C++-generation script, which the same effect as the preprocessor, except you need to remember to re-run the script whenever the database changes.
        // The generated file is TraceLogList.inc, and unfortunately the preprocessor puts it all on a single long line.

        size_t idx;

#define Get(propertyId) \
        ( idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( propertyId ), idx != SIZE_MAX ? this->usageMap[ idx ] : 0 )

// If the compiler complains about running out of heap-space, re-run TraceLogList.Generator.js with a modified entriesPerBlock value (line 39).

#include "TraceLogList.inc"

#undef Get
        
    }
#else
    {
        Output::Print( L"OnPropertyEncountered     : %d \r\n", count_OnPropertyEncountered      );
        Output::Print( L"OnConstructorCalled       : %d \r\n", count_OnConstructorCalled        );
        Output::Print( L"GetBuiltInTypeNameFunction: %d \r\n", count_GetBuiltInTypeNameFunction );
        Output::Print( L"GetBuiltInTypeNameObject  : %d \r\n", count_GetBuiltInTypeNameObject   );
        Output::Print( L"GetBuiltInTypeNameOther   : %d \r\n", count_GetBuiltInTypeNameOther    );

        Output::Print( L"ESBuiltInPropertyId \tObject               \tFunction            \tCallCount \r\n" );

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
                Output::Print( L"%20d\t%-20s\t%-20s\t255+\r\n", prop.esbiPropertyId, prop.constructorName, prop.propertyName );
                atLeast1 = true;
            }
            else if( count > 0 )
            {
                Output::Print( L"%20d\t%-20s\t%-20s\t%9d\r\n", prop.esbiPropertyId, prop.constructorName, prop.propertyName, count );
                atLeast1 = true;
            }
        }

        if( !atLeast1 )
        {
            Output::Print( L"No built-ins called.\r\n" );
        }
    }
#endif
}

#endif