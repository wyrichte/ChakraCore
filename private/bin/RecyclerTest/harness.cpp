/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

int g_totalTime = 60 * 5;   // 5 min
int g_reportInterval = 10;  // every 10 sec
char *g_testMix = NULL;
ProgressReporter* g_ProgressReporter = NULL;

#ifdef RECYCLER_PAGE_HEAP
int g_pageHeapModeType = 0;
#endif

void Usage()
{
    printf("Usage: RecyclerTest -tests:test1,[test2, ...] [-time:seconds] [-report:seconds] [-progress:true/false]\n");
    printf("Available tests:\n");
    for(TestCase *t = Tests; t->func; ++t)
    {
        printf("\t%s\n", t->name);
    }
    exit(1);
}

void ParseArg(__in __nullterminated char *arg)
{
    if(arg[0] != '-' && arg[0] != '/')
    {
        Fail("argument must start with '-' or '/'");
    }

    ++arg;
    
    if(arg[0] == '?')
    {
        Usage();
    }

    char *argend = strchr(arg, ':');
    if(argend == NULL)
    {
        Fail("argument must be of the form '-arg:val[,val]'");
    }
    *argend = '\0';

    if(!strcmp(arg, "time"))
    {
        g_totalTime = atoi(argend+1);
    }
#ifdef RECYCLER_PAGE_HEAP
    else if (!strcmp(arg, "pageheap"))
    {
        g_pageHeapModeType = atoi(argend + 1);
    }
#endif
    else if (!strcmp(arg, "report"))
    {
        g_reportInterval = atoi(argend+1);
    }
    else if(!strcmp(arg, "tests"))
    {
        size_t cch = strlen(argend+1)+2; // +2 so we can append a comma to make parsing easier         
        g_testMix = new char[cch];   
        strcpy_s(g_testMix, cch, argend+1);
        strcat_s(g_testMix, cch, ",");
    }
    else if(!strcmp(arg, "progress"))   
    {
        if(!strcmp(argend + 1, "true"))
        {
            g_ProgressReporter = new ProgressReporter();
        }
    }
    else
    {
        Fail("unknown argument");
    }
}

void Fail(__in __nullterminated char *reason)
{
    printf("FAIL: %s\n", reason);
    exit(1);
}

void DispatchTest(__in __nullterminated char *test, Recycler *recycler, ArenaAllocator *alloc)
{
    TestCase *curr = Tests;
    bool found = false;
    while(curr->func != NULL)
    {
        if(!strcmp(curr->name, test))
        {
            found = true;

            TestContext *ctx = new TestContext(g_ProgressReporter, g_totalTime, g_reportInterval, test);
            if(g_ProgressReporter)
            {
                g_ProgressReporter->StartTest(ctx);
            }

            curr->func(recycler, alloc, ctx);
            delete ctx;
            break;
        }
        ++curr;
    }
    if(!found)
    {
        printf("TEST ERROR: failed to find test '%s'\n", test);
    }
};

static void OutOfMemory()
{
    fprintf(stderr, "ERROR: out of memory\n");
    exit(-1);
}

#include "DefaultCommonExternalApi.cpp"

int __cdecl main(int argc, __in_ecount(argc) char* argv[])
{
    srand(0xabbafeed);

    PageAllocator::BackgroundPageQueue backgroundPageQueue;
    IdleDecommitPageAllocator pageAllocator(NULL, PageAllocatorType_Thread,
        0, IdleDecommitPageAllocator::DefaultMaxFreePageCount, false, &backgroundPageQueue);
    Recycler * recycler = nullptr;

    try
    {
        AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_OutOfMemory);

        // Parse the arguments first to get the settings 
        for (int i = 1; i < argc; ++i)
        {
            ParseArg(argv[i]);
        }

#ifdef RECYCLER_PAGE_HEAP
        if (g_pageHeapModeType != 0)
        {
            Js::Configuration::Global.flags.flagPresent[Js::PageHeapFlag] = true;
            Js::Configuration::Global.flags.PageHeap = g_pageHeapModeType;
        }
#endif

        recycler = new Recycler(nullptr, &pageAllocator, OutOfMemory);
        recycler->Initialize(false, nullptr);
        recycler->Prime();

        ArenaAllocator alloc(_u("TestAllocator"), &pageAllocator, OutOfMemory);

        if(g_testMix == NULL)
        {
            Usage();
        }
        if(g_totalTime % g_reportInterval != 0)
        {
            Fail("time % report != 0");
        }
        char *testEnd = NULL;
        char tmpbuf[256];

        while( (testEnd = strchr(g_testMix, ',')) != NULL)
        {
            strncpy_s(tmpbuf, 256, g_testMix, testEnd - g_testMix);
            DispatchTest(tmpbuf, recycler, &alloc);
            g_testMix = testEnd + 1;
        }

        if(g_ProgressReporter)
        {
            g_ProgressReporter->FinishAll();
            delete g_ProgressReporter;
            g_ProgressReporter = NULL;
        }
        recycler->ShutdownThread();
    }
    catch (Js::OutOfMemoryException)
    {
        fwprintf(stderr, _u("FATAL ERROR: Out of memory initializing thread context\n"));
        delete recycler;
        return -1;
    }
}
