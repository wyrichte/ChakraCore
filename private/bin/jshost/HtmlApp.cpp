//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"
#include <ExDisp.h>
#include <ExDispID.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>
#include "helpers.h"
#include <ieconfig.h>

extern "C"
{
    const IID IID_IWebBrowserApp = __uuidof(IWebBrowserApp);
    const IID IID_IBrowserService = __uuidof(IBrowserService);
}

typedef HRESULT(*PFN_IEEnableScriptDebugging)();

INTERNET_SCHEME GetScheme(LPCTSTR szURL);

CApp::CApp() :
    m_dwRef(1), m_pCP(NULL), m_pWinSink(NULL), m_pAppWindow(NULL), m_pWebBrowser(NULL), m_pMSHTML(NULL),
    m_hrConnected(CONNECT_E_CANNOTCONNECT),
    m_dwCookie(0), m_szURL(NULL), m_nScheme(INTERNET_SCHEME_UNKNOWN),
    m_lReadyState(READYSTATE_UNINITIALIZED),
    m_keepAlive(false),
    hInstance(nullptr)
{
}

CApp::~CApp()
{
    if (hInstance != nullptr)
    {
        FreeLibrary(hInstance);
        hInstance = nullptr;
    }
}

HRESULT CApp::Passivate()
{
    HRESULT hr = NOERROR;

    // Disconnect from property change notifications
    if (m_pCP)
    {
        if (m_dwCookie)
        {
            hr = m_pCP->Unadvise(m_dwCookie);
            m_dwCookie = 0;
        }

        // Release the connection point
        m_pCP->Release();
        m_pCP = NULL;
    }

    if (m_pWebBrowser)
    {
        m_pWebBrowser->Release();
        m_pWebBrowser = NULL;
    }

    if (m_pWinSink)
    {
        m_pWinSink->Passivate();
        m_pWinSink->Release();
        m_pWinSink = NULL;
    }

    if (m_pAppWindow)
    {
        m_pAppWindow->Release();
        m_pAppWindow = nullptr;
    }

    if (m_pMSHTML)
    {
        m_pMSHTML->Release();
        m_pMSHTML = NULL;
    }

    return NOERROR;
}

STDMETHODIMP CApp::QueryInterface(REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    if (IID_IUnknown == riid || __uuidof(IPropertyNotifySink) == riid)
    {
        *ppv = (LPUNKNOWN)(IPropertyNotifySink*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IOleClientSite) == riid)
    {
        *ppv = (IOleClientSite*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IOleInPlaceSite) == riid || __uuidof(IOleWindow) == riid)
    {
        *ppv = (IOleInPlaceSite*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IDispatch) == riid)
    {
        *ppv = (IDispatch*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IDocHostUIHandler) == riid)
    {
        *ppv = (IDocHostUIHandler*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IOleCommandTarget) == riid)
    {
        *ppv = (IOleCommandTarget*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IServiceProvider) == riid)
    {
        *ppv = (IServiceProvider*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IBrowserService) == riid)
    {
        *ppv = (IBrowserService *)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IDocObjectService) == riid)
    {
        *ppv = (IDocObjectService *)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IDispatchEx) == riid)
    {
        return E_NOTIMPL;
    }
    else
    {
        WCHAR wszBuff[39];
        int i = StringFromGUID2(riid, wszBuff, 39);
        Assert(i != 0);
        ODS(L"CApp QI: "); ODS(wszBuff); ODS(L"\n");
        return E_NOTIMPL;
    }
}

STDMETHODIMP_(ULONG) CApp::AddRef()
{
    TCHAR szBuff[255];
    swprintf_s(szBuff, 255, L"CApp refcount increased to %d\n", m_dwRef+1);
    ODS(szBuff);
    return ++m_dwRef;
}

STDMETHODIMP_(ULONG) CApp::Release()
{
    TCHAR szBuff[255];

    if (--m_dwRef == 0)
    {
        ODS(L"Deleting CApp\n");
        delete this;
        return 0;
    }

    swprintf_s(szBuff, 255, L"CApp refcount reduced to %d\n", m_dwRef);
    ODS(szBuff);
    return m_dwRef;
}

