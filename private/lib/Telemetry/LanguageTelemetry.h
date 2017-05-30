//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#pragma once

#define CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(builtin) LanguageStats* stats = scriptContext->GetThreadContext()->GetLanguageStats();stats->builtin.callCount++;\
                                                      if(scriptContext->IsScriptContextInDebugMode()){ stats->builtin.debugModeCallCount++;}

#define CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(feature,m_scriptContext) if(m_scriptContext != nullptr){Js::LanguageStats* stats = m_scriptContext->GetThreadContext()->GetLanguageStats();\
                                                          if(stats!=nullptr){stats->feature.parseCount++;}}

#define CHAKRATEL_LANGSTATS_INC_DATACOUNT(feature) scriptContext->GetThreadContext()->GetLanguageStats()->feature.count++;

namespace Js
{
    struct props
    {
        UINT32 callCount;
        UINT32 debugModeCallCount;
    };

    struct langFeature
    {
        uint parseCount;
    };

    struct telPoint
    {
        uint count;
    };
    // Anything that needs to be logged as part of Language Stats should be a part of this structure
    struct LanguageStats
    {
     #define BLOCK_START(blockname, count)
     #define BLOCK_END()
     #define ENTRY_BUILTIN(ver, base, type, n) Js::props base ## _ ## type ## _ ## n;
     #define ENTRY_LANGFEATURE(ver, n) Js::langFeature n;
     #define ENTRY_TELPOINT(n) Js::telPoint n;
     #include "ESBuiltins/ESBuiltinFields.h"
     #undef ENTRY_TELPOINT
     #undef ENTRY_LANGFEATURE
     #undef ENTRY_BUILTIN
     #undef BLOCK_END
     #undef BLOCK_START
    };

    class LanguageTelemetry
    {

    private:
        LanguageStats stats;    
        
    public:
        LanguageTelemetry()
        {
            this->Reset();            
        }

        void Reset() 
        { 
            stats = {0};
        }       
        
        LanguageStats* GetLanguageStats()
        {    
            return &stats;
        }

    };
} // namespace Js.
