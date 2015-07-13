//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

struct BpInfo
{
    BpInfo() : documentId(0), charOffset(0), charCount(0), hitCount(0), id(0)
    {
    }

    UINT64 documentId;
    DWORD charOffset;
    DWORD charCount;
    ULONG hitCount;
    ULONG id;

    CComPtr<IJsDebugBreakPoint> pBreakpoint;
};

//
// This implements a simple debugger that allows actions at break points.
//
class ATL_NO_VTABLE SimpleDebugger:
    public CComObjectRoot,
    public DummyTestGroup,
    public IDebugInputCallbacks,
    public IDebugOutputCallbacksT,
    public IDebugEventCallbacksWide
{
private:
    CComPtr<IDebugClient5> m_client;
    CComPtr<IDebugControl4> m_control;
    CComPtr<IDebugDataSpaces4> m_debugDataSpaces;
    string m_initialCommand;
    bool m_quiet;
    bool m_verbose;

public:
    SimpleDebugger();
    ~SimpleDebugger();

    BEGIN_COM_MAP(SimpleDebugger)
        COM_INTERFACE_ENTRY(IDebugInputCallbacks)
        COM_INTERFACE_ENTRY(IDebugOutputCallbacksT)
        COM_INTERFACE_ENTRY(IDebugEventCallbacksWide)
    END_COM_MAP()

    // IDebugInputCallbacks
    STDMETHOD(StartInput)(_In_ ULONG BufferSize);
    STDMETHOD(EndInput)();

    // IDebugOutputCallbacks
    STDMETHOD(Output)(_In_ ULONG Mask, _In_ PCTSTR Text);
    void Out(_In_ bool unitTest, _In_ PCTSTR fmt, ...);

    HRESULT Initialize(const string& initialCommand);
    void SetQuiet(bool quiet) { m_quiet = quiet; }
    void SetVerbose(bool verbose) { m_verbose = verbose; }
    void SetInspectMaxStringLength(int inspectMaxStringLength) { Assert(inspectMaxStringLength > 0); m_config.maxStringLengthToDump = inspectMaxStringLength; }
    void DebugLaunch(_In_ LPTSTR pCmdLine);
    void Attach(ULONG pid);

    static void Create(const string& initialCommand, _Out_ SimpleDebugger** dbg);
    IDebugDataSpaces4* GetReader() { return m_debugDataSpaces; }
#pragma warning(disable:4100) //  unreferenced formal parameter
    // IDebugEventCallbacksWide.

    // The engine calls GetInterestMask once when
    // the event callbacks are set for a client.
    STDMETHOD(GetInterestMask)(
        _Out_ PULONG Mask
        ) ;

    // A breakpoint event is generated when
    // a breakpoint exception is received and
    // it can be mapped to an existing breakpoint.
    // The callback method is given a reference
    // to the breakpoint and should release it when
    // it is done with it.
    STDMETHOD(Breakpoint)(
        _In_ PDEBUG_BREAKPOINT2 Bp
        ) ;

    // Exceptions include breaks which cannot
    // be mapped to an existing breakpoint
    // instance.
    STDMETHOD(Exception)(
        _In_ PEXCEPTION_RECORD64 Exception,
        _In_ ULONG FirstChance
        ) ;

    // Any of these values can be zero if they
    // cannot be provided by the engine.
    // Currently the kernel does not return thread
    // or process change events.
    STDMETHOD(CreateThread)(
        _In_ ULONG64 Handle,
        _In_ ULONG64 DataOffset,
        _In_ ULONG64 StartOffset
        ) {  return E_NOTIMPL; } 
    _Analysis_noreturn_
    STDMETHOD(ExitThread)(
        _In_ ULONG ExitCode
        ) { return E_NOTIMPL; } 

    // Any of these values can be zero if they
    // cannot be provided by the engine.
    STDMETHOD(CreateProcess)(
        _In_ ULONG64 ImageFileHandle,
        _In_ ULONG64 Handle,
        _In_ ULONG64 BaseOffset,
        _In_ ULONG ModuleSize,
        _In_opt_ PCWSTR ModuleName,
        _In_opt_ PCWSTR ImageName,
        _In_ ULONG CheckSum,
        _In_ ULONG TimeDateStamp,
        _In_ ULONG64 InitialThreadHandle,
        _In_ ULONG64 ThreadDataOffset,
        _In_ ULONG64 StartOffset
        ) { return E_NOTIMPL; } 
    STDMETHOD(ExitProcess)(

        _In_ ULONG ExitCode
        ) { return E_NOTIMPL; } 

    // Any of these values may be zero.
    STDMETHOD(LoadModule)(

        _In_ ULONG64 ImageFileHandle,
        _In_ ULONG64 BaseOffset,
        _In_ ULONG ModuleSize,
        _In_opt_ PCWSTR ModuleName,
        _In_opt_ PCWSTR ImageName,
        _In_ ULONG CheckSum,
        _In_ ULONG TimeDateStamp
        );
    STDMETHOD(UnloadModule)(

        _In_opt_ PCWSTR ImageBaseName,
        _In_ ULONG64 BaseOffset
        ) { return E_NOTIMPL; } 

    STDMETHOD(SystemError)(

        _In_ ULONG Error,
        _In_ ULONG Level
        ) { return E_NOTIMPL; } 

    // Session status is synchronous like the other
    // wait callbacks but it is called as the state
    // of the session is changing rather than at
    // specific events so its return value does not
    // influence waiting.  Implementations should just
    // return DEBUG_STATUS_NO_CHANGE.
    // Also, because some of the status
    // notifications are very early or very
    // late in the session lifetime there may not be
    // current processes or threads when the notification
    // is generated.
    STDMETHOD(SessionStatus)(
        _In_ ULONG Status
        ) { return E_NOTIMPL; } 

    // The following callbacks are informational
    // callbacks notifying the provider about
    // changes in debug state.  The return value
    // of these callbacks is ignored.  Implementations
    // can not call back into the engine.

    // Debuggee state, such as registers or data spaces,
    // has changed.
    STDMETHOD(ChangeDebuggeeState)(
        _In_ ULONG Flags,
        _In_ ULONG64 Argument
        ) { return E_NOTIMPL; } 
    // Engine state has changed.
    STDMETHOD(ChangeEngineState)(
        _In_ ULONG Flags,
        _In_ ULONG64 Argument
        ) { return E_NOTIMPL; } 
    // Symbol state has changed.
    STDMETHOD(ChangeSymbolState)(
        _In_ ULONG Flags,
        _In_ ULONG64 Argument
        ) { return E_NOTIMPL; } 

    //
    // Helpers to support DumpDebugProperty
    //
    typedef IJsDebugProperty* DebugProperty;
    typedef AutoJsDebugPropertyInfo AutoDebugPropertyInfo;

    const ControllerConfig& GetControllerConfig() const { return m_config; }
    bool IsHybridDebugger() const { return true;  }

    template <class Func>
    HRESULT MapPropertyInfo(IJsDebugProperty* pDebugProperty, DebugPropertyFlags flags, const Func& func)
    {
        HRESULT hr = S_OK;
        AutoDebugPropertyInfo info;

        IfFailGo(EnsureFullNameEvaluationValueIsEquivalent(pDebugProperty, flags));
        IfFailGo(pDebugProperty->GetPropertyInfo(DebuggerController::GetRadix(flags), &info));

        // Skip projections pointer variables.
        if (info.attr & JS_PROPERTY_NATIVE_WINRT_POINTER)
        {
            if (info.value == nullptr || wcslen(info.value) == 0)
            {
                DebuggerController::LogError(L"WinRT pointer value missing");
            }
            return S_OK;
        }

        IfFailGo(func(info));

    Error:
        return hr;
    }

    template <class Func>
    HRESULT EnumDebugProperties(IJsDebugProperty* pDebugProperty, DebugPropertyFlags flags, const Func& func)
    {
        HRESULT hr = S_OK;

        CComPtr<IJsEnumDebugProperty> pEnumDebugProperty;
        IfFailGo(pDebugProperty->GetMembers(JS_PROPERTY_MEMBERS::JS_PROPERTY_MEMBERS_ALL, &pEnumDebugProperty));
        if (pEnumDebugProperty == nullptr) return S_OK; // No properties.

        for (;;)
        {
            CComPtr<IJsDebugProperty> pMember;
            ULONG count = 0;
            IfFailGo(pEnumDebugProperty->Next(1, &pMember, &count));
            if (count == 0) break;

            IfFailGo(func(pMember));
        }

    Error:
        return hr;
    }

private:
    void RunDebugLoop();

    static const ULONG INVALID_BREAKPOINT_ID = ULONG_MAX;

    // Used for targeted and automatic test cases.
    bool                            m_initialized;
    DWORD                           m_procId;
    DWORD                           m_threadId;
    DebuggerController*             m_pController;
    CComPtr<IJsDebug2>              m_pJsDebug;
    CComPtr<IDebugSystemObjects>    m_pSystem;
    CComPtr<IJsDebugProcess>        m_pDebugProcess;
    ControllerConfig                m_config;
    std::vector<BpInfo*>            m_breakpoints;
    std::map<ULONG,UINT64>          m_docIdMap;
    std::map<UINT64,SourceMap>      m_sourceMaps;
    std::map<UINT64,std::wstring>   m_sourceText;
    RemoteScriptDebugEvent*         m_currentEvent;
    std::wstring                    m_filename;
    CComPtr<IJsDebugFrame>          m_currentFrame;
    ERRORRESUMEACTION               m_defaultErrorAction;
    SCRIPT_DEBUGGER_OPTIONS         m_scriptDebuggerOptions;
    BOOL                            m_scriptDebuggerOptionsValue;
    ULONG                           m_projectionCallBreakPointId;
    std::wstring                    m_projectionCallMessage;
    
    ULONG docIdCount;
    ULONG bpCount;

    // Implements a mapping from document ID's to source ID tokens
    ULONG   CreateUniqueSrcId(UINT64 docId);
    UINT64  GetDocIdForSrcId(ULONG srcId);
    ULONG   GetSrcIdForDocId(UINT64 docId);

    ULONG   CreateUniqueBreakpointId() { return bpCount++; }

    HRESULT OnInsertText(UINT64 docId, _In_ LPWSTR url, _In_ LPWSTR filename, _In_ LPWSTR text);
    HRESULT OnBreakpoint(RemoteScriptDebugEvent *evt);

    HRESULT PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, REFIID iid, LPVOID* ppunk);
    HRESULT EnsureDebugInterfaces();
    HRESULT GetCallstack(LocationToStringFlags flags = LTSF_None);
    HRESULT AsyncBreak();
    HRESULT InspectLocals();
    HRESULT GetLocation(CComPtr<IJsDebugFrame> pFrame, Location& location);
    HRESULT GetLocals(int expandLevel, DebugPropertyFlags flags);
    HRESULT EvaluateExpression(PCWSTR expression, int expandLevel, DebugPropertyFlags flags);
    HRESULT GetLocalsEnum(CComPtr<IJsDebugProperty>& pDebugProperty);
    HRESULT DumpBreakpoint(RemoteScriptDebugEvent *evt);
    HRESULT DumpSourceList();
    HRESULT GetCurrentBreakpoint(BpInfo **bpInfo);
    HRESULT LogJson(LPCWSTR logString);
    HRESULT TrackProjectionCall(PCWSTR message);
    
    // Sets the current stack frame that is active for inspection and expression evaluation
    HRESULT EnsureCurrentFrame();
    void ClearCurrentFrame();
    HRESULT SetCurrentFrame(ULONG depth);

    HRESULT SetDebuggerOptions(SCRIPT_DEBUGGER_OPTIONS mask, BOOL value);

    HRESULT ResumeFromBreakPoint(RemoteScriptDebugEvent *evt, BREAKRESUMEACTION breakResume, ERRORRESUMEACTION errorResume);
    HRESULT RemoveBreakpoint(BpInfo *bpInfo);
    HRESULT InsertBreakpoint(UINT64 docId, DWORD charOffset, DWORD charCount, BREAKPOINT_STATE bpState, _Outptr_ BpInfo **bpInfo);
    HRESULT ModifyBreakpoint(ULONG bpId, BREAKPOINT_STATE state);
    HRESULT HandleAutomaticBreakpointLogic(RemoteScriptDebugEvent *evt);

    // Javascript callbacks
    static JsValueRef CALLBACK JsInsertBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsResumeFromBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsDumpLocals(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsDumpCallstack(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsAsyncBreak(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsEvaluateExpression(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsModifyBreakpoint(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsSetFrame(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsLogJson(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsSetExceptionResume(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsSetDebuggerOptions(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
    static JsValueRef CALLBACK JsTrackProjectionCall(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

    // Expression evaluation
    HRESULT EnsureFullNameEvaluationValueIsEquivalent(IJsDebugProperty* debugPropertyInfo, DebugPropertyFlags flags);
};
