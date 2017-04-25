//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StaticLibPch.h"

namespace JsStaticAPI
{
    LONG Error::FatalExceptionFilter(__in LPEXCEPTION_POINTERS lpep)
    {
        LONG rc = UnhandledExceptionFilter(lpep);

        // re == EXCEPTION_EXECUTE_HANDLER means there is no debugger attached, let's terminate
        // the process. Otherwise give control to the debugger.
        // Note: in case when postmortem debugger is registered but no actual debugger attached,
        //       rc will be 0 (and EXCEPTION_EXECUTE_HANDLER is 1), so it acts as if there is debugger attached.
        if (rc == EXCEPTION_EXECUTE_HANDLER)
        {
            TerminateProcess(GetCurrentProcess(), (UINT)DBG_TERMINATE_PROCESS);
        }
        else
        {
            // However, if debugger was not attached for some reason, terminate the process.
            if (!IsDebuggerPresent())
            {
                TerminateProcess(GetCurrentProcess(), (UINT)DBG_TERMINATE_PROCESS);
            }
            DebugBreak();
        }

        return EXCEPTION_CONTINUE_SEARCH;
    }

    void Error::ReportFatalException(__in HRESULT exceptionCode, __in ErrorReason reasonCode)
    {
        if (IsDebuggerPresent())
        {
            DebugBreak();
        }

        __try
        {
            ULONG_PTR ExceptionInformation[2];
            ExceptionInformation[0] = (ULONG_PTR)reasonCode;
            ExceptionInformation[1] = NULL;
            RaiseException(exceptionCode, EXCEPTION_NONCONTINUABLE, 2, (ULONG_PTR*)ExceptionInformation);
        }
        __except (JsStaticAPI::Error::FatalExceptionFilter(GetExceptionInformation()))
        {
        }
    }

}