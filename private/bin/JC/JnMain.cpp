//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include "TestLargeAddress.h"
#include "hostsysinfo.h"

//#define REGEX_STATS
#ifdef REGEX_STATS
extern int _cacheHit;
extern char16* _regexSources[256];
extern int _regexIdCounts[256];
extern double _regexIdTimes[256];
extern int _regexIdStrLength[256];
extern int _bucketCounts[1001];
extern int _keyCount;
struct RegexTiming {
    int id;
    double perIter;
    double perChar;
    double total;
};
extern void PrintRegexKey(int);
extern int _regexHashKeyMisses[40000];

int _cdecl SortByPerIter(void*c,const void*a,const void*b) {
    RegexTiming* rta=(RegexTiming*)a;
    RegexTiming* rtb=(RegexTiming*)b;
    if (rtb->perIter>rta->perIter) {
        return 1;
    }
    else if (rtb->perIter<rta->perIter) {
        return -1;
    }
    else {
        return 0;
    }
}

int _cdecl SortByPerChar(void*c,const void*a,const void*b) {
    RegexTiming* rta=(RegexTiming*)a;
    RegexTiming* rtb=(RegexTiming*)b;
    if (rtb->perChar>rta->perChar) {
        return 1;
    }
    else if (rtb->perChar<rta->perChar) {
        return -1;
    }
    else {
        return 0;
    }
}

int _cdecl SortByTotal(void*c,const void*a,const void*b) {
    RegexTiming* rta=(RegexTiming*)a;
    RegexTiming* rtb=(RegexTiming*)b;
    if (rtb->total>rta->total) {
        return 1;
    }
    else if (rtb->total<rta->total) {
        return -1;
    }
    else {
        return 0;
    }
}

RegexTiming _regexTimings[256];
#endif

using namespace Js;

// TODO: Create a header file for the parser, similar to "Jn.h"
void ParserEntry(Js::Configuration*config);
int _cdecl wmain2(int argc, __in_ecount(argc) LPWSTR argv[]);

int JcExceptionFilter(int exceptionCode, PEXCEPTION_POINTERS exceptionInfo)
{
    Output::Flush();

#ifdef GENERATE_DUMP
    // We already reported assert at the assert point, don't do it here again
    if (exceptionInfo->ExceptionRecord->ExceptionCode != STATUS_ASSERTION_FAILURE)
    {
        if (Js::Configuration::Global.flags.IsEnabled(Js::DumpOnCrashFlag))
        {
            Js::Throw::GenerateDump(exceptionInfo, Js::Configuration::Global.flags.DumpOnCrash);
        }
    }
#endif

    fwprintf(stderr, _u("FATAL ERROR: jnconsole.exe failed due to exception code %x\n"), exceptionCode);

#ifdef SECURITY_TESTING
    if (exceptionCode == EXCEPTION_BREAKPOINT || (Js::Configuration::Global.flags.CrashOnException && exceptionCode != 0xE06D7363))
#else
    if (exceptionCode == EXCEPTION_BREAKPOINT)
#endif
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    return EXCEPTION_EXECUTE_HANDLER;
}


///----------------------------------------------------------------------------
///
/// wmain
///
/// wmain() defines the entry-point for the program.
///
///----------------------------------------------------------------------------

extern HANDLE g_hInstance;

