//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace JsrtUnitTests
{
    class TestComponent : public IDispatch
    {
    public:
        TestComponent();

        // IUnknown
    public:
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** object);
        ULONG STDMETHODCALLTYPE AddRef();
        ULONG STDMETHODCALLTYPE Release();

        // IDispatch
    public:
        HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT * count);        
        HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT index, LCID lcid, ITypeInfo ** typeInfo);        
        HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, __in LPOLESTR * names, UINT count, LCID lcid, DISPID * dispIds);
        HRESULT STDMETHODCALLTYPE Invoke(DISPID memberId, REFIID riid, LCID lcid, WORD flags, DISPPARAMS * params, 
            VARIANT * result, EXCEPINFO * excpInfo, UINT * argError);

    public:
        HRESULT DoCallback(VARIANT * arg1, VARIANT * arg2);

    private:
        HRESULT GetProperty(DISPID id, VARIANT * value);
        HRESULT SetProperty(DISPID id, VARIANT * value);        
        HRESULT VerifyEqual(VARIANT * arg1, VARIANT * arg2, VARIANT * result);        

    private:        
        ULONG refCount;
        CComVariant * values;
        static CComBSTR properties[];
        static int propCount;
    };
}