#include "windows.h"
#include "MemProtectHeap.h"
#include "stdio.h"
#include "stdlib.h"
#include "minmax.h"

class OSHeap
{
public:
    static HANDLE Create()
    {
        return ::HeapCreate(0, 0, 0);
    }
    static void Destroy(HANDLE heap)
    {
        ::HeapDestroy(heap);
    }
    static void * Alloc(HANDLE heap, size_t size)
    {
        return ::HeapAlloc(heap, 0, size);
    }
    static BOOL Free(HANDLE heap, void * address)
    {
        memset(address, 0, ::HeapSize(heap, 0, address));
        return ::HeapFree(heap, 0, address);
    }
};


class MPHeap
{
public:
    static void * Create()
    {
        void * heap = nullptr;
        MemProtectHeapCreate(&heap, MemProtectHeapCreateFlags::MemProtectHeapCreateFlags_ProtectCurrentStack);
        return heap;
    }
    static void Destroy(void * heap)
    {
        MemProtectHeapDestroy(heap);
    }
    static void * Alloc(void * heap, size_t size)
    {
        return MemProtectHeapRootAlloc(heap, size);
    }
    static BOOL Free(void * heap, void * address)
    {
        return SUCCEEDED(MemProtectHeapRootFree(heap, address));
    }

};


class Timer
{
public:
    static void Init()
    {
        QueryPerformanceFrequency((LARGE_INTEGER *) &freq);
    }
    static __int64 freq;

    Timer() { start(); }

    void start()
    {
        QueryPerformanceCounter((LARGE_INTEGER *) &startTime);
    }

    // in ms
    double elapsed()
    {
        __int64 now;
        QueryPerformanceCounter((LARGE_INTEGER *) &now);
        return (now - startTime) * (double)1000.0/(double)freq;
    }
    
    __int64 startTime;
};

__int64 Timer::freq;

template <typename Heap>
class Tests
{
public:    
    
    static const int numInterations = 128 * 1024;
    static const int allocFreeInterations = 1 * 1024 * 1024;
    static const int randomNumIteration = 128 * 1024;    
    static const int randomAllocFreeNumIteration = 2 * 1024 * 1024;
    static const int maxRandomSize = 9216;  // TODO: larger objects
    template <size_t size>
    static double AllocTest() 
    {
        const int numAllocation = numInterations / (size > 4096 ? 2 : 1);
        auto heap = Heap::Create();                
        Timer t;                
        for (int i = 0; i < numAllocation; i++)
        {
            Heap::Alloc(heap, size);
        }
        double result = t.elapsed();
        Heap::Destroy(heap);
        return result / (double)numAllocation;
    }

    template <size_t size>
    static double FreeTest()
    {
        const int numAllocation = numInterations / (size > 4096 ? 2 : 1);;
        auto heap = Heap::Create();        
       
        void ** allocations = (void **)malloc(sizeof(void *) * numAllocation);
        for (int i = 0; i < numAllocation; i++)
        {
            allocations[i] = Heap::Alloc(heap, size);
        }

        Timer t;
        for (int i = 0; i < numAllocation; i++)
        {
            Heap::Free(heap, allocations[i]);
        }
        
        double result = t.elapsed();
        Heap::Destroy(heap);
        free(allocations);
        return result / (double)numAllocation;
    }

    template <size_t size>
    static double AllocFreeTest()
    {
        const int numAllocation = allocFreeInterations;
        auto heap = Heap::Create();        
        Timer t;        
        for (int i = 0; i < numAllocation; i++)
        {
            Heap::Free(heap, Heap::Alloc(heap, size));
        }
        double result = t.elapsed();
        Heap::Destroy(heap);
        return result / (double)numAllocation;
    }

