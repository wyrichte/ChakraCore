#include "stdafx.h"

/*static*/
DiagnosticsHelper *DiagnosticsHelper::s_diagnosticsHelper = nullptr;

HRESULT WScriptDispatchCallbackMessage::CallJavascriptFunction(bool force/* = false*/)
{
    Assert(force);

    HRESULT hr = S_OK;
    if (m_function)
    {
        CComPtr<IDispatchEx> dispatchEx;
        if ((hr = m_function->QueryInterface(__uuidof(IDispatchEx), (void **)&dispatchEx)) == S_OK)
        {
            DISPPARAMS dpNoArgs = { 0 };
            VARIANT vtRes;
            hr = dispatchEx->InvokeEx(DISPID_VALUE, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dpNoArgs, &vtRes, NULL/*ExceptionInfo * */, NULL/*IServiceProvider * */);
            VariantClear(&vtRes);
        }
    }
    return hr;
}

DiagnosticsHelper::DiagnosticsHelper()
    : m_debugger(nullptr)
    , m_hInstPdm(nullptr)
    , m_debugAppCookie(0)
    , m_shouldPerformSourceRundown(false)
{
}

void DiagnosticsHelper::ReleaseDiagnosticsHelper(bool forceNull)
{
    if (m_debugApplication.p && m_processDebugManager.p && !IsHtmlHost())
    {
        m_processDebugManager->RemoveApplication(m_debugAppCookie);
        m_debugApplication->Close();
        if (forceNull)
        {
            // This is due to jsrt, where it was some addref mismatch.
            m_debugApplication.p = nullptr;
            m_processDebugManager = nullptr;
        }
    }

    m_htmlDocument.Release();

    if (m_debugger)
    {
        m_debugger->Disconnect();
        m_debugger->Release();
        m_debugger = nullptr;
    }
}

HRESULT DiagnosticsHelper::CreateDocumentHelper(__in IDebugDocumentHelper ** debugDocumentHelper)
{
    Assert(debugDocumentHelper != nullptr);
    HRESULT hr = S_OK;
    if (m_hInstPdm)
    {
        hr = PrivateCoCreate(m_hInstPdm, __uuidof(CDebugDocumentHelper), NULL, CLSCTX_INPROC_SERVER, _uuidof(IDebugDocumentHelper), (LPVOID*)debugDocumentHelper);
    }
    else
    {
        hr = m_processDebugManager->CreateDebugDocumentHelper(0, debugDocumentHelper);
    }
    return hr;
}

HRESULT DiagnosticsHelper::HtmlDynamicAttach(DWORD cmdID)
{
    Assert(IsHtmlHost());
    Assert(cmdID == IDM_DEBUGGERDYNAMICATTACH || cmdID == IDM_DEBUGGERDYNAMICATTACHSOURCERUNDOWN);

    CComPtr<IOleCommandTarget> spCmdTarget;
    HRESULT hr = m_htmlDocument->QueryInterface(&spCmdTarget);
    if (hr == S_OK)
    {
        hr = spCmdTarget->Exec(&CGID_MSHTML, cmdID, 0, NULL, NULL);
        if (hr == S_OK)
        {
            HostConfigFlags::flags.DebugLaunch = ::SysAllocString(L"");

            if (m_debugApplication == nullptr) // m_debugApplication will not be null for multiple dynamic attach scenario
            {
                CComPtr<IServiceProvider> spServiceProvider;
                hr = m_htmlDocument->QueryInterface(__uuidof(IServiceProvider), (LPVOID *)&spServiceProvider);
                if ((hr = spServiceProvider->QueryService(__uuidof(IDebugApplication), __uuidof(IRemoteDebugApplication), (LPVOID *)&m_debugApplication)) == S_OK)
                {
                    hr = AttachToDebugger();
                    if (hr != S_OK)
                    {
                        wprintf(L"Error: Failed to AttachDebugger\n");
                    }
                }
                else
                {
                    wprintf(L"Error: Failed to get IRemoteDebugApplication\n");
                }
            }
            if (hr == S_OK && cmdID == IDM_DEBUGGERDYNAMICATTACH)
            {
                m_debugger->OnDebuggerAttachedCompleted();
            }
        }
        else
        {
            wprintf(L"Error: Failed to IOleCmdTarget::Exec\n");
        }
    }
    return hr;
}

