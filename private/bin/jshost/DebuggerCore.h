//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once
class AutoDebugPropertyInfo;
class ScriptDebugNodeSource;

struct FrameDescriptor : public DebugStackFrameDescriptor
{
    FrameDescriptor();
    FrameDescriptor(DebugStackFrameDescriptor* src);
    ~FrameDescriptor();
};

typedef struct tagSourceLocationInfo 
{
    tagSourceLocationInfo()
        : scriptId((ULONG)-1), lineNumber(0), columnNumber(0), charPosition(0), contextCount(0)
    {}

    ULONG scriptId;
    ULONG lineNumber;
    ULONG columnNumber;
    ULONG charPosition;
    ULONG contextCount;
} SourceLocationInfo;

typedef struct tagBpInfo
{
    tagBpInfo() 
        : breakpointId((ULONG)-1), hitCount(0), bpState(BREAKPOINT_DISABLED)
    {}

    ULONG breakpointId;
    ULONG hitCount;
    BREAKPOINT_STATE bpState;
    SourceLocationInfo sourceLocation;
} BpInfo;


struct ExpressionEvalRequest
{
    Debugger* debugger;
    int expandLevel; 
    DebugPropertyFlags flags;
    std::wstring json;
};

class CExprCallback : public IDebugExpressionCallBack
    
{
public:
    typedef void(*CompletionDelegate)(IDebugExpression* debugExpression, const ExpressionEvalRequest& request);

    CExprCallback()
    {
        refCount = 0;
        m_hCompletionEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL );
        m_completionDelegate = nullptr;
        AddRef();
    }

    ~CExprCallback()
    {
        ::CloseHandle(m_hCompletionEvent);
    }

    void WaitForCompletion(void);


    void RegisterCompletionHandler(CompletionDelegate completionDelegate, IDebugExpression* debugExpression)
    {
        Assert(m_completionDelegate == nullptr);
        Assert(completionDelegate != nullptr);
        Assert(debugExpression != nullptr);
        
        m_debugExpression.Attach(debugExpression);
        m_completionDelegate = completionDelegate;
    }

    void SetExpressionEvalRequest(const ExpressionEvalRequest& request)
    {
        m_expressionEvalRequest = request;
    }

    void Complete()
    {
        WaitForCompletion();
        m_completionDelegate(m_debugExpression, m_expressionEvalRequest);
    }


public:
    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        if (riid == _uuidof(IUnknown))
        {
            *ppvObject =  static_cast<IUnknown*>(static_cast<IDebugExpressionCallBack*>(this));
        }
        else if (riid == _uuidof(IDebugExpressionCallBack))
        {
            *ppvObject =  static_cast<IDebugExpressionCallBack*>(this);
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
       return InterlockedIncrement(&refCount);
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        long currentCount = (long)InterlockedDecrement(&refCount);
        if (currentCount == 0)
        {
            delete this;
        }
        return (ULONG)currentCount;
    }

    // IDebugExpressionCallBack
    STDMETHOD(onComplete)(void)
    {
        ::SetEvent(m_hCompletionEvent);
        return S_OK;
    }

private:
    HANDLE m_hCompletionEvent;
    ULONG refCount;
    CComPtr<IDebugExpression> m_debugExpression;
    CompletionDelegate m_completionDelegate;
    ExpressionEvalRequest m_expressionEvalRequest;
};

class Debugger : public IApplicationDebugger
{
public:
    Debugger(void);
    ~Debugger(void);
    void Dispose();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

    STDMETHOD(QueryAlive)(void);

    STDMETHOD(CreateInstanceAtDebugger)( 
        __in REFCLSID rclsid,
        __in IUnknown __RPC_FAR *pUnkOuter,
        __in DWORD dwClsContext,
        __in REFIID riid,
        __out IUnknown __RPC_FAR *__RPC_FAR *ppvObject);

    STDMETHOD(onDebugOutput)( 
        __in LPCOLESTR pstr);

    STDMETHOD(onHandleBreakPoint)( 
        __in IRemoteDebugApplicationThread __RPC_FAR *prpt,
        __in BREAKREASON br,
        __in IActiveScriptErrorDebug __RPC_FAR *pError);

    STDMETHOD(onClose)(void);

    STDMETHOD(onDebuggerEvent)( 
        __in REFIID riid,
        __in IUnknown __RPC_FAR *punk);

