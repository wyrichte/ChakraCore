//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"

namespace JsDiag
{
    const _oomthrow_t oomthrow;
    const _nothrow_t nothrow;

    void* _oomthrow_t::check_alloc(void* p) throw(std::bad_alloc)
    {
        if (p == nullptr)
        {
            static const std::bad_alloc oom;
            throw oom;
        }

        return p;
    }

    void* _oomthrow_t::allocate(size_t size) const throw(std::bad_alloc)
    {
        return check_alloc(nothrow.allocate(size));
    }

    void* _oomthrow_t::allocate_array(size_t size) const throw(std::bad_alloc)
    {
        return check_alloc(nothrow.allocate_array(size));
    }

    void* _nothrow_t::allocate(size_t size) const throw()
    {
        JSDIAG_INJECT_ALLOC_FAIL
        {
            return nullptr;
        }

        return ::operator new(size);
    }

    void* _nothrow_t::allocate_array(size_t size) const throw()
    {
        JSDIAG_INJECT_ALLOC_FAIL
        {
            return nullptr;
        }

        return ::operator new[](size);
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    static int s_alloc_counter = 0;
    static int s_alloc_fault = -1;

    bool _ShouldAllocFail()
    {
        if (s_alloc_fault < 0)
        {
            char16 buf[16];
            if (GetEnvironmentVariableW(_u("JsDiag_AllocFail"), buf, _countof(buf)) > 0)
            {
                s_alloc_fault = _wtoi(buf);
            }

            if (s_alloc_fault < 0)
            {
                s_alloc_fault = 0; // Make it >= 0
            }
        }

        return ++s_alloc_counter == s_alloc_fault;
    }
#endif
}
