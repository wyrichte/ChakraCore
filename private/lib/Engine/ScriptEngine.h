//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

// Description: Declarations for OLE Scripting script engine object.

#pragma once
// Explicitly enable HeapDumper private interface in retail builds.
#define ENABLE_HEAP_DUMPER

extern const EXCEPINFO NoException;

ulong ComputeGrfscrUTF16();
ulong ComputeGrfscrUTF8();

#define IACTIVESCRIPTERROR64 (IActiveScriptError64*)

extern LPOLESTR g_pszLangName;


enum
{
    fsiHostManaged = 0x01,
    fsiScriptlet   = 0x02,
    fsiDeferredParse = 0x04
};

class CScriptBody;
class CScriptSourceDocumentText;
class ScriptDebugDocument;
class EventSink;
class BaseEventHandler;
class CDebugStackFrame;
class CJavascriptOperations;
interface IDebugSetValueCallback;

#ifdef ENABLE_PROJECTION
namespace Projection
{
    class ProjectionContext;
}
using namespace Projection;
#endif

// ---------------------------------------------------------------------------
// The ScriptEngine class.  Manages and maintains the state of the script.
// ---------------------------------------------------------------------------

class ScriptEngine :
    //
    // Active Script hosting interfaces (activscp.idl, activscp_private.idl)
    //
    public ScriptEngineBase,
    public IActiveScript,
#if !_WIN64 || USE_32_OR_64_BIT
    public IActiveScriptParse32,
    public IActiveScriptParseProcedure2_32,
    public IActiveScriptParseUTF832,
    public IActiveScriptParse232,
#endif // !_WIN64 || USE_32_OR_64_BIT
#if _WIN64 || USE_32_OR_64_BIT
    public IActiveScriptParse64,
    public IActiveScriptParseProcedure2_64,
    public IActiveScriptParseUTF864,
    public IActiveScriptParse264,
#endif // _WIN64 || USE_32_OR_64_BIT
    public IActiveScriptStats,
    public IActiveScriptProperty,
    public IActiveScriptGarbageCollector,
    public IActiveScriptLifecycleEventSink,

    //
    // Active Script Direct hosting interfaces (scriptdirect.idl)
    //
#ifdef ENABLE_PROJECTION
    public IActiveScriptProjection,
    public IPrivateScriptProjection,
#endif

    public IActiveScriptByteCode,
    public IDiagnosticsContextHelper,
    public IActiveScriptDirectAsyncCausality,

    //
    // Active Script profiling interfaces (activprof.idl)
    //
    public IActiveScriptProfilerControl5,

    public IActiveScriptDebugAttach,
    public IScriptInvocationContextSubscriber,
    //
    // Temporary private interface to provide functional equilvalent of Debug.DumpHeap (scriptdirect.idl)
    //
#ifdef ENABLE_HEAP_DUMPER
    public IHeapDumper,
#endif

#ifdef EDIT_AND_CONTINUE
    public IActiveScriptEdit,
#endif

    //
    // Active Script debugging interfaces (activdbg.idl)
    //
    public IDebugStackFrameSniffer,
#if !_WIN64 || USE_32_OR_64_BIT
    public IActiveScriptDebug32,
#endif // !_WIN64 || USE_32_OR_64_BIT
#if _WIN64 || USE_32_OR_64_BIT
    public IActiveScriptDebug64,
#endif // _WIN64 || USE_32_OR_64_BIT
    public IRemoteDebugApplicationEvents,

    //
    // Dispatch extension interfaces (dispex.idl)
    //
    public IVariantChangeType,

    //
    // Other interfaces
    //
    public IObjectSafety,
    public Js::HaltCallback,
    public Js::DebuggerOptionsCallback
{
    friend class ScriptSite;

public:
    ScriptEngine(REFIID riidLanguage, LPCOLESTR pszLanguageName);
    virtual ~ScriptEngine();

    //
    // IUnknown
    //

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) sealed;
    STDMETHOD_(ULONG,AddRef)(void) override;
    STDMETHOD_(ULONG,Release)(void) override;

    //
    // Active Script hosting interfaces
    //

    //
    // IActiveScript
    //

    STDMETHOD(SetScriptSite)(IActiveScriptSite* activeScriptSite) sealed;
    STDMETHOD(GetScriptSite)(REFIID iid, VOID** ppvSiteObject) sealed;
    STDMETHOD(SetScriptState)(SCRIPTSTATE ss) sealed;
    STDMETHOD(GetScriptState)(SCRIPTSTATE* pssState);
    STDMETHOD(Close)() sealed;
    STDMETHOD(AddNamedItem)(LPCOLESTR pstrName, DWORD dwFlags);
    STDMETHOD(AddTypeLib)(REFGUID rguidTypeLib, DWORD dwMajor, DWORD dwMinor, DWORD dwFlags);
    STDMETHOD(GetScriptDispatch)(LPCOLESTR pstrItemName, IDispatch** ppdisp);
    STDMETHOD(GetCurrentScriptThreadID)(SCRIPTTHREADID* pstidThread);
    STDMETHOD(GetScriptThreadID)(DWORD dwWin32ThreadId, SCRIPTTHREADID* pstidThread);
    STDMETHOD(GetScriptThreadState)(SCRIPTTHREADID stidThread, SCRIPTTHREADSTATE* pstsState);
    STDMETHOD(InterruptScriptThread)(SCRIPTTHREADID stidThread, const EXCEPINFO* pexcepinfo, DWORD dwFlags);
    STDMETHOD(Clone)(IActiveScript** ppscript);

    //
    // IActiveScriptParse
    //

    STDMETHOD(InitNew)() sealed;
#if !_WIN64 || USE_32_OR_64_BIT
    STDMETHOD(AddScriptlet)(THIS_
        /* [in]  */ LPCOLESTR pstrDefaultName,
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ LPCOLESTR pstrSubItemName,
        /* [in]  */ LPCOLESTR pstrEventName,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORD     dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ BSTR*     pbstrName,
        /* [out] */ EXCEPINFO* pexcepinfo
    );
    STDMETHOD(ParseScriptText)(THIS_
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown *punkContext,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORD     dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ VARIANT*  pvarResult,
        /* [out] */ EXCEPINFO* pexcepinfo
    );
