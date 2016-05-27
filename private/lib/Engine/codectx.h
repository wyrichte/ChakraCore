/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

class CScriptBody;
class CCodeContext;

// Internal interface so we can safetly get the bos from a code context.
interface IInternalCodeContext : public IUnknown
{
    STDMETHOD(GetCodeContextClass)(CCodeContext **ppcc) PURE;
};


//REVIEW: using pointers to ID's is necessary because some compilers don't
// like references as template arguments.

class ScriptEngine;
class ScriptDebugDocument;

interface IDebugCodeContext;
interface IDebugStackFrame;
interface IDebugExpressionContext;


class CCodeContext sealed : public IDebugCodeContext, public IInternalCodeContext
{
private:
    long m_refCount;
    long m_ibos;
    ULONG m_cch;
    ScriptDebugDocument * m_debugDocument;
    bool m_isLibraryCode;

public:
    CCodeContext(ScriptDebugDocument *debugDocument, long ibos, ULONG cch, bool isLibraryCode = false);
    ~CCodeContext(void);

    HRESULT Close(void);
    long GetStartPos() const { return m_ibos; }
    CScriptBody *Pbody(void);

    // IUnknown
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);

    // IDebugCodeContext
    STDMETHODIMP GetDocumentContext(IDebugDocumentContext **ppsc);
    STDMETHODIMP SetBreakPoint(BREAKPOINT_STATE bps);

    // IInternalCodeContext
    STDMETHOD(GetCodeContextClass)(CCodeContext **ppcc);
};


class CEnumCodeContexts sealed :
    public IEnumDebugCodeContexts
{
private:
    long m_refCount;
    IDebugCodeContext *m_pCodeContext;
    UINT m_cIndex;

public:
    CEnumCodeContexts();
    ~CEnumCodeContexts();

    HRESULT Init(IDebugCodeContext *pscc);

    // IUnknown:
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppv);

    // IEnumDebugCodeContexts:

    STDMETHODIMP Next(
        /* [in]  */ ULONG celt,
        /* [out] */IDebugCodeContext **pscc,
        /* [out] */ ULONG *pceltFetched);

    STDMETHODIMP Skip(
        /* [in]  */ ULONG celt);

    STDMETHODIMP Reset(void);

    STDMETHODIMP Clone(
        /* [out] */ IEnumDebugCodeContexts **ppescc);
};


