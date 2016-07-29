//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

HANDLE JitProcessManager::s_rpcServerProcessHandle = 0; // 0 is the "invalid handle" value for process handles
UUID JitProcessManager::s_connectionId = GUID_NULL;

HRESULT JitProcessManager::StartRpcServer(int argc, __in_ecount(argc) LPWSTR argv[])
{
    HRESULT hr = S_OK;

    if (IsEqualGUID(s_connectionId, GUID_NULL))
    {
        RPC_STATUS status = UuidCreate(&s_connectionId);
        if (status == RPC_S_OK || status == RPC_S_UUID_LOCAL_ONLY)
        {
            hr = CreateServerProcess(argc, argv);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(status);
        }
    }

    return hr;
}

/* static */
void
JitProcessManager::RemoveArg(LPCWSTR flag, int * argc, __in_ecount(*argc) LPWSTR * argv[])
{
    size_t flagLen = wcslen(flag);
    int flagIndex;
    while ((flagIndex = HostConfigFlags::FindArg(*argc, *argv, flag, flagLen)) >= 0)
    {
        HostConfigFlags::RemoveArg(*argc, *argv, flagIndex);
    }
}

HRESULT JitProcessManager::CreateServerProcess(int argc, __in_ecount(argc) LPWSTR argv[])
{
    HRESULT hr;
    PROCESS_INFORMATION processInfo = { 0 };
    STARTUPINFOW si = { 0 };

    RemoveArg(_u("-dynamicprofilecache:"), &argc, &argv);
    RemoveArg(_u("-dynamicprofileinput:"), &argc, &argv);

    // overallocate constant cmd line (jshost -jitserver:<guid>)
    size_t cmdLineSize = (MAX_PATH + argc) * sizeof(WCHAR);
    for (int i = 0; i < argc; ++i)
    {
        // calculate space requirement for each arg
        cmdLineSize += wcslen(argv[i]) * sizeof(WCHAR);
    }

    WCHAR* cmdLine = (WCHAR*)malloc(cmdLineSize);
    if (cmdLine == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    WCHAR* connectionUuidString = NULL;

    hr = StringCchCopyW(cmdLine, cmdLineSize, L"JsHost.exe -jitserver:");
    if (FAILED(hr))
    {
        return hr;
    }

    RPC_STATUS status = UuidToStringW(&s_connectionId, &connectionUuidString);
    if (status != S_OK)
    {
        return HRESULT_FROM_WIN32(status);
    }

    hr = StringCchCatW(cmdLine, cmdLineSize, connectionUuidString);
    if (FAILED(hr))
    {
        return hr;
    }

    for (int i = 1; i < argc; ++i)
    {
        hr = StringCchCatW(cmdLine, cmdLineSize, L" ");
        if (FAILED(hr))
        {
            return hr;
        }
        hr = StringCchCatW(cmdLine, cmdLineSize, argv[i]);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (!CreateProcessW(
        NULL,
        cmdLine,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        &si,
        &processInfo))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    free(cmdLine);

    if (ResumeThread(processInfo.hThread) == (DWORD)-1)
    {
        TerminateProcess(processInfo.hProcess, GetLastError());
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    CloseHandle(processInfo.hThread);
    s_rpcServerProcessHandle = processInfo.hProcess;

    // create job object so if parent jshost gets killed, server is killed as well
    HANDLE jobObject = CreateJobObject(nullptr, nullptr);
    if (jobObject == nullptr)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (!AssignProcessToJobObject(jobObject, s_rpcServerProcessHandle))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = {0};
    jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    if (!SetInformationJobObject(jobObject, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return NOERROR;
}

void JitProcessManager::StopRpcServer()
{
    // For now we just kill the process
    TerminateProcess(s_rpcServerProcessHandle, 1);

    CloseHandle(s_rpcServerProcessHandle);
    s_rpcServerProcessHandle = NULL;
}

HANDLE JitProcessManager::GetRpcProccessHandle()
{
    return s_rpcServerProcessHandle;
}

UUID JitProcessManager::GetRpcConnectionId()
{
    return s_connectionId;
}

