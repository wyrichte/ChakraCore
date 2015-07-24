//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

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

#if 0
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
#endif


