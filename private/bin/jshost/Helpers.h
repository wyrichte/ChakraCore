/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once
HRESULT JsHostLoadScriptFromFile(LPCWSTR filename, LPCWSTR& contents, bool* isUtf8Out = NULL, LPCWSTR* contentsRawOut = NULL, UINT* lengthBytesOut = NULL, bool printFileOpenError = true);
void GetShortNameFromUrl(__in LPCWSTR pchUrl, __in LPWSTR pchShortName, __in size_t cchBuffer);
HRESULT PrivateCoCreate(HINSTANCE hInstModule, REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID iid, LPVOID* ppunk);
HRESULT LoadPDM(__out HINSTANCE* phInstPdm, IProcessDebugManager ** ppPDM);
typedef HRESULT(STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
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
