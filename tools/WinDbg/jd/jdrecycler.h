//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

#ifdef JD_PRIVATE

// include headers that recycler needs
#include "core\api.h"
#include "common\MathUtil.h"
#include "core\CriticalSection.h"
#include "DataStructures\Comparer.h"
#include <math.h>
#include "DataStructures\SizePolicy.h"
#include "core\SysInfo.h"
#include "core\AllocSizeMath.h"

// Include recycler headers
#include "Exceptions\Throw.h"
#include "Memory\AllocationPolicyManager.h"
#include "Memory\Allocator.h"
#include "DataStructures\SList.h"
#include "DataStructures\DList.h"
#include "DataStructures\DoublyLinkedListElement.h"
#include "Memory\VirtualAllocWrapper.h"
#include "Memory\MemoryTracking.h"
#include "Memory\PageAllocator.h"
#include "Memory\IdleDecommitPageAllocator.h"
#include "Memory\FreeObject.h"
#include "Memory\HeapConstants.h"
#include "Memory\HeapBlock.h"
#include "Memory\SmallHeapBlockAllocator.h"
#include "Memory\SmallNormalHeapBlock.h"
#include "Memory\SmallLeafHeapBlock.h"
#include "Memory\SmallFinalizableHeapBlock.h"
#include "Memory\LargeHeapBlock.h"
#include "Memory\HeapBucket.h"
#include "Memory\SmallLeafHeapBucket.h"
#include "Memory\SmallNormalHeapBucket.h"
#include "Memory\SmallFinalizableHeapBucket.h"
#include "Memory\LargeHeapBucket.h"
#include <Memory\heapinfo.h>
#ifndef _M_X64
#define _M_X64  // force x64 so we get HeapBlockMap64
#include "Memory\HeapBlockMap.h"
#undef _M_X64
#else
#include "Memory\HeapBlockMap.h"
#endif

// STL headers
#include <hash_set>
#include <hash_map>
#include <stack>
#include <set>
#include <algorithm>
#include <string>

struct HeapObject
{
    ULONG64 heapBlockType;
    ULONG64 address;
    ushort index;
    ULONG64 heapBlock;
    ULONG64 objectInfoAddress;
    unsigned char objectInfoBits;
    ULONG64 objectSize;
    ULONG64 vtable;
    ushort addressBitIndex;
    ULONG64 freeBitWord;
    ULONG64 markBitWord;
    bool isFreeSet;
    bool isMarkSet;
};

class HeapBlockHelper
{
public:
    HeapBlockHelper(EXT_CLASS_BASE* extension, ExtRemoteTyped recycler)
        : ext(extension), recycler(recycler), objectAllocationShift(0)
    {
    }

    void DumpObjectInfoBits(unsigned char objectInfoBits);
    void DumpSmallHeapBlockObject(ExtRemoteTyped& heapBlock, ULONG64 address, bool verbose);
    void DumpLargeHeapBlockObject(ExtRemoteTyped& heapBlock, ULONG64 address, bool verbose);
    void DumpHeapObject(const HeapObject& heapObject, bool verbose);
    void DumpHeapBlockLink(ULONG64 heapBlockType, ULONG64 heapBlock);
    ULONG64 FindHeapBlock(ULONG64 address, ExtRemoteTyped recycler);    
    ExtRemoteTyped FindHeapBlockTyped(ULONG64 address, ExtRemoteTyped recycler, ULONG64* mapAddr = nullptr);
    ExtRemoteTyped FindHeapBlock32(ULONG64 address, ExtRemoteTyped heapBlockMap);
    ULONG64 GetHeapBlockType(ExtRemoteTyped& heapBlock);
    ushort GetAddressSmallHeapBlockBitIndex(ULONG64 objectAddress);

    uint GetObjectGranularity();
    uint GetObjectAllocationShift();
    uint GetObjectAlignmentMask();
    ushort GetSmallHeapBlockObjectIndex(ExtRemoteTyped heapBlockObject, ULONG64 objectAddress);
    ushort GetMediumHeapBlockObjectIndex(ExtRemoteTyped heapBlockObject, ULONG64 objectAddress);
    bool IsAlignedAddress(ULONG64 address);

private:
    EXT_CLASS_BASE* ext;
    ExtRemoteTyped recycler;
    ULONG objectAllocationShift;
};

enum BucketType
{
    NormalBucketType,
    FinalizableBucketType,
    LeafBucketType,
    NormalWithBarrierBucketType,
    FinalizeWithBarrierBucketType,
    MediumBucketType,
    LargeBucketType
};

