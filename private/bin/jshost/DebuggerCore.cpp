//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "ad1ex.h"
#include "hostsysinfo.h"

const GUID IID_ISetNextStatement = { 0x51973C03, 0xCB0C, 0x11D0, { 0xB5, 0xC9, 0x00, 0xA0, 0x24, 0x4A, 0x0E, 0x7A } };

const CComBSTR AutoDebugPropertyInfo::s_emptyBSTR(_u(""));

DWORD Debugger::s_scriptThreadId = 0;

void CExprCallback::WaitForCompletion(void)
{
    HRESULT hr = S_OK;
    while (hr == S_OK)
    {
        DWORD dwWaitResult = ::MsgWaitForMultipleObjects(1, &m_hCompletionEvent, FALSE /*bWaitAll*/, INFINITE, QS_ALLINPUT);
        if (dwWaitResult != WAIT_FAILED)
        {
            if (dwWaitResult == WAIT_OBJECT_0)
            {
                break;
            }
            else
            {
                // There was a message put into our queue, see if we can dispatch it.
                MSG msg;
                while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    ::TranslateMessage(&msg);
                    if (Message<Debugger>::TryDispatch(msg) == M_IGNORED)
                    {
                        ::DispatchMessage(&msg);
                    }
                    // Check if the event was triggered before pumping more messages
                    if (::WaitForSingleObject(m_hCompletionEvent, 0) == WAIT_OBJECT_0)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        }
    }

    Assert(hr == S_OK);
}

/*static*/
HRESULT Debugger::StartDebuggerThread(Debugger **ppDebugger, IRemoteDebugApplication *pRemoteDebugApp)
{
    Assert(pRemoteDebugApp != NULL);

    s_scriptThreadId = GetCurrentThreadId();

    DWORD threadId = 0;
    HANDLE hThread = CreateThread(NULL, 0, Debugger::MainLoop, (LPVOID)ppDebugger, 0, &threadId);

    while(!PostThreadMessage(threadId, WM_USER, 0, 0))
    {
        DWORD code;
        if (!GetExitCodeThread(hThread, &code) || code != STILL_ACTIVE)
        {
            DebuggerController::LogError(_u("failed Debugger::StartDebuggerThread"));
            CloseHandle(hThread);
            return E_FAIL;
        }

        // the PostMessage will fail until the target thread starts
        // processing messages.
        Sleep(10);
    }
    CloseHandle(hThread);

    Assert(*ppDebugger != NULL);
    (*ppDebugger)->SetDebugApp(pRemoteDebugApp);
    if (pRemoteDebugApp->ConnectDebugger(*ppDebugger) == S_OK)
    {
        DebuggerController::Log(_u("Debugger connected : IRemoteDebugApplication::ConnectDebugger Succeeded"));
        (*ppDebugger)->FetchSources();
    }

    return S_OK;
}

DWORD WINAPI Debugger::CauseBreakLoop(LPVOID args)
{
    Debugger* debugger = (Debugger*)args;
    MSG msg;
    UINT_PTR timerId = SetTimer(NULL, 1, 10, NULL);
    Assert(timerId != 0);
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_TIMER)
        {
            // debugger->m_spDebugApp could be null because this thread could run 
            // before that field is initialized, check here for sure.
            if (debugger->m_spDebugApp)
            {
                debugger->m_spDebugApp->CauseBreak();
            }
        }
        else if (msg.message == WM_QUIT)
        {
            break;
        }
    }

    return 0;
}

