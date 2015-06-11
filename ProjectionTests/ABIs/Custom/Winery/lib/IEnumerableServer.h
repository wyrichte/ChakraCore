//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdafx.h>
#include "winery.h"

namespace Winery
{
    namespace IEnumerable
    {
        inline HSTRING hs(LPCWSTR sz)
        {
            HSTRING hs;
            WindowsCreateString(sz, (UINT32)wcslen(sz), &hs);
            return hs;
        }

        namespace EnumerableOfDefaultInterface
        {
            class EnumerableOfDefaultInterface_IMethod;

            [uuid("A0B71A26-4EEC-49D0-BCA6-97B52A2B21DC")]
            class EnumerableOfDefaultInterfaceAccessServer :
                public Microsoft::WRL::RuntimeClass<
                    Winery::IEnumerable::EnumerableOfDefaultInterface::IMethod,
                    Winery::IEnumerable::EnumerableOfDefaultInterface::IMethodColl,
                    Windows::Foundation::Collections::IIterable<Winery::IEnumerable::EnumerableOfDefaultInterface::IMethod *>
                >
            {
            private:
                int index;

            public:
                InspectableClass(L"Winery.IEnumerable.EnumerableOfDefaultInterface.Access", BaseTrust);

            public:
                HRESULT STDMETHODCALLTYPE SetIndex(int newIndex, HSTRING * result)
                {
                    this->index = newIndex;

                    if (result != nullptr)
                    {
                        *result = hs(L"ok");
                    }

                    return S_OK;
                }

                EnumerableOfDefaultInterfaceAccessServer() : index(-1)
                {
                }

                HRESULT STDMETHODCALLTYPE HelloWorld(HSTRING * result) 
                { 
                    wchar_t buf[256];
                    swprintf_s(buf, _countof(buf), L"IMethod.HelloWorld(int) called with index=%d", this->index);
                    *result = hs(buf);  
                    return S_OK; 
                }

                //
                // IIterable members
                //
                IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<Winery::IEnumerable::EnumerableOfDefaultInterface::IMethod *> **first)
                {
                    if (!first) { return E_POINTER; }

                    // create IIterable
                    Microsoft::WRL::ComPtr<EnumerableOfDefaultInterface_IMethod> spNew = 
                        Microsoft::WRL::Make<EnumerableOfDefaultInterface_IMethod>();

                    spNew.CopyTo(first);

                    return S_OK;
                }
            };

            [uuid("A0B71A26-4EEC-49D0-BCA6-97B52A2B21DB")]
            class EnumerableOfDefaultInterface_IMethod :
                    public Microsoft::WRL::RuntimeClass<
                        Windows::Foundation::Collections::IIterator<Winery::IEnumerable::EnumerableOfDefaultInterface::IMethod *>>
            {
            private:
                static const int Count = 3;

                int currentIndex;
                Microsoft::WRL::ComPtr<Winery::IEnumerable::EnumerableOfDefaultInterface::EnumerableOfDefaultInterfaceAccessServer> testIMethods[Count];

            public:
                InspectableClass(L"Winery.IEnumerable.EnumerableOfDefaultInterface.EnumerableOfDefaultInterface_IMethod", BaseTrust);

            public:
                EnumerableOfDefaultInterface_IMethod()
                {
                    this->currentIndex = 0;
                    for (int i=0; i<Count; i++)
                    {
                        testIMethods[i] = nullptr;
                    }

                    for (int i=0; i<Count; i++)
                    {
                        // create sample collection
                        testIMethods[i] = Microsoft::WRL::Make<Winery::IEnumerable::EnumerableOfDefaultInterface::EnumerableOfDefaultInterfaceAccessServer>();
                        testIMethods[i]->SetIndex(i+1, nullptr);
                    }
                }

                virtual ~EnumerableOfDefaultInterface_IMethod()
                {
                    for (int i=0; i<Count; i++)
                    {
                        testIMethods[i] = nullptr;
                    }
                }

                //
                // IIterator members
                //
                IFACEMETHOD(get_Current)(__out Winery::IEnumerable::EnumerableOfDefaultInterface::IMethod **current)
                {
                    if (!current) { return E_POINTER; }

                    HRESULT hr = E_FAIL;

                    if (this->currentIndex < Count)
                    {
                        *current = (Winery::IEnumerable::EnumerableOfDefaultInterface::IMethod *)(testIMethods[currentIndex].Get());
                        (*current)->AddRef();
                        hr = S_OK;
                    }
                    else
                    {
                        *current = nullptr;
                    }

                    return hr;
                }

                IFACEMETHOD(get_HasCurrent)(__RPC__out boolean *hasCurrent)
                {
                    if (!hasCurrent) { return E_POINTER; }
                    *hasCurrent = (this->currentIndex < Count);
                    return S_OK;
                }

                IFACEMETHOD(MoveNext)(__RPC__out boolean *hasCurrent)
                {
                    HRESULT hr = get_HasCurrent(hasCurrent);

                    if (SUCCEEDED(hr) && hasCurrent)
                    {
                        this->currentIndex++;
                        hr = get_HasCurrent(hasCurrent);
                    }

                    return S_OK;
                }

                IFACEMETHOD(GetMany)(
                    __in unsigned int capacity, 
                    __RPC__out_ecount_part(capacity, *actual) Winery::IEnumerable::EnumerableOfDefaultInterface::IMethod **items, 
                    __RPC__out unsigned int *actual)
                {
                    if (!items) { return E_POINTER; }
                    if (!actual) { return E_POINTER; }

                    *actual = 0;
                    unsigned int offsetInOutput = 0;
                    for (int i=this->currentIndex; i<Count && offsetInOutput < capacity; i++, offsetInOutput++)
                    {
                        items[offsetInOutput] = (Winery::IEnumerable::EnumerableOfDefaultInterface::IMethod *)(testIMethods[i].Get());
                        (items[offsetInOutput])->AddRef();
                        this->currentIndex++;
                        (*actual)++;
                    }

                    return S_OK;
                }
            };
        }

