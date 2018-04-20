//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include "Core/CRC.h"

#ifdef ENABLE_BASIC_TELEMETRY

#define CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(esVersion, feature, m_scriptContext)                                  \
    {                                                                                                                  \
       Js::LanguageFeaturesCountTracker& tracker = m_scriptContext->GetTelemetry().GetLanguageFeaturesCountTracker();  \
       tracker.Increment(Js::LanguageFeaturesCountTracker::Facet:: ## esVersion ## _ ## feature);                      \
    }

namespace Js
{
    class LanguageFeaturesCountTracker
    {

    public:
        enum class Facet : size_t
        {
            _None,

#define ENTRY_LANGFEATURE(esVersion, featureName) esVersion ## _ ## featureName ,
#include "LanguageFeatures.h"
#undef ENTRY_LANGFEATURE

            _Max
        };

        static const size_t numFacets = static_cast<size_t>(Facet::_Max) + 1;

    private:
        uint32 counts[LanguageFeaturesCountTracker::numFacets] = { 0 };

        const char* names[LanguageFeaturesCountTracker::numFacets] = {
            "_None",
#define ENTRY_LANGFEATURE(esVersion, featureName) STRINGIZE( esVersion ## _ ## featureName ),
#include "LanguageFeatures.h"
#undef ENTRY_LANGFEATURE
            "_Max",
        };

        uint32 crcs[LanguageFeaturesCountTracker::numFacets] = {
            CalculateCRC32("_None"),

#define ENTRY_LANGFEATURE(esVersion, featureName) CalculateCRC32(STRINGIZE( esVersion ## _ ## featureName )),
#include "LanguageFeatures.h"
#undef ENTRY_LANGFEATURE

            CalculateCRC32("_Max")
        };

    public:
        uint32 * GetCountsArray()
        {
            return this->counts;
        }

        uint32* GetCRCArray()
        {
            return this->crcs;
        }

        inline void Increment(Facet c)
        {
            if (counts[static_cast<size_t>(c)] != UINT32_MAX)
            {
                ++counts[static_cast<size_t>(c)];
            }
        }

        void Reset()
        {
            for (size_t i = 0; i < LanguageFeaturesCountTracker::numFacets; i++)
            {
                this->counts[i] = 0;
            }
        }

    };
}

#endif  // ENABLE_BASIC_TELEMETRY
