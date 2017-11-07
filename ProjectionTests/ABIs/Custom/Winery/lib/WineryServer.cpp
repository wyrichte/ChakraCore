//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "WineryServer.h"


// Define this for convenience, but don't put this in a header, otherwise 
// it defeats the purpose of using a namespace.
using namespace Microsoft::WRL;
using namespace Winery;
using namespace Windows::Foundation::Diagnostics;
//using namespace Windows::Foundation::Collections;
//using namespace Windows::Foundation::Collections::Internal;

Winery::WineryServer::WineryServer()
    :_shopMsg(nullptr),_shopName(nullptr)
{
}
Winery::WineryServer::~WineryServer()
{
    if(_shopMsg) 
        WindowsDeleteString(_shopMsg);
    if(_shopName) 
        WindowsDeleteString(_shopName);
}

IFACEMETHODIMP
Winery::WineryServer::get_ShopArea(__out Area *pArea)
{
    *pArea = _shopArea;
    return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::put_ShopArea(Area area)
{
    _shopArea = area;
    return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::get_ShopDimension(__out Dimension *pDim)
{
    *pDim = _shopDim;
    return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::put_ShopDimension(Dimension dim)
{
    _shopDim = dim;
    return S_OK;
}
/*
IFACEMETHODIMP
Winery::WineryServer::CompareAreaOfNeighbors(
	IVector<IGeneralShop*>* neighbors, 
	__out double *largest)
{
	unsigned int vectorSize = 0;
	double tempLargest = 0;
	Winery::IGeneralShop *shop;
	Area tempArea;

	neighbors->get_Size(&vectorSize);
	
	for (unsigned int i = 0; i < vectorSize; i++)
	{
		neighbors->GetAt(i, &shop);
		shop->get_ShopArea(&tempArea);
		if (tempArea.Width * tempArea.Length > tempLargest)
		{
			tempLargest = tempArea.Width * tempArea.Length;
		}
	}

	*largest = tempLargest;

	return S_OK;
}
*/
IFACEMETHODIMP
Winery::WineryServer::put_ShopName(HSTRING msg)
{
    if(_shopName) WindowsDeleteString(_shopName);
	return WindowsDuplicateString(msg, &_shopName);
}

IFACEMETHODIMP
Winery::WineryServer::get_ShopName(__out HSTRING *msg)
{
	return WindowsDuplicateString(_shopName, msg);
}

/*
IFACEMETHODIMP
Winery::WineryServer::NeighborShopName(
	Windows::Foundation::Collections::IVector<IGeneralShop*> *neighbors,
	__out Windows::Foundation::Collections::IMap<HSTRING, IGeneralShop*> **namesMappings)
{
	ComPtr<IMap<HSTRING, IGeneralShop*>> spNameMap;
	ComPtr<HashMap<HSTRING, IGeneralShop*>> spTemp;
	unsigned int vectorSize = 0;
	Winery::IGeneralShop *pShop;
	Windows::Internal::String shopName;
	boolean replaced;

	HashMap<HSTRING, IGeneralShop*>::Make(&spTemp);
	spNameMap = spTemp;

	neighbors->get_Size(&vectorSize);
	
	for (unsigned int i = 0; i < vectorSize; i++)
	{
		neighbors->GetAt(i, &pShop);
		pShop->get_ShopName(shopName.Address());
		spNameMap->Insert(shopName, pShop, &replaced);
	}

	*namesMappings = spNameMap.Detach();
	
	return S_OK;
}
*/
IFACEMETHODIMP
Winery::WineryServer::add_AgeCompleteEvent(
    __in Winery::IAgeCompleteHandler *handler,
    __out EventRegistrationToken *pCookie)
{
    return _evtAgeComplete.Add(handler, pCookie);
}
                
IFACEMETHODIMP
Winery::WineryServer::remove_AgeCompleteEvent(
    __in EventRegistrationToken iCookie)
{
    return _evtAgeComplete.Remove(iCookie);
}

IFACEMETHODIMP
Winery::WineryServer::add_AgeEmptyEvent(
    __in Winery::IAgeCompleteHandler *,
    __out EventRegistrationToken *pCookie)
{
    pCookie->value = 0;
    return S_OK;
}
                
IFACEMETHODIMP
Winery::WineryServer::remove_AgeEmptyEvent(
    __in EventRegistrationToken )
{
    return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::SendToWarehouse(Winery::IWarehouse *pWarehouse)
{
	_evtAgeComplete.InvokeAll(this, pWarehouse);
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::Produce()
{
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::ThrowSelectedVinegar(
	__in Winery::IRandomIntGenerator *randomGenerator,
	__in Winery::Reds wineType,
	__out int *thrown)
{
	if ( (int)wineType >= 4 )
	{
		return E_FAIL;
	}
	randomGenerator->Invoke(thrown);
	
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::ThrowZinfandelVinegar(
	__in Winery::IRandomIntGenerator *randomGenerator,
	__out int *thrown)
{
	randomGenerator->Invoke(thrown);

	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::ClearWarehouse()
{
	_numAgedWine = 0;
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::StoreAgedWine()
{
	_numAgedWine++;
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::get_WineInStorage(__out int *val)
{
	*val = _numAgedWine;
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::put_WelcomeMessage(HSTRING msg)
{
    if(_shopMsg) WindowsDeleteString(_shopMsg);
    return WindowsDuplicateString(msg, &_shopMsg);
}

IFACEMETHODIMP
Winery::WineryServer::get_WelcomeMessage(__out HSTRING *msg)
{
    return WindowsDuplicateString(_shopMsg, msg);
}

IFACEMETHODIMP
Winery::WineryServer::InitDatabase()
{
	int i;
	for (i = 0; i < 4; i++)
	{
		_redSold[i] = 0;
	}
	for (i = 0; i < 3; i++)
	{
		_whiteSold[i] = 0;
	}
	for (i = 0; i < 3; i++)
	{
		_sweetSold[i] = 0;
	}
	_topRed = reds_CabernetSauvignon; // namespace is left out here intentionally to test bug 93729 (and regression)
	_topWhite = Winery::whites_Chardonnay;
	_topSweet = Winery::sweets_Riesling;

	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::SellReds(
	__in Reds wineType,
	__in int amount)
{
	_redSold[(int)wineType] += amount;

	if (_redSold[(int)wineType] > _redSold[(int)_topRed])
	{
		_topRed = wineType;
	}
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::SellWhites(
	__in Whites wineType,
	__in int amount)
{
	_whiteSold[(int)wineType] += amount;

	if (_whiteSold[(int)wineType] > _whiteSold[(int)_topWhite])
	{
		_topWhite = wineType;
	}
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::SellSweets(
	__in Sweets wineType,
	__in int amount)
{
	_sweetSold[(int)wineType] += amount;

	if (_sweetSold[(int)wineType] > _sweetSold[(int)_topSweet])
	{
		_topSweet = wineType;
	}
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::GetBestSellingRed(__out Reds *pVal)
{
	*pVal = _topRed;
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::GetBestSellingWhite(__out Whites *pVal)
{
	*pVal = _topWhite;
	return S_OK;
}

IFACEMETHODIMP
Winery::WineryServer::GetBestSellingSweet(__out Sweets *pVal)
{
	*pVal = _topSweet;
	return S_OK;
}


IFACEMETHODIMP Winery::WineryServer::MarshalIRetail(__in IRetail * _in, __out IRetail ** _out) {*_out = _in; return S_OK;}
IFACEMETHODIMP Winery::WineryServer::MarshalIGeneralShop(__in IGeneralShop * _in, __out IGeneralShop ** _out) {*_out = _in; return S_OK;}
IFACEMETHODIMP Winery::WineryServer::MarshalIProductionLine(__in IProductionLine * _in, __out IProductionLine ** _out) {*_out = _in; return S_OK;}
IFACEMETHODIMP Winery::WineryServer::MarshalIWineRetail(__in IWineRetail * _in, __out IWineRetail ** _out) {*_out = _in; return S_OK;}
IFACEMETHODIMP Winery::WineryServer::MarshalIWarehouse(__in IWarehouse * _in, __out IWarehouse ** _out) {*_out = _in; return S_OK;}






IFACEMETHODIMP
Winery::WineryFactory::CreateWinery(__in int val, __deref_out IGeneralShop **ppWinery)
{

    HRESULT hr = S_OK;
    //[mboris] This is a temporary workaround to get the ABI to build after the midlrt requires Create funcitons to have [in] params
    val++;
    ComPtr<IGeneralShop> spGeneralShop;
	ComPtr<IWarehouse> spWarehouse;
	ComPtr<IWineRetail> spWineRetail;

    if (ppWinery != nullptr)
    {
		spGeneralShop = Make<WineryServer>();
		spGeneralShop.As(&spWarehouse);
		spGeneralShop.As(&spWineRetail);
		
		spWarehouse->ClearWarehouse();
		spWineRetail->InitDatabase();

		*ppWinery = spGeneralShop.Get();
        (*ppWinery)->AddRef();
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

IFACEMETHODIMP
Winery::WineryFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}
