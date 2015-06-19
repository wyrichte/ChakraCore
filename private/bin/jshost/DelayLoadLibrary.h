//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <roapi.h>
#include <winstring.h>

namespace Js
{
    class DelayLoadLibrary
    {
    protected:
        HMODULE m_hModule;

    public:
        DelayLoadLibrary(__in  LPCTSTR lpLibraryName);
        virtual ~DelayLoadLibrary();

        FARPROC GetFunction(__in LPCSTR lpFunctionName);
        bool IsAvailable() { return m_hModule != NULL; }
    };

    class DelayLoadWinRtString : public DelayLoadLibrary
    {
    private:
        // WinrtString specific functions
        typedef HRESULT FNCWindowsCreateString(const WCHAR *, UINT32, HSTRING *);
        typedef FNCWindowsCreateString* PFNCWindowsCreateString;
        PFNCWindowsCreateString m_pfnWindowsCreateString;

        typedef PCWSTR FNCWindowsGetStringRawBuffer(HSTRING, UINT32*);
        typedef FNCWindowsGetStringRawBuffer* PFNCWindowsGetStringRawBuffer;
        PFNCWindowsGetStringRawBuffer m_pfWindowsGetStringRawBuffer;

        typedef HRESULT FNCWindowsDeleteString(HSTRING);
        typedef FNCWindowsDeleteString* PFNCWindowsDeleteString;
        PFNCWindowsDeleteString m_pfnWindowsDeleteString;
        
        typedef HRESULT FNCWindowsCompareStringOrdinal(HSTRING,HSTRING,INT32*);
        typedef FNCWindowsCompareStringOrdinal* PFNCWindowsCompareStringOrdinal;
        PFNCWindowsCompareStringOrdinal m_pfnWindowsCompareStringOrdinal;


    public:
        DelayLoadWinRtString() : DelayLoadLibrary(L"api-ms-win-core-winrt-string-l1-1-0.dll"), m_pfnWindowsCreateString(NULL), m_pfWindowsGetStringRawBuffer(NULL), m_pfnWindowsDeleteString(NULL){ }
        virtual ~DelayLoadWinRtString() { }

        HRESULT WindowsCreateString(__in_ecount_opt(length) const WCHAR * sourceString, UINT32 length, __out HSTRING * string);
        HRESULT WindowsDeleteString(HSTRING string);
        PCWSTR WindowsGetStringRawBuffer(HSTRING string, __out_opt UINT32 * length);
        HRESULT WindowsCompareStringOrdinal(HSTRING string1, HSTRING string2, __out INT32 * result);

    };
}
