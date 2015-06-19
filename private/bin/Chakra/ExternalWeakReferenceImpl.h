//---------------------------------------------------------------------------
// Copyright (C) 2011 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Represents the base class that would be used for all the that are wrapper around the js objects

namespace Projection
{
    class CWeakRefereceSourceImpl;

    class CExternalWeakReferenceImpl sealed : IWeakReference
    {
        friend class CUnknownImpl;

    private:
        unsigned long strongRefCount;
        unsigned long refCount;
        CUnknownImpl *m_pUnknown;

        CExternalWeakReferenceImpl(CUnknownImpl *pUnknown);
        ~CExternalWeakReferenceImpl() { }

        ULONG StrongAddRef();
        ULONG StrongRelease();

    public:
        STDMETHOD(QueryInterface)(REFIID riid,void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(Resolve)(__RPC__in REFIID riid, __RPC__deref_out_opt IInspectable **objectReference);

        IUnknown *ResolveUnknown();
        CUnknownImpl *ResolveUnknownImpl();

        CUnknownImpl* GetUnknownImpl() { return m_pUnknown; }

        ULONG GetStrongRefCount() { return strongRefCount; }
    };
};