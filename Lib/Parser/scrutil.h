//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include <errno.h>

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


// === IDispatch ===
#define DECLARE_IDISPATCH() \
public: \
    STDMETHOD(GetTypeInfoCount)(UINT *pcti); \
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **ppti); \
    STDMETHOD(GetIDsOfNames)(REFIID riid, __in_ecount(cpsz) OLECHAR **prgpsz, UINT cpsz, \
    LCID lcid, DISPID *prgid); \
    STDMETHOD(Invoke)(DISPID id, REFIID riid, LCID lcid, WORD wFlags, \
    DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, UINT *puArgErr);

// === IDispatchEx ===
#define DECLARE_IDISPATCHEX() \
public: \
    STDMETHOD(GetDispID)(BSTR bstrName, DWORD grfdex, DISPID *pdispid); \
    STDMETHOD(InvokeEx)(DISPID dispid, LCID lcid, WORD wFlags, \
    DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, \
    IServiceProvider *pspCaller); \
    STDMETHOD(DeleteMemberByName)(BSTR bstrName, DWORD grfdex); \
    STDMETHOD(DeleteMemberByDispID)(DISPID dispid); \
    STDMETHOD(GetMemberProperties)(DISPID dispid, DWORD grfdexFetch, \
    DWORD *pgrfdex); \
    STDMETHOD(GetMemberName)(DISPID dispid, BSTR *pbstrName); \
    STDMETHOD(GetNextDispID)(DWORD grfdex, DISPID dispid, DISPID *pdispid); \
    STDMETHOD(GetNameSpaceParent)(IUnknown **ppunk);


#if _WIN64
typedef unsigned __int64 uint64;
#endif // _WIN64

// tristate values
enum
{
    triNo = 0,
    triYes = 1,
    triMaybe = 2
};


/***************************************************************************
Alignment
***************************************************************************/
#if _WIN64
struct __ALIGN_FOO__ {
    int w1;
    double dbl;
};
#define ALIGN_FULL (offsetof(__ALIGN_FOO__, dbl))
#else 
// Force check for 4 byte alignment to support Win98/ME
#define ALIGN_FULL 4
#endif // _WIN64

#define AlignIt(VALUE, TYPE) (~(~((LONG_PTR)(VALUE) + (sizeof(TYPE)-1)) | (sizeof(TYPE)-1)))
#define FAligned(VALUE, TYPE) (((VALUE) & (sizeof(TYPE)-1)) == 0)

#define AlignFull(VALUE) (~(~((VALUE) + (ALIGN_FULL-1)) | (ALIGN_FULL-1)))
#define FAlignedFull(cb) (((cb) & (ALIGN_FULL-1)) == 0)

/***************************************************************************
Misc macros and inline functions
***************************************************************************/
#define CastTo(pv,typ) (*(UNALIGNED typ *)&(pv))
#define MakePtr(pv,ib,typ) ((typ)((byte *)(pv) + (ib)))

inline long LwMin(long lw1, long lw2)
{ return lw1 < lw2 ? lw1 : lw2; }
inline long LwMax(long lw1, long lw2)
{ return lw1 < lw2 ? lw2 : lw1; }

inline int WMin(int w1, int w2)
{ return w1 < w2 ? w1 : w2; }
inline int WMax(int w1, int w2)
{ return w1 < w2 ? w2 : w1; }

#define LCID_US_ENGLISH ((LCID)MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US))


/***************************************************************************
Type library functions
***************************************************************************/
//HRESULT GetStdOleTypeLib(ITypeLib **pptlib);
//HRESULT GetDispatchTypeInfo(ITypeInfo **ppti);


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
inline long LwFromDblNearest(double dbl)
{
    if (Js::NumberUtilities::IsNan(dbl))
        return 0;
    if (dbl > 0x7FFFFFFFL)
        return 0x7FFFFFFFL;
    if (dbl < (long)0x80000000L)
        return (long)0x80000000L;
    return (long)dbl;
}
inline ulong LuFromDblNearest(double dbl)
{
    if (Js::NumberUtilities::IsNan(dbl))
        return 0;
    if (dbl > (ulong)0xFFFFFFFFUL)
        return (ulong)0xFFFFFFFFUL;
    if (dbl < 0)
        return 0;
    return (ulong)dbl;
}

