/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#pragma hdrstop


// The variant conversion routines specifically check for english locale
// and the flag LOCALE_NOUSEROVERRIDE and use settings we'd expect for
// converting values into strings that can be reparsed. e.g. The conversions
// are based on constant parameters built into oleaut rather than off
// the NLS parameters that can be munged in the Regional settings control
// panel.
#define LOCALE_ENGLISH 0x0409


/***************************************************************************
ComDebugFormatter
***************************************************************************/
const int kcchBufferMax = 64; // Big enough for largest string rep of double
const OLECHAR kchEscapeQuote = '\\';
const LPCOLESTR kpstrHexPrefix = OLESTR("0x");
const int kcchHexPrefix = 2; // ostrlen(kpstrHexPrefix)

// Maximum number of hex digits in 64 bit integer = 2 hex per byte & 8 bytes
const int kcchMaxHex64 = 16;
const OLECHAR krgchHexMap[] = 
{ 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
    'A', 'B', 'C', 'D', 'E', 'F' 
};

static HRESULT GetHexLiteral(VARIANT *pvar, BSTR *pbstr);
static HRESULT GetDecimalLiteral(VARIANT *pvar, BSTR *pbstr);
static HRESULT GetStringLiteral(LPCOLESTR pstr, BSTR *pbstr);



ComDebugFormatter::ComDebugFormatter(void) : m_cref(1)
{ 
}

ComDebugFormatter::~ComDebugFormatter(void)
{	
}


// === ComDebugFormatter ===
HRESULT ComDebugFormatter::Create(ComDebugFormatter **ppdf)
{
    AssertMem(ppdf);
    if (NULL == (*ppdf = new ComDebugFormatter))
        return HR(E_OUTOFMEMORY);
    return NOERROR;
}


// === IUnknown ===
DEFINE_IUNKNOWN(ComDebugFormatter, m_cref)

    STDMETHODIMP ComDebugFormatter::QueryInterface(REFIID riid, void **ppv)
{
    CHECK_POINTER(ppv);
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDebugFormatter))
        *ppv = (IDebugFormatter *)this;
    else
    {
        *ppv = NULL;
        return HR(E_NOINTERFACE);
    }
    AddRef();
    return NOERROR;
}


