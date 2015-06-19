//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

enum MemProtectHeapCollectFlags
{
    MemProtectHeap_CollectWithHeuristic = 0x0,  // Only activate GC if it pass the heuristic
    MemProtectHeap_CollectForce = 0x1,
    MemProtectHeap_CollectForceConcurrent = 0x2,  // Start a concurrent GC if GC is not running already
    MemProtectHeap_CollectForcePartial = 0x4,  // Start a partial GC if GC is not running already
    MemProtectHeap_ForceFinishCollect = 0x8,  // Wait for and finish the GC if one is running

    MemProtectHeap_CollectOnUnrootWithHeuristic = 0x10,  // Collect with less aggressive heuristic (may be an inefficient point to collect, so do so less frequently)

    MemProtectHeap_CollectForceFull = MemProtectHeap_ForceFinishCollect | MemProtectHeap_CollectForce, // Finish previous GC, perform a new full GC.
    MemProtectHeap_CollectForceConcurrentPartial = MemProtectHeap_CollectForceConcurrent | MemProtectHeap_CollectForcePartial // Start a concurrent partial GC if GC is not running already

};

// Flags to set global options for the heap at creation time
enum MemProtectHeapCreateFlags
{
    // Only protect the stack of the current thread at collection time (no calls to MemProtectHeapProtectCurrentThread/MemProtectHeapUnprotectCurrentThread allowed/required)
    MemProtectHeapCreateFlags_ProtectCurrentStack = 0x0,

    // Protect the stacks of all threads registered with MemProtectHeapProtectCurrentThread using a cooperative callback
    MemProtectHeapCreateFlags_ProtectMultipleStacksCooperative = 0x1,

    // Protect the stacks of all threads registered with MemProtectHeapProtectCurrentThread using thread suspension (requires a concurrent collector thread)
    MemProtectHeapCreateFlags_ProtectMultipleStacksSuspend = 0x2,

    // Force all collection to be performed in thread (no background threads). Currently incompatibile with suspend to avoid two asynchronous suspend calls causing deadlock
    MemProtectHeapCreateFlags_ForceInThreadCollection = 0x4,

    MemProtectHeapCreateFlags_PageHeapModeBlockStart = 0x8,
    MemProtectHeapCreateFlags_PageHeapModeBlockEnd = 0x10,

    MemProtectHeapCreateFlags_PageHeapModeCaptureAllocStack = 0x20,
    MemProtectHeapCreateFlags_PageHeapModeCaptureFreeStack = 0x40,
};

// A section of additional (non-stack) roots explicitly registered with the collector
struct MemProtectHeapRootSection
{
    void**  start;
    size_t  size;
    MemProtectHeapRootSection* next;
};

// A cooperative thread suspension callback.
typedef void (__stdcall* MemProtectCooperativeThreadWakeProc)(void* argument);

HRESULT __stdcall MemProtectHeapCreate(__deref_out void ** heapHandle, __in MemProtectHeapCreateFlags flags);
__byte_writableTo(size) void * __stdcall MemProtectHeapRootAlloc(__in void * heapHandle, size_t size);
__byte_writableTo(size) void * __stdcall MemProtectHeapRootAllocLeaf(__in void * heapHandle, size_t size);
HRESULT __stdcall MemProtectHeapRootRealloc(__in void * heapHandle, size_t newSize, __deref_inout void ** memory);
HRESULT __stdcall MemProtectHeapRootReallocLeaf(__in void * heapHandle, size_t newSize, __deref_inout void ** memory);
HRESULT __stdcall MemProtectHeapUnrootAndZero(__in void * heapHandle, __in __post_invalid void * memory);
HRESULT __stdcall MemProtectHeapMemSize(__in void * heapHandle, __in void * memory, __out size_t * outSize);
void __stdcall MemProtectHeapReportHeapSize(__in void * heapHandle);
HRESULT __stdcall MemProtectHeapDestroy(__in __post_invalid void *heapHandle);
HRESULT __stdcall MemProtectHeapCollect(__in void * heapHandle, MemProtectHeapCollectFlags flags);
HRESULT __stdcall MemProtectHeapProtectCurrentThread(__in void * heapHandle, __in_opt MemProtectCooperativeThreadWakeProc wakeProc, __in_opt void* wakeArgument);
HRESULT __stdcall MemProtectHeapUnprotectCurrentThread(__in void * heapHandle);
void __stdcall MemProtectHeapNotifyCurrentThreadDetach(__in void* heapHandle);
HRESULT __stdcall MemProtectHeapRootFree(__in void * heapHandle, __in __post_invalid void * memory);
HRESULT __stdcall MemProtectHeapSynchronizeWithCollector(__in void * heapHandle);
void __stdcall MemProtectHeapDisableCollection(__in void * heapHandle);

// The following two functions are only available in static lib. In DLL version, these functions are replaced by DllMain for that binary.
void __stdcall MemProtectHeapProcessAttach();
void __stdcall MemProtectHeapProcessDetach();

void __stdcall MemProtectHeapAddRootSection(__in void* heapHandle, __in MemProtectHeapRootSection* rootSection);
HRESULT __stdcall MemProtectHeapRemoveRootSection(__in void* heapHandle, __in MemProtectHeapRootSection* rootSection);

// Determine if an allocation is still live.
// This function is used by IE unit tests, so must be unconditionally compiled. 
// It is not a DLL export though, so only available in the static lib version.
bool MemProtectHeapIsValidObject(__in void* heapHandle, __in void * memory);

#if DBG && defined(INTERNAL_MEM_PROTECT_HEAP_ALLOC)
void MemProtectHeapSetDisableConcurrentThreadExitedCheck(__in void * heapHandle);
#endif
#if DBG && defined(INTERNAL_MEM_PROTECT_HEAP_CMDLINE)
void MemProtectHeapParseCmdLineConfig(int argc, __in_ecount(argc) WCHAR* argv[]);
#endif