// Fired on change of the value of a 'bindable' property
STDMETHODIMP CApp::OnChanged(DISPID dispID)
{
    HRESULT hr = NOERROR;
    WCHAR szBuff[255];
    
    if (DISPID_READYSTATE == dispID)
    {
        // check the value of the readystate property
        assert(m_pMSHTML);

        VARIANT vResult = {0};
        EXCEPINFO excepInfo;
        UINT uArgErr;

        DISPPARAMS dp = {NULL, NULL, 0, 0};
        if (SUCCEEDED(hr = m_pMSHTML->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_SYSTEM_DEFAULT,
            DISPATCH_PROPERTYGET, &dp, &vResult, &excepInfo, &uArgErr)))
        {
      #define RSLENGTH 20
            WCHAR szReadyState[RSLENGTH];
            assert(VT_I4 == V_VT(&vResult));
            m_lReadyState = (READYSTATE)V_I4(&vResult);
            switch (m_lReadyState)
            {    
              case READYSTATE_UNINITIALIZED:    //= 0,
                wcscpy_s(szReadyState, RSLENGTH, L"Uninitialized");
                  break;
              case READYSTATE_LOADING: //    = 1,
                wcscpy_s(szReadyState, RSLENGTH, L"Loading");
                hr = InitAfterLoad();
                break;
              case READYSTATE_LOADED:    // = 2,
                wcscpy_s(szReadyState, RSLENGTH, L"Loaded");
                  break;
              case READYSTATE_INTERACTIVE: //    = 3,
                wcscpy_s(szReadyState, RSLENGTH, L"Interactive");
                  break;
              case READYSTATE_COMPLETE: // = 4
                wcscpy_s(szReadyState, RSLENGTH, L"Complete");
                // Let the app know that the page has loaded
                PostThreadMessage(GetCurrentThreadId(),
                    WM_USER_PAGE_LOADED,
                    (WPARAM)0,
                    (LPARAM)0);
                break;
            }

            swprintf_s(szBuff, 255, L"OnChanged: readyState = %s\n", szReadyState);
            VariantClear(&vResult);
        }
        else
        {
            wcscpy_s(szBuff, 255, L"Unable to cfvobtain readyState value\n");
        }
    }
    else
    {
        swprintf_s(szBuff, 255, L"OnChanged: %d\n", dispID);
    }

    ODS(szBuff);

    return hr;
}

STDMETHODIMP CApp::OnRequestEdit(DISPID dispID)
{
    // Property changes are OK any time as far as this app is concerned
    WCHAR szBuff[255];
    swprintf_s(szBuff, 255, L"OnRequestEdit: %d\n", dispID);
    ODS(szBuff);

    return NOERROR;
}

STDMETHODIMP CApp::GetWindow(
    /* [out] */ __RPC__deref_out_opt HWND *phwnd)
{
    *phwnd = m_pAppWindow->GetWindow();
    return S_OK;
}

STDMETHODIMP CApp::GetWindowContext(
    /* [out] */ __RPC__deref_out_opt IOleInPlaceFrame **ppFrame,
    /* [out] */ __RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
    /* [out] */ __RPC__out LPRECT lprcPosRect,
    /* [out] */ __RPC__out LPRECT lprcClipRect,
    /* [out][in] */ __RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    *ppFrame = m_pAppWindow;
    m_pAppWindow->AddRef();

    *ppDoc = nullptr;

    GetClientRect(m_pAppWindow->GetWindow(), lprcPosRect);
    *lprcClipRect = *lprcPosRect;

    lpFrameInfo->cAccelEntries = 0;
    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->haccel = NULL;
    lpFrameInfo->hwndFrame = m_pAppWindow->GetWindow();

    return S_OK;
}

STDMETHODIMP CApp::OnPosRectChange(
    /* [in] */ __RPC__in LPCRECT lprcPosRect)
{
    HRESULT hr = S_OK;

    if (m_pMSHTML)
    {
        CComPtr<IOleInPlaceObject> pOleInPlaceObject;
        IfFailGo(m_pMSHTML->QueryInterface(&pOleInPlaceObject));
        IfFailGo(pOleInPlaceObject->SetObjectRects(lprcPosRect, lprcPosRect));
    }

Error:
    return hr;
}