        namespace EnumerableOfDefaultInterfaceWithMultipleSameName
        {
            class EnumerableOfDefaultInterface_IMethod;

            [uuid("A0B71A26-4EEC-49D0-BCA6-A7B52A2B21DC")]
            class EnumerableOfDefaultInterfaceAccessServer :
                public Microsoft::WRL::RuntimeClass<
                    Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethod,
                    Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethodColl,
                    Windows::Foundation::Collections::IIterable<Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethod *>
                >
            {
            private:
                int index;

            public:
                InspectableClass(L"Winery.IEnumerable.EnumerableOfDefaultInterfaceWithMultipleSameName.Access", BaseTrust);

            public:
                HRESULT STDMETHODCALLTYPE SetIndex(int newIndex, HSTRING * result)
                {
                    // NOTE THE *10 for making it unique for this class.
                    this->index = newIndex * 10;

                    if (result != nullptr)
                    {
                        *result = hs(L"ok");
                    }

                    return S_OK;
                }

                EnumerableOfDefaultInterfaceAccessServer() : index(-1)
                {
                }

                HRESULT STDMETHODCALLTYPE HelloWorld(HSTRING * result) 
                { 
                    wchar_t buf[256];
                    swprintf_s(buf, _countof(buf), L"IMethod.HelloWorld(int) called with index=%d", this->index);
                    *result = hs(buf);  
                    return S_OK; 
                }

                //
                // IIterable members
                //
                IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethod *> **first)
                {
                    if (!first) { return E_POINTER; }

                    // create IIterable
                    Microsoft::WRL::ComPtr<EnumerableOfDefaultInterface_IMethod> spNew = 
                        Microsoft::WRL::Make<EnumerableOfDefaultInterface_IMethod>();

                    spNew.CopyTo(first);

                    return S_OK;
                }
            };

