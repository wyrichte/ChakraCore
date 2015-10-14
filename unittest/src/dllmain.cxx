/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include <StdAfx.h>

int UTRunTests(UTest::UTCOMMANDARGS *pCommandArgs)
{ 
    int errorCode;
    
    try
    {
        AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_OutOfMemory);
        errorCode = UTest::RunAll(pCommandArgs);
    }
    catch (Js::OutOfMemoryException)
    {
        fwprintf(stderr, L"FATAL ERROR: Out of memory running unit tests\n");
        errorCode = -1;
    }
    return errorCode;
}

// This is consumed by AutoSystemInfo. AutoSystemInfo is in Chakra.Common.Core.lib, which is linked
// into multiple DLLs. The hosting DLL provides the implementation of this function.
bool GetDeviceFamilyInfo(
    _Out_opt_ ULONGLONG* /*pullUAPInfo*/,
    _Out_opt_ ULONG* /*pulDeviceFamily*/,
    _Out_opt_ ULONG* /*pulDeviceForm*/)
{
    return false;
}