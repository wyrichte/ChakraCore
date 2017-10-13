//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// This file contains exported functions related to edgehtml allocating and managing objects whose memory
// lives in the chakra recycler, which are precisely traced and/or finalized according to chakra's
// mark and sweep GC.

typedef void* RecyclerNativeHeapHandle;
void* __stdcall RecyclerNativeHeapAllocTraced(RecyclerNativeHeapHandle recyclerHandle, size_t size);
void* __stdcall RecyclerNativeHeapAllocTracedFinalized(RecyclerNativeHeapHandle recyclerHandle, size_t size);
void* __stdcall RecyclerNativeHeapAllocFinalized(RecyclerNativeHeapHandle recyclerHandle, size_t size);
void* __stdcall RecyclerNativeHeapAllocLeaf(RecyclerNativeHeapHandle recyclerHandle, size_t size);

HRESULT __stdcall RecyclerNativeHeapRootAddRef(RecyclerNativeHeapHandle, void* object, unsigned int* count);
HRESULT __stdcall RecyclerNativeHeapRootRelease(RecyclerNativeHeapHandle, void* object, unsigned int* count);
