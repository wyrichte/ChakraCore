/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "GCHost.h"
#include "MemProtectHeap.h"

//////////////////// Test Harness ////////////////////

bool verbose = false;

#define DEFINE_TEST(name)                          \
    void name();                                   \
    TestEntry name ## _entry(_u(#name), &name);

#define Verify(expr) (LogVerify((expr), #expr, __FILE__, __LINE__))
int failures = 0;

template <class T>
T LogVerify(T value, const char * expr, const char * file, int line)
{
    if (!value)
    {
        ++failures;
        wprintf(_u("==== FAILURE: '%S' evaluated to false. %S(%d)\n"), expr, file, line);
    }
    else if (verbose)
    {
        wprintf(_u("==== verified: '%S'. %S(%d)\n"), expr, file, line);
    }
    return value;
}

struct TestEntry 
{
    const WCHAR* name;
    void (*proc)();
    const TestEntry* next;

    static const TestEntry* first;
    static const TestEntry** last;

    TestEntry(const WCHAR* name, void (*proc)()) : name(name), proc(proc)
    {
        this->Install();
    }

    void Install()
    {
        next = *last;
        *last = this;
        last = &next;
    }

    void Run() const
    {
        wprintf(_u("==== begin : %s\n"), name);
        proc();
        wprintf(_u("==== end : %s\n"), name);
    }

    template <class Fn>
    static void ForAll(Fn fn)
    {
        for (const TestEntry* iter = first; iter; iter = iter->next)
        {
            fn(*iter);
        }
    }
};
const TestEntry* TestEntry::first = nullptr;
const TestEntry** TestEntry::last = &TestEntry::first;

void RunAllTests()
{
    wprintf(_u("RunAllTests()\n"));

    TestEntry::ForAll([](const TestEntry& te)
    {
        te.Run();
    });
    wprintf(_u("Finished RunAllTests()\n"));
}

//////////////////// End test harness ////////////////////

MemProtectHeapCreateFlags memProtectHeapCreateFlags = MemProtectHeapCreateFlags_ProtectCurrentStack;

//////////////////// Begin test implementations ////////////////////

DEFINE_TEST(SimpleRecyclerTest)
void SimpleRecyclerTest()
{
    PageAllocator::BackgroundPageQueue backgroundPageQueue;
    IdleDecommitPageAllocator pageAllocator(nullptr, 
        PageAllocatorType::PageAllocatorType_Thread,
        Js::Configuration::Global.flags,
        0 /* maxFreePageCount */, PageAllocator::DefaultMaxFreePageCount /* maxIdleFreePageCount */,
        false /* zero pages */, &backgroundPageQueue);

    try
    {
#ifdef EXCEPTION_CHECK
        // REVIEW: Do we need a stack probe here? We don't care about OOM's since we deal with that below
        AUTO_NESTED_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);
#endif

        Recycler* recycler = HeapNew(Recycler, nullptr, &pageAllocator, Js::Throw::OutOfMemory, Js::Configuration::Global.flags);

        recycler->Initialize(false /* forceInThread */, nullptr /* threadService */);

        int* aInt = RecyclerNewArray(recycler, int, 30);

        for (int i = 0; i < 30; i++)
        {
            aInt[i] = i;
        }

        recycler->CollectNow<CollectNowForceInThread>();

        for (int i = 0; i < 30; i++)
        {
            Verify(i == aInt[i]);
        }
    }
    catch (Js::OutOfMemoryException)
    {
        printf("Error: OOM\n");
    }
}

template <bool leaf>
void MemProtectFillAndClearArray(void* gcHandle, int** allocs, const int k_allocCount, const int k_allocSize)
{
    HRESULT hr = S_FALSE;

    for (int i = 0; i < k_allocCount; i++)
    {
        int* allocation = nullptr;

        if (leaf)
        {
            allocation = (int*)MemProtectHeapRootAllocLeaf(gcHandle, k_allocSize);
        }
        else
        {
            allocation = (int*)MemProtectHeapRootAlloc(gcHandle, k_allocSize);
        }

        allocs[i] = allocation;
        if (allocation != nullptr)
        {
            *allocation = i;
        }
    }

    MemProtectHeapCollect(gcHandle, MemProtectHeapCollectFlags::MemProtectHeap_CollectForce);

    for (int i = 0; i < k_allocCount; i++)
    {
        int* allocation = allocs[i];
        if (allocation != nullptr)
        {
            Verify(*allocation == i);
            hr = MemProtectHeapUnrootAndZero(gcHandle, allocation);
            Verify(SUCCEEDED(hr));
            allocs[i] = nullptr;
        }
    }
}

