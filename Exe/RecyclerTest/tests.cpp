/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include <windows.h>
#include <process.h>
#include <assert.h>

typedef BOOL(*CheckFn_t)(char*, size_t);


TestCase Tests[] = {
    {&TestBasic, "Basic"},
    {&TestBasic2, "Basic2"},
    {&TestCycles, "Cycles"},
    {&TestList, "List"},
    {&TestPointerSwaps, "PointerSwaps"},
    {&TestMarkSweepDynamic, "MarkSweepDynamic"},
    {&TestMarkSweepRandom, "MarkSweepRandom"},
    {&TestMarkSweep, "MarkSweep"},
    {&TestLargeSimple, "LargeSimple"},
    {&TestMem, "Mem"},
    {&TestDoublyLinkedList, "DoublyLinkeTestDList"},
    {&TestWeakReferences, "WeakRef"},
    {&TestWeakReferenceHashTable, "WeakRefHashTable"},
    {&TestPageHeapAlloc, "PageHeapAlloc" },
    {NULL, NULL},
};

// starts N threads that spin doing nothing
void UseCPU(int numthreads)
{
    struct X
    {
        static unsigned int WINAPI ThreadProc(LPVOID)
        {
            volatile int x = 0xcafebabe;
            volatile int y = 0xdeadbeef;
            while(1)
            {
                x ^= y;
                y ^= x;
                x ^= y;
            }
            return TRUE;
        }
    };

    for(int i = 0; i < numthreads; ++i)
    {
        _beginthreadex(NULL, 0, &X::ThreadProc, NULL, NULL, NULL);
    }

}

tracker *objtracker;

BOOL CheckFnMemoryMapped(void *addr, size_t size)
{
    if(objtracker->dbg_level >= tracker::All)
        printf("NOTE: CheckFnMemoryMapped noticed collection of %p\n", addr);

    if(objtracker->object_tracker->check(addr))
    {
        if(objtracker->dbg_level >= tracker::Errors)
        {
            printf("RECYCLER ERROR: CheckFnMemoryMapped caught free of live object %p\n", addr);
            printf("PID: %x\n", GetCurrentProcessId());
            fflush(stdout);
            exit(1);
        }
    }

    return TRUE;
}

void AllocateWeakReferences(RecyclerWeakReference<void>** weakReferences, void** ptrs, tracker* objtracker, Recycler* recycler, int objcount)
{
    // allocate objects in the recycler
    for(int j = 0; j < objcount; ++j)
    {
        volatile void* object = ptrs[j];
        volatile RecyclerWeakReference<void>* weakRef = (RecyclerWeakReference<void>*) recycler->CreateWeakReferenceHandle<void>((void*) object);
        objtracker->allocator->update_pointer(reinterpret_cast<void**>(&(weakReferences[j])), (void*) weakRef);
        object = NULL; // clear out the var in the stack frame
        weakRef = NULL;
    }
}

void VerifyWeakReferences(RecyclerWeakReference<void>** weakReferences, void** ptrs, int objcount)
{
    for (int j = 0; j < objcount; ++j)
    {
        volatile RecyclerWeakReference<void>* weakRef = weakReferences[j];
        volatile void* object = ptrs[j];

        Assert(((RecyclerWeakReference<void>*)weakRef)->Get() == object);
        object = NULL; // clear out the var in the stack frame
        weakRef = NULL;
    }
}

void VerifyWeakReferences(RecyclerWeakReference<void>** weakReferences, int objcount)
{
    for (int j = 0; j < objcount; ++j)
    {
        volatile RecyclerWeakReference<void>* ref = weakReferences[j];

        Assert(((RecyclerWeakReference<void>*)ref)->Get() == NULL);
        ref = NULL; // clear out the var in the stack frame
    }
}

void FreeWeakReferences(RecyclerWeakReference<void>** weakReferences, tracker* objtracker, int objcount)
{
    for(int j = 0; j < objcount; ++j)
    {
        volatile RecyclerWeakReference<void>* ref = weakReferences[j];
        objtracker->allocator->update_pointer(reinterpret_cast<void**>(&(weakReferences[j])),0);
        ref=NULL;
    }

    //TODO: Have some way of checking that the weak-reference entry is actually freed during collection
}

