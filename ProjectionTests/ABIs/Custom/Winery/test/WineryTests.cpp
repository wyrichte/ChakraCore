//    Copyright (c) Microsoft Corporation.  All rights reserved.

#include "stdafx.h"
#include "winery.h"
#include "WineryServer.h"
#include "WineryTests.h"
#include <windowscollectionsp.h>

#define SZ_MESSAGE L"Welcome to Napa"
#define SHOP_NAME1 L"DevX Vinyard"
#define SHOP_NAME2 L"REX Vinyard"
#define SHOP_NAME3 L"Metadata Vinyard"
#define TEST_WIDTH 20.10
#define TEST_LENGTH 8.17
#define TEST_HEIGHT 4.58
#define TEST_WIDTH2 2.010
#define TEST_LENGTH2 8.17
#define TEST_AREA TEST_WIDTH * TEST_LENGTH
#define TEST_AREA2 TEST_WIDTH2 * TEST_LENGTH2

// Since the class is implemented using minATL, the module still needs 
// to be defined, even though this is a TAEF dll, not a COM server DLL. 
// dllexports.h should not be included.

using namespace Napa::Winery;
using namespace Microsoft::WRL;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation::Collections::Internal;

int numListener = 0;

class OneGenerator :
    public Microsoft::WRL::RuntimeClass<
	Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
	Napa::Winery::IRandomIntGenerator>
{
public:
    OneGenerator() 
    {
    }

    ~OneGenerator()
    {
    }

    IFACEMETHOD(Invoke)(__out int *pVal)
    {
        *pVal = 1;
        return S_OK;
    }
};

class TwoGenerator :
    public Microsoft::WRL::RuntimeClass<
	Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
	Napa::Winery::IRandomIntGenerator>
{
public:
    TwoGenerator() 
    {
    }

    ~TwoGenerator()
    {
    }

    IFACEMETHOD(Invoke)(__out int *pVal)
    {
        *pVal = 2;
        return S_OK;
    }
};

class AgeListener :
    public Microsoft::WRL::RuntimeClass<
	Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
	Napa::Winery::IAgeCompleteHandler>
{
public:
    AgeListener() 
    {
		numListener++;
    }

    ~AgeListener()
    {
		numListener--;
    }

    IFACEMETHOD(Invoke)(IProductionLine* /*sender*/, __in IWarehouse *pWarehouse)
    {
		HRESULT hr;
		VERIFY_SUCCEEDED(hr = pWarehouse->StoreAgedWine());
		return hr;
    }
};

namespace WEX { namespace TestExecution { namespace WineryTests {

	void NapaWineryTests::TestStruct(){ // struct, nested struct and properties
		ComPtr<IGeneralShop> spGeneralShop;
		Area myArea;
		Area myArea2;
		Dimension myDim;
		Dimension myDim2;

		spGeneralShop = Make<WineryServer>(); //Activation

		VERIFY_IS_TRUE(nullptr != spGeneralShop);

		if (spGeneralShop) 
		{
			myArea.Width = TEST_WIDTH;
			myArea.Length = TEST_LENGTH;

			myDim.BaseArea = myArea;
			myDim.Height = TEST_HEIGHT;

			VERIFY_SUCCEEDED(spGeneralShop->put_ShopArea(myArea));
			VERIFY_SUCCEEDED(spGeneralShop->put_ShopDimension(myDim));

			VERIFY_SUCCEEDED(spGeneralShop->get_ShopArea(&myArea2));
			VERIFY_SUCCEEDED(spGeneralShop->get_ShopDimension(&myDim2));

			VERIFY_IS_TRUE(myArea2.Width == TEST_WIDTH); // Test struct, property
			VERIFY_IS_TRUE(myArea2.Length == TEST_LENGTH);
			
			VERIFY_IS_TRUE(myDim2.BaseArea.Width == TEST_WIDTH); // Test nested struct
			VERIFY_IS_TRUE(myDim2.BaseArea.Length == TEST_LENGTH);
			VERIFY_IS_TRUE(myDim2.Height == TEST_HEIGHT);
		}
	}