HRESULT DiagnosticsHelper::HtmlDynamicDetach()
{
    Assert(IsHtmlHost());
    CComPtr<IOleCommandTarget> spCmdTarget;
    HRESULT hr = m_htmlDocument->QueryInterface(&spCmdTarget);
    if (hr == S_OK)
    {
        hr = spCmdTarget->Exec(&CGID_MSHTML, IDM_DEBUGGERDYNAMICDETACH, 0, NULL, NULL);
        if (hr == S_OK)
        {
            m_debugger->OnDebuggerDetachedCompleted();
        }
        else
        {
            wprintf(L"Error: Failed to IOleCmdTarget::Exec\n");
        }
    }

    return hr;
}

HRESULT DiagnosticsHelper::EnableHtmlDebugging()
{
    Assert(IsHtmlHost());

    HRESULT hr = S_OK;
    CComPtr<IServiceProvider> spserviceProvider;

    if ((hr = m_htmlDocument->QueryInterface(__uuidof(IServiceProvider), (LPVOID *)&spserviceProvider)) == S_OK)
    {
        // m_debugApplication will not be null if we have done dynamic attach more then once.
        if (m_debugApplication == nullptr && (hr = spserviceProvider->QueryService(__uuidof(IDebugApplication), __uuidof(IRemoteDebugApplication), (LPVOID *)&m_debugApplication)) == S_OK)
        {
            // This is the case when the host is already in debug mode, eg. set it thru internet option or using iertutil's help
            hr = AttachToDebugger();
        }
    }

    return hr;

}

HRESULT DiagnosticsHelper::InitializeDebugManager()
{
    Assert(m_processDebugManager == nullptr);
    Assert(m_debugApplication == nullptr);

    HRESULT hr = LoadPDM(&m_hInstPdm, &m_processDebugManager);
    if (hr != S_OK)
    {
        wprintf(L"[FAILED] to load the PDM\n");
        return hr;
    }

    hr = m_processDebugManager->GetDefaultApplication(&m_debugApplication);
    IfFailedGo(hr);

    hr = m_debugApplication->SetName(L"JsHost Application");
    IfFailedGo(hr);

    hr = m_processDebugManager->AddApplication(m_debugApplication, &m_debugAppCookie);
    IfFailedGo(hr);

    if (HostConfigFlags::flags.Break)
    {
        m_debugApplication->CauseBreak();
    }

LReturn:
    return hr;

}

bool DiagnosticsHelper::IsHtmlHost() const
{
    return m_htmlDocument != nullptr;
}

void DiagnosticsHelper::SetHtmlDocument(IHTMLDocument2 *htmlDocument)
{
    Assert(htmlDocument != nullptr);
    m_htmlDocument.Attach(htmlDocument);
    htmlDocument->AddRef();
}

// canSetBreakpoints: If this is the first initialization of debugger, set debugger's
// canSetBreakpoints state.
//
HRESULT DiagnosticsHelper::InitializeDebugging(bool canSetBreakpoints /*= false*/)
{
    HRESULT hr = S_OK;

    // Initialize the debugging objects.
    if (m_debugger == nullptr)
    {
        if (IsHtmlHost())
        {
            // html is the host
            hr = EnableHtmlDebugging();
        }
        else
        {
            hr = InitializeDebugManager();
            IfFailedGo(hr);

            hr = AttachToDebugger();
        }

        if (SUCCEEDED(hr) && m_debugger)
        {
            m_debugger->SetCanSetBreakpoints(canSetBreakpoints);
        }
    }

    if (!IsHtmlHost())
    {
        Assert(m_processDebugManager != nullptr);
        Assert(m_debugApplication != nullptr);
    }

LReturn:
    return hr;
}