void TestWeakReferences(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx)
{
    Output::UseDebuggerWindow();

    objtracker = new tracker(new recycler_wrapper(recycler));
    //tracker *objtracker = new tracker();

    objtracker->dbg_level = tracker::Errors;

    // # of objects
    const int objcount = 100;
    // # of bytes per object
    const int bytes = 256;

    // allocate some storage for pointers.
    void **ptrs = (void**)objtracker->track_malloc(sizeof(void*)*objcount);
    RecyclerWeakReference<void>** weakReferences = (RecyclerWeakReference<void>**)objtracker->track_malloc(sizeof(RecyclerWeakReference<void>*) * objcount);

    ctx->Start();
    while(ctx->NextIteration())
    {
        // allocate objects in the recycler
        for(int j = 0; j < objcount; ++j)
        {
            void* object = objtracker->track_malloc(bytes);
            objtracker->allocator->update_pointer(&(ptrs[j]), object);
            object = NULL; // Null it out so that it's not still on the stack
        }

        AllocateWeakReferences(weakReferences, ptrs, objtracker, recycler, objcount);
        VerifyWeakReferences(weakReferences, ptrs, objcount);

        // free every object
        for(int j = 0; j < objcount; ++j)
        {
            void* ptr=ptrs[j];
            objtracker->allocator->update_pointer(&(ptrs[j]),0);
            objtracker->track_free(ptr);
            ptr=NULL;
        }

        // Do a full in-thread collection
        recycler->CollectNow<CollectNowForceInThread>();

        VerifyWeakReferences(weakReferences, objcount);

        // free weak refs
        FreeWeakReferences(weakReferences, objtracker, objcount);
    }

    objtracker->track_free(ptrs);
    objtracker->track_free(weakReferences);

    ctx->Finish();
    delete objtracker;
}

void TestWeakReferenceHashTable(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx)
{
    Output::UseDebuggerWindow();

    objtracker = new tracker(new recycler_wrapper(recycler));
    //tracker *objtracker = new tracker();

    objtracker->dbg_level = tracker::Errors;

    // # of objects
    const int objcount = 100;
    // # of bytes per object
    const int bytes = 256;

    // allocate some storage for pointers.
    void **ptrs = (void**)objtracker->track_malloc(sizeof(void*)*objcount);
    RecyclerWeakReferenceBase** weakReferences = (RecyclerWeakReferenceBase**)objtracker->track_malloc(sizeof(RecyclerWeakReferenceBase*) * objcount);
    WeakReferenceHashTable<PrimePolicy> hashTable(objcount, &HeapAllocator::Instance);

    ctx->Start();
    while(ctx->NextIteration())
    {
        // allocate objects in the recycler
        for(int j = 0; j < objcount; ++j)
        {
            void* object = objtracker->track_malloc(bytes);
            objtracker->allocator->update_pointer(&(ptrs[j]), object);
            weakReferences[j] = hashTable.Add((char*) object, recycler);
            object = NULL;
        }

#ifdef RECYCLER_TRACE_WEAKREF
        Output::Print(L"After adding nodes\n");
        hashTable.Dump();
#endif

        for (int j = objcount - 1; j >= 0; j--)
        {
            volatile RecyclerWeakReferenceBase* weakRef = NULL;
            Assert(hashTable.TryGetValue((char*) ptrs[j], (RecyclerWeakReferenceBase**) &weakRef));

            // Hack: we know the layout of weakRef, and we know that the first item is the strong ref
            Assert((*((char**)(weakRef))) == ptrs[j]);

            hashTable.Remove((char *)ptrs[j]);
            weakRef = NULL;
        }

#ifdef RECYCLER_TRACE_WEAKREF
        Output::Print(L"After removing nodes\n");
        hashTable.Dump();
#endif

        // free every object
        for(int j = 0; j < objcount; ++j)
        {
            void* ptr=ptrs[j];
            objtracker->allocator->update_pointer(&(ptrs[j]),0);
            objtracker->track_free(ptr);
            ptr=NULL;
        }

        // Do a full in-thread collection
        recycler->CollectNow<CollectNowForceInThread>();

        // free weak reference array
        for(int j = 0; j < objcount; ++j)
        {
            volatile RecyclerWeakReferenceBase* weakRef = weakReferences[j];

            Assert(weakRef != null);
            weakRef = null;
            weakReferences[j] = null;
        }

        recycler->CollectNow<CollectNowForceInThread>();
    }

    objtracker->track_free(ptrs);
    objtracker->track_free(weakReferences);

    ctx->Finish();
    delete objtracker;
}