#endif // !_WIN64 || USE_32_OR_64_BIT
#if _WIN64 || USE_32_OR_64_BIT
    STDMETHOD(AddScriptlet)(THIS_
        /* [in]  */ LPCOLESTR pstrDefaultName,
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ LPCOLESTR pstrSubItemName,
        /* [in]  */ LPCOLESTR pstrEventName,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORDLONG dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ BSTR*     pbstrName,
        /* [out] */ EXCEPINFO* pexcepinfo
    );
    STDMETHOD(ParseScriptText)(THIS_
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown* punkContext,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORDLONG dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ VARIANT*  pvarResult,
        /* [out] */ EXCEPINFO* pexcepinfo
    );
#endif // _WIN64 || USE_32_OR_64_BIT

    //
    // IActiveScriptParse2
    //

#if !_WIN64 || USE_32_OR_64_BIT
    STDMETHOD(ParseScriptText)(THIS_
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ DWORD     dwLength,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown *punkContext,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORD     dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ VARIANT*  pvarResult,
        /* [out] */ EXCEPINFO* pexcepinfo
    );
#endif // !_WIN64 || USE_32_OR_64_BIT
#if _WIN64 || USE_32_OR_64_BIT
    STDMETHOD(ParseScriptText)(THIS_
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ DWORD     dwLength,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown* punkContext,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORDLONG dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ VARIANT*  pvarResult,
        /* [out] */ EXCEPINFO* pexcepinfo
    );
#endif // _WIN64 || USE_32_OR_64_BIT

    //
    // IActiveScriptParseProcedure2
    //

#if !_WIN64 || USE_32_OR_64_BIT
    STDMETHOD(ParseProcedureText)(
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ LPCOLESTR pstrFormalParams,
        /* [in]  */ LPCOLESTR pstrProcedureName,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown* punkContext,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORD     dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ IDispatch** ppdisp);
#endif // !_WIN64 || USE_32_OR_64_BIT
#if _WIN64 || USE_32_OR_64_BIT
    STDMETHOD(ParseProcedureText)(
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ LPCOLESTR pstrFormalParams,
        /* [in]  */ LPCOLESTR pstrProcedureName,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown* punkContext,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORDLONG dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ IDispatch** ppdisp);
#endif // _WIN64 || USE_32_OR_64_BIT

    //
    // IActiveScriptParseUTF8
    //

#if !_WIN64 || USE_32_OR_64_BIT
    STDMETHOD(ParseScriptText)(THIS_
        /* [in]  */ BYTE *    pstrCode,
        /* [in]  */ ULONG     ulCodeOffset,
        /* [in]  */ DWORD     dwLength,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown *punkContext,
        /* [in]  */ const BYTE *pstrDelimiter,
        /* [in]  */ DWORD     dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ VARIANT*  pvarResult,
        /* [out] */ EXCEPINFO* pexcepinfo
    );
#endif // !_WIN64 || USE_32_OR_64_BIT
#if _WIN64 || USE_32_OR_64_BIT
    STDMETHOD(ParseScriptText)(THIS_
        /* [in]  */ BYTE * pstrCode,
        /* [in]  */ ULONG     ulCodeOffset,
        /* [in]  */ DWORD     dwLength,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown* punkContext,
        /* [in]  */ const BYTE * pstrDelimiter,
        /* [in]  */ DWORDLONG dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ VARIANT*  pvarResult,
        /* [out] */ EXCEPINFO* pexcepinfo
    );
#endif // _WIN64 || USE_32_OR_64_BIT

    //
    // IActiveScriptStats
    //

    STDMETHOD(GetStat)  (DWORD   stid, ULONG *pluHi, ULONG *pluLo);
    STDMETHOD(GetStatEx)(REFGUID guid, ULONG *pluHi, ULONG *pluLo);
    STDMETHOD(ResetStats)(void);

    //
    // IActiveScriptProperty
    //

    STDMETHOD(GetProperty)(DWORD dwProperty, VARIANT* pvarIndex, VARIANT* pvarValue);
    STDMETHOD(SetProperty)(DWORD dwProperty, VARIANT* pvarIndex, VARIANT* pvarValue) sealed;

    //
    // IActiveScriptGarbageCollector
    //

    STDMETHOD(CollectGarbage)(SCRIPTGCTYPE scriptgctype);

    //
    // IActiveScriptLifecycleEventSink
    //

    STDMETHOD(OnEvent)(_In_ EventId eventId, _In_ VARIANT* pvarArgs);

    //
    // Active Script Direct hosting interfaces
    //


#ifdef ENABLE_PROJECTION
    //
    // IActiveScriptProjection
    //
    STDMETHOD(SetProjectionHost)(IActiveScriptProjectionHost * host, BOOL isConfigurable, DWORD targetVersion, IDelegateWrapper* delegateWrapper);
    STDMETHOD(ReserveNamespace)(LPCWSTR name, BOOL isExtensible);

    //
    // IPrivateScriptProjection
    //
    STDMETHOD(ResetDelegateWrapper)(IDelegateWrapper* newDelegateWrapper);
