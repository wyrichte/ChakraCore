/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
// ScpText.h : Declaration of the CScriptSourceTextDocument

#pragma once

//REVIEW -- using pointers to ID's is necessary because some compilers don't like
//references as template arguments.

class CScriptSourceDocumentText;
class CScriptSourceContext;
class ScriptEngine;

class CComAutoUnlockCS
{
public:
    CComAutoUnlockCS(CComAutoCriticalSection *pCS)
    {
        m_pCS = pCS;
        Assert(m_pCS);
        m_pCS->Lock();
    }

    ~CComAutoUnlockCS()
    {
        if (m_pCS)
        {
            m_pCS->Unlock();
        }
    }

private:
        CComAutoCriticalSection * m_pCS;
};

class CScriptSourceDocumentText :
    public IDebugDocumentText,
    public IDebugDocumentProvider,
    public CStandardScriptSourceNode
{
    friend class CScriptSourceContext;

protected:
    // Containing script object
    ScriptEngine *m_scriptEngine;

    // The corresponding code body. We don't have a ref count on this. It is
    // the responsibility of the body to own a ref count on us and notify us
    // when it is going away, by calling SetBody(NULL).
    CScriptBody *m_scriptBody;

    // For linking to a linked list
    CScriptSourceDocumentText *m_pdocNext;
    CScriptSourceDocumentText **m_ppdocPrev;

    // Flags
    bool m_isManagedByHost : 1;
    bool m_isScriptlet : 1;
    bool m_isLineCountValid : 1;
    bool m_fIsMarkedClosed :1;

    // Source code if we're not managed by the host. This is owned by m_pbody.
    Js::Utf8SourceInfo* m_utf8SourceInfo;
    long m_ichMinDisplay; // range to display
    long m_ichLimDisplay;
    charcount_t m_cln;           // number of lines (if m_fClnValid)

    // Cached attributes
    SOURCE_TEXT_ATTR *m_sourceTextAttirbutes;

    SRCINFO *GetSourceInfo(void);
    BOOL FAdvanceToNextLine(const OLECHAR **ppch);
    void UpdateLineCount(void);
    long CchDisplay(void);

    CComAutoCriticalSection m_csForClose;

public:

    CScriptSourceDocumentText(void);
    ~CScriptSourceDocumentText(void);

    STDMETHODIMP Close(void);

    // This will set a dirty bit, which will help us synchronizing threads. On a stress condition one thread might be closing current object and the debugger
    // thread is trying to GetText.
    HRESULT MarkForClose();

    HRESULT Init(ScriptEngine *pos, ulong grfsi);

    void Link(CScriptSourceDocumentText **ppdocPrev);
    CScriptSourceDocumentText *GetNext(void);

    void SetScriptBody(CScriptBody *pbody);
    CScriptBody *GetScriptBody(void);

    BOOL IsScriptlet(void);

    HRESULT GetHostSourceContext(DWORD_PTR *pdwContext);
    HRESULT GetSecondaryHostSourceContext(DWORD_PTR *pdwContext);
    HRESULT EnumCodeContextsOfHostPosition(long ich, long cch,
        IEnumDebugCodeContexts **ppescc);
    HRESULT EnumCodeContextsOfPosition(long ich, long cch,
        IEnumDebugCodeContexts **ppescc);
    HRESULT GetContextOfInternalPosition(long ich, long cch,
        IDebugDocumentContext **ppsc);

    HRESULT GetIchMinHost(long *pich);
    HRESULT GetIchLimHost(long *pich);

    // IUnknown:
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void) sealed;
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

    // IDebugDocumentProvider:
    STDMETHODIMP GetDocument(
        /* [out] */ IDebugDocument **ppssd);

    // IDebugDocumentInfo:
    STDMETHODIMP GetName(
        /* [in]  */ DOCUMENTNAMETYPE dnt,
        /* [out] */ BSTR *pbstrName);
    STDMETHODIMP GetDocumentClassId(CLSID *pclsidDocument);

    // IDebugDocument:
    STDMETHODIMP GetSourceContextFromString(__in LPOLESTR psz,
        IDebugDocumentContext **ppsc);

    // IDebugDocumentText
    STDMETHODIMP GetDocumentAttributes(TEXT_DOC_ATTR *ptextdocattr);
    virtual STDMETHODIMP GetLineOfPosition(ULONG charPosition, ULONG *outLineNumber, ULONG *outColumn);
    STDMETHODIMP GetSize(ULONG *pcln, ULONG *pcch);
    STDMETHODIMP GetPositionOfLine(ULONG ln, ULONG *outCharPosition);
    STDMETHODIMP GetText(ULONG ich, __out_ecount_part_opt(cchMax,*pcch) OLECHAR *prgch, SOURCE_TEXT_ATTR *prgsta,
        ULONG *pcch, ULONG cchMax);
    STDMETHODIMP GetPositionOfContext(IDebugDocumentContext *psc,
        ULONG *pich, ULONG *pcch);
    STDMETHODIMP GetContextOfPosition(ULONG ich, ULONG cch,
        IDebugDocumentContext **ppsc) sealed;
};


class CScriptSourceContext sealed :
    public IDebugDocumentContext
{
    friend class CScriptSourceDocumentText;

protected:
    UINT                       m_cRef;
    CScriptSourceDocumentText *m_scriptDocumentText;
    ULONG                      m_cCharacterPosition;
    ULONG                      m_cNumChars;

protected:
    UINT InternalAddRef(void);
    UINT InternalRelease(void);

public:
    IUnknown *ptextunk();
    CScriptSourceContext();
    CScriptSourceContext(
        CScriptSourceDocumentText *pdoc,
        ULONG cPos,
        ULONG cNumChars);
    void Set(
        CScriptSourceDocumentText *pdoc,
        ULONG cPos,
        ULONG cNumChars);
    ~CScriptSourceContext();

    // IUnknown:
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

    // IDebugDocumentContext:
    STDMETHODIMP GetDocument(IDebugDocument **ppsd);
    STDMETHODIMP EnumCodeContexts(IEnumDebugCodeContexts **ppescc);
    STDMETHODIMP GetContextString(BSTR *pbstrSourceContextString);
};

class CScriptNonDebugDocumentText :
    public CScriptSourceDocumentText
{
protected:
    SRCINFO const *m_pSrcInfo;
    uint m_uSourceIndex;

public:
    // it takes the URL and owns its management
    HRESULT Init(ScriptEngine *scriptEngine, Js::FunctionBody *functionBody, BSTR bstrUrl);

    virtual STDMETHODIMP GetLineOfPosition(ULONG ich, ULONG *pln, ULONG *pich);
    uint GetSourceIndex()
    {
        return m_uSourceIndex;
    }
};