/***************************************************************************
CDebugStackFrame
***************************************************************************/
class CDebugStackFrame sealed :
    public IDebugStackFrame,
    public IDebugExpressionContext,
    public IDebugThreadCall,
    public IDebugSetValueCallback,
    public ISetNextStatement
{
private:
    long m_refCount;

    ScriptSite* m_scriptSite;
    Js::DiagStackFrame* m_currentFrame;
    Js::WeakDiagStack* m_framePointers;
    CComPtr<IDebugBitCorrectApplicationThread> m_pApplicationThread;
    CComPtr<IDebugApplication> m_pDebugApplication;
    int m_frameIndex;

#if defined(DBG) || defined(ENABLE_TRACE)
    // This field mentions that the current frame object given to the debugger is synced with ProbeManager debugSessionNumber
    ulong m_debugSessionNumber;
#endif
    enum
    {
        xthread_GetDebugProperty,
        xthread_CanSetNextStatement,
        xthread_SetNextStatement
    };

    struct SetValueParam
    {
        VARIANT *pvarNode;
        DISPID dispid;
        ULONG ielt;
        LPCOLESTR pszValue;
    };

    HRESULT GetDebugPropertyCore(IDebugProperty **ppdp);
    HRESULT SetValueCore(SetValueParam *psvp);

    HRESULT CanDoSetNextStatement(__in IDebugStackFrame *stackFrame, __in IDebugCodeContext *pdcc, __in Js::FunctionBody *& _pFuncBody, __out int * pBytecodeAtNext);
    static bool IsJmpCrossInterpreterStackFrame(uint byteCodeOffsetA, uint byteCodeOffsetB, Js::FunctionBody* funcBody);

    bool CanDoBlockScopeSetNextStatement(Js::FunctionBody* pFuncBody, int startOffset, int endOffset);
    bool CanJumpWithinCurrentBlock(Js::DebuggerScope* debuggerScope, int startOffset, int endOffset);
    bool CanJumpIntoInnerBlock(
        Js::DebuggerScope* outerBlock,
        Js::DebuggerScope* innerBlock,
        int startOffset,
        int endOffset);
#if defined(DBG) || defined(ENABLE_TRACE)
    void ValidateLegitDebugSession();
#endif

public:
    CDebugStackFrame(void);
    ~CDebugStackFrame(void);

    HRESULT Init(ScriptSite* scriptSite, int frameIndex);

    HRESULT Close(void);

    // IUnknown
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);

    // IDebugStackFrame
    STDMETHODIMP GetCodeContext(IDebugCodeContext **ppscc);
    STDMETHODIMP GetDescriptionString(BOOL fLong, BSTR *pbstr);
    STDMETHODIMP GetLanguageString(BOOL fLong, BSTR *pbstr);
    STDMETHODIMP GetThread(IDebugApplicationThread **ppat);
    STDMETHODIMP GetDebugProperty(IDebugProperty **ppdp);

    // IDebugExpressionContext
    STDMETHODIMP ParseLanguageText(LPCOLESTR pszSrc, UINT uRadix,
        LPCOLESTR pszDelimiter, DWORD dwFlags, IDebugExpression **ppe);
    STDMETHODIMP GetLanguageInfo(BSTR *pbstrLang, GUID *pguidLang);

    // IDebugThreadCall
    STDMETHODIMP ThreadCallHandler(DWORD_PTR dwParam1, DWORD_PTR dwParam2,
        DWORD_PTR dwParam3);

    // IDebugSetValueCallback
    STDMETHOD(SetValue)(VARIANT *pvarNode, DISPID dispid, ULONG clIndicies,
        LONG *prglIndicies, LPCOLESTR pszValue, UINT nRadix,
        BSTR *pbstrError);

    // ISetNextStatement
    STDMETHOD(CanSetNextStatement)(IDebugStackFrame *stackFrame,
        IDebugCodeContext *pdcc);
    STDMETHOD(SetNextStatement)(IDebugStackFrame *stackFrame,
        IDebugCodeContext *pdcc);

    // CDebugStackFrame
    HRESULT EvaluateImmediate(LPCOLESTR pszSrc, DWORD dwFlags,
        IDebugProperty **ppdp);
    HRESULT GetNextFrame(CDebugStackFrame **ppdsf);
    HRESULT GetPhysicalStackRange(DWORD_PTR *pdwMin, DWORD_PTR *pdwLim);
};


/***************************************************************************
CDebugEval - our IDebugSyncOperation for debug evaluation.

This object is also multithreaded. The Execute method is guaranteed
to always be called on the host application thread.

The InProgressAbort, GetResult, and GetResultAsDebugProperty are called
from the debugger thread.

InProgressAbort is called by the PDM. The GetResult and
GetResultAsDebugProperty methods are called by CDebugExpression.
***************************************************************************/
class CDebugEval sealed : public IDebugSyncOperation
{
private:
    long m_refCount;
    DWORD m_dwFlags;
    BSTR m_bstrSrc;
    CDebugStackFrame *m_stackFrame;
    IDebugApplicationThread *m_applicationThread;
    bool m_isAborted;

    CDebugEval(void);
    virtual ~CDebugEval(void);

public:
    static HRESULT Create(CDebugEval **ppdev, LPCOLESTR pszSrc, DWORD dwFlags,
        CDebugStackFrame *stackFrame, IDebugApplicationThread *pat);

    /****************************************
    *   IUnknown                            *
    ****************************************/
    STDMETHODIMP_(ULONG) AddRef(void)
    { return (ulong)InterlockedIncrement(&m_refCount); }

