//depot/xboxmain/inetcore/jscript/Dll/thunk/util.h#1 - branch change 1301677 (text)
#pragma once

// from developer\jaykrell\util.h

class SRWLockHolderExclusive_t
{
    SRWLOCK* m_Lock;

    void Lock(SRWLOCK* Lock) { Unlock(); m_Lock = Lock; if (Lock) AcquireSRWLockExclusive(Lock); }
    void Unlock() { if (m_Lock) ReleaseSRWLockExclusive(m_Lock); m_Lock = 0; }

public:
    SRWLockHolderExclusive_t(SRWLOCK* L) : m_Lock(0) { Lock(L); }
    ~SRWLockHolderExclusive_t() { Unlock(); }
};

class SRWLockHolderShared_t
{
    SRWLOCK* m_Lock;

    void Lock(SRWLOCK* Lock) { Unlock(); m_Lock = Lock; if (Lock) AcquireSRWLockShared(Lock); }
    void Unlock() { if (m_Lock) ReleaseSRWLockShared(m_Lock); m_Lock = 0; }

public:
    SRWLockHolderShared_t(SRWLOCK* L) : m_Lock(0) { Lock(L); }
    ~SRWLockHolderShared_t() { Unlock(); }
};

template <typename Derived_t, typename T>
class VoidPtrBase_t
{
protected:
    T p;

public:
    VoidPtrBase_t() : p(0) { }
    VoidPtrBase_t(VoidPtrBase_t&& other) { p = other.p; other.p = 0; }
    VoidPtrBase_t(__in_opt T q) : p(q) { }

    ~VoidPtrBase_t() { static_cast<Derived_t*>(this)->StaticCleanup(Detach()); }

    T operator->() { return p; }
    operator T() { return p; }

    const T operator->() const { return p; }
    operator const T() const { return p; }

    void operator=(__in_opt T q)
    {
        T a = p;
        p = q;
        if (a != 0 && a != ((T)(ptrdiff_t)-1) && a != q)
            static_cast<Derived_t*>(this)->StaticCleanup(a);
    }

    bool operator==(const __in_opt T q) const { return q == p; }
    bool operator!=(const __in_opt T q) const { return q != p; }

    T get() { return p; } /* compat with unique_ptr/shared_ptr */
    T GetPointer() { return p; }

    T Detach() /* no free */
    {
        T a = p;
        p = 0;
        return a;
    }

    void Cleanup() { static_cast<Derived_t*>(this)->StaticCleanup(Detach()); }

    void Free() { Cleanup(); }
    void Release() { Cleanup(); }

    void Attach(__in_opt T q) { operator=(q); }

protected:
    class ConstOperatorAmpersandResult_t
    /* Allow & to provide either type, depending on what the context needs. */
    {
    protected:
        const VoidPtrBase_t* p;

    public:
        explicit ConstOperatorAmpersandResult_t(const VoidPtrBase_t* q) : p(q) { }
        operator T const **() const { return &((*p).p); }
        operator Derived_t const *() { return static_cast<Derived_t const *>(p); }
    };

    class OperatorAmpersandResult_t : public ConstOperatorAmpersandResult_t
    /* Allow & to provide either type, depending on what the context needs. */
    {
    public:
        explicit OperatorAmpersandResult_t(VoidPtrBase_t* q) : ConstOperatorAmpersandResult_t(q) { }
        operator T*() { return const_cast<T*>(&((*p).p)); }
        operator Derived_t*() { return static_cast<Derived_t*>(const_cast<VoidPtrBase_t*>(p)); }
    };

public:
    ConstOperatorAmpersandResult_t operator&() const { return ConstOperatorAmpersandResult_t(this); }
    OperatorAmpersandResult_t operator&() { return OperatorAmpersandResult_t(this); }

private:
    VoidPtrBase_t(const VoidPtrBase_t&); /* deliberately not implemented */
    void operator=(const VoidPtrBase_t&); /* deliberately not implemented */
};

template <typename Derived_t, typename T>
class PtrBase_t : public VoidPtrBase_t<Derived_t, T*>
{
    typedef VoidPtrBase_t<Derived_t, T*> Base_t;
public:
    PtrBase_t() { }
    PtrBase_t(PtrBase_t&& other) { p = other.p; other.p = 0; }
    PtrBase_t(__in_opt T* q) : Base_t(q) { }
    PtrBase_t& operator=(__in_opt T* q) { Base_t::operator=(q); return *this; }