DEFINE_TEST(MemProtectHeap_Simple)
void MemProtectHeap_Simple()
{
    void* gcHandle = nullptr;
    HRESULT hr = ::MemProtectHeapCreate(&gcHandle, memProtectHeapCreateFlags);
    Verify(SUCCEEDED(hr));
    void* p = ::MemProtectHeapRootAlloc(gcHandle, 4);
    Verify(p);
    hr = ::MemProtectHeapUnrootAndZero(gcHandle, p);
    Verify(SUCCEEDED(hr));
    hr = ::MemProtectHeapDestroy(gcHandle);
    Verify(SUCCEEDED(hr));
}

DEFINE_TEST(MemProtectHeap_AllSizes)
void MemProtectHeap_AllSizes()
{
    void* gcHandle = nullptr;
    HRESULT hr = ::MemProtectHeapCreate(&gcHandle, memProtectHeapCreateFlags);

    Verify(SUCCEEDED(hr));

    if (SUCCEEDED(hr))
    {
        const int k_allocCount = 50;
        int* allocs[k_allocCount];

        printf("Testing non-leaf allocations\n");
        MemProtectFillAndClearArray<false>(gcHandle, allocs, k_allocCount, sizeof(int)); // Small Allocs
        MemProtectFillAndClearArray<false>(gcHandle, allocs, k_allocCount, 2048);        // Medium Allocs
        MemProtectFillAndClearArray<false>(gcHandle, allocs, k_allocCount, 32 * 1024);   // Large Allocs

        printf("Testing leaf allocations\n");
        MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, sizeof(int)); // Small Allocs
        MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, 2048);        // Medium Allocs
        MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, 32 * 1024);   // Large Allocs

        hr = MemProtectHeapDestroy(gcHandle);
        Verify(SUCCEEDED(hr));
        printf("Success!\n");
    }
}

DEFINE_TEST(MemProtectHeap_StackUAF)
void MemProtectHeap_StackUAF()
{
    void* gcHandle = nullptr;
    void* volatile p;
    void* volatile q;
    
    Verify(SUCCEEDED(::MemProtectHeapCreate(&gcHandle, memProtectHeapCreateFlags)));

    // Root section is used to mock a protected stack.
    MemProtectHeapRootSection rs = { (void**)&q, sizeof(q) };
    ::MemProtectHeapAddRootSection(gcHandle, &rs);

    q = p = ::MemProtectHeapRootAlloc(gcHandle, 4);
    Verify(p);
    Verify(SUCCEEDED(::MemProtectHeapUnrootAndZero(gcHandle, p)));

    // UAF of stack pointer -- verify still allocated, even though unrooted
    Verify(SUCCEEDED(::MemProtectHeapCollect(gcHandle, MemProtectHeapCollectFlags::MemProtectHeap_CollectForceFull)));
    // NOTE: do not disable this verification. It indicates a failure of the security mitigation.
    Verify(::MemProtectHeapIsValidObject(gcHandle, q));
    
    // once unreachable, the memory should be collectable. Simulate overwrite by obfuscating the pointer, letting 
    // us retrieve it after GC to perform the verification.
    q = nullptr;    
    Verify(SUCCEEDED(::MemProtectHeapCollect(gcHandle, MemProtectHeapCollectFlags::MemProtectHeap_CollectForceFull)));
    // NOTE: the following verification could fail due to false positives, but it should be incredibly unlikely.
    // If you confirm a false positive, disable THIS VERIFICATION ONLY.
    Verify(!::MemProtectHeapIsValidObject(gcHandle, p));


    Verify(SUCCEEDED(::MemProtectHeapDestroy(gcHandle)));
}

