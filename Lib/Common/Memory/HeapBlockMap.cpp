//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

#if defined(_M_X64_OR_ARM64)
HeapBlockMap32::HeapBlockMap32(__in char * startAddress) : 
    startAddress(startAddress), 
#else
HeapBlockMap32::HeapBlockMap32() : 
#endif
    count(0)
{
    memset(map, 0, sizeof(map));

#if defined(_M_X64_OR_ARM64)
    Assert(((size_t)startAddress) % TotalSize == 0);
#endif
}

HeapBlockMap32::~HeapBlockMap32()
{
    for (uint i = 0; i < _countof(map); i++)
    {
        L2MapChunk * chunk = map[i];
        if (chunk)
        {
            NoMemProtectHeapDelete(chunk);
        }
    }
}

HeapBlock *
HeapBlockMap32::GetHeapBlock(void * address)
{    
    uint id1 = GetLevel1Id(address);

    L2MapChunk * l2map = map[id1];
    if (l2map == null) 
    {
        return null;
    }
    
    return l2map->Get(address);
}

bool
HeapBlockMap32::EnsureHeapBlock(void * address, uint pageCount)
{
    uint id1 = GetLevel1Id(address);
    uint id2 = GetLevel2Id(address);
    
    uint currentPageCount = min(pageCount, L2Count - id2);

    while (true)
    {
        if (map[id1] == null)
        {
            L2MapChunk * newChunk = NoMemProtectHeapNewNoThrowZ(L2MapChunk);
            if (newChunk == null)
            {
                // Leave any previously allocated L2MapChunks in place -- 
                // the concurrent thread may have already accessed them.
                // These will be cleaned up in the Cleanup method, after Sweep is complete.
                return false;
            }

            map[id1] = newChunk;
            count++;
        }
        
        pageCount -= currentPageCount; 
        if (pageCount == 0)
        {
            break;
        }
        
        id2 = 0;
        id1++;
        currentPageCount = min(pageCount, L2Count);     
    }

    return true;
}

void
HeapBlockMap32::SetHeapBlockNoCheck(void * address, uint pageCount, HeapBlock * heapBlock, HeapBlock::HeapBlockType blockType, byte bucketIndex)
{
    uint id1 = GetLevel1Id(address);
    uint id2 = GetLevel2Id(address);
    
    uint currentPageCount = min(pageCount, L2Count - id2);

    while (true)
    {
        Assert(map[id1] != null);
        
        map[id1]->Set(id2, currentPageCount, heapBlock, blockType, bucketIndex);
        
        pageCount -= currentPageCount; 
        if (pageCount == 0)
        {
            return;
        }
        
        id2 = 0;
        id1++;
        currentPageCount = min(pageCount, L2Count);     
    }
}

bool
HeapBlockMap32::SetHeapBlock(void * address, uint pageCount, HeapBlock * heapBlock, HeapBlock::HeapBlockType blockType, byte bucketIndex)
{
    // First, make sure we have all the necessary L2MapChunks we'll need.
    // This ensures that in case of failure, the concurrent thread won't see an inconsistent state.
    if (!EnsureHeapBlock(address, pageCount))
    {
        return false;
    }

    // Now, do the actual set, which cannot fail.
    SetHeapBlockNoCheck(address, pageCount, heapBlock, blockType, bucketIndex);
    return true;
}

void
HeapBlockMap32::ClearHeapBlock(void * address, uint pageCount)
{
    uint id1 = GetLevel1Id(address);
    uint id2 = GetLevel2Id(address);
    
    uint currentPageCount = min(pageCount, L2Count - id2);

    while (true)
    {
        Assert(map[id1] != null);
        
        map[id1]->Clear(id2, currentPageCount);
        
        pageCount -= currentPageCount; 
        if (pageCount == 0)
        {
            return;
        }
        
        id2 = 0;
        id1++;
        currentPageCount = min(pageCount, L2Count);     
    }
}

HeapBlockMap32::PageMarkBitVector *
HeapBlockMap32::GetPageMarkBitVector(void * address)
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * l2map = map[id1];
    if (l2map == null)
    {
        return null;
    }

    return l2map->GetPageMarkBitVector(address);
}

template <size_t BitCount>
BVStatic<BitCount>*
HeapBlockMap32::GetMarkBitVectorForPages(void * address)
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * l2map = map[id1];
    if (l2map == null)
    {
        return null;
    }

    return l2map->GetMarkBitVectorForPages<BitCount>(address);
}

template BVStatic<SmallAllocationBlockAttributes::BitVectorCount>* HeapBlockMap32::GetMarkBitVectorForPages<SmallAllocationBlockAttributes::BitVectorCount>(void * address);
template BVStatic<MediumAllocationBlockAttributes::BitVectorCount>* HeapBlockMap32::GetMarkBitVectorForPages<MediumAllocationBlockAttributes::BitVectorCount>(void * address);

uint
HeapBlockMap32::GetMarkCount(void * address, uint pageCount)
{
    uint markCount = 0;

    ForEachChunkInAddressRange(address, pageCount, [&](L2MapChunk* l2Map, uint chunkId)
    {
        markCount += l2Map->GetPageMarkBitVector(chunkId /* pageIndex */)->Count();
    });

    return markCount;
}

template <class Fn>
void
HeapBlockMap32::ForEachChunkInAddressRange(void * address, size_t pageCount, Fn fn)
{
    uint id1 = GetLevel1Id(address);
    uint id2 = GetLevel2Id(address);

    while (true)
    {
        L2MapChunk * l2map = map[id1];
        Assert(l2map != null);

        if (l2map != null)
        {
            while (id2 < L2Count)
            {
                fn(l2map, id2);

                id2++;
                pageCount--;
                if (pageCount == 0)
                {
                    return;
                }
            }

            id2 = 0;
            id1++;
        }
    }
}

