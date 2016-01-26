//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "jdrecycler.h"
#include "RemoteRecyclerList.h"
#include "RemoteRecycler.h"
#include "RecyclerRoots.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

#pragma warning(disable: 4481)  // allow use of abstract and override keywords

// TODO: Get the number of buckets from the recycler.h
static const DWORD PageSize = 4096;

bool
HeapBlockMapWalker::Run()
{
    ExtRemoteTyped hbm = recycler.Field("heapBlockMap");

    if (ext->m_PtrSize == 4)
    {
        ExtRemoteTyped map = hbm.Field("map");

        return ProcessL1Chunk(map);
    }
    else
    {
        ExtRemoteTyped current = hbm.Field("list");
        while (current.GetPtr() != 0)
        {
            ExtRemoteTyped map = current.Field("map");
            ExtRemoteTyped l1ChunkArray = map.Field("map");
            if (ProcessL1Chunk(l1ChunkArray))
            {
                return true;
            }

            current = current.Field("next");
        }
    }

    return false;
}

bool
HeapBlockMapWalker::ProcessL1Chunk(ExtRemoteTyped chunk)
{
    const size_t l1Count = chunk.GetTypeSize() / chunk.ArrayElement(0).GetTypeSize();

    for (size_t l1Id = 0; l1Id < l1Count; l1Id++)
    {
        ExtRemoteTyped l2MapChunk = chunk.ArrayElement(l1Id);
        if (GetAsPointer(l2MapChunk) != 0)
        {
            if (ProcessL2Chunk(l1Id, l2MapChunk.Dereference()))
            {
                return true;
            }
        }
    }

    return false;
}

