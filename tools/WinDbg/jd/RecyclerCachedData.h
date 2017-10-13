//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <map>
#include <set>
#include "CachedTypeInfo.h"
#include "RemoteHeapBlockMap.h"

class EXT_CLASS_BASE;
class Addresses;
class RootPointers;
class RecyclerObjectGraph;
class RemoteRecycler;
class RemoteThreadContext;

#define BLOCKTYPELIST(MACRO) \
    MACRO(SmallNormalBlockType) \
    MACRO(SmallLeafBlockType) \
    MACRO(SmallFinalizableBlockType) \
    MACRO(SmallNormalBlockWithBarrierType) \
    MACRO(SmallFinalizableBlockWithBarrierType) \
    MACRO(SmallRecyclerVisitedHostBlockType) \
    MACRO(MediumNormalBlockType) \
    MACRO(MediumLeafBlockType) \
    MACRO(MediumFinalizableBlockType) \
    MACRO(MediumNormalBlockWithBarrierType) \
    MACRO(MediumFinalizableBlockWithBarrierType) \
    MACRO(MediumRecyclerVisitedHostBlockType) \
    MACRO(LargeBlockType) \

//    MACRO(SmallBlockTypeCount) \
//    MACRO(BlockTypeCount)

#define BLOCKTYPELIST_JSCRIPT9(MACRO) \
    MACRO(SmallNormalBlock) \
    MACRO(SmallLeafBlock) \
    MACRO(SmallFinalizableBlock) \
    MACRO(LargeBlock) \

class RecyclerCachedData
{
public:
    RecyclerCachedData();
    
    Addresses * GetRootPointers(RemoteRecycler recycler, RemoteThreadContext * threadContext, ULONG64 stackTop);

    RemoteHeapBlockMap::Cache * GetHeapBlockMap(ULONG64 heapBlockMapAddr);
    void SetHeapBlockMap(ULONG64 heapBlockMapAddr, RemoteHeapBlockMap::Cache * cache);

    void Clear();

    ExtRemoteTyped GetAsHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsLargeHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsSmallHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsSmallFinalizableHeapBlock(ULONG64 address);

    RemoteHeapBlock * FindCachedHeapBlock(ULONG64 address);

    RecyclerObjectGraph * GetCachedRecyclerObjectGraph(ULONG64 recyclerAddress);
    void CacheRecyclerObjectGraph(ULONG64 recyclerAddress, RecyclerObjectGraph * graph);

    bool GetCachedDebuggeeMemory(ULONG64 address, ULONG size, char ** debuggeeMemory);
    void RemoveCachedDebuggeeMemory(char ** debuggeeMemory);
    void EnableCachedDebuggeeMemory();
    void DisableCachedDebuggeeMemory();
    bool IsCachedDebuggeeMemoryEnabled() { return m_debuggeeMemoryCache != nullptr; }

#define DEFINE_BLOCKTYPE_ENUM_ACCESSOR(name) \
    ULONG64 GetBlockTypeEnum##name() { EnsureBlockTypeEnum(); return m_blockTypeEnumValue##name; } \
    ULONG64 GetMPHBlockTypeEnum##name() { EnsureMPHBlockTypeEnum(); return m_mphblockTypeEnumValue##name; }

    BLOCKTYPELIST(DEFINE_BLOCKTYPE_ENUM_ACCESSOR)
#undef DEFINE_BLOCKTYPE_ENUM_ACCESSOR
private:
    void EnsureBlockTypeEnum();
    void EnsureMPHBlockTypeEnum();

#define DEFINE_BLOCKTYPE_ENUMVALUE(name) \
    ULONG64 m_blockTypeEnumValue##name; \
    ULONG64 m_mphblockTypeEnumValue##name##;

    BLOCKTYPELIST(DEFINE_BLOCKTYPE_ENUMVALUE)
#undef DEFINE_BLOCKTYPE_ENUMVALUE

    std::map<ULONG64, Addresses *> rootPointersCache;
    std::map<ULONG64, RemoteHeapBlockMap::Cache *> m_heapblockMapCache;
    
    RecyclerObjectGraph * cachedObjectGraph;
    ULONG64 cachedObjectGraphRecyclerAddress;

    CachedTypeInfo m_heapBlockTypeInfo;
    CachedTypeInfo m_smallHeapBlockTypeInfo;
    CachedTypeInfo m_smallFinalizableHeapBlockTypeInfo;
    CachedTypeInfo m_largeHeapBlockTypeInfo;
    bool m_blockTypeEnumInitialized;
    bool m_mphblockTypeEnumInitialized;

    HANDLE m_debuggeeMemoryCache;
    std::set<char **> m_debuggeeMemoryCacheReferences;
};