    template <size_t size>
    static void Prime(void * heap, const int numAllocation)
    {
        void ** allocations = (void **)malloc(sizeof(void *) * numAllocation);
        for (int i = 0; i < numAllocation; i++)
        {
            allocations[i] = Heap::Alloc(heap, size);
        }

        for (int i = 0; i < numAllocation; i++)
        {
            Heap::Free(heap, allocations[i]);
        }
        free(allocations);
    }
    template <size_t size>
    static double PrimedAllocTest()
    {
        const int numAllocation = numInterations / (size > 4096 ? 2 : 1);;
        auto heap = Heap::Create();        
        Prime<size>(heap, numAllocation);
        Timer t;
        for (int i = 0; i < numAllocation; i++)
        {
            Heap::Alloc(heap, size);
        }
        double result = t.elapsed();
        Heap::Destroy(heap);
        
        return result / (double)numAllocation;
    }
    
    template <size_t size>
    static double PrimedAllocFreeTest()
    {
        const int numAllocation = allocFreeInterations;
        auto heap = Heap::Create();        
        Prime<size>(heap, numInterations / (size > 4096 ? 2 : 1));
        Timer t;
        for (int i = 0; i < numAllocation; i++)
        {
            Heap::Free(heap, Heap::Alloc(heap, size));
        }
        double result = t.elapsed();
        Heap::Destroy(heap);
        return result / (double)numAllocation;
    }

    static double RandomAllocTest()
    {
        const int numAllocation = randomNumIteration;
        auto heap = Heap::Create();
        srand(0);
        size_t totalSize = 0;
        int iterations = 0;
        
        size_t * allocSize = (size_t *)malloc(sizeof(size_t) * numAllocation);
        do
        {
            size_t size = (size_t)((double)rand() / (double)RAND_MAX * (maxRandomSize - 1)) + 1;
            allocSize[iterations++] = size;
            totalSize += size;
        } while (iterations < numAllocation);

        Timer t;
        for (int i = 0; i < iterations; i++)
        {
            Heap::Alloc(heap, allocSize[i]);
        }
        double result = t.elapsed();
        Heap::Destroy(heap);
        free(allocSize);
        return result / (double)numAllocation;
    }

    static double RandomFreeTest()
    {
        const int numAllocation = randomNumIteration;
        auto heap = Heap::Create();
        srand(0);
        size_t totalSize = 0;
        int iterations = 0;
        void* * allocations = (void **)malloc(sizeof(void*) * numAllocation);
        do
        {
            size_t size = (size_t)((double)rand() / (double)RAND_MAX * (maxRandomSize - 1)) + 1;
            allocations[iterations++] = Heap::Alloc(heap, size);
            totalSize += size;
        } while (iterations < numAllocation);

        Timer t;
        for (int i = 0; i < iterations; i++)
        {
            Heap::Free(heap, allocations[i]);
        }
    
        double result = t.elapsed();
        Heap::Destroy(heap);
        free(allocations);
        return result / (double)numAllocation;
    }

    static double RandomAllocFreeTest()
    {
        const int maxSize = 1024;
        const int minSize = 1;
        const int numAllocation = randomAllocFreeNumIteration;
        auto heap = Heap::Create();
        srand(0);        
        int iterations = 0;
        int * allocSize = (int *)malloc(sizeof(size_t) * numAllocation);
        size_t const sizeRange = maxSize - minSize;
        do
        {
            int size = (int)((double)rand() / (double)RAND_MAX * sizeRange * 2.0) - sizeRange;
            allocSize[iterations++] = (size > 0 ? size += minSize : size -= minSize);
        } while (iterations < numAllocation);

        void ** allocations = (void **)malloc(sizeof(void *) * numAllocation);
        int currentAllocation = 0;
        Timer t;
        for (int i = 0; i < iterations; i++)
        {
            int currentSize = allocSize[i];
            if (currentSize <= 0)
            {
                if (currentAllocation != 0)
                {
                    int freeIndex = -currentSize % currentAllocation;
                    Heap::Free(heap, allocations[freeIndex]);
                    if (freeIndex != --currentAllocation)
                    {
                        allocations[freeIndex] = allocations[currentAllocation];
                    }
                    continue;
                }
                currentSize = currentSize == 0? 1 : -currentSize;
            }
            allocations[currentAllocation++] = Heap::Alloc(heap, currentSize);           
        }
        double result = t.elapsed();
        Heap::Destroy(heap);
        free(allocSize);
        return result / (double)numAllocation;
    }   
};
struct ThreadData
{
    double(*function)();
    HANDLE doneInit;
    double result;
};
HANDLE startTest;
HANDLE doneInit1;
HANDLE doneInit2;

