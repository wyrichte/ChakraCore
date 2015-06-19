//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

extern "C" 
{
    BOOL WINAPI JscriptInfoPrxDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
    STDAPI JscriptInfoPrxDllCanUnloadNow(void);
    STDAPI JscriptInfoPrxDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    STDAPI JscriptInfoPrxDllRegisterServer(void);
    STDAPI JscriptInfoPrxDllUnregisterServer(void);
}
