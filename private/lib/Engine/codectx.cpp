/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"
#include "Language\InterpreterStackFrame.h"

#pragma warning(push)
#pragma warning(disable:4211)       // nonstandard extension used: redefined extern to static
// TEMP: Until we get an msdbg.lib with the definition.
static const GUID IID_ISetNextStatement = { 0x51973C03, 0xCB0C, 0x11D0, { 0xB5, 0xC9, 0x00, 0xA0, 0x24, 0x4A, 0x0E, 0x7A } };
static const GUID IID_IDebugSetValueCallback = { 0xe0284f01, 0xeda1, 0x11d0, { 0xb4, 0x52, 0x00, 0xa0, 0x24, 0x4a, 0x1d, 0xd2 } };
#pragma warning(pop)

static const GUID IID_IInternalCodeContext = { 0x50467580, 0x71b7, 0x11d1, { 0x8c, 0xAE, 0x00, 0xa0, 0xc9, 0x0f, 0xff, 0xc0 } };

#if defined(DBG) || defined(ENABLE_TRACE)
#define VALIDATE_LEGIT_DEBUGSESSION()   ValidateLegitDebugSession()

void CDebugStackFrame::ValidateLegitDebugSession()
{
    Js::DebugManager * debugManager = m_scriptSite->GetScriptSiteContext()->GetThreadContext()->GetDebugManager();
    OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::ValidateLegitDebugSession: m_debugSessionNumber=%d, probeManager->GetDebugSessionNumber()=%d\n"),
        m_debugSessionNumber, debugManager->GetDebugSessionNumber());
    Assert(debugManager->IsAtDispatchHalt());
    AssertMsg(m_debugSessionNumber == debugManager->GetDebugSessionNumber(), "Stack frame is outdated");
}

#else
#define VALIDATE_LEGIT_DEBUGSESSION()
#endif

//***************************************************************************
// Private helper method
//***************************************************************************
bool IfFlagSet(DWORD value, DWORD flag)
{
    bool retValue = (value & flag) == flag;
    return retValue;
}

//***************************************************************************
// Code context
//***************************************************************************
CCodeContext::CCodeContext(ScriptDebugDocument *debugDocument, long ibos, ULONG cch, bool isLibraryCode/*=false*/)
    : m_refCount(1),
    m_debugDocument(debugDocument),
    m_ibos(ibos),
    m_cch(cch),
    m_isLibraryCode(isLibraryCode)
{
    DLLAddRef();
    if (m_debugDocument)
    {
        Assert(!m_isLibraryCode); // LibraryCode doesn't have debugDocument
        m_debugDocument->AddRef();
    }
}


CCodeContext::~CCodeContext(void)
{
    Close();
    DLLRelease();    
}


HRESULT CCodeContext::Close(void)
{
    if (m_debugDocument != nullptr)
    {
        m_debugDocument->Release();
        m_debugDocument = nullptr;
    }
    m_ibos = -1L;
    return NOERROR;
}

CScriptBody *CCodeContext::Pbody(void)
{
    return (m_debugDocument != nullptr) ? m_debugDocument->GetScriptBody() : nullptr;
}

ULONG CCodeContext::AddRef(void)
{
    return (ulong)InterlockedIncrement(&m_refCount);
}

ULONG CCodeContext::Release(void)
{
    ulong lu = (ulong)InterlockedDecrement(&m_refCount);
    if (lu == 0)
        delete this;
    return lu;
}

HRESULT CCodeContext::QueryInterface(REFIID iid, void ** ppv)
{
    CHECK_POINTER(ppv);
    AssertMem(ppv);

    if (iid == IID_IUnknown || iid == __uuidof(IDebugCodeContext))
        *ppv = (IDebugCodeContext *)this;
    else if (iid == IID_IInternalCodeContext)
        *ppv = (IInternalCodeContext *)this;
    else
    {
        *ppv = nullptr;
        return HR(E_NOINTERFACE);
    }

    AddRef();
    return NOERROR;
}


HRESULT CCodeContext::GetDocumentContext(IDebugDocumentContext **ppDebugDocumentContext)
{
    CHECK_POINTER(ppDebugDocumentContext);
    *ppDebugDocumentContext = nullptr;

    if (m_debugDocument != nullptr)
    {
        return m_debugDocument->GetDocumentContext(m_ibos, m_cch, ppDebugDocumentContext);
    }

    return m_isLibraryCode ? NOERROR : E_FAIL;
}


HRESULT CCodeContext::SetBreakPoint(BREAKPOINT_STATE bps)
{
    HRESULT hr = E_FAIL;
    if (m_debugDocument != nullptr && m_debugDocument->GetScriptBody())
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            hr = m_debugDocument->SetBreakPoint(m_ibos, bps);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
    }
    return hr;
}


HRESULT CCodeContext::GetCodeContextClass(CCodeContext **ppCodeContext)
{
    CHECK_POINTER(ppCodeContext);
    *ppCodeContext = this;
    return NOERROR;
}

//***************************************************************************
// CEnumCodeContexts
//***************************************************************************

CEnumCodeContexts::CEnumCodeContexts()
    : m_refCount(1),
      m_pCodeContext(nullptr),
      m_cIndex(0)
{
}

CEnumCodeContexts::~CEnumCodeContexts()
{
    if (m_pCodeContext) 
    {
        m_pCodeContext->Release();
    }
}

HRESULT CEnumCodeContexts::Init(IDebugCodeContext *codeContext)
{
    if (codeContext)
    {
        codeContext->AddRef();
    }
    if (m_pCodeContext)
    {
        m_pCodeContext->Release();
    }
    m_pCodeContext = codeContext;
    return S_OK;
}

ULONG CEnumCodeContexts::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_refCount);
}

ULONG CEnumCodeContexts::Release(void)
{
    ULONG l = (ULONG)InterlockedDecrement(&m_refCount);
    if (l == 0)
        delete this;
    return l;
}

HRESULT CEnumCodeContexts::QueryInterface(REFIID iid, void ** ppv)
{
    CHECK_POINTER(ppv);
    if (InlineIsEqualGUID(iid, IID_IUnknown))
        *ppv = (IUnknown *)this;
    else if (InlineIsEqualGUID(iid, __uuidof(IEnumDebugCodeContexts)))
        *ppv = (IEnumDebugCodeContexts *)this;
    else
    {
        *ppv = nullptr;
        return HR(E_NOINTERFACE);
    }
    AddRef();
    return S_OK;
}

HRESULT CEnumCodeContexts::Next(
    /* [in]  */ ULONG celt,
    /* [out] */IDebugCodeContext **ppCodeContext,
    /* [out] */ ULONG *pceltFetched)
{
    if (pceltFetched)
    {
        *pceltFetched = 0;
    }
    if (celt == 0)
    {
        return S_OK;
    }
    if (!ppCodeContext)
    {
        return E_POINTER;
    }
    if (celt != 1 && !pceltFetched)
    {
        return E_POINTER;
    }
    if (!m_pCodeContext || m_cIndex != 0)
    {
        return S_FALSE;
    }
    *ppCodeContext = m_pCodeContext;
    m_pCodeContext->AddRef();
    m_cIndex++;
    if (pceltFetched)
    {
        *pceltFetched = 1;
    }
    if (celt == 1) 
    {
        return S_OK;
    } 
    else 
    {
        return S_FALSE;
    }
}

HRESULT CEnumCodeContexts::Skip(
    /* [in]  */ ULONG celt)
{
    if (celt == 0)
    {
        return S_OK;
    }
    if (!m_pCodeContext)
    {
        return S_FALSE;
    }
    if (m_cIndex != 0)
    {
        return S_FALSE;
    }
    m_cIndex++;
    if (celt == 1)
    {
        return S_OK;
    }
    else 
    {
        return S_FALSE;
    }
}

HRESULT CEnumCodeContexts::Reset(void)
{
    m_cIndex = 0;
    return S_OK;
}

HRESULT CEnumCodeContexts::Clone(
    /* [out] */ IEnumDebugCodeContexts **ppCodeContexts)
{
    CEnumCodeContexts *codeContexts;

    if (!ppCodeContexts)
    {
        return E_POINTER;
    }

    *ppCodeContexts = nullptr;

    codeContexts = HeapNewNoThrow(CEnumCodeContexts);
    if (!codeContexts)
    {
        return E_OUTOFMEMORY;
    }

    codeContexts->m_pCodeContext = m_pCodeContext;
    if (m_pCodeContext)
    {
        m_pCodeContext->AddRef();
    }
    codeContexts->m_cIndex = m_cIndex;

    *ppCodeContexts = codeContexts;
    return S_OK;
}


