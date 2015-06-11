//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline JavascriptExceptionOperators::AutoCatchHandlerExists::AutoCatchHandlerExists(ScriptContext* scriptContext)
    {
        Assert(scriptContext);
        m_threadContext = scriptContext->GetThreadContext();
        Assert(m_threadContext);
        m_previousCatchHandlerExists = m_threadContext->HasCatchHandler();
        m_threadContext->SetHasCatchHandler(TRUE);
        m_previousCatchHandlerToUserCodeStatus = m_threadContext->IsUserCode();
        if (scriptContext->IsInDebugMode() && !BinaryFeatureControl::LanguageService())
        {
            FetchNonUserCodeStatus(scriptContext);
        }
    }

    inline JavascriptExceptionOperators::AutoCatchHandlerExists::~AutoCatchHandlerExists()
    {
        m_threadContext->SetHasCatchHandler(m_previousCatchHandlerExists);
        m_threadContext->SetIsUserCode(m_previousCatchHandlerToUserCodeStatus);
    }

    inline bool JavascriptExceptionOperators::CrawlStackForWER(Js::ScriptContext& scriptContext)
    {
        return Js::Configuration::Global.flags.WERExceptionSupport && ! scriptContext.GetThreadContext()->HasCatchHandler();
    }

    inline uint64 JavascriptExceptionOperators::StackCrawlLimitOnThrow(Var thrownObject, ScriptContext& scriptContext)
    {
        return CrawlStackForWER(scriptContext) ? MaxStackTraceLimit : GetStackTraceLimit(thrownObject, &scriptContext);
    }

} // namespace Js