bool
HeapBlockMapWalker::ProcessL2Chunk(size_t l1Id, ExtRemoteTyped chunk)
{
    ExtRemoteTyped l2map = chunk.Field("map");
    const size_t l2Count = l2map.GetTypeSize() / l2map.ArrayElement(0).GetTypeSize();
    for (size_t l2Id = 0; l2Id < l2Count; l2Id++)
    {
        ExtRemoteTyped heapBlock = l2map.ArrayElement(l2Id);
        ULONG64 block = GetAsPointer(heapBlock);
        if (block != 0)
        {
            heapBlock = ext->CastWithVtable(heapBlock);
            auto type = heapBlock.Field("heapBlockType").GetChar();

            if (type == ext->enum_LargeBlockType())
            {
                // Only LargeHeapBlock cross multiple entries
                if (!this->skipMultipleLargeHeapBlocks || lastLargeHeapBlock != block)
                {
                    if (ProcessLargeHeapBlock(l1Id, l2Id, block, heapBlock))
                    {
                        return true;
                    }
                    lastLargeHeapBlock = block;
                }
            }
            else
            {
                Assert(block != lastLargeHeapBlock);
                if (ProcessHeapBlock(l1Id, l2Id, block, heapBlock))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool
PrintHeapBlockStats::ProcessLargeHeapBlock(size_t, size_t, ULONG64 blockAddress, ExtRemoteTyped heapBlock)
{
    largeStats.count++;
    unsigned long finalizeCount = heapBlock.Field("finalizeCount").GetUlong();
    if (finalizeCount != 0)
    {
        largeStats.finalizeBlockCount++;
        largeStats.finalizeCount += finalizeCount;
    }

    unsigned int allocCount = heapBlock.Field("allocCount").GetUlong();
    ExtRemoteTyped headerList =
        ExtRemoteTyped(ext->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader **)@$extin"), heapBlock.GetPtr() + heapBlock.Dereference().GetTypeSize());

    for (unsigned int i = 0; i < allocCount; i++)
    {
        ExtRemoteTyped header = headerList.ArrayElement(i);
        if (header.GetPtr() == 0)
        {
            continue;
        }
        largeStats.objectCount++;
        largeStats.objectByteCount += EXT_CLASS_BASE::GetSizeT(header.Field("objectSize"));
    }

    largeStats.totalByteCount += 4096 * EXT_CLASS_BASE::GetSizeT(heapBlock.Field("pageCount"));
    return false; //  not done yet
}

bool
PrintHeapBlockStats::ProcessHeapBlock(size_t, size_t, ULONG64 blockAddress, ExtRemoteTyped heapBlock)
{

    auto type = heapBlock.Field("heapBlockType").GetChar();
    ushort objectSize = heapBlock.Field("objectSize").GetUshort();
    uint bucketIndex = (objectSize >> heapBlockHelper.GetObjectAllocationShift()) - 1;

    RecyclerBucketStats* stats = nullptr;

    if (type == ext->enum_SmallNormalBlockType())
        stats = &normalStats[bucketIndex];
    else if (type == ext->enum_SmallLeafBlockType())
        stats = &leafStats[bucketIndex];
    else if (type == ext->enum_SmallFinalizableBlockType())
        stats = &finalizableStats[bucketIndex];
    else if (type == ext->enum_SmallNormalBlockWithBarrierType())
        stats = &normalWBStats[bucketIndex];
    else if (type == ext->enum_SmallFinalizableBlockWithBarrierType())
        stats = &finalizableWBStats[bucketIndex];
    else
        Assert(false);

    if (!this->heapBlockCollector.HasBlock(blockAddress))
    {
        ext->Out("Unseen block: 0x%p, New: %d, InAllocator: %d, Size: 0x%x, Type: %d\n",
            blockAddress,
            heapBlock.Field("isNewHeapBlock").GetChar(),
            heapBlock.Field("isInAllocator").GetChar(),
            objectSize,
            type
            );
    }

    ushort objectCount = heapBlock.Field("objectCount").GetUshort();
    stats->count++;
    stats->objectCount += objectCount;
    stats->objectByteCount += (objectCount * heapBlock.Field("objectSize").GetUshort());
    stats->totalByteCount += 4096;
    if (heapBlock.HasField("finalizeCount"))
    {
        unsigned short finalizeCount = heapBlock.Field("finalizeCount").GetUshort();
        if (finalizeCount != 0)
        {
            stats->finalizeBlockCount++;
            stats->finalizeCount += finalizeCount;
        }
    }

    return false;
}

bool
PrintHeapBlockStats::Run()
{
    // Do the regular heap block walk
    heapBlockCollector.Run();
    ext->Out("Collected blocks: %d\n", heapBlockCollector.Count());

    memset(&largeStats, 0, sizeof(largeStats));
    memset(&smallStats, 0, sizeof(smallStats));

    for (int i = 0; i < _countof(normalStats); i++)
    {
        memset(&normalStats[i], 0, sizeof(RecyclerBucketStats));
    }

    for (int i = 0; i < _countof(normalWBStats); i++)
    {
        memset(&normalWBStats[i], 0, sizeof(RecyclerBucketStats));
    }

    for (int i = 0; i < _countof(leafStats); i++)
    {
        memset(&leafStats[i], 0, sizeof(RecyclerBucketStats));
    }

    for (int i = 0; i < _countof(finalizableStats); i++)
    {
        memset(&finalizableStats[i], 0, sizeof(RecyclerBucketStats));
    }

    for (int i = 0; i < _countof(finalizableWBStats); i++)
    {
        memset(&finalizableWBStats[i], 0, sizeof(RecyclerBucketStats));
    }

    ext->Out("Recycler Bucket stats\n");
    ext->Out("---------------------------------------------------------------------------------------\n");
    ext->Out("               #Blk #FinB   #Objs #Fin     PgBytes   FreeBytes  TotalBytes  UsedPercent\n");

    bool ret = __super::Run();

    ext->Out("---------------------------------------------------------------------------------------\n");

    for (int i = 0; i < HeapConstants::BucketCount; i++)
    {
        if (normalStats[i].count > 0)
        {
            ext->Out("Normal %4d : ", ((i + 1) * heapBlockHelper.GetObjectGranularity()));
            normalStats[i].Out(ext);
            smallStats.Merge(normalStats[i]);
            ext->Out("\n");
        }

        if (normalWBStats[i].count > 0)
        {
            ext->Out("NormWB %4d : ", ((i + 1) * heapBlockHelper.GetObjectGranularity()));
            normalWBStats[i].Out(ext);
            smallStats.Merge(normalWBStats[i]);
            ext->Out("\n");
        }

        if (finalizableStats[i].count > 0)
        {
            ext->Out("Fin    %4d : ", ((i + 1) * heapBlockHelper.GetObjectGranularity()));
            finalizableStats[i].Out(ext);
            smallStats.Merge(finalizableStats[i]);
            ext->Out("\n");
        }

        if (finalizableWBStats[i].count > 0)
        {
            ext->Out("FinWB  %4d : ", ((i + 1) * heapBlockHelper.GetObjectGranularity()));
            finalizableWBStats[i].Out(ext);
            smallStats.Merge(finalizableWBStats[i]);
            ext->Out("\n");
        }

        if (leafStats[i].count > 0)
        {
            ext->Out("Leaf   %4d : ", ((i + 1) * heapBlockHelper.GetObjectGranularity()));
            leafStats[i].Out(ext);
            smallStats.Merge(leafStats[i]);
            ext->Out("\n");
        }
    }

    if (largeStats.count > 0)
    {
        ext->Out("Large       : ");
        largeStats.Out(ext);
        ext->Out("\n");
    }

    RecyclerBucketStats totalStats = { 0 };
    totalStats.Merge(largeStats);
    totalStats.Merge(smallStats);

    ext->Out("-----------------------------------------------------------------------------------------\n");
    ext->Out("Total       : ");
    totalStats.Out(ext);
    ext->Out("\n");

    return ret;
}

bool
RecyclerForEachHeapBlock::Run()
{
    ExtRemoteTyped autoHeap = recycler.Field("autoHeap");
    ExtRemoteTyped buckets = autoHeap.Field("heapBuckets");
    ExtRemoteTyped mediumBuckets = autoHeap.Field("mediumHeapBuckets");
    ExtRemoteTyped largeBucket = autoHeap.Field("largeObjectBucket");

    ULONG typeId;
    bool oldStyleMediumBlock = g_Ext->m_Symbols3->GetTypeId(recycler.m_Typed.ModBase, "MediumAllocationBlockAttributes", &typeId) != S_OK;

    bool stopProcessing =
        ProcessBuckets(buckets, recycler) ||
        ProcessLargeHeapBucket(largeBucket) ||
        ProcessHeapInfo(autoHeap);

    if (!stopProcessing)
    {
        if (oldStyleMediumBlock)
        {
            stopProcessing = ProcessMediumBuckets(mediumBuckets);
        }
        else
        {
            stopProcessing = ProcessBuckets(mediumBuckets, recycler);
        }
    }

    return stopProcessing;
}

bool
RecyclerForEachHeapBlock::ProcessBuckets(ExtRemoteTyped buckets, ExtRemoteTyped recycler)
{
    const size_t BucketCount = buckets.GetTypeSize() / buckets.ArrayElement(0).GetTypeSize();
    for (DWORD i = 0; i < BucketCount; i++)
    {
        ExtRemoteTyped bucketgroup = buckets.ArrayElement(i);
        if (ProcessBucketGroup(i, bucketgroup, recycler))
        {
            return true;
        }
    }
    return false;
}

bool
RecyclerForEachHeapBlock::ProcessMediumBuckets(ExtRemoteTyped buckets)
{
    const size_t BucketCount = buckets.GetTypeSize() / buckets.ArrayElement(0).GetTypeSize();
    for (DWORD i = 0; i < BucketCount; i++)
    {
        ExtRemoteTyped bucket = buckets.ArrayElement(i);
        if (ProcessLargeHeapBucket(bucket))
        {
            return true;
        }
    }
    return false;
}

bool
RecyclerForEachHeapBlock::ProcessBucketGroup(unsigned int index, ExtRemoteTyped bucketgroup, ExtRemoteTyped recycler)
{
    return ProcessBucket(index, bucketgroup.Field("heapBucket").GetPointerTo(), NormalBucketType, recycler) ||
        ProcessBucket(index, bucketgroup.Field("finalizableHeapBucket").GetPointerTo(), FinalizableBucketType, recycler) ||
        ProcessBucket(index, bucketgroup.Field("leafHeapBucket").GetPointerTo(), LeafBucketType, recycler) ||
        bucketgroup.HasField("smallNormalWithBarrierHeapBucket") && ProcessBucket(index, bucketgroup.Field("smallNormalWithBarrierHeapBucket").GetPointerTo(), NormalWithBarrierBucketType, recycler) ||
        bucketgroup.HasField("smallFinalizableWithBarrierHeapBucket") && ProcessBucket(index, bucketgroup.Field("smallFinalizableWithBarrierHeapBucket").GetPointerTo(), FinalizeWithBarrierBucketType, recycler);
}

bool
RecyclerForEachHeapBlock::ProcessHeapInfo(ExtRemoteTyped heapInfo)
{
    ExtRemoteTyped recycler = heapInfo.Field("recycler");
    ExtRemoteTyped recyclerSweep = recycler.Field("recyclerSweep");

    bool processRecyclerSweep = (recyclerSweep.GetPtr() != 0);

    bool keepProcessing = ProcessHeapBlockList(heapInfo, "newLeafHeapBlockList") ||
        ProcessHeapBlockList(heapInfo, "newNormalHeapBlockList") ||
        ProcessHeapBlockList(heapInfo, "newFinalizableHeapBlockList");

    bool hasMediumBlockLists = false;

    if (heapInfo.HasField("newMediumNormalHeapBlockList"))
    {
        hasMediumBlockLists = true;
    }

    if (hasMediumBlockLists)
    {
        keepProcessing = keepProcessing ||
            ProcessHeapBlockList(heapInfo, "newMediumLeafHeapBlockList") ||
            ProcessHeapBlockList(heapInfo, "newMediumNormalHeapBlockList") ||
            ProcessHeapBlockList(heapInfo, "newMediumFinalizableHeapBlockList");
    }

    if (heapInfo.HasField("newNormalWithBarrierHeapBlockList"))
    {
        keepProcessing = keepProcessing ||
            ProcessHeapBlockList(heapInfo, "newNormalWithBarrierHeapBlockList") ||
            ProcessHeapBlockList(heapInfo, "newFinalizableWithBarrierHeapBlockList");

        if (hasMediumBlockLists)
        {
            keepProcessing = keepProcessing ||
                ProcessHeapBlockList(heapInfo, "newMediumNormalWithBarrierHeapBlockList") ||
                ProcessHeapBlockList(heapInfo, "newMediumFinalizableWithBarrierHeapBlockList");
        }
    }

    if (processRecyclerSweep)
    {
        keepProcessing = keepProcessing ||
            ProcessHeapBlockList(recyclerSweep.Field("leafData"), "pendingMergeNewHeapBlockList") ||
            ProcessHeapBlockList(recyclerSweep.Field("normalData"), "pendingMergeNewHeapBlockList") ||
            ProcessHeapBlockList(recyclerSweep.Field("finalizableData"), "pendingMergeNewHeapBlockList");

        if (hasMediumBlockLists)
        {
            keepProcessing = keepProcessing ||
                ProcessHeapBlockList(recyclerSweep.Field("mediumLeafData"), "pendingMergeNewHeapBlockList") ||
                ProcessHeapBlockList(recyclerSweep.Field("mediumNormalData"), "pendingMergeNewHeapBlockList") ||
                ProcessHeapBlockList(recyclerSweep.Field("mediumFinalizableData"), "pendingMergeNewHeapBlockList");
        }
    }

    return keepProcessing;
}

bool
RecyclerForEachHeapBlock::ProcessBucket(unsigned int bucketIndex, ExtRemoteTyped bucket, BucketType type, ExtRemoteTyped recycler)
{
    ExtRemoteTyped allocatorHead = bucket.Field("allocatorHead").GetPointerTo();
    ExtRemoteTyped allocatorCurr = allocatorHead;
    do
    {
        ExtRemoteTyped heapBlock = allocatorCurr.Field("heapBlock");
        if (heapBlock.GetPtr())
        {
            if (ProcessHeapBlock(heapBlock, true, allocatorCurr.Field("freeObjectList"), allocatorCurr.Field("endAddress").GetPtr() != 0))
            {
                return true;
            }
        }
        allocatorCurr = allocatorCurr.Field("next");
    }
    while (allocatorCurr.GetPtr() != allocatorHead.GetPtr());

    ExtRemoteTyped recyclerSweep = recycler.Field("recyclerSweep");
    ExtRemoteTyped bucketData;
    bool skipRecyclerSweepData = false;

    if (recyclerSweep.GetPtr() == 0)
    {
        skipRecyclerSweepData = true;
    }
    else
    {
        ExtRemoteTyped data;
        switch (type)
        {
        case NormalBucketType: data = recyclerSweep.Field("normalData"); break;
        case FinalizableBucketType: data = recyclerSweep.Field("finalizableData"); break;
        case LeafBucketType: data = recyclerSweep.Field("leafData"); break;
        case NormalWithBarrierBucketType: data = recyclerSweep.Field("withBarrierData"); break;
        case FinalizeWithBarrierBucketType: data = recyclerSweep.Field("finalizableWithBarrierData"); break;
        default: Assert(false);
        };

        bucketData = data.Field("bucketData").ArrayElement(bucketIndex);
    }

    bool keepProcessing = ProcessHeapBlockList(bucket, "fullBlockList") ||
        ProcessHeapBlockList(bucket, "heapBlockList") ||
        ProcessHeapBlockList(bucket, "pendingDisposeList") ||
        ProcessHeapBlockList(bucket, "partialHeapBlockList") ||
        ProcessHeapBlockList(bucket, "partialSweptHeapBlockList");

    if (!skipRecyclerSweepData)
    {
        keepProcessing = keepProcessing ||
            ProcessHeapBlockList(bucketData, "pendingSweepList") ||
            ProcessHeapBlockList(bucketData, "pendingFinalizableSweptList");
    }

    return keepProcessing;
}

bool
RecyclerForEachHeapBlock::ProcessHeapBlockList(ExtRemoteTyped bucket, char const * listName)
{
    if (!bucket.HasField(listName))
    {
        return false;
    }

    ExtRemoteTyped next = bucket.Field(listName);
    while (next.GetPtr())
    {
        if (ProcessHeapBlock(next))
        {
            return true;
        }

        ExtRemoteTyped nextField = next.Field("next");
        next.Set(false, next.m_Typed.ModBase, next.m_Typed.TypeId, nextField.m_Offset);
    }
    return false;
}

bool
RecyclerForEachHeapBlock::ProcessLargeHeapBucket(ExtRemoteTyped heapBucket)
{
    return ProcessLargeHeapBlockList(heapBucket, "largeBlockList") ||
        ProcessLargeHeapBlockList(heapBucket, "largePageHeapBlockList") ||
        ProcessLargeHeapBlockList(heapBucket, "fullLargeBlockList") ||
        ProcessLargeHeapBlockList(heapBucket, "pendingDisposeLargeBlockList") ||
        ProcessLargeHeapBlockList(heapBucket, "pendingDisposeLargeBlockList") ||
        ProcessLargeHeapBlockList(heapBucket, "pendingSweepLargeBlockList") ||
        ProcessLargeHeapBlockList(heapBucket, "partialSweptLargeBlockList");
}

bool
RecyclerForEachHeapBlock::ProcessLargeHeapBlockList(ExtRemoteTyped bucket, char const * listName)
{
    if (!bucket.HasField(listName))
    {
        return false;
    }

    ExtRemoteTyped next = bucket.Field(listName);
    while (next.GetPtr())
    {
        if (ProcessLargeHeapBlock(next))
        {
            return true;
        }
        next = next.Field("next");
    }
    return false;
}

#include "RemotePageAllocator.h"

bool
RecyclerPrintBucketStats::Run()
{
    memset(&totalStats, 0, sizeof(totalStats));

    memset(&largeObjectStats, 0, sizeof(largeObjectStats));
    memset(&mediumObjectStats, 0, sizeof(mediumObjectStats));
    memset(&smallObjectStats, 0, sizeof(smallObjectStats));
    memset(&newObjectStats, 0, sizeof(newObjectStats));

    ext->Out("Recycler Bucket stats\n");
    ext->Out("---------------------------------------------------------------------------------------\n");
    ext->Out("               #Blk #FinB   #Objs #Fin    ObjBytes   FreeBytes  TotalBytes  UsedPercent\n");

    bool ret = __super::Run();

    ext->Out("---------------------------------------------------------------------------------------\n");
    if (filter == StatsFilterSummary)
    {
        if (smallObjectStats.totalByteCount != 0)
        {
            ext->Out("SmallObjects: ");
            smallObjectStats.Out(ext);
            ext->Out("\n");
        }
        if (newObjectStats.totalByteCount != 0)
        {
            ext->Out("NewObjects:   ");
            newObjectStats.Out(ext);
            ext->Out("\n");
        }
        if (mediumObjectStats.totalByteCount != 0)
        {
            ext->Out("MedObjects  : ");
            mediumObjectStats.Out(ext);
            ext->Out("\n");
        }

        if (largeObjectStats.totalByteCount != 0)
        {
            ext->Out("LargeObjects: ");
            largeObjectStats.Out(ext);
            ext->Out("\n");
        }

        ext->Out("-----------------------------------------------------------------------------------------\n");
    }

    ext->Out("Total       : ");
    totalStats.Out(ext);
    ext->Out("\n");

    return ret;
}

bool
RecyclerPrintBucketStats::ProcessHeapInfo(ExtRemoteTyped heapInfo)
{
    memset(&current, 0, sizeof(current));
    bool ret = __super::ProcessHeapInfo(heapInfo);

    if (current.count != 0)
    {
        if (filter == StatsFilterBuckets)
            ext->Out("New Blocks  : ");
        newObjectStats.Merge(current);

        if (filter == StatsFilterBuckets)
        {
            current.Out(ext);
            ext->Out("\n");
        }
    }

    totalStats.Merge(current);
    return ret;
}

bool
RecyclerPrintBucketStats::ProcessLargeHeapBucket(ExtRemoteTyped bucket)
{
    memset(&current, 0, sizeof(current));
    bool ret = __super::ProcessLargeHeapBucket(bucket);

    if (current.count != 0)
    {
        ulong bucketSize = bucket.Field("sizeCat").GetUlong();

        if (bucketSize < HeapConstants::MaxMediumObjectSize)
        {
            if ((filter & StatsFilterBuckets) != 0)
                ext->Out("Medium %4d : ", bucketSize);
            mediumObjectStats.Merge(current);
        }
        else
        {
            if ((filter & StatsFilterBuckets) != 0)
                ext->Out("Large Bucket: ");
            largeObjectStats.Merge(current);
        }

        if ((filter & StatsFilterBuckets) != 0)
        {
            current.Out(ext);
            ext->Out("\n");
        }
    }

    totalStats.Merge(current);
    return ret;
}

bool
RecyclerPrintBucketStats::ProcessBucket(unsigned int bucketIndex, ExtRemoteTyped bucket, BucketType type, ExtRemoteTyped recycler)
{
    memset(&current, 0, sizeof(current));

    PSTR bucketType = "Norm";

    switch (type)
    {
    case FinalizableBucketType: bucketType = "Fin"; break;
    case LeafBucketType: bucketType = "Leaf"; break;
    case NormalWithBarrierBucketType: bucketType = "NormSWB"; break;
    case FinalizeWithBarrierBucketType: bucketType = "FinSWB"; break;
    default: bucketType = "Norm"; break;
    };

    bool ret = __super::ProcessBucket(bucketIndex, bucket, type, recycler);
    if ((filter & StatsFilterBuckets) != 0 && current.count != 0)
    {
        ext->Out("%-7s%4d : ", bucketType, bucket.Field("sizeCat").GetUlong());
        current.Out(ext);
        ext->Out("\n");
    }
    smallObjectStats.Merge(current);
    totalStats.Merge(current);

    return ret;
}
bool
RecyclerPrintBucketStats::ProcessHeapBlock(ExtRemoteTyped heapBlock, bool isAllocator, ExtRemoteTyped freeObjectList, bool isBumpAllocator)
{
    // Skip any allocator blocks that are in the new heap block list
    // TODO: Change the model so that we can update stats for different buckets out of order
    if (isAllocator && heapBlock.Field("isNewHeapBlock").GetChar() != 0)
    {
        return false;
    }

    current.count++;
    int objectCount;
    if (isBumpAllocator)
    {
        objectCount = (ushort)((GetAsPointer(freeObjectList) - GetAsPointer(heapBlock.Field("address"))) / heapBlock.Field("objectSize").GetUshort());
    }
    else
    {
        objectCount = heapBlock.Field("objectCount").GetUshort();
        if (freeObjectList.HasField("next"))
        {
            objectCount -= (unsigned short)EXT_CLASS_BASE::Count(freeObjectList, "next");
        }
        else
        {
            objectCount -= (unsigned short)EXT_CLASS_BASE::TaggedCount(freeObjectList, "taggedNext");
        }
    }

    // After CL#1149965, heap block in the allocator are also in the heap block list
    // But the allocator has the more up to date information. so subtract the heap block information
    // to counteract when we count the same block in the heap block list

    // use the existent of the needOOMRescan field to detect that change
    bool newHeapBlockLayout = heapBlock.HasField("needOOMRescan");
    if (isAllocator)
    {
        // After CL#1149965, heap block in the allocator are also in the heap block list
        // But the allocator has the more up to date information. so subtract the heap block information
        // to counteract when we count the same block in the heap block list

        // use the existent of the needOOMRescan field to detect that change
        if (newHeapBlockLayout)
        {
            objectCount -= heapBlock.Field("objectCount").GetUshort();
            objectCount += (unsigned short)EXT_CLASS_BASE::TaggedCount(heapBlock.Field("freeObjectList"), "taggedNext");
        }
    }

    current.objectCount += objectCount;
    current.objectByteCount += objectCount * heapBlock.Field("objectSize").GetUshort();

    if (!newHeapBlockLayout || !isAllocator)
    {
        // After CL#1139848, not all heap block has finalize count
        if (heapBlock.HasField("finalizeCount"))
        {
            unsigned short finalizeCount = heapBlock.Field("finalizeCount").GetUshort();
            if (finalizeCount != 0)
            {
                current.finalizeBlockCount++;
                current.finalizeCount += finalizeCount;
            }
        }
        current.totalByteCount += 4096;
    }
    return false; //  not done yet
}

bool
RecyclerPrintBucketStats::ProcessLargeHeapBlock(ExtRemoteTyped heapBlock)
{
    current.count++;

    unsigned long finalizeCount = heapBlock.Field("finalizeCount").GetUlong();
    if (finalizeCount != 0)
    {
        current.finalizeBlockCount++;
        current.finalizeCount += finalizeCount;
    }

    unsigned int allocCount = heapBlock.Field("allocCount").GetUlong();
    ExtRemoteTyped headerList =
        ExtRemoteTyped(ext->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader **)@$extin"), heapBlock.GetPtr() + heapBlock.Dereference().GetTypeSize());

    for (unsigned int i = 0; i < allocCount; i++)
    {
        ExtRemoteTyped header = headerList.ArrayElement(i);
        if (header.GetPtr() == 0)
        {
            continue;
        }
        current.objectCount++;
        current.objectByteCount += EXT_CLASS_BASE::GetSizeT(header.Field("objectSize"));
    }

    current.totalByteCount += 4096 * EXT_CLASS_BASE::GetSizeT(heapBlock.Field("pageCount"));
    return false; //  not done yet

}

ExtRemoteTyped HeapBlockHelper::FindHeapBlock32(ULONG64 address, ExtRemoteTyped heapBlockMap, bool allowOutput)
{
    uint id1 = HeapBlockMap32::GetLevel1Id((void *)address);
    ExtRemoteTyped l2map = heapBlockMap.Field("map").ArrayElement(id1);

    if (l2map.GetPtr() == NULL)
    {
        if (allowOutput)
        {
            ext->Out("Heap block not found\n");
        }
        return ExtRemoteTyped(ext->FillModuleAndMemoryNS("(%s!%sHeapBlock*)@$extin"), 0);
    }

    uint id2 = HeapBlockMap32::GetLevel2Id((void *)address);

    return l2map.Field("map").ArrayElement(id2);
}

ExtRemoteTyped HeapBlockHelper::FindHeapBlockTyped(ULONG64 address, ExtRemoteTyped recycler, bool allowOutput, ULONG64* mapAddr)
{
    ExtRemoteTyped heapBlockMap = recycler.Field("heapBlockMap");

    if (heapBlockMap.HasField("list"))
    {
        // 64-bit
        uint index = HeapBlockMap64::GetNodeIndex(address);

        ExtRemoteTyped node = heapBlockMap.Field("list");

        while (node.GetPtr() != NULL)
        {
            if (node.Field("nodeIndex").GetUlong() == index)
            {
                heapBlockMap = node.Field("map");
                if (mapAddr)
                {
                    *mapAddr = node.GetPtr();
                }
                return FindHeapBlock32(address, heapBlockMap, allowOutput);
            }
            node = node.Field("next");
        }

        return ExtRemoteTyped(ext->FillModuleAndMemoryNS("(%s!%sHeapBlock*)@$extin"), 0);
    }
    return FindHeapBlock32(address, heapBlockMap, allowOutput);
}

ULONG64 HeapBlockHelper::FindHeapBlock(ULONG64 address, ExtRemoteTyped recycler, bool allowOutput)
{
    return FindHeapBlockTyped(address, recycler, allowOutput).GetPtr();
}

ushort HeapBlockHelper::GetSmallHeapBlockObjectIndex(ExtRemoteTyped heapBlockObject, ULONG64 objectAddress)
{
    ExtRemoteTyped blockAddressField = heapBlockObject.Field("address");
    ULONG64 blockAddress = blockAddressField.GetPtr();
    ExtRemoteTyped validPointers = heapBlockObject.Field("validPointers");
    ExtRemoteTyped validPointerArray = validPointers.Field("validPointers");

    unsigned int offset = (unsigned int)(objectAddress - blockAddress);

    offset = offset >> GetObjectAllocationShift();

    return validPointerArray.ArrayElement(offset).GetUshort();
}

ushort HeapBlockHelper::GetMediumHeapBlockObjectIndex(ExtRemoteTyped heapBlockObject, ULONG64 objectAddress)
{
    ExtRemoteTyped blockAddressField = heapBlockObject.Field("address");
    ULONG64 blockAddress = blockAddressField.GetPtr();
    ExtRemoteTyped validPointers = heapBlockObject.Field("validPointers");
    ExtRemoteTyped validPointerArray = validPointers.Field("validPointers");

    unsigned int offset = (unsigned int)(objectAddress - blockAddress);

    offset = offset >> GetObjectAllocationShift(); // TODO: fix for medium objects

    return validPointerArray.ArrayElement(offset).GetUshort();
}

PCSTR EXT_CLASS_BASE::GetPageAllocatorType()
{
    static char* pageAllocatorTypeName = nullptr;
    if (pageAllocatorTypeName == nullptr) {
        pageAllocatorTypeName = "Memory::PageAllocatorBase<Memory::VirtualAllocWrapper>";
        if (!CheckTypeName(pageAllocatorTypeName))
        {
            pageAllocatorTypeName = "PageAllocator";
        }
    }
    return pageAllocatorTypeName;
}

PCSTR EXT_CLASS_BASE::GetSegmentType()
{
    static char* segmentTypeName = nullptr;
    if (segmentTypeName == nullptr) {
        segmentTypeName = "Memory::SegmentBase<Memory::VirtualAllocWrapper>";
        if (!CheckTypeName(segmentTypeName))
        {
            segmentTypeName = "Segment";
        }
    }
    return segmentTypeName;
}
PCSTR EXT_CLASS_BASE::GetPageSegmentType()
{
    static char* pageSegmentTypeName = nullptr;
    if (pageSegmentTypeName == nullptr) {
        pageSegmentTypeName = "Memory::PageSegmentBase<Memory::VirtualAllocWrapper>";
        if (!CheckTypeName(pageSegmentTypeName))
        {
            pageSegmentTypeName = "PageSegment";
        }
    }
    return pageSegmentTypeName;
}

bool EXT_CLASS_BASE::CheckTypeName(PCSTR typeName, ULONG* typeId /*= nullptr*/)
{
    char buf[MAX_SYM_NAME];
    ULONG id = 0;
    sprintf_s(buf, "%s!%s", typeName, GetModuleName());
    HRESULT hr = this->m_Symbols2->GetSymbolTypeId(buf, &id, nullptr);
    if (typeId != nullptr)
    {
        *typeId = id;
    }
    return hr == S_OK;
}


JD_PRIVATE_COMMAND(markmap,
    "Dump the mark map",
    "{;s;filename;Filename to output to}"
    "{;e,o,d=0;recycler;Recycler address}")
{
    PCSTR filename = GetUnnamedArgStr(0);
    ULONG64 arg = GetUnnamedArgU64(1);
    ExtRemoteTyped recycler;
    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
    }
    else
    {
        recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
    }

    if (!recycler.HasField("markMap"))
    {
        Out(L"Recycler doesn't have mark map field. Please rebuild jscript9 with RECYCLER_MARK_TRACK enabled.\n");
        return;
    }

    FILE* f = fopen(filename, "w+");
    if (f != nullptr)
    {
        Out("Recycler is 0x%p\n", recycler);
        ExtRemoteTyped map = recycler.Field("markMap");
        uint bucketCount = map.Field("bucketCount").GetUlong();
        ExtRemoteTyped buckets = map.Field("buckets");
        ExtRemoteTyped entries = map.Field("entries");

        int numEntries = 0;

        fprintf(f, "import networkx as nx\n");
        fprintf(f, "G = nx.DiGraph()\n");

        for (uint i = 0; i < bucketCount; i++)
        {
            if (buckets.ArrayElement(i).GetLong() != -1)
            {
                for (int currentIndex = buckets.ArrayElement(i).GetLong(); currentIndex != -1; currentIndex = entries.ArrayElement(currentIndex).Field("next").GetLong())
                {
                    numEntries++;
                    //if (numEntries > 10) break;

                    ExtRemoteTyped data = entries.ArrayElement(currentIndex);
                    ULONG64 key = 0;
                    ULONG64 value = 0;

                    //Out("0x%p\n", data.m_Offset);
                    //Out("Key: 0x%p\n", data.Field("key").m_Offset);

                    if (m_PtrSize == 4)
                    {
                        key = data.Field("key").GetUlongPtr();
                        value = data.Field("value").GetUlongPtr();
                    }
                    else
                    {
                        key = data.Field("key").GetUlong64();
                        value = data.Field("value").GetUlong64();
                    }

                    fprintf(f, "# Item %d\n", currentIndex);
                    fprintf(f, "G.add_edge(");
                    fprintf(f, "'0x%p'", value);
                    fprintf(f, ", ");
                    fprintf(f, "'0x%p'", key);
                    fprintf(f, ")\n");

                }
            }
        }

        fclose(f);

        Out("%d entries written to '%s'\n", numEntries, filename);
    }
    else
    {
        Out("Could not open '%s'\n", filename);
    }
}

JD_PRIVATE_COMMAND(gcstats,
    "Count recycler objects",
    "{;e,o,d=0;recycler;Recycler address}"
    "{summary;b,o;;Display only a summary}"
    "{filter;s,o;type;Filter the output to either alloc (PageAllocator) or buckets (Heap Buckets)}")
{
    ULONG64 arg = GetUnnamedArgU64(0);

    PrintBucketStatsFilter filter = StatsFilterBuckets;

    if (this->HasArg("summary"))
    {
        filter = StatsFilterSummary;
    }

    if (this->HasArg("filter"))
    {
        PCSTR arg = this->GetArgStr("filter");

        if (_stricmp("alloc", arg) == 0)
        {
            filter = StatsFilterPageAllocator;
        }
        else if (_stricmp("buckets", arg) == 0)
        {
            filter = StatsFilterBuckets;
        }
        else
        {
            Out("Invalid argument: %s\n", arg);
            return;
        }
    }

    RemoteRecycler recycler;
    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
    }
    else
    {
        recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler();
    }

    // If we're only interested in the page allocator data, don't bother collecting bucket stats
    if (filter == StatsFilterBuckets || filter == StatsFilterSummary)
    {
        RecyclerPrintBucketStats collect(this, filter, recycler.GetExtRemoteTyped());
        collect.Run();
        Out("\n");
    }

    if ((filter & StatsFilterPageAllocator) == StatsFilterPageAllocator)
    {
        Out("Recycler Page Allocator stats\n");

        RemotePageAllocator::DisplayDataHeader("Allocator");
        recycler.ForEachPageAllocator("Thread", [](PCSTR name, RemotePageAllocator pageAllocator)
        {
            pageAllocator.DisplayData(name, true);
        });
    }
}

void EXT_CLASS_BASE::DumpBlock(ExtRemoteTyped block, LPCSTR desc, LPCSTR sizeField, int index)
{
    ULONG64 sizeOfBigBlock = this->EvalExprU64(this->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sBigBlock))"));
    // ULONG sizeOfBigBlockPtr = (ULONG) this->EvalExprU64("@@c++(sizeof(BigBlock*))");
    ULONG64 ptrBlock = block.GetPtr();
    ULONG64 length = block.Field(sizeField).GetUlong64();

    Out("%s %d: ", desc, index);
    Dml("<link cmd=\"db 0x%p l0x%x\">0x%p</link>", ptrBlock + sizeOfBigBlock, length, ptrBlock); // Hack- since ArenaMemoryData and BigBlock have the same size
    Out(" (");

    Out("0x%x", length);
    Out(" bytes)\n");
}
ULONG64 HeapBlockHelper::GetHeapBlockType(ExtRemoteTyped& heapBlock)
{
    ExtRemoteTyped heapBlockType = heapBlock.Field("heapBlockType");
    auto type = heapBlockType.GetChar();
    heapBlock = ext->CastWithVtable(heapBlock);
    return type;
}

void HeapBlockHelper::DumpObjectInfoBits(unsigned char info )
{
    info = info & ObjectInfoBits::InternalObjectInfoBitMask;

    ext->Out(L"Info: 0x%x (", info);

    if (info & ObjectInfoBits::FinalizeBit) ext->Out(" Finalize ");
    if (info & ObjectInfoBits::LeafBit) ext->Out(" Leaf ");
    if (info & ObjectInfoBits::TrackBit) ext->Out(" Track ");
    if (info & ObjectInfoBits::ImplicitRootBit) ext->Out(" ImplticitRoot ");
    if (info & ObjectInfoBits::MemoryProfilerOldObjectBit) ext->Out(" MemoryProfilerOldObject ");
    if (info & ObjectInfoBits::EnumClass_1_Bit) ext->Out(" EnumClass_1_Bit ");
    if (info & ObjectInfoBits::ClientTrackedBit) ext->Out(" ClientTrackedBit ");
    if (info & ObjectInfoBits::TraceBit) ext->Out(" TraceBit ");

    ext->Out(L")");
}

uint HeapBlockHelper::GetObjectAlignmentMask()
{
    return GetObjectGranularity() - 1;
}

uint HeapBlockHelper::GetObjectGranularity()
{
    return 1u << GetObjectAllocationShift();
}

uint HeapBlockHelper::GetObjectAllocationShift()
{
    if (objectAllocationShift == 0)
    {
        try
        {
            auto firstSizeCat = ext->GetNumberValue<ULONG64>(recycler.Field("autoHeap.heapBuckets").ArrayElement(0).Field("heapBucket.sizeCat"));
            int i = 0;
            while (firstSizeCat > (ULONG64)(1 << (++i)));
            objectAllocationShift = i;
        }
        catch (ExtException&)
        {
            return 4;
        }
    }
    return objectAllocationShift;
}

bool HeapBlockHelper::IsAlignedAddress(ULONG64 address)
{
    return (0 == (((size_t)address) & GetObjectAlignmentMask()));
}

// Same logic as SmallHeapBlock::GetAddressBitIndex
// Ideally that would be static but it has an assert that uses the this pointer
ushort HeapBlockHelper::GetAddressSmallHeapBlockBitIndex(ULONG64 objectAddress)
{
    ushort offset = (ushort)(objectAddress % (SmallAllocationBlockAttributes::BitVectorCount * GetObjectGranularity()));

    offset = offset >> GetObjectAllocationShift();

    return offset;
}

void HeapBlockHelper::DumpLargeHeapBlockObject(ExtRemoteTyped& heapBlockObject, ULONG64 objectAddress, bool verbose)
{
    ULONG cookie = 0;
    if (recycler.HasField("Cookie"))
    {
        cookie = recycler.Field("Cookie").GetUlong();
    }

    ULONG64 heapBlock = heapBlockObject.GetPtr();
    ULONG64 blockAddress = heapBlockObject.Field("address").GetPtr();

    ULONG64 sizeOfHeapBlock = ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeHeapBlock))"));
    ULONG64 sizeOfObjectHeader = ext->EvalExprU64(ext->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sLargeObjectHeader))"));

    ULONG64 headerAddress = objectAddress - sizeOfObjectHeader;

    if (headerAddress < blockAddress)
    {
        ext->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
        DumpHeapBlockLink(ext->enum_LargeBlockType(), heapBlock);
        return;
    }

    ExtRemoteTyped largeObjectHeader(ext->FillModuleAndMemoryNS("%s!%sLargeObjectHeader"), headerAddress, false);

    HeapObject heapObject;
    heapObject.index = (ushort) largeObjectHeader.Field("objectIndex").m_Typed.Data; // Why does calling UShort not work?

    ULONG largeObjectHeaderPtrSize = ext->m_PtrSize;

    ULONG64 headerArrayAddress = heapBlock + sizeOfHeapBlock + (heapObject.index * largeObjectHeaderPtrSize);
    ExtRemoteData  headerData(headerArrayAddress, largeObjectHeaderPtrSize);

    if (headerData.GetPtr() != headerAddress)
    {
        ext->Out("Object with address 0x%p was not found in corresponding heap block\n", objectAddress);
        DumpHeapBlockLink(ext->enum_LargeBlockType(), heapBlock);
        ext->Out("Header address: 0x%p, Header in index %d is 0x%p\n", headerAddress, heapObject.index, headerData.GetPtr());
        return;
    }

    heapObject.address = objectAddress;
    heapObject.addressBitIndex = heapObject.index;
    heapObject.heapBlock = heapBlock;
    heapObject.heapBlockType = ext->enum_LargeBlockType();
    heapObject.objectInfoAddress = headerAddress + sizeof(uint)+sizeof(uint)+largeObjectHeaderPtrSize;
    if (largeObjectHeader.HasField("attributesAndChecksum"))
    {
        heapObject.objectInfoBits = (UCHAR)((largeObjectHeader.Field("attributesAndChecksum").GetUshort() ^ (USHORT)cookie) >> 8);
    }
    else
    {
        heapObject.objectInfoBits = largeObjectHeader.Field("attributes").GetUchar();
    }
    heapObject.objectSize = EXT_CLASS_BASE::GetSizeT(largeObjectHeader.Field("objectSize"));

    ULONG64 objectCount = EXT_CLASS_BASE::GetSizeT(heapBlockObject.Field("objectCount"));

    ExtRemoteTyped freeBitWord;
    heapObject.isFreeSet = (headerAddress >= blockAddress && heapObject.index < EXT_CLASS_BASE::GetSizeT(heapBlockObject.Field("allocCount")) && headerData.m_Data == NULL);
    heapObject.freeBitWord = NULL;

    ExtRemoteTyped markBitWord;
    ULONG64 markBitVector = (heapBlock + sizeOfHeapBlock + largeObjectHeaderPtrSize * objectCount);
    heapObject.isMarkSet = ext->TestFixed(markBitVector, heapObject.addressBitIndex, markBitWord);
    heapObject.markBitWord = markBitWord.GetPointerTo().GetPtr();

    ExtRemoteData heapObjectData(heapObject.address, this->ext->m_PtrSize);
    heapObject.vtable = heapObjectData.GetPtr();

    DumpHeapObject(heapObject, verbose);
}

