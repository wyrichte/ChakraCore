#include "stdafx.h"

HINSTANCE g_jscript9dll = NULL;
DllGetClassObjectPtr pfDllGetClassObject = NULL;
int g_debugPrint = 0;
CRITICAL_SECTION PrintCS;
TestContext g_testContext;
LARGE_INTEGER g_frequency;
HANDLE hProcess;

int wmain(int argc, wchar_t *argv[])
{
    for(int i = 1; i < argc; ++i)
        ParseArg(argv[i]);

    if(g_testContext.pfnTest == NULL)
        Usage();


    IfFailedGo(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
    InitializeCriticalSection(&PrintCS);
    hTestEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    if(hTestEvent == INVALID_HANDLE_VALUE)
        Fail("Failed to create test event");

    QueryPerformanceFrequency(&g_frequency);

    g_jscript9dll = LoadLibrary(L"jscript9test.dll");
    if (!g_jscript9dll)
    {
        return E_FAIL;
    }
    pfDllGetClassObject = (DllGetClassObjectPtr)GetProcAddress(g_jscript9dll, "DllGetClassObject");
    if(!pfDllGetClassObject)
    {
        return E_FAIL;
    }

    // Collect the initial memory information
    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    if(!GetProcessMemoryInfo(hProcess, &g_testContext.initialMemoryCounters, sizeof(PROCESS_MEMORY_COUNTERS)))
        Fail("GetProcessMemoryInfo (initial call) failed");

    // Run the test case.
    if(g_testContext.numThreads == 0)
        g_testContext.numThreads = 1;

    printf("Running test: %S, %d thread(s), %d iterations\n", g_testContext.testName, g_testContext.numThreads, g_testContext.numIterations);

    int argCount = g_testContext.args.size();
    int *args = new int[argCount];
    std::copy(g_testContext.args.begin(), g_testContext.args.end(), args);

    if(g_testContext.numThreads > 1)
    {
        PerfTest::RunMultithreaded(g_testContext.pfnTest, g_testContext.numThreads, g_testContext.numIterations, 1, args, argCount);
    }
    else
    {
        g_testContext.pfnTest(g_testContext.numIterations, 1, 0, args, argCount);
    }

        /* end */
    delete[] args;
    CloseHandle(hProcess);
    CoUninitialize();
    return 0;

LReturn:
    printf("Initialization error!\n");
    return 1;
}

void Fail(__in __nullterminated char *reason)
{
    printf("FAIL: %s\n", reason);
    exit(1);
}

void ParseArg(__in __nullterminated wchar_t *arg)
{
    if(arg[0] != L'-' && arg[0] != L'/')
    {
        Fail("argument must start with '-' or '/'");
    }

    ++arg;
    
    if(arg[0] == L'?')
    {
        Usage();
    }

    // Find the argument separator, if it exists.
    wchar_t *argend = wcschr(arg, L':');
    if(argend != NULL)
    {
        *argend++ = L'\0';
    }

    // Parse the arguments.
    // TODO: factor
    if(!_wcsicmp(arg, L"debug"))
    {
        g_debugPrint = 1;
    }
    else if(!_wcsicmp(arg, L"v8"))
    {
#ifdef ALLOW_V8
        g_testContext.testKind = Engine::Kind::V8;
#else
        Fail("-v8 specified, but this build does not support the V8 engine");
#endif
    }
    else if(!_wcsicmp(arg, L"v8isolates"))
    {
#ifdef ALLOW_V8
        g_testContext.testKind = Engine::Kind::V8Isolates;
#else
        Fail("-v8isolate specified, but this build does not support the V8 engine");
#endif
    }
    else if(!_wcsicmp(arg, L"chakra"))
    {
        g_testContext.testKind = Engine::Kind::Chakra;
    }
    else if(!_wcsicmp(arg, L"lagunacontexts"))
    {
        g_testContext.testKind = Engine::Kind::LagunaContext;
    }
    else if(!_wcsicmp(arg, L"lagunaruntimes"))
    {
        g_testContext.testKind = Engine::Kind::LagunaRuntime;
    }
    else if(!_wcsicmp(arg, L"lagunasamecontext"))
    {
        g_testContext.testKind = Engine::Kind::LagunaSameContext;
    }
    else if(!_wcsicmp(arg, L"test"))
    {
        if(argend == NULL)
            Fail("-test requires an argument");

        g_testContext.testName = argend;
        g_testContext.pfnTest = PerfTest::GetTestFunction(argend);
        if(g_testContext.pfnTest == NULL)
            Fail("Test function not found");
    }
    else if(!_wcsicmp(arg, L"threads"))
    {
        if(argend == NULL)
            Fail("-threads requires an argument");
        g_testContext.numThreads = _wtoi(argend);
        if(g_testContext.numThreads == 0)
            Fail("-threads requires an integer argument");
    }
    else if(!_wcsicmp(arg, L"iterations"))
    {
        if(argend == NULL)
            Fail("-iterations requires an argument");
        g_testContext.numIterations = _wtoi(argend);
        if(g_testContext.numIterations == 0)
            Fail("-iterations requires an integer argument");
    }
    else if(!_wcsicmp(arg, L"args"))
    {
        wchar_t *ptr = argend;
        wchar_t *end;
        do {
            end = wcschr(ptr, L',');
            if(end != NULL)
                *end = L'\0';
            
            // convert the argument
            g_testContext.args.push_back(_wtoi(ptr));

            if(end == NULL)
                break;

            ptr = end + 1;
        } while(1);
    }
    else if(!_wcsicmp(arg, L"samplerate"))
    {
        if(argend == NULL)
            Fail("-samplerate requires an argument");
        g_testContext.workingSetSampleRate = _wtoi(argend);
    }
    else if(!_wcsicmp(arg, L"script"))
    {
        if(argend == NULL)
            Fail("-script requires an argument");
        g_testContext.LoadScriptFile(argend);
    }
    else if(!_wcsicmp(arg, L"resultsxml"))
    {
        if(argend == NULL)
            Fail("-resultsxml requires an argument");
        g_testContext.resultsXMLFileName = argend;
    }
    else
    {
        printf("Unknown argument: %S\n", arg);
        Usage();
    }
}

void Usage()
{
    printf("Usage: jsperf [options]\n");
    printf("    -debug                           print debug output\n");
    printf("    -lagunacontexts                  run using Laguna contexts\n");
    printf("    -lagunaruntimes                  run using Laguna runtimes\n");
    printf("    -lagunasamecontext               run with all iterations in the same Laguna context\n");
    printf("    -v8                              run using v8 engine (no isolates)\n");
    printf("    -v8isolates                      run using v8 engine (use isolates)\n");
    printf("    -chakra                          run using Chakra engine with IActiveScript (default)\n");
    printf("    -samplerate:N                    maximum working set sample rate, ms (default 5ms)\n");
    printf("    -iterations:N                    number of iterations for test\n");
    printf("    -threads:N                       number of threads to execute on (default 1)\n");
    printf("    -test:name                       test to execute\n");
    printf("    -script:file                     script file to execute\n");
    printf("    -args:a,b,...                    arguments for test case\n");
    printf("    -resultsxml:resultsxmlfile       output results in xml format to the file\n");
    printf("\n");
    printf("Available tests:\n");
    PerfTest::DumpTestList();
    exit(1);
}