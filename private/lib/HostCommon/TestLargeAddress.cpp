//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "HostCommonPch.h"
#include "TestLargeAddress.h"

#include <process.h>
int _cdecl wmain(int argc, __in_ecount(argc) LPWSTR argv[]);

bool UseLargeAddresses(int& argc, __in_ecount(argc) LPWSTR argv[])
{
    bool useLargeAddress = false;
    
#if _M_X64
#if DBG
    // Default to always test for large address on amd64 debug build
    useLargeAddress = true;
#endif
#else
    ::MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    ::GlobalMemoryStatusEx(&mem);
    if (mem.ullTotalVirtual <= (unsigned)2 * (1024 * 1024 * 1024))
    {
        // Not large address aware
        return false;
    }
#endif
    for (int i = 0; i < argc;)
    {
        bool foundSwitch = false;
        if ((argv[i][0] == _u('-') || argv[i][0] == _u('/')))
        {            
            if (_wcsicmp(&argv[i][1], _u("largeaddress")) == 0)
            {
                useLargeAddress = true;
                foundSwitch = true;
            }
            else if (_wcsicmp(&argv[i][1], _u("largeaddress-")) == 0)
            {
                useLargeAddress = false;
                foundSwitch = true;
            }            
        }

        if (foundSwitch)
        {
            memmove(&argv[i], &argv[i + 1], sizeof(LPWSTR) * (argc - i - 1));
            argc--;
            argv[argc] = NULL;
        }
        else
        {
            i++;
        }
    }
    return useLargeAddress;
}

struct LargeAddressThreadProcParam
{
    MainFunc pfunc;
    int argc;
    LPWSTR * argv;
};

unsigned int WINAPI
LargeAddressThreadProc(LPVOID lpParameter)
{
    LargeAddressThreadProcParam * param = (LargeAddressThreadProcParam *)lpParameter;
    
    return param->pfunc(param->argc, param->argv);
}

int 
TestLargeAddress(int argc, __in_ecount(argc) LPWSTR argv[], MainFunc pfunc)
{
    // Reserve all lower 4G memory space    

    unsigned int const allocationGranuality = 64 * 1024;
#if _M_X64
    unsigned __int64 const endAddress = (unsigned __int64)4 * (1024 * 1024 * 1024);
#else
    unsigned __int64 const endAddress = (unsigned __int64)2 * (1024 * 1024 * 1024);
#endif
    unsigned __int64 lastAddress = endAddress;

    // Try reserve 2G memory space    
    unsigned __int64 address = (unsigned __int64)VirtualAlloc(NULL, endAddress >> 1, MEM_RESERVE, PAGE_NOACCESS);
    if (address != NULL)
    {
        if ( (unsigned __int64)address < endAddress)
        {
            // We have reserved some of the lower 4G in the 2G reserve
            if (address + (endAddress >> 1) < endAddress)
            {
                // Allocate the end of the lower 4G
                for (unsigned __int64 i = address + (endAddress >> 1); i < endAddress; i += allocationGranuality)
                {
                    address = (unsigned __int64)VirtualAlloc((LPVOID)i, allocationGranuality, MEM_RESERVE, PAGE_NOACCESS);
                }
            }

            // Fill out the gap in the begining of the lower 4G
            lastAddress = address;
        }
        else
        {
            // The 4G we reserve is not in the lower 4G, just free it and do it the slow way
            VirtualFree((LPVOID)address, 0, MEM_RELEASE);
        }
    }

    for (unsigned __int64 i = allocationGranuality; i < lastAddress; i += allocationGranuality)
    {
        address = (unsigned __int64)VirtualAlloc((LPVOID)i, allocationGranuality, MEM_RESERVE, PAGE_NOACCESS);
    }

    // Create an separate thread to run the test so the stack addresses can be large as well.
    LargeAddressThreadProcParam threadParam = { pfunc, argc, argv };

    HANDLE threadHandle = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, LargeAddressThreadProc, &threadParam, 0, NULL));
    if (threadHandle == NULL)
    {
        wprintf(_u("FATAL ERROR: Large address thread create failed\n"));
        exit(1);
    }

    // Wait until the test is finished.
    if (WaitForSingleObject(threadHandle, INFINITE) != WAIT_OBJECT_0)
    {
        wprintf(_u("FATAL ERROR: WaitForSingleObject on large address thread failed\n"));
        exit(1);
    }

    // Get the exit code an return it.
    DWORD exitCode;
    if (!::GetExitCodeThread(threadHandle, &exitCode))
    {
        wprintf(_u("FATAL ERROR: GetExitCodeThread on large address thread failed\n"));
        exit(1);
    }
    return exitCode;
}
