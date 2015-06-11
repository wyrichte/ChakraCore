/********************************************************
us*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

// Allocate tree nodes from the CRT heap
#include <Memory\CustomHeap.h>

int g_totalTime = 60 * 5;   // 5 min
int g_reportInterval = 10;  // every 10 sec
char *g_testMix = NULL;

#pragma region "Utilities"
void DebugPrint(__in __nullterminated char *str)
{
    printf("DBG: %s\n", str);
    fflush(stdout);
}

void Fail(__in __nullterminated char *reason)
{
    printf("FAIL: %s\n", reason);
    exit(1);
}


void Usage()
{
    printf("Usage: HeapTest -tests:test1,[test2, ...] [-time:seconds] [-report:seconds]\n");
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
    else if(!strcmp(arg, "report"))
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
    else
    {
        Fail("unknown argument");
    }
}
#pragma endregion 

static void OutOfMemory()
{
    fprintf(stderr, "ERROR: out of memory\n");
    exit(-1);
}

class SmartArena
{
public:
    SmartArena(ArenaAllocator* arena):
      arena(arena)
    {
    }

    ~SmartArena()
    {
        this->arena->Clear();
    }

private:
    ArenaAllocator* arena;
};

#if DBG_DMP

CustomHeap::Allocation* Test(CustomHeap::Heap& heap, size_t size)
{
    printf("\nTesting %d bytes\n", size);
    CustomHeap::Allocation* alloc = heap.Alloc(size);
    printf("\nAllocation: 0x%p\n", alloc->address);
    heap.DumpStats();

    return alloc;
}

void Call_Helper()
{
    printf("Call Helper was called\n");
    fflush(stdout);
}

void TestExecutableCodeAlloc(CustomHeap::Heap& heap)
{
    printf("\nTesting custom heap executable code alloc\n");

    CustomHeap::Allocation* allocation = heap.Alloc(0x16);

    char* buffer = allocation->address;

    // jump instruction to Call_Helper's address
    buffer[0] = (char) 0xff;
    buffer[1] = (char) 0x25;
    *reinterpret_cast<DWORD*>(buffer+2) = reinterpret_cast<DWORD>(buffer + 6);
    *reinterpret_cast<DWORD*>(buffer+6) = reinterpret_cast<DWORD>(Call_Helper);

    DWORD oldProtect;
    heap.ProtectAllocation(allocation, PAGE_EXECUTE, &oldProtect);

    typedef void (*Func)();
    Func myFunc = (Func)(buffer);
    (myFunc)();

    // REVIEW: Should this be done within free itself? Should free imply remove execute, add read/write?
    heap.ProtectAllocation(allocation, PAGE_READWRITE, &oldProtect);
    heap.Free(allocation);
}

void TestCustomHeap()
{
    IdleDecommitPageAllocator pageAllocator;

    ArenaAllocator* alloc = new ArenaAllocator(L"customHeapArena", &pageAllocator, OutOfMemory);
    SmartArena allocGuard(alloc);

    DebugPrint("\nTesting Heap\n");
    // Arena needed for storing auxilliary tree data
    CustomHeap::Heap heap(alloc);

    CustomHeap::Allocation* c = Test(heap, 128);

    // First alloc from the 128 bucket
    Assert(c->address == c->page->address);
    Assert(c->page->currentBucket == CustomHeap::BucketId::SmallObjectList);

    CustomHeap::Allocation* d = Test(heap, 280);

    // First alloc from the 512 bucket
    Assert(d->address == d->page->address);
    Assert(d->page->currentBucket == CustomHeap::BucketId::Bucket512);

    CustomHeap::Allocation* e = Test(heap, 512);

    // second alloc from the 512 bucket
    Assert(e->address == (e->page->address + 512));
    Assert(e->page->currentBucket == CustomHeap::BucketId::Bucket512);

    CustomHeap::Allocation* f = Test(heap, 252);

    // Allocing from 256 bucket- this should cause the 512 page to split
    Assert(f->page->address == e->page->address);
    Assert(f->address == f->page->address + (2 * 512));
    Assert(f->page->currentBucket == CustomHeap::BucketId::Bucket256);

    heap.Free(f);

    CustomHeap::Allocation* f2 = Test(heap, 252);

    // Reused alloc from the 512 bucket
    Assert(f2->address == f2->page->address + (2 * 512));
    Assert(f2->page->currentBucket == CustomHeap::BucketId::Bucket256);

    heap.Free(f2);

    int numAllocs = 15;
    CustomHeap::Allocation** p = new CustomHeap::Allocation*[numAllocs];

    for (int i = 0; i < numAllocs; i++)
    {
        p[i] = Test(heap, 4092);
        Assert(p[i]->address == p[i]->page->address);
    }

    heap.DumpStats();

    for (int i = 0; i < numAllocs; i++)
    {
        printf("Freeing 0x%p\n", p[i]);
        heap.Free(p[i]);
    }

    delete[] p;

    CustomHeap::Allocation* g = Test(heap, 30 * 1024);

    heap.Free(g);

    CustomHeap::Allocation* h = Test(heap, 1024);

    heap.Free(h);

    heap.DumpStats();

    TestExecutableCodeAlloc(heap);
    heap.DumpStats();
}

void TestBitVector()
{
    Output::Print(L"BitVector tests\n");

    BVUnit bv(0xffffffff); bv.DumpWord(); VerboseHeapTrace(L"\n");

    bv.Clear(31); bv.DumpWord(); VerboseHeapTrace(L"\n"); Assert(bv.GetWord() == 0x7fffffff);
    bv.Clear(0); bv.DumpWord(); VerboseHeapTrace(L"\n"); Assert(bv.GetWord() == 0x7ffffffe);

    bv = BVUnit(0xffffffff); bv.DumpWord(); VerboseHeapTrace(L"\n"); Assert(bv.GetWord() == 0xffffffff);
    bv.ClearRange(28, 4); bv.DumpWord(); VerboseHeapTrace(L"\n");Assert(bv.GetWord() == 0x0fffffff);

    bv.ClearRange(0, 1); bv.DumpWord(); VerboseHeapTrace(L"\n"); Assert(bv.GetWord() == 0x0ffffffe);

    int l = 4;

    bv.Dump(); VerboseHeapTrace(L"\n");

    VerboseHeapTrace(L"Num leading zeroes: %d\n", bv.GetNumberOfLeadingZeroes());
    int i = bv.FirstStringOfOnes(l);
    VerboseHeapTrace(L"First string of 1's of length %d: %d\n", l, i + l);

    bv.ClearRange(i, l); 
    bv.DumpWord(); VerboseHeapTrace(L"\n"); 
    Assert(bv.GetWord() == 0x0fffffe0);
}
#endif

int __cdecl main(int argc, __in_ecount(argc) char* argv[])
{
    if (!ThreadContext::Initialize())
    {
        fwprintf(stderr, L"FATAL ERROR: Failed to initialize ThreadContext");
        return(-1);
    }

    ThreadContext * threadContext;

    try
    {
        AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_OutOfMemory);
        threadContext = ThreadContext::EnsureContextForCurrentThread();
        threadContext->EnsureRecycler()->Prime();               

#if DBG_DMP
        TestBitVector();
        TestCustomHeap();
#endif
    }
    catch (Js::OutOfMemoryException)
    {
        fwprintf(stderr, L"FATAL ERROR: Out of memory initializing thread context\n");
        return -1;
    }


    /*
    for(int i = 1; i < argc; ++i)
    {
        ParseArg(argv[i]);
    }
    if(g_testMix == NULL)
    {
        Usage();
    }
    if(g_totalTime % g_reportInterval != 0)
    {
        Fail("time % report != 0");
    }
    */
}
