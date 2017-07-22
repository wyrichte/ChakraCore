/*
 * EngineDataSCT.h
 * This file is meant to be included by ScriptContextTelemetry.cpp a number of times.
 * Reference that file for more context.
 */

#ifndef SCT_STATE
#error "SCT_STATE is undefined - EngineDataSCT is being used improperly!"
#endif

#if SCT_STATE == 0
            // Happens before anything in the telemetry subsystem

            // stats:rejit
            UINT32 rejitReasons[] = {
#define REJIT_REASON(f) crc32(STRINGIZE(f)),
#include "../../core/lib/Common/Common/RejitReasons.h"
#undef REJIT_REASON
            };
            UINT16 rejitReasonCount = _countof(rejitReasons);

            AssertOrFailFast(scriptContext.rejitReasonCounts != nullptr);
            AssertOrFailFast(scriptContext.rejitReasonCountsCap != nullptr);

            UINT64 rejitReasonCounts[_countof(rejitReasons)] = { 0 };
            for (UINT16 i = 0; i < rejitReasonCount; i++)
            {
                rejitReasonCounts[i] = scriptContext.rejitReasonCounts[i];
            }
            UINT64 rejitReasonCountsCap[_countof(rejitReasons)] = { 0 };
            for (UINT16 i = 0; i < rejitReasonCount; i++)
            {
                rejitReasonCountsCap[i] = scriptContext.rejitReasonCountsCap[i];
            }

            // stats:bailout
            UINT32 bailoutKinds[] = {
#define BAIL_OUT_KIND(a, b) crc32(STRINGIZE(a)), 
#define BAIL_OUT_KIND_VALUE(a, b) crc32(STRINGIZE(a)),
#define BAIL_OUT_KIND_VALUE_LAST(a, b) crc32(STRINGIZE(a))
#include "../../core/lib/Backend/BailOutKind.h"
#undef BAIL_OUT_KIND_VALUE_LAST
#undef BAIL_OUT_KIND_VALUE
#undef BAIL_OUT_KIND
            };

            constexpr UINT16 bailoutKindCount = _countof(bailoutKinds);

#ifdef DBG
            ExceptionCheck::Data saved = ExceptionCheck::Save();
            ExceptionCheck::SetHandledExceptionType(ExceptionType::ExceptionType_OutOfMemory);
#endif
            // This is a small, fixed allocation, after everything else has been cleaned up. An
            // exception here will prevent telemetry from being sent.
            JsUtil::BaseDictionary<uint, uint, HeapAllocator> bailoutMap(&HeapAllocator::Instance, bailoutKindCount);
#ifdef DBG
            ExceptionCheck::ClearHandledExceptionType();
            ExceptionCheck::Restore(saved);
#endif

            enum bailoutKindIndexes {
                NO_USE = -1,
#define BAIL_OUT_KIND(a, b) a, 
#define BAIL_OUT_KIND_VALUE(a, b) a,
#define BAIL_OUT_KIND_VALUE_LAST(a, b) a
#include "../../core/lib/Backend/BailOutKind.h"
#undef BAIL_OUT_KIND_VALUE_LAST
#undef BAIL_OUT_KIND_VALUE
#undef BAIL_OUT_KIND
            };
#define BAIL_OUT_KIND(a, b) Assert(!bailoutMap.ContainsKey(IR::BailOutKind::a)); bailoutMap.Item( IR::BailOutKind::a, bailoutKindIndexes::a);
#define BAIL_OUT_KIND_VALUE(a, b) Assert(!bailoutMap.ContainsKey(IR::BailOutKind::a)); bailoutMap.Item( IR::BailOutKind::a, bailoutKindIndexes::a);
#define BAIL_OUT_KIND_VALUE_LAST(a, b) Assert(!bailoutMap.ContainsKey(IR::BailOutKind::a)); bailoutMap.Item( IR::BailOutKind::a, bailoutKindIndexes::a);
#include "../../core/lib/Backend/BailOutKind.h"
#undef BAIL_OUT_KIND_VALUE_LAST
#undef BAIL_OUT_KIND_VALUE
#undef BAIL_OUT_KIND

            AssertOrFailFast(scriptContext.bailoutReasonCounts != nullptr);
            AssertOrFailFast(scriptContext.bailoutReasonCountsCap != nullptr);
            UINT32 bailoutCounts[_countof(bailoutKinds)] = { 0 };
            scriptContext.bailoutReasonCounts->Map([&bailoutCounts, &bailoutMap](uint kind, uint val) {
                if (bailoutMap.ContainsKey(kind)) {
                    bailoutCounts[bailoutMap.Item(kind)] = val;
                }
            });
            UINT32 bailoutCountsCap[_countof(bailoutKinds)] = { 0 };
            scriptContext.bailoutReasonCountsCap->Map([&bailoutCountsCap, &bailoutMap](uint kind, uint val) {
                if (bailoutMap.ContainsKey(kind)) {
                    bailoutCountsCap[bailoutMap.Item(kind)] = val;
                }
            });