// IOleCommandTarget
STDMETHODIMP CApp::QueryStatus(
    /* [unique][in] */ __RPC__in_opt const GUID *pguidCmdGroup,
    /* [in] */ ULONG cCmds,
    /* [out][in][size_is] */ __RPC__inout_ecount_full(cCmds) OLECMD prgCmds[],
    /* [unique][out][in] */ __RPC__inout_opt OLECMDTEXT *pCmdText)
{
    return OLECMDERR_E_UNKNOWNGROUP;
}

STDMETHODIMP CApp::Exec(
    /* [unique][in] */ __RPC__in_opt const GUID *pguidCmdGroup,
    /* [in] */ DWORD nCmdID,
    /* [in] */ DWORD nCmdexecopt,
    /* [unique][in] */ __RPC__in_opt VARIANT *pvaIn,
    /* [unique][out][in] */ __RPC__inout_opt VARIANT *pvaOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP; // Trident backup handler will take default action (show UI)

    if (pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler))
    {
        switch (nCmdID)
        {
        case OLECMDID_SHOWSCRIPTERROR:
            {
                const int onScriptError = HostConfigFlags::flags.HtmlOnScriptError;
                if (onScriptError == 1 || onScriptError == 0)
                {
                    if (pvaOut)
                    {
                        IfFailGo(VariantClear(pvaOut));
                        V_VT(pvaOut) = VT_BOOL;
                        V_BOOL(pvaOut) = (onScriptError == 1 ? VARIANT_TRUE : VARIANT_FALSE);
                    }
                    return S_OK; // indicate handled
                }
            }
            break;

        case OLECMDID_SHOWMESSAGE:
            wprintf(L"Exec: Ignore DocHostUIHandler command %d\n", nCmdID);
            // trident calls into here to show script abort dialog, we don't want to abort everytime.
            // we can also have some kind of heuristic here to test abort.
            if (pvaIn && V_VT(pvaIn) == VT_UNKNOWN)
            {
                SimulateShowMessage(pvaIn, pvaOut);
            }
            return S_OK;
        }
    }

Error:
    return hr;
}

HRESULT  
CApp::GetParamsFromEvent(  
    __in IHTMLEventObj         * pEventObj,  
    __in unsigned int            cExpandos,  
    __out_ecount(cExpandos) DISPID                  aDispid [],  
    __out_ecount(cExpandos) VARIANT                 aVariant [],  
    __in_ecount(cExpandos)  const SExpandoInfo      aExpandos [])  
{  
    HRESULT hr;
    CComPtr<IDispatchEx> pDispatchEx = nullptr;  
    unsigned int i;  
  
    if (!pEventObj || !aVariant || !aExpandos)  
    {  
        return E_INVALIDARG;  
    }  
  
    for (i = 0; i < cExpandos; i++)  
    {  
        VariantInit(aVariant + i);  
        aDispid[i] = DISPID_UNKNOWN;  
    }  
  
    IfFailGo(pEventObj->QueryInterface(IID_PPV_ARGS(&pDispatchEx)));  
  
    for (i = 0; i < cExpandos; i++)  
    {  
        CComBSTR bstrName;  
        DISPPARAMS dispparamsNoArgs;  
  
        bstrName.Attach(aExpandos[i].name);  
  
        IfFailGo(pDispatchEx->GetDispID(  
            bstrName,  
            fdexNameCaseSensitive,  
            aDispid + i));  
  
        dispparamsNoArgs = { nullptr, nullptr, 0, 0 };  
        IfFailGo(pDispatchEx->InvokeEx(  
            aDispid[i],  
            LOCALE_USER_DEFAULT,  
            DISPATCH_PROPERTYGET,  
            &dispparamsNoArgs,  
            aVariant + i,  
            NULL,  
            NULL));  

        bstrName.Detach();
    }  

Error:
    return hr;  
}  
  
