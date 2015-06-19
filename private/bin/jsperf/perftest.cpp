#include "stdafx.h"
#include "share.h"

HANDLE hTestEvent;

const PerfTest::TestCase PerfTest::TestList[4] = {
    { L"CreateDestroyContexts", &PerfTest::CreateDestroyContexts },
    { L"CreateHoldDestroyContextsFIFO", &PerfTest::CreateHoldDestroyContextsFIFO },
    { L"CreateHoldDestroyContextsLIFO", &PerfTest::CreateHoldDestroyContextsLIFO },
    { L"CreateDestroyContextsWithCache", &PerfTest::CreateDestroyContextsWithCache },
};


void DataCollector::DumpStats()
{
    AutoCriticalSection printCS(&PrintCS);
    
    bool ranScript = m_parseScriptTimes.size() != 0;

    if(ranScript)
    {
        if(m_startupTimes.size() != m_shutdownTimes.size() || 
            m_startupTimes.size() != m_parseScriptTimes.size() ||
            m_startupTimes.size() != m_runTestTimes.size())
        {
            printf("ERROR - sizes don't match\n");
            return;
        }
        printf("Startup,ParseScript,RunTest,Shutdown\n");

    }
    else
    {
        if(m_startupTimes.size() != m_shutdownTimes.size())
        {
            printf("ERROR - sizes don't match\n");
            return;
        }
        printf("Startup,Shutdown\n");
    }

    for(int i = 0; i < m_startupTimes.size(); ++i)
    {
        if(ranScript)
            printf("%f,%f,%f,%f\n", m_startupTimes[i], m_parseScriptTimes[i], m_runTestTimes[i], m_shutdownTimes[i]);
        else
            printf("%f,%f\n", m_startupTimes[i], m_shutdownTimes[i]);
    }    

    printf("\n");

    printf("Timestamp,Working Set\n");

    // First, print the initial memory information.
    printf("INIT,%f\n", g_testContext.initialMemoryCounters.WorkingSetSize / 1024.0 / 1024.0);

    for(int i = 0; i < m_workingSet.size(); ++i)
    {
        double timestamp = 1000.0 * (m_workingSet[i].first.QuadPart - m_workingSet[0].first.QuadPart) / g_frequency.QuadPart;
        printf("%f,%f\n", timestamp, m_workingSet[i].second.WorkingSetSize / 1024.0 / 1024.0);
    }

    printf("\n\n");

    if(g_testContext.resultsXMLFileName != NULL) 
    {
        DumpStatsToXML();
    }
}

