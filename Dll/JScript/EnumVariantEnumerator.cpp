//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

EnumVariantEnumerator::EnumVariantEnumerator(ScriptSite* scriptSite, IEnumVARIANT *enumerator) :
    JavascriptEnumerator(scriptSite->GetScriptSiteContext()),
    scriptSite(scriptSite),
    enumerator(enumerator),
    currentItem(null),
    fetched(false)
{
}

void EnumVariantEnumerator::Dispose(bool isShutdown)
{
    if (NULL != enumerator)
    {
        enumerator->Release();
        enumerator = NULL;
    }
}

void EnumVariantEnumerator::Finalize(bool isShutdown)
{
}

Var EnumVariantEnumerator::GetCurrentIndex()
{
    Js::Throw::NotImplemented();
}

Var EnumVariantEnumerator::GetCurrentValue()
{
    if (fetched)
    {
        return currentItem;
    }

    return this->GetLibrary()->GetUndefined();
}

BOOL EnumVariantEnumerator::MoveNext(Js::PropertyAttributes* attributes)
{
    HRESULT      hr;
    VARIANT      varItem = { VT_EMPTY };

    currentItem = null;
    fetched = false;
    
    if (NULL == enumerator)
        return fetched;

    hr = enumerator->Next(1, &varItem, NULL);

    if (S_OK == hr)
    {
        Js::Var var;
        hr = DispatchHelper::MarshalVariantToJsVar(&varItem, &var, this->GetScriptContext());

        VariantClear(&varItem);

        if (SUCCEEDED(hr))
        {            
            currentItem = var;
            fetched = true;
        }
    }

    return fetched;
}

void EnumVariantEnumerator::Reset()
{
    HRESULT      hr;

    currentItem = null;
    fetched = false;

    if (NULL == enumerator)
        return;

    hr = enumerator->Reset();

    if (FAILED(hr))
        return;
}
