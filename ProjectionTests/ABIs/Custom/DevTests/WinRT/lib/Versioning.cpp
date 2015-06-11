//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "Versioning.h"

using namespace Microsoft::WRL;
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;

IFACEMETHODIMP
DevTests::Versioning::MinVersionFactory::ActivateInstance(__deref_out IInspectable ** ppInspectable)
{
    HRESULT hr = S_OK;
    ComPtr<IMinVersionInterface> spIMin;
    *ppInspectable = nullptr;

    ComPtr<MinVersionClass> spObj = Make<MinVersionClass>();

    hr = spObj.As(&spIMin);
    if (SUCCEEDED(hr))
    {
        *ppInspectable = spIMin.Detach();
    }

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::MinVersionFactory::CreateClass(__in int value, __out IMinVersionInterface ** ppMin)
{
    HRESULT hr = S_OK;
    ComPtr<IMinVersionInterface> spIMin;
    *ppMin = nullptr;

    ComPtr<MinVersionClass> spObj = Make<MinVersionClass>(value);

    hr = spObj.As(&spIMin);
    if (SUCCEEDED(hr))
    {
        *ppMin = spIMin.Detach();
    }

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::Win8Factory::ActivateInstance(__deref_out IInspectable ** ppInspectable)
{
    HRESULT hr = S_OK;
    ComPtr<IMinVersionInterface> spIMin;
    *ppInspectable = nullptr;

    ComPtr<Win8Class> spObj = Make<Win8Class>();

    hr = spObj.As(&spIMin);
    if (SUCCEEDED(hr))
    {
        *ppInspectable = spIMin.Detach();
    }

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::Win8Factory::CreateClass(__in int value, __out IMinVersionInterface ** ppWin8)
{
    HRESULT hr = S_OK;
    ComPtr<IMinVersionInterface> spIMin;
    *ppWin8 = nullptr;

    ComPtr<Win8Class> spObj = Make<Win8Class>(value);

    hr = spObj.As(&spIMin);
    if (SUCCEEDED(hr))
    {
        *ppWin8 = spIMin.Detach();
    }

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::Win8SP1Factory::ActivateInstance(__deref_out IInspectable ** ppInspectable)
{
    HRESULT hr = S_OK;
    ComPtr<IMinVersionInterface> spIMin;
    *ppInspectable = nullptr;

    ComPtr<Win8SP1Class> spObj = Make<Win8SP1Class>();

    hr = spObj.As(&spIMin);
    if (SUCCEEDED(hr))
    {
        *ppInspectable = spIMin.Detach();
    }

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::Win8SP1Factory::CreateClass(__in int value, __out IMinVersionInterface ** ppWin8SP1)
{
    HRESULT hr = S_OK;
    ComPtr<IMinVersionInterface> spIMin;
    *ppWin8SP1 = nullptr;

    ComPtr<Win8SP1Class> spObj = Make<Win8SP1Class>(value);

    hr = spObj.As(&spIMin);
    if (SUCCEEDED(hr))
    {
        *ppWin8SP1 = spIMin.Detach();
    }

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::Win9Factory::ActivateInstance(__deref_out IInspectable ** ppInspectable)
{
    HRESULT hr = S_OK;
    ComPtr<IWin8Interface> spIWin8;
    *ppInspectable = nullptr;

    ComPtr<Win9Class> spObj = Make<Win9Class>();

    hr = spObj.As(&spIWin8);
    if (SUCCEEDED(hr))
    {
        *ppInspectable = spIWin8.Detach();
    }

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::Win9Factory::CreateClass(__in int value, __out IWin8Interface ** ppWin9)
{
    HRESULT hr = S_OK;
    ComPtr<IWin8Interface> spIWin8;
    *ppWin9 = nullptr;

    ComPtr<Win9Class> spObj = Make<Win9Class>(value);

    hr = spObj.As(&spIWin8);
    if (SUCCEEDED(hr))
    {
        *ppWin9 = spIWin8.Detach();
    }

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::MarshalVersionedTypes::CallMinVersionDelegate(__in int inputVersion, __in IMinVersionDelegate * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    ComPtr<IMinVersionInterface> spIMin = nullptr;
    ComPtr<Win8Class> spWin8Obj = nullptr;
    ComPtr<Win8SP1Class> spWin8SP1Obj = nullptr;
    ComPtr<MinVersionClass> spMinObj = nullptr;
    switch(inputVersion)
    {
    case VersionedEnumFields_Win8:
        spWin8Obj = Make<Win8Class>();
        hr = spWin8Obj.As(&spIMin);
        break;
    case VersionedEnumFields_Win8SP1:
        spWin8SP1Obj = Make<Win8SP1Class>();
        hr = spWin8SP1Obj.As(&spIMin);
        break;
    default:
        spMinObj = Make<MinVersionClass>();
        hr = spMinObj.As(&spIMin);
        break;
    }

    if (SUCCEEDED(hr))
    {
        hr = value->Invoke(spIMin.Get());
    }
    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::MarshalVersionedTypes::CallWin8Delegate(__in IWin8Delegate * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    HRESULT hr = value->Invoke(m_win8sp1Struct);
    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::MarshalVersionedTypes::CallWin8SP1Delegate(__in IWin8SP1Delegate * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    HRESULT hr = value->Invoke(MaxVersionEnum_Max);
    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::MarshalVersionedTypes::CallWin9Delegate(__in IWin9Delegate * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }

    ComPtr<Win9Class> spObj = Make<Win9Class>();
    HRESULT hr = value->Invoke(spObj.Get());
    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::MarshalVersionedTypes::CallMaxVersionDelegate(__in IMaxVersionDelegate * value)
{
    if (nullptr == value)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    ComPtr<IWin8Interface> spIWin8 = nullptr;
    ComPtr<Win9Class> spObj = Make<Win9Class>();
    hr = spObj.As(&spIWin8);
    if (SUCCEEDED(hr))
    {
        hr = value->Invoke(spIWin8.Get());
    }
    return hr;
}

DevTests::Versioning::CVectorInt::CVectorInt()
{
    ComPtr<Vector<int>> sp;
    Vector<int>::Make(&sp);
    for (int i = 1; i < 10; i++)
    {
        sp->Append(i);
    }

    sp.CopyTo(&m_pVector);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterable<int> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<int>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::GetAt(__in unsigned index, __out int *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::IndexOf(__in_opt int value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::GetView(__deref_out_opt IVectorView<int> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::SetAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::InsertAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::Append(__in_opt int value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorInt::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) int *value)
{
    return m_pVector->ReplaceAll(count, value);
}


DevTests::Versioning::CVectorIWin9Interface::CVectorIWin9Interface()
{
    ComPtr<Vector<IWin9Interface *>> sp;
    Vector<IWin9Interface *>::Make(&sp);

    IWin9Interface *pWin9;

    ComPtr<IWin9Interface> spWin9 = Make<Win9Class>();
    spWin9.CopyTo(&pWin9);
    sp->Append(pWin9);
    pWin9->Release();

    spWin9 = Make<Win9Class>(125);
    spWin9.CopyTo(&pWin9);
    sp->Append(pWin9);
    pWin9->Release();

    spWin9 = Make<Win9Class>(42);
    spWin9.CopyTo(&pWin9);
    sp->Append(pWin9);
    pWin9->Release();

    sp.CopyTo(&m_pVector);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::First(__out Windows::Foundation::Collections::IIterator<IWin9Interface *> **first)
{
    Windows::Foundation::Collections::IIterable<IWin9Interface *> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<IWin9Interface *>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::GetAt(__in unsigned index, __out IWin9Interface **item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::IndexOf(__in_opt IWin9Interface * value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::GetView(__deref_out_opt IVectorView<IWin9Interface *> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::SetAt(__in unsigned index, __in_opt IWin9Interface * value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::InsertAt(__in unsigned index, __in_opt IWin9Interface * value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::Append(__in_opt IWin9Interface * value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) IWin9Interface **items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
DevTests::Versioning::CVectorIWin9Interface::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) IWin9Interface **value)
{
    return m_pVector->ReplaceAll(count, value);
}


DevTests::Versioning::CObservableVectorInt::CObservableVectorInt()
{
    ComPtr<ObservableVector<int>> sp;
    ObservableVector<int>::Make(&sp);
    for (int i = 1; i < 10; i++)
    {
        sp->Append(i);
    }

    sp.CopyTo(&m_pObservableVector);
    m_pObservableVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IVector<int>), (void **)&m_pVector);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterable<int> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<int>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::GetAt(__in unsigned index, __out int *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::IndexOf(__in_opt int value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::GetView(__deref_out_opt IVectorView<int> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::SetAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::InsertAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::Append(__in_opt int value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) int *value)
{
    return m_pVector->ReplaceAll(count, value);
}


IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::add_VectorChanged(__RPC__in_opt Windows::Foundation::Collections::VectorChangedEventHandler<int> *handler, __RPC__out EventRegistrationToken *token)
{
    return m_pObservableVector->add_VectorChanged(handler, token);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorInt::remove_VectorChanged(__in EventRegistrationToken token)
{
    return m_pObservableVector->remove_VectorChanged(token);
}

DevTests::Versioning::CObservableVectorIWin9Interface::CObservableVectorIWin9Interface()
{
    ComPtr<ObservableVector<IWin9Interface *>> sp;
    ObservableVector<IWin9Interface *>::Make(&sp);

    IWin9Interface *pWin9;

    ComPtr<IWin9Interface> spWin9 = Make<Win9Class>();
    spWin9.CopyTo(&pWin9);
    sp->Append(pWin9);
    pWin9->Release();

    spWin9 = Make<Win9Class>(125);
    spWin9.CopyTo(&pWin9);
    sp->Append(pWin9);
    pWin9->Release();

    spWin9 = Make<Win9Class>(42);
    spWin9.CopyTo(&pWin9);
    sp->Append(pWin9);
    pWin9->Release();

    sp.CopyTo(&m_pObservableVector);
    m_pObservableVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IVector<IWin9Interface *>), (void **)&m_pVector);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::First(__out Windows::Foundation::Collections::IIterator<IWin9Interface *> **first)
{
    Windows::Foundation::Collections::IIterable<IWin9Interface *> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<IWin9Interface *>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::GetAt(__in unsigned index, __out IWin9Interface **item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::IndexOf(__in_opt IWin9Interface * value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::GetView(__deref_out_opt IVectorView<IWin9Interface *> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::SetAt(__in unsigned index, __in_opt IWin9Interface * value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::InsertAt(__in unsigned index, __in_opt IWin9Interface * value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::Append(__in_opt IWin9Interface * value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) IWin9Interface **items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) IWin9Interface **value)
{
    return m_pVector->ReplaceAll(count, value);
}


IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::add_VectorChanged(__RPC__in_opt Windows::Foundation::Collections::VectorChangedEventHandler<IWin9Interface *> *handler, __RPC__out EventRegistrationToken *token)
{
    return m_pObservableVector->add_VectorChanged(handler, token);
}

IFACEMETHODIMP
DevTests::Versioning::CObservableVectorIWin9Interface::remove_VectorChanged(__in EventRegistrationToken token)
{
    return m_pObservableVector->remove_VectorChanged(token);
}
