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
    m_mphblockTypeEnumInitialized(false),
    m_debuggeeMemoryCache(NULL)
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
    DisableCachedDebuggeeMemory();
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

bool RecyclerCachedData::GetCachedDebuggeeMemory(ULONG64 address, ULONG size, char ** debuggeeMemory)
{
    if (m_debuggeeMemoryCache == NULL)
    {
        return false;
    }

    ExtRemoteData data(address, size);
    class AutoCleanup
    {
    public:
        AutoCleanup(HANDLE heap, ULONG size) : heap(heap)
        {
            address = HeapAlloc(heap, 0, size);
        };
        ~AutoCleanup()
        {
            if (address)
            {
                HeapFree(heap, 0, address);
            }
        }
        char * Detach()
        {
            char * buffer = GetBuffer();
            address = nullptr;
            return buffer;
        }
        char * GetBuffer() { return (char *)address; }
    private:
        HANDLE heap;
        LPVOID address;
    } newBuffer(m_debuggeeMemoryCache, size);
    data.ReadBuffer(newBuffer.GetBuffer(), size);
    m_debuggeeMemoryCacheReferences.insert(debuggeeMemory);
    *debuggeeMemory = newBuffer.Detach();
    return true;
}

void RecyclerCachedData::RemoveCachedDebuggeeMemory(char ** debuggeeMemory)
{
    if (*debuggeeMemory)
    {
        HeapFree(m_debuggeeMemoryCache, 0, *debuggeeMemory);
        *debuggeeMemory = nullptr;
    }
}

void RecyclerCachedData::EnableCachedDebuggeeMemory()
{
    if (m_debuggeeMemoryCache != NULL)
    {
        return;
    }
    m_debuggeeMemoryCache = HeapCreate(0, 0, 0);
}

void RecyclerCachedData::DisableCachedDebuggeeMemory()
{
    if (m_debuggeeMemoryCache == NULL)
    {
        return;
    }

    for (auto i = m_debuggeeMemoryCacheReferences.begin(); i != m_debuggeeMemoryCacheReferences.end(); i++)
    {
        *(*i) = nullptr;
    }
    m_debuggeeMemoryCacheReferences.clear();
    HeapDestroy(m_debuggeeMemoryCache);
    m_debuggeeMemoryCache = NULL;
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

    if (GetExtension()->IsJScript9())
    {
#define INIT_BLOCKTYPE_ENUM(name) \
        m_blockTypeEnumValue##name##Type = GetExtension()->GetEnumValue(#name, false); \
        if (m_blockTypeEnumValue##name##Type == (ULONG64)-1) \
        { \
            g_Ext->Out("Enum value for block type " #name " doesn't exist\n"); \
        }

        BLOCKTYPELIST_JSCRIPT9(INIT_BLOCKTYPE_ENUM)
#undef INIT_BLOCKTYPE_ENUM
    }
    else
    {
        bool useMemoryNamespace = (GetExtension()->GetEnumValue("SmallNormalBlockType", false) == (ULONG64)-1);
#define INIT_BLOCKTYPE_ENUM(name) \
        m_blockTypeEnumValue##name = GetExtension()->GetEnumValue(#name, useMemoryNamespace); \
        if (m_blockTypeEnumValue##name == (ULONG64)-1) \
        { \
            g_Ext->Out("Enum value for block type " #name " doesn't exist\n"); \
        }

        BLOCKTYPELIST(INIT_BLOCKTYPE_ENUM)
#undef INIT_BLOCKTYPE_ENUM
    }
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
