//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "jdrecycler.h"
#include "recyclerroots.h"

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
    RootPointers * rootPointers = ComputeRoots(_ext, recycler, threadContext, false);
    rootPointersCache[recycler.GetPtr()] = rootPointers;
    return rootPointers;
}

void
RecyclerCachedData::Clear()
{
    for (auto i = rootPointersCache.begin(); i != rootPointersCache.end(); i++)
    {
        delete (*i).second;        
    }
    rootPointersCache.clear();
}