//***************************************************************************
// Execution context
//***************************************************************************
CDebugStackFrame::CDebugStackFrame(void)
    : m_refCount(0),
     m_scriptSite(nullptr),
     m_framePointers(nullptr),
     m_currentFrame(nullptr),
     m_frameIndex(-1)
{
    DLLAddRef();
}


CDebugStackFrame::~CDebugStackFrame(void)
{
    Close();
    DLLRelease();

    if (nullptr != m_framePointers)
    {
        HeapDelete(m_framePointers);
        m_framePointers = nullptr;
    }
}


HRESULT CDebugStackFrame::Close(void)
{
    return S_OK;
}

HRESULT CDebugStackFrame::Init(ScriptSite* scriptSite, int frameIndex)
{
    HRESULT hr = S_OK;
    if (DebugHelper::IsScriptSiteClosed(scriptSite, &hr))
    {
        return hr;
    }

    m_scriptSite = scriptSite;
    CComPtr<IDebugApplicationThread> pAppThread;
    hr = scriptSite->GetScriptEngine()->GetApplicationThread(&pAppThread);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pAppThread.QueryInterface(&m_pApplicationThread);
    if (FAILED(hr))
    {
        return hr;
    }

    IDebugApplication* debugApplication;
    hr = scriptSite->GetScriptEngine()->GetDebugApplicationCoreNoRef(&debugApplication);
    if (FAILED(hr))
    {
        return hr;
    }
    m_pDebugApplication = debugApplication;

    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        m_framePointers = scriptContext->GetDebugContext()->GetProbeContainer()->GetFramePointers();

#if DBG
        m_debugSessionNumber = scriptContext->GetThreadContext()->GetDebugManager()->GetDebugSessionNumber();
#endif
        OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::Init:  this=%p, debug session #%d\n"), 
            this, scriptContext->GetThreadContext()->GetDebugManager()->GetDebugSessionNumber());

        Js::DiagStack* framePointers = this->m_framePointers->GetStrongReference();
        m_currentFrame = framePointers->Peek(frameIndex);
        m_framePointers->ReleaseStrongReference();

        m_frameIndex = frameIndex;
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

ULONG CDebugStackFrame::AddRef(void)
{
    return (ulong)InterlockedIncrement(&m_refCount);
}

ULONG CDebugStackFrame::Release(void)
{
    ulong lu = (ulong)InterlockedDecrement(&m_refCount);
    if (lu == 0)
        delete this;
    return lu;
}

HRESULT CDebugStackFrame::QueryInterface(REFIID iid, void ** ppv)
{
    CHECK_POINTER(ppv);
    AssertMem(ppv);

    if (iid == IID_IUnknown)
        *ppv = (IUnknown *)(IDebugStackFrame *)this;
    else if (iid == __uuidof(IDebugStackFrame))
        *ppv = (IDebugStackFrame *)this;
    else if (iid == __uuidof(IDebugExpressionContext))
        *ppv = (IDebugExpressionContext *)this;
    else if (iid == IID_IDebugSetValueCallback)
        *ppv = (IDebugSetValueCallback *)this;
    else if (iid == IID_ISetNextStatement)
        *ppv = (ISetNextStatement *)this;
    else
    {
        *ppv = nullptr;
        return HR(E_NOINTERFACE);
    }

    AddRef();
    return NOERROR;
}

HRESULT CDebugStackFrame::GetCodeContext(IDebugCodeContext **ppCodeContext)
{
    return DebugApiWrapper([=] {
        if (!ppCodeContext)
        {
            return E_INVALIDARG;
        }
        
        HRESULT hr = S_OK;
        if (DebugHelper::IsScriptSiteClosed(m_scriptSite, &hr))
        {
            return hr;
        }

        VALIDATE_LEGIT_DEBUGSESSION();

        CCodeContext* pCodeContext = nullptr;
        const bool isLibraryCode = m_currentFrame->GetJavascriptFunction()->IsLibraryCode();
        Js::FunctionBody* pFunction = m_currentFrame->GetFunction();
        if (pFunction)
        {
            int iCurrentByteCodeOffset = m_currentFrame->GetByteCodeOffset();

            // If the setnext statement is called after on break, the current bytecode offset is adjust based on user's request.
            // We need to give the modified bytecode offset in this case.
            if (m_frameIndex == 0 && m_scriptSite->GetScriptSiteContext()->GetDebugContext()->GetProbeContainer()->IsSetNextStatementCalled())
            {
                iCurrentByteCodeOffset = m_scriptSite->GetScriptSiteContext()->GetDebugContext()->GetProbeContainer()->GetByteCodeOffset();
            }

            if (m_currentFrame->IsInterpreterFrame() && m_frameIndex != 0)
            {
                // For non-leaf interpreter frames back up 1 instruction so we see the caller
                // rather than the statement after the caller
                iCurrentByteCodeOffset--;
            }

            Js::FunctionBody::StatementMap* pStatementMap = pFunction->GetEnclosingStatementMapFromByteCode(iCurrentByteCodeOffset, false);

            ScriptDebugDocument *debugDocument = nullptr;
            if (pFunction->GetUtf8SourceInfo()->HasDebugDocument())
            {
                debugDocument = static_cast<ScriptDebugDocument*>(pFunction->GetUtf8SourceInfo()->GetDebugDocument());
            }
            // Note that debugDocument can be NULL if it failed to register with PDM in ScriptDebugDocument::Register
            // (e.g. because of OOM, etc) in which case click-navigate would be disabled but most of experience is recovered.
            
            if (pStatementMap)
            {
                pCodeContext = HeapNewNoThrow(CCodeContext, debugDocument, pStatementMap->sourceSpan.begin, pStatementMap->sourceSpan.end - pStatementMap->sourceSpan.begin, isLibraryCode);
            }
            else
            {
                // We didn't stop inside a recognized statement, default to start of function
                pCodeContext = HeapNewNoThrow(CCodeContext, debugDocument, pFunction->StartInDocument(), pFunction->LengthInChars(), isLibraryCode);
            }
        }
        else
        {
            // Runtime function (native library). No debugDocument.
            pCodeContext = HeapNewNoThrow(CCodeContext, /*debugDocument*/nullptr, 0, 0, isLibraryCode);
        }

        *ppCodeContext = pCodeContext;

        return *ppCodeContext == nullptr ? E_OUTOFMEMORY : S_OK;
    });
}


HRESULT CDebugStackFrame::GetDescriptionString(BOOL fLong, BSTR *pbstr)
{
    return DebugApiWrapper([=] {
        CHECK_POINTER(pbstr);
        AssertMem(pbstr);

        VALIDATE_LEGIT_DEBUGSESSION();

        PCWSTR pDisplayName = m_currentFrame->GetDisplayName();
        *pbstr = SysAllocString(pDisplayName);
        return S_OK;
    });
}


HRESULT CDebugStackFrame::GetLanguageString(BOOL fLong, BSTR *pbstr)
{
    CHECK_POINTER(pbstr);
    AssertMem(pbstr);
    *pbstr = SysAllocString(_u("JavaScript"));

    return S_OK;
}


HRESULT CDebugStackFrame::GetPhysicalStackRange(DWORD_PTR *pdwMin, DWORD_PTR *pdwLim)
{
    // This stack range will be used to identify which stack is frame among multiple stacks
    // is above then others.
    // Multiple stacks will be identified in multiple iframe course.

    AssertMem(pdwMin);
    AssertMem(pdwLim);

    *pdwMin = m_currentFrame->GetStackAddress();

    // TODO : need to get range
    *pdwLim = m_currentFrame->GetStackAddress() + 1;

    return S_OK;
}

// This is called from the debugger thread
HRESULT CDebugStackFrame::ParseLanguageText(LPCOLESTR pszSrc, UINT uRadix,
                                            LPCOLESTR pszDelimiter, DWORD dwFlags, IDebugExpression **ppDebugExpression)
{
    return DebugApiWrapper([=] {
        AssertPsz(pszSrc);
        AssertPszN(pszDelimiter);

        HRESULT hr;
        CComPtr<CDebugEval> eval;
        CDebugExpression *expression = nullptr;

        CHECK_POINTER(ppDebugExpression);
        AssertMem(ppDebugExpression);
        *ppDebugExpression = nullptr;

        hr = CDebugEval::Create(&eval.p, pszSrc, dwFlags, this, m_pApplicationThread);
        if (S_OK != hr)
        {
            goto Error;
        }

        hr = CDebugExpression::Create(&expression, eval.p, m_pDebugApplication);
        if (S_OK != hr)
        {
            goto Error;
        }

        *ppDebugExpression = static_cast<IDebugExpression*>(expression);

Error:
        return hr;
    });
}


