//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

// Description:top level functions for dllMain and COM server 

#include "stdafx.h"

const IID GUID_NULL = {};

// Copied from com\ole32\comcat\src\guids.cpp
const CLSID CLSID_StdComponentCategoriesMgr = { 0x0002E005,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };

Module _Module;
HINSTANCE Module::g_ModuleInstance = 0;
DWORD Module::majorVersion = 0;
DWORD Module::minorVersion = 0;

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow(void)
{
    return _Module.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    return _Module.DllRegisterServer();
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer(void)
{
    return _Module.DllUnregisterServer();
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    Module::g_ModuleInstance = hInstance;
    return _Module.DllMain(dwReason, lpReserved); 
}

//
// Returns the major and minor version of the loaded binary. If the version info has been fetched once, it will be cached
// and returned without any system calls to find the version number.
//
HRESULT Module::GetFileVersion(DWORD* majorVersion, DWORD* minorVersion)
{
    HRESULT hr = E_FAIL;;
    if(Module::majorVersion == 0 && Module::minorVersion == 0)
    {
        // uninitialized state  - call the system API to get the version info.
        hr = GetFileVersion(/*processHandle*/ nullptr, Module::g_ModuleInstance, majorVersion, minorVersion);

        Module::majorVersion = *majorVersion;
        Module::minorVersion = *minorVersion;
    }
    else if(Module::majorVersion != INVALID_VERSION)
    {
        Assert(Module::minorVersion != INVALID_VERSION);

        // if the cached copy is valid, use it and return S_OK.
        *majorVersion = Module::majorVersion;
        *minorVersion = Module::minorVersion;
        hr = S_OK;
    }
    return hr;
}

HRESULT Module::GetFileVersion(HANDLE process, HINSTANCE module, DWORD* majorVersion, DWORD* minorVersion)
{
    HRESULT hr;
    WCHAR filename[MAX_PATH];
    Assert(module != 0);
    if (process == nullptr)
    {
        if(GetModuleFileNameW(module, filename, _countof(filename)) == 0)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        if(GetModuleFileNameExW(process, module, filename, _countof(filename)) == 0)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    hr = GetVersionInfo(filename, majorVersion, minorVersion);
    return hr;
}

//
// Returns the major and minor version of the binary passed as argument.
//
HRESULT Module::GetVersionInfo(__in LPCWSTR pszPath, DWORD* majorVersion, DWORD* minorVersion)
{
    DWORD   dwTemp;
    DWORD   cbVersionSz;
    HRESULT hr = E_FAIL;
    BYTE*    pVerBuffer = NULL;
    VS_FIXEDFILEINFO* pFileInfo = NULL;
    cbVersionSz = GetFileVersionInfoSizeEx(FILE_VER_GET_LOCALISED, pszPath, &dwTemp);
    if(cbVersionSz > 0)
    {
        pVerBuffer = new(JsDiag::nothrow) BYTE[cbVersionSz];
        if(pVerBuffer)
        {
            if(GetFileVersionInfoEx(FILE_VER_GET_LOCALISED|FILE_VER_GET_NEUTRAL, pszPath, 0, cbVersionSz, pVerBuffer))
            {
                UINT    uiSz = sizeof(VS_FIXEDFILEINFO);
                if(!VerQueryValue(pVerBuffer, L"\\", (LPVOID*)&pFileInfo, &uiSz))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
                else
                {
                    hr = S_OK;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr))
    {
        *majorVersion = pFileInfo->dwFileVersionMS;
        *minorVersion = pFileInfo->dwFileVersionLS;
    }
    else
    {
        *majorVersion = INVALID_VERSION;
        *minorVersion = INVALID_VERSION;
    }
    if(pVerBuffer)
    {
        delete[] pVerBuffer;
    }
    return hr;
}
