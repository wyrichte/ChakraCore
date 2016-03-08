//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

#include <initguid.h>
#include "guids.h"

SimpleDebugger::SimpleDebugger()
    : m_quiet(false),
    m_initialized(false), 
    m_pController(nullptr),
    m_projectionCallBreakPointId(INVALID_BREAKPOINT_ID),
    docIdCount(0), 
    bpCount(0), 
    m_threadId(0),
    m_defaultErrorAction(ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller),
    m_scriptDebuggerOptions(SDO_NONE),
    m_scriptDebuggerOptionsValue(FALSE)
{
    HRESULT hr = S_OK;

    if(g_autoBreakpoints || g_targetedTest)
    {
        m_pController = new DebuggerController(g_dbgBaselineFilename);

        // Install the callbacks for the controller.
        IfFailGo(m_pController->InstallHostCallback(_u("InsertBreakpoint"), &SimpleDebugger::JsInsertBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("ModifyBreakpoint"), &SimpleDebugger::JsModifyBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("ResumeFromBreakpoint"), &SimpleDebugger::JsResumeFromBreakpoint, this));
        IfFailGo(m_pController->InstallHostCallback(_u("DumpLocals"), &SimpleDebugger::JsDumpLocals, this));
        IfFailGo(m_pController->InstallHostCallback(_u("DumpCallstack"), &SimpleDebugger::JsDumpCallstack, this));
        IfFailGo(m_pController->InstallHostCallback(_u("EvaluateExpression"), &SimpleDebugger::JsEvaluateExpression, this));
        IfFailGo(m_pController->InstallHostCallback(_u("SetFrame"), &SimpleDebugger::JsSetFrame, this));
        IfFailGo(m_pController->InstallHostCallback(_u("LogJson"), &SimpleDebugger::JsLogJson, this));
        IfFailGo(m_pController->InstallHostCallback(_u("SetExceptionResume"), &SimpleDebugger::JsSetExceptionResume, this));
        IfFailGo(m_pController->InstallHostCallback(_u("SetDebuggerOptions"), &SimpleDebugger::JsSetDebuggerOptions, this));
        IfFailGo(m_pController->InstallHostCallback(_u("TrackProjectionCall"), &SimpleDebugger::JsTrackProjectionCall, this));
    }
Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("host callback initialization failed"));
    }
}

SimpleDebugger::~SimpleDebugger()
{
    for (auto it = m_breakpoints.begin(); it != m_breakpoints.end(); it++) {
        delete *it;
    }

    if(m_pController)
    {
        delete m_pController;
    }
}

HRESULT SimpleDebugger::Initialize(const string& initialCommand)
{
    HRESULT hr = S_OK;

    try
    {
        m_initialCommand = initialCommand;
    }
    catch(const std::bad_alloc&)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

void SimpleDebugger::Create(const string& initialCommand, _Out_ SimpleDebugger** dbg)
{
    UT_USE_DUMMY_TEST_GROUP();

    UT_COM_SUCCEEDED(CreateComObject(dbg));
    UT_COM_SUCCEEDED((*dbg)->Initialize(initialCommand));
}

void SimpleDebugger::DebugLaunch(_In_ LPTSTR pCmdLine)
{
    UT_COM_SUCCEEDED(DebugCreate(IID_PPV_ARGS(&m_client)));

    UT_COM_SUCCEEDED(m_client->SetEventCallbacksT(this));
    UT_COM_SUCCEEDED(m_client->CreateProcessT(0, pCmdLine, DEBUG_ONLY_THIS_PROCESS));
    UT_COM_SUCCEEDED(m_client.QueryInterface(&m_control));
    UT_COM_SUCCEEDED(m_client.QueryInterface(&m_debugDataSpaces));
    RunDebugLoop();
    // Since we created the process, make sure we kill the process too.
    UT_COM_SUCCEEDED(m_client->TerminateProcesses());
    m_client->EndSession(DEBUG_END_PASSIVE); // Cleanup session (and extensions)
}

void SimpleDebugger::Attach(ULONG pid)
{
    // Generate the event name
    WCHAR buf[64];
    std::wstring eventName = _u("jdtest");
    _itow_s(pid, buf, 10);
    eventName += buf;

    // Create the event
    HANDLE hEvent = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, eventName.c_str());
    IfFalseAssertReturn(hEvent != NULL);

    UT_COM_SUCCEEDED(DebugCreate(IID_PPV_ARGS(&m_client)));
    
    UT_COM_SUCCEEDED(m_client->SetEventCallbacksT(this));
    UT_COM_SUCCEEDED(m_client->AttachProcess(0, pid, 0));
    UT_COM_SUCCEEDED(m_client.QueryInterface(&m_control));
    UT_COM_SUCCEEDED(m_client.QueryInterface(&m_debugDataSpaces));

    IfFalseAssertReturn(SetEvent(hEvent));
    IfFalseAssertReturn(CloseHandle(hEvent));

    RunDebugLoop();
    m_client->EndSession(DEBUG_END_PASSIVE); // Cleanup session (and extensions)
}

