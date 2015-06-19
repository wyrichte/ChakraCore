//----------------------------------------------------------------------------

// Copyright (C) 2008 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Projection
{
    class NamespaceProjection;

    class NamespaceProjectionEnumerator sealed : public IVarEnumerator
    {
    private:
        unsigned long refCount;

        bool firstMove;

        AutoRecyclerRootPtr<NamespaceProjection> m_pNamespaceProjection;
        ImmutableList<LPCWSTR> * m_namespaceChildren;
        IVarEnumerator* m_pDefaultPropertyEnumerator;
        RecyclerRootVar m_instance;        

    public:
        NamespaceProjectionEnumerator(NamespaceProjection *pNamespaceProjection, Var instance, IVarEnumerator *pDefaultPropertyEnumerator);
        ~NamespaceProjectionEnumerator();

        STDMETHOD(QueryInterface)(REFIID riid,void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(MoveNext)(__out BOOL* itemsAvailable, __out_opt PropertyAttributes* attributes);
        STDMETHOD(GetCurrentName)(__out Var* item);
    };
}
