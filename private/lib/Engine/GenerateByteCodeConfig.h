//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// Private interface and instance between jshost and jscript9 to pass generate byte code flags

// {EF523B12-FD28-460B-A32E-0D2A57E1087D}
static const GUID IID_IGenerateByteCodeConfig = 
    { 0xef523b12, 0xfd28, 0x460b, { 0xa3, 0x2e, 0xd, 0x2a, 0x57, 0xe1, 0x8, 0x7d } };

interface IGenerateByteCodeConfig : public IUnknown
{
public:
    STDMETHOD_(DWORD, GetFlags)() PURE;
};

class CGenerateByteCodeConfig sealed : public IGenerateByteCodeConfig
{
public:
    CGenerateByteCodeConfig(DWORD dwFlags) : dwFlags(dwFlags) {}

    /****************************************
        IUnknown methods
    ****************************************/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj)
    {
        if(!ppvObj)
        {
            return E_INVALIDARG;
        }
        if (IID_IGenerateByteCodeConfig == riid)
        {
            *ppvObj = static_cast<IGenerateByteCodeConfig *>(this);
            return S_OK;
        }
        if (__uuidof(IUnknown) == riid)
        {
            *ppvObj = static_cast<IUnknown *>(this);
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
    ULONG STDMETHODCALLTYPE Release(void) { return 1; }

    STDMETHOD_(DWORD, GetFlags)() { return dwFlags; }
private:
    DWORD const dwFlags;
};