    HRESULT SetDebugApp(IRemoteDebugApplication * pDebugApp);
    HRESULT DetachFromTarget();

    HRESULT Disconnect();
    HRESULT QuitThread();

    HRESULT ResumeFromBreakPoint(BREAKRESUMEACTION resumeAction, ERRORRESUMEACTION errorAction);

    // Will ensure to create root debug node.
    HRESULT FetchSources();

    HRESULT GetCurrentBreakpoint(BpInfo **bp);
    HRESULT PerformOnBreak();
    HRESULT HandleAutomaticBreakpointLogic();
    

    // Inspection and dumping operations.
    HRESULT InspectLocals();
    HRESULT DumpBreakpoint();
    HRESULT DumpSourceList();
    HRESULT GetLocalsEnum(CComPtr<IDebugProperty>& pDebugProperty);
    HRESULT GetLocalsEnum(CComPtr<IEnumDebugPropertyInfo>& enumLocals);
    HRESULT GetLocals(int expandLevel, DebugPropertyFlags flags);
    HRESULT GetCallstack(LocationToStringFlags flags = LTSF_None);
    HRESULT GetLocation(IDebugCodeContext* codeContext, Location& location );
    HRESULT GetLocation(FrameDescriptor& frameDescriptor, Location& location );
    HRESULT GetNextFrameLocation(IEnumDebugStackFrames* enumFrames, Location& loc);
    HRESULT EvaluateExpression(std::wstring expression, int expandLevel, DebugPropertyFlags flags, std::wstring& varsEvent);
    HRESULT EvaluateExpressionAsync(std::wstring expression, int expandLevel, DebugPropertyFlags flags, std::wstring& varsEvent, JsValueRef* completion);
    HRESULT EvaluateExpressionAsync(const std::wstring& expression, CExprCallback* callback);
    HRESULT EvaluateExpressionAsDebugProperty(const std::wstring& expression, DebugPropertyFlags flags, DebugPropertyInfo& debugPropertyInfo);
    HRESULT EnsureFullNameEvaluationValueIsEquivalent(const DebugPropertyInfo& debugPropertyInfo, DebugPropertyFlags flags);

    // Sets the current stack frame that is active for inspection and expression evaluation
    HRESULT EnsureCurrentFrame();
    void ClearCurrentFrame();
    HRESULT SetCurrentFrame(ULONG depth);
    HRESULT LogJson(LPCWSTR logString);
    HRESULT LogJson(__in __nullterminated wchar_t *msg, ...);

    HRESULT AddNode(__in IDebugApplicationNode *pRootNode, ULONG ulContainerId);
    HRESULT RemoveNode(__in IDebugApplicationNode *pRootNode);
    HRESULT SetDebuggerOptions(SCRIPT_DEBUGGER_OPTIONS mask, BOOL value);

    HRESULT PopulateSourceLocationInfo(IDebugCodeContext *pDebugCodeContext, SourceLocationInfo *pSourceContext);
    ScriptDebugNodeSource * FindSourceNode(ULONG ulSourceId);
    ScriptDebugNodeSource * FindSourceNode(IDebugDocumentText *pDebugDocumentText);

    HRESULT GetBreakpointHelpers(ScriptDebugNodeSource * pNode,
        ULONG ulLineNumber,
        ULONG ulColumnNumber,
        ULONG ulCharPosCount,
        SourceLocationInfo *pSourceContext,
        IDebugCodeContext **ppDebugCodeContext);

    HRESULT InsertBreakpoint(ScriptDebugNodeSource * pNode, ULONG ulLineNumber, ULONG ulColumnNumber, BREAKPOINT_STATE bpState, BpInfo * pCommittedBpInfo);
    HRESULT InsertBreakpoint(ULONG ulSourceId, ULONG ulLineNumber, ULONG ulColumnNumber, BREAKPOINT_STATE bpSTate, BpInfo * pCommittedBpInfo);

    HRESULT ModifyBreakpoint(ULONG bpId, BREAKPOINT_STATE state);

    HRESULT SetNextStatement(ULONG line, ULONG column);

    HRESULT RemoveBreakpoint(ULONG bpId);

    HRESULT GetSourceContextLocation(ULONG scriptId, ULONG lineNumber, ULONG columnNumber, BSTR* locationText);