int _cdecl
wmain1(int argc, __in_ecount(argc) LPWSTR argv[])
{
    // Initialize the module handle for resource loading
    g_hInstance = GetModuleHandle(NULL);
    int ret = 1;


#ifdef DBG
    AssertsToConsole = true;
#endif


    if (CoInitializeEx(NULL, HostSystemInfo::SupportsOnlyMultiThreadedCOM() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED) != S_OK)
    {
        wprintf(_u("FATAL ERROR: failed call to CoInitializeEx()\n"));
        exit(1);
    }


    // Wrap entrypoint in __try/__finally to catch and log fatal errors.
    // TODO: Watson support?
    __try
    {
        // Call the real entrypoint.
        ret = wmain2(argc, argv);
#ifdef FAULT_INJECTION
        if(Js::Configuration::Global.flags.FaultInjection == Js::FaultInjection::Global.CountOnly)
        {
            wprintf(_u("FaultInjection - Total Allocation Count:%d\n"),Js::FaultInjection::Global.countOfInjectionPoints);
        }
#endif
    }
    __except(JcExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
    {
        // Exception happened, so we probably didn't clean up properly, 
        // Don't exit normally, just terminate
        TerminateProcess(::GetCurrentProcess(), GetExceptionCode());
    }
    Output::Flush();

    return ret;
}

void PrintUsage()
{
    wprintf(_u("\n\nusage :jc.exe [flaglist] filename\n"));
    HostConfigFlags::PrintUsageString();
    Js::ConfigFlagsTable::PrintUsageString();
}

int _cdecl
wmain(int argc, __in_ecount(argc) LPWSTR argv[])
{
    HostConfigFlags::pfnPrintUsage = PrintUsage;
    if (UseLargeAddresses(argc, argv))
    {
        return TestLargeAddress(argc, argv, wmain1);
    }

    return wmain1(argc, argv);
}

#include <psapi.h>

void DisplayMemStats(char16 const * message)
{
    PROCESS_MEMORY_COUNTERS_EX memCounters;

    if (CONFIG_ISENABLED(Js::DisplayMemStatsFlag))
    {
        memCounters.cb=sizeof(memCounters);
        GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS*)&memCounters,memCounters.cb);
        Output::Print(_u("%s:\n"), message);
        Output::Print(_u("  Peak working set %-7.2fM\n"),memCounters.PeakWorkingSetSize/1000000.0);
        Output::Print(_u("  Working set      %-7.2fM\n"),memCounters.WorkingSetSize/1000000.0);
        Output::Print(_u("  Private memory   %-7.2fM\n"),memCounters.PrivateUsage/1000000.0);
    }
}

Js::JavascriptFunction* LoadFile(Js::ScriptContext * scriptContext, String& filename)
{
    FILE * file;
    LPCOLESTR contents = NULL;
    LPUTF8 contentsUtf8 = NULL;
    BOOL   fOpenFailed = FALSE;
    HRESULT hr = S_OK;

    //
    // Open the file as a binary file to prevent CRT from handling encoding, line-break conversions,
    // etc.
    //

    if(_wfopen_s(&file, filename, _u("rb")))
    {
        fOpenFailed = TRUE;
    }

    if (fOpenFailed)
    {
        char16 wszBuff[512];
        fwprintf(stderr, _u("_wfopen of %s failed"), (LPCWSTR)filename);
        wszBuff[0] = 0;
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            GetLastError(),
                            0,
                            wszBuff,
                            _countof(wszBuff),
                            NULL))
        {
            fwprintf(stderr, _u(": %s"), wszBuff);
        }
        fwprintf(stderr, _u("\n"));
        return null;
    }


    //
    // Determine the file length, in bytes.
    //

    fseek(file, 0, SEEK_END);
    uint lengthBytes = ftell(file);
    fseek(file, 0, SEEK_SET);
    LPCOLESTR contentsRaw = (LPCOLESTR) calloc(lengthBytes + 2, 1);

    if (NULL == contentsRaw)
        fwprintf(stderr, _u("out of memory"));


    //
    // Read the entire content as a binary block.
    //

    fread((void*) contentsRaw, sizeof(char), lengthBytes, file);
    fclose(file);


    //
    // Read encoding, handling any conversion to Unicode.
    //
    // Warning: The UNICODE buffer for parsing is supposed to be provided by the host.
    // this is temporary code to read from Unicode and ANSI files.
    // It is not a complete read of the encoding. Some encodings like UTF7, UTF1, EBCDIC, SCSU, BOCU could be
    // wrongly classified as ANSI
    //

    byte * pRawBytes = (byte*)contentsRaw;
    if( (0xEF == *pRawBytes && 0xBB == *(pRawBytes+1) && 0xBF == *(pRawBytes+2)) )
        contentsUtf8 = LPUTF8(contentsRaw);
    else if ( 0xFFFE == *contentsRaw ||
        0x0000 == *contentsRaw && 0xFEFF == *(contentsRaw+1) )
    {
        // UTF-16BE or UTF-32BE, both are unsupported
        fwprintf(stderr, _u("unsupported file encoding"));
        return null;
    }
    else if (0xFEFF == *contentsRaw)
    {
        // unicode LE
        contents = contentsRaw;
    }
    else
    {
        // Assume UTF8
        contentsUtf8 = LPUTF8(contentsRaw);

        // note: some other encodings might fall wrongly in this category.
        // This will result in 0xFFFD replacement characters when
        // the format doesn't match UTF-8.
    }
    CompileScriptException se;

