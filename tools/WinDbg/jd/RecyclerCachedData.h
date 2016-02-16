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

class RecyclerCachedData
{
public:
    RecyclerCachedData(EXT_CLASS_BASE * ext);
    
    Addresses * GetRootPointers(ExtRemoteTyped recycler, ExtRemoteTyped * threadContext);

    RemoteHeapBlockMap::Cache * GetHeapBlockMap(ExtRemoteTyped recycler);
    void SetHeapBlockMap(ExtRemoteTyped heapBlockMap, RemoteHeapBlockMap::Cache * cache);

    void Clear();

    ExtRemoteTyped GetAsHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsLargeHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsSmallHeapBlock(ULONG64 address);
private:
    std::map<ULONG64, Addresses *> rootPointersCache;
    std::map<ULONG64, RemoteHeapBlockMap::Cache *> m_heapblockMapCache;
    
    CachedTypeInfo m_heapBlockTypeInfo;
    CachedTypeInfo m_smallHeapBlockTypeInfo;
    CachedTypeInfo m_largeHeapBlockTypeInfo;
    EXT_CLASS_BASE * _ext;
};