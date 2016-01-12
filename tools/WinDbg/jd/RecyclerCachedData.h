//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <map>
#include "CachedTypeInfo.h"

class EXT_CLASS_BASE;
class Addresses;
class RootPointers;
class RemoteHeapBlockMapWithCache;

class RecyclerCachedData
{
public:
    RecyclerCachedData(EXT_CLASS_BASE * ext);
    
    Addresses * GetRootPointers(ExtRemoteTyped recycler, ExtRemoteTyped * threadContext);

    RemoteHeapBlockMapWithCache * GetHeapBlockMap(ExtRemoteTyped recycler, bool create = true);

    void Clear();

    ExtRemoteTyped GetAsHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsLargeHeapBlock(ULONG64 address);
    ExtRemoteTyped GetAsSmallHeapBlock(ULONG64 address);
private:
    std::map<ULONG64, Addresses *> rootPointersCache;
    std::map<ULONG64, RemoteHeapBlockMapWithCache *> m_heapblockMapCache;
    
    CachedTypeInfo m_heapBlockTypeInfo;
    CachedTypeInfo m_smallHeapBlockTypeInfo;
    CachedTypeInfo m_largeHeapBlockTypeInfo;
    EXT_CLASS_BASE * _ext;
};