#ifdef MUTATORS
    if(Js::Configuration::Global.flags.IsEnabled(Js::MutatorsFlag))
    {
        Js::SourceMutator* sourceMutator = scriptContext->GetSourceMutator();
        if (contentsUtf8)
        {
            // The mutator expects UTF-16LE but we have UTF8, convert the file, mutate it and then assume we read UTF-16.
            // This potentially allocates too much but which could be fixed with a realloc() since it is at most
            // lengthBytes / 2 and usually only a 10-20 bytes, it is being ignored. This would only occur if the script
            // contains non ASCII characters.
            LPOLESTR contentsUtf16 = LPOLESTR(calloc(lengthBytes + 1, sizeof(OLECHAR)));
            utf8::DecodeUnitsIntoAndNullTerminate(contentsUtf16, *((LPCUTF8*)&contentsUtf8), contentsUtf8 + lengthBytes, utf8::doAllowThreeByteSurrogates);
            contents = contentsUtf16;
            contentsUtf8 = NULL;
            free((void*)contentsRaw);
        }
        
        if(Js::Configuration::Global.flags.Mutators == nullptr)
        {
            contents = sourceMutator->ModifySource(contents);
        }
        else 
        {
            contents = sourceMutator->ModifySourceWithMutators(contents, Js::Configuration::Global.flags.Mutators);
        }
    }
#endif

    char16 * fullpath = _wfullpath(NULL, filename, 0);
    if (fullpath == null)
    {
        fwprintf(stderr, _u("Out of memory"));
        return null;
    }

    // canonicalize that path name to lower case for the profile storage
    size_t len = wcslen(fullpath);
    for (size_t i = 0; i < len; i ++)
    {
        fullpath[i] = towlower(fullpath[i]);
    }

    SourceContextInfo * sourceContextInfo = NULL;
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        static uint sourceId = 0;
        BSTR url = SysAllocString(fullpath);
        scriptContext->SetUrl(url);
        sourceContextInfo = scriptContext->CreateSourceContextInfo(sourceId++, fullpath, len, NULL);
        SysFreeString(url);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    if(FAILED(hr))
    {
        fwprintf(stderr, _u("Out of memory"));
        return null;
    }

    free(fullpath);
    SRCINFO si = {
        /* sourceContextInfo   */ sourceContextInfo,
        /* dlnHost             */ 0,
        /* ulColumnHost        */ 0,
        /* lnMinHost           */ 0,
        /* ichMinHost          */ 0,
        /* ichLimHost          */ 0,
        /* mod                 */ kmodGlobal,
        /* grfsi               */ 0
        };
    Js::Utf8SourceInfo* sourceInfo;
    JavascriptFunction* rootFunction = contents ? scriptContext->LoadScript(contents, &si, &se, false, false, false, &sourceInfo, Js::Constants::UnknownScriptCode)
        : scriptContext->LoadScript(contentsUtf8, lengthBytes, &si, &se, false, false, false, &sourceInfo, Js::Constants::UnknownScriptCode);

    if (rootFunction == null)
    {
        LONG lCharacterPosition = 0;
        if (se.ichMin > se.ichMinLine)
            lCharacterPosition = se.ichMin - se.ichMinLine;
        fwprintf(stderr, _u("%s(%d, %d) %s: %s\n"), (LPCWSTR)filename, se.line,
            lCharacterPosition, se.ei.bstrSource, se.ei.bstrDescription);
        // Reclaim strings allocated while processing any errors
        se.Free();
    }