bool
HeapBlockMap32::IsMarked(void * address) const
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * chunk = map[id1];

    Assert(chunk != null);
    return chunk->IsMarked(address);
}

void
HeapBlockMap32::SetMark(void * address)
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * chunk = map[id1];
    Assert(chunk != null);

    return chunk->SetMark(address);
}

bool
HeapBlockMap32::TestAndSetMark(void * address)
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * chunk = map[id1];
    if (chunk == null)
    {
        // False refernce
        return false;
    }

    uint bitIndex = chunk->GetMarkBitIndex(address);
    return (chunk->markBits.TestAndSet(bitIndex) != 0);
}

void 
HeapBlockMap32::ResetMarks()
{
    for (uint i = 0; i < L1Count; i++)
    {
        L2MapChunk * chunk = map[i];
        if (chunk == null)
        {
            continue;
        }

        chunk->markBits.ClearAll();

#ifdef RECYCLER_VERIFY_MARK
        chunk->isNewChunk = false;
#endif

#if DBG
        for (uint j = 0; j < L2Count; j++)
        {
            chunk->pageMarkCount[j] = 0;
        }
#endif
    }
}

#if DBG
ushort 
HeapBlockMap32::GetPageMarkCount(void * address) const
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * l2map = map[id1];
    Assert(l2map != null);

    uint id2 = GetLevel2Id(address);

    return l2map->pageMarkCount[id2];
}

void 
HeapBlockMap32::SetPageMarkCount(void * address, ushort markCount)
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * l2map = map[id1];
    Assert(l2map != null);
    
    uint id2 = GetLevel2Id(address);

    // Callers should already have updated the mark bits by the time they call this,
    // so check that the new count is correct for the current mark bits.
    // Not true right now, will be true...
    Assert(l2map->GetPageMarkBitVector(id2)->Count() == markCount);


    l2map->pageMarkCount[id2] = markCount;
}

template void HeapBlockMap32::VerifyMarkCountForPages<SmallAllocationBlockAttributes::BitVectorCount>(void* address, size_t pageCount);
template void HeapBlockMap32::VerifyMarkCountForPages<MediumAllocationBlockAttributes::BitVectorCount>(void* address, size_t pageCount);

template <uint BitVectorCount>
void
HeapBlockMap32::VerifyMarkCountForPages(void * address, size_t pageCount)
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * l2map = map[id1];
    Assert(l2map != null);

    uint id2 = GetLevel2Id(address);

    Assert(id2 + pageCount <= L2Count);
    for (size_t i = id2; i < pageCount + id2; i++)
    {
        uint markCountForPage = l2map->GetPageMarkBitVector(i)->Count();
        Assert(markCountForPage == l2map->pageMarkCount[i]);
    }
}
#endif

HeapBlockMap32::L2MapChunk::L2MapChunk()
{
    // We are zero-initialized so don't need to actually init.

    // Mark bits should be cleared by default
    Assert(markBits.Count() == 0);
    
#ifdef RECYCLER_VERIFY_MARK
    this->isNewChunk = true;
#endif

#if DBG
    for (uint i = 0; i < L2Count; i++)
    {
        Assert(pageMarkCount[i] == 0);
    }
#endif
}


HeapBlockMap32::L2MapChunk::~L2MapChunk()
{    
    // In debug builds, we guarantee that the heap block is clear on shutdown.
    // In free builds, we skip this to save time.
    // So this assert is only true in debug builds.
    Assert(IsEmpty());
}

HeapBlock *
HeapBlockMap32::L2MapChunk::Get(void * address)
{
    uint id2 = GetLevel2Id(address);
    Assert(id2 < L2Count);
    __analysis_assume(id2 < L2Count);
    return map[id2];
}

void
HeapBlockMap32::L2MapChunk::Set(uint id2, uint pageCount, HeapBlock * heapBlock, HeapBlock::HeapBlockType blockType, byte bucketIndex)
{
    uint id2End = id2 + pageCount;

    Assert(id2 < L2Count);
    Assert(id2End <= L2Count);

    for (uint i = id2; i < id2End; i++)
    {
        __analysis_assume(i < L2Count);
        Assert(map[i] == null);
        Assert(blockInfo[i].blockType == HeapBlock::HeapBlockType::FreeBlockType);

        // Set the blockType last, because we will test this first during marking.        
        // If it's not FreeBlock, then we expect bucketIndex and heapBlock to be valid.
        map[i] = heapBlock;
        blockInfo[i].bucketIndex = bucketIndex;
        blockInfo[i].blockType = blockType;
    }
}

void
HeapBlockMap32::L2MapChunk::Clear(uint id2, uint pageCount)
{ 
    uint id2End = id2 + pageCount;

    Assert(id2 < L2Count);
    Assert(id2End <= L2Count);

    for (uint i = id2; i < id2End; i++)
    {
        __analysis_assume(i < L2Count);
        Assert(map[i] != null);
        Assert(blockInfo[i].blockType != HeapBlock::HeapBlockType::FreeBlockType);

        // This shouldn't be called when concurrent marking is happening, so order does not matter.
        // Regardless, set the blockType first just to be internally consistent.
        // We don't actually clear the bucketIndex because it doesn't matter if the blockType is FreeBlock.
        blockInfo[i].blockType = HeapBlock::HeapBlockType::FreeBlockType;
        map[i] = null;
    }
}

bool
HeapBlockMap32::L2MapChunk::IsEmpty() const
{
    for (uint i = 0; i < L2Count; i++)
    {
        if (map[i] != null)
        {
            return false;
        }
    }

    return true;
}

HeapBlockMap32::PageMarkBitVector *
HeapBlockMap32::L2MapChunk::GetPageMarkBitVector(void * address)
{
    uint id2 = GetLevel2Id(address);
    Assert(id2 < L2Count);
    __analysis_assume(id2 < L2Count);
    return GetPageMarkBitVector(id2);;
}

