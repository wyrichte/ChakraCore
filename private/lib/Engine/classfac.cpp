//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Description: defs for the main class factory

#include "EnginePch.h"

#define szSERVERKEY         L"InprocServer32"
#define szOTHERSERVERKEY    L"InprocServer"

// Helper function to create a component category and associated description
static HRESULT CreateComponentCategory(CATID catid, LPCOLESTR catDescription)
{
    HRESULT         hr  = S_OK;
    ICatRegister *  pcr = NULL;

    IFFAILRET(CoCreateInstance(CLSID_StdComponentCategoriesMgr,
        NULL, CLSCTX_INPROC_SERVER, __uuidof(ICatRegister), (void**)&pcr));

    // Make sure the HKCR\Component Categories\{..catid...}
    // key is registered
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid  = 0x0409 ; // english

    // Make sure the provided description is not too long.
    // Only copy the first 127 characters if it is
    size_t len = ostrlen(catDescription);
    if (len > 127)
        len = 127;
    wcsncpy_s(catinfo.szDescription, ARRAYSIZE(catinfo.szDescription), catDescription, len);
    // Make sure the description is null terminated
    catinfo.szDescription[len] = '\0';

    hr = pcr->RegisterCategories(1, &catinfo);
    pcr->Release();

    return hr;
}

// Helper function to register a CLSID as belonging to a component category
static HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
// Register your component categories information.
    HRESULT         hr = S_OK ;
    ICatRegister *  pcr = NULL ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
            NULL, CLSCTX_INPROC_SERVER, __uuidof(ICatRegister), (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Register this category as being "implemented" by
       // the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }
    RELEASEPTR(pcr);
    return hr;
}

//int StringFromGuidW(REFIID riid, __out_ecount(cchBuf) LPSTR pszBuf, UINT cchBuf);

static BOOL FExistsCLSID(REFCLSID clsid)
{
    LONG lRes;
    HKEY hkeyCLSID;
    HKEY hkey;
    WCHAR buf[MAX_PROGID_LENGTH];

        StringFromGUID2(clsid, buf, ARRAYSIZE(buf));
        lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"CLSID", 0, KEY_READ, &hkeyCLSID);
        if (0 != lRes)
                return FALSE;

        lRes = RegOpenKeyExW(hkeyCLSID, buf, 0, KEY_READ, &hkey);
        if (0 != lRes)
        {
                RegCloseKey(hkeyCLSID);
                return FALSE;
        }

    RegCloseKey(hkey);
    RegCloseKey(hkeyCLSID);
    return TRUE;
}

// Helper function to unregister a CLSID as belonging to a component category
static HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
    ICatRegister*   pcr = NULL ;
    HRESULT         hr  = S_OK ;

    // There is a bug in UnRegisterClassImplCategories which causes
    // an Access Violation if the CLSID is not in the registry.
    // (The error number returned in this case is incorrect, which
    // doesn't make the situation any better.)  Therefore,
    // we check to see if the key exists before deleting it.

    if (!FExistsCLSID(clsid))
        return NOERROR;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
            NULL, CLSCTX_INPROC_SERVER, __uuidof(ICatRegister), (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Unregister this category as being "implemented" by
       // the class.
       CATID rgcatid[1];
       rgcatid[0] = catid;
       hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }

    RELEASEPTR(pcr);

    return hr;
}

//=--------------------------------------------------------------------------=
// StringFromGuidW
//=--------------------------------------------------------------------------=
// Forms an ANSI string from a CLSID or GUID
//
// Parameters:
//    REFIID    [in]  clsid to make string out of.
//    LPSTR     [in]  buffer in which to place resultant GUID.
//    cch       [in]  size of pszBuf.
//
// Output:
//    int       number of chars written out.
//
// Commented out for now as we use StringFromGUID2. Which seems fine.  
//int StringFromGuidW (REFIID riid, __out_ecount(cch) LPWSTR psz, UINT cch)
//{
//    return sprintf_s((char *)psz, cch, 
//        "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", 
//        riid.Data1, riid.Data2, riid.Data3, 
//        riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], 
//        riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);
//}

// ****************************** CClassFactory *************************

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppvObj)
{
    AssertMem(ppvObj);
    CHECK_POINTER(ppvObj);

    if (IID_IUnknown == riid)
        *ppvObj = (IUnknown *) this;
    else if (__uuidof(IClassFactory) == riid)
        *ppvObj = (IClassFactory *) this;
    else
    {
        *ppvObj = NULL;
        return ResultFromScode (E_NOINTERFACE) ;
    }
    ((IUnknown*)*ppvObj)->AddRef();
    return NOERROR;
}


STDMETHODIMP_(ULONG) CClassFactory::AddRef(void)
{
    return (ULONG)::InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) CClassFactory::Release(void)
{
    long l = ::InterlockedDecrement(&m_refCount);
    if (0 == l)
    {
        delete this;
        return 0;
    }
    return (ULONG)l;
}



STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        DLLAddRef();
    }
    else
    {
        DLLRelease();
    }
    return NOERROR;
}

