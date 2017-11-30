//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class RemoteRecycler;
class RemoteBitVector;

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
    RemoteHeapBlock(RemoteHeapBlock const&);
    ~RemoteHeapBlock();

    char GetType();

    bool IsSmallNormalHeapBlock();
    bool IsMediumNormalHeapBlock();
    bool IsSmallNormalWithBarrierHeapBlock();
    bool IsMediumNormalWithBarrierHeapBlock();
    bool IsSmallLeafHeapBlock();
    bool IsMediumLeafHeapBlock();
    bool IsSmallFinalizableHeapBlock();
    bool IsMediumFinalizableHeapBlock();
    bool IsSmallFinalizableWithBarrierHeapBlock();
    bool IsMediumFinalizableWithBarrierHeapBlock();
    bool IsSmallRecyclerVisitedHostHeapBlock();
    bool IsMediumRecyclerVisitedHostHeapBlock();
    bool IsLargeHeapBlock();
    bool IsLeafHeapBlock();
    bool IsFinalizableHeapBlock();
    bool IsSmallHeapBlock();

    ULONG64 GetHeapBlockAddress() { return heapBlockAddress; }
    ULONG64 GetAddress() { return address; }
    ULONG64 GetSize();
    ULONG64 GetAllocatableSize();
    ULONG GetFinalizeCount();

    ULONG64 GetTotalObjectSize();
    ULONG GetTotalObjectCount();

    ULONG64 GetFreeObjectSize();
    ULONG GetFreeObjectCount();

    ULONG64 GetAllocatedObjectSize();
    ULONG GetAllocatedObjectCount();

    ULONG GetBucketObjectSize() { return bucketObjectSize; }

    USHORT GetAddressBitIndex(ULONG64 objectAddress);

    RemoteBitVector GetMarkBits();
    RemoteBitVector GetFreeBits();

    ExtRemoteTyped GetExtRemoteTyped();

    bool GetRecyclerHeapObjectInfo(ULONG64 originalAddress, HeapObjectInfo& info, bool interior, bool verbose = false);
    void VerboseOut();

    template <typename Fn>
    bool ForEachLargeObjectHeader(Fn fn)
    {
        Assert(IsLargeHeapBlock());

        JDRemoteTyped heapBlock = GetExtRemoteTyped();

        unsigned int allocCount = heapBlock.Field("allocCount").GetUlong();
        JDRemoteTyped headerList =
            JDRemoteTyped(GetExtension()->FillModuleAndMemoryNS("(%s!%sLargeObjectHeader **)@$extin"), this->GetHeapBlockAddress() + heapBlock.GetTypeSize());

        for (unsigned int i = 0; i < allocCount; i++)
        {
            JDRemoteTyped header = headerList.ArrayElement(i);
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

    class AutoDebuggeeMemory
    {
    public:
        AutoDebuggeeMemory(RemoteHeapBlock * heapBlock, ULONG64 address, ULONG size) : heapBlock(heapBlock) 
        {
            debuggeeMemory = heapBlock->GetDebuggeeMemory(address, size, &cached);
        };
        ~AutoDebuggeeMemory()
        {
            if (!cached)
            {
                delete[] debuggeeMemory;
            }            
        }
        operator char*() { return debuggeeMemory;  }

    private:
        RemoteHeapBlock * heapBlock;
        char * debuggeeMemory;
        bool cached;
    };

private:
    RemoteRecycler GetRecycler();

    char * GetDebuggeeMemory(ULONG64 address, ULONG size, bool * cached);
    ULONG GetObjectIndex(ULONG64 objectAddress);
    ULONG64 GetObjectAddressFromIndex(ULONG objectIndex);
    ULONG GetRecyclerCookie();
    ULONG GetLargeHeapBlockHeaderList(JDRemoteTyped& headerList);
    void Initialize();
    void EnsureCachedTotalObjectCountAndSize();
    void EnsureCachedAllocatedObjectCountAndSize();

    ULONG64 heapBlockAddress;
    ULONG64 address;
    ULONG64 size;

    char type;
    bool hasCachedTotalObjectCountAndSize;
    bool hasCachedAllocatedObjectCountAndSize;

    ULONG bucketObjectSize;
    ULONG totalObjectCount;
    ULONG64 totalObjectSize;

    ULONG allocatedObjectCount;
    ULONG64 allocatedObjectSize;
    std::vector<bool> freeObject;
    UCHAR * attributes;

    char * debuggeeMemory;
};