void HeapBlockHelper::DumpHeapBlockLink(ULONG64 heapBlockType, ULONG64 heapBlock)
{
    if (heapBlockType == ext->enum_LargeBlockType())
    {
        PCSTR heapblockSymbol = ext->FillModuleAndMemoryNS("%s!%sLargeHeapBlock");
        ext->Dml("Large Heap Block: <link cmd=\"dt %s 0x%p\">0x%p</link>\n", heapblockSymbol, heapBlock, heapBlock);
    }
    else
    {
        PCSTR heapBlockTypeString = "unknown Heap Block";

        if (heapBlockType == ext->enum_SmallNormalBlockType())
            heapBlockTypeString = "Small Normal Heap Block";
        else if (heapBlockType == ext->enum_SmallLeafBlockType())
            heapBlockTypeString = "Small Leaf Block";
        else if (heapBlockType == ext->enum_SmallFinalizableBlockType())
            heapBlockTypeString = "Small Finalizable Block";
        else if (heapBlockType == ext->enum_SmallNormalBlockWithBarrierType())
            heapBlockTypeString = "Small Normal Block (with SWB)";
        else if (heapBlockType == ext->enum_SmallFinalizableBlockWithBarrierType())
            heapBlockTypeString = "Small Finalizable Block (with SWB)";
        else if (heapBlockType == ext->enum_MediumNormalBlockType())
            heapBlockTypeString = "Medium Normal Heap Block";
        else if (heapBlockType == ext->enum_MediumLeafBlockType())
            heapBlockTypeString = "Medium Leaf Block";
        else if (heapBlockType == ext->enum_MediumFinalizableBlockType())
            heapBlockTypeString = "Medium Finalizable Block";
        else if (heapBlockType == ext->enum_MediumNormalBlockWithBarrierType())
            heapBlockTypeString = "Medium Normal Block (with SWB)";
        else if (heapBlockType == ext->enum_MediumFinalizableBlockWithBarrierType())
            heapBlockTypeString = "Medium Finalizable Block (with SWB)";
        else
            Assert(false);

        if (recycler.GetPtr())
        {
            ext->Dml("%s: <link cmd=\"!showblockinfo 0x%p 0x%p\">0x%p</link>\n", heapBlockTypeString, heapBlock, recycler.GetPtr(), heapBlock);
        }
        else
        {
            ext->Dml("%s: <link cmd=\"!showblockinfo 0x%p\">0x%p</link>\n", heapBlockTypeString, heapBlock, heapBlock);
        }
    }
}

