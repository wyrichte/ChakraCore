//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// Avoid ATL CComStr dependency in lib.
class AutoBSTR : public BasePtr<OLECHAR>
{
public:
    AutoBSTR(BSTR ptr = nullptr) : BasePtr(ptr) {}

    ~AutoBSTR()
    {
        Release();
    }

    void Release()
    {
        if (ptr != nullptr)
        {
            ::SysFreeString(ptr);
            this->ptr = nullptr;
        }
    }

    BSTR Copy() const
    {
        return ::SysAllocStringLen(ptr, ::SysStringLen(ptr));
    }

    HRESULT CopyTo(BSTR* pbstr)
    {
        if (pbstr == NULL)
            return E_POINTER;
        *pbstr = ::SysAllocStringLen(ptr, ::SysStringLen(ptr));
        if (*pbstr == NULL)
            return E_OUTOFMEMORY;
        return S_OK;
    }

    HRESULT Append(const AutoBSTR& bstrSrc)
    {
        return Append(bstrSrc.ptr, SysStringLen(bstrSrc.ptr));
    }

    HRESULT Append(LPCOLESTR lpsz)
    {
        return Append(lpsz, static_cast<int>(wcslen(lpsz)));
    }

    HRESULT Append(LPCOLESTR lpsz, int nLen)
    {
        if (lpsz == NULL)
        {
            if (nLen != 0)
                return E_INVALIDARG;
            else
                return S_OK;
        }

        int n1 = Length();
        if (n1 + nLen < n1)
            return E_OUTOFMEMORY;

        BSTR b;
        b = ::SysAllocStringLen(NULL, n1 + nLen);
        if (b == NULL)
            return E_OUTOFMEMORY;

        if (ptr != NULL)
            memcpy(b, ptr, n1 * sizeof(OLECHAR));

        memcpy(b + n1, lpsz, nLen * sizeof(OLECHAR));
        b[n1 + nLen] = NULL;
        SysFreeString(ptr);
        ptr = b;
        return S_OK;
    }

    unsigned int Length() const
    {
        return (ptr == NULL) ? 0 : SysStringLen(ptr);
    }

    AutoBSTR& operator=(const AutoBSTR& src)
    {
        if (ptr != src.ptr)
        {
            if (ptr)
                ::SysFreeString(ptr);
            ptr = src.Copy();
        }
        return *this;
    }

    AutoBSTR& operator=(const BSTR* psrc)
    {
        if (!psrc) {
            if (ptr) {
                ::SysFreeString(ptr);
                ptr = NULL;
            }
        }
        else if (*psrc != ptr) {
            if (ptr) {
                ::SysFreeString(ptr);
            }
            ptr = ::SysAllocStringByteLen(reinterpret_cast<LPCSTR>(*psrc), ::SysStringByteLen(*psrc));
        }
        return *this;
    }

    AutoBSTR& operator=(LPCOLESTR pSrc)
    {
        ::SysFreeString(ptr);
        ptr = ::SysAllocString(pSrc);
        return *this;
    }

    BSTR* operator&()
    {
        return &ptr;
    }

    operator BSTR() const
    {
        return ptr;
    }
};