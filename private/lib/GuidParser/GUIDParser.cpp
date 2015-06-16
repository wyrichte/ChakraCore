//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ntassert.h>
#include <intsafe.h>
#include <ATLBase.h>
#include <strsafe.h>
#include "GUIDParser.h"

static const  wchar_t whitespace[] = {L'\x0020', L'\x1680', L'\x180E', L'\x2000', L'\x2001', L'\x2002', L'\x2003', L'\x2004', L'\x2005', L'\x2006', L'\x2007', L'\x2008', L'\x2009', L'\x200A', 
    L'\x202F', L'\x205F', L'\x3000', L'\x2028', L'\x2029', L'\x0009', L'\x000A', L'\x000B', L'\x000C', L'\x000D', L'\x0085', L'\x00A0', L'\0'};


// This code was copied and slightly modified from the support code for 
// StrTrim from shlwapi.h.  This represents the cloapsing and inlining of
// two (or so) functions that were called from StrTrim. 
static PWSTR FindFirstOccurence(_In_z_ PWSTR pStart, WCHAR wMatch)
{
    for ( ; *pStart; pStart++)
    {
      if (*pStart ==wMatch)
        {
            return((PWSTR)pStart);
        }
    }

  return (NULL);
}

// This code was copied and slightly modified from the support code for 
// StrTrim from shlwapi.h.  This represents the cloapsing and inlining of
// two (or so) functions that were called from StrTrim.
static PCWSTR FindFirstOccurence(_In_z_ PCWSTR pStart, WCHAR wMatch)
{
  return FindFirstOccurence(const_cast<PWSTR>(pStart), wMatch);
}


// The code for Trim is copied and slighly modified from he StrTrim function from shlwapi.h.
// The code was copied here to keep from adding an upward layer-map dependency.

/*----------------------------------------------------------
Purpose: Trim the string pszTrimMe of any leading or trailing
         characters that are in pszTrimChars.

Returns: true if anything was stripped

*/
static bool Trim(_Inout_z_ PWSTR pszTrimMe, _In_z_ PCWSTR pszTrimChars)
{
    bool bRet = false;
    
    if (pszTrimMe && pszTrimChars)
    {
        PWSTR psz;
        PWSTR pszStartMeat;
        PWSTR pszMark = NULL;
    
        /* Trim leading characters. */
        
        psz = pszTrimMe;
        
        while (*psz && FindFirstOccurence(pszTrimChars, *psz))
            psz++;
        
        pszStartMeat = psz;
        
        /* Trim trailing characters. */

        // although this code no longer supports 8-byte characters,
        // the algorithm (and most of the code) was borrowed from 
        // code that did
        
        // (The old algorithm used to start from the end and go
        // backwards, but that is piggy because DBCS version of
        // CharPrev iterates from the beginning of the string
        // on every call.)
        
        while (*psz)
        {
            if (FindFirstOccurence(pszTrimChars, *psz))
            {
                if (!pszMark)
                {
                    pszMark = psz;
                }
            }
            else
            {
                pszMark = NULL;
            }
            psz++;
        }
        
        // Any trailing characters to clip?
        if (pszMark)
        {
            // Yes
            *pszMark = '\0';
            bRet = true;
        }
        
        /* Relocate stripped string. */
        
        if (pszStartMeat > pszTrimMe)
        {
            HRESULT hr = S_OK;
            size_t cb = 0;
            hr = StringCbLength(pszStartMeat, STRSAFE_MAX_CCH * sizeof(WCHAR), &cb);
            NT_ASSERT(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                /* (+ 1) for null terminator. */
                MoveMemory(pszTrimMe, pszStartMeat, cb + sizeof(WCHAR));
                bRet = true;
            }
        }
        else
        {
            NT_ASSERT(pszStartMeat == pszTrimMe);
        }
    }
    
    return bRet;
}