inline BOOL FDblIsLong(double dbl, long *plw)
{
    AssertMem(plw);
    double dblT;

    *plw = (long)dbl;
    dblT = (double)*plw;
    return Js::NumberUtilities::LuHiDbl(dblT) == Js::NumberUtilities::LuHiDbl(dbl) && Js::NumberUtilities::LuLoDbl(dblT) == Js::NumberUtilities::LuLoDbl(dbl);
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
#define ATOI _wtoi
#define ITOA _itow
#define ISSPACE   iswspace
#define ISALNUM   iswalnum
typedef OLECHAR   UOLECHAR;

#define ostrlen wcslen
#define ostrcpy wcscpy
#define ostrcat wcscat
#define otolower towlower
#define ostrtol wcstol
#define oasctime _wasctime
#define ostrftime wcsftime
#define osprintf swprintf
#define ostrdup _wcsdup
#define ostrcmp wcscmp
#define ostricmp _wcsicmp
#define ostrnicmp _wcsnicmp
#define ostrncmp wcsncmp
#define ostrchr wcschr
#define ostrcspn wcscspn
#define osnprintf _snwprintf
#define ovsnprintf _vsnwprintf
#define ostrncpy wcsncpy
#define ostrupr _wcsupr
#define ostrlwr _wcslwr
#define oltoa _ltow_s
#define oultoa _ultow
#define ostrstr wcsstr


// synonyms
#define WIDE OLESTR
#define STRICMP ostricmp

#define ostrcmpCase ostrcmp

inline BOOL FHexDigit(UOLECHAR ch, int *pw)
{
    if ((ch -= '0') <= 9)
    {
        *pw = ch;
        return TRUE;
    }
    if ((ch -= 'A' - '0') <= 5 || (ch -= 'a' - 'A') <= 5)
    {
        *pw = 10 + ch;
        return TRUE;
    }
    return FALSE;
}

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


/***************************************************************************
Misc BSTR functions. The parameters are LPCOLESTR instead of BSTR so
we can pass const things.
***************************************************************************/
inline long CbBstr(LPCOLESTR bstr)
{
    if (bstr == NULL)
        return 0;
#if BSTR64
    ULONG_PTR len = ((ULONG_PTR *)bstr)[-1];
    Assert(len <= LONG_MAX);
#else // BSTR64
    ULONG len = ((ULONG *)bstr)[-1];
#endif // BSTR64
    return (long)len;
}

inline long CchRawBstr(LPCOLESTR bstr)
{
    if (bstr == NULL)
        return 0;
#if BSTR64
    ULONG_PTR len = ((ULONG_PTR *)bstr)[-1] / sizeof(OLECHAR);
    Assert(len <= LONG_MAX);
#else // BSTR64
    ULONG len = ((ULONG *)bstr)[-1] / sizeof(OLECHAR);
#endif // BSTR64
    return (long)len;
}


/***************************************************************************
Creates a static constant bstr.
***************************************************************************/
#if BSTR64
#define StaticBstr(name, str) \
    struct StaticBstr_##name { \
        ULONG_PTR cb; \
        OLECHAR sz[sizeof(str)]; \
    }; \
    cosnt StaticBstr_##name name = { \
    sizeof(OLESTR(str)) - sizeof(OLECHAR), \
    OLESTR(str) \
};
#else // BSTR64
#define StaticBstr(name, str) \
    struct StaticBstr_##name { \
        ULONG cb; \
        OLECHAR sz[sizeof(str)]; \
    }; \
    const StaticBstr_##name name = { \
    sizeof(OLESTR(str)) - sizeof(OLECHAR), \
    OLESTR(str) \
};
#endif // BSTR64


//
// Floating point unit utility functions
//


inline errno_t GetFPUControl(unsigned int *pctrl)
{
    if (pctrl == NULL)
    {
        SetLastError(EINVAL);
        return EINVAL;
    }
#if _M_IX86
    *pctrl = _control87(0,0);
    return 0;
#else
    return _controlfp_s(pctrl,0,0);
#endif
}

inline errno_t SetFPUControl(unsigned int fpctrl)
{
#if _M_IX86
    _control87(fpctrl, (unsigned int)(-1));
    return 0;
#else
    return _controlfp_s(0, fpctrl, (unsigned int)(-1));
#endif
}