void SimpleDebugger::RunDebugLoop()
{
    HRESULT hr = S_OK;

    
    // Enable initial breakpoint
    {
        DEBUG_SPECIFIC_FILTER_PARAMETERS initbp = {DEBUG_FILTER_BREAK, DEBUG_FILTER_GO_NOT_HANDLED};
        UT_COM_SUCCEEDED(m_control->SetSpecificFilterParameters(DEBUG_FILTER_INITIAL_BREAKPOINT, 1, &initbp));
    }

    bool initialBreak = true;

    UT_COM_SUCCEEDED(m_client->SetOutputCallbacksT(this));
    UT_COM_SUCCEEDED(m_client->SetInputCallbacks(this));

    const bool isInteractive = !(g_autoBreakpoints || g_targetedTest);
    while (true)
    {
        hr = m_control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
        if (hr != S_OK)
        {
            break;
        }

        _TCHAR cmd[MAX_PATH];
        cmd[0] = _T('\0');

        if (initialBreak)
        {
            if(m_verbose)
            {
                ULONG mask;
                UT_COM_SUCCEEDED(m_client->GetOutputMask(&mask));
                UT_COM_SUCCEEDED(m_client->SetOutputMask(mask | DEBUG_OUTPUT_VERBOSE));
            }

            initialBreak = false;
            if (!m_initialCommand.empty())
            {
                _tcscpy_s(cmd, m_initialCommand.c_str());
            }
            else if(!isInteractive) // provide automatic intial bp handling for auto/targeted debugging
            {
                _tcscpy_s(cmd, _T("g"));
            }
        }

        ULONG status = DEBUG_STATUS_BREAK;
        while (status == DEBUG_STATUS_BREAK)
        {
            if (cmd[0] == _T('\0'))
            {
                if (isInteractive)
                {
                    m_control->OutputPromptT(DEBUG_OUTCTL_THIS_CLIENT | DEBUG_OUTCTL_NOT_LOGGED, _T(" "));

                    ULONG size;
                    m_control->InputT(cmd, _countof(cmd), &size);
                }
                else
                {
                    _tcscpy_s(cmd, _T("gn")); // Auto/targeted debugging: resume target with exception not handled
                }
            }

            UT_COM_SUCCEEDED(m_control->ExecuteT(DEBUG_OUTCTL_ALL_CLIENTS, cmd, DEBUG_EXECUTE_DEFAULT));
            cmd[0] = _T('\0');

            if (m_control->GetExecutionStatus(&status) != S_OK)
            {
                status = DEBUG_STATUS_NO_DEBUGGEE;
            }
        }
    }

    if (m_pController)
    {
        m_pController->VerifyAndWriteNewBaselineFile(m_filename);
    }
    
    m_client->FlushCallbacks();
    UT_COM_SUCCEEDED(m_client->SetInputCallbacks(NULL));
    UT_COM_SUCCEEDED(m_client->SetOutputCallbacksT(NULL));
    UT_COM_SUCCEEDED(m_client->SetEventCallbacksT(NULL));
}

STDMETHODIMP SimpleDebugger::GetInterestMask(_Out_ PULONG Mask)
{
    *Mask = DEBUG_EVENT_EXCEPTION | DEBUG_EVENT_LOAD_MODULE | DEBUG_EVENT_BREAKPOINT;
    return S_OK;
}


STDMETHODIMP SimpleDebugger::Exception (
        _In_ PEXCEPTION_RECORD64 Exception,
        _In_ ULONG FirstChance
        ) 
{
    if(Exception->ExceptionCode == SCRIPT_DEBUGGER_EXCEPTION_CODE)
    {
        if(FAILED(EnsureDebugInterfaces()))
            return DEBUG_STATUS_GO_NOT_HANDLED;

        Assert(Exception->NumberParameters == 1);
        ScriptDebugEvent* debugEvent = (ScriptDebugEvent*)Exception->ExceptionInformation[0];
        RemoteScriptDebugEvent remoteDebugEvent(GetReader(), debugEvent);
        Assert(remoteDebugEvent->m_applicationId != 0);

        m_currentEvent = &remoteDebugEvent;

        if(m_threadId == 0)
        {
            m_threadId = remoteDebugEvent->m_scriptThreadId;
        }

        switch(remoteDebugEvent->m_type)
        {
        case ET_InsertDocumentText:
        {
            UINT64 textAddress = remoteDebugEvent->InsertDocumentText.Text.Address;
            DWORD length = remoteDebugEvent->InsertDocumentText.Text.Length;
            Assert(remoteDebugEvent->InsertDocumentText.DocumentId != 0);
            Assert(textAddress != NULL);
            Assert(length != 0);
            WCHAR* documentText = Reader::ReadString(GetReader(), 
                textAddress, 
                length);
                        
            WCHAR* url =  Reader::ReadString(GetReader(), 
                remoteDebugEvent->InsertDocumentText.DocumentUrl.Address, 
                remoteDebugEvent->InsertDocumentText.DocumentUrl.Length);
            
            char16 filename[_MAX_FNAME];
            char16 ext[_MAX_EXT];
            
            _wsplitpath_s(url, NULL, 0, NULL, 0, filename, _MAX_FNAME, ext, _MAX_EXT);
            wcscat_s(filename, ext);
            Out(false, _u("Script loaded: %s\n"), filename);
        
            if(FAILED(OnInsertText(remoteDebugEvent->InsertDocumentText.DocumentId, url, filename, documentText)))
                DebuggerController::LogError(_u("OnInsertText"));

            // Update the script debugger options to their current values.  This is currently only checked by the in-proc module
            // on document insertion events.
            remoteDebugEvent->m_scriptDebuggerOptions = m_scriptDebuggerOptions;
            remoteDebugEvent->m_scriptDebuggerOptionsValue = m_scriptDebuggerOptionsValue;
            remoteDebugEvent.Flush();

            delete[] documentText;
            delete[] url;
            break;
        }
        case ET_Breakpoint:
        {
            Out(false, _u("Breakpoint hit! Reason: %s \n"), m_pController->GetBreakpointReason(remoteDebugEvent->Breakpoint.reason));

            if(FAILED(OnBreakpoint(&remoteDebugEvent)))
                DebuggerController::LogError(_u("OnBreakpoint"));

            if(remoteDebugEvent->Breakpoint.reason == BREAKREASON_BREAKPOINT)
            {
                //return DEBUG_STATUS_BREAK;
            }
            break;
        }
        default:
            Assert(false);
            throw E_UNEXPECTED;
        }
        m_currentEvent = NULL;

        // Script exceptions are considered handled exceptions
        return DEBUG_STATUS_GO_HANDLED;
    }
    else
    {
        if (g_autoBreakpoints || g_targetedTest)
        {
            // Auto/targeted debugging: fail on these target exceptions.
            switch (Exception->ExceptionCode)
            {
            case STATUS_ACCESS_VIOLATION:
            case STATUS_ILLEGAL_INSTRUCTION:
            case STATUS_NONCONTINUABLE_EXCEPTION:
            case STATUS_STACK_OVERFLOW:
            case STATUS_HEAP_CORRUPTION:
            case STATUS_STACK_BUFFER_OVERRUN:
            case STATUS_ASSERTION_FAILURE:
                IfFalseAssertReturnEFAIL(false/*Fatal target failure*/); //We can now attach a debugger to jshost non-invasively.
            }
        }
        
        // Native exceptions should be passed on to the target.
        return DEBUG_STATUS_GO_NOT_HANDLED;
    }
}

