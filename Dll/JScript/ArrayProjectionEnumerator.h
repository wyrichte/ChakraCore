//----------------------------------------------------------------------------

// Copyright (C) 2008 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Projection
{
    class ArrayProjection;

    class ArrayProjectionEnumerator sealed : public IVarEnumerator
    {
    private:
        unsigned long refCount;

        int m_iIndex;
        
        AutoRecyclerRootPtr<ArrayProjection> m_pArrayProjection;
        IVarEnumerator* m_pDefaultPropertyEnumerator;
        RecyclerRootVar m_instance;

    public:
        ArrayProjectionEnumerator(ArrayProjection *pArrayProjection, Var instance, IVarEnumerator *pDefaultPropertyEnumerator);
        ~ArrayProjectionEnumerator();

        STDMETHOD(QueryInterface)(REFIID riid,void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(MoveNext)(__out BOOL* itemsAvailable, __out_opt PropertyAttributes* attributes);
        STDMETHOD(GetCurrentName)(__out Var* item);
    };
}