/*static*/
DWORD WINAPI Debugger::MainLoop(LPVOID args)
{
    CoInitializeEx(NULL, HostSystemInfo::SupportsOnlyMultiThreadedCOM() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);

    Debugger *debugger = new Debugger();

    Debugger **ppDebugger = (Debugger **)args;
    *ppDebugger = debugger;

    MSG msg;

    HANDLE hThread = nullptr;
    DWORD threadId = 0;
    if (HostConfigFlags::flags.AsyncBreak)
    {
        hThread = CreateThread(NULL, 0, Debugger::CauseBreakLoop, (LPVOID)debugger, 0, &threadId);
    }
    
    while (GetMessage ( &msg, NULL, 0, 0) )
    {
        MessageReply reply = Message<Debugger>::TryDispatch(msg);
        if (reply == M_QUIT)
        {
            break;
        }
        else if (reply == M_HANDLED)
        {
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (HostConfigFlags::flags.AsyncBreak)
    {
        PostThreadMessage(threadId, WM_QUIT, 0, 0);
    }


    if(debugger->m_pController)
    {
        debugger->m_pController->VerifyAndWriteNewBaselineFile(debugger->m_filename);
    }
    debugger->Dispose();

    CoUninitialize();

    return 0;
}



Debugger::Debugger(void)
    : m_isRootNodeAdded(false),
    m_isDetached(false),
    m_isPaused(false),
    m_isShutdown(false),
    m_config(),
    m_ulSourceNumber(0),
    m_ulBpNumber(0),
    m_message(this, GetCurrentThreadId()),
    m_refCount(1),
    m_pController(nullptr),
    m_currentFrame(nullptr),
    m_defaultErrorAction(ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller),
    m_canSetBreakpoints(false),
    m_exhaustiveSearch(false)
{
    HRESULT hr = S_OK;

    // Set up the controller
    if(HostConfigFlags::flags.Auto || HostConfigFlags::flags.Targeted || HostConfigFlags::flags.DumpLocalsOnDebuggerBp)
    {
        m_pController = new DebuggerController(dbgBaselineFilename, HostConfigFlags::baselinePath);

        // Install the callbacks for the controller.
        IfFailGo(m_pController->InstallHostCallback(_u("InsertBreakpoint"), &Debugger::JsInsertBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("ModifyBreakpoint"), &Debugger::JsModifyBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("ResumeFromBreakpoint"), &Debugger::JsResumeFromBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("DumpLocals"), &Debugger::JsDumpLocals, this));
        IfFailGo(m_pController->InstallHostCallback(_u("DumpCallstack"), &Debugger::JsDumpCallstack, this));
        IfFailGo(m_pController->InstallHostCallback(_u("SetNextStatement"), &Debugger::JsSetNextStatement, this));
        IfFailGo(m_pController->InstallHostCallback(_u("EvaluateExpression"), &Debugger::JsEvaluateExpression, this));
        IfFailGo(m_pController->InstallHostCallback(_u("EvaluateExpressionAsync"), &Debugger::JsEvaluateExpressionAsync, this));
        IfFailGo(m_pController->InstallHostCallback(_u("SetFrame"), &Debugger::JsSetFrame, this));
        IfFailGo(m_pController->InstallHostCallback(_u("LogJson"), &Debugger::JsLogJson, this));
        IfFailGo(m_pController->InstallHostCallback(_u("SetExceptionResume"), &Debugger::JsSetExceptionResume, this));
        IfFailGo(m_pController->InstallHostCallback(_u("SetDebuggerOptions"), &Debugger::JsSetDebuggerOptions, this));
        IfFailGo(m_pController->InstallHostCallback(_u("RecordEdit"), &Debugger::JsRecordEdit, this));
        IfFailGo(m_pController->InstallHostCallback(_u("DumpBreakpoint"), &Debugger::JsDumpBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("SetMutationBreakpoint"), &Debugger::JsSetMutationBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("DeleteMutationBreakpoint"), &Debugger::JsDeleteMutationBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("DumpSourceList"), &Debugger::JsDumpSourceList, this));
    }

    if (HostConfigFlags::flags.InspectMaxStringLengthIsEnabled)
    {
        m_config.maxStringLengthToDump = HostConfigFlags::flags.InspectMaxStringLength;
        Assert(m_config.maxStringLengthToDump > 0);
    }

Error:
    if(FAILED(hr))
    {
        m_pController->LogError(_u("host callback initialization failed"));
    }
}


Debugger::~Debugger(void)
{
    RemoveAllBreakpoint();
    RemoveAllSourceNodes();
    RemoveAllMutationBreakpoint();
}

void Debugger::Dispose()
{
    if(m_pController)
        delete m_pController;
    m_pController = NULL;

    if(m_currentFrame)
    {
        delete m_currentFrame;
    }
}

HRESULT Debugger::QueryInterface(REFIID riid, void** ppvObject) 
{
    if (riid == _uuidof(IUnknown))
    {
        *ppvObject =  static_cast<IUnknown*>(this);
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
    return InterlockedIncrement(&m_refCount);
}

ULONG Debugger::Release()
{
    long currentCount = InterlockedDecrement(&m_refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

HRESULT Debugger::QueryAlive(void) 
{ 
    return S_OK; 
}


HRESULT Debugger::CreateInstanceAtDebugger( 
    __in REFCLSID rclsid,
    __in IUnknown __RPC_FAR *pUnkOuter,
    __in DWORD dwClsContext,
    __in REFIID riid,
    __out IUnknown __RPC_FAR *__RPC_FAR *ppvObject)
{
    return ::CoCreateInstance(rclsid,
        pUnkOuter,
        dwClsContext,
        riid,
        (LPVOID*)ppvObject);
}

HRESULT Debugger::onDebugOutput( 
    __in LPCOLESTR pstr)
{ 
    return E_NOTIMPL; 
}

HRESULT Debugger::onHandleBreakPoint( 
    __in IRemoteDebugApplicationThread __RPC_FAR *prpt,
    __in BREAKREASON br,
    __in IActiveScriptErrorDebug __RPC_FAR *pError)
{
    DebuggerController::Log(_u("Debugger::onHandleBreakPoint(): BREAKREASON = %s\n"), DebuggerController::GetBreakpointReason(br));

    if (!IsAttached())
    {
        DebuggerController::LogError(_u("should be attached at Debugger::onHandleBreakPoint"));
        return E_FAIL;
    }

    if (m_isDetached)
    {
        ResumeFromBreakPoint(BREAKRESUMEACTION_ABORT, m_config.GetErrorResumeAction());
        return S_OK;
    }

    if (m_spScriptThread.p != NULL)
    {
        // Already broken before
        DebuggerController::LogError(_u("not resumed from previous break Debugger::onHandleBreakPoint"));
    }

    // Rule of thumb : if you cache some variables which are supposed to live during this debugger break then
    // make sure they go away on or before ResumeFromBreakpoint, because after that debuggee thread will be resumed and it 
    // is possible that we might hit onHandleBreakpoint immediately.

    m_spScriptThread = prpt; 
    m_eCurrentBreakReason = br;
    m_spCurrentError = pError;
    PerformOnBreak();

    DebuggerController::Log(_u("/Debugger::onHandleBreakPoint(): BREAKREASON = %s\n"), DebuggerController::GetBreakpointReason(br));

    return S_OK;
}

HRESULT Debugger::onClose(void) 
{ 
    // As part of detach -> pdm will release pointer to us and we will release a pointer to us, making ref count 0
    // Which would mean that we would be gone from memory and m_pState wont have any value for calling AppClose event
    // Hence do add ref and release it after call.
    AddRef();

    if (!m_isShutdown)
    {
        DetachFromTarget();
    }

    Release();
    return S_OK;
}

HRESULT Debugger::onDebuggerEvent( 
    __in REFIID riid,
    __in IUnknown __RPC_FAR *punk)
{ 
    if (riid == __uuidof(IRemoteDebugCriticalErrorEvent110))
    {
        CComPtr<IRemoteDebugCriticalErrorEvent110> pCriticalErrorInfo;
        HRESULT hr = punk->QueryInterface(__uuidof(IRemoteDebugCriticalErrorEvent110), (void**)&pCriticalErrorInfo);
        AssertMsg(SUCCEEDED(hr), "Failed: QueryInterface IRemoteDebugCriticalErrorEvent110");
        if (pCriticalErrorInfo == nullptr)
        {
            Assert(false);
            return E_UNEXPECTED;
        }

        CComBSTR bstrSource, bstrMessage; int messageId;
        CComPtr<IDebugDocumentContext> pDocumentContext;
        hr = pCriticalErrorInfo->GetErrorInfo(&bstrSource, &messageId, &bstrMessage, &pDocumentContext);
        AssertMsg(SUCCEEDED(hr), "Failed IRemoteDebugCriticalErrorEvent110 GetErrorInfo");

        CComPtr<IDebugDocument> pDebugDocument;
        hr = pDocumentContext->GetDocument(&pDebugDocument);
        AssertMsg(SUCCEEDED(hr), "Failed: IDebugDocumentContext GetDocument");

        CComPtr<IDebugDocumentText> pDebugDocumentText;
        hr = pDebugDocument->QueryInterface(__uuidof(IDebugDocumentText), (void**)&pDebugDocumentText);
        AssertMsg(SUCCEEDED(hr), "Failed: QueryInterface IDebugDocumentText");

        ULONG startIndex, length;
        hr = pDebugDocumentText->GetPositionOfContext(pDocumentContext, &startIndex, &length);
        AssertMsg(SUCCEEDED(hr), "Failed: IDebugDocumentText GetPositionOfContext");
        return S_OK;
    }
    else if (riid == __uuidof(IRemoteDebugInfoEvent110))
    {
        CComPtr<IRemoteDebugInfoEvent110> messageInfo;
        HRESULT hr = punk->QueryInterface(__uuidof(IRemoteDebugInfoEvent110), (void**)&messageInfo);
        if (hr == S_OK && messageInfo != nullptr)
        {
            DEBUG_EVENT_INFO_TYPE type;
            CComBSTR bstrMessage;
            CComBSTR url;
            CComPtr<IDebugDocumentContext> pDocumentContext;

            if (messageInfo->GetEventInfo(&type, &bstrMessage, &url, &pDocumentContext) == S_OK)
            {
                pDocumentContext;// currently pDocumentContext is not used.

                if ((type == DEIT_ASMJS_IN_DEBUGGING || type == DEIT_ASMJS_SUCCEEDED || type == DEIT_ASMJS_FAILED)
                    && bstrMessage.m_str != nullptr)
                {
                    TCHAR szShortName[255];
                    if (url.m_str != nullptr)
                    {
                        GetShortNameFromUrl(url.m_str, szShortName, 255);
                        fwprintf(stdout, _u("%ls [@ %ls]\n"), bstrMessage.m_str, szShortName);
                    }
                    else
                    {
                        fwprintf(stdout, _u("%ls\n"), bstrMessage.m_str);
                    }

                }
            }
        }
    }

    return E_NOTIMPL; 
}


bool Debugger::IsAttached() const
{
    return m_spDebugApp.p != NULL;
}

bool Debugger::IsAtBreak() const
{
    return m_spScriptThread.p != NULL;
}

HRESULT Debugger::SetDebugApp(IRemoteDebugApplication * pDebugApp)
{
    if (IsAttached())
    {
        // Do we want allow attach again??
        DebuggerController::LogError(_u("multiple attach at Debugger::SetDebugApp"));

        //if (m_spDebugApp.p == pDebugApp)
        //{
        //    return S_OK; 
        //}
    }

    m_spDebugApp = pDebugApp;

    return S_OK;
}

HRESULT Debugger::SetDebuggerOptions(SCRIPT_DEBUGGER_OPTIONS mask, BOOL value)
{
    HRESULT hr = S_OK;
    CComPtr<IRemoteDebugApplication110> m_spDebugApp110;

    IfFailGo(m_spDebugApp->QueryInterface<IRemoteDebugApplication110>(&m_spDebugApp110));

    IfFailGo(m_spDebugApp110->SetDebuggerOptions(mask, value == TRUE ? mask : SDO_NONE));

Error:
    return hr;
}

HRESULT Debugger::DetachFromTarget()
{
    if (!IsAttached())
    {
        DebuggerController::LogError(_u("detach without attach at Debugger::DetachFromTarget"));
        return  E_FAIL;
    }

    m_isShutdown = true;

    RemoveAllBreakpoint();
    RemoveAllSourceNodes();
    RemoveAllMutationBreakpoint();

    // Sources will be removed from the pdm side, Check whether you have to call RemoveAllSourceNodes

    HRESULT hr = m_spDebugApp->DisconnectDebugger();

    if (hr == S_OK)
    {
        DebuggerController::Log(_u("Debugger disconnected : IRemoteDebugApplication::DisconnectDebugger Succeeded"));
    }

    // Resume from any breakpoint if that happens
    m_isDetached = true;

    m_spScriptThread.Release();
    m_spDebugApp.Release();

    return hr;
}

HRESULT Debugger::ResumeFromBreakPoint(BREAKRESUMEACTION resumeAction, ERRORRESUMEACTION errorAction)
{
    HRESULT hr = E_FAIL;

    DebuggerController::Log(_u("Debugger::ResumeFromBreakPoint(): BREAKRESUMEACTION = %s, ERRORRESUMEACTION = %s\n"), DebuggerController::GetBreakResumeAction(resumeAction), DebuggerController::GetErrorResumeAction(errorAction));

    if(!IsAttached() || !IsAtBreak())
    {
        DebuggerController::LogError(_u("Debugger::ResumeFromBreakPoint(): did not break - returning E_FAIL"));
        return E_FAIL;
    }

    m_spCurrentError.Release();

    // Clear the old stack frame pointers.
    ClearCurrentFrame();

    IRemoteDebugApplicationThread *pScriptThread = m_spScriptThread.p;
    m_spScriptThread.p = NULL;

    //m_spDbgThread.Release();

    m_eLastBreakAction = resumeAction;
    m_eLastErrorAction = errorAction;

    hr = m_spDebugApp->ResumeFromBreakPoint(
        pScriptThread,
        resumeAction,
        errorAction);

    pScriptThread->Release();

    DebuggerController::Log(_u("/Debugger::ResumeFromBreakPoint(): BREAKRESUMEACTION = %s, ERRORRESUMEACTION = %s, hr = 0x%08X\n"), DebuggerController::GetBreakResumeAction(resumeAction), DebuggerController::GetErrorResumeAction(errorAction), hr);

    return hr;
}

void Debugger::EnsureRootNode()
{
    if (!m_isRootNodeAdded)
    {
        // Adding the root node
        CComPtr<IDebugApplicationNode> pToptNode;
        if (m_spDebugApp->GetRootNode(&pToptNode) == S_OK)
        {
            m_isRootNodeAdded = true;
            AddNode(pToptNode, (ULONG)-1);
        }
    }
}

HRESULT Debugger::FetchSources()
{
    Message<Debugger>::FunctionCallResult result;
    if (S_OK == m_message.AsyncCall(&Debugger::FetchSources, &result))
    {
        return result.BlockOnResult();
    }

    // Will happen at the test debugger thread.

    EnsureRootNode();

    return S_OK;
}

HRESULT Debugger::QuitThread()
{
    if (S_OK == m_message.Quit(&Debugger::QuitThread,NULL))
    {
        m_message.WaitForThread();
        return S_OK;
    }

    return S_OK;
}

HRESULT Debugger::Disconnect()
{
    if (!m_isShutdown)
    {
        DetachFromTarget();
    }

    return QuitThread();
}

HRESULT Debugger::PerformOnBreak()
{
    // The handling of PerformOnBreak needs to be done on the debugger thread.

    DebuggerController::Log(_u("Debugger::PerformOnBreak()\n"));

    // TODO : if you need to store the exception detail, we need to store here in the pdm's thread.
    HRESULT hr = S_OK;
    Message<Debugger>::FunctionCallResult result;
    if (S_OK == m_message.AsyncCall(&Debugger::PerformOnBreak, &result))
    {
        // We dont' have to wait.
        return S_OK;
    }

    // Resume
    if(HostConfigFlags::flags.Targeted)
    {
        if(m_eCurrentBreakReason == BREAKREASON_ERROR_EX || m_eCurrentBreakReason == BREAKREASON_DEBUGGER_HALT_EX)
        {
            // The breakpoint was from an exception or async break
            IfFailGo(m_pController->HandleException());
        }
        else if (m_eCurrentBreakReason == BREAKREASON_MUTATION_BREAKPOINT_EX)
        {
            IfFailGo(m_pController->HandleMutationBreakpoint());
        }
        else if (m_eCurrentBreakReason == BREAKREASON_DOMMUTATION_BREAKPOINT_EX)
        {
            IfFailGo(m_pController->HandleDOMMutationBreakpoint());
        }
        else
        {
            // Get the current breakpoint ID, and let the controller script
            // handle the actions that occur (including resuming).
            BpInfo *bp;
            IfFailGo(GetCurrentBreakpoint(&bp));

            // If the breakpoint wasn't found, it could be because we executed a step. In
            // that case, pass in -1 so that the controller script can continue executing
            // its list of commands.
            LONG bpId = bp ? bp->breakpointId : -1;
            IfFailGo(m_pController->HandleBreakpoint(bpId));
        }
    }
    else if(HostConfigFlags::flags.Auto)
    {
        // Dump the breakpoint info.
        IfFailGo(DumpBreakpoint());

        // Enumerate and inspect locals.
        IfFailGo(InspectLocals());

        // Dump the callstack.
        IfFailGo(GetCallstack());

        // Un-set any automatic breakpoints that have hit their maximum limit.
        IfFailGo(HandleAutomaticBreakpointLogic());

        // Resume
        IfFailGo(ResumeFromBreakPoint(m_config.ResumeActionOnBreak(), m_config.GetErrorResumeAction()));
    }
    else if (HostConfigFlags::flags.DumpLocalsOnDebuggerBp)
    {
        // Enumerate and inspect locals.
        IfFailGo(InspectLocals());

        // Resume
        IfFailGo(ResumeFromBreakPoint(m_config.ResumeActionOnBreak(), m_config.GetErrorResumeAction()));
    }
    else
    {
        IfFailGo(ResumeFromBreakPoint(BREAKRESUMEACTION_CONTINUE, m_config.GetErrorResumeAction()));
    }

Error:
    DebuggerController::Log(_u("/Debugger::PerformOnBreak(): hr = 0x%08X\n"), hr);
    return hr;
}

HRESULT Debugger::DumpBreakpoint()
{
    HRESULT hr = S_OK;
    CComPtr<IEnumDebugStackFrames> enumFrames;
    CComPtr<IDebugCodeContext> codeContext;
    FrameDescriptor frameDescriptor;
    ULONG ct;
    Location location;
    std::wstring breakpoint;

    LPCWSTR reason = DebuggerController::GetBreakpointReason(m_eCurrentBreakReason);

    IfFailGo(m_spScriptThread->EnumStackFrames(&enumFrames));
    IfFailGo(enumFrames->Next(1, &frameDescriptor, &ct));
    IfFailGo(GetLocation(frameDescriptor, location));

    breakpoint += _u("{\"breakpoint\": {\"reason\": \"");
    breakpoint += reason;
    breakpoint += _u("\"");
    breakpoint += _u(", \"location\": ");
    breakpoint += location.ToString();
    breakpoint += _u("}}");

    IfFailGo(m_pController->LogBreakpoint(breakpoint.c_str()));

Error:
    return hr;
}

std::wstring Debugger::DumpSourceListInternal(CComPtr<IDebugApplicationNode> pRootNode, bool rootNode)
{
    std::wstring sourceString;
    std::wstring childrenString;
    CComBSTR m_spUrlBstr;
    CComPtr<IDebugDocument> debugDocument;
    CComPtr<IDebugDocumentText> documentText;
    std::wstring encodedText;

    if (!rootNode) 
    {
        if (S_OK != pRootNode->GetName(DOCUMENTNAMETYPE_UNIQUE_TITLE, &m_spUrlBstr))
        {
            if (S_OK != pRootNode->GetName(DOCUMENTNAMETYPE_APPNODE, &m_spUrlBstr))
            {
                AssertMsg(false, "Failed to get name of source node");
            }
        }
        else 
        {
            // Let's get some portion of text so that we can identify what this code is
            if (pRootNode->GetDocument(&debugDocument) == S_OK && debugDocument->QueryInterface(&documentText) == S_OK)
            {
                WCHAR* uniqueTitleText = new WCHAR[m_config.maxStringLengthToDump + 1];
                if (uniqueTitleText)
                {
                    uniqueTitleText[m_config.maxStringLengthToDump] = '\0';
                    ULONG ctRet = 0;
                    if (documentText->GetText(0, uniqueTitleText, NULL, &ctRet, m_config.maxStringLengthToDump) == S_OK)
                    {
                        if (ctRet <= m_config.maxStringLengthToDump)
                        {
                            uniqueTitleText[ctRet] = '\0';
                        }
                        CComBSTR uniqueTitleBstr(uniqueTitleText);
                        m_pController->EncodeString(uniqueTitleBstr, encodedText);
                    }
                    delete[] uniqueTitleText;
                }
            }
        }
    }
    else 
    {
        m_spUrlBstr = "Root";
    }

    sourceString += _u("{\"Name\": \"");
    sourceString += m_spUrlBstr;
    if (!encodedText.empty())
    {
        sourceString += _u("\",");
        sourceString += _u("\"Text\": \"") + encodedText;
    }
    sourceString += _u("\"");
    
    CComPtr<IEnumDebugApplicationNodes> spEnumDebugApp;
    HRESULT hr = pRootNode->EnumChildren(&spEnumDebugApp);
    if (hr == S_OK)
    {
        spEnumDebugApp->Reset();
        IDebugApplicationNode * pChildNode;
        ULONG nFetched;
        ULONG numChildren = 0;
        childrenString += _u("\"Children\": [");
        while (SUCCEEDED(spEnumDebugApp->Next(1, &pChildNode, &nFetched)) && (nFetched == 1))
        {
            if (numChildren++ > 0) 
            {
                childrenString += _u(",");
            }
            childrenString += DumpSourceListInternal(pChildNode);
            pChildNode->Release();
        }
        if (numChildren > 0) 
        {
            sourceString += _u(",") + childrenString + _u("]");
        }
    }
    sourceString += _u("}");
    return sourceString;
}

HRESULT Debugger::DumpSourceList()
{
    HRESULT hr = S_OK;
    CComPtr<IDebugApplicationNode> pRootNode;
    std::wstring sourceString;

    IfFailGo(m_spDebugApp->GetRootNode(&pRootNode));
    sourceString += _u("{\"SourceList\":");
    sourceString += DumpSourceListInternal(pRootNode, true);
    sourceString += _u("}");
    IfFailGo(m_pController->LogMessage(sourceString.c_str()));
    
Error:
    return hr;
}

HRESULT Debugger::GetCurrentBreakpoint(BpInfo **bpInfo)
{
    HRESULT hr = S_OK;
    *bpInfo = NULL;

    // Match the breakpoint if either (a) we hit a breakpoint, or (b) it's targeted mode and we stepped into a breakpoint
    if (m_eCurrentBreakReason == BREAKREASON_BREAKPOINT_EX || (HostConfigFlags::flags.Targeted && m_eCurrentBreakReason == BREAKREASON_STEP_EX))
    {
        CComPtr<IEnumDebugStackFrames> enumFrames;
        CComPtr<IDebugCodeContext> codeContext;
        FrameDescriptor frameDescriptor;
        ULONG ct;

        if (m_spScriptThread->EnumStackFrames(&enumFrames) == S_OK
            && enumFrames->Next(1, &frameDescriptor, &ct) == S_OK
            && frameDescriptor.pdsf->GetCodeContext(&codeContext) == S_OK)
        {
            SourceLocationInfo srcInfo;
            if (PopulateSourceLocationInfo(codeContext, &srcInfo) == S_OK && FindMatchingBp(&srcInfo))
            {
                *bpInfo = FindMatchingBp(&srcInfo);
            }
        }
        else
        {
            IfFailGo(E_FAIL);
        }
    }
Error:
    return hr;
}

HRESULT Debugger::HandleAutomaticBreakpointLogic()
{
    HRESULT hr = S_OK;
    BpInfo *bpInfo = NULL;
    IfFailGo(GetCurrentBreakpoint(&bpInfo));
    if (bpInfo != NULL)
    {
        bpInfo->hitCount++;
        // Remove the breakpoint if it exceed the hitcount
        if (bpInfo->hitCount > m_config.maxHitCountForABreakpoint)
        {
            //fwprintf(stdout, _u("Breakpoint hitcount has crossed the limit %d, the breakpoint removed\n"), m_config.maxHitCountForABreakpoint);
            IfFailGo(RemoveBreakpoint(bpInfo->breakpointId));
        }

    }

Error:
    return hr;
}

HRESULT Debugger::InspectLocals()
{
    // Enumerate and inspect locals
    return GetLocals(m_config.inspectionNestingLevel, DebugPropertyFlags::LOCALS_DEFAULT);
}

HRESULT Debugger::GetLocalsEnum(CComPtr<IDebugProperty>& pDebugProperty)
{
    HRESULT hr = S_OK;

    IfFailGo(EnsureCurrentFrame());
    IfFailGo(m_currentFrame->pdsf->GetDebugProperty(&pDebugProperty));

Error:
    return hr;
}

HRESULT Debugger::GetLocalsEnum(CComPtr<IEnumDebugPropertyInfo>& enumLocals)
{
    HRESULT hr = S_OK;

    CComPtr<IDebugProperty> pDebugProperty;
    IfFailGo(GetLocalsEnum(pDebugProperty));
    IfFailGo(pDebugProperty->EnumMembers(DBGPROP_INFO_ALL, 10, __uuidof(IEnumDebugPropertyInfo), &enumLocals));

Error:
    return hr;
}

HRESULT Debugger::GetLocals(int expandLevel, DebugPropertyFlags flags)
{
    HRESULT hr = S_OK;

    CComPtr<IDebugProperty> pDebugProperty;
    AutoDebugPropertyInfo info;
    IfFailGo(GetLocalsEnum(pDebugProperty));
    IfFailGo(pDebugProperty->GetPropertyInfo(DBGPROP_INFO_DEBUGPROP, 10, &info));

    IfFailGo(m_pController->DumpLocals(*this, info, expandLevel, flags, [&](std::wstring& json)
    {
        // In IE/VS F12 debugger evaluates "this" and adds it to locals list
        if (SUCCEEDED(EvaluateExpression(_u("this"), expandLevel, flags, json)))
        {
            json += _u(",");
        }

        return S_OK;
    }));

Error:
    return hr;
}

HRESULT Debugger::EvaluateExpression(std::wstring expression, int expandLevel, DebugPropertyFlags flags, std::wstring& json)
{
    if (expandLevel < 0)
    {
        return E_FAIL;
    }

    if (!IsAtBreak())
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    AutoDebugPropertyInfo debugPropertyInfo;
    IfFailGo(EvaluateExpressionAsDebugProperty(expression, flags, debugPropertyInfo));
    IfFailGo(m_pController->DumpProperty(*this, debugPropertyInfo, expandLevel, flags, json));

Error:
    return hr;
}

JsValueRef CALLBACK Debugger::JsEvaluateExpressionCompletion(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    JsValueRef retVal;
    CComPtr<CExprCallback> exprCallback;
    exprCallback.Attach(static_cast<CExprCallback*>(callbackState));    

    DebuggerController::Log(_u("Completing async expression eval..."));
    exprCallback->Complete();

    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    return retVal;
}

HRESULT Debugger::EvaluateExpressionAsync(std::wstring expression, int expandLevel, DebugPropertyFlags flags, std::wstring& json, JsValueRef* completion)
{
    HRESULT hr = S_OK;
    CComPtr<CExprCallback> exprCallback;
    exprCallback.Attach(new CExprCallback());       // CExprCallback is created with 1 ref count already. So just attach to the CComPtr

    ExpressionEvalRequest request = { this, expandLevel, flags, json };
    exprCallback->SetExpressionEvalRequest(request);
    DebuggerController::Log(_u("Evaluating expression async ..."));
    IfFailGo(EvaluateExpressionAsync(expression, exprCallback));
    JsrtCheckError(JScript9Interface::JsrtCreateFunction(Debugger::JsEvaluateExpressionCompletion, exprCallback.Detach(), completion));
Error:
    return hr;
}

HRESULT Debugger::EnsureFullNameEvaluationValueIsEquivalent(const DebugPropertyInfo& debugPropertyInfo, DebugPropertyFlags flags)
{
    HRESULT hr = S_OK;
    if (HostConfigFlags::flags.VerifyShortAndFullNameValues)
    {
        if (wcscmp(debugPropertyInfo.m_bstrFullName, debugPropertyInfo.m_bstrName) != 0 && SysStringLen(debugPropertyInfo.m_bstrName) > 0)
        {
            // If the short name doesn't match the full name, do a full name evaluation to see if the
            // resulting values are the same for both full and short name.
            std::wstring fullNameExpression = std::wstring(debugPropertyInfo.m_bstrFullName);
            AutoDebugPropertyInfo fullNameDebugPropertyInfo;
            IfFailGo(EvaluateExpressionAsDebugProperty(fullNameExpression, flags, fullNameDebugPropertyInfo));

            bool bothPropertiesAreReal = (debugPropertyInfo.m_dwAttrib & DBGPROP_ATTRIB_VALUE_IS_FAKE) == 0
                                      && (fullNameDebugPropertyInfo.m_dwAttrib & DBGPROP_ATTRIB_VALUE_IS_FAKE) == 0;

            // Exclude maps, sets, and weakmaps because any child properties are automatically resolved
            // to the map/set/weakmap variable's full name.
            bool isCollection = wcsstr(fullNameDebugPropertyInfo.m_bstrType, _u("Map")) != nullptr
                             || wcsstr(fullNameDebugPropertyInfo.m_bstrType, _u("Set")) != nullptr
                             || wcsstr(fullNameDebugPropertyInfo.m_bstrType, _u("WeakMap")) != nullptr
                             || wcsstr(fullNameDebugPropertyInfo.m_bstrType, _u("WeakSet")) != nullptr;

            // Exclude {error} and {exception} properties.
            bool isError = wcsstr(fullNameDebugPropertyInfo.m_bstrFullName, _u("{error}")) != nullptr
                        || wcsstr(fullNameDebugPropertyInfo.m_bstrFullName, _u("{exception}")) != nullptr;

            // When the error happens, during locals enumeration we populate the value as an Error object. But accessing that property as an eval, generates the error itself
            // instead of an Error object. So both comparison will not match.
            bool isErrorType = wcscmp(fullNameDebugPropertyInfo.m_bstrType, _u("Error")) == 0;

            if (bothPropertiesAreReal && !isCollection && !isError && !isErrorType)
            {
                Assert(wcscmp(debugPropertyInfo.m_bstrValue, fullNameDebugPropertyInfo.m_bstrValue) == 0);
            }
        }
    }

Error:
    return hr;
}

HRESULT Debugger::EvaluateExpressionAsDebugProperty(const std::wstring& expression, DebugPropertyFlags flags, DebugPropertyInfo& debugPropertyInfo)
{
    HRESULT hr = S_OK;

    CComPtr <IDebugExpressionContext> debugExpressionContext;
    CComPtr <IDebugExpression> debugExpression;
    CComPtr<IDebugProperty> debugProperty;

    CComPtr<CExprCallback> exprCallback;
    exprCallback.Attach(new CExprCallback());       // CExprCallback is created with 1 ref count already. So just attach to the CComPtr
    HRESULT returnResult;

    IfFailGo(EnsureCurrentFrame());

    // Expression context is related to the stack frame
    IfFailGo(m_currentFrame->pdsf->QueryInterface(__uuidof(IDebugExpressionContext),
        (LPVOID*)&debugExpressionContext
        ));

    // Load the expression text into the expression context
    IfFailGo(debugExpressionContext->ParseLanguageText(
        expression.c_str(),         // Expression
        10,                         // Radix
        _u(""),                        // Text delimiter
        DEBUG_TEXT_RETURNVALUE |    // Need return
        DEBUG_TEXT_ISEXPRESSION,
        &debugExpression            // Expression result
        ));

    IfFailGo(debugExpression->Start(exprCallback));
    exprCallback->WaitForCompletion();

    if (debugExpression->GetResultAsDebugProperty(&returnResult, &debugProperty) != S_OK || debugProperty == NULL)
    {
        // The expression (such as 'this') is not working for native frame as of now, ignore the result and bail out.
        return E_FAIL;
    }
    IfFailGo(returnResult);
    IfFailGo(debugProperty->GetPropertyInfo(DBGPROP_INFO_ALL, DebuggerController::GetRadix(flags), &debugPropertyInfo));

Error:
    return hr;
}

void OnCompleteExpressionEval(IDebugExpression* debugExpression, ExpressionEvalRequest& expressionEvalRequest) {
    HRESULT hr;
    HRESULT returnResult;
    AutoDebugPropertyInfo debugPropertyInfo;

    CComPtr<IDebugProperty> debugProperty;
    if (debugExpression->GetResultAsDebugProperty(&returnResult, &debugProperty) != S_OK || debugProperty == NULL)
    {
        // The expression (such as 'this') is not working for native frame as of now, ignore the result and bail out.
        hr = E_FAIL;
        goto Error;
    }

    IfFailGo(returnResult);
    IfFailGo(debugProperty->GetPropertyInfo(DBGPROP_INFO_ALL, 10, &debugPropertyInfo));
    IfFailGo(expressionEvalRequest.debugger->GetController()->DumpProperty(
        *expressionEvalRequest.debugger, debugPropertyInfo, expressionEvalRequest.expandLevel, expressionEvalRequest.flags, expressionEvalRequest.json));
    expressionEvalRequest.json += _u("}}");
    IfFailGo(expressionEvalRequest.debugger->GetController()->LogEvaluateExpression(expressionEvalRequest.json.c_str()));
Error:
    if (FAILED(hr))
    {
        DebuggerController::LogError(_u("OnCompleteExpressionEval"));
    }

}

HRESULT Debugger::EvaluateExpressionAsync(const std::wstring& expression, CExprCallback* callback)
{
    HRESULT hr = S_OK;

    CComPtr <IDebugExpressionContext> debugExpressionContext;
    CComPtr <IDebugExpression> debugExpression;

    IfFailGo(EnsureCurrentFrame());

    // Expression context is related to the stack frame
    IfFailGo(m_currentFrame->pdsf->QueryInterface(__uuidof(IDebugExpressionContext),
        (LPVOID*)&debugExpressionContext
        ));

    // Load the expression text into the expression context
    IfFailGo(debugExpressionContext->ParseLanguageText(
        expression.c_str(),         // Expression
        10,                         // Radix
        _u(""),                        // Text delimiter
        DEBUG_TEXT_RETURNVALUE |    // Need return
        DEBUG_TEXT_ISEXPRESSION,
        &debugExpression            // Expression result
        ));

    IfFailGo(debugExpression->Start(callback));

    callback->RegisterCompletionHandler((CExprCallback::CompletionDelegate)OnCompleteExpressionEval, debugExpression.Detach());

Error:
    return hr;
}

HRESULT Debugger::SetNextStatement(ULONG line, ULONG column)
{
    if (!IsAtBreak())
    {
        return E_FAIL;
    }

    // Setnext statement is allowed only on the top frame, so switch to it : TODO


    // Get current codecontext

    CComPtr<IEnumDebugStackFrames> enumFrames;
    CComPtr<IDebugCodeContext> codeContext;
    FrameDescriptor frameDescriptor;
    ULONG ct;

    HRESULT hr = E_FAIL;

    if (m_spScriptThread->EnumStackFrames(&enumFrames) == S_OK
        && enumFrames->Next(1, &frameDescriptor, &ct) == S_OK
        && frameDescriptor.pdsf->GetCodeContext(&codeContext) == S_OK)
    {
        SourceLocationInfo currentLocation, nextLocation;
        if (PopulateSourceLocationInfo(codeContext, &currentLocation) == S_OK)
        {
            nextLocation.scriptId = currentLocation.scriptId;
            nextLocation.lineNumber = line;
            nextLocation.columnNumber = column;
            nextLocation.contextCount = 1;

            CComPtr<IDebugCodeContext> nextCodeContext;
            if ((hr = GetDebugCodeContext(&nextLocation, &nextCodeContext, true)) == S_OK)
            {
                // Update the nextlocation info.
                PopulateSourceLocationInfo(nextCodeContext, &nextLocation);
                std::wstring json = _u("{\"setnextstatement\" : { ");

                json += _u("\"from\" : ");

                WCHAR buf[20];
                _itow_s(currentLocation.charPosition,buf,20,10);
                json += buf;

                json += _u(", \"to\" : ");
                _itow_s(nextLocation.charPosition,buf,20,10);
                json += buf;

                json += _u(", \"IsAllowed\" : ");

                CComPtr<ISetNextStatement> spNextStatement;
                hr = frameDescriptor.pdsf->QueryInterface(IID_ISetNextStatement, reinterpret_cast<void**>(&spNextStatement));
                if (hr != S_OK)
                {
                    json += _u("\"Not supported\"");
                }
                else
                {
                    hr = spNextStatement->SetNextStatement(frameDescriptor.pdsf, nextCodeContext);

                    if (hr == S_OK)
                    {
                        json += _u("\"Yes\"");
                    }
                    else
                    {
                        json += _u("\"No\"");
                    }
                }
                if (hr == S_OK)
                {
                    // Both old F12 and new F12 does ERRORRESUMEACTION_SkipErrorStatement
                    // inetcore\devtoolbar\jstools\debugger\JSDbgStackFrameInfo.cpp
                    //      HRESULT CJSDbgStackFrameInfo::SetNextStatement(IDebugCodeContext *pContext)
                    // inetcore\devtoolbar\v3\Host\DebuggerCore\DebuggerDispatch.cpp
                    //      STDMETHODIMP CDebuggerDispatch::setNextStatement(_In_ ULONG docId, _In_ ULONG position, _Out_ VARIANT_BOOL* pSuccess)
                    
                    hr = ResumeFromBreakPoint(BREAKRESUMEACTION_STEP_OVER, ERRORRESUMEACTION_SkipErrorStatement);
                }
                json += _u("}}");
                m_pController->LogSetNextStatement(json.c_str());
            }
        }
    }

    return hr;
}

HRESULT Debugger::GetNextFrameLocation(IEnumDebugStackFrames* enumFrames, Location& loc)
{
    HRESULT hr = S_OK;
    FrameDescriptor frameDescriptor;
    ULONG ct;
    AutoDebugPropertyInfo debugPropertyInfo;
    CComPtr<IDebugProperty> debugProperty;

    if( (hr = enumFrames->Next(1, &frameDescriptor, &ct)) != S_OK)
    {
        // Apparently we communicate "no more frames to enumerate" by returning 1,
        // so this may be expected and does not indicate an error.
        goto Error;
    }
    IfFailGo( GetLocation(frameDescriptor, loc) );
    IfFailGo( frameDescriptor.pdsf->GetDebugProperty(&debugProperty) );
    IfFailGo( debugProperty->GetPropertyInfo(DBGPROP_INFO_ALL, 10, &debugPropertyInfo) );
    loc.debugPropertyAttributes = debugPropertyInfo.m_dwAttrib;

Error:

    return hr;
}

HRESULT Debugger::GetLocation(IDebugCodeContext* codeContext, Location& location )
{
    HRESULT hr = S_OK;
    CComPtr<IDebugDocumentContext> docContext;
    CComPtr<IDebugDocument> doc;
    CComPtr<IDebugDocumentText> docText;
    ULONG ctRet = 0;
    WCHAR* buf = NULL;

    if (FAILED(codeContext->GetDocumentContext(&docContext)) || docContext == nullptr/*isLibraryCode*/)
    {
        ZeroMemory(&location, sizeof(Location));
        return S_OK;
    }
    IfFailGo( docContext->GetDocument(&doc) );
    IfFailGo( doc->QueryInterface(&docText) );

    ScriptDebugNodeSource* debugNodeSource = Debugger::FindSourceNode(docText);
    Assert(debugNodeSource != nullptr);

    location.docId = location.srcId = debugNodeSource->GetSourceId();

    IfFailGo( docText->GetPositionOfContext(docContext, &location.startChar, &location.length) );

    // Because of the some bug in the engine, we are not getting text from any other position then 0,
    // so put workaround to get the text fro the 0 and crop that buffer here itself.

    ULONG cChars = location.startChar + location.length;

    buf = new WCHAR[cChars+1];
    buf[cChars] = '\0';

    IfFailGo( docText->GetText(0, buf, NULL, &ctRet, cChars) );
    location.text = &buf[location.startChar];

    IfFailGo( docText->GetLineOfPosition(location.startChar, &location.lineNumber, &location.columnNumber) );

    // 1-index based
    location.lineNumber++;
    location.columnNumber++;

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
    CComBSTR tmp;

    IfFailGo( frameDescriptor.pdsf->GetCodeContext(&codeContext) );
    IfFailGo( GetLocation(codeContext, location) );

    IfFailGo( frameDescriptor.pdsf->GetDescriptionString(TRUE,&tmp) );
    location.frameDescription = tmp;

Error:
    return hr;
}

HRESULT Debugger::GetCallstack(LocationToStringFlags flags)
{
    HRESULT hr = S_OK;
    CComPtr<IEnumDebugStackFrames> enumFrames;
    std::wstring callstackEvent = _u("{\"callstack\" : [");
    std::wstring separator = _u("");

    IfFailGo( m_spScriptThread->EnumStackFrames(&enumFrames) );

    do
    {
        Location loc;
        hr = GetNextFrameLocation(enumFrames, loc);
        if (S_OK == hr)
        {
            callstackEvent += separator;
            callstackEvent += loc.ToString(flags);
            separator = _u(",");
        }
    } 
    while (hr == S_OK);

    callstackEvent += _u("]}");
    IfFailGo(m_pController->LogCallstack(callstackEvent.c_str()));

Error:

    return hr;
}

HRESULT Debugger::SetCurrentFrame(ULONG depth)
{
    HRESULT hr = S_OK;
    CComPtr<IEnumDebugStackFrames> enumFrames;
    ULONG ct;

    DebugStackFrameDescriptor rawFrameDescriptor = { 0 };

    IfFailGo(m_spScriptThread->EnumStackFrames(&enumFrames));

    // Get the N'th frame
    for (ULONG i = 0; i <= depth; ++i)
    {
        if (rawFrameDescriptor.pdsf)
        {
            rawFrameDescriptor.pdsf->Release();
            rawFrameDescriptor.pdsf = nullptr;
        }
        if (rawFrameDescriptor.punkFinal)
        {
            rawFrameDescriptor.punkFinal->Release();
            rawFrameDescriptor.punkFinal = nullptr;
        }        
        hr = enumFrames->Next(1, &rawFrameDescriptor, &ct);
        if (hr == S_FALSE && ct == 0)
        {
            return S_OK;  // Less number of frames then depth
        }
        if (hr != S_OK || ct != 1)
        {
            hr = E_FAIL;
            goto Error;
        }
    }
    // Delete the old frame pointers
    ClearCurrentFrame();

    // Take ownership of the frame.
    m_currentFrame = new FrameDescriptor(&rawFrameDescriptor);

Error:
    return hr;
}

HRESULT Debugger::EnsureCurrentFrame()
{
    HRESULT hr = S_OK;
    if(!m_currentFrame)
    {
        IfFailedReturn(SetCurrentFrame(0));
        Assert(m_currentFrame);
    }
    return hr;
}

void Debugger::ClearCurrentFrame()
{
    if(m_currentFrame)
    {
        delete m_currentFrame;
        m_currentFrame = nullptr;
    }
}

HRESULT Debugger::LogJson(LPCWSTR logString)
{
    HRESULT hr = S_OK;

    IfFailGo(m_pController->LogJson(logString));

Error:
    return hr;
}

HRESULT Debugger::LogJson(__in __nullterminated char16 *msg, ...)
{
    HRESULT hr = S_OK;
    va_list args;
    va_start(args, msg);
    char16 buf[2048];
    _vsnwprintf_s(buf, _countof(buf) - 1, _TRUNCATE, msg, args);
    StringCchCatW(buf, _countof(buf), _u("\0"));
    va_end(args);
    IfFailGo(m_pController->LogJson(buf));

Error:
    return hr;
}

HRESULT Debugger::AddNode(__in IDebugApplicationNode *pRootNode, ULONG ulContainerId)
{
    if (pRootNode == NULL || !IsAttached())
    {
        DebuggerController::LogError(_u("failed at Debugger::AddNode"));
        return E_FAIL;
    }

    EnsureRootNode();

    ScriptDebugNodeSource * pSourceNode = new ScriptDebugNodeSource();
    pSourceNode->AddRef();
    pSourceNode->Init(pRootNode, ulContainerId, CreateUniqueSourceNumber(), this);

    m_listScriptNodes.push_back(pSourceNode);

    // We need a filename so the controller knows what to use as a baseline.  The
    // 2nd source node should be the file that was passed in - use that as the base
    // filename.  If we ever support debugging multiple files from jshost, we'll
    // need to revisit this logic.
    if(CurrentNodeIsMainScriptFile())
    {
        BSTR longFilename = pSourceNode->GetUrl();
        WCHAR filename[_MAX_PATH];
        WCHAR ext[_MAX_EXT];
        _wsplitpath_s(longFilename, NULL, 0, NULL, 0, filename, _countof(filename), ext, _countof(ext));

        // test.js/.html
        m_filename = filename;
        m_filename += ext;
    }

    if (HostConfigFlags::flags.DebugDumpText)
    {
        TCHAR szShortName[255];
        GetShortNameFromUrl(pSourceNode->GetUrl(), szShortName, 255);
        DebuggerController::Log(_u("Debugger: Source added (AddNode) { url : %s, sourceId : %d }\n"), szShortName, pSourceNode->GetSourceId());
    }

    CComPtr<IEnumDebugApplicationNodes> spEnumDebugApp;
    HRESULT hr = pRootNode->EnumChildren(&spEnumDebugApp);

    if (hr == S_OK)
    {
        ULONG currentContainerId = pSourceNode->GetSourceId();

        spEnumDebugApp->Reset();

        IDebugApplicationNode * pChildNode;
        ULONG nFetched;

        while (SUCCEEDED(spEnumDebugApp->Next(1, &pChildNode, &nFetched)) && (nFetched == 1))
        {
            AddNode(pChildNode, currentContainerId);
            pChildNode->Release();
        }
    }

    IDebugDocumentText* documentText = pSourceNode->GetDocumentText();
    if (documentText)
    {
        ULONG cLine = 0, cNumChar = 0;
        if (documentText->GetSize(&cLine, &cNumChar) == S_OK)
        {
            if(cNumChar > 0)
            {
                this->OnInsertText(pSourceNode);
            }
        }
    }

    return S_OK;
}

HRESULT Debugger::RemoveNode(IDebugApplicationNode *pRootNode)
{
    if (pRootNode == NULL)
    {
        return E_UNEXPECTED;
    }

    CComPtr<IEnumDebugApplicationNodes> spEnumDebugApp;
    HRESULT hRes = pRootNode->EnumChildren(&spEnumDebugApp);

    if (hRes == S_OK)
    {
        spEnumDebugApp->Reset();

        IDebugApplicationNode * pChildNode;
        ULONG nFetched;

        while(SUCCEEDED(spEnumDebugApp->Next(1, &pChildNode, &nFetched)) && (nFetched == 1))
        {
            RemoveNode(pChildNode);
            pChildNode->Release();
        }
    }

    // Removing sources on the PDM thread as well (onRemoveChild).

    std::vector<ScriptDebugNodeSource *>::iterator it;

    for (it = m_listScriptNodes.begin(); it != m_listScriptNodes.end(); ++it)
    {
        if ((*it)->GetDebugApplicationNode() == pRootNode)
        {
            ScriptDebugNodeSource * pSourceNode = (*it);
            m_listScriptNodes.erase(it);
            pSourceNode->Release();
            return S_OK;
        }
    }

    return S_OK;
}

void Debugger::RemoveAllSourceNodes()
{
    std::vector<ScriptDebugNodeSource *>::iterator it;

    while (!m_listScriptNodes.empty())
    {
        ScriptDebugNodeSource * pSourceNode = m_listScriptNodes.back();
        m_listScriptNodes.pop_back();
        pSourceNode->DisconnectEventSinks();
        pSourceNode->Release();
    }

    m_listScriptNodes.clear();
}

ScriptDebugNodeSource * Debugger::FindSourceNode(ULONG ulSourceId)
{
    std::vector<ScriptDebugNodeSource *>::iterator it;

    for (it = m_listScriptNodes.begin(); it != m_listScriptNodes.end(); ++it)
    {
        if ((*it)->GetSourceId() == ulSourceId)
        {
            break;
        }
    }

    if (it != m_listScriptNodes.end()) 
    {
        return (*it);
    }

    return NULL;
}

ScriptDebugNodeSource * Debugger::FindSourceNode(IDebugDocumentText *pDebugDocumentText)
{
    std::vector<ScriptDebugNodeSource *>::iterator it;

    for (it = m_listScriptNodes.begin(); it != m_listScriptNodes.end(); ++it)
    {
        if ((*it)->GetDocumentText() == pDebugDocumentText)
        {
            break;
        }
    }

    if (it != m_listScriptNodes.end()) 
    {
        return (*it);
    }

    return NULL;
}

HRESULT Debugger::PopulateSourceLocationInfo(IDebugCodeContext *pDebugCodeContext, SourceLocationInfo *pSourceContext)
{
    if (pDebugCodeContext == NULL || pSourceContext == NULL)
    {
        return E_FAIL;
    }

    EnsureRootNode();

    CComPtr<IDebugDocumentContext> spDebugDocumentContext;
    HRESULT hr = pDebugCodeContext->GetDocumentContext(&spDebugDocumentContext);
    IDebugDocumentContext *pContext = NULL;
    pContext = spDebugDocumentContext;

    if (hr == S_OK)
    {
        CComPtr<IDebugDocument> spDebugDocument;
        hr = spDebugDocumentContext->GetDocument(&spDebugDocument);
        Assert(hr == S_OK);

        CComPtr<IDebugDocumentText> spDebugDocumentText;
        spDebugDocument->QueryInterface(__uuidof(IDebugDocumentText),(LPVOID*)&spDebugDocumentText);

        spDebugDocumentText->GetPositionOfContext(spDebugDocumentContext, &(pSourceContext->charPosition), &(pSourceContext->contextCount));
        spDebugDocumentText->GetLineOfPosition(pSourceContext->charPosition, &(pSourceContext->lineNumber), &(pSourceContext->columnNumber));

        ScriptDebugNodeSource *pNode = FindSourceNode(spDebugDocumentText);
        if (pNode)
        {
            pSourceContext->scriptId = pNode->GetSourceId();
        }
        else
        {
            Assert(false);
        }
    }
    return hr;
}


HRESULT  Debugger::GetBreakpointHelpers(ScriptDebugNodeSource * pNode,
                                        ULONG ulLineNumber,
                                        ULONG ulColumnNumber,
                                        ULONG ulCharPosCount,
                                        SourceLocationInfo *pSourceContext,
                                        IDebugCodeContext **ppDebugCodeContext)
{
    Assert(IsAttached());

    HRESULT hr = E_FAIL;

    if (pNode)
    {
        hr = pNode->GetDebugCodeContext(ulLineNumber, ulColumnNumber, ulCharPosCount, ppDebugCodeContext);
        if (hr == S_OK)
        {
            hr = PopulateSourceLocationInfo(*ppDebugCodeContext, pSourceContext);
        }
    }

    return hr;
}

BpInfo *  Debugger::FindMatchingBp(SourceLocationInfo * pSrcLocation)
{
    std::vector<BpInfo *>::iterator it;

    for (it = m_listBps.begin(); it != m_listBps.end(); ++it)
    {
        if ((*it)->sourceLocation.scriptId ==  pSrcLocation->scriptId
            && (*it)->sourceLocation.lineNumber ==  pSrcLocation->lineNumber
            && (*it)->sourceLocation.columnNumber ==  pSrcLocation->columnNumber)
        {
            break;
        }
    }

    if (it != m_listBps.end()) 
    {
        return (*it);
    }

    return NULL;
}

HRESULT Debugger::InsertBreakpoint(ULONG ulSourceId, ULONG ulLineNumber, ULONG ulColumnNumber, BREAKPOINT_STATE bpState, BpInfo * pCommittedBpInfo)
{
    return InsertBreakpoint(FindSourceNode(ulSourceId), ulLineNumber, ulColumnNumber, bpState, pCommittedBpInfo);
}

HRESULT Debugger::InsertBreakpoint(ScriptDebugNodeSource * pNode, ULONG ulLineNumber, ULONG ulColumnNumber, BREAKPOINT_STATE bpState, BpInfo * pCommittedBpInfo)
{
    SourceLocationInfo bpSrcContext;
    CComPtr<IDebugCodeContext> spDebugCodeContext;
    HRESULT hr = GetBreakpointHelpers(pNode, ulLineNumber, ulColumnNumber, 1, &bpSrcContext, &spDebugCodeContext);

    if (hr == S_OK)
    {
        BpInfo * pBreakpoint = FindMatchingBp(&bpSrcContext);
        if (pBreakpoint == NULL)
        {
            // Set the breakpoint
            pBreakpoint = new BpInfo();
            pBreakpoint->breakpointId = CreateUniqueBpNumber();
            pBreakpoint->sourceLocation = bpSrcContext;
            hr = spDebugCodeContext->SetBreakPoint(bpState);
            if (hr == S_OK)
            {
                pBreakpoint->bpState = bpState;
                m_listBps.push_back(pBreakpoint);

                // Log the bp
                DebuggerController::Log(_u("InsertBreakpoint : breakpointId : %d, breakpoint state : %s,\n\t\tlocation : {\n\t\t\tsourceId : %d,\n\t\t\tlinenumber : %d,\n\t\t\tcolumnnumber : %d,\n\t\t\tactualPos : %d,\n\t\t\tcontextlength : %d\n\t\t}\n"),
                    pBreakpoint->breakpointId, DebuggerController::GetBreakpointState(bpState), bpSrcContext.scriptId, bpSrcContext.lineNumber, bpSrcContext.columnNumber, bpSrcContext.charPosition, bpSrcContext.contextCount);

                if (bpSrcContext.scriptId != pNode->GetSourceId())
                {
                    // The bp asked doc and resultant doc should be same.
                    DebuggerController::LogError(_u("ERROR : InsertBreakpoint asked and resultant source difference"));
                }
            }
            else 
            {
                DebuggerController::LogError(_u("ERROR : InsertBreakpoint failed hr = 0x%08X"), hr);
            }
        }

        *pCommittedBpInfo = *pBreakpoint;
    }

    return hr;
}

HRESULT Debugger::ModifyBreakpoint(ULONG bpId, BREAKPOINT_STATE state)
{
    std::vector<BpInfo *>::iterator it;

    for (it = m_listBps.begin(); it != m_listBps.end(); ++it)
    {
        if ((*it)->breakpointId == bpId)
        {
            break;
        }
    }

    if (it == m_listBps.end()) 
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    if ((*it)->bpState != state)
    {
        CComPtr<IDebugCodeContext> spDebugCodeContext;
        hr = GetDebugCodeContext(&((*it)->sourceLocation), &spDebugCodeContext);
        if (hr == S_OK)
        {
            hr = spDebugCodeContext->SetBreakPoint(state);
            if (hr == S_OK)
            {
                DebuggerController::Log(_u("ModifyBreakpoint Succeeded : breakpointId : %d, breakpoint state : %s\n"), bpId, DebuggerController::GetBreakpointState(state));
                (*it)->bpState = state;
            }
            else
            {
                DebuggerController::LogError(_u("ModifyBreakpoint, no breakpoint found : %d\n"), bpId);
            }
        }
    }
    else 
    {
        DebuggerController::Log(_u("ModifyBreakpoint breakpointId : %d state is already set to %s, not modifying breakpoint\n"), (*it)->breakpointId, DebuggerController::GetBreakpointState(state));
    }

    return hr;
}

HRESULT Debugger::RemoveAllBreakpoint()
{
    std::vector<BpInfo *>::iterator it;

    for (it = m_listBps.begin(); it != m_listBps.end(); ++it)
    {
        CComPtr<IDebugCodeContext> spDebugCodeContext;
        HRESULT hr = GetDebugCodeContext(&((*it)->sourceLocation), &spDebugCodeContext);
        if (hr == S_OK)
        {
            hr = spDebugCodeContext->SetBreakPoint(BREAKPOINT_DELETED);
        }

        delete (*it);
    }

    m_listBps.clear();

    return S_OK;
}

HRESULT Debugger::RemoveBreakpoint(ULONG bpId)
{
    HRESULT hr = S_OK;
    IfFailGo(ModifyBreakpoint(bpId, BREAKPOINT_DELETED));

    for(auto it = m_listBps.begin(); it != m_listBps.end(); ++it)
    {
        if((*it)->breakpointId == bpId)
        {
            delete *it;
            m_listBps.erase(it);
            break;
        }
    }

Error:
    return hr;
}

HRESULT Debugger::GetDebugCodeContext(SourceLocationInfo *pSourceContext, IDebugCodeContext **ppDebugCodeContext, bool useLineColumn)
{
    if (pSourceContext != NULL && ppDebugCodeContext != NULL)
    {
        ScriptDebugNodeSource *pNode = FindSourceNode(pSourceContext->scriptId);
        if (pNode)
        {
            if (useLineColumn)
            {
                return pNode->GetDebugCodeContext(pSourceContext->lineNumber, pSourceContext->columnNumber, pSourceContext->contextCount, ppDebugCodeContext);
            }
            else
            {
                return pNode->GetDebugCodeContext(pSourceContext->charPosition, pSourceContext->contextCount, ppDebugCodeContext);
            }
        }
    }

    return E_FAIL;
}

bool Debugger::IsTestHarnessFile(ScriptDebugNodeSource *pNode)
{
    LPWSTR url = pNode->GetUrl();
    if(url)
    {
        LPWSTR searchStrs[] = { _u("newGlue.js"), _u("loggerglue.js") };
        for(int i = 0; i < _countof(searchStrs); ++i)
        {
            LPWSTR ptr = StrStrI(url, searchStrs[i]);
            if(ptr == url + wcslen(url) - wcslen(searchStrs[i]))
            {
                // It's one of the glue files, skip it.
                return true;
            }
        }
    }
    return false;
}

HRESULT Debugger::OnInsertText(ScriptDebugNodeSource * pNode)
{
    // This function is called on the PDM's thread, but we want to execute it
    // on the debugger thread.  Marshal the call and block on the result.
    Message<Debugger>::FunctionCallResult result;
    if (S_OK == m_message.AsyncCall(&Debugger::OnInsertText, pNode, &result))
    {
        result.BlockOnResult(INFINITE);
        return S_OK;
    }

    // When running using automatic breakpoints, we want to exclude the glue files.
    // The reason is that the nightlies load up large infrastructure scripts, which 
    // execute extremely slowly due to the large log files and numbers of breakpoints.  
    // At the same time, we want to allow eval'd code to have automatic breakpoints.
    if(HostConfigFlags::flags.Auto && !CurrentNodeIsMainScriptFile())
    {
        if(pNode && IsTestHarnessFile(pNode))
        {
            // skip it
            return S_OK;
        }
    }

    ULONG cLine = 0, cNumChar = 0;
    WCHAR *pszSrcText = NULL;
    HRESULT hr = S_OK;

    // Get the source code, line count, and total number of characters.
    if(HostConfigFlags::flags.Auto || HostConfigFlags::flags.Targeted || HostConfigFlags::flags.DumpLocalsOnDebuggerBp)
    {
        if(pNode)
        {
            IDebugDocumentText *pDocText = pNode->GetDocumentText();
            if (pDocText)
            {
                if (pDocText->GetSize(&cLine, &cNumChar) == S_OK)
                {
                    // Allocate buffer for source text and attributes
                    pszSrcText = (WCHAR *)::CoTaskMemAlloc((ULONG)((cNumChar +1) * sizeof(WCHAR)));
                    pszSrcText[cNumChar] = '\0';

                    if (pszSrcText != NULL)
                    {
                        ULONG cchSrcText = 0;
                        hr = pDocText->GetText(0, pszSrcText, NULL, &cchSrcText, cNumChar);
                    }
                }
            }
        }
    }

    if(HostConfigFlags::flags.Targeted || HostConfigFlags::flags.DumpLocalsOnDebuggerBp)
    {
        if(pNode)
        {
            if (this->m_canSetBreakpoints)
            {
                // Call the controller to add the desired breakpoints.
                IfFailGo(m_pController->AddSourceFile(pszSrcText, pNode->GetSourceId()));
                DebuggerController::Log(_u("Inserted targeted breakpoint.\n"));
            }
            else
            {
                // We need to track the breakpoint and defer setting until attach has occurred.
                pNode->SetParticipateInInsertBp(true);
                DebuggerController::Log(_u("Deferring target breakpoint set until after attach.\n"));
            }
        }
    }
    else if(HostConfigFlags::flags.Auto)
    {
        // Set the automatic breakpoints.
        // TODO: refactor this and eliminate code duplicated in InsertAutoBreakpoints()
        if (pNode && m_config.setBpEveryNthLine > 0)
        {
            for (ULONG i = 0; i < cLine; i += m_config.setBpEveryNthLine)
            {
                BpInfo bpInfo;
                if (InsertBreakpoint(pNode, i, 1, BREAKPOINT_ENABLED, &bpInfo)== S_OK)
                {
                    DebuggerController::Log(_u("Inserted automatic breakpoint.\n"));

                    // Adjust for the next bp position.
                    if (bpInfo.sourceLocation.lineNumber > i)
                    {
                        i = bpInfo.sourceLocation.lineNumber;
                    }
                }
                else
                {
                    DebuggerController::Log(_u("Deferring auto breakpoint set until after attach.\n"));
                    pNode->SetParticipateInInsertBp(true);
                    break;
                }
            }
        }
    }

    if(pszSrcText)
    {
        CoTaskMemFree(pszSrcText);
    }

Error:
    return hr;
}

// Inserts all of the auto breakpoints.  Needs to be called after attach of the debugger
// to ensure that the debug document information has been parsed.
void Debugger::InsertAutoBreakpoints()
{
    DebuggerController::Log(_u("Inserting auto breakpoints.\n"));
    Assert(this->m_canSetBreakpoints);

    for (ULONG i = 0; i < this->m_listScriptNodes.size(); ++i)
    {
        ScriptDebugNodeSource* debugNodeSource = this->m_listScriptNodes[i];
        Assert(debugNodeSource);
        if (!debugNodeSource->ShouldParticipateInInsertBp() || IsTestHarnessFile(debugNodeSource))
        {
            continue;
        }

        debugNodeSource->SetParticipateInInsertBp(false);

        IDebugDocumentText* debugDocumentText = debugNodeSource->GetDocumentText();
        if (debugDocumentText)
        {
            ULONG lineCount = 0, characterCount = 0;
            HRESULT hr  = debugDocumentText->GetSize(&lineCount, &characterCount);
            if (SUCCEEDED(hr))
            {
                for (ULONG j = 0; j < lineCount; j += m_config.setBpEveryNthLine)
                {
                    BpInfo bpInfo;

                    hr = InsertBreakpoint(debugNodeSource, j, 1, BREAKPOINT_ENABLED, &bpInfo);
                    if (SUCCEEDED(hr))
                    {
                        // Adjust for the next breakpoint position.
                        if (bpInfo.sourceLocation.lineNumber > j)
                        {
                            j = bpInfo.sourceLocation.lineNumber;
                        }
                    }
                }
            }
        }
    }
}

void Debugger::OnDebuggerAttachedCompleted()
{
    // If we have auto or targeted breakpoints that were set before attach, add them now.

    SetCanSetBreakpoints(true);
    if (HostConfigFlags::flags.Auto)
    {
        InsertAutoBreakpoints();
    }
    else
    {
        InsertTargetedBreakpoints();
    }
}

void Debugger::OnDebuggerDetachedCompleted()
{
    SetCanSetBreakpoints(false);

    // Preparing sources for the next attach (if that happens)
    for (ULONG i = 0; i < this->m_listScriptNodes.size(); ++i)
    {
        if (this->m_listScriptNodes[i])
        {
            this->m_listScriptNodes[i]->SetParticipateInInsertBp(true);
        }
    }

    ResetBpMap();

    m_listBps.clear();
}

HRESULT Debugger::ResetBpMap()
{
    // This need to happen on the application thread.
    Message<Debugger>::FunctionCallResult result;
    if (S_OK == m_message.AsyncCall(&Debugger::ResetBpMap, &result))
    {
        return result.BlockOnResult(INFINITE);
    }

    // the dbgcontroller.js logic relies on this being start from beginning.s
    this->m_ulBpNumber = 0;
    return m_pController->ResetBpMap();
}

void Debugger::AddSourceForBreakpoint(ScriptDebugNodeSource *node)
{
    Assert(node);

    ULONG lineCount;
    ULONG characterCount;

    HRESULT hr = S_OK;

    WCHAR* sourceText = NULL;
    IDebugDocumentText *pDocText = node->GetDocumentText();
    if (pDocText)
    {

        if (pDocText->GetSize(&lineCount, &characterCount) == S_OK)
        {
            // Allocate buffer for source text and attributes
            sourceText = (WCHAR *)::CoTaskMemAlloc((ULONG)((characterCount +1) * sizeof(WCHAR)));
            sourceText[characterCount] = '\0';

            if (sourceText != NULL)
            {
                ULONG cchSrcText = 0;
                hr = pDocText->GetText(0, sourceText, NULL, &cchSrcText, characterCount);
            }
        }
    }

    if (FAILED(hr))
    {
        DebuggerController::Log(_u("Failed to insert breakpoint after deferring setting after attach (couldn't get the source document text from the node).\n"));
        if (sourceText)
        {
            CoTaskMemFree(sourceText);
        }

        return;
    }

    Assert(m_pController);
    hr = m_pController->AddSourceFile(sourceText, node->GetSourceId());

    Assert(sourceText);
    CoTaskMemFree(sourceText);

    if (FAILED(hr))
    {
        DebuggerController::Log(_u("Failed to insert breakpoint after deferring setting after attach (couldn't add the source file to the controller).\n"));
    }
}

// Inserts all of the targeted breakpoints.  Needs to be called after attach of the debugger
// to ensure that the debug document information has been parsed.
HRESULT Debugger::InsertTargetedBreakpoints()
{
    // This function is called on the main thread, but we want to execute it
    // on the debugger thread.  Marshal the call and block on the result.
    Message<Debugger>::FunctionCallResult result;
    if (S_OK == m_message.AsyncCall(&Debugger::InsertTargetedBreakpoints, &result))
    {
        return result.BlockOnResult(INFINITE);
    }

    DebuggerController::Log(_u("Inserting deferred breakpoints.\n"));
    Assert(this->m_canSetBreakpoints);

    // 0th index for 'JShost Application' which is just parent nothing in it.
    for (ULONG i = 1; i < this->m_listScriptNodes.size(); ++i)
    {
        ScriptDebugNodeSource *node = this->m_listScriptNodes[i];
        if (node->ShouldParticipateInInsertBp())
        {
            // Reset current node, so that will not add again.
            node->SetParticipateInInsertBp(false);
            AddSourceForBreakpoint(node);
        }
    }

    return S_OK;
}

FrameDescriptor::FrameDescriptor()
{
    pdsf = NULL;
    punkFinal = NULL;
}

FrameDescriptor::FrameDescriptor(DebugStackFrameDescriptor *src) : DebugStackFrameDescriptor(*src)
{
}

FrameDescriptor::~FrameDescriptor()
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

JsValueRef CALLBACK Debugger::JsInsertBreakpoint(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsInsertBreakpoint(ULONG srcId, ULONG lineNum, ULONG colNum, ULONG breakpointState);
    //

    HRESULT hr = S_OK;
    JsValueRef retVal = nullptr;
    BpInfo bp;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 5)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    ULONG args[4];
    for(int i = 1; i < argumentCount; i ++)
    {
        double tmp;
        JsValueRef numValue;
        JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[i], &numValue));
        JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
        args[i-1] = (ULONG)tmp;
    }

    // Call the real InsertBreakpoint
    hr = pDebugger->InsertBreakpoint(args[0], args[1], args[2], (BREAKPOINT_STATE)args[3], &bp);

    IfFailedGoLabel_NoLog(hr, Error);

    // Generate the return value, the breakpoint ID
    JsrtCheckError(JScript9Interface::JsrtDoubleToNumber(bp.breakpointId, &retVal));

Error:
    if(retVal == nullptr)
    {
        // Something went wrong - attempt to just return undefined.
        JScript9Interface::JsrtGetUndefinedValue(&retVal);
    }

    return retVal;
}

JsValueRef CALLBACK Debugger::JsModifyBreakpoint(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsModifyBreakpoint(ULONG bpId, ULONG state);
    //

    HRESULT hr = S_OK;
    JsValueRef retVal = nullptr;
    BpInfo bp;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 3)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    ULONG args[2];
    for(int i = 1; i < 3; i ++)
    {
        double tmp;
        JsValueRef numValue;
        JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[i], &numValue));
        JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
        args[i-1] = (int)tmp;
    }

    // Call the real ModifyBreakpoint
    IfFailGo(pDebugger->ModifyBreakpoint(args[0], (BREAKPOINT_STATE)args[1]));

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsModifyBreakpoint"));
    }

    JScript9Interface::JsrtGetUndefinedValue(&retVal);

    return retVal;
}

