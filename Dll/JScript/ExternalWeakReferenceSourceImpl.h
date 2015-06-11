//---------------------------------------------------------------------------
// Copyright (C) 2011 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Represents the base class that would be used for all the that are wrapper around the js objects

namespace Projection
{
    class CExternalWeakReferenceSourceImpl sealed : IWeakReferenceSource
    {
        friend class CUnknownImpl;

    private:
        CUnknownImpl *m_pUnknown;

        CExternalWeakReferenceSourceImpl(CUnknownImpl *pUnknown);
        ~CExternalWeakReferenceSourceImpl() { }

    public:
        STDMETHOD(QueryInterface)(REFIID riid,void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(GetWeakReference)(__RPC__deref_out_opt IWeakReference **weakReference);
    };
};