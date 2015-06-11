//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

using namespace Microsoft::WRL;

namespace DevTests 
{
    namespace Repros
    {
        namespace InterfaceOutFastPath
        {
            class COutInterface : 
                public Microsoft::WRL::Implements<IOutInterface>
            {
            public:
                IFACEMETHOD(TestMethod)(__deref_out boolean * result)
                {
                    *result = true;
                    return S_OK;
                }
            };

            class COutInterfaceRuntimeClass :
                public Microsoft::WRL::RuntimeClass<COutInterface>
            {
            public:
                COutInterfaceRuntimeClass() { }
                ~COutInterfaceRuntimeClass() { }

                IFACEMETHOD(GetRuntimeClassName)(_Out_ HSTRING* runtimeName)  
                {  
                    *runtimeName = nullptr;  
                    HRESULT hr = S_OK;  
                    // Return type that does not exist in metadata
                    const wchar_t *name = L"DevTests.Repros.InterfaceOutFastPath.OutInterface";  
                    hr = WindowsCreateString(name, static_cast<UINT32>(::wcslen(name)), runtimeName);  
                    return hr;  
                }  
                IFACEMETHOD(GetTrustLevel)(_Out_ TrustLevel* trustLvl)  
                {  
                    *trustLvl = BaseTrust;  
                    return S_OK;  
                }  
                IFACEMETHOD(GetIids)(_Out_ ULONG *iidCount, _Outptr_result_buffer_(*iidCount) IID **iids)  
                {  
                    return E_NOTIMPL;
                }
            };

            class TestsFactory :
                public Microsoft::WRL::ActivationFactory<ITestInterface>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable)
                {
                    *ppInspectable = nullptr;
                    return E_NOTIMPL;
                }
 
                // ITestInterface
                virtual HRESULT STDMETHODCALLTYPE InterfaceOutTest(__deref_out IOutInterface ** result)
                {
                    if (!result)
                    {
                        return E_POINTER;
                    }
                    *result = nullptr;
                    ComPtr<COutInterfaceRuntimeClass> spOutInterface = Make<COutInterfaceRuntimeClass>();
                    HRESULT hr = spOutInterface ? S_OK : E_OUTOFMEMORY;
                    if (SUCCEEDED(hr))
                    {
                        *result = spOutInterface.Detach();
                    }
                    return hr;
                }
 
            };

            class TestsServer :
                public Microsoft::WRL::RuntimeClass<IInspectable>
            {
                InspectableClass(L"DevTests.Repros.InterfaceOutFastPath.Tests", BaseTrust);
            };
        }
    }
}