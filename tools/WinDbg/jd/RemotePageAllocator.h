//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#include "RemoteRecyclerList.h"

class RemotePageAllocator
{
public:
    RemotePageAllocator(JDRemoteTyped pageAllocator) : pageAllocator(pageAllocator.GetExtRemoteTyped()) {}
    ULONG64 GetUsedBytes();
    ULONG64 GetReservedBytes(bool forceCompute);
    ULONG64 GetCommittedBytes(bool forceCompute);
    ULONG64 GetUnusedBytes();

    static void DisplayDataHeader(PCSTR name);
    static void DisplayDataLine();
    static void DisplayData(ULONG nameLength, ULONG64 used, ULONG64 reserved, ULONG64 committed, ULONG64 unused);
    void DisplayData(PCSTR name, bool showZeroEntries, bool forceCompute);
    template <typename Fn>
    bool ForEachSegment(Fn fn)
    {
        return ForEachSegment(fn, fn);
    }

    template <typename Fn1, typename Fn2>
    bool ForEachSegment(Fn1 fnPageSegment, Fn2 fnSegment)
    {
        char const* pageSegmentsNames[] = { "segments", "fullSegments", "emptySegments", "decommitSegments" };
        char const * segmentsName[] = { "largeSegments" };
        return ForEachSegment(pageSegmentsNames, _countof(pageSegmentsNames), fnPageSegment)
            || ForEachSegment(segmentsName, _countof(segmentsName), fnSegment);
    }
    ExtRemoteTyped GetExtRemoteTyped() { return pageAllocator; }
private:
    template <typename Fn>
    bool ForEachSegment(char const ** names, uint count, Fn fn)
    {
        for (uint i = 0; i < count; i++)
        {
            if (pageAllocator.HasField(names[i]))
            {
                auto list = pageAllocator.Field(names[i]).GetPointerTo();
                if (DListForEach(list, [&](ExtRemoteTyped segment) { return fn(names[i], segment); }))
                {
                    return true;
                }
            }
        }
        return false;
    }

    void ComputeReservedAndCommittedBytes();
    
    ExtRemoteTyped pageAllocator;
    Nullable<ULONG64> freedBytes;
    Nullable<ULONG64> reservedBytes;
    Nullable<ULONG64> committedBytes;
};
