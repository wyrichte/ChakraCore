/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "EnginePch.h"
#pragma hdrstop

CScriptBody::CScriptBody(Js::ParseableFunctionInfo* functionInfo, ScriptEngine *scriptEngine, Js::Utf8SourceInfo* utf8SourceInfo) : 
    m_refCount(1),
    m_scriptEngine(scriptEngine),
    m_scriptDocumentText(nullptr),
    m_utf8SourceInfo(utf8SourceInfo)
{
    scriptEngine->AddRef();
    if(functionInfo != nullptr)
    {
        this->functionInfo.Root(functionInfo, m_scriptEngine->GetScriptContext()->GetRecycler());
    }
}

CScriptBody::~CScriptBody()
{
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    Assert(threadContext != nullptr);

    Recycler* recycler = threadContext->GetRecycler();
    Assert(recycler != nullptr);

    if(functionInfo)
    {
        functionInfo.Unroot(recycler);
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
    Assert(functionInfo->HasBody());
    return scriptContext->GetLibrary()->CreateScriptFunction(functionInfo);
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
