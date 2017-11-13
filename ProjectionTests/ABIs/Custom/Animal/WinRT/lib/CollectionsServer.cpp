#include "stdafx.h"
#include "CollectionsServer.h"
#include "AnimalServer.h"

#include <winrt\windowscollectionsp.h>
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;
using namespace Microsoft::WRL;

Animals::CVectorInt::CVectorInt()
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
Animals::CVectorInt::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterable<int> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<int>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CVectorInt::GetAt(__in unsigned index, __out int *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
Animals::CVectorInt::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
Animals::CVectorInt::IndexOf(__in_opt int value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::CVectorInt::GetView(__deref_out_opt IVectorView<int> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
Animals::CVectorInt::SetAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
Animals::CVectorInt::InsertAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
Animals::CVectorInt::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
Animals::CVectorInt::Append(__in_opt int value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
Animals::CVectorInt::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
Animals::CVectorInt::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
Animals::CVectorInt::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::CVectorInt::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) int *value)
{
    return m_pVector->ReplaceAll(count, value);
}

Animals::CVectorHSTRING::CVectorHSTRING()
{
    ComPtr<Vector<HSTRING>> spHSTRING;
    Vector<HSTRING>::Make(&spHSTRING);

    HSTRING hString;
    WindowsCreateString(L"String1", 7, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"String2", 7, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"String3", 7, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"String4", 7, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);

    spHSTRING.CopyTo(&m_pVector);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::First(__out Windows::Foundation::Collections::IIterator<HSTRING> **first)
{
    Windows::Foundation::Collections::IIterable<HSTRING> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<HSTRING>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CVectorHSTRING::GetAt(__in unsigned index, __out HSTRING *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::IndexOf(__in_opt HSTRING value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::GetView(__deref_out_opt IVectorView<HSTRING> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::SetAt(__in unsigned index, __in_opt HSTRING value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::InsertAt(__in unsigned index, __in_opt HSTRING value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::Append(__in_opt HSTRING value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
Animals::CVectorHSTRING::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
Animals::CVectorHSTRING::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::CVectorHSTRING::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) HSTRING *value)
{
    return m_pVector->ReplaceAll(count, value);
}

Animals::CVectorIAnimal::CVectorIAnimal()
{
    ComPtr<Vector<IAnimal *>> sp;
    Vector<IAnimal *>::Make(&sp);

    HSTRING hString;
    IAnimal *pAnimal;

    ComPtr<IAnimal> spAnimal = Make<AnimalServer>();
    spAnimal.CopyTo(&pAnimal);
    WindowsCreateString(L"Animal1", 7, &hString);
    spAnimal->SetGreeting(hString);
    WindowsDeleteString(hString);
    sp->Append(pAnimal);
    pAnimal->Release();

    spAnimal = Make<AnimalServer>();
    spAnimal.CopyTo(&pAnimal);
    WindowsCreateString(L"Animal2", 7, &hString);
    pAnimal->SetGreeting(hString);
    WindowsDeleteString(hString);
    sp->Append(pAnimal);
    pAnimal->Release();

    spAnimal = Make<AnimalServer>();
    spAnimal.CopyTo(&pAnimal);
    WindowsCreateString(L"Animal3", 7, &hString);
    pAnimal->SetGreeting(hString);
    WindowsDeleteString(hString);
    sp->Append(pAnimal);
    pAnimal->Release();

    sp.CopyTo(&m_pVector);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::First(__out Windows::Foundation::Collections::IIterator<IAnimal *> **first)
{
    Windows::Foundation::Collections::IIterable<IAnimal *> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<IAnimal *>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CVectorIAnimal::GetAt(__in unsigned index, __out IAnimal **item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::IndexOf(__in_opt IAnimal * value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::GetView(__deref_out_opt IVectorView<IAnimal *> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::SetAt(__in unsigned index, __in_opt IAnimal * value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::InsertAt(__in unsigned index, __in_opt IAnimal * value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::Append(__in_opt IAnimal * value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
Animals::CVectorIAnimal::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
Animals::CVectorIAnimal::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) IAnimal **items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::CVectorIAnimal::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) IAnimal **value)
{
    return m_pVector->ReplaceAll(count, value);
}

Animals::CVectorViewFloat::CVectorViewFloat()
{
    ComPtr<Vector<float>> sp;
    Vector<float>::Make(&sp);

    sp->Append((float)0.25);
    sp->Append((float)0.50);
    sp->Append((float)0.75);
    sp->Append((float)1.25);
    sp->Append((float)1.50);

    sp.CopyTo(&m_pVector);

    m_pVector->GetView(&m_pVectorView);
}

IFACEMETHODIMP
Animals::CVectorViewFloat::First(__out Windows::Foundation::Collections::IIterator<float> **first)
{
    Windows::Foundation::Collections::IIterable<float> *pIIterable = NULL;
    HRESULT hr = m_pVectorView->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<float>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CVectorViewFloat::GetAt(__in unsigned index, __out float *item)
{
    return m_pVectorView->GetAt(index, item);
}

IFACEMETHODIMP
Animals::CVectorViewFloat::get_Size(__out unsigned *size)
{
    return m_pVectorView->get_Size(size);
}

IFACEMETHODIMP
Animals::CVectorViewFloat::IndexOf(__in_opt float value, __out unsigned *index, __out boolean *found)
{
    return m_pVectorView->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::CVectorViewFloat::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) float *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

Animals::CVectorViewHSTRING::CVectorViewHSTRING()
{
    ComPtr<Vector<HSTRING>> spHSTRING;
    Vector<HSTRING>::Make(&spHSTRING);

    HSTRING hString;
    WindowsCreateString(L"ViewString1", 11, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"ViewString2", 11, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"ViewString3", 11, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"ViewString4", 11, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);

    WindowsCreateString(L"ViewString5", 11, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);

    WindowsCreateString(L"ViewString6", 11, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);

    WindowsCreateString(L"ViewString7", 11, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);

    spHSTRING.CopyTo(&m_pVector);
    m_pVector->GetView(&m_pVectorView);
}

IFACEMETHODIMP
Animals::CVectorViewHSTRING::First(__out Windows::Foundation::Collections::IIterator<HSTRING> **first)
{
    Windows::Foundation::Collections::IIterable<HSTRING> *pIIterable = NULL;
    HRESULT hr = m_pVectorView->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<HSTRING>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CVectorViewHSTRING::GetAt(__in unsigned index, __out HSTRING *item)
{
    return m_pVectorView->GetAt(index, item);
}

IFACEMETHODIMP
Animals::CVectorViewHSTRING::get_Size(__out unsigned *size)
{
    return m_pVectorView->get_Size(size);
}

IFACEMETHODIMP
Animals::CVectorViewHSTRING::IndexOf(__in_opt HSTRING value, __out unsigned *index, __out boolean *found)
{
    return m_pVectorView->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::CVectorViewHSTRING::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

// {8EB82CB5-03D6-49D2-80EE-8583E949B5BF}
static const GUID guid1 = 
{ 0x8eb82cb5, 0x3d6, 0x49d2, { 0x80, 0xee, 0x85, 0x83, 0xe9, 0x49, 0xb5, 0xbf } };

// {B960A7AC-F275-43FC-B154-91E7ADEEE7AA}
static const GUID guid2 = 
{ 0xb960a7ac, 0xf275, 0x43fc, { 0xb1, 0x54, 0x91, 0xe7, 0xad, 0xee, 0xe7, 0xaa } };

// {9F1AF037-BAF9-473B-B8C3-183AB3F5B3CE}
static const GUID guid3 = 
{ 0x9f1af037, 0xbaf9, 0x473b, { 0xb8, 0xc3, 0x18, 0x3a, 0xb3, 0xf5, 0xb3, 0xce } };

// {C4A1CC26-EB02-435B-AE67-18A25D86A787}
static const GUID guid4 = 
{ 0xc4a1cc26, 0xeb02, 0x435b, { 0xae, 0x67, 0x18, 0xa2, 0x5d, 0x86, 0xa7, 0x87 } };

Animals::CVectorViewGUID::CVectorViewGUID()
{
    ComPtr<Vector<GUID>> spGUID;
    Vector<GUID>::Make(&spGUID);

    spGUID->Append(guid1);
    spGUID->Append(guid2);
    spGUID->Append(guid3);
    spGUID->Append(guid4);
    
    spGUID.CopyTo(&m_pVector);
    m_pVector->GetView(&m_pVectorView);
}

IFACEMETHODIMP
Animals::CVectorViewGUID::First(__out Windows::Foundation::Collections::IIterator<GUID> **first)
{
    Windows::Foundation::Collections::IIterable<GUID> *pIIterable = NULL;
    HRESULT hr = m_pVectorView->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<GUID>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CVectorViewGUID::GetAt(__in unsigned index, __out GUID *item)
{
    return m_pVectorView->GetAt(index, item);
}

IFACEMETHODIMP
Animals::CVectorViewGUID::get_Size(__out unsigned *size)
{
    return m_pVectorView->get_Size(size);
}

IFACEMETHODIMP
Animals::CVectorViewGUID::IndexOf(__in_opt GUID value, __out unsigned *index, __out boolean *found)
{
    return m_pVectorView->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::CVectorViewGUID::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) GUID *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

Animals::CObservableVectorInt::CObservableVectorInt()
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
Animals::CObservableVectorInt::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterable<int> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<int>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CObservableVectorInt::GetAt(__in unsigned index, __out int *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::IndexOf(__in_opt int value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::GetView(__deref_out_opt IVectorView<int> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::SetAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::InsertAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::Append(__in_opt int value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
Animals::CObservableVectorInt::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
Animals::CObservableVectorInt::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) int *value)
{
    return m_pVector->ReplaceAll(count, value);
}


IFACEMETHODIMP
Animals::CObservableVectorInt::add_VectorChanged(__RPC__in_opt Windows::Foundation::Collections::VectorChangedEventHandler<int> *handler, __RPC__out EventRegistrationToken *token)
{
    return m_pObservableVector->add_VectorChanged(handler, token);
}

IFACEMETHODIMP
Animals::CObservableVectorInt::remove_VectorChanged(__in EventRegistrationToken token)
{
    return m_pObservableVector->remove_VectorChanged(token);
}

Animals::CObservableVectorHSTRING::CObservableVectorHSTRING()
{
    ComPtr<ObservableVector<HSTRING>> spHSTRING;
    ObservableVector<HSTRING>::Make(&spHSTRING);

    HSTRING hString;
    WindowsCreateString(L"String1", 7, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"String2", 7, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"String3", 7, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);
    
    WindowsCreateString(L"String4", 7, &hString);
    spHSTRING->Append(hString);
    WindowsDeleteString(hString);

    spHSTRING.CopyTo(&m_pVector);
    spHSTRING.CopyTo(&m_pObservableVector);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::First(__out Windows::Foundation::Collections::IIterator<HSTRING> **first)
{
    Windows::Foundation::Collections::IIterable<HSTRING> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<HSTRING>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::GetAt(__in unsigned index, __out HSTRING *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::IndexOf(__in_opt HSTRING value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::GetView(__deref_out_opt IVectorView<HSTRING> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::SetAt(__in unsigned index, __in_opt HSTRING value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::InsertAt(__in unsigned index, __in_opt HSTRING value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::Append(__in_opt HSTRING value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) HSTRING *value)
{
    return m_pVector->ReplaceAll(count, value);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::add_VectorChanged(__RPC__in_opt Windows::Foundation::Collections::VectorChangedEventHandler<HSTRING> * handler, __RPC__out EventRegistrationToken * token)
{
    return m_pObservableVector->add_VectorChanged(handler, token);
}

IFACEMETHODIMP
Animals::CObservableVectorHSTRING::remove_VectorChanged(__in EventRegistrationToken token)
{
    return m_pObservableVector->remove_VectorChanged(token);
}

Animals::CMapHStringAndInt::CMapHStringAndInt()
{
    Microsoft::WRL::ComPtr<HashMap<HSTRING, int>> spMap;
    HRESULT hr = HashMap<HSTRING, int>::Make(&spMap); 
    if (SUCCEEDED(hr))
    {
        boolean fReplaced;
        HSTRING hString = nullptr;

        WindowsCreateString(L"Hundred", 7, &hString);
        spMap->Insert(hString, 7, &fReplaced);
        WindowsDeleteString(hString);

        WindowsCreateString(L"by", 2, &hString);
        spMap->Insert(hString, 2, &fReplaced);
        WindowsDeleteString(hString);

        WindowsCreateString(L"Hundred And Fifty", 17, &hString);
        spMap->Insert(hString, 17, &fReplaced);
        WindowsDeleteString(hString);

        spMap.CopyTo(&m_pMap);
    }
}

IFACEMETHODIMP
Animals::CMapHStringAndInt::First(__out Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *> **first)
{
    Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *> *pIIterable = NULL;
    HRESULT hr = m_pMap->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CMapHStringAndInt::Lookup(__in_opt HSTRING key, __out int *value)
{
    return m_pMap->Lookup(key, value);
}

IFACEMETHODIMP
Animals::CMapHStringAndInt::get_Size(__out unsigned int *size)
{
    return m_pMap->get_Size(size);
}

IFACEMETHODIMP
Animals::CMapHStringAndInt::HasKey(__in_opt HSTRING key, __out boolean *found)
{
    return m_pMap->HasKey(key, found);
}

IFACEMETHODIMP
Animals::CMapHStringAndInt::GetView(__deref_out_opt Windows::Foundation::Collections::IMapView<HSTRING, int> **view)
{
    return m_pMap->GetView(view);
}

IFACEMETHODIMP
Animals::CMapHStringAndInt::Insert(__in_opt HSTRING key, __in_opt int value, __out boolean *replaced)
{
    return m_pMap->Insert(key, value, replaced);
}

IFACEMETHODIMP
Animals::CMapHStringAndInt::Remove(__in_opt HSTRING key)
{
    return m_pMap->Remove(key);
}

IFACEMETHODIMP
Animals::CMapHStringAndInt::Clear()
{
    return m_pMap->Clear();
}

Animals::CObservableMapHStringAndInt::CObservableMapHStringAndInt()
{
    Microsoft::WRL::ComPtr<ObservableHashMap<HSTRING, int>> spMap;
    HRESULT hr = ObservableHashMap<HSTRING, int>::Make(&spMap); 
    if (SUCCEEDED(hr))
    {
        boolean fReplaced;
        HSTRING hString = nullptr;

        WindowsCreateString(L"Hundred", 7, &hString);
        spMap->Insert(hString, 7, &fReplaced);
        WindowsDeleteString(hString);

        WindowsCreateString(L"by", 2, &hString);
        spMap->Insert(hString, 2, &fReplaced);
        WindowsDeleteString(hString);

        WindowsCreateString(L"Hundred And Fifty", 17, &hString);
        spMap->Insert(hString, 17, &fReplaced);
        WindowsDeleteString(hString);

        spMap.CopyTo(&m_pMap);
        spMap.CopyTo(&m_pObservableMap);
    }
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::First(__out Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *> **first)
{
    Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *> *pIIterable = NULL;
    HRESULT hr = m_pMap->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<HSTRING, int> *>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::Lookup(__in_opt HSTRING key, __out int *value)
{
    return m_pMap->Lookup(key, value);
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::get_Size(__out unsigned int *size)
{
    return m_pMap->get_Size(size);
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::HasKey(__in_opt HSTRING key, __out boolean *found)
{
    return m_pMap->HasKey(key, found);
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::GetView(__deref_out_opt Windows::Foundation::Collections::IMapView<HSTRING, int> **view)
{
    return m_pMap->GetView(view);
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::Insert(__in_opt HSTRING key, __in_opt int value, __out boolean *replaced)
{
    return m_pMap->Insert(key, value, replaced);
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::Remove(__in_opt HSTRING key)
{
    return m_pMap->Remove(key);
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::Clear()
{
    return m_pMap->Clear();
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::add_MapChanged(__RPC__in_opt Windows::Foundation::Collections::MapChangedEventHandler<HSTRING, int> * handler, __RPC__out EventRegistrationToken * token)
{
    return m_pObservableMap->add_MapChanged(handler, token);
}

IFACEMETHODIMP
Animals::CObservableMapHStringAndInt::remove_MapChanged(__in EventRegistrationToken token)
{
    return m_pObservableMap->remove_MapChanged(token);
}

Animals::CObservableMapGUIDAndInspectable::CObservableMapGUIDAndInspectable()
{
    Microsoft::WRL::ComPtr<ObservableHashMap<GUID, IInspectable*>> spMap;
    HRESULT hr = ObservableHashMap<GUID, IInspectable*>::Make(&spMap); 
    if (SUCCEEDED(hr))
    {
        boolean fReplaced;
        HSTRING hString;
        IAnimal *pAnimal;

        ComPtr<IAnimal> spAnimal = Make<AnimalServer>();
        spAnimal.CopyTo(&pAnimal);
        WindowsCreateString(L"Animal1", 7, &hString);
        spAnimal->SetGreeting(hString);
        WindowsDeleteString(hString);
        spMap->Insert(guid1, pAnimal, &fReplaced);
        pAnimal->Release();

        spAnimal = Make<AnimalServer>();
        spAnimal.CopyTo(&pAnimal);
        WindowsCreateString(L"Animal2", 7, &hString);
        pAnimal->SetGreeting(hString);
        WindowsDeleteString(hString);
        spMap->Insert(guid2, pAnimal, &fReplaced);
        pAnimal->Release();

        spAnimal = Make<AnimalServer>();
        spAnimal.CopyTo(&pAnimal);
        WindowsCreateString(L"Animal3", 7, &hString);
        pAnimal->SetGreeting(hString);
        WindowsDeleteString(hString);
        spMap->Insert(guid3, pAnimal, &fReplaced);
        pAnimal->Release();

        spMap.CopyTo(&m_pMap);
        spMap.CopyTo(&m_pObservableMap);
    }
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::First(__out Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<GUID, IInspectable*> *> **first)
{
    Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<GUID, IInspectable*> *> *pIIterable = NULL;
    HRESULT hr = m_pMap->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<GUID, IInspectable*> *>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::Lookup(__in_opt GUID key, __out IInspectable **value)
{
    return m_pMap->Lookup(key, value);
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::get_Size(__out unsigned int *size)
{
    return m_pMap->get_Size(size);
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::HasKey(__in_opt GUID key, __out boolean *found)
{
    return m_pMap->HasKey(key, found);
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::GetView(__deref_out_opt Windows::Foundation::Collections::IMapView<GUID, IInspectable*> **view)
{
    return m_pMap->GetView(view);
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::Insert(__in_opt GUID key, __in_opt IInspectable* value, __out boolean *replaced)
{
    return m_pMap->Insert(key, value, replaced);
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::Remove(__in_opt GUID key)
{
    return m_pMap->Remove(key);
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::Clear()
{
    return m_pMap->Clear();
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::add_MapChanged(__RPC__in_opt Windows::Foundation::Collections::MapChangedEventHandler<GUID, IInspectable*> * handler, __RPC__out EventRegistrationToken * token)
{
    return m_pObservableMap->add_MapChanged(handler, token);
}

IFACEMETHODIMP
Animals::CObservableMapGUIDAndInspectable::remove_MapChanged(__in EventRegistrationToken token)
{
    return m_pObservableMap->remove_MapChanged(token);
}

Animals::CIterableHSTRING::CIterableHSTRING()
{
    Microsoft::WRL::ComPtr<Vector<HSTRING>> spVector;
    HRESULT hr = Vector<HSTRING>::Make(&spVector); 
    if (SUCCEEDED(hr))
    {
        HSTRING hString = nullptr;
        WindowsCreateString(L"Monday", 6, &hString);
        spVector->Append(hString);
        WindowsDeleteString(hString);

        WindowsCreateString(L"Tuesday", 7, &hString);
        spVector->Append(hString);
        WindowsDeleteString(hString);

        WindowsCreateString(L"Wednesday", 9, &hString);
        spVector->Append(hString);
        WindowsDeleteString(hString);

        WindowsCreateString(L"Thursday", 8, &hString);
        spVector->Append(hString);
        WindowsDeleteString(hString);

        WindowsCreateString(L"Friday", 6, &hString);
        spVector->Append(hString);
        WindowsDeleteString(hString);

        WindowsCreateString(L"Saturday", 8, &hString);
        spVector->Append(hString);
        WindowsDeleteString(hString);

        WindowsCreateString(L"Sunday", 6, &hString);
        spVector->Append(hString);
        WindowsDeleteString(hString);

        spVector.CopyTo(&m_pVector);
    }
}

IFACEMETHODIMP
Animals::CIterableHSTRING::First(__out Windows::Foundation::Collections::IIterator<HSTRING> **first)
{
    Windows::Foundation::Collections::IIterable<HSTRING> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<HSTRING>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}