HeapBlockMap32::PageMarkBitVector *
HeapBlockMap32::L2MapChunk::GetPageMarkBitVector(uint pageIndex)
{
    return markBits.GetRange<PageMarkBitCount>(pageIndex * PageMarkBitCount);
}

template <size_t BitCount>
BVStatic<BitCount> *
HeapBlockMap32::L2MapChunk::GetMarkBitVectorForPages(void * address)
{
    uint id2 = GetLevel2Id(address);
    Assert(id2 < L2Count);
    __analysis_assume(id2 < L2Count);
    return GetMarkBitVectorForPages<BitCount>(id2);;
}

template <size_t BitCount>
BVStatic<BitCount> *
HeapBlockMap32::L2MapChunk::GetMarkBitVectorForPages(uint pageIndex)
{
    return markBits.GetRange<BitCount>(pageIndex * PageMarkBitCount);
}

bool
HeapBlockMap32::L2MapChunk::IsMarked(void * address) const
{
    return markBits.Test(GetMarkBitIndex(address)) == TRUE;
}

void
HeapBlockMap32::L2MapChunk::SetMark(void * address)
{
    markBits.Set(GetMarkBitIndex(address));
}

#ifdef RECYCLER_STRESS
void
HeapBlockMap32::InduceFalsePositives(Recycler * recycler)
{
    for (uint i = 0; i < L1Count; i++)
    {
        L2MapChunk * chunk = map[i];
        if (chunk == null)
        {
            continue;
        }
        
        for (uint j = 0; j < L2Count; j++)
        {
            HeapBlock * block = chunk->map[j];

            if (block == null)
            {
                // Unallocated block.  Try to mark the first offset, in case
                // we are simultaneously allocating this block on the main thread.
                recycler->TryMarkNonInterior((void *)GetAddressFromIds(i, j), null);
            }
            else if (!block->IsLargeHeapBlock())
            {
                ((SmallHeapBlock *)block)->InduceFalsePositive(recycler);
            }
        }
    }
}
#endif

#ifdef RECYCLER_VERIFY_MARK
bool 
HeapBlockMap32::IsAddressInNewChunk(void * address)
{
    uint id1 = GetLevel1Id(address);

    L2MapChunk * l2map = map[id1];
    Assert(l2map != null);
    return l2map->isNewChunk;
}
#endif

#ifdef CONCURRENT_GC_ENABLED

template <class Fn>
void 
HeapBlockMap32::ForEachSegment(Recycler * recycler, Fn func)
{
    Segment * currentSegment = nullptr;
    
    for (uint i = 0; i < L1Count; i++)
    {
        L2MapChunk * chunk = map[i];
        if (chunk == null)
        {
            continue;
        }
        
        for (uint j = 0; j < L2Count; j++)
        {
            HeapBlock * block = chunk->map[j];
            if (block == null)
            {
                continue;
            }

            Assert(block->GetSegment() != nullptr);
            if (block->GetSegment() == currentSegment)
            {
                Assert(currentSegment->IsInSegment(block->GetAddress()));
                continue;
            }

            // New segment.  
            Assert(currentSegment == nullptr || !currentSegment->IsInSegment(block->GetAddress()));
            currentSegment = block->GetSegment();

            char * segmentStart = currentSegment->GetAddress();
            size_t segmentLength = currentSegment->GetPageCount() * PageSize;

            PageAllocator* segmentPageAllocator = (PageAllocator*)currentSegment->GetAllocator();

            Assert(segmentPageAllocator == block->GetPageAllocator(recycler));
#if defined(_M_X64_OR_ARM64)
            // On 64 bit, the segment may span multiple HeapBlockMap32 structures.
            // Limit the processing to the portion of the segment in this HeapBlockMap32.
            // We'll process other portions when we visit the other HeapBlockMap32 structures.
            if (segmentStart < this->startAddress)
            {
                Assert(segmentLength > (size_t)(this->startAddress - segmentStart));
                segmentLength -= (this->startAddress - segmentStart);
                segmentStart = this->startAddress;
            }

            if ((segmentStart - this->startAddress) + segmentLength > HeapBlockMap32::TotalSize) 
            { 
                segmentLength = HeapBlockMap32::TotalSize - (segmentStart - this->startAddress); 
            }
#endif

            func(segmentStart, segmentLength, currentSegment, segmentPageAllocator);
        }
    }
}

void
HeapBlockMap32::ResetWriteWatch(Recycler * recycler)
{
    this->ForEachSegment(recycler, [=] (char * segmentStart, size_t segmentLength, Segment * segment, PageAllocator * segmentPageAllocator) {

        Assert(segmentLength % AutoSystemInfo::PageSize == 0);
        
        if (segmentPageAllocator == recycler->GetRecyclerPageAllocator() ||
            segmentPageAllocator == recycler->GetRecyclerLargeBlockPageAllocator())
        {
            // Call ResetWriteWatch for Small non-leaf and Large segments.
            UINT ret = ::ResetWriteWatch(segmentStart, segmentLength);
            Assert(ret == 0);
        }
#ifdef RECYCLER_WRITE_BARRIER
        else if (segmentPageAllocator == recycler->GetRecyclerWithBarrierPageAllocator())
        {
            // Reset software write barrier for barrier segments.
            RecyclerWriteBarrierManager::ResetWriteBarrier(segmentStart, segmentLength / AutoSystemInfo::PageSize);
        }
#endif   
    });
}

