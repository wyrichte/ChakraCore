/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

void* tracker::track_malloc(size_t size)
{
    void *tmp = allocator->allocate(size);
    Assert(tmp != 0);

    if(dbg_level >= All)
        printf("NOTE: recycler allocated object at address %p\n", tmp);

    object_tracker->set(tmp);

    return tmp;
}

void tracker::verify_free()
{
    // TODO
    //assert(0);
}

bool tracker::is_alive(void *p)
{
    return object_tracker->check(p);
}

void tracker::track_free(void *p)
{
    if(dbg_level >= All)
        printf("NOTE: test freed object at address %p\n", p);

    object_tracker->clear(p);
    allocator->free(p);
}


void tracker::dump(bool dump_allocated, bool dump_free)
{

}

void stlmap_tracker::set(void *addr)
{
    iter_t it = objects.find(addr);
    if(it == objects.end())
    {
        objects[addr] = Alive;
    }
    else if(it->second == Alive)
    {
        printf("RECYCLER ERROR: stlmap reported re-allocated live object at address %p\n", addr);
        fflush(stdout);
        exit(1);
    }
    else
    {
        objects[addr] = Alive;
    }
}

void stlmap_tracker::clear(void *addr)
{
    iter_t it = objects.find(addr);
    if(it == objects.end())
    {
        printf("RECYCLER ERROR: stlmap reported free of unknown object at address %p\n", addr);
    }
    else if(it->second == Dead)
    {
        printf("RECYCLER ERROR: stlmap reported double-free of object at address %p\n", addr);
    }
    else
    {
        objects[addr] = Dead;
    }
}
bool stlmap_tracker::check(void *addr)
{
    iter_t it = objects.find(addr);
    if(it == objects.end() || it->second == Dead)
    {
        return false;
    }
    else
    {  
        return true;
    }
}
void stlmap_tracker::dump_live()
{
    printf("=== stlmap_tracker live object dump ===\n");
    for each(std::pair<void*,Liveness> obj in objects)
    {
        if(obj.second == Alive)
        {
            printf("%p\n", obj.first);
        }
    }
}

memorymapped_tracker::memorymapped_tracker()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    minaddress = (size_t)systemInfo.lpMinimumApplicationAddress;
    maxaddress = (size_t)systemInfo.lpMaximumApplicationAddress;
    tablesize = ((maxaddress - minaddress + 1) + ((1 << HeapConstants::ObjectAllocationShift) * BitsPerByte - 1)) / (1 << HeapConstants::ObjectAllocationShift) / BitsPerByte;