// === IDebugFormatter ===
STDMETHODIMP ComDebugFormatter::GetStringForVariant(VARIANT *pvarIn, 
                                                    ULONG ulRadix, BSTR *pbstr)
{
    return DebugApiWrapper( [&] {
        CHECK_POINTER(pbstr);
        *pbstr = NULL;
        if (NULL == pvarIn)
            return HR(E_INVALIDARG);
        if (16 != ulRadix && 10 != ulRadix)
            return HR(E_NOTIMPL);

        // Get rid of any ref's
        HRESULT hr = NOERROR;
        VARIANT varT;
        varT.vt = VT_EMPTY;
        if (VT_BYREF & pvarIn->vt)
        {
            if (FAILED(hr = VariantCopyInd(&varT, pvarIn)))
                return hr;
            pvarIn = &varT;		
        }

        LPCOLESTR pstrT;
        DebugOnly(pstrT = NULL);
        Assert(SUCCEEDED(hr));

#ifdef TODO_DEBUGGER
LRestart:
#endif
        long lVal;
        ULONG ulVal;
        double dblVal;
        VAR varVal;
        OLECHAR strBuffer[kcchBufferMax];
        switch (pvarIn->vt)
        {
        case VT_EMPTY:
            pstrT = OLESTR("undefined");
            break;
        case VT_NULL:
            pstrT = OLESTR("null");
            break;
        case VT_BOOL:
            pstrT = (VARIANT_FALSE == pvarIn->boolVal ? OLESTR("false") : 
                OLESTR("true"));
            break;
        case VT_BSTR:
            pstrT = pvarIn->bstrVal;
            if (NULL == pstrT)
                pstrT = OLESTR("");
            hr = GetStringLiteral(pstrT, pbstr);
            goto LExit;

        case VT_I1:
            lVal = pvarIn->cVal;
            goto LSigned;
        case VT_I2:
            lVal = pvarIn->iVal;
            goto LSigned;
        case VT_I4:
            lVal = pvarIn->lVal;
            goto LSigned;
        case VT_INT:
            lVal = pvarIn->intVal;

LSigned:
            if (16 == ulRadix)
            {
                hr = GetHexLiteral(pvarIn, pbstr);
                goto LExit;
            }
            oltoa(lVal, strBuffer, 10);
            Assert(ostrlen(strBuffer) < kcchBufferMax);
            pstrT = strBuffer;
            break;

        case VT_UI1:
            ulVal = pvarIn->bVal;
            goto LUnsigned;
        case VT_UI2:
            ulVal = pvarIn->uiVal;
            goto LUnsigned;
        case VT_UI4:
            ulVal = pvarIn->ulVal;
            goto LUnsigned;
        case VT_UINT:
            ulVal = pvarIn->uintVal;

LUnsigned:
            if (16 == ulRadix)
            {
                hr = GetHexLiteral(pvarIn, pbstr);
                goto LExit;
            }
            _ultow_s(ulVal, strBuffer, kcchBufferMax, 10);
            Assert(ostrlen(strBuffer) < kcchBufferMax);
            pstrT = strBuffer;
            break;

        case VT_R4:
            dblVal = pvarIn->fltVal;
            goto LReal; 
        case VT_R8:
            dblVal = pvarIn->dblVal;

LReal:
            if (Js::NumberUtilities::IsFinite(dblVal))
            {
                if (!Js::NumberUtilities::FDblToStr(dblVal, strBuffer, kcchBufferMax))
                {
                    hr = HR(E_OUTOFMEMORY);
                    goto LExit;
                }

                pstrT = strBuffer;
                Assert(ostrlen(strBuffer) < kcchBufferMax);
                // fall through
            }
            else if (Js::NumberUtilities::IsNan(dblVal))
                pstrT = OLESTR("NaN");
            else
                pstrT = dblVal < 0 ? OLESTR("-Infinity") : OLESTR("Infinity");
            break;
        case VT_DISPATCH:
        case VT_UNKNOWN:
            if (NULL == pvarIn->punkVal)
            {
                pstrT = OLESTR("null");
                break;
            }

            // We want to display builtin JScript objects specially.
            IUnknown *punk;
            pstrT = OLESTR("");
            punk = pvarIn->punkVal;
#ifdef TODO_DEBUGGER
            DateObj *pdate;
            if (SUCCEEDED(punk->QueryInterface(IID_IJsDateObj, (void **)&pdate)))
            {
                // Dates return the string.
                hr = pdate->GetString(&varVal, kdsfDefault);
                pdate->Release();

LVarVal:
                if (FAILED(hr))
                    goto LExit;
                VariantClear(&varT);
                if (FAILED(hr = varVal.GetStdVar(&varT)))
                    goto LExit;
                Assert(VT_BSTR == varT.vt);
                // Take ownership of the bstr.
                *pbstr = varT.bstrVal; 
                varT.vt = VT_EMPTY;
                goto LExit;
            }
            RegExpObj *pre;
            if (SUCCEEDED(punk->QueryInterface(IID_IJsRegExpObj, (void **)&pre)))
            {
                // Regular expressions return the string.
                hr = pre->GetString(&varVal);
                pre->Release();
                goto LVarVal;
            }
            NameTbl *pntbl;
            if (SUCCEEDED(punk->QueryInterface(IID_INameTbl, (void **)&pntbl)))
            {
                VAR var;
                hr = pntbl->GetVarThis(&var);
                pntbl->Release();
                if (FAILED(hr))
                    goto LExit;
                VariantClear(&varT);
                if (FAILED(hr = var.GetStdVar(&varT)))
                    goto LExit;

                // We don't try to convert the this value if it is an object
                // so that we don't go into an infinite loop. GetVarThis
                // will return a ref to itself (pntbl) if it is not assocaiated
                // with a value. Otherwise, setup the variant to convert
                // and retry.
                Assert(VT_EMPTY != varT.vt);
                switch (varT.vt)
                {
                case VT_UNKNOWN:
                case VT_DISPATCH:
                    break;
                default:
                    pvarIn = &varT;
                    goto LRestart;
                }
            }
#endif

            break;
        default:
            // No string by default - we need to allocate a string because
            // the pdm treats a NULL value for *pbstr to mean "use default 
            // processing"
            *pbstr = NULL;
            goto LExit;
        }

        AssertMemR(pstrT);
        *pbstr = SysAllocString(pstrT);
        if (NULL == *pbstr)
            hr = HR(E_OUTOFMEMORY);

LExit:
        VariantClear(&varT);
        return FAILED(hr) ? hr : NOERROR;
    });
}

