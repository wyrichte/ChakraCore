//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class CAppWebBrowser;

class CApp : public IPropertyNotifySink, IOleClientSite, IOleInPlaceSite, IDocHostUIHandler, IOleCommandTarget, IServiceProvider, IDispatch, IBrowserService, IDocObjectService
{
public:
    CApp();
    ~CApp();

    HRESULT Passivate();

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IPropertyNotifySink methods
    STDMETHOD(OnChanged)(DISPID dispID);
    STDMETHOD(OnRequestEdit)(DISPID dispID);

    // IOleClientSite methods
    STDMETHOD(SaveObject)(void) { ODS(L"SaveObject\n"); return E_NOTIMPL; }

    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk) { ODS(L"GetMoniker\n"); return E_NOTIMPL; }

    STDMETHOD(GetContainer)(IOleContainer** ppContainer)
            { ODS(L"GetContainer\n"); return E_NOTIMPL; }

    STDMETHOD(ShowObject)(void)
            { ODS(L"ShowObject\n"); return E_NOTIMPL; }

    STDMETHOD(OnShowWindow)(BOOL fShow)
            { ODS(L"OnShowWindow\n"); return E_NOTIMPL; }

    STDMETHOD(RequestNewObjectLayout)(void)
            { ODS(L"RequestNewObjectLayout\n"); return E_NOTIMPL; }

    // IOleWindow
    STDMETHOD(GetWindow)(
        /* [out] */ __RPC__deref_out_opt HWND *phwnd);
    STDMETHOD(ContextSensitiveHelp)(
        /* [in] */ BOOL fEnterMode)                     { return S_OK; }

    // IOleInPlaceSite
    STDMETHOD(CanInPlaceActivate)(void)                 { return S_OK; }
    STDMETHOD(OnInPlaceActivate)(void)                  { return S_OK; }
    STDMETHOD(OnUIActivate)(void)                       { return S_OK; }
    STDMETHOD(GetWindowContext)(
        /* [out] */ __RPC__deref_out_opt IOleInPlaceFrame **ppFrame,
        /* [out] */ __RPC__deref_out_opt IOleInPlaceUIWindow **ppDoc,
        /* [out] */ __RPC__out LPRECT lprcPosRect,
        /* [out] */ __RPC__out LPRECT lprcClipRect,
        /* [out][in] */ __RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHOD(Scroll)(
        /* [in] */ SIZE scrollExtant)                   { return S_OK; }
    STDMETHOD(OnUIDeactivate)(
        /* [in] */ BOOL fUndoable)                      { return S_OK; }
    STDMETHOD(OnInPlaceDeactivate)(void)                { return S_OK; }
    STDMETHOD(DiscardUndoState)(void)                   { return S_OK; }
    STDMETHOD(DeactivateAndUndo)(void)                  { return S_OK; }
    STDMETHOD(OnPosRectChange)(
        /* [in] */ __RPC__in LPCRECT lprcPosRect);

    // IDocHostUIHandler methods
    STDMETHOD(ShowContextMenu)(DWORD dwID,
            POINT *ppt,
            IUnknown *pcmdtReserved,
            IDispatch *pdispReserved) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOTIMPL; }
    STDMETHOD(GetHostInfo)(
            DOCHOSTUIINFO *pInfo) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOTIMPL; }
    STDMETHOD(ShowUI)(
            DWORD dwID,
            IOleInPlaceActiveObject *pActiveObject,
            IOleCommandTarget *pCommandTarget,
            IOleInPlaceFrame *pFrame,
            IOleInPlaceUIWindow *pDoc) { ODS(__WFUNCTION__); ODS(L"\n"); return S_OK; }
    STDMETHOD(HideUI)(void) { ODS(__WFUNCTION__); ODS(L"\n"); return S_OK; }
    STDMETHOD(UpdateUI)(void) { ODS(__WFUNCTION__); ODS(L"\n"); return S_OK; }
    STDMETHOD(EnableModeless)(BOOL fEnable) { ODS(__WFUNCTION__); ODS(L"\n"); return S_OK; }
    STDMETHOD(OnDocWindowActivate)(BOOL fEnable) { ODS(__WFUNCTION__); ODS(L"\n"); return S_OK; }
    STDMETHOD(OnFrameWindowActivate)(BOOL fEnable) { ODS(__WFUNCTION__); ODS(L"\n"); return S_OK; }
    STDMETHOD(ResizeBorder)(
            LPCRECT prcBorder,
            IOleInPlaceUIWindow *pUIWindow,
            BOOL fRameWindow) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOTIMPL; }
    STDMETHOD(TranslateAccelerator)(
            LPMSG lpMsg,
            const GUID *pguidCmdGroup,
            DWORD nCmdID) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOTIMPL; }
    STDMETHOD(GetOptionKeyPath)(
            __deref_out LPOLESTR *pchKey,
            DWORD dw) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOTIMPL; }
    STDMETHOD(GetDropTarget)(
            IDropTarget *pDropTarget,
            IDropTarget **ppDropTarget) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOTIMPL; }
    STDMETHOD(GetExternal)( 
            IDispatch **ppDispatch) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOINTERFACE; } // window.external
    STDMETHOD(TranslateUrl)(
            DWORD dwTranslate,
            __in_opt OLECHAR *pchURLIn,
            __deref_out OLECHAR **ppchURLOut) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOTIMPL; }
    STDMETHOD(FilterDataObject)( 
            IDataObject *pDO,
            IDataObject **ppDORet) { ODS(__WFUNCTION__); ODS(L"\n"); return E_NOTIMPL; }

    // IOleCommandTarget
    STDMETHOD(QueryStatus)(
        /* [unique][in] */ __RPC__in_opt const GUID *pguidCmdGroup,
        /* [in] */ ULONG cCmds,
        /* [out][in][size_is] */ __RPC__inout_ecount_full(cCmds) OLECMD prgCmds[],
        /* [unique][out][in] */ __RPC__inout_opt OLECMDTEXT *pCmdText);
    STDMETHOD(Exec)(
        /* [unique][in] */ __RPC__in_opt const GUID *pguidCmdGroup,
        /* [in] */ DWORD nCmdID,
        /* [in] */ DWORD nCmdexecopt,
        /* [unique][in] */ __RPC__in_opt VARIANT *pvaIn,
        /* [unique][out][in] */ __RPC__inout_opt VARIANT *pvaOut);

    // IServiceProvider method
    STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppv);

    // IDispatch method
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
            { ODS(L"GetTypeInfoCount\n"); return E_NOTIMPL; }

    STDMETHOD(GetTypeInfo)(UINT iTInfo,
            LCID lcid,
            ITypeInfo** ppTInfo)
            { ODS(L"GetTypeInfo\n"); return E_NOTIMPL; }

    STDMETHOD(GetIDsOfNames)(REFIID riid,
            __in_ecount(cNames) LPOLESTR *rgszNames,
            UINT cNames,
            LCID lcid,
            DISPID* rgDispId)
            { ODS(L"GetIDsOfNames\n"); return E_NOTIMPL; }

    STDMETHOD(Invoke)(DISPID dispIdMember,
            REFIID riid,
            LCID lcid,
            WORD wFlags,
            DISPPARAMS *pDispParams,
            VARIANT *pVarResult,
            EXCEPINFO *pExcepInfo,
            UINT *puArgErr);

    // IBrowserService methods
    // We don't actually need these - we just need to claim to be an IBrowserService
    // to get past the QueryService in CDoc::InitDocHost and successfully register as
    // an IDocObjectService element
    STDMETHOD(CacheOLEServer)(__in IOleObject *pole) { return E_NOTIMPL; }
    STDMETHOD(CanNavigateNow)(void) { return E_NOTIMPL; }
    STDMETHOD(DisplayParseError)(HRESULT hres, __in LPCWSTR pwszPath) { return E_NOTIMPL; }
    STDMETHOD(GetBrowserByIndex)(DWORD dwID, __deref_out IUnknown **ppunk) { return E_NOTIMPL; }
    STDMETHOD_(DWORD, GetBrowserIndex)(void) { return (DWORD)E_NOTIMPL; }
    STDMETHOD(GetFlags)(__out DWORD *pdwFlags) { return E_NOTIMPL; }
    STDMETHOD(GetHistoryObject)(__deref_out IOleObject **ppole, __deref_out IStream **ppstm, __deref_out IBindCtx **ppbc) { return E_NOTIMPL; }
    STDMETHOD(GetNavigateState)(BNSTATE *pbnstate) { return E_NOTIMPL; }
    STDMETHOD(GetOleObject)(__deref_out IOleObject** ppobjv) { return E_NOTIMPL; }
    STDMETHOD(GetPalette)(__out HPALETTE * hpal) { return E_NOTIMPL; }
    STDMETHOD(GetParentSite)(__deref_out IOleInPlaceSite** ppipsite) { return E_NOTIMPL; }
    STDMETHOD(GetPidl)(__deref_out PIDLIST_ABSOLUTE *ppidl) { return E_NOTIMPL; }
    STDMETHOD(GetSetCodePage)(__in VARIANT* pvarIn, __out VARIANT* pvarOut) { return E_NOTIMPL; }
    STDMETHOD(GetTitle)(__in IShellView *psv, __out_ecount(cchName) LPWSTR pszName, DWORD cchName) { return E_NOTIMPL; }
    STDMETHOD(GetTravelLog)(__deref_out ITravelLog **pptl) { return E_NOTIMPL; }
    STDMETHOD(IEGetDisplayName)(__in PCIDLIST_ABSOLUTE pidl, __out_ecount(MAX_PATH) LPWSTR pwszName, UINT uFlags) { return E_NOTIMPL; }
    STDMETHOD(IEParseDisplayName)(UINT uiCP, __in LPCWSTR pwszPath, __deref_out PIDLIST_ABSOLUTE * ppidlOut) { return E_NOTIMPL; }
    STDMETHOD(IsControlWindowShown)(UINT id, BOOL *pfShown) { return E_NOTIMPL; }
    STDMETHOD(NavigateToPidl)(__in PCIDLIST_ABSOLUTE pidl, DWORD grfHLNF) { return E_NOTIMPL; }
    STDMETHOD(NotifyRedirect)(__in IShellView* psv, __in PCIDLIST_ABSOLUTE pidl, __out BOOL *pfDidBrowse) { return E_NOTIMPL; }
    STDMETHOD(OnHttpEquiv)(__in IShellView* psv, BOOL fDone, __in VARIANT *pvarargIn, __out VARIANT *pvarargOut) { return E_NOTIMPL; }
    STDMETHOD(RegisterWindow)(BOOL fUnregister, int swc) { return E_NOTIMPL; }
    STDMETHOD(SetFlags)(DWORD dwFlags, DWORD dwFlagMask) { return E_NOTIMPL; }
    STDMETHOD(SetHistoryObject)(__in IOleObject *pole, BOOL fIsLocalAnchor) { return E_NOTIMPL; }
    STDMETHOD(SetNavigateState)(BNSTATE bnstate) { return E_NOTIMPL; }
    STDMETHOD(SetReferrer)(__in PCIDLIST_ABSOLUTE pidl) { return E_NOTIMPL; }
    STDMETHOD(SetTitle)(__in IShellView *psv, __in LPCWSTR pszName) { return E_NOTIMPL; }
    STDMETHOD(ShowControlWindow)(UINT id, BOOL fShow) { return E_NOTIMPL; }
    STDMETHOD(UpdateBackForwardState)(void) { return E_NOTIMPL; }
    STDMETHOD(UpdateWindowList)(void) { return E_NOTIMPL; }

    // IDocObjectService methods
    STDMETHOD(FireBeforeNavigate2)(
        __in IDispatch* pDispatch,
        __in LPCWSTR lpszUrl,
        __in DWORD dwFlags,
        __in LPCWSTR lpszFrameName,
        __in BYTE* pPostData,
        __in DWORD cbPostData,
        __in LPCWSTR lpszHeaders,
        __in BOOL fPlayNavSound,
        __out BOOL* pfCancel) { return E_NOTIMPL; }

    STDMETHOD(FireNavigateComplete2)(__in IHTMLWindow2* pHTMLWindow2, __in DWORD dwFlags) { return E_NOTIMPL; }
    STDMETHOD(FireDownloadBegin)()  { return E_NOTIMPL; }
    STDMETHOD(FireDownloadComplete)()  { return E_NOTIMPL; }
    STDMETHOD(FireDocumentComplete)(__in IHTMLWindow2* pHTMLWindow, __in DWORD dwFlags)  { return E_NOTIMPL; }
    STDMETHOD(UpdateDesktopComponent)(__in IHTMLWindow2* pHTMLWindow) { return E_NOTIMPL; }
    STDMETHOD(GetPendingUrl)(__out BSTR* pbstrPendingUrl) { return E_NOTIMPL; }
    STDMETHOD(ActiveElementChanged)(IHTMLElement* pHTMLElement) { return E_NOTIMPL; }
    STDMETHOD(GetUrlSearchComponent)(__out BSTR* pbstrSearch) { return E_NOTIMPL; }
    STDMETHOD(IsErrorUrl)(__in LPCWSTR lpszUrl, __out BOOL* pfIsError) { return E_NOTIMPL; }

    // Additional class methods
    HRESULT Init(CAppWindow* pAppWindow);
    HRESULT InitAfterLoad();
    HRESULT ProcessArgs(__in_ecount(urlLen) LPCWSTR url, size_t urlLen);
    HRESULT OnPageLoaded();
    HRESULT Run();
    HRESULT Term();
    void    SetKeepAlive(bool keepAlive) { m_keepAlive = keepAlive; }
    HRESULT PrivateCoCreateForEdgeHtml(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID iid, LPVOID* ppunk);
    
