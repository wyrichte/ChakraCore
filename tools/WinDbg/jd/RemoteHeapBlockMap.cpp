//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "RemoteHeapBlockMap.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------


RemoteHeapBlockMap::RemoteHeapBlockMap(ExtRemoteTyped heapBlockMap, bool cache)
    : heapBlockMap(heapBlockMap)
{    

    cachedHeapBlock = GetExtension()->recyclerCachedData.GetHeapBlockMap(heapBlockMap);
    if (cachedHeapBlock == nullptr && cache)
    {
        std::auto_ptr<Cache> localCachedHeapBlock(new Cache());
        ForEachHeapBlockRaw([this, &localCachedHeapBlock](ULONG64 nodeIndex, ULONG64 l1, ULONG64 l2, ULONG64 block, RemoteHeapBlock& heapBlock)
        {
            ULONG64 address = ((nodeIndex * l1ChunkSize + l1) * l2ChunkSize + l2) * g_Ext->m_PageSize;
            (*localCachedHeapBlock.get())[address] = heapBlock;
            return false;
        });
        GetExtension()->recyclerCachedData.SetHeapBlockMap(heapBlockMap, localCachedHeapBlock.get());
        cachedHeapBlock = localCachedHeapBlock.release();
    }    
}

RemoteHeapBlock * RemoteHeapBlockMap::FindHeapBlock(ULONG64 address)
{
    Assert(cachedHeapBlock);
    
    ULONG64 pageAddress = address & ~((ULONG64)g_Ext->m_PageSize - 1);
    auto i = cachedHeapBlock->find(pageAddress);
    if (i != cachedHeapBlock->end())
    {
        return &(*i).second;
    }
    return nullptr;
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------