JsValueRef CALLBACK Debugger::JsDumpLocals(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsDumpLocals(ULONG expandLevel);
    //

    HRESULT hr = S_OK;
    JsValueRef retVal;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 3)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    ULONG expandLevel = 0;

    double tmp;
    JsValueRef numValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[1], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    expandLevel = (ULONG)tmp;

    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[2], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    DebugPropertyFlags flags = static_cast<DebugPropertyFlags>(static_cast<ULONG>(tmp));

    // Call the real GetLocals
    IfFailGo(pDebugger->GetLocals(expandLevel, flags));

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsDumpLocals"));
    }

    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    return retVal;
}

JsValueRef CALLBACK Debugger::JsSetFrame(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsSetFrame(ULONG depth);
    //

    HRESULT hr = S_OK;
    JsValueRef retVal;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 2)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    ULONG depth = 0;
    Assert(argumentCount == 2);

    double tmp;
    JsValueRef numValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[1], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    depth = (ULONG)tmp;

    // Call the internal API
    IfFailGo(pDebugger->SetCurrentFrame(depth));

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsSetFrame"));
    }

    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    return retVal;
}

JsValueRef CALLBACK Debugger::JsDumpCallstack(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsDumpCallstack();
    //

    HRESULT hr = S_OK;
    JsValueRef retVal;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 2)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    LocationToStringFlags flags = LTSF_None;

    double tmp;
    JsValueRef numValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[1], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    flags = static_cast<LocationToStringFlags>(static_cast<ULONG>(tmp));

    // Call the real GetCallstack
    IfFailGo(pDebugger->GetCallstack(flags));

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsDumpCallstack"));
    }

    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    return retVal;
}