            [uuid("A0B71A26-4EEC-49D0-BCA6-A7B52A2B21DB")]
            class EnumerableOfDefaultInterface_IMethod :
                    public Microsoft::WRL::RuntimeClass<
                        Windows::Foundation::Collections::IIterator<Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethod *>>
            {
            private:
                static const int Count = 3;

                int currentIndex;
                Microsoft::WRL::ComPtr<Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::EnumerableOfDefaultInterfaceAccessServer> testIMethods[Count];

            public:
                InspectableClass(L"Winery.IEnumerable.EnumerableOfDefaultInterface.EnumerableOfDefaultInterface_IMethod", BaseTrust);

            public:
                EnumerableOfDefaultInterface_IMethod()
                {
                    this->currentIndex = 0;
                    for (int i=0; i<Count; i++)
                    {
                        testIMethods[i] = nullptr;
                    }

                    for (int i=0; i<Count; i++)
                    {
                        // create sample collection
                        testIMethods[i] = Microsoft::WRL::Make<Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::EnumerableOfDefaultInterfaceAccessServer>();
                        testIMethods[i]->SetIndex(i+1, nullptr);
                    }
                }

                virtual ~EnumerableOfDefaultInterface_IMethod()
                {
                    for (int i=0; i<Count; i++)
                    {
                        testIMethods[i] = nullptr;
                    }
                }

                //
                // IIterator members
                //
                IFACEMETHOD(get_Current)(__out Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethod **current)
                {
                    if (!current) { return E_POINTER; }

                    HRESULT hr = E_FAIL;

                    if (this->currentIndex < Count)
                    {
                        *current = (Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethod *)(testIMethods[currentIndex].Get());
                        (*current)->AddRef();
                        hr = S_OK;
                    }
                    else
                    {
                        *current = nullptr;
                    }

                    return hr;
                }

                IFACEMETHOD(get_HasCurrent)(__RPC__out boolean *hasCurrent)
                {
                    if (!hasCurrent) { return E_POINTER; }
                    *hasCurrent = (this->currentIndex < Count);
                    return S_OK;
                }

                IFACEMETHOD(MoveNext)(__RPC__out boolean *hasCurrent)
                {
                    HRESULT hr = get_HasCurrent(hasCurrent);

                    if (SUCCEEDED(hr) && hasCurrent)
                    {
                        this->currentIndex++;
                        hr = get_HasCurrent(hasCurrent);
                    }

                    return S_OK;
                }

                IFACEMETHOD(GetMany)(
                    __in unsigned int capacity, 
                    __RPC__out_ecount_part(capacity, *actual) Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethod **items, 
                    __RPC__out unsigned int *actual)
                {
                    if (!items) { return E_POINTER; }
                    if (!actual) { return E_POINTER; }

                    *actual = 0;
                    unsigned int offsetInOutput = 0;
                    for (int i=this->currentIndex; i<Count && offsetInOutput < capacity; i++, offsetInOutput++)
                    {
                        items[offsetInOutput] = (Winery::IEnumerable::EnumerableOfDefaultInterfaceWithMultipleSameName::IMethod *)(testIMethods[i].Get());
                        (items[offsetInOutput])->AddRef();
                        this->currentIndex++;
                        (*actual)++;
                    }

                    return S_OK;
                }
            };
        }

        namespace EnumerableOfItself
        {
            class EnumerableOfDefaultInterfaceAccessServer;
            class EnumerableOfDefaultInterface_IMethod;

            class EnumerableOfDefaultInterfaceAccessServer :
                public Microsoft::WRL::RuntimeClass<
                    Winery::IEnumerable::EnumerableOfItself::IMethod,
                    Winery::IEnumerable::EnumerableOfItself::IMethod2,
                    Windows::Foundation::Collections::IIterable<Winery::IEnumerable::EnumerableOfItself::IMethod *>
                >
            {
            private:
                int index;

            public:
                InspectableClass(L"Winery.IEnumerable.EnumerableOfItself.Access", BaseTrust);

            public:
                HRESULT STDMETHODCALLTYPE SetIndex(int newIndex, HSTRING * result)
                {
                    this->index = newIndex * 100;

                    if (result != nullptr)
                    {
                        *result = hs(L"ok");
                    }

                    return S_OK;
                }

                EnumerableOfDefaultInterfaceAccessServer() : index(-100)
                {
                }

                HRESULT STDMETHODCALLTYPE HelloWorld(HSTRING * result) 
                { 
                    wchar_t buf[256];
                    swprintf_s(buf, _countof(buf), L"IMethod.HelloWorld(int) called with index=%d", this->index);
                    *result = hs(buf);  
                    return S_OK; 
                }

                HRESULT STDMETHODCALLTYPE HelloWorld2(HSTRING * result) 
                { 
                    *result = hs(L"IMethod.HelloWorld2() called");  
                    return S_OK; 
                }

                //
                // IIterable members
                //
                IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<Winery::IEnumerable::EnumerableOfItself::IMethod *> **first)
                {
                    if (!first) { return E_POINTER; }

                    // create IIterable
                    Microsoft::WRL::ComPtr<EnumerableOfDefaultInterface_IMethod> spNew = 
                        Microsoft::WRL::Make<EnumerableOfDefaultInterface_IMethod>();

                    spNew.CopyTo(first);

                    return S_OK;
                }
            };

            class EnumerableOfDefaultInterface_IMethod :
                    public Microsoft::WRL::RuntimeClass<
                        Windows::Foundation::Collections::IIterator<Winery::IEnumerable::EnumerableOfItself::IMethod *>>
            {
            private:
                static const int Count = 3;

                int currentIndex;
                Microsoft::WRL::ComPtr<Winery::IEnumerable::EnumerableOfItself::EnumerableOfDefaultInterfaceAccessServer> testIMethods[Count];

            public:
                InspectableClass(L"Winery.IEnumerable.EnumerableOfItself.EnumerableOfDefaultInterface_IMethod", BaseTrust);

            public:
                EnumerableOfDefaultInterface_IMethod()
                {
                    this->currentIndex = 0;
                    for (int i=0; i<Count; i++)
                    {
                        testIMethods[i] = nullptr;
                    }

                    for (int i=0; i<Count; i++)
                    {
                        // create sample collection
                        testIMethods[i] = Microsoft::WRL::Make<Winery::IEnumerable::EnumerableOfItself::EnumerableOfDefaultInterfaceAccessServer>();
                        testIMethods[i]->SetIndex(i+1, nullptr);
                    }
                }

                virtual ~EnumerableOfDefaultInterface_IMethod()
                {
                    for (int i=0; i<Count; i++)
                    {
                        testIMethods[i] = nullptr;
                    }
                }

                //
                // IIterator members
                //
                IFACEMETHOD(get_Current)(__out Winery::IEnumerable::EnumerableOfItself::IMethod **current)
                {
                    if (!current) { return E_POINTER; }

                    HRESULT hr = E_FAIL;

                    if (this->currentIndex < Count)
                    {
                        *current = (Winery::IEnumerable::EnumerableOfItself::IMethod *)(testIMethods[currentIndex].Get());
                        (*current)->AddRef();
                        hr = S_OK;
                    }
                    else
                    {
                        *current = nullptr;
                    }

                    return hr;
                }

                IFACEMETHOD(get_HasCurrent)(__RPC__out boolean *hasCurrent)
                {
                    if (!hasCurrent) { return E_POINTER; }
                    *hasCurrent = (this->currentIndex < Count);
                    return S_OK;
                }

                IFACEMETHOD(MoveNext)(__RPC__out boolean *hasCurrent)
                {
                    HRESULT hr = get_HasCurrent(hasCurrent);

                    if (SUCCEEDED(hr) && hasCurrent)
                    {
                        this->currentIndex++;
                        hr = get_HasCurrent(hasCurrent);
                    }

                    return S_OK;
                }

                IFACEMETHOD(GetMany)(
                    __in unsigned int capacity, 
                    __RPC__out_ecount_part(capacity, *actual) Winery::IEnumerable::EnumerableOfItself::IMethod **items, 
                    __RPC__out unsigned int *actual)
                {
                    if (!items) { return E_POINTER; }
                    if (!actual) { return E_POINTER; }

                    *actual = 0;
                    unsigned int offsetInOutput = 0;
                    for (int i=this->currentIndex; i<Count && offsetInOutput < capacity; i++, offsetInOutput++)
                    {
                        items[offsetInOutput] = (Winery::IEnumerable::EnumerableOfItself::IMethod *)(testIMethods[i].Get());
                        (items[offsetInOutput])->AddRef();
                        this->currentIndex++;
                        (*actual)++;
                    }

                    return S_OK;
                }
            };
        }

        namespace EnumerableOfItselfAsRTC
        {
            class EnumerableOfDefaultInterfaceAccessServer;
            class EnumerableOfDefaultInterface_RTC;

            class EnumerableOfDefaultInterfaceAccessServer :
                public Microsoft::WRL::RuntimeClass<
                    Winery::IEnumerable::EnumerableOfItselfAsRTC::IMethod,
                    Winery::IEnumerable::EnumerableOfItselfAsRTC::IMethod2,
                    Windows::Foundation::Collections::IIterable<Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC*>
                >
            {
            private:
                int index;

            public:
                InspectableClass(L"Winery.IEnumerable.EnumerableOfItselfAsRTC.Access", BaseTrust);

            public:
                HRESULT STDMETHODCALLTYPE SetIndex(int newIndex, HSTRING * result)
                {
                    this->index = newIndex * 1000;

                    if (result != nullptr)
                    {
                        *result = hs(L"ok");
                    }

                    return S_OK;
                }

                EnumerableOfDefaultInterfaceAccessServer() : index(-1000)
                {
                }

                HRESULT STDMETHODCALLTYPE HelloWorld(HSTRING * result) 
                { 
                    wchar_t buf[256];
                    swprintf_s(buf, _countof(buf), L"IMethod.HelloWorld(int) called with index=%d", this->index);
                    *result = hs(buf);  
                    return S_OK; 
                }

                HRESULT STDMETHODCALLTYPE HelloWorld2(HSTRING * result) 
                { 
                    *result = hs(L"RTC.HelloWorld2() called");  
                    return S_OK; 
                }

                //
                // IIterable members
                //
                IFACEMETHOD(First)(__out Windows::Foundation::Collections::IIterator<Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC *> **first)
                {
                    if (!first) { return E_POINTER; }

                    // create IIterable
                    Microsoft::WRL::ComPtr<EnumerableOfDefaultInterface_RTC> spNew = 
                        Microsoft::WRL::Make<EnumerableOfDefaultInterface_RTC>();

                    spNew.CopyTo(first);

                    return S_OK;
                }
            };

            class EnumerableOfDefaultInterface_RTC :
                    public Microsoft::WRL::RuntimeClass<
                        Windows::Foundation::Collections::IIterator<Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC *>>
            {
            private:
                static const int Count = 3;

                int currentIndex;
                Microsoft::WRL::ComPtr<Winery::IEnumerable::EnumerableOfItselfAsRTC::EnumerableOfDefaultInterfaceAccessServer> testIMethods[Count];

            public:
                InspectableClass(L"Winery.IEnumerable.EnumerableOfItselfAsRTC.EnumerableOfDefaultInterface_RTC", BaseTrust);

            public:
                EnumerableOfDefaultInterface_RTC()
                {
                    this->currentIndex = 0;
                    for (int i=0; i<Count; i++)
                    {
                        testIMethods[i] = nullptr;
                    }

                    for (int i=0; i<Count; i++)
                    {
                        // create sample collection
                        testIMethods[i] = Microsoft::WRL::Make<Winery::IEnumerable::EnumerableOfItselfAsRTC::EnumerableOfDefaultInterfaceAccessServer>();
                        testIMethods[i]->SetIndex(i+1, nullptr);
                    }
                }

                virtual ~EnumerableOfDefaultInterface_RTC()
                {
                    for (int i=0; i<Count; i++)
                    {
                        testIMethods[i] = nullptr;
                    }
                }

                //
                // IIterator members
                //
                /// seems not needed to be implemented - the winery.h is implementing those automatically?
                /// IIterator<Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC*> : IIterator_impl<Windows::Foundation::Internal::AggregateType<Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC*, Winery::IEnumerable::EnumerableOfItselfAsRTC::IMethod*>> {
                ///
                //IFACEMETHOD(get_Current)(__out Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC **current)
                //{
                //    if (!current) { return E_POINTER; }

                //    HRESULT hr = E_FAIL;

                //    if (this->currentIndex < Count)
                //    {
                //        *current = (Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC *)(testIMethods[currentIndex].Get());
                //        (*current)->AddRef();
                //        hr = S_OK;
                //    }
                //    else
                //    {
                //        *current = nullptr;
                //    }

                //    return hr;
                //}

                //IFACEMETHOD(GetMany)(
                //    __in unsigned int capacity, 
                //    __RPC__out_ecount_part(capacity, *actual) Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC **items, 
                //    __RPC__out unsigned int *actual)
                //{
                //    if (!items) { return E_POINTER; }
                //    if (!actual) { return E_POINTER; }

                //    *actual = 0;
                //    unsigned int offsetInOutput = 0;
                //    for (int i=this->currentIndex; i<Count && offsetInOutput < capacity; i++, offsetInOutput++)
                //    {
                //        items[offsetInOutput] = nullptr;
                //        //items[offsetInOutput] = (Winery::IEnumerable::EnumerableOfItselfAsRTC::RTC *)(testIMethods[i].Get());
                //        //(items[offsetInOutput])->AddRef();
                //        this->currentIndex++;
                //        (*actual)++;
                //    }

                //    return S_OK;
                //}

                IFACEMETHOD(get_Current)(__out Winery::IEnumerable::EnumerableOfItselfAsRTC::IMethod **current)
                {
                    if (!current) { return E_POINTER; }

                    HRESULT hr = E_FAIL;

                    if (this->currentIndex < Count)
                    {
                        *current = (Winery::IEnumerable::EnumerableOfItselfAsRTC::IMethod *)(testIMethods[currentIndex].Get());
                        (*current)->AddRef();
                        hr = S_OK;
                    }
                    else
                    {
                        *current = nullptr;
                    }

                    return hr;
                }

                IFACEMETHOD(get_HasCurrent)(__RPC__out boolean *hasCurrent)
                {
                    if (!hasCurrent) { return E_POINTER; }
                    *hasCurrent = (this->currentIndex < Count);
                    return S_OK;
                }

                IFACEMETHOD(MoveNext)(__RPC__out boolean *hasCurrent)
                {
                    HRESULT hr = get_HasCurrent(hasCurrent);

                    if (SUCCEEDED(hr) && hasCurrent)
                    {
                        this->currentIndex++;
                        hr = get_HasCurrent(hasCurrent);
                    }

                    return S_OK;
                }

            };
        }
   }
}