STDMETHODIMP ComDebugFormatter::GetVariantForString(LPCOLESTR bstrValue, 
                                                    VARIANT *pvar)
{
    return HR(E_NOTIMPL);
}

STDMETHODIMP ComDebugFormatter::GetStringForVarType(VARTYPE vt, TYPEDESC *ptd, 
                                                    BSTR *pbstr)
{
    return DebugApiWrapper( [=] {
        LPCOLESTR pstrT;
        DebugOnly(pstrT = NULL);

        switch(vt){
        case VT_EMPTY:	  	//0
            //
            // undefined value is of 'Undefined' type (section 4.3.10, ECMA 262 V3)
            //
            pstrT = OLESTR("Undefined");
            break;
        case VT_NULL:	   	//1
            //
            // null value is of 'Null' type (section 4.3.12, ECMA 262 V3)
            //
            pstrT = OLESTR("Null");
            break;
        case VT_I2:		//2
        case VT_I4:		//3
        case VT_R4:		//4
        case VT_R8:		//5
        case VT_DECIMAL:	//14
        case VT_I1:		//16
        case VT_UI1:		//17
        case VT_UI2:		//18
        case VT_UI4:		//19
        case VT_I8:		//20
        case VT_UI8:		//21
        case VT_INT:		//22
        case VT_UINT:		//23
            //
            // A number value is a member of type 'Number' (section 4.3.19, ECMA 262 V3)
            //
            pstrT = OLESTR("Number");
            break;
        case VT_BSTR:		//8
            //
            // A string value is a member of type 'String' (section 4.3.16, ECMA 262 V3)
            //
            pstrT = OLESTR("String");
            break;
        case VT_BOOL:		//11
            //
            // A boolean value is a member of type 'Boolean' (section 4.3.13, ECMA 262 V3)
            //
            pstrT = OLESTR("Boolean");
            break;
        default:
            //
            // Rest are handled by PDM without the use of IDebugFormatter
            //
            return HR(E_NOTIMPL);
        }

        AssertMemR(pstrT);
        *pbstr = SysAllocString(pstrT);
        if (NULL == *pbstr)
            return HR(E_OUTOFMEMORY);

        return HR(NOERROR);
    });
}


// === Helpers ===
static HRESULT GetHexLiteral(VARIANT *pvar, BSTR *pbstr)
{
    AssertMem(pvar);
    AssertMem(pbstr);

    int cbVal;
    switch (pvar->vt)
    {
    case VT_I1:
    case VT_UI1:
        cbVal = 1;
        break;	
    case VT_I2:
    case VT_UI2:
        cbVal = 2;
        break;
    case VT_I4:
    case VT_UI4:
        cbVal = 4;
        break;
    case VT_I8:
    case VT_UI8:
        cbVal = 8;
        break;		
    case VT_INT:
    case VT_UINT:
        cbVal = sizeof(int);
        break;
    default:
        AssertMsg(FALSE, "Which vt don't we handle?");
        return HR(E_NOTIMPL);
    }

    BYTE *pbyte = (BYTE *)&pvar->bVal;
    OLECHAR strBuffer[kcchMaxHex64 + 1];
    LPOLESTR pstrT = strBuffer;

#if BIGENDIAN
    const int knScanDirection = 1;
#else // !BIGENDIAN
    const int knScanDirection = -1;
    pbyte += cbVal-1;	// Point to the last byte.
#endif // !BIGENDIAN

    // Find the first non-zero byte.
    while (0 != cbVal && 0 == *pbyte)
    {
        pbyte += knScanDirection;
        cbVal--;
    }

    if (0 == cbVal)
        *pstrT++ = '0';
    else
    {
        BYTE lo, hi;
        lo = *pbyte & 0x0F;
        hi = *pbyte >> 4;
        if (0 != hi)
            *pstrT++ = krgchHexMap[hi];
        *pstrT++ = krgchHexMap[lo];

        pbyte += knScanDirection;
        cbVal--;
        while (0 != cbVal--)
        {
            BYTE b = *pbyte;
            pbyte += knScanDirection;
            *pstrT++ = krgchHexMap[b >> 4];
            *pstrT++ = krgchHexMap[b & 0x0F];
        }
    }
    *pstrT = 0;

    Assert(pstrT - strBuffer >= 0);
    Assert(pstrT - strBuffer <= INT_MAX);
    int cchHex = (int)(pstrT - strBuffer);
    *pbstr = SysAllocStringLen(NULL, kcchHexPrefix + cchHex);
    if (NULL == *pbstr)
        return HR(E_OUTOFMEMORY);

    LPCOLESTR pstrFrom = kpstrHexPrefix;
    pstrT = *pbstr;
    while (0 != *pstrFrom)
        *pstrT++ = *pstrFrom++;
    pstrFrom = strBuffer;
    while (0 != *pstrFrom)
        *pstrT++ = *pstrFrom++;
    return NOERROR;
}

