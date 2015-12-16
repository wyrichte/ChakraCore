//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "JsHostJITPch.h"

void __stdcall PrintUsage()
{
    wprintf(L"\n\nUsage: JsHostJIT.exe\n");
}
typedef HRESULT(WINAPI *JsInitializeRpcServerPtr)(UUID* connectionUuid);

int _cdecl wmain(int argc, __in_ecount(argc) WCHAR** argv)
{
    BSTR filename = NULL;
    JScript9Interface::ArgInfo argInfo = { argc, argv, ::PrintUsage, &filename };

    HINSTANCE chakra = JScript9Interface::LoadDll(false, nullptr, argInfo);

    if(!chakra)
    {
        wprintf(L"\nDll load failed\n");
        return ERROR_DLL_INIT_FAILED;
    }
    if (argc != 2)
    {
        return ERROR_INVALID_PARAMETER;
    }

    LPCWSTR connectionUuidString = argv[1];
    UUID connectionUuid;
    DWORD status = UuidFromStringW((RPC_WSTR)connectionUuidString, &connectionUuid);
    if (status != RPC_S_OK)
    {
        return status;
    }

    JsInitializeRpcServerPtr initRpcServer = (JsInitializeRpcServerPtr)GetProcAddress(chakra, "JsInitializeRpcServer");
    HRESULT hr = initRpcServer(&connectionUuid);
    if (FAILED(hr))
    {
        wprintf(L"InitializeRpcServer failed by 0x%x\n", hr);
        return hr;
    }

    return 0;

}
