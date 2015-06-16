//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include "StdAfx.h"
#include "StackOverflowException.h"
#include "Dbghelp.h"

extern "C"{
    BOOLEAN IsMessageBoxWPresent();
}

namespace Js {
#ifdef GENERATE_DUMP
    StackBackTrace * Throw::stackBackTrace = nullptr;
#endif 
    void Throw::FatalInternalError()
    {
        int scenario = 2;
        ReportFatalException(NULL, E_FAIL, Fatal_Internal_Error, scenario);
    }

    void Throw::FatalProjectionError()
    {
        RaiseException((DWORD)DBG_TERMINATE_PROCESS, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

    void Throw::InternalError()
    {
        AssertMsg(false, "Internal error!!");
        throw InternalErrorException();
    }

    void Throw::OutOfMemory()
    {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (CONFIG_FLAG(PrintSystemException))
        {
            Output::Print(L"SystemException: OutOfMemory\n");
            Output::Flush();
        }
#endif
        if (JsUtil::ExternalApi::RaiseOutOfMemoryIfScriptActive())
        {
            AssertMsg(false, "We shouldn't be here");
        }
        throw OutOfMemoryException();
    }
    void Throw::CheckAndThrowOutOfMemory(BOOLEAN status)
    {
        if (!status)
        {
            OutOfMemory();
        }    
    }
    void Throw::StackOverflow(ScriptContext *scriptContext, PVOID returnAddress)
    {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (CONFIG_FLAG(PrintSystemException))
        {
            Output::Print(L"SystemException: StackOverflow\n");
            Output::Flush();
        }
#endif
        if (JsUtil::ExternalApi::RaiseStackOverflowIfScriptActive(scriptContext, returnAddress))
        {
            AssertMsg(false, "We shouldn't be here");
        }
        throw StackOverflowException();
    }

    void Throw::NotImplemented()
    {
        AssertMsg(false, "This functionality is not yet implemented");

        throw NotImplementedException();
    }

    // Returns true when the process is either TE.exe or TE.processhost.exe
    bool Throw::IsTEProcess()
    {
        wchar_t fileName[_MAX_PATH];
        wchar_t moduleName[_MAX_PATH];
        GetModuleFileName(0, moduleName, _MAX_PATH);
        errno_t err = _wsplitpath_s(moduleName, nullptr, 0, nullptr, 0, fileName, _MAX_PATH, nullptr, 0);

        return err == 0 && _wcsnicmp(fileName, L"TE", 2) == 0;
    }

    void Throw::GenerateDumpAndTerminateProcess(PEXCEPTION_POINTERS exceptInfo)
    {
        if (!Throw::IsTEProcess() 
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            && !Js::Configuration::Global.flags.IsEnabled(Js::DumpOnCrashFlag)
#endif
            )
        {
            return;
        }

#ifdef GENERATE_DUMP
        Js::Throw::GenerateDump(exceptInfo, Js::Configuration::Global.flags.DumpOnCrash);
#endif

        // For now let's terminate the process.
        TerminateProcess(GetCurrentProcess(), (UINT)DBG_TERMINATE_PROCESS);
    }

#ifdef GENERATE_DUMP
    CriticalSection Throw::csGenereateDump;
    void Throw::GenerateDump(LPCWSTR filePath, bool terminate)
    {
        __try
        {
            if (terminate)
            {
                RaiseException((DWORD)DBG_TERMINATE_PROCESS, EXCEPTION_NONCONTINUABLE, 0, NULL);
            }
            else
            {
                RaiseException(0, 0, 0, NULL);
            }
        }
        __except(Throw::GenerateDump(GetExceptionInformation(), filePath, 
            terminate? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER))
        {
            // we don't do anything interesting in this handler
        }
    }

    int Throw::GenerateDump(PEXCEPTION_POINTERS exceptInfo, LPCWSTR filePath, int ret)
    {
        WCHAR tempFilePath[MAX_PATH];
        WCHAR tempFileName[MAX_PATH];
        HANDLE hTempFile;
        DWORD retVal;
        
        if (filePath == NULL)
        {
            retVal = GetTempPath(MAX_PATH, tempFilePath);

            if (retVal > MAX_PATH || (retVal == 0))
            {
                return ret;
            }
            filePath = tempFilePath;
        }

        if (BinaryFeatureControl::LanguageService())
        {
            StringCchPrintf(tempFileName, _countof(tempFileName), L"%s\\JSLS_%d_%d.dmp", filePath, GetCurrentProcessId(), GetCurrentThreadId());
        }
        else
        {
            StringCchPrintf(tempFileName, _countof(tempFileName), L"%s\\JC_%d_%d.dmp", filePath, GetCurrentProcessId(), GetCurrentThreadId());
        }
        hTempFile = CreateFile(tempFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
            FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTempFile == INVALID_HANDLE_VALUE)
        {
            return GetLastError();
        }

        MINIDUMP_EXCEPTION_INFORMATION dumpExceptInfo;
        dumpExceptInfo.ThreadId = GetCurrentThreadId();
        dumpExceptInfo.ExceptionPointers = exceptInfo;
        dumpExceptInfo.ClientPointers = FALSE;

        {
            MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(MiniDumpWithDataSegs | MiniDumpWithPrivateReadWriteMemory);

            // Generating full dump for the TE process (reason : it contains both managed and native memory)
            if (CONFIG_FLAG(FullMemoryDump) || (BinaryFeatureControl::LanguageService() && Throw::IsTEProcess()))
            {
                dumpType = static_cast<MINIDUMP_TYPE>(dumpType | MiniDumpWithFullMemory);
            }

            AutoCriticalSection autocs(&csGenereateDump);
            if (!MiniDumpWriteDump(GetCurrentProcess(),
                GetCurrentProcessId(),
                hTempFile,
                dumpType,
                &dumpExceptInfo,
                NULL,
                NULL))
            {
                Output::Print(L"Unable to write minidump (0x%08X)\n", GetLastError());
                Output::Flush();
            }
        }
        FlushFileBuffers(hTempFile);
        CloseHandle(hTempFile);
        return ret;
    }
#endif // GENERATE_DUMP

#if DBG
    // After assert the program should terminate. Sometime we saw the program continue somehow
    // log the existance of assert for debugging.
    void Throw::LogAssert()
    {
        IsInAssert = true;
        // We don't really need to worry about leak: this should be the last thing happen in the process.
        stackBackTrace = StackBackTrace::Capture(&NoCheckHeapAllocator::Instance, Throw::StackToSkip, Throw::StackTraceDepth);
    }

    bool Throw::ReportAssert(LPSTR fileName, uint lineNumber, LPSTR error, LPSTR message)
    {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::AssertBreakFlag))
        {
            DebugBreak();
            return false;
        }
        if (Js::Configuration::Global.flags.IsEnabled(Js::AssertIgnoreFlag))
        {
            return true;
        }
#endif
        if (AssertsToConsole)
        {
            fprintf(stderr, "ASSERTION %d: (%s, line %d) %s\n Failure: %s\n", GetCurrentProcessId(), fileName, lineNumber, message, error);
            fflush(stderr);
#ifdef GENERATE_DUMP
            // force dump if we have assert in jc.exe. check build only.
            if (!Js::Configuration::Global.flags.IsEnabled(Js::DumpOnCrashFlag))
            {
                return false;
            }
            Throw::GenerateDump(Js::Configuration::Global.flags.DumpOnCrash, true);            
#else
            return false;
#endif
        }
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        // Actually following code is dead in jshost. AssertsToConsole is always true in jshost, it can be false in JC.exe
        // Then if DumpOncrashFlag is not specified it directly returns, 
        // otherwise if will raise a non-continuable exception, generate the dump and terminate the process.
        // the popup message box might be useful when testing in IE
        if (Js::Configuration::Global.flags.AssertPopUp && IsMessageBoxWPresent())
        {
            wchar_t buff[1024];

            swprintf_s(buff, _countof(buff), L"%S (%d)\n%S\n%S", fileName, lineNumber, message, error);
            buff[_countof(buff)-1] = 0;

            int ret = MessageBox(null, buff, L"CHAKRA ASSERT", MB_ABORTRETRYIGNORE);

            switch (ret)
            {
            case IDIGNORE:
                return true;
            case IDABORT:
                Throw::FatalInternalError();
            default:
                return false;
            }
        }
#endif
        return false;
    }

#endif

} // namespace Js
