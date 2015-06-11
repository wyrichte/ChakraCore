//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Memory
{
template <typename T>
class RecyclerRootPtr 
{
public:
    RecyclerRootPtr() : ptr(null) {};
    ~RecyclerRootPtr() { Assert(ptr == null); }
    void Root(T * ptr, Recycler * recycler) { Assert(this->ptr == null); recycler->RootAddRef(ptr); this->ptr = ptr; }
    void Unroot(Recycler * recycler) { Assert(this->ptr != null); recycler->RootRelease(this->ptr); this->ptr = null; }
    
    T * operator->() const { Assert(ptr != null); return ptr; }
    operator T*() const { return ptr; }
protected:
    T * ptr;
private:
    RecyclerRootPtr(const RecyclerRootPtr<T>& ptr); // Disable
    RecyclerRootPtr& operator=(RecyclerRootPtr<T> const& ptr); // Disable
};

typedef RecyclerRootPtr<void> RecyclerRootVar;

template <typename T>
class AutoRecyclerRootPtr : public RecyclerRootPtr<T>
{
public:
    AutoRecyclerRootPtr(T * ptr, Recycler * recycler) : recycler(recycler)
    {
        Root(ptr);
    }
    ~AutoRecyclerRootPtr()
    {
        Unroot();
    }

    void Root(T * ptr) 
    {
        Unroot();
        __super::Root(ptr, recycler);
    }
    void Unroot()
    {
        if (ptr != null)
        {
            __super::Unroot(recycler);
        }
    }    
    Recycler * GetRecycler() const
    {
        return recycler;
    }
private:
    Recycler * const recycler;
};

typedef AutoRecyclerRootPtr<void> AutoRecyclerRootVar;
}