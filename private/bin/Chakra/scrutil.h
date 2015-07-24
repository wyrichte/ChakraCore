//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// (BUG 1202468, 1202470, 1202471, 1202473, 1202474 - Windows OS Bugs)
// SAL: TODO: Move to common header for SAL work
#ifdef _PREFIX_
#if __cplusplus
extern "C" void __pfx_assume(bool, const char *);
#else
void __pfx_assume(int, const char *);
#endif
#else
#define __pfx_assume(X, Y)
#endif


#if _WIN64
#define INTERLOCKED_EXCHANGE(TARGET, VALUE) \
    InterlockedExchangePointer((PVOID *)(TARGET), (PVOID)(VALUE))
#else // _WIN64
#define INTERLOCKED_EXCHANGE(TARGET, VALUE) \
    InterlockedExchange((LPLONG)(TARGET), (LONG)(VALUE))
#endif // _WIN64

/***************************************************************************
Macros for common code sequences.
***************************************************************************/

// === IUnknown ===
#define DECLARE_IUNKNOWN() \
public: \
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv); \
    STDMETHOD_(ULONG, AddRef)(void); \
    STDMETHOD_(ULONG, Release)(void);

#define DEFINE_IUNKNOWN(ComClass, refcount) \
    STDMETHODIMP_(ULONG) ComClass::AddRef(void) \
{ \
    return InterlockedIncrement((long *)&refcount); \
} \
    \
    STDMETHODIMP_(ULONG) ComClass::Release(void) \
{ \
    ULONG cref = (ULONG)InterlockedDecrement((long *)&refcount); \
    Assert(refcount >= 0); \
    if (0 == cref) \
{ \
    /* Artificial ref to prevent reenterancy. */ \
    cref = 1; \
    delete this; \
} \
    return cref; \
}

/***************************************************************************
Floating point inline functions.
***************************************************************************/
inline long LwFromDbl(double dbl)
{
    // NaN and Infinity should be converted to 0.
#if _M_PPC || _M_MRX000
    if (!Js::NumberUtilities::IsFinite(dbl))
        return 0;
    dbl = fmod(dbl, 4294967296.0);
    if (dbl < 0)
        return -(long)(ulong)-dbl;
    return (long)(ulong)dbl;
#elif defined(_M_ARM32_OR_ARM64)
    if (!Js::NumberUtilities::IsFinite(dbl))
        return 0;
    // For Intel int64 overflow is always 0x80000000`00000000 -- for which we return 0 
    // (cast to int32 simply takes out non-fitting bits).
    // For ARM negative overflow is same, positive is 7FFFFFFF`FFFFFFFF.
    // Be consistent with Intel.
    __int64 i64 = (__int64)dbl;
    if (i64 == 0x7FFFFFFFFFFFFFFF) return 0;
    return (long)(i64);
#elif _M_AMD64
    Assert(Js::NumberUtilities::IsFinite(dbl) || 0 == (long)((__int64)dbl));
    return (long)((__int64)dbl);    // convert to 64-bit int before going to long to cure conversion issues.
#else // _M_PPC || _M_MRX000
    // AND with 0xFFFFFFFF to prevent -RTCc failures.
    Assert(Js::NumberUtilities::IsFinite(dbl) || 0 == (long)(((__int64)dbl) & 0xFFFFFFFF));
    return (long)(((__int64)dbl)  & 0xFFFFFFFF);
#endif // _M_PPC || _M_MRX000
}

/***************************************************************************
Parameter validation
***************************************************************************/

#define VALIDATE_WRITE_POINTER(pv, type) \
    if ((pv) == NULL) \
{ AssertMsg(FALSE, "NULL pointer"); return HR(E_POINTER); } \
    else \
    AssertMem((type *)(pv))

#define VALIDATE_POINTER(pv, type) \
    if ((pv) == NULL) \
{ AssertMsg(FALSE, "NULL pointer"); return HR(E_POINTER); } \
    else \
    AssertMemR((type *)(pv))

#define VALIDATE_INTERFACE_POINTER(pv, type) \
    if ((pv) == NULL) \
{ AssertMsg(FALSE, "NULL interface pointer"); return HR(E_POINTER); } \
    else \
    AssertMemR((type *)(pv))

#define VALIDATE_STRING(psz) \
    if ((psz) == NULL) \
{ AssertMsg(FALSE, "NULL string"); return HR(E_POINTER); } \
    else \
    AssertPsz(psz)


/***************************************************************************
Name mapping of crt functions
***************************************************************************/
#define ostrlen wcslen
#define ostrdup _wcsdup
#define ostrcmp wcscmp
#define ostricmp _wcsicmp
#define oltoa _ltow_s
#define ostrchr wcschr

/***************************************************************************
Debug output
***************************************************************************/

#if DEBUG
#define DebugPrintf(arg) dprintf##arg
void dprintf(const WCHAR *fmt, ...);
void vdprintf(const WCHAR *fmt, va_list argptr);
#else //!DEBUG
#define DebugPrintf(arg)
#endif //!DEBUG

/***************************************************************************
Misc utility objects
***************************************************************************/