HRESULT CApp::SimulateShowMessage(VARIANT* pvarIn, VARIANT* pvaOut)
{
    HRESULT hr = NOERROR;

    static const int MAX_STRING_RESOURCE_LENGTH = 65535;
    static const LPCWSTR qcStartString = L"Stop running this script?";
    static const SExpandoInfo s_aMessageExpandos[] =
    {
        { TEXT("messageText"), VT_BSTR },
        { TEXT("messageCaption"), VT_BSTR },
        { TEXT("messageStyle"), VT_I4 },
        { TEXT("messageHelpFile"), VT_BSTR },
        { TEXT("messageHelpContext"), VT_I4 }
    };

    enum MessageEnum
    {
        MessageText,
        MessageCaption,
        MessageStyle,
        MessageHelpFile,
        MessageHelpContext
    };

    static const int cExpandos = ARRAYSIZE(s_aMessageExpandos);
    CComPtr<IUnknown> pUnk = V_UNKNOWN(pvarIn);
    DISPID aDispid[cExpandos];
    CComVariant aVariant[cExpandos];
    CComPtr<IHTMLEventObj> pEvent = nullptr;
    CComPtr<IHTMLDocument2> pOmDoc = nullptr;
    CComPtr<IHTMLWindow2> pOmWindow = nullptr;
    LPCWSTR lpstrText;

    // Get the event parameter from the CDoc which has the text of message that will be shown.
    IfFailGo(pUnk->QueryInterface(IID_PPV_ARGS(&pOmDoc)));
    IfFailGo(pOmDoc->get_parentWindow(&pOmWindow));
    IfFailGo(pOmWindow->get_event(&pEvent));
    IfFailGo(GetParamsFromEvent(
        pEvent,
        cExpandos,
        aDispid,
        aVariant,
        s_aMessageExpandos));

    // Get the message text to be shown
    lpstrText = V_BSTR(&aVariant[MessageText]);
    if (lpstrText && memcmp(lpstrText, qcStartString, wcslen(qcStartString) * 2) == 0)
    {
         pvaOut->vt = VT_I4;
         pvaOut->intVal = IDNO;
    }

Error:
    return hr;
}

// Return the protocol associated with the specified URL
INTERNET_SCHEME GetScheme(LPCTSTR szURL)
{
    URL_COMPONENTS urlComponents;
    DWORD dwFlags = 0;
    INTERNET_SCHEME nScheme = INTERNET_SCHEME_UNKNOWN;

  // CODE REVIEW: Dangerous API. Ensure zeroed buffer size in bytes. It is.
    ZeroMemory((PVOID)&urlComponents, sizeof(URL_COMPONENTS));
    urlComponents.dwStructSize = sizeof(URL_COMPONENTS);

    if (szURL)
    {
        if (InternetCrackUrl(szURL, 0, dwFlags, &urlComponents))
        {
            nScheme = urlComponents.nScheme;
        }
    }

    return nScheme;
}

HRESULT CApp::ProcessArgs(__in_ecount(urlLen) LPCWSTR url, size_t urlLen)
{
    Assert(url);

    // Check the scheme (protocol)
    // to determine which persistence interface to use
    // see the Run method for where the scheme is used
    m_szURL = SysAllocString(url);
    if (m_szURL == NULL)
    {
        return E_FAIL;
    }
    m_nScheme = GetScheme(m_szURL);
    return S_OK;
}

// We will put the host to debug mode upfront by using the IERTUTIL.dll's function
void CApp::SetHostToDebugMode()
{
    HMODULE  hIeRtUtilModule = LoadLibraryEx(L"iertutil.dll", nullptr, 0);

    if (hIeRtUtilModule)
    {
        PFN_IEEnableScriptDebugging pfnIEEnableScriptDebugging = (PFN_IEEnableScriptDebugging)GetProcAddress(hIeRtUtilModule, MAKEINTRESOURCEA(175 /*a magic number*/));
        if (pfnIEEnableScriptDebugging != NULL)
        {
            pfnIEEnableScriptDebugging();
        }
        FreeLibrary(hIeRtUtilModule);
    }
}

