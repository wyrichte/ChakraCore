/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
// ScpText.cpp : Implementation of CProcDMApp and DLL registration.

#include "EnginePch.h"
#pragma hdrstop

/////////////////////////////////////////////////////////////////////////////
//

CScriptSourceDocumentText::CScriptSourceDocumentText(void)
{
    m_scriptEngine = NULL;
    m_scriptBody = NULL;

    m_pdocNext = NULL;
    m_ppdocPrev = NULL;

    m_isManagedByHost = FALSE;
    m_isScriptlet = FALSE;
    m_isLineCountValid = FALSE;
    
    m_utf8SourceInfo = nullptr;
    m_ichMinDisplay = 0;
    m_ichLimDisplay = 0;
    m_cln = 0;

    m_sourceTextAttirbutes = NULL;

    m_fIsMarkedClosed = FALSE;
}


CScriptSourceDocumentText::~CScriptSourceDocumentText(void)
{
    AddRef(); // so we don't get released again!
    Close();
}

ULONG CScriptSourceDocumentText::AddRef(void)
{
    return InternalAddRef();
}

ULONG CScriptSourceDocumentText::Release(void)
{
    ULONG l = InternalRelease();
    if (l == 0)
        delete this;
    return l;
}

HRESULT CScriptSourceDocumentText::QueryInterface(REFIID iid, void ** ppvObject)
{
    CHECK_POINTER(ppvObject);

    if (InlineIsEqualGUID(iid, IID_IDebugDocumentText))
        *ppvObject = static_cast<IDebugDocumentText *>(this);
    else if (InlineIsEqualGUID(iid, IID_IDebugDocument))
        *ppvObject = static_cast<IDebugDocument *>(this);
    else if (InlineIsEqualGUID(iid, IID_IDebugDocumentProvider))
        *ppvObject = static_cast<IDebugDocumentProvider *>(this);
    else if (InlineIsEqualGUID(iid, IID_IDebugDocumentInfo))
        *ppvObject = static_cast<IDebugDocumentInfo *>(static_cast<IDebugDocument *>(this));
    else
    {
        *ppvObject = NULL;
        return CStandardScriptSourceNode::QueryInterface(iid, ppvObject);
    }

    AddRef();
    return NOERROR;
}

long CScriptSourceDocumentText::CchDisplay(void)
{
    return m_ichLimDisplay - m_ichMinDisplay;
}

void CScriptSourceDocumentText::Link(CScriptSourceDocumentText **ppdocPrev)
{
    // Unlink from the current location
    if (NULL != m_ppdocPrev)
    {
        Assert(this == *m_ppdocPrev);
        *m_ppdocPrev = m_pdocNext;
        if (NULL != m_pdocNext)
        {
            Assert(m_pdocNext->m_ppdocPrev == &m_pdocNext);
            m_pdocNext->m_ppdocPrev = m_ppdocPrev;
            m_pdocNext = NULL;
        }
        m_ppdocPrev = NULL;
    }

    // Link to the new position
    if (NULL != ppdocPrev)
    {
        m_ppdocPrev = ppdocPrev;
        m_pdocNext = *m_ppdocPrev;
        *m_ppdocPrev = this;
        if (NULL != m_pdocNext)
        {
            Assert(m_pdocNext->m_ppdocPrev == ppdocPrev);
            m_pdocNext->m_ppdocPrev = &m_pdocNext;
        }
    }
}

CScriptSourceDocumentText *CScriptSourceDocumentText::GetNext(void)
{
    return m_pdocNext;
}

CScriptBody *CScriptSourceDocumentText::GetScriptBody(void)
{
    return m_scriptBody;
}

BOOL CScriptSourceDocumentText::IsScriptlet(void)
{
    return m_isScriptlet;
}