// end of #ifdef SCT_STATE == 0


#elif SCT_STATE == 1
        // allocate space for field names
        char UNIQNAME(_TlgNamer)[sizeof("stats_rejit_tags")]; UINT8 UNIQNAME(_TlgInr);
        char UNIQNAME(_TlgNamer)[sizeof("stats_rejit_values")]; UINT8 UNIQNAME(_TlgInr);
        char UNIQNAME(_TlgNamer)[sizeof("stats_rejit_cap")]; UINT8 UNIQNAME(_TlgInr);
        char UNIQNAME(_TlgNameb)[sizeof("stats_bailout_tags")]; UINT8 UNIQNAME(_TlgInb);
        char UNIQNAME(_TlgNameb)[sizeof("stats_bailout_values")]; UINT8 UNIQNAME(_TlgInb);
        char UNIQNAME(_TlgNameb)[sizeof("stats_bailout_cap")]; UINT8 UNIQNAME(_TlgInb);
// end of #elif SCT_STATE == 1


#elif SCT_STATE == 2
        // store the field names and types
        , ("stats_rejit_tags"), TlgInUINT32 | 64
        , ("stats_rejit_values"), TlgInUINT64 | 64
        , ("stats_rejit_cap"), TlgInUINT64 | 64
        , ("stats_bailout_tags"), TlgInUINT32 | 64
        , ("stats_bailout_values"), TlgInUINT32 | 64
        , ("stats_bailout_cap"), TlgInUINT32 | 64
// end of #elif SCT_STATE == 2


#elif SCT_STATE == 3
        // add the number of bytes for the fielddef
        + 2 /*stats_rejit_tags*/
        + 2 /*stats_rejit_values*/
        + 2 /*stats_rejit_cap*/
        + 2 /*stats_bailout_tags*/
        + 2 /*stats_bailout_values*/
        + 2 /*stats_bailout_cap*/
// end of #elif SCT_STATE == 3


#elif SCT_STATE == 4
        // reference the data
        _TlgCreateArray(&_TlgData[_TlgIdx], rejitReasons, rejitReasonCount, sizeof(UINT32)), _TlgIdx += 2,
        _TlgCreateArray(&_TlgData[_TlgIdx], rejitReasonCounts, rejitReasonCount, sizeof(UINT64)), _TlgIdx += 2,
        _TlgCreateArray(&_TlgData[_TlgIdx], rejitReasonCountsCap, rejitReasonCount, sizeof(UINT64)), _TlgIdx += 2,
        _TlgCreateArray(&_TlgData[_TlgIdx], bailoutKinds, bailoutKindCount, sizeof(UINT32)), _TlgIdx += 2,
        _TlgCreateArray(&_TlgData[_TlgIdx], bailoutCounts, bailoutKindCount, sizeof(UINT32)), _TlgIdx += 2,
        _TlgCreateArray(&_TlgData[_TlgIdx], bailoutCountsCap, bailoutKindCount, sizeof(UINT32)), _TlgIdx += 2,
// end of #elif SCT_STATE == 4


#elif SCT_STATE == 5
        // do any necessary clean-up
        // Bailout and Rejit maps are cleared in ScriptContext::PrintStats
#undef STRINGOF
// end of #elif SCT_STATE == 5


#elif SCT_STATE == -1
        // run only at compile-time to generate integrity checks
// end of #elif SCT_STATE == -1


#elif SCT_STATE == -2
        // register the telemetry provider with the ScriptContextTelemetry
        this->engineDataTelemetryProvider = Anew(scriptContext.TelemetryAllocator(), EngineDataTelemetryProvider, *this);
        this->telemetryProviders.Add( this->engineDataTelemetryProvider );
// end of #elif SCT_STATE == -2


#elif SCT_STATE == -3
        // header definition of local property
        EngineDataTelemetryProvider* engineDataTelemetryProvider;
// end of #elif SCT_STATE == -3


#else
#error "SCT_STATE value not supported by EngineDataSCT!"
#endif