#define WSZCAT_SCRIPT       L"Active Scripting Engine"
#define WSZCAT_SCRIPTPARSE  L"Active Scripting Engine with Parsing"

STDMETHODIMP CClassFactory::RegisterServer(LPCWSTR threadModel)
{
    HKEY hk;
    HKEY hkSub;
    HKEY hkSub2;
    int ipsz;
    ulong cbDllPath;

    DWORD cszDescription = (DWORD)wcslen(m_pszDescription);
    DWORD cszClassID     = (DWORD)wcslen(m_pszClassID);

    WCHAR szDllPath[512];
    cbDllPath = GetModuleFileName(GetModuleHandle(m_pszDllName), szDllPath, ARRAYSIZE(szDllPath));
    if (0 == cbDllPath)
    {
        return E_FAIL;
    }

    // Make a clean start
    UnregisterServer();

    // Register server info.
    for (ipsz = 0; ipsz < m_cpszNames; ipsz++)
    {
        if (NOERROR != RegCreateKeyExW(HKEY_CLASSES_ROOT, m_prgpszNames[ipsz], 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL))
        {
            return E_FAIL;
        }
        if (NOERROR != RegCreateKeyExW(hk, L"OLEScript", 0, NULL, 0, KEY_WRITE, NULL, &hkSub, NULL))
        {
            RegCloseKey(hk);
            return E_FAIL;
        }
        RegCloseKey(hkSub);
        RegSetValueExW(HKEY_CLASSES_ROOT, m_prgpszNames[ipsz], 0, REG_SZ, 
                       (const BYTE *)m_pszDescription, cszDescription*sizeof(wchar_t));

        if (NOERROR != RegCreateKeyExW(hk, L"CLSID", 0, NULL, 0, KEY_WRITE, NULL, &hkSub, NULL))
        {
            RegCloseKey(hk);
            return E_FAIL;
        }

        RegSetValueExW(hkSub, NULL, 0, REG_SZ, (const BYTE *)m_pszClassID, cszClassID*sizeof(wchar_t));
        RegCloseKey(hkSub);

        RegCloseKey(hk);
    }

    // Register createable class
    if (NOERROR != RegCreateKeyExW(HKEY_CLASSES_ROOT, L"CLSID", 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL))
    {
        return E_FAIL;
    }
    if (NOERROR != RegCreateKeyExW(hk, m_pszClassID, 0, NULL, 0, KEY_WRITE, NULL, &hkSub, NULL))
    {
        RegCloseKey(hk);
        return E_FAIL;
    }
    if (NOERROR != RegCreateKeyExW(hkSub, L"OLEScript", 0, NULL, 0, KEY_WRITE, NULL, &hkSub2, NULL))
    {
        RegCloseKey(hk);
        RegCloseKey(hkSub);
        return E_FAIL;
    }

    RegSetValueExW(hk, m_pszClassID, 0, REG_SZ, (const BYTE *)m_pszDescription, cszDescription*sizeof(wchar_t));

    if (0 != m_cpszNames)
    {
        RegSetValueExW(hkSub, L"ProgID", 0, REG_SZ, (const BYTE *)m_prgpszNames[0], (DWORD)wcslen(m_prgpszNames[0])*sizeof(wchar_t));
    }

    HKEY hkthread;

    // To set the threading model reg entry I had to use the RegOpenKeyEx &
    // SetValueEx. Otherwise (using the Win3.1 compatable versions) you get
    // new keys.
    if (NOERROR != RegCreateKeyExW(hkSub, szSERVERKEY, 0, NULL, 0, KEY_WRITE, NULL, &hkthread, NULL))
    {
        RegCloseKey(hkSub2);
        RegCloseKey(hkSub);
        RegCloseKey(hk);
        return E_FAIL;
    }
    RegSetValueExW(hkthread, NULL, 0, REG_SZ, (const BYTE *)szDllPath, cbDllPath*sizeof(wchar_t));

    RegSetValueEx(hkthread,L"ThreadingModel", 0, REG_SZ, 
        (BYTE *)threadModel, 
        (DWORD)wcslen(threadModel)*sizeof(wchar_t)
        );

    RegCloseKey(hkthread);

    RegCloseKey(hkSub2);

    RegCloseKey(hkSub);
    RegCloseKey(hk);

    // Register the extension
    if ((NULL != m_pszExtension) && (0 != m_cpszNames))
    {
            // REVIEW: list of extensions?
            if (NOERROR != RegSetValueExW(HKEY_CLASSES_ROOT, m_pszExtension,
                            0, REG_SZ, (const BYTE *)m_prgpszNames[0], (DWORD)wcslen(m_prgpszNames[0])*sizeof(wchar_t)))
                    return E_FAIL;
    }

    // Create scripting categories
    if (m_grfCategories & inCatActiveScript)
    {
        CreateComponentCategory(CATID_ActiveScript, WSZCAT_SCRIPT);
        RegisterCLSIDInCategory(m_guidClassID, CATID_ActiveScript);
    }
    if (m_grfCategories & inCatActiveScriptParse)
    {
        CreateComponentCategory(CATID_ActiveScriptParse, WSZCAT_SCRIPTPARSE);
        RegisterCLSIDInCategory(m_guidClassID, CATID_ActiveScriptParse);
    }
    return NOERROR;
}


