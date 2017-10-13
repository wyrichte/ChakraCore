//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "Common.h"

#include "RecyclerHeap.h"
#include "Recycler.h"
#include "rterror.h"

#ifdef RECYCLER_VISITED_HOST
void* RecyclerNativeHeapAllocTraced(RecyclerNativeHeapHandle recyclerHandle, size_t size)
{
    Recycler* recycler = static_cast<Recycler*>(recyclerHandle);
    return RecyclerAllocVisitedHostTracedZero(recycler, size);
}
void* RecyclerNativeHeapAllocTracedFinalized(RecyclerNativeHeapHandle recyclerHandle, size_t size)
{
    Recycler* recycler = static_cast<Recycler*>(recyclerHandle);
    return RecyclerAllocVisitedHostTracedAndFinalizedZero(recycler, size);
}

void* RecyclerNativeHeapAllocFinalized(RecyclerNativeHeapHandle recyclerHandle, size_t size)
{
    Recycler* recycler = static_cast<Recycler*>(recyclerHandle);
    return RecyclerAllocVisitedHostFinalizedZero(recycler, size);
}

void* RecyclerNativeHeapAllocLeaf(RecyclerNativeHeapHandle recyclerHandle, size_t size)
{
    Recycler* recycler = static_cast<Recycler*>(recyclerHandle);
    return RecyclerAllocLeafZero(recycler, size);
}

HRESULT RecyclerNativeHeapRootAddRef(RecyclerNativeHeapHandle recyclerHandle, void* object, unsigned int* count)
{
    // RootAddRef will throw on OOM (it can do a hash table allocation). If we see OOM, return it to
    // the caller; any other exception will be treated as failfast.
    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        Recycler* recycler = static_cast<Recycler*>(recyclerHandle);
        recycler->RootAddRef(object, count);
    }
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr)
    CATCH_UNHANDLED_EXCEPTION(hr)

    return hr;
}
HRESULT RecyclerNativeHeapRootRelease(RecyclerNativeHeapHandle recyclerHandle, void* object, unsigned int* count)
{
    Recycler* recycler = static_cast<Recycler*>(recyclerHandle);
    recycler->RootRelease(object, count);
    return S_OK;
}
#endif
