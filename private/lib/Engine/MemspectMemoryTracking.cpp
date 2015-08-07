//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

#if defined(MEMSPECT_TRACKING)

// Dynamically detect whether Memspect's DLL is loaded and, if so, GetProcAddr on the reporting routines.
class MemspectMemoryTracker 
{
public:
    typedef struct {} const * HArena;
    typedef struct {} const * HMark;
    typedef HArena (__stdcall *TArenaCreate)(HANDLE hHeap, __in LPCWSTR lpwName, DWORD dwUserData);
    typedef BOOL (__stdcall *TArenaAllocation)(HArena hArena, size_t sSize, void *pAddress, size_t sUserDefinedType);
    typedef BOOL (__stdcall *TArenaFree)(HArena hArena, void *pAddress);
    // Mark is available but currently unused
    //typedef HMark (__stdcall *TArenaMark)(HArena hArena);
    typedef BOOL (__stdcall *TArenaRelease)(HArena hArena, HMark hMark);
    typedef BOOL (__stdcall *TArenaDestroy)(HArena);

private:
    static bool _loadTried;
    static TArenaCreate _ArenaCreate;
    static TArenaAllocation _ArenaAllocation;
    static TArenaFree _ArenaFree;
    //static TArenaMark _ArenaMark;
    static TArenaRelease _ArenaRelease;
    static TArenaDestroy _ArenaDestroy;

    static HArena recyclerArena;
    
public:
    static bool IsActive() { return _ArenaCreate != null; }

    static void EnsureRecyclerArena() 
    {
        if (!Initialize()) return;

        if (!recyclerArena)
        {
            recyclerArena = _ArenaCreate(null, L"Recycler", 0);
        }
    }

    static void DestroyRecyclerArena()
    {
        if (recyclerArena && _ArenaRelease && _ArenaDestroy)
        {
            _ArenaRelease(recyclerArena, (HMark)0);
            _ArenaDestroy(recyclerArena);
            recyclerArena = (HArena)0;
        }
    }

    static BOOL ReportRecyclerAlloc(__in char *address, size_t size)
    {
        if (_ArenaAllocation && recyclerArena)
            return _ArenaAllocation(recyclerArena, size, address, 0);
        return true;
    }

    static BOOL ReportRecyclerFree(__in char *address, size_t size)
    {
        if (_ArenaFree && recyclerArena)
            return _ArenaFree(recyclerArena, address);
        return true;
    }

    static BOOL ReportRecyclerUnallocated(__in char * address, __in char *endAddress, size_t sizeCat)
    {
        if (_ArenaAllocation && recyclerArena)
        {
            Assert(endAddress >= address);
            Assert(sizeCat > 0);

            while (address + sizeCat <= endAddress)
            {
                _ArenaAllocation(recyclerArena, sizeCat, address, 0);
                address += sizeCat;
            }
        }
        return true;
    }

    static HArena ArenaCreate(__in LPCWSTR lpwName)
    {
        if (_ArenaCreate) return _ArenaCreate(0, lpwName, 0);
        return (HArena)0;
    }

    static BOOL ArenaAllocation(HArena hArena, size_t sSize, void *pAddress, size_t sUserDefinedType)
    {
        if (_ArenaAllocation) return _ArenaAllocation(hArena, sSize, pAddress, sUserDefinedType);
        return true;
    }

    static BOOL ArenaFree(HArena hArena, void *pAddress)
    {
        if (_ArenaFree) return _ArenaFree(hArena, pAddress);
        return true;
    }

    //static HMark ArenaMark(HArena hArena)
    //{
    //    if (_ArenaMark) return _ArenaMark(hArena);
    //    return (HMark)null;
    //}

    static BOOL ArenaRelease(HArena hArena, HMark hMark)
    {
        if (_ArenaRelease) return _ArenaRelease(hArena, hMark);
        return true;
    }

    static BOOL ArenaDestroy(HArena hArena)
    {
        if (_ArenaDestroy) return _ArenaDestroy(hArena);
        return true;
    }

