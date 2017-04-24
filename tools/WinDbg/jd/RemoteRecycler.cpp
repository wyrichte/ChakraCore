//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"


// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

RemoteRecycler::RemoteRecycler(ULONG64 recycler) :
    recycler(GetExtension()->FillModuleAndMemoryNS("(%s!%sRecycler*)@$extin"), recycler)
{
    InitializeObjectAllocationShift();
}

RemoteRecycler::RemoteRecycler(ExtRemoteTyped recycler) : recycler(recycler)
{
    InitializeObjectAllocationShift();
}

void
RemoteRecycler::InitializeObjectAllocationShift()
{
    auto firstSizeCat = GetExtension()->GetNumberValue<ULONG64>(recycler.Field("autoHeap").Field("heapBuckets").ArrayElement(0).Field("heapBucket").Field("sizeCat"));
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
    return recycler.Field("Cookie").GetUlong();
}

ExtRemoteTyped RemoteRecycler::GetExtRemoteTyped()
{
    return recycler;
}

RemoteThreadContext RemoteRecycler::GetThreadContext()
{
    return recycler.Field("collectionWrapper").CastWithVtable();
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
    return recycler.Field("isPageHeapEnabled").GetStdBool();
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------