HRESULT CApp::Init(CAppWindow* pAppWindow)
{
    HRESULT hr;
    CComPtr<IOleObject> pOleObject;
    CComPtr<IOleControl> pOleControl;
    CComPtr<IConnectionPointContainer> pCPC;

    if (FAILED(hr = PrivateCoCreateForEdgeHtml(__uuidof(HTMLDocument), NULL,
        CLSCTX_INPROC_SERVER, __uuidof(IHTMLDocument2),
        (LPVOID*)&g_pApp->m_pMSHTML)))
    {
        ODS(L"FATAL ERROR: PrivateCoCreateForEdgeHtml failed\n");
        return hr;
    }

    m_pAppWindow = pAppWindow;
    m_pAppWindow->AddRef();

    m_pWebBrowser = new CAppWebBrowser();

    if (HostConfigFlags::flags.DebugLaunch)
    {
        SetHostToDebugMode();
    }

    // copied from htmlpad to make it work. Will investigate 
    // webplatformhelper.lib.
    IEConfiguration_SetBool(IEPS_EdgeContentHost, true);

     {// scope
        CComPtr<IHTMLWindow2> pWin;
        if (SUCCEEDED(hr = m_pMSHTML->get_parentWindow(&pWin)))
        {
            m_pWinSink = new CWinSink();
            if (!m_pWinSink)
            {
                goto Error;
            }
            if (FAILED(m_pWinSink->Init(pWin)))
            {
                goto Error;
            }
        }
    }
    IfFailGo(m_pMSHTML->QueryInterface(&pOleObject));
    IfFailGo(pOleObject->SetClientSite((IOleClientSite*)this));
    IfFailGo(pOleObject->DoVerb(OLEIVERB_SHOW, NULL, this, 0, NULL, NULL)); // Puts mshtml in OS_RUNNING state right away, needed by CMarkup::EnsureFormatCacheChange

    IfFailGo(m_pMSHTML->QueryInterface(__uuidof(IOleControl), (LPVOID*)&pOleControl));
    IfFailGo(pOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_USERMODE));

    // Hook up sink to catch ready state property change
    IfFailGo(m_pMSHTML->QueryInterface(&pCPC));
    IfFailGo(pCPC->FindConnectionPoint(__uuidof(IPropertyNotifySink), &m_pCP));

    m_hrConnected = m_pCP->Advise((LPUNKNOWN)(IPropertyNotifySink*)this, &m_dwCookie);
Error:

   return hr;
}

HRESULT CApp::InitAfterLoad()
{
    HRESULT hr = NOERROR;
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
    diagnosticsHelper->SetHtmlDocument(m_pMSHTML);
    if (HostConfigFlags::flags.DebugLaunch)
    {
        hr = diagnosticsHelper->InitializeDebugging(/*canSetBreakpoints*/true);
    }
    return hr;
}