STDMETHODIMP SimpleDebugger::Breakpoint (
        _In_ PDEBUG_BREAKPOINT2 Bp
        )
{
    HRESULT hr = S_OK;

    ULONG id;
    IfFailGo(Bp->GetId(&id));

    if (id == m_projectionCallBreakPointId)
    {
        IfFailGo(m_pController->LogJson(m_projectionCallMessage.c_str()));
        hr = DEBUG_STATUS_GO_HANDLED; // mark this bp handled
    }

Error:
    return hr;
}

STDMETHODIMP SimpleDebugger::LoadModule(
        _In_ ULONG64 ImageFileHandle,
        _In_ ULONG64 BaseOffset,
        _In_ ULONG ModuleSize,
        _In_opt_ PCWSTR ModuleName,
        _In_opt_ PCWSTR ImageName,
        _In_ ULONG CheckSum,
        _In_ ULONG TimeDateStamp
        )
{
    HRESULT hr = NOERROR;

    // in dynamic attach mode, don't preemptively call EnsureDebugInterfaces, or the engine
    // will be put into debug mode before the test case requests it to be.
    if(g_dynamicAttach)
        return NOERROR;
    
    if(wcscmp(ModuleName, _u("chakratest")) == 0
    || wcscmp(ModuleName, _u("chakra")) == 0
    || wcscmp(ModuleName, _u("jscript9")) == 0
    || wcscmp(ModuleName, _u("jscript9test")) == 0 )
    {
        IfFailedReturn(EnsureDebugInterfaces());
    }
    return hr;
}

STDMETHODIMP SimpleDebugger::StartInput(_In_ ULONG BufferSize)
{
    UNREFERENCED_PARAMETER(BufferSize);

    _TCHAR cmd[MAX_PATH];
    size_t size;
    _cgetts_s(cmd, _countof(cmd), &size);

    m_control->ReturnInputT(cmd);
    return S_OK;
}

STDMETHODIMP SimpleDebugger::EndInput()
{
    return S_OK;
}

STDMETHODIMP SimpleDebugger::Output(_In_ ULONG Mask, _In_ PCTSTR Text)
{
    UNREFERENCED_PARAMETER(Mask);

    // In "quiet" mode, only output text prefixed with $ut$
    const int PREFIXLEN = 4;
    if (m_quiet)
    {
        if (_tcsncmp(Text, _T("$ut$"), PREFIXLEN) == 0)
        {
            Text += PREFIXLEN;
        }
        else
        {
            return S_OK;
        }
    }

    _tprintf_s(_T("%s"), Text);
    return S_OK;
}

void SimpleDebugger::Out(_In_ bool unitTest, _In_ PCTSTR fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if(unitTest)
    {
        std::wstring s = std::wstring(_u("$ut$")) + fmt;
        fmt = s.c_str();
    }

    m_control->OutputVaListWide(DEBUG_OUTPUT_NORMAL, fmt, args);

    va_end(args);
}

HRESULT SimpleDebugger::PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, REFIID iid, LPVOID* ppunk)
{
    typedef HRESULT (STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);

    HRESULT hr = NOERROR;
    CComPtr <IClassFactory> pClassFactory;
    FN_DllGetClassObject pProc = NULL;

    HINSTANCE hInstance = LoadLibraryEx(strModule, NULL, 0);
    IfNullGo(hInstance, E_FAIL);
    IfNullGo(pProc = (FN_DllGetClassObject) GetProcAddress(hInstance, "DllGetClassObject"), E_FAIL);
    IfFailGo(pProc(rclsid, __uuidof(IClassFactory), (LPVOID*)&pClassFactory));
    IfFailGo(pClassFactory->CreateInstance(NULL, iid, ppunk));

Error:
    return hr;
}

USE_DbgEngDataTarget(); // Use DbgEngDataTarget implementation

HRESULT SimpleDebugger::EnsureDebugInterfaces()
{
    HRESULT hr = S_OK;
    UINT64 baseAddress; ULONG index;
    CComPtr<IDebugSymbols> symbols;

    if(m_initialized)
    {
        return S_OK;
    }

    // Once the unittest are using only vs build, we will not be needing chakradiagtest.dll
    hr = PrivateCoCreate(_u("chakradiagtest.dll"), CLSID_ChakraDiag, IID_PPV_ARGS(&m_pJsDebug));
    if (FAILED(hr))
    {
        IfFailGo(PrivateCoCreate(_u("chakradiag.dll"), CLSID_ChakraDiag, IID_PPV_ARGS(&m_pJsDebug)));
    }

    IfFailGo(m_client.QueryInterface<IDebugSystemObjects>(&m_pSystem));
    IfFailGo(m_pSystem->GetCurrentProcessSystemId(&m_procId));

    IfFailGo(m_client.QueryInterface(&symbols));
    IfFailGo(FindJScriptModuleByName</*IsPublic*/false>(symbols, &index, &baseAddress));

    // scope
    {
        CComPtr<DbgEngDataTarget> dataTarget;
        CComPtr<IDebugClient> client;
        IfFailGo(m_client->QueryInterface(IID_PPV_ARGS(&client)));
        IfFailGo(CreateComObject(&dataTarget));
        IfFailGo(dataTarget->Init(client));

        IfFailGo(m_pJsDebug->OpenVirtualProcess(dataTarget, /*debugMode*/ true, m_procId, baseAddress, NULL, &m_pDebugProcess));
    }

    m_initialized = true;

Error:
    return hr;
}

