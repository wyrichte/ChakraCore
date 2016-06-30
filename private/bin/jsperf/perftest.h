#pragma once

class DataCollector
{
public:
    DataCollector(std::string const& title, int count) : m_title(title)
    {
        m_lastWSMeasurement.QuadPart = 0;
        // Reserve all vectors to prevent automatic reallocations
        m_startupTimes.reserve(count);
        m_shutdownTimes.reserve(count);
        m_parseScriptTimes.reserve(count);
        m_runTestTimes.reserve(count);
        m_workingSet.reserve(1024);
        
    }
    void AddStartupTime(LARGE_INTEGER start, LARGE_INTEGER end);
    void AddShutdownTime(LARGE_INTEGER start, LARGE_INTEGER end);
    void AddParseScriptTime(LARGE_INTEGER start, LARGE_INTEGER end);
    void AddRunTestTime(LARGE_INTEGER start, LARGE_INTEGER end);
    void DumpStats();
    void DumpStatsToXML();

private:
    void CollectMemoryUsage();
    typedef std::vector<double> datalist_t;
    datalist_t m_startupTimes;
    datalist_t m_shutdownTimes;
    datalist_t m_parseScriptTimes;
    datalist_t m_runTestTimes;
    std::vector<std::pair<LARGE_INTEGER,PROCESS_MEMORY_COUNTERS>> m_workingSet;
    std::string m_title;
    LARGE_INTEGER m_lastWSMeasurement;
};

class PerfTest
{
public:
    //
    // count - the number of iterations to run, total
    // 
    // runScript - whether to run script, or just initialize/terminate the engine
    //
    // waitEvent - whether to wait on the global event in between initializing/running script/terminating
    //
    // args - list of arguments a test may accept (TODO: no reason to not just use a vector here)
    //
    // argCount - count of args
    //
    typedef void(*PfnTest)(int count, int runScript, int waitEvent, int *args, int argCount);

    // Creates a context, runs script, destroys a context on a single thread.
    static void CreateDestroyContexts(int count, int runScript, int waitEvent, int *args, int argCount);

    // Creates N contexts, runs script, then destroys the contexts, on a single thread (FIFO order)
    static void CreateHoldDestroyContextsFIFO(int count, int runScript, int waitEvent, int *args, int argCount);

    // Creates N contexts, runs script, then destroys the contexts, on a single thread (LIFO order)
    static void CreateHoldDestroyContextsLIFO(int count, int runScript, int waitEvent, int *args, int argCount);

    // Creates N contexts (caching the last M), then runs a script and destroys each context.
    static void CreateDestroyContextsWithCache(int count, int runScript, int waitEvent, int *args, int argCount);

    // Runs the requested test on multiple threads (synchronized).
    static void RunMultithreaded(PfnTest pfnTest, int numThreads, int count, int runScript, int *args, int argCount);

    // Returns the test function pointer for a given string.
    static PfnTest GetTestFunction(char16 *str);

    // Dumps the list of available tests
    static void DumpTestList()
    {
        for(int i = 0; i < NumTestCases; ++i)
            printf("%S\n", TestList[i].Name);
    }

private:
    static DWORD WINAPI PerfTestThreadProc(LPVOID param);

    struct TestCase { 
        const char16* Name;
        PfnTest Function;
    };
    static const int NumTestCases = 4;
    static const TestCase TestList[NumTestCases];
};


struct TestContext
{
    TestContext() : 
        testName(NULL), pfnTest(NULL), numThreads(1), numIterations(0), testKind(Engine::Kind::Chakra),
            workingSetSampleRate(5), script(_u("function max(x,y) { return x > y ? x : y; }; output=max(3,5);")), resultsXMLFileName(NULL)
        { 
        }

    void LoadScriptFile(char16 *filename);

    // Name of the test to run.
    char16 *testName;

    // Pointer to the test function
    PerfTest::PfnTest pfnTest;

    // List of args
    std::list<int> args;

    // Number of threads to execute on
    int numThreads;

    // Number of iterations the test should run for
    int numIterations;

    // V8 or Chakra
    Engine::Kind testKind;
    
    // Working set sample rate in ms
    int workingSetSampleRate;

    // Script to execute.  Hack: must set the property "z" on the global object.
    char16 *script;

    // Initial working set information
    PROCESS_MEMORY_COUNTERS initialMemoryCounters;

    char16 *resultsXMLFileName;
};

extern TestContext g_testContext;

extern HANDLE hTestEvent;