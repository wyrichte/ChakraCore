//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <map>
#include "CachedTypeInfo.h"
#include "RemoteHeapBlockMap.h"

class EXT_CLASS_BASE;
class Addresses;
class RootPointers;

#define BLOCKTYPELIST(MACRO) \
    MACRO(SmallNormalBlockType) \
    MACRO(SmallLeafBlockType) \
    MACRO(SmallFinalizableBlockType) \
    MACRO(SmallNormalBlockWithBarrierType) \
    MACRO(SmallFinalizableBlockWithBarrierType) \
    MACRO(MediumNormalBlockType) \
    MACRO(MediumLeafBlockType) \
    MACRO(MediumFinalizableBlockType) \
    MACRO(MediumNormalBlockWithBarrierType) \
    MACRO(MediumFinalizableBlockWithBarrierType) \
    MACRO(LargeBlockType) \
    MACRO(SmallBlockTypeCount) \
    MACRO(BlockTypeCount)

class RecyclerCachedData
{
public:
    RecyclerCachedData(EXT_CLASS_BASE * ext);
    
    Addresses * GetRootPointers(ExtRemoteTyped recycler, ExtRemoteTyped * threadContext);

    RemoteHeapBlockMap::Cache * GetHeapBlockMap(ULONG64 heapBlockMapAddr);
    void SetHeapBlockMap(ULONG64 heapBlockMapAddr, RemoteHeapBlockMap::Cache * cache);

    void Clear();

    ExtRemoteTyped GetAsHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsLargeHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsSmallHeapBlock(ULONG64 address);

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
    
    CachedTypeInfo m_heapBlockTypeInfo;
    CachedTypeInfo m_smallHeapBlockTypeInfo;
    CachedTypeInfo m_largeHeapBlockTypeInfo;
    EXT_CLASS_BASE * _ext;
    bool m_blockTypeEnumInitialized;
    bool m_mphblockTypeEnumInitialized;
};
