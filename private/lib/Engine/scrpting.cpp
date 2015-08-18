
/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "StdAfx.h"
#pragma hdrstop

// Function to use when comparing identifiers. JScript is case sensitive,

CScriptBody::CScriptBody(Js::FunctionBody* functionBody, ScriptEngine *scriptEngine, Js::Utf8SourceInfo* utf8SourceInfo) : 
    m_refCount(1),
    m_scriptEngine(scriptEngine),
    m_breakpointList(NULL),
    m_scriptDocumentText(NULL),
    m_utf8SourceInfo(utf8SourceInfo)
{
    scriptEngine->AddRef();
    if(functionBody != nullptr)
    {
        this->functionBody.Root(functionBody, m_scriptEngine->GetScriptContext()->GetRecycler());
    }
}

CScriptBody *
CScriptBody::Clone(ScriptEngine * pos)
{
    Assert(pos->GetScriptContext() != null);

    Js::ScriptContext* newScriptContext = pos->GetScriptContext();

    Js::FunctionBody* newFunctionBody = GetRootFunction()->Clone(newScriptContext);

    // Finally, create the new script body, point it to the new global function, and pass it back to the caller.
    // This is called by ScriptEngine::CloneScriptBodies which handles OOMs so it is ok to throw here
    return HeapNew(CScriptBody, newFunctionBody, pos, newFunctionBody->GetUtf8SourceInfo()); 

}

CScriptBody::~CScriptBody()
{
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    Assert(threadContext != null);

    Recycler* recycler = threadContext->GetRecycler();
    Assert(recycler != null);

    if(functionBody)
    {
        functionBody.Unroot(recycler);
    }

    m_scriptEngine->Release();
    m_scriptEngine = NULL;
}

Js::ScriptContext* CScriptBody::GetScriptContext()
{
    return m_scriptEngine->GetScriptContext();    
}

BreakpointProbeList* CScriptBody::GetBreakpointList()
{
    if (m_breakpointList)
    {
        return m_breakpointList;
    }

    Js::ScriptContext * pScriptContext = this->GetScriptContext();
    if (!pScriptContext)
    {
        return NULL;
    }

    ArenaAllocator* pAlloc = pScriptContext->AllocatorForDiagnostics();
    AssertMem(pAlloc);

    m_breakpointList = this->NewBreakpointList(pAlloc);
    return m_breakpointList;
}

void CScriptBody::RemoveBreakpointProbe(BreakpointProbe *probe)
{
    Assert(probe);
    if (m_breakpointList)
    {
        m_breakpointList->Remove(probe);
    }
}


BreakpointProbeList* CScriptBody::NewBreakpointList(ArenaAllocator* arena)
{
    return BreakpointProbeList::New(arena);
}

void CScriptBody::Release(void)
{
    if (0 == ::InterlockedDecrement(&m_refCount))
    {
        delete this;
    }
}

Js::DynamicObject * 
CScriptBody::CreateEntryPoint(ScriptSite* scriptSite)
{
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    Assert(functionBody->HasBody());
    return scriptContext->GetLibrary()->CreateScriptFunction(functionBody);
}

void CScriptBody::ClearAllBreakPoints(void)
{
    // The Diagnostics arena has already gone down, so all breakpoint probes are anyway not valid.

    // Just clearing the list.
    if (m_breakpointList)
    {
        m_breakpointList->Clear();
    }
}

void CScriptBody::SetDoc(CScriptSourceDocumentText *scriptDocumentText)
{
    if (scriptDocumentText == m_scriptDocumentText)
        return;
    if (NULL != m_scriptDocumentText)
    {
        CScriptSourceDocumentText *pdoc = m_scriptDocumentText;
        m_scriptDocumentText = NULL;
        pdoc->SetScriptBody(NULL);
        pdoc->Release();
    }
    if (NULL != scriptDocumentText)
    {
        m_scriptDocumentText = scriptDocumentText;
        m_scriptDocumentText->AddRef();
        m_scriptDocumentText->SetScriptBody(this);
    }
}

