// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include <versionhelpers.h>

BEGIN_MODULE()
    MODULE_PROPERTY(L"Parallel", L"true")
    // Since we just invoke jshost for now, we don't need any isolation
    MODULE_PROPERTY(L"IsolationLevel", L"Module")
END_MODULE()

namespace ChakraNativeTaefTests
{
    template <DWORD size = 1024>
    bool GetEnv(wchar_t * name, WEX::Common::String& outString, bool doError = true)
    {
        wchar_t buffer[size];
        DWORD envSize = ::GetEnvironmentVariable(name, buffer, size);
        if (envSize == 0)
        {
            if (doError)
            {
                Log::Error(WEX::Common::String().Format(L"'%s' environment variable not defined", name));
            }
            return false;

        }
        if (envSize >= size)
        {
            if (doError)
            {
                Log::Error(WEX::Common::String().Format(L"'%s' environment variable too long", name));
            }
            return false;
        }
        outString = buffer;
        return true;
    }

    WEX::Common::String Globals::unitTestDir;
    WEX::Common::String Globals::unitTestOutDir;
    WEX::Common::String Globals::projectionTestDir;
    WEX::Common::String Globals::projectionTestOutDir;
    WEX::Common::String Globals::testBinDir;
    
    WEX::Common::String Globals::jshostPath;
    WEX::Common::String Globals::diffPath;
    WEX::Common::String Globals::baselineArchOverride;
    WEX::Common::String Globals::baselineOSOverride;
    WEX::Common::String Globals::flagsArchOverride;
    WEX::Common::String Globals::flagsOSOverride;

    WEX::Common::String Globals::targetArch;
    WEX::Common::String Globals::targetOS;
    WEX::Common::String Globals::targetFlavor;
    WEX::Common::String Globals::extraFlags;

    bool Globals::hasDiff = false;
    bool Globals::skipExec = false;
    bool Globals::hasInitialized = false;
    bool Globals::verbose = false;

    // Parameters
    //  TestRootDir:    the root directory where to find the unittest   (default: env[JSCRIPT_ROOT], or env[_NTROOT]\inetcore\jscript)
    //  TestBinDir:     directory where we can find the test binaries   (default: env[_NTTREE]\jscript)
    //  Arch:           target architecture                             (default: env[build.arch])
    //  Flavor:         chk or fre                                      (default: env[build.type])
    //  ExtraFlags:     extra flags to add to the command line          (default: <none>)
    //
    //  Verbose:        output verbose messages                         (default: false)
    //  SkipExec:       don't do any execution in the tests             (default: false)
    //  Diff:           Use diff.exe                                    (default: true)
    //  
    //  GenerateXml:    Generate the xml file from custom data source   (default: false)
    //                  Use with /p:SkipExec=1 /inproc
    //
    // Exclude Tags
    //  Snap:           run the snap tests                              (default: true)
    //
    // Include Tags
    //  Fail:           run the fail tests                              (default: false)
    //  JsEtwConsole:   run the JsEtwConsole tests                      (default: false)
    bool Globals::EnsureTestBinDir()
    {
        if (FAILED(RuntimeParameters::TryGetValue(L"TestBinDir", testBinDir)))
        {            
            static WEX::Common::String env_NTTREE;
            if (!GetEnv(L"_NTTREE", env_NTTREE)) { return false; }

            // %_NTTREE\jscript
            testBinDir.Format(L"%s\\jscript", (LPCWSTR)env_NTTREE);
        }
     
        return true;
    }

#ifdef _M_IX86
    static wchar_t const * const HostArch = L"x86";
#elif defined(_M_AMD64)
    static wchar_t const * const HostArch = L"amd64";
#elif defined(_M_ARM)
    static wchar_t const * const HostArch = L"arm";
#elif defined(_M_ARM64)
    static wchar_t const * const HostArch = L"arm64";
#endif
    bool Globals::EnsureTestSourceDir()
    {

        WEX::Common::String NTROOT;
        WEX::Common::String testRootDir;
        if (FAILED(RuntimeParameters::TryGetValue(L"TestRootDir", testRootDir)))
        {
            if (!GetEnv(L"JSCRIPT_ROOT", testRootDir, false))
            {
                if (!GetNTROOT(NTROOT)) { return false; }

                // %_NTROOT%\\inetcore\\jscript
                testRootDir.Format(L"%s\\inetcore\\jscript", (LPCWSTR)NTROOT);
            }
        }

        // %JSCRIPT_ROOT%\unittest
        unitTestDir.Format(L"%s\\unittest", (LPCWSTR)testRootDir);

        // %JSCRIPT_ROOT%\ProjectionTests\Tests
        projectionTestDir.Format(L"%s\\ProjectionTests\\Tests", (LPCWSTR)testRootDir);

        bool diff;
        if ((FAILED(RuntimeParameters::TryGetValue(L"Diff", diff)) || diff)
            && (!NTROOT.IsEmpty() || GetNTROOT(NTROOT)))
        {
            // %_NTROOT%\tools\\diff.exe
            diffPath.Format(L"%s\\tools\\%s\\diff.exe", (LPCWSTR)NTROOT, HostArch);
            if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(diffPath))
            {               
                hasDiff = true;
            }
        }        