HRESULT CDebugStackFrame::GetLanguageInfo(BSTR * pbstrLang, GUID * pguidLang)
{
    if (pbstrLang || pguidLang)
    {
        if (nullptr == m_scriptSite)
            return E_UNEXPECTED;
        ScriptEngine * pos = m_scriptSite->GetScriptEngine();
        if (nullptr == pos)
            return E_UNEXPECTED;
        return pos->GetLanguageInfo(pbstrLang, pguidLang);
    }
    return E_INVALIDARG;
}


HRESULT CDebugStackFrame::GetThread(IDebugApplicationThread **ppat)
{
    CHECK_POINTER(ppat);
    AssertMem(ppat);
    *ppat = m_pApplicationThread;
    return S_OK;
}


HRESULT CDebugStackFrame::GetDebugPropertyCore(IDebugProperty **ppDebugProperty)
{
    HRESULT hr = S_OK;

    // We should be on the correct thread now.
    AssertMem(ppDebugProperty);
    CComPtr<IDispatch> pDisp;

    if (DebugHelper::IsScriptSiteClosed(m_scriptSite, &hr))
    {
        return hr;
    }

    Js::ScriptContext* scriptContext = m_currentFrame->GetScriptContext();
    if (!scriptContext)
    {
        return  E_FAIL;
    }

    ReferencedArenaAdapter* pRefArena = scriptContext->GetThreadContext()->GetDebugManager()->GetDiagnosticArena();
    if (!pRefArena)
    {
        return E_OUTOFMEMORY;
    }

    VALIDATE_LEGIT_DEBUGSESSION();

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        Js::IDiagObjectModelDisplay* pOMDisplay = Anew(pRefArena->Arena(), Js::LocalsDisplay, m_currentFrame);
        AutoPtr<WeakArenaReference<Js::IDiagObjectModelDisplay>>  pWeakRef(HeapNew(WeakArenaReference<Js::IDiagObjectModelDisplay>, pRefArena, pOMDisplay));

        DWORD frameDbgPropAttribFlags = 0;
        if( m_currentFrame->IsInterpreterFrame() )
        {
            UINT16 frameFlags = m_currentFrame->AsInterpreterFrame()->GetFlags();
            frameDbgPropAttribFlags |= IfFlagSet(frameFlags, Js::InterpreterStackFrameFlags_WithinTryBlock) ? DBGPROP_ATTRIB_FRAME_INTRYBLOCK : 0;
            frameDbgPropAttribFlags |= IfFlagSet(frameFlags, Js::InterpreterStackFrameFlags_WithinCatchBlock) ? DBGPROP_ATTRIB_FRAME_INCATCHBLOCK : 0;
            frameDbgPropAttribFlags |= IfFlagSet(frameFlags, Js::InterpreterStackFrameFlags_WithinFinallyBlock) ? DBGPROP_ATTRIB_FRAME_INFINALLYBLOCK : 0;
        }

        *ppDebugProperty = HeapNew(DebugProperty, pWeakRef, m_pApplicationThread, m_scriptSite->GetScriptEngine(), nullptr, false/*isInReturnValueHierarchy*/, frameDbgPropAttribFlags);
        pWeakRef.Detach();
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT CDebugStackFrame::ThreadCallHandler(DWORD_PTR dw1, DWORD_PTR dw2,
                                            DWORD_PTR dw3)
{
    return DebugApiWrapper([=] {
        switch (dw1)
        {
        case xthread_GetDebugProperty:
            return GetDebugPropertyCore((IDebugProperty **)dw2);
        case xthread_CanSetNextStatement:
            return CanSetNextStatement((IDebugStackFrame *)dw2, (IDebugCodeContext *)dw3);
        case xthread_SetNextStatement:
            return SetNextStatement((IDebugStackFrame *)dw2, (IDebugCodeContext *)dw3);
        }
        return HR(E_INVALIDARG);
    });
}

STDMETHODIMP CDebugStackFrame::SetValue(VARIANT *pvarNode, DISPID dispid,
                                        ULONG clIndicies, LONG *prglIndicies, LPCOLESTR pszValue, UINT nRadix,
                                        BSTR *pbstrError)
{
    return DebugApiWrapper([=] {
        Js::DiagStackFrame* frame = nullptr;
        Js::ScriptContext* scriptContext = nullptr;
        Js::DiagStack* stack = nullptr;
        ReferencedArenaAdapter* pRefArena = nullptr;

        if (nullptr == pvarNode || nullptr == pszValue || pvarNode->vt != VT_BSTR)
        {
            return E_INVALIDARG;
        }

        if (pbstrError)
        {
            *pbstrError = nullptr;
        }

        HRESULT hr = S_OK;
        if (DebugHelper::IsScriptSiteClosed(m_scriptSite, &hr))
        {
            return hr;
        }

        VALIDATE_LEGIT_DEBUGSESSION();

        scriptContext = m_scriptSite->GetScriptSiteContext();
        pRefArena = scriptContext->GetThreadContext()->GetDebugManager()->GetDiagnosticArena();

        if (!pRefArena)
        {
            return E_FAIL;
        }

        stack = m_framePointers->GetStrongReference();

        if (!stack)
        {
            return E_FAIL;
        }

        frame = stack->Peek(m_frameIndex);

        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, true)
        {
            ENFORCE_ENTRYEXITRECORD_HASCALLER(scriptContext);

            Js::ResolvedObject resolveObject;
            frame->TryFetchValueAndAddress(pvarNode->bstrVal, ::SysStringLen(pvarNode->bstrVal), &resolveObject);

            hr = DebugProperty::BuildExprAndEval(pvarNode->bstrVal, nullptr, resolveObject.address, scriptContext, frame, pszValue);
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

        m_framePointers->ReleaseStrongReference();
        return hr;
    });
}

HRESULT CDebugStackFrame::CanDoSetNextStatement(
    __in IDebugStackFrame *stackFrame,
    __in IDebugCodeContext *codeContext,
    __in Js::FunctionBody *& _pFuncBody,
    __out int * pBytecodeAtNext
    )
{
    //
    // In order to understand the logic in this routine clearly lets assume, the setnext statement happens between A -> B. (A is current statement, B is the one user has asked to jump)
    OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: start: this=%p, m_currentFrame->GetFunction()=%p\n"), 
        this, m_currentFrame->GetFunction());

    if (stackFrame == nullptr || codeContext == nullptr)
    {
        return HR(E_POINTER);
    }

    Js::FunctionBody* pFuncBody = m_currentFrame->GetFunction();

    if (pFuncBody == nullptr)
    {
        return HR(E_UNEXPECTED);
    }

    if ((IDebugStackFrame *)this != stackFrame)
    {
        return HR(E_INVALIDARG);
    }

    CComPtr<IInternalCodeContext> pInternalCodeContext;

    if (FAILED(codeContext->QueryInterface(IID_IInternalCodeContext, (void **)&pInternalCodeContext)))
    {
        return HR(E_NOTIMPL);
    }

    AssertMem(pInternalCodeContext.p);

    HRESULT hr;
    CCodeContext *pCodeContext;
    if (FAILED(hr = pInternalCodeContext->GetCodeContextClass(&pCodeContext)))
    {
        return HR(E_FAIL);
    }

    long ibos = pCodeContext->GetStartPos();
    CScriptBody * pBody = pCodeContext->Pbody();

    if (pBody == nullptr)
    {
        OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: pCodeContext->Pbody() is NULL: this=%p\n"), this);
        return HR(E_FAIL);
    }

    _pFuncBody =  pBody->GetFunctionBodyAt(ibos);

    if (_pFuncBody != pFuncBody)
    {
        OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: func body mismatch: this=%p, _pFuncBody=%p, pFuncBody=%p\n"), this, _pFuncBody, pFuncBody);
        return HR(E_FAIL);
    }

    VALIDATE_LEGIT_DEBUGSESSION();
    OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: checking for statements: this=%p, pFuncBody=%p\n"), this, pFuncBody);

    // Get the matching statementSpan for the B.
    // If we are going forward, account for recorded branch offsets (i.e. cases when statement start in map is after actual start - example is ForIn)
    // between end of prev statement of B and B statement: if there is such offset recorded, use it as start of statement for B.
    // Backward - we don't have to because we need the logic to set next statement to 1st stmt inside ForIn when SetNet is from inside to begin of ForIn.
    int mapIndex;
    Js::FunctionBody::StatementMap *pMapB = _pFuncBody->GetMatchingStatementMapFromSource(ibos, &mapIndex);
    if (pMapB == nullptr || Js::FunctionBody::IsDummyGlobalRetStatement(&pMapB->sourceSpan))
    {
        return HR(E_FAIL);
    }

    Assert(mapIndex >= 0);

    Js::FunctionBody::StatementMapList* pStatementMaps = _pFuncBody->GetStatementMaps();
    Assert(pStatementMaps && pStatementMaps->Count() > 0);

    // 1 : for the nextstatement on the forward direction, -1 for the reverse.
    int currentOffset = m_currentFrame->GetByteCodeOffset();
    regex::Interval byteCodeSpanB(pMapB->byteCodeSpan.begin, pMapB->byteCodeSpan.end);
    int direction = (currentOffset < byteCodeSpanB.begin) ? 1 : -1;

    if (direction > 0)
    {
        // Take care of statement adjustment and fix byteCodeSpanB.begin.
        int prevBStatementEndByteCodeOffset = 0;
        if (mapIndex > 0)
        {
            int prevMapIndex = mapIndex;
            Js::FunctionBody::StatementMap* pPrevMap = Js::FunctionBody::GetPrevNonSubexpressionStatementMap(pStatementMaps, --prevMapIndex);
            if (pPrevMap)
            {
                prevBStatementEndByteCodeOffset = pPrevMap->byteCodeSpan.end;
            }
        }

        int actualBStartOffset;
        if (!_pFuncBody->GetScriptContext()->GetDebugContext()->GetProbeContainer()->GetNextUserStatementOffsetForSetNext(
            _pFuncBody, prevBStatementEndByteCodeOffset, &actualBStartOffset))
        {
            actualBStartOffset = pMapB->byteCodeSpan.begin;
        }
        Assert(actualBStartOffset <= pMapB->byteCodeSpan.begin);

        byteCodeSpanB.begin = actualBStartOffset;
    }

    if (byteCodeSpanB.Includes(currentOffset))
    {
        // Bytecode for A and B on the same statement.
        OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: byteCodeSpanB includes current offset: this=%p, current=0x%x, BStart=0x%x, BEnd=0x%x\n"), 
            this, currentOffset, byteCodeSpanB.begin, byteCodeSpanB.end);
        return HR(S_FALSE);
    }
    AssertMsg(pStatementMaps->Count() > 1, "We already identified that A (which must belong to a stmt) is outside range of MapB. How can there be only 1 stmt map?");

    //
    // Make sure we are not jumping to another catch/with block. the result is unknown. better not allow them.
    // This will be the same for the block scope, so they by default will not be allowed. We should revisit that once we enable that.

    // Check for block scope jump behavior.
    if (!CanDoBlockScopeSetNextStatement(pFuncBody, currentOffset, byteCodeSpanB.begin))
    {
        OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: can't do block scope jump: this=%p\n"), this);
        return E_FAIL;
    }

    // Check for non-block scope jumps.
    if (direction == 1 && pFuncBody->GetScopeObjectChain())
    {
        for (int i = 0; i < pFuncBody->GetScopeObjectChain()->pScopeChain->Count(); i++)
        {
            Js::DebuggerScope *debuggerScope = pFuncBody->GetScopeObjectChain()->pScopeChain->Item(i);

            // Jumping in is not allowed, but jumping out is fine.
            if (!debuggerScope->IsBlockScope() && !debuggerScope->IsOffsetInScope(currentOffset) 
                 && debuggerScope->IsOffsetInScope(byteCodeSpanB.begin))
            {
                OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: can't do non-block scope jump: this=%p\n"), this);
                return E_FAIL;
            }
        }
    }

    // Check for cross-interpreter frame jumps (enter/leave try-catch blocks).
    // Note: regular statements never span across try-catch boundary, thus it doesn't matter whether we pass begin or end of range.
    if (this->IsJmpCrossInterpreterStackFrame(currentOffset, byteCodeSpanB.begin, pFuncBody))
    {
        OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: can't do cross-interpreter stack frame jump: this=%p\n"), this);
        return E_FAIL;
    }

    if (pBytecodeAtNext != nullptr)
    {
        *pBytecodeAtNext = byteCodeSpanB.begin;
    }

    bool isInterpreterFrame = m_currentFrame->IsInterpreterFrame();
    Js::ByteCodeReader *pReader = nullptr;
    if (isInterpreterFrame)
    {
        pReader = m_currentFrame->AsInterpreterFrame()->GetReader();
    }
    else
    {
        pReader = HeapNewNoThrow(Js::ByteCodeReader);
        pReader->Create(pFuncBody);
    }   

    bool IsTmpRegCountIncreased = Js::ProbeContainer::IsTmpRegCountIncreased(_pFuncBody, pReader, currentOffset, byteCodeSpanB.begin, isInterpreterFrame);
    
    if (!isInterpreterFrame)
    {
        HeapDelete(pReader);
    }

    hr = IsTmpRegCountIncreased ? HR(E_FAIL) : HR(S_OK) ;
    OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::CanDoSetNextStatement: done: this=%p, from=0x%x, to=0x%x, hr=0x%x\n"),
        this, currentOffset, byteCodeSpanB.begin, hr);
    return hr;
}