JsValueRef CALLBACK Debugger::JsSetNextStatement(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsSetNextStatement(ULONG line, ULONG column);
    //

    HRESULT hr = S_OK;
    JsValueRef retVal;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 3)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    ULONG line = 0;
    ULONG column = 0;

    double tmp;
    JsValueRef numValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[1], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    line = (ULONG)tmp;

    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[2], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    column = (ULONG)tmp;

    // Perform the setnextstatement
    hr = pDebugger->SetNextStatement(line, column);

Error:
    if (hr == S_OK)
    {
        JScript9Interface::JsrtGetTrueValue(&retVal);
    }
    else 
    {
        JScript9Interface::JsrtGetFalseValue(&retVal);
    }
    return retVal;
}

JsValueRef CALLBACK Debugger::JsEvaluateExpression(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsEvaluateExpression(std::wstring & expression, int expandLevel, VARIANT_BOOL checkOrder);
    //

    return JsEvaluateExpression_Internal(callee, isConstructCall, arguments, argumentCount);
}

JsValueRef CALLBACK Debugger::JsEvaluateExpressionAsync(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    return JsEvaluateExpression_Internal(callee, isConstructCall, arguments, argumentCount, /*isAsync*/ true);
}

JsValueRef CALLBACK Debugger::JsEvaluateExpression_Internal(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, bool isAsync)
{
    HRESULT hr = S_OK;
    JsValueRef retVal;
    void *data;

    std::wstring expression;
    LPCWSTR expressionTmp;
    size_t length;
    std::wstring json = _u("{\"evaluate\" : {");

    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));
   
    Debugger *pDebugger = (Debugger*)data;

    if (argumentCount != 4)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;

    ULONG expandLevel = 0;

    double tmp;
    JsValueRef numValue;
    JsValueRef strValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToString(arguments[1], &strValue));
    JsrtCheckError(JScript9Interface::JsrtStringToPointer(strValue, &expressionTmp, &length));
    expression = expressionTmp;

    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[2], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    expandLevel = (ULONG)tmp;

    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[3], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    DebugPropertyFlags flags = static_cast<DebugPropertyFlags>(static_cast<ULONG>(tmp));

    if (!isAsync)
    {
        IfFailGo(pDebugger->EvaluateExpression(expression, expandLevel, flags, json));
        json += _u("}}");
        IfFailGo(pDebugger->m_pController->LogEvaluateExpression(json.c_str()));
    }
    else
    {
        IfFailGo(pDebugger->EvaluateExpressionAsync(expression, expandLevel, flags, json, &retVal));
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsEvaluateExpression_Internal"));
    }

    return retVal;
}