    /* These distinguish PtrBase_t from VoidPtrBase_t. */
    T& operator*() { return *p; }
    const T& operator*() const { return *p; }

private:
    PtrBase_t(const PtrBase_t&); /* deliberately not implemented */
    void operator=(const PtrBase_t&); /* deliberately not implemented */
};

template <typename T>
class ProcessHeapPtr_t : public PtrBase_t<ProcessHeapPtr_t<T>, T>
{
public:
    ProcessHeapPtr_t() { }
    ProcessHeapPtr_t(ProcessHeapPtr_t&& other) { p = other.p; other.p = 0; }
    ProcessHeapPtr_t(__in_opt T* q) : PtrBase_t(q) { }
    ProcessHeapPtr_t& operator=(__in_opt T* q) { PtrBase_t::operator=(q); return *this; }
    T& operator*() { return *p; }
    const T& operator*() const { return *p; }

    static void StaticCleanup(__in_opt T* a) { HeapFree(GetProcessHeap(), 0, a); }

private:
    ProcessHeapPtr_t(const ProcessHeapPtr_t&); /* deliberately not implemented */
    void operator=(const ProcessHeapPtr_t&); /* deliberately not implemented */
};

template <typename T = void>
class MappedViewOfFile_t : public PtrBase_t<MappedViewOfFile_t<T>, T>
{
public:
    MappedViewOfFile_t() { }
    MappedViewOfFile_t(MappedViewOfFile_t&& other) { p = other.p; other.p = 0; }
    MappedViewOfFile_t(__in_opt T* q) : PtrBase_t(q) { }
    MappedViewOfFile_t& operator=(__in_opt T* q) { PtrBase_t::operator=(q); return *this; }

    static void StaticCleanup(__in_opt void* a) { if (a != NULL) UnmapViewOfFile(a);  }

private:
    MappedViewOfFile_t(const MappedViewOfFile_t&); /* deliberately not implemented */
    void operator=(const MappedViewOfFile_t&); /* deliberately not implemented */
};

template <typename Derived_t, typename T = void*>
class HandleBase_t : public VoidPtrBase_t<Derived_t, T>
{
    typedef VoidPtrBase_t<Derived_t, T> Base_t;
public:
    HandleBase_t() { }
    HandleBase_t(HandleBase_t&& other) { p = other.p; other.p = 0; }
    HandleBase_t(T q) : Base_t(q) { }
    void operator=(T q) { Base_t::operator=(q); }

    T GetHandle() { return GetPointer(); }

    bool operator==(T q) const { return p == q; }
    bool operator!=(T q) const { return p != q; }

private:
    HandleBase_t(const HandleBase_t&); /* deliberately not implemented */
    void operator=(const HandleBase_t&); /* deliberately not implemented */
};

class Handle_t : public HandleBase_t<Handle_t>
{
    typedef HandleBase_t<Handle_t> Base_t;
public:
    Handle_t() { }
    Handle_t(Handle_t&& other) { p = other.p; other.p = 0; }
    Handle_t(HANDLE q) : Base_t(q) { }
    Handle_t& operator=(HANDLE q) { Base_t::operator=(q); return *this; }

    static void StaticCleanup(HANDLE a)
    {
        if (a != NULL && a != INVALID_HANDLE_VALUE)
           CloseHandle(a); 
    }

private:
    Handle_t(const Handle_t&); /* deliberately not implemented */
    void operator=(const Handle_t&); /* deliberately not implemented */
};

class RtlGrowableFunctionTable_t : public HandleBase_t<Handle_t>
// Growable function table is PVOID, the same as HANDLE. Internally it is a pointer.
{
    typedef HandleBase_t<Handle_t> Base_t;
public:
    RtlGrowableFunctionTable_t() { }
    RtlGrowableFunctionTable_t(RtlGrowableFunctionTable_t&& other) { p = other.p; other.p = 0; }
    RtlGrowableFunctionTable_t(PVOID q) : Base_t(q) { }
    RtlGrowableFunctionTable_t& operator=(PVOID q) { Base_t::operator=(q); return *this; }

#ifdef _X86_
    static void StaticCleanup(PVOID) { }
#else
    static void StaticCleanup(PVOID a)
    {
        if (a != NULL && a != INVALID_HANDLE_VALUE)
           RtlDeleteGrowableFunctionTable(a); 
    }
#endif
};
