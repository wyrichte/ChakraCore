//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

//
// Helper class to read a stream.
//
class StreamReader
{
private:
    CComPtr<IStream> m_pStream;

public:
    StreamReader(_In_ IStream* pStream)
        : m_pStream(pStream)
    {
    }

    HRESULT Read(_In_bytecount_(cb) void* pv, _In_ ULONG cb) const;

    template <class T>
    HRESULT Read(_In_ T* pv) const
    {
        return Read(pv, sizeof(T));
    }

    HRESULT Read(_Out_ SCATypeId* pTypeID) const;
    HRESULT Skip(_In_ UINT bytes) const;
    HRESULT Seek(_In_ scaposition_t pos) const;
    HRESULT GetPosition(_Out_ scaposition_t *outPos) const;
};