JsValueRef CALLBACK Debugger::JsDumpBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = S_OK;
    JsValueRef retVal;
    void *data;
    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;
    hr = pDebugger->DumpBreakpoint();

Error:
    if (FAILED(hr))
    {
        DebuggerController::LogError(_u("JsDumpBreakpoint"));
    }

    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    return retVal;
}

JsValueRef CALLBACK Debugger::JsDumpSourceList(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = S_OK;
    JsValueRef retVal;
    void *data;
    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;
    hr = pDebugger->DumpSourceList();

Error:
    if (FAILED(hr))
    {
        DebuggerController::LogError(_u("JsDumpSourceList"));
    }

    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    return retVal;
}


JsValueRef CALLBACK Debugger::JsResumeFromBreakpoint(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsResumeFromBreakpoint(LPCWSTR resumeAction);
    //

    HRESULT hr = S_OK;

    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 2)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    LPCWSTR resumeAction;
    size_t length;
    JsValueRef strValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToString(arguments[1], &strValue));
    JsrtCheckError(JScript9Interface::JsrtStringToPointer(strValue, &resumeAction, &length));

    Assert(length == wcslen(resumeAction));

    BREAKRESUMEACTION action = BREAKRESUMEACTION_ABORT;

    if(!wcscmp(resumeAction, _u("continue")))
        action = BREAKRESUMEACTION_CONTINUE;
    else if(!wcscmp(resumeAction, _u("step_into")))
        action = BREAKRESUMEACTION_STEP_INTO;
    else if(!wcscmp(resumeAction, _u("step_over")))
        action = BREAKRESUMEACTION_STEP_OVER;
    else if(!wcscmp(resumeAction, _u("step_out")))
        action = BREAKRESUMEACTION_STEP_OUT;
    else if(!wcscmp(resumeAction, _u("step_document")))
        action = BREAKRESUMEACTION_STEP_DOCUMENT;
    else
    {
        DebuggerController::LogError(_u("invalid BREAKRESUMEACTION: \"%s\""), resumeAction);
        IfFailGo(E_FAIL);
    }

    // Call the real ResumeFromBreakpoint
    IfFailGo(pDebugger->ResumeFromBreakPoint(action, pDebugger->m_defaultErrorAction));


Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsInsertBreakpoint"));
    }

    JsValueRef undef;
    JScript9Interface::JsrtGetUndefinedValue(&undef);

    return undef;
}

