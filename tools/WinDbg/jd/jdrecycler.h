//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

#ifdef JD_PRIVATE
#include <math.h>
#include "RemoteRecycler.h"

// STL headers
#include <hash_set>
#include <hash_map>
#include <stack>
#include <set>
#include <algorithm>
#include <string>

class AutoFree
{
public:
    ~AutoFree()
    {
        if (_ptr) free(_ptr);
    }

    AutoFree(void* ptr):
        _ptr(ptr)
    {
    }
private:
    void* _ptr;
};

template <typename T>
class AutoDelete
{
public:
    AutoDelete(T * t) : _ptr(t) {};
    ~AutoDelete() { if (_ptr) delete _ptr; }
    operator T*() const { return _ptr; }
    T *operator->() const { return _ptr; }
    T * Detach()
    {
        T * t = _ptr;
        _ptr = nullptr;
        return t;
    }
private:
    T * _ptr;
};
#endif
