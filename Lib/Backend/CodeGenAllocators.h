// Copyright (C) Microsoft. All rights reserved. 

#pragma once

#if PDATA_ENABLED
#define ALLOC_XDATA (true)
#else
#define ALLOC_XDATA (false)
#endif

struct CodeGenAllocators
{
    // emitBufferManager dependes on allocator which intern depends on pageAllocator, make sure the sequence is right
    PageAllocator pageAllocator;
    NoRecoverMemoryArenaAllocator    allocator;
    EmitBufferManager<CriticalSection> emitBufferManager;
#if !_M_X64_OR_ARM64 && _CONTROL_FLOW_GUARD
    bool canCreatePreReservedSegment;
#endif

#ifdef PERF_COUNTERS
    size_t staticNativeCodeData;
#endif

    CodeGenAllocators(AllocationPolicyManager * policyManager, Js::ScriptContext * scriptContext);
    PageAllocator *GetPageAllocator() { return &pageAllocator; };
    HeapPageAllocator<VirtualAllocWrapper> *GetHeapPageAllocator() { emitBufferManager.GetHeapPageAllocator(); };
    ~CodeGenAllocators();
};
