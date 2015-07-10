// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include <errno.h>

using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace ChakraNativeTaefTests
{
    TestBase::TestBase() :
        verifySettings(VerifyOutputSettings::LogOnlyFailures)   // Reduce verify logs messages to failure only
    {}

    bool TestBase::InitTest(wchar_t const * testDir, bool isProjection)
    {
        this->testDir = testDir;
        this->isProjection = isProjection;

        // Working directory            
        fullTestDir.Format(L"%s\\%s", Globals::GetTestDir(isProjection), (LPCWSTR)testDir);

        if (FAILED(TestData::TryGetValue(L"Index", testIndex)))
        {
            Log::Error(L"Failed to get test index");
            return false;
        }        
        logsDir.Format(L"%s\\%s", Globals::GetTestOutDir(isProjection) , (LPCWSTR)this->testDir);
        CreateDirectory(logsDir, NULL);
        return true;
    }

    // Ported from RL
    bool TestBase::CheckForPass(wchar_t const * filename)
    {
        FILE * fp;
        const int BUFFER_SIZE = 8;
        char buf[BUFFER_SIZE];

        // Check to see if the exe ran at all.
        int retry_count = 0;
        for (;;)
        {
            if ((fp = _wfopen(filename, L"r")) == NULL)
            {
                // Wait and retry a few times if output file is still locked by test
                if (errno == EACCES && ++retry_count < DIFF_OUTPUT_MAX_RETRY)
                {
                    Log::Warning(L"CheckForPass: Open output file failed. Wait and retry.");
                    ::Sleep(DIFF_OUTPUT_WAIT);
                    continue;
                }

                Log::Error(WEX::Common::String().Format(L"Unable to open output file %s", filename));
                return false;
            }

            break; // Opened successfully
        }

        // Parse the output file and verify that all lines must be pass/passed, or empty lines
        bool pass = false;
        while (fgets(buf, 8, fp) != NULL)
        {
            if (!_strcmpi(buf, "pass\n") || !_strcmpi(buf, "passed\n"))
            {
                // Passing strings were found - pass
                pass = true;
            }
            else if (_strcmpi(buf, "\n") != 0)
            {
                // Something else other than a newline was found - this is a failure.
                pass = false;
                break;
            }
        }

        fclose(fp);

        if (!pass)
        {
            Log::Error(WEX::Common::String().Format(L"Pass not found in output file %s", filename));
        }

        return pass;
    }
    
    // If we don't have diff.exe, we will do the diff ourselves
    bool TestBase::FileDiff(wchar_t const * baseline, wchar_t const * diffFile)
    {
        AutoHANDLE baseFileHandle = CreateFile(baseline, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        VERIFY_ARE_NOT_EQUAL(INVALID_HANDLE_VALUE, baseFileHandle);

        AutoHANDLE diffFileHandle;
        int retry_count = 0;
        for (;;)
        {
            diffFileHandle = CreateFile(diffFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            // Wait and retry a few times if output file is still locked by test
            if (diffFileHandle == INVALID_HANDLE_VALUE && GetLastError() == ERROR_SHARING_VIOLATION
                && ++retry_count < DIFF_OUTPUT_MAX_RETRY)
            {
                Log::Warning(L"FileDiff: Open diff file failed. Wait and retry.");
                ::Sleep(DIFF_OUTPUT_WAIT);
                continue;
            }

            VERIFY_ARE_NOT_EQUAL(INVALID_HANDLE_VALUE, diffFileHandle);
            break;
        }

        AutoHANDLE baseFileMapping = CreateFileMapping(baseFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
        VERIFY_IS_NOT_NULL(baseFileMapping);
        AutoHANDLE diffFileMapping = CreateFileMapping(diffFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
        VERIFY_IS_NOT_NULL(diffFileMapping);

        void * baseFileView = MapViewOfFile(baseFileMapping, FILE_MAP_READ, 0, 0, 0);
        VERIFY_IS_NOT_NULL(baseFileMapping);
        void * diffFileView = MapViewOfFile(diffFileMapping, FILE_MAP_READ, 0, 0, 0);
        if (diffFileView == NULL) { ::UnmapViewOfFile(baseFileView); }
        VERIFY_IS_NOT_NULL(diffFileView);

        LARGE_INTEGER baseFileSize;
        VERIFY_WIN32_BOOL_SUCCEEDED(GetFileSizeEx(baseFileHandle, &baseFileSize));
        LARGE_INTEGER diffFileSize;
        VERIFY_WIN32_BOOL_SUCCEEDED(GetFileSizeEx(diffFileHandle, &diffFileSize));

        bool failed = false;
        if (baseFileSize.QuadPart != diffFileSize.QuadPart)
        {
            Log::Error(WEX::Common::String().Format(L"Output different size from baseline"), L"Diff");
            failed = true;
        }
       

        // TODO: Only compare up to size_t
        if (!failed && memcmp(baseFileView, diffFileView, (size_t)baseFileSize.QuadPart) != 0)
        {
            Log::Error(WEX::Common::String().Format(L"Output different from baseline"), L"Diff");
            failed = true;
        }

        if (failed)
        {
            Log::Comment(baseline, L"Baseline");
            Log::Comment(diffFile, L"Output");
            Log::Comment(WEX::Common::String().Format(L"%S", (LPCSTR)diffFileView));
        }

        ::UnmapViewOfFile(baseFileView);
        ::UnmapViewOfFile(diffFileView);

        return !failed;
    }

    bool TestBase::Diff(wchar_t const * baseline, wchar_t const * diffFile)
    {
        if (!Globals::hasDiff)
        {
            return TestBase::FileDiff(baseline, diffFile);            
        }

        int retry_count = 0; // retry a few more times if can't open file
        for (;;)
        {
            AutoHANDLE readPipe;
            AutoHANDLE writePipe;
            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = TRUE;
            VERIFY_WIN32_BOOL_SUCCEEDED(::CreatePipe(&readPipe, &writePipe, &sa, 0));

            STARTUPINFO si = { 0 };
            si.hStdError = writePipe;
            si.hStdOutput = writePipe;
            si.hStdInput = INVALID_HANDLE_VALUE;
            si.dwFlags = STARTF_USESTDHANDLES;

            WEX::Common::String diffCommandLine;
            diffCommandLine.Format(L"\"%s\" \"%s\" \"%s\"", (LPCWSTR)Globals::diffPath, baseline, diffFile);
            if (Globals::verbose)
            {
                Log::Comment(diffCommandLine, L"INFO: Diff Command");
            }

            PROCESS_INFORMATION pi;
            VERIFY_WIN32_BOOL_SUCCEEDED(::CreateProcessW(NULL, (LPWSTR)diffCommandLine.GetBuffer(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi));
            AutoHANDLE processHandle = pi.hProcess;
            ::CloseHandle(pi.hThread);
            writePipe.Close();

            WEX::Common::String diffOutput;
            char diffBuffer[4096];
            DWORD diffRead;
            while (ReadFile(readPipe, diffBuffer, sizeof(diffBuffer) - 1, &diffRead, NULL) && diffRead != 0)
            {
                diffBuffer[diffRead] = 0;
                diffOutput.Append(diffBuffer);
            }

            VERIFY_ARE_EQUAL(WAIT_OBJECT_0, ::WaitForSingleObject(processHandle, INFINITE));

            DWORD exitCode;
            VERIFY_WIN32_BOOL_SUCCEEDED(::GetExitCodeProcess(processHandle, &exitCode));

            // Wait and retry a few times if output file is still locked by test
            if (exitCode != 0 && ++retry_count < DIFF_OUTPUT_MAX_RETRY)
            {
                Log::Warning(L"Diff command failed. Wait and retry.");
                ::Sleep(DIFF_OUTPUT_WAIT);
                continue;
            }

            VERIFY_ARE_EQUAL(L"", diffOutput);
            VERIFY_ARE_EQUAL((DWORD)0, exitCode);
            break;
        }

        return true;
    }

    bool TestBase::RunTest(WEX::Common::String& fullCommandline,
        WEX::Common::String const& outputFileName, bool pass, WEX::Common::String const& baselineFileName)
    {
        if (Globals::skipExec) { return true; }

        //-------------------------
        // Create output file
        //-------------------------
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        AutoHANDLE outputFileHandle = ::CreateFile(outputFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        WEX::Common::String errorMessage;
        errorMessage.AppendFormat(L"Opening file %s for output (gle=%d)", (wchar_t const *)outputFileName, ::GetLastError());
        VERIFY_ARE_NOT_EQUAL(INVALID_HANDLE_VALUE, outputFileHandle, errorMessage);

        //-------------------------
        // Run test
        //-------------------------

        STARTUPINFO si = { 0 };
        si.cb = sizeof(si);
        si.hStdOutput = outputFileHandle;
        si.hStdError = outputFileHandle;
        si.hStdInput = INVALID_HANDLE_VALUE;
        si.dwFlags = STARTF_USESTDHANDLES;
       
        Log::Comment(fullCommandline, L"INFO: Command");

        PROCESS_INFORMATION pi;
        VERIFY_WIN32_BOOL_SUCCEEDED(::CreateProcessW(NULL, (LPWSTR)fullCommandline.GetBuffer(), NULL, NULL, TRUE, 0, NULL, fullTestDir, &si, &pi));
        AutoHANDLE processHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);

        // Wait for process to finish and verify exit code
        VERIFY_ARE_EQUAL(WAIT_OBJECT_0, ::WaitForSingleObject(processHandle, INFINITE));

        outputFileHandle.Close();
        // Log::File(outputFileName);

        DWORD exitCode;
        VERIFY_WIN32_BOOL_SUCCEEDED(::GetExitCodeProcess(processHandle, &exitCode));

        // TODO: We can't veriy the exitCode right now for some are failure tests
        // we need data for test that are testing failure cases
        if (pass || baselineFileName.GetLength() == 0)
        {
            // We can still do it if we don't have a baseline diff
            VERIFY_ARE_EQUAL((DWORD)0, exitCode);
        }
        else
        {
            Log::Comment(WEX::Common::String().Format(L"%d", exitCode), L"INFO: Exit Code");
        }

        //-------------------------
        // Compare output file
        //-------------------------
        if (pass)
        {
            return CheckForPass(outputFileName);
        }
        if (baselineFileName.GetLength() != 0)
        {
            WEX::Common::String baselineFullPath;
            baselineFullPath.Format(L"%s\\%s", (LPCWSTR)fullTestDir, (LPCWSTR)baselineFileName);
            return Diff(baselineFullPath, outputFileName);
        }        

        // No Diff
        return true;
    }

    bool TestBase::Filter(wchar_t const * filterName, wchar_t const * currentValue, wchar_t const * filteredMessage, wchar_t const * variant)
    {
        TestDataArray<WEX::Common::String> excludeFilter;

        if (SUCCEEDED(TestData::TryGetValue(filterName, excludeFilter)))
        {
            for (size_t i = 0; i < excludeFilter.GetSize(); i++)
            {
                if (excludeFilter[i].CompareNoCase(currentValue) == 0)
                {
                    LogFilter(filteredMessage, variant);
                    return true;
                }
            }
        }
        return false;
    }

    // We usually report NotRun to TAEF when a test is filtered. But in our test suite we have cases
    // that one test A produces dynamic profile and multiple other tests B, C, D... read it. B, C, D...
    // all depend on A and have no dependency on each other. TAEF doesn't support this kind of dependency.
    // To work around, we put all of them in the same ExecutionGroup. The only blocking test in this group
    // is the dynamic profile producer test A in Interpreter run. If other tests/runs are NotRun, report
    // as PASS so that they don't block each other.
    void TestBase::LogFilter(wchar_t const * filteredMessage, wchar_t const * variant)
    {
        TestResults::Result result = TestResults::NotRun; // Generally NotRun

#ifndef RLEXE_XML_DATASOURCE
        WEX::Common::String flags;
        if (SUCCEEDED(GetTestFlags(flags)))
        {
            flags.ToLower();

            const wchar_t PREFIX[] = L"-dynamicprofile";
            int i = 0;
            for (;;)
            {
                i = flags.Find(PREFIX, i);
                if (i < 0) break; // No more -dynamicprofile like switches

                i += _countof(PREFIX) - 1; // skip "-dynamicprofile"
                if (wcsncmp((const wchar_t*)flags + i, L"cache:", 6) == 0)
                {
                    if (wcscmp(variant, L"Interpreted") != 0)
                    {
                        // This is a dynamic profile producer. It is not blocking if this is not Interpreter run.
                        result = TestResults::Passed;
                    }
                    break; // Done
                }
                else if (wcsncmp((const wchar_t*)flags + i, L"input:", 6) == 0)
                {
                    // This is a dynamic profile consumer. It doesn't block anything.
                    result = TestResults::Passed;
                    break;
                }
            }
            
        }
#endif

        if (result != TestResults::Passed)
        {
            Log::Result(result, filteredMessage);
        }
    }

    HRESULT TestBase::GetTestFlags(WEX::Common::String& testflags)
    {
        return SUCCEEDED(TestData::TryGetValue(Globals::flagsArchOverride, testflags))
            || SUCCEEDED(TestData::TryGetValue(Globals::flagsOSOverride, testflags))
            || SUCCEEDED(TestData::TryGetValue(L"Flags", testflags)) ? S_OK : E_FAIL;
    }

    // output the repro command if the test failed
    class AutoRepro
    {
    public:
        AutoRepro(WEX::Common::String const& command) : pass(false), command(command)
        {
        
        }
        ~AutoRepro()
        {
            if (pass) { return; }
            Log::Comment(command, L"Repro Command");
        }
        bool pass;        
        WEX::Common::String command;
    };

    void TestBase::RunVariant()
    {
        // Filter exclude variant
        WEX::Common::String variant;
        TestData::TryGetValue(L"Variant", variant);
        if (Filter(L"ExcludeVariant", variant, L"Excluded Variant", variant)
            || Filter(L"ExcludeArch", Globals::targetArch, L"Excluded Arch", variant)
            || Filter(L"ExcludeOS", Globals::targetOS, L"Excluded OS", variant)
            || Filter(L"ExcludeFlavor", Globals::targetFlavor, L"Excluded Flavor", variant))
        {
            return;
        }

        // Exclude Tags will disable a test if user specify /p:<tag>=1 or /p<tag>=true
        // E.g. /p:Snap=true will exclude all test with Snap in the Exclude Tags set.
        TestDataArray<WEX::Common::String> excludeTags;

        if (SUCCEEDED(TestData::TryGetValue(L"ExcludeTags", excludeTags)))
        {
            for (size_t i = 0; i < excludeTags.GetSize(); i++)
            {
                bool value;
                if (SUCCEEDED(RuntimeParameters::TryGetValue(excludeTags[i], value)) && value)
                {
                    LogFilter(WEX::Common::String().Format(L"Exclude Tag '%s'", (LPCWSTR)excludeTags[i]), variant);
                    return;
                }
            }
        }

        // Include Tags are disable by default, unless the parameter tell us to run it
        TestDataArray<WEX::Common::String> includeTags;
        if (SUCCEEDED(TestData::TryGetValue(L"IncludeTags", includeTags)))
        {
            bool found = false;
            for (size_t i = 0; i < includeTags.GetSize(); i++)
            {
                bool value;
                if (SUCCEEDED(RuntimeParameters::TryGetValue(includeTags[i], value)) && value)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                LogFilter(WEX::Common::String().Format(L"No Include Tag"), variant);
                return;
            }
        }

        // Filter extension
        WEX::Common::String filename;
        TestData::TryGetValue(L"FileName", filename);
        const bool isHtmlTest = (filename.Right(4).CompareNoCase(L".htm") == 0 || filename.Right(5).CompareNoCase(L".html") == 0);
        bool isHtmlRun = false;
        isHtmlRun = SUCCEEDED(RuntimeParameters::TryGetValue(L"html", isHtmlRun)) && isHtmlRun;
        if (isHtmlTest != isHtmlRun)
        {
            Log::Result(TestResults::NotRun, isHtmlTest ? L"Excluded HTML" : L"Excluded Non-HTML");
            return;
        }

        // If the test doesn't specify a baseline, "PASS" is expected from the output
        WEX::Common::String baseline;        
        bool verifyPassOnly = 
            FAILED(TestData::TryGetValue(Globals::baselineArchOverride, baseline))
            && FAILED(TestData::TryGetValue(Globals::baselineOSOverride, baseline))
            && FAILED(TestData::TryGetValue(L"Baseline", baseline));

        // Construct the flags (variantFlags testflags filename)           
        WEX::Common::String testflags;
        GetTestFlags(testflags);

        WEX::Common::String variantFlags;
        TestData::TryGetValue(L"VariantFlags", variantFlags);
        variantFlags.Replace(L"{TestFile}", filename);
        variantFlags.Replace(L"{TestIndex}", WEX::Common::String().Format(L"%d", testIndex));
        variantFlags.Replace(L"{TestOutDir}", Globals::GetTestOutDir(isProjection));
        variantFlags.Replace(L"{TestDir}", this->testDir);

        wchar_t const * dumpOnCrashFlag = Globals::targetFlavor.CompareNoCase(L"chk") ? L"-DumpOnCrash" : L"";

        WEX::Common::String allFlags;
        allFlags.Format(L"-bvt %s %s %s %s %s", dumpOnCrashFlag, (LPCWSTR)variantFlags, (LPCWSTR)testflags, (LPCWSTR)Globals::extraFlags, (LPCWSTR)filename);

        // Output file name
        WEX::Common::String outputFilename;
        outputFilename.Format(L"%s\\Output_%s%d_%s.txt", (LPCWSTR)this->logsDir, (LPCWSTR)filename, testIndex, (LPCWSTR)variant);

        // TODO: Try to be explicit on what command we we accept so that we can remove the special casing later
        WEX::Common::String command;
        if (FAILED(TestData::TryGetValue(L"Command", command)))
        {
            command = Globals::jshostPath;
        }
        else if (::_wcsnicmp(command, L"jdtest", 6) == 0
            || ::_wcsnicmp(command, L"testhost", 8) == 0
            || ::_wcsnicmp(command, L"jshost", 6) == 0)
        {
            WEX::Common::String oldCommand = command;
            command.Format(L"%s\\%s", (LPCWSTR)Globals::testBinDir, (LPCWSTR)oldCommand);
        }
        else if (::_wcsnicmp(command, L"ValidateGeneratedByteCode.cmd", _countof(L"ValidateGenerateByteCode.cmd") - 1) == 0
            || ::_wcsnicmp(command, L"ValidateValidPointersMap.cmd", _countof(L"ValidateGenerateByteCode.cmd") - 1) == 0
            || ::_wcsnicmp(command, L"dotest.cmd", _countof(L"dotest.cmd") - 1) == 0)
        {
            WEX::Common::String oldCommand = command;
            command.Format(L"cmd.exe /c %s", (LPCWSTR)oldCommand);
        }
        else
        {
            Log::Error(L"Invalid command");
            return;
        }

        WEX::Common::String fullCommandline;
        fullCommandline.Format(L"%s %s", (LPCWSTR)command, (LPCWSTR)allFlags);

        AutoRepro repro(fullCommandline);
        repro.pass = RunTest(fullCommandline, outputFilename, verifyPassOnly, baseline);
    }

#ifdef RLEXE_XML_DATASOURCE
    HRESULT CreateDataSource(DataSourceFlags flags, wchar_t const * dirname, wchar_t const * dirtags, IDataSource ** ppDataSource)
    {
        if (!ChakraNativeTaefTests::Globals::EnsureInitialized()) { return E_FAIL; }

        const bool isProjection = flags & ProjectionTests;
        WEX::Common::String rlexexml;
        rlexexml.Format(L"%s\\%s\\rlexe.xml", Globals::GetTestDir(isProjection), dirname);

        bool generateXml;
        if (SUCCEEDED(RuntimeParameters::TryGetValue(L"GenerateXml", generateXml)) && generateXml)
        {
            WEX::Common::String outputXml;
            outputXml.Format(isProjection ? L"%s\\%s\\Taef_Projection_%s.xml" : L"%s\\%s\\Taef_Unit_%s.xml", 
                Globals::GetTestDir(isProjection), dirname, dirname);
            // TODO: Error?
            RLExeXmlDataSource::GenerateXmlDataSource(rlexexml, dirtags, flags, outputXml); 
        }
        return RLExeXmlDataSource::Open(rlexexml, dirtags, flags, ppDataSource);
    }   
#endif // #ifdef RLEXE_XML_DATASOURCE
};
