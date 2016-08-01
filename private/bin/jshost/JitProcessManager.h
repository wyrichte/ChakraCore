//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class JitProcessManager
{
public:
    static HRESULT StartRpcServer(int argc, __in_ecount(argc) LPWSTR argv[]);
    static void StopRpcServer();

    static DWORD GetRpcProccessId();
    static UUID GetRpcConnectionId();
    static void RemoveArg(LPCWSTR flag, int * argc, __in_ecount(*argc) LPWSTR * argv[]);

private:
    static HRESULT CreateServerProcess(int argc, __in_ecount(argc) LPWSTR argv[]);    

    static HANDLE s_rpcServerProcessHandle;
    static UUID s_connectionId;
};