STDMETHODIMP CDebugStackFrame::CanSetNextStatement(IDebugStackFrame *stackFrame,
                                                   IDebugCodeContext *codeContext)
{
    return DebugApiWrapper([=] {
        Assert(stackFrame != nullptr);
        Assert(codeContext != nullptr);

        if (S_OK != this->m_pApplicationThread->QueryIsCurrentThread())
        {
            return this->m_pApplicationThread->SynchronousCallIntoThread(
                (IDebugThreadCall *)this,
                xthread_CanSetNextStatement,
                (DWORD_PTR)stackFrame,
                (DWORD_PTR)codeContext);
        }

        Js::FunctionBody *pFunc = nullptr;
        return CanDoSetNextStatement(stackFrame, codeContext, pFunc, nullptr/*nextbytecode*/);
    });
}

STDMETHODIMP CDebugStackFrame::SetNextStatement(IDebugStackFrame *stackFrame,
                                                IDebugCodeContext *codeContext)
{
    return DebugApiWrapper([=] {
        Assert(stackFrame != nullptr);
        Assert(codeContext != nullptr);

        if (S_OK != this->m_pApplicationThread->QueryIsCurrentThread())
        {
            return this->m_pApplicationThread->SynchronousCallIntoThread(
                (IDebugThreadCall *)this,
                xthread_SetNextStatement,
                (DWORD_PTR)stackFrame,
                (DWORD_PTR)codeContext);
        }

        OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::SetNextStatement: start: this=%p\n"), this);
        HRESULT hr = S_OK;
        if (DebugHelper::IsScriptSiteClosed(m_scriptSite, &hr))
        {
            OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::SetNextStatement: script site is closed: this=%p\n"), this);
            return hr;
        }

        Js::FunctionBody * pFBody =  nullptr;
        int bytecodeAtNext;
        hr = CanDoSetNextStatement(stackFrame, codeContext, pFBody, &bytecodeAtNext);

        if (hr == S_OK)
        {
            Js::ScriptContext *scriptContext = m_scriptSite->GetScriptSiteContext();

            Assert(bytecodeAtNext >= 0 && (uint)bytecodeAtNext < pFBody->GetByteCode()->GetLength());
            scriptContext->GetDebugContext()->GetProbeContainer()->SetNextStatementAt(bytecodeAtNext);
            
        }

        OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::SetNextStatement: done: this=%p, ret=0x%x\n"), this, hr);
        return hr;
    });
}

HRESULT CDebugStackFrame::GetDebugProperty(IDebugProperty **ppDebugProperty)
{
    return DebugApiWrapper([=] {
        HRESULT hr = S_OK;
        
        CHECK_POINTER(ppDebugProperty);
        *ppDebugProperty = nullptr;      
        
        if (S_OK != this->m_pApplicationThread->QueryIsCurrentThread())
        {
            hr = this->m_pApplicationThread->SynchronousCallIntoThread(
                                        (IDebugThreadCall *)this,
                                        xthread_GetDebugProperty,
                                        (DWORD_PTR)ppDebugProperty,
                                        0);

            return hr;
        }

        hr = GetDebugPropertyCore(ppDebugProperty);

        return hr;
    });
}

