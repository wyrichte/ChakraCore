/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "StdAfx.h"
#include <VersionHelpers.h>

using namespace std;

#define FAIL(x) do { VERIFY_IS_TRUE(0==1 && x); return; } while(0)

#if _M_IX86
#define TEST_BUILD_TYPE L"\\i386"
#elif _M_X64
#define TEST_BUILD_TYPE L"\\amd64"
#elif _M_ARM
#define TEST_BUILD_TYPE L"\\arm"
#elif _M_ARM64
#define TEST_BUILD_TYPE L"\\arm64"
#else
#error Platform not supported.
#endif

class runalltests
{
    TEST_CLASS(runalltests)

    TEST_METHOD(test1)
    {
        WCHAR buf[8192];
        DWORD pos;
        wstring rlpath;
        wstring rlfullpath;
        wstring jcpath;
        wstring jcfullpath;
        wstring cwd;

        // build the path to rl.exe
        pos = GetEnvironmentVariable(L"OBJECT_ROOT", buf, _countof(buf));
        if(pos == 0 || pos > _countof(buf))
            FAIL("failed to retrieve %OBJECT_ROOT%");

        rlpath = buf;
        pos = GetEnvironmentVariable(L"_BUILDTYPE", buf, _countof(buf));
        if(pos == 0 || pos > _countof(buf))
            FAIL("failed to retrieve %_BUILDTYPE%");

        rlpath += L"\\inetcore\\jscript\\unittest\\ut_rl\\obj" + wstring(buf) + TEST_BUILD_TYPE;
        rlfullpath = rlpath + L"\\rl.exe";

        // quick check if rl exists
        if(GetFileAttributes(rlfullpath.c_str()) == INVALID_FILE_ATTRIBUTES)
            FAIL("rl.exe doesn't exist: bcz inetcore\\jscript\\unittest");

        // build the path to jshost.exe
        pos = GetEnvironmentVariable(L"_NTTREE", buf, _countof(buf));
        if(pos == 0 || pos > _countof(buf))
            FAIL("failed to retrieve %_NTTREE%");

        jcpath = buf;
        jcpath += L"\\jscript";
        jcfullpath = jcpath + L"\\jshost.exe";

        // quick check if jc exists
        if(GetFileAttributes(jcfullpath.c_str()) == INVALID_FILE_ATTRIBUTES)
            FAIL("jc.exe doesn't exist: bcz inetcore\\jscript");


        // add rl.exe and jc.exe to path
        pos = GetEnvironmentVariable(L"PATH", buf, _countof(buf));
        if(pos == 0 || pos > _countof(buf))
            FAIL("failed trying to retrieve %PATH%");

        wstring newpath = wstring(buf) + L";" + jcpath + L";" + rlpath;
        if(SetEnvironmentVariable(L"PATH", newpath.c_str()) == 0)
            FAIL("failed to set new PATH");


        // set up the proper working directory
        pos = GetEnvironmentVariable(L"_NTROOT", buf, _countof(buf));
        if(pos == 0 || pos > _countof(buf))
            FAIL("failed trying to retrieve %_NTROOT%");

        cwd = wstring(buf) + L"\\inetcore\\jscript\\tools";

        // determine target os
        wstring os;
        {
            if (IsWindows8Point1OrGreater())
            {
                os = L" -winBlue";
            }
            else if (IsWindows8OrGreater())
            {
                os = L" -win8";
            }
            else if (IsWindows7OrGreater())
            {
                os = L" -win7";
            }
        }

        // Assume we are being called via SNAP, let RunAllTests.cmd know it should do something special.
        wstring snapTag = L" -snap";

        // spawn the child process
        STARTUPINFO si;
        PROCESS_INFORMATION pinfo;
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);

        WCHAR cmd[MAX_PATH];
        StringCchPrintf(cmd, _countof(cmd), L"cmd.exe /c RunAllTests.cmd%s%s", os.c_str(), snapTag.c_str());

        if(CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, cwd.c_str(), &si, &pinfo) == 0)
            FAIL("failed to start cmd.exe");

        if(WaitForSingleObject(pinfo.hProcess, INFINITE) != WAIT_OBJECT_0)
        {
            CloseHandle(pinfo.hProcess);
            CloseHandle(pinfo.hThread);
            FAIL("failed waiting for cmd.exe to complete");
        }

        DWORD ret;
        if(GetExitCodeProcess(pinfo.hProcess, &ret) == 0)
        {
            CloseHandle(pinfo.hProcess);
            CloseHandle(pinfo.hThread);
            FAIL("failed to retrieve exit code");
        }

        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);

        if(ret != 0)
            FAIL("runalltests.cmd failed: check log files at unittest\\logs");

        // pass

    };
};

