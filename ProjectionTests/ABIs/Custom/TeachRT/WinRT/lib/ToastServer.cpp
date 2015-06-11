//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "kitchen_common.h"
#include "ToastServer.h"

#include <stdio.h>

HRESULT
Fabrikam::Kitchen::ToastServer::PrivateInitialize(HSTRING hstrMessage)
{
    return _hstrMessage.Initialize(hstrMessage);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToastServer::get_Message(__out HSTRING *phstrMessage)
{
    Windows::Internal::String result;
    HRESULT hr = result.Duplicate(_hstrMessage);

    if (SUCCEEDED(hr))
    {
        result.Detach(phstrMessage);
    }

    return hr;
}