HRESULT CScriptSourceDocumentText::Close(void)
{
    HRESULT hr;

    AddRef();
    Link(NULL);

    MarkForClose();

    if (NULL != m_scriptEngine)
    {
        m_scriptEngine->Release();
        m_scriptEngine = NULL;
    }
    if (NULL != m_scriptBody)
    {
        // tell the body that we're being shut down
        CScriptBody *pbody = m_scriptBody;
        m_scriptBody = NULL;
        pbody->SetDoc(NULL);
        pbody->Release();
    }

    m_utf8SourceInfo = nullptr;
    m_ichMinDisplay = 0;
    m_ichLimDisplay = 0;

    if (NULL != m_sourceTextAttirbutes)
    {
        free(m_sourceTextAttirbutes);
        m_sourceTextAttirbutes = NULL;
    }

    hr = CStandardScriptSourceNode::Close();
    // BUGBUG - fire onClose() event

    Release();
    return hr;
}

HRESULT CScriptSourceDocumentText::MarkForClose()
{
    if (!m_fIsMarkedClosed)
    {
        CComAutoUnlockCS autoUnlock(&m_csForClose);
        m_fIsMarkedClosed = TRUE;
    }

    return S_OK;
}

HRESULT CScriptSourceDocumentText::Init(ScriptEngine *scriptEngine, ulong grfsi)
{
    // Assert that we're clean
    AssertMem(this);
    Assert(NULL == m_scriptEngine);
    Assert(NULL == m_scriptBody);
    Assert(NULL == m_pdocNext);
    Assert(NULL == m_ppdocPrev);
    Assert(!m_isManagedByHost);
    Assert(!m_isScriptlet);
    Assert(!m_isLineCountValid);
    Assert(nullptr == m_utf8SourceInfo);
    Assert(0 == m_ichMinDisplay);
    Assert(0 == m_ichLimDisplay);
    Assert(0 == m_cln);
    Assert(NULL == m_sourceTextAttirbutes);

    AssertMem(scriptEngine);

    m_isManagedByHost = (grfsi & fsiHostManaged) != 0;
    m_isScriptlet = (grfsi & fsiScriptlet) != 0;
    scriptEngine->AddRef();
    m_scriptEngine = scriptEngine;

    return NOERROR;
}


void CScriptSourceDocumentText::SetScriptBody(CScriptBody *pbody)
{
    if (m_scriptBody == pbody)
        return;

    if (NULL != m_scriptBody)
    {
        Assert(NULL == pbody);
        m_scriptBody->Release();
        m_scriptBody = NULL;
        Close();
        return;
    }

    m_scriptBody = pbody;
    if(m_scriptBody)
    {
        m_scriptBody->AddRef();
    }
    AssertMem(pbody);
    AssertMem(m_scriptEngine);

    if (!m_isManagedByHost)
    {
        m_utf8SourceInfo = m_scriptBody->GetUtf8SourceInfo();
        m_ichMinDisplay = 0;
        m_ichLimDisplay = m_utf8SourceInfo->GetCchLength();

        if (IsScriptlet())
        {
            GetIchMinHost(&m_ichMinDisplay);
            GetIchLimHost(&m_ichLimDisplay);
            LPCUTF8 pchSrc = m_utf8SourceInfo->GetSource(L"CScriptSourceDocumentText::SetScriptBody");            
            LPCUTF8 pchMin = pchSrc + m_utf8SourceInfo->CharacterIndexToByteIndex(m_ichMinDisplay);
            LPCUTF8 pchLim = pchSrc + m_utf8SourceInfo->CharacterIndexToByteIndex(m_ichLimDisplay);

            while (m_ichMinDisplay > 0 &&
                *pchMin != '{')
            {
                pchMin = utf8::PrevChar(pchMin, pchSrc);
                m_ichMinDisplay--;
            }

            long cchSrc = static_cast<long>(m_utf8SourceInfo->GetCchLength());
            while (m_ichLimDisplay < cchSrc &&
                (m_ichLimDisplay == 0 ||
                *pchLim != '}'))
            {
                pchLim = utf8::NextChar(pchLim);
                m_ichLimDisplay++;
            }
        }
    }
}


SRCINFO *CScriptSourceDocumentText::GetSourceInfo(void)
{
    long cb;
    SRCINFO *srcInfo;

    if (NULL == m_scriptBody)
        return NULL;

    srcInfo = (SRCINFO *)(m_scriptBody->PvGetData(&cb));
    if (cb != sizeof(SRCINFO))
        return NULL;

    return srcInfo;
}