DEFINE_TEST(MemProtectHeap_HeapUAF)
void MemProtectHeap_HeapUAF()
{
    struct Obj
    {
        Obj* volatile field;
    };

    void* gcHandle = nullptr;
    Obj* obj1;
    Obj* obj2;

    Verify(SUCCEEDED(::MemProtectHeapCreate(&gcHandle, memProtectHeapCreateFlags)));
    
    obj1 = (Obj*)::MemProtectHeapRootAlloc(gcHandle, sizeof(Obj));
    obj2 = (Obj*)::MemProtectHeapRootAlloc(gcHandle, sizeof(Obj));
    Verify(obj1);
    Verify(obj2);
    obj1->field = obj2;
    ::MemProtectHeapUnrootAndZero(gcHandle, obj2);
   
    // UAF of heap pointer -- verify still allocated, even though unrooted
    Verify(SUCCEEDED(::MemProtectHeapCollect(gcHandle, MemProtectHeapCollectFlags::MemProtectHeap_CollectForceFull)));
    // NOTE: do not disable this verification. It indicates a failure of the security mitigation.
    Verify(::MemProtectHeapIsValidObject(gcHandle, obj1->field));
    
    // Once unreachable, the memory should be collectable. 
    obj1->field = nullptr;    
    Verify(SUCCEEDED(::MemProtectHeapCollect(gcHandle, MemProtectHeapCollectFlags::MemProtectHeap_CollectForceFull)));
    // NOTE: the following verification could fail due to false positives, but it should be incredibly unlikely.
    // If you confirm a false positive, disable THIS VERIFICATION ONLY.
    Verify(!::MemProtectHeapIsValidObject(gcHandle, obj2));

    ::MemProtectHeapUnrootAndZero(gcHandle, obj1);

    Verify(SUCCEEDED(::MemProtectHeapDestroy(gcHandle)));
}

// This thread barrier is aware of safe points, so can be woken by the GC via APC.
class ThreadBarrier
{
public:
    ThreadBarrier(LONG totalThreads)
    : totalThreads(totalThreads), currentlyWaiting(0)
    {
        Verify(active = ::CreateEvent(NULL, TRUE, FALSE, NULL));
        Verify(inactive = ::CreateEvent(NULL, TRUE, FALSE, NULL));
    }
    ~ThreadBarrier()
    {
        ::CloseHandle(inactive);
        ::CloseHandle(active);
    }

    class Wake
    {
    public:
        Wake(void* gcHandle) : gcHandle(gcHandle) 
        {
            Verify(::DuplicateHandle(
                ::GetCurrentProcess(), 
                ::GetCurrentThread(),
                ::GetCurrentProcess(),
                &thread,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS));
        }
        ~Wake()
        {
            ::CloseHandle(thread);
        }

        static void __stdcall Invoke(void* pthis)
        {
            Verify(::QueueUserAPC(&Wake::DoWake, static_cast<Wake*>(pthis)->thread, reinterpret_cast<ULONG_PTR>(pthis)));
        }

        static VOID CALLBACK DoWake(ULONG_PTR pthis)
        {
            ::MemProtectHeapSynchronizeWithCollector(reinterpret_cast<Wake*>(pthis)->gcHandle);
        }

    private:
        Wake(const Wake&) = delete;

        HANDLE thread;
        void* gcHandle;
    };

    void Join()
    {
        HANDLE currentActive = active;
        if (InterlockedIncrement(&currentlyWaiting) == totalThreads) 
        {
            // all other threads are waiting on 'active', allowing us to set up the next
            // join safely before we permit the other threads progress.
            currentlyWaiting = 0;    
            ::ResetEvent(inactive);
            active = inactive; inactive = currentActive;
            ::SetEvent(currentActive);
        }
        else 
        {
            for (;;)
            {
                DWORD result = ::WaitForMultipleObjectsEx(1, &currentActive, FALSE, INFINITE, TRUE);
                if (result == WAIT_IO_COMPLETION) 
                {
                    continue;
                }
                else if (result == WAIT_OBJECT_0)
                {
                    break;
                }
                else
                {
                    Verify(0);
                }
            }
        }
    }

private:
    ThreadBarrier(const ThreadBarrier&) = delete;


    volatile LONG totalThreads;
    volatile LONG currentlyWaiting;
    HANDLE active;
    HANDLE inactive;
};

template <class Proc>
HANDLE RunOnThread(Proc proc)
{
    auto threadStart = [](LPVOID lpParameter) -> DWORD
    {
        Proc* proc = (Proc*)lpParameter;
        (*proc)();
        delete proc;
        return 0;
    };
    Proc* p = new Proc(proc);
    HANDLE thread = ::CreateThread(NULL, 0, threadStart, p, 0, NULL);
    if (!Verify(thread)) 
    {
        return NULL;
    }
    return thread;
}