void TestBasic(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx)
{
    objtracker = new tracker(new recycler_wrapper(recycler));
    //tracker *objtracker = new tracker();

    objtracker->dbg_level = tracker::Errors;

    // # of objects
    const int objcount = 100;
    // # of bytes per object
    const int bytes = 256;

    // allocate some storage for pointers.
    void **ptrs = (void**)objtracker->track_malloc(sizeof(void*)*objcount);

    ctx->Start();
    while(ctx->NextIteration())
    {
        // allocate objects in the recycler
        for(int j = 0; j < objcount; ++j)
        {
            objtracker->allocator->update_pointer(&(ptrs[j]),objtracker->track_malloc(bytes));
        }

        // free every object
        for(int j = 0; j < objcount; ++j)
        {
            void* ptr=ptrs[j];
            objtracker->allocator->update_pointer(&(ptrs[j]),0);
            objtracker->track_free(ptr);
            ptr=NULL;
        }

    }

    objtracker->track_free(ptrs);
    ctx->Finish();
    delete objtracker;
}


void TestList(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx)
{
    objtracker = new tracker(new recycler_wrapper(recycler));
    objtracker->dbg_level = tracker::Errors;
    recycler->SetCheckFn((CheckFn_t)CheckFnMemoryMapped);

    {
        tracked::list l(objtracker);

        int len = 0;
        ctx->Start();
        while(ctx->NextIteration())
        {
            for(int j = 0; j < 50; ++j)
            {
                if(rand()%3)
                    l.append(128);
                else
                    l.append(16);
                ++len;

                if(rand()%50 == 0)
                    l.reverse();
            }
            if(len > 500)
            {
                l.prune(l.root);
                len = 0;
            }
            // l.dump();

        }

        l.objtracker->dump();
    }
    ctx->Finish();
    recycler->SetCheckFn(null);
    delete objtracker;
}

void TestBasic2(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx)
{
    objtracker = new tracker(new recycler_wrapper(recycler));
    recycler->SetCheckFn((CheckFn_t)CheckFnMemoryMapped);
    //tracker *objtracker = new tracker(new allocator_base());

    objtracker->dbg_level = tracker::Errors;


    // # of 1st level objects
    const int objcount1 = 153;
    // # of 2nd level objects per 1st level object
    const int objcount2 = 127;
    // # of iterations for the test
    const int iters = 500;
    // # of bytes per object
    int bytes;

    // allocate some storage for first level objects.
    void ***ptrs = (void***)objtracker->track_malloc(sizeof(void**)*objcount1);

    ctx->Start();
    while(ctx->NextIteration())
    {

        for (bytes=16;bytes<=192;bytes+=16) {
            for(int iter = 0; iter < iters; ++iter)
            {
                // allocate first level 
                //puts("======== allocating 1st level objects");
                for(int i = 0; i < objcount1; ++i)
                {
                    void *newobj = objtracker->track_malloc(sizeof(void*)*objcount2);
                    objtracker->allocator->update_pointer((void**)&ptrs[i], newobj);
                }

                // allocate 2nd level objects
                for(int i = 0; i < objcount1; ++i)
                {
                    //printf("======== allocating 2nd level objects for %p\n", ptrs[i]);
                    for(int j = 0; j < objcount2; ++j)
                    {
                        void *newobj = objtracker->track_malloc(bytes);
                        objtracker->allocator->update_pointer((void**)&(ptrs[i][j]), newobj);
                    }
                }
                // ok, free up everything
                for(int i = 0; i < objcount1; ++i)
                {
                    //printf("======== freeing 2nd level objects for %p\n", ptrs[i]);
                    for(int j = 0; j < objcount2; ++j)
                    {
                        void *tmp = ptrs[i][j];
                        objtracker->allocator->update_pointer((void**)&(ptrs[i][j]), 0);
                        objtracker->track_free(tmp);          
                    }
                    //printf("======== freeing %p\n", ptrs[i]);
                    void *tmp = ptrs[i];
                    objtracker->allocator->update_pointer((void**)&ptrs[i], 0);
                    objtracker->track_free(tmp);
                }
            }
        }
    }
    ctx->Finish();
    recycler->SetCheckFn(null);
    delete objtracker;
}

