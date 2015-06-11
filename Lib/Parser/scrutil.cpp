//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include <strsafe.h>

HRESULT GetStdOleTypeLib(ITypeLib **pptlib)
{
    AssertMem(pptlib);
    ITypeLib * ptlib;

    ptlib = NULL;
    *pptlib = NULL;

    HRESULT hr;
    // Try to load the v2 StdOLE type library.
    hr= LoadRegTypeLib(IID_StdOle, STDOLE2_MAJORVERNUM, STDOLE2_MINORVERNUM,
        STDOLE2_LCID, &ptlib);

    if (FAILED(hr))
        return hr;

    if (NULL == ptlib)
        return HR(E_FAIL);

    *pptlib = ptlib;
    return NOERROR;
}

HRESULT GetDispatchTypeInfo(ITypeInfo **ppti)
{
    AssertMem(ppti);
    *ppti = NULL;

    HRESULT hr;
    ITypeLib *ptlibStdOle;
    if (FAILED(hr = GetStdOleTypeLib(&ptlibStdOle)))
        return hr;
    hr = ptlibStdOle->GetTypeInfoOfGuid(IID_IDispatch, ppti);
    ptlibStdOle->Release();
    return hr;
}

/***************************************************************************
General list implementation
***************************************************************************/
const long kcbMax = 0x01000000;
#if DEBUG
#define TRASH(pv, cb) memset((pv), 0xA3, (cb))
#else
#define TRASH(pv, cb)
#endif

BOOL GL::EnsureSize(long cb)
{
    if ((cb < 0) || (cb >= kcbMax))
    {
        Assert(FALSE);
        return FALSE;
    }

    if (cb <= m_byteBufferSize)
        return TRUE;

    if (NULL == m_byteBuffer)
    {
        if (NULL == (m_byteBuffer = (BYTE *)malloc(cb)))
            return FALSE;
        m_byteBufferSize = cb;
    }
    else
    {
        BYTE *prgb;
        long totalBytes;

        if (NULL == (prgb = (BYTE *)realloc(m_byteBuffer, totalBytes = 2 * cb)) &&
            NULL == (prgb = (BYTE *)realloc(m_byteBuffer, totalBytes = cb)))
        {
            return FALSE;
        }
        m_byteBuffer = prgb;
        m_byteBufferSize = totalBytes;
    }

    return TRUE;
}


BOOL GL::FSetCv(long cv)
{
    Assert(cv >= 0 && cv < kcbMax);
    long cb;

    if (cv > m_entryCount)
    {
        if (cv >= kcbMax / m_entrySize)
        {
            AssertMsg(FALSE, "who's trying to allocate a list this big?");
            return FALSE;
        }
        cb = cv * m_entrySize;
        if (cb > m_byteBufferSize && !EnsureSize(cb))
            return FALSE;
        TRASH(m_byteBuffer + m_entryCount * m_entrySize, (cv - m_entryCount) * m_entrySize);
    }
    else
        TRASH(m_byteBuffer + cv * m_entrySize, (m_entryCount - cv) * m_entrySize);
    m_entryCount = cv;

    return TRUE;
}


BOOL GL::FEnsureSpace(long cvAdd)
{
    Assert(cvAdd >= 0 && cvAdd < kcbMax);
    return EnsureSize((m_entryCount + cvAdd) * m_entrySize);
}


BOOL GL::FInsert(long iv, void *pv, long cv)
{
    Assert(iv >= 0 && iv <= m_entryCount);
    Assert(cv >= 0 && cv < kcbMax);

    BYTE *pb;
    long totalBytes, cbIns, ibIns;

    if (cv <= 0)
        return TRUE;

    // (BUG 1247104 - Windows OS Bugs)
    // We used to open an uninitialized slot for a NULL pv, causing 
    // either double release/delete or crash/security hole.
    Assert(NULL != pv);
    if (NULL == pv)
        return FALSE;

    totalBytes = (m_entryCount + cv) * m_entrySize;
    cbIns = cv * m_entrySize;
    ibIns = iv * m_entrySize;

    if (totalBytes > m_byteBufferSize && !EnsureSize(totalBytes))
        return FALSE;

    pb = m_byteBuffer + ibIns;
    if (iv < m_entryCount)
        memmove(pb + cbIns, pb, totalBytes - cbIns - ibIns);

    js_memcpy_s(pb, m_byteBufferSize - ibIns, pv, cbIns);
    m_entryCount += cv;
    return TRUE;
}


