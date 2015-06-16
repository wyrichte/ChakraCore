#pragma once

namespace Memory
{
#define PreReservedHeapTrace(...) { \
    OUTPUT_TRACE(Js::PreReservedHeapAllocPhase, __VA_ARGS__); \
}

/*
* VirtualAllocWrapper is just a delegator class to call VirtualAlloc and VirtualFree.
*/
class VirtualAllocWrapper
{
public:
    LPVOID  Alloc(LPVOID lpAddress, size_t dwSize, DWORD allocationType, DWORD protectFlags, bool isCustomHeapAllocation = false);
    BOOL    Free(LPVOID lpAddress, size_t dwSize, DWORD dwFreeType);
    bool        IsPreReservedRegionPresent();
    bool        IsInRange(void * address);
    LPVOID      GetPreReservedStartAddress();
    LPVOID      GetPreReservedEndAddress();
};

/*
* PreReservedVirtualAllocWrapper class takes care of Reserving a large memory region initially
* and then commiting mem regions for the size requested.
* Committed pages are being tracked with a bitVector.
* Committing memory outside of the preReserved Memeory region is not handled by this allocator
*/

class PreReservedVirtualAllocWrapper
{
public:
#if _M_IX86_OR_ARM32
    static const size_t PreReservedAllocationSegmentCount = 256; // (256 * 64K) == 16 MB, if 64k is the AllocationGranularity
#else // _M_X64_OR_ARM64
    static const size_t PreReservedAllocationSegmentCount = 1024; //(1024 * 64K) == 64MB, if 64k is the AllocationGranularity
#endif
    
public:
    PreReservedVirtualAllocWrapper();
    BOOL Shutdown();
    LPVOID      Alloc(LPVOID lpAddress, size_t dwSize, DWORD allocationType, DWORD protectFlags, bool isCustomHeapAllocation = false);
    BOOL        Free(LPVOID lpAddress, size_t dwSize, DWORD dwFreeType);
    bool        IsPreReservedRegionPresent();
    bool        IsInRange(void * address);
    LPVOID      GetPreReservedStartAddress();
    LPVOID      GetPreReservedEndAddress();
private:
    BVStatic<PreReservedAllocationSegmentCount>     freeSegments;
    LPVOID                                          preReservedStartAddress;
    CriticalSection                                 cs;
};

template<typename TVirtualAlloc = VirtualAllocWrapper>
class PageAllocatorBase;

template<typename TVirtualAlloc = VirtualAllocWrapper>
class PageSegmentBase;

template<typename TVirtualAlloc = VirtualAllocWrapper>
class SegmentBase;

typedef PageAllocatorBase<> PageAllocator;
typedef PageSegmentBase<> PageSegment;
typedef SegmentBase<> Segment;
}