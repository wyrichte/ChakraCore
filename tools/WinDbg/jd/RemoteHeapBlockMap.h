//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

#include "RemoteHeapBlock.h"

class RemoteHeapBlockMap
{
public:
    class HeapBlockAddressHashCompare
    {
    public:
        static const size_t bucket_size = 4;
        static const size_t min_buckets = 8;
        size_t operator()(ULONG64 address) const
        {
            return (size_t)(address / 8);       // On 64-bit  the heap block address are 8 byte aligned, but on 32-bit, they can be 4 byte, but we can use one bit and still have a reasonable hash.
        }
        bool operator()(ULONG64 a, ULONG64 b) const
        {
            return a < b;
        }
    };
    class PageAddressHashCompare
    {
    public:
        static const size_t bucket_size = 4;
        static const size_t min_buckets = 8;
        size_t operator()(ULONG64 address) const
        {
            Assert((address / 4096) * 4096 == address);
            return (size_t)(address / 4096);
        }
        bool operator()(ULONG64 a, ULONG64 b) const
        {
            return a < b;
        }
    };
    struct Cache
    {
        stdext::hash_map<ULONG64, RemoteHeapBlock *, PageAddressHashCompare> addressToHeapBlockMap;
        stdext::hash_map<ULONG64, RemoteHeapBlock, HeapBlockAddressHashCompare> heapBlockAddressToHeapBlockMap;
    };
    RemoteHeapBlockMap(JDRemoteTyped heapBlockMap);

    template <typename Fn>
    bool ForEachHeapBlock(Fn fn);

    template <typename Fn>
    bool ForEachHeapBlockDirect(ExtRemoteTyped& heapBlockMap, Fn processFunction);

    RemoteHeapBlock * FindHeapBlock(ULONG64 address);
private:
    template <typename Fn>
    bool ForEachHeapBlockRaw(JDRemoteTyped& heapBlockMap, Fn processFunction);

    template <typename Fn>
    bool ProcessL1Chunk(ULONG64 nodeIndex, JDRemoteTyped& l1ChunkArray, Fn processFunction);

    template <typename Fn>
    bool ProcessL2Chunk(ULONG64 nodeIndex, ULONG64 l1Id, JDRemoteTyped& l2ChunkArray, Fn processFunction);

protected:    
    Cache * cachedHeapBlock;
    ULONG64 l1ChunkSize;
    ULONG64 l2ChunkSize;
};

template <typename Fn>
bool RemoteHeapBlockMap::ForEachHeapBlock(Fn fn)
{
    for (auto i = cachedHeapBlock->heapBlockAddressToHeapBlockMap.begin(); i != cachedHeapBlock->heapBlockAddressToHeapBlockMap.end(); i++)
    {
        if (fn((*i).second))
        {
            return true;
        }
    }

    return false;
}

template <typename Fn>
bool RemoteHeapBlockMap::ForEachHeapBlockDirect(ExtRemoteTyped& heapBlockMap, Fn fn)
{
    // Heap block can cross multiple entries, filter them
    ULONG64 lastHeapBlock = 0;
    auto processFunction = [&](ULONG64 nodeIndex, ULONG64 l1, ULONG64 l2, ULONG64 block, RemoteHeapBlock& heapBlock)
    {
        if (lastHeapBlock == heapBlock.GetHeapBlockAddress())
        {
            return false;
        }
        lastHeapBlock = heapBlock.GetHeapBlockAddress();
        return fn(nodeIndex, l1, l2, block, heapBlock);
    };

    return ForEachHeapBlockRaw(heapBlockMap, processFunction);
}

template <typename Fn>
bool RemoteHeapBlockMap::ForEachHeapBlockRaw(JDRemoteTyped& heapBlockMap, Fn processFunction)
{
    if (g_Ext->m_PtrSize == 4)
    {
        JDRemoteTyped map = heapBlockMap.Field("map");
        return ProcessL1Chunk(0, map, processFunction);
    }
    else
    {
        JDRemoteTyped current = heapBlockMap.Field("list");
        while (current.GetPtr() != 0)
        {
            ULONG64 nodeIndex = current.Field("nodeIndex").GetUlong();
            JDRemoteTyped map = current.Field("map");
            JDRemoteTyped l1ChunkArray = map.Field("map");
            if (ProcessL1Chunk(nodeIndex, l1ChunkArray, processFunction))
            {
                return true;
            }

            current = current.Field("next");
        }
    }

    return false;
}

template <typename Fn>
bool RemoteHeapBlockMap::ProcessL1Chunk(ULONG64 nodeIndex, JDRemoteTyped& chunk, Fn fn)
{
    const size_t l1Count = chunk.GetTypeSize() / chunk.ArrayElement(0).GetTypeSize();
    l1ChunkSize = l1Count;
    for (size_t l1Id = 0; l1Id < l1Count; l1Id++)
    {
        JDRemoteTyped l2MapChunk = chunk.ArrayElement(l1Id);
        if (ExtRemoteTypedUtil::GetAsPointer(l2MapChunk.GetExtRemoteTyped()) != 0)
        {
            JDRemoteTyped l2MapChunkDeref = l2MapChunk.Dereference();
            if (ProcessL2Chunk(nodeIndex, l1Id, l2MapChunkDeref, fn))
            {
                return true;
            }
        }
    }

    return false;
}

template <typename Fn>
bool
RemoteHeapBlockMap::ProcessL2Chunk(ULONG64 nodeIndex, ULONG64 l1Id, JDRemoteTyped& chunk, Fn fn)
{
    ExtRemoteTyped l2map = chunk.Field("map").GetExtRemoteTyped();
    const size_t l2Count = l2map.GetTypeSize() / l2map.ArrayElement(0).GetTypeSize();
    l2ChunkSize = l2Count;
    for (size_t l2Id = 0; l2Id < l2Count; l2Id++)
    {
        ExtRemoteTyped heapBlock = l2map.ArrayElement(l2Id);
        ULONG64 block = ExtRemoteTypedUtil::GetAsPointer(heapBlock);
        if (block != 0)
        {
            RemoteHeapBlock remoteHeapBlock(block);
            if (fn(nodeIndex, l1Id, l2Id, block, remoteHeapBlock))
            {
                return true;
            }
        }
    }

    return false;
}