void GL::Delete(long iv, long cv)
{
    Assert(iv >= 0 && iv <= m_entryCount);
    Assert(cv >= 0 && cv <= m_entryCount - iv);

    if (cv <= 0)
        return;

    if (iv < (m_entryCount -= cv))
    {
        BYTE *pb = m_byteBuffer + iv * m_entrySize;
        memmove(pb, pb + cv * m_entrySize, (m_entryCount - iv) * m_entrySize);
    }
    TRASH(m_byteBuffer + m_entryCount * m_entrySize, cv * m_entrySize);
}


BOOL GL::FPop(void *pv)
{
    if (m_entryCount <= 0)
        return FALSE;
    if (NULL != pv)
        Get(m_entryCount - 1, pv);
    m_entryCount--;
    TRASH(m_byteBuffer + m_entryCount * m_entrySize, m_entrySize);
    return TRUE;
}


/***************************************************************************
BuildString implementation
***************************************************************************/
BuildString::BuildString(void)
{
    m_psz = NULL;
    m_cchAlloc = 0;
    m_cch = 0;
    m_fError = FALSE;
}

BuildString::~BuildString(void)
{
    Reset();
}

void BuildString::Reset(void)
{
    if (NULL != m_psz)
        free(m_psz);
    m_psz = NULL;
    m_cchAlloc = 0;
    m_cch = 0;
    m_fError = FALSE;
}


OLECHAR *BuildString::PszReset(void)
{
    OLECHAR *psz;

    if (m_fError)
        psz = NULL;
    else if (NULL == m_psz)
        psz = ostrdup(OLESTR(""));
    else
    {
        psz = m_psz;
        m_psz = NULL;
    }
    Reset();
    return psz;
}

BSTR BuildString::BstrReset(void)
{
    BSTR bstr;

    if (m_fError)
        bstr = NULL;
    else
        bstr = SysAllocStringLen(m_psz, (uint)m_cch);
    Reset();
    return bstr;
}

BOOL BuildString::FEnsureSpace(long cch)
{
    OLECHAR *pszT;
    long cchNew;

    if (++cch <= m_cchAlloc)
        return TRUE;
    if (m_fError)
        return FALSE;
    if ((cchNew = cch * 2) < cch)
    {    
        m_fError = TRUE;
        return FALSE;
    }
    if (cchNew < 100)
        cchNew = 100;

    //BUG 1117303 Windows OS Bug
    //'cchNew*sizeof(OLECHAR)' may be smaller than its operands.
    if (LONG_MAX/sizeof(OLECHAR) < cchNew || LONG_MAX/sizeof(OLECHAR) < cch ) 
    {    
        m_fError = TRUE;
        return FALSE;
    }

    if (NULL == (pszT = (OLECHAR *) realloc(m_psz, cchNew * sizeof(OLECHAR))) &&
        NULL == (pszT = (OLECHAR *) realloc(m_psz,
        (cchNew = cch) * sizeof(OLECHAR))))
    {
        m_fError = TRUE;
        return FALSE;
    }
    m_psz = pszT;
    m_cchAlloc = cchNew;

    return TRUE;
}