#ifdef MUTATORS
    if(!Js::Configuration::Global.flags.IsEnabled(Js::MutatorsFlag))
    {
#endif
        free((void*) contents);
#ifdef MUTATORS
    }
#endif
    return rootFunction;
}

int DoOneIteration(ThreadContext * threadContext, String& fileName)
{
#ifdef REGEX_STATS
    for (int i=0;i<256;i++) {
        _regexIdCounts[i]=0;
        _regexIdTimes[i]=0.0;
        _regexIdStrLength[i]=0;
    }
    int j;
    for (j=0;j<1001;j++) {
        _bucketCounts[j]=0;
    }
    for (j=0;j<40000;j++) {
        _regexHashKeyMisses[j]=0;
    }

#endif
    int error = 0;
    Js::ScriptContext* scriptContext = null;
    threadContext->ResetFunctionCount();
    try
    {
        AUTO_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
        scriptContext = ScriptContext::New(threadContext);
        DisplayMemStats(_u("Script Context Created"));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.ForceDiagnosticsMode)
        {
            if (!Js::Configuration::Global.EnableJitInDebugMode())
            {
                scriptContext->ForceNoNative();
            }

            scriptContext->SetInDebugMode();
        }
#endif

        scriptContext->Initialize();
        WScript::Initialize(scriptContext);

        DisplayMemStats(_u("Script Context Initialized"));
    }
    catch (Js::OutOfMemoryException)
    {
        if (scriptContext != null)
        {
            HeapDelete(scriptContext);
        }
        fwprintf(stderr, _u("FATAL ERROR: Out of memory intializing script context\n"));
        return 1;
    }
    catch (Js::StackOverflowException)
    {
        if (scriptContext != null)
        {
            HeapDelete(scriptContext);
        }
        fwprintf(stderr, _u("FATAL ERROR: Stack overflow intializing script context\n"));
        return 1;
    }

    Js::Tick tkStart;
    Js::Tick tkParse;
    Js::TickDelta tdElapsedParse;

    if(!HostConfigFlags::flags.BVT)
    {
        tkStart = Js::Tick::Now();
    }

    Js::JavascriptFunction* rootFunction = LoadFile(scriptContext, fileName);

    if(!HostConfigFlags::flags.BVT)
    {
        tkParse = Js::Tick::Now();
    }

    if (rootFunction == NULL)
    {
        error++;
    }
    else if (rootFunction && !PHASE_OFF1(Js::RunPhase))
    {
        try
        {
            AUTO_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_JavascriptException));
            rootFunction->CallRootFunction(Js::Arguments(0, null), scriptContext);
        }
        catch (JavascriptExceptionObject * exceptionObject)
        {
            LCID lcid = GetUserLocale();
            HRESULT hr = JSERR_UncaughtException;
            char16 const * messageSz = null;
            Js::Var errorObject = exceptionObject->GetThrownObject(null);
            if (errorObject != NULL && Js::JavascriptError::Is(errorObject))
            {
                hr = Js::JavascriptError::GetRuntimeErrorWithScriptEnter(Js::RecyclableObject::FromVar(errorObject), &messageSz);
            }

            BSTR bstrError = null;
            if (messageSz == null)
            {
                if (FACILITY_CONTROL == HRESULT_FACILITY(hr))
                {
                    bstrError = BstrGetResourceString(HRESULT_CODE(hr), lcid);
                }
            }
            else
            {
                // Ignore out of memory here or string too big
                size_t len = wcslen(messageSz);
                if (len <= UINT_MAX)
                {
                    bstrError = SysAllocStringLen(messageSz, (uint)len);
                }
            }

            BSTR bstrSource = BstrGetResourceString(IDS_RUNTIME_ERROR_SOURCE, lcid);

            Js::FunctionBody * funcBody = exceptionObject->GetFunctionBody();
            /* TODO-ERROR: Shouldn't we always have a function body to report? */
            if (funcBody)
            {
                uint32 offset = exceptionObject->GetByteCodeOffset();
                ULONG line = 0;
                LONG col = 0;
                if (!funcBody->GetLineCharOffset(offset, &line, &col))
                {
                    line = 0;
                    col = 0;
                }

                char16 filename[_MAX_FNAME];
                char16 ext[_MAX_EXT];
                _wsplitpath_s(Configuration::Global.flags.Filename, NULL, 0, NULL, 0, filename, _MAX_FNAME, ext, _MAX_EXT);
                fwprintf(stderr, _u("%s%s(%d, %d) %s: %s\n"), filename, ext,
                    line + 1, col + 1, bstrSource, bstrError);
            }
            else
            {
                fwprintf(stderr, _u("%s %s: %s\n"), (LPCWSTR)Configuration::Global.flags.Filename,
                    bstrSource, bstrError);
            }
            SysFreeString(bstrSource);
            if (bstrError != null)
            {
                SysFreeString(bstrError);
            }
            error++;
        }
        catch (Js::InternalErrorException)
        {
            Assert(false);
            fwprintf(stderr, _u("FATAL ERROR: Internal error exception in script execution\n"));
            error++;
        }
        catch (Js::OutOfMemoryException)
        {
            fwprintf(stderr, _u("FATAL ERROR: out of memory error in script execution\n"));
            error++;
        }
        catch (Js::NotImplementedException)
        {
            Assert(false);
            fwprintf(stderr, _u("FATAL ERROR: Not implemented in script execution\n"));
            error++;
        }
    }

    if(!HostConfigFlags::flags.BVT)
    {
        Js::Tick tkStop = Js::Tick::Now();
        Js::TickDelta tdElapsedRun = tkStop - tkStart;

        wprintf(_u("TIME: %d ms (%d ms)\n"), (tkStop - tkParse).ToMilliseconds(), (tkStop - tkStart).ToMilliseconds());
        DisplayMemStats(_u("After Execution"));
    }

    HeapDelete(scriptContext);

