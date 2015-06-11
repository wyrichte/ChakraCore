//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

#if DBG
VirtualTableRegistry::TableEntry VirtualTableRegistry::m_knownVtables[MAX_KNOWN_VTABLES];
UINT VirtualTableRegistry::m_knownVtableCount = 0;

void VirtualTableRegistry::Add(INT_PTR vtable, LPCSTR className)
{
    Assert(m_knownVtableCount < MAX_KNOWN_VTABLES);
    if (m_knownVtableCount < MAX_KNOWN_VTABLES)
    {
        m_knownVtables[m_knownVtableCount].vtable = vtable;
        m_knownVtables[m_knownVtableCount].className = className;
        ++m_knownVtableCount;
    }
}

VtableHashMap *
VirtualTableRegistry::CreateVtableHashMap(ArenaAllocator * alloc)
{
    VtableHashMap * vtableHashMap = Anew(alloc, VtableHashMap, /*size=*/ MAX_KNOWN_VTABLES, alloc);

    // All classes that derive from RecyclableObject must include DEFINE_VTABLE_CTOR which invokes VirtualTableRegistry::Add
    // at class initialization time. Here we add them to our hash table for easy lookup. Note that on a free build
    // the vtables are merged and thus not all of our types will be registered. So can only use this method
    // in a non-fre build. If wanted use in a fre build, then must explicitly add the vtables to the hash and
    // validate that have got them all by comparing in chk build against VirtualTableRegistry.
    for (UINT i=0; i < VirtualTableRegistry::m_knownVtableCount; i++)
    {
        INT_PTR vtable = VirtualTableRegistry::m_knownVtables[i].vtable;
        LPCSTR className = VirtualTableRegistry::m_knownVtables[i].className;
        vtableHashMap->Add(vtable, className);
    }
    return vtableHashMap;
}
#endif