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

    void SortMembersList(JsUtil::List<Js::DebuggerPropertyDisplayInfo *, ArenaAllocator> * pMembersList, Js::ScriptContext* scriptContext)
    {
        pMembersList->Sort(ElementsComparer, scriptContext);
    }

    ScriptSite * GetScriptSite() { return scriptSite; }
private:
    ScriptSite * scriptSite;

    static int __cdecl ElementsComparer(__in void* context, __in const void* item1, __in const void* item2)
    {
        Js::ScriptContext *scriptContext = (Js::ScriptContext *)context;
        Assert(scriptContext);

        const DWORD_PTR *p1 = reinterpret_cast<const DWORD_PTR*>(item1);
        const DWORD_PTR *p2 = reinterpret_cast<const DWORD_PTR*>(item2);

        Js::DebuggerPropertyDisplayInfo * pPVItem1 = (Js::DebuggerPropertyDisplayInfo *)(*p1);
        Js::DebuggerPropertyDisplayInfo * pPVItem2 = (Js::DebuggerPropertyDisplayInfo *)(*p2);

        const Js::PropertyRecord *propertyRecord1 = scriptContext->GetPropertyName(pPVItem1->propId);
        const Js::PropertyRecord *propertyRecord2 = scriptContext->GetPropertyName(pPVItem2->propId);

        const char16 *str1 = propertyRecord1->GetBuffer();
        const char16 *str2 = propertyRecord2->GetBuffer();

        // Do the natural comparison, for example test2 comes before test11.
        return StrCmpLogicalW(str1, str2);
    }
};