HRESULT CScriptBody::SetBreakPoint(long ibos, BREAKPOINT_STATE bps)
{
    HRESULT hr = NOERROR;
    ScriptSite *scriptSite = m_scriptEngine->GetScriptSiteHolder();
    if (NULL == scriptSite)
        return E_UNEXPECTED;
    Js::ScriptContext* pScriptContext = scriptSite->GetScriptSiteContext();

    switch (bps)
    {
    default:
        AssertMsg(FALSE, "bad bps");
        // fall thru
    case BREAKPOINT_DISABLED:
    case BREAKPOINT_DELETED:
        {
            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                BreakpointProbeList* pBreakpointList = this->GetBreakpointList();
                if (pBreakpointList)
                {
                    ArenaAllocator arena(L"TemporaryBreakpointList", pScriptContext->GetThreadContext()->GetDebugManager()->GetDiagnosticPageAllocator(), Js::Throw::OutOfMemory);
                    BreakpointProbeList* pDeleteList = this->NewBreakpointList(&arena);
                    Js::StatementLocation statement;
                    if (!this->GetStatementLocation(ibos, &statement))
                    {
                        return E_FAIL;
                    }
 
                    pBreakpointList->Map([&statement, pScriptContext, pDeleteList](int index, BreakpointProbe * breakpointProbe)
                    {
                        if (breakpointProbe->Matches(statement.function, statement.statement.begin))
                        {
                            pScriptContext->GetDebugContext()->GetProbeContainer()->RemoveProbe(breakpointProbe);
                            pDeleteList->Add(breakpointProbe);
                        }
                    });                    

                    pDeleteList->Map([pBreakpointList](int index, BreakpointProbe * breakpointProbe)
                    {                        
                        pBreakpointList->Remove(breakpointProbe);
                    });
                    pDeleteList->Clear();
                }
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr);

            break;
        }
    case BREAKPOINT_ENABLED:
        {
            Js::StatementLocation statement;
            if(!this->GetStatementLocation(ibos, &statement))
            {
                return E_FAIL;
            }

            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                BreakpointProbe* pProbe = Anew(pScriptContext->AllocatorForDiagnostics(), BreakpointProbe, this, statement);
                pScriptContext->GetDebugContext()->GetProbeContainer()->AddProbe(pProbe);
                BreakpointProbeList* pBreakpointList = this->GetBreakpointList();
                pBreakpointList->Add(pProbe);
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr);

            break;
        }
    }
    return hr;
}

Js::FunctionBody* CScriptBody::ContainsFunction(Js::FunctionBody* pFunction)
{
    if (pFunction && 
        pFunction->GetUtf8SourceInfo() == m_utf8SourceInfo)
    {
        return pFunction;
    }
    return NULL;
}

BOOL CScriptBody::HasLineBreak(long _start, long _end)
{
    return this->functionBody->HasLineBreak(_start, _end);
}


// The next two functions need to translate from a character location to the 'matching' statement in a script.
// They implictly define a partition of the source and assign each range to a statements or failure.
// The partition defined implements the user's expectation of look ahead/look behind for breakpoint binding.

BOOL CScriptBody::GetStatementSpan(long ibos, StatementSpan* pStatement)
{
    Js::StatementLocation statement;
    if (GetStatementLocation(ibos,&statement))
    {
        pStatement->ich = statement.statement.begin;
        pStatement->cch = statement.statement.end - statement.statement.begin;
        return TRUE;
    }
    return FALSE;
}

Js::FunctionBody * CScriptBody::GetFunctionBodyAt(long ibos)
{
    Js::StatementLocation location = {};
    if (GetStatementLocation(ibos, &location))
    {
        return location.function;
    }

    return NULL;
}