void HeapBlockHelper::DumpHeapObject(const HeapObject& heapObject, bool verbose)
{
    // DumpHeapBlockLink(heapObject.heapBlockType, heapObject.heapBlock);

    ext->Out(L"Object: ");
    std::string className = ext->GetTypeNameFromVTable(heapObject.vtable);

    if (!className.empty())
    {
        ext->Dml("<link cmd=\"dt %s 0x%p\">0x%p</link>", className.c_str(), heapObject.address, heapObject.address);
    }
    else
    {
        ext->Out(L"0x%p ", heapObject.address);
    }

    ext->Out(L" (Symbol @ 0x%p: ", heapObject.vtable);
    ext->m_Symbols3->OutputSymbolByOffset(DEBUG_OUTCTL_AMBIENT, DEBUG_OUTSYM_ALLOW_DISPLACEMENT, heapObject.vtable);
    ext->Out(")");

    ext->Out("\n");
    ext->Out(L"Object size: 0x%x\n", heapObject.objectSize);

    DumpObjectInfoBits(heapObject.objectInfoBits);
    ext->Out(" @0x%p\n", heapObject.objectInfoAddress);

    ext->Out("Object index: %d\n", heapObject.index);

    if (heapObject.heapBlockType == ext->enum_SmallLeafBlockType()
        || heapObject.heapBlockType == ext->enum_SmallNormalBlockType()
#ifdef RECYCLER_WRITE_BARRIER
        || heapObject.heapBlockType == ext->enum_SmallNormalBlockWithBarrierType()
#endif
        )
    {
        ext->Out(L"Address bit index: %d\n", heapObject.addressBitIndex);
    }

    if (verbose)
    {
        ext->Out("FreeBit: %d", heapObject.isFreeSet);
        if (!ext->IsMinidumpDebugging())
        {
            ext->Out(" (");
            ext->Dml("<link cmd=\"ba w1 0x%p\">Set Breakpoint</link>", heapObject.freeBitWord);
            ext->Out(")");
        }
        ext->Out("\n");

        ext->Out("MarkBit: %d", heapObject.isMarkSet);
        if (!ext->IsMinidumpDebugging())
        {
            ext->Out(" (");
            ext->Dml("<link cmd=\"ba w1 0x%p\">Set Breakpoint</link>", heapObject.markBitWord);
            ext->Out(")");
        }
        ext->Out("\n");
    }
}

void HeapBlockHelper::DumpSmallHeapBlockObject(ExtRemoteTyped& heapBlockObject, ULONG64 objectAddress, bool verbose)
{
    ULONG64 heapBlock = heapBlockObject.GetPtr();

    HeapObject heapObject;
    heapObject.heapBlock = heapBlock;
    heapObject.heapBlockType = heapBlockObject.Field("heapBlockType").GetChar();

    ULONG64 sizeOfHeapBlock = ext->EvalExprU64("@@c++(sizeof(SmallHeapBlock))");
    heapObject.objectSize = heapBlockObject.Field("objectSize").GetUshort();

    uint objectCount = (uint) heapBlockObject.Field("objectCount").m_Typed.Data; // Assume objectCount is 32 bit
    heapObject.index = GetSmallHeapBlockObjectIndex(heapBlockObject, objectAddress);
    if (heapObject.index != SmallHeapBlock::InvalidAddressBit)
    {
        heapObject.address = heapBlockObject.Field("address").GetPtr() + heapObject.index * heapObject.objectSize;

        ExtRemoteData heapObjectData(heapObject.address, ext->m_PtrSize);
        heapObject.vtable = heapObjectData.GetPtr();

        heapObject.addressBitIndex = GetAddressSmallHeapBlockBitIndex(heapObject.address);

        ExtRemoteTyped freeBitWord;
        ULONG64 freeBitVector = (heapBlock + sizeOfHeapBlock + Math::Align<ULONG>(sizeof(unsigned char)* objectCount, ext->m_PtrSize) + ext->GetBVFixedAllocSize(objectCount));
        heapObject.isFreeSet = ext->TestFixed(freeBitVector, heapObject.addressBitIndex, freeBitWord);
        heapObject.freeBitWord = freeBitWord.GetPointerTo().GetPtr();

        ExtRemoteTyped markBitWord;
        ULONG64 markBitVector = (heapBlock + sizeOfHeapBlock + Math::Align<ULONG>(sizeof(unsigned char)* objectCount, ext->m_PtrSize));
        heapObject.isMarkSet = ext->TestFixed(markBitVector, heapObject.addressBitIndex, markBitWord);
        heapObject.markBitWord = markBitWord.GetPointerTo().GetPtr();
        heapObject.objectInfoAddress = heapBlock - heapObject.index - 1;

        ExtRemoteData objectInfo(heapObject.objectInfoAddress, 1);
        heapObject.objectInfoBits = objectInfo.GetChar();
        DumpHeapObject(heapObject, verbose);
    }
    else
    {
        ext->Out("Pointer is not valid in this heap block");
    }
}

ExtRemoteTyped
EXT_CLASS_BASE::GetRecycler(ULONG64 optionalRecyclerAddress)
{
    ExtRemoteTyped recycler;

    if (optionalRecyclerAddress != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), optionalRecyclerAddress);
    }
    else
    {
        recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
    }

    return recycler;
}

size_t EXT_CLASS_BASE::GetBVFixedAllocSize(int length)
{
    return sizeof(BVFixed) + sizeof(BVUnit) * (((length - 1) >> BVUnit::ShiftValue) + 1);
}

#if defined(_M_X64)
    typedef UnitWord32 UnitWord;
#else
    typedef UnitWord64 UnitWord;
#endif

void
EXT_CLASS_BASE::DisplayLargeHeapBlockInfo(ExtRemoteTyped& largeHeapBlock)
{
    largeHeapBlock.OutFullValue();

    // TODO
}

