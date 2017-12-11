//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "WinRTPch.h"
#include "jscriptdllcommon.h"
#include <Shlwapi.h>

#pragma warning(push)
#pragma warning(disable:4838) // conversion from 'int' to 'UINT' requires a narrowing conversion
#pragma warning(disable:4456) // declaration of '' hides previous local declaration
#include <atlbase.h>
#pragma warning(pop)
#include "GUIDParser.h"

using namespace Js;

static const  char16 whitespace[] = {_u('\x0020'), _u('\x1680'), _u('\x180E'), _u('\x2000'), _u('\x2001'), _u('\x2002'), _u('\x2003'), _u('\x2004'), _u('\x2005'), _u('\x2006'), _u('\x2007'), _u('\x2008'), _u('\x2009'), _u('\x200A'), 
    _u('\x202F'), _u('\x205F'), _u('\x3000'), _u('\x2028'), _u('\x2029'), _u('\x0009'), _u('\x000A'), _u('\x000B'), _u('\x000C'), _u('\x000D'), _u('\x0085'), _u('\x00A0'), _u('\0')};


// Name: TryGUIDToString
// Info: parses a guid string
// Params: g - The guid struct	
//		   result	- The GUID string parsed from the given struct
// Returns: E_POINTER in case one of the arguments is NULL
//			E_INVALIDARG in case an null guid was specified
//			S_OK in case of success
HRESULT GUIDParser::TryGUIDToString(GUID* g, __in_ecount(length) char16* result, size_t length)
{
    IfNullReturnError(result, E_POINTER);
    IfNullReturnError(g, E_POINTER);

    if (length < 37)
    {
        return E_INVALIDARG;
    }

    // Set the GUID string length
    int guidlen = 37;
                         
    result[0] = '\0';

    char16 tempStr[9];
    tempStr[0] = '\0';

    int numChar = swprintf_s(tempStr, 9, _u("%.8x"), (int)(g->Data1));
    if (numChar != 8)
    {
        return E_INVALIDARG;
    }
    errno_t error = wcscat_s(result, guidlen, tempStr);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, _u("-"));
    if (error != 0)
    {
        return E_INVALIDARG;
    }

    numChar = swprintf_s(tempStr, 9, _u("%.4x"), (int)(g->Data2));
    if (numChar != 4)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, tempStr);  
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, _u("-"));
    if (error != 0)
    {
        return E_INVALIDARG;
    }

    numChar = swprintf_s(tempStr, 9, _u("%.4x"), (int)(g->Data3));
    if (numChar != 4)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, tempStr);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    error = wcscat_s(result, guidlen, _u("-"));
    if (error != 0)
    {
        return E_INVALIDARG;
    }

    for(int i = 0; i < 8; i++)
    {
        numChar = swprintf_s(tempStr, 9, _u("%.2x"), (int)(g->Data4[i]));
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
            error = wcscat_s(result, guidlen, _u("-"));
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
HRESULT GUIDParser::TryParseGUID(LPCWSTR g, GUID* result)
{
    IfNullReturnError(result, E_POINTER);
    IfNullReturnError(g, E_POINTER);

    CHeapPtr<char16> guidStr;
    guidStr.Allocate((wcslen(g)+1) * sizeof(char16));
    errno_t error = wcscpy_s(guidStr, wcslen(g)+1, g);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    StrTrimW(guidStr, whitespace);

    bool containsDashes = (wcschr(guidStr, '-') != nullptr);
    bool containsBraces = (wcschr(guidStr, '{') != nullptr);

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

HRESULT GUIDParser::TryParseHexValueExact(LPCWSTR g, int length, unsigned long* value)
{
    for (int index = 0; index < length; index ++)
    {
        if (!IsHexDigit(g[index]))
        {
            return E_INVALIDARG;
        }
    }

    CHeapPtr<char16> destStr;
    destStr.Allocate((length+1) * sizeof(char16));
    destStr[0] = 0;
    errno_t error = wcsncat_s(destStr, (length + 1), g, length);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    unsigned long lValue = wcstoul(destStr, nullptr, 16);

    *value = lValue;
    return S_OK;
}

HRESULT GUIDParser::TryParseHexValue(LPCWSTR g, int maxLength, int* offset, unsigned long* value)
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

    CHeapPtr<char16> destStr;
    destStr.Allocate((index+1) * sizeof(char16));
    destStr[0] = 0;
    errno_t error = wcsncat_s(destStr, (index + 1), g, index);
    if (error != 0)
    {
        return E_INVALIDARG;
    }
    unsigned long lValue = wcstoul(destStr, nullptr, 16);

    *value = lValue;
    *offset = index;
    return S_OK;
}

HRESULT GUIDParser::TryParseGUIDWithHexFormat(LPCWSTR g, GUID* result)
{
    HRESULT hr = S_OK;
    unsigned long value = 0;
    int pos = 0;
    int offset = 0;

    // Remove all whitespace from guid string
    CHeapPtr<char16> guidStr;
    guidStr.Allocate((wcslen(g)+1) * sizeof(char16));
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
    IfFailedReturn(hr);
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
    IfFailedReturn(hr);
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
    IfFailedReturn(hr);
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
        IfFailedReturn(hr);
        result->Data4[index] = (byte)value;
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
    if ((guidStr[pos] != '}') || (guidStr[pos+1] != '}') || (guidStr[pos+2] != 0))
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT GUIDParser::TryParseGUIDWithDashes(LPCWSTR g, GUID* result)
{
    HRESULT hr;
    int pos = 0;
    unsigned long value;
    char16 final = '\0';

    if (g[pos] == '{')
    {
        final = '}';
        pos++;
    }
    else if (g[pos] == '(')
    {
        final = ')';
        pos++;
    }

    // Parse the first DWORD
    hr = TryParseHexValueExact(&g[pos], 8, &value);
    IfFailedReturn(hr);
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
    IfFailedReturn(hr);
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
    IfFailedReturn(hr);
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
        IfFailedReturn(hr);
        result->Data4[index] = (byte)value;
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
        IfFailedReturn(hr);
        result->Data4[index] = (byte)value;
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

HRESULT GUIDParser::TryParseGUIDWithNoStyle(LPCWSTR g, GUID* result)
{
    HRESULT hr;
    int pos = 0;
    unsigned long value;

    // Parse the first DWORD
    hr = TryParseHexValueExact(g, 8, &value);
    IfFailedReturn(hr);
    result->Data1 = value;
    // Move current position past first DWORD
    pos += 8;

    // Parse first WORD
    hr = TryParseHexValueExact(&g[pos], 4, &value);
    IfFailedReturn(hr);
    result->Data2 = (short)value;
    // Move current position past WORD
    pos += 4;

    // Parse second WORD
    hr = TryParseHexValueExact(&g[pos], 4, &value);
    IfFailedReturn(hr);
    result->Data3 = (short)value;
    // Move current position past WORD
    pos += 4;

    // Parse remaining 8 bytes
    for (int index = 0; index < 8; index++)
    {
        hr = TryParseHexValueExact(&g[pos], 2, &value);
        IfFailedReturn(hr);
        result->Data4[index] = (byte)value;
        pos += 2;
    }

    if (g[pos] != 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

bool GUIDParser::IsWhitespace(char16 c)
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

void GUIDParser::EatAllWhitespace(__in_ecount(length) char16* string, size_t length)
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