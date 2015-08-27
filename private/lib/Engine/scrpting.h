//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Description: Top level declarations compiled scripts: CscriptBody and ExecBody.


#pragma once

class ScriptEngine;
class CScriptSourceDocumentText;


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

    ~CScriptBody(void);
public:
    CScriptBody(Js::FunctionBody* functionBody, ScriptEngine *scriptEngine, Js::Utf8SourceInfo* sourceInfo);
    CScriptBody * Clone(ScriptEngine *pos);

    // reference count stuff
    void AddRef(void) { InterlockedIncrement(&m_refCount); }
    void Release(void);

    Js::ScriptContext* GetScriptContext();
    Js::FunctionBody *GetRootFunction() { return this->functionBody; }

    void SetDoc(CScriptSourceDocumentText *pdoc);
    CScriptSourceDocumentText *GetScriptDocumentText(void) { return m_scriptDocumentText; }

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

    BOOL GetStatementSpan(long ibos, StatementSpan* pBos);

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

    Js::DynamicObject * CreateEntryPoint(ScriptSite* psess);
};

