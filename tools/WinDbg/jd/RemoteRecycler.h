//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#include "RemotePageAllocator.h"
class RemoteThreadContext;
class RemoteRecycler
{
public:
    RemoteRecycler(ULONG64 recycler);
    RemoteRecycler(ExtRemoteTyped recycler);

    // TODO: avoid using this
    ExtRemoteTyped GetExtRemoteTyped();

    template <typename Fn>
    bool ForEachPageAllocator(PCSTR leafPageAllocatorName, Fn fn);

    ULONG64 GetExternalRootMarker();
    ULONG64 GetPtr();

    bool EnableScanImplicitRoots();
    bool EnableScanInteriorPointers();

    RemoteThreadContext GetThreadContext();
    RemoteHeapBlockMap GetHeapBlockMap();
    JDRemoteTyped GetDefaultHeap();
    JDRemoteTyped GetFieldWithAllocators();

    ULONG GetCookie();

    bool CollectionInProgress();

    bool IsAlignedAddress(ULONG64 address);
    uint GetObjectAlignmentMask();
    uint GetObjectGranularity();
    uint GetObjectAllocationShift();

    bool IsPageHeapEnabled();
private:
    void InitializeObjectAllocationShift();

    JDRemoteTyped recycler;
    ULONG objectAllocationShift;
};

template <typename Fn>
bool RemoteRecycler::ForEachPageAllocator(PCSTR leafPageAllocatorName, Fn fn)
{
    JDRemoteTyped objectWithAllocatorField = this->GetFieldWithAllocators();

    //
    // Before commit 394340a41, the leaf page allocator field was called threadPageAllocator
    // After this commit, the field was moved to HeapInfo and renamed recyclerLeafPageAllocator
    //
    if (objectWithAllocatorField.HasField("threadPageAllocator"))
    {
        if (fn(leafPageAllocatorName, RemotePageAllocator(objectWithAllocatorField.Field("threadPageAllocator"))))
        {
            return true;
        }
    }

    if (objectWithAllocatorField.HasField("recyclerLeafPageAllocator"))
    {
        if (fn(leafPageAllocatorName, RemotePageAllocator(objectWithAllocatorField.Field("recyclerLeafPageAllocator"))))
        {
            return true;
        }
    }

    if (fn("WriteWatch", RemotePageAllocator(objectWithAllocatorField.Field("recyclerPageAllocator"))))
    {
        return true;
    }
    if (objectWithAllocatorField.HasField("recyclerLargeBlockPageAllocator"))
    {
        if (fn("WriteWatchLarge", RemotePageAllocator(objectWithAllocatorField.Field("recyclerLargeBlockPageAllocator"))))
        {
            return true;
        }
    } 
    if (objectWithAllocatorField.HasField("recyclerWithBarrierPageAllocator"))
    {
        if (fn("WriteBarrier", RemotePageAllocator(objectWithAllocatorField.Field("recyclerWithBarrierPageAllocator"))))
        {
            return true;
        }
    }
    if (objectWithAllocatorField.HasField("markStackPageAllocator"))
    {
        if (fn("MarkStack", RemotePageAllocator(objectWithAllocatorField.Field("markStackPageAllocator"))))
        {
            return true;
        }
    }
    else
    {
        // MarkContext
        if (fn("MarkCxt", RemotePageAllocator(recycler.Field("markContext").Field("pagePool").Field("pageAllocator"))))
        {
            return true;
        }
        if (fn("ParaMarkCxt1", RemotePageAllocator(recycler.Field("parallelMarkContext1").Field("pagePool").Field("pageAllocator"))))
        {
            return true;
        }
        if (fn("ParaMarkCxt2", RemotePageAllocator(recycler.Field("parallelMarkContext2").Field("pagePool").Field("pageAllocator"))))
        {
            return true;
        }
        if (fn("ParaMarkCxt3", RemotePageAllocator(recycler.Field("parallelMarkContext3").Field("pagePool").Field("pageAllocator"))))
        {
            return true;
        }
    }  

    if (objectWithAllocatorField.HasField("backgroundProfilerPageAllocator"))
    {
        if (fn("BGProfiler", RemotePageAllocator(objectWithAllocatorField.Field("backgroundProfilerPageAllocator"))))
        {
            return true;
        }
    }
    return false;
}
