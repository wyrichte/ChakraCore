/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "StdAfx.h"
#pragma hdrstop

CScriptBody::CScriptBody(Js::FunctionBody* functionBody, ScriptEngine *scriptEngine, Js::Utf8SourceInfo* utf8SourceInfo) : 
    m_refCount(1),
    m_scriptEngine(scriptEngine),
    m_scriptDocumentText(nullptr),
    m_utf8SourceInfo(utf8SourceInfo)
{
    scriptEngine->AddRef();
    if(functionBody != nullptr)
    {
        this->functionBody.Root(functionBody, m_scriptEngine->GetScriptContext()->GetRecycler());
    }
}

CScriptBody* CScriptBody::Clone(ScriptEngine * pos)
{
    Assert(pos->GetScriptContext() != nullptr);

    Js::ScriptContext* newScriptContext = pos->GetScriptContext();

    Js::FunctionBody* newFunctionBody = GetRootFunction()->Clone(newScriptContext);

    // Finally, create the new script body, point it to the new global function, and pass it back to the caller.
    // This is called by ScriptEngine::CloneScriptBodies which handles OOMs so it is ok to throw here
    return HeapNew(CScriptBody, newFunctionBody, pos, newFunctionBody->GetUtf8SourceInfo()); 
}

CScriptBody::~CScriptBody()
{
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    Assert(threadContext != nullptr);

    Recycler* recycler = threadContext->GetRecycler();
    Assert(recycler != nullptr);

    if(functionBody)
    {
        functionBody.Unroot(recycler);
    }
    m_scriptEngine->Release();
    m_scriptEngine = nullptr;
}

Js::ScriptContext* CScriptBody::GetScriptContext()
{
    return m_scriptEngine->GetScriptContext();
}

void CScriptBody::Release(void)
{
    if (0 == ::InterlockedDecrement(&m_refCount))
    {
        delete this;
    }
}

Js::DynamicObject* CScriptBody::CreateEntryPoint(ScriptSite* scriptSite)
{
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    Assert(functionBody->HasBody());
    return scriptContext->GetLibrary()->CreateScriptFunction(functionBody);
}

void CScriptBody::ClearAllBreakPoints(void)
{
    if (this->GetUtf8SourceInfo()->HasDebugDocument())
    {
        this->GetUtf8SourceInfo()->GetDebugDocument()->ClearAllBreakPoints();
    }
}

void CScriptBody::SetDoc(CScriptSourceDocumentText *scriptDocumentText)
{
    if (scriptDocumentText == m_scriptDocumentText)
        return;
    if (nullptr != m_scriptDocumentText)
    {
        CScriptSourceDocumentText *pdoc = m_scriptDocumentText;
        m_scriptDocumentText = nullptr;
        pdoc->SetScriptBody(nullptr);
        pdoc->Release();
    }
    if (nullptr != scriptDocumentText)
    {
        m_scriptDocumentText = scriptDocumentText;
        m_scriptDocumentText->AddRef();
        m_scriptDocumentText->SetScriptBody(this);
    }
}

BOOL CScriptBody::GetStatementSpan(long ibos, StatementSpan* pStatement)
{
    return this->GetUtf8SourceInfo()->GetDebugDocument()->GetStatementSpan(ibos, pStatement);
}

Js::FunctionBody* CScriptBody::GetFunctionBodyAt(long ibos)
{
    return this->GetUtf8SourceInfo()->GetDebugDocument()->GetFunctionBodyAt(ibos);
}