class GUIDParser
{    
public:
    static HRESULT TryParseGUID(_In_z_ PCWSTR g, _Out_ GUID* result);
    static HRESULT TryGUIDToString(_In_ const GUID* g, _Out_writes_z_(length) wchar_t* result, size_t length);

private:
    static bool IsWhitespace(wchar_t c);
    static void EatAllWhitespace(_Inout_updates_z_(length) wchar_t* string, size_t length);
    static bool IsHexDigit(wchar_t c) { return (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))); }
    static HRESULT TryParseHexValueExact(_In_z_ PCWSTR g, int length, _Out_ unsigned long* value);
    static HRESULT TryParseHexValue(_In_z_ PCWSTR g, int maxLength, _Out_ int* offset, _Out_ unsigned long* value);
    static HRESULT TryParseGUIDWithHexFormat(_In_z_ PCWSTR g, _Out_ GUID* result);
    static HRESULT TryParseGUIDWithDashes(_In_z_ PCWSTR g, _Out_ GUID* result);
    static HRESULT TryParseGUIDWithNoStyle(_In_z_ PCWSTR g, _Out_ GUID* result);
};

HRESULT TryParseGUID(_In_z_ PCWSTR g, _Out_ GUID* result)
{
    return GUIDParser::TryParseGUID(g,result);
}

HRESULT TryGUIDToString(_In_ const GUID* g, _Out_writes_z_(length) PWSTR result, size_t length)
{
    return GUIDParser::TryGUIDToString(g,result,length);
}


// Name: TryGUIDToString
// Info: parses a guid string
// Params: g - The guid struct	
//		   result	- The GUID string parsed from the given struct
// Returns: E_POINTER in case one of the arguments is NULL
//			E_INVALIDARG in case an null guid was specified
//			S_OK in case of success
STDAPI GUIDParser::TryGUIDToString(_In_ const GUID* g, _Out_writes_z_(length) wchar_t* result, size_t length)
{
    if (nullptr == result)
    {
        return E_POINTER;
    }

    if (nullptr == g)
    {
        return E_POINTER;
    }

    if (length < 37)
    {
        return E_INVALIDARG;
    }

    // Set the GUID string length
    int guidlen = 37;
                         
    result[0] = '\0';

    wchar_t tempStr[9];
    tempStr[0] = '\0';

    int numChar = swprintf_s(tempStr, 9, L"%.8x", (int)(g->Data1));
    if (numChar != 8)
    {
        return E_INVALIDARG;
    }
    errno_t error = wcscat_s(result, guidlen, tempStr);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, L"-");
    if (error != 0)
    {
        return E_INVALIDARG;
    }

    numChar = swprintf_s(tempStr, 9, L"%.4x", (int)(g->Data2));
    if (numChar != 4)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, tempStr);  
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, L"-");
    if (error != 0)
    {
        return E_INVALIDARG;
    }

    numChar = swprintf_s(tempStr, 9, L"%.4x", (int)(g->Data3));
    if (numChar != 4)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, tempStr);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, L"-");
    if (error != 0)
    {
        return E_INVALIDARG;
    }

    for(int i = 0; i < 8; i++)
    {
        numChar = swprintf_s(tempStr, 9, L"%.2x", (int)(g->Data4[i]));
        if (numChar != 2)
        {
            return E_INVALIDARG;
        }
        error = wcscat_s(result, guidlen, tempStr);
        if (error != 0)
        {
            return E_INVALIDARG;
        }
       if (i == 1)
        {
            error = wcscat_s(result, guidlen, L"-");
            if (error != 0)
            {
                return E_INVALIDARG;
            }
        }
    }

    return S_OK;
}

// Name: TryParseGUID
// Info: parses a guid string
// Params: g - The guid string	
//		   result	- The GUID struct parsed from the given string
// Returns: E_POINTER in case one of the arguments is NULL
//			E_INVALIDARG in case an null guid was specified
//			S_OK in case of success
STDAPI GUIDParser::TryParseGUID(_In_z_ PCWSTR g, _Out_ GUID* result)
{
    if (nullptr == result)
    {
        return E_POINTER;
    }

    if (nullptr == g)
    {
        return E_POINTER;
    }

    CHeapPtr<wchar_t> guidStr;
    guidStr.Allocate((wcslen(g)+1) * sizeof(wchar_t));
    errno_t error = wcscpy_s(guidStr, wcslen(g)+1, g);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    Trim(guidStr, whitespace);

    bool containsDashes = (wcschr(guidStr, '-') != NULL);
    bool containsBraces = (wcschr(guidStr, '{') != NULL);

    if (containsDashes)
    {
        return TryParseGUIDWithDashes(guidStr, result);
    }
    else if (containsBraces)
    {
        return TryParseGUIDWithHexFormat(guidStr, result);
    }
    else
    {
        return TryParseGUIDWithNoStyle(guidStr, result);
    }
}