void
EXT_CLASS_BASE::DisplaySmallHeapBlockInfo(ExtRemoteTyped& smallHeapBlock, ExtRemoteTyped recycler)
{
    HeapBlockHelper heapBlockHelper(this, recycler);
    smallHeapBlock.OutFullValue();

    ExtRemoteTyped heapBlockType = smallHeapBlock.Field("heapBlockType");
    auto type = heapBlockType.GetChar();

    if (type == this->enum_SmallFinalizableBlockType())
    {
        Out("Small finalizable block\n");
    }
    else if (type == this->enum_SmallLeafBlockType())
    {
        Out("Small leaf block\n");
    }
    else if (type == this->enum_SmallNormalBlockType())
    {
        Out("Small normal block\n");
    }
#ifdef RECYCLER_WRITE_BARRIER
    else if (type == this->enum_SmallNormalBlockWithBarrierType())
    {
        Out("Small normal block (with SWB)\n");
    }
    else if (type == this->enum_SmallFinalizableBlockWithBarrierType())
    {
        Out("Small finalizable block (with SWB)\n");
    }
#endif
    else if (type == this->enum_MediumFinalizableBlockType())
    {
        Out("Medium finalizable block\n");
    }
    else if (type == this->enum_MediumLeafBlockType())
    {
        Out("Medium leaf block\n");
    }
    else if (type == this->enum_MediumNormalBlockType())
    {
        Out("Medium normal block\n");
    }
#ifdef RECYCLER_WRITE_BARRIER
    else if (type == this->enum_MediumNormalBlockWithBarrierType())
    {
        Out("Medium normal block (with SWB)\n");
    }
    else if (type == this->enum_MediumFinalizableBlockWithBarrierType())
    {
        Out("Medium finalizable block (with SWB)\n");
    }
#endif
    else
    {
        Out("Unexpected heapblock type: 0x%x\n", type);
        return;
    }

    ushort objectSize = smallHeapBlock.Field("objectSize").GetUshort();
    uint bucketIndex = (objectSize >> heapBlockHelper.GetObjectAllocationShift()) - 1;
    uint objectGranularity = heapBlockHelper.GetObjectGranularity();
    uint objectBitDelta = objectSize / objectGranularity;

    Out("Object size: %u\n", objectSize);
    Out("Object allocation shift: %u\n", heapBlockHelper.GetObjectAllocationShift());
    Out("Bucket index: %u\n", bucketIndex);
    Out("Object bit delta: %u\n", objectBitDelta);

    ExtRemoteTyped validPointersMap(FillModuleAndMemoryNS(
        strstr(heapBlockType.GetSimpleValue(), "Medium") == nullptr ?
        "%s!%sHeapInfo::mediumAllocValidPointersMap" : "%s!%sHeapInfo::smallAllocValidPointersMap"
        ));

    ExtRemoteTyped invalidBitBuffersPtr;
    ExtRemoteTyped invalidBitBuffers;
    try
    {
        invalidBitBuffersPtr = validPointersMap.Field("invalidBitsBuffers");
        invalidBitBuffers = invalidBitBuffersPtr.Dereference();
    }
    catch (ExtRemoteException&)
    {
        invalidBitBuffers = validPointersMap.Field("invalidBitsData");
    }
    ExtRemoteTyped bitVector = invalidBitBuffers.ArrayElement(bucketIndex);
    ExtRemoteTyped dataPtr;
    uint wordCount = 2;
    if (bitVector.HasField("data"))
    {
        dataPtr = bitVector.Field("data");
        if (m_PtrSize == 4)
        {
            wordCount = 8;
        }
    }
    else
    {
        dataPtr = bitVector;
        wordCount = dataPtr.GetTypeSize() / dataPtr.ArrayElement(0).GetTypeSize();
    }

    Out("Invalid bit vector: ");
    for (uint i = 0; i < wordCount; i++)
    {
        ExtRemoteTyped word = dataPtr.ArrayElement(i).Field("word");

        if (m_PtrSize == 4)
        {
            Out("%p ", word.GetUlong());
        }
        else
        {
            Out("%p ", word.GetUlong64());
        }
    }

    Out("\n");
}


bool EXT_CLASS_BASE::TestFixed(ULONG64 bitVector, int index, ExtRemoteTyped& bvUnit)
{
    ExtRemoteTyped bitVectorType(FillModule("(%s!BVFixed*) @$extin"), bitVector);

    // Out("Length is %d\n", bitVectorType.Field("len").m_Typed.Data);

    ExtRemoteTyped data = bitVectorType.Field("data");
    bvUnit = data[(LONG) BVUnit::Position(index)];
    int offset = BVUnit::Offset(index);

    UnitWord word = (UnitWord) bvUnit.Field("word").m_Typed.Data;

    return (word & ((UnitWord) 1 << offset)) != 0;
}

// GC Debugger commands
JD_PRIVATE_COMMAND(recycler,
    "Dumps the given recycler or the recycler in the current thread context",
    "{;e,o,d=0;recycler;Recycler address}{webjd;b;;Output in WebJD format}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    ExtRemoteTyped recycler = GetRecycler(arg);
    BOOL webJd = HasArg("webjd");

    if (webJd)
    {
        Out("%p", recycler.GetPtr());
    }
    else
    {
        recycler.OutFullValue();
    }
}

JD_PRIVATE_COMMAND(showblockinfo,
    "Show heap block information",
    "{;e,r;address;Address of the heap block}{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG64 arg = GetUnnamedArgU64(1);
    ExtRemoteTyped recycler;
    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
    }
    else
    {
        recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
    }

    if (address != NULL)
    {
        ExtRemoteTyped heapBlock(FillModuleAndMemoryNS("(%s!%sHeapBlock*)@$extin"), address);
        heapBlock = CastWithVtable(heapBlock);
        ExtRemoteTyped heapBlockType = heapBlock.Field("heapBlockType");
        auto type = heapBlockType.GetChar();

        if (type == this->enum_LargeBlockType())
        {
            DisplayLargeHeapBlockInfo(heapBlock);
        }
        else
        {
            DisplaySmallHeapBlockInfo(heapBlock, recycler);
        }
    }
}

JD_PRIVATE_COMMAND(findblock,
    "Find recycler heap block",
    "{;e,r;address;Address to find heap block for}{;e,o,d=0;recycler;Recycler address}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG64 arg = GetUnnamedArgU64(1);
    ExtRemoteTyped recycler;
    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
    }
    else
    {
        recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
    }
    HeapBlockHelper helper(this, recycler);
    ULONG64 heapBlock = helper.FindHeapBlock(address, recycler);

    if (heapBlock != NULL)
    {
        ExtRemoteTyped heapBlock(FillModuleAndMemoryNS("%s!%sHeapBlock"), heapBlock, false);
        helper.GetHeapBlockType(heapBlock);
        heapBlock.OutFullValue();
    }
    else
    {
        Out("Could not find heap block corresponding to address\n");
    }
}



void EXT_CLASS_BASE::DisplaySegmentList(PCSTR strListName, ExtRemoteTyped segmentList, PageAllocatorStats& stats, CommandOutputType outputType, bool pageSegment)
{
    ULONG64 segmentListAddress = segmentList.GetPointerTo().GetPtr();
    PCSTR segmentType = (pageSegment ? GetPageSegmentType() : GetSegmentType());
    RemoteListIterator<false> pageSegmentListIterator(segmentType, segmentListAddress);

    ULONG64 totalSize = 0;
    ULONG64 count = 0;

    if (outputType == CommandOutputType::NormalOutputType ||
        outputType == CommandOutputType::VerboseOutputType)
    {
        Out("%s\n", strListName);
    }

    while (pageSegmentListIterator.Next())
    {
        count++;
        ExtRemoteTyped segment = pageSegmentListIterator.Data();
        ULONG64 addressOfSegment = pageSegmentListIterator.GetDataPtr();

        ULONG64 address = segment.Field("address").GetPtr();
        ULONG64 segmentSize = EXT_CLASS_BASE::GetSizeT(segment.Field("segmentPageCount")) * 4096;
        totalSize += segmentSize;

        if (outputType == CommandOutputType::NormalOutputType ||
            outputType == CommandOutputType::VerboseOutputType)
        {
            if (pageSegment)
            {
                PCSTR fullyQualifiedSegmentType = FillModuleV("%s!%s",GetModuleName(), GetPageSegmentType());
                Dml("<link cmd=\"?? (%s*) 0x%p\">PageSegment</link>: ",
                    fullyQualifiedSegmentType, addressOfSegment);
            }
            else
            {
                PCSTR fullyQualifiedSegmentType = FillModuleV("%s!%s", GetModuleName(), GetSegmentType());
                Dml("<link cmd=\"?? (%s*) 0x%p\">Segment</link>: ",
                    fullyQualifiedSegmentType, addressOfSegment);
            }
            Out("(%p - %p)\n", address, address + segmentSize);
        }
    }

    if (outputType == CommandOutputType::SummaryOutputType)
    {
        Out("%s: %11I64u %11I64u\n", strListName, count, totalSize);
    }
    stats.count += count;
    stats.totalByteCount += totalSize;
}

void EXT_CLASS_BASE::DisplayPageAllocatorInfo(ExtRemoteTyped pageAllocator, CommandOutputType outputType)
{
    pageAllocator = CastWithVtable(pageAllocator);
    Out("Page Allocator: 0x%x\n", pageAllocator.m_Offset);

    if (outputType == CommandOutputType::SummaryOutputType)
    {
        Out("-----------------------------------------\n");
        Out("Type                     Count       Size\n");
        Out("-----------------------------------------\n");
    }

    PageAllocatorStats stats = { 0 };
    DisplaySegmentList("Segments        ", pageAllocator.Field("segments"), stats, outputType);
    DisplaySegmentList("FullSegments    ", pageAllocator.Field("fullSegments"), stats, outputType);
    DisplaySegmentList("EmptySegments   ", pageAllocator.Field("emptySegments"), stats, outputType);
    DisplaySegmentList("DecommitSegments", pageAllocator.Field("decommitSegments"), stats, outputType);
    DisplaySegmentList("LargeSegments   ", pageAllocator.Field("largeSegments"), stats, outputType);
    if (outputType == CommandOutputType::SummaryOutputType)
    {
        Out("----------------------------------------\n");
        Out("Total           : %11I64u %11I64u\n", stats.count, stats.totalByteCount);
    }

    if (pageAllocator.HasField("zeroPageQueue"))
    {
        ExtRemoteTyped zeroPageQueue = pageAllocator.Field("zeroPageQueue");
        int count = 0;
        int pageCount = 0;
        ULONG64 freePageList = zeroPageQueue.Field("freePageList").GetPointerTo().GetPtr();
        ULONG64 alignment = 0;

        if (m_PtrSize == 8)
        {
            alignment = 8;
        }

        if (freePageList != 0)
        {
            RemoteListIterator<true> iter(this->FillModuleV("%s::FreePageEntry", this->GetPageAllocatorType()), freePageList + alignment);

            while (iter.Next())
            {
                // Operating on the data ptr directly because the symbol for PageAllocator::FreePageEntry
                // includes slist header which is not we want (we want just the data, not the list metadata)
                ULONG64 offsetOfPageCount = iter.GetDataPtr() + this->m_PtrSize;
                ExtRemoteData pageCountData(offsetOfPageCount, sizeof(uint));

                ulong pageCountValue = pageCountData.GetUlong();

                pageCount += pageCountValue;
                count++;
            }
        }

        Out("\nFree page list: %d entries (%d bytes)\n", count, (pageCount * 4096));

        count = 0;
        pageCount = 0;
        ULONG64 pendingZeroPageList = zeroPageQueue.Field("pendingZeroPageList").GetPointerTo().GetPtr();

        if (pendingZeroPageList != 0)
        {
            RemoteListIterator<true> iter(this->FillModuleV("%s::FreePageEntry", this->GetPageAllocatorType()), pendingZeroPageList + alignment);

            while (iter.Next())
            {
                ULONG64 offsetOfPageCount = iter.GetDataPtr() + this->m_PtrSize;
                ExtRemoteData pageCountData(offsetOfPageCount, sizeof(uint));

                ulong pageCountValue = pageCountData.GetUlong();

                pageCount += pageCountValue;
                count++;
            }
        }
        Out("Pending Zero page list: %d entries (%d bytes)\n", count, (pageCount * 4096));
    }
}

JD_PRIVATE_COMMAND(pagealloc,
    "Display information about a page allocator",
    "{;e,o,d=0;alloc;Page allocator address}"
    "{recycler;b;recycler;Display the recycler's page allocator}"
    "{webjd;b;;Output in WebJD format}"
    "{summary;b;;Display just a summary}"
    "{verbose;b;;Display verbose output}")
{
    if (!HasUnnamedArg(0) && !HasArg("recycler"))
    {
        Out("Usage: !pagealloc [page allocator address] | !pagealloc -recycler [recycler address]\n");
        return;
    }

    ExtRemoteTyped pageAllocator;

    if (HasArg("recycler"))
    {
        ExtRemoteTyped recycler;
        if (!HasUnnamedArg(0) && GetUnnamedArgU64(0) != 0)
        {
            Out("Recycler is 0x%p\n", GetUnnamedArgU64(0));
            recycler = ExtRemoteTyped(FillModuleV("(%s!%s*)@$extin", this->GetModuleName(), this->GetPageAllocatorType()), GetUnnamedArgU64(0));
        }
        else
        {
            recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
        }
        pageAllocator = recycler.Field("recyclerPageAllocator");
    }
    else
    {
        Out("Allocator is 0x%p\n", GetUnnamedArgU64(0));

        pageAllocator = ExtRemoteTyped(FillModuleV("(%s!%s*)@$extin", this->GetModuleName(), this->GetPageAllocatorType()), GetUnnamedArgU64(0));
    }

    CommandOutputType outputType = NormalOutputType;

    if (HasArg("summary"))
    {
        outputType = SummaryOutputType;
    }
    else if (HasArg("webjd"))
    {
        outputType = WebJdOutputType;
    }
    else if (HasArg("verbose"))
    {
        outputType = VerboseOutputType;
    }

    DisplayPageAllocatorInfo(pageAllocator, outputType);
}

#pragma region("Heap Block Map Walker")
class HeapBlockMapWalkerImpl : public HeapBlockMapWalker
{
public:
    HeapBlockMapWalkerImpl(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, PCSTR execStr, bool filter) :
        HeapBlockMapWalker(ext, recycler),
        largeCount(0),
        smallCount(0),
        largeBlockBytes(0),
        smallBlockBytes(0),
        execStr(execStr),
        filter(filter)
    {
    }

