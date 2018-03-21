//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#include "TelemetryPch.h"
#include "ScriptContextTelemetry.h"
#include "TelemetryMacros.h"
#include "Core/CRC.h"

namespace Js
{
    CriticalSection ScriptContextTelemetry::initLock;
    volatile bool ScriptContextTelemetry::isInitialized = false;


#ifdef REJIT_STATS

    // Map from IR::BailOutKind uint to index in the bailoutKindCRCs[] above.
    JsUtil::BaseDictionary<uint, uint, HeapAllocator> ScriptContextTelemetry::bailoutMap(&HeapAllocator::Instance, 0);

    // It simplifies things to define these arrays outside of the class below.
    // If defined in class, the size of the arrays need to be a compile-time 
    // constant

    const UINT32 rejitReasonsCRCs[] = {
#define REJIT_REASON(f) CalculateCRC32(STRINGIZE(f)),
#include "../../../../core/lib/Common/Common/RejitReasons.h"
#undef REJIT_REASON
    };

    // stats:bailout
    const UINT32 bailoutKindCRCs[] = {
#define BAIL_OUT_KIND(a, b) CalculateCRC32(STRINGIZE(a)), 
#define BAIL_OUT_KIND_VALUE(a, b) CalculateCRC32(STRINGIZE(a)),
#define BAIL_OUT_KIND_VALUE_LAST(a, b) CalculateCRC32(STRINGIZE(a))
#include "../../../../core/lib/Backend/BailOutKind.h"
#undef BAIL_OUT_KIND_VALUE_LAST
#undef BAIL_OUT_KIND_VALUE
#undef BAIL_OUT_KIND
    };

    const size_t numBailOutKinds = _countof(bailoutKindCRCs);
    const size_t numRejitReasons = _countof(rejitReasonsCRCs);

#endif // REJIT_STATS