// Following function advances a character pointer to the beginning of the next
// line. If there are no more lines, the pointer is left at the end of
// the string, and the function returns FALSE.
BOOL CScriptSourceDocumentText::FAdvanceToNextLine(const OLECHAR **ppch)
{
    AssertMem(ppch);
    AssertMemR(*ppch);

    const OLECHAR *pch;

    for (pch = *ppch; ; )
    {
        switch (*pch++)
        {
        case OLESTR('\r'):
            if (*pch == OLESTR('\n'))
                pch++;
            *ppch = pch;
            return TRUE;
        case OLESTR('\n'):
            *ppch = pch;
            return TRUE;
        case OLESTR('\0'):
            *ppch = pch - 1;
            return FALSE;
        }
    }
}


void CScriptSourceDocumentText::UpdateLineCount(void)
{
    if (m_isLineCountValid)
        return;
    m_utf8SourceInfo->EnsureLineOffsetCache();
    size_t lines = m_utf8SourceInfo->GetLineCount();
    Assert(lines < MAXLONG);
    m_cln = static_cast< charcount_t>(lines);

    m_isLineCountValid = TRUE;
}


HRESULT CScriptSourceDocumentText::GetIchMinHost(long *pich)
{
    AssertMem(pich);
    SRCINFO *srcInfo;

    *pich = 0;
    if (NULL == (srcInfo = GetSourceInfo()))
        return HR(E_FAIL);
    *pich = srcInfo->ichMinHost;

    return NOERROR;
}


HRESULT CScriptSourceDocumentText::GetIchLimHost(long *pich)
{
    AssertMem(pich);
    SRCINFO *psi;

    *pich = m_utf8SourceInfo->GetCchLength();
    if (NULL == (psi = GetSourceInfo()))
        return HR(E_FAIL);
    *pich = psi->ichLimHost;

    return NOERROR;
}


HRESULT CScriptSourceDocumentText::GetHostSourceContext(DWORD_PTR *pdwContext)
{
    AssertMem(pdwContext);
    SRCINFO *srcInfo;

    *pdwContext = 0;
    if (NULL == (srcInfo = GetSourceInfo()))
        return HR(E_FAIL);
    *pdwContext = srcInfo->sourceContextInfo->dwHostSourceContext;

    return NOERROR;
}

HRESULT CScriptSourceDocumentText::GetSecondaryHostSourceContext(DWORD_PTR *pdwContext)
{
    AssertMem(pdwContext);
    if (this->m_scriptBody == NULL)
    {
        return E_FAIL;
    }

    *pdwContext = this->m_scriptBody->GetSecondaryHostSourceContext();

    return NOERROR;
}

HRESULT CScriptSourceDocumentText::EnumCodeContextsOfHostPosition(
    long ich, long cch, IEnumDebugCodeContexts **ppescc)
{
    Assert(ich >= 0);
    Assert(cch >= 0);
    AssertMem(ppescc);

    long ichMinHost;

    *ppescc = NULL;
    if (FAILED(GetIchMinHost(&ichMinHost)))
        return HR(E_FAIL);

    return EnumCodeContextsOfPosition(ich + ichMinHost, cch, ppescc);
}


HRESULT CScriptSourceDocumentText::EnumCodeContextsOfPosition(
    long ich, long cch, IEnumDebugCodeContexts **ppDebugCodeContexts)
{
    Assert(ich >= 0);
    Assert(cch >= 0);
    AssertMem(ppDebugCodeContexts);

    CCodeContext *codeContext = NULL;
    CEnumCodeContexts *codeContexts = NULL;
    HRESULT hr;
    StatementSpan bos;
    bos.cch = cch;
    bos.ich = ich;

    *ppDebugCodeContexts = NULL;
    if (NULL == m_scriptBody)
        return HR(E_FAIL);

    if (!m_scriptBody->GetStatementSpan(ich, &bos))
    {
        hr = E_FAIL;
        goto Exit;
    }

    // Create a Code context that points to us:
    if (NULL == (codeContext = new CCodeContext(NULL, bos.ich, bos.cch )))
        return HR(E_OUTOFMEMORY);


    // Create a dummy enumerator for our code context
    if (NULL == (codeContexts = new CEnumCodeContexts))
    {
        hr = HR(E_OUTOFMEMORY);
        goto Exit;
    }

    if (FAILED(hr = codeContexts->Init(codeContext)))
        goto Exit;

    *ppDebugCodeContexts = codeContexts;
    codeContext->Release();
    return NOERROR;

Exit:
    if (NULL != codeContexts)
        codeContexts->Release();
    if (NULL != codeContext)
        codeContext->Release();

    return hr;
}