#endif

    //
    // IActiveScriptByteCode
    //

    STDMETHOD(GenerateByteCodeBuffer)(THIS_
            /* [in] */ DWORD dwSourceCodeLength,
            /* [size_is][in] */ __RPC__in_ecount_full(dwSourceCodeLength) BYTE *utf8Code,
            /* [in] */ __RPC__in_opt IUnknown *punkContext,
            /* [in] */ DWORD_PTR dwSourceContext,
            /* [in] */ __RPC__in EXCEPINFO *pexcepinfo,
            /* [size_is][out] */ __RPC__deref_out_ecount_full_opt(*pdwByteCodeSize) BYTE **byteCode,
            /* [out] */ __RPC__out DWORD *pdwByteCodeSize) override;

    STDMETHOD(ExecuteByteCodeBuffer)(THIS_
            /* [in] */ DWORD dwByteCodeSize,
            /* [size_is][in] */ __RPC__in_ecount_full(dwByteCodeSize) BYTE *byteCode,
            /* [in] */ IActiveScriptByteCodeSource *pbyteCodeSource,
            /* [in] */ __RPC__in_opt IUnknown *punkContext,
            /* [in] */ DWORD_PTR dwSourceContext,
            /* [out] */ __RPC__out EXCEPINFO *pexcepinfo) override;

    //
    // IDiagnosticsContextHelper
    //

    STDMETHOD(GetFunctionName)(_In_ Var instance, _Out_ BSTR * pBstrName);
    STDMETHOD(GetFunctionContext)(_In_ Var instance, _Out_ IUnknown ** ppDebugDocumentContext);
    STDMETHOD(GetFunctionIds)(
        _In_ Var instance,
        _Out_ UINT32* pFunctionId,
        _Out_ DWORD_PTR* pScriptContextId);
    STDMETHOD(GetFunctionInfo)(
        _In_ Var instance,
        _Out_opt_ BSTR* pBstrName,
        _Out_opt_ UINT32* pLine,
        _Out_opt_ UINT32* pColumn,
        _Out_opt_ UINT32* pCchLength);


    //
    // IActiveScriptDirectAsyncCausality
    //

    STDMETHOD(TraceAsyncOperationStarting)(
        _In_ GUID* platformId,
        _In_ LPCWSTR operationName,
        _In_ AsyncCausality_LogLevel logLevel,
        _Out_ UINT64* pOperationId);

    STDMETHOD(TraceAsyncCallbackStarting)(
        _In_ GUID* platformId,
        _In_ UINT64 operationId,
        _In_ AsyncCausality_CallbackType workType,
        _In_ AsyncCausality_LogLevel logLevel);

    STDMETHOD(TraceAsyncCallbackCompleted)(
        _In_ AsyncCausality_CallbackType workType,
        _In_ AsyncCausality_LogLevel logLevel);

    STDMETHOD(UpdateAsyncCallbackRelation)(
        _In_ GUID* platformId,
        _In_ UINT64 operationId,
        _In_ AsyncCausality_RelationType relation,
        _In_ AsyncCausality_LogLevel logLevel);

    STDMETHOD(TraceAsyncOperationCompleted)(
        _In_ GUID* platformId,
        _In_ UINT64 operationId,
        _In_ AsyncCausality_OperationStatus status,
        _In_ AsyncCausality_LogLevel logLevel);

    //
    // Active Script Profiling interfaces
    //

    //
    // IActiveScriptProfilerControl
    //

    STDMETHOD(StartProfiling)(
        __RPC__in REFCLSID clsidProfilerObject,
        __in DWORD dwEventMask,
        __in DWORD dwContext);

    STDMETHOD(SetProfilerEventMask)(
        __in DWORD dwEventMask);

    STDMETHOD(StopProfiling)(
        __in HRESULT hrShutdownReason) sealed;

    // Test hooks
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    HRESULT StartScriptProfiling(__in IActiveScriptProfilerCallback * profilerObject, __in DWORD dwEventMask, __in DWORD dwContext);
#endif

    //
    // IActiveScriptProfilerControl2
    //

    STDMETHOD(CompleteProfilerStart)();

    STDMETHOD(PrepareProfilerStop)();

    //
    // IActiveScriptProfilerControl3
    //

    STDMETHODIMP EnumHeap(IActiveScriptProfilerHeapEnum** ppEnum) sealed;

#ifdef ENABLE_HEAP_DUMPER
    //
    // IHeapDumper
    //
    STDMETHODIMP DumpHeap(const WCHAR* outputFile, HeapDumperObjectToDumpFlag objectsToDump, BOOL minimalDump, BOOL dumpAllEngines);
#endif

#ifdef EDIT_AND_CONTINUE
    //
    // IActiveScriptEdit
    //
    STDMETHOD(QueryEdit)(
        /* [size_is][in] */ ScriptEditRequest *requests,
        /* [in] */ ULONG count,
        /* [out] */ IScriptEditQuery **ppQueryResult);
#endif

    //
    // IActiveScriptProfilerControl4
    //
    STDMETHODIMP SummarizeHeap(PROFILER_HEAP_SUMMARY* pHeapSummary);

    //
    // IActiveScriptProfilerControl5
    //
    STDMETHODIMP EnumHeap2(PROFILER_HEAP_ENUM_FLAGS enumFlags, IActiveScriptProfilerHeapEnum** ppEnum);

    //
    // Active Script debugging interfaces
    //

    //
    // IActiveScriptDebug
    //

    STDMETHOD(GetScriptTextAttributes)(
            /* [size_is][in] */ __RPC__in_ecount_full(uNumCodeChars) LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ __RPC__in LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ __RPC__inout_ecount_full(uNumCodeChars) SOURCE_TEXT_ATTR* pattr);

    STDMETHOD(GetScriptletTextAttributes)(
            /* [size_is][in] */ __RPC__in_ecount_full(uNumCodeChars) LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ __RPC__in LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ __RPC__inout_ecount_full(uNumCodeChars) SOURCE_TEXT_ATTR* pattr);

#if !_WIN64 || USE_32_OR_64_BIT
    STDMETHOD(EnumCodeContextsOfPosition)(
        DWORD dwSourceContext,
        ULONG uCharacterOffset,
        ULONG uNumChars,
        IEnumDebugCodeContexts** ppescc);
#endif // !_WIN64 || USE_32_OR_64_BIT
#if _WIN64 || USE_32_OR_64_BIT
    STDMETHOD(EnumCodeContextsOfPosition)(
        DWORDLONG   dwSourceContext,
        ULONG       uCharacterOffset,
        ULONG       uNumChars,
        IEnumDebugCodeContexts** ppescc);
