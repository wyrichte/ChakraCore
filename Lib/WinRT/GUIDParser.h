//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class GUIDParser
{    
public:
    static HRESULT TryParseGUID(LPCWSTR g, GUID* result);
    static HRESULT TryGUIDToString(GUID* g, __in_ecount(length) wchar_t* result, size_t length);

private:
    static bool IsWhitespace(wchar_t c);
    static void EatAllWhitespace(__in_ecount(length) wchar_t* string, size_t length);
    static bool IsHexDigit(wchar_t c) { return (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))); }
    static HRESULT TryParseHexValueExact(LPCWSTR g, int length, unsigned long* value);
    static HRESULT TryParseHexValue(LPCWSTR g, int maxLength, int* offset, unsigned long* value);
    static HRESULT TryParseGUIDWithHexFormat(LPCWSTR g, GUID* result);
    static HRESULT TryParseGUIDWithDashes(LPCWSTR g, GUID* result);
    static HRESULT TryParseGUIDWithNoStyle(LPCWSTR g, GUID* result);
};