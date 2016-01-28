//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

ULONG64
RemotePageAllocator::GetUsedBytes()
{
    return ExtRemoteTypedUtil::GetSizeT(pageAllocator.Field("usedBytes"));
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
    auto accumulateSegments = [&](ExtRemoteTyped segment)
    {
        reserved += ExtRemoteTypedUtil::GetSizeT(segment.Field("segmentPageCount")) * 4096;
        return false;
    };
    auto accumulatePageSegments = [&](ExtRemoteTyped pageSegment)
    {
        accumulateSegments(pageSegment);
        decommitted += ExtRemoteTypedUtil::GetSizeT(pageSegment.Field("decommitPageCount")) * 4096;
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
RemotePageAllocator::DisplayDataHeader(PCSTR name)
{
    DisplayDataLine();
    ExtOut("%-16s   Used Bytes    Reserved   Committed      Unused    Commit%%      Used%%\n", name);
    DisplayDataLine();
}

void
RemotePageAllocator::DisplayDataLine()
{
    ExtOut("---------------------------------------------------------------------------------------\n");
}

void
RemotePageAllocator::DisplayData(ULONG nameLength, ULONG64 used, ULONG64 reserved, ULONG64 committed)
{
    int numSpace = 16 - nameLength;
    for (int i = 0; i < numSpace; i++)
    {
        ExtOut(" ");
    }
    ExtOut(": ");
    ExtOut("% 11I64u % 11I64u % 11I64u % 11I64u    %6.2f%%    %6.2f%%\n",
        used, reserved, committed, (committed - used), 100.0 * (committed / (double)reserved), 100.0 * (used / (double)committed));
}

void
RemotePageAllocator::DisplayData(PCSTR name, bool showZeroEntries)
{
    ULONG64 used = this->GetUsedBytes();
    ULONG64 reserved = this->GetReservedBytes();
    ULONG64 committed = this->GetCommittedBytes();

    if (showZeroEntries || used != 0 || reserved != 0 || committed != 0)
    {
        PCSTR typeName = pageAllocator.GetTypeName();
        std::string encodedTypeName = JDUtil::EncodeDml(JDUtil::StripStructClass(typeName));
        g_Ext->Dml("<link cmd=\"dt %s!%s %p\">%s</link>", ((EXT_CLASS_BASE*)g_ExtInstancePtr)->FillModule("%s"), encodedTypeName.c_str(), JDUtil::IsPointerType(typeName)? pageAllocator.GetPtr() : pageAllocator.GetPointerTo().GetPtr(), name);
        DisplayData((ULONG)strlen(name), used, reserved, committed);
    }
}


// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
