#include "stdafx.h"

namespace Animals
{
    class CHiddenInterface : public Microsoft::WRL::Implements<IHiddenInterface>
    {
    private:
        int m_value;

    public:
        CHiddenInterface() { m_value = 20; }
        ~CHiddenInterface() { }

        IFACEMETHOD(HiddenMethod)() override
        {
            return S_OK;
        }

        IFACEMETHOD(get_Prop1)(__out int *value)
        {
            if (value == nullptr)
            {
                return E_POINTER;
            }
            *value = m_value;
            return S_OK;
        }

        IFACEMETHOD(put_Prop1)(__in int value)
        {
            m_value = value;
            return S_OK;
        }
    };


    class CRequiredHiddenInterface : public Microsoft::WRL::Implements<IRequiredHiddenInterface>
    {
    private:
        int m_value;

    public:
        CRequiredHiddenInterface() { m_value = 40; }
        ~CRequiredHiddenInterface() { }

        IFACEMETHOD(RequiredHiddenMethod)() override
        {
            return S_OK;
        }

        IFACEMETHOD(get_Prop2)(__out int *value)
        {
            if (value == nullptr)
            {
                return E_POINTER;
            }
            *value = m_value;
            return S_OK;
        }

        IFACEMETHOD(put_Prop2)(__in int value)
        {
            m_value = value;
            return S_OK;
        }
    };

    class CVisibleInterface : public Microsoft::WRL::Implements<IVisibleInterface>
    {
        HiddenStruct hiddenStruct;
        StructWithHiddenInnerStruct structWithHiddenInnerStruct;

    public:
        CVisibleInterface() 
        {  
            hiddenStruct.HiddenStructMember = 50; 
            structWithHiddenInnerStruct.VisibleStructMember = 150;
            structWithHiddenInnerStruct.HiddenInnerStruct.HiddenStructMember = 200;
        }
        ~CVisibleInterface() { }
    
        IFACEMETHOD(VisibleMethod)() override;

        IFACEMETHOD(MethodUsing_HiddenInterface_In)(__in IHiddenInterface* _in) override;
        IFACEMETHOD(MethodUsing_HiddenClass_In)(__in IHiddenInterface* _in) override;
        IFACEMETHOD(MethodUsing_VisibleClassWithHiddenInterfaceOnly_In)(__in IHiddenInterface* _in) override;
        IFACEMETHOD(MethodUsing_VisibleClassWithDefaultHiddenInterface_In)(__in IHiddenInterface* _in) override;
        IFACEMETHOD(MethodUsing_VisibleClassWithDefaultVisibleInterface_In)(__in IVisibleInterface* _in) override;

        IFACEMETHOD(MethodUsing_HiddenInterface_Out)(__out IHiddenInterface** _out) override;
        IFACEMETHOD(MethodUsing_HiddenClass_Out)(__out IHiddenInterface** _out) override;
        IFACEMETHOD(MethodUsing_VisibleClassWithHiddenInterfaceOnly_Out)(__out IHiddenInterface** _out) override;
        IFACEMETHOD(MethodUsing_VisibleClassWithDefaultHiddenInterface_Out)(__out IHiddenInterface** _out) override;
        IFACEMETHOD(MethodUsing_VisibleClassWithDefaultVisibleInterface_Out)(__out IVisibleInterface** _out) override;

        IFACEMETHOD(get_Property_HiddenInterface)(__out IHiddenInterface** value) override;
        IFACEMETHOD(put_Property_HiddenInterface)(__in IHiddenInterface* value) override;
        IFACEMETHOD(get_Property_HiddenClass)(__out IHiddenInterface** value) override;
        IFACEMETHOD(put_Property_HiddenClass)(__in IHiddenInterface* value) override;
        IFACEMETHOD(get_Property_VisibleClassWithHiddenInterfaceOnly)(__out IHiddenInterface** value) override;
        IFACEMETHOD(put_Property_VisibleClassWithHiddenInterfaceOnly)(__in IHiddenInterface* value) override;
        IFACEMETHOD(get_Property__VisibleClassWithDefaultHiddenInterface)(__out IHiddenInterface** value) override;
        IFACEMETHOD(put_Property__VisibleClassWithDefaultHiddenInterface)(__in IHiddenInterface* value) override;
        IFACEMETHOD(get_Property__VisibleClassWithDefaultVisibleInterface)(__out IVisibleInterface** value) override;
        IFACEMETHOD(put_Property__VisibleClassWithDefaultVisibleInterface)(__in IVisibleInterface* value) override;

