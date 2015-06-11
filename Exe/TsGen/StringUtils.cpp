//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "StringUtils.h"

// Determine whether a string ends with a particular suffix
bool StringUtils::EndsWith(_In_ LPCWSTR str, _In_ LPCWSTR suffix)
{
    if (str == nullptr || suffix == nullptr)
    {
        Assert(0);
        return false;
    }

    int strLen = (int)wcslen(str);
    int suffixLen = (int)wcslen(suffix);
    if (strLen - suffixLen < 0)
    {
        return false;
    }

    auto strSuffix = str + (strLen - suffixLen);
    return _wcsnicmp(strSuffix, suffix, suffixLen) == 0;
}

// Return true if ch contains a whitespace char
bool StringUtils::IsWhiteSpaceChar(wchar_t ch)
{
    // This list doesn't need to be exhaustive. We only communicate with the language service
    return ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t';
}

// Return true if ch contains a whitespace char
void StringUtils::TrimTrailingWhiteSpace(_Inout_ LPWSTR str)
{
    if (str == nullptr)
    {
        Assert(0);
        return;
    }

    int pos = (int)wcslen(str) - 1;
    while (pos >= 0 && IsWhiteSpaceChar(str[pos]))
    {
        str[pos] = L'\0';
        --pos;
    }
}

// Creates a copy inwhich any grave accent '`' and any subsequent digits after it is removed
LPCWSTR StringUtils::RemoveGenericsGraveAccent(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc)
{
    size_t newSize = 0, strSize = wcslen(str);
    for (int q = 0; q < strSize; q++)
    {
        if (str[q] == '`')
        {
            for (q++; q < strSize && str[q] >= '0' && str[q] <= '9'; q++);
            q--;
        }
        else
        {
            newSize++;
        }
    }
    wchar_t* updatedStr = AnewArray(alloc, wchar_t, newSize + 1);
    int index = 0;
    for (int q = 0; q < strSize; q++)
    {
        if (str[q] == '`')
        {
            for (q++; q < strSize && str[q] >= '0' && str[q] <= '9'; q++);
            q--;
        }
        else
        {
            updatedStr[index] = str[q];
            index++;
        }
    }
    updatedStr[index] = L'\0';
    return (LPCWSTR) updatedStr;
}

// Checks whether a string contains a specific character or not
bool StringUtils::Contains(_In_ LPCWSTR str, char token)
{
    size_t strSize = wcslen(str);
    for (int q = 0; q < strSize; q++)
    {
        if (str[q] == token)
        {
            return true;
        }
    }

    return false;
}

// Creates a copy of the given string and returns it
LPCWSTR StringUtils::GetCopy(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc)
{
    if (!str)
    {
        return nullptr;
    }
    size_t strSize = wcslen(str);
    WCHAR* output = AnewArray(alloc, WCHAR, strSize + 1);
    for (int q = 0; q < strSize; q++)
    {
        output[q] = str[q];
    }
    output[strSize] = L'\0';
    
    return (LPCWSTR)output;
}

// Concatenate two strings and return the result
LPCWSTR StringUtils::Concat(_In_ LPCWSTR str1, _In_ LPCWSTR str2, _Inout_ ArenaAllocator* alloc)
{
    if (str1 == nullptr && str2 == nullptr)
    {
        return nullptr;
    }

    if (str1 == nullptr)
    {
        return str2;
    }

    if (str2 == nullptr)
    {
        return str1;
    }

    size_t str1Size = wcslen(str1);
    size_t str2Size = wcslen(str2);
    WCHAR* output = AnewArray(alloc, WCHAR, str1Size + str2Size + 1);

    for (int q = 0; q < str1Size; q++)
    {
        output[q] = str1[q];
    }
    for (int q = 0; q < str2Size; q++)
    {
        output[str1Size + q] = str2[q];
    }
    output[str1Size + str2Size] = L'\0';

    return (LPCWSTR)output;
}

// Returns a copy of the string with the first letter capitalized
LPCWSTR StringUtils::CapitalizeFirstLetter(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc)
{
    size_t size = wcslen(str);

    if (size == 0)
    {
        return L"";
    }

    WCHAR* output = AnewArray(alloc, WCHAR, size + 1);
    wcscpy(output, str);

    if (output[0] >= 'a' && output[0] <= 'z')
    {
        output[0] = output[0] - 'a' + 'A';
    }

    return (LPCWSTR)output;
}

// Converts an int to a string and returns the string
LPCWSTR StringUtils::IntToStr(int number, _Inout_ ArenaAllocator* alloc)
{
    int size = 0, tmpNumber = number;
    while (tmpNumber)
    {
        size++;
        tmpNumber /= 10;
    }

    WCHAR* output = AnewArray(alloc, WCHAR, size + 2);
    wsprintf(output, L"%d", number);

    return output;
}

// Checks if a string has a specific prefix
bool StringUtils::StartsWith(_In_ LPCWSTR str, _In_ LPCWSTR prefix)
{
    size_t prefixSize = wcslen(prefix);
    size_t strSize = wcslen(str);

    if (prefixSize > strSize)
    {
        return false;
    }

    return wcsncmp(str, prefix, prefixSize) == 0;
}

// Creates a copy of the string inwhich every comma has an added space after it and returns that copy
LPCWSTR StringUtils::AddSpaceAfterComma(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc)
{
    size_t numberOfCommas = 0;
    size_t strSize = wcslen(str);
    for (int q = 0; q < strSize; q++)
    {
        if (str[q] == L',')
        {
            numberOfCommas++;
        }
    }

    WCHAR* output = AnewArray(alloc, WCHAR, strSize + numberOfCommas + 1);
    int index = 0;
    for (int q = 0; q < strSize; q++)
    {
        output[index] = str[q];
        index++;

        if (str[q] == L',')
        {
            output[index] = L' ';
            index++;
        }
    }
    output[index] = L'\0';

    return output;
}

// Creates a copy of the given string without any whitespaces in it
LPCWSTR StringUtils::RemoveWhiteSpaces(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc)
{
    int strSize = (int) wcslen(str);
    int newSize = 0;
    for (int q = 0; q < strSize; q++)
    {
        if (!IsWhiteSpaceChar(str[q]))
        {
            newSize++;
        }
    }

    WCHAR* output = AnewArray(alloc, WCHAR, newSize + 1);
    int index = 0;
    for (int q = 0; q < strSize; q++)
    {
        if (!IsWhiteSpaceChar(str[q]))
        {
            output[index] = str[q];
            index++;
        }
    }
    output[index] = L'\0';

    return output;
}

// Creates a copy of the given string that replaces a given character with another character
LPCWSTR StringUtils::Replace(_In_ LPCWSTR str, char oldChar, char newChar, _Inout_ ArenaAllocator* alloc)
{
    int strSize = (int) wcslen(str);
    WCHAR* output = AnewArray(alloc, WCHAR, strSize + 1);
    for (int q = 0; q < strSize; q++)
    {
        if (str[q] == oldChar)
        {
            output[q] = newChar;
        }
        else
        {
            output[q] = str[q];
        }
    }
    output[strSize] = L'\0';

    return output;
}