#ifdef REGEX_STATS
    wprintf(_u("cache hits %d\n"),_cacheHit);
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    if (freq.QuadPart == 0) {
        wprintf(_u("Your computer does not support High Resolution Performance counter\n"));
    }

    double totalTime=0.0;
    int timingCount=0;
    int cacheMissCount=0;
    for (int i=0;i<256;i++) {
        if (_regexIdCounts[i]>0) {
            double etime=(((double)(_regexIdTimes[i]*(double)1000.0/(double)freq.QuadPart)));
            double etimeUsec=(((double)(_regexIdTimes[i]*(double)1000000.0/(double)freq.QuadPart)));
            totalTime+=etime;
            RegexTiming *rt= &(_regexTimings[timingCount++]);
            rt->perIter=etimeUsec/_regexIdCounts[i];
            if (_regexIdStrLength[i]>0) {
                rt->perChar=etimeUsec/(double)_regexIdStrLength[i];
            }
            else {
                rt->perChar=0;
            }
            rt->total=etime;
            rt->id=i;
            wprintf(_u("regex %d: %d %5.2fms per iteration %6.2fus\n"),i,_regexIdCounts[i],etime,rt->perIter);
            cacheMissCount+=_regexIdCounts[i];
        }
    }
    wprintf(_u("total ms accounted for %4.2f\n"),totalTime);
    wprintf(_u("BY PER Iteration\n\n"));
    qsort_s(_regexTimings,timingCount,sizeof(RegexTiming),&SortByPerIter,NULL);
    for (int i=0;i<timingCount;i++) {
        wprintf(_u("Regex %3d: %6.2fus per iteration %5.2fms total\n"),_regexTimings[i].id,
            _regexTimings[i].perIter,_regexTimings[i].total);
        wprintf(_u("Regex source %s\n"),_regexSources[_regexTimings[i].id]);
    }

    wprintf(_u("BY PER Character\n\n"));
    qsort_s(_regexTimings,timingCount,sizeof(RegexTiming),&SortByPerChar,NULL);
    for (int i=0;i<timingCount;i++) {
        double aveLen=(double)_regexIdStrLength[_regexTimings[i].id]/_regexIdCounts[_regexTimings[i].id];
        if (aveLen>20.0) {
            wprintf(_u("Regex %3d: %6.2fus per char %5.2fms total average string length %4.2f\n"),_regexTimings[i].id,
                _regexTimings[i].perChar,_regexTimings[i].total,aveLen);
            wprintf(_u("Regex source %s\n"),_regexSources[_regexTimings[i].id]);
        }
    }

    wprintf(_u("BY TOTAL\n\n"));
    qsort_s(_regexTimings,timingCount,sizeof(RegexTiming),&SortByTotal,NULL);
    for (int i=0;i<timingCount;i++) {
        double aveLen=(double)_regexIdStrLength[_regexTimings[i].id]/_regexIdCounts[_regexTimings[i].id];
        wprintf(_u("Regex %3d: %5.2fms total %6.2fus per iter; count %4d ave len %4.2f\n"),
            _regexTimings[i].id,_regexTimings[i].total,
            _regexTimings[i].perIter,_regexIdCounts[_regexTimings[i].id],aveLen);
        wprintf(_u("Regex source %s\n"),_regexSources[_regexTimings[i].id]);
    }
    wprintf(_u("Total cache misses %d\n"),cacheMissCount);
    int bucketEntryCount=0;
    for (int j=0;j<1001;j++) {
        if (_bucketCounts[j]>0) {
            wprintf(_u("Bucket %4d: %4d\n"),j,_bucketCounts[j]);
            bucketEntryCount+=_bucketCounts[j];
        }
    }
    wprintf(_u("Total bucket entries %d\n"),bucketEntryCount);

    cacheMissCount=0;
    for (int k=0;k<_keyCount;k++) {
        wprintf(_u("Key %4d: misses %4d\n"),k,_regexHashKeyMisses[k]);
        PrintRegexKey(k);
        cacheMissCount+=_regexHashKeyMisses[k];
    }
    wprintf(_u("Total cache misses collected by key %d\n"),cacheMissCount);