HRESULT SimpleDebugger::InspectLocals()
{
    return GetLocals(m_config.inspectionNestingLevel, DebugPropertyFlags::LOCALS_DEFAULT);
}

HRESULT SimpleDebugger::DumpBreakpoint(RemoteScriptDebugEvent *evt)
{
    HRESULT hr = S_OK;
    CComPtr<IJsDebugStackWalker> pStackWalker;    
    CComPtr<IJsDebugFrame> pDebugFrame;
    Location location;
    std::wstring reason = DebuggerController::GetBreakpointReason((*evt)->Breakpoint.reason);
    std::wstring json;

    IfFailGo(EnsureDebugInterfaces());
    IfFailGo(m_pDebugProcess->CreateStackWalker(m_threadId, &pStackWalker));
    IfFailGo(pStackWalker->GetNext(&pDebugFrame));
    IfFailGo(GetLocation(pDebugFrame, location));

    json += _u("{\"breakpoint\" : {\"reason\": \"");
    json += reason;
    json += _u("\"");
    json += _u(", \"location\": ");
    json += location.ToString();
    json += _u("}}");

    IfFailGo(m_pController->LogBreakpoint(json.c_str()));

Error:
    return hr;
}

HRESULT SimpleDebugger::DumpSourceList()
{
    return E_NOTIMPL;
}

HRESULT SimpleDebugger::InsertBreakpoint(UINT64 docId, DWORD charOffset, DWORD charCount, BREAKPOINT_STATE bpState, _Outptr_ BpInfo **bpInfo)
{
    HRESULT hr = S_OK;
    BOOL isEnabled;

    CComPtr<IJsDebugBreakPoint> pBreakpoint;
    IfFailGo(EnsureDebugInterfaces());
    IfFailGo(m_pDebugProcess->CreateBreakPoint(docId, charOffset, charCount, bpState == BREAKPOINT_ENABLED ? TRUE : FALSE, &pBreakpoint));
    IfFailGo(pBreakpoint->IsEnabled(&isEnabled));

    if(bpState == BREAKPOINT_ENABLED && !isEnabled)
    {
        DebuggerController::LogError(_u("breakpoint not enabled"));
    }

    *bpInfo = new BpInfo;
    IfFailGo(pBreakpoint->GetDocumentPosition(&(*bpInfo)->documentId, &(*bpInfo)->charOffset, &(*bpInfo)->charCount));
    (*bpInfo)->pBreakpoint = pBreakpoint;
    (*bpInfo)->id = CreateUniqueBreakpointId();
    m_breakpoints.push_back(*bpInfo);

Error:
    return hr;
}

HRESULT SimpleDebugger::ModifyBreakpoint(ULONG bpId, BREAKPOINT_STATE state)
{
    HRESULT hr = S_OK;

    std::vector<BpInfo*>::iterator it;
    for(it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it)
    {
        if((*it)->id == bpId)
            break;
    }
    if(it == m_breakpoints.end())
        return E_FAIL;

    switch(state)
    {
    case BREAKPOINT_ENABLED:
        IfFailGo((*it)->pBreakpoint->Enable());
        break;

    case BREAKPOINT_DISABLED:
        IfFailGo((*it)->pBreakpoint->Disable());
        break;

    case BREAKPOINT_DELETED:
        IfFailGo((*it)->pBreakpoint->Delete());
        break;

    default:
        return E_FAIL;
    }

Error:
    return hr;
}

HRESULT SimpleDebugger::RemoveBreakpoint(BpInfo *bpInfo)
{
    HRESULT hr = S_OK;

    std::vector<BpInfo*>::iterator it;
    for(it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it)
    {
        if(*it == bpInfo)
            break;
    }
    if(it == m_breakpoints.end())
        return E_FAIL;

    IfFailGo(bpInfo->pBreakpoint->Delete());
    delete bpInfo;
    m_breakpoints.erase(it);

Error:
    return hr;
}

HRESULT SimpleDebugger::GetLocation(CComPtr<IJsDebugFrame> pFrame, Location& location)
{
    HRESULT hr = S_OK;
    std::map<UINT64,std::wstring>::const_iterator iter;
    CComBSTR name;
    AutoJsDebugPropertyInfo debugPropertyInfo;
    CComPtr<IJsDebugProperty> debugProperty;

    IfFailGo(pFrame->GetDocumentPositionWithId(&location.docId, &location.startChar, &location.length));
    IfFailGo(pFrame->GetName(&name));

    // compute the location.srcId
    location.srcId = GetSrcIdForDocId(location.docId);

    location.frameDescription = name;
    
    if((iter = m_sourceText.find(location.docId)) != m_sourceText.end())
    {
        // Retrieve the source for the line, if it exists.
        location.text = iter->second.substr(location.startChar, location.length);
    }

    IfFailGo(pFrame->GetDebugProperty(&debugProperty));
    IfFailGo(debugProperty->GetPropertyInfo(/* nRadix */ 10, &debugPropertyInfo));
    location.debugPropertyAttributes = debugPropertyInfo.attr;

    // ALREADY 1-index based row and column numbers
    IfFailGo(pFrame->GetDocumentPositionWithName(&name, &location.lineNumber, &location.columnNumber));

Error:
    return hr;
}

HRESULT ValidateStackFrame(IJsDebugFrame* frame, IJsDebugFrame* previousFrame)
{
    HRESULT hr = S_OK;
    ULONG64 returnAddress;
    IfFailedReturn(frame->GetReturnAddress(&returnAddress));
    Assert(returnAddress != NULL);
    ULONG64 start, end;
    IfFailedReturn(frame->GetStackRange(&start, &end));
    Assert(end <= start);
    if(previousFrame)
    {
        ULONG64 previousStart, previousEnd;
        IfFailedReturn(previousFrame->GetStackRange(&previousStart, &previousEnd));
        Assert(previousStart <= end);
    }
    return hr;
}

