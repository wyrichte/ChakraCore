//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

void
ForcedMemoryConstraint::Apply()
{
    if (Js::Configuration::Global.flags.IsEnabled(Js::ForceFragmentAddressSpaceFlag))
    {
        FragmentAddressSpace(Js::Configuration::Global.flags.ForceFragmentAddressSpace);
    }
}

#pragma prefast(suppress:6262, "Where this function is call should have ample of stack space")
void ForcedMemoryConstraint::FragmentAddressSpace(size_t usableSize)
{
    // AMD64 address space is too big
#if !defined(_M_X64_OR_ARM64)
    uint const allocationGranuality = 64 * 1024;     // 64 KB
    Assert(allocationGranuality == AutoSystemInfo::Data.dwAllocationGranularity);    
    uint64 const addressEnd = ((uint64)4) * 1024 * 1024 * 1024;

    uint const freeSpaceSize = Math::Align<size_t>(usableSize, allocationGranuality);
    void * address[addressEnd / allocationGranuality];
    
    // Reserve a contiguous usable space
    void * freeAddress = ::VirtualAlloc(NULL, freeSpaceSize, MEM_RESERVE, PAGE_NOACCESS);

    // Reserve the reset the address space
    for (uint i = 1; i < _countof(address); i++)
    {
        address[i] = ::VirtualAlloc((LPVOID)(i * allocationGranuality), allocationGranuality, MEM_RESERVE, PAGE_NOACCESS);
    }

    // fragment
    int j = _countof(address) - 2;
    do
    {
        if (address[j + 1] == null)
        {
             j--;
             continue;
        }

        ::VirtualFree(address[j + 1], 0, MEM_RELEASE);
        j -= 2;
    }
    while (j > 0);

    ::VirtualFree(freeAddress, 0, MEM_RELEASE);
#endif
}

#endif