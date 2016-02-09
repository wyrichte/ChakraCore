//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "CookieServer.h"

// ICookie::put_Doneness
STDMETHODIMP Fabrikam::Kitchen::CookieServer::put_Doneness(
    __in CookieDoneness doneness)
{
    if (doneness < Fabrikam::Kitchen::CookieDoneness_Raw ||
        doneness > Fabrikam::Kitchen::CookieDoneness_Burnt)
    {
        return E_INVALIDARG;
    }
    else
    {
        _doneness = doneness;
    }
    return S_OK;
}

// ICookie::get_Doneness
IFACEMETHODIMP Fabrikam::Kitchen::CookieServer::get_Doneness(
    __out CookieDoneness *pDoneness)
{
    *pDoneness = _doneness;
    return S_OK;
}