void TestCycles(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx)
{
    objtracker = new tracker(new recycler_wrapper(recycler));
    recycler->SetCheckFn((CheckFn_t)CheckFnMemoryMapped);    

    const int cycle_count = 125;
    
    void *volatile*root = 0;

    ctx->Start();
    while(ctx->NextIteration())
    {
        void **prev = 0;
        void **curr = 0;

        // build up a cycle_count length linked list
        for(int i = 0; i < cycle_count; ++i)
        {
            prev = curr;
            curr = (void**)objtracker->track_malloc(sizeof(void*)*128);
            if(i == 0)
                root = curr;
            else
            {
                objtracker->allocator->update_pointer(prev, (void*)curr);
            }
        }

        // close the cycle
        objtracker->allocator->update_pointer(curr, (void*)root);

        // free up the cycle in the object tracker
        void **tmp = (void**)root;
        void **next = 0;
        do
        {
            // the debug CRT will zero out freed memory, so save a pointer first
            next = (void**)*tmp;
            objtracker->track_free(tmp);
            tmp = next;
        } while(tmp != (void**)root);

        // release the root to it
        root = 0;
    }

    ctx->Finish();
    recycler->SetCheckFn(null);
    delete objtracker;
}

void OutOfMemory()
{
    fprintf(stderr, "ERROR: out of memory\n");
    exit(1);
}

#include <psapi.h>

void TestMem(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx) {
    Recycler* r2;
    ArenaAllocator* a2;
    PROCESS_MEMORY_COUNTERS memCounters;
    memCounters.cb=sizeof(memCounters);
    int prevPeak=0;
    int initialPeak=0;
    int i = 0;
    ctx->Start();
    while(ctx->NextIteration()) {
        ++i;
        a2=new ArenaAllocator(L"eeek", alloc->GetPageAllocator(), OutOfMemory);
        r2=new Recycler(NULL, (IdleDecommitPageAllocator *)alloc->GetPageAllocator(), OutOfMemory);
        r2->Initialize(false, NULL);
        delete r2;
        delete a2;
        if ((i%100)==0) {
            GetProcessMemoryInfo(GetCurrentProcess(),&memCounters,memCounters.cb);
            if (prevPeak==0) {
                printf("%d alloc/delete pairs; peak working set %d\n",i, (int)memCounters.PeakWorkingSetSize);
                initialPeak=(int)memCounters.PeakWorkingSetSize;
            }
            else {
                printf("%d alloc/delete pairs; peak working set %d; delta %d\n",i,(int)memCounters.PeakWorkingSetSize,
                    (int)(memCounters.PeakWorkingSetSize-prevPeak));
            }
            prevPeak=(int)memCounters.PeakWorkingSetSize;
        }
    }
    GetProcessMemoryInfo(GetCurrentProcess(),&memCounters,memCounters.cb);
    printf("final peak working set change %d; per iteration %4.2f\n",(int)(memCounters.PeakWorkingSetSize-initialPeak),
        (memCounters.PeakWorkingSetSize-initialPeak)/10000.0);
    ctx->Finish();
}

