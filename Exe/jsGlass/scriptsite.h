/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

class JsGlass;

class ActiveScriptController : public IActiveScriptSite, public IActiveScriptSiteDebug
{
    long refCount;

protected:
    CComPtr<IProcessDebugManager> _processDebugManager;
    CComPtr<IDebugApplication> _debugApplication;
    CComPtr<IDebugDocumentHelper> _debugDocumentHelper;
    DWORD _dwAppCookie;

public:
    ActiveScriptController();
    virtual ~ActiveScriptController();

    HRESULT Initialize(IProcessDebugManager* processDebugManager, DWORD cookie);

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    // IActiveScriptSite
    HRESULT STDMETHODCALLTYPE GetLCID(
        /* [out] */ __RPC__out LCID *plcid);
    HRESULT STDMETHODCALLTYPE GetItemInfo(
        /* [in] */ __RPC__in LPCOLESTR pstrName,
        /* [in] */ DWORD dwReturnMask,
        /* [out] */ __RPC__deref_out_opt IUnknown **ppiunkItem,
        /* [out] */ __RPC__deref_out_opt ITypeInfo **ppti);
    HRESULT STDMETHODCALLTYPE GetDocVersionString(
        /* [out] */ __RPC__deref_out_opt BSTR *pbstrVersion);
    HRESULT STDMETHODCALLTYPE OnScriptTerminate(
        /* [in] */ __RPC__in const VARIANT *pvarResult,
        /* [in] */ __RPC__in const EXCEPINFO *pexcepinfo);
    HRESULT STDMETHODCALLTYPE OnStateChange(
        /* [in] */ SCRIPTSTATE ssScriptState);
    HRESULT STDMETHODCALLTYPE OnScriptError(
        /* [in] */ __RPC__in_opt IActiveScriptError *pscripterror);
    HRESULT STDMETHODCALLTYPE OnEnterScript();
    HRESULT STDMETHODCALLTYPE OnLeaveScript();

    // IActiveScriptSiteDebug
    HRESULT STDMETHODCALLTYPE GetDocumentContextFromPosition(
        DWORD_PTR                dwSourceContext,
        ULONG                    uCharacterOffset,
        ULONG                    uNumChars,
        IDebugDocumentContext**  ppsc);
    HRESULT STDMETHODCALLTYPE GetApplication(
        IDebugApplication**  ppda);
    HRESULT STDMETHODCALLTYPE GetRootApplicationNode(
       IDebugApplicationNode**  ppdanRoot);
    HRESULT STDMETHODCALLTYPE  OnScriptErrorDebug(
       IActiveScriptErrorDebug*  pErrorDebug,
       BOOL*                     pfEnterDebugger,
       BOOL*                     pfCallOnScriptErrorWhenContinuing);
};

class Location
{
    CComBSTR stringRep;
    CComBSTR encodedText;



public:
    ULONG startChar;
    ULONG length;
    CComBSTR text;
    CComBSTR frameDescription;

    LPCWSTR ToString();
    static LPCWSTR Encode(CComBSTR& raw, CComBSTR& encodeSpace);
};

class DebugTargetHost;

class CExprCallback : public IDebugExpressionCallBack
    
{
public:
    CExprCallback()
    {
        refCount = 0;
        m_hCompletionEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL );
        AddRef();
    }

    ~CExprCallback()
    {
        ::CloseHandle(m_hCompletionEvent);
    }

    void WaitForCompletion(void)
    {
        WaitForSingleObject(m_hCompletionEvent, 10000);
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
};

class Debugger: public IApplicationDebugger
{
    JsGlass* _jsGlass;
    long refCount;
    bool _aborting;
    CComPtr<IRemoteDebugApplicationThread> _remoteThread;

    HANDLE _event;

    struct FrameDescriptor : public DebugStackFrameDescriptor
    {
        FrameDescriptor();
        ~FrameDescriptor();
    };

public:
    Debugger(JsGlass* jsGlass);

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    // IApplicationDebugger
    HRESULT STDMETHODCALLTYPE QueryAlive();
    HRESULT STDMETHODCALLTYPE CreateInstanceAtDebugger(
        REFCLSID    rclsid,
        IUnknown*   pUnkOuter,
        DWORD       dwClsContext,
        REFIID      riid,
        IUnknown**  ppvObject
    );
    HRESULT STDMETHODCALLTYPE onDebugOutput(
        LPCOLESTR  pstr
    );
    HRESULT STDMETHODCALLTYPE onHandleBreakPoint(
       IRemoteDebugApplicationThread*  prpt,
       BREAKREASON                     br,
       IActiveScriptErrorDebug*        pError
    );
    HRESULT STDMETHODCALLTYPE onClose();
    HRESULT STDMETHODCALLTYPE onDebuggerEvent(
       REFIID     riid,
       IUnknown*  punk
    );

    HRESULT Resume(BREAKRESUMEACTION action);
    HRESULT GetLocation();
    HRESULT GetCallstack();

    HRESULT GetLocalsEnum(CComPtr<IEnumDebugPropertyInfo> &enumLocals);

    HRESULT GetLocals(int expandLevel);
    HRESULT GetAllProperties(CComPtr<IEnumDebugPropertyInfo> enumLocals, int expandLevel, CComBSTR & varsEvent, CComBSTR separator, VARIANT_BOOL checkOrder);
    HRESULT EvaluateExpr(BSTR bstrExpr, int expandLevel, VARIANT_BOOL checkOrder);
    HRESULT EditLocalValue(BSTR bstrLocalRoot, BSTR bstrLocalChild, BSTR bstrValue);

    void StoreWaitResultEvent(HANDLE evnt)
    {
        // This is for setting event, when the we break on onHandleBreakpoint.
        _event = evnt;
    }

    static HRESULT GetNextFrameLocation(IEnumDebugStackFrames* enumFrames, Location& loc);
    static HRESULT GetLocation(FrameDescriptor& frameDescriptor, Location& location );
    static HRESULT GetLocation(IDebugCodeContext* codeContext, Location& location );
};

class DebugTargetHost : public ActiveScriptController
{
    JsGlass* _jsGlass;
    CComPtr<IActiveScript> _targetActiveScript;
    unsigned int _threadId;
    CComPtr<IRemoteDebugApplicationThread> _remoteThread;
    CComBSTR _pdmPath;
    Message<DebugTargetHost> _message;

public:
    struct HostLoopArgs
    {
        JsGlass* jsGlass;
        LPCWSTR pdmPath;
        LPCWSTR jscriptPath;
    };
    static unsigned int HostLoop(void* args);

public:
    DebugTargetHost(JsGlass* jsGlass, LPCWSTR pdmPath);
    virtual ~DebugTargetHost();

    HRESULT Initialize(LPCWSTR targetdll, IProcessDebugManager* processDebugManager, IApplicationDebugger* debugger);
    HRESULT AddScript(LPCWSTR scriptCode, LPCWSTR filename);
    HRESULT RunPendingScripts();
    HRESULT SetBreakpoint(LPCWSTR scriptName, int charOffset);
    HRESULT EnableFirstChanceException(BOOL fEnable);
    HRESULT Quit();

    HRESULT ADummyBlockCall();
};
