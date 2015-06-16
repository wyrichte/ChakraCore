//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <map>

class EXT_CLASS_BASE;
class Addresses;
class RootPointers;

class RecyclerCachedData
{
public:
    RecyclerCachedData(EXT_CLASS_BASE * ext) : _ext(ext) {};
    
    Addresses * GetRootPointers(ExtRemoteTyped recycler, ExtRemoteTyped * threadContext);

    void Clear();
private:
    std::map<ULONG64, RootPointers *> rootPointersCache;
    EXT_CLASS_BASE * _ext;
};