    HRESULT GetDebugCodeContext(SourceLocationInfo *pSourceContext, IDebugCodeContext **ppDebugCodeContext, bool useLineColumn = false);
    HRESULT OnInsertText(ScriptDebugNodeSource * pNode);

    void InsertAutoBreakpoints();
    HRESULT InsertTargetedBreakpoints();

    bool IsAttached() const;
    bool IsAtBreak() const;

    static DWORD WINAPI MainLoop(LPVOID args);
    static DWORD WINAPI CauseBreakLoop(LPVOID args);

    static HRESULT StartDebuggerThread(Debugger **ppDebugger, IRemoteDebugApplication *pRemoteDebugApp);

    void SetCanSetBreakpoints(bool canSet) { this->m_canSetBreakpoints = canSet; }
    bool CanSetBreakpoints() const { return this->m_canSetBreakpoints; }
    void OnDebuggerAttachedCompleted();
    void OnDebuggerDetachedCompleted();
    HRESULT ResetBpMap();
    DebuggerController* GetController() { return m_pController; }

    // Object mutation breakpoint
    HRESULT SetMutationBreakpoint(const std::vector<std::wstring>& names, bool setOnObject, MutationType type, std::wstring strId);

    //
    // Helpers to support DumpDebugProperty
    //
    typedef ::AutoDebugPropertyInfo DebugProperty;
    typedef ::AutoDebugPropertyInfo AutoDebugPropertyInfo;

    const ControllerConfig& GetControllerConfig() const { return m_config; }
    bool IsHybridDebugger() const { return false; }

    template <class Func>
    HRESULT MapPropertyInfo(const DebugProperty& debugProperty, DebugPropertyFlags flags, const Func& func)
    {
        HRESULT hr = S_OK;

        if (debugProperty.HasFullName())
        {
            IfFailGo(EnsureFullNameEvaluationValueIsEquivalent(debugProperty, flags));
        }

        IfFailGo(func(debugProperty));

    Error:
        return hr;
    }

    template <class Func>
    HRESULT EnumDebugProperties(const DebugProperty& debugProperty, DebugPropertyFlags flags, const Func& func)
    {
        HRESULT hr = S_OK;

        CComPtr<IEnumDebugPropertyInfo> enumProps;
        IfFailGo(debugProperty.m_pDebugProp->EnumMembers(DBGPROP_INFO_ALL, DebuggerController::GetRadix(flags), __uuidof(IEnumDebugPropertyInfo), &enumProps));
        if (!enumProps) return S_OK; // No properties.

        ULONG count = 0;
        IfFailGo(enumProps->GetCount(&count));
        for (ULONG i = 0; i < count; i++)
        {
            DebugProperty member;
            ULONG ct;
            IfFailGo(enumProps->Next(1, &member, &ct));
            if (ct == 0) break;

            IfFailGo(func(member));
        }

    Error:
        return hr;
    }

private:

    void AddSourceForBreakpoint(ScriptDebugNodeSource *node);

    bool CurrentNodeIsMainScriptFile() { return m_listScriptNodes.size() == 2; }

    HRESULT RemoveAllBreakpoint();
    void RemoveAllSourceNodes();
    void RemoveAllMutationBreakpoint();

    void EnsureRootNode();
    BpInfo * FindMatchingBp(SourceLocationInfo * pSrcLocation);

    bool IsTestHarnessFile(ScriptDebugNodeSource *pNode);

    ULONG CreateUniqueSourceNumber() { return m_ulSourceNumber++; }
    ULONG CreateUniqueBpNumber() { return m_ulBpNumber++; }

    // Object mutation breakpoint
    bool m_exhaustiveSearch;
    std::map<std::wstring, IMutationBreakpoint *> m_mutationBreakpointMap;
    IDebugProperty * FindDebugProperty(IEnumDebugPropertyInfo *enumLocals, const std::vector<std::wstring>& names, int pos);
    bool HasMutationBreakpointWithId(std::wstring strId);
    HRESULT InsertMutationBreakpoint(std::wstring strId, IMutationBreakpoint *mutationBreakpoint);
    HRESULT DeleteMutationBreakpoint(std::wstring strId);
    std::wstring DumpSourceListInternal(CComPtr<IDebugApplicationNode> pRootNode, bool rootNode = false);

