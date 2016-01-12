//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "RemoteHeapBlockMap.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------


RemoteHeapBlockMapWithCache::RemoteHeapBlockMapWithCache(ExtRemoteTyped heapBlockMap)
    : RemoteHeapBlockMap(heapBlockMap)
{    
    ForEachHeapBlockDirect([this](ULONG64 nodeIndex, ULONG64 l1, ULONG64 l2, ULONG64 block, ExtRemoteTyped heapBlock)
    {
        ULONG64 address = ((nodeIndex * l1ChunkSize + l1) * l2ChunkSize + l2) * g_Ext->m_PageSize;
        cachedHeapBlock[address] = heapBlock;
        return false;
    });    
}

RemoteHeapBlockMapWithCache::~RemoteHeapBlockMapWithCache()
{
    // when we are not executing a command, the ExtExtension isn't populated, so we can't release the ExtRemoteTyped.
    // We can change this to just save the mod/type/address information here instead of the ExtRemoteTyped as well.
    if (g_Ext->m_CurCommand == nullptr)
    {
        for (auto i = cachedHeapBlock.begin(); i != cachedHeapBlock.end(); i++)
        {
            (*i).second.m_Release = false;
        }
        heapBlockMap.m_Release = false;
    }
}

ExtRemoteTyped RemoteHeapBlockMapWithCache::FindHeapBlock(ULONG64 address)
{
    ULONG64 pageAddress = address & ~((ULONG64)g_Ext->m_PageSize - 1);
    auto i = cachedHeapBlock.find(pageAddress);
    if (i != cachedHeapBlock.end())
    {
        return (*i).second;
    }
    return ExtRemoteTyped("(void *)0");
}

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------