static HRESULT GetDecimalLiteral(VARIANT *pvar, BSTR *pbstr)
{
    AssertMem(pvar);
    AssertMem(pbstr);

    long lVal;
    ULONG ulVal;
    switch (pvar->vt)
    {
    case VT_I1:
        lVal = pvar->cVal;
        goto LSigned;
    case VT_I2:
        lVal = pvar->iVal;
        goto LSigned;
    case VT_I4:
        lVal = pvar->lVal;
        goto LSigned;
    case VT_INT:
        lVal = pvar->intVal;
        goto LSigned;

    case VT_UI1:
        ulVal = pvar->bVal;
        goto LUnsigned;
    case VT_UI2:
        ulVal = pvar->uiVal;
        goto LUnsigned;
    case VT_UI4:
        ulVal = pvar->ulVal;
        goto LUnsigned;
    case VT_UINT:
        ulVal = pvar->uintVal;
        goto LUnsigned;

    default:
        AssertMsg(FALSE, "Which vt don't we handle?");
        return HR(E_NOTIMPL);
    }

LSigned:
    return VarBstrFromI4(lVal, LOCALE_ENGLISH, LOCALE_NOUSEROVERRIDE, pbstr);

LUnsigned:
    return VarBstrFromUI4(ulVal, LOCALE_ENGLISH, LOCALE_NOUSEROVERRIDE, pbstr);
}

static HRESULT GetStringLiteral(LPCOLESTR pstr, BSTR *pbstr)
{
    AssertMemR(pstr);
    AssertMem(pbstr);

    int cquote;
    LPCOLESTR pstrT;

    cquote = 0;
    for (pstrT = pstr; 0 != *pstrT; pstrT++)
    {
        if ('"' == *pstrT)
            cquote++;
    }

    Assert(pstrT - pstr >= 0);
    Assert(pstrT - pstr + cquote + 2 <= INT_MAX);
    //BUG 1118997 Windows OS BUG
    //PREFAST: '(int)(pstrT-pstr)+cquote' may be smaller than its operands. 
    if ((pstrT - pstr < 0) || (INT_MAX -2 - (pstrT - pstr) < cquote) )
        return HR(E_OUTOFMEMORY);
    int cch = (int)(pstrT - pstr) + cquote + 2;  // + 2 for quotes
    LPOLESTR pstrW = SysAllocStringLen(NULL, cch);
    if (NULL == pstrW)	
        return HR(E_OUTOFMEMORY);

    *pbstr = pstrW;
    *pstrW++ = '"';
    if (0 == cquote)
    {
        OLECHAR ch;
        for (pstrT = pstr;  0 != (ch = *pstrT); pstrT++)
            *pstrW++ = ch;
    }
    else
    {
        OLECHAR ch;
        for (pstrT = pstr;  0 != (ch = *pstrT); pstrT++)
        {
            if ('"' == ch)
                *pstrW++ = kchEscapeQuote;
            *pstrW++ = ch;
        }
    }
    *pstrW++ = '"';
    Assert(pstrW - *pbstr == cch);
    return NOERROR;		
}



