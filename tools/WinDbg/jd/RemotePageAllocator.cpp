//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------
#include "RemotePageAllocator.h"

ULONG64
RemotePageAllocator::GetUsedBytes()
{
    return EXT_CLASS_BASE::GetSizeT(pageAllocator.Field("usedBytes"));
}

ULONG64
RemotePageAllocator::GetReservedBytes()
{
    if (!this->reservedBytes.HasValue())
    {
        if (GetExtension()->PageAllocatorHasExtendedCounters())
        {
            this->reservedBytes = EXT_CLASS_BASE::GetSizeT(pageAllocator.Field("reservedBytes"));
        }
        else
        {
            ComputeReservedAndCommittedBytes();
        }
    }
    return this->reservedBytes;
}

ULONG64
RemotePageAllocator::GetCommittedBytes()
{
    if (!this->committedBytes.HasValue())
    {
        if (GetExtension()->PageAllocatorHasExtendedCounters())
        {
            this->committedBytes = EXT_CLASS_BASE::GetSizeT(pageAllocator.Field("committedBytes"));
        }
        else
        {
            ComputeReservedAndCommittedBytes();
        }
    }
    return this->committedBytes;
}

void
RemotePageAllocator::ComputeReservedAndCommittedBytes()
{    
    ULONG64 reserved = 0;
    ULONG64 decommitted = 0;
    auto accumulateSegments = [&](ExtRemoteTyped segment)
    {
        reserved += EXT_CLASS_BASE::GetSizeT(segment.Field("segmentPageCount")) * 4096;
        return false;
    };
    auto accumulatePageSegments = [&](ExtRemoteTyped pageSegment)
    {
        accumulateSegments(pageSegment);
        decommitted += EXT_CLASS_BASE::GetSizeT(pageSegment.Field("decommitPageCount")) * 4096;
        return false;
    };
    SListForEach(pageAllocator.Field("segments").GetPointerTo(), accumulatePageSegments);
    SListForEach(pageAllocator.Field("fullSegments").GetPointerTo(), accumulatePageSegments);
    SListForEach(pageAllocator.Field("emptySegments").GetPointerTo(), accumulatePageSegments);
    SListForEach(pageAllocator.Field("decommitSegments").GetPointerTo(), accumulatePageSegments);
    SListForEach(pageAllocator.Field("largeSegments").GetPointerTo(), accumulateSegments);

    this->reservedBytes = reserved;
    this->committedBytes = reserved - decommitted;
}

void
RemotePageAllocator::DisplayDataHeader()
{
    ExtOut("-------------------------------------------------------------------------------------\n");
    ExtOut("Allocator     Used Bytes    Reserved    Committed         Unused    Commit%%      Used%%\n");
    ExtOut("-------------------------------------------------------------------------------------\n");
}

void
RemotePageAllocator::DisplayData(PCSTR name, bool showZeroEntries)
{
    ULONG64 used = this->GetUsedBytes();
    ULONG64 reserved = this->GetReservedBytes();
    ULONG64 committed = this->GetCommittedBytes();

    if (showZeroEntries || used != 0 || reserved != 0 || committed != 0)
    {
        ExtOut("%-11s: %11I64u %11I64u  %11I64u    %11I64u    %6.2f%%    %6.2f%%\n", name, used, reserved, committed, (committed - used), 100.0 * (committed / (double)reserved), 100.0 * (used / (double)committed));
    }      
}


// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------