    void ScriptContextTelemetry::OutputTraceLogging(GUID activityId, DWORD hostType, bool isJSRT)
    {
        // Respect the throttle:
        if (g_TraceLoggingClient->IsThrottled())
        {
            return;
        }

        try
        {
#ifdef REJIT_STATS
            AssertOrFailFast(this->scriptContext->bailoutReasonCounts != nullptr);
            AssertOrFailFast(this->scriptContext->bailoutReasonCountsCap != nullptr);

            UINT32 bailoutCounts[numBailOutKinds] = { 0 };


            auto pbailoutMap = &ScriptContextTelemetry::bailoutMap;

            this->scriptContext->bailoutReasonCounts->Map([&bailoutCounts, pbailoutMap](uint kind, uint val) {
                if (pbailoutMap->ContainsKey(kind)) {
                    bailoutCounts[pbailoutMap->Item(kind)] = val;
                }
            });

            UINT32 bailoutCountsCap[numBailOutKinds] = { 0 };
            this->scriptContext->bailoutReasonCountsCap->Map([&bailoutCountsCap, pbailoutMap](uint kind, uint val) {
                if (pbailoutMap->ContainsKey(kind)) {
                    bailoutCountsCap[pbailoutMap->Item(kind)] = val;
                }
            });

            TraceLogChakra("ScriptContextTelemetry",
                TraceLoggingGuid(activityId, "activityID"),
                TraceLoggingUInt32(hostType, "hostType"),
                TraceLoggingBool(isJSRT, "isJSRT"),
                TraceLoggingUInt32FixedArray(builtInCountTracker.GetCRCArray(), BuiltInCountTracker::numFacets, "BuiltInCountNameCRCs", "CRC values of built-in names"),
                TraceLoggingUInt32FixedArray(builtInCountTracker.GetCountsArray(), BuiltInCountTracker::numFacets, "BuiltInCountValues", "values indicating how many times each built-in was called"),

                TraceLoggingUInt32FixedArray(languageFeaturesCountTracker.GetCRCArray(), LanguageFeaturesCountTracker::numFacets, "LanguageFeaturesNameCRCs", "CRC values of various language feature names"),
                TraceLoggingUInt32FixedArray(languageFeaturesCountTracker.GetCountsArray(), LanguageFeaturesCountTracker::numFacets, "LanguageFeaturesValues", "values indicating how many times each language feature was used"),

                TraceLoggingUInt32FixedArray(rejitReasonsCRCs, static_cast<uint16>(numRejitReasons), "RejitReasonCRCs", "CRC values of rejit reason names"),
                TraceLoggingUInt32FixedArray(this->scriptContext->rejitReasonCounts, static_cast<uint16>(numRejitReasons), "RejitReasonCounts", "counts of each rejit reason"),
                TraceLoggingUInt32FixedArray(this->scriptContext->rejitReasonCountsCap, static_cast<uint16>(numRejitReasons), "RejitReasonCountsCap", "count cap value of each rejit reason"),

                TraceLoggingUInt32FixedArray(bailoutKindCRCs, numBailOutKinds, "BailoutReasonCRCs", "CRC values of each bailout name"),
                TraceLoggingUInt32FixedArray(bailoutCounts, numBailOutKinds, "BailOutCounts", "count values of each bailout reason"),
                TraceLoggingUInt32FixedArray(bailoutCountsCap, numBailOutKinds, "BailOutCountsCap", "count cap values of each bailout reason")
            );
#else
            TraceLogChakra("ScriptContextTelemetry_NoRejit",
                TraceLoggingGuid(activityId, "activityID"),
                TraceLoggingUInt32(hostType, "hostType"),
                TraceLoggingBool(isJSRT, "isJSRT"),

                TraceLoggingUInt32FixedArray(builtInCountTracker.GetCRCArray(), static_cast<size_t>(BuiltInCountTracker::Facet::_Max), "BuiltInCountNameCRCs"),
                TraceLoggingUInt32FixedArray(builtInCountTracker.GetCountsArray(), static_cast<size_t>(BuiltInCountTracker::Facet::_Max), "BuiltInCountValues"),

                TraceLoggingUInt32FixedArray(langaugeFeaturesCountTracker.GetCRCArray(), static_cast<size_t>(LanguageFeaturesCountTracker::Facet::_Max), "LanguageFeaturesNameCRCs"),
                TraceLoggingUInt32FixedArray(langaugeFeaturesCountTracker.GetCountsArray(), static_cast<size_t>(LanguageFeaturesCountTracker::Facet::_Max), "LanguageFeaturesValues")
            );
#endif // REJIT_STATS

        }
        catch (...)
        {
            // We don't particularly care about this; the only situation in which we'd hit some
            // exception is if we were to fail a stack or heap allocation, and we don't do many
            // of them. Given that most stuff should be cleaned up or about to be cleaned up at
            // this point, this is unlikely; given that telemetry is mostly non-critical to any
            // individual execution of chakra, we can simply not report telemetry for what will
            // likely anyways be an extraordinary session. We do want to catch it in debug runs
            // though; this is unlikely to ever hit, but we will be fine if we just go and add:
            Assert(false);
        }
    }

    void ScriptContextTelemetry::Initialize()
    {
        // Note this will fail if ExceptionCheck::CanHandleOutOfMemory() is false.  It's currently called
        // once from the ScriptContextTelemetry constructor.

        // Initialize should only ever be called once
        if (!ScriptContextTelemetry::isInitialized)
        {
            AutoCriticalSection acs(&ScriptContextTelemetry::initLock);

            if (!ScriptContextTelemetry::isInitialized)
            {
#ifdef REJIT_STATS
#define BAIL_OUT_KIND(a, b) Assert(!bailoutMap.ContainsKey(IR::BailOutKind::a)); bailoutMap.Item( IR::BailOutKind::a, bailoutKindIndexes::a);
#define BAIL_OUT_KIND_VALUE(a, b) Assert(!bailoutMap.ContainsKey(IR::BailOutKind::a)); bailoutMap.Item( IR::BailOutKind::a, bailoutKindIndexes::a);
#define BAIL_OUT_KIND_VALUE_LAST(a, b) Assert(!bailoutMap.ContainsKey(IR::BailOutKind::a)); bailoutMap.Item( IR::BailOutKind::a, bailoutKindIndexes::a);
#include "../../../../core/lib/Backend/BailOutKind.h"
#undef BAIL_OUT_KIND_VALUE_LAST
#undef BAIL_OUT_KIND_VALUE
#undef BAIL_OUT_KIND
#endif // REJIT_STATS
                ScriptContextTelemetry::isInitialized = true;
            }

        }
    }

    void ScriptContextTelemetry::Cleanup()
    {
    }
}

