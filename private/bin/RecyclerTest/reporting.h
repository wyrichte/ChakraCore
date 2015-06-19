/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

unsigned __int64 FileTimeToLongLong(FILETIME *ft);
unsigned __int64 SecondsToFileTimeInteger(unsigned __int64 seconds);
unsigned __int64 FileTimeIntegerToSeconds(unsigned __int64 seconds);

class ProgressReporter;

class TestContext
{
    // The TestContext is used to synchronize the main thread and reporting thread.  The sequence
    // for a test execution is:
    //
    //      1) Main thread signals reporting thread that a test is ready to execute (via hTestReady)
    //      2) Main thread begins executing test
    //      3) Reporting thread wakes up at periodic intervals and reports on test progress
    //      4) Once the time interval has passed, reporting thread signals main thread to finish
    //          the test (via 'done')
    //      5) Main thread finishes its last iteration.
    //      6) Main thread signals reporting thread that it's finished (via hTestFinished)
    //
    // Historically, this test has been *extremely* perf-sensitive.  This is the main reason that 
    // progress reporting is done on a separate thread, and also why every attempt has been made to
    // make tests lock-free while running their workload.


    // The current iteration, volatile so it can be updated without synchronization.  Theoretically,
    // the reader thread could see a "torn" int64 if the writer is in the process of updating, but
    // the worst that would happen is the progress report would have an incorrect iteration count.
    volatile unsigned __int64 currentIter;

    // Indicator used to tell the test case that it should stop iterating and complete.
    volatile bool done;

    unsigned __int64 startTime;
    unsigned __int64 testDuration;
    unsigned __int64 reportingInterval;

    char *testName;

    ProgressReporter *progressReporter;

public:
    TestContext(ProgressReporter *pr, unsigned __int64 testDuration, unsigned __int64 reportingInterval, __in __nullterminated char *testName) :
      testDuration(SecondsToFileTimeInteger(testDuration)), reportingInterval(SecondsToFileTimeInteger(reportingInterval)), 
          currentIter(0), done(false), startTime(0), testName(testName), progressReporter(pr)
    {
    }

    ~TestContext()
    {
    }

    unsigned __int64 GetTestDuration()
    {
        return testDuration;
    }

    unsigned __int64 GetReportingInterval()
    {
        return reportingInterval;
    }

    unsigned __int64 GetCurrentIterations()
    {
        return currentIter;
    }

    void SetCurrentIterations(unsigned __int64 iters)
    {
        currentIter = iters;
    }

    char *GetTestName()
    {
        return testName;
    }

    void Start()
    {
    }

    void Finish();

    bool GetCompleted()
    {
        return done;
    }

    void SetCompleted(bool x)
    {
        done = x;
    }

    unsigned __int64 GetStartTime()
    {
        return startTime;
    }

    void SetStartTime(unsigned __int64 currTime)
    {
        startTime = currTime;
    }

    bool NextIteration()
    {
        currentIter++;

        // if we're waiting based on time, completion is signaled by the timer thread
        return !done;
    }

};

class ProgressReporter
{
    TestContext *currentContext;

    HANDLE hTestFinished;
    HANDLE hTestReady;
    HANDLE hThread;

    volatile bool allTestsFinished;

    static VOID CALLBACK TimerProc(LPVOID, DWORD low, DWORD high);
    static unsigned int WINAPI ThreadProc(LPVOID param);

public:
    ProgressReporter();
    ~ProgressReporter();
    void StartTest(TestContext *ctx);
    void FinishTest();
    void FinishAll();
};