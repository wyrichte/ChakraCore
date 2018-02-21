//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// Avoid ATL CComVariant dependency in lib. Provide the minimum set for IDispatch
class COMVariant : public tagVARIANT
{
public:
    COMVariant(IDispatch* pSrc)
    {
        vt = VT_DISPATCH;
        pdispVal = pSrc;
        // Need to AddRef as VariantClear will Release
        if (pdispVal != NULL)
            pdispVal->AddRef();
    }

    ~COMVariant()
    {
        Clear();
    }

    COMVariant& operator=(IDispatch* pSrc)
    {
        InternalClear();
        vt = VT_DISPATCH;
        pdispVal = pSrc;
        // Need to AddRef as VariantClear will Release
        if (pdispVal != NULL)
            pdispVal->AddRef();
        return *this;
    }

    HRESULT InternalClear()
    {
        HRESULT hr = Clear();
        Assert(SUCCEEDED(hr));
        if (FAILED(hr))
        {
            vt = VT_ERROR;
            scode = hr;
        }
        return hr;
    }

    HRESULT Clear() {
        if (vt == VT_BSTR_BLOB) {
            vt = VT_BSTR;
        }
        return ::VariantClear(this);
    }
};