// GL is a general list. For Hungarian gl is used as a prefix. Eg, a GL
// containing elements of type FOO should be referred to by pglfoo.
//
// (BUG 1202470, 1202471 - Windows OS Bugs)
// Consider replacing this class with a typesafe (template) dynamic array class.
class GL sealed
{
private:
    long m_refCount;        // ref count
    long m_entrySize;       // size of each entry
    BYTE* m_byteBuffer;     // data
    long m_byteBufferSize;  // size of m_byteBuffer block
    long m_entryCount;      // number of elements

    virtual ~GL(void)
    {
        if (NULL != m_byteBuffer)
        {
            free(m_byteBuffer);
        }
    }

    BOOL EnsureSize(long cb);

public:
    GL(long cb)
    {
        Assert(cb > 0);
        m_refCount = 1;
        m_entrySize = cb;
        m_byteBuffer = NULL;
        m_byteBufferSize = 0;
        m_entryCount = 0;
    }

    void AddRef(void)
    { m_refCount++; }

    void Release(void)
    { if (--m_refCount <= 0) delete this; }

    long Cv(void)
    { return m_entryCount; }

    BOOL FSetCv(long cv);
    BOOL FEnsureSpace(long cvAdd);
    BOOL FInsert(long iv, void *pv = NULL, long cv = 1);
    void Delete(long iv, long cv = 1);

    BOOL FAdd(void *pv = NULL, long *piv = NULL)
    {
        if (NULL != piv)
            *piv = m_entryCount;
        return FInsert(m_entryCount, pv);
    }

    void Get(long iv, /*__out_bcount(this->m_entrySize)*/ void *pv)
    {
        // (BUG 1202468, 1202470, 1202471, 1202473, 1202474 - Windows OS Bugs)
        __pfx_assume(m_entrySize > 0, "m_entrySize always initialized to >0 in constructor");
        Assert(iv < m_entryCount);
        js_memcpy_s(pv, m_entrySize, m_byteBuffer + iv * m_entrySize, m_entrySize);
    }

    void Put(long iv, /*__in_bcount(this->m_entrySize)*/ void *pv)
    {
        // (BUG 1202468, 1202470, 1202471, 1202473, 1202474 - Windows OS Bugs)
        __pfx_assume(m_entrySize > 0, "m_entrySize always initialized to >0 in constructor");
        Assert(iv < m_entryCount);
        js_memcpy_s(m_byteBuffer + iv * m_entrySize, m_byteBufferSize, pv, m_entrySize);
    }

    void *PvGet(long iv)
    {
        Assert(iv <= m_entryCount);
        return m_byteBuffer + iv * m_entrySize;
    }

    BOOL FPush(void *pv)
    { return FInsert(m_entryCount, pv); }

    BOOL FPop(void *pv = NULL);
    BOOL FEnqueue(void *pv)
    { return FInsert(0, pv); }

    BOOL FDequeue(void *pv = NULL)
    { return FPop(pv); }
};


/***************************************************************************
critical section wrapper
***************************************************************************/

class MUTX
{
protected:
    CRITICAL_SECTION m_cs;
    bool m_fErr;
    bool m_fInit;

public:
    MUTX(void) 
    {
        m_fErr  = false;
        m_fInit = false;
        __try
        {
            InitializeCriticalSection(&m_cs); 
            m_fInit = true;
        }
        __except(1)
        {
            m_fErr = true;
        }
        Assert(m_fInit && !m_fErr);
    }

    ~MUTX(void) 
    { 
        if (m_fInit)
            DeleteCriticalSection(&m_cs); 
    }

    BOOL Enter(void) 
    { 
        Assert(m_fInit && !m_fErr);
        if (m_fErr)
            return false;
        __try
        {
            EnterCriticalSection(&m_cs); 
        }
        __except(1)
        {
            Assert(FALSE);
            m_fErr = true;
        }
        return !m_fErr;
    }

    void Leave(void) 
    {
        Assert(m_fInit && !m_fErr);
        if (m_fErr)
            return;
        __try
        {
            LeaveCriticalSection(&m_cs); 
        }
        __except(1)
        {
            Assert(FALSE);
            m_fErr = true;
        }
    }

    BOOL IsValid(void)
    { 
        return m_fInit && !m_fErr; 
    }
};


/***************************************************************************
Class to build strings of unknown length
***************************************************************************/

class BuildString
{
private:
    OLECHAR *m_psz;
    long m_cchAlloc;
    long m_cch;
    BOOL m_fError;

    BOOL FEnsureSpace(long cch);

public:
    BuildString(void);
    ~BuildString(void);

    void Reset(void);
    OLECHAR *PszReset(void);
    BSTR BstrReset(void);    
    LPCOLESTR PszCur(void)
    {
        if (NULL == m_psz)
            return OLESTR("");
        return m_psz;
    }
    long CchCur(void)
    { return m_cch; }

    void RestoreAt(long cch)
    {
        m_cch = cch;
        m_psz[m_cch] = 0;
    }

    HRESULT AppendSz(LPCOLESTR psz, long cch = -1);
    HRESULT AppendCh(OLECHAR ch)
    {
        if ((m_cch < 0) || (m_cch + 1 < 0) || ((m_cch + 1 >= m_cchAlloc) && !FEnsureSpace(m_cch + 1)))
            return HR(E_OUTOFMEMORY);
        m_psz[m_cch++] = ch;
        m_psz[m_cch] = 0;
        return NOERROR;
    }
    BOOL FError(void)
    { return m_fError; }
};