DWORD ThreadProc(LPVOID lpParameter)
{
    ThreadData * data = (ThreadData *)lpParameter;
    SetEvent(data->doneInit);
    WaitForSingleObject(startTest, INFINITE);
    data->result = data->function();
    return 0;
}

double ThreadTest(double(*function)())
{
    ThreadData data1, data2;
    data1.function = function;
    data1.doneInit = doneInit1;
    data2.function = function;
    data2.doneInit = doneInit2;
    ResetEvent(startTest);
    ResetEvent(doneInit1);
    ResetEvent(doneInit2);

    HANDLE thread1 = CreateThread(NULL, 0, &ThreadProc, &data1, 0, NULL);
    HANDLE thread2 = CreateThread(NULL, 0, &ThreadProc, &data2, 0, NULL);

    WaitForSingleObject(doneInit1, INFINITE);
    WaitForSingleObject(doneInit2, INFINITE);
    SetEvent(startTest);
    WaitForSingleObject(thread1, INFINITE);
    WaitForSingleObject(thread2, INFINITE);

    CloseHandle(thread1);
    CloseHandle(thread2);
    return (data1.result + data2.result) / 2;

}

#define TIME_PER_N_ALLOCATION 100000
#define RUNTEST(name, func) \
    { \
        double r1 = Tests<OSHeap>::func(); \
        double r2 = Tests<MPHeap>::func(); \
        double r3 = ThreadTest(&Tests<OSHeap>::func); \
        double r4 = ThreadTest(&Tests<MPHeap>::func); \
        printf("%-30s %6.2f %6.2f %8.2f%% | %6.2f %6.2f %8.2f%%\n", name, \
            r1 * TIME_PER_N_ALLOCATION, r2 * TIME_PER_N_ALLOCATION, (r1 - r2) / r1 * 100,   \
            r3 * TIME_PER_N_ALLOCATION, r4 * TIME_PER_N_ALLOCATION, (r3 - r4) / r3 * 100);  \
    }