bool
HeapBlockMap32::RescanPage(void * dirtyPage, bool* anyObjectsMarkedOnPage, Recycler * recycler)
{
    uint id1 = GetLevel1Id(dirtyPage);
    L2MapChunk * chunk = map[id1];
    if (chunk != null)
    {
        uint id2 = GetLevel2Id(dirtyPage);
        HeapBlock::HeapBlockType blockType = chunk->blockInfo[id2].blockType;
        // Determine block type and process as appropriate
        switch (blockType)
        {
        case HeapBlock::HeapBlockType::FreeBlockType:
            // We had a false reference to a free block.  Do nothing.
            break;

        case HeapBlock::HeapBlockType::SmallNormalBlockType:
        case HeapBlock::HeapBlockType::SmallNormalBlockWithBarrierType:
            return RescanHeapBlock<SmallNormalHeapBlock>(dirtyPage, blockType, chunk, id2, anyObjectsMarkedOnPage, recycler);
        case HeapBlock::HeapBlockType::SmallFinalizableBlockType:
        case HeapBlock::HeapBlockType::SmallFinalizableBlockWithBarrierType:
            return RescanHeapBlock<SmallFinalizableHeapBlock>(dirtyPage, blockType, chunk, id2, anyObjectsMarkedOnPage, recycler);
        case HeapBlock::HeapBlockType::MediumNormalBlockType:
        case HeapBlock::HeapBlockType::MediumNormalBlockWithBarrierType:
            return RescanHeapBlock<MediumNormalHeapBlock>(dirtyPage, blockType, chunk, id2, anyObjectsMarkedOnPage, recycler);
        case HeapBlock::HeapBlockType::MediumFinalizableBlockType:
        case HeapBlock::HeapBlockType::MediumFinalizableBlockWithBarrierType:
            return RescanHeapBlock<MediumFinalizableHeapBlock>(dirtyPage, blockType, chunk, id2, anyObjectsMarkedOnPage, recycler);
        default:
            // Shouldn't be here -- leaf blocks aren't rescanned, and large blocks are handled separately
            Assert(false);
            break;
        }
    }

    return false;
}

template bool HeapBlockMap32::RescanHeapBlock<SmallNormalHeapBlock>(void * dirtyPage, HeapBlock::HeapBlockType blockType, L2MapChunk* chunk, uint id2, bool* anyObjectsMarkedOnPage, Recycler * recycler);
template bool HeapBlockMap32::RescanHeapBlock<SmallFinalizableHeapBlock>(void * dirtyPage, HeapBlock::HeapBlockType blockType, L2MapChunk* chunk, uint id2, bool* anyObjectsMarkedOnPage, Recycler * recycler);
template bool HeapBlockMap32::RescanHeapBlock<MediumNormalHeapBlock>(void * dirtyPage, HeapBlock::HeapBlockType blockType, L2MapChunk* chunk, uint id2, bool* anyObjectsMarkedOnPage, Recycler * recycler);
template bool HeapBlockMap32::RescanHeapBlock<MediumFinalizableHeapBlock>(void * dirtyPage, HeapBlock::HeapBlockType blockType, L2MapChunk* chunk, uint id2, bool* anyObjectsMarkedOnPage, Recycler * recycler);

template <class TBlockType>
bool
HeapBlockMap32::RescanHeapBlock(void * dirtyPage, HeapBlock::HeapBlockType blockType, L2MapChunk* chunk, uint id2, bool* anyObjectsMarkedOnPage, Recycler * recycler)
{
    Assert(chunk != null);
    char* heapBlockPageAddress = TBlockType::GetBlockStartAddress((char*) dirtyPage);

    typedef TBlockType::HeapBlockAttributes TBlockAttributes;

    // We need to check the entire mark bit vector here. It's not sufficient to just check the page's 
    // mark bit vector because the object that's dirty on the page could have started on an earlier page
    auto markBits = chunk->GetMarkBitVectorForPages<TBlockAttributes::BitVectorCount>(heapBlockPageAddress);
    if (!markBits->IsAllClear())
    {
        Assert(chunk->map[id2]->GetHeapBlockType() == blockType);

        // Small finalizable heap blocks require the HeapBlock * (to look up object attribtues).
        // For others, this is null
        TBlockType* block = GetHeapBlockForRescan<TBlockType>(chunk, id2);
        uint bucketIndex = chunk->blockInfo[id2].bucketIndex;
        if (!SmallNormalHeapBucketBase<TBlockType>::RescanObjectsOnPage(block, 
            (char *)dirtyPage, heapBlockPageAddress, markBits, HeapInfo::GetObjectSizeForBucketIndex<TBlockAttributes>(bucketIndex), bucketIndex, anyObjectsMarkedOnPage, recycler))
        {
            // Failed due to OOM
            ((TBlockType*) chunk->map[id2])->SetNeedOOMRescan(recycler);
            return false;
        }

        return true;
    }

    // Didn't actually rescan the block.
    return false;
}

template <typename TBlockType>
TBlockType*
HeapBlockMap32::GetHeapBlockForRescan(HeapBlockMap32::L2MapChunk* chunk, uint id2) const
{
    return null;
}

template <>
SmallFinalizableHeapBlock*
HeapBlockMap32::GetHeapBlockForRescan(HeapBlockMap32::L2MapChunk* chunk, uint id2) const
{
    return (SmallFinalizableHeapBlock*) chunk->map[id2];
}

template <>
MediumFinalizableHeapBlock*
HeapBlockMap32::GetHeapBlockForRescan(HeapBlockMap32::L2MapChunk* chunk, uint id2) const
{
    return (MediumFinalizableHeapBlock*)chunk->map[id2];
}

void
HeapBlockMap32::MakeAllPagesReadOnly(Recycler* recycler)
{
    this->ChangeProtectionLevel(recycler, PAGE_READONLY, PAGE_READWRITE);
}

void
HeapBlockMap32::MakeAllPagesReadWrite(Recycler* recycler)
{
    this->ChangeProtectionLevel(recycler, PAGE_READWRITE, PAGE_READONLY);
}

