//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

struct HeapObjectInfo
{
    ULONG64 originalAddress;
    ULONG64 objectAddress;
    ULONG objectSize;
    UCHAR attributes;

    bool IsLeaf();
};

class RemoteHeapBlock
{
public:

    RemoteHeapBlock();
    RemoteHeapBlock(ULONG64 heapBlockAddress);
    RemoteHeapBlock(ExtRemoteTyped& heapBlock);

    char GetType();

    bool IsSmallLeafHeapBlock();
    bool IsMediumLeafHeapBlock();
    bool IsSmallFinalizableHeapBlock();
    bool IsMediumFinalizableHeapBlock();
    bool IsSmallFinalizableWithBarrierHeapBlock();
    bool IsMediumFinalizableWithBarrierHeapBlock();
    bool IsLargeHeapBlock();
    bool IsLeafHeapBlock();
    bool IsFinalizableHeapBlock();

    ULONG64 GetHeapBlockAddress() { return heapBlockAddress; }
    ULONG64 GetAddress() { return address; }
    ULONG64 GetSize();
    ULONG GetFinalizeCount();

    ULONG64 GetTotalObjectSize();
    ULONG GetTotalObjectCount();

    ULONG64 GetFreeObjectCount();
    ULONG64 GetFreeObjectSize();    

    ULONG GetBucketObjectSize() { return bucketObjectSize; }

    ExtRemoteTyped GetExtRemoteTyped();

    bool GetRecyclerHeapObjectInfo(ULONG64 originalAddress, HeapObjectInfo& info, bool interior, bool verbose = false);
    void VerboseOut();

    template <typename Fn>
    bool ForEachLargeObjectHeader(Fn fn)
    {
        Assert(IsLargeHeapBlock());
        
        ExtRemoteTyped heapBlock = GetExtRemoteTyped();

        unsigned int allocCount = heapBlock.Field("allocCount").GetUlong();
        ExtRemoteTyped headerList =
            ExtRemoteTyped(GetExtension()->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader **)@$extin"), this->GetHeapBlockAddress() + heapBlock.GetTypeSize());

        for (unsigned int i = 0; i < allocCount; i++)
        {
            ExtRemoteTyped header = headerList.ArrayElement(i);
            if (header.GetPtr() == 0)
            {
                continue;
            }
            if (fn(header))
            {
                return true;
            }
        }
        return false;
    }

    static RemoteHeapBlock NullHeapBlock;
private:
    ULONG GetRecyclerCookie();
    ULONG GetLargeHeapBlockHeaderList(ExtRemoteTyped& headerList);
    void Initialize();
    void EnsureCachedTotalObjectCountAndSize();

    ULONG64 heapBlockAddress;
    ULONG64 address;
    char type;
    ULONG64 size;

    bool hasCachedTotalObjectCountAndSize;      
    ULONG bucketObjectSize;
    ULONG totalObjectCount;
    ULONG64 totalObjectSize;
};