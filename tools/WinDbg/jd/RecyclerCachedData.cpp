//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "jdrecycler.h"
#include "recyclerroots.h"
#include "RemoteHeapBlockMap.h"

Addresses * ComputeRoots(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, ExtRemoteTyped* threadContext, bool dump);

static char const * StaticGetSmallHeapBlockTypeName()
{
    return GetExtension()->GetSmallHeapBlockTypeName();
}

RecyclerCachedData::RecyclerCachedData(EXT_CLASS_BASE * ext) :
    _ext(ext),
    m_heapBlockTypeInfo("HeapBlock", true),
    m_smallHeapBlockTypeInfo(StaticGetSmallHeapBlockTypeName),
    m_largeHeapBlockTypeInfo("LargeHeapBlock", true)
{}

Addresses * RecyclerCachedData::GetRootPointers(ExtRemoteTyped recycler, ExtRemoteTyped * threadContext)
{
    ExtRemoteTyped externalRootMarker = recycler.Field("externalRootMarker");

    if (externalRootMarker.GetPtr() != 0)
    {
        _ext->Out("WARNING: External root marker installed (Address: 0x%p), some roots might be missed\n", externalRootMarker.GetPtr());
    }

    auto i = rootPointersCache.find(recycler.GetPtr());
    if (i != rootPointersCache.end())
    {
        return (*i).second;
    }

    // TODO: external weak ref support may be missing once it is implemented
    Addresses * rootPointers = ComputeRoots(_ext, recycler, threadContext, false);
    rootPointersCache[recycler.GetPtr()] = rootPointers;
    return rootPointers;
}

RemoteHeapBlockMapWithCache * RecyclerCachedData::GetHeapBlockMap(ExtRemoteTyped recycler, bool create)
{
    auto i = m_heapblockMapCache.find(recycler.GetPtr());
    if (i != m_heapblockMapCache.end())
    {
        return (*i).second;
    }

    if (create)
    {
        RemoteHeapBlockMapWithCache  * remoteHeapBlockMap = new RemoteHeapBlockMapWithCache(recycler.Field("heapBlockMap"));
        m_heapblockMapCache[recycler.GetPtr()] = remoteHeapBlockMap;
        return remoteHeapBlockMap;
    }

    return nullptr;
}

void RecyclerCachedData::Clear()
{
    for (auto i = rootPointersCache.begin(); i != rootPointersCache.end(); i++)
    {
        delete (*i).second;
    }
    rootPointersCache.clear();

    for (auto i = m_heapblockMapCache.begin(); i != m_heapblockMapCache.end(); i++)
    {
        delete (*i).second;
    }
    m_heapblockMapCache.clear();
    m_heapBlockTypeInfo.Clear();
    m_smallHeapBlockTypeInfo.Clear();
    m_largeHeapBlockTypeInfo.Clear();
}

ExtRemoteTyped RecyclerCachedData::GetAsHeapBlock(ULONG64 address)
{
    return m_heapBlockTypeInfo.Cast(address);
}

ExtRemoteTyped RecyclerCachedData::GetAsSmallHeapBlock(ULONG64 address)
{
    return m_smallHeapBlockTypeInfo.Cast(address);
}

ExtRemoteTyped RecyclerCachedData::GetAsLargeHeapBlock(ULONG64 address)
{
    return m_largeHeapBlockTypeInfo.Cast(address);
}
