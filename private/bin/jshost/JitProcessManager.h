//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class JitProcessManager
{
public:
    static HRESULT StartRpcServer();
    static void StopRpcServer();

    static DWORD GetRpcProccessId();
    static UUID GetRpcConnectionId();

private:
    static HRESULT CreateServerProcess();

    static HANDLE s_rpcServerProcessHandle;
    static UUID s_connectionId;
};
