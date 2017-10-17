//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// This file contains exported functions related to EdgeHTML allocating and managing objects whose memory
// lives in the chakra recycler, which are precisely traced and/or finalized according to chakra's
// mark and sweep GC.

// A thread-specific handle to the Javascript heap for use in allocating native host objects.
typedef void* RecyclerNativeHeapHandle;

// Allocates a new traced block of memory that is at least size bytes.
void* __stdcall RecyclerNativeHeapAllocTraced(_In_ RecyclerNativeHeapHandle handle, size_t size);

// Allocates a new traced block of memory that is at least size bytes; the allocated 
// object will have its finalizer run when the object becomes unreachable.
void* __stdcall RecyclerNativeHeapAllocTracedFinalized(_In_ RecyclerNativeHeapHandle handle, size_t size);

// Allocates a new leaf (untraced) block of memory that is at least size bytes; the
// allocated object will have its finalizer run when the object becomes unreachable.
void* __stdcall RecyclerNativeHeapAllocLeafFinalized(_In_ RecyclerNativeHeapHandle handle, size_t size);

// Allocates a new leaf (untraced) block of memory that is at least size bytes.
void* __stdcall RecyclerNativeHeapAllocLeaf(_In_ RecyclerNativeHeapHandle handle, size_t size);

// Increases the root ref count by 1 to prevent collection of object.
// If supplied, count is set to the new root ref count.
// Returns E_OUTOFMEMORY if the memory required to root the object cannot be allocated and S_OK otherwise.
// Interior pointers for object are OK but must be paired consistently with release.
HRESULT __stdcall RecyclerNativeHeapRootAddRef(_In_ RecyclerNativeHeapHandle handle, _In_ void* object, _Out_opt_ unsigned int* count);

// Decreases the root ref count by 1 to allow collection of object (once the count returns to 0).
// If supplied, count is set to the new root ref count.
// Interior pointers for object are OK but must be paired consistently with addref.
void __stdcall RecyclerNativeHeapRootRelease(_In_ RecyclerNativeHeapHandle handle, _In_ void* object, _Out_opt_ unsigned int* count);

// A handle to a weak reference which can be used to obtain strong references before the weakly referenced object is swept.
typedef void* RecyclerNativeHeapWeakReferenceHandle;

// Creates a weak reference and provides a handle to it.
// Returns E_OUTOFMEMORY if the memory required to create the reference cannot be allocated and S_OK otherwise.
// object may NOT be an interior pointer.
HRESULT __stdcall RecyclerNativeHeapCreateWeakReference(_In_ RecyclerNativeHeapHandle handle, _In_ void* object, _Out_ RecyclerNativeHeapWeakReferenceHandle* weakReferenceHandle);

// Returns a strong reference to the weakly referenced object or null if the object has already been swept.
void* __stdcall RecyclerNativeHeapGetStrongReference(_In_ RecyclerNativeHeapHandle handle, _In_ RecyclerNativeHeapWeakReferenceHandle weakReferenceHandle);

// A cookie used to test if any weak references have been cleared by the recycler since the last cookie was obtained.
typedef size_t RecyclerNativeHeapWeakReferenceCleanupCookie;

// An value which can be used with .RecyclerNativeHeapHaveWeakReferencesBeenCleared to obtain an initial cookie for use in subsequent calls.
extern const RecyclerNativeHeapWeakReferenceCleanupCookie RecyclerNativeHeapInitialWeakReferenceCleanupCookie;

// Returns true if weak references have been nulled by the recycler since the supplied cookie was obtained.
// Useful to optimize the implementation of data structures which lazily cleanup weak references.
// Use the constant RecyclerNativeHeapInitialWeakReferenceCleanupCookie to obtain an initial cookie value that can be used in subsequent calls.
bool __stdcall RecyclerNativeHeapHasWeakReferenceCleanupOccurred(_In_ RecyclerNativeHeapHandle handle, _Inout_ RecyclerNativeHeapWeakReferenceCleanupCookie* cookie);

// Given a pointer which may refer to the interior of an object, returns a pointer to the head of the object or null if the candidate doesn't refer to an allocated block of memory.
// Chakra assumes all weak references are 16-byte aligned, but due to multiple inheritance and other C++ features, EdgeHTML pointers don't often meet that requirement.
// Rather than adding overhead to lookup interior pointers during the sweeping of weak references, this API allows a smart pointer type external to Chakra to store an offset and 
// properly aligned pointer pair so that Chakra only ever sees the properly aligned pointer in its map of weak reference handles.
void* __stdcall RecyclerNativeHeapGetRealAddressFromInterior(_In_ RecyclerNativeHeapHandle handle, _In_ void* candidate);