#endif

    if (HostConfigFlags::flags.IgnoreScriptErrorCode)
    {
        return 0;
    }

    return error;
}

HRESULT SetOutputFile(const WCHAR* outputFile, const WCHAR* openMode)
{
    // If present, replace the {PID} token with the process ID
    const WCHAR* pidStr = nullptr;
    WCHAR buffer[_MAX_PATH];
    if ((pidStr = wcsstr(outputFile, _u("{PID}"))) != nullptr)
    {
        size_t pidStartPosition = pidStr - outputFile;

        WCHAR* pDest = buffer;
        size_t bufferLen = _MAX_PATH;

        // Copy the filename before the {PID} token
        wcsncpy_s(pDest, bufferLen, outputFile, pidStartPosition);
        pDest += pidStartPosition;
        bufferLen = bufferLen - pidStartPosition;

        // Copy the PID
        _ultow_s(GetCurrentProcessId(), pDest, /*bufferSize=*/_MAX_PATH - pidStartPosition, /*radix=*/10);
#pragma prefast(suppress: 26014, "ultow string length is smaller than 256")
        pDest += wcslen(pDest);
        bufferLen = bufferLen - wcslen(pDest);

        // Copy the rest of the string.
        wcscpy_s(pDest, bufferLen, outputFile + pidStartPosition + /*length of {PID}*/ 5);

        outputFile = buffer;
    }

    char16 fileName[_MAX_PATH];
    char16 moduleName[_MAX_PATH];
    GetModuleFileName(0, moduleName, _MAX_PATH);
    _wsplitpath_s(moduleName, nullptr, 0, nullptr, 0, fileName, _MAX_PATH, nullptr, 0);
    if (_wcsicmp(fileName, _u("WWAHost")) == 0 ||
        _wcsicmp(fileName, _u("ByteCodeGenerator")) == 0)
    {
        // we need to output to %temp% directory in wwa. we don't have permission otherwise.
        if (outputFile[1] == _u(':'))
        {
            // skip drive name
            outputFile += 2;
        }

        if (GetEnvironmentVariable(_u("temp"), fileName, _MAX_PATH) != 0)
        {
            wcscat_s(fileName, _MAX_PATH, _u("\\"));
            wcscat_s(fileName, _MAX_PATH, outputFile);
        }
        else
        {
            AssertMsg(FALSE, "Get temp environment failed");
        }
        outputFile = fileName;
    }

    FILE *fp;
    if ((fp = _wfsopen(outputFile, openMode, _SH_DENYWR)) != nullptr)
    {
        Output::SetOutputFile(fp);
        return S_OK;
    }

    AssertMsg(false, "Could not open file for logging output.");
    return E_FAIL;
}

