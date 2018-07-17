//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include "Core/CRC.h"

#ifdef ENABLE_BASIC_TELEMETRY

#define CHAKRATEL_SCRIPT_CONTEXT_INC_CUSTOM(counter, m_scriptContext)                                                  \
    {                                                                                                                  \
       Js::CustomScriptContextCounters& tracker = m_scriptContext->GetTelemetry().GetCustomCountTracker();             \
       tracker.Increment(Js::CustomScriptContextCounters::Facet:: ## counter);                                         \
    }

namespace Js
{
    class CustomScriptContextCounters
    {

    public:
        enum class Facet : size_t
        {
            _Count = 0,
        };

        static const size_t numFacets = static_cast<size_t>(Facet::_Count);

    private:
        uint32 counts[CustomScriptContextCounters::numFacets] = { 0 };

    public:
        uint32 * GetCountsArray()
        {
            return this->counts;
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
#pragma warning(push)
#pragma warning(disable : 4296) // disable warning if this value is 0
#pragma prefast(suppress : __WARNING_LOOP_BODY_NEVER_EXECUTED)
            for (size_t i = 0; i < CustomScriptContextCounters::numFacets; i++)
            {
#pragma warning(pop)
                this->counts[i] = 0;
            }

        }
    };
}

#endif  // ENABLE_BASIC_TELEMETRY