DEFINE_TEST(MemProtectHeap_MultiThreaded_NoProtection)
void MemProtectHeap_MultiThreaded_NoProtection()
{
    void* gcHandle = nullptr;
    Verify(SUCCEEDED(::MemProtectHeapCreate(&gcHandle, memProtectHeapCreateFlags)));
    
    HANDLE threads[2];
    ThreadBarrier barrier(_countof(threads));
    for (int i = 0; i < _countof(threads); ++i)
    {
        threads[i] = RunOnThread([&]
        {
            const int k_allocCount = 50;
            int* allocs[k_allocCount];

            barrier.Join();
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, sizeof(int)); // Small Allocs

            barrier.Join();
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, 2048);        // Medium Allocs

            barrier.Join();
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, 32 * 1024);   // Large Allocs
        });
    }
    Verify(WAIT_OBJECT_0 == ::WaitForMultipleObjects(_countof(threads), threads, TRUE, INFINITE));

    Verify(SUCCEEDED(::MemProtectHeapDestroy(gcHandle)));
}

DEFINE_TEST(MemProtectHeap_MultiThreaded_OneProtected)
void MemProtectHeap_MultiThreaded_OneProtected()
{
    void* gcHandle = nullptr;
    Verify(SUCCEEDED(::MemProtectHeapCreate(&gcHandle, memProtectHeapCreateFlags)));
    
    HANDLE threads[2];
    ThreadBarrier barrier(_countof(threads));
    for (int i = 0; i < _countof(threads); ++i)
    {
        threads[i] = RunOnThread([&]
        {
            ThreadBarrier::Wake wake(gcHandle);

            if (i == 0)
            {
                Verify(SUCCEEDED(::MemProtectHeapProtectCurrentThread(gcHandle, ThreadBarrier::Wake::Invoke, &wake)));
            }

            const int k_allocCount = 50;
            int* allocs[k_allocCount];

            barrier.Join();
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, sizeof(int)); // Small Allocs

            barrier.Join();            
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, 2048);        // Medium Allocs

            barrier.Join();
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, 32 * 1024);   // Large Allocs

            if (i == 0)
            {
                Verify(SUCCEEDED(::MemProtectHeapUnprotectCurrentThread(gcHandle)));
            }
        });
    }
    Verify(WAIT_OBJECT_0 == ::WaitForMultipleObjects(_countof(threads), threads, TRUE, INFINITE));

    Verify(SUCCEEDED(::MemProtectHeapDestroy(gcHandle)));
}

DEFINE_TEST(MemProtectHeap_MultiThreaded_AllProtected)
void MemProtectHeap_MultiThreaded_AllProtected()
{
    void* gcHandle = nullptr;
    Verify(SUCCEEDED(::MemProtectHeapCreate(&gcHandle, memProtectHeapCreateFlags)));
    
    HANDLE threads[2];
    ThreadBarrier barrier(_countof(threads));
    for (int i = 0; i < _countof(threads); ++i)
    {
        threads[i] = RunOnThread([&]
        {
            ThreadBarrier::Wake wake(gcHandle);

            Verify(SUCCEEDED(::MemProtectHeapProtectCurrentThread(gcHandle, ThreadBarrier::Wake::Invoke, &wake)));
            
            const int k_allocCount = 50;
            int* allocs[k_allocCount];

            barrier.Join();
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, sizeof(int)); // Small Allocs

            barrier.Join();            
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, 2048);        // Medium Allocs

            barrier.Join();
            MemProtectFillAndClearArray<true>(gcHandle, allocs, k_allocCount, 32 * 1024);   // Large Allocs

            Verify(SUCCEEDED(::MemProtectHeapUnprotectCurrentThread(gcHandle)));
        });
    }
    Verify(WAIT_OBJECT_0 == ::WaitForMultipleObjects(_countof(threads), threads, TRUE, INFINITE));

    Verify(SUCCEEDED(::MemProtectHeapDestroy(gcHandle)));
}

//////////////////// End test implementations ////////////////////

//////////////////// Begin test stubs ////////////////////

