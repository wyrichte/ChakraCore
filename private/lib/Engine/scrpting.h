//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Description: Top level declarations compiled scripts: CscriptBody and ExecBody.


#pragma once

class ScriptEngine;
class CScriptSourceDocumentText;

// Beginning of statement descriptor
struct StatementSpan
{
    long ich;
    long cch;
};

/***************************************************************************
    CScriptBody wraps an ExecBody. This adds script specific state to an
    executable block. This is not intended to be shared between scripts.
***************************************************************************/
class CScriptBody
{
private:
    long m_refCount;
    Js::Utf8SourceInfo* m_utf8SourceInfo;
    CScriptSourceDocumentText * m_scriptDocumentText;
    ScriptEngine * m_scriptEngine;
    RecyclerRootPtr<Js::FunctionBody> functionBody;
    
    BreakpointProbeList* m_breakpointList;

    ~CScriptBody(void);
    BreakpointProbeList* GetBreakpointList();
    BreakpointProbeList* NewBreakpointList(ArenaAllocator* arena);
public:
    CScriptBody(Js::FunctionBody* functionBody, ScriptEngine *scriptEngine, Js::Utf8SourceInfo* sourceInfo);
    CScriptBody * Clone(ScriptEngine *pos);

    // reference count stuff
    void AddRef(void) { InterlockedIncrement(&m_refCount); }
    void Release(void);

    Js::ScriptContext* GetScriptContext();
    Js::FunctionBody *GetRootFunction() { return this->functionBody; }

    void RemoveBreakpointProbe(BreakpointProbe *probe);

    void SetDoc(CScriptSourceDocumentText *pdoc);
    CScriptSourceDocumentText *GetScriptDocumentText(void) { return m_scriptDocumentText; }

    // Set or clear a breakpoint at the given StatementSpan
    HRESULT SetBreakPoint(long ibos, BREAKPOINT_STATE bps);

    // Clear all break points
    void ClearAllBreakPoints(void);

    Js::Utf8SourceInfo* GetUtf8SourceInfo() const
    {
        return m_utf8SourceInfo;
    }

    ScriptEngine * GetScriptEngine() const
    {
        return m_scriptEngine;
    }

    BOOL CScriptBody::HasLineBreak(long _start, long _end);

    BOOL GetStatementSpan(long ibos, StatementSpan* pBos);
    BOOL GetStatementLocation(long ibos, Js::StatementLocation* plocation);
    Js::FunctionBody* ContainsFunction(Js::FunctionBody* pFunction);

    Js::FunctionBody * GetFunctionBodyAt(long ibos);
    void *PvGetData(long *pcb)
    {
        *pcb = (long)sizeof(SRCINFO);
        return (void*)m_utf8SourceInfo->GetSrcInfo();
    }

    DWORD_PTR GetSecondaryHostSourceContext()
    {
        return this->m_utf8SourceInfo->GetSecondaryHostSourceContext();
    }

    DWORD_PTR GetHostSourceContext()
    {
        return this->m_utf8SourceInfo->GetSrcInfo()->sourceContextInfo->dwHostSourceContext;
    }

    Js::DynamicObject * CreateEntryPoint(
        __in ScriptSite* psess);
};

