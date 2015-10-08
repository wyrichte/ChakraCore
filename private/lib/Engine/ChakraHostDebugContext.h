//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class ChakraHostDebugContext sealed : public HostDebugContext
{
public:
    ChakraHostDebugContext(ScriptSite * scriptSite)
        : HostDebugContext(scriptSite->GetScriptSiteContext())
    {
        this->scriptSite = scriptSite;
        scriptSite->AddRef();
    }
    ~ChakraHostDebugContext()
    {
        scriptSite->Release();
    }

    virtual void Delete()
    {
        HeapDelete(this);
    }

    DWORD_PTR GetHostSourceContext(Js::Utf8SourceInfo * sourceInfo)
    {
        DWORD_PTR dwDebugHostSourceContext = Js::Constants::NoHostSourceContext;

        ScriptEngine* scriptEngine = this->GetScriptSite()->GetScriptEngine();
        SourceContextPair* pSourceContextPairs = scriptEngine->GetSourceContextPairs();

        for (ULONG i = 0; i < scriptEngine->GetSourceContextPairCount(); i++)
        {
            if (pSourceContextPairs[i].dwHostSourceContext == sourceInfo->GetHostSourceContext() &&
                pSourceContextPairs[i].ulCharOffset == sourceInfo->GetSrcInfo()->ulCharOffset)
            {
                dwDebugHostSourceContext = pSourceContextPairs[i].dwDebugHostSourceContext;
                break;
            }
        }
#if DBG
        if (dwDebugHostSourceContext != Js::Constants::NoHostSourceContext)
        {
            // then it must be hostmanaged.
            Assert(sourceInfo->IsHostManagedSource());
        }
        else
        {
            Assert(!sourceInfo->IsHostManagedSource());
        }
#endif
        return dwDebugHostSourceContext;
    }

    HRESULT SetThreadDescription(__in LPCWSTR url)
    {
        return this->GetScriptSite()->GetScriptEngine()->SetThreadDescription(url);
    }

    HRESULT DbgRegisterFunction(Js::ScriptContext * scriptContext, Js::FunctionBody * functionBody, DWORD_PTR dwDebugSourceContext, LPCWSTR title)
    {
        return this->GetScriptSite()->GetScriptEngine()->DbgRegisterFunction(scriptContext, functionBody, dwDebugSourceContext, title);
    }

    void ReParentToCaller(Js::Utf8SourceInfo* sourceInfo)
    {
        if (sourceInfo && sourceInfo->HasDebugDocument())
        {
            ScriptDebugDocument* document = static_cast<ScriptDebugDocument*>(sourceInfo->GetDebugDocument());
            document->ReParentToCaller();
        }
    }

    ScriptSite * GetScriptSite() { return scriptSite; }
private:
    ScriptSite * scriptSite;
};