    virtual bool ProcessHeapBlock(size_t l1Id, size_t l2Id, ULONG64 blockAddress, ExtRemoteTyped block) override
    {
        ProcessBlock(blockAddress, block);
        return false;
    }

    virtual bool ProcessLargeHeapBlock(size_t l1Id, size_t l2Id, ULONG64 blockAddress, ExtRemoteTyped block) override
    {
        ProcessBlock(blockAddress, block);
        return false;
    }

    void PrintSummary()
    {
        ext->Out("----------------------------\n");
        ext->Out("Type       Count       Bytes\n");
        ext->Out("----------------------------\n");
        ext->Out("Large: %7u %11I64u\n", largeCount, largeBlockBytes);
        ext->Out("Small: %7u %11I64u\n", smallCount, smallBlockBytes);
        ext->Out("----------------------------\n");
        ext->Out("Total: %7u %11I64u\n", smallCount + largeCount, largeBlockBytes + smallBlockBytes);
    }

private:
    void ProcessBlock(ULONG64 block, ExtRemoteTyped heapBlock)
    {
        auto type = heapBlock.Field("heapBlockType").GetChar();

        if (type == ext->enum_LargeBlockType())
        {
            ExtRemoteTyped largeBlock(ext->FillModuleAndMemoryNS("(%s!%sLargeHeapBlock*)@$extin"), block);
            largeBlockBytes += (4096 * EXT_CLASS_BASE::GetSizeT(largeBlock.Field("pageCount")));
            // Out("Large block: 0x%x\n", heapBlock.GetPtr());
            largeCount++;
        }
        else
        {
            smallBlockBytes += 4096;
            smallCount++;
        }

        if (execStr != nullptr)
        {
            CHAR    buffer[20] = "0x";
            std::string cmd = execStr;

            _ui64toa_s(block, buffer + 2, _countof(buffer) - 2, 16);
            ReplacePlaceHolders("%1", buffer, cmd);

            DEBUG_VALUE value = { 0 };
            ULONG rem;

            ext->ThrowInterrupt();
            if (filter)
            {
                if (SUCCEEDED(ext->m_Control->Evaluate(cmd.c_str(), DEBUG_VALUE_INT64, &value, &rem) != 0) &&
                    value.I64 != 0)
                {
                    ext->Out("Matched: 0x%x\n", block);
                }
            }
            else
            {
                ext->m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS, cmd.c_str(), 0);
            }
        }
    }

private:
    int largeCount;
    int smallCount;
    ULONG64 largeBlockBytes;
    ULONG64 smallBlockBytes;
    bool filter;
    PCSTR execStr;
};

JD_PRIVATE_COMMAND(hbm,
    "Display information about the heap block map",
    "{recycler;e,o,d=0;recycler;Recycler address}"
    "{webjd;b;;Output in WebJD format}"
    "{match;x,o;;Command to filter against}"
    "{exec;x,o;;Command to execute}")
{
    ULONG64 recyclerAddress = GetArgU64("recycler");
    ExtRemoteTyped recycler;

    if (recyclerAddress != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recyclerAddress);
    }
    else
    {
        recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
    }

    PCSTR execStr = nullptr;
    bool filter = false;
    if (HasArg("match"))
    {
        execStr = GetArgStr("match");
        filter = true;
    }
    else if (HasArg("exec"))
    {
        execStr = GetArgStr("exec");
    }

    AutoCppExpressionSyntax cppSyntax(m_Control5);
    HeapBlockMapWalkerImpl walker(this, recycler, execStr, filter);
    walker.Run();

    if (!HasArg("exec") && !HasArg("match"))
    {
        walker.PrintSummary();
    }
}

JD_PRIVATE_COMMAND(swb,
    "Find the software write barrier bit for an address",
    "{;e,r;address;Address of object to look up}")
{
    ULONG64 objectAddress = GetUnnamedArgU64(0);

    if (this->m_PtrSize == 4) {
        Nullable<bool> usesSoftwareWriteBarrier;

        DetectFeatureBySymbol(usesSoftwareWriteBarrier, FillModuleAndMemoryNS("%s!%sRecyclerWriteBarrierManager::cardTable"));

        if (usesSoftwareWriteBarrier == false) {
            Out("Target process is not using Software Write Barrier (or symbols are incorrect)\n");
            return;
        }

        ExtRemoteTyped cardTable = ExtRemoteTyped(FillModuleAndMemoryNS("%s!%sRecyclerWriteBarrierManager::cardTable"));
        ULONG64 bytesPerCardOffset = 0;

        if (FAILED(this->m_Symbols->GetOffsetByName(FillModuleAndMemoryNS("%s!%sRecyclerWriteBarrierManager::s_BytesPerCard"), &bytesPerCardOffset)))
        {
            Out("Error resolving RecyclerWriteBarrierManager::s_BytesPerCard\n");
            return;
        }

        uint bytesPerCard = 0;
        ULONG bytesRead = 0;
        HRESULT hr = S_FALSE;
        if (FAILED(hr = this->m_Data->ReadVirtual(bytesPerCardOffset, (void*)&bytesPerCard, sizeof(bytesPerCard), &bytesRead)))
        {
            Out("Failed to read from 0x%p, HR = 0x%x\n", bytesPerCardOffset, hr);
            return;
        }

        uint cardIndex = ((uint) objectAddress) / bytesPerCard;
        BYTE value = cardTable.ArrayElement(cardIndex).GetChar();

        Out("Card Index: %d\nDirty: %d\n", cardIndex, value);
    }
    else
    {
        Out("Write barriers not implemented for 64 bit yet\n");
    }
}

JD_PRIVATE_COMMAND(hbstats,
    "Count recycler objects",
    "{;e,o,d=0;recycler;Recycler address}"
    "{summary;b,o;;Display only a summary}"
    "{filter;s,o;type;Filter the output to either alloc (PageAllocator) or buckets (Heap Buckets)}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    PrintBucketStatsFilter filter = StatsFilterBuckets;

    if (this->HasArg("summary"))
    {
        filter = StatsFilterSummary;
    }

    if (this->HasArg("filter"))
    {
        PCSTR arg = this->GetArgStr("filter");

        if (_stricmp("alloc", arg) == 0)
        {
            filter = StatsFilterPageAllocator;
        }
        else if (_stricmp("buckets", arg) == 0)
        {
            filter = StatsFilterBuckets;
        }
        else
        {
            Out("Invalid argument: %s\n", arg);
            return;
        }
    }

    ExtRemoteTyped recycler;
    if (arg != 0)
    {
        recycler = ExtRemoteTyped(FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), arg);
    }
    else
    {
        recycler = RemoteThreadContext::GetCurrentThreadContext().GetRecycler().GetExtRemoteTyped();
    }

    // If we're only interested in the page allocator data, don't bother collecting bucket stats
    if (filter == StatsFilterBuckets || filter == StatsFilterSummary)
    {
        PrintHeapBlockStats collect(this, recycler);
        collect.Run();
        Out("\n");
    }
}

void DisplayArenaAllocatorDataHeader()
{

    ExtOut("-------------------------------------------------------------------------------------\n");
    ExtOut("Allocator     #Block       Total        Used      Unused    Overhead OverHead%% Unused%%\n");
    ExtOut("-------------------------------------------------------------------------------------\n");

}

struct ArenaAllocatorData
{
    ULONG64 blockSize;
    ULONG64 blockUsedSize;
    ULONG64 overheadSize;
    uint blockCount;
};

void AccumulateArenaAllocatorData(ExtRemoteTyped arenaAllocator, ArenaAllocatorData& data)
{
    ULONG64 sizeofBigBlockHeader = GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sBigBlock))"));
    ULONG64 sizeofArenaMemoryBlockHeader = GetExtension()->EvalExprU64(GetExtension()->FillModuleAndMemoryNS("@@c++(sizeof(%s!%sArenaMemoryBlock))"));
    auto arenaMemoryBlockFn = [&](ExtRemoteTyped mallocBlock)
    {
        data.overheadSize += sizeofArenaMemoryBlockHeader;
        data.blockCount++;
        ULONG64 nbytes = EXT_CLASS_BASE::GetSizeT(mallocBlock.Field("nbytes"));
        data.blockSize += nbytes;
        data.blockUsedSize += nbytes;
        return false;
    };
    auto bigBlockFn = [&](ExtRemoteTyped bigBlock)
    {
        data.overheadSize += sizeofBigBlockHeader;
        data.blockCount++;
        data.blockSize += EXT_CLASS_BASE::GetSizeT(bigBlock.Field("nbytes"));
        data.blockUsedSize += EXT_CLASS_BASE::GetSizeT(bigBlock.Field("currentByte"));
        return false;
    };
    ExtRemoteTyped firstBigBlock = arenaAllocator.Field("bigBlocks");
    if (firstBigBlock.GetPtr())
    {
        data.blockUsedSize +=
            arenaAllocator.Field("cacheBlockCurrent").GetPtr() - firstBigBlock.GetPtr() - sizeofBigBlockHeader
            - EXT_CLASS_BASE::GetSizeT(firstBigBlock.Field("currentByte"));
    }
    LinkListForEach(firstBigBlock, "nextBigBlock", bigBlockFn);
    LinkListForEach(arenaAllocator.Field("fullBlocks"), "nextBigBlock", bigBlockFn);
    LinkListForEach(arenaAllocator.Field("mallocBlocks"), "next", arenaMemoryBlockFn);
}

void DisplayArenaAllocatorData(char const * name, ExtRemoteTyped arenaAllocator, bool showZeroEntries)
{
    ArenaAllocatorData data = { 0 };
    AccumulateArenaAllocatorData(arenaAllocator, data);

    if (showZeroEntries || data.blockCount != 0)
    {
        ULONG64 unused = data.blockSize - data.blockUsedSize;
        ULONG64 totalSize = data.blockSize + data.overheadSize;
        ExtOut("%-15s: %3u %11I64u %11I64u %11I64u %11I64u %6.2f%% %6.2f%%\n", name, data.blockCount, totalSize, data.blockUsedSize, unused, data.overheadSize,
            100.0 * (data.overheadSize / (double)data.blockSize), 100.0 * (unused / (double)data.blockSize));
    }
}

void DisplayArenaAllocatorPtrData(char const * name, ExtRemoteTyped arenaAllocator, bool showZeroEntries)
{
    if (arenaAllocator.GetPtr() == 0) { return; }
    DisplayArenaAllocatorData(name, arenaAllocator, showZeroEntries);
}

