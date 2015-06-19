//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Projection
{
    class MapWithStringKeyEnumerator sealed : public IVarEnumerator
    {
    private:
        unsigned long refCount;

        SpecialProjection * specialization;
        AutoRecyclerRootVar m_instance;        
        SList<HSTRING, HeapAllocator> keys;
        Js::DelayLoadWinRtString *winrtStringLib;

    public:
        MapWithStringKeyEnumerator(SpecialProjection * specialization, Var instance);
        ~MapWithStringKeyEnumerator();

        HRESULT InitializeKeys(Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<HSTRING, IInspectable *> *> *iterator);

        STDMETHOD(QueryInterface)(REFIID riid,void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(MoveNext)(__out BOOL* itemsAvailable, __out_opt PropertyAttributes* attributes);
        STDMETHOD(GetCurrentName)(__out Var* item);
    };
}