HRESULT DiagnosticsHelper::AttachToDebugger()
{
    HRESULT hr = S_OK;

    if (HostConfigFlags::flags.DebugLaunch)
    {
        // Attach the test debugger to the current target engine.
        CComPtr<IRemoteDebugApplication> remoteDebugApp;
        if (m_debugApplication->QueryInterface(&remoteDebugApp) == S_OK)
        {
            // Create the debugger in different thread.
            Debugger::StartDebuggerThread(&m_debugger, remoteDebugApp);
        }

        if (!HostConfigFlags::flags.NoLibraryStackFrameDebugger) // By default enable library stack frame debugger options
        {
            CComPtr<IRemoteDebugApplication110> spDebugApp110;
            IfFailedGo(m_debugApplication->QueryInterface<IRemoteDebugApplication110>(&spDebugApp110));
            IfFailedGo(spDebugApp110->SetDebuggerOptions(SDO_ENABLE_LIBRARY_STACK_FRAME, SDO_ENABLE_LIBRARY_STACK_FRAME));
        }
    }

LReturn:
    return hr;
}

bool DiagnosticsHelper::IsHostInDebugMode() const
{
    return m_debugger && m_debugger->CanSetBreakpoints();
}

/*static*/
DiagnosticsHelper* DiagnosticsHelper::GetDiagnosticsHelper()
{
    if (DiagnosticsHelper::s_diagnosticsHelper == nullptr)
    {
        DiagnosticsHelper::s_diagnosticsHelper = new DiagnosticsHelper();
    }

    Assert(DiagnosticsHelper::s_diagnosticsHelper != nullptr);

    return DiagnosticsHelper::s_diagnosticsHelper;
}

/*static*/
void DiagnosticsHelper::DisposeHelper(bool forceNull/*=false default*/)
{
    if (DiagnosticsHelper::s_diagnosticsHelper != nullptr)
    {
        DiagnosticsHelper::s_diagnosticsHelper->ReleaseDiagnosticsHelper(forceNull);
        delete DiagnosticsHelper::s_diagnosticsHelper;
        DiagnosticsHelper::s_diagnosticsHelper = nullptr;
    }
}

bool DiagnosticsHelper::AddEditRangeAndContent(const wchar_t* editLabel, IDebugDocumentText* debugDocumentText, ULONG  startOffset, ULONG  length, const wchar_t*  editContent, ULONG newLength)
{
    try
    {
        EditMapType::const_iterator foundResult = this->m_editRangeAndContents.find(editLabel);
        if (foundResult != m_editRangeAndContents.end())
        {
            // Duplicated entry with the same label is not allowed
            return false;
        }

        this->m_editRangeAndContents.insert(std::make_pair(editLabel, EditRangeAndContent(debugDocumentText, startOffset, length, editContent, newLength)));
        return true;
    }
    catch (const std::bad_alloc&)
    {
        // Out of memory
        return false;
    }
}

bool DiagnosticsHelper::GetEditRangeAndContent(const wchar_t* editLabel, IDebugDocumentText** ppDebugDocumentText, ULONG* startOffset, ULONG* length, const wchar_t** editContent, ULONG* newLength)
{
    try
    {
        EditMapType::const_iterator foundResult = this->m_editRangeAndContents.find(editLabel);
        if (foundResult == m_editRangeAndContents.end())
        {
            // entry not found
            return false;
        }

        *ppDebugDocumentText = foundResult->second.debugDocumentText;
        (*ppDebugDocumentText)->AddRef();

        *startOffset = foundResult->second.m_startOffset;
        *length = foundResult->second.m_length;
        *editContent = foundResult->second.m_content.c_str();
        *newLength = static_cast<ULONG>(foundResult->second.m_content.size());
        return true;
    }
    catch (const std::bad_alloc&)
    {
        // Out of memory
        return false;
    }
}

// TODO: Consider updating existing edits after an edit is applied

DiagnosticsHelper::EditRangeAndContent::EditRangeAndContent(IDebugDocumentText* debugDocumentText, ULONG startOffset, ULONG length, const wchar_t* content, ULONG newLength)
: debugDocumentText(debugDocumentText)
, m_startOffset(startOffset)
, m_length(length)
, m_content(content, newLength)
{
}
