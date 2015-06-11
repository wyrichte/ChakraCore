//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

using namespace Microsoft::WRL;

namespace DevTests 
{
    namespace Repros
    {
        namespace VersionedProperties
        {
            inline HSTRING hs(LPCWSTR sz)
            {
                HSTRING hs;
                WindowsCreateString(sz, (UINT32)wcslen(sz), &hs);
                return hs;
            }

            class CWriteProperty :
                public Microsoft::WRL::Implements<IToReadWriteProperty, IVerifyReadWriteProperty>
            {
            public:
                CWriteProperty() : _propIntValue(0) { }
                ~CWriteProperty() { }

                IFACEMETHOD(put_TestProperty)(__in int value)
                {
                    _propIntValue = value;
                    return S_OK;
                }

                IFACEMETHOD(VerifyTestProperty)(__in int expectedValue, __deref_out boolean* result)
                {
                    if (!result) { return E_POINTER; }
                    *result = (_propIntValue == expectedValue);
                    return S_OK;
                }

            private: 
                int _propIntValue;
            };

            class CWritePropertyRuntimeClass :
                public Microsoft::WRL::RuntimeClass<CWriteProperty>
            {
            public:
                CWritePropertyRuntimeClass() { }
                ~CWritePropertyRuntimeClass() { }

                IFACEMETHOD(GetRuntimeClassName)(_Out_ HSTRING* runtimeName)  
                {  
                    *runtimeName = nullptr;  
                    HRESULT hr = S_OK;  
                    // Return type that does not exist in metadata
                    const wchar_t *name = L"DevTests.Repros.VersionedProperties.IVerifyReadWriteProperty";  
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

            class CVersionedProperty :
                public Microsoft::WRL::Implements<IReadOnlyProperty, IToReadWriteProperty>
            {
            public:
                CVersionedProperty() : _propIntValue(0) { }
                ~CVersionedProperty() { }

                IFACEMETHOD(get_TestProperty)(__deref_out int* value)
                {
                    if (!value) { return E_POINTER; }
                    *value = _propIntValue;
                    return S_OK;
                }

                IFACEMETHOD(put_TestProperty)(__in int value)
                {
                    _propIntValue = value;
                    return S_OK;
                }

            private: 
                int _propIntValue;
            };

            class VersionedProperty :
                public Microsoft::WRL::RuntimeClass<CVersionedProperty>
            {
                InspectableClass(L"DevTests.Repros.VersionedProperties.VersionedProperty", BaseTrust);
            public:
                VersionedProperty() { }
                ~VersionedProperty() { }
            };

            class VersionedPropertyFactory :
                public Microsoft::WRL::ActivationFactory<IWriteOnlyPropertyStatic>
            {
            public:
                // IActivationFactory
                IFACEMETHOD(ActivateInstance)(__deref_out IInspectable **ppInspectable)
                {
                    HRESULT hr = S_OK;
                    ComPtr<IReadOnlyProperty> spIReadOnlyProperty;
                    *ppInspectable = nullptr;

                    ComPtr<VersionedProperty> spObj = Make<VersionedProperty>();

                    hr = spObj.As(&spIReadOnlyProperty);
                    if (SUCCEEDED(hr))
                    {
                        *ppInspectable = spIReadOnlyProperty.Detach();
                    }

                    return hr;
                }
 
                // IWriteOnlyPropertyStatic
                virtual HRESULT STDMETHODCALLTYPE GetInterfaceWithReadWriteProp(__deref_out IVerifyReadWriteProperty ** result)
                {
                    if (!result)
                    {
                        return E_POINTER;
                    }
                    *result = nullptr;
                    ComPtr<CWritePropertyRuntimeClass> spWriteOnly = Make<CWritePropertyRuntimeClass>();
                    HRESULT hr = spWriteOnly ? S_OK : E_OUTOFMEMORY;
                    if (SUCCEEDED(hr))
                    {
                        *result = spWriteOnly.Detach();
                    }
                    return hr;
                }
 
            };

            class ReadOnlyVersionedProperty :
                public Microsoft::WRL::RuntimeClass<CVersionedProperty>
            {
                InspectableClass(L"DevTests.Repros.VersionedProperties.ReadOnlyVersionedProperty", BaseTrust);
            public:
                ReadOnlyVersionedProperty() { }
                ~ReadOnlyVersionedProperty() { }
            };

            class ConflictWithVersionedProperty :
                public Microsoft::WRL::RuntimeClass<IReadOnlyProperty, IToReadWriteProperty, IConflictingReadWriteProperty>
            {
                InspectableClass(L"DevTests.Repros.VersionedProperties.ConflictWithVersionedProperty", BaseTrust);
            public:
                ConflictWithVersionedProperty() : _propIntValue(0) 
                {
                    _propStringValue = hs(L"DevTests.Repros.VersionedProperties.IConflictingReadWriteProperty.testProperty");
                }
                ~ConflictWithVersionedProperty() 
                {
                    if (_propStringValue)
                        WindowsDeleteString(_propStringValue);
                }

                IFACEMETHOD(get_TestProperty)(__deref_out int* value)
                {
                    if (!value) { return E_POINTER; }
                    *value = _propIntValue;
                    return S_OK;
                }

                IFACEMETHOD(put_TestProperty)(__in int value)
                {
                    _propIntValue = value;
                    return S_OK;
                }

                IFACEMETHOD(get_TestProperty)(__deref_out HSTRING* value)
                {
                    if (!value) { return E_POINTER; }
                    return WindowsDuplicateString(_propStringValue, value);
                }

                IFACEMETHOD(put_TestProperty)(__in HSTRING value)
                {
                    if (_propStringValue)
                        WindowsDeleteString(_propStringValue);
                    return WindowsDuplicateString(value, &_propStringValue);
                }

            private: 
                int _propIntValue;
                HSTRING _propStringValue;
            };
        }
    }
}