#endif // _WIN64 || USE_32_OR_64_BIT

    //
    // IDebugStackFrameSniffer
    //

    STDMETHOD(EnumStackFrames)(IEnumDebugStackFrames** ppedsf);

    //
    // IRemoteDebugApplicationEvents
    //

    STDMETHOD(OnConnectDebugger)(IApplicationDebugger* pad);
    STDMETHOD(OnDisconnectDebugger)(void);
    STDMETHOD(OnSetName)(LPCOLESTR pstrName);
    STDMETHOD(OnDebugOutput)(LPCOLESTR pstr);
    STDMETHOD(OnClose)(void);
    STDMETHOD(OnEnterBreakPoint)(IRemoteDebugApplicationThread* prdat);
    STDMETHOD(OnLeaveBreakPoint)(IRemoteDebugApplicationThread* prdat);
    STDMETHOD(OnCreateThread)(IRemoteDebugApplicationThread* prdat);
    STDMETHOD(OnDestroyThread)(IRemoteDebugApplicationThread* prdat);
    STDMETHOD(OnBreakFlagChange)(APPBREAKFLAGS abf, IRemoteDebugApplicationThread* prdatSteppingThread);

    //
    // Dispatch extension interfaces
    //

    //
    // IVariantChangeType
    //

    STDMETHOD(ChangeType)(VARIANT* pvarDst, VARIANT* pvarSrc, LCID lcid, VARTYPE vtNew) sealed;

    //
    // Other interfaces
    //

    //
    // IObjectSafety
    //

    STDMETHOD(GetInterfaceSafetyOptions)(REFIID iid, DWORD* pdwSupportedOptions,
            DWORD* pdwEnabledOptions);
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID iid, DWORD dwOptionsSetMask,
            DWORD dwEnabledOptions);

    //
    // Js::HaltCallback
    //

    virtual bool CanHalt(Js::InterpreterHaltState* pHaltState);
    virtual void DispatchHalt(Js::InterpreterHaltState* pHaltState);
    virtual void CleanupHalt() sealed;
    virtual bool CanAllowBreakpoints();
    virtual bool IsInClosedState();

    bool IsScriptDebuggerOptionsEnabled(SCRIPT_DEBUGGER_OPTIONS flag);
    // Js::DebuggerOptionsCallback
    virtual bool IsFirstChanceExceptionEnabled();
    virtual bool IsNonUserCodeSupportEnabled();
    virtual bool IsLibraryStackFrameSupportEnabled();

    // Helper method
    static void HandleResumeAction(Js::InterpreterHaltState* pHaltState, BREAKRESUMEACTION resumeAction);
    static void RaiseMessageToDebugger(Js::ScriptContext * scriptContext, DEBUG_EVENT_INFO_TYPE messageType, LPCWSTR message, LPCWSTR url);
    static void TransitionToDebugModeIfFirstSource(Js::ScriptContext * scriptContext, Js::Utf8SourceInfo * sourceInfo);

    // *** ScriptEngine ***

    HRESULT InitializeThreadBound();
    HRESULT Initialize(ThreadContext * threadContext);
    HRESULT GetLanguageInfo(BSTR* pbstrLang, GUID* pguidLang);
    HRESULT CloseInternal();

    // Misc stubs and utils

    HRESULT CreateHeapEnum(ActiveScriptProfilerHeapEnum **ppEnum, bool preEnumHeap2, PROFILER_HEAP_ENUM_FLAGS enumFlags = PROFILER_HEAP_ENUM_FLAGS::PROFILER_HEAP_ENUM_FLAGS_NONE);

    HRESULT StartProfilingInternal(
        __in IActiveScriptProfilerCallback *pProfileCallback,
        __in DWORD dwEventMask,
        __in DWORD dwContext);
    static HRESULT ScriptEngine::CheckForExternalProfiler(IActiveScriptProfilerCallback **ppProfileCallback);

    void OnEnterScript(void);
    void OnLeaveScript(void);

    IActiveScriptDirectHost* GetActiveScriptDirectHostNoRef();
    HRESULT GetIActiveScriptSite (IActiveScriptSite** ppass);
    HRESULT GetActiveScriptSiteWindow (IActiveScriptSiteWindow** ppassw);

    HRESULT GetActiveScriptSiteUIControl(IActiveScriptSiteUIControl** ppuic);

    HRESULT OnScriptError (IActiveScriptError* pase); // Error reporting to the site
    HRESULT ReportCompilerError(SRCINFO* srcInfo, CompileScriptException* se, EXCEPINFO * pexcepinfo, Js::Utf8SourceInfo* sourceInfo);
    LCID GetUserLcid (void) { return m_lcidUser; }
    UINT GetCodePage (void) { return m_codepage; }
    BOOL IsValidCodePage (void) { return m_fIsValidCodePage; }
    virtual HRESULT STDMETHODCALLTYPE GetInterruptInfo (EXCEPINFO* pexcepinfo);

    BOOL IsPseudoDisconnected (void) { return m_fIsPseudoDisconnected; }

    BOOL SetCurrentLocale (LCID lcid);
    void ResetLocales (void);

    DWORD GetInvokeVersion() { return SCRIPTLANGUAGEVERSION_5_8; }

    // We don't do Collect at ScriptSite close time for non primary Engines. This is set from host for engines like frames
    // that does not require full cleanup at close time.
    void SetNonPrimaryEngine(bool nonPrimaryEngine) {fNonPrimaryEngine = nonPrimaryEngine; }

    DWORD GetHostType() { return hostType; }
    HRESULT GetHostContextUrl(__in DWORD_PTR hostSourceContext, __out BSTR* pUrl);
    BOOL SetCurrentCodePage (UINT codepage);
    HRESULT GetObjectOfItem(IDispatch** ppdisp, NamedItem* pnid, LPCOLESTR pszSubItem = NULL);
    NamedItem* FindNamedItem (LPCOLESTR pcszName);

    // Compile Function type
    typedef HRESULT (ScriptEngine::*CoreCompileFunction)(void * pszSrc, size_t cbLength,
        ulong grfscr, SRCINFO* srcInfo, LPCOLESTR pszTitle, CompileScriptException* pse,
        CScriptBody** ppbody, Js::ParseableFunctionInfo** ppFuncInfo, BOOL &fUsedExisting, Js::Utf8SourceInfo** pSourceInfo);
    typedef ulong (ComputeGrfscrFunction)();
    typedef HRESULT (ScriptEngine::*CompileScriptType)(void * pszSrc, size_t len, ulong grfscr,
        SRCINFO* srcInfo, LPCOLESTR pszTitle, CompileScriptException* pse,
        CScriptBody** ppbody, Js::ParseableFunctionInfo** ppFuncInfo, BOOL &fUsedExisting, Js::Utf8SourceInfo** ppSourceInfo, CoreCompileFunction fnCoreCompile);
    CompileScriptType CompileFunction;

    HRESULT CreateScriptBody(void * pszSrc, size_t len, DWORD dwFlags, bool allowDeferParse, SRCINFO* psi,
        const void *pszDelimiter, LPCOLESTR pszTitle, CoreCompileFunction fnCoreCompile,
        ComputeGrfscrFunction ComputeGrfscr, BOOL &fUsedExisting, Js::ParseableFunctionInfo** ppFuncInfo, Js::Utf8SourceInfo** ppSourceInfo, EXCEPINFO* pei = NULL, CScriptBody** ppbody = NULL);

    void RemoveEventSinks(EventSink *eventSink) { return eventSinks->Remove(eventSink);}
    ArenaAllocator* GetScriptAllocator() { return scriptAllocator;}

#ifdef ENABLE_PROJECTION
    HRESULT EnsureProjection();
    Projection::ProjectionContext* GetProjectionContext()  const {return projectionContext; }
    void ResetProjectionContext();