void TestPointerSwaps(Recycler *recycler, ArenaAllocator *alloc, TestContext *ctx)
{
    objtracker = new tracker(new recycler_wrapper(recycler));
    //objtracker = new tracker(new allocator_base());
    recycler->SetCheckFn((CheckFn_t)CheckFnMemoryMapped);

    const int num_tmpobj = 10000;
    const int num_testobj = 5;
    const int num_bytes = 64;

    void *tmpbuf[num_tmpobj + 1];
    int bufidx = 0;

    void ***testobjs[num_testobj];

    // create 50 container objects
    for(int i = 0; i < num_testobj; ++i)
    {
        testobjs[i] = (void***)objtracker->track_malloc(num_bytes);
    }

    // create 50 2-item linked lists
    for(int i = 0; i < num_testobj; ++i)
    {
        void *tmp1 = objtracker->track_malloc(num_bytes);
        objtracker->allocator->update_pointer((void**)testobjs[i], tmp1);
    }
    for(int i = 0; i < num_testobj; ++i)
    {
        void *tmp1 = objtracker->track_malloc(num_bytes*4);
        objtracker->allocator->update_pointer((void**)*testobjs[i], tmp1);
    }

    ctx->Start();
    while(ctx->NextIteration())
    {
        // reverse linked lists
        for(int i = 0; i < num_testobj; ++i)
        {
            //allocate an object
            if(bufidx < _countof(tmpbuf))
            {
                tmpbuf[bufidx++] = objtracker->track_malloc(num_bytes);
            }
            else
            {
                Fail("logic error in TestPointerSwaps");
            }
            
            // free objects if needed
            if(bufidx == num_tmpobj)
            {
                for(int i = 0; i < bufidx; ++i)
                {
                    objtracker->track_free(tmpbuf[i]);
                    tmpbuf[i] = 0;
                }
                bufidx = 0;
            }

            void *tmp = (void*)*testobjs[i];
            objtracker->allocator->update_pointer((void**)testobjs[i], (void*)**testobjs[i]);
  
            //allocate an object
            if(bufidx < _countof(tmpbuf))
            {
                tmpbuf[bufidx++] = objtracker->track_malloc(num_bytes);
            }
            else
            {
                Fail("logic error in TestPointerSwaps");
            }
            // free objects if needed
            if(bufidx == num_tmpobj)
            {
                for(int i = 0; i < bufidx; ++i)
                {
                    objtracker->track_free(tmpbuf[i]);
                    tmpbuf[i] = 0;
                }
                bufidx = 0;
            }

            // complete the reversal
            objtracker->allocator->update_pointer((void**)*testobjs[i], tmp);
            objtracker->allocator->update_pointer((void**)tmp, NULL);

        }
    }
    ctx->Finish();
    recycler->SetCheckFn(null);
    delete objtracker;
}

void TestMarkSweep(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx) {
    LARGE_INTEGER startTime,finishTime;
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    if (freq.QuadPart == 0) {
        printf("Your computer does not support High Resolution Performance counter\n");
    }

    QueryPerformanceCounter(&startTime);
    int k;
    for (k=0;k<1;k++) {
        for (int i=0;i<1000000;i++) {
            recycler->Alloc(16);
        }
    }
    QueryPerformanceCounter(&finishTime);
    printf("Elapsed Time per %dM allocs w/ parallel collect : %8.6f mSec\n",k,
         (((double)( (finishTime.QuadPart - startTime.QuadPart)* (double)1000.0/(double)freq.QuadPart )))/(1) );
    QueryPerformanceCounter(&startTime);
    for (k=0;k<1;k++) {
        for (int i=0;i<1000000;i++) {
            alloc->Alloc(16);
        }
    }
    QueryPerformanceCounter(&finishTime);
    printf("Elapsed Time per %dM allocs w/ arena : %8.6f mSec\n",k,
         (((double)( (finishTime.QuadPart - startTime.QuadPart)* (double)1000.0/(double)freq.QuadPart )))/(1) );
}

