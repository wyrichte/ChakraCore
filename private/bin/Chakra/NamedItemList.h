//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
class ScriptEngine;

// Named item descriptor
struct NamedItem
{
    NamedItem * pnidNext;    // For linked list of named items
    NamedItem * pnidPrev;
    NamedItem * pnidNextHash;
    NamedItem * pnidPrevHash;
    BSTR        bstrItemName;
    ulong       ulHash;
    long        lwCookie;
    DWORD       dwFlags;            // Flags provided by AddNamedItem
    Js::ModuleID       moduleID;                // "module" number of item
    byte        fRegistered : 1;    // TRUE if already registered in binder
    byte        fHasCode    : 1;    // TRUE if any scriptlets have been added
    IDispatch * pdisp;              // The object (NULL if not fetched yet)

    NamedItem (void);
    ~NamedItem (void);
    HRESULT Clone (NamedItem ** ppnid);
};

const int knilTableSize = 23;

class NamedItemList
{
public:
    NamedItemList();
    ~NamedItemList();

    void  AddFirst (NamedItem * pnid);
    void  AddLast  (NamedItem * pnid);
    void  Remove   (NamedItem * pnid);
    BOOL  FEmpty (void);
    Js::ModuleID FindHighestModuleID (void);
    void  DestroyAll (void);
    void  Reset (void);
    HRESULT Clone (NamedItemList * pnil);
    NamedItem * First (void);
    NamedItem * Find  (LPCOLESTR psz);

private:
    NamedItem * m_pnidFirst;
    NamedItem * m_pnidLast;
    NamedItem * m_pnidTable[knilTableSize];
    MUTX        m_mutx;

    void Clear (void);
    BOOL Enter(void) { return m_mutx.Enter(); }
    void Leave(void) { m_mutx.Leave(); }
};