HRESULT CScriptSourceDocumentText::GetContextOfInternalPosition(
    long ich, long cch, IDebugDocumentContext **ppsc)
{
    if (!m_isManagedByHost)
    {
        if (ich >= m_ichMinDisplay)
            ich -= m_ichMinDisplay;
        else
        {
            ich = 0;
            cch = max<long>(0, cch + ich - m_ichMinDisplay);
        }

        if (cch > m_ichLimDisplay - ich)
            cch = m_ichLimDisplay - ich;
    }

    return GetContextOfPosition(ich, cch, ppsc);
}


HRESULT CScriptSourceDocumentText::GetDocument(IDebugDocument **ppDebugDocument)
{
    CHECK_POINTER(ppDebugDocument);

    *ppDebugDocument = (IDebugDocument *)this;
    (*ppDebugDocument)->AddRef();
    return NOERROR;
}

HRESULT CScriptSourceDocumentText::GetName(
    DOCUMENTNAMETYPE dnt,
    BSTR *pbstrName)
{
    return CStandardScriptSourceNode::GetName(dnt, pbstrName);
}

HRESULT CScriptSourceDocumentText::GetDocumentClassId(CLSID *pclsidDocument)
{
    CHECK_POINTER(pclsidDocument);
    *pclsidDocument = CLSID_NULL;
    return E_NOTIMPL;
}

HRESULT CScriptSourceDocumentText::GetSourceContextFromString(__in LPOLESTR psz,
    IDebugDocumentContext **ppsc)
{
    CHECK_POINTER(ppsc);
    *ppsc = NULL;
    return E_NOTIMPL;
}

HRESULT CScriptSourceDocumentText::GetDocumentAttributes(TEXT_DOC_ATTR *ptextdocattr)
{
    CHECK_POINTER(ptextdocattr);
    *ptextdocattr = TEXT_DOC_ATTR_READONLY;
    return NOERROR;
}

HRESULT CScriptSourceDocumentText::GetLineOfPosition(ULONG charPosition, ULONG *outLineNumber,
                                                     ULONG *outColumn)
{
    charcount_t lineNumber;
    charcount_t column;
    charcount_t ignoreByteOffset;
    if (NULL != outLineNumber)
        *outLineNumber = 0;
    if (NULL != outColumn)
        *outColumn = 0;
    if (charPosition > (ulong)CchDisplay())
        return HR(E_INVALIDARG);

    if (outLineNumber || outColumn)
    {
        HRESULT hr = this->m_utf8SourceInfo->EnsureLineOffsetCacheNoThrow(); 

        if (FAILED(hr)) 
        {
            return hr;
        }

        m_utf8SourceInfo->GetLineInfoForCharPosition(charPosition, &lineNumber, &column, &ignoreByteOffset);

        if (outLineNumber) 
        {
            *outLineNumber = lineNumber;
        }

        if (outColumn) 
        {
            *outColumn = column;
        }
    }

    return NOERROR;
}


HRESULT CScriptSourceDocumentText::GetSize(ULONG *pcln, ULONG *pcch)
{
    CComAutoUnlockCS autoUnlock(&m_csForClose);

    if (m_fIsMarkedClosed)
    {
        return E_FAIL;
    }

    long cch = CchDisplay();

    if (NULL != pcch)
        *pcch = cch;
    if (NULL != pcln)
    {
        if (!m_isLineCountValid)
            UpdateLineCount();
        *pcln = m_cln;
    }
    return NOERROR;
}


