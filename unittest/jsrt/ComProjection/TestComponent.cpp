//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace JsrtUnitTests
{
    CComBSTR TestComponent::properties[] = { L"foo", L"bar", L"callback" };
    int TestComponent::propCount = sizeof(properties) / sizeof(properties[0]);

    TestComponent::TestComponent() :
        refCount(1)
    {        
        this->values = new CComVariant[propCount];    
    }

    HRESULT TestComponent::QueryInterface(REFIID riid, void ** object)
    {
        if (IsEqualIID(riid, __uuidof(IUnknown)))
        {
            *object = static_cast<IUnknown *>(this);
            this->AddRef();
            return S_OK;
        }

        if (IsEqualIID(riid, __uuidof(IDispatch)))
        {
            *object = static_cast<IDispatch *>(this);
            this->AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG TestComponent::AddRef()
    {
        return ++this->refCount;
    }

    ULONG TestComponent::Release()
    {
        if (--this->refCount == 0)
        {
            delete this;
            return 0;
        }

        return this->refCount;
    }

    HRESULT TestComponent::GetTypeInfoCount(UINT * count)
    {
        *count = 0;
        return S_OK;
    }

    HRESULT TestComponent::GetTypeInfo(UINT /* index */, LCID /* lcid */, ITypeInfo ** /* typeInfo */)
    {
        return E_NOTIMPL;
    }

    HRESULT TestComponent::GetIDsOfNames(REFIID /* riid */, __in LPOLESTR * names, UINT count, LCID /* lcid */, DISPID * dispIds)
    {
        CComBSTR member = names[0];

        if (count != 1)
        {
            return E_NOTIMPL;
        }       

        for (int i = 0; i < propCount; i++)
        {
            if (member == properties[i])
            {
                dispIds[0] = i + 1;
                return S_OK;
            }
        }

        if (member == L"verifyEqual")
        {
            dispIds[0] = propCount + 1;
            return S_OK;
        }

        return DISP_E_UNKNOWNNAME;
    }

    HRESULT TestComponent::Invoke(DISPID memberId, REFIID /* riid */, LCID /* lcid */, WORD flags, DISPPARAMS * params, 
        VARIANT * result, EXCEPINFO * /* excpInfo */, UINT * /* argError */)
    {        
        if (memberId > 0 && memberId <= propCount)
        {            
            if ((flags & DISPATCH_PROPERTYGET) == DISPATCH_PROPERTYGET)
            {
                return this->GetProperty(memberId, result);
            }
            else if ((flags & DISPATCH_PROPERTYPUT) == DISPATCH_PROPERTYPUT)
            {
                return this->SetProperty(memberId, &params->rgvarg[0]);
            }
            
            return DISP_E_MEMBERNOTFOUND;
        }

        if (memberId == propCount + 1)
        {
            if (params->cArgs != 2)
            {
                return DISP_E_BADPARAMCOUNT;
            }

            return this->VerifyEqual(&params->rgvarg[1], &params->rgvarg[0], result);
        }

        return DISP_E_MEMBERNOTFOUND;
    }

    HRESULT TestComponent::GetProperty(DISPID id, VARIANT * value)
    {                      
        this->values[id - 1].CopyTo(value);
        return S_OK;
    }

    HRESULT TestComponent::SetProperty(DISPID id, VARIANT * value)
    {                      
        this->values[id - 1] = *value;
        return S_OK;
    }

    HRESULT TestComponent::VerifyEqual(VARIANT * arg1, VARIANT * arg2, VARIANT * result)
    {                       
        VARIANT_BOOL b = (arg1->vt == arg2->vt && VariantCompare(*arg1, *arg2) == 0) ? VARIANT_TRUE : VARIANT_FALSE;

        if (result != NULL)
        {
            result->vt = VT_BOOL;
            result->boolVal = b;
        }

        VERIFY_IS_TRUE(b == VARIANT_TRUE);

        return S_OK;
    }

    HRESULT TestComponent::DoCallback(VARIANT * arg1, VARIANT * arg2)
    {         
        VERIFY_IS_TRUE(this->values->vt == VT_DISPATCH);        
        
        IDispatch * callback = this->values[2].pdispVal;
        DISPID id;
        OLECHAR * names = L"call";

        VERIFY_SUCCEEDED(callback->GetIDsOfNames(IID_NULL, &names, 1, LOCALE_SYSTEM_DEFAULT, &id));
        DISPPARAMS params;
        VARIANT empty;
        VariantInit(&empty);
        VARIANT args[3] = { *arg2, *arg1, empty };
        VARIANT result;

        params.cArgs = 3;
        params.rgvarg = args;
        params.cNamedArgs = 0;
        params.rgdispidNamedArgs = NULL;
        
        VERIFY_SUCCEEDED(callback->Invoke(id, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD , &params, &result, NULL, NULL));
        
        return S_OK;
    }
}
