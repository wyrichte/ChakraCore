//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "DelayLoadLibrary.h"

namespace Js
{
    DelayLoadLibrary::DelayLoadLibrary(__in  LPCTSTR lpLibraryName)
    {
        m_hModule = LoadLibraryEx(lpLibraryName, nullptr, 0);
    }

    DelayLoadLibrary::~DelayLoadLibrary()
    {
        FreeLibrary(m_hModule);
    }

    FARPROC DelayLoadLibrary::GetFunction(__in  LPCSTR lpFunctionName)
    {
        if (m_hModule)
        {
            return GetProcAddress(m_hModule, lpFunctionName);
        }

        return NULL;
    }

    HRESULT DelayLoadWinRtString::WindowsCreateString(__in_ecount_opt(length) const WCHAR * sourceString, UINT32 length, __out HSTRING * string)
    {
        if (m_hModule)
        {
            if (m_pfnWindowsCreateString == NULL)
            {
                m_pfnWindowsCreateString = (PFNCWindowsCreateString)GetFunction("WindowsCreateString");
                if (m_pfnWindowsCreateString == NULL)
                {
                    return E_UNEXPECTED;
                }
            }

            Assert(m_pfnWindowsCreateString != NULL);
            return m_pfnWindowsCreateString(sourceString, length, string);
        }

        return E_NOTIMPL;
    }

    HRESULT DelayLoadWinRtString::WindowsDeleteString(HSTRING string)
    {
        if (m_hModule)
        {
            if (m_pfnWindowsDeleteString == NULL)
            {
                m_pfnWindowsDeleteString = (PFNCWindowsDeleteString)GetFunction("WindowsDeleteString");
                if (m_pfnWindowsDeleteString == NULL)
                {
                    return E_UNEXPECTED;
                }
            }

            Assert(m_pfnWindowsDeleteString != NULL);
            return m_pfnWindowsDeleteString(string);
        }

        return E_NOTIMPL;
    }

    PCWSTR DelayLoadWinRtString::WindowsGetStringRawBuffer(HSTRING string, __out_opt UINT32 * length)
    {
        if (m_hModule)
        {
            if (m_pfWindowsGetStringRawBuffer == NULL)
            {
                m_pfWindowsGetStringRawBuffer = (PFNCWindowsGetStringRawBuffer)GetFunction("WindowsGetStringRawBuffer");
                if (m_pfWindowsGetStringRawBuffer == NULL)
                {
                    return L"\0";
                }
            }

            Assert(m_pfWindowsGetStringRawBuffer != NULL);
            return m_pfWindowsGetStringRawBuffer(string, length);
        }

        return L"\0";
    }

    HRESULT DelayLoadWinRtString::WindowsCompareStringOrdinal(HSTRING string1, HSTRING string2, __out INT32 * result)
    {
        if (m_hModule)
        {
            if (m_pfnWindowsCompareStringOrdinal == NULL)
            {
                m_pfnWindowsCompareStringOrdinal = (PFNCWindowsCompareStringOrdinal)GetFunction("WindowsCompareStringOrdinal");
                if (m_pfnWindowsCompareStringOrdinal == NULL)
                {
                    return E_UNEXPECTED;
                }
            }

            Assert(m_pfnWindowsCompareStringOrdinal != NULL);
            return m_pfnWindowsCompareStringOrdinal(string1,string2,result);
        }

        return E_NOTIMPL;
    }
}
