/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include <process.h>

unsigned __int64 FileTimeToLongLong(FILETIME *ft)
{
    return (((unsigned __int64)ft->dwHighDateTime)<<32) + ft->dwLowDateTime;
}

unsigned __int64 SecondsToFileTimeInteger(unsigned __int64 seconds)
{
    // seconds to 100-nanoseconds
    return seconds * 1000 * 1000 * 10;
}

unsigned __int64 FileTimeIntegerToSeconds(unsigned __int64 ft)
{
    // 100-nanoseconds to seconds
    return ft / 1000 / 1000 / 10;
}

unsigned __int64 GetCurrentFileTimeAsInteger()
{
        SYSTEMTIME st;
        FILETIME ft;
        GetSystemTime(&st);    
        if(SystemTimeToFileTime(&st, &ft) == FALSE)
            Fail("SystemTimeToFileTime");     

        return FileTimeToLongLong(&ft);
}


ProgressReporter::ProgressReporter() : allTestsFinished(false)
{
    hThread = (HANDLE)_beginthreadex(NULL, 0, &ProgressReporter::ThreadProc, this, 0, NULL);
    if(hThread == NULL)
    {
        Fail("CreateThread");
    }

    hTestReady = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(hTestReady == NULL)
    {
        Fail("failed to create hTestReady event");
    }

    hTestFinished = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(hTestFinished == NULL)
    {
        Fail("failed to create hTestFinished event");
    }
}

ProgressReporter::~ProgressReporter()
{
    CloseHandle(hThread);
    CloseHandle(hTestReady);
    CloseHandle(hTestFinished);
}

unsigned int WINAPI ProgressReporter::ThreadProc(LPVOID reporterObj)
{
    ProgressReporter *pr = (ProgressReporter*)reporterObj;

    HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

    LARGE_INTEGER now;
    now.QuadPart = 0;

    while(1)
    {
        // wait for a test to be created
        if(WaitForSingleObject(pr->hTestReady, INFINITE) != WAIT_OBJECT_0 && !pr->allTestsFinished)
        {
            Fail("waiting for a test to be created");           
        }

        if(pr->allTestsFinished)
        {
            return 0;
        }

        printf("Starting test '%s'...\n", pr->currentContext->GetTestName());

        pr->currentContext->SetStartTime(GetCurrentFileTimeAsInteger());
        unsigned __int64 nextFire = pr->currentContext->GetStartTime() + pr->currentContext->GetReportingInterval();

        // loop until the test is finished
        while(!pr->currentContext->GetCompleted())
        {
            LARGE_INTEGER due;
            due.QuadPart = nextFire;

            if(SetWaitableTimer(hTimer, &due, 0, NULL, NULL, TRUE) == FALSE)
            {
                Fail("SetWaitableTimer");
            }

            if(WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
            {
                Fail("waiting for the timer to fire");
            }

            unsigned __int64 currTime = nextFire;
            unsigned __int64 startTime = pr->currentContext->GetStartTime();
            unsigned __int64 testDuration = pr->currentContext->GetTestDuration();   

            // calculate the next firing time
            nextFire += pr->currentContext->GetReportingInterval();

            printf("%I64d / %I64d sec (%0.2f %%), %I64d iterations\n", 
                FileTimeIntegerToSeconds(currTime - startTime), 
                FileTimeIntegerToSeconds(testDuration), (currTime - startTime) / (1.0 * testDuration) * 100,
                pr->currentContext->GetCurrentIterations());
            fflush(stdout);

            // check if we're finished
            if(currTime >= startTime + testDuration)
            {
                pr->currentContext->SetCompleted(true);
            }

        }

        char *name = pr->currentContext->GetTestName();

        // Even though the test won't perform any further iterations of its workload,
        // it may still take a few seconds to complete (for instance, if it's in the
        // middle of a huge mark phase).  Wait until the test tells us that it's finished.
        if(WaitForSingleObject(pr->hTestFinished, INFINITE))
        {
            Fail("waiting for the active test to complete");
        }
        // don't access currentContext below here - it may have been updated for a subsequent test

        printf("Finished with test '%s'...\n", name);
        fflush(stdout);
    }
}

void ProgressReporter::StartTest(TestContext *ctx)
{
    currentContext = ctx;
    ctx->SetCurrentIterations(0);
    ResetEvent(hTestFinished);
    SetEvent(hTestReady);
}

void ProgressReporter::FinishTest()
{
    SetEvent(hTestFinished);
}

void ProgressReporter::FinishAll()
{
    // allow the reporter thread to shut down
    allTestsFinished = true;
    SetEvent(hTestReady);
}

void TestContext::Finish()
{
    if(progressReporter)
    {
        progressReporter->FinishTest();
    }
}