	void NapaWineryTests::TestString(){ // interface require, string, regular method, enum, enum as member of struct
		ComPtr<IRetail> spRetail;
		ComPtr<IWineRetail> spWineRetail;
		Windows::Internal::String strMessage;
		Windows::Internal::String strMessageResponse;
		Reds topRed;

		spRetail = Make<WineryServer>(); //Activation
		
		VERIFY_IS_TRUE(nullptr != spRetail);

		if (spRetail) 
		{
			VERIFY_SUCCEEDED(strMessage.Initialize(SZ_MESSAGE, ARRAYSIZE(SZ_MESSAGE))); 
			VERIFY_SUCCEEDED(spRetail->put_WelcomeMessage(strMessage));

			VERIFY_SUCCEEDED(spRetail->get_WelcomeMessage(strMessageResponse.Address()));
			VERIFY_IS_TRUE(strMessageResponse == strMessage); // Test string

			spRetail.As(&spWineRetail);

			VERIFY_IS_TRUE(nullptr != spWineRetail);

			if (spWineRetail) // Test interface require
			{
				VERIFY_SUCCEEDED(spWineRetail->InitDatabase()); // Test regular method

				VERIFY_SUCCEEDED(spWineRetail->GetBestSellingRed(&topRed));
				VERIFY_IS_TRUE(topRed == Napa::Winery::reds_CabernetSauvignon); // Test enum, enum within struct

				VERIFY_SUCCEEDED(spWineRetail->SellReds(Napa::Winery::reds_Zinfandel, 5)); 

				VERIFY_SUCCEEDED(spWineRetail->GetBestSellingRed(&topRed));
				VERIFY_IS_TRUE(topRed == Napa::Winery::reds_Zinfandel);
			}
		}
	}

	void NapaWineryTests::TestEvent(){ // interface group, events
		// TODO: add tests on events
		ComPtr<IProductionLine> spProductionLine;
		ComPtr<IWarehouse> spWarehouse;
		ComPtr<IAgeCompleteHandler> spListener;
		ComPtr<IAgeCompleteHandler> spListener2;
		EventRegistrationToken tokEventHandler;
		int storedWine;

		spProductionLine = Make<WineryServer>(); //Activation
		spProductionLine.As(&spWarehouse);
		spListener = Make<AgeListener>();
		spListener2 = Make<AgeListener>();
		tokEventHandler = EventRegistrationToken();

		VERIFY_IS_TRUE(nullptr != spProductionLine);
		VERIFY_IS_TRUE(nullptr != spWarehouse);
		VERIFY_IS_TRUE(nullptr != spListener);
		VERIFY_IS_TRUE(nullptr != spListener2);

		if (spProductionLine && spWarehouse && spListener && spListener2)
		{
			VERIFY_SUCCEEDED(spWarehouse->ClearWarehouse());
			VERIFY_IS_TRUE(numListener == 2);

			VERIFY_SUCCEEDED(spProductionLine->add_AgeCompleteEvent(spListener.Get(), &tokEventHandler));

			VERIFY_SUCCEEDED(spProductionLine->SendToWarehouse(spWarehouse.Get())); //invoke
			VERIFY_SUCCEEDED(spWarehouse->get_WineInStorage(&storedWine));
			VERIFY_IS_TRUE(storedWine == 1);

			VERIFY_SUCCEEDED(spProductionLine->add_AgeCompleteEvent(spListener2.Get(), &tokEventHandler));

			VERIFY_SUCCEEDED(spProductionLine->SendToWarehouse(spWarehouse.Get())); //invoke
			VERIFY_SUCCEEDED(spWarehouse->get_WineInStorage(&storedWine));
			VERIFY_IS_TRUE(storedWine == 3);

			VERIFY_SUCCEEDED(spProductionLine->remove_AgeCompleteEvent(tokEventHandler));
			VERIFY_SUCCEEDED(spProductionLine->remove_AgeCompleteEvent(tokEventHandler));

			VERIFY_SUCCEEDED(spProductionLine->Produce()); // Test interface group
		}
	}