// Checks whether or not a set next statement can be performed between block scopes.
bool CDebugStackFrame::CanDoBlockScopeSetNextStatement(Js::FunctionBody* pFuncBody, int startOffset, int endOffset)
{
    Assert(pFuncBody);

    OUTPUT_TRACE_DEBUGONLY(Js::DebuggerPhase, _u("CanDoBlockScopeSetNextStatement() - Checking if set next statment is allowed in blocks from offset 0x%x to offset 0x%x.\n"), startOffset, endOffset);
    if (!pFuncBody->GetScopeObjectChain())
    {
        OUTPUT_TRACE_DEBUGONLY(Js::DebuggerPhase, _u("CanDoBlockScopeSetNextStatement() - No scope chain so allowing set next to proceed.\n"));
        return true;
    }

    bool isJumpingForward = startOffset < endOffset;

    Js::DebuggerScope* startDebuggerScope = nullptr;
    Js::DebuggerScope* endDebuggerScope = nullptr;

    // Scopes found towards the end of the list will be the leaf scopes.
    pFuncBody->GetScopeObjectChain()->pScopeChain->Last([&](Js::DebuggerScope* debuggerScope)
    {
        return debuggerScope->IsBlockScope() && debuggerScope->IsOffsetInScope(startOffset);
    },
    startDebuggerScope);

    pFuncBody->GetScopeObjectChain()->pScopeChain->Last([&](Js::DebuggerScope* debuggerScope)
    {
        return debuggerScope->IsBlockScope() && debuggerScope->IsOffsetInScope(endOffset);
    },
    endDebuggerScope);

    // If one of the scopes is null, it represents the global scope with no let/const
    // declarations inside of it.
    if (startDebuggerScope == nullptr && endDebuggerScope == nullptr)
    {
        // Not jumping between block scopes.
        OUTPUT_TRACE_DEBUGONLY(Js::DebuggerPhase, _u("CanDoBlockScopeSetNextStatement() - Two block scopes to jump between not found so allowing set next to proceed.\n"));
        return true;
    }

    // Check if we're jumping in the same block scope.
    bool isValidJump = true;
    if (startDebuggerScope == endDebuggerScope)
    {
        OUTPUT_TRACE_DEBUGONLY(Js::DebuggerPhase, _u("CanDoBlockScopeSetNextStatement() - Attempting jump in the same scope block.\n"));
        isValidJump = CanJumpWithinCurrentBlock(startDebuggerScope, startOffset, endOffset);
    }
    else if (startDebuggerScope == nullptr || startDebuggerScope->IsAncestorOf(endDebuggerScope))
    {
        OUTPUT_TRACE_DEBUGONLY(Js::DebuggerPhase, _u("CanDoBlockScopeSetNextStatement() - Attempting jump from an outer scope to an inner scope.\n"));
        isValidJump = CanJumpIntoInnerBlock(startDebuggerScope, endDebuggerScope, startOffset, endOffset);
    }
    else if (endDebuggerScope == nullptr || endDebuggerScope->IsAncestorOf(startDebuggerScope))
    {
        OUTPUT_TRACE_DEBUGONLY(Js::DebuggerPhase, _u("CanDoBlockScopeSetNextStatement() - Attempting jump from an inner scope to an outer scope.\n"));

        // If we're jumping to an outer scope, backwards is always allowed.
        if (isJumpingForward)
        {
            // If jumping forward, the common ancestor is the end scope so validate that
            // we can jump to the desired location at that level.
            isValidJump = CanJumpWithinCurrentBlock(endDebuggerScope, startOffset, endOffset);
        }
    }
    else
    {
        OUTPUT_TRACE_DEBUGONLY(Js::DebuggerPhase, _u("CanDoBlockScopeSetNextStatement() - Attempting jump from a sibling/grandchild block to another.\n"));

        // Performing a sibling jump across a common ancestor.
        Js::DebuggerScope* commonAncestorDebuggerScope = startDebuggerScope->FindCommonAncestor(endDebuggerScope);
        isValidJump = CanJumpIntoInnerBlock(commonAncestorDebuggerScope, endDebuggerScope, startOffset, endOffset);
    }

    OUTPUT_TRACE_DEBUGONLY(Js::DebuggerPhase, _u("CanDoBlockScopeSetNextStatement() - Block jump allowed? '%s'.\n"), isValidJump ? _u("Yes") : _u("No"));
    return isValidJump;
}

// Determines if set next statment can be performed within a range that is part of the same scope block.
bool CDebugStackFrame::CanJumpWithinCurrentBlock(Js::DebuggerScope* debuggerScope, int startOffset, int endOffset)
{
    if (debuggerScope == nullptr)
    {
        // If the scope is null, it represents the global scope with no let/const
        // variables declared inside of it so the jump is ok.
        return true;
    }

    Assert(debuggerScope->IsBlockScope());
    Assert(debuggerScope->IsOffsetInScope(startOffset));
    Assert(debuggerScope->IsOffsetInScope(endOffset));

    if (startOffset > endOffset)
    {
        // Can always jump backwards.
        return true;
    }

    if (!debuggerScope->HasProperties())
    {
        return true;
    }

    bool isValidJump = !debuggerScope->scopeProperties->Any([&](Js::DebuggerScopeProperty& propertyItem)
    {
        // Jumping from a non-dead zone to a dead zone is ok, the other way is not.
        return propertyItem.IsInDeadZone(startOffset) && !propertyItem.IsInDeadZone(endOffset);
    });

    return isValidJump;
}

// Determines if set next statement can be performed from an outer to inner block jump.
bool CDebugStackFrame::CanJumpIntoInnerBlock(
    Js::DebuggerScope* outerBlock,
    Js::DebuggerScope* innerBlock,
    int startOffset,
    int endOffset)
{
    Assert(innerBlock);
    Assert(outerBlock == nullptr || outerBlock->IsAncestorOf(innerBlock));
    Assert(startOffset != endOffset);

    bool isJumpingForward = startOffset < endOffset;

    // If we're jumping into an inner scope, ensure that
    // we're only jumping into the dead zone of the destination scope.
    bool isValidJump = !innerBlock->HasProperties() || innerBlock->AreAllPropertiesInDeadZone(endOffset);

    if (isValidJump && isJumpingForward)
    {
        // Also ensure that we're not jumping over any let/const declarations
        // at intermediate scope levels (jumping backwards over them is ok). 
        Js::DebuggerScope* currentScope = innerBlock;
        while (isValidJump && currentScope != outerBlock)
        {
            isValidJump = !currentScope->HasProperties() || currentScope->AreAllPropertiesInDeadZone(endOffset);

            if (isValidJump)
            {
                // Make sure the whole chain is block scopes.
                isValidJump = currentScope->IsBlockScope();
            }

            currentScope = currentScope->GetParentScope();
        }

        // For the start scope level, it's ok to jump forward but make sure we're not jumping
        // over any let/const declarations there as well.
        if (isValidJump)
        {
            isValidJump = CanJumpWithinCurrentBlock(outerBlock, startOffset, endOffset);
        }
    }

    return isValidJump;
}

// JMP due to set next statement from A to B.
// Check whether the JMP is to different interpreter frame than current (such as inside/outside try block).
// Algorithm:
//   Linear search from both ends. We assume that it should not be too many try/catch/finally statements in a function,
//   so that number of records is small. Otherwise binary search would be more appropriate.
// Some details:
//   When we process try and catch in interpreted  mode we set up a new interpreter frame for their body/block.
//   In the end of the block there is OpCode::Leave which is like a RET to parent interpreter frame.
//   We can't set next statement if A and B are in different interpreter frames.
bool CDebugStackFrame::IsJmpCrossInterpreterStackFrame(uint byteCodeOffsetA, uint byteCodeOffsetB, Js::FunctionBody* funcBody)
{
    Assert(funcBody);
    Assert(byteCodeOffsetA != byteCodeOffsetB);

    Js::FunctionBody::CrossFrameEntryExitRecordList* records = funcBody->GetCrossFrameEntryExitRecords();
    if (!records)
    {
        return false;   // No try-catch block records at all.
    }

    // For now assume that B > A, i.e. direction is forward.
    int direction = byteCodeOffsetA <= byteCodeOffsetB ? 1 : -1;
    uint topBoundary = byteCodeOffsetA;
    uint bottomBoundary = byteCodeOffsetB;
    if (direction < 0)
    {
        uint temp = topBoundary;
        topBoundary = bottomBoundary;
        bottomBoundary = temp;
    }
    Assert(topBoundary <= bottomBoundary);

    // Note that the list is sorted by byteCodeOffset in increasing order.
    // Find 1st enter/leave record after Top.
    int i;
    for (i = 0; i < records->Count() && records->Item(i).GetByteCodeOffset() <= topBoundary; ++i);
    if (i >= records->Count())
    {
        return false;    // No records after Top.
    }
    int topRecordIndex = i;

    // Find 1st enter/leave record before Bottom.
    for (i = records->Count() - 1; i >= 0 && records->Item(i).GetByteCodeOffset() >= bottomBoundary; --i);
    if (i < 0)
    {
        return false;    // No records before Bottom.
    }
    int bottomRecordIndex = i;

    if (topRecordIndex > bottomRecordIndex)
    {
        return false;    // No records between Top and Bottom.
    }

    // Now iterate each record in between A and B and and track level of nestness.
    int nestLevel = 0;
    for (i = direction > 0 ? topRecordIndex : bottomRecordIndex; 
        direction > 0 && i <= bottomRecordIndex || direction < 0 && i >= topRecordIndex; 
        i += direction)
    {
        Js::FunctionBody::CrossFrameEntryExitRecord record = records->Item(i);
        
        // When going down, enter is increment, leave is decrement.
        // When going up, it's vice versa.
        int increment = record.GetIsEnterBlock() ? direction : -direction;
        nestLevel += increment;

        if (nestLevel < 0)
        {
            // Can't go out of try or catch block using same interpreter frame, as it would have to be different one.
            break;
        }
    }

    return nestLevel != 0;  // If we don't end up at the same level, it's a cross-interpreter frame jump!
}

