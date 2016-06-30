//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once
#include <new>

// We expect operator new to throw std::bad_alloc when OOM as in C++ standard. However,
// by default Windows components link to msvcrt.dll which returns null instead of throw.
//
// Override operator new to specify the throw behavior explicitly.

namespace JsDiag
{
    class _oomthrow_t
    {
    private:
        static void* check_alloc(void* p) throw(std::bad_alloc);
    public:
        void* allocate(size_t size) const throw(std::bad_alloc);
        void* allocate_array(size_t size) const throw(std::bad_alloc);
    };

    class _nothrow_t
    {
    public:
        void* allocate(size_t size) const throw();
        void* allocate_array(size_t size) const throw();
    };

    extern const _oomthrow_t oomthrow;
    extern const _nothrow_t nothrow;

    bool _ShouldAllocFail();
}

inline void* __cdecl operator new(size_t size, const JsDiag::_oomthrow_t& alloc) throw(std::bad_alloc)
{
    return alloc.allocate(size);
}

inline void* __cdecl operator new[](size_t size, const JsDiag::_oomthrow_t& alloc) throw(std::bad_alloc)
{
    return alloc.allocate_array(size);
}

inline void* __cdecl operator new(size_t size, const JsDiag::_nothrow_t& alloc) throw()
{
    return alloc.allocate(size);
}

inline void* __cdecl operator new[](size_t size, const JsDiag::_nothrow_t& alloc) throw()
{
    return alloc.allocate_array(size);
}

inline void __cdecl operator delete[](void * pointer, const JsDiag::_oomthrow_t&) throw()
{
    ::operator delete[](pointer);
}

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#define JSDIAG_INJECT_ALLOC_FAIL if (JsDiag::_ShouldAllocFail())
#else
#define JSDIAG_INJECT_ALLOC_FAIL if (false)
#endif