	void NapaWineryTests::TestDelegate(){ // delegate, method overload
		ComPtr<IWarehouse> spWarehouse;
		ComPtr<IRandomIntGenerator> spIntGen;
		int x;
		int y;

		spWarehouse = Make<WineryServer>();

		VERIFY_IS_TRUE(nullptr != spWarehouse);

		if (spWarehouse)
		{
			spIntGen = Make<TwoGenerator>();

			VERIFY_IS_TRUE(nullptr != spIntGen);

			if (spIntGen)
			{
				spWarehouse->ThrowSelectedVinegar(spIntGen.Get(), Napa::Winery::reds_Merlot, &x); // Test method overload
				spWarehouse->ThrowZinfandelVinegar(spIntGen.Get(), &y);

				VERIFY_IS_TRUE(x == 2 && y == 2);
			}

			spIntGen = Make<OneGenerator>();

			VERIFY_IS_TRUE(nullptr != spIntGen);

			if (spIntGen)
			{
				spWarehouse->ThrowSelectedVinegar(spIntGen.Get(), Napa::Winery::reds_Merlot, &x); // Test method overload
				spWarehouse->ThrowZinfandelVinegar(spIntGen.Get(), &y);

				VERIFY_IS_TRUE(x == 1 && y == 1);
			}
		}
	}

	void NapaWineryTests::TestParameterizedInterface(){ // parameterized interface
		ComPtr<IGeneralShop> spGeneralShop;
		ComPtr<IGeneralShop> spGeneralShop2;
		ComPtr<IVector<IGeneralShop*>> spShopVector;
		ComPtr<Vector<IGeneralShop*>> spTemp;

		Area myArea;
		Area myArea2;
		double result;

		spGeneralShop = Make<WineryServer>(); // Activation
		spGeneralShop2 = Make<WineryServer>(); // Activation
		Vector<IGeneralShop*>::Make(&spTemp);
		spShopVector = spTemp;

		VERIFY_IS_TRUE(nullptr != spShopVector);
		VERIFY_IS_TRUE(nullptr != spGeneralShop);
		VERIFY_IS_TRUE(nullptr != spGeneralShop2);

		if (spGeneralShop && spGeneralShop2 && spShopVector) 
		{
			myArea.Width = TEST_WIDTH;
			myArea.Length = TEST_LENGTH;

			myArea2.Width = TEST_WIDTH2;
			myArea2.Length = TEST_LENGTH2;

			VERIFY_SUCCEEDED(spGeneralShop->put_ShopArea(myArea));
			VERIFY_SUCCEEDED(spGeneralShop2->put_ShopArea(myArea2));

			VERIFY_SUCCEEDED(spShopVector->Append(spGeneralShop2.Get()));
			VERIFY_SUCCEEDED(spShopVector->Append(spGeneralShop.Get()));

			VERIFY_SUCCEEDED(spGeneralShop->CompareAreaOfNeighbors(spShopVector.Get(), &result));
		
			VERIFY_IS_TRUE(result == (TEST_AREA2 > TEST_AREA ? TEST_AREA2 : TEST_AREA));
		}
	}

	void NapaWineryTests::TestActivation(){ // parameterized interface
		ComPtr<IWineryFactory> spWineryFactory;
		ComPtr<IGeneralShop> spGeneralShop;
		ComPtr<IWarehouse> spWarehouse;
		IGeneralShop* pGeneralShop;
		int val;

		spWineryFactory = Make<WineryFactory>();

		spWineryFactory->CreateWinery(&pGeneralShop);
		
		spGeneralShop = Make<WineryServer>();
		spGeneralShop.Attach(pGeneralShop);

		spGeneralShop.As(&spWarehouse);

		spWarehouse->get_WineInStorage(&val);
		VERIFY_IS_TRUE(val == 0);
	}

