//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "ExtensionRegistration.h"
#include "ExtensionRegistrationp.h"
#include "ChefServer.h"

using namespace Fabrikam::Kitchen;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Internal;

#include <winrt\windowscollectionsp.h>
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;

#define EXTENSION_CATALOG_ID L"Windows.Foundation.ExtensionCatalog"
#define EXTENSION_CONTRACT_ID L"Fabrikam.Kitchen.PriceContract"

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::get_Name(__out HSTRING *phstrName)
{
    String strResult;
    
    // note the order here:
    //            (dest)             (source)
    HRESULT hr = strResult.Duplicate(_strName);

    if (SUCCEEDED(hr))
    {
         strResult.Detach(phstrName);
    }

    return hr;
}

// Not projected.  
STDMETHODIMP
Fabrikam::Kitchen::ChefServer::Initialize(__in HSTRING hstrName, __in IKitchen *pKitchen)
{
    HRESULT hr = _strName.Initialize(hstrName);
    StringReference strCatalogClassId(EXTENSION_CATALOG_ID, ARRAYSIZE(EXTENSION_CATALOG_ID)-1);
    StringReference strContractId(EXTENSION_CONTRACT_ID, ARRAYSIZE(EXTENSION_CONTRACT_ID)-1);
    ComPtr<IExtensionCatalog> spCatalog;
    ComPtr<IIterator<IExtensionRegistration*>> spExtensionIterator;
    boolean fHasCurrent = true;

    _spKitchen = pKitchen;
    _role = Fabrikam::Kitchen::ChefRole_AssistantChef;
    _capabilities = static_cast<Fabrikam::Kitchen::ChefCapabilities>(0);
    fCheckRoundOff = false;

    if (SUCCEEDED(hr))
    {
        hr = Windows::Foundation::ActivateInstance(strCatalogClassId.Get(), &spCatalog);
    }

    if (SUCCEEDED(hr))
    {
        hr = spCatalog->QueryCatalog(strContractId.Get(), &spExtensionIterator);
    }

    //  Activate each extension... for the sake of demonstration, 
    // only keep the last extension activated for further use. The others are discarded.
    if (SUCCEEDED(hr))
    {
        hr = spExtensionIterator->get_HasCurrent(&fHasCurrent);
    }

    while(SUCCEEDED(hr) && fHasCurrent)
    {
        ComPtr<IExtensionRegistration> spCurrentExtensionRegistration;
        hr = spExtensionIterator->get_Current(&spCurrentExtensionRegistration);

        if (SUCCEEDED(hr))
        {
            hr = ActivateExtension(
                spCurrentExtensionRegistration.Get(),
                _spExtension.ReleaseAndGetAddressOf());
        }

        if (SUCCEEDED(hr))
        {
            hr = spExtensionIterator->MoveNext(&fHasCurrent);
        }
    }

    return hr;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::MakeBreakfastToaster(__in IToaster *pToaster, __out int *pnCost)
{
    return MakeBreakfastToasterInt(pToaster, 1, pnCost);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::MakeBreakfastToasterInt(
    __in IToaster *pToaster, 
    __in int howMany, 
    __out int *pnCost)
{
    ComPtr<IToast> spBreakfast;
    HRESULT hr = S_OK;

    *pnCost = 0;
    int nRet = 0;

    if (pToaster == nullptr)
    {
        return E_INVALIDARG;
    }

    Microsoft::WRL::ComPtr<Vector<IToast *>> sp;
    Vector<IToast *>::Make(&sp);

    typedef IToast * LPTOAST;
    LPTOAST* myArray = new LPTOAST[howMany];

    for (int i = 0; SUCCEEDED(hr) && i < howMany; i++) 
    {
        IToast *pToast;
        hr = pToaster->MakeToast(_strName, &spBreakfast);
        spBreakfast.CopyTo(&pToast);
        sp->Append(pToast);
        myArray[i] = pToast; // since we are doing in we dont need to release here
        nRet += 10;
    }

    IVector<IToast *> *pToasts;
    sp.CopyTo(&pToasts);
    _evtCollectionToastComplete.InvokeAll(this, pToasts);
    pToasts->Release();

    _evtArrayToastComplete.InvokeAll(this, howMany, myArray);
    for (int i=0; i < howMany; i++)
    {
        myArray[i]->Release();
    }
    delete[] myArray;

    if (_spExtension)
    {
        hr = _spExtension->DeterminePrice(howMany, pnCost);
    }
    else
    {
        *pnCost = nRet;
    }

    return hr;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::MakeBreakfastToasterDouble(
    __in IToaster *pToaster, 
    __in double howMany, 
    __out int *pnCost)
{
    int nRoundUp = static_cast<int>(howMany);

    if (nRoundUp < howMany)
    {
        nRoundUp++;
    }

    if (fCheckRoundOff)
    {
        boolean isCorrect = false;
        _evtBreakFastToasterRoundOff.InvokeAll(this, &isCorrect, nRoundUp);
        if (isCorrect == false)
        {
            return E_FAIL;
        }
    }
    return MakeBreakfastToasterInt(pToaster, nRoundUp, pnCost);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::get_Role(__out ChefRole* pValue)
{
    *pValue = _role;
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::put_Role(__in ChefRole value)
{
    _role = value;
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::get_Capabilities(__out ChefCapabilities* pValue)
{
    *pValue = _capabilities;
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::put_Capabilities(__in ChefCapabilities value)
{
    _capabilities = value;
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::add_MultipleToastCompleteCollection(
    __in Fabrikam::Kitchen::IMultipleToastCompleteCollectionHandler *inHandler,
    __out EventRegistrationToken *pCookie)
{
    return _evtCollectionToastComplete.Add(inHandler, pCookie);
}
                
IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::remove_MultipleToastCompleteCollection(
    __in EventRegistrationToken iCookie)
{
    return _evtCollectionToastComplete.Remove(iCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::add_MultipleToastCompleteArray(
    __in Fabrikam::Kitchen::IMultipleToastCompleteArrayHandler *inHandler,
    __out EventRegistrationToken *pCookie)
{
    return _evtArrayToastComplete.Add(inHandler, pCookie);
}
                
IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::remove_MultipleToastCompleteArray(
    __in EventRegistrationToken iCookie)
{
    return _evtArrayToastComplete.Remove(iCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::add_MakeToastRoundOff(
    __in Fabrikam::Kitchen::IMakeToastRoundOffHandler *inHandler,
    __out EventRegistrationToken *pCookie)
{
    fCheckRoundOff = true;
    return _evtBreakFastToasterRoundOff.Add(inHandler, pCookie);
}
                
IFACEMETHODIMP
Fabrikam::Kitchen::ChefServer::remove_MakeToastRoundOff(
    __in EventRegistrationToken iCookie)
{
    return _evtBreakFastToasterRoundOff.Remove(iCookie);
}
              
IFACEMETHODIMP
Fabrikam::Kitchen::ChefFactory::CreateChef(
    __in HSTRING hstrName, 
    __in IKitchen *pKitchen, 
    __deref_out IChef **ppChef)
{
    HRESULT hr = S_OK;
    ComPtr<IChef> spIChef;

    if (ppChef != nullptr)
    {
        ComPtr<ChefServer> spChef = Make<ChefServer>();
        spChef->Initialize(hstrName, pKitchen);
        hr = spChef.As(&spIChef);
        if (SUCCEEDED(hr))
        {
            *ppChef = spIChef.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ChefFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}