HRESULT GUIDParser::TryParseHexValueExact(_In_z_ PCWSTR g, int length, _Out_ unsigned long* value)
{
    for (int index = 0; index < length; index ++)
    {
        if (!IsHexDigit(g[index]))
        {
            return E_INVALIDARG;
        }
    }

    CHeapPtr<wchar_t> destStr;
    destStr.Allocate((length+1) * sizeof(wchar_t));
    destStr[0] = NULL;
    errno_t error = wcsncat_s(destStr, (length + 1), g, length);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    unsigned long lValue = wcstoul(destStr, NULL, 16);

    *value = lValue;
    return S_OK;
}

HRESULT GUIDParser::TryParseHexValue(_In_z_ PCWSTR g, int maxLength, _Out_ int* offset, _Out_ unsigned long* value)
{
    int index;
    int sigDigits = 0; 
    
    for (index = 0; IsHexDigit(g[index]); index++)
    {
        if ((sigDigits > 0) || (g[index] != '0'))
        {
            sigDigits++;
            if (sigDigits > maxLength)
            {
                return E_INVALIDARG;
            }
        }
    }

    if (index <= 0)
    {
        return E_INVALIDARG;
    }

    CHeapPtr<wchar_t> destStr;
    destStr.Allocate((index+1) * sizeof(wchar_t));
    destStr[0] = NULL;
    errno_t error = wcsncat_s(destStr, (index + 1), g, index);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    unsigned long lValue = wcstoul(destStr, NULL, 16);

    *value = lValue;
    *offset = index;
    return S_OK;
}

