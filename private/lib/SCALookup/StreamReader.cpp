//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"

//
// Read specified number of bytes. Fail if can't read requested number of bytes.
//
HRESULT StreamReader::Read(_In_bytecount_(cb) void* pv, _In_ ULONG cb) const
{
    ULONG cbRead;
    HRESULT hr = m_pStream->Read(pv, cb, &cbRead);

    // If failed to read requested bytes, return failure
    if (SUCCEEDED(hr) && cbRead != cb)
    {
        hr = E_SCA_DATACORRUPT;
    }

    return hr;
}

//
// Read next DWORD as SCATypeId.
//
HRESULT StreamReader::Read(_Out_ SCATypeId* pTypeID) const
{
    DWORD typeId;
    HRESULT hr = Read(&typeId);
    if (SUCCEEDED(hr))
    {
        *pTypeID = static_cast<SCATypeId>(typeId);
    }
    return hr;
}

//
// Skip the next given number of bytes.
//
HRESULT StreamReader::Skip(_In_ UINT bytes) const
{
    LARGE_INTEGER li = {0};
    li.LowPart = bytes;
    return m_pStream->Seek(li, SEEK_CUR, NULL);
}

//
// Seek to a given position relative to stream beginning.
//
HRESULT StreamReader::Seek(_In_ scaposition_t pos) const
{
    LARGE_INTEGER li = {0};
    li.LowPart = pos;
    return m_pStream->Seek(li, SEEK_SET, NULL);
}

//
// Returns current position
//
HRESULT StreamReader::GetPosition(_Out_ scaposition_t *outPos) const
{
    HRESULT hr;
    ULARGE_INTEGER uli = { 0 };
    LARGE_INTEGER li = { 0 };
    IfFailGo(m_pStream->Seek(li, SEEK_CUR, &uli));
    ASSERT(uli.HighPart == 0);
    *outPos = uli.LowPart;
Error:
    return hr;
}
