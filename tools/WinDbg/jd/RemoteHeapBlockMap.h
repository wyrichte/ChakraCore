//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

class RemoteHeapBlockMap
{
public:
    RemoteHeapBlockMap(ExtRemoteTyped heapBlockMap) : heapBlockMap(heapBlockMap) {};

    template <typename Fn>
    bool ForEachHeapBlock(Fn fn);

    template <typename Fn>
    bool ForEachHeapBlockDirect(Fn processFunction);

private:
    template <typename Fn>
    bool ProcessL1Chunk(ULONG64 nodeIndex, ExtRemoteTyped l1ChunkArray, Fn processFunction);

    template <typename Fn>
    bool ProcessL2Chunk(ULONG64 nodeIndex, ULONG64 l1Id, ExtRemoteTyped l2ChunkArray, Fn processFunction);

protected:
    ExtRemoteTyped heapBlockMap;

    ULONG64 l1ChunkSize;
    ULONG64 l2ChunkSize;
};


class RemoteHeapBlockMapWithCache : public RemoteHeapBlockMap
{
public:
    RemoteHeapBlockMapWithCache(ExtRemoteTyped heapBlockMap);
    ~RemoteHeapBlockMapWithCache();

    ExtRemoteTyped FindHeapBlock(ULONG64 address);

private:
    bool hasCache;
    stdext::hash_map<ULONG64, ExtRemoteTyped> cachedHeapBlock;
};

template <typename Fn>
bool RemoteHeapBlockMap::ForEachHeapBlock(Fn fn)
{
    // Heap block can cross multiple entries, filter them
    ExtRemoteTyped lastHeapBlock;
    auto processFunction = [&](ULONG64 nodeIndex, ULONG64 l1, ULONG64 l2, ULONG64 block, ExtRemoteTyped heapBlock)
    {
        if (lastHeapBlock.GetPtr() == heapBlock.GetPtr())
        {
            return false;
        }
        lastHeapBlock = heapBlock;
        return fn(heapBlock);
    };
    return ForEachHeapBlockDirect(fn);
}

template <typename Fn>
bool RemoteHeapBlockMap::ForEachHeapBlockDirect(Fn processFunction)
{

    if (g_Ext->m_PtrSize == 4)
    {
        ExtRemoteTyped map = heapBlockMap.Field("map");        
        return ProcessL1Chunk(0, map, processFunction);
    }
    else
    {
        ExtRemoteTyped current = heapBlockMap.Field("list");
        while (current.GetPtr() != 0)
        {
            ULONG64 nodeIndex = current.Field("nodeIndex").GetUlong();
            ExtRemoteTyped map = current.Field("map");
            ExtRemoteTyped l1ChunkArray = map.Field("map");
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
bool RemoteHeapBlockMap::ProcessL1Chunk(ULONG64 nodeIndex, ExtRemoteTyped chunk, Fn fn)
{
    const size_t l1Count = chunk.GetTypeSize() / chunk.ArrayElement(0).GetTypeSize();
    l1ChunkSize = l1Count;
    for (size_t l1Id = 0; l1Id < l1Count; l1Id++)
    {
        ExtRemoteTyped l2MapChunk = chunk.ArrayElement(l1Id);
        if (GetAsPointer(l2MapChunk) != 0)
        {
            if (ProcessL2Chunk(nodeIndex, l1Id, l2MapChunk.Dereference(), fn))
            {
                return true;
            }
        }
    }
    return false;
}

template <typename Fn>
bool
RemoteHeapBlockMap::ProcessL2Chunk(ULONG64 nodeIndex, ULONG64 l1Id, ExtRemoteTyped chunk, Fn fn)
{
    ExtRemoteTyped l2map = chunk.Field("map");
    const size_t l2Count = l2map.GetTypeSize() / l2map.ArrayElement(0).GetTypeSize();
    l2ChunkSize = l2Count;
    for (size_t l2Id = 0; l2Id < l2Count; l2Id++)
    {
        ExtRemoteTyped heapBlock = l2map.ArrayElement(l2Id);
        ULONG64 block = GetAsPointer(heapBlock);
        if (block != 0)
        {
            heapBlock = GetExtension()->CastWithVtable(heapBlock);

            if (fn(nodeIndex, l1Id, l2Id, block, heapBlock))
            {
                return true;
            }
        }
    }

    return false;
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------