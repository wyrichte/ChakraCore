// Copyright (C) Microsoft. All rights reserved. 

#include "BackEnd.h"

CodeGenAllocators::CodeGenAllocators(AllocationPolicyManager * policyManager, Js::ScriptContext * scriptContext)
: pageAllocator(policyManager, Js::Configuration::Global.flags, PageAllocatorType_BGJIT)
, allocator(L"NativeCode", &pageAllocator, Js::Throw::OutOfMemory)
, emitBufferManager(policyManager, &allocator, scriptContext, L"JIT code buffer", ALLOC_XDATA)
#if !_M_X64_OR_ARM64 && _CONTROL_FLOW_GUARD
, canCreatePreReservedSegment(false)
#endif
#ifdef PERF_COUNTERS
, staticNativeCodeData(0)
#endif
{
}

CodeGenAllocators::~CodeGenAllocators()
{
    PERF_COUNTER_SUB(Code, StaticNativeCodeDataSize, staticNativeCodeData);
    PERF_COUNTER_SUB(Code, TotalNativeCodeDataSize, staticNativeCodeData);
}
