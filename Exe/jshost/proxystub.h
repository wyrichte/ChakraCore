#pragma once

extern "C" 
{
    BOOL WINAPI JsHostScriptSitePrxDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
    STDAPI JsHostScriptSitePrxDllCanUnloadNow(void);
    STDAPI JsHostScriptSitePrxDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    STDAPI JsHostScriptSitePrxDllRegisterServer(void);
    STDAPI JsHostScriptSitePrxDllUnregisterServer(void);
}