inline errno_t SetFPUControlDefault(void)
{
#if _M_AMD64 || _M_IA64
    return _controlfp_s(0, _RC_NEAR + _DN_SAVE + _EM_INVALID + _EM_ZERODIVIDE +
        _EM_OVERFLOW + _EM_UNDERFLOW + _EM_INEXACT,
        _MCW_EM | _MCW_DN | _MCW_PC | _MCW_RC | _MCW_IC);
#elif _M_IX86
    _control87(_CW_DEFAULT, _MCW_EM | _MCW_DN | _MCW_PC | _MCW_RC | _MCW_IC);
    return 0;
#else
    return _controlfp_s(0, _CW_DEFAULT, _MCW_EM | _MCW_DN | _MCW_PC | _MCW_RC | _MCW_IC);
#endif
}


inline unsigned int GetFPUStatus(void)
{
    return _statusfp();
}

inline void ClearFPUStatus(void)
{
    // WinSE 187789 
    // _clearfp gives up the thread's time slice, so clear only if flags are set
    if (_statusfp())
        _clearfp();
}

// This class basically saves the FPU control word, and sets it to the default value
// The default value will prevent all floating point exceptions other than denormal operand
// from being generated
// When the instance goes out of scope, the control word will be restored to the original value
class SmartFPUControl
{
    static const uint INVALID_FPUCONTROL = (uint)-1;

public:

    SmartFPUControl(bool isRestoreFPUToDefaultValue) :
        restoreFPUToDefaultValue(isRestoreFPUToDefaultValue),
        m_oldFpuControl(INVALID_FPUCONTROL)
    {
        ClearFPUStatus(); // Clear pending exception status first (blue 555235)

        m_err = GetFPUControl(&m_oldFpuControl);
        if (m_err == 0)
        {
            if (restoreFPUToDefaultValue)
            {
                m_err = SetFPUControlDefault();
            }
#if DBG
            else
            {
                m_oldFpuControlForConsistencyCheck = m_oldFpuControl;
            }
#endif
        }
    }

    SmartFPUControl():
        SmartFPUControl(true)
    {
    }

    ~SmartFPUControl()
    {
        // If restoreFpuControlValue = false, then caller didn't opt for restoring FPU control to default value
        // So don't restore the FPU control automatically. If needed caller should call RestoreFPUControl.
        if (restoreFPUToDefaultValue)
        {
            RestoreFPUControl();
        }
#if DBG
        else
        {
            uint currentFpuControl;
            m_err = GetFPUControl(&currentFpuControl);
            if (m_err == 0 && m_oldFpuControlForConsistencyCheck != INVALID_FPUCONTROL)
            {
                Assert(m_oldFpuControlForConsistencyCheck == currentFpuControl);
            }
        }
#endif
    }

    bool HasErr() const
    {
        return m_err != 0;
    }

    HRESULT GetErr() const
    {
        Assert(HasErr());
        return HRESULT_FROM_WIN32(m_err);
    }

    void RestoreFPUControl()
    {
        if (m_oldFpuControl != INVALID_FPUCONTROL)
        {
            m_err = SetFPUControl(m_oldFpuControl);
            m_oldFpuControl = INVALID_FPUCONTROL; // Only restore once
        }
    }

private:
#if DBG
    uint m_oldFpuControlForConsistencyCheck = INVALID_FPUCONTROL;
#endif
    uint m_oldFpuControl;
    errno_t m_err;
    bool restoreFPUToDefaultValue = true;
};

inline LCID GetDefaultLocale(void)
{
    LCID lcid;
    lcid = GetUserDefaultLCID();

    //Win8 858821: Remove the following assertion. IsValidLocale fails at system shutdown because registry is not available.
    //Assert(IsValidLocale(lcid, LCID_INSTALLED));

    return lcid;
}



// Get the hash value for a compiler generated string - stored in the
// ulong immediately preceeding the bstr.
inline ulong GetHash(LPCOLESTR psz)
{
#if BSTR64
    ULONG_PTR hash = ((ULONG_PTR *)psz)[-2];
#if _WIN64
    return ((ulong *)&hash)[1];
#else // _WIN64
    return hash;
#endif // _WIN64
#else // BSTR64
    return ((ulong *)psz)[-2];
#endif // BSTR64
}

ULONG CaseInsensitiveComputeHash(LPCOLESTR posz);
ULONG CaseInsensitiveComputeHashCch(LPCOLESTR prgch, long cch);
ULONG CaseInsensitiveComputeHashCch(LPCUTF8 prgch, long cch);

