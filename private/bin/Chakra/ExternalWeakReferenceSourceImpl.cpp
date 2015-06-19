//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IIterator in JavaScript
// *******************************************************


#include "stdafx.h"

namespace Projection
{

    CExternalWeakReferenceSourceImpl::CExternalWeakReferenceSourceImpl(CUnknownImpl *pUnknown)
        : m_pUnknown(pUnknown)
    {
    }

    STDMETHODIMP CExternalWeakReferenceSourceImpl::QueryInterface(REFIID riid, void **ppv)
    {
        return m_pUnknown->QueryInterface(riid, ppv);
    }

    STDMETHODIMP_(ULONG) CExternalWeakReferenceSourceImpl::AddRef()
    {
        return m_pUnknown->AddRef();
    }
        
    STDMETHODIMP_(ULONG) CExternalWeakReferenceSourceImpl::Release()
    {
        return m_pUnknown->Release();
    }

    STDMETHODIMP CExternalWeakReferenceSourceImpl::GetWeakReference(__RPC__deref_out_opt IWeakReference **weakReference)
    {
        IfNullReturnError(weakReference, E_INVALIDARG);
        *weakReference = nullptr;

        return m_pUnknown->GetWeakReference(weakReference);
    }
}
