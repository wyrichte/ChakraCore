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
