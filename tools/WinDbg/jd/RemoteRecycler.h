//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------
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
    void ForEachPageAllocator(PCSTR leafPageAllocatorName, Fn fn);

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
private:
    void InitializeObjectAllocationShift();

    JDRemoteTyped recycler;
    ULONG objectAllocationShift;
};

template <typename Fn>
void RemoteRecycler::ForEachPageAllocator(PCSTR leafPageAllocatorName, Fn fn)
{
    fn(leafPageAllocatorName, RemotePageAllocator(recycler.Field("threadPageAllocator")));
    fn("WriteWatch", RemotePageAllocator(recycler.Field("recyclerPageAllocator")));
    if (recycler.HasField("recyclerLargeBlockPageAllocator"))
    {
        fn("WriteWatchLarge", RemotePageAllocator(recycler.Field("recyclerLargeBlockPageAllocator")));
    } 
    if (recycler.HasField("recyclerWithBarrierPageAllocator"))
    {
        fn("WriteBarrier", RemotePageAllocator(recycler.Field("recyclerWithBarrierPageAllocator")));
    }
    if (recycler.HasField("markStackPageAllocator"))
    {
        fn("MarkStack", RemotePageAllocator(recycler.Field("markStackPageAllocator")));
    }
    else
    {
        // MarkContext
        fn("MarkCxt", RemotePageAllocator(recycler.Field("markContext").Field("pagePool").Field("pageAllocator")));
        fn("ParaMarkCxt1", RemotePageAllocator(recycler.Field("parallelMarkContext1").Field("pagePool").Field("pageAllocator")));
        fn("ParaMarkCxt2", RemotePageAllocator(recycler.Field("parallelMarkContext2").Field("pagePool").Field("pageAllocator")));
        fn("ParaMarkCxt3", RemotePageAllocator(recycler.Field("parallelMarkContext3").Field("pagePool").Field("pageAllocator")));
    }  

    if (recycler.HasField("backgroundProfilerPageAllocator"))
    {
        fn("BGProfiler", RemotePageAllocator(recycler.Field("backgroundProfilerPageAllocator")));
    }
}
// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------