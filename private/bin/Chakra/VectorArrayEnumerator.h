//----------------------------------------------------------------------------

// Copyright (C) 2008 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Projection
{
    class VectorArrayEnumerator sealed : public IVarEnumerator
    {
    private:
        unsigned long refCount;

        int m_iIndex;

        SpecialProjection * specialization;
        IVarEnumerator* m_pDefaultPropertyEnumerator;
        AutoRecyclerRootVar m_instance;        

    public:
        VectorArrayEnumerator(SpecialProjection * specialization, Var instance, IVarEnumerator *pDefaultPropertyEnumerator);
        ~VectorArrayEnumerator();

        STDMETHOD(QueryInterface)(REFIID riid,void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(MoveNext)(__out BOOL* itemsAvailable, __out_opt PropertyAttributes* attributes);
        STDMETHOD(GetCurrentName)(__out Var* item);
    };
}
