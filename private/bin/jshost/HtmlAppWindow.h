//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

using namespace std;

class CAppWindow : public IOleInPlaceFrame
{
public:
    CAppWindow()
        : _dwRef(1), _hwnd(nullptr)
    {}

    static void RegisterClass(HINSTANCE hInstance);
    HWND CreateHWnd(DWORD dwStyle, HWND hwndParent, int x, int y, int cx, int cy);
    HWND GetWindow() { return _hwnd; }

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IOleWindow
    STDMETHOD(GetWindow)(
        /* [out] */ __RPC__deref_out_opt HWND *phwnd);
    STDMETHOD(ContextSensitiveHelp)(
        /* [in] */ BOOL fEnterMode);

    // IOleInPlaceUIWindow
    STDMETHOD(GetBorder)(
        /* [out] */ __RPC__out LPRECT lprectBorder);
    STDMETHOD(RequestBorderSpace)(
        /* [unique][in] */ __RPC__in_opt LPCBORDERWIDTHS pborderwidths);
    STDMETHOD(SetBorderSpace)(
        /* [unique][in] */ __RPC__in_opt LPCBORDERWIDTHS pborderwidths);
    STDMETHOD(SetActiveObject)(
        /* [unique][in] */ __RPC__in_opt IOleInPlaceActiveObject *pActiveObject,
        /* [unique][string][in] */ __RPC__in_opt_string LPCOLESTR pszObjName);

    // IOleInPlaceFrame
    STDMETHOD(InsertMenus)(
        /* [in] */ __RPC__in HMENU hmenuShared,
        /* [out][in] */ __RPC__inout LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHOD(SetMenu)(
        /* [in] */ __RPC__in HMENU hmenuShared,
        /* [in] */ __RPC__in HOLEMENU holemenu,
        /* [in] */ __RPC__in HWND hwndActiveObject);
    STDMETHOD(RemoveMenus)(
        /* [in] */ __RPC__in HMENU hmenuShared);
    STDMETHOD(SetStatusText)(
        /* [unique][in] */ __RPC__in_opt LPCOLESTR pszStatusText);
    STDMETHOD(EnableModeless)(
        /* [in] */ BOOL fEnable);
    STDMETHOD(TranslateAccelerator)(
        /* [in] */ __RPC__in LPMSG lpmsg,
        /* [in] */ WORD wID);

private:
    static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    static PCWSTR const s_szWindowClass;

    DWORD _dwRef;
    HWND _hwnd;
};