    STDMETHODIMP_(ULONG) Release(void)
    {
        ulong lu = (ulong)InterlockedDecrement(&m_refCount);
        if (lu == 0)
            delete this;
        return lu;
    }

    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);


    /****************************************
    *   IDebugSyncOperation                 *
    ****************************************/
    STDMETHODIMP GetTargetThread(IDebugApplicationThread **ppatTarget);
    STDMETHODIMP Execute(IUnknown **ppunkRes);
    STDMETHODIMP InProgressAbort(void);

public:
    void SetAborted(bool set) { m_isAborted = set; }
};


/***************************************************************************
CDebugExpression

This object is free threaded. It executes in both the debugger thread and
the host application threads.
***************************************************************************/
class CDebugExpression sealed : public IDebugExpression,
    public IDebugAsyncOperationCallBack
{
private:
    long m_refCount;
    CDebugStackFrame *m_stackFrame;
    CDebugEval *m_debugEval;
    IDebugAsyncOperation *m_asyncOperation;
    IDebugExpressionCallBack *m_exprCallback;

    CDebugExpression(CDebugEval *debugEval);
    ~CDebugExpression(void);

public:
    static HRESULT Create(CDebugExpression **ppdexp, CDebugEval *debugEval, IDebugApplication *pda);

    /****************************************
    IUnknown
    ****************************************/
    STDMETHODIMP_(ULONG) AddRef(void)
    { return (ulong)InterlockedIncrement(&m_refCount); }

    STDMETHODIMP_(ULONG) Release(void)
    {
        ulong lu = (ulong)InterlockedDecrement(&m_refCount);
        if (lu == 0)
            delete this;
        return lu;
    }

    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);

    /****************************************
    IDebugExpression
    ****************************************/
    STDMETHODIMP Start(IDebugExpressionCallBack *pdecb);
    STDMETHODIMP Abort(void);
    STDMETHODIMP QueryIsComplete(void);
    STDMETHODIMP GetResultAsString(HRESULT *phrRes, BSTR *pbstrRes);
    STDMETHODIMP GetResultAsDebugProperty(HRESULT *phrRes,
        IDebugProperty **ppdp);

    /****************************************
    IDebugAsyncOperationCallBack
    ****************************************/
    STDMETHODIMP onComplete(void);
};


/***************************************************************************
CEnumDebugStackFrames
***************************************************************************/
class CEnumDebugStackFrames sealed : public IEnumDebugStackFrames64
{
private:
    long m_refCount;
    DWORD m_dwThread;
    DWORD_PTR m_dwSpMin;


    ulong m_currentFrameIndex;
    bool m_fDone: 1;
    bool m_fError: 1;
    CDebugStackFrame *m_stackFramePrev;
    ScriptSite *m_scriptSite;
    Js::WeakDiagStack* m_framePointers;

    ~CEnumDebugStackFrames(void);

    HRESULT NextCommon();
    template <typename Descriptor>
    HRESULT NextImpl(ulong celt, Descriptor *prgdsfd, ulong *pceltFetched);

public:
    CEnumDebugStackFrames(DWORD_PTR dwSpMin, ScriptSite *psess);
    HRESULT Init();

    /****************************************
    IUnknown
    ****************************************/
    STDMETHODIMP_(ULONG) AddRef(void)
    { return (ulong)InterlockedIncrement(&m_refCount); }

    STDMETHODIMP_(ULONG) Release(void)
    {
        ulong lu = (ulong)InterlockedDecrement(&m_refCount);
        if (lu == 0)
            delete this;
        return lu;
    }

    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);

    /****************************************
    IEnumDebugStackFrames
    ****************************************/
    STDMETHODIMP Next(ulong celt, DebugStackFrameDescriptor *prgdsfd,
        ulong *pceltFetched);
    STDMETHODIMP Next64(ulong celt, DebugStackFrameDescriptor64 *prgdsfd,
        ulong *pceltFetched);
    STDMETHODIMP Skip(ulong celt);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumDebugStackFrames **ppedsf);
};
