#include "stdafx.h"
#include "ESBuiltInsDatabase.h"

#ifdef TELEMETRY_ESB

using namespace JsUtil;
using namespace Js;

#ifndef TELEMETRY_TRACELOGGING
bool                  ESBuiltInsDatabase::isInitialized;
ESBuiltInPropertyList ESBuiltInsDatabase::esbiPropertyList( &HeapAllocator::Instance );
#endif

void ESBuiltInsDatabase::Initialize()
{
#ifndef TELEMETRY_TRACELOGGING
    if( ESBuiltInsDatabase::isInitialized ) return;

    esbiPropertyList.Add(
        ESBuiltInProperty( ESBuiltInPropertyId::_None, L"", true, L"", static_cast<Js::PropertyIds>( 0 ) )
    );

#define Constructor 0x8000 // set the 15th bit high
#define Instance    0
#define Prototype   0
#define BUILTIN(typeName,location,propertyName,propertyKind,esVersion) \
    esbiPropertyList.Add( \
        ESBuiltInProperty( ESBuiltInPropertyId::typeName ## _ ## location ## _ ## propertyName, L#typeName, location == Constructor, L#propertyName, Js::PropertyIds:: ## propertyName ) \
    );
#include "ESBuiltInsDatabase.inc"
#undef BUILTIN
#undef Constructor
#undef Instance
#undef Prototype

    ESBuiltInsDatabase::isInitialized = true;
#endif
}

#ifndef TELEMETRY_TRACELOGGING
ESBuiltInProperty::ESBuiltInProperty() :
    esbiPropertyId( ESBuiltInPropertyId::_None ),
    constructorName( nullptr ),
    isInstance( false ),
    propertyName( nullptr )
{
}

ESBuiltInProperty::ESBuiltInProperty( ESBuiltInPropertyId esbiPropertyId, _In_z_ wchar_t* constructorName, bool isInstanceProperty, _In_z_ wchar_t* const propertyName, const Js::PropertyId propertyId ) :
    esbiPropertyId( esbiPropertyId ),
    constructorName( constructorName ),
    isInstance( isInstanceProperty ),
    propertyName( propertyName ),
    propertyId( propertyId )
{
}
#endif

void ESBuiltInsDatabase::Cleanup()
{
    // Code disabled because Cleanup() seems to be called even when there are ESBuiltInScriptContextTelemetry instances with items still in their usage dicts which reference this database.
    return; /*
    
    if( !ESBuiltInsDatabase::isInitialized ) return;
    ESBuiltInsDatabase::allBuiltIns.ClearAndZero();
    ESBuiltInsDatabase::isInitialized = false; */
}

#pragma region Publics

size_t ESBuiltInsDatabase::GetESBuiltInArrayIndex(ESBuiltInPropertyId propertyId)
{
    switch(propertyId)
    {
#define BUILTIN(typeName,location,propertyName,propertyKind,esVersion)      case ESBuiltInPropertyId:: ## typeName ## _ ## location ## _ ## propertyName: \
        return static_cast<size_t>( ESBuiltInPropertyIdIdx:: ## typeName ## _ ## location ## _ ## propertyName );
#include "ESBuiltInsDatabase.inc"
#undef BUILTIN
    }
    return SIZE_MAX;
}

#ifndef TELEMETRY_TRACELOGGING
ESBuiltInProperty* ESBuiltInsDatabase::GetESBuiltInProperty( const ESBuiltInPropertyId esBuiltInPropertyId )
{
    if( !ESBuiltInsDatabase::isInitialized ) ESBuiltInsDatabase::Initialize();
    
    size_t idx = ESBuiltInsDatabase::GetESBuiltInArrayIndex( esBuiltInPropertyId );
    if( idx == SIZE_MAX ) return nullptr;

    return &esbiPropertyList.Item( idx );
}

ESBuiltInPropertyList& ESBuiltInsDatabase::GetESBuiltInPropertyIdIdxList()
{
    if( !ESBuiltInsDatabase::isInitialized ) ESBuiltInsDatabase::Initialize();

    return esbiPropertyList;
}
#endif

ESBuiltInTypeNameId ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByTypeId( const Js::TypeId typeId )
{
    switch( typeId )
    {
#define TYPENAME(ctor,name) 
#define TYPENAME_MAP(typeId, name) \
        case Js:: ## typeId ##: \
            return ESBuiltInTypeNameId:: ## name ## ;
#include "ESBuiltInsTypeNames.inc"
#undef TYPENAME_MAP
#undef TYPENAME
        default:
            return ESBuiltInTypeNameId::_None;
    }
}

ESBuiltInTypeNameId ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByString( Js::JavascriptString* typeName )
{
    const wchar_t* value       = typeName->GetString();
    size_t         valueLength = typeName->GetLength();

#include "ESBuiltInsTypeNames.trie.inc"
}

ESBuiltInTypeNameId ESBuiltInsDatabase::GetESBuiltInTypeNameId_ByPointer( const ScriptContext& scriptContext, const JavascriptFunction* constructorFunction )
{
    JavascriptLibrary* lib = scriptContext.GetLibrary();
#define TYPENAME(ctor,name) TYPENAME_##ctor(name)
#define TYPENAME_0(name) 
#define TYPENAME_1(name) if( constructorFunction == lib->Get## name ##Constructor() ) return ESBuiltInTypeNameId::##name;
#define TYPENAME_MAP(typeId,name) 
#include "ESBuiltInsTypeNames.inc"
#undef TYPENAME_MAP
#undef TYPENAME_1
#undef TYPENAME_0
#undef TYPENAME

    return ESBuiltInTypeNameId::_None;
}

ESBuiltInPropertyId ESBuiltInsDatabase::GetESBuiltInPropertyId( ESBuiltInTypeNameId esbiTypeNameId, bool isConstructorProperty, Js::PropertyId propertyId )
{
    AssertMsg( static_cast<uint32>( propertyId ) < 0x8000, "propertyId is under 32768");
    
    uint32 ctor = isConstructorProperty ? 0x8000 : 0;

    uint32 key = static_cast<uint32>( esbiTypeNameId ) << 16 | ctor | static_cast<uint32>( propertyId );

    return static_cast<ESBuiltInPropertyId>( key );
}

#endif