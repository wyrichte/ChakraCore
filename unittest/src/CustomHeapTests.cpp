/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"


static void OutOfMemory1()
{
    fprintf(stderr, "ERROR: out of memory\n");
    exit(-1);
}
using namespace CustomHeap;

#ifdef _M_X64
#define GET_PDATA_COUNT(num) (num / num)
#define GET_XDATA_SIZE(num) (num)
#elif defined(_M_ARM) || defined(_M_ARM64)
#define GET_PDATA_COUNT(num) (num)
#define GET_XDATA_SIZE(num) ((num * 3) / 2)
#else
#define GET_PDATA_COUNT(num) (num * 0)
#define GET_XDATA_SIZE(num) (num * 0)
#endif

UTEST_GROUP(CustomHeapTests)
{
    PageAllocator * pageAllocator;
    ArenaAllocator* arena;
    bool allocXdata;

    UTEST_CASE(Alloc_Basic)
    {
        Heap heap(NULL, arena, allocXdata);

        for(int i = 100; i < 2000000; i += 100)
        {
            bool isAllJITCodeInPreReservedRegion;
            Allocation* allocation = heap.Alloc(i, GET_PDATA_COUNT(3), GET_XDATA_SIZE(60), false, false, &isAllJITCodeInPreReservedRegion);
#if DBG
            allocation->isAllocationUsed = true;
#endif
            VerifyAllocation(allocation, i, GET_PDATA_COUNT(3), GET_XDATA_SIZE(60));
            Free(&heap, allocation);
            VerifyFreed(allocation);
        }
    }

    UTEST_CASE(Alloc_Small_Functions)
    {
        Heap heap(NULL, arena, allocXdata);
        uint count = 1000;
        Allocation** allocations = new Allocation*[count];
        for(uint i = 8; i < count; i += 1)
        {
            bool isAllJITCodeInPreReservedRegion;
            allocations[i] = heap.Alloc(i, GET_PDATA_COUNT(5), GET_XDATA_SIZE(45), false, false, &isAllJITCodeInPreReservedRegion);
#if DBG
            allocations[i]->isAllocationUsed = true;
#endif
            VerifyAllocation(allocations[i], i, GET_PDATA_COUNT(5), GET_XDATA_SIZE(45));
        }
        for(uint i = 8; i < count; i += 1)
        {
            Free(&heap, allocations[i]);
            VerifyFreed(allocations[i]);
        }
        delete[] allocations;
    }

    UTEST_CASE(Alloc_Free_Alloc_Small_Functions)
    {
        Heap heap(NULL, arena, allocXdata);
        uint count = 160;
        Allocation** allocations = new Allocation*[count];
        for(int j = 0; j < 400; j++)
        {
            for(uint i = 8; i < count; i += 1)
            {
                bool isAllJITCodeInPreReservedRegion;
                allocations[i] = heap.Alloc(i, GET_PDATA_COUNT(1), GET_XDATA_SIZE(20), false, false, &isAllJITCodeInPreReservedRegion);
#if DBG
                allocations[i]->isAllocationUsed = true;
#endif
                VerifyAllocation(allocations[i], i, GET_PDATA_COUNT(1), GET_XDATA_SIZE(20));
            }
            for(uint i = 8; i < count; i += 1)
            {
                Free(&heap, allocations[i]);
                VerifyFreed(allocations[i]);
            }
        }
        
        delete[] allocations;
    }

    UTEST_CASE(Alloc_NoFree_Small_Functions)
    {
        Heap heap(NULL, arena, allocXdata);
        uint count = 1000;
        Allocation** allocations = new Allocation*[count];
        for(int j = 0; j < 5; j++)
        {
            for(uint i = 1; i < count; i += 1)
            {
                bool isAllJITCodeInPreReservedRegion;
                allocations[i] = heap.Alloc(i, GET_PDATA_COUNT(8), GET_XDATA_SIZE(20), false, false, &isAllJITCodeInPreReservedRegion);
#if DBG
                allocations[i]->isAllocationUsed = true;
#endif
                VerifyAllocation(allocations[i], i, GET_PDATA_COUNT(8), GET_XDATA_SIZE(20));
            }
        }

        delete[] allocations;
    }

    UTEST_CASE(Alloc_Free_Alloc_Large_Functions)
    {
        Heap heap(NULL, arena, allocXdata);
        uint count = 400;
        Allocation** allocations = new Allocation*[count];
        for(uint j = 0; j < 5; j++)
        {
            for(uint i = 1; i < count / 2; i += 2)
            {
                bool isAllJITCodeInPreReservedRegion;
                allocations[i] = heap.Alloc(i * 100, GET_PDATA_COUNT(2), GET_XDATA_SIZE(60), false, false, &isAllJITCodeInPreReservedRegion);
#if DBG
                allocations[i]->isAllocationUsed = true;
#endif
                VerifyAllocation(allocations[i], i * 100, GET_PDATA_COUNT(2), GET_XDATA_SIZE(60));
                allocations[i + 1] = heap.Alloc(i * 50, GET_PDATA_COUNT(3), GET_XDATA_SIZE(25), false, false, &isAllJITCodeInPreReservedRegion);
#if DBG
                allocations[i + 1]->isAllocationUsed = true;
#endif
                VerifyAllocation(allocations[i+1], i * 50, GET_PDATA_COUNT(3), GET_XDATA_SIZE(25));
            }
            for(uint i = 1; i < count / 2; i += 2)
            {
                heap.Decommit(allocations[i]);
                heap.Decommit(allocations[i+1]);
            }
            for(uint i = 1; i < count / 2; i += 2)
            {
                heap.Free(allocations[i]);
                heap.Free(allocations[i + 1]);
                VerifyFreed(allocations[i]);
                VerifyFreed(allocations[i+1]);
            }
        }
        
        delete[] allocations;
    }

    UTEST_CASE(Alloc_Free_Alloc_Realistic)
    {
        Heap heap(NULL, arena, allocXdata);
        uint sizes[] = { 200, 130, 400, 800, 230, 210, 100, 150, 180, 400, 900, 4096, 8192, 323, 421, 123, 432, 543, 543, 1232, 123, 22, 2345, 2343, 53, 4232, 231, 12, 123 , 214, 2354, 543, 2312 };
        ushort xdataSizes[] = { 20, 13, 40, 55, 0, 21, 0, 28, 18, 40, 50, 40, 72, 32, 42, 12, 43, 54, 54, 12, 12, 22, 23, 23, 53, 42, 23, 12, 12 , 21, 23, 54, 23 };
        ushort pdataCounts[] = { 2, 1, 4, 2, 5, 8, 3, 1, 1, 2, 2, 3, 1, 250, 2, 2, 3, 1, 5, 2, 2, 3, 6, 1, 2, 2, 3, 1, 1, 2, 2, 6, 8 };
        uint count = _countof(sizes);
        
        std::list<Allocation*> allocations;
        for(uint i = 0; i < count; i++)
        {
            bool isAllJITCodeInPreReservedRegion;
            Allocation* allocation = heap.Alloc(sizes[i], GET_PDATA_COUNT(pdataCounts[i]), GET_XDATA_SIZE(xdataSizes[i]), false, false, &isAllJITCodeInPreReservedRegion);
            allocations.push_back(allocation);
#if DBG
            allocation->isAllocationUsed = true;
#endif
            VerifyAllocation(allocation, sizes[i], GET_PDATA_COUNT(pdataCounts[i]), GET_XDATA_SIZE(xdataSizes[i]));
        }
        
        for(auto it = allocations.begin(); it != allocations.end(); it++)
        {
            heap.Decommit(*it);
        }

        for(auto it = allocations.begin(); it != allocations.end(); it++)
        {
            heap.Free(*it);
            VerifyFreed(*it);
        }
    }

    void VerifyAllocation(Allocation* allocation, uint size, ushort pdataCount, ushort xdataSize )
    {
        UT_ASSERT(allocation->page != nullptr);
        UT_ASSERT(allocation->size >= size);
        if(allocation->IsLargeAllocation())
        {
            UT_ASSERT(allocation->largeObjectAllocation.segment != nullptr);
            UT_ASSERT(allocation->largeObjectAllocation.isDecommitted == false);
        }

        MEMORY_BASIC_INFORMATION codeInfo;
        VirtualQuery(allocation->address, &codeInfo, sizeof(codeInfo));
        UT_ASSERT(codeInfo.State == MEM_COMMIT);
#if PDATA_ENABLED
        if(allocXdata && xdataSize > 0)
        {
            UT_ASSERT(allocation->xdata.address != nullptr);
            
            MEMORY_BASIC_INFORMATION xdataInfo;
            VirtualQuery(allocation->xdata.address, &xdataInfo, sizeof(xdataInfo));
            UT_ASSERT(xdataInfo.Protect == PAGE_READWRITE);
            UT_ASSERT(xdataInfo.State == MEM_COMMIT);

            // Validate that the xdata memory is writeable.
            memset(allocation->xdata.address, 0xAC, xdataSize);
#ifdef _M_X64
            ULONG64            imageBase       = 0;
            RUNTIME_FUNCTION  *runtimeFunction = RtlLookupFunctionEntry((ULONG64)allocation->address, &imageBase, nullptr);
            UT_ASSERT(runtimeFunction != nullptr);
            UT_ASSERT(runtimeFunction->BeginAddress == ((ULONG64)allocation->address) - imageBase);
            UT_ASSERT_SZ(allocation->xdata.address >= (BYTE*)(allocation->address + allocation->size), "There should be no overlap in xdata and code allocation");
#endif
           
        }
#if defined(_M_ARM) || defined(_M_ARM64)
        UT_ASSERT(allocation->xdata.pdataCount == pdataCount);
        UT_ASSERT(allocation->xdata.xdataSize == xdataSize);
#endif
#endif
    }

    void VerifyFreed(Allocation* allocation)
    {
#ifdef _M_X64
        ULONG64            imageBase       = 0;
        RUNTIME_FUNCTION  *runtimeFunction = RtlLookupFunctionEntry((ULONG64)allocation->address, &imageBase, nullptr);
        UT_ASSERT(runtimeFunction == nullptr);
#endif
    }


    void Free(Heap* heap, Allocation* allocation)
    {
        // It is requirement to free only executable memory
        UT_ASSERT(heap->ProtectAllocation(allocation, PAGE_EXECUTE, PAGE_EXECUTE));
        heap->Free(allocation);
    }

    void CommonCleanup() override
    {
        if(arena)
        {
            arena->Clear();
            delete arena;
        }

        if (pageAllocator)
        {
            delete pageAllocator;
        }
    }

    void CommonSetup() override
    {
#if PDATA_ENABLED
        allocXdata = true;
#else
        allocXdata = false;
#endif
        pageAllocator = new PageAllocator(NULL, Js::Configuration::Global.flags);
        arena = new ArenaAllocator(L"customHeapArena", pageAllocator, OutOfMemory1);
    }
};
