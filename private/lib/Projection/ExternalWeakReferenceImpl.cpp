//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IIterator in JavaScript
// *******************************************************


#include "ProjectionPch.h"

namespace Projection
{
    CExternalWeakReferenceImpl::CExternalWeakReferenceImpl(CUnknownImpl *pUnknown)
        : refCount(1),
        strongRefCount(0),
        m_pUnknown(pUnknown)
    {
    }

    STDMETHODIMP CExternalWeakReferenceImpl::QueryInterface(REFIID riid, void **ppv)
    {
        IfNullReturnError(ppv, E_POINTER);

        if (IsEqualGUID(riid, IID_IUnknown) 
            || IsEqualGUID(riid, IID_IWeakReference)) 
        {
            *ppv = this;
            AddRef();
            return S_OK;
        }
        
        *ppv = NULL;
        return E_NOINTERFACE;

    }

    STDMETHODIMP_(ULONG) CExternalWeakReferenceImpl::AddRef()
    {
        return InterlockedIncrement(&refCount); 
    }
        
    STDMETHODIMP_(ULONG) CExternalWeakReferenceImpl::Release()
    {
        ULONG uRetVal = InterlockedDecrement(&refCount);

        if (uRetVal == 0)
        {
            Assert(strongRefCount == 0);
            delete this;
        }

        return uRetVal;
    }

    STDMETHODIMP CExternalWeakReferenceImpl::Resolve(__RPC__in REFIID riid, __RPC__deref_out_opt IInspectable **objectReference)
    {
        IfNullReturnError(objectReference, E_POINTER);
        *objectReference = nullptr;

        if (strongRefCount == 0)
        {
            return S_OK;
        }

        if (IsEqualGUID(riid, IID_IWeakReferenceSource))
        {
            return E_UNEXPECTED;
        }

        return m_pUnknown->QueryInterface(riid, (void **)objectReference);
    }

    ULONG CExternalWeakReferenceImpl::StrongAddRef()
    {
        return InterlockedIncrement(&strongRefCount); 
    }
        
    ULONG CExternalWeakReferenceImpl::StrongRelease()
    {
        return InterlockedDecrement(&strongRefCount);
    }

    IUnknown * CExternalWeakReferenceImpl::ResolveUnknown()
    {
        if (strongRefCount == 0)
        {
            return nullptr;
        }

        return m_pUnknown->GetUnknown();
    }

    CUnknownImpl * CExternalWeakReferenceImpl::ResolveUnknownImpl()
    {
        if (strongRefCount == 0)
        {
            return nullptr;
        }

        return m_pUnknown;
    }
}