HRESULT GUIDParser::TryParseGUIDWithHexFormat(_In_z_ PCWSTR g, _Out_ GUID* result)
{
    HRESULT hr = S_OK;
    unsigned long value = NULL;
    int pos = 0;
    int offset = 0;

    // Remove all whitespace from guid string
    CHeapPtr<wchar_t> guidStr;
    guidStr.Allocate((wcslen(g)+1) * sizeof(wchar_t));
    errno_t error = wcscpy_s(guidStr, wcslen(g)+1, g);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    EatAllWhitespace(guidStr, wcslen(g)+1);

    // Check for beginning brace and hex prefix
    if ((guidStr[pos] != '{') || (guidStr[pos+1] != '0') || ((guidStr[pos+2] != 'x') && (guidStr[pos+2] != 'X')))
    {
        return E_INVALIDARG;
    }
    pos += 3;

    // Parse the first DWORD
    hr = TryParseHexValue(&guidStr[pos], 8, &offset, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data1 = value;
    pos += offset;

    // Check that it begins with a hex prefix (,0x)
    if ((guidStr[pos] != ',') || (guidStr[pos+1] != '0') || ((guidStr[pos+2] != 'x') && (guidStr[pos+2] != 'X')))
    {
        return E_INVALIDARG;
    }
    pos += 3;

    // Parse the first WORD
    hr = TryParseHexValue(&guidStr[pos], 4, &offset, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data2 = (short)value;
    pos += offset;

    // Check that it begins with a hex prefix (,0x)
    if ((guidStr[pos] != ',') || (guidStr[pos+1] != '0') || ((guidStr[pos+2] != 'x') && (guidStr[pos+2] != 'X')))
    {
        return E_INVALIDARG;
    }
    pos += 3;

    // Parse the second WORD
    hr = TryParseHexValue(&guidStr[pos], 4, &offset, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data3 = (short)value;
    pos += offset;

    // Check that the series of bytes begins with a brace
    if ((guidStr[pos] != ',') || (guidStr[pos+1] != '{'))
    {
        return E_INVALIDARG;
    }
    pos += 2;

    for (int index = 0; index < 8; index ++)
    {
        // Check that it begins with a hex prefix (0x)
        if ((guidStr[pos] != '0') || ((guidStr[pos+1] != 'x') && (guidStr[pos+1] != 'X')))
        {
            return E_INVALIDARG;
        }
        pos += 2;

        // Parse byte
        hr = TryParseHexValue(&guidStr[pos], 2, &offset, &value);
        if(FAILED(hr))
    {
        return hr;
    }
        result->Data4[index] = (BYTE)value;
        pos += offset;

        // Check that all but the last byte is followed by a comma seperator
        if (index < 7)
        {
            if (guidStr[pos] != ',')
            {
                return E_INVALIDARG;
            }
            pos++;
        }
    }

    // Check for two closing braces and a null terminator
    if ((guidStr[pos] != '}') || (guidStr[pos+1] != '}') || (guidStr[pos+2] != NULL))
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT GUIDParser::TryParseGUIDWithDashes(_In_z_ PCWSTR g, _Out_ GUID* result)
{
    HRESULT hr;
    int pos = 0;
    unsigned long value;
    wchar_t final = '\0';

    if (g[pos] != L'\0' && g[pos] == '{')
    {
        final = '}';
        pos++;
    }
    else if (g[pos] != L'\0' && g[pos] == '(')
    {
        final = ')';
        pos++;
    }

    // Parse the first DWORD
    hr = TryParseHexValueExact(&g[pos], 8, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data1 = value;
    // Move current position past first DWORD
    pos += 8;

    // Check for dash
    if (g[pos] != '-')
    {
        return E_INVALIDARG;
    }
    pos++;

    // Parse first WORD
    hr = TryParseHexValueExact(&g[pos], 4, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data2 = (short)value;
    // Move current position past WORD
    pos += 4;

    // Check for dash
    if (g[pos] != '-')
    {
        return E_INVALIDARG;
    }
    pos++;

    // Parse second WORD
    hr = TryParseHexValueExact(&g[pos], 4, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data3 = (short)value;
    // Move current position past WORD
    pos += 4;

    // Check for dash
    if (g[pos] != '-')
    {
        return E_INVALIDARG;
    }
    pos++;

    // Parse next 2 bytes
    for (int index = 0; index < 2; index++)
    {
        hr = TryParseHexValueExact(&g[pos], 2, &value);
        if(FAILED(hr))
    {
        return hr;
    }
        result->Data4[index] = (BYTE)value;
        pos += 2;
    }

    // Check for dash
    if (g[pos] != '-')
    {
        return E_INVALIDARG;
    }
    pos++;

    // Parse remaining 6 bytes
    for (int index = 2; index < 8; index++)
    {
        hr = TryParseHexValueExact(&g[pos], 2, &value);
        if(FAILED(hr))
    {
        return hr;
    }
        result->Data4[index] = (BYTE)value;
        pos += 2;
    }

    if (final != '\0')
    {
        if (g[pos] != final)
        {
            return E_INVALIDARG;
        }
        pos++;
    }

    if (g[pos] != '\0')
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT GUIDParser::TryParseGUIDWithNoStyle(_In_z_ PCWSTR g, _Out_ GUID* result)
{
    HRESULT hr;
    int pos = 0;
    unsigned long value;

    // Parse the first DWORD
    hr = TryParseHexValueExact(g, 8, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data1 = value;
    // Move current position past first DWORD
    pos += 8;

    // Parse first WORD
    hr = TryParseHexValueExact(&g[pos], 4, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data2 = (short)value;
    // Move current position past WORD
    pos += 4;

    // Parse second WORD
    hr = TryParseHexValueExact(&g[pos], 4, &value);
    if(FAILED(hr))
    {
        return hr;
    }
    result->Data3 = (short)value;
    // Move current position past WORD
    pos += 4;

    // Parse remaining 8 bytes
    for (int index = 0; index < 8; index++)
    {
        hr = TryParseHexValueExact(&g[pos], 2, &value);
        if(FAILED(hr))
    {
        return hr;
    }
        result->Data4[index] = (BYTE)value;
        pos += 2;
    }

    if (g[pos] != NULL)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

bool GUIDParser::IsWhitespace(wchar_t c)
{
    size_t length = wcslen(whitespace);
    for (size_t i = 0; i < length; i++)
    {
        if (c == whitespace[i]) 
        {
            return true;
        }
    }
    return false;
}

void GUIDParser::EatAllWhitespace(_Inout_updates_z_(length) wchar_t* string, size_t length)
{
    int pos = 0;
    for (unsigned int i = 0; i < (length-1); i++)
    {
        if (!IsWhitespace(string[i]))
        {
            string[pos] = string[i];
            pos++;
        }
    }
    string[pos] = '\0';
    return;
}