HRESULT CDebugStackFrame::EvaluateImmediate(LPCOLESTR pszSrc, DWORD dwFlags,
                                            IDebugProperty **ppdp)
{
    //TODO: implement dwFlags such as NOSideEffects and disallow breakpoints
    return DebugApiWrapper([&] {
        AssertPsz(pszSrc);
        AssertMemN(ppdp);

        OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::EvaluateImmediate: start: this=%p, dwFlags=0x%x, pszSrc='%s'\n"),
            this, dwFlags, pszSrc != nullptr ? pszSrc : _u("NULL"));

        if (!ppdp)
        {
            return E_INVALIDARG;
        }
        *ppdp = nullptr;

        HRESULT hr = S_OK;
        if (DebugHelper::IsScriptSiteClosed(m_scriptSite, &hr))
        {
            OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::EvaluateImmediate: script site is closed: this=%p\n"));
            return hr;
        }

        Js::ScriptContext* scriptContext = m_scriptSite->GetScriptSiteContext();

        if (!scriptContext->GetThreadContext()->GetDebugManager()->IsAtDispatchHalt())
        {
            // EvaluateImmediate should only be called when we are at break
            return E_UNEXPECTED;
        }

        Js::DiagStack* stack = m_framePointers->GetStrongReference();

        if (stack == nullptr)
        {
            AssertMsg(m_debugSessionNumber != scriptContext->GetThreadContext()->GetDebugManager()->GetDebugSessionNumber(),
                "Stack frame not found for valid debug session");
            return E_UNEXPECTED;
        }

        AssertMsg(m_debugSessionNumber == scriptContext->GetThreadContext()->GetDebugManager()->GetDebugSessionNumber(), "EvaluateImmediate on outdated stack frame");

        Js::DiagStackFrame* frame = stack->Peek(m_frameIndex);

        Js::ResolvedObject resolvedObject;
        resolvedObject.scriptContext = scriptContext;

        // Fast path for some expression which are part of the locals.
        charcount_t len = Js::JavascriptString::GetBufferLength(pszSrc);

        Js::JavascriptExceptionObject *exceptionObject = nullptr;
        // scope
        {
            BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_AND_GET_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, true)
            {
                ENFORCE_ENTRYEXITRECORD_HASCALLER(scriptContext);
                frame->EvaluateImmediate(pszSrc, len, dwFlags & DEBUG_TEXT_ISNONUSERCODE, &resolvedObject);
            }
            END_JS_RUNTIME_CALL_AND_TRANSLATE_AND_GET_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr, scriptContext, exceptionObject);
        }

        m_framePointers->ReleaseStrongReference();

        if (resolvedObject.obj == nullptr)
        {
            if (exceptionObject != nullptr)
            {
                // We need to check again because user code above might have caused the script engine to be closed
                if (DebugHelper::IsScriptSiteClosed(m_scriptSite, &hr))
                {
                    return hr;
                }

                hr = S_OK;
                ActiveScriptError * pase = nullptr;

                // We don't need to support the error object in debugger scenario. 
                if (SUCCEEDED(ActiveScriptError::CreateRuntimeError(exceptionObject, &hr, nullptr, nullptr, &pase)))
                {
                    ScriptEngine* scriptEngine = m_scriptSite->GetScriptEngine();
                    AssertMem(scriptEngine);

                    hr = scriptEngine->DbgCreateBrowserFromError((IActiveScriptError *) IACTIVESCRIPTERROR64 pase, pszSrc, ppdp);
                    pase->Release();
                    return hr;
                }
            }

            // The fallback strategy is to use undefined if something goes wrong.
            resolvedObject.obj = scriptContext->GetLibrary()->GetUndefined();
        }

        // We need to check again because user code above might have caused the script engine to be closed
        if (DebugHelper::IsScriptSiteClosed(m_scriptSite, &hr))
        {
            OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::EvaluateImmediate: script site is closed (2): this=%p\n"));
            return hr;
        }

        // Package the result as a property
        ReferencedArenaAdapter* pRefArena = scriptContext->GetThreadContext()->GetDebugManager()->GetDiagnosticArena();
        if (!pRefArena)
        {
            return E_OUTOFMEMORY;
        }

        resolvedObject.name = AnewNoThrowArray(pRefArena->Arena(), WCHAR, len+1);
        if (resolvedObject.name == nullptr)
        {
            return E_OUTOFMEMORY;
        }
        wcscpy_s((WCHAR*)resolvedObject.name, len+1, pszSrc);

        resolvedObject.typeId = Js::JavascriptOperators::GetTypeId(resolvedObject.obj);

        if (resolvedObject.typeId != Js::TypeIds_HostDispatch)
        {
            hr = S_OK;
            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                AutoPtr<WeakArenaReference<Js::IDiagObjectModelDisplay>> pWeakRef = resolvedObject.GetObjectDisplay();    
                DebugProperty* pNewProp = HeapNew(DebugProperty, pWeakRef, m_pApplicationThread, m_scriptSite->GetScriptEngine(), nullptr, false/*isInReturnValueHierarchy*/);
                pWeakRef.Detach();
                *ppdp = pNewProp;   
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr);
        }
        else
        {
            HostDispatch* pHostDisp = (HostDispatch*)resolvedObject.obj;
            IDispatch* pDisp = pHostDisp->GetDispatch();
            if (nullptr == pDisp)
            {
                return E_FAIL;
            }
            CComVariant variant = pDisp;
            pDisp->Release();
            ScriptEngine* scriptEngine = m_scriptSite->GetScriptEngine();

            return scriptEngine->DbgCreateBrowserFromProperty(&variant, this, resolvedObject.name, ppdp);
        }

        OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugStackFrame::EvaluateImmediate: done: this=%p, result=%x, hr=%x\n"),
            this, dwFlags, resolvedObject.obj, hr);
        return hr;
    });
}


HRESULT CDebugStackFrame::GetNextFrame(CDebugStackFrame **ppStackFrame)
{
    AssertMem(ppStackFrame);
    *ppStackFrame = nullptr;
    return S_FALSE;
}


/***************************************************************************
CDebugEval
***************************************************************************/
CDebugEval::CDebugEval(void)
    : m_refCount(1),
      m_dwFlags(0),
      m_bstrSrc(nullptr),
      m_stackFrame(nullptr),
      m_applicationThread(nullptr),
      m_isAborted(false)
{
}


CDebugEval::~CDebugEval(void)
{
    if (nullptr != m_bstrSrc)
    {
        SysFreeString(m_bstrSrc);
        m_bstrSrc = nullptr;
    }
    if (nullptr != m_stackFrame)
    {
        m_stackFrame->Release();
        m_stackFrame = nullptr;
    }
    if (nullptr != m_applicationThread)
    {
        m_applicationThread->Release();
        m_applicationThread = nullptr;
    }
}


HRESULT CDebugEval::Create(CDebugEval **ppDebugEval, LPCOLESTR pszSource, DWORD dwFlags,
                           CDebugStackFrame *stackFrame, IDebugApplicationThread *applicationThread)
{
    AssertMem(ppDebugEval);
    AssertPsz(pszSource);
    AssertMem(stackFrame);
    AssertMem(applicationThread);

    OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugEval::Create: start: stackFrame=%p, dwFlags=0x%x, pszSrc='%s'\n"),
        stackFrame, dwFlags, pszSource != nullptr ? pszSource : _u("NULL"));

    CDebugEval *pdev;

    *ppDebugEval = nullptr;
    if (nullptr == (pdev = HeapNewNoThrow(CDebugEval)))
        return HR(E_OUTOFMEMORY);

    pdev->m_stackFrame = stackFrame;
    pdev->m_stackFrame->AddRef();
    pdev->m_applicationThread = applicationThread;
    pdev->m_applicationThread->AddRef();

    if (nullptr == (pdev->m_bstrSrc = SysAllocString(pszSource)))
    {
        pdev->Release();
        return HR(E_OUTOFMEMORY);
    }
    pdev->m_dwFlags = dwFlags;

    *ppDebugEval = pdev;

    OUTPUT_TRACE(Js::DebuggerPhase, _u("CDebugEval::Create: done: stackFrame=%p, *ppDebugEval=%p\n"), stackFrame, *ppDebugEval);

    return NOERROR;
}