#endif

    Js::ScriptContext* EnsureScriptContext();
    BOOL ShouldKeepAlive() const { return fKeepEngineAlive; }

    CJavascriptOperations* jsOps;

    // code bodies that should be persisted or run
    enum
    {
        fbodNil              = 0x00,
        fbodPersist          = 0x01,
        fbodRun              = 0x02,
        fbodReturnExpression = 0x04,

        // This differs from fbodPersist in that bodies that have fbodPersist
        // survive reset's and clones. Bodies that have fbodKeep are typically
        // global code blocks that could be thrown away except for the fact
        // that we want to be able to debug them.
        fbodKeep             = 0x08,
    };

    static const unsigned int ScriptBodyMRUSize = 128;
    static const char16 *GetDispatchFunctionNameAndContext(Js::JavascriptFunction *pFunction, Js::ScriptContext **ppFunctionScriptContext);
    typedef JsUtil::Cache<Js::Utf8SourceInfo *, CScriptBody *, RecyclerNonLeafAllocator, PrimeSizePolicy, JsUtil::MRURetentionPolicy<Js::Utf8SourceInfo *, ScriptBodyMRUSize>, DefaultComparer, JsUtil::DictionaryEntry> ScriptBodyDictionary;

protected:
    // This allows us to override notification in a derived test class
    virtual void NotifyScriptStateChange(SCRIPTSTATE ss);

private:
    HRESULT SerializeByteCodes(DWORD dwSourceCodeLength, BYTE *utf8Code, IUnknown *punkContext, DWORD_PTR dwSourceContext, ComputeGrfscrFunction ComputeGrfscr, EXCEPINFO *pexcepinfo, BYTE **byteCode, DWORD *pdwByteCodeSize);
    HRESULT DeserializeByteCodes(DWORD dwByteCodeSize, BYTE *byteCode, IActiveScriptByteCodeSource* sourceProvider, IUnknown *punkContext, DWORD_PTR dwSourceContext, ComputeGrfscrFunction ComputeGrfscr, bool execute, Js::NativeModule *nativeModule, EXCEPINFO *pexcepinfo);

    HRESULT AddScriptletCore(
        /* [in]  */ LPCOLESTR pstrDefaultName,
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ LPCOLESTR pstrSubItemName,
        /* [in]  */ LPCOLESTR pstrEventName,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORD_PTR dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ BSTR*     pbstrName,
        /* [out] */ EXCEPINFO* pexcepinfo
    );

    HRESULT ParseScriptTextCore(
        /* [in]  */ void * pstrCode,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown* punkContext,
        /* [in]  */ const void * pstrDelimiter,
        /* [in]  */ DWORD_PTR dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
                    bool allowDeferredParse,
        /* [in]  */ size_t      len,
        /* [in]  */ CoreCompileFunction fnCoreCompile,
        /* [in]  */ ComputeGrfscrFunction ComputeGrfscr,
        /* [out] */ BOOL &fUsedExisting,
        /* [out] */ VARIANT*  pvarResult,
        /* [out] */ EXCEPINFO* pexcepinfo,
        /* [out] */ Js::Utf8SourceInfo** ppSourceInfo = NULL
    );

    HRESULT ParseProcedureTextCore(
        /* [in]  */ LPCOLESTR pstrCode,
        /* [in]  */ LPCOLESTR pstrFormalParams,
        /* [in]  */ LPCOLESTR pstrProcedureName,
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [in]  */ IUnknown* punkContext,
        /* [in]  */ LPCOLESTR pstrDelimiter,
        /* [in]  */ DWORD_PTR dwSourceContext,
        /* [in]  */ ULONG     ulStartingLineNumber,
        /* [in]  */ DWORD     dwFlags,
        /* [out] */ IDispatch** ppdisp,
        /* [in]  */ BOOL      fReportError);

    STDMETHOD(Run)(void);
    STDMETHOD(Stop)(void);
    STDMETHOD(Reset)(BOOL fFull) sealed;
    STDMETHOD(ResetNamedItems)(void);
    STDMETHOD(Disconnect)(void);
    STDMETHOD(PseudoDisconnect)(void);
    STDMETHOD(Reconnect)(void) sealed;

    HRESULT SinkEvents (void);
    HRESULT ExecutePendingScripts (VARIANT* pvarRes = NULL, EXCEPINFO* pei = NULL);
    HRESULT AddEventSinks(ScriptEngine* pos, IDispatch* pdisp);

    // Call ChangeScriptState ONLY in the base thread
    void ChangeScriptState (SCRIPTSTATE ss);

    HRESULT RegisterNamedItems (void);
    HRESULT RegisterNamedItem        (NamedItem* pnid);
    HRESULT RegisterNamedItemHasCode (NamedItem* pnid);
    HRESULT RegisterObject (LPCOLESTR pszName, IDispatch* pdisp, Js::ModuleID moduleID, long lwCookie);
    HRESULT AddDefaultDispatch (Js::ModuleID moduleID, IDispatch* pdisp);
    HRESULT ConnectEventHandlers    (void);
    HRESULT DisconnectEventHandlers (void);

    void DisableInterrupts (void);
    void EnableInterrupts  (void);

    void FreeEventSinks (void);

    template<class TMapFunction>
    void MapDebugDocument(TMapFunction map)
    {
        MapDebugDocumentUntil([=] (ScriptDebugDocument* document) -> bool {
            map(document);
            return false;
        });
    }

    template<class TMapFunction>
    void MapUTF8SourceInfoUntil(TMapFunction map)
    {
        this->GetScriptContext()->GetSourceList()->MapUntil([=](int i, RecyclerWeakReference<Js::Utf8SourceInfo>* sourceInfoWeakRef) -> bool {
            Js::Utf8SourceInfo* sourceInfo = sourceInfoWeakRef->Get();
            if(sourceInfo)
            {
                return map(sourceInfo);
            }
            return false;
        });
    }

    template<class TMapFunction>
    void MapDebugDocumentUntil(TMapFunction map)
    {
        MapUTF8SourceInfoUntil([=] (Js::Utf8SourceInfo * utf8SourceInfo) -> bool {
            if (utf8SourceInfo->HasDebugDocument())
            {
                ScriptDebugDocument* document = static_cast<ScriptDebugDocument*>(utf8SourceInfo->GetDebugDocument());
                return map(document);
            }
            return false;
        });
    }

    void SetEntryPointsForRestrictedEval();

