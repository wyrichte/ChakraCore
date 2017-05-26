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
    if (fn(leafPageAllocatorName, RemotePageAllocator(recycler.Field("threadPageAllocator"))))
    {
        return true;
    }
    if (fn("WriteWatch", RemotePageAllocator(recycler.Field("recyclerPageAllocator"))))
    {
        return true;
    }
    if (recycler.HasField("recyclerLargeBlockPageAllocator"))
    {
        if (fn("WriteWatchLarge", RemotePageAllocator(recycler.Field("recyclerLargeBlockPageAllocator"))))
        {
            return true;
        }
    } 
    if (recycler.HasField("recyclerWithBarrierPageAllocator"))
    {
        if (fn("WriteBarrier", RemotePageAllocator(recycler.Field("recyclerWithBarrierPageAllocator"))))
        {
            return true;
        }
    }
    if (recycler.HasField("markStackPageAllocator"))
    {
        if (fn("MarkStack", RemotePageAllocator(recycler.Field("markStackPageAllocator"))))
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

    if (recycler.HasField("backgroundProfilerPageAllocator"))
    {
        if (fn("BGProfiler", RemotePageAllocator(recycler.Field("backgroundProfilerPageAllocator"))))
        {
            return true;
        }
    }
    return false;
}