STDMETHODIMP CApp::QueryService(REFGUID sid, REFIID iid, LPVOID * ppv)
{
    HRESULT hr = E_FAIL;

    if (iid == IID_IBrowserService)
    {
        hr = QueryInterface(iid, ppv);
    }
    else if (sid == SID_SWebBrowserApp)
    {
        if (m_pWebBrowser)
        {
            hr = m_pWebBrowser->QueryInterface(iid, ppv);
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

// Clean up connection points, GC, post WM_QUIT
HRESULT CApp::Term()
{
    HRESULT hr = Passivate(); // Release all mshtml references
    DiagnosticsHelper::DisposeHelper();

    JScript9Interface::FinalGC(); // Cleanup

    PostQuitMessage(0);

    return hr;
}

// Load the specified document and start pumping messages
HRESULT CApp::Run()
{
    HRESULT hr;
    MSG msg;

    switch(m_nScheme)
    {
    case INTERNET_SCHEME_HTTP:
    case INTERNET_SCHEME_FTP:
    case INTERNET_SCHEME_GOPHER:
    case INTERNET_SCHEME_HTTPS:
    case INTERNET_SCHEME_FILE:
        // load URL using IPersistMoniker
        hr = LoadURLFromMoniker();
        break;
    case INTERNET_SCHEME_NEWS:
    case INTERNET_SCHEME_MAILTO:
    case INTERNET_SCHEME_SOCKS:
        // we don't handle these
        return E_FAIL;
        break;
    //case INTERNET_SCHEME_DEFAULT:
    //case INTERNET_SCHEME_PARTIAL = -2,
    //case INTERNET_SCHEME_UNKNOWN = -1,
    default:
        // try loading URL using IPersistFile
        hr = LoadURLFromFile();
        break;
    }

    if (SUCCEEDED(hr) || E_PENDING == hr)
    {
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (WM_USER_PAGE_LOADED == msg.message && NULL == msg.hwnd)
            {
                g_pApp->OnPageLoaded();
            }
            else if (WM_USER_DEBUG_MESSAGE == msg.message && NULL == msg.hwnd && NULL != msg.wParam)
            {
                WScriptDispatchCallbackMessage *message = (WScriptDispatchCallbackMessage *)msg.wParam;
                message->Call();
                delete message;
            }
            else if (WM_USER_QUIT == msg.message && NULL == msg.hwnd)
            {
                if (m_pMSHTML) // We can have multiple quit messages posted to the queue. Prev message might have cleared m_pMSHTML.
                {
                    //Scope (ensure temporary interfaces released before Term() call).
                    CComPtr<IOleObject> pOleObject;
                    IfFailGo(m_pMSHTML->QueryInterface(&pOleObject));
                    pOleObject->DoVerb(OLEIVERB_HIDE, NULL, this, 0, NULL, NULL); // Send hide verb for better cleanup before releasing mshtml.
                }

                g_pApp->Term();
            }
            else
            {
                DispatchMessage(&msg);
            }
        }
    }

Error:
    return hr;
}

// Use an asynchronous Moniker to load the specified resource
HRESULT CApp::LoadURLFromMoniker()
{
    HRESULT hr;

    // Ask the system for a URL Moniker
    LPMONIKER pMk = NULL;
    LPBINDCTX pBCtx = NULL;
    LPPERSISTMONIKER pPMk = NULL;

    if (FAILED(hr = CreateURLMoniker(NULL, m_szURL, &pMk)))
    {
        return hr;
    }

    if (FAILED(hr = CreateBindCtx(0, &pBCtx)))
    {
        goto Error;
    }

    // Use MSHTML moniker services to load the specified document
    if (SUCCEEDED(hr = m_pMSHTML->QueryInterface(__uuidof(IPersistMoniker),
                                (LPVOID*)&pPMk)))
    {
        // Call Load on the IPersistMoniker
        // This may return immediately, but the document isn't loaded until
        // MSHTML reports READYSTATE_COMPLETE. See the implementation of
        // IPropertyNotifySink::OnChanged above and see how the app waits
        // for this state change
        TCHAR szBuff[MAX_PATH];

        swprintf_s(szBuff, MAX_PATH, L"Loading %s...\n", m_szURL);

        ODS(szBuff);
        hr = pPMk->Load(FALSE, pMk, pBCtx, STGM_READ);
    }
        
Error:
    if (pMk) pMk->Release();
    if (pBCtx) pBCtx->Release();
    if (pPMk) pPMk->Release();
    return hr;
}

// A more traditional form of persistence.
// MSHTML performs this asynchronously as well.
HRESULT CApp::LoadURLFromFile()
{
    LPPERSISTFILE  pPF;
    HRESULT hr;

    FILE* htmlFile;
    if (_wfopen_s(&htmlFile, m_szURL, L"r") != 0)
    {
        wprintf(L"FATAL ERROR: Could not open file %s\n", m_szURL);
        return E_FAIL;
    }
    fclose(htmlFile);

    // MSHTML supports file persistence for ordinary files.
    if ( SUCCEEDED(hr = m_pMSHTML->QueryInterface(__uuidof(IPersistFile), (LPVOID*) &pPF)))
    {
        TCHAR szBuff[MAX_PATH];
        swprintf_s(szBuff, MAX_PATH, L"Loading %s...\n", m_szURL);
        ODS(szBuff);

        DWORD result = GetFullPathName(m_szURL, _countof(szBuff), szBuff, NULL);
        if (result == 0)
        {
            wprintf(L"FATAL ERROR: GetFullPathName failed on %s, GLE: %d\n", m_szURL, GetLastError());
            return E_FAIL;
        }

        hr = pPF->Load(szBuff, 0);
        pPF->Release();
    }

    return hr;
}

HRESULT CApp::OnPageLoaded()
{
    HRESULT hr = S_OK;

    // NOTE: Script may call WScript.Quit() before page loaded event, which may have released Trident.

    if (READYSTATE_COMPLETE != m_lReadyState)
    {
        ODS(L"Shouldn't get here 'til MSHTML changes readyState to READYSTATE_COMPLETE\n");
        DebugBreak();
        return E_UNEXPECTED;
    }

    // ***** Add any work to be done when page loads here *****

    // When done, post ourselves a quit to terminate the message pump.
    if (m_keepAlive == false)
    {
        QuitHtmlHost();
    }

    return hr;
}

// MSHTML Queries for the IDispatch interface of the host through the IOleClientSite
// interface that MSHTML is passed through its implementation of IOleObject::SetClientSite()
STDMETHODIMP CApp::Invoke(DISPID dispIdMember,
            REFIID riid,
            LCID lcid,
            WORD wFlags,
            DISPPARAMS __RPC_FAR *pDispParams,
            VARIANT __RPC_FAR *pVarResult,
            EXCEPINFO __RPC_FAR *pExcepInfo,
            UINT __RPC_FAR *puArgErr)
{
    if (!pVarResult)
    {
        return E_POINTER;
    }

    PrintDISPID(dispIdMember);

    switch(dispIdMember)
    {
    case DISPID_AMBIENT_USERMODE:
        // put MSHTML into edit mode so can run scripts
        V_VT(pVarResult) = VT_BOOL;
        V_BOOL(pVarResult) = VARIANT_TRUE;
        break;
    default:
        return DISP_E_MEMBERNOTFOUND;
    }

    return NOERROR;
}

// Diagnostic helper to discover what ambient properties MSHTML
// asks of the host
void PrintDISPID(DISPID dispidMember)
{
    #define ALEN(x) (sizeof(x)/(sizeof(x[0])))

    typedef struct {
        DISPID dispid;
        LPCTSTR szDesc;
    } DISPIDDESC;

    static DISPIDDESC aDISPIDS[] = {    
                            {DISPID_AMBIENT_DLCONTROL, _T("DISPID_AMBIENT_DLCONTROL")}, // (-5512)
                            {DISPID_AMBIENT_USERAGENT, _T("DISPID_AMBIENT_USERAGENT")}, // (-5513)
                            {DISPID_AMBIENT_BACKCOLOR, _T("DISPID_AMBIENT_BACKCOLOR")},  //        (-701)
                            {DISPID_AMBIENT_DISPLAYNAME, _T("DISPID_AMBIENT_DISPLAYNAME")},  //      (-702)
                            {DISPID_AMBIENT_FONT, _T("DISPID_AMBIENT_FONT")},  //             (-703)
                            {DISPID_AMBIENT_FORECOLOR, _T("DISPID_AMBIENT_FORECOLOR")},  //        (-704)
                            {DISPID_AMBIENT_LOCALEID, _T("DISPID_AMBIENT_LOCALEID")},  //         (-705)
                            {DISPID_AMBIENT_MESSAGEREFLECT, _T("DISPID_AMBIENT_MESSAGEREFLECT")},  //   (-706)
                            {DISPID_AMBIENT_SCALEUNITS, _T("DISPID_AMBIENT_SCALEUNITS")},  //       (-707)
                            {DISPID_AMBIENT_TEXTALIGN, _T("DISPID_AMBIENT_TEXTALIGN")},  //        (-708)
                            {DISPID_AMBIENT_USERMODE, _T("DISPID_AMBIENT_USERMODE")},  //         (-709)
                            {DISPID_AMBIENT_UIDEAD, _T("DISPID_AMBIENT_UIDEAD")},  //           (-710)
                            {DISPID_AMBIENT_SHOWGRABHANDLES, _T("DISPID_AMBIENT_SHOWGRABHANDLES")},  //  (-711)
                            {DISPID_AMBIENT_SHOWHATCHING, _T("DISPID_AMBIENT_SHOWHATCHING")},  //     (-712)
                            {DISPID_AMBIENT_DISPLAYASDEFAULT, _T("DISPID_AMBIENT_DISPLAYASDEFAULT")},  // (-713)
                            {DISPID_AMBIENT_SUPPORTSMNEMONICS, _T("DISPID_AMBIENT_SUPPORTSMNEMONICS")},  // (-714)
                            {DISPID_AMBIENT_AUTOCLIP, _T("DISPID_AMBIENT_AUTOCLIP")},  //         (-715)
                            {DISPID_AMBIENT_APPEARANCE, _T("DISPID_AMBIENT_APPEARANCE")},  //       (-716)
                            {DISPID_AMBIENT_PALETTE, _T("DISPID_AMBIENT_PALETTE")},  //          (-726)
                            {DISPID_AMBIENT_TRANSFERPRIORITY, _T("DISPID_AMBIENT_TRANSFERPRIORITY")},  // (-728)
                            {DISPID_AMBIENT_RIGHTTOLEFT, _T("DISPID_AMBIENT_RIGHTTOLEFT")},  //      (-732)
                            {DISPID_AMBIENT_TOPTOBOTTOM, _T("DISPID_AMBIENT_TOPTOBOTTOM")}  //      (-733)
    };

    TCHAR szBuff[255];

    for (int i = 0; i < ALEN(aDISPIDS); i++)
    {
        if (dispidMember == aDISPIDS[i].dispid)
        {
            swprintf_s(szBuff, 255, L"MSHTML requesting '%s'\n", aDISPIDS[i].szDesc);
            ODS(szBuff);
            return;
        }
    }
    
    swprintf_s(szBuff, 255, L"MSHTML requesting DISPID '%d'\n", dispidMember);
    ODS(szBuff);
}

STDMETHODIMP CAppWebBrowser::QueryInterface(REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    if (IID_IUnknown == riid)
    {
        *ppv = (IUnknown*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IDispatch) == riid)
    {
        *ppv = (IDispatch*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IWebBrowser) == riid)
    {
        *ppv = (IWebBrowser*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IWebBrowserApp) == riid)
    {
        *ppv = (IWebBrowserApp*)this;
        AddRef();
        return NOERROR;
    }
    else if (__uuidof(IWebBrowser2) == riid)
    {
        *ppv = (IWebBrowser2*)this;
        AddRef();
        return NOERROR;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG) CAppWebBrowser::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) CAppWebBrowser::Release()
{
    long currentCount = InterlockedDecrement(&m_refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

typedef HRESULT(STDAPICALLTYPE* FN_InitializeLocalHtmlEngine)();
HRESULT CApp::PrivateCoCreateForEdgeHtml(
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD dwClsContext,
    REFIID iid,
    LPVOID* ppunk
    )
{
    HRESULT hr = NOERROR;
    CComPtr <IClassFactory> pClassFactory;
    FN_DllGetClassObject pProc = NULL;
    FN_InitializeLocalHtmlEngine pProcEdgeHtml = NULL;
    if (hInstance == nullptr)
    {
        hInstance = LoadLibraryEx(L"edgehtml.dll", nullptr, 0);
        if (hInstance == NULL)
        {
            return E_FAIL;
        }
    }

    pProcEdgeHtml = (FN_InitializeLocalHtmlEngine)GetProcAddress(hInstance, "InitializeLocalHtmlEngine");
    if (pProcEdgeHtml == NULL) return E_FAIL;
    IfFailGo(pProcEdgeHtml());

    pProc = (FN_DllGetClassObject)GetProcAddress(hInstance, "DllGetClassObject");
    if (pProc == NULL) return E_FAIL;

    IfFailGo(pProc(rclsid, __uuidof(IClassFactory), (LPVOID*)&pClassFactory));
    IfFailGo(pClassFactory->CreateInstance(pUnkOuter, iid, ppunk));
Error:
    return hr;
}