HRESULT BuildString::AppendSz(LPCOLESTR psz, long cch)
{
    AssertPszN(psz);

    if (cch < 0)
        cch = (NULL == psz) ? 0 : (long)ostrlen(psz);

    AssertArrMemR(psz, cch);

    if ((m_cch + cch < m_cch) || (m_cch + cch >= m_cchAlloc && !FEnsureSpace(m_cch + cch)))
        return HR(E_OUTOFMEMORY);

    js_memcpy_s(m_psz + m_cch, (m_cchAlloc - m_cch) * sizeof(OLECHAR), psz, cch * sizeof(OLECHAR));
    m_cch += cch;
    if (m_psz) m_psz[m_cch] = 0;

    return NOERROR;
}

/***************************************************************************
debug printf 
***************************************************************************/
#if DEBUG
void vdprintf(const WCHAR *pszFmt, va_list argptr)
{
    WCHAR szT[513];
    StringCchVPrintf(szT, sizeof(szT)/sizeof(WCHAR), pszFmt, argptr);
    szT[sizeof(szT)/sizeof(WCHAR) - 1] = '\0';
    OutputDebugString(szT);
}

void dprintf(const WCHAR *fmt, ...)
{
    va_list marker;
    va_start(marker, fmt);
    vdprintf(fmt, marker);
}
#endif

template<typename EncodedChar>
double DblFromHex(const EncodedChar *psz, const EncodedChar **ppchLim)
{
    double dbl;
    uint uT;
    byte bExtra;
    int cbit;

    // Skip leading zeros.
    while (*psz == '0')
        psz++;

    dbl = 0;
    Assert(Js::NumberUtilities::LuHiDbl(dbl) == 0);
    Assert(Js::NumberUtilities::LuLoDbl(dbl) == 0);

    // Get the first digit.
    if ((uT = *psz - '0') > 9)
    {
        if ((uT -= 'A' - '0') <= 5 || (uT -= 'a' - 'A') <= 5)
            uT += 10;
        else
        {
            *ppchLim = psz;
            return dbl;
        }
    }
    psz++;

    if (uT & 0x08)
    {
        cbit = 4;
        Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)(uT & 0x07) << 17;
    }
    else if (uT & 0x04)
    {
        cbit = 3;
        Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)(uT & 0x03) << 18;
    }
    else if (uT & 0x02)
    {
        cbit = 2;
        Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)(uT & 0x01) << 19;
    }
    else
    {
        Assert(uT & 0x01);
        cbit = 1;
    }
    bExtra = 0;

    for ( ; ; psz++)
    {
        if ((uT = (*psz - '0')) > 9)
        {
            if ((uT -= 'A' - '0') <= 5 || (uT -= 'a' - 'A') <= 5)
                uT += 10;
            else
                break;
        }

        if (cbit <= 17)
            Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)uT << (17 - cbit);
        else if (cbit < 21)
        {
            Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)uT >> (cbit - 17);
            Js::NumberUtilities::LuLoDbl(dbl) |= (ulong)uT << (49 - cbit);
        }
        else if (cbit <= 49)
            Js::NumberUtilities::LuLoDbl(dbl) |= (ulong)uT << (49 - cbit);
        else if (cbit <= 53)
        {
            Js::NumberUtilities::LuLoDbl(dbl) |= (ulong)uT >> (cbit - 49);
            bExtra = (byte) (uT << (57 - cbit));
        }
        else if (0 != uT)
            bExtra |= 1;
        cbit += 4;
    }

    // Set the lim.
    *ppchLim = psz;

    // Set the exponent.
    cbit += 1022;
    if (cbit > 2046)
    {
        // Overflow to Infinity
        Js::NumberUtilities::LuHiDbl(dbl) = 0x7FF00000;
        Js::NumberUtilities::LuLoDbl(dbl) = 0;
        return dbl;
    }
    Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)cbit << 20;

    // Use bExtra to round.
    if ((bExtra & 0x80) && ((bExtra & 0x7F) || (Js::NumberUtilities::LuLoDbl(dbl) & 1)))
    {
        // Round up. Note that this overflows the mantissa correctly,
        // even to Infinity.
        if (0 == ++Js::NumberUtilities::LuLoDbl(dbl))
            ++Js::NumberUtilities::LuHiDbl(dbl);
    }

    return dbl;
}