void
HeapBlockMap32::ChangeProtectionLevel(Recycler* recycler, DWORD protectFlags, DWORD expectedOldFlags)
{
    this->ForEachSegment(recycler, [&](char* segmentStart, size_t segmentLength, Segment* currentSegment, PageAllocator* segmentPageAllocator)
    {
        // Ideally, we shouldn't to exclude LargeBlocks here but guest arenas are allocated
        // from this allocator and we touch them during marking if they're pending delete
        if ((segmentPageAllocator != recycler->GetRecyclerLeafPageAllocator())
            && (segmentPageAllocator != recycler->GetRecyclerLargeBlockPageAllocator()))
        {
            Assert(currentSegment->IsPageSegment());
            ((PageSegment*)currentSegment)->ChangeSegmentProtection(protectFlags, expectedOldFlags);
        }
    });
}

uint
HeapBlockMap32::Rescan(Recycler * recycler, bool resetWriteWatch)
{
    // Loop through segments and find dirty pages.    
    const DWORD writeWatchFlags = (resetWriteWatch ? WRITE_WATCH_FLAG_RESET : 0);

    uint scannedPageCount = 0;
    bool anyObjectsScannedOnPage = false;
    
    this->ForEachSegment(recycler, [&] (char * segmentStart, size_t segmentLength, Segment * currentSegment, PageAllocator * segmentPageAllocator) {
        Assert(segmentLength % AutoSystemInfo::PageSize == 0);
        
        // Call GetWriteWatch for Small non-leaf segments.
        // Large blocks have their own separate write watch handling.
        if (segmentPageAllocator == recycler->GetRecyclerPageAllocator())
        {
            // array for WW results
            void * dirtyPageAddresses[MaxGetWriteWatchPages];

            Assert(segmentLength <= MaxGetWriteWatchPages * PageSize);
            
            ULONG_PTR pageCount = MaxGetWriteWatchPages;
            DWORD pageSize = PageSize;

            UINT ret = ::GetWriteWatch(writeWatchFlags, segmentStart, segmentLength, dirtyPageAddresses, &pageCount, &pageSize);
            Assert(ret == 0);
            Assert(pageSize == PageSize);
            Assert(pageCount <= MaxGetWriteWatchPages);

            // Process results: 
            // Loop through reported dirty pages and set their write watch bit.
            for (uint i = 0; i < pageCount; i++)
            {
                char * dirtyPage = (char *)dirtyPageAddresses[i];
                Assert((((size_t)dirtyPage) % PageSize) == 0);
                Assert(dirtyPage >= segmentStart);
                Assert(dirtyPage < segmentStart + segmentLength);
                
#if defined(_M_X64_OR_ARM64)
                Assert(HeapBlockMap64::GetNodeStartAddress(dirtyPage) == this->startAddress);
#endif

                if (RescanPage(dirtyPage, &anyObjectsScannedOnPage, recycler) && anyObjectsScannedOnPage)
                {
                    scannedPageCount++;            
                }
            }
        }
        else if (segmentPageAllocator == recycler->GetRecyclerWithBarrierPageAllocator())
        {
            // Loop through pages for this segment and check write barrier.
            uint pageCount = segmentLength / AutoSystemInfo::PageSize;
            for (uint i = 0; i < pageCount; i++)
            {
                char * pageAddress = segmentStart + (i * AutoSystemInfo::PageSize);
                Assert((size_t)(pageAddress - segmentStart) < segmentLength);
                
#if defined(_M_X64_OR_ARM64)
                Assert(HeapBlockMap64::GetNodeStartAddress(pageAddress) == this->startAddress);
#endif

                // TODO: We are not resetting the write barrier here when RescanFlags_ResetWriteWatch is passed.
                // We never have previously, but it still seems like we should.

                BYTE writeBarrierByte = RecyclerWriteBarrierManager::GetWriteBarrier(pageAddress);
                SwbVerboseTrace(recycler->GetRecyclerFlagsTable(), L"Address: 0x%p, Write Barrier value: %u\n", pageAddress, writeBarrierByte);
                bool isDirty = (writeBarrierByte == 1);

                if (isDirty)
                {
                    if (RescanPage(pageAddress, &anyObjectsScannedOnPage, recycler) && anyObjectsScannedOnPage)
                    {
                        scannedPageCount++;
                    }
                }
            }            
        }
        else
        {
            Assert(segmentPageAllocator == recycler->GetRecyclerLeafPageAllocator() ||
                segmentPageAllocator == recycler->GetRecyclerLargeBlockPageAllocator());
        }
    });

    return scannedPageCount;
}