HRESULT SimpleDebugger::GetCallstack(LocationToStringFlags flags)
{
    HRESULT hr = S_OK;

    CComPtr<IJsDebugStackWalker> pStackWalker;    
    CComPtr<IJsDebugFrame> pDebugFrame;
    CComPtr<IJsDebugFrame> pPreviousDebugFrame;

    std::wstring callstackEvent = _u("{\"callstack\" : [");
    std::wstring separator = _u("");

    IfFailGo(EnsureDebugInterfaces());
    IfFailGo(m_pDebugProcess->CreateStackWalker(m_threadId, &pStackWalker));

    while((hr = pStackWalker->GetNext(&pDebugFrame)) != E_JsDEBUG_OUTSIDE_OF_VM)
    {
        IfFailGo(hr);
        
        Location location;
        IfFailGo(GetLocation(pDebugFrame, location));
        ValidateStackFrame(pDebugFrame, pPreviousDebugFrame);

        callstackEvent += separator;
        separator = _u(", ");
        callstackEvent += location.ToString(flags, true);
        pPreviousDebugFrame = pDebugFrame;

        pDebugFrame.Release();
    }

    if(hr == E_JsDEBUG_OUTSIDE_OF_VM)
        hr = S_OK;
    

    callstackEvent += _u("]}");

    IfFailGo(m_pController->LogCallstack(callstackEvent.c_str()));

Error:

    return hr;
}

HRESULT SimpleDebugger::LogJson(LPCWSTR logString)
{
    HRESULT hr = S_OK;

    IfFailGo(m_pController->LogJson(logString));

Error:
    return hr;
}

HRESULT SimpleDebugger::GetLocals(int expandLevel, DebugPropertyFlags flags)
{
    HRESULT hr = S_OK;

    CComPtr<IJsDebugProperty> pDebugProperty;
    IfFailGo(EnsureDebugInterfaces());
    IfFailGo(GetLocalsEnum(pDebugProperty));
    IfFailGo(m_pController->DumpLocals(*this, pDebugProperty.p, expandLevel, flags));

Error:
    return hr;
}

HRESULT SimpleDebugger::EnsureFullNameEvaluationValueIsEquivalent(IJsDebugProperty* debugProperty, DebugPropertyFlags flags)
{
    AssertMsg(debugProperty, "Debug property was not supplied.");

    HRESULT hr = S_OK;
#if ENABLE_HYBRID_FULLNAME_CHECKING
    AutoJsDebugPropertyInfo debugPropertyInfo;
    IfFailGo(debugProperty->GetPropertyInfo(10, &debugPropertyInfo));
    if (wcscmp(debugPropertyInfo.fullName, debugPropertyInfo.name) != 0)
    {
        // If the short name doesn't match the full name, do a full name evaluation to see if the
        // resulting values are the same for both full and short name.
        std::wstring fullNameExpression = std::wstring(debugPropertyInfo.fullName);

        CComBSTR error;
        CComPtr<IJsDebugProperty> fullNameDebugProperty;
        IfFailGo(m_currentFrame->Evaluate(fullNameExpression.c_str(), &fullNameDebugProperty, &error));

        AutoJsDebugPropertyInfo fullNameDebugPropertyInfo;
        IfFailGo(fullNameDebugProperty->GetPropertyInfo(DebuggerController::GetRadix(flags), &fullNameDebugPropertyInfo));

        bool bothPropertiesAreReal = (debugPropertyInfo.attr & DBGPROP_ATTRIB_VALUE_IS_FAKE) == 0
                                  && (fullNameDebugPropertyInfo.attr & DBGPROP_ATTRIB_VALUE_IS_FAKE) == 0;

        // We need to exclude __proto__.size properties that throw exceptions because Maps/Sets/Weakmaps
        // will throw an exception if __proto__.size is accessed.
        bool isMapProtoSize = wcscmp(fullNameDebugPropertyInfo.type, _u("Error")) == 0
                           && wcsstr(fullNameDebugPropertyInfo.fullName, _u(".size")) != nullptr;

        // Exclude maps, sets, and weakmaps because any child properties are automatically resolved
        // to the map/set/weakmap variable's full name.
        bool isCollection = wcsstr(fullNameDebugPropertyInfo.type, _u("(Map)")) != nullptr
                         || wcsstr(fullNameDebugPropertyInfo.type, _u("(Set)")) != nullptr
                         || wcsstr(fullNameDebugPropertyInfo.type, _u("(WeakMap)")) != nullptr
                         || wcsstr(fullNameDebugPropertyInfo.type, _u("(WeakSet)")) != nullptr;

        // Exclude {error} and {exception} properties.
        bool isError = wcsstr(fullNameDebugPropertyInfo.fullName, _u("{error}")) != nullptr
                    || wcsstr(fullNameDebugPropertyInfo.fullName, _u("{exception}")) != nullptr;

        if (bothPropertiesAreReal && !isMapProtoSize && !isCollection && !isError)
        {
            AssertMsg(
                wcscmp(debugPropertyInfo.value, fullNameDebugPropertyInfo.value) == 0,
                _u("Short name and full name expression evaluation values don't match."));
        }
    }

Error:
#endif // ENABLE_HYBRID_FULLNAME_CHECKING
    return hr;
}

HRESULT SimpleDebugger::EvaluateExpression(PCWSTR expression, int expandLevel, DebugPropertyFlags flags)
{
    if (expandLevel < 0)
    {
        return E_FAIL;
    }    

    HRESULT hr = S_OK;
    std::wstring json = _u("{\"evaluate\" : {");

    CComPtr<IJsDebugProperty> pDebugProperty;
    CComBSTR error;
    
    IfFailGo(EnsureDebugInterfaces());
    IfFailGo(EnsureCurrentFrame());
    IfFailGo(m_currentFrame->Evaluate(expression, &pDebugProperty, &error));

    if (error)
    {
        std::wstring encodedExpression;
        DebuggerController::EncodeString(expression, encodedExpression);
        json += _u("\"") + encodedExpression + _u("\" : ");

        std::wstring encodedError;
        DebuggerController::EncodeString(error, encodedError);
        json += _u("\"") + encodedError + _u("\"");
    }
    else if (pDebugProperty)
    {
        IfFailGo(m_pController->DumpProperty(*this, pDebugProperty, expandLevel, flags, json));
    }

    json += _u("}}");
    IfFailGo(m_pController->LogLocals(json.c_str()));
Error:
    return hr;
}