void TestLargeSimple(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx) {
    ctx->Start();
    while(ctx->NextIteration()) {
        for (int j=0;j<1000;j++) {
            recycler->Alloc(4096);
        }
        for (int j=0;j<1000;j++) {
            recycler->Alloc(16);
        }
    }
    ctx->Finish();
}

void TestMarkSweepRandom(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx) {
    _int64 totalAlloc=0;
    int k = 0;
    ctx->Start();
    while(ctx->NextIteration()) {
        ++k;
        char* pointers[20000];
        for (int j=0;j<100;j++) {
            for (int i=0;i<20000;i++) {
                int nBytes=(rand()%(HeapConstants::MaxSmallObjectSize-1))+1;
                pointers[i]=recycler->Alloc(nBytes);
                totalAlloc+=nBytes;
            }
            int x=0;
            for (int i=0;i<20000;i++) {
                x^=(int)pointers[i];
            }
        }
        if (0==(k%10)) {
            printf("allocated  %.2fM\n",(double)totalAlloc/1000000.0);
            fflush(stdout);
        }
    }
    printf("allocated  %.2fM\n",(double)totalAlloc/1000000.0);
    ctx->Finish();
}

struct DynamicObject {
    int type;
    void** slots;
    BOOL free;
};

BOOL CheckDynamic(void* addr,int size) {
    if (size>16) {
        void** slots=(void**)addr;
        int nSlots=size/sizeof(void*);
        for (int k=0;k<nSlots;k+=2) {
            if (slots[k+1]!=(slots+k+1))
                printf("corruption in slot check\n");
        }
    }
    else {
        DynamicObject* obj=(DynamicObject*)addr;
        if (obj->free)
            printf("double free\n");
        if (obj->type!=((int)obj+1)) {
            printf("corruption in type field\n");            
        }
#ifdef CHECK_CHILD
        if (obj->slots!=NULL) {
            int nSlots=8;
            for (int k=0;k<nSlots;k+=2) {
                if (obj->slots[k+1]!=obj->slots+k+1)
                    printf("corruption in child slot check\n");
            }
        }
#endif
        obj->free=true;
    }
    return true;
}

void TestMarkSweepDynamic(Recycler* recycler,ArenaAllocator *alloc, TestContext *ctx) {
    recycler->SetCheckFn((CheckFn_t)CheckDynamic);
    _int64 totalAlloc=0;
    int k = 0;
    ctx->Start();
    while(ctx->NextIteration()) {
        ++k;
        volatile DynamicObject* pointers[100000];
        for (int j=0;j<10;j++) {
            for (int i=0;i<100000;i++) {
                int nSlots=8;
                int nBytes=nSlots*sizeof(void*);
                totalAlloc+=16;
                totalAlloc+=nBytes;
                pointers[i]=(DynamicObject*)recycler->Alloc(16);
                pointers[i]->type=(int)pointers[i]+1;
                pointers[i]->free=false;
                pointers[i]->slots=NULL;
                if (i>2) {
                    pointers[i]->slots = (void **)recycler->Alloc(nBytes);
                    for (int k=0;k<nSlots;k+=2) {
                        int refIndex=rand()%(i-1);
                        pointers[i]->slots[k] = (void*)pointers[refIndex];
                        pointers[i]->slots[k+1] = pointers[i]->slots+k+1;
                    }
                }
            }
        }
        if (0==(k%10)) {
            printf("allocated %.2fM\n",(double)totalAlloc/1000000.0);
            fflush(stdout);
        }
    }
    printf("allocated %.2fM\n",(double)totalAlloc/1000000.0);
    recycler->SetCheckFn(null);
    ctx->Finish();
}

struct TestDList {
    int isHead;
    TestDList *next;
    TestDList *prev;
    int data;
};

TestDList* TestDListMakeListEntry(Recycler* recycler,HeapInfo* heapInfo) {
    TestDList *entry=(TestDList*)recycler->RealAlloc<NoBit>(heapInfo, sizeof(TestDList));
    entry->next=entry;
    entry->prev=entry;
    entry->isHead=false;
    return(entry);
}