class RecyclerForEachHeapBlock
{
public:
    RecyclerForEachHeapBlock(ExtRemoteTyped recycler)
        :recycler(recycler)
    {
    }
    virtual bool Run();
protected:
    bool ProcessHeapBlock(ExtRemoteTyped block)
    {
        return ProcessHeapBlock(block, false, block.Field("freeObjectList"), false);
    }

    virtual bool ProcessHeapBlock(ExtRemoteTyped block, bool isAllocator, ExtRemoteTyped freeObjectList, bool isBumpAllocator) = 0;
    virtual bool ProcessLargeHeapBlock(ExtRemoteTyped block) = 0;

    virtual bool ProcessBuckets(ExtRemoteTyped buckets, ExtRemoteTyped recycler);
    virtual bool ProcessMediumBuckets(ExtRemoteTyped buckets);
    virtual bool ProcessHeapInfo(ExtRemoteTyped buckets);
    virtual bool ProcessBucketGroup(unsigned int bucketIndex, ExtRemoteTyped bucketGroup, ExtRemoteTyped recycler);
    virtual bool ProcessBucket(unsigned int bucketIndex, ExtRemoteTyped bucket, BucketType type, ExtRemoteTyped recycler);
    virtual bool ProcessLargeHeapBucket(ExtRemoteTyped largeHeapBucket);
    virtual bool ProcessHeapBlockList(ExtRemoteTyped bucket, char const * listname);
    virtual bool ProcessLargeHeapBlockList(ExtRemoteTyped bucket, char const * listname);
    ExtRemoteTyped recycler;
};

///
/// Class which simply accumulates all small heap blocks
/// Useful for debugging, in order to make sure that we are accounting for all our small heap blocks
///
class CollectSmallHeapBlocks : public RecyclerForEachHeapBlock
{

public:
    CollectSmallHeapBlocks(Extension* ext, ExtRemoteTyped recycler) :
        RecyclerForEachHeapBlock(recycler),
        ext(ext)
    {}

    bool HasBlock(ULONG64 b)
    {
        return blocks.find(b) != blocks.end();
    }

    int Count() {
        return (int)blocks.size();
    }

    virtual bool ProcessLargeHeapBlock(ExtRemoteTyped block) { return false; }

    virtual bool ProcessHeapBlock(ExtRemoteTyped block, bool isAllocator, ExtRemoteTyped freeObjectList, bool isBumpAllocator) override
    {
        ULONG64 key = block.GetPtr();
        if (blocks.find(key) == blocks.end())
        {
            blocks.insert(key);
        }
        return false;
    }
private:
    Extension* ext;
    stdext::hash_set<ULONG64> blocks;
};

///
/// Similar to RecyclerForEachHeapBlock, in that it's used to enumerate all heap blocks
/// The difference is that it operates off the heap block map instead of HeapInfo
/// The results should be effectively the same
///
class HeapBlockMapWalker
{
public:
    HeapBlockMapWalker(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, bool skipMultipleHeapBlocks = true) :
        ext(ext), recycler(recycler), skipMultipleHeapBlocks(skipMultipleHeapBlocks)
    {}

    virtual bool Run();
protected:
    virtual bool ProcessHeapBlock(ULONG64 l1Id, ULONG64 l2Id, ULONG64 blockAddress, ExtRemoteTyped block) = 0;
    virtual bool ProcessLargeHeapBlock(ULONG64 l1Id, ULONG64 l2Id, ULONG64 blockAddress, ExtRemoteTyped block) = 0;

    EXT_CLASS_BASE* ext;
    bool skipMultipleHeapBlocks;
    ExtRemoteTyped recycler;
};


struct RecyclerBucketStats
{
    ULONG64 count;
    ULONG64 emptyCount;
    ULONG64 finalizeBlockCount;
    ULONG64 objectCount;
    ULONG64 finalizeCount;
    ULONG64 objectByteCount;
    ULONG64 totalByteCount;

    void Merge(RecyclerBucketStats const& current)
    {
        count += current.count;
        emptyCount += current.emptyCount;
        finalizeBlockCount += current.finalizeBlockCount;
        finalizeCount += current.finalizeCount;
        objectCount += current.objectCount;
        objectByteCount += current.objectByteCount;
        totalByteCount += current.totalByteCount;
    }