HRESULT CScriptSourceDocumentText::GetPositionOfLine(ULONG ln, ULONG *outCharPosition)
{
    CHECK_POINTER(outCharPosition);

    CComAutoUnlockCS autoUnlock(&m_csForClose);

    if (m_fIsMarkedClosed)
    {
        return E_FAIL;
    }

    if (!m_isLineCountValid)
        UpdateLineCount();

    if (ln >= (ulong)m_cln)
        return HR(E_INVALIDARG);

    *outCharPosition = 0;

    charcount_t charPosition;
    charcount_t byteOffset;
    m_utf8SourceInfo->GetCharPositionForLineInfo((charcount_t)ln, &charPosition, &byteOffset);

    *outCharPosition = charPosition;

    return NOERROR;
}


HRESULT CScriptSourceDocumentText::GetText(ULONG ich, __out_ecount_part_opt(cchMax,*pcch) OLECHAR *prgch,
                                           SOURCE_TEXT_ATTR *sourceTextAttributes, ULONG *pcch, ULONG cchMax)
{
    long cch, cchDisp;
    HRESULT hr;

    CHECK_POINTER(pcch);

    CComAutoUnlockCS autoUnlock(&m_csForClose);

    if (m_fIsMarkedClosed)
    {
        return E_FAIL;
    }

    cchDisp = CchDisplay();
    if (*pcch != 0 || ich > (ulong)cchDisp)
        return HR(E_INVALIDARG);

    cch = cchMax;
    if (cch > cchDisp - (long)ich)
        cch = cchDisp - (long)ich;
    Assert(cch >= 0 && cch <= cchDisp - (long)ich);

    if (cch == 0)
    {
        *pcch = 0;
        return NOERROR;
    }

    if (NULL != sourceTextAttributes)
    {
        if (NULL == m_sourceTextAttirbutes)
        {
            if (NULL == m_scriptEngine)
                return HR(E_NOINTERFACE);
            if (NULL == (m_sourceTextAttirbutes = (SOURCE_TEXT_ATTR *)malloc(
                m_utf8SourceInfo->GetCchLength() * sizeof(SOURCE_TEXT_ATTR))))
            {
                return HR(E_OUTOFMEMORY);
            }

            hr = m_scriptEngine->GetScriptTextAttributesUTF8(m_utf8SourceInfo->GetSource(L"ScpText::GetText"), static_cast< ULONG >(m_utf8SourceInfo->GetCbLength(L"ScpText::GetText")), 
                NULL, static_cast< ULONG >(m_utf8SourceInfo->GetCchLength()), 0, m_sourceTextAttirbutes);
            if (FAILED(hr))
            {
                free(m_sourceTextAttirbutes);
                m_sourceTextAttirbutes = NULL;
                return hr;
            }

            long ista;

            for (ista = 0 ; ista < m_ichMinDisplay; ista++)
                m_sourceTextAttirbutes[ista] |= SOURCETEXT_ATTR_NONSOURCE;
            long cch = static_cast<long>(m_utf8SourceInfo->GetCchLength());
            for (ista = m_ichLimDisplay; ista < cch; ista++)
                m_sourceTextAttirbutes[ista] |= SOURCETEXT_ATTR_NONSOURCE;
        }
        js_memcpy_s(sourceTextAttributes, cchMax, m_sourceTextAttirbutes + ich + m_ichMinDisplay,
            cch * sizeof(SOURCE_TEXT_ATTR));
    }

    if (NULL != prgch)
    {
        m_utf8SourceInfo->RetrieveSourceText(prgch, ich, ich + cch);
    }

    *pcch = cch;

    return NOERROR;
}


STDMETHODIMP CScriptSourceDocumentText::GetPositionOfContext(
    IDebugDocumentContext *psc, ULONG *pich, ULONG *pcch)
{
    CScriptSourceContext *scriptSourceContext;

    long ich, cch;

    if (NULL != pich)
        *pich = 0;
    if (NULL != pcch)
        *pcch = 0;

    if (NULL == pich || NULL == psc)
        return HR(E_POINTER);

    scriptSourceContext = (CScriptSourceContext *)psc;

    // BUGBUG - verify safely that this source context points to us
    if (scriptSourceContext->m_scriptDocumentText != this)
        return HR(E_UNEXPECTED);

    ich = scriptSourceContext->m_cCharacterPosition;
    cch = scriptSourceContext->m_cNumChars;

    if (ich >= m_ichMinDisplay)
        ich -= m_ichMinDisplay;
    else
    {
        cch += ich - m_ichMinDisplay;
        if (cch < 0)
            cch = 0;
        ich = 0;
    }

    *pich = ich;
    if (NULL != pcch)
        *pcch = cch;

    return NOERROR;
}


