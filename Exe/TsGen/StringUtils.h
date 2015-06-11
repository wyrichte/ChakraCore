//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"

namespace StringUtils
{
    bool StartsWith(_In_ LPCWSTR str, _In_ LPCWSTR prefix);
    bool EndsWith(_In_ LPCWSTR str, _In_ LPCWSTR suffix);
    bool IsWhiteSpaceChar(wchar_t ch);
    void TrimTrailingWhiteSpace(_Inout_ LPWSTR str);
    LPCWSTR RemoveGenericsGraveAccent(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc);
    bool Contains(_In_ LPCWSTR str, char token);
    LPCWSTR GetCopy(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc);
    LPCWSTR Concat(_In_ LPCWSTR str1, _In_ LPCWSTR str2, _Inout_ ArenaAllocator* alloc);
    LPCWSTR CapitalizeFirstLetter(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc);
    LPCWSTR IntToStr(int number, _Inout_ ArenaAllocator* alloc);
    LPCWSTR AddSpaceAfterComma(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc);
    LPCWSTR RemoveWhiteSpaces(_In_ LPCWSTR str, _Inout_ ArenaAllocator* alloc);
    LPCWSTR Replace(_In_ LPCWSTR str, char oldChar, char newChar, _Inout_ ArenaAllocator* alloc);
}