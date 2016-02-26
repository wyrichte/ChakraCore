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
    m_largeHeapBlockTypeInfo("LargeHeapBlock", true),
    m_blockTypeEnumInitialized(false),
    m_mphblockTypeEnumInitialized(false)
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

RemoteHeapBlockMap::Cache * RecyclerCachedData::GetHeapBlockMap(ULONG64 heapBlockMapAddr)
{
    auto i = m_heapblockMapCache.find(heapBlockMapAddr);
    if (i != m_heapblockMapCache.end())
    {
        return (*i).second;
    }
    return nullptr;
}

void RecyclerCachedData::SetHeapBlockMap(ULONG64 heapBlockMapAddr, RemoteHeapBlockMap::Cache * cache)
{
    Assert(m_heapblockMapCache.find(heapBlockMapAddr) == m_heapblockMapCache.end());
    m_heapblockMapCache[heapBlockMapAddr] = cache;
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

void RecyclerCachedData::EnsureBlockTypeEnum()
{
    if (m_blockTypeEnumInitialized)
    {
        return;
    }
    bool useMemoryNamespace = (GetExtension()->GetEnumValue("SmallNormalBlockType", false) == (ULONG64)-1);
    
#define INIT_BLOCKTYPE_ENUM(name) \
    m_blockTypeEnumValue##name = GetExtension()->GetEnumValue(#name, useMemoryNamespace); \
    if (m_blockTypeEnumValue##name == (ULONG64)-1) \
    { \
        g_Ext->Out("Enum value for block type " #name " doesn't exist\n"); \
    }

    BLOCKTYPELIST(INIT_BLOCKTYPE_ENUM)
#undef INIT_BLOCKTYPE_ENUM

    m_blockTypeEnumInitialized = true;
}

void RecyclerCachedData::EnsureMPHBlockTypeEnum()
{
    if (m_mphblockTypeEnumInitialized)
    {
        return;
    }

    bool useMemoryNamespace = (GetExtension()->GetEnumValue("SmallNormalBlockType", false) == (ULONG64)-1);

#define INIT_BLOCKTYPE_ENUM(name) \
    m_blockTypeEnumValue##name = GetExtension()->GetEnumValue(#name, useMemoryNamespace); \
    if (m_mphblockTypeEnumValue##name == (ULONG64)-1) \
    { \
        g_Ext->Out("Enum value for block type " #name " doesn't exist\n"); \
    }

    BLOCKTYPELIST(INIT_BLOCKTYPE_ENUM)
#undef INIT_BLOCKTYPE_ENUM

    m_mphblockTypeEnumInitialized = true;
}