#ifdef EDIT_AND_CONTINUE
public:
    template <class Fn>
    HRESULT DispatchOnApplicationThread(const Fn& fn)
    {
        auto runOnApplicationThread = [&]() -> HRESULT
        {
            HRESULT hr = VerifyOnEntry();
            if (SUCCEEDED(hr))
            {
                hr = fn();
            }
            return hr;
        };

        CComPtr<IDebugBitCorrectApplicationThread> spAppThread;
        HRESULT hr = this->GetBitCorrectApplicationThread(&spAppThread);
        if (SUCCEEDED(hr))
        {
            hr = EditAndContinue::Dispatch(spAppThread, runOnApplicationThread);
        }
        else
        {
            // Otherwise we can't dispatch (maybe not debugging). But we may be on application thread
            // already. runOnApplicationThread() will VerifyOnEntry().
            hr = runOnApplicationThread();
        }

        return hr;
    }

private:
#endif

    // Queries the host to see if it is set to debug mode or not.
    BOOL IsHostInDebugMode();
    void CheckHostInDebugMode();

    HRESULT SinkEventsOfNamedItems (long ipehMin = 0);
    HRESULT RegisterNamedEventHandler (
        __in NamedItem* pnid,
        __in_opt LPCOLESTR pszSubItem,
        __in LPCOLESTR pszEvt,
        DWORD dwFlags,
        __in CScriptBody* pbody);

    HRESULT DefaultCompile(
        __in void * pszSrc,
        __in size_t len,
        __in ulong grfscr,
        __in SRCINFO* srcInfo,
        __in LPCOLESTR pszTitle,
        __in_opt CompileScriptException* pse,
        __out CScriptBody** ppbody,
        __out Js::ParseableFunctionInfo** ppFuncInfo,
        __out BOOL &fUsedExisting,
        __out Js::Utf8SourceInfo** ppSourceInfo,
        __in CoreCompileFunction fnCoreCompile);

    HRESULT ProfileModeCompile(
        __in void * pszSrc,
        __in size_t len,
        __in ulong grfscr,
        __in SRCINFO* srcInfo,
        __in LPCOLESTR pszTitle,
        __in_opt CompileScriptException* pse,
        __out CScriptBody** ppbody,
        __out Js::ParseableFunctionInfo** ppFuncInfo,
        __out BOOL &fUsedExisting,
        __out Js::Utf8SourceInfo** ppSourceInfo,
        __in CoreCompileFunction fnCoreCompile);

    HRESULT CompileByteCodeBuffer(
        __in void * pszSrc,
        __in size_t cbLength,
        __in ulong grfscr,
        __in SRCINFO* srcInfo,
        __in LPCOLESTR pszTitle,
        __in_opt CompileScriptException* pse,
        __out CScriptBody** ppbody,
        __out Js::ParseableFunctionInfo** ppFuncInfo,
        __out BOOL &fUsedExisting,
        __out Js::Utf8SourceInfo** ppSourceInfo = NULL);

    HRESULT CompileUTF16(
        __in void * pszSrc,
        __in size_t cbLength,
        __in ulong grfscr,
        __in SRCINFO* srcInfo,
        __in LPCOLESTR pszTitle,
        __in_opt CompileScriptException* pse,
        __out CScriptBody** ppbody,
        __out Js::ParseableFunctionInfo** ppFuncInfo,
        __out BOOL &fUsedExisting,
        __out Js::Utf8SourceInfo** ppSourceInfo = NULL);

    HRESULT CompileUTF8(
        __in void *pszSrc,
        __in size_t cbLength,
        __in ulong grfscr,
        __in SRCINFO *srcInfo,
        __in LPCOLESTR pszTitle,
        __in_opt CompileScriptException *pse,
        __out CScriptBody **ppbody,
        __out Js::ParseableFunctionInfo** ppFuncInfo,
        __out BOOL &fUsedExisting,
        __out Js::Utf8SourceInfo** ppSourceInfo = NULL);

    HRESULT CompileUTF8Core(
        __in Js::Utf8SourceInfo* sourceInfo,
        __in charcount_t cchLength,
        __in ulong grfscr,
        __in SRCINFO *srcInfo,
        __in LPCOLESTR pszTitle,
        __in BOOL fOriginalUTF8Code,
        __in_opt CompileScriptException *pse,
        __out CScriptBody **ppbody,
        __out Js::ParseableFunctionInfo** ppFuncInfo,
        __out BOOL &fUsedExisting);

    HRESULT GetUrl(__out BSTR *pUrl);


    // This will be called from the OnBreakFlagChange and SetupNewDebugApplication.
    HRESULT SetBreakFlagChange(APPBREAKFLAGS abf, IRemoteDebugApplicationThread* prdatSteppingThread, bool fDuringSetupDebugApp);

    ArenaAllocator*   scriptAllocator;

#ifdef ENABLE_PROJECTION
    // Used for win8 ABI stuff.
    Projection::ProjectionContext* projectionContext;
#endif

    long             m_lwCookieCount;
    SCRIPTSTATE      m_ssState;             // Current script state
    IActiveScriptSite* m_pActiveScriptSite;             // owning IActiveScriptSite (AddRefed)
    IActiveScriptDirectHost* m_activeScriptDirectHost;

    // Default: 1 poll per second
    static const DWORD DefaultTicksPerPoll = 1000;
    DWORD            m_dwTicksPerPoll;


    LPCOLESTR        m_pszLanguageName;     // name of language
    REFIID           m_riidLanguage;        // iid of language
    LCID             m_lcidUser;            // Local identifier
    LCID             m_lcidUserDefault;

    BOOL  m_fIsValidCodePage;
    UINT  m_codepage;              // Code page

    DWORD m_dwBaseThread;          // Win32 thread we were created in
    BOOL  m_fPersistLoaded;        // TRUE if IPersist*::Load() or InitNew() has completed
    BOOL  m_fIsPseudoDisconnected; // TRUE if we are in a pseudo-disconnected state

    // This will be set to TRUE when we are about to clear debug documents, during in CloseInternal.
    // This will be used in EnumCodeContextsOfPosition to bail out early instead of facing hang when trying to acquire critical section.
    BOOL  m_fClearingDebugDocuments;

    DWORD m_remoteDbgThreadId; // This is thread on which the last stepping was done. This will be used in cross context stepping

    // Note: We currently support only one active script thread on the base thread
    SCRIPTTHREADSTATE   m_stsThreadState;     // State of the script thread
    UINT                m_cNesting;           // Count of times nested in script
                                            //   (0 = not in a script)
    Js::ModuleID        m_moduleIDNext;            // Next module number to use
    NamedItemList       m_NamedItemList;

    CRITICAL_SECTION    m_csInterrupt;        // Mutex to temporarily disable "interrupts"
    EXCEPINFO           m_excepinfoInterrupt; // If interrupt raised, exception information