    // Canning Initialize() will return true if a usable version of Memspect is loaded.
    // This routine can be called multiple times and will try to detect Memspect once returning
    // the same return result for multiple calls.
    static bool Initialize() 
    {
        if (!_ArenaCreate)
        {
            if (!_loadTried)
            {
                _loadTried = true;
                HMODULE dll = ::GetModuleHandle(L"MEMSPECTDLL");
                if (dll)
                {
                    _ArenaCreate = (TArenaCreate) GetProcAddress(dll, "_ArenaCreated@12");
                    _ArenaAllocation = (TArenaAllocation) GetProcAddress(dll, "_ArenaAllocation@16");
                    _ArenaFree = (TArenaFree) GetProcAddress(dll, "_ArenaFree@8");
                    //_ArenaMark = (TArenaMark) GetProcAddress(dll, "_ArenaMark@4");
                    _ArenaRelease = (TArenaRelease) GetProcAddress(dll, "_ArenaRelease@8");
                    _ArenaDestroy = (TArenaDestroy) GetProcAddress(dll, "_ArenaDestroy@4");
            
                    if (_ArenaCreate && _ArenaAllocation && _ArenaFree && /* _ArenaMark && */ _ArenaRelease && _ArenaDestroy)
                    {                    
                        return true;
                    }
                    else 
                    {
                        // ArenaCreate is used as the flag to indicate whether MEMSPECT.DLL was successfully loaded.
                        _ArenaCreate = null;
                    }
                }
            }
            return false;
        }
        else
            return true;
    }
};

bool MemspectMemoryTracker::_loadTried = false;
MemspectMemoryTracker::TArenaCreate MemspectMemoryTracker::_ArenaCreate = null;
MemspectMemoryTracker::TArenaAllocation MemspectMemoryTracker::_ArenaAllocation = null;
MemspectMemoryTracker::TArenaFree MemspectMemoryTracker::_ArenaFree = null;
//MemspectMemoryTracker::TArenaMark MemspectMemoryTracker::_ArenaMark = null;
MemspectMemoryTracker::TArenaRelease MemspectMemoryTracker::_ArenaRelease = null;
MemspectMemoryTracker::TArenaDestroy MemspectMemoryTracker::_ArenaDestroy = null;
MemspectMemoryTracker::HArena MemspectMemoryTracker::recyclerArena = null;

// Arena Tracking

typedef JsUtil::BaseDictionary<Allocator *, MemspectMemoryTracker::HArena, HeapAllocator, PrimeSizePolicy, RecyclerPointerComparer> ArenaMap;

ArenaMap* arenaMap = null;

bool arenaMemoryTrackingEnabled = false;
void ArenaMemoryTracking::Activate()
{
    arenaMemoryTrackingEnabled = true;
}

void ArenaMemoryTracking::ArenaCreated(Allocator *arena, __in LPCWSTR name)
{
    if (arenaMemoryTrackingEnabled)
    {
        if (MemspectMemoryTracker::Initialize())
        {
            if (!arenaMap)
                arenaMap = new ArenaMap(&HeapAllocator::Instance, 1);
            auto tracker = MemspectMemoryTracker::ArenaCreate(name);
            arenaMap->Item(arena, tracker);
        }
    }
}

MemspectMemoryTracker::HArena FindTracker(Allocator *arena)
{
    MemspectMemoryTracker::HArena result = (MemspectMemoryTracker::HArena)0;
    if (arenaMap)
    {
        result = arenaMap->Lookup(arena, null);
    }
    return result;
}

void ArenaMemoryTracking::ArenaDestroyed(Allocator *arena)
{
    auto tracker = FindTracker(arena);
    if (tracker) 
    {
        arenaMap->Remove(arena);
        MemspectMemoryTracker::ArenaDestroy(tracker);
    }
}

void ArenaMemoryTracking::ReportAllocation(Allocator *arena, void *address, size_t size)
{
    auto tracker = FindTracker(arena);
    if (tracker) 
    {
        MemspectMemoryTracker::ArenaAllocation(tracker, size, address, 0);
    }
}

void ArenaMemoryTracking::ReportReallocation(Allocator *arena, void *address, size_t existingSize, size_t newSize)
{
    auto tracker = FindTracker(arena);
    if (tracker) 
    {
        Assert(existingSize > newSize);

        // This is an odd case that is not special case'd by the Memspect api so we explicitly do
        // operations to ensure Memspect is in sync. What we do is report the enitre block as free
        // then report the part we keeping and the part we are freeing as being allocated independantly,
        // then calling free will report the part we are free'ing as free.
        MemspectMemoryTracker::ArenaFree(tracker, address);
        MemspectMemoryTracker::ArenaAllocation(tracker, newSize, address, 0);
        MemspectMemoryTracker::ArenaAllocation(tracker, existingSize - newSize, ((char *)address) + newSize, 1);
    }
}