JsValueRef CALLBACK Debugger::JsLogJson(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsLogJson();
    //

    HRESULT hr = S_OK;
    JsValueRef retVal;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 2)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    LPCWSTR logString;
    size_t length;
    JsValueRef strValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToString(arguments[1], &strValue));
    JsrtCheckError(JScript9Interface::JsrtStringToPointer(strValue, &logString, &length));

    Assert(length == wcslen(logString));

    // Call the real LogJson
    IfFailGo(pDebugger->LogJson(logString));

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsLogJson"));
    }

    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    return retVal;
}

JsValueRef CALLBACK Debugger::JsSetExceptionResume(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsSetExceptionResume(LPCWSTR resumeAction);
    //

    HRESULT hr = S_OK;

    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 2)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    LPCWSTR resumeAction;
    size_t length;
    JsValueRef strValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToString(arguments[1], &strValue));
    JsrtCheckError(JScript9Interface::JsrtStringToPointer(strValue, &resumeAction, &length));

    Assert(length == wcslen(resumeAction));

    ERRORRESUMEACTION action = ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller;
    if(!wcscmp(resumeAction, _u("ignore")))
        action = ERRORRESUMEACTION_SkipErrorStatement;
    else if(!wcscmp(resumeAction, _u("break")))
        action = ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller;
    else
    {
        DebuggerController::LogError(_u("invalid ERRORRESUMEACTION"));
        IfFailGo(E_FAIL);
    }

    // Set the default exception action.
    pDebugger->m_defaultErrorAction = action;

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsSetExceptionResume"));
    }

    JsValueRef undef;
    JScript9Interface::JsrtGetUndefinedValue(&undef);

    return undef;
}