#if defined(USED_IN_STATIC_LIB)
    public:
#endif
    struct BOD
    {
        ulong           grfbod;
        Js::ModuleID    moduleID;
        CScriptBody*    pbody;
    };
    GL* m_pglbod;
    BOOL                m_fIsEvalRestrict;
    BOOL                m_fAllowWinRTConstructor;

    // Event handlers.

    JsUtil::List<BaseEventHandler*, ArenaAllocator>* eventHandlers;

    JsUtil::List<EventSink*, ArenaAllocator>* eventSinks;

    ScriptBodyDictionary* scriptBodyMap;
    void RemoveScriptBodyMap();

    // This will contain the list of CDebugStackFrame. The items will be populated during
    // halt break by CEnumDebugStackFrame.
    JsUtil::List<CDebugStackFrame *, HeapAllocator> * debugStackFrame;

#ifdef EDIT_AND_CONTINUE
    CComPtr<IActiveScriptEdit> m_scriptEdit; // Edit and Continue service. Only one instance used per script engine. Created lazily.
#endif

public:

    // internal debugger methods
    // Conventions:
    //   Methods that do not addref return object pointers are of the form
    //     Get..NoRef.
    //   Methods that should be called only when debugging is enabled are
    //     prefixed with Dbg.

    BOOL IsDebuggerEnvironmentAvailable(bool requery = false);
    HRESULT TransitionToDebugModeIfFirstSource(Js::Utf8SourceInfo* utf8SourceInfo);
    bool CanRegisterDebugSources();
    HRESULT DbgHandleBreakpoint(BREAKREASON br, BREAKRESUMEACTION* pBra);
    HRESULT DbgRegisterScriptBlock(CScriptBody* pbody, DWORD_PTR dwDebugSourceContext, LPCWSTR title = NULL);
    HRESULT DbgRegisterScriptBlock(CScriptBody* pbody);
    HRESULT DbgRegisterFunction(Js::ScriptContext * scriptContext, Js::FunctionBody * functionBody, DWORD_PTR dwDebugSourceContext, LPCWSTR title);
    HRESULT DbgGetRootApplicationNode(IDebugApplicationNode** ppdan);
    HRESULT GetDebugSiteNoRef(IActiveScriptSiteDebug** pscriptSiteDebug);
    HRESULT GetDebugSiteCoreNoRef(IActiveScriptSiteDebug** pscriptSiteDebug);
    HRESULT GetDebugApplicationCoreNoRef(IDebugApplication** ppda);
    HRESULT GetDefaultDebugApplication(IDebugApplication **ppda);
    HRESULT SetupNewDebugApplication (void);
    HRESULT NonDbgGetSourceDocumentFromHostContext(Js::FunctionBody *pFunctionBody, CScriptSourceDocumentText **ppdoc);
    HRESULT GetDocumentContextFromPosition(
        DWORD_PTR                   dwSourceContext,
        ULONG                       uCharacterOffset,
        ULONG                       uNumChars,
        IDebugDocumentContext**     ppsc
        );

    IDebugApplication * GetDebugApplication() const
    {
        return m_pda;
    }

    void RegisterDebugDocument(CScriptBody *pBody, const char16 * title, DWORD_PTR dwDebugSourceContext);
    ScriptDebugDocument * FindDebugDocument(SourceContextInfo * pInfo);

    HRESULT CleanupDocumentContextListInternal(Js::ScriptContext *pScriptContext);

    HRESULT DbgStepOutComplete(void);

    HRESULT DebugDocMarkForClose();

    HRESULT SetThreadDescription(__in LPCWSTR url);

    BOOL NamedBPEnter(void) { return m_NBPmutx.Enter(); }
    void NamedBPLeave(void) { m_NBPmutx.Leave(); }

    SourceContextInfo * GetSourceContextInfo(DWORD_PTR hostSourceContext, uint hash, BOOL isDynamicDocument, BSTR sourceMapUrl, IActiveScriptDataCache* profileDataCache);

    DWORD DwResetGeneration (void) { return m_dwResetGeneration; }
    HRESULT DbgCreateBrowserFromCodeContext (
        IDebugCodeContext*  pcc,
        LPCOLESTR           pstrName,
        IDebugProperty**    ppdp
        );
    HRESULT DbgCreateBrowserFromError (
        IActiveScriptError*      pase,
        LPCOLESTR                pstrName,
        IDebugProperty**         ppdp
        );
    HRESULT DbgCreateBrowserFromProperty (
        VARIANT*                 pvar,
        IDebugSetValueCallback*  psetvalue,
        LPCOLESTR                pstrName,
        IDebugProperty**         ppdp
        );

    // IActiveScriptDebugAttach
    STDMETHOD(PerformSourceRundown)(__in ULONG pairCount, /* [size_is][in] */__RPC__in_ecount_full(pairCount) SourceContextPair *pSourceContextPairs);
    STDMETHOD(OnDebuggerAttached) (__in ULONG pairCount, /* [size_is][in] */__RPC__in_ecount_full(pairCount) SourceContextPair *pSourceContextPairs) sealed;
    STDMETHOD(OnDebuggerDetached)(void);

    // IScriptInvocationContextSubscriber
    STDMETHOD(PushInvocationContext)(__in SCRIPT_INVOCATION_CONTEXT_TYPE contextType,
                                     __in_opt IUnknown * pContextObject,
                                     __in_opt LPCWSTR contextDescription,
                                     __out DWORD * pCookie);
    STDMETHOD(PopInvocationContext)(__in DWORD cookie);

    static HRESULT GetDebugDocumentContextFromHostPosition(
        Js::FunctionBody            *pFunctionBody,
        IDebugDocumentContext       **ppDebugDocumentContext
        );

    static HRESULT CleanupDocumentContextList(Js::ScriptContext *pScriptContext);

    void CleanScriptBodyMap();

protected:
    void ResetDebugger();
    HRESULT EnsureBrowserMembers(void);
    static void DbgSetAllowUserToRecoverTab(IActiveScriptSite* scriptSite, bool fAllowUserToRecoverTab);
    // end internal debugger methods
    // debugger fields