HRESULT SimpleDebugger::GetLocalsEnum(CComPtr<IJsDebugProperty>& pDebugProperty)
{
    HRESULT hr = S_OK;

    CComPtr<IJsDebugStackWalker> pStackWalker;

    IfFailGo(EnsureCurrentFrame());
    
    // Retrieve the enumerator
    IfFailGo(m_currentFrame->GetDebugProperty(&pDebugProperty));

Error:
    return hr;
}

HRESULT SimpleDebugger::EnsureCurrentFrame()
{
    HRESULT hr = S_OK;

    if(m_currentFrame == NULL)
    {
        IfFailedReturn(SetCurrentFrame(0));
        Assert(m_currentFrame != NULL);
    }
    return hr;
}

void SimpleDebugger::ClearCurrentFrame()
{
    m_currentFrame.Release();
}

HRESULT SimpleDebugger::SetCurrentFrame(ULONG depth)
{
    HRESULT hr = S_OK;
    CComPtr<IJsDebugStackWalker> pStackWalker;
    CComPtr<IJsDebugFrame> pDebugFrame;

    IfFailGo(m_pDebugProcess->CreateStackWalker(m_threadId, &pStackWalker));

    for (ULONG i = 0; i <= depth; ++i)
    {
        pDebugFrame.Release();
        hr = pStackWalker->GetNext(&pDebugFrame);
        if (E_JsDEBUG_OUTSIDE_OF_VM == hr)
        {
            return S_OK; // Less number of frames then depth
        }
    }

    m_currentFrame = pDebugFrame;

Error:
    return hr;
}



HRESULT SimpleDebugger::OnInsertText(UINT64 docId, _In_ LPWSTR url, _In_ LPWSTR filename, _In_ LPWSTR text)
{
    HRESULT hr = S_OK;

    SourceMap sourceMap;
    sourceMap.Initialize(text);
    m_sourceMaps[docId] = sourceMap;

    ULONG srcId = CreateUniqueSrcId(docId);

    if(m_filename.empty())
    {
        m_filename = filename;
    }

    // Store the text
    if(g_autoBreakpoints || g_targetedTest)
    {
        if(m_sourceText.find(docId) == m_sourceText.end())
        {
            m_sourceText[docId] = std::wstring(text);
        }
    }

    if(g_autoBreakpoints)
    {
        // When running using automatic breakpoints, we want to exclude the glue files.
        // The reason is that the nightlies load up large infrastructure scripts, which 
        // execute extremely slowly due to the large log files and numbers of breakpoints.  
        // At the same time, we want to allow eval'd code to have automatic breakpoints.
        if(m_filename != filename)
        {
            LPWSTR searchStrs[] = { _u("newGlue.js"), _u("loggerglue.js") };
            for(int i = 0; i < _countof(searchStrs); ++i)
            {
                LPWSTR ptr = StrStrI(url, searchStrs[i]);
                if(ptr == url + wcslen(url) - wcslen(searchStrs[i]))
                {
                    // It's one of the glue files, skip it.
                    return S_OK;
                }
            }
        }

        for(int i = 0; i < sourceMap.GetNumLines(); i += m_config.setBpEveryNthLine)
        {
            BpInfo *bpInfo;
            IfFailGo(InsertBreakpoint(docId, sourceMap.GetOffset(i), 1, BREAKPOINT_ENABLED, &bpInfo));
            
            // Figure out which offset we actually got, and bump the line count accordingly
            UINT64 actualDocId;
            DWORD actualCharOffset;
            DWORD actualCharCount;
            IfFailGo(bpInfo->pBreakpoint->GetDocumentPosition(&actualDocId, &actualCharOffset, &actualCharCount));
            int actualLine = sourceMap.GetLine(actualCharOffset);

            // This can happen at the end of the file - we may move the breakpoint back to the previous statement.
            // Assert(actualLine >= i);

            if(actualLine > i)
                i = actualLine;
        }
    }
    else if(g_targetedTest)
    {
        IfFailGo(m_pController->AddSourceFile(text, srcId));
    }

Error:
    return hr;
}

HRESULT SimpleDebugger::OnBreakpoint(RemoteScriptDebugEvent *evt)
{
    HRESULT hr = S_OK;

    if(g_autoBreakpoints)
    {
        // Dump the breakpoint info.
        IfFailGo(DumpBreakpoint(evt));

        // Enumerate and inspect locals.
        IfFailGo(InspectLocals());

        // Dump the callstack.
        IfFailGo(GetCallstack());

        // Un-set any automatic breakpoints that have hit their maximum limit.
        IfFailGo(HandleAutomaticBreakpointLogic(evt));

        // Resume
        IfFailGo(ResumeFromBreakPoint(evt, m_config.ResumeActionOnBreak(), m_config.GetErrorResumeAction()));
    }
    else if(g_targetedTest)
    {
        if((*evt)->Breakpoint.reason == BREAKREASON_ERROR || (*evt)->Breakpoint.reason == BREAKREASON_DEBUGGER_HALT)
        {
            // The breakpoint was from an exception
            IfFailGo(m_pController->HandleException());
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
            LONG bpId = bp ? bp->id : -1;
            IfFailGo(m_pController->HandleBreakpoint(bpId));
        }
    }
    else
    {
        (*evt)->Breakpoint.breakResumeAction = BREAKRESUMEACTION_CONTINUE;
        (*evt)->Breakpoint.errorResumeAction = m_config.GetErrorResumeAction();
        (*evt).Flush();
    }

Error:
    return hr;
}

