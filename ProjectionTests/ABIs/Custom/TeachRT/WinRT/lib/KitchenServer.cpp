//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "KitchenServer.h"

using namespace Fabrikam::Kitchen;
using namespace Microsoft::WRL;

STDMETHODIMP KitchenServer::Initialize()
{
    HRESULT hr = S_OK;

    return hr;
}

IFACEMETHODIMP KitchenFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;

    ComPtr<KitchenServer> spObj = Make<KitchenServer>();

    HRESULT hr = (spObj) ? S_OK : E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
    {
        hr = spObj->Initialize();
    }

    if (SUCCEEDED(hr))
    {
        *ppInspectable = spObj.Detach();
    }

    return hr;
}