bool
HeapBlockMap32::OOMRescan(Recycler * recycler)
{
    this->anyHeapBlockRescannedDuringOOM = false;
    bool noHeapBlockNeedsRescan = true;

    // Loop through segments and find pages that need OOM Rescan.
    
    this->ForEachSegment(recycler, [=, &noHeapBlockNeedsRescan] (char * segmentStart, size_t segmentLength, Segment * currentSegment, PageAllocator * segmentPageAllocator) {
        Assert(segmentLength % AutoSystemInfo::PageSize == 0);
        
        // Process Small non-leaf segments (including write barrier blocks).
        // Large blocks have their own separate write watch handling.
        if (segmentPageAllocator == recycler->GetRecyclerPageAllocator() ||
            segmentPageAllocator == recycler->GetRecyclerWithBarrierPageAllocator())
        {
            if (recycler->NeedOOMRescan())
            {
                // We hit OOM again.  Don't try to process any more blocks, leave them for the next OOM pass.
                return;
            }
            
            // Loop through pages for this segment and check OOM flag.
            uint pageCount = segmentLength / AutoSystemInfo::PageSize;
            for (uint i = 0; i < pageCount; i++)
            {
                char * pageAddress = segmentStart + (i * AutoSystemInfo::PageSize);
                Assert((size_t)(pageAddress - segmentStart) < segmentLength);
                
#if defined(_M_X64_OR_ARM64)
                Assert(HeapBlockMap64::GetNodeStartAddress(pageAddress) == this->startAddress);
#endif

                uint id1 = GetLevel1Id(pageAddress);
                L2MapChunk * chunk = map[id1];
                if (chunk != null)
                {
                    uint id2 = GetLevel2Id(pageAddress);
                    HeapBlock * heapBlock = chunk->map[id2];
                    if (heapBlock != null && heapBlock->GetAddress() == pageAddress)
                    {
                        if (heapBlock->GetAndClearNeedOOMRescan())
                        {
                            noHeapBlockNeedsRescan = false;

                            HeapBlock::HeapBlockType blockType = chunk->blockInfo[id2].blockType;

                            // Determine block type and process as appropriate
                            switch (blockType)
                            {
                            case HeapBlock::HeapBlockType::FreeBlockType:
                                // Can't have a free block that has OOMRescan flag set
                                Assert(false);
                                break;

                            case HeapBlock::HeapBlockType::SmallNormalBlockType:
                            case HeapBlock::HeapBlockType::SmallNormalBlockWithBarrierType:
                                if (!RescanHeapBlockOnOOM<SmallNormalHeapBlock>((SmallNormalHeapBlock*)heapBlock, pageAddress, blockType, chunk->blockInfo[id2].bucketIndex, chunk, recycler))
                                {
                                    return;
                                }
                                break;

                            case HeapBlock::HeapBlockType::SmallFinalizableBlockType:
                            case HeapBlock::HeapBlockType::SmallFinalizableBlockWithBarrierType:
                                if (!RescanHeapBlockOnOOM<SmallFinalizableHeapBlock>((SmallFinalizableHeapBlock*) heapBlock, pageAddress, blockType, chunk->blockInfo[id2].bucketIndex, chunk, recycler))
                                {
                                    return;
                                }
                                break;

                            case HeapBlock::HeapBlockType::MediumNormalBlockType:
                            case HeapBlock::HeapBlockType::MediumNormalBlockWithBarrierType:
                                if (!RescanHeapBlockOnOOM<MediumNormalHeapBlock>((MediumNormalHeapBlock*)heapBlock, pageAddress, blockType, chunk->blockInfo[id2].bucketIndex, chunk, recycler))
                                {
                                    return;
                                }
                                break;

                            case HeapBlock::HeapBlockType::MediumFinalizableBlockType:
                            case HeapBlock::HeapBlockType::MediumFinalizableBlockWithBarrierType:
                                if (!RescanHeapBlockOnOOM<MediumFinalizableHeapBlock>((MediumFinalizableHeapBlock*) heapBlock, pageAddress, blockType, chunk->blockInfo[id2].bucketIndex, chunk, recycler))
                                {
                                    return;
                                }
                                break;

                            default:
                                // Shouldn't be here -- leaf blocks aren't rescanned, and large blocks are handled separately
                                Assert(false);
                                break;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            Assert(segmentPageAllocator == recycler->GetRecyclerLeafPageAllocator() ||
                segmentPageAllocator == recycler->GetRecyclerLargeBlockPageAllocator());
        }
    });

    // TODO: Enable this assert post-Win10
    // Assert(this->anyHeapBlockRescannedDuringOOM);
    // Success if:
    //  No heap block needs OOM rescan OR
    //  A single heap block was rescanned
    return noHeapBlockNeedsRescan || this->anyHeapBlockRescannedDuringOOM;
}

template bool HeapBlockMap32::RescanHeapBlockOnOOM<SmallNormalHeapBlock>(SmallNormalHeapBlock* heapBlock, char* pageAddress, HeapBlock::HeapBlockType blockType, uint bucketIndex, L2MapChunk * chunk, Recycler * recycler);
template bool HeapBlockMap32::RescanHeapBlockOnOOM<SmallFinalizableHeapBlock>(SmallFinalizableHeapBlock* heapBlock, char* pageAddress, HeapBlock::HeapBlockType blockType, uint bucketIndex, L2MapChunk * chunk, Recycler * recycler);
template bool HeapBlockMap32::RescanHeapBlockOnOOM<MediumNormalHeapBlock>(MediumNormalHeapBlock* heapBlock, char* pageAddress, HeapBlock::HeapBlockType blockType, uint bucketIndex, L2MapChunk * chunk, Recycler * recycler);
template bool HeapBlockMap32::RescanHeapBlockOnOOM<MediumFinalizableHeapBlock>(MediumFinalizableHeapBlock* heapBlock, char* pageAddress, HeapBlock::HeapBlockType blockType, uint bucketIndex, L2MapChunk * chunk, Recycler * recycler);

template <class TBlockType>
bool
HeapBlockMap32::RescanHeapBlockOnOOM(TBlockType* heapBlock, char* pageAddress, HeapBlock::HeapBlockType blockType, uint bucketIndex, L2MapChunk * chunk, Recycler * recycler)
{
    // In the OOM codepath, we expect the heap block to be dereferenced since perf is not critical
    Assert(heapBlock != nullptr);
    Assert(heapBlock->GetHeapBlockType() == blockType);
    auto markBits = this->GetMarkBitVectorForPages<TBlockType::HeapBlockAttributes::BitVectorCount>(heapBlock->GetAddress());
    char* blockStartAddress = TBlockType::GetBlockStartAddress(pageAddress);

    // Rescan all pages in this block
    // The following assert makes sure that this method is called only once per heap block
    Assert(blockStartAddress == pageAddress);

    for (int i = 0; i < TBlockType::HeapBlockAttributes::PageCount; i++)
    {
        char* pageAddressToScan = blockStartAddress + (i * AutoSystemInfo::PageSize);

        if (!SmallNormalHeapBucketBase<TBlockType>::RescanObjectsOnPage(heapBlock,
            pageAddressToScan, blockStartAddress, markBits, HeapInfo::GetObjectSizeForBucketIndex<TBlockType::HeapBlockAttributes>(bucketIndex), bucketIndex, nullptr, recycler))
        {
            // Failed due to OOM
            ((TBlockType*)heapBlock)->SetNeedOOMRescan(recycler);
        }

        if (recycler->NeedOOMRescan())
        {
            // We hit OOM again.  Don't try to process any more blocks, leave them for the next OOM pass.
            return false;
        }
    }

    this->anyHeapBlockRescannedDuringOOM = true;

    return true;
}
#endif

// This function is called in-thread after Sweep, to find empty L2 Maps and release them.

void
HeapBlockMap32::Cleanup(bool concurrentFindImplicitRoot)
{
    for (uint id1 = 0; id1 < L1Count; id1++)
    {
        L2MapChunk * l2map = map[id1];
        if (l2map != null && l2map->IsEmpty())
        {
            // Concurrent searches for implicit roots will never see empty L2 maps.
            map[id1] = null;
            NoMemProtectHeapDelete(l2map);
            Assert(count > 0);
            count--;
        }
    }
}

#if defined(_M_X64_OR_ARM64)

HeapBlockMap64::HeapBlockMap64():
    list(null)
{
}

HeapBlockMap64::~HeapBlockMap64()
{
    Node * node = list;
    list = null;

    while (node != null)
    {
        Node * next = node->next;
        NoMemProtectHeapDelete(node);
        node = next;
    }
}

bool
HeapBlockMap64::EnsureHeapBlock(void * address, size_t pageCount)
{
    uint lowerBitsAddress = (uint)address;
    size_t pageCountLeft = pageCount;
    uint nodePages = HeapBlockMap64::PagesPer4GB - lowerBitsAddress / AutoSystemInfo::PageSize;    
    if (pageCountLeft < nodePages)
    {
        nodePages = (uint)pageCountLeft;
    }
    
    do
    {        
        Node * node = FindOrInsertNode(address);      
        if (node == null || !node->map.EnsureHeapBlock(address, nodePages))
        {
            return false;
        }
        
        pageCountLeft -= nodePages;
        if (pageCountLeft == 0)
        {
            return true;
        }
        address = (void *)((size_t)address + (nodePages * AutoSystemInfo::PageSize));
        nodePages = HeapBlockMap64::PagesPer4GB;
        if (pageCountLeft < HeapBlockMap64::PagesPer4GB)
        {
            nodePages = (uint)pageCountLeft;
        }        
    }
    while (true);
}

void
HeapBlockMap64::SetHeapBlockNoCheck(void * address, size_t pageCount, HeapBlock * heapBlock, HeapBlock::HeapBlockType blockType, byte bucketIndex) 
{
    ForEachNodeInAddressRange(address, pageCount, [&](Node * node, void * address, size_t nodePages)
    {
        Assert(node != null);
        node->map.SetHeapBlockNoCheck(address, nodePages, heapBlock, blockType, bucketIndex);
    });
}

bool
HeapBlockMap64::SetHeapBlock(void * address, size_t pageCount, HeapBlock * heapBlock, HeapBlock::HeapBlockType blockType, byte bucketIndex) 
{
    if (!EnsureHeapBlock(address, pageCount))
    {
        return false;
    }

    SetHeapBlockNoCheck(address, pageCount, heapBlock, blockType, bucketIndex);

    return true;
}

void HeapBlockMap64::ClearHeapBlock(void * address, size_t pageCount)
{
    ForEachNodeInAddressRange(address, pageCount, [&](Node* node, void* address, size_t nodePages)
    {
        Assert(node != null);
        node->map.ClearHeapBlock(address, nodePages);
    });
}

template <class Fn>
void HeapBlockMap64::ForEachNodeInAddressRange(void * address, size_t pageCount, Fn fn)
{
    uint lowerBitsAddress = (uint)address;
    uint nodePages = HeapBlockMap64::PagesPer4GB - lowerBitsAddress / AutoSystemInfo::PageSize;
    if (pageCount < nodePages)
    {
        nodePages = (uint)pageCount;
    }

    do
    {
        Node * node = FindNode(address);

        fn(node, address, nodePages);

        pageCount -= nodePages;
        if (pageCount == 0)
        {
            break;
        }
        address = (void *)((size_t)address + (nodePages * AutoSystemInfo::PageSize));
        nodePages = HeapBlockMap64::PagesPer4GB;
        if (pageCount < HeapBlockMap64::PagesPer4GB)
        {
            nodePages = (uint)pageCount;
        }
    } while (true);
}

HeapBlock *
HeapBlockMap64::GetHeapBlock(void * address) 
{ 
    Node * node = FindNode(address);
    if (node == null)
    {
        return null;
    }
    return node->map.GetHeapBlock(address);
}

HeapBlockMap32::PageMarkBitVector * 
HeapBlockMap64::GetPageMarkBitVector(void * address)
{
    Node * node = FindNode(address);
    Assert(node != null);
    return node->map.GetPageMarkBitVector(address);
}

template <size_t BitCount>
BVStatic<BitCount>* HeapBlockMap64::GetMarkBitVectorForPages(void * address)
{
    Node * node = FindNode(address);
    Assert(node != null);
    return node->map.GetMarkBitVectorForPages<BitCount>(address);
}

template BVStatic<SmallAllocationBlockAttributes::BitVectorCount>* HeapBlockMap64::GetMarkBitVectorForPages<SmallAllocationBlockAttributes::BitVectorCount>(void * address);
template BVStatic<MediumAllocationBlockAttributes::BitVectorCount>* HeapBlockMap64::GetMarkBitVectorForPages<MediumAllocationBlockAttributes::BitVectorCount>(void * address);

uint
HeapBlockMap64::GetMarkCount(void * address, uint pageCount)
{
    uint markCount = 0;

    ForEachNodeInAddressRange(address, pageCount, [&](Node* node, void* address, size_t nodePageCount)
    {
        Assert(node != null);
        markCount += node->map.GetMarkCount(address, nodePageCount);
    });

    return markCount;
}

bool
HeapBlockMap64::IsMarked(void * address) const
{
    Node * node = FindNode(address);
    if (node != null)
    {
        return node->map.IsMarked(address);
    }
    return false;
}

void
HeapBlockMap64::SetMark(void * address)
{
    Node * node = FindNode(address);
    if (node != null)
    {
        node->map.SetMark(address);
    }
}

bool
HeapBlockMap64::TestAndSetMark(void * address)
{
    Node * node = FindNode(address);
    if (node == null)
    {
        return false;
    }
    
    return node->map.TestAndSetMark(address);
}

HeapBlockMap64::Node *
HeapBlockMap64::FindOrInsertNode(void * address)
{
    Node * node = FindNode(address);

    if (node == null)
    {
        node = NoMemProtectHeapNewNoThrowZ(Node, GetNodeStartAddress(address));
        if (node != null)
        {
            node->nodeIndex = GetNodeIndex(address);
            node->next = list;
#ifdef _M_ARM64
            // For ARM we need to make sure that the list remains traversible during this insert.
            MemoryBarrier();
#endif
            list = node;
        }
    }

    return node;
}

HeapBlockMap64::Node *
HeapBlockMap64::FindNode(void * address) const
{
    uint index = GetNodeIndex(address);
    
    Node * node = list;
    while (node != null)
    {
        if (node->nodeIndex == index)
        {
            return node;
        }

        node = node->next;
    }

    return null;
}

void
HeapBlockMap64::ResetMarks()
{
    Node * node = this->list;
    while (node != null)
    {
        node->map.ResetMarks();
        node = node->next;
    }
}

#ifdef CONCURRENT_GC_ENABLED
void
HeapBlockMap64::ResetWriteWatch(Recycler * recycler)
{
    Node * node = this->list;
    while (node != null)
    {
        node->map.ResetWriteWatch(recycler);
        node = node->next;
    }
}

void
HeapBlockMap64::MakeAllPagesReadOnly(Recycler* recycler)
{
    Node * node = this->list;
    while (node != null)
    {
        node->map.MakeAllPagesReadOnly(recycler);
        node = node->next;
    }
}

void
HeapBlockMap64::MakeAllPagesReadWrite(Recycler* recycler)
{
    Node * node = this->list;
    while (node != null)
    {
        node->map.MakeAllPagesReadWrite(recycler);
        node = node->next;
    }
}

uint
HeapBlockMap64::Rescan(Recycler * recycler, bool resetWriteWatch)
{
    uint scannedPageCount = 0;
    
    Node * node = this->list;
    while (node != null)
    {
        scannedPageCount += node->map.Rescan(recycler, resetWriteWatch);
        node = node->next;
    }

    return scannedPageCount;
}

bool
HeapBlockMap64::OOMRescan(Recycler * recycler)
{
    Node * node = this->list;
    while (node != null)
    {
        if (!node->map.OOMRescan(recycler))
        {
            return false;
        }

        node = node->next;
    }
    return true;
}

void 
HeapBlockMap64::Cleanup(bool concurrentFindImplicitRoot)
{
    Node ** prevnext = &this->list;
    Node * node = *prevnext;    
    while (node != null)
    {
        node->map.Cleanup(concurrentFindImplicitRoot);
        Node * nextNode = node->next;
        if (!concurrentFindImplicitRoot && node->map.Empty())
        {            
            // Concurrent traversals of the node list would result in a race and possible UAF.
            // Currently we simply defer node free for the lifetime of the heap (only affects MemProtect).
            *prevnext = node->next;
            NoMemProtectHeapDelete(node);
        }
        else
        {
            prevnext = &node->next;
        }
        node = nextNode;
    }
}

#if DBG
ushort 
HeapBlockMap64::GetPageMarkCount(void * address) const
{
    Node * node = FindNode(address);
    Assert(node != null);

    return node->map.GetPageMarkCount(address);
}

void 
HeapBlockMap64::SetPageMarkCount(void * address, ushort markCount)
{
    Node * node = FindNode(address);
    Assert(node != null);

    node->map.SetPageMarkCount(address, markCount);
}

template void HeapBlockMap64::VerifyMarkCountForPages<SmallAllocationBlockAttributes::BitVectorCount>(void* address, size_t pageCount);
template void HeapBlockMap64::VerifyMarkCountForPages<MediumAllocationBlockAttributes::BitVectorCount>(void* address, size_t pageCount);

template <uint BitVectorCount>
void
HeapBlockMap64::VerifyMarkCountForPages(void * address, size_t pageCount)
{
    Node * node = FindNode(address);
    Assert(node != null);

    node->map.VerifyMarkCountForPages<BitVectorCount>(address, pageCount);
}

#endif

#endif

#ifdef RECYCLER_STRESS
void
HeapBlockMap64::InduceFalsePositives(Recycler * recycler)
{
    Node * node = this->list;
    while (node != null)
    {
        node->map.InduceFalsePositives(recycler);
        node = node->next;
    }
}
#endif

#ifdef RECYCLER_VERIFY_MARK
bool
HeapBlockMap64::IsAddressInNewChunk(void * address)
{
    Node * node = FindNode(address);
    Assert(node != null);

    return node->map.IsAddressInNewChunk(address);
}
#endif

#endif