int __cdecl wmain(int argc, wchar_t * argv[])
{
    startTest = CreateEvent(NULL, true, FALSE, NULL);
    doneInit1 = CreateEvent(NULL, true, FALSE, NULL);
    doneInit2 = CreateEvent(NULL, true, FALSE, NULL);

    // DebugBreak();
    Timer::Init();

    RUNTEST("Alloc8", AllocTest<8>);
    RUNTEST("Alloc16", AllocTest<16>);
    RUNTEST("Alloc32", AllocTest<32>);
    RUNTEST("Alloc64", AllocTest<64>);
    RUNTEST("Alloc128", AllocTest<128>);
    RUNTEST("Alloc256", AllocTest<256>);
    RUNTEST("Alloc512", AllocTest<512>);
    RUNTEST("Alloc768", AllocTest<768>);
    RUNTEST("Alloc1024", AllocTest<1024>);
    RUNTEST("Alloc2048", AllocTest<2048>);
    RUNTEST("Alloc4096", AllocTest<4096>);
    RUNTEST("Alloc8192", AllocTest<8192>);

    printf("=======================================================\n");
    RUNTEST("Free8", FreeTest<8>);
    RUNTEST("Free16", FreeTest<16>);
    RUNTEST("Free32", FreeTest<32>);
    RUNTEST("Free64", FreeTest<64>);
    RUNTEST("Free128", FreeTest<128>);
    RUNTEST("Free256", FreeTest<256>);
    RUNTEST("Free512", FreeTest<512>);
    RUNTEST("Free768", FreeTest<768>);
    RUNTEST("Free1024", FreeTest<1024>);
    RUNTEST("Free2048", FreeTest<2048>);
    RUNTEST("Free4096", FreeTest<4096>);
    RUNTEST("Free8192", FreeTest<8192>);

    printf("=======================================================\n");
    RUNTEST("AllocFree8", AllocFreeTest<8>);
    RUNTEST("AllocFree16", AllocFreeTest<16>);
    RUNTEST("AllocFree32", AllocFreeTest<32>);
    RUNTEST("AllocFree64", AllocFreeTest<64>);
    RUNTEST("AllocFree128", AllocFreeTest<128>);
    RUNTEST("AllocFree256", AllocFreeTest<256>);
    RUNTEST("AllocFree512", AllocFreeTest<512>);
    RUNTEST("AllocFree768", AllocFreeTest<768>);
    RUNTEST("AllocFree1024", AllocFreeTest<1024>);
    RUNTEST("AllocFree2048", AllocFreeTest<2048>);
    RUNTEST("AllocFree4096", AllocFreeTest<4096>);
    RUNTEST("AllocFree8192", AllocFreeTest<8192>);

    printf("=======================================================\n");
    RUNTEST("PrimedAlloc8", PrimedAllocTest<8>);
    RUNTEST("PrimedAlloc16", PrimedAllocTest<16>);
    RUNTEST("PrimedAlloc32", PrimedAllocTest<32>);
    RUNTEST("PrimedAlloc64", PrimedAllocTest<64>);
    RUNTEST("PrimedAlloc128", PrimedAllocTest<128>);
    RUNTEST("PrimedAlloc256", PrimedAllocTest<256>);
    RUNTEST("PrimedAlloc512", PrimedAllocTest<512>);
    RUNTEST("PrimedAlloc768", PrimedAllocTest<768>);
    RUNTEST("PrimedAlloc1024", PrimedAllocTest<1024>);
    RUNTEST("PrimedAlloc2048", PrimedAllocTest<2048>);
    RUNTEST("PrimedAlloc4096", PrimedAllocTest<4096>);
    RUNTEST("PrimedAlloc8192", PrimedAllocTest<8192>);

    printf("=======================================================\n");
    RUNTEST("PrimedAllocFree8", PrimedAllocFreeTest<8>);
    RUNTEST("PrimedAllocFree16", PrimedAllocFreeTest<16>);
    RUNTEST("PrimedAllocFree32", PrimedAllocFreeTest<32>);
    RUNTEST("PrimedAllocFree64", PrimedAllocFreeTest<64>);
    RUNTEST("PrimedAllocFree128", PrimedAllocFreeTest<128>);
    RUNTEST("PrimedAllocFree256", PrimedAllocFreeTest<256>);
    RUNTEST("PrimedAllocFree512", PrimedAllocFreeTest<512>);
    RUNTEST("PrimedAllocFree768", PrimedAllocFreeTest<768>);
    RUNTEST("PrimedAllocFree1024", PrimedAllocFreeTest<1024>);
    RUNTEST("PrimedAllocFree2048", PrimedAllocFreeTest<2048>);
    RUNTEST("PrimedAllocFree4096", PrimedAllocFreeTest<4096>);
    RUNTEST("PrimedAllocFree8192", PrimedAllocFreeTest<8192>);

    printf("=======================================================\n");
    RUNTEST("RandomAllocTest", RandomAllocTest);
    RUNTEST("RandomFreeTest", RandomFreeTest);
    RUNTEST("RandomAllocFreeTest", RandomAllocFreeTest);    
}

// This is consumed by AutoSystemInfo. AutoSystemInfo is in Chakra.Common.Core.lib, which is linked
// into multiple DLLs. The hosting DLL provides the implementation of this function.
bool GetDeviceFamilyInfo(
    _Out_opt_ ULONGLONG* /*pullUAPInfo*/,
    _Out_opt_ ULONG* /*pulDeviceFamily*/,
    _Out_opt_ ULONG* /*pulDeviceForm*/)
{
    return false;
}