JsValueRef CALLBACK Debugger::JsSetDebuggerOptions(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsSetDebuggerOptions(SCRIPT_DEBUGGER_OPTIONS mask, BOOL value);
    //

    HRESULT hr = S_OK;

    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if(argumentCount != 3)
    {
        hr = E_FAIL;
        goto Error;
    }

    // Marshal the arguments;
    SCRIPT_DEBUGGER_OPTIONS mask;
    BOOL value;
    double tmp;
    JsValueRef numValue;

    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[1], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    mask = static_cast<SCRIPT_DEBUGGER_OPTIONS>(static_cast<ULONG>(tmp));

    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arguments[2], &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    value = tmp == 0.0 ? FALSE : TRUE;

    IfFailGo(pDebugger->SetDebuggerOptions(mask, value));

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsSetDebuggerOptions"));
    }

    JsValueRef undef;
    JScript9Interface::JsrtGetUndefinedValue(&undef);

    return undef;
}

HRESULT Debugger::ArgToULONG(JsValueRef arg, ULONG* pValue)
{
    HRESULT hr = S_OK;

    double tmp;
    JsValueRef numValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(arg, &numValue));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
    *pValue = (ULONG)tmp;

Error:
    return hr;
}

HRESULT Debugger::ArgToString(JsValueRef arg, PCWSTR* pValue, size_t* pLength)
{
    HRESULT hr = S_OK;

    JsValueRef strValue;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToString(arg, &strValue));
    JsrtCheckError(JScript9Interface::JsrtStringToPointer(strValue, pValue, pLength));

Error:
    return hr;
}

