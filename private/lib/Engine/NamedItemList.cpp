//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"

NamedItem::NamedItem(void)
: pnidNext(0)
, pnidPrev(0)
, pnidNextHash(0)
, pnidPrevHash(0)
, bstrItemName(NULL)
, ulHash(0)
, lwCookie(0)
, dwFlags(0)
, moduleID(0)
, fRegistered(FALSE)
, fHasCode(FALSE)
, pdisp(NULL)
{
}

NamedItem::~NamedItem(void)
{
    SYSFREE(bstrItemName);
    RELEASEPTR(pdisp);
}

HRESULT NamedItem::Clone(NamedItem ** ppnid)
{
    CHECK_POINTER(ppnid);
    Assert(NULL != bstrItemName);
    *ppnid = NULL;

    NamedItem * pnid = HeapNewNoThrow(NamedItem);
    IFNULLMEMRET(pnid);

    pnid->dwFlags       = dwFlags;
    pnid->fHasCode      = fHasCode;
    pnid->moduleID           = moduleID;
    pnid->bstrItemName  = SysAllocString(bstrItemName);
    if (NULL == pnid->bstrItemName)
    {
        delete pnid;
        return E_OUTOFMEMORY;
    }
    *ppnid = pnid;
    return NOERROR;
}

NamedItemList::NamedItemList()
{
    Clear();
}

void NamedItemList::Clear(void)
{
    if (!Enter())
        return;
    m_pnidFirst = NULL;
    m_pnidLast = NULL;
    for (int i = 0 ; i < knilTableSize ; ++i)
        m_pnidTable[i] = NULL;
    Leave();
}

NamedItemList::~NamedItemList()
{
    DestroyAll();
}

void NamedItemList::DestroyAll(void)
{
    NamedItem * pnid;
    NamedItem * pnidNext;

    if (!Enter())
        return;

    for (pnid = m_pnidFirst; NULL != pnid ;)
    {
        pnidNext = pnid->pnidNext;
        delete pnid;
        pnid = pnidNext;
    }
    Clear();
    Leave();
}

void NamedItemList::AddFirst(NamedItem * pnid)
{
    if (!Enter())
        return;
    if (NULL == m_pnidFirst)
    {
        m_pnidFirst = pnid;
        m_pnidLast = pnid;
        pnid->pnidNext = NULL;
        pnid->pnidPrev = NULL;
    }
    else
    {
        pnid->pnidPrev = NULL;
        pnid->pnidNext = m_pnidFirst;
        m_pnidFirst->pnidPrev = pnid;
        m_pnidFirst = pnid;
    }

    pnid->ulHash = CaseInsensitiveComputeHash(pnid->bstrItemName);
    pnid->pnidNextHash = m_pnidTable[pnid->ulHash % knilTableSize];
    pnid->pnidPrevHash = NULL;
    if (NULL != m_pnidTable[pnid->ulHash % knilTableSize])
        m_pnidTable[pnid->ulHash % knilTableSize]->pnidPrevHash = pnid;
    m_pnidTable[pnid->ulHash % knilTableSize] = pnid;
    Leave();
}

void NamedItemList::AddLast(NamedItem * pnid)
{
    if (!Enter())
        return;
    if (NULL == m_pnidLast)
    {
        m_pnidFirst = pnid;
        m_pnidLast = pnid;
        pnid->pnidNext = NULL;
        pnid->pnidPrev = NULL;
    }
    else
    {
        pnid->pnidNext = NULL;
        pnid->pnidPrev = m_pnidLast;
        m_pnidLast->pnidNext = pnid;
        m_pnidLast = pnid;
    }

    pnid->ulHash = CaseInsensitiveComputeHash(pnid->bstrItemName);
    pnid->pnidNextHash = m_pnidTable[pnid->ulHash % knilTableSize];
    pnid->pnidPrevHash = NULL;
    if (NULL != m_pnidTable[pnid->ulHash % knilTableSize])
        m_pnidTable[pnid->ulHash % knilTableSize]->pnidPrevHash = pnid;
    m_pnidTable[pnid->ulHash % knilTableSize] = pnid;
    Leave();
}

