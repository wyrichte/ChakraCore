//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

void CAppWindow::RegisterClass(HINSTANCE hInstance)
{
    g_hInst = hInstance;

    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = CAppWindow::s_WndProc;
    wcex.cbWndExtra = sizeof(CAppWindow *);
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH) GetStockObject (WHITE_BRUSH);
    wcex.lpszClassName = s_szWindowClass;

    RegisterClassEx(&wcex);
}

HWND CAppWindow::CreateHWnd(DWORD dwStyle, HWND hwndParent, int x, int y, int cx, int cy)
{
    return (_hwnd = CreateWindowEx(0, s_szWindowClass, nullptr, dwStyle, x, y, cx, cy, hwndParent, nullptr, g_hInst, this));
}

LRESULT CALLBACK CAppWindow::s_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (g_pApp)
        {
            RECT rect;
            SetRect(&rect, 0, 0, LOWORD(lParam), HIWORD(lParam));
            g_pApp->OnPosRectChange(&rect);
        }
        break;

    case WM_DESTROY:
        {
            QuitHtmlHost();
            return 0;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

// IUnknown
STDMETHODIMP CAppWindow::QueryInterface(REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    if (__uuidof(IOleInPlaceFrame) == riid
        || __uuidof(IOleInPlaceUIWindow) == riid
        || __uuidof(IOleWindow) == riid
        || IID_IUnknown == riid)
    {
        *ppv = (IOleInPlaceFrame*)this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CAppWindow::AddRef()
{
    return ++_dwRef;
}

STDMETHODIMP_(ULONG) CAppWindow::Release()
{
    if (--_dwRef == 0)
    {
        ODS(_u("Deleting CAppWindow\n"));
        delete this;
        return 0;
    }

    return _dwRef;
}

// IOleWindow
STDMETHODIMP CAppWindow::GetWindow(
    /* [out] */ __RPC__deref_out_opt HWND *phwnd)
{
    *phwnd = _hwnd;
    return S_OK;
}

STDMETHODIMP CAppWindow::ContextSensitiveHelp(
    /* [in] */ BOOL fEnterMode)
{
    return S_OK;
}

// IOleInPlaceUIWindow
STDMETHODIMP CAppWindow::GetBorder(
    /* [out] */ __RPC__out LPRECT lprectBorder)
{
    GetClientRect(_hwnd, lprectBorder);
    return S_OK;
}

STDMETHODIMP CAppWindow::RequestBorderSpace(
    /* [unique][in] */ __RPC__in_opt LPCBORDERWIDTHS pborderwidths)
{
    return S_OK;
}

STDMETHODIMP CAppWindow::SetBorderSpace(
    /* [unique][in] */ __RPC__in_opt LPCBORDERWIDTHS pborderwidths)
{
    return S_OK;
}

STDMETHODIMP CAppWindow::SetActiveObject(
    /* [unique][in] */ __RPC__in_opt IOleInPlaceActiveObject *pActiveObject,
    /* [unique][string][in] */ __RPC__in_opt_string LPCOLESTR pszObjName)
{
    return S_OK;
}

// IOleInPlaceFrame
STDMETHODIMP CAppWindow::InsertMenus(
    /* [in] */ __RPC__in HMENU hmenuShared,
    /* [out][in] */ __RPC__inout LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    return S_OK;
}

STDMETHODIMP CAppWindow::SetMenu(
    /* [in] */ __RPC__in HMENU hmenuShared,
    /* [in] */ __RPC__in HOLEMENU holemenu,
    /* [in] */ __RPC__in HWND hwndActiveObject)
{
    return S_OK;
}

STDMETHODIMP CAppWindow::RemoveMenus(
    /* [in] */ __RPC__in HMENU hmenuShared)
{
    return S_OK;
}

STDMETHODIMP CAppWindow::SetStatusText(
    /* [unique][in] */ __RPC__in_opt LPCOLESTR pszStatusText)
{
    return S_OK;
}

STDMETHODIMP CAppWindow::EnableModeless(
    /* [in] */ BOOL fEnable)
{
    return S_OK;
}

STDMETHODIMP CAppWindow::TranslateAccelerator(
    /* [in] */ __RPC__in LPMSG lpmsg,
    /* [in] */ WORD wID)
{
    return S_FALSE;
}