        IFACEMETHOD(Vector_HiddenInterface_In)(__in Windows::Foundation::Collections::IVector<IHiddenInterface*> * value) override;
        IFACEMETHOD(Vector_HiddenClass_In)(__in Windows::Foundation::Collections::IVector<HiddenClass*> * value) override;
        IFACEMETHOD(Vector_VisibleClassWithHiddenInterfaceOnly_In)(__in Windows::Foundation::Collections::IVector<VisibleClassWithHiddenInterfaceOnly*> * value) override;
        IFACEMETHOD(Vector_VisibleClassWithDefaultHiddenInterface_In)(__in Windows::Foundation::Collections::IVector<VisibleClassWithDefaultHiddenInterface*> * value) override;
        IFACEMETHOD(Vector_VisibleClassWithDefaultVisibleInterface_In)(__in Windows::Foundation::Collections::IVector<VisibleClassWithDefaultVisibleInterface*> * value) override;

        IFACEMETHOD(Vector_HiddenInterface_Out)(__out Windows::Foundation::Collections::IVector<IHiddenInterface*> ** value) override;
        IFACEMETHOD(Vector_HiddenClass_Out)(__out Windows::Foundation::Collections::IVector<HiddenClass*> ** value) override;
        IFACEMETHOD(Vector_VisibleClassWithHiddenInterfaceOnly_Out)(__out Windows::Foundation::Collections::IVector<VisibleClassWithHiddenInterfaceOnly*> ** value) override;
        IFACEMETHOD(Vector_VisibleClassWithDefaultHiddenInterface_Out)(__out Windows::Foundation::Collections::IVector<VisibleClassWithDefaultHiddenInterface*> ** value) override;
        IFACEMETHOD(Vector_VisibleClassWithDefaultVisibleInterface_Out)(__out Windows::Foundation::Collections::IVector<VisibleClassWithDefaultVisibleInterface*> ** value) override;

        IFACEMETHOD(PassArray_HiddenInterface)(__in UINT32 length,  __RPC__in_ecount_full(length) IHiddenInterface** value) override;
        IFACEMETHOD(FillArray_HiddenInterface)(__in UINT32 length,  __RPC__out_ecount_full(length) IHiddenInterface** value) override;
        IFACEMETHOD(ReceiveArray_HiddenInterface)(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IHiddenInterface*** value) override;

        IFACEMETHOD(PassArray_HiddenClass)(__in UINT32 length,  __RPC__in_ecount_full(length) IHiddenInterface** value) override;
        IFACEMETHOD(FillArray_HiddenClass)(__in UINT32 length,  __RPC__out_ecount_full(length) IHiddenInterface** value) override;
        IFACEMETHOD(ReceiveArray_HiddenClass)(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IHiddenInterface*** value) override;

        IFACEMETHOD(PassArray_VisibleClassWithHiddenInterfaceOnly)(__in UINT32 length,  __RPC__in_ecount_full(length) IHiddenInterface** value) override;
        IFACEMETHOD(FillArray_VisibleClassWithHiddenInterfaceOnly)(__in UINT32 length,  __RPC__out_ecount_full(length) IHiddenInterface** value) override;
        IFACEMETHOD(ReceiveArray_VisibleClassWithHiddenInterfaceOnly)(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IHiddenInterface*** value) override;

        IFACEMETHOD(PassArray_VisibleClassWithDefaultHiddenInterface)(__in UINT32 length,  __RPC__in_ecount_full(length) IHiddenInterface** value) override;
        IFACEMETHOD(FillArray_VisibleClassWithDefaultHiddenInterface)(__in UINT32 length,  __RPC__out_ecount_full(length) IHiddenInterface** value) override;
        IFACEMETHOD(ReceiveArray_VisibleClassWithDefaultHiddenInterface)(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IHiddenInterface*** value) override;

        IFACEMETHOD(PassArray_VisibleClassWithDefaultVisibleInterface)(__in UINT32 length,  __RPC__in_ecount_full(length) IVisibleInterface** value) override;
        IFACEMETHOD(FillArray_VisibleClassWithDefaultVisibleInterface)(__in UINT32 length,  __RPC__out_ecount_full(length) IVisibleInterface** value) override;
        IFACEMETHOD(ReceiveArray_VisibleClassWithDefaultVisibleInterface)(__RPC__out UINT32* length,  __RPC__deref_out_ecount_full_opt(*length) IVisibleInterface*** value) override;

        IFACEMETHOD(HiddenStruct_In)(__in HiddenStruct value) override;
        IFACEMETHOD(HiddenStruct_Out)(__out HiddenStruct* value) override;
        IFACEMETHOD(StructWithHiddenInnerStruct_In)(__in StructWithHiddenInnerStruct value) override;
        IFACEMETHOD(StructWithHiddenInnerStruct_Out)(__out StructWithHiddenInnerStruct* value) override;

