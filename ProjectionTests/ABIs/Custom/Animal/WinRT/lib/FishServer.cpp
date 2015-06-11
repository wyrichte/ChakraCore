#include "stdafx.h"
#include <stdio.h>
#include <strsafe.h>
#include "FishServer.h"
#include "PropertyValueTests.h"
#include "AnimalServer.h"

#include <winrt\windowscollectionsp.h>
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;

using namespace Microsoft::WRL;

IFACEMETHODIMP Animals::CFastSigInterface::GetOneVector(__out Windows::Foundation::Collections::IVector<int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<Vector<int>> sp;

    hr = Vector<int>::Make(&sp);
    for (int i = 1; SUCCEEDED(hr) && i < 4; i++)
    {
        hr = sp->Append(i);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(outVal);
    }

    return hr;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetNullAsVector(__out Windows::Foundation::Collections::IVector<int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetOneObservableVector(__out Windows::Foundation::Collections::IObservableVector<int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = NULL;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<ObservableVector<int>> sp;

    hr = ObservableVector<int>::Make(&sp);
    for (int i = 1; SUCCEEDED(hr) && i < 5; i++)
    {
        hr = sp->Append(i);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(outVal);
    }

    return hr;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetNullAsObservableVector(__out Windows::Foundation::Collections::IObservableVector<int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetOneAnimal(__out Animals::IAnimal **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    ComPtr<IAnimal> spAnimal = Make<AnimalServer>();
    spAnimal.CopyTo(outVal);

    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetNullAsAnimal(__out Animals::IAnimal **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetOneMap(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    Microsoft::WRL::ComPtr<HashMap<HSTRING, int>> spMap;
    HRESULT hr = HashMap<HSTRING, int>::Make(&spMap); 
    IfFailedReturn(hr);

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

    spMap.CopyTo(outVal);
    return hr;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetNullAsMap(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetOnePropertyValue(__out Windows::Foundation::IPropertyValue **outVal)
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    Microsoft::WRL::ComPtr<Windows::Foundation::IPropertyValueStatics> spPropertyValueFactory;
    Windows::Internal::StringReference strFactory(L"Windows.Foundation.PropertyValue");
    Windows::Foundation::GetActivationFactory(strFactory.Get(), &spPropertyValueFactory);

    IInspectable *inspectable = nullptr;
    HRESULT hr = spPropertyValueFactory->CreateDouble(10.5, &inspectable);
    if (SUCCEEDED(hr))
    {
        hr = inspectable->QueryInterface(Windows::Foundation::IID_IPropertyValue, (void **) outVal);
        inspectable->Release();
    }

    return hr;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetNullAsPropertyValue(__out Windows::Foundation::IPropertyValue **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetOneEmptyGRCNInterface(__out Animals::IEmptyGRCN **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    ComPtr<IEmptyGRCN> spDuplicate = Make<CEmptyGRCNInterface>();
    spDuplicate.CopyTo(outVal);
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetOneEmptyGRCNNull(__out Animals::IEmptyGRCN **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    
    ComPtr<IEmptyGRCN> spDuplicate = Make<CEmptyGRCN>();
    spDuplicate.CopyTo(outVal);
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterface::GetOneEmptyGRCNFail(__out Animals::IEmptyGRCN **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    
    ComPtr<IEmptyGRCN> spDuplicate = Make<CEmptyFailingGRCNString>();
    spDuplicate.CopyTo(outVal);
    return S_OK;
}

Animals::FishServer::FishServer() : m_NumFins(5), m_name(NULL)
{}

Animals::FishServer::~FishServer()
{
    if (m_name)
    {
        WindowsDeleteString(m_name);
    }
}

IFACEMETHODIMP
Animals::FishServer::GetNumFins(__out int* numberOfFins)
{
    *numberOfFins = m_NumFins;
    return S_OK;
}

IFACEMETHODIMP
Animals::FishServer::SetNumFins(int numberOfFins)
{
    m_NumFins = numberOfFins;
    return S_OK;
}

IFACEMETHODIMP
Animals::FishServer::MarshalIFish(__in IFish * _in, __out IFish ** _out)
{
    if (nullptr == _in)
    {
        *_out = nullptr;
        return S_OK;
    }
    *_out = _in;
    (*_out)->AddRef();
    return S_OK;
}

IFACEMETHODIMP
Animals::FishServer::MarshalILikeToSwim(__in ILikeToSwim * _in, __out ILikeToSwim ** _out)
{
    if (nullptr == _in)
    {
        *_out = nullptr;
        return S_OK;
    }
    *_out = _in;
    (*_out)->AddRef();
    return S_OK;
}

IFACEMETHODIMP
Animals::FishServer::MarshalIFishToFish(__in IFish * _in, __out IFish ** _out)
{
    if (nullptr == _in)
    {
        *_out = nullptr;
        return S_OK;
    }
    *_out = _in;
    (*_out)->AddRef();
    return S_OK;
}

IFACEMETHODIMP
Animals::FishServer::MarshalILikeToSwimToFish(__in ILikeToSwim * _in, __out ILikeToSwim ** _out)
{
    if (nullptr == _in)
    {
        *_out = nullptr;
        return S_OK;
    }
    *_out = _in;
    (*_out)->AddRef();
    return S_OK;
}

IFACEMETHODIMP
Animals::FishServer::SingTheSwimmingSong(__out HSTRING* lyrics)
{
    auto song  = L"I feed from the bottom, you feed from the top \n"
                 L"I live upon morsels you happen to drop \n"
                 L"And coffee that somehow leaks out of your cup \n"
                 L"If nothing comes down then I'm forced to swim up \n";
    size_t len;
    StringCchLength(song, 1000, &len);
    return WindowsCreateString(song, (UINT32)len, lyrics);
}

IFACEMETHODIMP
Animals::FishServer::get_Name(__out HSTRING *value)
{
    if (value == NULL)
    {
        return E_POINTER;
    }

    if (m_name == NULL)
    {
        *value = NULL;
        return S_OK;
    }

    return WindowsDuplicateString(m_name, value);
}

IFACEMETHODIMP
Animals::FishServer::put_Name(__in HSTRING value)
{
    if (m_name)
    {
        WindowsDeleteString(m_name);
    }

    if (value == NULL)
    {
        m_name = NULL;
    }
    else
    {
        WindowsDuplicateString(value, &m_name);
    }
    return S_OK;
}


