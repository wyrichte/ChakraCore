//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include "ScriptContext/ESBuiltInsCountTracker.h"
#include "ScriptContext/LanguageFeaturesCountTracker.h"
#include "Runtime.h"
#include "Core/CriticalSection.h"

namespace Js
{
    class ScriptContextTelemetry
    {
    private:
        static CriticalSection ScriptContextTelemetry::initLock;
        static volatile bool ScriptContextTelemetry::isInitialized;

        ScriptContext* scriptContext;

        BuiltInCountTracker builtInCountTracker;
        LanguageFeaturesCountTracker languageFeaturesCountTracker;

#ifdef REJIT_STATS
        // Map from IR::BailOutKind uint to index in the bailoutKindCRCs[] above.
        static JsUtil::BaseDictionary<uint, uint, HeapAllocator> bailoutMap;

        enum bailoutKindIndexes
        {
            NO_USE = -1,
#define BAIL_OUT_KIND(a, b) a, 
#define BAIL_OUT_KIND_VALUE(a, b) a,
#define BAIL_OUT_KIND_VALUE_LAST(a, b) a,
#include "../../../../core/lib/Backend/BailOutKind.h"
#undef BAIL_OUT_KIND_VALUE_LAST
#undef BAIL_OUT_KIND_VALUE
#undef BAIL_OUT_KIND
            _COUNT,
        };
#endif


    public:
        ScriptContextTelemetry(ScriptContext* sc) :
            scriptContext(sc)
        {
            // do static initialization
            if (!ScriptContextTelemetry::isInitialized)
            {
                ScriptContextTelemetry::Initialize();
            }
        }

        BuiltInCountTracker& GetBuiltInCountTracker() { return this->builtInCountTracker; }
        LanguageFeaturesCountTracker& GetLanguageFeaturesCountTracker() { return this->languageFeaturesCountTracker; }

        void OutputTraceLogging(GUID activityId, DWORD hostType, bool isJSRT);

        void OutputPrint()
        {
        }

        void Reset()
        {
            this->builtInCountTracker.Reset();
            this->languageFeaturesCountTracker.Reset();
        }

        static void Initialize();

        static void Cleanup();

        BuiltInCountTracker& GetOpcodeTelemetry() { return this->builtInCountTracker; }
    };
}