//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include "Core/CRC.h"

#define CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(builtin)                                              \
    {                                                                                              \
       Js::BuiltInCountTracker& tracker = scriptContext->GetTelemetry().GetBuiltInCountTracker();  \
       tracker.Increment(Js::BuiltInCountTracker::Facet:: ##builtin);                              \
    }

namespace Js
{
    class BuiltInCountTracker
    {

    public:

        enum class Facet : size_t
        {
            _None,

#define ENTRY_BUILTIN(esVersion, typeName, location, propertyName) typeName ## _ ## location ## _ ## propertyName ,
#include "ESBuiltIns.h"
#undef ENTRY_BUILTIN

            _Max
        };

        static const size_t numFacets = static_cast<size_t>(Facet::_Max) + 1;


    private: 
        uint32 counts[BuiltInCountTracker::numFacets];

        const char* names[BuiltInCountTracker::numFacets] = {
        "_None",

#define ENTRY_BUILTIN(esVersion, typeName, location, propertyName) STRINGIZE( typeName ## _ ## location ## _ ## propertyName )
#include "ESBuiltIns.h"
#undef ENTRY_BUILTIN

        "_Max"
        };

        uint32 crcs[BuiltInCountTracker::numFacets] = {
            CalculateCRC32("_None"),

#define ENTRY_BUILTIN(esVersion, typeName, location, propertyName) CalculateCRC32(STRINGIZE( typeName ## _ ## location ## _ ## propertyName )),
#include "ESBuiltIns.h"
#undef ENTRY_BUILTIN

            CalculateCRC32("_Max")
        };

    public: 

        inline void Increment(Facet c)
        {
            ++counts[static_cast<size_t>(c)];
        }

        void Reset()
        {
            for (size_t i = 0; i < BuiltInCountTracker::numFacets; i++)
            {
                this->counts[i] = 0;
            }
        }

        uint32* GetCountsArray()
        {
            return this->counts;
        }

        uint32* GetCRCArray()
        {
            return this->crcs;
        }

        // Hooks below are useful as they allow us to check for polyfills, since sites will test and use a particular property, 
        // even if we don't have an implementation of it.

        void GetMethodProperty(const Var& instance, const PropertyId& propertyId, const Var& value, const bool successful)
        {
        //    // TODO
        //    //this->esBuiltInsOpcodeTelemetry->GetMethodProperty(instance, propertyId, value, successful);
        }

        void GetProperty(const Var& instance, const PropertyId& propertyId, const Var& value, const bool successful)
        {
        //    // TODO
        //    //this->esBuiltInsOpcodeTelemetry->GetProperty(instance, propertyId, value, successful);
        }

        void IsInstanceOf(const Var& instance, const Var& constructorFunction, const Var& result)
        {
        //    // TODO
        //    //this->esBuiltInsOpcodeTelemetry->IsInstanceOf(instance, constructorFunction, result);
        }

        void TypeOfProperty(const Var& instance, const PropertyId& propertyId, const Var& value, const Var& result, const bool successful)
        {
        //    // TODO
        //    //this->esBuiltInsOpcodeTelemetry->TypeOfProperty(instance, propertyId, value, result, successful);
        }

        void IsIn(const Var& instance, const PropertyId& propertyId, const bool successful)
        {
        //    // TODO
        //    //this->esBuiltInsOpcodeTelemetry->IsIn(instance, propertyId, successful);
        }

        void NewScriptObject(const Var& constructorFunction, const Arguments& arguments, const Var& constructedInstance)
        {
        //    // TODO
        //    //this->esBuiltInsOpcodeTelemetry->NewScriptObject(constructorFunction, arguments, constructedInstance);
        }


        /*
        ESBuiltInTypeNameId GetESBuiltInTypeNameId(const Js::Var instance, const Js::PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound);

        ESBuiltInTypeNameId GetESBuiltInTypeNameId_Function(const Js::Var instance, const Js::PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound);
        ESBuiltInTypeNameId GetESBuiltInTypeNameId_Object(const Js::Var instance, const Js::PropertyId propertyId, _Out_ bool& isConstructorProperty, _Out_ ESBuiltInPropertyId& shortcutPropertyFound);
        ESBuiltInTypeNameId GetESBuiltInTypeNameId_Other(const Js::Var instance, const Js::PropertyId propertyId, _Out_ bool& isConstructorProperty, Js::TypeId typeId, _Out_ ESBuiltInPropertyId& shortcutPropertyFound);
        */



    };

}