JD_PRIVATE_COMMAND(memstats,
    "All memory stats",
    "{all;b,o;;Display all information}"
    "{a;b,o;;Display arena allocator information}"
    "{p;b,o;;Display page allocator information}"
    "{t;e,o,d=0;Display information for thread context}"
    "{z;b,o;;Display zero entries}")
{


    ULONG64 threadContextAddress = GetArgU64("t");
    bool showZeroEntries = this->HasArg("z");
    bool showAll = this->HasArg("all");
    bool showArenaAllocator = showAll || this->HasArg("a");
    bool showPageAllocator = showAll || this->HasArg("p") || (threadContextAddress && !showArenaAllocator);
    bool showThreadSummary = (!showArenaAllocator && !showPageAllocator);
    ULONG numThreads = 0;
    ULONG64 totalReservedBytes = 0;
    ULONG64 totalCommittedBytes = 0;
    ULONG64 totalUsedBytes = 0;
    if (showThreadSummary || !threadContextAddress)
    {
        ExtRemoteTyped totalUsedBytes(this->FillModule("%s!totalUsedBytes"));
        this->Out("Page Allocator Total Used Bytes: %u\n", EXT_CLASS_BASE::GetSizeT(totalUsedBytes));
    }
    if (showThreadSummary)
    {
        RemotePageAllocator::DisplayDataHeader("Thread Context");
    }
    RemoteThreadContext::ForEach([=, &numThreads, &totalReservedBytes, &totalCommittedBytes, &totalUsedBytes](RemoteThreadContext threadContext)
    {
        numThreads++;

        ULONG64 threadContextPtr = threadContext.GetExtRemoteTyped().GetPtr();
        if (threadContextAddress && threadContextAddress != threadContextPtr)
        {
            return false;  // continue iterating
        }

        ulong threadContextSystemThreadId = threadContext.GetThreadId();
        ulong threadContextThreadId = 0;

        HRESULT hr = this->m_System4->GetThreadIdBySystemId(threadContextSystemThreadId, &threadContextThreadId);

        if (showPageAllocator || showArenaAllocator)
        {
            if (SUCCEEDED(hr))
            {
                this->Dml("Thread context: %p <link cmd=\"~%us\">(Switch To Thread)</link>\n", threadContextPtr, threadContextThreadId);
            }
            else
            {
                this->Out("Thread context: %p\n", threadContextPtr);
            }
        }
        if (showPageAllocator)
        {
            this->Out("Page Allocators:\n");
            RemotePageAllocator::DisplayDataHeader("Allocator");
        }

        ULONG64 reservedBytes = 0;
        ULONG64 committedBytes = 0;
        ULONG64 usedBytes = 0;
        threadContext.ForEachPageAllocator([=, &reservedBytes, &committedBytes, &usedBytes](PCSTR name, RemotePageAllocator pageAllocator)
        {
            reservedBytes += pageAllocator.GetReservedBytes();
            committedBytes += pageAllocator.GetCommittedBytes();
            usedBytes += pageAllocator.GetUsedBytes();

            if (showPageAllocator)
            {
                pageAllocator.DisplayData(name, showZeroEntries);
            }
            return false;
        });

        totalReservedBytes += reservedBytes;
        totalCommittedBytes += committedBytes;
        totalUsedBytes += usedBytes;

        if (showPageAllocator && !showThreadSummary)
        {
            RemotePageAllocator::DisplayDataLine();
        }
        if (showPageAllocator || showThreadSummary)
        {
            g_Ext->Dml("<link cmd=\"!jd.memstats -tc %p\">%016p</link>", threadContextPtr, threadContextPtr);
            RemotePageAllocator::DisplayData(16, usedBytes, reservedBytes, committedBytes);
        }

        if (showArenaAllocator)
        {
            ExtRemoteTyped threadContextExtRemoteTyped = threadContext.GetExtRemoteTyped();
            this->Out("Arena Allocators:\n");
            DisplayArenaAllocatorDataHeader();
            DisplayArenaAllocatorData("TC", threadContextExtRemoteTyped.Field("threadAlloc"), showZeroEntries);
            DisplayArenaAllocatorData("TC-InlineCache", threadContextExtRemoteTyped.Field("inlineCacheThreadInfoAllocator"), showZeroEntries);
            if (threadContextExtRemoteTyped.HasField("isInstInlineCacheThreadInfoAllocator"))
            {
                // IE11 don't have this arena allocator
                DisplayArenaAllocatorData("TC-IsInstIC", threadContextExtRemoteTyped.Field("isInstInlineCacheThreadInfoAllocator"), showZeroEntries);
            }
            DisplayArenaAllocatorData("TC-ProtoChain", threadContextExtRemoteTyped.Field("prototypeChainEnsuredToHaveOnlyWritableDataPropertiesAllocator"), showZeroEntries);

            threadContext.ForEachScriptContext([showZeroEntries](ExtRemoteTyped scriptContext)
            {
                DisplayArenaAllocatorData("SC", scriptContext.Field("generalAllocator"), showZeroEntries);
                DisplayArenaAllocatorData("SC-DynamicProfile", scriptContext.Field("dynamicProfileInfoAllocator"), showZeroEntries);
                DisplayArenaAllocatorData("SC-InlineCache", scriptContext.Field("inlineCacheAllocator"), showZeroEntries);
                if (scriptContext.HasField("isInstInlineCacheAllocator"))
                {
                    // IE11 don't have this arena allocator
                    DisplayArenaAllocatorData("SC-IsInstIC", scriptContext.Field("isInstInlineCacheAllocator"), showZeroEntries);
                }
                DisplayArenaAllocatorPtrData("SC-Interpreter", scriptContext.Field("interpreterArena"), showZeroEntries);
                DisplayArenaAllocatorPtrData("SC-Guest", scriptContext.Field("guestArena"), showZeroEntries);
                DisplayArenaAllocatorPtrData("SC-Diag", scriptContext.Field("diagnosticArena"), showZeroEntries);

                if (scriptContext.HasField("sourceCodeAllocator"))
                {
                    DisplayArenaAllocatorData("SC-SourceCode", scriptContext.Field("sourceCodeAllocator"), showZeroEntries);
                }
                if (scriptContext.HasField("regexAllocator"))
                {
                    DisplayArenaAllocatorData("SC-Regex", scriptContext.Field("regexAllocator"), showZeroEntries);
                }
                if (scriptContext.HasField("miscAllocator"))
                {
                    DisplayArenaAllocatorData("SC-Misc", scriptContext.Field("miscAllocator"), showZeroEntries);
                }

                ExtRemoteTyped nativeCodeGen = scriptContext.Field("nativeCodeGen");

                auto forEachCodeGenAllocatorArenaAllocator = [showZeroEntries](ExtRemoteTyped codeGenAllocators)
                {
                    if (codeGenAllocators.GetPtr() == 0) { return; }

                    DisplayArenaAllocatorData("SC-BGJIT", codeGenAllocators.Field("allocator"), showZeroEntries);
                };
                forEachCodeGenAllocatorArenaAllocator(nativeCodeGen.Field("foregroundAllocators"));
                forEachCodeGenAllocatorArenaAllocator(nativeCodeGen.Field("backgroundAllocators"));
                return false;
            });
        }

        return false; // Don't stop iterating
    });

    if (!showArenaAllocator && numThreads > 1)
    {
        RemotePageAllocator::DisplayDataLine();
        this->Out("Total");
        RemotePageAllocator::DisplayData(_countof("Total") - 1, totalUsedBytes, totalReservedBytes, totalCommittedBytes);
    }
}

ExtRemoteTyped EXT_CLASS_BASE::CastWithVtable(ExtRemoteTyped original, std::string* typeName)
{
    static std::map<ULONG64, ULONG> vtableTypeIdMap;
    static std::map<ULONG64, std::string> vtableTypeNameMap;

    if (original.m_Typed.Tag != SymTagPointerType)
    {
        original = original.GetPointerTo();
    }

    ULONG64 vtbleAddr = ExtRemoteData(original.GetPtr(), this->m_PtrSize).GetPtr();
    if (vtbleAddr == 0)
    {
        return original;
    }
    ExtRemoteTyped result = original;

    if (typeName && vtableTypeNameMap.find(vtbleAddr) != vtableTypeNameMap.end())
    {
        *typeName = vtableTypeNameMap[vtbleAddr];
    }

    if (vtableTypeIdMap.find(vtbleAddr) != vtableTypeIdMap.end())
    {
        result.Set(true, original.m_Typed.ModBase, vtableTypeIdMap[vtbleAddr], original.GetPtr());
    }
    else
    {
        ExtBuffer<char> vtableName;
        if (this->GetOffsetSymbol(vtbleAddr, &vtableName))
        {
            int len = (int)(strlen(vtableName.GetBuffer()) - strlen("::`vftable'"));
            if (len > 0 && strcmp(vtableName.GetBuffer() + len, "::`vftable'") == 0)
            {
                vtableName.GetBuffer()[len] = '\0';
                if (typeName)
                {
                    *typeName = vtableName.GetBuffer();
                }

                vtableTypeNameMap[vtbleAddr] = vtableName.GetBuffer();

                ULONG typeId;
                this->m_Symbols3->GetTypeId(original.m_Typed.ModBase, vtableName.GetBuffer(), &typeId);

                result.Set(true, original.m_Typed.ModBase, typeId, original.GetPtr());
                vtableTypeIdMap[vtbleAddr] = typeId;
            }
        }
    }
    return result;
}

typedef struct _OBJECTINFO
{
    enum
    {
        freed,
        rooted,
        unrooted
    } state = freed;
    ULONG64 heapEntry = 0;
    ULONG64 userPtr = 0;
    ULONG64 userSize = 0;
    UCHAR attributes = 0;
    ExtRemoteTyped heapBlock;
    bool succeeded = false;
    std::string message;
    std::string typeName;
    ULONG64 x64MapAddr = 0;
} OBJECTINFO;

OBJECTINFO GetObjectInfo(ULONG64 address, ExtRemoteTyped recycler, EXT_CLASS_BASE* ext)
{
    OBJECTINFO info;
    HeapBlockHelper helper(ext, recycler);
    ExtRemoteTyped& heapBlock = info.heapBlock;
    heapBlock = helper.FindHeapBlockTyped(address, recycler, false, &info.x64MapAddr);
    if (heapBlock.GetPtr() == 0)
    {
        info.message = "Could not find heap block corresponding to this address";
        return info;
    }

    std::string& typeName = info.typeName;
    heapBlock = ext->CastWithVtable(heapBlock, &typeName);

    ExtRemoteTyped heapBlockType = heapBlock.Field("heapBlockType");
    auto typeEnumName = heapBlockType.GetSimpleValue();
    if (strstr(typeEnumName, "Large") == typeEnumName)
    {
        if (strstr(typeName.c_str(), "LargeHeapBlock") == nullptr)
        {
            info.message = "not a valid large block.";
            return info;
        }

        ULONG64 headerListAddress = heapBlock.GetPtr() + heapBlock.GetTypeSize();
        ULONG objectCount = heapBlock.Field("objectCount").GetUlong();
        ExtRemoteTyped headerList(ext->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader**)@$extin"), headerListAddress);

        bool foundInfo = false;
        ExtRemoteTyped header;
        for (ULONG i = 0; i < objectCount; i++)
        {
            header = headerList.ArrayElement(i);
            info.heapEntry = header.GetPtr();
            info.userPtr = info.heapEntry + header.GetTypeSize();
            info.userSize = ext->GetNumberValue<ULONG64>(header.Field("objectSize"));

            if (address >= info.userPtr  && address < info.userPtr + info.userSize)
            {
                ushort attributesAndChecksum = header.Field("attributesAndChecksum").GetUshort();
                info.attributes = (UCHAR)((attributesAndChecksum ^ (USHORT)recycler.Field("Cookie").GetUlong()) >> 8);
                foundInfo = true;
                break;
            }
        }

        if (!foundInfo)
        {
            info.message = "fatal error: target address not found on the large heap block object header list.";
            return info;
        }
    }
    else
    {
        auto blockAddress = heapBlock.Field("address").GetPtr();
        auto objectSize = ext->GetNumberValue<ULONG64>(heapBlock.Field("objectSize"));
        info.userPtr = address - ((address - blockAddress) % objectSize);
        ULONG64 index = (address - blockAddress) / objectSize;
        Assert(index < USHORT_MAX);

        if (strstr(typeEnumName, "Small") == typeEnumName)
        {
            if (strstr(typeName.c_str(), "Small") == nullptr)
            {
                info.message = "not a valid small block.";
                return info;
            }
        }
        else if (strstr(typeEnumName, "Medium") == typeEnumName)
        {
            if (strstr(typeName.c_str(), "Medium") == nullptr)
            {
                info.message = "not a valid Medium block.";
                return info;
            }
        }
        else
        {
            info.message = ext->FillModuleV("Can't handle HeapBlockType: %s", typeEnumName);
            return info;
        }

        info.attributes = ExtRemoteData(heapBlock.GetPtr() - index - 1, sizeof(info.attributes)).GetUchar();
        info.userSize = objectSize;
        info.heapEntry = blockAddress + index * info.userSize;
    }

    if ((info.attributes & ObjectInfoBits::PendingDisposeBit))
    {
        info.state = info.freed;
    }
    else
    {
        if ((info.attributes & ObjectInfoBits::ImplicitRootBit))
        {
            info.state = info.rooted;
        }
        else
        {
            info.state = info.unrooted;
        }
    }
    info.succeeded = true;
    return info;
};

void ShowStack(ExtRemoteTyped heapBlock, PCSTR stackType, EXT_CLASS_BASE* ext)
{
    // stackType is "Alloc" or "Free"
    std::string stackFieldName = ext->FillModuleV("pageHeap%sStack", stackType);
    if (!heapBlock.HasField(stackFieldName.c_str()))
    {
        ext->Out("Page heap %s stack trace is not supported\n", stackType);
        return;
    }
    ExtRemoteTyped stackField = heapBlock.Field(stackFieldName.c_str());
    std::string HeapBlockType = ext->FillModuleAndMemoryNS("%s!%sHeapBlock");
    char buffer[1024];
    if (stackField.GetPtr() != NULL)
    {
        sprintf_s(buffer, "%s", ext->FillModuleV("dps @@c++(((%s*)(0x%llx))->%s->stackBackTrace) L@@c++(((%s*)(0x%llx))->%s->framesCount)",
            HeapBlockType.c_str(), heapBlock.GetPtr(), stackFieldName.c_str(), HeapBlockType.c_str(), heapBlock.GetPtr(), stackFieldName.c_str()));
        ext->Dml("\t<b>%s</b> <link cmd=\"%s\">stack</link>:\n", stackType, buffer);

        ExtBuffer<char> symbol;
        ULONG64 displacement;
        auto stack = stackField.Field("stackBackTrace");
        for (ULONG i = 0; i < stackField.Field("framesCount").GetUlong(); i++)
        {
            ext->GetOffsetSymbol(stack.ArrayElement(i).GetPtr(), &symbol, &displacement);
            ext->Out("\t  %016I64x %s+0X%x\n", stack.ArrayElement(i).GetPtr(), symbol.GetBuffer(), displacement);
        }
    }
}