    void Out(ExtExtension * ext)
    {
        ext->Out("%5I64u %5I64u %7I64u %4I64u %11I64u %11I64u %11I64u   %6.2f%%",
            count, finalizeBlockCount, objectCount, finalizeCount,
            objectByteCount, totalByteCount - objectByteCount, totalByteCount,
            100.0 * (static_cast<double>(objectByteCount) / totalByteCount));
    }
};

class PrintHeapBlockStats : HeapBlockMapWalker
{
public:
    PrintHeapBlockStats(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler) :
        HeapBlockMapWalker(ext, recycler),
        heapBlockHelper(ext, recycler),
        heapBlockCollector((Extension*)ext, recycler)
    {}

    virtual bool Run();
protected:
    virtual bool ProcessHeapBlock(ULONG64 l1Id, ULONG64 l2Id, ULONG64 blockAddress, ExtRemoteTyped block);
    virtual bool ProcessLargeHeapBlock(ULONG64 l1Id, ULONG64 l2Id, ULONG64 blockAddress, ExtRemoteTyped block);

    RecyclerBucketStats normalStats[HeapConstants::BucketCount];
    RecyclerBucketStats leafStats[HeapConstants::BucketCount];
    RecyclerBucketStats finalizableStats[HeapConstants::BucketCount];
    RecyclerBucketStats normalWBStats[HeapConstants::BucketCount];
    RecyclerBucketStats finalizableWBStats[HeapConstants::BucketCount];

    RecyclerBucketStats largeStats;
    RecyclerBucketStats smallStats;

    HeapBlockHelper heapBlockHelper;
    CollectSmallHeapBlocks heapBlockCollector;
};

enum PrintBucketStatsFilter
{
    StatsFilterBuckets = 1,
    StatsFilterPageAllocator = 2,
    StatsFilterSummary = 6  // Print the page allocator stats in the summary too
};

class RecyclerPrintBucketStats : public RecyclerForEachHeapBlock
{
public:
    RecyclerPrintBucketStats(EXT_CLASS_BASE * ext, PrintBucketStatsFilter filter, ExtRemoteTyped recycler)
        : RecyclerForEachHeapBlock(recycler), ext(ext), filter(filter)
    {}
    virtual bool Run() override;
protected:
    virtual bool ProcessBucket(unsigned int bucketIndex, ExtRemoteTyped bucket, BucketType type, ExtRemoteTyped recycler) override;
    virtual bool ProcessLargeHeapBucket(ExtRemoteTyped autoHeap) override;
    virtual bool ProcessHeapInfo(ExtRemoteTyped buckets);
    virtual bool ProcessHeapBlock(ExtRemoteTyped block, bool isAllocator, ExtRemoteTyped freeObjectList, bool isBumpAllocator) override;
    virtual bool ProcessLargeHeapBlock(ExtRemoteTyped block) override;
private:
    EXT_CLASS_BASE * ext;
    PrintBucketStatsFilter filter;

    RecyclerBucketStats totalStats;
    RecyclerBucketStats current;

    RecyclerBucketStats largeObjectStats;
    RecyclerBucketStats mediumObjectStats;
    RecyclerBucketStats smallObjectStats;
    RecyclerBucketStats newObjectStats;
};

// Walk through every heap block, scan the valid objects for a reference to
// the given object
class RecyclerFindReference : public RecyclerForEachHeapBlock
{
    struct FindRefData
    {
        ULONG64 address;
        ULONG64 offset;
        bool isRoot;
        bool isLarge;
    };
public:
    RecyclerFindReference(EXT_CLASS_BASE * ext, ULONG64 referencedObject, Addresses* rootPointerManager, ExtRemoteTyped recycler) :
        RecyclerForEachHeapBlock(recycler),
        ext(ext),
        referencedObject(referencedObject),
        skippedAddresses(false),
        rootPointerManager(rootPointerManager)
    {}

    bool FoundNoReferences() { return results.size() == 0; }
    bool SkippedSomeAddresses() { return skippedAddresses; }
    std::vector<FindRefData> results;

protected:
    virtual bool ProcessHeapBlock(ExtRemoteTyped block, bool isAllocator, ExtRemoteTyped freeObjectList, bool isBumpAllocator) override;
    virtual bool ProcessLargeHeapBlock(ExtRemoteTyped block) override;
private:
    EXT_CLASS_BASE * ext;
    ULONG64        referencedObject;
    bool skippedAddresses;
    Addresses* rootPointerManager;
};

class AutoFree
{
public:
    ~AutoFree()
    {
        if (_ptr) free(_ptr);
    }

    AutoFree(void* ptr):
        _ptr(ptr)
    {
    }
private:
    void* _ptr;
};

#endif