void NamedItemList::Remove(NamedItem * pnid)
{
    if (!Enter())
        return;
    if (NULL != pnid->pnidPrev)
        pnid->pnidPrev->pnidNext = pnid->pnidNext;
    if (NULL != pnid->pnidNext)
        pnid->pnidNext->pnidPrev = pnid->pnidPrev;
    if (pnid == m_pnidFirst)
        m_pnidFirst = pnid->pnidNext;
    if (pnid == m_pnidLast)
        m_pnidLast = pnid->pnidPrev;
    if (pnid == m_pnidTable[pnid->ulHash % knilTableSize])
        m_pnidTable[pnid->ulHash % knilTableSize] = pnid->pnidNextHash;
    if (NULL != pnid->pnidPrevHash)
        pnid->pnidPrevHash->pnidNextHash = pnid->pnidNextHash;
    if (NULL != pnid->pnidNextHash)
        pnid->pnidNextHash->pnidPrevHash = pnid->pnidPrevHash;
    Leave();
}

NamedItem * NamedItemList::Find(LPCOLESTR psz)
{
    if (!Enter())
        return NULL;

    NamedItem * pnid;
    for (pnid = m_pnidTable[CaseInsensitiveComputeHash(psz) % knilTableSize];
        NULL != pnid;
        pnid = pnid->pnidNextHash
        )
    {
        if (0 == ostrcmp(psz, pnid->bstrItemName))
        {
            break;
        }
    }
    Leave();
    return pnid;
}

BOOL NamedItemList::FEmpty(void)
{
    BOOL f;

    if (!Enter())
        return TRUE;
    f = (m_pnidFirst == NULL);
    Leave();

    return f;
}

void NamedItemList::Reset(void)
{
    NamedItem * pnid;
    NamedItem * pnidTemp;

    if (!Enter())
        return;
    for (pnid = m_pnidFirst; pnid != NULL;)
    {
        if (pnid->dwFlags & SCRIPTITEM_ISPERSISTENT)
        {
            pnid->fRegistered = FALSE;
            pnid->lwCookie = 0;
            if (NULL != pnid->pdisp)
            {
                pnid->pdisp->Release();
                pnid->pdisp = NULL;
            }
            pnid = pnid->pnidNext;
            continue;
        }
        else
        {
            pnidTemp = pnid;
            pnid = pnid->pnidNext;
            Remove(pnidTemp);
            delete pnidTemp;
        }
    }
    Leave();
}

NamedItem * NamedItemList::First(void)
{
    NamedItem * pnid = NULL;
    if (Enter())
    {
        pnid = m_pnidFirst;
        Leave();
    }
    return pnid;
}

Js::ModuleID NamedItemList::FindHighestModuleID(void)
{
    Js::ModuleID moduleID = 0;
    NamedItem * pnid;

    if (Enter())
    {
        for (pnid = m_pnidFirst ; pnid != NULL; pnid = pnid->pnidNext)
        {
            if (pnid->moduleID > moduleID)
                moduleID = pnid->moduleID;
        }
        Leave();
    }
    return moduleID;
}

HRESULT NamedItemList::Clone(NamedItemList * pnil)
{
    NamedItem * pnid;
    NamedItem * pnidNew;
    HRESULT hr = NOERROR;

    if (!Enter())
        return E_FAIL;
    for (pnid = m_pnidFirst ; NULL != pnid ; pnid = pnid->pnidNext)
    {
        if (!(pnid->dwFlags & SCRIPTITEM_ISPERSISTENT))
            continue;
        IFFAILGO(pnid->Clone(&pnidNew));
        pnil->AddLast(pnidNew);
    }
LReturn:
    Leave();
    return hr;
}



