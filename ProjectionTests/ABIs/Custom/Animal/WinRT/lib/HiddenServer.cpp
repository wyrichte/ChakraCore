#include "stdafx.h"
#include "HiddenServer.h"


using namespace Microsoft::WRL;
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;



IFACEMETHODIMP
Animals::CVisibleInterface::VisibleMethod()
{
    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_HiddenInterface_In(__in IHiddenInterface* hiddenInterface)
{
    if (nullptr == hiddenInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_HiddenClass_In(__in IHiddenInterface* hiddenInterface)
{
    if (nullptr == hiddenInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_VisibleClassWithHiddenInterfaceOnly_In(__in IHiddenInterface* hiddenInterface)
{
    if (nullptr == hiddenInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_VisibleClassWithDefaultHiddenInterface_In(__in IHiddenInterface* hiddenInterface)
{
    if (nullptr == hiddenInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_VisibleClassWithDefaultVisibleInterface_In(__in IVisibleInterface* visibleInterface)
{
    if (nullptr == visibleInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_HiddenInterface_Out(__out IHiddenInterface** hiddenInterface)
{
    if (nullptr == hiddenInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_HiddenClass_Out(__out IHiddenInterface** hiddenInterface)
{
    if (nullptr == hiddenInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_VisibleClassWithHiddenInterfaceOnly_Out(__out IHiddenInterface** hiddenInterface)
{
    if (nullptr == hiddenInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_VisibleClassWithDefaultHiddenInterface_Out(__out IHiddenInterface** hiddenInterface)
{
    if (nullptr == hiddenInterface)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::MethodUsing_VisibleClassWithDefaultVisibleInterface_Out(__out IVisibleInterface** visibleInterface)
{
    if (nullptr == visibleInterface)
    {
        return E_POINTER;
    }

    this->AddRef();
    *visibleInterface = this;

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::get_Property_HiddenInterface(IHiddenInterface** value)
{
    return this->QueryInterface(value);
}

IFACEMETHODIMP
Animals::CVisibleInterface::put_Property_HiddenInterface(IHiddenInterface* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::get_Property_HiddenClass(IHiddenInterface** value)
{
    return this->QueryInterface(value);
}

IFACEMETHODIMP
Animals::CVisibleInterface::put_Property_HiddenClass(IHiddenInterface* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::get_Property_VisibleClassWithHiddenInterfaceOnly(IHiddenInterface** value)
{
    return this->QueryInterface(value);
}

IFACEMETHODIMP
Animals::CVisibleInterface::put_Property_VisibleClassWithHiddenInterfaceOnly(IHiddenInterface* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::get_Property__VisibleClassWithDefaultHiddenInterface(IHiddenInterface** value)
{
    return this->QueryInterface(value);
}

IFACEMETHODIMP
Animals::CVisibleInterface::put_Property__VisibleClassWithDefaultHiddenInterface(IHiddenInterface* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::get_Property__VisibleClassWithDefaultVisibleInterface(IVisibleInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    this->AddRef();
    *value = this;

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::put_Property__VisibleClassWithDefaultVisibleInterface(IVisibleInterface* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_HiddenInterface_In(__in Windows::Foundation::Collections::IVector<IHiddenInterface*> * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_HiddenClass_In(__in Windows::Foundation::Collections::IVector<HiddenClass*> * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_VisibleClassWithHiddenInterfaceOnly_In(__in Windows::Foundation::Collections::IVector<VisibleClassWithHiddenInterfaceOnly*> * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_VisibleClassWithDefaultHiddenInterface_In(__in Windows::Foundation::Collections::IVector<VisibleClassWithDefaultHiddenInterface*> * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_VisibleClassWithDefaultVisibleInterface_In(__in Windows::Foundation::Collections::IVector<VisibleClassWithDefaultVisibleInterface*> * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_HiddenInterface_Out(__in Windows::Foundation::Collections::IVector<IHiddenInterface*> ** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_HiddenClass_Out(__in Windows::Foundation::Collections::IVector<HiddenClass*> ** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_VisibleClassWithHiddenInterfaceOnly_Out(__in Windows::Foundation::Collections::IVector<VisibleClassWithHiddenInterfaceOnly*> ** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_VisibleClassWithDefaultHiddenInterface_Out(__in Windows::Foundation::Collections::IVector<VisibleClassWithDefaultHiddenInterface*> ** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Vector_VisibleClassWithDefaultVisibleInterface_Out(__in Windows::Foundation::Collections::IVector<VisibleClassWithDefaultVisibleInterface*> ** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::PassArray_HiddenInterface(__in UINT32 length,  __RPC__in_ecount_full(length) IHiddenInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::FillArray_HiddenInterface(__in UINT32 length,  __RPC__out_ecount_full(length) IHiddenInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::ReceiveArray_HiddenInterface(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IHiddenInterface*** value)
{
    if ((nullptr == length) || (nullptr == value))
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::PassArray_VisibleClassWithHiddenInterfaceOnly(__in UINT32 length,  __RPC__in_ecount_full(length) IHiddenInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::FillArray_VisibleClassWithHiddenInterfaceOnly(__in UINT32 length,  __RPC__out_ecount_full(length) IHiddenInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::ReceiveArray_VisibleClassWithHiddenInterfaceOnly(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IHiddenInterface*** value)
{
    if ((nullptr == length) || (nullptr == value))
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::PassArray_VisibleClassWithDefaultHiddenInterface(__in UINT32 length,  __RPC__in_ecount_full(length) IHiddenInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::FillArray_VisibleClassWithDefaultHiddenInterface(__in UINT32 length,  __RPC__out_ecount_full(length) IHiddenInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::ReceiveArray_VisibleClassWithDefaultHiddenInterface(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IHiddenInterface*** value)
{
    if ((nullptr == length) || (nullptr == value))
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::PassArray_VisibleClassWithDefaultVisibleInterface(__in UINT32 length,  __RPC__in_ecount_full(length) IVisibleInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::FillArray_VisibleClassWithDefaultVisibleInterface(__in UINT32 length,  __RPC__out_ecount_full(length) IVisibleInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::ReceiveArray_VisibleClassWithDefaultVisibleInterface(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IVisibleInterface*** value)
{
    if ((nullptr == length) || (nullptr == value))
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::PassArray_HiddenClass(__in UINT32 length,  __RPC__in_ecount_full(length) IHiddenInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::FillArray_HiddenClass(__in UINT32 length,  __RPC__out_ecount_full(length) IHiddenInterface** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    if (length == 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::ReceiveArray_HiddenClass(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IHiddenInterface*** value)
{
    if ((nullptr == length) || (nullptr == value))
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::HiddenStruct_In(__in HiddenStruct value)
{
    hiddenStruct.HiddenStructMember = value.HiddenStructMember;

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::HiddenStruct_Out(__out HiddenStruct* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    *value = hiddenStruct;

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::StructWithHiddenInnerStruct_In(__in StructWithHiddenInnerStruct value)
{
    structWithHiddenInnerStruct.VisibleStructMember = value.VisibleStructMember;
    structWithHiddenInnerStruct.HiddenInnerStruct = value.HiddenInnerStruct;

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::StructWithHiddenInnerStruct_Out(__out StructWithHiddenInnerStruct* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    *value = structWithHiddenInnerStruct;

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_DelegateUsing_HiddenInterface_Out(__in IDelegateUsing_HiddenInterface_Out* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    IHiddenInterface* hiddenInterface;
    HRESULT hr = value->Invoke(&hiddenInterface);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_DelegateUsing_HiddenClass_Out(__in IDelegateUsing_HiddenClass_Out* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    IHiddenInterface* hiddenInterface;
    HRESULT hr = value->Invoke(&hiddenInterface);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out(__in IDelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    IHiddenInterface* hiddenInterface;
    HRESULT hr = value->Invoke(&hiddenInterface);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out(__in IDelegateUsing_VisibleClassWithDefaultHiddenInterface_Out* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    IHiddenInterface* hiddenInterface;
    HRESULT hr = value->Invoke(&hiddenInterface);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_DelegateUsing_HiddenInterface_In(__in IDelegateUsing_HiddenInterface_In* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    IHiddenInterface* hiddenInterface = nullptr;
    HRESULT hr = value->Invoke(hiddenInterface);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_DelegateUsing_HiddenClass_In(__in IDelegateUsing_HiddenClass_In* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    IHiddenInterface* hiddenInterface = nullptr;
    HRESULT hr = value->Invoke(hiddenInterface);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In(__in IDelegateUsing_VisibleClassWithHiddenInterfaceOnly_In* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    IHiddenInterface* hiddenInterface = nullptr;
    HRESULT hr = value->Invoke(hiddenInterface);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In(__in IDelegateUsing_VisibleClassWithDefaultHiddenInterface_In* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    IHiddenInterface* hiddenInterface = nullptr;
    HRESULT hr = value->Invoke(hiddenInterface);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Call_HiddenDelegate(__in IHiddenDelegate* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    // try to invoke the delegate
    HRESULT hr = value->Invoke(111);

    return hr;
}

IFACEMETHODIMP
Animals::CVisibleInterface::Get_HiddenDelegate(__in IHiddenDelegate** value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    *value = nullptr;
    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::HiddenEnum_In(__in HiddenEnum value)
{
    if (value > 2)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::HiddenEnum_Out(__out HiddenEnum* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::HiddenOverload1(__out HiddenEnum* value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::CVisibleInterface::HiddenOverload2(__in int inValue, __out int* outValue)
{
    if (nullptr == outValue)
    {
        return E_POINTER;
    }

    *outValue = inValue;

    return S_OK;
}
