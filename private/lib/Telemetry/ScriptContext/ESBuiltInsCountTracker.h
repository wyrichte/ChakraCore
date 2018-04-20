//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include "Core/CRC.h"
#include "BuiltInFacets.h"
#include "BuiltInMapper.h"

#ifdef ENABLE_BASIC_TELEMETRY

#define CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(builtin)                                              \
    {                                                                                              \
       Js::BuiltInCountTracker& tracker = scriptContext->GetTelemetry().GetBuiltInCountTracker();  \
       tracker.Increment(Js::BuiltInFacet:: ##builtin);                              \
    }

namespace Js
{
    class BuiltInCountTracker
    {

    public:

        BuiltInCountTracker(ScriptContext* sc) :
            builtInMapper(sc)
        {
        }

        static const size_t numFacets = static_cast<size_t>(BuiltInFacet::_Max) + 1;

    private: 
        BuiltInMapper builtInMapper;

        uint32 counts[BuiltInCountTracker::numFacets] = { 0 };

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

        inline void Increment(BuiltInFacet c)
        {
            if (counts[static_cast<size_t>(c)] != UINT32_MAX)
            {
                ++counts[static_cast<size_t>(c)];
            }
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

        void GetMethodProperty(const Var& instance, const PropertyId& propertyId, const Var& value)
        {
            BuiltInFacet f = this->builtInMapper.GetFacetForProperty(instance, propertyId);
            if (f != BuiltInFacet::_None)
            {
                this->Increment(f);
            }
        }

        void GetProperty(const Var& instance, const PropertyId& propertyId, const Var& value)
        {
            BuiltInFacet f = this->builtInMapper.GetFacetForProperty(instance, propertyId);
            if (f != BuiltInFacet::_None)
            {
                this->Increment(f);
            }
        }

        void IsInstanceOf(const Var& instance, const Var& constructorFunction, const Var& result)
        {
            // TODO: Investigate the use of `instanceof` for feature detection, and if commonly used, add telemetry here.
        }

        void TypeOfProperty(const Var& instance, const PropertyId& propertyId, const Var& value, const Var& result)
        {
            BuiltInFacet f = this->builtInMapper.GetFacetForProperty(instance, propertyId);
            if (f != BuiltInFacet::_None)
            {
                this->Increment(f);
            }
        }

        void IsIn(const Var& instance, const PropertyId& propertyId)
        {
            BuiltInFacet f = this->builtInMapper.GetFacetForProperty(instance, propertyId);
            if (f != BuiltInFacet::_None)
            {
                this->Increment(f);
            }
        }

        void NewScriptObject(const Var& constructorFunction, const Arguments& arguments, const Var& constructedInstance)
        {
            BuiltInFacet f = this->builtInMapper.GetFacetForConstructor(constructorFunction);
            if (f != BuiltInFacet::_None)
            {
                this->Increment(f);
            }
        }

    };

}

#endif  // ENABLE_BASIC_TELEMETRY
