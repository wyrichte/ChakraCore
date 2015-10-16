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

void ChakraInitPerImageSystemPolicy(AutoSystemInfo * autoSystemInfo)
{
    /* Do Nothing */
}