void DataCollector::DumpStatsToXML()
{
    bool ranScript = m_parseScriptTimes.size() != 0;
    if(ranScript)
    {
        if(m_startupTimes.size() != m_shutdownTimes.size() || 
            m_startupTimes.size() != m_parseScriptTimes.size() ||
            m_startupTimes.size() != m_runTestTimes.size())
        {
            printf("ERROR - sizes don't match\n");
            return;
        }
    }
    else
    {
        if(m_startupTimes.size() != m_shutdownTimes.size())
        {
            printf("ERROR - sizes don't match\n");
            return;
        }
    }

    FILE *fp;
    if((fp = _wfsopen(g_testContext.resultsXMLFileName, L"wt", _SH_DENYWR)) == NULL)
    {
        Fail("results xml file open failed.");
    }

    fwprintf_s(fp, L"%s\n", L"<?xml version=\"1.0\"?>");
    fwprintf_s(fp, L"%s\n", L"<Results>");
    fwprintf_s(fp, L"%s\n", L"    <Measurement Name=\"Startup\" Type=\"Iteration\">");
    for(int i = 0; i < m_startupTimes.size(); ++i)
    {
        fwprintf_s(fp, L"%s%f%s\n", L"        <Times>", m_startupTimes[i], L"</Times>");
    }
    fwprintf_s(fp, L"%s\n", L"    </Measurement>");

    if(ranScript) 
    {
        fwprintf_s(fp, L"%s\n", L"    <Measurement Name=\"ParseScript\" Type=\"Iteration\">");
        for(int i = 0; i < m_parseScriptTimes.size(); ++i)
        {
            fwprintf_s(fp, L"%s%f%s\n", L"        <Times>", m_parseScriptTimes[i], L"</Times>");
        }
        fwprintf_s(fp, L"%s\n", L"    </Measurement>");
        fflush(fp);

        fwprintf_s(fp, L"%s\n", L"    <Measurement Name=\"RunTest\" Type=\"Iteration\">");
        for(int i = 0; i < m_runTestTimes.size(); ++i)
        {
            fwprintf_s(fp, L"%s%f%s\n", L"        <Times>", m_runTestTimes[i], L"</Times>");
        }
        fwprintf_s(fp, L"%s\n", L"    </Measurement>");
    }

    fwprintf_s(fp, L"%s\n", L"    <Measurement Name=\"Shutdown\" Type=\"Iteration\">");
    for(int i = 0; i < m_shutdownTimes.size(); ++i)
    {
        fwprintf_s(fp, L"%s%f%s\n", L"        <Times>", m_shutdownTimes[i], L"</Times>");
    }
    fwprintf_s(fp, L"%s\n", L"    </Measurement>");

    
    fwprintf_s(fp, L"%s\n", L"    <Measurement Name=\"WorkingSet\" Type=\"Sample\">");
    fwprintf_s(fp, L"%s\n", L"        <Sample>");
    fwprintf_s(fp, L"%s\n", L"            <Timestamp>Init</Timestamp>");
    fwprintf_s(fp, L"%s%f%s\n", L"            <Times>", (g_testContext.initialMemoryCounters.WorkingSetSize / 1024.0 / 1024.0) ,L"</Times>");
    fwprintf_s(fp, L"%s\n", L"        </Sample>");
    for(int i = 0; i < m_workingSet.size(); ++i)
    {
        double timestamp = 1000.0 * (m_workingSet[i].first.QuadPart - m_workingSet[0].first.QuadPart) / g_frequency.QuadPart;
        fwprintf_s(fp, L"%s\n", L"        <Sample>");
        fwprintf_s(fp, L"%s%f%s\n", L"            <Timestamp>",timestamp,L"</Timestamp>");
        fwprintf_s(fp, L"%s%f%s\n", L"            <Times>", (m_workingSet[i].second.WorkingSetSize / 1024.0 / 1024.0) ,L"</Times>");
        fwprintf_s(fp, L"%s\n", L"        </Sample>");
    }
    fwprintf_s(fp, L"%s\n", L"    </Measurement>");
    fwprintf_s(fp, L"%s\n", L"</Results>");
    
    fflush(fp);
    fclose(fp);
}

void PerfTest::CreateDestroyContextsWithCache(int count, int runScript, int waitEvent, int *args, int argCount)
{
    if(argCount != 1)
        Fail("CreateDestroyContextsWithCache requires 1 arguments");

    std::deque<Engine*> engines;
    LARGE_INTEGER start, end;
    int cache = args[0];


    std::stringstream buf;
    if(runScript)
    {
        buf << "Create, store (" << cache << " most recent), run a script, then destroy " << count << " contexts.";
    }
    else
    {
        buf << "Create, store (" << cache << " most recent), then destroy " << count << " contexts.";
    }
    DataCollector data(buf.str(), count);

    for(int i = 0; i < count; ++i)
    {
        if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
            Fail("Failed to wait for test event");
        QueryPerformanceCounter(&start);
        Engine *engine = Engine::CreateEngine(g_testContext.testKind);
        QueryPerformanceCounter(&end);
        engines.push_back(engine);
        data.AddStartupTime(start, end);
        if(waitEvent && SetEvent(hTestEvent) == 0)
            Fail("Failed to set test event");

        // Check to see if we've got enough cached SCs.
        if(engines.size() >= cache)
        {
            Engine *engine = engines.front();
            engines.pop_front();

            if(runScript)
            {
                if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
                    Fail("Failed to wait for test event");

                QueryPerformanceCounter(&start);
                engine->ParseScript(g_testContext.script);
                QueryPerformanceCounter(&end);
                data.AddParseScriptTime(start, end);

                engine->SetupTest();

                QueryPerformanceCounter(&start);
                engine->RunTest();
                QueryPerformanceCounter(&end);
                data.AddRunTestTime(start, end);

                if(waitEvent && SetEvent(hTestEvent) == 0)
                    Fail("Failed to set test event");
            }

            if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
                Fail("Failed to wait for test event");
            QueryPerformanceCounter(&start);
            delete engine;
            QueryPerformanceCounter(&end);
            data.AddShutdownTime(start, end);  
            if(waitEvent && SetEvent(hTestEvent) == 0)
                Fail("Failed to set test event");

        }
    }

    // Clean up any remaining.
    for each(Engine *engine in engines)
    {
        if(runScript)
        {
            if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
                Fail("Failed to wait for test event");

            QueryPerformanceCounter(&start);
            engine->ParseScript(g_testContext.script);
            QueryPerformanceCounter(&end);
            data.AddParseScriptTime(start, end);

            engine->SetupTest();

            QueryPerformanceCounter(&start);
            engine->RunTest();
            QueryPerformanceCounter(&end);
            data.AddRunTestTime(start, end);

            if(waitEvent && SetEvent(hTestEvent) == 0)
                Fail("Failed to set test event");
        }

        if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
            Fail("Failed to wait for test event");
        QueryPerformanceCounter(&start);
        delete engine;
        QueryPerformanceCounter(&end);
        data.AddShutdownTime(start, end);
        if(waitEvent && SetEvent(hTestEvent) == 0)
            Fail("Failed to set test event");
    }
    engines.clear();

    data.DumpStats();

LReturn:
    return;
}