// ComputeHash and ComputeHashCch are the default hash functions.
// JScript uses a case insensitive hash value so we can do either type of
// lookup. See IDispatchEx::GetDispID.
#define ComputeHash CaseInsensitiveComputeHash
#define ComputeHashCch CaseInsensitiveComputeHashCch




template<typename EncodedChar>
double DblFromHex(const EncodedChar *psz, const EncodedChar **ppchLim);
template <typename EncodedChar>
double DblFromBinary(const EncodedChar *psz, const EncodedChar **ppchLim);
template<typename EncodedChar>
double DblFromOctal(const EncodedChar *psz, const EncodedChar **ppchLim);
template<typename EncodedChar>
double StrToDbl(const EncodedChar *psz, const EncodedChar **ppchLim, Js::ScriptContext *const scriptContext);


/***************************************************************************
Binding option flags
***************************************************************************/
enum
{
    fbindNil          = 0x0000,
    fbindReadOnly     = 0x0001,
    fbindErrorOnWrite = 0x0002,
    fbindInternal     = 0x0004, // Don't expose through IDispatchEx.
    fbindNoEnum       = 0x0008,
    fbindDontDelete   = 0x0010,
    fbindLim          = 0x0020

};

const ulong kgrfbindProtected = fbindReadOnly | fbindErrorOnWrite;
const ulong kgrfbindAll       = fbindLim - 1;

enum
{
    fscrNil                             = 0,
    fscrHtmlComments                    = 1 << 0,   // throw away html style comments
    fscrReturnExpression                = 1 << 1,   // call should return the last expression
    fscrImplicitThis                    = 1 << 2,   // 'this.' is optional (for Call)
    fscrImplicitParents                 = 1 << 3,   // the parents of 'this' are implicit
    fscrMapQuote                        = 1 << 4,   // map single quote to double quote
    fscrDynamicCode                     = 1 << 5,   // The code is being generated dynamically (eval, new Function, etc.)
    fscrSyntaxColor                     = 1 << 6,   // used by the scanner for syntax coloring
    fscrStmtCompletion                  = 1 << 7,   // parse for statement completion
    fscrNoImplicitHandlers              = 1 << 8,   // same as Opt NoConnect at start of block

    // hack to prevent a copy needed to strip off trailing html comments
    // - modifies the behavior of fscrHtmlComments
    fscrDoNotHandleTrailingHtmlComments = 1 << 9, 

#if DEBUG
    fscrEnforceJSON                     = 1 << 10,  // used together with fscrReturnExpression 
    // enforces JSON semantics in the parsing.
#endif

    fscrEval                            = 1 << 11,  // this expression has eval semantics (i.e., run in caller's context 
    fscrEvalCode                        = 1 << 12,  // this is an eval expression
    fscrGlobalCode                      = 1 << 13,  // this is a global script
    fscrDeferFncParse                   = 1 << 14,  // parser: defer creation of AST's for non-global code
    fscrDeferredFncExpression           = 1 << 15,  // the function decl node we deferred is an expression,
                                                    // i.e., not a declaration statement
    fscrDeferredFnc                     = 1 << 16,  // the function we are parsing is deferred
    fscrNoPreJit                        = 1 << 17,  // ignore prejit global flag
#if ERROR_RECOVERY
    fscrFunctionHeaderOnly              = 1 << 18,  // parse only the function header, not the body of the function
#endif
    fscrAllowFunctionProxy              = 1 << 19,  // Allow creation of function proxies instead of function bodies
    fscrIsLibraryCode                   = 1 << 20,  // Current code is engine library code written in Javascript
    fscrNoDeferParse                    = 1 << 21,  // Do not defer parsing
    fscrIsNativeCode                    = 1 << 22,  // We are either serializing or deserializing native code
#ifdef IR_VIEWER
    fscrIrDumpEnable                    = 1 << 23,  // Allow parseIR to generate an IR dump
#endif /* IRVIEWER */

    // Throw a ReferenceError when the global 'this' is used (possibly in a lambda),
    // for debugger when broken in a lambda that doesn't capture 'this'
    fscrDebuggerErrorOnGlobalThis       = 1 << 24,
    
    fscrDeferredClassMemberFnc          = 1 << 25,

    fscrAll                             = (1 << 26) - 1
};