#ifdef _M_X64
    slottable = (volatile char**)VirtualAlloc(0, SlotCount * sizeof(volatile char *), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    tablesize = (tablesize + SlotCount - 1) / SlotCount;
    if(slottable == 0)
        printf("TEST ERROR: VirtualAlloc failed\n");
    memset((void*)slottable, 0, SlotCount);
#else
    base = (char*)VirtualAlloc(0, tablesize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    if(base == 0)
        printf("TEST ERROR: VirtualAlloc failed\n");
    memset((void*)base, 0, tablesize);
#endif



}
memorymapped_tracker::~memorymapped_tracker()
{
#ifdef _M_X64
    for (size_t i = 0; i < SlotCount; i++)
    {
        if (slottable[i] != nullptr)
        {
            VirtualFree((void *)slottable[i], 0, MEM_RELEASE);
        }
    }
    VirtualFree((void *)slottable, 0, MEM_RELEASE);
#else
    VirtualFree((void*)base, 0, MEM_RELEASE);
#endif
}

volatile char * memorymapped_tracker::from_address(void * addr, size_t& byte, size_t& bit)
{
    size_t iaddr = (size_t)addr;

    if (iaddr < minaddress)
        printf("RECYCLER ERROR: found address < minaddress: %p\n", addr);

    if (iaddr > maxaddress)
        printf("RECYCLER ERROR: found address > maxaddress: %p\n", addr);

    if(iaddr % (1 << HeapConstants::ObjectAllocationShift) != 0)
        printf("RECYCLER ERROR: address was not aligned properly: %p\n", addr);

    iaddr = (iaddr - minaddress) >> HeapConstants::ObjectAllocationShift;
    
#ifdef _M_X64
    size_t slot = iaddr / (tablesize * BitsPerByte) ;
    iaddr = iaddr % (tablesize * BitsPerByte);    
    if (slot >= SlotCount)
        printf("TEST ERROR: calculated address > slotcount.  Address was %p\n", addr);

    volatile char * base = slottable[slot];
    if (base == nullptr)
    {
        base = (char*)VirtualAlloc(0, tablesize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        if(base == 0)
            printf("TEST ERROR: VirtualAlloc failed\n");
        memset((void*)base, 0, tablesize);
        slottable[slot] = base;
    }
#else
    size_t slot = 0;
#endif

    byte = iaddr / BitsPerByte;
    bit = iaddr % BitsPerByte;

    if(byte >= tablesize)
        printf("TEST ERROR: calculated address > tablesize.  Address was %p\n", addr);

    if (to_address(slot, byte, bit) != addr)
        printf("TEST ERROR: to_address bug\n");

    return base;
}

void memorymapped_tracker::set_internal(void *addr, tracker_impl_base::Liveness liveness)
{
    size_t byte;
    size_t bit;
    volatile char * table = from_address(addr, byte, bit);
    if(liveness)
    {
        // TODO: thread safety
        if( (table[byte] & (1 << bit)) != 0)
            printf("RECYCLER ERROR: memory map reported re-allocated live object at %p\n", addr);

        // TODO: thread safety
        // table[byte] |= (1 << bit);
        _InterlockedOr8(&table[byte], 1<<bit);
    }
    else
    {
        // TODO: thread safety
        if( (table[byte] & (1 << bit)) == 0)
            printf("RECYCLER ERROR: memory map reported double-free of object at %p\n", addr);

        // TODO: thread safety
        //table[byte] &= ~(1<<bit);
        _InterlockedAnd8(&table[byte], ~(1<<bit));
    }
    // Clear the item from the stack for cleanliness since otherwise 
    // the GC could construe this object as being alive
    addr = nullptr; 
}
void memorymapped_tracker::set(void *addr)
{
    set_internal(addr, Alive);
    // Clear the item from the stack for cleanliness since otherwise 
    // the GC could construe this object as being alive
    addr = nullptr; 
}
void memorymapped_tracker::clear(void *addr)
{
    set_internal(addr, Dead);
    // Clear the item from the stack for cleanliness since otherwise 
    // the GC could construe this object as being alive
    addr = nullptr; 
}

bool memorymapped_tracker::check(void *addr)
{
    size_t byte;
    size_t bit;
    volatile char * table = from_address(addr, byte, bit);
    // Clear the item from the stack for cleanliness since otherwise 
    // the GC could construe this object as being alive
    addr = nullptr; 

    // TODO: thread safety
    return (table[byte] & (1<<bit)) ? true : false;
}

void* memorymapped_tracker::to_address(size_t slot, size_t loc, size_t bit)
{
    size_t iaddr = 0;  

    iaddr = loc * BitsPerByte;
    iaddr += bit;
#ifdef _M_X64
    iaddr += slot * BitsPerByte * tablesize;
#else
    if (slot != 0)    
        printf("TEST ERROR: slot not zero for 32-bit platforms");    
#endif
    iaddr <<= HeapConstants::ObjectAllocationShift;
    return (void*)(iaddr + minaddress);
}

void memorymapped_tracker::dump_live()
{
#ifdef _M_X64
    for (size_t i = 0; i < SlotCount; i++)
    {
        if (slottable[i] != nullptr)
        {
            dump_live(i, slottable[i]);
        }
    }
#else
    dump_live(0, base);
#endif
}
void memorymapped_tracker::dump_live(size_t slot, volatile char * table)
{
    printf("=== memorymapped_tracker live object dump ===\n");
    for(size_t i = 0; i < tablesize; ++i)
    {
        volatile char curr = table[i];
        for(size_t j = 0; j < BitsPerByte; ++j)
        {
            if(curr & (1<<j))
            {
                printf("%p\n", to_address(slot, i, j));
            }
        }
    }
}

void dual_tracker::set(void *addr)
{
    if(t1.check(addr) != t2.check(addr))
    {
        printf("TEST ERROR: t1, t2 disagree on address %p\n", addr);
    }

    t1.set(addr);
    t2.set(addr);

    if(t1.check(addr) != t2.check(addr))
    {
        printf("TEST ERROR: t1, t2 disagree on address %p\n", addr);
    }
}
void dual_tracker::clear(void *addr)
{
    if(t1.check(addr) != t2.check(addr))
    {
        printf("TEST ERROR: t1, t2 disagree on address %p\n", addr);
    }

    t1.clear(addr);
    t2.clear(addr);

    if(t1.check(addr) != t2.check(addr))
    {
        printf("TEST ERROR: t1, t2 disagree on address %p\n", addr);
    }
}
bool dual_tracker::check(void *addr)
{
    bool b1 = t1.check(addr);
    bool b2 = t2.check(addr);
    if(b1 != b2)
    {
        printf("TEST ERROR: t1, t2 disagree on address %p\n", addr);
    }
    return b2;
}
void dual_tracker::dump_live()
{
    t1.dump_live();
    t2.dump_live();
}