STDMETHODIMP CScriptSourceDocumentText::GetContextOfPosition(
    ULONG ich, ULONG cch, IDebugDocumentContext **ppDebugDocumentContext)
{
    if (m_fIsMarkedClosed)
    {
        return E_FAIL;
    }

    CScriptSourceContext *sourceContext;
    DWORD_PTR dwContext;

    CHECK_POINTER(ppDebugDocumentContext);
    *ppDebugDocumentContext = NULL;

    if (m_isManagedByHost && SUCCEEDED(GetHostSourceContext(&dwContext)))
    {
        long ichMinHost;
        long ichLimHost;

        if (SUCCEEDED(GetIchMinHost(&ichMinHost)) &&
            SUCCEEDED(GetIchLimHost(&ichLimHost)))
        {
            long ichMin = ich;
            long ichLim = ich + cch;

            // Truncate range to host-owned portion
            if (ichMin > ichLimHost)
                ichMin = ichLimHost;
            if (ichMin < ichMinHost)
                ichMin = ichMinHost;
            if (ichLim > ichLimHost)
                ichLim = ichLimHost;
            if (ichLim < ichMin)
                ichLim = ichMin;

            return m_scriptEngine->GetDocumentContextFromPosition(dwContext,
                ichMin - ichMinHost, ichLim - ichMin, ppDebugDocumentContext);
        }
    }

    if (ich + cch > (ulong)CchDisplay())
        return HR(E_INVALIDARG);

    if (NULL == (sourceContext = new CScriptSourceContext))
        return HR(E_OUTOFMEMORY);

    ich += m_ichMinDisplay;
    sourceContext->Set(this, ich, cch);

    *ppDebugDocumentContext = (IDebugDocumentContext *)sourceContext;

    return NOERROR;
}

UINT CScriptSourceContext::InternalAddRef(void)
{
    return (ULONG)InterlockedIncrement((LPLONG)&m_cRef);
}

UINT CScriptSourceContext::InternalRelease(void)
{
    return (ULONG)InterlockedDecrement((LPLONG)&m_cRef);
}

IUnknown *CScriptSourceContext::ptextunk()
{
    return static_cast<IUnknown *>(static_cast<IDebugDocumentText *>(m_scriptDocumentText));
}

CScriptSourceContext::CScriptSourceContext()
{
    _Module.Lock();
    m_cRef = 1;
    m_scriptDocumentText = NULL;
    m_cCharacterPosition = 0;
    m_cNumChars = 0;
}

CScriptSourceContext::CScriptSourceContext(
        CScriptSourceDocumentText *scriptDocumentText,
        ULONG cPos,
        ULONG cNumChars)
{
    _Module.Lock();
    m_cRef = 1;
    m_scriptDocumentText = scriptDocumentText;
    if (m_scriptDocumentText)
    {
        ptextunk()->AddRef();
    }
    m_cCharacterPosition = cPos;
    m_cNumChars = cNumChars;
}

void CScriptSourceContext::Set(
    CScriptSourceDocumentText *pdoc,
    ULONG cPos,
    ULONG cNumChars)
{
    if (m_scriptDocumentText)
    {
        ptextunk()->Release();
    }
    m_scriptDocumentText = pdoc;
    if (m_scriptDocumentText)
    {
        ptextunk()->AddRef();
    }
    m_cCharacterPosition = cPos;
    m_cNumChars = cNumChars;
}

CScriptSourceContext::~CScriptSourceContext()
{
    if (m_scriptDocumentText)
    {
        ptextunk()->Release();
    }
    _Module.Unlock();
}

    // IUnknown:

ULONG CScriptSourceContext::AddRef(void)
{
    return InternalAddRef();
}