void PerfTest::CreateHoldDestroyContextsFIFO(int count, int runScript, int waitEvent, int *args, int argCount)
{
    if(argCount != 0)
        Fail("CreateHoldDestroyContextsFIFO requires 0 arguments");

    std::list<Engine*> engines;
    LARGE_INTEGER start, end;

    std::stringstream buf;
    if(runScript)
    {
        buf << "Create " << count << " contexts, run a simple script on each, destroy the contexts.";
    }
    else
    {
        buf << "Create " << count << " contexts, destroy the contexts.";
    }
    DataCollector data(buf.str(), count);

    for(int i = 0; i < count; ++i)
    {
        if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
            Fail("Failed to wait for test event");
        QueryPerformanceCounter(&start);
        Engine *engine = Engine::CreateEngine(g_testContext.testKind);
        QueryPerformanceCounter(&end);
        engines.push_back(engine);
        data.AddStartupTime(start, end);  
        if(waitEvent && SetEvent(hTestEvent) == 0)
            Fail("Failed to set test event");
    }

    if(runScript)
    {
        for each(Engine *engine in engines)
        {
            if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
                Fail("Failed to wait for test event");

            QueryPerformanceCounter(&start);
            engine->ParseScript(g_testContext.script);
            QueryPerformanceCounter(&end);
            data.AddParseScriptTime(start, end);

            engine->SetupTest();

            QueryPerformanceCounter(&start);
            engine->RunTest();
            QueryPerformanceCounter(&end);
            data.AddRunTestTime(start, end);

            if(waitEvent && SetEvent(hTestEvent) == 0)
                Fail("Failed to set test event");
        }
    }

    for each(Engine *engine in engines)
    {
        if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
            Fail("Failed to wait for test event");
        QueryPerformanceCounter(&start);
        delete engine;
        QueryPerformanceCounter(&end);
        data.AddShutdownTime(start, end);
        if(waitEvent && SetEvent(hTestEvent) == 0)
            Fail("Failed to set test event");
    }


    engines.clear();
    data.DumpStats();

LReturn:
    return;
}