        return true;
    }

    bool Globals::GetNTROOT(WEX::Common::String& NTROOT)
    {
        WEX::Common::String env_NTDRIVE;
        WEX::Common::String env_NTROOT;

        if (GetEnv(L"_NTDRIVE", env_NTDRIVE) && GetEnv(L"_NTROOT", env_NTROOT))
        {
            NTROOT = env_NTDRIVE + env_NTROOT;
            return true;
        }

        return false;
    }

    bool Globals::EnsurePath()
    {
        if (!EnsureTestBinDir() || !EnsureTestSourceDir()) { return false; }
        if (!EnsureArch() || !EnsureFlavor()) { return false; }

        WEX::Common::String targetArchFlavor = L"\\" + targetArch + targetFlavor;
       
        // %_NTROOT\unittest\logs\taef
        WEX::Common::String unitTestLogsDir;
        unitTestLogsDir.Format(L"%s\\logs", (LPCWSTR)unitTestDir);
        unitTestOutDir.Format(L"%s\\taef", (LPCWSTR)unitTestLogsDir);

        if (ERROR_PATH_NOT_FOUND == CreateDirectory(unitTestLogsDir, NULL))
        {
            Log::Error(WEX::Common::String().Format(L"Unable to create logs directory %s", (LPCWSTR)unitTestLogsDir));
            return false;
        }
        CreateDirectory(unitTestOutDir, NULL);

        unitTestOutDir += targetArchFlavor; // Append ArchFlavor subdir
        CreateDirectory(unitTestOutDir, NULL);

        // %_NTROOT\projectionTests\Tests\logs\taef
        WEX::Common::String projectionTestLogsDir;
        projectionTestLogsDir.Format(L"%s\\logs", (LPCWSTR)projectionTestDir);
        projectionTestOutDir.Format(L"%s\\taef", (LPCWSTR)projectionTestLogsDir);

        if (ERROR_PATH_NOT_FOUND == CreateDirectory(projectionTestLogsDir, NULL))
        {
            Log::Error(WEX::Common::String().Format(L"Unable to create logs directory %s", (LPCWSTR)projectionTestLogsDir));
            return false;
        }
        CreateDirectory(projectionTestOutDir, NULL);

        projectionTestOutDir += targetArchFlavor; // Append ArchFlavor subdir
        CreateDirectory(projectionTestOutDir, NULL);

        return true;
    }

    bool Globals::EnsureArch()
    {
        if (FAILED(RuntimeParameters::TryGetValue(L"Arch", targetArch)))
        {
            if (!GetEnv(L"build.arch", targetArch)) { return false; }
        }
        else
        {
            if (::_wcsicmp(targetArch, L"x86") != 0
                && ::_wcsicmp(targetArch, L"amd64") != 0
                && ::_wcsicmp(targetArch, L"arm") != 0
                && ::_wcsicmp(targetArch, L"arm64") != 0)
            {
                Log::Error(WEX::Common::String().Format(L"Invalid Arch '%s'", targetArch));
                return false;
            }
        }
        return true;
    }

    bool Globals::EnsureFlavor()
    {
        if (FAILED(RuntimeParameters::TryGetValue(L"Flavor", targetFlavor)))
        {
            if (!GetEnv(L"build.type", targetFlavor)) { return false; }
        }
        else
        {
            if (::_wcsicmp(targetFlavor, L"chk") != 0
                && ::_wcsicmp(targetFlavor, L"fre") != 0)
            {
                Log::Error(WEX::Common::String().Format(L"Invalid flavor '%s'", targetFlavor));
                return false;
            }
        }
        return true;
    }

    bool Globals::EnsureParameters()
    {
        // Loads the parameters to the test
        if (FAILED(RuntimeParameters::TryGetValue(L"ExtraFlags", extraFlags)))
        {
            extraFlags = L"";
        }
       
        if (FAILED(RuntimeParameters::TryGetValue(L"SkipExec", skipExec)))
        {
            skipExec = false;
        }

        if (FAILED(RuntimeParameters::TryGetValue(L"Verbose", verbose)))
        {
            verbose = false;
        }
        return true;
    }
    bool Globals::EnsureInitialized()
    {
        if (hasInitialized) { return true; }

        if (!EnsurePath() || !EnsureArch() || !EnsureFlavor() || !EnsureParameters()) { return false; }
        
        // %_NTTREE%\jscript\jshost.exe
        jshostPath.Format(L"%s\\jshost.exe", (LPCWSTR)testBinDir);

        // TODO: Threshold also marked as winBlue
        targetOS = IsWindows8Point1OrGreater() ? L"winBlue" : IsWindows8OrGreater() ? L"win8" : L"win7";

        // TODO: This set it for the whole process, but it only need it for the JD test
        ::SetEnvironmentVariable(L"_NT_DEBUGGER_EXTENSION_PATH", testBinDir);

        baselineArchOverride.Format(L"Baseline-%s", (LPCWSTR)targetArch);
        flagsArchOverride.Format(L"Flags-%s", (LPCWSTR)targetArch);
        baselineOSOverride.Format(L"Baseline-%s", (LPCWSTR)targetOS);
        flagsOSOverride.Format(L"Flags-%s", (LPCWSTR)targetOS);

        hasInitialized = true;   

        if (verbose)
        {
            Log::Comment(testBinDir, L"INFO: Test Binary Directory:");
            Log::Comment(unitTestDir, L"INFO: Unit Test Directory:");
            Log::Comment(projectionTestDir, L"INFO: Projection Test Directory:");
        }
        return true;
    }

    MODULE_SETUP(ModuleSetup)
    {
        return Globals::EnsureInitialized();        
    }
};