//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

//
// SCAStringWalker walks and optionally reads a SCA string value layout: byteLen content [padding]
//  T: The base class that determines skip/read and provides buffer for reading.
//
template <class T>
class SCAStringWalker: public T
{
private:
    HRESULT Walk(const StreamReader& reader, UINT byteLen);

public:
    HRESULT Walk(const StreamReader& reader, bool allowsPropertyTerminator = false);
};

//
// Walks the content part of a SCA string value layout: content [padding]
//
template <class T>
HRESULT SCAStringWalker<T>::Walk(const StreamReader& reader, UINT byteLen)
{
    HRESULT hr = S_OK;

    UINT len = byteLen / 2;
    UINT unalignedByteLen = byteLen % sizeof(DWORD);
    UINT paddingByteLen = unalignedByteLen ? sizeof(DWORD) - unalignedByteLen : 0;

    if (ShouldRead(len))
    {
        char16* buf = NULL;
        IfFailGo(EnsureSize(len, &buf));
        IfFailGo(reader.Read(buf, byteLen));
        buf[len] = NULL;

        if (paddingByteLen)
        {
            IfFailGo(reader.Skip(paddingByteLen));
        }
    }
    else
    {
        IfFailGo(reader.Skip(byteLen + paddingByteLen));
    }
Error:
    return hr;
}

//
// Walks a SCA string value layout: byteLen content [padding]
// If byteLen is SCA_PROPERTY_TERMINATOR, returns S_FALSE if allowsPropertyTerminator, otherwise fails.
//
template <class T>
HRESULT SCAStringWalker<T>::Walk(const StreamReader& reader, bool allowsPropertyTerminator)
{
    HRESULT hr = S_OK;

    DWORD byteLen;
    IfFailGo(reader.Read(&byteLen));

    if (byteLen == SCA_PROPERTY_TERMINATOR)
    {
        hr = allowsPropertyTerminator ? S_FALSE : E_SCA_DATACORRUPT;
    }
    else
    {
        UINT len;
        IfFailGo(DWordToUInt(byteLen, &len));
        hr = Walk(reader, len);
    }
Error:
    return hr;
}

//
// BSTRStringWalker walks and reads a SCA string value into a BSTR.
//
class BSTRStringWalker
{
private:
    CComBSTR m_bstr;

protected:
    bool ShouldRead(UINT /* len */) const
    {
        return true;
    }

    HRESULT EnsureSize(UINT len, _Outptr_result_buffer_(len + 1) char16** ppBuf);

    BSTR Detach()
    {
        return m_bstr.Detach();
    }

public:
    static HRESULT Read(const StreamReader& reader, BSTR* pbstr);
};

//
// SkipStringWalker walks and skips a SCA string value.
//
class SkipStringWalker
{
protected:
    bool ShouldRead(UINT /* len */) const
    {
        return false;
    }

    HRESULT EnsureSize(UINT len, _Outptr_result_buffer_(len + 1) char16** /* ppBuf */)
    {
        UNREFERENCED_PARAMETER(len);
        ATLASSERT(FALSE);
        return E_UNEXPECTED;
    }

public:
    static HRESULT Walk(const StreamReader& reader, bool allowsPropertyTerminator = false);
};

//
// A string walker for reading strings from SCA layout. It uses a small staic buffer for short
// strings, and allocates a BSTR for strings longer than its static buffer size.
//
template <UINT StaticBufLen>
class StringWalker
{
private:
    char16 m_buf[StaticBufLen]; // Static buffer
    CComBSTR m_bstr; // Dynamic buffer

    char16* m_pStr; // Current buffer
    UINT m_capacity; // Capacity of current buffer
    UINT m_length;   // Current string length

protected:
    bool ShouldRead(UINT len) const
    {
        return true;
    }

    HRESULT EnsureSize(UINT len, _Outptr_result_buffer_(len + 1) char16** ppBuf);

public:
    StringWalker()
    {
        m_pStr = m_buf;
        m_capacity = StaticBufLen - 1;
    }

    const char16* GetString() const
    {
        return m_pStr;
    }

    size_t GetLength() const
    {
        return m_length;
    }
};

//
// Ensure a buffer to hold the requested string length.
//
template <UINT StaticBufLen>
HRESULT StringWalker<StaticBufLen>::EnsureSize(UINT len, _Outptr_result_buffer_(len + 1) char16** ppBuf)
{
    if (m_capacity < len)
    {
        BSTR bstr = ::SysAllocStringLen(NULL, len);
        IfNullReturnError(bstr, E_OUTOFMEMORY);

        m_bstr.Empty();
        m_bstr.Attach(bstr);

        m_pStr = bstr;
        m_capacity = len;
    }

    *ppBuf = m_pStr;
    m_length = len;
    return S_OK;
}

const UINT DefaultStaticStringBufferLen = 256;

//
// FindStringWalker walks and finds a target string. It skips SCA string values with different
// length, ands reads string values to check if it matches target string.
//
class FindStringWalker:
    public StringWalker<DefaultStaticStringBufferLen>
{
private:
    LPCWSTR m_target;       // The target string to look for
    UINT m_targetLen;       // Length of the target string
    bool m_isLengthMatch;   // If current SCA string value length matches target string

protected:
    bool ShouldRead(UINT len)
    {
        m_isLengthMatch = (len == m_targetLen);
        return m_isLengthMatch;
    }

public:
    void SetTarget(LPCWSTR target, UINT targetLen);
    bool Found() const;
};
