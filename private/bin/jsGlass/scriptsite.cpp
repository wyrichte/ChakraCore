/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "HostSysInfo.h"

ActiveScriptController::ActiveScriptController()
    : refCount(1)
     
{
}

HRESULT ActiveScriptController::Initialize(IProcessDebugManager* processDebugManager, DWORD cookie)
{
    _processDebugManager = processDebugManager;
     _dwAppCookie = cookie;
     return S_OK;
}

ActiveScriptController::~ActiveScriptController()
{
}

HRESULT ActiveScriptController::QueryInterface(REFIID riid, void** ppvObject) 
{
    if (riid == _uuidof(IUnknown))
    {
        *ppvObject =  static_cast<IUnknown*>(static_cast<IActiveScriptSite*>(this));
    }
    else if (riid == _uuidof(IActiveScriptSite))
    {
        *ppvObject =  static_cast<IActiveScriptSite*>(this);
    }
    else if (riid == _uuidof(IActiveScriptSiteDebug))
    {
        *ppvObject = static_cast<IActiveScriptSiteDebug*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG ActiveScriptController::AddRef()
{
    return InterlockedIncrement(&refCount);
}

ULONG ActiveScriptController::Release()
{
    long currentCount = InterlockedDecrement(&refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

HRESULT ActiveScriptController::GetLCID(
    /* [out] */ __RPC__out LCID *plcid)
{
    //??
    return E_NOTIMPL;
}

HRESULT ActiveScriptController::GetItemInfo(
    /* [in] */ __RPC__in LPCOLESTR pstrName,
    /* [in] */ DWORD dwReturnMask,
    /* [out] */ __RPC__deref_out_opt IUnknown **ppiunkItem,
    /* [out] */ __RPC__deref_out_opt ITypeInfo **ppti)
{
    //??
    return E_NOTIMPL;
}

HRESULT ActiveScriptController::GetDocVersionString(
    /* [out] */ __RPC__deref_out_opt BSTR *pbstrVersion)
{
    //??
    return E_NOTIMPL;
}

HRESULT ActiveScriptController::OnScriptTerminate(
    /* [in] */ __RPC__in const VARIANT *pvarResult,
    /* [in] */ __RPC__in const EXCEPINFO *pexcepinfo)
{
    // TODO echo to host to log the error
    return S_OK;
}

HRESULT ActiveScriptController::OnStateChange(
    /* [in] */ SCRIPTSTATE ssScriptState)
{
    // TODO echo to host to log the error
    return S_OK;
}

HRESULT ActiveScriptController::OnScriptError(
    /* [in] */ __RPC__in_opt IActiveScriptError *pscripterror)
{
    HRESULT hr;
    EXCEPINFO exceptionInfo;
    hr = pscripterror->GetExceptionInfo(&exceptionInfo);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: Unhandled exception: Unable to get exception info\n");
    }
    else
    {
        wprintf(L"ERROR: Unhandled exception: %s\n", exceptionInfo.bstrDescription);
    }
    return S_OK;
}

HRESULT ActiveScriptController::OnEnterScript()
{
    //echo to the host
    return S_OK;
}

HRESULT ActiveScriptController::OnLeaveScript()
{
    //echo to the host
    return S_OK;
}

HRESULT ActiveScriptController::GetDocumentContextFromPosition(
    DWORD_PTR                dwSourceContext,
    ULONG                    uCharacterOffset,
    ULONG                    uNumChars,
    IDebugDocumentContext**  ppsc)
{
    HRESULT hr = S_OK;

    IfFalseGo(_processDebugManager, E_NOTIMPL);
    if (!_debugDocumentHelper)
    {
        // TODO convey the correct pdm here...
        IfFailGo( PrivateCoCreate(L"c:\\dd\\Lab26vsts\\binaries\\x86fre\\bin\\i386\\pdm.dll",
                        CLSID_CDebugDocumentHelper,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        _uuidof(IDebugDocumentHelper),
                        (LPVOID*)&_debugDocumentHelper) );
        //IfFailGo( _processDebugManager->CreateDebugDocumentHelper(NULL, &_debugDocumentHelper) );
    }

    // TODO: allow for multiple documents, as is this allows for only a single document
    IfFailGo( _debugDocumentHelper->CreateDebugDocumentContext(uCharacterOffset, uNumChars, ppsc) );

Error:
    return hr;
}

HRESULT STDMETHODCALLTYPE ActiveScriptController::GetApplication(
    IDebugApplication**  ppda) 
{
    HRESULT hr = S_OK;

    IfFalseGo(_processDebugManager, E_NOTIMPL);
    IfFalseGo(ppda, E_INVALIDARG);
    if (!_debugApplication)
    {
        IfFailGo( _processDebugManager->CreateApplication(&_debugApplication) );
        IfFailGo( _debugApplication->SetName(L"Glass Application") );
        IfFailGo( _processDebugManager->AddApplication(_debugApplication, &_dwAppCookie) );
    }

    *ppda = _debugApplication;
    (*ppda)->AddRef();

Error:
    return hr;
}

HRESULT ActiveScriptController::GetRootApplicationNode(
   IDebugApplicationNode**  ppdanRoot)
{
    return E_NOTIMPL;
}

HRESULT ActiveScriptController::OnScriptErrorDebug(
   IActiveScriptErrorDebug*  pErrorDebug,
   BOOL*                     pfEnterDebugger,
   BOOL*                     pfCallOnScriptErrorWhenContinuing)
{
    return E_NOTIMPL;
}

Debugger::Debugger(JsGlass* jsGlass)
    :_jsGlass(jsGlass),
    _aborting(false),
    _event(NULL)
{
}

HRESULT Debugger::QueryInterface(REFIID riid, void** ppvObject) 
{
    if (riid == _uuidof(IUnknown))
    {
        *ppvObject =  static_cast<IUnknown*>(static_cast<IApplicationDebugger*>(this));
    }
    else if (riid == _uuidof(IApplicationDebugger))
    {
        *ppvObject =  static_cast<IApplicationDebugger*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG Debugger::AddRef()
{
    return InterlockedIncrement(&refCount);
}

ULONG Debugger::Release()
{
    long currentCount = InterlockedDecrement(&refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
    }

HRESULT Debugger::QueryAlive()
{
    return S_OK;
}

HRESULT Debugger::CreateInstanceAtDebugger(
    REFCLSID    rclsid,
    IUnknown*   pUnkOuter,
    DWORD       dwClsContext,
    REFIID      riid,
    IUnknown**  ppvObject
)
{
    return S_OK;
}

HRESULT Debugger::onDebugOutput(
    LPCOLESTR  pstr
)
{
    return S_OK;
}

HRESULT Debugger::onHandleBreakPoint(
   IRemoteDebugApplicationThread*  prpt,
   BREAKREASON                     br,
   IActiveScriptErrorDebug*        pError
)
{
    HRESULT hr = S_OK;
    CComBSTR bpEventBSTR = "{\"breakpoint\" : ";

    _remoteThread = prpt;
    if (_aborting)
    {
        Resume(BREAKRESUMEACTION_ABORT);
    }

    IfFalseGo( _remoteThread, E_FAIL );
    switch(br)
    {
    case BREAKREASON_STEP:
        bpEventBSTR += "\"step\"}";
        break;
    case BREAKREASON_BREAKPOINT:
        bpEventBSTR += "\"breakpoint\"}";
        break;
    case BREAKREASON_DEBUGGER_BLOCK:
        bpEventBSTR += "\"debugger block\"}";
        break;
    case BREAKREASON_HOST_INITIATED:
        bpEventBSTR += "\"host initiated\"}";
        break;
    case BREAKREASON_LANGUAGE_INITIATED:
        bpEventBSTR += "\"language initiated\"}";
        break;
    case BREAKREASON_DEBUGGER_HALT:
        bpEventBSTR += "\"debugger halt\"}";
        break;
    case BREAKREASON_ERROR:
        {
            BOOL fSet = FALSE;
            CComPtr<IActiveScriptErrorDebug110> spScriptErrorDebug110 = NULL;
            SCRIPT_ERROR_DEBUG_EXCEPTION_THROWN_KIND exceptionKind = ETK_UNHANDLED;

            if (pError->QueryInterface(__uuidof(IActiveScriptErrorDebug110), (void **)&spScriptErrorDebug110) == S_OK)
            {
                if (spScriptErrorDebug110->GetExceptionThrownKind(&exceptionKind) == S_OK)
                {
                    if (exceptionKind == ETK_USER_UNHANDLED)
                    {
                        bpEventBSTR += "\"error user unhandled\"}";
                        fSet = TRUE;
                    }
                    else if (exceptionKind == ETK_FIRST_CHANCE)
                    {
                        bpEventBSTR += "\"error first chance\"}";
                        fSet = TRUE;
                    }

                }
            }

            if (!fSet)
            {
                bpEventBSTR += "\"error\"}";
            }
        }
        break;
    case BREAKREASON_JIT:
        bpEventBSTR += "\"jit\"}";
        break;
    default:
        bpEventBSTR += "\"UNKNOWN\"}";
    }
    _jsGlass->EventCallback(bpEventBSTR);

    // Ping the blocked main thread.
    if (_event)
    {
        SetEvent(_event);
    }

Error:

    return S_OK;
}

HRESULT Debugger::onClose()
{
    return S_OK;
}

HRESULT Debugger::onDebuggerEvent(
   REFIID     riid,
   IUnknown*  punk
)
{
    return S_OK;
}

HRESULT Debugger::Resume(BREAKRESUMEACTION action)
{
    HRESULT hr = S_OK;
    CComPtr<IRemoteDebugApplication> debugApplication;
    CComPtr<IRemoteDebugApplicationThread> remoteThread;

    IfFalseGo( _remoteThread, E_FAIL );
    remoteThread = _remoteThread;

    //HACK ... to simplify shutdown 
    //
    // Both VS and devtoolbar remove BPs on shutdown
    // I need to investigate how abort worked with the old engine.
    if (action == BREAKRESUMEACTION_ABORT)
    {
        CComPtr<IEnumDebugStackFrames> enumFrames;
        CComPtr<IDebugCodeContext> codeContext;
        FrameDescriptor frameDescriptor;
        ULONG ct;

        _aborting = true;

        IfFailGo( _remoteThread->EnumStackFrames(&enumFrames) );
        hr = enumFrames->Next(1, &frameDescriptor, &ct);
        IfFalseGo( hr == S_OK, hr );
        IfFailGo( frameDescriptor.pdsf->GetCodeContext(&codeContext) );
        IfFailGo( codeContext->SetBreakPoint(BREAKPOINT_DELETED) );
    }
    // END HACK

    IfFailGo( _remoteThread->GetApplication(&debugApplication) );
    IfFalseGo( debugApplication, E_FAIL );

    // NULL out remote thread before continue
    _remoteThread = NULL;
    IfFailGo( debugApplication->ResumeFromBreakPoint(remoteThread,
                                 action,
                                 ERRORRESUMEACTION_SkipErrorStatement
                                 ) );

Error:

    return hr;
}

Debugger::FrameDescriptor::FrameDescriptor()
{
    pdsf = NULL;
    punkFinal = NULL;
}

Debugger::FrameDescriptor::~FrameDescriptor()
{
    if (pdsf)
    {
        pdsf->Release();
    }
    if (punkFinal)
    {
        punkFinal->Release();
    }
}

HRESULT Debugger::GetLocation(IDebugCodeContext* codeContext, Location& location )
{
    HRESULT hr = S_OK;
    CComPtr<IDebugDocumentContext> docContext;
    CComPtr<IDebugDocument> doc;
    CComPtr<IDebugDocumentText> docText;
    ULONG ctRet = 0;
    WCHAR* buf = NULL;

    IfFailGo( codeContext->GetDocumentContext(&docContext) );
    IfFailGo( docContext->GetDocument(&doc) );
    IfFailGo( doc->QueryInterface(&docText) );
    IfFailGo( docText->GetPositionOfContext(docContext, &location.startChar, &location.length) );

    // Because of the some bug in the engine, we are not getting text from any other position then 0,
    // so put workaround to get the text fro the 0 and crop that buffer here itself.

    ULONG cChars = location.startChar + location.length;

    buf = new WCHAR[cChars+1];
    IfFalseGo( buf, E_OUTOFMEMORY);
    buf[cChars] = '\0';

    IfFailGo( docText->GetText(0, buf, NULL, &ctRet, cChars) );
    location.text = &buf[location.startChar];

Error:

    if (buf)
    {
        delete [] buf;
    }

    return hr;
}

HRESULT Debugger::GetLocation(FrameDescriptor& frameDescriptor, Location& location )
{
    HRESULT hr = S_OK;
    CComPtr<IDebugCodeContext> codeContext;

    IfFailGo( frameDescriptor.pdsf->GetCodeContext(&codeContext) );
    IfFailGo( GetLocation(codeContext, location) );
    IfFailGo( frameDescriptor.pdsf->GetDescriptionString(TRUE,&location.frameDescription) );

Error:

    return hr;
}

HRESULT Debugger::GetNextFrameLocation(IEnumDebugStackFrames* enumFrames, Location& loc)
{
    HRESULT hr = S_OK;
    FrameDescriptor frameDescriptor;
    ULONG ct;

    hr = enumFrames->Next(1, &frameDescriptor, &ct);
    IfFalseGo( hr == S_OK, hr );
    IfFailGo( GetLocation(frameDescriptor, loc) );

Error:

    return hr;
}

HRESULT Debugger::GetLocation()
{
    HRESULT hr = S_OK;
    CComPtr<IEnumDebugStackFrames> enumFrames;
    Location loc;

    IfFalseGo( _remoteThread, E_FAIL );
    IfFailGo( _remoteThread->EnumStackFrames(&enumFrames) );
    hr = GetNextFrameLocation(enumFrames, loc);
    IfFalseGo( hr == S_OK, hr );

    _jsGlass->EventCallback(loc.ToString());

Error:

    return hr;
}

HRESULT Debugger::GetCallstack()
{
    HRESULT hr = S_OK;
    CComPtr<IEnumDebugStackFrames> enumFrames;
    CComBSTR callstackEvent = L"{\"callstack\" : [";
    CComBSTR separator = L"";

    IfFalseGo( _remoteThread, E_FAIL );
    IfFailGo( _remoteThread->EnumStackFrames(&enumFrames) );

    do
    {
        Location loc;
        hr = GetNextFrameLocation(enumFrames, loc);
        if (S_OK == hr)
        {
            callstackEvent += separator;
            callstackEvent += loc.ToString();
            separator = L",";
        }
    } 
    while (hr == S_OK);

    callstackEvent += L"]}";
    _jsGlass->EventCallback(callstackEvent);

Error:

    return hr;
}

HRESULT Debugger::GetAllProperties(CComPtr<IEnumDebugPropertyInfo> enumLocals, int expandLevel, CComBSTR & varsEvent, CComBSTR separator, VARIANT_BOOL checkOrder)
{
    if (expandLevel < 0 )
    {
        return S_OK;
    }

    HRESULT hr = S_OK;

    ULONG ct;
    ULONG totalProps;
    ULONG iter = 0;
    ULONG scopeId = 1;
    BOOL  childrenFetched = FALSE;
    DebugPropertyInfo debugPropertyInfo;

    IfFailGo( enumLocals->GetCount( &totalProps) );

    ULONG orderId = 1;

    while (iter++ < totalProps)
    {
        enumLocals->Next(1, &debugPropertyInfo, &ct);

        varsEvent += separator;
        varsEvent += L"\"";
        varsEvent += debugPropertyInfo.m_bstrName;
        // Since there can be multiple scopes at the same level, while building JSON out of all scopes value will be trimmed since they have same name
        // Appending some ID, so that they will remain and tested.
        if (checkOrder == VARIANT_TRUE || 0 == _wcsicmp(debugPropertyInfo.m_bstrName, L"[Scope]"))
        {
            ULONG * pVal = &scopeId;
            if (checkOrder == VARIANT_TRUE)
            {
                 pVal = &orderId;
            }
            wchar_t buff[10];
            ZeroMemory(buff, sizeof(buff));
            swprintf_s(buff, 10, L"%d", *pVal);

            *pVal = *pVal + 1;

            varsEvent += buff;
        }
        varsEvent += L"\"";

        varsEvent += L" : ";

        childrenFetched = FALSE;

        if (expandLevel > 0 && debugPropertyInfo.m_pDebugProp && ((debugPropertyInfo.m_dwAttrib & DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE) == DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE))
        {
            CComPtr<IEnumDebugPropertyInfo> enumProps;
            hr = debugPropertyInfo.m_pDebugProp->EnumMembers(DBGPROP_INFO_ALL, 10, IID_IEnumDebugPropertyInfo, &enumProps) ;
            if (hr == S_OK)
            {
                varsEvent += L" {";
                GetAllProperties(enumProps, expandLevel-1, varsEvent, L"", checkOrder);
                varsEvent += L"}";
                childrenFetched = TRUE;
            }
        }

        if (!childrenFetched)
        {
            if (wcslen(debugPropertyInfo.m_bstrValue) < 20)
            {
                if (0 == _wcsicmp(debugPropertyInfo.m_bstrType, L"String"))
                {
                    varsEvent += debugPropertyInfo.m_bstrValue;
                }
                else
                {
                    varsEvent += L"\"";
                    varsEvent += debugPropertyInfo.m_bstrValue;
                    varsEvent += L"\"";
                }
            }
            else
            {
                // it is always difficult to justify the big string. better just tell them it is a big string.
                varsEvent += L"\"_bigstr\"";
            }
        }
        separator = L",";
    }

Error:
    return hr;
}

HRESULT Debugger::GetLocalsEnum(CComPtr<IEnumDebugPropertyInfo> &enumLocals)
{
    HRESULT hr = S_OK;

    CComPtr<IEnumDebugStackFrames> enumFrames;
    CComPtr<IDebugProperty> debugProperty;

    FrameDescriptor frameDescriptor;
    ULONG ct;

    IfFalseGo( _remoteThread, E_FAIL );
    IfFailGo( _remoteThread->EnumStackFrames(&enumFrames) );

    IfFailGo( enumFrames->Next(1, &frameDescriptor, &ct) );
    IfFailGo( frameDescriptor.pdsf->GetDebugProperty(&debugProperty) );

    IfFailGo( debugProperty->EnumMembers(DBGPROP_INFO_ALL, 10, IID_IEnumDebugPropertyInfo, &enumLocals) );

Error:
    return hr;
}

HRESULT Debugger::GetLocals(int expandLevel)
{
    if (expandLevel < 0)
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    CComBSTR localsEvent = L"{\"locals\" : {";
    CComBSTR separator = L"";

    CComPtr<IEnumDebugPropertyInfo> enumLocals;

    GetLocalsEnum(enumLocals);
    GetAllProperties(enumLocals, expandLevel, localsEvent, separator, VARIANT_FALSE);

    localsEvent += L"}}";
    _jsGlass->EventCallback(localsEvent);

    return hr;
}

HRESULT Debugger::EvaluateExpr(BSTR bstrExpr, int expandLevel, VARIANT_BOOL checkOrder)
{
    if (expandLevel < 0)
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    CComBSTR evaluateExprEvent = L"{\"evaluate\" : {";

    CComPtr<IEnumDebugStackFrames> enumFrames;
    CComPtr <IDebugExpressionContext> debugExpressionContext;
    CComPtr <IDebugExpression> debugExpression;
    CComPtr<IDebugProperty> debugProperty;

    ULONG ct;
    FrameDescriptor frameDescriptor;
    DebugPropertyInfo debugPropertyInfo;

    CComPtr<CExprCallback> exprCallback;
    exprCallback.Attach(new CExprCallback());       // CExprCallback is created with 1 ref count already. So just attach to the CComPtr
    HRESULT returnResult;

    IfFalseGo( _remoteThread, E_FAIL );
    IfFailGo( _remoteThread->EnumStackFrames(&enumFrames) );

    IfFailGo( enumFrames->Next(1, &frameDescriptor, &ct) );

    // Expression context is related to the stack frame
    frameDescriptor.pdsf->QueryInterface(IID_IDebugExpressionContext,
        (LPVOID*)&debugExpressionContext
        );

    // Load the expression text into the expression context
    debugExpressionContext->ParseLanguageText(
        bstrExpr,                        // Expression
        10,                            // Radix
        L"",                        // Text delimiter
        DEBUG_TEXT_RETURNVALUE |    // Need return
        DEBUG_TEXT_ISEXPRESSION,
        &debugExpression            // Expression result
        );

    debugExpression->Start(exprCallback);
    exprCallback->WaitForCompletion();


    IfFailGo( debugExpression->GetResultAsDebugProperty(&returnResult, &debugProperty) );
    IfFailGo( debugProperty->GetPropertyInfo(DBGPROP_INFO_ALL, 10, &debugPropertyInfo) );

    evaluateExprEvent += L"\"";
    evaluateExprEvent += debugPropertyInfo.m_bstrName;
    evaluateExprEvent += L"\"";

    evaluateExprEvent += L" : ";

    if (expandLevel > 0 && debugPropertyInfo.m_pDebugProp && ((debugPropertyInfo.m_dwAttrib & DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE) == DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE))
    {
        CComPtr<IEnumDebugPropertyInfo> enumProps;
        hr = debugPropertyInfo.m_pDebugProp->EnumMembers(DBGPROP_INFO_ALL, 10, IID_IEnumDebugPropertyInfo, &enumProps) ;
        if (hr == S_OK)
        {
            evaluateExprEvent += L" {";
            GetAllProperties(enumProps, expandLevel-1, evaluateExprEvent, L"", checkOrder);
            evaluateExprEvent += L"}";
        }
    }
    else
    {
        if (wcslen(debugPropertyInfo.m_bstrValue) < 20)
        {
            if (0 == _wcsicmp(debugPropertyInfo.m_bstrType, L"String"))
            {
                evaluateExprEvent += debugPropertyInfo.m_bstrValue;
            }
            else
            {
                evaluateExprEvent += L"\"";
                evaluateExprEvent += debugPropertyInfo.m_bstrValue;
                evaluateExprEvent += L"\"";
            }
        }
        else
        {
            evaluateExprEvent += L"_bigstr";
        }
    }


    evaluateExprEvent += L"}}";
    _jsGlass->EventCallback(evaluateExprEvent);

Error:
    return hr;
}

HRESULT Debugger::EditLocalValue(BSTR bstrLocalRoot, BSTR bstrLocalChild, BSTR bstrValue)
{
    HRESULT hr = S_OK;

    CComPtr<IEnumDebugPropertyInfo> enumLocals;

    GetLocalsEnum(enumLocals);

    ULONG ct;
    ULONG totalProps;
    ULONG iter = 0;
    BOOL fFound = FALSE;
    DebugPropertyInfo debugPropertyInfo = { 0 };

    IfFailGo( enumLocals->GetCount( &totalProps) );

    while (iter++ < totalProps)
    {
        enumLocals->Next(1, &debugPropertyInfo, &ct);
        if (0 == _wcsicmp(debugPropertyInfo.m_bstrName, bstrLocalRoot))
        {
            if (0 == _wcsicmp(bstrLocalChild, L""))
            {
                fFound = TRUE;
            }
            else
            {
                if (debugPropertyInfo.m_pDebugProp)
                {
                    CComPtr<IEnumDebugPropertyInfo> enumProps;
                    hr = debugPropertyInfo.m_pDebugProp->EnumMembers(DBGPROP_INFO_ALL, 10, IID_IEnumDebugPropertyInfo, &enumProps) ;
                    if (hr == S_OK)
                    {
                        IfFailGo( enumProps->GetCount( &totalProps) );
                        iter = 0;
                        while (iter++ < totalProps)
                        {
                            enumProps->Next(1, &debugPropertyInfo, &ct);
                            if (0 == _wcsicmp(debugPropertyInfo.m_bstrName, bstrLocalChild))
                            {
                                fFound = TRUE;
                                break;
                            }
                        }
                    }
                }
            }

            break;
        }
    }

    if (fFound)
    {
        if ((hr = debugPropertyInfo.m_pDebugProp->SetValueAsString(bstrValue, 10)) == S_OK)
        {
            _jsGlass->EventCallback(L"{\"editLocal\" : \"SUCCESS\"}");
        }
    }

Error:
    return hr;
}

unsigned int DebugTargetHost::HostLoop(void* args)
{
    HRESULT hr = S_OK;
    CComPtr<IProcessDebugManager> _processDebugManager;
    DebugTargetHost* host;
    HostLoopArgs* loopArgs = (HostLoopArgs*)args;

    CoInitializeEx(NULL, HostSystemInfo::SupportsOnlyMultiThreadedCOM() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);

    host = new DebugTargetHost(loopArgs->jsGlass, loopArgs->pdmPath);
    loopArgs->jsGlass->_targetHost = host;

    hr = PrivateCoCreate(loopArgs->pdmPath,
        CLSID_ProcessDebugManager, NULL, CLSCTX_INPROC_SERVER, _uuidof(IProcessDebugManager),
        (LPVOID*)&_processDebugManager);

    if (FAILED(hr))
    {
        fwprintf(stdout, L"Failed to PrivateCoCreate : %s at DebugTargetHost::HostLoop\n", loopArgs->pdmPath);
        fflush(stdout);
        goto Error;
    }

    IfFailGo( host->Initialize(loopArgs->jscriptPath, 
        _processDebugManager, loopArgs->jsGlass->_inlineDebugger) );

    // TODO this is a hack for getting commands from the main thread
    // to the scriptHost thread.
    // I need to consider how to make this affordable for expanding the command set
    MSG msg;
    bool quit = false;
    while ( !quit && GetMessage ( &msg, NULL, 0, 0) )
    {
        switch(Message<DebugTargetHost>::TryDispatch(msg))
        {
        case Message<DebugTargetHost>::M_HANDLED:
            continue;
        case Message<DebugTargetHost>::M_IGNORED:
            break;
        case Message<DebugTargetHost>::M_QUIT:
            quit = true;
            continue;
        default:
            AssertMsg(false, "incorrect return from TryDispatch");
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

Error:
    CoUninitialize();
    _endthreadex(0);
    return 0;
}

DebugTargetHost::DebugTargetHost(JsGlass* jsGlass, LPCWSTR pdmPath)
    :_message(this,GetCurrentThreadId()),
     _jsGlass(jsGlass),
     _pdmPath(pdmPath)
{
    _threadId = GetCurrentThreadId();
}

DebugTargetHost::~DebugTargetHost()
{
    if (_targetActiveScript)
    {
        _targetActiveScript->SetScriptState(SCRIPTSTATE_CLOSED);
    }
}

HRESULT DebugTargetHost::Initialize(LPCWSTR targetdll, IProcessDebugManager* processDebugManager, IApplicationDebugger* debugger)
{
    if (_threadId != GetCurrentThreadId())
    {
        AssertMsg(TRUE, "Called host initialize on the wrong thread");
        return E_FAIL;
    }

    const CLSID CLSID_Chakra = { 0x1b7cd997, 0xe5ff, 0x4932, 0xa7, 0xa6, 0x2a, 0x9e, 0x63, 0x6d, 0xa3, 0x85 };
    HRESULT hr = S_OK;
    CComPtr<IActiveScriptParse> parse;
    DWORD cookie = 13;
    CComPtr<IRemoteDebugApplication> remoteDebugApp;
    CComPtr<IDebugApplication> debugApplication;       
    CComPtr<IRemoteDebugApplication110> spDebugApp110 = NULL;

    IfFailGo( ActiveScriptController::Initialize(processDebugManager, cookie) );

    // TODO need to allow for CLSID_Chakra when loading Eze when we drop the old CLSID
    hr = PrivateCoCreate(targetdll, CLSID_Chakra, NULL, CLSCTX_INPROC_SERVER, _uuidof(IActiveScript), (LPVOID*)&_targetActiveScript);

    if (FAILED(hr))
    {
        fwprintf(stdout, L"Failed to PrivateCoCreate : %s at DebugTargetHost::Initialize\n", targetdll);
        fflush(stdout);
        goto Error;
    }

#if DBG
    {
        CComPtr<IActiveScriptProperty> pActiveScriptProperty;
        CComVariant VarVersion;
        IfFailGo(_targetActiveScript->QueryInterface(&pActiveScriptProperty));
        hr = pActiveScriptProperty->GetProperty(SCRIPTPROP_INVOKEVERSIONING, nullptr, &VarVersion);
        AssertMsg(SUCCEEDED(hr) && VarVersion.vt == VT_I4 && VarVersion.iVal == SCRIPTLANGUAGEVERSION_5_12, L"Incorrect runtime version");
    }
#endif       

    IfFailGo( _targetActiveScript->SetScriptSite(static_cast<IActiveScriptSite*>(this)) );

    IfFailGo( _targetActiveScript->QueryInterface(&parse) );
    IfFailGo( parse->InitNew() );

    IfFailGo( GetApplication(&debugApplication) );
    IfFailGo( debugApplication->QueryInterface(&remoteDebugApp) );
    IfFailGo( remoteDebugApp->ConnectDebugger(debugger) );

    if ((hr = remoteDebugApp->QueryInterface(__uuidof(IRemoteDebugApplication110), (void **)&spDebugApp110)) == S_OK)
    {
        hr = spDebugApp110->SetDebuggerOptions(SDO_ENABLE_NONUSER_CODE_SUPPORT, SDO_ENABLE_NONUSER_CODE_SUPPORT);
    }

Error:

    return hr;
}

DWORD_PTR _dwSource = 42;

HRESULT DebugTargetHost::AddScript(LPCWSTR scriptCode, LPCWSTR filename)
{
    // TODO ... we currently don't support opening a 2nd script! :-) ... 
    Message<DebugTargetHost>::FunctionCallResult result;
    if (S_OK == _message.AsyncCall(&DebugTargetHost::AddScript, scriptCode, filename, &result))
    {
        // This makes this call synchronous.  That is alright for Adding a script, and allows
        // us to just use the LPCWSTR rather than put it in a BSTR.
        return result.BlockOnResult();
    }

    HRESULT hr = S_OK;
    CComQIPtr<IActiveScriptParse> parse(_targetActiveScript);
    EXCEPINFO exceptionInfo;
    CComBSTR scriptOpenEvent = L"{\"scriptOpen\" : \"";


    CComPtr<IDebugApplication> debugApplication;

    // There needs to be 1 of these per Document ... which requires work to 
    // allow for multiple documents.  in the mean time if someone calls open twice
    // we will fail with an ATL assertion here.
    IfFailGo( PrivateCoCreate(_pdmPath,
                    CLSID_CDebugDocumentHelper,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    _uuidof(IDebugDocumentHelper),
                    (LPVOID*)&_debugDocumentHelper) );

    IfFailGo( GetApplication(&debugApplication) );
    IfFailGo( _debugDocumentHelper->Init(debugApplication, L"foo", L"Foobaz", TEXT_DOC_ATTR_READONLY) );

    // Attach to NULL says it's a global document
    IfFailGo( _debugDocumentHelper->Attach(NULL) );
    
    int ctCharsScript = (int) wcslen(scriptCode);

    // The script is the entire file .. unlike html
    IfFailGo( _debugDocumentHelper->DefineScriptBlock(0,ctCharsScript,_targetActiveScript, FALSE, &_dwSource) );

    IfFailGo( _debugDocumentHelper->AddUnicodeText(scriptCode) );

    // Set the engine state to initialized, so that the script does not run automatically
    IfFailGo( _targetActiveScript->SetScriptState(SCRIPTSTATE_INITIALIZED) );

    // Parse the text .. but do not run .. Eze and Old jscript work slightly differently here!!!
    IfFailGo( parse->ParseScriptText((LPCOLESTR)scriptCode, NULL, NULL, NULL, 
    _dwSource, 0, SCRIPTTEXT_ISVISIBLE|SCRIPTTEXT_ISPERSISTENT|SCRIPTTEXT_HOSTMANAGESSOURCE, NULL/*&variantResult*/, &exceptionInfo) );


    if (filename)
    {
        CComBSTR bstrFileName = filename;
        CComBSTR bstrEncoded;
        scriptOpenEvent += Location::Encode(bstrFileName,bstrEncoded);
    }
    else
    {
        scriptOpenEvent += L"NULL";
    }
    scriptOpenEvent += "\"}";
    _jsGlass->EventCallback(scriptOpenEvent);

Error:

    return hr;
}

HRESULT DebugTargetHost::RunPendingScripts()
{
    HRESULT hr = S_OK;

    Message<DebugTargetHost>::FunctionCallResult result;
    if (S_OK == _message.AsyncCall(&DebugTargetHost::RunPendingScripts, &result))
    {
        this->_jsGlass->_debugger->StoreWaitResultEvent(result.GetTheWaitEvent());
        return result.BlockOnResult(3000);
    }

    // Cause the script to run...
    IfFailGo( _targetActiveScript->SetScriptState(SCRIPTSTATE_CONNECTED) );

Error:

    return hr;
}

HRESULT DebugTargetHost::SetBreakpoint(LPCWSTR scriptName, int charOffset)
{
    HRESULT hr = S_OK;

    Message<DebugTargetHost>::FunctionCallResult result;
    if (S_OK == _message.AsyncCall(&DebugTargetHost::SetBreakpoint, scriptName, charOffset, &result))
    {
        // This makes this call synchronous.  That is alright for BP setting, and allows
        // us to just use the LPCWSTR rather than put it in a BSTR.
        return result.BlockOnResult();
    }

    CComPtr<IDebugDocumentContext> documentContext;
    ULONG fetchedCount = 0;
    CComPtr<IEnumDebugCodeContexts> enumCodeContexts;
    CComPtr<IDebugCodeContext> codeContext;
    Location loc;
    CComBSTR bpSetEvent = "{\"breakpointSet\" : ";

    IfFalseGo( _debugDocumentHelper, E_FAIL );
    IfFailGo( _debugDocumentHelper->CreateDebugDocumentContext(charOffset, 1, &documentContext) );
    IfFailGo( documentContext->EnumCodeContexts(&enumCodeContexts) );
    IfFailGo( enumCodeContexts->Next(1,&codeContext, &fetchedCount) );
    IfFailGo( codeContext->SetBreakPoint(BREAKPOINT_ENABLED) );

    IfFailGo( Debugger::GetLocation(codeContext, loc) );
    bpSetEvent += loc.ToString();
    bpSetEvent += L"}";

    _jsGlass->EventCallback(bpSetEvent);

Error:

    return hr;
}

HRESULT DebugTargetHost::EnableFirstChanceException(BOOL fEnable)
{
    HRESULT hr = S_OK;

    Message<DebugTargetHost>::FunctionCallResult result;
    if (S_OK == _message.AsyncCall(&DebugTargetHost::EnableFirstChanceException, fEnable, &result))
    {
        // This makes this call synchronous.  That is alright for BP setting, and allows
        // us to just use the LPCWSTR rather than put it in a BSTR.
        return result.BlockOnResult();
    }

    CComPtr<IDebugApplication> debugApplication;
    CComPtr<IRemoteDebugApplication> remoteDebugApp;
    CComPtr<IRemoteDebugApplication110> spDebugApp110 = NULL;

    IfFailGo( GetApplication(&debugApplication) );
    IfFailGo( debugApplication->QueryInterface(&remoteDebugApp) );
    IfFalseGo( remoteDebugApp, E_FAIL );

    if ((hr = remoteDebugApp->QueryInterface(__uuidof(IRemoteDebugApplication110), (void **)&spDebugApp110)) == S_OK)
    {
        hr = spDebugApp110->SetDebuggerOptions(SDO_ENABLE_FIRST_CHANCE_EXCEPTIONS, fEnable ? SDO_ENABLE_FIRST_CHANCE_EXCEPTIONS : SDO_NONE);
    }

Error:
    if (hr == S_OK)
    {
        _jsGlass->EventCallback(L"{\"enableFirstChance\" : \"SUCCESS\"}");
    }
    else
    {
        _jsGlass->EventCallback(L"{\"enableFirstChance\" : \"FAILED\"}");
    }

    return hr;
}

HRESULT DebugTargetHost::Quit()
{
    if (S_OK == _message.Quit(&DebugTargetHost::Quit,NULL))
    {
        _message.WaitForThread();
        return S_OK;
    }

    return S_OK;
}

HRESULT DebugTargetHost::ADummyBlockCall()
{
    Message<DebugTargetHost>::FunctionCallResult result;

    if (S_OK == _message.AsyncCall(&DebugTargetHost::ADummyBlockCall, &result))
    {
        this->_jsGlass->_debugger->StoreWaitResultEvent(result.GetTheWaitEvent());
        result.BlockOnResult(1000);
    }

    return S_OK;
}

LPCWSTR Location::Encode(CComBSTR& rawstr, CComBSTR& encodedStr)
{

    // If Test speed becomes an issue here is a Usual suspect.
    // This is horribly inefficient, but was easy to do.

    LPCWSTR str = rawstr.m_str;
    encodedStr = "";
    for(unsigned int i = 0; i < rawstr.Length(); i++)
    {
        switch(str[i])
        {
        case '\"':
            encodedStr += L"\\\"";
            break;
        case '\\':
            encodedStr += L"\\\\";
            break;
        case '\r':
            //swallow \r
            break;
        case '\n':
            encodedStr += L"\\n";
            break;
        case '\t':
            encodedStr += L"\\t";
            break;
        default:
            {
                WCHAR charStr[2];
                charStr[0] = str[i];
                charStr[1] = L'\0';
                encodedStr += charStr;
                break;
            }
        }
    }
    return encodedStr.m_str;
}

LPCWSTR Location::ToString()
{
    WCHAR buf[20];

    stringRep = L"{\"start\" : ";
    _itow_s(startChar,buf,20,10);
    stringRep += buf;

    stringRep += L", \"length\" : ";
    _itow_s(length,buf,20,10);
    stringRep += buf;

    stringRep += ", \"text\" : \"";
    stringRep += Encode(text,encodedText);

    if (frameDescription.Length())
    {
        stringRep += "\", \"frameDescription\" : \"";

        // I don't believe the frame description needs to be encoded.
        stringRep += frameDescription;
    }

    stringRep += L"\"}";

    return stringRep.m_str;
}
