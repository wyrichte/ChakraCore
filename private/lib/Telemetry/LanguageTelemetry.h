//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#pragma once

#define CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(builtin) LanguageStats* stats = scriptContext->GetThreadContext()->GetLanguageStats();stats->builtin.callCount++;\
                                                      if(scriptContext->IsScriptContextInDebugMode()){ stats->builtin.debugModeCallCount++;}

#define CHAKRATEL_LANGSTATS_INC_LANGFEATURECOUNT(feature,m_scriptContext) if(m_scriptContext != nullptr){Js::LanguageStats* stats = m_scriptContext->GetThreadContext()->GetLanguageStats();\
                                                          if(stats!=nullptr){stats->feature.parseCount++;}}

namespace Js
{
    struct props
    {
        uint callCount;
        uint debugModeCallCount;
    };

    struct langFeature
    {
        uint parseCount;
    };
    // Anything that needs to be logged as part of Language Stats should be a part of this structure
    struct LanguageStats
    {
     #define ENTRY_BUILTIN(ver, n) Js::props n;
     #define ENTRY_LANGFEATURE(ver, n) Js::langFeature n;
     #include "LangTelFields.h"
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