template <typename EncodedChar>
double DblFromBinary(const EncodedChar *psz, const EncodedChar **ppchLim)
{
    double dbl = 0;
    Assert(Js::NumberUtilities::LuHiDbl(dbl) == 0);
    Assert(Js::NumberUtilities::LuLoDbl(dbl) == 0);
    uint uT;
    byte bExtra = 0;
    int cbit = 0;
    // Skip leading zeros.
    while (*psz == '0')
        psz++;
    // Get the first digit.
    uT = *psz - '0';
    if (uT > 1)
    {
        *ppchLim = psz;
        return dbl;
    }
    //Now that leading zeros are skipped first bit should be one so lets 
    //go ahead and count it and increment psz
    cbit = 1;
    psz++;

    // According to the existing implementations these numbers 
    // should n bits away from 21 and 53. The n bits are determined by the
    // numerical type. for example since 4 bits are necessary to represent a
    // hexadecimal number and 3 bits to represent an octal you will see that 
    // the hex case is represented by 21-4 = 17 and the octal case is represented
    // by 21-3 = 18, thus for binary where 1 bit is need to represent 2 numbers 21-1 = 20
    const uint rightShiftValue = 20;
    // Why 52? 52 is the last explicit bit and 1 bit away from 53 (max bits of precision
    // for double precision floating point)
    const uint leftShiftValue  = 52;
    for (; (uT = (*psz - '0')) <= 1; psz++)
    {
        if (cbit <= rightShiftValue)
        {
            Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)uT << (rightShiftValue - cbit);
           
        }
        else if (cbit <= leftShiftValue)
        {
            Js::NumberUtilities::LuLoDbl(dbl) |= (ulong)uT << (leftShiftValue - cbit);
        }
        else if (cbit == leftShiftValue+1)//53 bits
        {
            Js::NumberUtilities::LuLoDbl(dbl) |= (ulong)uT >> (cbit - leftShiftValue);
            bExtra = (byte)(uT << (60 - cbit));
        }
        else if (0 != uT)
        {
            bExtra |= 1;
        }
        cbit++;
    }
    // Set the lim.
    *ppchLim = psz;

    // Set the exponent.
    cbit += 1022;
    if (cbit > 2046)
    {
        // Overflow to Infinity
        Js::NumberUtilities::LuHiDbl(dbl) = 0x7FF00000;
        Js::NumberUtilities::LuLoDbl(dbl) = 0;
        return dbl;
    }

    Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)cbit << 20;

    // Use bExtra to round.
    if ((bExtra & 0x80) && ((bExtra & 0x7F) || (Js::NumberUtilities::LuLoDbl(dbl) & 1)))
    {
        // Round up. Note that this overflows the mantissa correctly,
        // even to Infinity.
        if (0 == ++Js::NumberUtilities::LuLoDbl(dbl))
            ++Js::NumberUtilities::LuHiDbl(dbl);
    }
    return dbl;
}