MPH_COMMAND(mpheap,
    "Memory protect heap",
    "{v;b,o;s;Verbose output}"
    "{s;b,o;s;Summary}"
    "{srch;b,o;srch;Search}{b;e8;o;DWORD}{w;e16;o;DWORD}{d;e32;o;DWORD}{q;e64;o;QWORD}"
    "{p;b,o;p;Specifies that page heap information is being requested}""{a;e64,o;a;Address}"
    "{isref;e64,o;isref;Find reference}"
    )
{
    MphCmdsWrapper::AutoMPHCmd autoMphCmd((MphCmdsWrapper*)this);
    bool verbose = HasArg("v");
    char buffer[1024];

    ExtRemoteTyped g_pHeapHandle(FillModuleV("%s!g_pHeapHandle", tridentModule));
    if (verbose)
    {
        this->Out("%s!g_pHeapHandle: %x\n", tridentModule, g_pHeapHandle.GetPtr());
    }

    ExtRemoteTyped heapInstance(FillModuleV("(%s!MemProtectHeap*)(%s!g_pHeapHandle)", memGCModule, tridentModule));
    if (verbose)
    {
        this->Dml("<link cmd=\"?? %s\">g_pHeapHandle</link>: %x\n", FillModuleV("(%s!MemProtectHeap*)(%s!g_pHeapHandle)", memGCModule, tridentModule), heapInstance.GetPtr());
        this->Out("threadContextTlsIndex: %x\n", heapInstance.Field("threadContextTlsIndex").GetUlong());
    }

    // list of thread contexts:
    if (strcmp(this->GetRawArgStr(), "") == 0)
    {
        ExtRemoteTyped next = heapInstance.Field("threadList").Field("head");
        if (next.GetUlongPtr() == NULL)
        {
            this->Out("threadList is not initialized yet.\n");
        }
        else
        {
            this->Out("threadId  memProtectHeap     Recycler\n");
            this->Out("-------------------------------------\n");
            do
            {
                auto threadId = next.Field("threadId").GetLong();
                auto memProtectHeapPtr = next.Field("memProtectHeap").GetPtr();
                auto recyclerPtr = next.Field("memProtectHeap").Field("recycler").GetPointerTo().GetPtr();
                this->Out("%8x     %11I64x  %11I64x\n", threadId, memProtectHeapPtr, recyclerPtr);
                next = next.Field("next");
            } while (next.GetPtr() != 0);
        }
        return;
    }

    // TODO: get heap instance from input parameter and check existance
    RemoteRecycler remoteRecycler(heapInstance.Field("recycler").GetPointerTo());
    ExtRemoteTyped recycler = remoteRecycler.GetExtRemoteTyped();

    // Summary
    if (HasArg("s"))
    {
        bool showZeroEntries = true;
        RemotePageAllocator::DisplayDataHeader("Allocator");
        remoteRecycler.ForEachPageAllocator("Thread", [showZeroEntries](PCSTR name, RemotePageAllocator pageAllocator)
        {
            pageAllocator.DisplayData(name, showZeroEntries);
            return false;
        });
        return;
    }

    // Search
    if (HasArg("srch"))
    {
        byte len = 0;
        char type;
        ULONG64 pattern = 0;
        if (this->HasArg("b"))
        {
            len = 1;
            pattern = this->GetArgU64("b");
            type = 'b';
        }
        else if (this->HasArg("w"))
        {
            len = 2;
            pattern = this->GetArgU64("w");
            type = 'w';
        }
        else if (this->HasArg("d"))
        {
            len = 4;
            pattern = this->GetArgU64("d");
            type = 'd';
        }
        else if (this->HasArg("q"))
        {
            len = 8;
            pattern = this->GetArgU64("q");
            type = 'q';
        }
        else
        {
            this->Out("Unknown search pattern.\n");
        }

        std::map<ULONG64, ULONG> validAddresses;
        remoteRecycler.ForEachPageAllocator("Thread", [&](PCSTR name, RemotePageAllocator pageAllocator)
        {
            pageAllocator.ForEachSegment([&](PCSTR segName, ExtRemoteTyped pageSegment)->bool{
                this->ThrowInterrupt();

                ULONG64 address = pageSegment.Field("address").GetPtr();
                ULONG pageCount = (ULONG)pageSegment.Field("segmentPageCount").GetPtr();
                ULONG leadingGuardPageCount = pageSegment.Field("leadingGuardPageCount").GetUlong();
                ULONG trailingGuardPageCount = pageSegment.Field("trailingGuardPageCount").GetUlong();
                ULONG validPageCount = pageCount - leadingGuardPageCount - trailingGuardPageCount;
                ULONG64 validAddress = address + leadingGuardPageCount * 4096;

                if (validAddresses.find(validAddress) != validAddresses.end())
                {
                    this->Out("duplicate segment address found\n");
                    return false;
                }

                PCSTR searchcmd = FillModuleV(this->m_PtrSize == 8 ? "s -%c 0x%016I64x L?0x%x 0x%016I64x" : "s -%c 0x%08I64x L?0x%x 0x%08I64x",
                    type, validAddress, validPageCount * 4096, pattern);
                if (verbose)
                {
                    this->Dml("\nSegment: %p+<link cmd=\"%s\">%x</link>:\n", validAddress, searchcmd, validPageCount * 4096);
                }
                this->Execute(searchcmd);
                return false;
            });
            return false;
        });
        return;
    }

    // inspect address(-isref and -p -a)
    // TODO: combine -isref and -p -a
    ULONG64 address = 0;
    // -isref
    bool isref = HasArg("isref");
    if (isref)
    {
        address = GetArgU64("isref");
    }

    // -p -a
    if (isref || HasArg("p"))
    {
        if (HasArg("a"))
        {
            address = GetArgU64("a");
        }
        else if (!isref)
        {
            this->Out("Please specify address.\n");
            return;
        }

        if (verbose)
        {
            if (this->m_PtrSize == 8)
            {
                //!list -t chakra!HeapBlockMap64::Node.next -x ".if(poi(@$extret)==(%x>>0n32)){?? ((chakra!HeapBlockMap64::Node*)@$extret)->map}" @@c++(((chakra!MemProtectHeap*)(edgehtml!g_pHeapHandle))->recycler.heapBlockMap.list)
                sprintf_s(buffer, "!list -t %s!%sHeapBlockMap64::Node.next -x "
                    "\".if(poi(@$extret)==(0x%I64x>>0n32)){ ?? @@c++((((%s!%sHeapBlockMap64::Node*)@$extret)->map.map[(0x%I64x&0xffffffff)>>0x14]->map[(0x%I64x&0x000FF000)>>0xc]))}\""
                    " @@c++(((%s!MemProtectHeap*)(edgehtml!g_pHeapHandle))->recycler.heapBlockMap.list)",
                    memGCModule, this->GetMemoryNS(), address, memGCModule, this->GetMemoryNS(), address, address, memGCModule, tridentModule);
            }
            else
            {
                //?? ((mshtml!MemProtectHeap*)(mshtml!g_pHeapHandle))->recycler.heapBlockMap.map[0x051e8130>>0x14]->map[(0x051e8130&0x000FF000)>>0xc]
                sprintf_s(buffer, "?? @@c++(((%s!MemProtectHeap*)(%s!g_pHeapHandle))->recycler.heapBlockMap.map[0x%llx>>0x14]->map[(0x%llx&0x000FF000)>>0xc])",
                    memGCModule, tridentModule, address, address);
            }
            this->Out("Command to show block: %s\n", buffer);
        }

        OBJECTINFO info = GetObjectInfo(address, recycler, this);
        if (!info.succeeded)
        {
            this->Err(info.message.c_str());
            this->Err("\n");
            return;
        }

        ExtRemoteTyped heapBlock = info.heapBlock;
        ULONG64 x64MapAddr = info.x64MapAddr;
        std::string typeName = info.typeName;
        PageHeapMode pageHeapMode = (PageHeapMode)heapBlock.Field("pageHeapMode").GetLong();

        if (m_PtrSize == 8)
        {
            // DML does not support the command containing double quote, so use the already found block map
            sprintf_s(buffer, "?? @@c++((%s*)(((%s!%sHeapBlockMap64::Node*)0x%I64x)->map.map[(0x%I64x&0xffffffff)>>0x14]->map[(0x%I64x&0x000FF000)>>0xc]))",
                typeName.c_str(), memGCModule, this->GetMemoryNS(), x64MapAddr, address, address);
        }
        else
        {
            sprintf_s(buffer, "?? @@c++((%s*)((%s!MemProtectHeap*)(%s!g_pHeapHandle))->recycler.heapBlockMap.map[0x%llx>>0x14]->map[(0x%llx&0x000FF000)>>0xc])",
                typeName.c_str(), memGCModule, tridentModule, address, address);
        }

        this->Dml("\taddress %p found in\n", address);
        this->Dml("\tHeapBlock @ <link cmd=\"%s\">%p</link> (page heap is %senabled)\n", buffer, heapBlock.GetPtr(),
            pageHeapMode == PageHeapMode::PageHeapModeOff ? "not " : "");

        const char* states[3] = { "freed", "rooted", "unrooted" };
        this->Out("\t   HEAP_ENTRY Attributes     UserPtr UserSize - State\n");
        this->Out("\t  %11I64x       %4x %11I64x     %4I64x   (%s)\n", info.heapEntry, info.attributes, info.userPtr, info.userSize, states[info.state]);

        if (isref)
        {
            if (info.state == info.freed)
            {
                return;
            }

            Addresses * rootPointers = this->recyclerCachedData.GetRootPointers(recycler, nullptr);
            rootPointers->Map([this, &recycler, &verbose, &address](ULONG64 rootAddress)
            {
                this->ThrowInterrupt();
                auto info = GetObjectInfo(rootAddress, recycler, this);
                if (verbose)
                {
                    this->Dml(this->m_PtrSize == 8 ? "\tRoot:  <link cmd=\"!mpheap -p -a 0x%016I64x\">0x%016I64x</link>" : "\tRoot:  <link cmd=\"!mpheap -p -a 0x%08I64x\">0x%08I64x</link>",
                        rootAddress, rootAddress);
                    if (info.succeeded)
                    {
                        this->Dml(this->m_PtrSize == 8 ? "  +<link cmd=\"dp 0x%016I64x L0x%x\">0x%x</link>\n" : "  +<link cmd=\"dp 0x%08I64x L0x%x\">0x%x</link>\n",
                            rootAddress, info.userSize / this->m_PtrSize, info.userSize);
                    }
                    else
                    {
                        this->Out((info.message + "\n").c_str());
                    }
                }

                if (info.succeeded)
                {
                    ExtRemoteTyped rootMem("(void**)@$extin", rootAddress);
                    for (size_t i = 0; i < info.userSize / this->m_PtrSize; i++)
                    {
                        auto data = rootMem.ArrayElement(i).GetPtr();
                        if ((ULONG64)data == address)
                        {
                            this->Dml(this->m_PtrSize == 8 ? "\t      Ref: 0x%016I64x +0x%x\n" : "\t      Ref: 0x%08I64x +0x%x\n",
                                rootAddress, i*this->m_PtrSize);
                        }
                    }
                }
            });

            return;
        }

        // page heap
        if (pageHeapMode != PageHeapMode::PageHeapModeOff)
        {
            ShowStack(heapBlock, "Alloc", this);
            ShowStack(heapBlock, "Free", this);
        }

        return;
    }
}

void MphCmdsWrapper::InitializeForMPH()
{
    EXT_CLASS_BASE* ext = (EXT_CLASS_BASE*)this;
    ext->inMPHCmd = true;

    enum IEMode
    {
        legacy = 0,
        edge = 1
    };

    bool verbose = ext->HasArg("v");

    char* tridentModules[] = { "mshtml", "edgehtml" };
    char* memGCModules[] = { "mshtml", "chakra" };
    char buffer[1024];

    IEMode mode = legacy;
    sprintf_s(buffer, "%s!MemProtectHeap", memGCModules[mode]); // should not be inlined
    ULONG typeIdMemProtectHeap;
    if (ext->m_Symbols2->GetSymbolTypeId(buffer, &typeIdMemProtectHeap, nullptr) == S_OK)
    {
        if (verbose)
        {
            ext->Out("Found MemProtectHeap in mshtml(legacy mode).\n");
        }
    }
    else
    {
        mode = edge;
        sprintf_s(buffer, "%s!MemProtectHeap", memGCModules[mode]); // should not be inlined
        if (ext->m_Symbols2->GetSymbolTypeId(buffer, &typeIdMemProtectHeap, nullptr) == S_OK)
        {
            if (verbose)
            {
                ext->Out("Found MemProtectHeap in chakra(edge mode).\n");
            }
        }
        else
        {
            ext->Err("Cannot find MemProtectHeap.\n");
            return;
        }
    }

    tridentModule = tridentModules[mode];
    memGCModule = memGCModules[mode];

}

JD_PRIVATE_COMMAND(findpage,
    "Find page for an address",
    "{;e,r;;Address to find}")
{

    ULONG64 targetAddress = this->GetUnnamedArgU64(0);


    RemoteThreadContext::ForEach([&](RemoteThreadContext threadContext)
    {
        this->Out("Thread context: %p\n", threadContext.GetExtRemoteTyped().GetPtr());
        threadContext.ForEachPageAllocator([&](PCSTR name, RemotePageAllocator pageAllocator)
        {
            pageAllocator.ForEachSegment([&](PCSTR segName, ExtRemoteTyped pageSegment)->bool{
                ULONG64 address = pageSegment.Field("address").GetUlongPtr();
                ULONG pageCount = pageSegment.Field("segmentPageCount").GetUlong();
                if (address <= targetAddress && pageCount * 4096 + address > targetAddress)
                {
                    this->Out("Page Allocator: %p, %s\n", pageAllocator.GetExtRemoteTyped().GetPointerTo().GetPtr(), name);
                    pageAllocator.GetExtRemoteTyped().OutFullValue();
                    this->Out("PageSegment:%p, %s\n", pageSegment.GetPointerTo().GetPtr(), segName);
                    pageSegment.OutFullValue();
                }
                return false;
            });
            return false;
        });


        return false; // Don't stop iterating
    });
}
#endif

#pragma endregion()

// ---- End jd private commands implementation ----------------------------------------------------
//JD_PRIVATE
// ------------------------------------------------------------------------------------------------