HRESULT CDebugEval::QueryInterface(REFIID iid, void ** ppv)
{
    CHECK_POINTER(ppv);
    AssertMem(ppv);

    if (iid == IID_IUnknown)
        *ppv = (IUnknown *)this;
    else if (iid == __uuidof(IDebugSyncOperation))
        *ppv = (IDebugSyncOperation *)this;
    else
    {
        *ppv = nullptr;
        return HR(E_NOINTERFACE);
    }
    AddRef();
    return NOERROR;
}


HRESULT CDebugEval::GetTargetThread(IDebugApplicationThread **ppApplicationThread)
{
    AssertMem(m_applicationThread);

    CHECK_POINTER(ppApplicationThread);

    *ppApplicationThread = m_applicationThread;
    m_applicationThread->AddRef();
    return NOERROR;
}

HRESULT CDebugEval::Execute(IUnknown **ppunkRes)
{
    return DebugApiWrapper([=] {
        AssertMem(m_stackFrame);
        AssertMem(m_applicationThread);
        AssertMemN(ppunkRes);

        if (m_isAborted)
        {
            return E_FAIL;
        }

        HRESULT hr;
        IDebugProperty *pdp;

        if (nullptr != ppunkRes)
            *ppunkRes = nullptr;

        // store the result - pdp may be set even if the call fails.
        hr = m_stackFrame->EvaluateImmediate(m_bstrSrc, m_dwFlags, &pdp);

        AssertMemN(pdp);
        if (nullptr != pdp)
        {
            if (nullptr != ppunkRes)
                hr = pdp->QueryInterface(IID_IUnknown, (void **)ppunkRes);
            pdp->Release();
        }
        return hr;
    });
}

HRESULT CDebugEval::InProgressAbort(void)
{
    m_isAborted = true;
    return S_OK;
}


/***************************************************************************
CDebugExpression
***************************************************************************/
CDebugExpression::CDebugExpression(CDebugEval *debugEval)
    : m_refCount(1),
      m_debugEval(debugEval),
      m_asyncOperation(nullptr),
      m_exprCallback(nullptr)
{
    DLLAddRef();
    AssertMem(m_debugEval);
    m_debugEval->AddRef();
}


CDebugExpression::~CDebugExpression(void)
{
    AssertMem(m_debugEval);
    m_debugEval->Release();
    if (nullptr != m_asyncOperation)
    {
        m_asyncOperation->Release();
        m_asyncOperation = nullptr;
    }
    Assert(m_exprCallback == nullptr);
    DLLRelease();    
}


HRESULT CDebugExpression::Create(CDebugExpression **ppDebugExpression,
                                 CDebugEval *debugEval,
                                 IDebugApplication *debugApplication)
{
    AssertMem(ppDebugExpression);
    AssertMem(debugEval);
    AssertMem(debugApplication);

    CDebugExpression *pdexp;
    HRESULT hr;

    *ppDebugExpression = nullptr;

    if (nullptr == (pdexp = HeapNewNoThrow(CDebugExpression, debugEval)))
        return HR(E_OUTOFMEMORY);
    if (FAILED(hr = debugApplication->CreateAsyncDebugOperation((IDebugSyncOperation *)debugEval, &pdexp->m_asyncOperation)))
    {
        pdexp->m_asyncOperation = nullptr;
        pdexp->Release();
        return hr;
    }

    *ppDebugExpression = pdexp;
    return NOERROR;
}


HRESULT CDebugExpression::QueryInterface(REFIID iid, void ** ppv)
{
    CHECK_POINTER(ppv);
    AssertMem(ppv);

    if (iid == IID_IUnknown)
        *ppv = (IUnknown *)(IDebugExpression *)this;
    else if (iid == __uuidof(IDebugExpression))
        *ppv = (IDebugExpression *)this;
    else if (iid == __uuidof(IDebugAsyncOperationCallBack))
        *ppv = (IDebugAsyncOperationCallBack *)this;
    else
    {
        *ppv = nullptr;
        return HR(E_NOINTERFACE);
    }
    AddRef();
    return NOERROR;
}


HRESULT CDebugExpression::Start(IDebugExpressionCallBack *expressionCallBack)
{
    AssertMemN(expressionCallBack);

    HRESULT hr;

    if (nullptr != m_exprCallback)
        return HR(E_FAIL);

    if (nullptr != expressionCallBack)
    {
        expressionCallBack->AddRef();
        expressionCallBack = (IDebugExpressionCallBack *)INTERLOCKED_EXCHANGE(&m_exprCallback, expressionCallBack);
        if (nullptr != expressionCallBack)
        {
            AssertMsg(FALSE, "why is this IDebugExpression being used in multiple threads?");
            expressionCallBack->Release();
        }
    }

    // Clear the abort flag so code will execute. Caller can Start/Abort multiple times, so we always need to reset it here.
    m_debugEval->SetAborted(false);
    hr = m_asyncOperation->Start((IDebugAsyncOperationCallBack *)this);
    if (FAILED(hr))
    {
        expressionCallBack = (IDebugExpressionCallBack *)INTERLOCKED_EXCHANGE(&m_exprCallback, nullptr);
        if (nullptr != expressionCallBack)
            expressionCallBack->Release();
    }

    return hr;
}


HRESULT CDebugExpression::onComplete(void)
{
    IDebugExpressionCallBack *pdecb;

    pdecb = (IDebugExpressionCallBack *)INTERLOCKED_EXCHANGE(&m_exprCallback, nullptr);
    if (nullptr != pdecb)
    {
        pdecb->onComplete();
        pdecb->Release();
    }

    return NOERROR;
}


HRESULT CDebugExpression::Abort(void)
{
    return m_asyncOperation->Abort();
}


HRESULT CDebugExpression::QueryIsComplete(void)
{
    return m_asyncOperation->QueryIsComplete();
}

HRESULT CDebugExpression::GetResultAsString(HRESULT *resultHr, BSTR *resultString)
{
    AssertMemN(resultHr);
    AssertMemN(resultString);

    if (nullptr == resultHr && nullptr == resultString)
        return NOERROR;
    if (nullptr != resultString)
        *resultString = nullptr;

    HRESULT hr, hrRes;
    IUnknown *punkRes;
    if (FAILED(hr = m_asyncOperation->GetResult(&hrRes, &punkRes)))
        return hr;
    if (nullptr != resultHr)
        *resultHr = hrRes;
    if (nullptr != punkRes)
    {
        if (nullptr != resultString)
        {
            IDebugProperty *pdp;
            hr = punkRes->QueryInterface(__uuidof(IDebugProperty), (void **)&pdp);
            punkRes->Release();
            if (FAILED(hr))
                return hr;

            DebugPropertyInfo dpi;
            if (SUCCEEDED(hr = pdp->GetPropertyInfo(DBGPROP_INFO_VALUE, 10, &dpi)))
                *resultString = dpi.m_bstrValue;
            pdp->Release();
        }
        else
            punkRes->Release();
    }
    return NOERROR;
}


HRESULT CDebugExpression::GetResultAsDebugProperty(HRESULT *phrRes,
                                                   IDebugProperty **ppDebugProperty)
{
    AssertMemN(phrRes);
    AssertMemN(ppDebugProperty);

    if (nullptr == phrRes && nullptr == ppDebugProperty)
        return NOERROR;
    if (nullptr != ppDebugProperty)
        *ppDebugProperty = nullptr  ;

    HRESULT hr, hrRes;
    IUnknown *punkRes;
    if (FAILED(hr = m_asyncOperation->GetResult(&hrRes, &punkRes)))
        return hr;
    if (nullptr != phrRes)
        *phrRes = hrRes;
    if (nullptr != punkRes)
    {
        if (nullptr != ppDebugProperty)
            punkRes->QueryInterface(__uuidof(IDebugProperty), (void **)ppDebugProperty);
        punkRes->Release();
    }
    return NOERROR;
}


/***************************************************************************
CEnumDebugStackFrames
***************************************************************************/
CEnumDebugStackFrames::CEnumDebugStackFrames(ScriptSite* _activeScriptSite)
    : m_refCount(1),
     m_dwThread(GetCurrentThreadId()),
     m_currentFrameIndex(0),
     m_fDone(false),
     m_fError(false),
     m_stackFramePrev(nullptr),
     m_scriptSite(_activeScriptSite),
     m_framePointers(nullptr)
{
    if (!m_scriptSite->IsClosed())
    {
        // Clean up all debug stack frame.
        m_scriptSite->GetScriptEngine()->CleanupHalt();
    }
}