	void NapaWineryTests::TestMap(){ // IMap
		ComPtr<IGeneralShop> spGeneralShop;
		ComPtr<IGeneralShop> spGeneralShop2;
		ComPtr<IGeneralShop> spGeneralShop3;
		ComPtr<IGeneralShop> spTargetShop;
		ComPtr<IVector<IGeneralShop*>> spShopVector;
		ComPtr<Vector<IGeneralShop*>> spTempVector;
		ComPtr<IMap<HSTRING, IGeneralShop*>> spNameMap;
		ComPtr<HashMap<HSTRING, IGeneralShop*>> spTempMap;
		IMap<HSTRING, IGeneralShop*> *pNameMap;
		IGeneralShop* pGeneralShop;
		Windows::Internal::String shopName1;
		Windows::Internal::String shopName2;
		Windows::Internal::String shopName3;
		Windows::Internal::String shopNameResponse;
		boolean found;

		spGeneralShop = Make<WineryServer>();
		spGeneralShop2 = Make<WineryServer>();
		spGeneralShop3 = Make<WineryServer>();
		spTargetShop = Make<WineryServer>();
		Vector<IGeneralShop*>::Make(&spTempVector);
		spShopVector = spTempVector;
		HashMap<HSTRING, IGeneralShop*>::Make(&spTempMap);
		spNameMap = spTempMap;

		VERIFY_IS_TRUE(nullptr != spShopVector);
		VERIFY_IS_TRUE(nullptr != spNameMap);
		VERIFY_IS_TRUE(nullptr != spGeneralShop);
		VERIFY_IS_TRUE(nullptr != spGeneralShop2);
		VERIFY_IS_TRUE(nullptr != spGeneralShop3);
		VERIFY_IS_TRUE(nullptr != spTargetShop);

		if (spShopVector && spNameMap && spGeneralShop && spGeneralShop2 && spGeneralShop3 && spTargetShop)
		{
			// set shop name
			VERIFY_SUCCEEDED(shopName1.Initialize(SHOP_NAME1, ARRAYSIZE(SZ_MESSAGE))); 
			VERIFY_SUCCEEDED(spGeneralShop->put_ShopName(shopName1));
			VERIFY_SUCCEEDED(shopName2.Initialize(SHOP_NAME2, ARRAYSIZE(SZ_MESSAGE))); 
			VERIFY_SUCCEEDED(spGeneralShop2->put_ShopName(shopName2));
			VERIFY_SUCCEEDED(shopName3.Initialize(SHOP_NAME3, ARRAYSIZE(SZ_MESSAGE))); 
			VERIFY_SUCCEEDED(spGeneralShop3->put_ShopName(shopName3));

			// set neighbor vector
			VERIFY_SUCCEEDED(spShopVector->Append(spGeneralShop2.Get()));
			VERIFY_SUCCEEDED(spShopVector->Append(spGeneralShop3.Get()));

			VERIFY_SUCCEEDED(spGeneralShop->NeighborShopName(spShopVector.Get(), &pNameMap));
			spNameMap.Attach(pNameMap);

			VERIFY_SUCCEEDED(spNameMap->HasKey(shopName1, &found));
			VERIFY_IS_TRUE(found == 0);
			VERIFY_SUCCEEDED(spNameMap->HasKey(shopName2, &found));
			VERIFY_IS_TRUE(found != 0);
			VERIFY_SUCCEEDED(spNameMap->HasKey(shopName3, &found));
			VERIFY_IS_TRUE(found != 0);

			VERIFY_SUCCEEDED(spNameMap->Lookup(shopName2, &pGeneralShop));
			spTargetShop.Attach(pGeneralShop);
			VERIFY_SUCCEEDED(spTargetShop->get_ShopName(shopNameResponse.Address()));
			VERIFY_IS_TRUE(shopNameResponse == shopName2);

			VERIFY_SUCCEEDED(spNameMap->Lookup(shopName3, &pGeneralShop));
			spTargetShop.Attach(pGeneralShop);
			VERIFY_SUCCEEDED(spTargetShop->get_ShopName(shopNameResponse.Address()));
			VERIFY_IS_TRUE(shopNameResponse == shopName3);
		}
	}
}}}