// Copyright (C) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <stdio.h>
#include <wrl\client.h>

#include "animal_i.c"
using namespace Microsoft::WRL;

#define SZ_CLASSNAME L"Animals.AnimalServer"

typedef HRESULT (STDAPICALLTYPE *PDLLGETCLASSOBJECT)(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
typedef HRESULT (STDAPICALLTYPE *PDLLCANUNLOADNOW)();

HRESULT GetEntryFuncs(wchar_t* szFile, HMODULE *phLib, 
	PDLLGETCLASSOBJECT* pDllGetClassObject, PDLLCANUNLOADNOW* pDllCanUnloadNow)
{
	*phLib = LoadLibraryW(szFile);
	if (*phLib != nullptr)
	{
		*pDllGetClassObject = (PDLLGETCLASSOBJECT)GetProcAddress(*phLib, "DllGetClassObject");
		*pDllCanUnloadNow = (PDLLCANUNLOADNOW)GetProcAddress(*phLib, "DllCanUnloadNow");
		if (nullptr != *pDllGetClassObject && nullptr != *pDllCanUnloadNow)
		{
			return S_OK;
		}
		else
		{
			FreeLibrary(*phLib);
			*phLib = nullptr;
			return S_FALSE;
		}
	}

	return S_FALSE;
}

int __cdecl wmain()
{
	wprintf(L"Starting!\n");

	HMODULE hLib = nullptr;
	PDLLGETCLASSOBJECT pDllGetClassObject = nullptr;
	PDLLCANUNLOADNOW pDllCanUnloadNow = nullptr;

	if(GetEntryFuncs(L"AnimalServer.dll", &hLib, &pDllGetClassObject, &pDllCanUnloadNow) != S_OK)
	{
		wprintf(L"ERROR getting entry funcs\n");
		return 1;
	}

	ComPtr<IClassFactory> spFactory;

    HRESULT hr = pDllGetClassObject(CLSID_AnimalFactory, __uuidof(IClassFactory), &spFactory);
    if(hr != S_OK)
	{
		wprintf(L"error getting animal factory\n");
        wprintf(L"hr: %d\n", hr);
		return 1;
	}
	ComPtr<IAnimalFactory> spAnimalFactory;
    spFactory->CreateInstance(nullptr, __uuidof(spAnimalFactory), (void**)&spAnimalFactory );

	ComPtr<IAnimal> spAnimal;
	spAnimalFactory->Create((IAnimal**)&spAnimal);

	int num;
	spAnimal->GetNumLegs(&num);
	wprintf(L"Number of legs: %d\n", num);

    int weight;
    spAnimal->get_Weight(&weight);
    wprintf(L"weight: %d\n", weight);
    spAnimal->put_Weight(7);
    spAnimal->get_Weight(&weight);
    wprintf(L"weight: %d\n", weight);

    Dimensions dim;
    spAnimal->GetDimensions(&dim);
    wprintf(L"Dimensions: %d x %d\n", dim.Length, dim.Width);

    OuterStruct strct;
    spAnimal->GetOuterStruct(&strct);
    wprintf(L"OuterStruct.Inner.a: %d\n", strct.Inner.a );
	wprintf(L"Complete!\n");
}
