//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

template <typename TBlockType>
template <ObjectInfoBits attributes, bool nothrow>
__inline char *
HeapBucketT<TBlockType>::RealAlloc(Recycler * recycler, size_t sizeCat)
{
    Assert(sizeCat == this->sizeCat);

    char * memBlock = allocatorHead.InlinedAlloc<(ObjectInfoBits)(attributes & InternalObjectInfoBitMask)>(recycler, sizeCat);

    if (memBlock == null)
    {
        memBlock = SnailAlloc(recycler, &allocatorHead, sizeCat, attributes, nothrow);
        Assert(memBlock != null || nothrow);
    }
    else
    {
        Assert(allocatorHead.heapBlock == nullptr || !allocatorHead.heapBlock->InPageHeapMode());
    }

    // If this API is called and throwing is not allowed, 
    // check if we actually allocated a block before verifying 
    // its zero fill state. If it is null, return that here.
    if (nothrow)
    {
        if (memBlock == nullptr)
        {
            return nullptr;
        }
    }

#ifdef RECYCLER_ZERO_MEM_CHECK
    // Do the verify zero fill only if it's not a nothrow alloc
    if ((attributes & ObjectInfoBits::LeafBit) == 0
#ifdef RECYCLER_WRITE_BARRIER_ALLOC_THREAD_PAGE
        && ((attributes & ObjectInfoBits::WithBarrierBit) == 0)
#endif
        )
    {
        // Skip the first and the last pointer objects- the first may have next pointer for the free list
        // the last might have the old size of the object if this was allocated from an explicit free list
        recycler->VerifyZeroFill(memBlock + sizeof(FreeObject), sizeCat - (2 * sizeof(FreeObject)));
    }
#endif

    return memBlock;
}

template <typename TBlockType>
void
HeapBucketT<TBlockType>::ExplicitFree(void* object, size_t sizeCat)
{
    FreeObject* explicitFreeObject = (FreeObject*) object;
    if (lastExplicitFreeListAllocator->IsExplicitFreeObjectListAllocMode())
    {
        explicitFreeObject->SetNext(lastExplicitFreeListAllocator->GetFreeObjectList());
        lastExplicitFreeListAllocator->SetFreeObjectList(explicitFreeObject);
    }
    else
    {
        explicitFreeObject->SetNext(this->explicitFreeList);
        this->explicitFreeList = explicitFreeObject;
    }

    // Dont' fill memory fill pattern here since we're still pretending like the object
    // is allocated to other parts of the GC
}

#if DBG || defined(RECYCLER_SLOW_CHECK_ENABLED)
inline
Recycler *
HeapBucket::GetRecycler() const
{
    return this->heapInfo->recycler;
}
#endif