BOOL CScriptBody::GetStatementLocation(long ibos, Js::StatementLocation* plocation)
{
    if (ibos < 0)
    {
        return FALSE;
    }
    // If the script body is registered due to syntax error - function body will be NULL
    if(functionBody == NULL)
    {
        return false;
    }

    ulong ubos = static_cast<ulong>(ibos);

    // Getting the appropiate statement on the asked position works on the heuristic which requires two
    // probable candidates.
    // These candidates will be closest to the ibos. where the first.range.start < ibos and 
    // the second.range.start >= ibos.
    // These candidates will be fetched out by going into each FunctionBody.

    Js::StatementLocation candidateMatch1 = {};
    Js::StatementLocation candidateMatch2 = {};

    ScriptSite *activeScriptSite = NULL;
    if (m_scriptEngine == NULL || (activeScriptSite = m_scriptEngine->GetScriptSiteHolder()) == NULL)
    {
        return FALSE;
    }

    Js::ScriptContext* pScriptContext = activeScriptSite->GetScriptSiteContext();
    Assert(pScriptContext);

    DWORD_PTR hostContext = this->functionBody->GetHostSourceContext();    
    this->m_utf8SourceInfo->MapFunction([&] (Js::FunctionBody* pFuncBody)  
    {
        Assert(pFuncBody->GetHostSourceContext() == hostContext);
        Assert(pFuncBody->GetSecondaryHostSourceContext() == this->functionBody->GetSecondaryHostSourceContext());

        ulong functionStart = pFuncBody->StartInDocument();
        ulong functionEnd = functionStart + pFuncBody->LengthInBytes();

        bool funcContains = functionStart <= ubos && ubos < functionEnd;

        // For the first candidate, we should allow the current function to participate if its range (instead of just start offset) is closer to the ubos compared to already found candidate1.

        if (candidateMatch1.function == NULL
            || ((candidateMatch1.statement.begin <= static_cast<int>(functionStart) || 
                    candidateMatch1.statement.end <= static_cast<int>(functionEnd)) 
                && ubos > functionStart
                )
            || candidateMatch2.function == NULL
            || (candidateMatch2.statement.begin > static_cast<int>(functionStart)
                && ubos <= functionStart
                )
            || funcContains
            )
        {
            // We need to find out two possible candidate fromn the current FunctionBody.
            pFuncBody->FindClosestStatements(ibos, &candidateMatch1, &candidateMatch2);
        }
    });


    if (candidateMatch1.function == NULL && candidateMatch2.function == NULL)
    {
        // No Match found
        return FALSE;
    }

    if (candidateMatch1.function == NULL || candidateMatch2.function == NULL)
    {
        *plocation = (candidateMatch1.function == NULL) ? candidateMatch2 : candidateMatch1;

        return TRUE;
    }

    // If one of the func is inner to another one, and ibos is in the inner one, disregard the outer one/let the inner one win. 
    // See WinBlue 575634. Scenario is like this: var foo = function () {this;} -- and BP is set to 'this;' 'function'.
    if (candidateMatch1.function != candidateMatch2.function)
    {
        Assert(candidateMatch1.function && candidateMatch2.function);

        regex::Interval func1Range(candidateMatch1.function->StartInDocument());
        func1Range.End(func1Range.Begin() + candidateMatch1.function->LengthInBytes());
        regex::Interval func2Range(candidateMatch2.function->StartInDocument());
        func2Range.End(func2Range.Begin() + candidateMatch2.function->LengthInBytes());

        if (func1Range.Includes(func2Range) && func2Range.Includes(ibos))
        {
            *plocation = candidateMatch2;
            return TRUE;
        }
        else if (func2Range.Includes(func1Range) && func1Range.Includes(ibos))
        {
            *plocation = candidateMatch1;
            return TRUE;
        }
    }

    // At this point we have both candidate to consider.

    Assert(candidateMatch1.statement.begin < candidateMatch2.statement.begin);
    Assert(candidateMatch1.statement.begin < ibos);
    Assert(candidateMatch2.statement.begin >= ibos);

    // Default selection
    *plocation = candidateMatch1;

    // if the second candidate start at ibos or
    // If the first candidate has line break between ibos, and the second candidate is on the same line as ibos.
    // consider the second one.

    BOOL fNextHasLineBreak = HasLineBreak(ibos, candidateMatch2.statement.begin);

    if ((candidateMatch2.statement.begin == ibos)
        || (HasLineBreak(candidateMatch1.statement.begin,  ibos) && !fNextHasLineBreak))
    {
        *plocation = candidateMatch2;
    }
    // If ibos is out of the range of first candidate, choose second candidate if  ibos is on the same line as second candidate 
    // or ibos is not on the same line of the end of the first candidate.
    else if (candidateMatch1.statement.end < ibos && (!fNextHasLineBreak || HasLineBreak(candidateMatch1.statement.end, ibos)))
    {
        *plocation = candidateMatch2;
    }

    return TRUE;
}