// This is consumed by AutoSystemInfo. AutoSystemInfo is in Chakra.Common.Core.lib, which is linked
// into multiple DLLs. The hosting DLL provides the implementation of this function.
bool GetDeviceFamilyInfo(
    _Out_opt_ ULONGLONG* /*pullUAPInfo*/,
    _Out_opt_ ULONG* /*pulDeviceFamily*/,
    _Out_opt_ ULONG* /*pulDeviceForm*/)
{
    return false;
}

#if DBG && defined(RECYCLER_VERIFY_MARK)
bool IsLikelyRuntimeFalseReference(char* objectStartAddress, size_t offset,
    const char* typeName)
{
    return false;
}
#endif

//////////////////// End test stubs ////////////////////

//////////////////// Begin program entrypoint ////////////////////

void usage(const WCHAR* self)
{
    wprintf(
        _u("usage: %s [-?|-v|-s|-S]* <test filter>* [-js <jscript options from here on>]\n")
        _u("  -v\n\tverbose logging\n")
        _u("  -s|--suspend_mode\n\tMemProtectHeap in suspend threads mode\n")
        _u("  -S|--coop_mode\n\tMemProtectHeap in cooperative safepoint mode\n"),
        self);
}

int __cdecl wmain(int argc, __in_ecount(argc) WCHAR* argv[])
{
    MemProtectHeapProcessAttach();
    


    const WCHAR** testFilter = (const WCHAR**)calloc(argc, sizeof(void*));
    size_t testFilterCount = 0;
    bool suspendMode = false;
    bool coopMode = false;
    int jscriptOptions = 0;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (wcscmp(argv[i], _u("-?")) == 0)
            {
                usage(argv[0]);
                exit(1);
            }
            else if (wcscmp(argv[i], _u("-v")) == 0)
            {
                verbose = true;
            }
            else if (wcscmp(argv[i], _u("-s")) == 0 || wcscmp(argv[i], _u("-suspend_mode")) == 0)
            {
                suspendMode = true;
            }
            else if (wcscmp(argv[i], _u("-S")) == 0 || wcscmp(argv[i], _u("-coop_mode")) == 0)
            {
                coopMode = true;
            }
            else if (wcscmp(argv[i], _u("-js")) == 0 || wcscmp(argv[i], _u("-JS")) == 0)
            {
                jscriptOptions = i;
                break;
            }
            else 
            {
                wprintf(_u("unknown argument '%s'\n"), argv[i]);
                exit(1);
            }
        }
        else
        {
            testFilter[testFilterCount++] = argv[i];
        }
    }

#if DBG && defined(INTERNAL_MEM_PROTECT_HEAP_CMDLINE)
    // Parse the rest of the command line as js options
    if (jscriptOptions)
    {
        MemProtectHeapParseCmdLineConfig(argc - jscriptOptions, argv + jscriptOptions);
    }
#endif

    int testsCompleted = 0;
    auto runTests = [&]
    {
        TestEntry::ForAll([&](const TestEntry& te)
        {
            if (testFilterCount == 0) 
            {
                te.Run();
                ++testsCompleted;
                return;
            }

            for (size_t i = 0; i < testFilterCount; ++i)
            {
                if (wcsstr(te.name, testFilter[i]) != nullptr)
                {
                    // substring match. run test.
                    te.Run();
                    ++testsCompleted;
                    return;
                }
            }
        });
    };

    if (!suspendMode && !coopMode)
    {
        // by default, run both sets of tests
        suspendMode = coopMode = true;
    }
    if (suspendMode)
    {
        wprintf(_u("==== Suspend mode.\n"));
        memProtectHeapCreateFlags = MemProtectHeapCreateFlags_ProtectMultipleStacksSuspend;
        runTests();
    }
    if (coopMode)
    {
        wprintf(_u("==== Coop mode.\n"));
        memProtectHeapCreateFlags = MemProtectHeapCreateFlags_ProtectMultipleStacksCooperative;
        runTests();
    }

    if (testFilterCount != 0 && testsCompleted == 0)
    {
        wprintf(_u("==== no tests found matching patterns.\n"));
        failures = 1;
    }

    free(testFilter);

    MemProtectHeapProcessDetach();

    if (failures)
    {
        wprintf(_u("==== FAILURES OCCURRED\n"));
        return 1;
    }
    else if (testsCompleted)
    {
        wprintf(_u("==== SUCCESS (%d tests run)\n"), testsCompleted);
    }

    return 0;
}

//////////////////// End program entrypoint ////////////////////