void PerfTest::CreateHoldDestroyContextsLIFO(int count, int runScript, int waitEvent, int *args, int argCount)
{
    if(argCount != 0)
        Fail("CreateHoldDestroyContextsLIFO requires 0 arguments");

    std::list<Engine*> engines;
    LARGE_INTEGER start, end;

    std::stringstream buf;
    if(runScript)
    {
        buf << "Create " << count << " contexts, run a simple script on each, destroy the contexts.";
    }
    else
    {
        buf << "Create " << count << " contexts, destroy the contexts.";
    }
    DataCollector data(buf.str(), count);

    for(int i = 0; i < count; ++i)
    {
        if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
            Fail("Failed to wait for test event");
        QueryPerformanceCounter(&start);
        Engine *engine = Engine::CreateEngine(g_testContext.testKind);
        QueryPerformanceCounter(&end);
        engines.push_back(engine);
        data.AddStartupTime(start, end);  
        if(waitEvent && SetEvent(hTestEvent) == 0)
            Fail("Failed to set test event");
    }

    // Reverse the list for LIFO order.
    engines.reverse();

    if(runScript)
    {
        for each(Engine *engine in engines)
        {
            if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
                Fail("Failed to wait for test event");

            QueryPerformanceCounter(&start);
            engine->ParseScript(g_testContext.script);
            QueryPerformanceCounter(&end);
            data.AddParseScriptTime(start, end);

            engine->SetupTest();

            QueryPerformanceCounter(&start);
            engine->RunTest();
            QueryPerformanceCounter(&end);
            data.AddRunTestTime(start, end);

            if(waitEvent && SetEvent(hTestEvent) == 0)
                Fail("Failed to set test event");
        }
    }

    for each(Engine *engine in engines)
    {
        if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
            Fail("Failed to wait for test event");
        QueryPerformanceCounter(&start);
        delete engine;
        QueryPerformanceCounter(&end);
        data.AddShutdownTime(start, end);
        if(waitEvent && SetEvent(hTestEvent) == 0)
            Fail("Failed to set test event");
    }


    engines.clear();
    data.DumpStats();

LReturn:
    return;
}


void PerfTest::CreateDestroyContexts(int count, int runScript, int waitEvent, int *args, int argCount)
{
    if(argCount != 0)
        Fail("CreateDestroyContexts requires 0 arguments");

    LARGE_INTEGER start, end;

    std::stringstream buf;
    if(runScript)
    {
        buf << "Create a context, run a simple script, destroy the context. (N = " << count << ")";
    }
    else
    {
        buf << "Create a context, destroy the context. (N = " << count << ")";
    }
    DataCollector data(buf.str(), count);

    for(int i = 0; i < count; ++i)
    {
        if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
            Fail("Failed to wait for test event");
        QueryPerformanceCounter(&start);
        Engine *engine = Engine::CreateEngine(g_testContext.testKind);
        QueryPerformanceCounter(&end);
        data.AddStartupTime(start, end);    
        if(waitEvent && SetEvent(hTestEvent) == 0)
            Fail("Failed to set test event");

        if(runScript)
        {
            if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
                Fail("Failed to wait for test event");
            
            QueryPerformanceCounter(&start);
            engine->ParseScript(g_testContext.script);
            QueryPerformanceCounter(&end);
            data.AddParseScriptTime(start, end);

            engine->SetupTest();
            
            QueryPerformanceCounter(&start);
            engine->RunTest();
            QueryPerformanceCounter(&end);
            data.AddRunTestTime(start, end);
            
            if(waitEvent && SetEvent(hTestEvent) == 0)
                Fail("Failed to set test event");
        }

        if(waitEvent && WaitForSingleObject(hTestEvent, INFINITE) != WAIT_OBJECT_0)
            Fail("Failed to wait for test event");
        QueryPerformanceCounter(&start);
        delete engine;
        QueryPerformanceCounter(&end);   
        data.AddShutdownTime(start, end);
        if(waitEvent && SetEvent(hTestEvent) == 0)
            Fail("Failed to set test event");
    }

    data.DumpStats();

LReturn:
    return;
}

void DataCollector::AddStartupTime(LARGE_INTEGER start, LARGE_INTEGER end)
{
    m_startupTimes.push_back(1000.0*(end.QuadPart - start.QuadPart)/(g_frequency.QuadPart));
    PROCESS_MEMORY_COUNTERS pmc;
    if(1000.0 * (end.QuadPart - m_lastWSMeasurement.QuadPart)/g_frequency.QuadPart > g_testContext.workingSetSampleRate)
    {
        CollectMemoryUsage();
    }
}