TestDList* TestDListMakeListHead(Recycler* recycler,HeapInfo* heapInfo) {
    TestDList *entry=(TestDList*)recycler->RealAlloc<NoBit>(heapInfo, sizeof(TestDList));
    entry->next=entry;
    entry->prev=entry;
    entry->isHead=false;
    return(entry);
}

TestDList *TestDListAdd(TestDList *head,int value, Recycler* recycler,HeapInfo* heapInfo) {
    TestDList *entry=TestDListMakeListEntry(recycler,heapInfo);
    entry->data=value;

    head->prev->next=entry;
    entry->next=head;
    entry->prev=head->prev;
    head->prev=entry;
    return(entry);
}

TestDList* TestDListRemoveEntry(TestDList *entry,Recycler* recycler) {
    if (entry == NULL)
        printf("NULL entry to list_remove\n");
    else if (entry->isHead)
        printf("Tried to delete head of list\n");
    else {
        entry->next->prev=entry->prev;
        entry->prev->next=entry->next;
    }
    return(entry);
}

TestDList* TestDListInsertAfter(TestDList* before,int data,Recycler* recycler,HeapInfo* heapInfo) {
    TestDList* entry=TestDListMakeListEntry(recycler,heapInfo);
    entry->data=data;
    entry->next=before->next;
    entry->prev=before;
    before->next=entry;
    entry->next->prev=entry;
    return(entry);
}

#define LIST_LEN 5000

TestDList* listEntries[LIST_LEN];
BOOL stop=false;

BOOL CheckTestDList(void* addr,int size) {
    TestDList* dlist=(TestDList*) addr;
    for (int i=0;i<LIST_LEN;i++) {
        if (dlist==listEntries[i]) {
            stop=true;
            printf("free of item on list 0x%Ix %d\n",dlist,dlist->data);
            return false;
        }
    }
    return  true;
}

void TestDoublyLinkedList(Recycler* recycler,ArenaAllocator* alloc, TestContext *ctx) {
    int i;
    recycler->SetCheckFn((CheckFn_t)CheckTestDList);
    HeapInfo* heapInfo=recycler->GetAutoHeap();
    TestDList* dlist=TestDListMakeListHead(recycler,heapInfo);
    printf("list head is 0x%Ix at stack address 0x%Ix\n",dlist,&dlist);
    // add LIST_LEN elements
    for (i=0;i<LIST_LEN;i++) {
        listEntries[i]=TestDListAdd(dlist,i,recycler,heapInfo);
    }
    // randomly replace one of the elements
    ctx->Start();
    while(ctx->NextIteration()) {
        if (stop)
            break;
        int x=rand()%LIST_LEN;
        TestDList* newEntry=TestDListInsertAfter(listEntries[x],501,recycler,heapInfo);
        TestDList* oldEntry=listEntries[x];
        listEntries[x]=newEntry;
        TestDListRemoveEntry(oldEntry,recycler);
        newEntry->data=x;
    }
    ctx->Finish();
    recycler->SetCheckFn(null);
}

#ifdef RECYCLER_PAGE_HEAP
// Basic Page heap mode test
// Assert that the allocated object is on the page boundary
// And that the previous/next page is protected appropriately
void TestPageHeapAlloc(Recycler* recycler, ArenaAllocator* alloc, TestContext *ctx)
{
    uint objectSize = 16;
    char* allocated = recycler->Alloc(objectSize);

    char* guardPage = allocated + objectSize;

    if (g_pageHeapModeType == 1)
    {
        guardPage = allocated - AutoSystemInfo::PageSize;
    }

    Assert(((long) guardPage % AutoSystemInfo::PageSize) == 0);

    MEMORY_BASIC_INFORMATION info = { 0 };
    size_t ret = ::VirtualQuery((LPVOID)guardPage, &info, sizeof(MEMORY_BASIC_INFORMATION));
    Assert(ret != 0);
    Assert(info.Protect == PAGE_NOACCESS);

    printf("Pass");
}
#endif