ULONG CScriptSourceContext::Release(void)
{
    ULONG l = InternalRelease();
    if (l == 0)
        delete this;
    return l;
}

HRESULT CScriptSourceContext::QueryInterface(REFIID iid, void ** ppvObject)
{
    CHECK_POINTER(ppvObject);

    if (InlineIsEqualGUID(iid, IID_IUnknown))
        *ppvObject = static_cast<IUnknown *>(this);
    else if (InlineIsEqualGUID(iid, IID_IDebugDocumentContext))
        *ppvObject = static_cast<IDebugDocumentContext *>(this);
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

    // IDebugDocumentContext:

HRESULT CScriptSourceContext::GetDocument(IDebugDocument **ppDebugDocument)
{
    CHECK_POINTER (ppDebugDocument);
    *ppDebugDocument = static_cast<IDebugDocument *>(m_scriptDocumentText);
    if (!m_scriptDocumentText)
    {
        return E_NOINTERFACE;
    }
    (*ppDebugDocument)->AddRef();
    return S_OK;
}

HRESULT CScriptSourceContext::EnumCodeContexts(IEnumDebugCodeContexts **ppDebugCodeContexts)
{
    CHECK_POINTER(ppDebugCodeContexts);
    *ppDebugCodeContexts = NULL;

    if (!m_scriptDocumentText)
    {
        return E_FAIL;
    }

    return m_scriptDocumentText->EnumCodeContextsOfPosition(
        m_cCharacterPosition,
        m_cNumChars,
        ppDebugCodeContexts);
}

HRESULT CScriptSourceContext::GetContextString(BSTR *pbstrSourceContextString)
{
    CHECK_POINTER(pbstrSourceContextString);
    *pbstrSourceContextString = NULL;
    return E_NOTIMPL;
}

HRESULT CScriptNonDebugDocumentText::Init(ScriptEngine *scriptEngine, Js::FunctionBody* functionBody, BSTR bstrUrl)
{
    Assert(functionBody->GetHostSrcInfo() != nullptr);
    
    m_pSrcInfo = functionBody->GetHostSrcInfo();
    m_uSourceIndex = functionBody->GetSourceIndex();

    HRESULT hr = CScriptSourceDocumentText::Init(scriptEngine, this->m_pSrcInfo->grfsi);
    if (hr == S_OK)
    {
        Js::ScriptContext *scriptContext = scriptEngine->GetScriptContext();
        Assert(scriptContext != nullptr);
        if (scriptContext->IsItemValidInSourceList(m_uSourceIndex)) // Guard due to bug : 380959
        {
            this->m_isManagedByHost = false;

            m_utf8SourceInfo = scriptContext->GetSource(m_uSourceIndex);
            m_ichMinDisplay = 0;
            m_ichLimDisplay = m_utf8SourceInfo->GetCchLength();

            m_bstrUrl = bstrUrl;
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP CScriptNonDebugDocumentText::GetLineOfPosition(ULONG ich, ULONG *pln, ULONG *pich)
{
    Assert(m_pSrcInfo != NULL);

    if (ich < m_pSrcInfo->ichMinHost)
    {
        ich = m_pSrcInfo->ichMinHost;
    }

    if (ich > m_pSrcInfo->ichLimHost)
    {
        ich = m_pSrcInfo->ichLimHost;
    }

    ULONG ulLine, ulColumn;
    HRESULT hr = CScriptSourceDocumentText::GetLineOfPosition(ich, &ulLine, &ulColumn);

    if (hr == NOERROR)
    {
        if (ulLine >= m_pSrcInfo->lnMinHost)
        {
            ulLine = ulLine - m_pSrcInfo->lnMinHost;
        }
        else
        {
            // We are in range before the host specified range but we had already trucated the ich
            Assert(FALSE);
        }

        ulLine = ulLine + m_pSrcInfo->dlnHost;

        if (ulLine == m_pSrcInfo->dlnHost)
        {
            ulColumn = ulColumn + m_pSrcInfo->ulColumnHost;
        }

        if (pln != NULL)
        {
            *pln = ulLine;
        }

        if (pich != NULL)
        {
            *pich = ulColumn;
        }
    }

    return hr;
}