void ArenaMemoryTracking::ReportFree(Allocator *arena, void *address, size_t size)
{
    auto tracker = FindTracker(arena);
    if (tracker) 
    {
        MemspectMemoryTracker::ArenaFree(tracker, address);
    }
}

void ArenaMemoryTracking::ReportFreeAll(Allocator *arena)
{
    auto tracker = FindTracker(arena);
    if (tracker) 
    {
        MemspectMemoryTracker::ArenaRelease(tracker, (MemspectMemoryTracker::HMark)0);
    }
}

// Recycler tracking

bool recyclerMemoryTrackingEnabled = false;

void RecyclerMemoryTracking::Activate()
{
    recyclerMemoryTrackingEnabled = true;
}

bool RecyclerMemoryTracking::IsActive()
{
    return recyclerMemoryTrackingEnabled && MemspectMemoryTracker::IsActive();
}

// The external reporting for the recycler uses the MemspectMemoryTracker
void RecyclerMemoryTracking::ReportRecyclerCreate(Recycler * recycler)
{
    if (recyclerMemoryTrackingEnabled)
        MemspectMemoryTracker::EnsureRecyclerArena();
}

void RecyclerMemoryTracking::ReportRecyclerDestroy(Recycler * recycler)
{
    MemspectMemoryTracker::DestroyRecyclerArena();
}

void RecyclerMemoryTracking::ReportAllocation(Recycler * recycler, __in void *address, size_t size)
{
    MemspectMemoryTracker::ReportRecyclerAlloc((char*)address, size);
}

void RecyclerMemoryTracking::ReportFree(Recycler * recycler, __in void *address, size_t size)
{
    MemspectMemoryTracker::ReportRecyclerFree((char*)address, size);
}

void RecyclerMemoryTracking::ReportUnallocated(Recycler * recycler, __in void* address, __in void *endAddress, size_t sizeCat)
{
    MemspectMemoryTracker::ReportRecyclerUnallocated((char*)address, (char*)endAddress, sizeCat);
}

// Page tracking

typedef JsUtil::BaseDictionary<PageAllocator *, MemspectMemoryTracker::HArena, HeapAllocator, PrimeSizePolicy, RecyclerPointerComparer> PageTrackerMap;
bool pageTrackingEnabled = false;
PageTrackerMap* pageTrackerMap = null;

void PageTracking::Activate()
{
    pageTrackingEnabled = true;
}

MemspectMemoryTracker::HArena FindTracker(PageAllocator *pageAllocator)
{
    MemspectMemoryTracker::HArena result = (MemspectMemoryTracker::HArena)0;
    if (pageTrackerMap)
    {
        result = pageTrackerMap->Lookup(pageAllocator, null);
    }
    return result;
}

void PageTracking::PageAllocatorCreated(PageAllocator *pageAllocator)
{
    if (pageTrackingEnabled)
    {
        if (MemspectMemoryTracker::Initialize())
        {
            if (!pageTrackerMap)
                pageTrackerMap = new PageTrackerMap(&HeapAllocator::Instance, 1);
            auto tracker = MemspectMemoryTracker::ArenaCreate(L"PageAllocator");
            pageTrackerMap->Item(pageAllocator, tracker);
        }
    }
}

void PageTracking::PageAllocatorDestroyed(PageAllocator *pageAllocator)
{
    auto tracker = FindTracker(pageAllocator);
    if (tracker)
    {
        MemspectMemoryTracker::ArenaDestroy(tracker);
        pageTrackerMap->Remove(pageAllocator);
    }
}

void PageTracking::ReportAllocation(PageAllocator *pageAllocator, __in void *address, size_t size)
{
    if (address) 
    {
        auto tracker = FindTracker(pageAllocator);
        if (tracker)
        {
            char const *current = (char const *)address;
            char const *end = current + size;
            while (current < end)
            {
                MemspectMemoryTracker::ArenaAllocation(tracker, AutoSystemInfo::Data.dwPageSize, (void*)current, 0);
                current += AutoSystemInfo::Data.dwPageSize;
            }
        }
    }
}

void PageTracking::ReportFree(PageAllocator *pageAllocator, __in void *address, size_t size)
{
    auto tracker = FindTracker(pageAllocator);
    if (tracker)
    {
        char const *current = (char const *)address;
        char const *end = current + size;
        while (current < end)
        {
            MemspectMemoryTracker::ArenaFree(tracker, (void*)current);
            current += AutoSystemInfo::Data.dwPageSize;
        }
    }
}
#endif