HRESULT
CEnumDebugStackFrames::Init()
{
    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if (!m_scriptSite->IsClosed())
        {
            m_framePointers = m_scriptSite->GetScriptSiteContext()->GetDebugContext()->GetProbeContainer()->GetFramePointers();
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

CEnumDebugStackFrames::~CEnumDebugStackFrames(void)
{
    if (nullptr != m_stackFramePrev)
    {
        m_stackFramePrev->Release();
        m_stackFramePrev = nullptr;
    }
    if (nullptr != m_framePointers)
    {
        HeapDelete(m_framePointers);
        m_framePointers = nullptr;
    }
}


HRESULT CEnumDebugStackFrames::QueryInterface(REFIID iid, void ** ppv)
{
    CHECK_POINTER(ppv);
    AssertMem(ppv);

    if (iid == IID_IUnknown)
        *ppv = (IUnknown *)this;
    else if (iid == __uuidof(IEnumDebugStackFrames))
        *ppv = (IEnumDebugStackFrames *)this;
    else if (iid == __uuidof(IEnumDebugStackFrames64))
        *ppv = (IEnumDebugStackFrames64 *)this;
    else
    {
        *ppv = nullptr;
        return HR(E_NOINTERFACE);
    }
    AddRef();
    return NOERROR;
}

template <typename Descriptor>
HRESULT CEnumDebugStackFrames::NextImpl(ulong celt, Descriptor *frameDescriptors, ulong *pceltFetched)
{
    using TDWORD = decltype(frameDescriptors[0].dwMin);

    HRESULT hr;
    CDebugStackFrame *stackFrame;
    Descriptor *frameDescriptor;
    DWORD_PTR dwMin, dwLim;

    if (nullptr != pceltFetched)
        *pceltFetched = 0;

    if (0 == celt)
        return HR(S_OK);

    if (GetCurrentThreadId() != m_dwThread)
    {
        AssertMsg(FALSE, "wrong thread");
        return HR(E_UNEXPECTED);
    }

    CHECK_POINTER(frameDescriptors);
    AssertArrMem(frameDescriptors, celt);

    if (1 != celt && nullptr == pceltFetched)
        return HR(E_POINTER);

    // get the frames
    hr = this->NextCommon();
    if (S_OK != hr)
        return hr;

    // store the current frame
    AssertMem(m_stackFramePrev);
    frameDescriptors[0].pdsf = m_stackFramePrev;
    m_stackFramePrev->AddRef();
    m_stackFramePrev->GetPhysicalStackRange(&dwMin, &dwLim);
    frameDescriptors[0].dwMin = (TDWORD)dwMin;
    frameDescriptors[0].dwLim = (TDWORD)dwLim;
    frameDescriptors[0].punkFinal = nullptr;
    celt--;

    // fill in the rest of the output array
    for (frameDescriptor = frameDescriptors + 1; celt > 0; celt--)
    {
        AssertMem(m_stackFramePrev);
        hr = m_stackFramePrev->GetNextFrame(&stackFrame);
        m_stackFramePrev->Release();
        m_stackFramePrev = stackFrame;

        if (FAILED(hr))
        {
            Assert(nullptr == m_stackFramePrev);
            m_fError = true;
            break;
        }
        if (hr != NOERROR)
        {
            Assert(nullptr == m_stackFramePrev);
            m_fDone = true;
            break;
        }

        frameDescriptor->pdsf = m_stackFramePrev;
        m_stackFramePrev->AddRef();
        m_stackFramePrev->GetPhysicalStackRange(&dwMin, &dwLim);
        frameDescriptor->dwMin = (TDWORD)dwMin;
        frameDescriptor->dwLim = (TDWORD)dwLim;
        frameDescriptor->punkFinal = nullptr;
        frameDescriptor++;
    }

    if (nullptr != pceltFetched)
    {
        Assert(frameDescriptor - frameDescriptors >= 0);
        Assert(frameDescriptor - frameDescriptors <= ULONG_MAX);
        *pceltFetched = (ulong)(frameDescriptor - frameDescriptors);
    }

    return HR(S_OK);
}

HRESULT CEnumDebugStackFrames::Next(ulong celt,
                                    DebugStackFrameDescriptor *frameDescriptors, ulong *pceltFetched)
{
    return DebugApiWrapper ( [&] {
        return this->NextImpl(celt, frameDescriptors, pceltFetched);
    });
}


HRESULT CEnumDebugStackFrames::Next64(ulong celt,
                                      DebugStackFrameDescriptor64 *frameDescriptors, ulong *pceltFetched)
{
    return DebugApiWrapper ( [&] {
        return this->NextImpl(celt, frameDescriptors, pceltFetched);
    });
}


HRESULT CEnumDebugStackFrames::NextCommon()
{
    if (!m_framePointers)
    {
        // If multiple script contexts exist on the thread we may be called on one that
        // we did not park.  return E_FAIL in that case.
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    if (DebugHelper::IsScriptSiteClosed(m_scriptSite, &hr))
    {
        return hr;
    }

    Js::DiagStack* stack = m_framePointers->GetStrongReference();
    if (!stack)
    {
        return E_FAIL;
    }

    ulong frameCount = (ulong) stack->Count();

    m_framePointers->ReleaseStrongReference();

    
    JsUtil::List<CDebugStackFrame *, HeapAllocator> * stackFrames = nullptr;
    hr = m_scriptSite->GetScriptEngine()->GetDebugStackFrame(&stackFrames);
    if (FAILED(hr))
    {
        return E_FAIL;
    }

    if (m_currentFrameIndex < frameCount)
    {
        CDebugStackFrame *stackFrame = nullptr;

        if (m_currentFrameIndex < (ulong)stackFrames->Count())
        {
            stackFrame = stackFrames->Item(m_currentFrameIndex);
            if (!stackFrame)
            {
                return E_UNEXPECTED;
            }
        }
        else
        {
            stackFrame = new CDebugStackFrame();

            if (!stackFrame)
            {
                return E_OUTOFMEMORY;
            }
            stackFrame->AddRef();

            HRESULT hrT = stackFrame->Init(m_scriptSite, m_currentFrameIndex);
            if (!SUCCEEDED(hrT))
            {
                stackFrame->Release();
                return hrT;
            }

            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                stackFrames->Add(stackFrame);
            }
            END_TRANSLATE_OOM_TO_HRESULT(hrT);
            if (FAILED(hrT))
            {
                stackFrame->Release();
                return hrT;
            }
        }

        if (m_stackFramePrev)
        {
            m_stackFramePrev->Release();
        }

        m_stackFramePrev = stackFrame;
        (m_stackFramePrev)->AddRef();
        m_currentFrameIndex++;
        return NOERROR;
    }
    else
    {
        return S_FALSE;
    }
}


HRESULT CEnumDebugStackFrames::Skip(ulong celt)
{
    Assert((long)celt >= 0);

    if (GetCurrentThreadId() != m_dwThread)
    {
        AssertMsg(FALSE, "wrong thread");
        return HR(E_UNEXPECTED);
    }

    m_currentFrameIndex += celt;
    return NOERROR;
}


HRESULT CEnumDebugStackFrames::Reset(void)
{
    if (GetCurrentThreadId() != m_dwThread)
    {
        AssertMsg(FALSE, "wrong thread");
        return HR(E_UNEXPECTED);
    }

    m_fDone = false;
    m_fError = false;
    m_currentFrameIndex = 0;
    if (nullptr != m_stackFramePrev)
    {
        m_stackFramePrev->Release();
        m_stackFramePrev = nullptr;
    }

    return NOERROR;
}


HRESULT CEnumDebugStackFrames::Clone(IEnumDebugStackFrames **ppStackFrames)
{
    CEnumDebugStackFrames *stackFramesEnumerator;

    CHECK_POINTER(ppStackFrames);
    AssertMem(ppStackFrames);
    *ppStackFrames = nullptr;

    if (GetCurrentThreadId() != m_dwThread)
    {
        AssertMsg(FALSE, "wrong thread");
        return E_UNEXPECTED;
    }

    stackFramesEnumerator = new CEnumDebugStackFrames(m_scriptSite);
    if (nullptr == stackFramesEnumerator)
    {
        return E_OUTOFMEMORY;
    }

    if (FAILED(stackFramesEnumerator->Init()))
    {
        stackFramesEnumerator->Release();
        return E_OUTOFMEMORY;
    }

    stackFramesEnumerator->m_fDone = m_fDone;
    stackFramesEnumerator->m_fError = m_fError;
    stackFramesEnumerator->m_currentFrameIndex = m_currentFrameIndex;
    if (nullptr != (stackFramesEnumerator->m_stackFramePrev = m_stackFramePrev))
        stackFramesEnumerator->m_stackFramePrev->AddRef();

    *ppStackFrames = stackFramesEnumerator;

    return NOERROR;
}

