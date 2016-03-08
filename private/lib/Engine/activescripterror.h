/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
class ActiveScriptError sealed:
#if _WIN64 || USE_32_OR_64_BIT
                           public IActiveScriptErrorEx,
                           public IActiveScriptErrorDebug,
                           public IActiveScriptErrorDebug110,
                           public IActiveScriptWinRTErrorDebug,
                           public IRemoteDebugCriticalErrorEvent110,
                           public IActiveScriptError64
#else // _WIN64 || USE_32_OR_64_BIT
                           public IActiveScriptErrorEx,
                           public IActiveScriptErrorDebug,
                           public IActiveScriptErrorDebug110,
                           public IActiveScriptWinRTErrorDebug,
                           public IRemoteDebugCriticalErrorEvent110
#endif // _WIN64 || USE_32_OR_64_BIT
{
    ActiveScriptError();
    ~ActiveScriptError();
    void Free(void);

    class ExternalStackTrace
    {
        const uint m_maxStackFramesOnSO;

        bool isSO;
        int m_startOffset;
        int m_numFramesToCopy;
        int m_totalFrameCount;
        int m_framesToFillOffset;
        CallStackFrame* m_frames;

        int StackTraceStartOffsetOnSO(Js::JavascriptExceptionObject& exceptionObject);
    
    public:
        ExternalStackTrace(HRESULT exceptionHR, Js::JavascriptExceptionObject& exceptionObject);
        int StartOffset() { return m_startOffset; }
        int NumFramesToCopy() { return m_numFramesToCopy; }
        int TotalFrameCount() { return m_totalFrameCount; }
        CallStackFrame* FramesToFill() { return m_frames ? m_frames + m_framesToFillOffset : NULL; }
        CallStackFrame* AllFrames() { return m_frames; }
        void Dump();
    };
public:

    // === IUnknown ===
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // === IActiveScriptError ===
    STDMETHOD(GetExceptionInfo)(THIS_ EXCEPINFO *pei);
    STDMETHOD(GetSourcePosition)(THIS_ DWORD *pdwSourceContext,
            ULONG *pulLineNumber, LONG *plCharacterPosition);
#if _WIN64 || USE_32_OR_64_BIT
    STDMETHOD(GetSourcePosition64)(THIS_ DWORDLONG *pdwSourceContext,
            ULONG *pulLineNumber, LONG *plCharacterPosition);
#endif // _WIN64 || USE_32_OR_64_BIT
    STDMETHOD(GetSourceLineText)(THIS_ BSTR *pbstrSourceLine);

    // === IActiveScriptErrorDebug ===
    STDMETHOD(GetDocumentContext)(IDebugDocumentContext **ppssc);
    STDMETHOD(GetStackFrame)(IDebugStackFrame **ppdsf);

    // === IActiveScriptErrorEx ===
    STDMETHOD(GetExtendedExceptionInfo)(ExtendedExceptionInfo *exceptionInfo);
    STDMETHOD(HasDispatchedToDebugger)(BOOL *pfHasDispatched);
    STDMETHOD(GetThrownObject)(Var* thrownObject);


    // === IActiveScriptErrorDebug110  ===
    // Returns S_OK on success of the operation.
    STDMETHOD(GetExceptionThrownKind)( SCRIPT_ERROR_DEBUG_EXCEPTION_THROWN_KIND *pExceptionKind);

    // === IActiveScriptWinRTErrorDebug ===
    STDMETHOD(GetRestrictedErrorString)(__deref_out BSTR * errorString);
    STDMETHOD(GetRestrictedErrorReference)(__deref_out BSTR * referenceString);
    STDMETHOD(GetCapabilitySid)(__deref_out BSTR * capabilitySid);

    // === IRemoteDebugCriticalErrorEvent110 ===
    STDMETHOD(GetErrorInfo)(BSTR* pbstrSource,
        int* pMessageId,
        BSTR* pbstrMessage,
        IDebugDocumentContext** ppLocation);

    static HRESULT CreateCompileError(const SRCINFO * psi, CompileScriptException * pcse, Js::Utf8SourceInfo* sourceInfo, ActiveScriptError **ppase);
    static HRESULT CreateRuntimeError(Js::JavascriptExceptionObject *pError, HRESULT * pHrError, IDebugStackFrame* pStackFrame, Js::ScriptContext* requestContext, ActiveScriptError **ppase);
    static void FillExcepInfo(HRESULT hr, char16 const * messageSz, EXCEPINFO *pei);
    static void FillParseErrorInfo(ExtendedExceptionInfo &exInfo);
    static HRESULT CanHandleException(Js::ScriptContext * scriptContext, Js::JavascriptExceptionObject * exceptionObject, IServiceProvider * pspCaller);
    HRESULT GetCompileErrorInfo(_Out_ BSTR* description, _Out_ ULONG* line, _Out_ ULONG* column);
private:
    static HRESULT FillExcepInfo(Js::JavascriptExceptionObject * exceptionObject, EXCEPINFO *pei, ExtendedExceptionInfo *peei, Js::RestrictedErrorStrings *perrstr);
    static HRESULT FillErrorType(JsErrorType typeNumber, ExtendedExceptionInfo& extendedExcepInfo);
    static HRESULT FillText(LPCWSTR& target, LPCWSTR source);
    static void FillStackTrace(HRESULT exceptionHR, Js::JavascriptExceptionObject& exceptionObject, ExtendedExceptionInfo& pei);
    static void FreeStackTrace(CallStack& callStack);
    void CopyStackTrace(CallStack& callStackOut, CallStack& callStackIn);
    
    static void StoreErrorInfo(Js::JavascriptExceptionObject * exceptionObject, IErrorInfo ** errInfo);

    long m_cRef;

    ExtendedExceptionInfo m_ei;
    Js::RestrictedErrorStrings m_restrictedStr;
    IErrorInfo * m_errInfo;    
    ScriptDebugDocument* m_scriptDebugDocument;
    Var thrownObject;
    Recycler* recycler;
    DWORD_PTR m_dwSourceContext;
    ULONG m_ulLineNumber;
    LONG m_lCharacterPosition; // character position on line
    LONG m_ichMin;
    LONG m_ichLim;
    BSTR m_bstrSourceLine;
    BOOL m_fHasDispatchedToDebugger : 1;
    BOOL m_fIsFirstChance : 1;
    BOOL m_fIsExceptionCaughtInNonUserCode : 1;
    BOOL m_wasRooted : 1;

    CComPtr<IDebugStackFrame> m_pStackFrame;
};

class CallStackFrameHelper 
{
public:
    CallStackFrameHelper(CallStackFrame* callStackFrame, ULONG totalFrameCount, HRESULT& hr);
    ~CallStackFrameHelper();
    void AppendFrame(CallStackFrame newFrame);
    static LPCWSTR  DelimiterString;
private:
    CallStackFrame* callStackFrames;
    ULONG frameCount;
    ULONG currentFrame;
    HRESULT& hResult;
};