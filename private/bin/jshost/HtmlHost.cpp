//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

HINSTANCE g_hInst = NULL;
BOOL __stdcall OnScriptStateChangedCallBack(IActiveScriptDirect* scriptDirectRef, SCRIPTSTATE ss, void** engineSpecificStorage);
HRESULT __stdcall GetHeapObjectInfo(Var instance, HostProfilerHeapObject** heapObjOut, HeapObjectInfoReturnResult& returnResult);

PCWSTR const CAppWindow::s_szWindowClass = L"JSMsHtmlHostClass";

INTERNET_SCHEME GetScheme(LPCTSTR szURL);

LPAPP g_pApp = nullptr;

extern "C"
HRESULT __stdcall OnJScript9LoadedEntry(TestHooks& testHooks)
{
    HRESULT hr = JScript9Interface::OnJScript9Loaded(testHooks); 
    if (SUCCEEDED(hr))
    {
        testHooks.pfSetGetHeapObjectInfoPtr(GetHeapObjectInfo);
    }
    return hr;
}

HRESULT DoOneHtmlIteration(BSTR filename)
{
    HRESULT hr = S_OK;
    CAppWindow* pAppWindow = nullptr;

    MessageQueue* messageQueue = new MessageQueue();
    WScriptFastDom::AddMessageQueue(messageQueue);
    JScript9Interface::NotifyOnScriptStateChanged(OnScriptStateChangedCallBack);

    g_pApp = new CApp;
    if (!g_pApp)
    {
        IfFailGo(E_OUTOFMEMORY);
    }
    
    if (HostConfigFlags::flags.HtmlKeepAlive || HostConfigFlags::flags.HtmlVisible)
    {
        g_pApp->SetKeepAlive(true);
    }

    IfFailGo(g_pApp->ProcessArgs(filename, wcslen(filename)));

    HINSTANCE hInstance = GetModuleHandle(NULL);
    CAppWindow::RegisterClass(hInstance);

    pAppWindow = new CAppWindow();
    if (!pAppWindow)
    {
        IfFailGo(E_OUTOFMEMORY);
    }
    IfFailGo(pAppWindow->CreateHWnd(WS_OVERLAPPEDWINDOW, nullptr, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT) ? S_OK : E_FAIL);

    ShowWindow(pAppWindow->GetWindow(), HostConfigFlags::flags.HtmlVisible ? SW_SHOW : SW_HIDE);
    UpdateWindow(pAppWindow->GetWindow());

    IfFailGo(g_pApp->Init(pAppWindow));
    g_pApp->Run();

    ODS(L"Terminating html test ....\n");

Error:
    if (pAppWindow)
    {
        pAppWindow->Release(); // Ref count based
        pAppWindow = nullptr;
    }

    if (g_pApp)
    {
        g_pApp->Release(); // Ref count based
        g_pApp = nullptr;
    }

    return hr;
}

int _cdecl ExecuteHtmlTests(int argc, __in_ecount(argc) LPWSTR argv[])
{
    HRESULT hr = S_OK;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        wprintf(L"FATAL ERROR: CoInitializeEx() failed. hr=0x%x\n", hr);
        exit(1);
    }

    // For html tests, mshtml.dll will explicitly load chakra.dll, so we won't be able to load chakratest.dll
    // and have mshtml pick it up via DllGetClassObject. To achieve same functionality, the test harness must instead
    // copy chakratest.dll to jshost.exe.local\chakra.dll.
    hr = ExecuteTests(argc, argv, DoOneHtmlIteration, /*useChakra*/true);
    
    // Force one final GC before we CoUninitialize
    JScript9Interface::FinalGC();

    CoUninitialize();

    return hr;
}

void SetKeepAliveWrapper()
{
    Assert(g_pApp != NULL);

    g_pApp->SetKeepAlive(true);
}

void QuitHtmlHost()
{
    // Post a custom quit message to delay shutdown gracefully. This is invoked from WScript.Quit,
    // thus we are in the middle of some dispatch call. If we cleanup right now, we may null out
    // some objects unexpected by the dispatch call path (e.g. HostDispatch::scriptSite).
    PostThreadMessage(GetCurrentThreadId(), WM_USER_QUIT, (WPARAM)0, (LPARAM)0);
}

template <class Func>
void CreateDebugCallbackMessage(IDispatch *function, const Func& func)
{
    WScriptDispatchCallbackMessage* message = WScriptDispatchCallbackMessage::Create(function, [=](WScriptDispatchCallbackMessage &msg)
    {
        HRESULT hr = func();
        if (hr == S_OK)
        {
            hr = msg.CallJavascriptFunction(true /*force*/);
        }
        return hr;
    });
    PostThreadMessage(GetCurrentThreadId(), WM_USER_DEBUG_MESSAGE, (WPARAM)message, (LPARAM)0);
 }

void DynamictAttachDetachDebugger(UINT cmdId, IDispatch *function)
{
    if (cmdId == IDM_DEBUGGERDYNAMICATTACH)
    {
        CreateDebugCallbackMessage(function, [=]()
        {
            return DiagnosticsHelper::GetDiagnosticsHelper()->HtmlDynamicAttach(IDM_DEBUGGERDYNAMICATTACH);
        });
    }
    else if (cmdId == IDM_DEBUGGERDYNAMICDETACH)
    {
        CreateDebugCallbackMessage(function, [=]()
        {
            return DiagnosticsHelper::GetDiagnosticsHelper()->HtmlDynamicDetach();
        });
    }
    else if (cmdId == IDM_DEBUGGERDYNAMICATTACHSOURCERUNDOWN)
    {
        Assert(function == nullptr);/*no function to call*/
        CreateDebugCallbackMessage(function, [=]()
        {
            return DiagnosticsHelper::GetDiagnosticsHelper()->HtmlDynamicAttach(IDM_DEBUGGERDYNAMICATTACHSOURCERUNDOWN);
        });
    }
}

BOOL __stdcall OnScriptStateChangedCallBack(IActiveScriptDirect* scriptDirectRef, SCRIPTSTATE ss, void** engineSpecificStorage)
{
    HRESULT hr = NOERROR;
    if (ss == SCRIPTSTATE_STARTED)
    {
        hr = WScriptFastDom::Initialize((IActiveScript*)scriptDirectRef, TRUE, SetKeepAliveWrapper);
    }
    return true;
}