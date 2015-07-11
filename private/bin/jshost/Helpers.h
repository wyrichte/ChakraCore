/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once
HRESULT JsHostLoadScriptFromFile(LPCWSTR filename, LPCWSTR& contents, bool* isUtf8Out = NULL, LPCWSTR* contentsRawOut = NULL, UINT* lengthBytesOut = NULL, bool printFileOpenError = true);
void GetShortNameFromUrl(__in LPCWSTR pchUrl, __in LPWSTR pchShortName, __in size_t cchBuffer);
HRESULT PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID iid, LPVOID* ppunk);
HRESULT LoadPDM(__deref_out_z WCHAR ** ppPdmPath, IProcessDebugManager ** ppPDM);
typedef HRESULT(STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
