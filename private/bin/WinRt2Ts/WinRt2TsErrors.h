//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "Errors.h"

class MetadataDispenserError : public ErrorBase
{
public:
    MetadataDispenserError(HRESULT hr) : ErrorBase(ErrorCodeMetadataDispenser), m_hresult(hr) {}

    std::wstring Description() override
    {
        wchar_t formattedIntBuf[100];
        swprintf_s(formattedIntBuf, L"0x%x", m_hresult);
        return std::wstring(L"Cannot create metadata dispenser, hresult: ") + formattedIntBuf;
    }

private:
    HRESULT m_hresult;
};

class WinmdReadError : public ErrorBase
{
public:
    WinmdReadError(HRESULT hr, LPCWSTR winmd) : ErrorBase(ErrorCodeWinmdRead), m_hresult(hr), m_winmd(winmd) {}

    std::wstring Description() override
    {
        wchar_t formattedIntBuf[100];
        swprintf_s(formattedIntBuf, L"0x%x", m_hresult);
        return L"Cannot import metadata from " + m_winmd + L", hresult: " + formattedIntBuf;
    }

private:
    std::wstring m_winmd;
    HRESULT m_hresult;
};

class UnknownError : public ErrorBase
{
public:
    UnknownError(HRESULT hr) : ErrorBase(ErrorCodeMetadataDispenser), m_hresult(hr) {}

    std::wstring Description() override
    {
        wchar_t formattedIntBuf[100];
        swprintf_s(formattedIntBuf, L"0x%x", m_hresult);
        return std::wstring(L"Unknown error hresult: ") + formattedIntBuf;
    }

private:
    HRESULT m_hresult;
};