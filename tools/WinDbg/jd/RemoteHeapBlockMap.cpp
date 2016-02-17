//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "RemoteHeapBlockMap.h"

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

RemoteHeapBlockMap::RemoteHeapBlockMap(ExtRemoteTyped heapBlockMap, bool cache)
{
    cachedHeapBlock = GetExtension()->recyclerCachedData.GetHeapBlockMap(heapBlockMap);
    if (cachedHeapBlock == nullptr && cache)
    {
        auto start = _time64(nullptr);
        std::auto_ptr<Cache> localCachedHeapBlock(new Cache());

        ULONG64 iter = 0;
        ForEachHeapBlockRaw(heapBlockMap, [this, &localCachedHeapBlock, &iter](ULONG64 nodeIndex, ULONG64 l1, ULONG64 l2, ULONG64 block, RemoteHeapBlock& heapBlock)
        {
            ULONG64 address = ((nodeIndex * l1ChunkSize + l1) * l2ChunkSize + l2) * g_Ext->m_PageSize;
            (*localCachedHeapBlock.get())[address] = heapBlock;

            iter++;
            if (iter % 0x10000 == 0)
            {
                g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rReading heap block map: %ld", iter);
            }
            return false;
        });
        GetExtension()->recyclerCachedData.SetHeapBlockMap(heapBlockMap, localCachedHeapBlock.get());
        cachedHeapBlock = localCachedHeapBlock.release();

        g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rHeap block map reading completed - elapsed time: %us\n", (ULONG)(_time64(nullptr) - start));
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
