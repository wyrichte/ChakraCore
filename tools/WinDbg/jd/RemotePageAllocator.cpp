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
    return ExtRemoteTypedUtil::GetSizeT(pageAllocator.Field("usedBytes"));
}

ULONG64
RemotePageAllocator::GetUnusedBytes()
{
    if (!this->freedBytes.HasValue())
    {
        // usedBytes in the page allocator
        ComputeReservedAndCommittedBytes();
    }
    return this->freedBytes;
}

ULONG64
RemotePageAllocator::GetReservedBytes()
{
    if (!this->reservedBytes.HasValue())
    {
        if (GetExtension()->PageAllocatorHasExtendedCounters())
        {
            this->reservedBytes = ExtRemoteTypedUtil::GetSizeT(pageAllocator.Field("reservedBytes"));
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
            this->committedBytes = ExtRemoteTypedUtil::GetSizeT(pageAllocator.Field("committedBytes"));
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
    ULONG64 freed = 0;
    auto accumulateSegments = [&](ExtRemoteTyped segment)
    {
        reserved += ExtRemoteTypedUtil::GetSizeT(segment.Field("segmentPageCount")) * 4096;
        return false;
    };
    auto accumulatePageSegments = [&](ExtRemoteTyped pageSegment)
    {
        accumulateSegments(pageSegment);
        decommitted += ExtRemoteTypedUtil::GetSizeT(pageSegment.Field("decommitPageCount")) * 4096;
        freed += ExtRemoteTypedUtil::GetSizeT(pageSegment.Field("freePageCount")) * 4096;
        return false;
    };
    SListForEach(pageAllocator.Field("segments").GetPointerTo(), accumulatePageSegments);
    SListForEach(pageAllocator.Field("fullSegments").GetPointerTo(), accumulatePageSegments);
    SListForEach(pageAllocator.Field("emptySegments").GetPointerTo(), accumulatePageSegments);
    SListForEach(pageAllocator.Field("decommitSegments").GetPointerTo(), accumulatePageSegments);
    SListForEach(pageAllocator.Field("largeSegments").GetPointerTo(), accumulateSegments);

    this->reservedBytes = reserved;
    this->committedBytes = reserved - decommitted;
    this->freedBytes = freed;
}

void
RemotePageAllocator::DisplayDataHeader(PCSTR name)
{
    DisplayDataLine();
    ExtOut("%-16s    Committed        Used      Unused |   Used%% |    Reserved    Disabled   Commit%%     Disabled%%\n", name);
    DisplayDataLine();
}

void
RemotePageAllocator::DisplayDataLine()
{
    ExtOut("-----------------------------------------------------------------------------------------------------------------\n");
}

void
RemotePageAllocator::DisplayData(ULONG nameLength, ULONG64 used, ULONG64 reserved, ULONG64 committed, ULONG64 unused)
{
    int numSpace = 16 - nameLength;
    for (int i = 0; i < numSpace; i++)
    {
        ExtOut(" ");
    }

    ULONG64 actualCommitted = used + unused;
    ULONG64 disabled = committed - actualCommitted;
    ExtOut(": ");
    ExtOut("% 11I64u % 11I64u % 11I64u | %6.2f%% | % 11I64u % 11I64u   %6.2f%%      %6.2f%%\n",
        actualCommitted, used, unused, 100.0 * (used / (double)actualCommitted),
        reserved, disabled, 100.0 * (actualCommitted / (double)reserved), 100.0 * (disabled / (double)reserved));
}

void
RemotePageAllocator::DisplayData(PCSTR name, bool showZeroEntries)
{
    ULONG64 used = this->GetUsedBytes();
    ULONG64 reserved = this->GetReservedBytes();
    ULONG64 committed = this->GetCommittedBytes();
    ULONG64 unused = this->GetUnusedBytes();

    if (showZeroEntries || used != 0 || reserved != 0 || committed != 0 || unused != 0)
    {
        PCSTR typeName = pageAllocator.GetTypeName();
        if (GetExtension()->PreferDML())
        {
            std::string encodedTypeName = JDUtil::EncodeDml(JDUtil::StripStructClass(typeName));
            GetExtension()->Dml("<link cmd=\"dt %s!%s %p\">%s</link>", GetExtension()->FillModule("%s"), encodedTypeName.c_str(), JDUtil::IsPointerType(typeName) ? pageAllocator.GetPtr() : pageAllocator.GetPointerTo().GetPtr(), name);
        }
        else
        {
            GetExtension()->Out("/*\"dt %s!%s %p\" to display*/\n", GetExtension()->FillModule("%s"), JDUtil::StripStructClass(typeName), JDUtil::IsPointerType(typeName) ? pageAllocator.GetPtr() : pageAllocator.GetPointerTo().GetPtr());
            GetExtension()->Out("%s", name);
        }
        DisplayData((ULONG)strlen(name), used, reserved, committed, unused);
    }
}


// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