HRESULT SimpleDebugger::HandleAutomaticBreakpointLogic(RemoteScriptDebugEvent *evt)
{
    HRESULT hr = S_OK;
    BpInfo *bpInfo = NULL;

    if((*evt)->Breakpoint.reason == BREAKREASON_BREAKPOINT)
    {

        IfFailGo(GetCurrentBreakpoint(&bpInfo));

        if(bpInfo)
        {
            if(++bpInfo->hitCount > m_config.maxHitCountForABreakpoint)
            {
                IfFailGo(RemoveBreakpoint(bpInfo));
            }
        }
    }

Error:
    return hr;
}

HRESULT SimpleDebugger::GetCurrentBreakpoint(BpInfo **bpInfo)
{
    HRESULT hr = S_OK;

    CComBSTR scriptName;
    DWORD line, col;
    CComPtr<IJsDebugStackWalker> pStackWalker;    
    CComPtr<IJsDebugFrame> pDebugFrame;
    Location location;

    *bpInfo = NULL;

    IfFailGo(EnsureDebugInterfaces());
    IfFailGo(m_pDebugProcess->CreateStackWalker(m_threadId, &pStackWalker));
    IfFailGo(pStackWalker->GetNext(&pDebugFrame));
    IfFailGo(GetLocation(pDebugFrame, location));

    IfFailGo(pDebugFrame->GetDocumentPositionWithName(&scriptName, &line, &col));

    for(unsigned int i = 0; i < m_breakpoints.size(); i++)
    {
        if(location.startChar == m_breakpoints[i]->charOffset && location.docId == m_breakpoints[i]->documentId)
        {
            *bpInfo = m_breakpoints[i];
            break;
        }
    }

Error:
    return hr;
}

HRESULT SimpleDebugger::ResumeFromBreakPoint(RemoteScriptDebugEvent *evt, BREAKRESUMEACTION breakResume, ERRORRESUMEACTION errorResume)
{
    (*evt)->Breakpoint.breakResumeAction = breakResume;
    (*evt)->Breakpoint.errorResumeAction = errorResume;
    (*evt).Flush();

    // Release the old debug frame
    ClearCurrentFrame();

    return S_OK;
}

HRESULT SimpleDebugger::SetDebuggerOptions(SCRIPT_DEBUGGER_OPTIONS mask, BOOL value)
{
    m_scriptDebuggerOptions = mask;
    m_scriptDebuggerOptionsValue = value;
    return S_OK;
}

JsValueRef CALLBACK SimpleDebugger::JsModifyBreakpoint(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsModifyBreakpoint(ULONG srcId, ULONG lineNum, ULONG colNum);
    //
    
    HRESULT hr = S_OK;
    void *data;
    CComPtr<IJsDebugBreakPoint> pBreakpoint;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));
        
    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 3)
    {
        ULONG bpId;
        ULONG state;

        JsrtValueConverter converter(arguments, argumentCount);
        IfFailGo(converter.Convert(&bpId));
        IfFailGo(converter.Convert(&state));

        IfFailGo(pDebugger->ModifyBreakpoint(bpId, (BREAKPOINT_STATE)state));

    }
    else
    {
        IfFailGo(E_FAIL);
    }


Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsModifyBreakpoint"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsInsertBreakpoint(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsInsertBreakpoint(ULONG srcId, ULONG lineNum, ULONG colNum, ULONG breakpointState);
    //
    
    HRESULT hr = S_OK;
    void *data;
    JsValueRef retval = nullptr;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));
        
    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 5)
    {
        ULONG srcId;
        ULONG lineNum;
        ULONG colNum;
        ULONG bpState;

        JsrtValueConverter converter(arguments, argumentCount);
        IfFailGo(converter.Convert(&srcId));
        IfFailGo(converter.Convert(&lineNum));
        IfFailGo(converter.Convert(&colNum));
        IfFailGo(converter.Convert(&bpState));

        UINT64 docId = pDebugger->GetDocIdForSrcId(srcId);

        // Determine the character offset for the breakpoint
        DWORD charOffset = pDebugger->m_sourceMaps[docId].GetOffset(lineNum) + colNum;
        DWORD charCount = 1;
        BpInfo *bpInfo;

        IfFailGo(pDebugger->InsertBreakpoint(docId, charOffset, charCount, static_cast<BREAKPOINT_STATE>(bpState), &bpInfo));

        // Generate the return value, the breakpoint ID
        retval = DebuggerController::ConvertDoubleToNumber(bpInfo->id);

    }
    else
    {
        IfFailGo(E_FAIL);
    }


Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsInsertBreakpoint"));
    }

    return retval ? retval : DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsDumpLocals(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsDumpLocals(ULONG expandLevel);
    //
    isConstructCall;
    
    HRESULT hr = S_OK;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 3)
    {
        ULONG expandLevel;
        ULONG flags;

        JsrtValueConverter converter(arguments, argumentCount);
        IfFailGo(converter.Convert(&expandLevel, &flags));
        IfFailGo(pDebugger->GetLocals(expandLevel, (DebugPropertyFlags)flags));
    }
    else
    {
        hr = E_FAIL;
        goto Error;
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsDumpLocals"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsSetFrame(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsSetFrame(ULONG depth);
    //
    isConstructCall;
    
    HRESULT hr = S_OK;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 2)
    {
        ULONG depth;

        JsrtValueConverter converter(arguments, argumentCount);
        IfFailGo(converter.Convert(&depth));
        IfFailGo(pDebugger->SetCurrentFrame(depth));
    }
    else
    {
        hr = E_FAIL;
        goto Error;
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsSetFrame"));
    }

    return DebuggerController::GetJavascriptUndefined();
}


