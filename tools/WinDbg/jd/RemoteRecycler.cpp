//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"

RemoteRecycler::RemoteRecycler(ULONG64 recycler) :
    recycler(GetExtension()->FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recycler),
    objectAllocationShift(0), hasCookie(false)
{
}

RemoteRecycler::RemoteRecycler(JDRemoteTyped recycler) : recycler(recycler),
    objectAllocationShift(0), hasCookie(false)
{
}

void
RemoteRecycler::InitializeObjectAllocationShift()
{
    auto firstSizeCat = GetExtension()->GetNumberValue<ULONG64>(this->GetDefaultHeap().Field("heapBuckets").ArrayElement(0).Field("heapBucket").Field("sizeCat"));
    int i = 0;
    while (firstSizeCat > ((ULONG64)1 << (++i)));
    objectAllocationShift = i;
}

uint RemoteRecycler::GetObjectAlignmentMask()
{
    return this->GetObjectGranularity() - 1;
}

uint RemoteRecycler::GetObjectGranularity()
{
    return 1u << this->GetObjectAllocationShift();
}

uint RemoteRecycler::GetObjectAllocationShift()
{
    if (objectAllocationShift == 0)
    {
        InitializeObjectAllocationShift();
    }
    return objectAllocationShift;
}

bool RemoteRecycler::IsAlignedAddress(ULONG64 address)
{
    return (0 == (((size_t)address) & this->GetObjectAlignmentMask()));
}

bool RemoteRecycler::EnableScanImplicitRoots()
{
    return recycler.HasField("enableScanImplicitRoots") && recycler.Field("enableScanImplicitRoots").GetStdBool();
}

bool RemoteRecycler::EnableScanInteriorPointers()
{
    return recycler.HasField("enableScanInteriorPointers") && recycler.Field("enableScanInteriorPointers").GetStdBool();
}

ULONG64 RemoteRecycler::GetPtr()
{
    return recycler.GetPtr();
}

ULONG64 RemoteRecycler::GetExternalRootMarker()
{
    return recycler.Field("externalRootMarker").GetPtr();
}

ULONG RemoteRecycler::GetCookie()
{
    ULONG cookie = this->cookie;
    if (!hasCookie)
    {
        cookie = recycler.Field("Cookie").GetUlong();
        this->cookie = cookie;
    }
    return cookie;
}

ExtRemoteTyped RemoteRecycler::GetExtRemoteTyped()
{
    return recycler.GetExtRemoteTyped();
}

RemoteThreadContext RemoteRecycler::GetThreadContext()
{
    char const * typeName;
    JDRemoteTyped collectionWrapper = recycler.Field("collectionWrapper").CastWithVtable(&typeName);
    if (strcmp(typeName, "ThreadContext") == 0)
    {
        return collectionWrapper;
    }
    return RemoteThreadContext();
}

JDRemoteTyped RemoteRecycler::GetDefaultHeap()
{
    JDRemoteTyped autoHeap = recycler.Field("autoHeap");

    if (autoHeap.HasField("defaultHeap"))
    {
        JDRemoteTyped defaultHeap = autoHeap.Field("defaultHeap");
        return defaultHeap;
    }

    return autoHeap;
}

JDRemoteTyped RemoteRecycler::GetFieldWithAllocators()
{
    JDRemoteTyped objectWithAllocatorField;

    if (this->GetDefaultHeap().HasField("recyclerPageAllocator"))
    {
        objectWithAllocatorField = this->GetDefaultHeap();
    }
    else
    {
        objectWithAllocatorField = recycler;
    }

    return objectWithAllocatorField;
}

RemoteHeapBlockMap RemoteRecycler::GetHeapBlockMap()
{
    return RemoteHeapBlockMap(recycler.Field("heapBlockMap"));
}

bool RemoteRecycler::CollectionInProgress()
{
    return (!ENUM_EQUAL(recycler.Field("collectionState").GetSimpleValue(), CollectionStateNotCollecting));
}

bool RemoteRecycler::IsPageHeapEnabled()
{
    return recycler.HasField("isPageHeapEnabled") && recycler.Field("isPageHeapEnabled").GetStdBool();
}
