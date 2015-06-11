/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once
#define _DO_NOT_DECLARE_INTERLOCKED_INTRINSICS_IN_MEMORY
#define __PLACEMENT_NEW_INLINE
#pragma warning(push)
#pragma warning(disable : 4995)
#include <map>
#pragma warning(pop)

// just wraps malloc/free
struct allocator_base
{
    char tmp;

    virtual void* allocate(size_t n)
    {
        return malloc(n);
    }
    virtual void free(void *p)
    {
        ::free(p);
    }
    virtual void update_buffer(__out_bcount(len) void *dest, __in_bcount(len) const void* src, size_t len)
    {
        js_memcpy_s(dest, len, src, len);
    }
    virtual void update_pointer(void** dest,void* src) 
    {
        *dest = src;
    }

    virtual ~allocator_base() { }
};

// wraps the recycler
struct recycler_wrapper : allocator_base
{
    Recycler *recycler;

    recycler_wrapper(Recycler* recycler) : recycler(recycler) { }

    virtual void* allocate(size_t n)
    {
        void *tmp = recycler->Alloc((int)n);
        if(tmp == 0)
            printf("RECYCLER ERROR: 0x0 allocated\n");
        return tmp;
    }
    virtual void free(void *p)
    {
        // nothing
    }
    virtual void update_buffer(void **dest, const void *src, size_t len)
    {
        js_memcpy_s(dest, len, src, len);        
    }
    virtual void update_pointer(void **dest, void* src)
    {
        *dest = src;        
    }
};

struct tracker_impl_base
{
    enum Liveness { Alive = 1, Dead = 0 };

    virtual bool check(void *addr) = 0;
    virtual void set(void *addr) = 0;
    virtual void clear(void *addr) = 0;
    virtual void dump_live() = 0;
    virtual ~tracker_impl_base() { };
};

// uses a std::map for tracking live objects
struct stlmap_tracker : tracker_impl_base
{
    // map of objects to liveness (NOTE: we should probably store obfuscated pointers, in case 
    // these ever end up on the stack)
    std::map<void*, Liveness> objects;
    typedef std::map<void*, Liveness>::iterator iter_t;

    virtual void set(void *addr);
    virtual void clear(void *addr);
    virtual bool check(void *addr);
    virtual void dump_live();
 
};

// uses a giant (~64MB) bit-vector to track live objects
struct memorymapped_tracker : tracker_impl_base
{
    

    // assume 4-byte aligned allocations
    static const size_t BitsPerByte = 8;
    size_t minaddress;
    size_t maxaddress;
    size_t tablesize;

#ifdef _M_X64
    static const size_t SlotCount = 16 * 1024;
    volatile char** slottable;
#else
    volatile char* base;
#endif
    memorymapped_tracker();
    ~memorymapped_tracker();

    virtual void set(void *addr);
    virtual void clear(void *addr);
    virtual bool check(void *addr);
    virtual void dump_live();

    volatile char * from_address(void * addr, size_t& byte, size_t& bit);
    void *to_address(size_t slot, size_t loc, size_t bit);

    void set_internal(void *addr, Liveness liveness);

private:
    void dump_live(size_t slot, volatile char * table);
};

// runs both stlmap_tracker and memorymapped_tracked, and verifies they agree
struct dual_tracker : tracker_impl_base
{
    stlmap_tracker t1;
    memorymapped_tracker t2;

    virtual void set(void *addr);
    virtual void clear(void *addr);
    virtual bool check(void *addr);
    virtual void dump_live();
};



// class to wrap recycler memory allocations, and separately track object lifetimes
struct tracker
{
    enum DebugLevel { None, Errors, All };
    DebugLevel dbg_level;

    tracker(allocator_base *allocator) : dbg_level(Errors), allocator(allocator), object_tracker(new memorymapped_tracker) { }

    ~tracker() { delete allocator; delete object_tracker; }

    // allocator to fwd calls to
    allocator_base *allocator;

    // object tracker implementation
    tracker_impl_base *object_tracker;


    // allocates memory using the recycler
    void *track_malloc(size_t size);

    // does not actually free memory, but marks the address as freed
    void track_free(void *p);

    // checks whether an address is alive
    bool is_alive(void *p);

    // not implemented
    void verify_free();

    // dump the tracker's state.  various params.
    void dump(bool dump_allocated = true, bool dump_free = true);
};