///----------------------------------------------------------------------------
///
/// wmain2
///
/// wmain2() contains the real entrypoint logic for the program.  It's split
/// out from wmain() so that it can be wrapped with a __try/__finally.
///
///----------------------------------------------------------------------------
int _cdecl wmain2(int argc, __in_ecount(argc) LPWSTR argv[])
{
    int error = 0;

    if (!ThreadContextTLSEntry::InitializeProcess())
    {
        fwprintf(stderr, _u("FATAL ERROR: Failed to initialize ThreadContext"));
        return(-1);
    }

    if (argc >= 2)
    {
#if defined(_M_IX86)
        // Enable SSE2 math functions in CRT if SSE2 is available
        _set_SSE2_enable(TRUE);
#endif

        ULONG stackGuarantee = 16*1024;
        SetThreadStackGuarantee(&stackGuarantee);

        int parseResult;
        {
            CmdLineArgsParser parser(&HostConfigFlags::flags);
            parseResult = parser.Parse(argc, argv);
            // 'parser' destructor post-processes some configuration
        }
        if(0 == parseResult)
        {
            SmartFPUControl smartFpuControl;
            if (smartFpuControl.HasErr())
            {
                return -1;
            }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            if (Js::Configuration::Global.flags.IsEnabled(Js::OutputFileFlag)
                && Js::Configuration::Global.flags.OutputFile != nullptr)
            {
                SetOutputFile(Js::Configuration::Global.flags.OutputFile, Js::Configuration::Global.flags.OutputFileOpenMode);
            }

            if (Js::Configuration::Global.flags.DebugWindow)
            {
                Output::UseDebuggerWindow();
            }
#endif

#ifdef DYNAMIC_PROFILE_STORAGE
            if (!DynamicProfileStorage::Initialize())
            {
                return -1;
            }
#endif

#ifdef CONTROL_FLOW_GUARD_LOGGER
            if (CONFIG_FLAG(CFGLog))
            {
                CFGLogger::Enable();
            }
#endif
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            ForcedMemoryConstraint::Apply();
#endif
            EtwTrace::Register();
            ValueType::Initialize();
            ThreadContext::GlobalInitialize();

            // If -utf8 and output is file, set output mode to UTF8.
            if (CONFIG_ISENABLED(Js::Utf8Flag)
                && GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) != FILE_TYPE_CHAR)
            {
                _setmode(_fileno(stdout), _O_U8TEXT);
            }

#if DBG
            if (CONFIG_ISENABLED(Js::AssertPopUpFlag))
            {
                AssertsToConsole = false;
            }

            // Always enable this in console CHK builds
            Js::Configuration::Global.flags.CheckOpHelpers = true;
#endif
            if(!Configuration::Global.flags.IsEnabled(Js::HostTypeFlag) ||
                Configuration::Global.flags.HostType <= HostTypeDefault ||
                Configuration::Global.flags.HostType > HostTypeMax)
            {
                Configuration::Global.flags.HostType = HostTypeBrowser;
            }

            if (CONFIG_ISENABLED(Js::DebugWindowFlag))
            {
                Output::UseDebuggerWindow();
            }

            
#ifdef CHECK_MEMORY_LEAK
            // Always check memory leak in jc.exe, unless user specfied the flag already
            if (! Js::Configuration::Global.flags.IsEnabled(Js::CheckMemoryLeakFlag))
            {
                Js::Configuration::Global.flags.CheckMemoryLeak = true;
            }

            // Disable the output in case an unhandled exception happens
            // We will reenable it if there is no unhandled exceptions
            MemoryLeakCheck::SetEnableOutput(false);
#endif
            DisplayMemStats(_u("Start"));           

            ThreadContext * threadContext;

            try
            {
                AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_OutOfMemory);
                threadContext = ThreadBoundThreadContextManager::EnsureContextForCurrentThread();
                threadContext->EnsureRecycler()->Prime();
            }
            catch (Js::OutOfMemoryException)
            {
                fwprintf(stderr, _u("FATAL ERROR: Out of memory initializing thread context\n"));
                return -1;
            }

            DisplayMemStats(_u("Thread Context Primed"));

            WCHAR drive[_MAX_DRIVE];
            WCHAR directory[_MAX_DIR];
            _wsplitpath_s(Configuration::Global.flags.Filename, drive, _MAX_DRIVE, directory, _MAX_DIR, NULL, 0, NULL, 0);
            WCHAR baseDir[_MAX_PATH];
            wcscpy_s(baseDir, drive);
            wcscat_s(baseDir, directory);

            for(int iterationCount = 0; iterationCount != CONFIG_FLAG(Loop); ++iterationCount)
            {
                // Supports execution of multiple JS files with wildcard specified as inputs.
                WIN32_FIND_DATA findData;
                HANDLE findHandle = FindFirstFile(Configuration::Global.flags.Filename, &findData);
                if(findHandle == INVALID_HANDLE_VALUE)
                {
                    if(GetLastError() == ERROR_FILE_NOT_FOUND)
                    {
                        fwprintf(stderr, _u("File not found: %s"), (LPCWSTR)Configuration::Global.flags.Filename);
                    }
                    else
                    {
                        fwprintf(stderr, _u("Unknown error in finding files."));
                    }
                    return -1;
                }

                do
                {
                    WCHAR fullFileName[_MAX_PATH];
                    wcscpy_s(fullFileName, baseDir);
                    wcscat_s(fullFileName, findData.cFileName);
                    String fileName(fullFileName);
                    if(!HostConfigFlags::flags.BVT)
                    {
                        wprintf(_u("Executing file: %s\n"), (LPCWSTR)fileName);
                    }
                    int iterationError = DoOneIteration(threadContext, fileName);

                    if( iterationError != 0 )
                    {
                        error += iterationError;
                        break;
                    }

                    if(!HostConfigFlags::flags.BVT)
                    {
                        wprintf(_u("\n"));
                    }
                } while(FindNextFile(findHandle, &findData));

                FindClose(findHandle);
            }
#ifdef DYNAMIC_PROFILE_STORAGE
            DynamicProfileStorage::Uninitialize();
#endif
        }

    }
    else
    {
        PrintUsage();
    }

    Output::Flush();

    ThreadBoundThreadContextManager::DestroyAllContextsAndEntries();
#if DBG
    error += AssertCount;
#endif
#if PROFILE_DICTIONARY
    DictionaryStats::OutputStats();
#endif

    // Flush all I/O buffers
    Output::Flush();
    _flushall();

    EtwTrace::UnRegister();

#ifdef CHECK_MEMORY_LEAK
    // Only check memory leak output if there is no unhandled exceptions
    MemoryLeakCheck::SetEnableOutput(true);
#endif

    return error;
}

#ifdef DBG
STDAPI_(BOOL) DoNotUseDirectly_CULPrintf(__in_opt LPCWSTR szMsg, ...)
{
    UNREFERENCED_PARAMETER(szMsg);

    return FALSE;
}
#endif