JsValueRef CALLBACK Debugger::JsRecordEdit(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // RecordEdit(ULONG srcId, string label, ULONG startOffset, ULONG length, string content)
    //

    HRESULT hr = S_OK;
    JsValueRef retVal = nullptr;
    BpInfo bp;
    void *data;
    const char16* editLabel;
    const char16* editContent;
    size_t length;
    CComPtr<IDebugDocumentText> spDebugDocumentText;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    Debugger *pDebugger = (Debugger*)data;

    if (argumentCount != 6)
    {
        hr = E_FAIL;
        goto Error;
    }

    int argIndex = 1;

    // Marshal the arguments;
    {
        ULONG srcId;
        IfFailGo(ArgToULONG(arguments[argIndex++], &srcId));

        ScriptDebugNodeSource* sourceNode = pDebugger->FindSourceNode(srcId);
        if (!sourceNode)
        {
            IfFailGo(E_FAIL);
        }
        spDebugDocumentText = sourceNode->GetDocumentText();
    }

    IfFailGo(ArgToString(arguments[argIndex++], &editLabel, &length));

    ULONG args[2];
    for (int i = 0; i < 2; i++)
    {
        IfFailGo(ArgToULONG(arguments[argIndex++], &args[i]));
    }

    IfFailGo(ArgToString(arguments[argIndex++], &editContent, &length));

    bool addSucceed = DiagnosticsHelper::GetDiagnosticsHelper()->AddEditRangeAndContent(editLabel, spDebugDocumentText, args[0], args[1], editContent, static_cast<ULONG>(length));
    if (!addSucceed)
    {
        DebuggerController::LogError(_u("JsRecordEdit"));
    }

Error:
    JScript9Interface::JsrtGetUndefinedValue(&retVal);
    return retVal;
}

bool Debugger::HasMutationBreakpointWithId(wstring strId)
{
    return m_mutationBreakpointMap.find(strId) != m_mutationBreakpointMap.end();
}

HRESULT Debugger::InsertMutationBreakpoint(wstring strId, IMutationBreakpoint *mutationBreakpoint)
{
    if (HasMutationBreakpointWithId(strId))
    {
        DebuggerController::LogError(_u("InsertMutationBreakpoint: duplicate breakpoint id."));
        return E_FAIL;
    }

    m_mutationBreakpointMap[strId] = mutationBreakpoint;
    return S_OK;
}

HRESULT Debugger::DeleteMutationBreakpoint(wstring strId)
{
    auto result = m_mutationBreakpointMap.find(strId);
    if (result != m_mutationBreakpointMap.end())
    {
        // Delete breakpoint object
        IMutationBreakpoint *bp = result->second;
        // Notify runtime for deletion
        bp->Delete();
        // Decrement ref count of delegate object
        bp->Release();
        // Remove the result from the map
        m_mutationBreakpointMap.erase(result);

        // Erase all entries associated with the same object but different strId
        for (auto it = m_mutationBreakpointMap.begin(); it != m_mutationBreakpointMap.end();)
        {
            if (it->second == bp)
            {
                m_mutationBreakpointMap.erase(it++);
                bp->Release();
                continue;
            }
            ++it;
        }
        return S_OK;
    }

    DebuggerController::LogError(_u("DeleteMutationBreakpoint: unable to find property"));

    // No runtime call return failed HR, not finding a bp is user error, just log it and return ok.
    return S_OK;
}

void Debugger::RemoveAllMutationBreakpoint()
{    
    for (auto it = m_mutationBreakpointMap.begin(); it != m_mutationBreakpointMap.end();)
    {
        IMutationBreakpoint * bp = it->second;
        m_mutationBreakpointMap.erase(it++);         
        bp->Release();
    }
}

static bool IsGlobalsPropertyInfo(const AutoDebugPropertyInfo& info)
{
    const char16 *name = info.m_bstrName;
    return wcscmp(name, _u("[Globals]")) == 0;
}

static bool IsScopePropertyInfo(const AutoDebugPropertyInfo& info)
{
    const char16 *name = info.m_bstrName;
    return wcsncmp(name, _u("[Scope]"), 7) == 0;
}

static MutationType MutationTypeFromString(wstring typeStr)
{
    if (typeStr == _u("update"))
    {
        return MutationTypeUpdate;
    }
    else if (typeStr == _u("delete"))
    {
        return MutationTypeDelete;
    }
    else if (typeStr == _u("add"))
    {
        return MutationTypeAdd;
    }
    else if (typeStr == _u("all"))
    {
        return MutationTypeAll;
    }
    else if (typeStr == _u("none"))
    {
        return MutationTypeNone;
    }
    AssertMsg(false, "Unknown mutation type.");
    return MutationTypeNone;
}

static void tokenizeNames(__in_z char16 *name, __in_z const char16 * delim, vector<wstring>& names)
{
    char16 *tok = nullptr;
    char16 *context = nullptr;
    
    tok = wcstok_s(name, delim, &context);
    while (tok != nullptr)
    {
        names.push_back(wstring(tok));
        tok = wcstok_s(nullptr, delim, &context);
    }
}

IDebugProperty * Debugger::FindDebugProperty(IEnumDebugPropertyInfo *enumLocals, const vector<wstring>& names, int pos)
{
    // Check for empty/invalid property name input
    if (pos < 0 || enumLocals == NULL
        || names.size() <= (size_t)pos)
    {
        return nullptr;
    }

    HRESULT hr = S_OK;
    ULONG ct;
    ULONG propCount = 0;    
    CComPtr<IEnumDebugPropertyInfo> nextEnumLocals = NULL;
    IfFailGo(enumLocals->GetCount(&propCount));

    for (ULONG i = 0; i < propCount; i++)
    {
        AutoDebugPropertyInfo info;
        // Enumerate over every info
        enumLocals->Next(1, &info, &ct);

        std::wstring strName((const char16*)info.m_bstrName);

        // Name matches
        if (strName == names[pos])
        {
            // Increment position; check if we found the property
            if ((size_t)++pos == names.size())
            {
                IDebugProperty *debugProperty = info.m_pDebugProp;
                // null out the m_pDebugProp to take its reference;
                info.m_pDebugProp = nullptr;
                return debugProperty;
            }
            else if ((info.m_dwAttrib & DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE) == DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE)
            {
                // Get next enum locals
                info.m_pDebugProp->EnumMembers(DBGPROP_INFO_ALL, 10, __uuidof(IEnumDebugPropertyInfo), &nextEnumLocals);
                return FindDebugProperty(nextEnumLocals, names, pos);
            }
            // Value matches so far, but cannot expand anymore! - wrong path!
        }
        
        // Exhaustively search for a match from [Globals] and [Scope] if not specified in the name
        bool isScope = IsScopePropertyInfo(info);
        if (isScope || IsGlobalsPropertyInfo(info))
        {
            // Get next enum locals
            nextEnumLocals = NULL; // CComPtr::operator & will assert if non-NULL
            info.m_pDebugProp->EnumMembers(DBGPROP_INFO_ALL, 10, __uuidof(IEnumDebugPropertyInfo), &nextEnumLocals);

            IDebugProperty *debugProperty = nullptr;
            // [Scope] - just search it from beginning
            if (isScope)
            {
                debugProperty = FindDebugProperty(nextEnumLocals, names, 0);
            }
            // !m_exhaustiveSearch && [Globals] - turn on m_exhaustiveSearch
            else if (!m_exhaustiveSearch)
            {
                m_exhaustiveSearch = true;
                debugProperty = FindDebugProperty(nextEnumLocals, names, 0);
            }

            if (debugProperty)
            {
                return debugProperty;
            }
        }
    }
Error:
    return nullptr;
}

HRESULT Debugger::SetMutationBreakpoint(const vector<wstring>& names, bool setOnObject, MutationType type, wstring strId)
{
    HRESULT hr = S_OK;
    CComPtr<IEnumDebugPropertyInfo> enumLocals;
    CComPtr<IDebugProperty> dbgProperty;
    CComPtr<IDebugPropertyObjectMutation> dbgPropertyOM;
    CComPtr<IMutationBreakpoint> bp;

    // Find the DebugProperty by Enumerate over names
    IfFailGo(GetLocalsEnum(enumLocals));

    // Find matching debug property
    dbgProperty.Attach(FindDebugProperty(enumLocals, names, 0));
    IfFailGo(hr = dbgProperty ? S_OK : E_FAIL);
    m_exhaustiveSearch = false; // Reset state

    // QueryInterface for IDebugPropertyObjectMutation
    IfFailGo(dbgProperty.QueryInterface(&dbgPropertyOM));

    BOOL canSetBreak = false;
    IfFailGo(dbgPropertyOM->CanSetMutationBreakpoint(setOnObject, type, &canSetBreak));

    if (canSetBreak) 
    {
        IfFailGo(dbgPropertyOM->SetMutationBreakpoint(setOnObject, type, &bp));

        DebuggerController::Log(_u("Debugger SetMutationBreakpoint : Set mutation breakpoint setOnObject %d, type %d, ID %s"), setOnObject, type, strId.c_str());

        // Add bp to debugger->m_mutationBreakpointList
        if (!HasMutationBreakpointWithId(strId))
        {
            hr = InsertMutationBreakpoint(wstring(strId), bp.Detach());
        }
    }
    else 
    {
        LogJson(_u("Cannot set mutation breakpoint setOnObject %d, type %d, ID %s"), setOnObject, type, strId.c_str());
        DebuggerController::Log(_u("Debugger SetMutationBreakpoint : Can't set mutation breakpoint setOnObject %d, type %d, ID %s"), setOnObject, type, strId.c_str());
    }

    AssertMsg(SUCCEEDED(hr), "SetMutationBreakpoint: List of breakpoints on debugger out of sync with runtime");
Error:
    return hr;
}

JsValueRef CALLBACK Debugger::JsSetMutationBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = S_OK;
    JsValueRef retVal = nullptr;
    void * data = nullptr;

    // name: fully qualified name to object/property
    const char16 *name = nullptr;
    size_t nameLen = 0;
    
    // setOnObject: whether setting on whole object (setOnObject == true) or individual property
    bool setOnObject = false;
    // type: mutation type string
    const char16 *propertiesOrValue = nullptr;
    size_t propertiesOrValueLen = 0;
    
    
    // type: mutation type string
    const char16 *type = nullptr;
    size_t typeLen = 0;
    
    // strId: (unique) string id for user to reference a breakpoint
    const char16 *strId = nullptr;
    size_t strIdLen = 0;

    vector<wstring> names;
    MutationType mutationType = MutationTypeNone;
    
    // Get parameters
    IfFailGo(ArgToString(arguments[1], &name, &nameLen));
    IfFailGo(ArgToString(arguments[2], &propertiesOrValue, &propertiesOrValueLen));
    IfFailGo(ArgToString(arguments[3], &type, &typeLen));
    IfFailGo(ArgToString(arguments[4], &strId, &strIdLen));

    if (0 == _wcsicmp(propertiesOrValue, _u("properties"))) 
    {
        setOnObject = true;
    }
    else if (0 == _wcsicmp(propertiesOrValue, _u("value")))
    {
        setOnObject = false;
    }
    else 
    {
        DebuggerController::LogError(_u("Invalid 2nd argument passed to mbp. Valid values are 'properties' and 'value'. Passed value %s"), propertiesOrValue);
    }

    // Tokenize names; split by '.', then fill into vector
    tokenizeNames(const_cast<char16 *>(name), _u("."), names);
    
    mutationType = MutationTypeFromString(wstring(type));

    // Get debugger
    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));
    Debugger *debugger = (Debugger*)data;

    // Call internal set method
    IfFailGo(hr = debugger->SetMutationBreakpoint(names, setOnObject, mutationType, wstring(strId)));
Error:
    return retVal;
}

JsValueRef CALLBACK Debugger::JsDeleteMutationBreakpoint(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = S_OK;
    JsValueRef retVal = nullptr;
    void * data = nullptr;

    // Get debugger
    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));
    Debugger *debugger = (Debugger*)data;

    // Get strId
    const char16 *strId = nullptr;
    size_t strIdLen = 0;
    IfFailGo(ArgToString(arguments[1], &strId, &strIdLen));

    // Remove mutation breakpoint
    hr = debugger->DeleteMutationBreakpoint(wstring(strId));

    AssertMsg(SUCCEEDED(hr), "JsDeleteMutationBreakpoint: List of breakpoints on debugger out of sync with runtime");
Error:
    return retVal;
}