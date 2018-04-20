//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include "BuiltInFacets.h"

#ifdef ENABLE_BASIC_TELEMETRY

namespace Js
{
    /**
     *  Utility class that allows mapping of properties on vars to BuiltInTracker::Facet enums
     */
    class BuiltInMapper
    {
    public:
        BuiltInMapper(ScriptContext* sc) :
            scriptContext(sc)
        {

        }

        ~BuiltInMapper()
        {

        }

        /**
        *  Given a Js::Var instance and a propertyId, find the corresponding BuiltInFacet, or return BuiltInFacet::_None;
        */
        BuiltInFacet GetFacetForProperty(const Var& instance, const PropertyId& propertyId);

        /**
        *  Given a Js::Var instance return the corresponding BuiltInFacet if it is a constructor, or return BuiltInFacet::_None;
        */
        BuiltInFacet GetFacetForConstructor(const Js::Var& constructorFunction);

    private:
        ScriptContext * scriptContext;

        enum class ESBuiltInTypeNameId : uint16
        {
            _None,

#define TYPENAME(ctor,name) name,
#define TYPENAME_MAP(typeId,name) 
#include "ESBuiltInsTypeNames.h"
#undef TYPENAME_MAP
#undef TYPENAME

            _Max
        };

        /// <summary>A composite key that uniquely identifies a Type and Property combined, along with the Property's location/membership (Instance, Prototype, or Constructor).</summary>
        enum class ESBuiltInPropertyId : uint32
        {
            _None,
#define COMPUTE_KEY_ESBuiltInPropertyId(typeName,isConstructorMask,propertyName) (( ((uint32)( ESBuiltInTypeNameId:: ## typeName ## )) << 16) | (uint32)isConstructorMask | (uint32)(Js::PropertyIds:: ## propertyName ) )
#define Constructor 0x8000 // set the 15th bit high
#define Instance    0
#define Prototype   0
#define ENTRY_BUILTIN(esVersion, typeName,location,propertyName) typeName ## _ ## location ## _ ## propertyName ## = COMPUTE_KEY_ESBuiltInPropertyId(typeName,location,propertyName),
#include "ESBuiltins.h"
#undef ENTRY_BUILTIN
#undef COMPUTE_KEY_ESBuiltInPropertyId
#undef Constructor
#undef Instance
#undef Prototype
        };

        static ESBuiltInPropertyId GetESBuiltInPropertyId(ESBuiltInTypeNameId esbiTypeNameId, bool isConstructorProperty, Js::PropertyId propertyId);
        ESBuiltInTypeNameId GetESBuiltInTypeNameId(const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound);
        ESBuiltInTypeNameId GetESBuiltInTypeNameId_ByTypeId(const Js::TypeId typeId);

        ESBuiltInTypeNameId GetESBuiltInTypeNameId_ByPointer(const JavascriptFunction* constructorFunction);

        ESBuiltInTypeNameId GetESBuiltInTypeNameId_Function(const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound);

        ESBuiltInTypeNameId GetESBuiltInTypeNameId_Object(const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound);

        ESBuiltInTypeNameId GetESBuiltInTypeNameId_Other(const Var instance, const PropertyId propertyId, _Out_ bool& isConstructorProperty, TypeId typeId, _Out_ ESBuiltInPropertyId& shortcutPropertyFound);

        static BuiltInFacet GetESBuiltInArrayIndex(ESBuiltInPropertyId propertyId);

        bool IsBuiltInMath(const Var& instance) const;

        bool IsBuiltInJson(const Var& instance) const;

    };
}

#endif  // ENABLE_BASIC_TELEMETRY