void DataCollector::AddShutdownTime(LARGE_INTEGER start, LARGE_INTEGER end)
{
    m_shutdownTimes.push_back(1000.0*(end.QuadPart - start.QuadPart)/(g_frequency.QuadPart));
    PROCESS_MEMORY_COUNTERS pmc;
    if(1000.0 * (end.QuadPart - m_lastWSMeasurement.QuadPart)/g_frequency.QuadPart  > g_testContext.workingSetSampleRate)
    {
        CollectMemoryUsage();
    }
}

void DataCollector::AddParseScriptTime(LARGE_INTEGER start, LARGE_INTEGER end)
{
    m_parseScriptTimes.push_back(1000.0*(end.QuadPart - start.QuadPart)/(g_frequency.QuadPart));
    PROCESS_MEMORY_COUNTERS pmc;
    if(1000.0 * (end.QuadPart - m_lastWSMeasurement.QuadPart)/g_frequency.QuadPart  > g_testContext.workingSetSampleRate)
    {
        CollectMemoryUsage();
    }
}

void DataCollector::AddRunTestTime(LARGE_INTEGER start, LARGE_INTEGER end)
{
    m_runTestTimes.push_back(1000.0*(end.QuadPart - start.QuadPart)/(g_frequency.QuadPart));
    PROCESS_MEMORY_COUNTERS pmc;
    if(1000.0 * (end.QuadPart - m_lastWSMeasurement.QuadPart)/g_frequency.QuadPart  > g_testContext.workingSetSampleRate)
    {
        CollectMemoryUsage();
    }
}

void DataCollector::CollectMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS pmc;
    QueryPerformanceCounter(&m_lastWSMeasurement);
    if (!GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) 
    {
        printf("GetProcessMemoryInfo failed!\n");
        FatalExit(-1);
    }
    m_workingSet.push_back(std::make_pair(m_lastWSMeasurement, pmc));
}



DWORD WINAPI PerfTest::PerfTestThreadProc(LPVOID param)
{
    PfnTest pfnTest = ((PfnTest*)param)[0];  
    int *args = (int*)param;
    int argCount = args[1];
    int count = args[2];
    int runScript = args[3];
    pfnTest(count, runScript, 1, args + 4, argCount);

    return 0;
}

void PerfTest::RunMultithreaded(PfnTest pfnTest, int numThreads, int count, int runScript, int *args, int argCount)
{
    HANDLE *handles = new HANDLE[numThreads];

    // TODO: clean up and fix for amd64 (int -> size_t)
    // TODO: should just create a data structure on the heap, and pass a ptr
    int *threadProcArgs = new int[argCount + 4];
    threadProcArgs[0] = (int)(int*)pfnTest;
    threadProcArgs[1] = argCount;
    threadProcArgs[2] = count;
    threadProcArgs[3] = runScript;
    for(int i = 0; i < argCount; ++i)
        threadProcArgs[i+4] = args[i];

    for(int i = 0; i < numThreads; ++i)
    {
        handles[i] = CreateThread(NULL, 0, &PerfTestThreadProc, threadProcArgs, 0, NULL);
    }

    // TODO: errors
    WaitForMultipleObjects(numThreads, handles, TRUE, INFINITE);

    // TODO: review whether this should allow for closing threads at different times.
    for(int i = 0; i < numThreads; ++i)
    {
        CloseHandle(handles[i]);
    }

    delete[] threadProcArgs;
    delete[] handles;
}

PerfTest::PfnTest PerfTest::GetTestFunction(wchar_t *str)
{
    for(int i = 0; i < NumTestCases; ++i)
    {
        if(!wcscmp(str, TestList[i].Name))
            return TestList[i].Function;
    }

    return NULL;
}

void TestContext::LoadScriptFile(wchar_t *filename)
{
    FILE *file = _wfopen(filename, L"rt");
    if(file == NULL)
        Fail("Couldn't open script file");

    const int MaxSize = 1024 * 512;
    char *data = new char[MaxSize];
    g_testContext.script = new wchar_t[MaxSize];

    int numRead = fread(data, 1, MaxSize, file);
    if(numRead == 0)
        Fail("Couldn't read from script file");

    data[numRead] = '\0';

    fclose(file);

    mbstowcs(g_testContext.script, data, MaxSize);
}