        IFACEMETHOD(Call_DelegateUsing_HiddenInterface_Out)(__in IDelegateUsing_HiddenInterface_Out* value) override;
        IFACEMETHOD(Call_DelegateUsing_HiddenClass_Out)(__in IDelegateUsing_HiddenClass_Out* value) override;
        IFACEMETHOD(Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out)(__in IDelegateUsing_VisibleClassWithHiddenInterfaceOnly_Out* value) override;
        IFACEMETHOD(Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_Out)(__in IDelegateUsing_VisibleClassWithDefaultHiddenInterface_Out* value) override;
        IFACEMETHOD(Call_DelegateUsing_HiddenInterface_In)(__in IDelegateUsing_HiddenInterface_In* value) override;
        IFACEMETHOD(Call_DelegateUsing_HiddenClass_In)(__in IDelegateUsing_HiddenClass_In* value) override;
        IFACEMETHOD(Call_DelegateUsing_VisibleClassWithHiddenInterfaceOnly_In)(__in IDelegateUsing_VisibleClassWithHiddenInterfaceOnly_In* value) override;
        IFACEMETHOD(Call_DelegateUsing_VisibleClassWithDefaultHiddenInterface_In)(__in IDelegateUsing_VisibleClassWithDefaultHiddenInterface_In* value) override;

        IFACEMETHOD(Call_HiddenDelegate)(__in IHiddenDelegate* value) override;
        IFACEMETHOD(Get_HiddenDelegate)(__out IHiddenDelegate** value) override;

        IFACEMETHOD(HiddenEnum_In)(__in HiddenEnum value) override;
        IFACEMETHOD(HiddenEnum_Out)(__out HiddenEnum* value) override;

        IFACEMETHOD(HiddenOverload1)(__out HiddenEnum* value) override;
        IFACEMETHOD(HiddenOverload2)(__in int inValue, __out int* outvalue) override;
    };


    class HiddenClassServer : public Microsoft::WRL::RuntimeClass<CHiddenInterface>
    {
        InspectableClass(L"Animals.HiddenClass", BaseTrust);

    public:
        HiddenClassServer() { }
        ~HiddenClassServer() { }
    };

    class VisibleClassWithHiddenInterfaceOnlyServer: public Microsoft::WRL::RuntimeClass<CHiddenInterface>
    {
        InspectableClass(L"Animals.VisibleClassWithHiddenInterfaceOnly", BaseTrust);

    public:
        VisibleClassWithHiddenInterfaceOnlyServer() { }
        ~VisibleClassWithHiddenInterfaceOnlyServer() { }
    };

    class VisibleClassWithDefaultHiddenInterfaceServer : public Microsoft::WRL::RuntimeClass<CVisibleInterface, CHiddenInterface, CRequiredHiddenInterface>
    {
        InspectableClass(L"Animals.VisibleClassWithDefaultHiddenInterface", BaseTrust);
    public:
        VisibleClassWithDefaultHiddenInterfaceServer() { }
        ~VisibleClassWithDefaultHiddenInterfaceServer() { }
    };

    class VisibleClassWithDefaultVisibleInterfaceServer : public Microsoft::WRL::RuntimeClass<CVisibleInterface, CHiddenInterface, CRequiredHiddenInterface>
    {
        InspectableClass(L"Animals.VisibleClassWithDefaultVisibleInterface", BaseTrust);

    public:
        VisibleClassWithDefaultVisibleInterfaceServer() { }
        ~VisibleClassWithDefaultVisibleInterfaceServer() { }
    };

    class VisibleClassWithVisibleInterfaceAndHiddenStaticInterfaceServer : public Microsoft::WRL::RuntimeClass<CVisibleInterface, CRequiredHiddenInterface>
    {
        InspectableClass(L"Animals.VisibleClassWithVisibleInterfaceAndHiddenStaticInterface", BaseTrust);

    public:
        VisibleClassWithVisibleInterfaceAndHiddenStaticInterfaceServer() { }
        ~VisibleClassWithVisibleInterfaceAndHiddenStaticInterfaceServer() { }
    };

    class VisibleClassWithVisibleInterfaceAndHiddenStaticInterfaceFactory :
		public Microsoft::WRL::ActivationFactory<CHiddenInterface>
	{
	public:
        // IActivationFactory
		IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable) {
			HRESULT hr = S_OK;

            ComPtr<IInspectable> spI2;
			*ppInspectable = nullptr;
			ComPtr<VisibleClassWithVisibleInterfaceAndHiddenStaticInterfaceServer> spObj = Make<VisibleClassWithVisibleInterfaceAndHiddenStaticInterfaceServer>();

			hr = spObj.As(&spI2);
			if (SUCCEEDED(hr))
			{
				*ppInspectable = spI2.Detach();
			}

			return hr;
		};
	};
}
