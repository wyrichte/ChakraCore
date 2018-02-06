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
    uint const allocationGranuality = 64 * 1024;
#if _M_X64
    uintptr_t const endAddress = (uintptr_t)4 * (1024 * 1024 * 1024);
#else
    uintptr_t const endAddress = (uintptr_t)2 * (1024 * 1024 * 1024);
#endif
    uintptr_t address = allocationGranuality;
    while (address < endAddress)
    {
        MEMORY_BASIC_INFORMATION info = {};
        VirtualQuery((LPVOID)address, &info, sizeof(info));
        if (info.RegionSize == 0)
        {
            break;
        }
        size_t offsetInRegion = address - (uintptr_t)info.BaseAddress;
        size_t remainingRegionSize = info.RegionSize - offsetInRegion;
        if (info.State == MEM_FREE)
        {
            size_t remaining = endAddress - (uintptr_t)address;
            if (remainingRegionSize  > remaining)
            {
                remainingRegionSize = remaining;
            }
            LPVOID reservedAddr = VirtualAlloc((LPVOID)address, remainingRegionSize, MEM_RESERVE, PAGE_NOACCESS);
            Assert(reservedAddr);
        }
        address = (size_t)address + remainingRegionSize;
        if (address % allocationGranuality != 0)
        {
            // address for reserve is required to be multiple of 64k
            address = address & ~(allocationGranuality - 1);
            address += allocationGranuality;
        }
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
