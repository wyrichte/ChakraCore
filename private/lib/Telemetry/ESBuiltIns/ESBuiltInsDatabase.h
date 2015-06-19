#pragma once

#ifdef TELEMETRY_ESB

enum class ESBuiltInTypeNameId : uint16
{
    _None,
#define TYPENAME(ctor,name) name,
#define TYPENAME_MAP(typeId,name) 
#include "ESBuiltInsTypeNames.inc"
#undef TYPENAME_MAP
#undef TYPENAME
    _Max
};

#define COMPUTE_KEY_ESBuiltInPropertyId(typeName,isConstructorMask,propertyName) (( ((uint32)( ESBuiltInTypeNameId:: ## typeName ## )) << 16) | (uint32)isConstructorMask | (uint32)(Js::PropertyIds:: ## propertyName ) )

/// <summary>A composite key that uniquely identifies a Type and Property combined, along with the Property's location/membership (Instance, Prototype, or Constructor).</summary>
enum class ESBuiltInPropertyId : uint32
{
    _None,
#define Constructor 0x8000 // set the 15th bit high
#define Instance    0
#define Prototype   0
#define BUILTIN(typeName,location,propertyName,propertyKind,esVersion) typeName ## _ ## location ## _ ## propertyName ## = COMPUTE_KEY_ESBuiltInPropertyId(typeName,location,propertyName),
#include "ESBuiltInsDatabase.inc"
#undef BUILTIN
#undef Constructor
#undef Instance
#undef Prototype
};

#ifndef TELEMETRY_TRACELOGGING
/// <summary>POD type for an ECMAScript built-in property and its associated function value in the case of methods (i.e. an unaliased function property, e.g. String.prototype.substring).</summary>
class ESBuiltInProperty
{
public:
    ESBuiltInPropertyId       esbiPropertyId; // not the same thing as `PropertyId`, which is a key in the interned string table of property names.
    bool                      isInstance; // the property is an Instance or Prototype property, not a Constructor property.

    const wchar_t*            constructorName;
    ESBuiltInTypeNameId       typeNameId;
    
    const wchar_t*            propertyName;
    Js::PropertyId            propertyId;

    ESBuiltInProperty(); // Uses `ESBuiltInPropertyId::None` as it's the zero value.
    ESBuiltInProperty( ESBuiltInPropertyId functionId, _In_z_ wchar_t* const constructorName, const bool isInstanceProperty, _In_z_ wchar_t* const functionName, const Js::PropertyId propertyId );
};

// Use the HeapAllocator rather than Recycler so the instance lasts the life of the process, not the ScriptContext.
typedef JsUtil::List< ESBuiltInProperty, HeapAllocator > ESBuiltInPropertyList;
#endif

class ESBuiltInsDatabase
{
private:

// Identical to ESBuiltInPropertyId, except values are 0...n instead of being a computed value.
enum class ESBuiltInPropertyIdIdx : size_t
{
    _None,
#define BUILTIN(typeName,location,propertyName,propertyKind,esVersion) typeName ## _ ## location ## _ ## propertyName,
#include "ESBuiltInsDatabase.inc"
#undef BUILTIN
    _Max
};

private:
    // This is a static class, delete the default members.
    ESBuiltInsDatabase() = delete;
    ESBuiltInsDatabase(const ESBuiltInsDatabase&) = delete;
    ESBuiltInsDatabase& operator=(const ESBuiltInsDatabase&) = delete;
    ~ESBuiltInsDatabase() = delete;

#ifndef TELEMETRY_TRACELOGGING
    static bool                  isInitialized;
    static ESBuiltInPropertyList esbiPropertyList;
#endif

    static ESBuiltInPropertyId   AddBuiltInExtension(const bool isInstanceProperty, const wchar_t* typeNameCStr2, const size_t typeNameCStr2Length, const wchar_t* propertyNameCStr, const size_t propertyNameLength);

    /// <remarks>Cannot be called from ScriptEngine's constructor because AssertCanHandleOutOfMemory() fails at that point.</remarks>
    static void Initialize();
    
public:
    
    static const size_t ESBuiltInPropertyIdCount = static_cast<size_t>( ESBuiltInPropertyIdIdx::_Max );

    static size_t              GetESBuiltInArrayIndex(ESBuiltInPropertyId propertyId);

    template<typename TAllocator>
    static byte*               CreateUsageMap(TAllocator& allocator)
    {
        size_t nofBytes = static_cast<size_t>( ESBuiltInPropertyIdIdx::_Max );
        return reinterpret_cast<byte*>( allocator.AllocZero( nofBytes ) ); // This reinterpret_cast is unavoidable.
    }

    template<typename TAllocator>
    static void                FreeUsageMap(TAllocator& allocator, byte* buffer)
    {
        size_t nofBytes = static_cast<size_t>( ESBuiltInPropertyIdIdx::_Max );
        allocator.Free( buffer, nofBytes );
    }

#ifndef TELEMETRY_TRACELOGGING
    static ESBuiltInPropertyList& GetESBuiltInPropertyIdIdxList();

    static ESBuiltInProperty* GetESBuiltInProperty(const ESBuiltInPropertyId propertyId);
#endif

    static ESBuiltInPropertyId GetESBuiltInPropertyId(ESBuiltInTypeNameId typeNameId, bool isConstructorProperty, Js::PropertyId propertyId);

    static ESBuiltInTypeNameId GetESBuiltInTypeNameId_ByString(Js::JavascriptString* typeName);

    static ESBuiltInTypeNameId GetESBuiltInTypeNameId_ByTypeId(const Js::TypeId typeId);

    static ESBuiltInTypeNameId GetESBuiltInTypeNameId_ByPointer(const Js::ScriptContext& scriptContext, const Js::JavascriptFunction* constructorFunction);

    static Js::JavascriptString* GetConstructorName(const Js::ScriptContext& scriptContext, const Js::Var instance);

    /// <summary>Frees the contents of the lists.</summary>
    static void Cleanup();
};

#endif