//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "RemoteHeapBlockMap.h"

RemoteHeapBlockMap::RemoteHeapBlockMap(JDRemoteTyped heapBlockMap)
{
    ULONG64 heapBlockMapAddr = heapBlockMap.GetPointerTo().GetPtr();
    cachedHeapBlock = GetExtension()->recyclerCachedData.GetHeapBlockMap(heapBlockMapAddr);
    if (cachedHeapBlock == nullptr)
    {
        auto start = _time64(nullptr);
        std::auto_ptr<Cache> localCachedHeapBlock(new Cache());

        ULONG64 iter = 0;
        ForEachHeapBlockRaw(heapBlockMap, [this, &localCachedHeapBlock, &iter](ULONG64 nodeIndex, ULONG64 l1, ULONG64 l2, ULONG64 block, RemoteHeapBlock& heapBlock)
        {
            ULONG64 heapBlockAddress = heapBlock.GetHeapBlockAddress();
            auto heapBlockFromMap = localCachedHeapBlock.get()->heapBlockAddressToHeapBlockMap.find(heapBlockAddress);
            RemoteHeapBlock * heapBlockEntry;
            if (heapBlockFromMap != localCachedHeapBlock.get()->heapBlockAddressToHeapBlockMap.end())
            {
                heapBlockEntry = &heapBlockFromMap->second;
            }
            else
            {
                heapBlockEntry = &(localCachedHeapBlock.get()->heapBlockAddressToHeapBlockMap[heapBlockAddress] = heapBlock);
            }
            ULONG64 address = ((nodeIndex * l1ChunkSize + l1) * l2ChunkSize + l2) * g_Ext->m_PageSize;
            localCachedHeapBlock.get()->addressToHeapBlockMap[address] = heapBlockEntry;

            iter++;
            
            if (GetExtension()->ShowProgress() && (iter % 0x10000 == 0))
            {
                g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\rReading heap block map: %ld", iter);
            }
            return false;
        });
        GetExtension()->recyclerCachedData.SetHeapBlockMap(heapBlockMapAddr, localCachedHeapBlock.get());
        cachedHeapBlock = localCachedHeapBlock.release();

        if (GetExtension()->ShowProgress())
        {
            g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "\r");
        }
        g_Ext->m_Control->ControlledOutput(DEBUG_OUTCTL_NOT_LOGGED, DEBUG_OUTPUT_NORMAL, "Heap block map reading completed - elapsed time: %us\n", (ULONG)(_time64(nullptr) - start));
    }
}

RemoteHeapBlock * RemoteHeapBlockMap::FindHeapBlock(ULONG64 address)
{
    Assert(cachedHeapBlock);

    ULONG64 pageAddress = address & ~((ULONG64)g_Ext->m_PageSize - 1);
    auto i = cachedHeapBlock->addressToHeapBlockMap.find(pageAddress);
    if (i != cachedHeapBlock->addressToHeapBlockMap.end())
    {
        return (*i).second;
    }
    return nullptr;
}
