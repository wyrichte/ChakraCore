//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// 
// Mark a particular object
// If the object is already marked, or if it's invalid, return true 
// (indicating there's no further processing to be done for this object)
// If the object is newly marked, then the out param heapBlock is written to, and false is returned
//

template <bool interlocked>
__inline
void
HeapBlockMap32::Mark(void * candidate, MarkContext * markContext)
{
    uint id1 = GetLevel1Id(candidate);

    L2MapChunk * chunk = map[id1];
    if (chunk == null)
    {
        // False refernce; no further processing needed.
        return;
    }

    uint bitIndex = chunk->GetMarkBitIndex(candidate);
    if (interlocked)
    {
        // Use an interlocked BTS instruction to ensure atomicity.
        // Since this is expensive, do a non-interlocked test first.
        // Mark bits never go from set to clear during marking, so if we find the bit is already set, we're done.
        if (chunk->markBits.TestIntrinsic(bitIndex))
        {
            // Already marked; no further processing needed
            return;
        }

        if (chunk->markBits.TestAndSetInterlocked(bitIndex))
        {
            // Already marked; no further processing needed
            return;
        }
    }
    else
    {
        if (chunk->markBits.TestAndSet(bitIndex))
        {
            // Already marked; no further processing needed
            return;
        }
    }

#if DBG
    InterlockedIncrement16((short *)&chunk->pageMarkCount[GetLevel2Id(candidate)]);
#endif

    uint id2 = GetLevel2Id(candidate);
    HeapBlock::HeapBlockType blockType = chunk->blockInfo[id2].blockType;

    Assert(blockType == HeapBlock::HeapBlockType::FreeBlockType || chunk->map[id2]->GetHeapBlockType() == blockType);

    // Switch on the HeapBlockType to determine how to process the newly marked object.
    switch (blockType)
    {
    case HeapBlock::HeapBlockType::FreeBlockType:
        // False reference.  Do nothing.
        break;
        
    case HeapBlock::HeapBlockType::SmallLeafBlockType:
    case HeapBlock::HeapBlockType::MediumLeafBlockType:
        // Leaf blocks don't need to be scanned.  Do nothing.
        break;
        
    case HeapBlock::HeapBlockType::SmallNormalBlockType:
#ifdef RECYCLER_WRITE_BARRIER
    case HeapBlock::HeapBlockType::SmallNormalBlockWithBarrierType:
#endif
        {
            byte bucketIndex = chunk->blockInfo[id2].bucketIndex;
            
            // See if it's an invalid offset using the invalid bit vector and if so, do nothing.
            if (!HeapInfo::GetInvalidBitVectorForBucket<SmallAllocationBlockAttributes>(bucketIndex)->Test(SmallHeapBlock::GetAddressBitIndex(candidate)))
            {
                uint objectSize = HeapInfo::GetObjectSizeForBucketIndex<SmallAllocationBlockAttributes>(bucketIndex);
                if (!markContext->AddMarkedObject(candidate, objectSize))
                {
                    // Failed to mark due to OOM.
                    ((SmallHeapBlock *)chunk->map[id2])->SetNeedOOMRescan(markContext->GetRecycler());
                }
            }
        }
        break;
    case HeapBlock::HeapBlockType::MediumNormalBlockType:
#ifdef RECYCLER_WRITE_BARRIER
    case HeapBlock::HeapBlockType::MediumNormalBlockWithBarrierType:
#endif
        {
            byte bucketIndex = chunk->blockInfo[id2].bucketIndex;
            // See if it's an invalid offset using the invalid bit vector and if so, do nothing.
            if (!HeapInfo::GetInvalidBitVectorForBucket<MediumAllocationBlockAttributes>(bucketIndex)->Test(MediumHeapBlock::GetAddressBitIndex(candidate)))
            {
                uint objectSize = HeapInfo::GetObjectSizeForBucketIndex<MediumAllocationBlockAttributes>(bucketIndex);
                if (!markContext->AddMarkedObject(candidate, objectSize))
                {
                    // Failed to mark due to OOM.
                    ((MediumHeapBlock *)chunk->map[id2])->SetNeedOOMRescan(markContext->GetRecycler());
                }
            }
        }
        break;
    case HeapBlock::HeapBlockType::SmallFinalizableBlockType:
#ifdef RECYCLER_WRITE_BARRIER
    case HeapBlock::HeapBlockType::SmallFinalizableBlockWithBarrierType:
#endif
        ((SmallFinalizableHeapBlock*)chunk->map[id2])->ProcessMarkedObject(candidate, markContext);
        break;
    case HeapBlock::HeapBlockType::MediumFinalizableBlockType:
#ifdef RECYCLER_WRITE_BARRIER
    case HeapBlock::HeapBlockType::MediumFinalizableBlockWithBarrierType:
#endif
        ((MediumFinalizableHeapBlock*)chunk->map[id2])->ProcessMarkedObject(candidate, markContext);
        break;

    case HeapBlock::HeapBlockType::LargeBlockType:
        ((LargeHeapBlock*)chunk->map[id2])->Mark(candidate, markContext);
        break;
        
#if DBG
    default:
        AssertMsg(false, "what's the new heap block type?");
#endif
    }
}

#if defined(_M_X64_OR_ARM64)

//
// 64-bit Mark
// See HeapBlockMap32::Mark for explanation of return values
//

template <bool interlocked>
__inline
void
HeapBlockMap64::Mark(void * candidate, MarkContext * markContext)
{
    uint index = GetNodeIndex(candidate);
    
    Node * node = list;
    while (node != null)
    {
        if (node->nodeIndex == index)
        {
            // Found the correct Node.
            // Process the mark and return.
            node->map.Mark<interlocked>(candidate, markContext);
            return;
        }

        node = node->next;
    }

    // No Node found; must be an invalid reference. Do nothing.
}

#endif // defined(_M_X64_OR_ARM64)

