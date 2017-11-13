// Copyright (C) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <stdio.h>
#include <windowsstringp.h>

#define SZ_CLASSNAME L"Animals.Animal"
#define SZ_CLASSNAME2 L"Animals.Fish"

int __cdecl wmain()
{
    wprintf(L"Starting!\n");

    HRESULT hr = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        {
            // the additional scope here is used to ensure that COM pointers 
            // release before calling Windows::Foundation::Uninitialize.
            Microsoft::WRL::ComPtr<IInspectable> pInspectable;
            Microsoft::WRL::ComPtr<Animals::IAnimal> pTeachWinRT;
            Microsoft::WRL::ComPtr<Animals::IFish> pFish;

            Windows::Internal::String strActivatableClassId;
            Windows::Internal::String strActivatableClassId2;
            hr = strActivatableClassId.Initialize(SZ_CLASSNAME, ARRAYSIZE(SZ_CLASSNAME));

            if (SUCCEEDED(hr))
            {
                wprintf(L"Windows runtime initialized\n");
                hr = Windows::Foundation::ActivateInstance(strActivatableClassId, &pInspectable);
            }

            if (SUCCEEDED(hr))
            {
                wprintf(L"Animal Class instance created\n");

                hr = pInspectable.As<Animals::IAnimal>(&pTeachWinRT);
                if (S_OK == hr)
                {
                    wprintf(L"Found the animal interface\n");
                }

        		int num;
		        hr = pTeachWinRT->GetNumLegs(&num);
		        if (S_OK == hr)
		        {
			        wprintf(L"Number of legs: %d\n", num);
        		}
		        else
        		{
        			wprintf(L"Error calling GetNumLegs\n");
		        }
                int weight;
                pTeachWinRT->get_Weight(&weight);
                wprintf(L"weight: %d\n", weight);
                pTeachWinRT->put_Weight(7);
                pTeachWinRT->get_Weight(&weight);
                wprintf(L"weight: %d\n", weight);

                Animals::Dimensions dim;
                pTeachWinRT->GetDimensions(&dim);
                wprintf(L"Dimensions: %d x %d\n", dim.Length, dim.Width);

                Animals::OuterStruct strct;
                pTeachWinRT->GetOuterStruct(&strct);
                wprintf(L"OuterStruct.Inner.a: %d\n", strct.Inner.a );

                hr = Windows::Foundation::ActivateInstance(strActivatableClassId2, &pInspectable);

                wprintf(L"Fish Class instance created\n");

                hr = pInspectable.As<Animals::IFish>(&pFish);
                if (S_OK == hr)
                {
                    wprintf(L"Found the fish interface\n");
                }

		        hr = pFish->GetNumFins(&num);
		        if (S_OK == hr)
		        {
			        wprintf(L"Number of Fins: %d\n", num);
        		}
		        else
        		{
        			wprintf(L"Error calling GetNumFins\n");
		        }
            }
        }

        Windows::Foundation::Uninitialize();
    }


    if (FAILED(hr))
    {
        wprintf(L"ERROR: operation failed with HRESULT %#08x\n",hr);
    }

    wprintf(L"Complete!\n");

    return SUCCEEDED(hr) ? 0 : 1;
}


