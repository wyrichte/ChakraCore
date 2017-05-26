//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemotePageAllocator
{
public:
    RemotePageAllocator(ExtRemoteTyped pageAllocator) : pageAllocator(pageAllocator) {}
    ULONG64 GetUsedBytes();
    ULONG64 GetReservedBytes();
    ULONG64 GetCommittedBytes();
    ULONG64 GetUnusedBytes();

    static void DisplayDataHeader(PCSTR name);
    static void DisplayDataLine();
    static void DisplayData(ULONG nameLength, ULONG64 used, ULONG64 reserved, ULONG64 committed, ULONG64 unused);
    void DisplayData(PCSTR name, bool showZeroEntries);
    template <typename Fn>
    bool ForEachSegment(Fn fn)
    {
        char* segmentsNames[] = { "segments", "fullSegments", "emptySegments", "decommitSegments", "largeSegments" };

        for (int i=0; i < _countof(segmentsNames); i++)
        {            
            if(pageAllocator.HasField(segmentsNames[i]))
            {
                auto list = pageAllocator.Field(segmentsNames[i]).GetPointerTo();
                ExtRemoteTyped curr = list.Field("next");
                while (curr.Field("base").GetPtr() != list.GetPtr())
                {
                    if (fn(segmentsNames[i], curr.Field("node.data")))
                    {
                        return true;
                    }
                    curr = curr.Field("base.next");
                }
            }
        }
        return false;
    }
    ExtRemoteTyped GetExtRemoteTyped() { return pageAllocator; }
private:
    void ComputeReservedAndCommittedBytes();
    
    ExtRemoteTyped pageAllocator;
    Nullable<ULONG64> freedBytes;
    Nullable<ULONG64> reservedBytes;
    Nullable<ULONG64> committedBytes;
};