private:
    // Persistence helpers
    HRESULT LoadURLFromFile();
    HRESULT LoadURLFromMoniker();
    void SetHostToDebugMode();

    IHTMLDocument2* m_pMSHTML;
    DWORD m_dwRef;
    DWORD m_dwCookie;
    LPCONNECTIONPOINT m_pCP;
    HRESULT m_hrConnected;
    LPTSTR m_szURL;
    INTERNET_SCHEME m_nScheme;
    READYSTATE m_lReadyState;
    HINSTANCE hInstance;

    CAppWindow* m_pAppWindow;
    CAppWebBrowser *m_pWebBrowser;

    CWinSink* m_pWinSink;
    bool m_keepAlive;
};
typedef CApp* LPAPP;

// This dummy implementation is required to make mshtml happy.
// (One assert in the mshtml gets fired in the code path to enable debugging if it doesn't find the IWebBrower2)

class CAppWebBrowser : IWebBrowser2
{
public:
    CAppWebBrowser()
    : m_refCount(1)
    {
    }

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IDispatch methods
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) { return E_NOTIMPL; }

    STDMETHOD(GetTypeInfo)(UINT iTInfo,
        LCID lcid,
        ITypeInfo** ppTInfo)    
    {
        return E_NOTIMPL;
    }

    STDMETHOD(GetIDsOfNames)(REFIID riid,
        __in_ecount(cNames) LPOLESTR *rgszNames,
        UINT cNames,
        LCID lcid,
        DISPID* rgDispId)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(Invoke)(DISPID dispIdMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS *pDispParams,
        VARIANT *pVarResult,
        EXCEPINFO *pExcepInfo,
        UINT *puArgErr)
    {
        return E_NOTIMPL; 
    }

    // IWebBrowser2/IWebBrowser methods 
    //------------------------------------
    STDMETHOD(get_Document)(IDispatch * * Document) { return E_NOTIMPL; }
    STDMETHOD(Navigate)(BSTR URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers) { return E_NOTIMPL; }
    STDMETHOD(Refresh)(void) { return E_NOTIMPL; }
    STDMETHOD(Refresh2)(VARIANT* Level) { return E_NOTIMPL; }
    STDMETHOD(get_LocationURL)(BSTR* pbstrLocationURL) { return E_NOTIMPL; }
    STDMETHOD(get_LocationName)(BSTR* pbstrLocationName) { return E_NOTIMPL; }
    STDMETHOD(get_HWND)(LONG_PTR* pHWND) { return E_NOTIMPL; }
    STDMETHOD(put_ToolBar)(__in int Value) { return E_NOTIMPL; }
    STDMETHOD(get_Resizable)(__out VARIANT_BOOL* Value) { return E_NOTIMPL; }
    STDMETHOD(get_Left)(__out long* pl) { return E_NOTIMPL; }
    STDMETHOD(get_Top)(__out long* pl) { return E_NOTIMPL; }
    STDMETHOD(GoBack)(void) { return E_NOTIMPL; }
    STDMETHOD(GoForward)(void) { return E_NOTIMPL; }
    STDMETHOD(GoHome)(void) { return E_NOTIMPL; }
    STDMETHOD(GoSearch)(void) { return E_NOTIMPL; }
    STDMETHOD(Stop)(void) { return E_NOTIMPL; }
    STDMETHOD(get_Application)(IDispatch ** ppDisp) { return E_NOTIMPL; }
    STDMETHOD(get_Parent)(IDispatch ** ppDisp) { return E_NOTIMPL; }
    STDMETHOD(get_Container)(IDispatch ** ppDisp) { return E_NOTIMPL; }
    STDMETHOD(get_TopLevelContainer)(VARIANT_BOOL* pBool) { return E_NOTIMPL; }
    STDMETHOD(get_Type)(BSTR* pbstrType) { return E_NOTIMPL; }
    STDMETHOD(put_Left)(long Left) { return E_NOTIMPL; }
    STDMETHOD(put_Top)(long Top) { return E_NOTIMPL; }
    STDMETHOD(get_Width)(__out long* pl) { return E_NOTIMPL; }
    STDMETHOD(put_Width)(long Width) { return E_NOTIMPL; }
    STDMETHOD(get_Height)(__out long* pl) { return E_NOTIMPL; }
    STDMETHOD(put_Height)(long Height) { return E_NOTIMPL; }
    STDMETHOD(get_Busy)(VARIANT_BOOL* pBool) { return E_NOTIMPL; }
    STDMETHOD(Quit)(void) { return E_NOTIMPL; }
    STDMETHOD(ClientToWindow)(int* pcx, int* pcy) { return E_NOTIMPL; }
    STDMETHOD(PutProperty)(BSTR szProperty, VARIANT vtValue) { return E_NOTIMPL; }
    STDMETHOD(GetProperty)(BSTR szProperty, VARIANT* pvtValue) { return E_NOTIMPL; }
    STDMETHOD(get_Name)(BSTR* pbstrName) { return E_NOTIMPL; }
    STDMETHOD(get_FullName)(BSTR* pbstrFullName) { return E_NOTIMPL; }
    STDMETHOD(get_Path)(BSTR* pbstrPath) { return E_NOTIMPL; }
    STDMETHOD(get_FullScreen)(__out VARIANT_BOOL* pBool) { return E_NOTIMPL; }
    STDMETHOD(put_FullScreen)(VARIANT_BOOL Value) { return E_NOTIMPL; }
    STDMETHOD(get_Visible)(VARIANT_BOOL* pBool) { return E_NOTIMPL; }
    STDMETHOD(put_Visible)(VARIANT_BOOL Value) { return E_NOTIMPL; }
    STDMETHOD(get_StatusBar)(VARIANT_BOOL* pBool) { return E_NOTIMPL; }
    STDMETHOD(put_StatusBar)(VARIANT_BOOL Value) { return E_NOTIMPL; }
    STDMETHOD(get_StatusText)(BSTR* pbstr) { return E_NOTIMPL; }
    STDMETHOD(put_StatusText)(BSTR bstr) { return E_NOTIMPL; }
    STDMETHOD(get_ToolBar)(__out int* pnToolbar) { return E_NOTIMPL; }
    STDMETHOD(get_MenuBar)(VARIANT_BOOL* pValue) { return E_NOTIMPL; }
    STDMETHOD(put_MenuBar)(VARIANT_BOOL Value) { return E_NOTIMPL; }
    STDMETHOD(Navigate2)(VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers) { return E_NOTIMPL; }
    STDMETHOD(ShowBrowserBar)(VARIANT* pvaClsid, VARIANT* pvaShow, VARIANT* pvaSize) { return E_NOTIMPL; }
    STDMETHOD(QueryStatusWB)(OLECMDID cmdID, OLECMDF* pcmdf) { return E_NOTIMPL; }
    STDMETHOD(ExecWB)(OLECMDID cmdID, OLECMDEXECOPT cmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut) { return E_NOTIMPL; }
    STDMETHOD(get_ReadyState)(READYSTATE* plReadyState) { return E_NOTIMPL; }
    STDMETHOD(get_Offline)(VARIANT_BOOL* pbOffline) { return E_NOTIMPL; }
    STDMETHOD(put_Offline)(VARIANT_BOOL bOffline) { return E_NOTIMPL; }
    STDMETHOD(get_Silent)(VARIANT_BOOL* pbSilent) { return E_NOTIMPL; }
    STDMETHOD(put_Silent)(VARIANT_BOOL bSilent) { return E_NOTIMPL; }
    STDMETHOD(get_RegisterAsBrowser)(VARIANT_BOOL* pbRegister) { return E_NOTIMPL; }
    STDMETHOD(put_RegisterAsBrowser)(VARIANT_BOOL bRegister) { return E_NOTIMPL; }
    STDMETHOD(get_RegisterAsDropTarget)(__out VARIANT_BOOL* pbRegister) { return E_NOTIMPL; }
    STDMETHOD(put_RegisterAsDropTarget)(VARIANT_BOOL bRegister) { return E_NOTIMPL; }
    STDMETHOD(get_TheaterMode)(__out VARIANT_BOOL* pbRegister) { return E_NOTIMPL; }
    STDMETHOD(put_TheaterMode)(VARIANT_BOOL bRegister) { return E_NOTIMPL; }
    STDMETHOD(get_AddressBar)(VARIANT_BOOL* Value) { return E_NOTIMPL; }
    STDMETHOD(put_AddressBar)(VARIANT_BOOL Value) { return E_NOTIMPL; }
    STDMETHOD(put_Resizable)(VARIANT_BOOL Value) { return E_NOTIMPL; }

private:
    long m_refCount;
};