    // Javascript callbacks
    static JsValueRef CALLBACK JsInsertBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsResumeFromBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsDumpLocals(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsDumpCallstack(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsSetNextStatement(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsEvaluateExpression(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsEvaluateExpressionAsync(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsModifyBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsSetFrame(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsLogJson(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsSetExceptionResume(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsSetDebuggerOptions(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsEvaluateExpressionCompletion(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsRecordEdit(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsDumpBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsSetMutationBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsDeleteMutationBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsDumpSourceList(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    
    static JsValueRef JsEvaluateExpression_Internal(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, bool isAsync = false);

    // Hybrid debugging support
    static void RaiseScriptDebugEvent(ScriptDebugEvent& scriptDebugEvent);

    inline void AssignTargetString(LPCOLESTR szString, TARGET_STRING* pTargetString)
    {
        Assert(pTargetString);
        pTargetString->Address = (UINT64)(szString);
        if (szString == NULL)
            pTargetString->Length = 0;
        else
            pTargetString->Length = (DWORD)wcslen(szString);
    }

    static HRESULT ArgToULONG(JsValueRef arg, ULONG* pValue);
    static HRESULT ArgToString(JsValueRef arg, PCWSTR* pValue, size_t* pLength);

private:
    long                                    m_refCount;
    CComPtr<IRemoteDebugApplication>        m_spDebugApp; // Application that debugger is attached to
    CComPtr<IRemoteDebugApplicationThread>  m_spScriptThread; // thread corresponding to the break point
    CComPtr<IActiveScriptErrorDebug>        m_spCurrentError; // Error corresponding to the break
    bool                                    m_isPaused;
    bool                                    m_isRootNodeAdded;
    bool                                    m_isDetached;
    bool                                    m_isShutdown;
    BREAKRESUMEACTION                       m_eLastBreakAction;
    ERRORRESUMEACTION                       m_eLastErrorAction;
    ERRORRESUMEACTION                       m_defaultErrorAction;
    BREAKREASON                             m_eCurrentBreakReason;
    bool                                    m_canSetBreakpoints; // Need to defer breakpoint setting when not attached in debug mode.
    
    static DWORD                            s_scriptThreadId;

    // The controller part
    ControllerConfig                        m_config;

    std::vector<ScriptDebugNodeSource *>    m_listScriptNodes;
    std::vector<BpInfo *>                   m_listBps;

    ULONG                                   m_ulSourceNumber;
    ULONG                                   m_ulBpNumber;
    std::wstring                            m_filename;

    Message<Debugger>                       m_message;
    DebuggerController*                     m_pController;

    FrameDescriptor*                        m_currentFrame;
};

//
// Auto release resources contained in DebugPropertyInfo.
//
class AutoDebugPropertyInfo :
    public DebugPropertyInfo
{
private:
    static const CComBSTR s_emptyBSTR;

    static void ReleaseMember(BSTR s)
    {
        if (s != s_emptyBSTR.m_str)
        {
            ::SysFreeString(s);
        }
    }
public:
    AutoDebugPropertyInfo()
    {
        m_dwValidFields = 0;

        // Initialize all BSTR members to empty BSTR. There are too many usages without null check.
        m_bstrName = s_emptyBSTR;
        m_bstrFullName = s_emptyBSTR;
        m_bstrType = s_emptyBSTR;
        m_bstrValue = s_emptyBSTR;

        m_pDebugProp = nullptr;
    }

    ~AutoDebugPropertyInfo()
    {
        ReleaseMember(m_bstrName);
        ReleaseMember(m_bstrFullName);
        ReleaseMember(m_bstrType);
        ReleaseMember(m_bstrValue);
        if (m_pDebugProp)
        {
            m_pDebugProp->Release();
        }
    }

    BSTR FullName() const { return m_bstrFullName; }
    BSTR Name() const { return m_bstrName; }
    BSTR Name(boolean full) const { return full && HasFullName() ? FullName() : Name(); }
    bool HasFullName() const { return (m_dwValidFields & DBGPROP_INFO_FULLNAME) != 0; }
    BSTR Type() const { return m_bstrType; }
    BSTR Value() const { return m_bstrValue; }
    DWORD Attr() const { return m_dwAttrib; }
    bool IsExpandable() const { return (m_dwAttrib & DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE) != 0; }
};
