//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

// Description: declarations for the main class factory
#pragma once

// If this were a completely general class, we'd provide a list of CATIDs to
// register. But we'll do the easier flag set for the scripting categories.
enum
{
    inCatActiveScript       = 0x00001,
    inCatActiveScriptParse  = 0x00002,
    inCatActiveScriptAuthor = 0x00004,
    inCatActiveScriptEncode = 0x00008,
};

class CClassFactory : public IClassFactory
{
    long    m_refCount;

public:
    CClassFactory(
        REFCLSID        guidClassID,
        const WCHAR *    pszClassID,
        const WCHAR *    pszDescription,
        const WCHAR *    pszExtension,
        const WCHAR *    pszDllName,
        const WCHAR **   prgpszNames,
        int             cpszNames,
        DWORD           grfCategories
    )
        : m_refCount(1)
        , m_pszClassID(pszClassID)
        , m_pszDescription(pszDescription)
        , m_pszExtension(pszExtension)
        , m_pszDllName(pszDllName)
        , m_prgpszNames(prgpszNames)
        , m_cpszNames(cpszNames)
        , m_guidClassID(guidClassID)
        , m_grfCategories(grfCategories)
    {
        // Take a global lock to syncrontize with DllCanUnloadNow
        AutoCriticalSection autocs(ThreadContext::GetCriticalSection());
        DLLAddRef();
    }

    ~CClassFactory()
    {
        DLLRelease();
    }
    virtual REFIID GetTypeId() = 0;

    STDMETHODIMP QueryInterface (REFIID, void**) sealed;
    STDMETHODIMP_(ULONG) AddRef (void);
    STDMETHODIMP_(ULONG) Release(void) sealed;
    STDMETHODIMP LockServer (BOOL);  //TBD to remove LockServer method. keep it now to avoid an abstract class

    // override these methods in subclasses
    STDMETHODIMP CreateInstance (IUnknown*, REFIID, void **) = 0;
    STDMETHODIMP CClassFactory::RegisterServer(LPCWSTR threadModel);
    STDMETHODIMP CClassFactory::UnregisterServer();

    const WCHAR*     m_pszClassID;
    const WCHAR*     m_pszDescription;
    const WCHAR*     m_pszExtension;
    const WCHAR*     m_pszDllName;
    const WCHAR **   m_prgpszNames;
    int             m_cpszNames;
    CLSID           m_guidClassID;
    DWORD           m_grfCategories;
};