private:
    IActiveScriptSiteDebug*         m_scriptSiteDebug;
    CScriptSourceDocumentText*      m_pNonDebugDocFirst; // Linked list of doclets
    IDebugApplication*              m_pda;
    IDebugApplicationThread*        m_debugApplicationThread;
    IRemoteDebugApplication110*     m_debugApp110;
    IDebugHelper*                   m_debugHelper;
    IDebugFormatter*                m_debugFormatter;
    IConnectionPoint*               m_pcpAppEvents;    // Connection point to application
    DWORD                           m_dwAppAdviseID;   // Advise ID for application
    DWORD                           m_dwSnifferCookie;
    DWORD_PTR                       m_dwExprContextProviderCookie;

    bool m_fDumbHost                 : 1;
    bool m_fStackFrameSnifferAdded   : 1;

    bool m_fExprContextProviderAdded : 1;
    bool m_isHostInDebugMode         : 1;   // Exepcted to be true if host returns IActiveSscriptSiteDebugHelper::IsInDebugMode to be true

    // This will be used to track the first call to CompileUTF8Core as we can only transition to debugger state on first call and not in between
    bool m_isFirstSourceCompile      : 1;

    // Set to true when the Diagnostics holder mentions that current engine belongs to Diagnostics OM.
    bool m_isDiagnosticsOM : 1;

    // This mutex guards the member variables of ScriptEngine that may be
    // accessed from both the app thread and the debugger thread. Currently
    // the only operations that run on the debugger thread that need to
    // lock this is the code for named breakpoints in debugger\namedbp.cpp.
    MUTX                        m_NBPmutx;

    // If the script engine is ever reset, then any objects in the debugger
    // thread must become invalid. When objects are created, they record the
    // current value of m_dwResetGeneration. If they find that their
    // generation doesn't match the current generation they zombie themselves.
    DWORD                       m_dwResetGeneration;

    // end debugger fields
public:

    HRESULT GetDebugStackFrame(JsUtil::List<CDebugStackFrame *, HeapAllocator> ** ppdbgFrame);
public:
    HRESULT GetApplicationThread(IDebugApplicationThread** ppdat);
    HRESULT GetBitCorrectApplicationThread(IDebugBitCorrectApplicationThread** ppdat);

    HRESULT STDMETHODCALLTYPE ParseInternal(
        __in LPWSTR scriptText,
        __out Var *scriptFunc,
        __in_opt LoadScriptFlag* pLoadScriptFlag);

    HRESULT STDMETHODCALLTYPE GetJavascriptOperationsInternal(
        __out IJavascriptOperations **operations);

    HRESULT STDMETHODCALLTYPE InspectableUnknownToVarInternal(
        __in IUnknown* unknown,
        __out Var* instance);

//
protected:
    // SHA-256 hash length
    void ResetSecurity(void);
    HRESULT SetObjectSafety(IObjectSafety* psafe, REFIID riid, DWORD dwMask, DWORD dwSet);
    HRESULT GetSiteHostSecurityManagerNoRef(IInternetHostSecurityManager** ppsecman);
    HRESULT GetHostSecurityManager(IInternetHostSecurityManager** ppsecman);
    HRESULT GetINETSecurityManagerNoRef(IInternetSecurityManager** ppsecman);
    BOOL IsObjectSafeForScripting(REFCLSID clsid, IUnknown *punk);

//
private:
     DWORD                           m_dwSafetyOptions;
     BOOL                            m_fNoHostSecurityManager;
     BOOL                            m_fNoINETSecurityManager;
     IInternetHostSecurityManager*   m_psitehostsecman;
     IInternetSecurityManager*       m_pinetsecman;
//    void*                           m_pvLastReportedScriptBody;
//
//

    // This is used for telemetry purposes so that we can Join the data using below
    // activity ID and get more info like URL etc.
    GUID                            m_activityID;

public:
    GUID GetActivityId() const { return m_activityID; }
    HRESULT STDMETHODCALLTYPE SetActivityId(__in const GUID* activityId);
    HRESULT STDMETHODCALLTYPE SetTridentLoadAddress(__in void* loadAddress);
    HRESULT STDMETHODCALLTYPE SetJITConnectionInfo(__in HANDLE jitProcHandle, __in_opt void* serverSecurityDescriptor, __in UUID connectionId);
    HRESULT STDMETHODCALLTYPE SetJITInfoForScript();

private:
    // flags and values pased by the host via  IActiveScriptProperty
    DWORD                       hostType;       // One of enum SCRIPTHOSTTYPE values.
    DWORD                       webWorkerID;
    bool                        fCanOptimizeGlobalLookup : 1; //free to optimize globals when true
    bool                        fNonPrimaryEngine : 1; //is this engine an iframe or non-top level navigation engine
    bool                        fKeepEngineAlive: 1;  // keep the host alive if there is outstanding reference to IDispatch.
    bool                        fSetThreadDescription: 1;  // Used for setting the thread description once.

    // ToDo (SaAgarwa): Temporarily holds the pairCount and pSourceContextPairs passed to ScriptEngine::OnDebuggerAttached, Move to a seperate debug class specific to ScriptEngine
    ULONG pairCount;
    SourceContextPair *pSourceContextPairs;
public:
    ULONG GetSourceContextPairCount() const { return this->pairCount; }
    SourceContextPair * GetSourceContextPairs() const { return this->pSourceContextPairs; }
};

inline HRESULT ScriptEngine::DbgStepOutComplete(void)
{
    Assert(IsDebuggerEnvironmentAvailable());
    return m_pda->StepOutComplete();
}

class SimpleSourceMapper : public IActiveScriptByteCodeSource
{
private:
    BYTE* sourceCode;
    DWORD sourceCodeSize;
    ULONG currentRefCount;

public:
    SimpleSourceMapper(BYTE* sourceCode, DWORD sourceCodeSize)
        : sourceCode(sourceCode),
        sourceCodeSize(sourceCodeSize),
        currentRefCount(0)
    {

    }

    virtual HRESULT STDMETHODCALLTYPE MapSourceCode(
            /* [size_is][size_is][out] */ BYTE **sourceCode,
            /* [out] */ DWORD *pdwSourceCodeSize) override
    {
         *sourceCode = this->sourceCode;
         *pdwSourceCodeSize = this->sourceCodeSize;
         return S_OK;
    }

    virtual void STDMETHODCALLTYPE UnmapSourceCode( void) override
    {
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
    {
        *ppvObject = (IActiveScriptByteCodeSource *)this;
        return S_OK;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef( void) override
    {
        return ++currentRefCount;
    }

    virtual ULONG STDMETHODCALLTYPE Release( void) override
    {
        if(currentRefCount == 1)
        {
            // Destroy self as the last ref was released
            delete this;
            return 0;
        }
        return --currentRefCount;
    }
};


