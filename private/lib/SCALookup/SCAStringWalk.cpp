//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "SCALookupPch.h"

//
// Allocates a BSTR buffer to contain a string value of given length.
//  len:    Requested string length (not including NULL terminator).
//  ppBuf:  Contains the buffer pointer on successful return. The buffer can hold
//          len number of characters plus a NULL terminator and is owned by BSTRStringWalker.
//          Caller can write to this buffer.
//
HRESULT BSTRStringWalker::EnsureSize(UINT len, _Outptr_result_buffer_(len + 1) wchar_t** ppBuf)
{
    BSTR bstr = ::SysAllocStringLen(NULL, len);
    IfNullReturnError(bstr, E_OUTOFMEMORY);

    m_bstr.Empty();
    m_bstr.Attach(bstr); // m_bstr owns allocated BSTR

    *ppBuf = bstr;
    return S_OK;
}

//
// Reads the current SCA string value from a SCA stream into a BSTR.
//
HRESULT BSTRStringWalker::Read(const StreamReader& reader, BSTR* pbstr)
{
    SCAStringWalker<BSTRStringWalker> walker;

    HRESULT hr = walker.Walk(reader);
    if (SUCCEEDED(hr))
    {
        *pbstr = walker.Detach(); // Detach and return BSTRStringWalker owned BSTR
    }

    return hr;
}

//
// Walks and skips the current SCA string value in the stream.
//
HRESULT SkipStringWalker::Walk(const StreamReader& reader, bool allowsPropertyTerminator)
{
    return SCAStringWalker<SkipStringWalker>().Walk(reader, allowsPropertyTerminator);
}

//
// Set the target string to look for.
//
void FindStringWalker::SetTarget(LPCWSTR target, UINT targetLen)
{
    m_target = target;
    m_targetLen = targetLen;
    m_isLengthMatch = false;
}

//
// Check if the target string was found after walk.
//
bool FindStringWalker::Found() const
{
    return m_isLengthMatch
        && wmemcmp(GetString(), m_target, m_targetLen) == 0;
}