JsValueRef CALLBACK SimpleDebugger::JsSetDebuggerOptions(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsSetDebuggerOptions(SCRIPT_DEBUGGER_OPTIONS mask, BOOL value);
    //
    isConstructCall;
    
    HRESULT hr = S_OK;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 3)
    {
        ULONG mask;
        ULONG value;

        JsrtValueConverter converter(arguments, argumentCount);
        IfFailGo(converter.Convert(&mask));
        IfFailGo(converter.Convert(&value));
        IfFailGo(pDebugger->SetDebuggerOptions(static_cast<SCRIPT_DEBUGGER_OPTIONS>(mask), value ? TRUE : FALSE));
    }
    else
    {
        hr = E_FAIL;
        goto Error;
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsSetDebuggerOptions"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsDumpCallstack(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsDumpCallstack();
    //

    isConstructCall;
    arguments;
    
    HRESULT hr = S_OK;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 2)
    {
        ULONG flags;

        JsrtValueConverter converter(arguments, argumentCount);
        IfFailGo(converter.Convert(&flags));
        IfFailGo(pDebugger->GetCallstack(static_cast<LocationToStringFlags>(flags)));
    }
    else
    {
        hr = E_FAIL;
        goto Error;
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsDumpCallstack"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsEvaluateExpression(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsEvaluateExpression(std::wstring & expression, int expandLevel);
    //
    HRESULT hr = S_OK;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));
    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 4)
    {
        PCWSTR expression;
        ULONG expandLevel;
        ULONG flags;

        JsrtValueConverter converter(arguments, argumentCount);
        IfFailGo(converter.Convert(&expression, &expandLevel, &flags));
        IfFailGo(pDebugger->EvaluateExpression(expression, expandLevel, (DebugPropertyFlags)flags));
    }
    else
    {
        IfFailGo(E_FAIL);
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsEvaluateExpression_Internal"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsResumeFromBreakpoint(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsResumeFromBreakpoint(LPCWSTR resumeAction);
    //
    isConstructCall;

    HRESULT hr = S_OK;

    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 2)
    {
        JsrtValueConverter converter(arguments, argumentCount);
        LPCWSTR resumeAction;
        IfFailGo(converter.Convert(&resumeAction));

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
        IfFailGo(pDebugger->ResumeFromBreakPoint(pDebugger->m_currentEvent, action, pDebugger->m_defaultErrorAction));
    }
    else
    {
        hr = E_FAIL;
        goto Error;
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsResumeFromBreakpoint"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsSetExceptionResume(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsSetExceptionResume(LPCWSTR resumeAction);
    //
    isConstructCall;

    HRESULT hr = S_OK;

    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 2)
    {
        JsrtValueConverter converter(arguments, argumentCount);
        LPCWSTR resumeAction;
        IfFailGo(converter.Convert(&resumeAction));

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

        pDebugger->m_defaultErrorAction = action;
    }
    else
    {
        hr = E_FAIL;
        goto Error;
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsSetExceptionResume"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsLogJson(JsValueRef callee, bool isConstructCall,  JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    //
    // JsLogJson(LPCWSTR logString);
    //
    isConstructCall;

    HRESULT hr = S_OK;

    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));

    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 2)
    {
        JsrtValueConverter converter(arguments, argumentCount);
        LPCWSTR logString;
        IfFailGo(converter.Convert(&logString));

        // Log
        IfFailGo(pDebugger->LogJson(logString));
    }
    else
    {
        hr = E_FAIL;
        goto Error;
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsLogJson"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

JsValueRef CALLBACK SimpleDebugger::JsTrackProjectionCall(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = S_OK;
    void *data;

    IfFailGo(ScriptEngineWrapper::GetExternalData(callee, &data));
    SimpleDebugger *pDebugger = (SimpleDebugger*)data;

    if(argumentCount == 2)
    {
        PCWSTR message;

        JsrtValueConverter converter(arguments, argumentCount);
        IfFailGo(converter.Convert(&message));
        IfFailGo(pDebugger->TrackProjectionCall(message));
    }
    else
    {
        IfFailGo(E_FAIL);
    }

Error:
    if(FAILED(hr))
    {
        DebuggerController::LogError(_u("JsTrackProjectionCall"));
    }

    return DebuggerController::GetJavascriptUndefined();
}

HRESULT SimpleDebugger::TrackProjectionCall(PCWSTR message)
{
    HRESULT hr = S_OK;

    if (m_projectionCallBreakPointId == INVALID_BREAKPOINT_ID)
    {
        IfFailGo(EnsureDebugInterfaces());

        ULONG64 address;
        IfFailGo(m_pDebugProcess->GetExternalStepAddress(&address));

        PDEBUG_BREAKPOINT pNativeBreakPoint; // IDebugBreakpoint life time isn't managed by COM ref
        IfFailGo(m_control->AddBreakpoint(DEBUG_BREAKPOINT_CODE, DEBUG_ANY_ID, &pNativeBreakPoint));

        IfFailGo(pNativeBreakPoint->SetOffset(address));
        IfFailGo(pNativeBreakPoint->AddFlags(DEBUG_BREAKPOINT_ENABLED)); // enable the new bp

        IfFailGo(pNativeBreakPoint->GetId(&m_projectionCallBreakPointId));
    }

    m_projectionCallMessage = message;

Error:
    return hr;
}

ULONG SimpleDebugger::CreateUniqueSrcId(UINT64 docId)
{
    docIdCount++;

    m_docIdMap[docIdCount] = docId;

    return docIdCount;
}

UINT64 SimpleDebugger::GetDocIdForSrcId(ULONG srcId)
{
    std::map<ULONG,UINT64>::iterator iter = m_docIdMap.find(srcId);
    if(iter == m_docIdMap.end())
    {
        DebuggerController::LogError(_u("unknown srcid"));
        return 0;
    }   
    else
    {
        return iter->second;
    }
}

ULONG SimpleDebugger::GetSrcIdForDocId(UINT64 docid)
{
    for(auto iter = m_docIdMap.begin(); iter != m_docIdMap.end(); ++iter)
    {
        if(iter->second == docid)
            return iter->first;
    }
    DebuggerController::LogError(_u("unknown docid"));
    return 0;
}