STDMETHODIMP CClassFactory::UnregisterServer()
{
    HRESULT hr = NOERROR;
    int ipsz;
    HKEY hk, hkSub;

#if _M_IX86 || _M_AMD64 || _M_IA64 || _M_ARM
    // If we're on x86, then if the other platform (win32 if we're win16
    // and win16 if we're win32) is registered, we only need to delete
    // the server key.
    if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CLASSES_ROOT, L"CLSID", 0, KEY_WRITE, &hk))
    {
        if (ERROR_SUCCESS == RegOpenKeyExW(hk, m_pszClassID, 0, KEY_WRITE, &hkSub))
        {
            RegCloseKey(hk);
            hk = hkSub;
            
            if (ERROR_SUCCESS == RegOpenKeyExW(hk, szOTHERSERVERKEY, 0, KEY_WRITE, &hkSub))
            {
                RegCloseKey(hkSub);
                if (ERROR_SUCCESS != RegDeleteKeyEx(hk, szSERVERKEY, 0, 0))
                {
                    hr = E_FAIL;
                }
                if (m_grfCategories & inCatActiveScript)
                {
                    UnRegisterCLSIDInCategory(m_guidClassID, CATID_ActiveScript);
                }
                if (m_grfCategories & inCatActiveScriptParse)
                {
                    UnRegisterCLSIDInCategory(m_guidClassID, CATID_ActiveScriptParse);
                }
                RegDeleteKeyEx(hk, L"Implemented Categories", 0, 0);
                RegCloseKey(hk);
                return hr;
            }
        }
        RegCloseKey(hk);
    }
#endif // _M_IX86 || _M_AMD64 || _M_IA64 || _M_ARM

    // UnRegister ourselves in the category:
    if (m_grfCategories & inCatActiveScript)
    {
        UnRegisterCLSIDInCategory(m_guidClassID, CATID_ActiveScript);
    }
    if (m_grfCategories & inCatActiveScriptParse)
    {
        UnRegisterCLSIDInCategory(m_guidClassID, CATID_ActiveScriptParse);
    }

    for (ipsz = 0; ipsz < m_cpszNames; ipsz++)
    {
        if (NOERROR != RegOpenKeyExW(HKEY_CLASSES_ROOT, m_prgpszNames[ipsz], 0, KEY_WRITE, &hk))
        {
                hr = E_FAIL;
        }
        else
        {
            if (NOERROR != RegDeleteKeyEx(hk, L"CLSID", 0, 0))
            {
                hr = E_FAIL;
            }
            if (NOERROR != RegDeleteKeyEx(hk, L"OLEScript", 0, 0))
            {
                hr = E_FAIL;
            }
            RegCloseKey(hk);
            if (NOERROR != RegDeleteKeyEx(HKEY_CLASSES_ROOT, m_prgpszNames[ipsz], 0, 0))
            {
                hr = E_FAIL;
            }
        }
    }

    // delete classes_root\clsid\{our classid}\*
    if (NOERROR != RegCreateKeyExW(HKEY_CLASSES_ROOT, L"CLSID", 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL))
    {
        hr = E_FAIL;
    }
    else
    {
        if (NOERROR != RegOpenKeyExW(hk, m_pszClassID, 0, KEY_WRITE, &hkSub))
        {
            hr = E_FAIL;
        }
        else
        {
            // delete classes_root\clsid\{our classid}\progid
            if ((0 != m_cpszNames) && (NOERROR != RegDeleteKeyEx(hkSub, L"ProgID", 0, 0)))
            {
                hr = E_FAIL;
            }
            // delete classes_root\clsid\{our classid}\OLEScript
            if (NOERROR != RegDeleteKeyEx(hkSub, L"OLEScript", 0, 0))
            {
                hr = E_FAIL;
            }
            // delete classes_root\clsid\{our classid}\Implemented Categories
            if (NOERROR != RegDeleteKeyEx(hkSub, L"Implemented Categories", 0, 0))
            {
                hr = E_FAIL;
            }
            // delete classes_root\clsid\{our classid}
            if (NOERROR != RegDeleteKeyEx(hkSub, szSERVERKEY, 0, 0))
            {
                hr = E_FAIL;
            }
            RegCloseKey(hkSub);
        }

        // delete classes_root\clsid\{our classid}\*
        if (NOERROR != RegDeleteKeyEx(hk, m_pszClassID, 0, 0))
        {
            hr = E_FAIL;
        }
        RegCloseKey(hk);
    }

    if ((NULL != m_pszExtension) && (0 != m_cpszNames))
    {
        // delete extension
        if (NOERROR != RegDeleteKeyEx(HKEY_CLASSES_ROOT, m_pszExtension, 0, 0))
        {
            hr = E_FAIL;
        }
    }

    return hr;
}
