//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "Common.h"

#include "RecyclerHeap.h"
#include "Recycler.h"
#include "Recycler.inl"
#include "rterror.h"

#ifdef RECYCLER_VISITED_HOST
const RecyclerNativeHeapWeakReferenceCleanupCookie RecyclerNativeHeapInitialWeakReferenceCleanupCookie = 0;

void* RecyclerNativeHeapAllocTraced(_In_ RecyclerNativeHeapHandle handle, size_t size)
{
    Recycler* recycler = static_cast<Recycler*>(handle);
    return RecyclerAllocVisitedHostTracedZero(recycler, size);
}

void* RecyclerNativeHeapAllocTracedFinalized(_In_ RecyclerNativeHeapHandle handle, size_t size)
{
    Recycler* recycler = static_cast<Recycler*>(handle);
    return RecyclerAllocVisitedHostTracedAndFinalizedZero(recycler, size);
}

void* RecyclerNativeHeapAllocLeafFinalized(_In_ RecyclerNativeHeapHandle handle, size_t size)
{
    Recycler* recycler = static_cast<Recycler*>(handle);
    return RecyclerAllocVisitedHostFinalizedZero(recycler, size);
}

void* RecyclerNativeHeapAllocLeaf(_In_ RecyclerNativeHeapHandle handle, size_t size)
{
    Recycler* recycler = static_cast<Recycler*>(handle);
    return RecyclerAllocLeafZero(recycler, size);
}

HRESULT RecyclerNativeHeapRootAddRef(_In_ RecyclerNativeHeapHandle handle, _In_ void* object, _Out_opt_ unsigned int* count)
{
    // Addref may require memory for adding an entry to a hash table - don't throw
    HRESULT hr = S_OK;
    
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED;
    {
        Recycler* recycler = static_cast<Recycler*>(handle);
        void* realAddress = recycler->GetRealAddressFromInterior(object);
        AssertOrFailFast(realAddress != nullptr);
        recycler->RootAddRef(realAddress, count);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    return hr;
}

void RecyclerNativeHeapRootRelease(_In_ RecyclerNativeHeapHandle handle, _In_ void* object, _Out_opt_ unsigned int* count)
{
    Recycler* recycler = static_cast<Recycler*>(handle);
    void* realAddress = recycler->GetRealAddressFromInterior(object);
    AssertOrFailFast(realAddress != nullptr);
    recycler->RootRelease(realAddress, count);
}

HRESULT RecyclerNativeHeapCreateWeakReference(_In_ RecyclerNativeHeapHandle handle, _In_ void* object, _Out_ RecyclerNativeHeapWeakReferenceHandle* weakReferenceHandle)
{
    Recycler* recycler = (Recycler*)handle;
    *weakReferenceHandle = nullptr;

    // Creating a weak reference requires memory and may OOM - don't throw
    HRESULT hr = S_OK;

    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED;
    {
        RecyclerWeakReference<void>* weakReference;
        recycler->FindOrCreateWeakReferenceHandle(object, &weakReference);
        *weakReferenceHandle = weakReference;
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    return hr;
}

void* RecyclerNativeHeapGetStrongReference(_In_ RecyclerNativeHeapHandle handle, _In_ RecyclerNativeHeapWeakReferenceHandle weakReferenceHandle)
{
    return ((RecyclerWeakReference<void>*)weakReferenceHandle)->Get();
}

bool RecyclerNativeHeapHasWeakReferenceCleanupOccurred(_In_ RecyclerNativeHeapHandle handle, _Inout_ RecyclerNativeHeapWeakReferenceCleanupCookie* cookie)
{
    Recycler* recycler = (Recycler*)handle;
    RecyclerNativeHeapWeakReferenceCleanupCookie previousCookie = *cookie;
    *cookie = recycler->GetWeakReferenceCleanupId();
    return *cookie > previousCookie;
}

void* RecyclerNativeHeapGetRealAddressFromInterior(_In_ RecyclerNativeHeapHandle handle, _In_ void* candidate)
{
    Recycler* recycler = static_cast<Recycler*>(handle);
    return recycler->GetRealAddressFromInterior(candidate);
}

#endif