template <typename EncodedChar>
double DblFromOctal(const EncodedChar *psz, const EncodedChar **ppchLim)
{
    double dbl;
    uint uT;
    byte bExtra;
    int cbit;

    // Skip leading zeros.
    while (*psz == '0')
        psz++;

    dbl = 0;
    Assert(Js::NumberUtilities::LuHiDbl(dbl) == 0);
    Assert(Js::NumberUtilities::LuLoDbl(dbl) == 0);

    // Get the first digit.
    uT = *psz - '0';
    if (uT > 7)
    {
        *ppchLim = psz;
        return dbl;
    }
    psz++;

    if (uT & 0x04)//is the 3rd bit set
    {
        cbit = 3;
        Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)(uT & 0x03) << 18;
    }
    else if (uT & 0x02)//is the 2nd bit set
    {
        cbit = 2;
        Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)(uT & 0x01) << 19;
    }
    else// then is the first bit set
    {
        Assert(uT & 0x01);
        cbit = 1;
    }
    bExtra = 0;

    for ( ; (uT = (*psz - '0')) <= 7; psz++)
    {
        if (cbit <= 18)
            Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)uT << (18 - cbit);
        else if (cbit < 21)
        {
            Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)uT >> (cbit - 18);
            Js::NumberUtilities::LuLoDbl(dbl) |= (ulong)uT << (50 - cbit);
        }
        else if (cbit <= 50)
            Js::NumberUtilities::LuLoDbl(dbl) |= (ulong)uT << (50 - cbit);
        else if (cbit <= 53)
        {
            Js::NumberUtilities::LuLoDbl(dbl) |= (ulong)uT >> (cbit - 50);
            bExtra = (byte) (uT << (58 - cbit));
        }
        else if (0 != uT)
            bExtra |= 1;
        cbit += 3;
    }

    // Set the lim.
    *ppchLim = psz;

    // Set the exponent.
    cbit += 1022;
    if (cbit > 2046)
    {
        // Overflow to Infinity
        Js::NumberUtilities::LuHiDbl(dbl) = 0x7FF00000;
        Js::NumberUtilities::LuLoDbl(dbl) = 0;
        return dbl;

    }
    Js::NumberUtilities::LuHiDbl(dbl) |= (ulong)cbit << 20;

    // Use bExtra to round.
    if ((bExtra & 0x80) && ((bExtra & 0x7F) || (Js::NumberUtilities::LuLoDbl(dbl) & 1)))
    {
        // Round up. Note that this overflows the mantissa correctly,
        // even to Infinity.
        if (0 == ++Js::NumberUtilities::LuLoDbl(dbl))
            ++Js::NumberUtilities::LuHiDbl(dbl);
    }

    return dbl;
}

// Turn off warning that there is no return value
#pragma warning(disable:4035) // re-enable below
int wmemcmp(LPCOLESTR pstrL, LPCOLESTR pstrR, long cch)
{
#if _M_IX86
    /* INDENT OFF */
    _asm
    {
        mov   edi, pstrR
            mov   esi, pstrL
            mov   ecx, cch
            xor   eax, eax
            repe  cmpsw
            jz    Equal
            sbb   eax,eax    ;0 for >, -1 for < (CY still set)
            sbb   eax,-1    ;1 for >, -1 for <
Equal:
    }
    /* INDENT ON */
#else // _M_IX86

    for ( ; cch > 0; cch--, pstrL++, pstrR++)
    {
        if (*pstrL != *pstrR)
            return (USHORT)*pstrL < (USHORT)*pstrR ? -1 : 1;
    }
    return 0;

#endif // _M_IX86
}
#pragma warning(default:4035)

double StrToDbl(LPCOLESTR psz, const OLECHAR **ppchLim, Js::ScriptContext *const scriptContext)
{
    Assert(scriptContext);
    bool likelyInt = true;
    return Js::NumberUtilities::StrToDbl<wchar_t>(psz, ppchLim, likelyInt);
}

double StrToDbl(LPCUTF8 psz, const utf8char_t **ppchLim, Js::ScriptContext *const scriptContext)
{
    Assert(scriptContext);
    bool likelyInt = true;
    return Js::NumberUtilities::StrToDbl<utf8char_t>( psz, ppchLim, likelyInt );
}

template double DblFromHex<wchar_t>(const wchar_t *psz, const wchar_t **ppchLim);
template double DblFromHex<utf8char_t>(const utf8char_t *psz, const utf8char_t **ppchLim);
template double DblFromBinary<wchar_t>(const wchar_t *psz, const wchar_t **ppchLim);
template double DblFromBinary<utf8char_t>(const utf8char_t *psz, const utf8char_t **ppchLim);
template double DblFromOctal<wchar_t>(const wchar_t *psz, const wchar_t **ppchLim);
template double DblFromOctal<utf8char_t>(const utf8char_t *psz, const utf8char_t **ppchLim);

