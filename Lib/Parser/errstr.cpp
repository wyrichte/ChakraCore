//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

// scafolding - get a g_hInstance from scrbgase.cpp
HANDLE g_hInstance;

//#ifdef SCRIPT_MUI


#include "muiload.h"

HINSTANCE LoadMUI(HINSTANCE hMain, LCID lcid)
{
    HINSTANCE hRes = NULL;
    WCHAR szModuleName[MAX_PATH];
    if (GetModuleFileNameW(hMain, szModuleName, MAX_PATH))
    {
        // load by language name
        hRes = LoadMUILibraryW(szModuleName, MUI_LANGUAGE_NAME, LANGIDFROMLCID(lcid));
    }
    if (NULL == hRes)
        hRes = hMain;
    return hRes;
}


//#endif // SCRIPT_MUI

// Used as a prefix to generate the resource dll name.
const wchar_t g_wszPrefix[] = L"js";


MUTX NEAR g_mutxCache;
GL *g_pgllmap;

static BOOL   g_fWinVista = FALSE ;
static BOOL   g_fVistaChecked = FALSE ;

static HANDLE HlibLoadDll(LCID lcid, BOOL * pfUnload);

static BOOL FGetHlibForLcid(LCID lcid, HANDLE *phlib, long *pilmap)
{
    AssertMem(phlib);
    AssertMem(pilmap);
    long ivMin, ivLim, iv;
    LMAP lmap;

    *phlib = NULL;
    *pilmap = 0;

    if (NULL == g_pgllmap)
        return FALSE;

    for (ivMin = 0, ivLim = g_pgllmap->Cv(); ivMin < ivLim; )
    {
        iv = (ivMin + ivLim) / 2;
        g_pgllmap->Get(iv, &lmap);
        if (lmap.lcid < lcid)
            ivMin = iv + 1;
        else if (lmap.lcid > lcid)
            ivLim = iv;
        else
        {
            *pilmap = iv;
            *phlib = lmap.hlib;
            return TRUE;
        }
    }

    *pilmap = ivMin;
    return FALSE;
}


static BOOL FEnsureHlibForLcid(LCID lcid, HANDLE *phlib)
{
    AssertMem(phlib);
    long ilmap;

    if (!g_mutxCache.Enter())
        return false;

    if (!g_fVistaChecked)
    {
        OSVERSIONINFO vi;
        vi.dwOSVersionInfoSize = sizeof(vi);
        if (GetVersionEx(&vi))
            g_fWinVista = vi.dwMajorVersion >= 6;

        g_fVistaChecked = TRUE ;
    }

    if (g_fWinVista)
    {
        *phlib = g_hInstance ;
        g_mutxCache.Leave( ) ;
        return TRUE ;
    }

    if (!FGetHlibForLcid(lcid, phlib, &ilmap) &&
        (NULL != g_pgllmap || NULL != (g_pgllmap = HeapNewNoThrow(GL,sizeof(LMAP)))) &&
        g_pgllmap->FEnsureSpace(1))
    {
        LMAP lmap;

        lmap.lcid = lcid;
        //#ifdef SCRIPT_MUI
        lmap.hlib = LoadMUI((HINSTANCE)g_hInstance, lcid);
        if (NULL != lmap.hlib)
        {
            lmap.fMui = TRUE;
            lmap.fUnload = TRUE;
        }
        else 
        {
            lmap.fMui = FALSE;
            //#endif // SCRIPT_MUI
            lmap.hlib = HlibLoadDll(lcid, &lmap.fUnload);
            if (NULL == lmap.hlib)
            {
                LCID lcidTry;
                WORD langID = LANGIDFROMLCID(lcid);

                // try these sublanguages
                const WORD rgsub[] =
                {
                    SUBLANG_NEUTRAL,
                    SUBLANG_DEFAULT,
                    SUBLANG_SYS_DEFAULT,
                };
                const WORD *psub = rgsub + sizeof(rgsub) / sizeof(rgsub[0]);

                while (psub-- > rgsub && NULL == lmap.hlib)
                {
                    if (SUBLANGID(langID) == *psub)
                        continue;
                    lcidTry = MAKELANGID(PRIMARYLANGID(langID), *psub);
                    lmap.hlib = HlibLoadDll(lcidTry, &lmap.fUnload);
                }
            }
            //#ifdef SCRIPT_MUI
        }
        //#endif // SCRIPT_MUI

        *phlib = lmap.hlib;

        // we already guaranteed the space above
        AssertVerify(g_pgllmap->FInsert(ilmap, &lmap));
    }

    g_mutxCache.Leave();

    return NULL != *phlib;
}

static BOOL FGetStringFromLibrary(HMODULE hlib, int istring, __out_ecount(cchMax) WCHAR * psz, int cchMax)
{
    // NOTE - istring is expected to be HRESULT

    Assert(0 < cchMax);
    AssertArrMem(psz, cchMax);

    HGLOBAL hgl = NULL;
    WCHAR * pchRes = NULL;
    HRSRC hrsrc;
    WCHAR * pchCur;
    int cch;
    int cstring;
    DWORD cbRes;
    // take SCODE_CODE(hr == istring), DIV 16, + 1
    int itable = ((WORD)istring >> 4) + 1;
    istring &= 0x0F;
    BOOL fRet = FALSE;

    psz[0] = '\0';

    if (NULL == hlib)
        goto LError;

    hrsrc = FindResourceEx((HMODULE)hlib, RT_STRING, MAKEINTRESOURCE(itable), 0);
    if (NULL == hrsrc)
        goto LError;

    hgl = LoadResource((HMODULE)hlib, hrsrc);
    if (NULL == hgl)
        goto LError;

    pchRes = (WCHAR *)LockResource(hgl);
    if (NULL == pchRes)
        goto LError;

    cbRes = SizeofResource((HMODULE)hlib, hrsrc);

    if (cbRes < sizeof(WORD))
        goto LError;

    pchCur = pchRes;
    for (cstring = istring; cstring-- > 0;)
    {
        if (cbRes - sizeof(WORD) < sizeof(WCHAR) * (pchCur - pchRes))
            goto LError;

        cch = (*(WORD *) pchCur) + 1;
        // Note: Casting is required to avoid endian issues on Unix platforms.

        if (cch <= 0)
            goto LError;

        if (cbRes < sizeof(WCHAR) * cch)
            goto LError;

        if (cbRes - sizeof(WCHAR) * cch < sizeof(WCHAR) * (pchCur - pchRes))
            goto LError;

        pchCur += cch;
    }

    if (cbRes - sizeof(WORD) < sizeof(WCHAR) * (pchCur - pchRes))
        goto LError;
    cch = * (WORD *) pchCur;

    if (cch <= 0)
        goto LError;

    if (cbRes < sizeof(WCHAR) * (cch + 1))
        goto LError;

    if (cbRes - sizeof(WCHAR) * (cch + 1) < sizeof(WCHAR) * (pchCur - pchRes))
        goto LError;

    if (cch > cchMax - 1)
        cch = cchMax - 1;

    js_memcpy_s(psz, cchMax * sizeof(WCHAR), pchCur + 1, cch * sizeof(WCHAR));
    psz[cch] = '\0';
    fRet = TRUE;

LError:

#if !_WIN32 && !_WIN64

    //
    // Unlock/FreeResource non-essential on win32/64.
    //
    if (NULL != hgl)
    {
        if (NULL != pchRes)
            UnlockResource(hgl);
        FreeResource(hgl);
    }

#endif

    return fRet;
}


BOOL FGetResourceString(long isz, __out_ecount(cchMax) OLECHAR *psz, int cchMax, LCID lcid,
                        BOOL fResourceDllOnly)
{
    // NOTE - isz is expected to be HRESULT

    HANDLE hlib;

    if (!FEnsureHlibForLcid(lcid, &hlib))
        hlib = NULL;

    if (FGetStringFromLibrary((HINSTANCE)hlib, isz, psz, cchMax))
        return TRUE;

    if (fResourceDllOnly)
        return FALSE;

    return FGetStringFromLibrary((HINSTANCE)g_hInstance, isz, psz, cchMax);
}

// Get a bstr version of the error string
__declspec(noinline) // Don't inline. This function needs 2KB stack.
BSTR BstrGetResourceString(long isz, LCID lcid)
{
    // NOTE - isz is expected to be HRESULT

    OLECHAR szT[1024];

    if (!FGetResourceString(isz, szT,
        sizeof(szT) / sizeof(szT[0]) - 1, lcid))
    {
        return NULL;
    }

    return SysAllocString(szT);
}

#undef LoadStr



#if (_WIN32 || _WIN64) && CHECKRSRCLANG
// These next couple of functions are used to determine if the users
// primary language matches that in the resources built into the
// main dll. If it does, then we don't have to load any language
// specific resource dll's. This improves load performance by
// allowing us to not have to attempt possibly two LoadLibrary calls.
struct LANGUAGEENUMDATA
{
    BOOL fFound;
    WORD wLangID;
};


static BOOL CALLBACK LanguageEnumProc(HANDLE hModule, LPCTSTR lpType, LPTSTR lpName,
                                      WORD wLangID, LONG lParam)
{
    LANGUAGEENUMDATA *pled = (LANGUAGEENUMDATA *)lParam;
    if (pled->wLangID == (WORD)PRIMARYLANGID(wLangID))
    {
        pled->fFound = TRUE;
        return FALSE;
    }
    return TRUE;
}


static BOOL FPrimaryLangExists(WORD wLangID)
{
    LANGUAGEENUMDATA led;

    led.fFound = FALSE;
    led.wLangID = wLangID;
    EnumResourceLanguages(g_hInstance, RT_VERSION, MAKEINTRESOURCE(1),
        (FARPROC)LanguageEnumProc, (LPARAM)&led);
    return led.fFound;
}

#else // (_WIN32 || _WIN64) && CHECKRSRCLANG

static BOOL FPrimaryLangExists(WORD wLangID)
{
    return wLangID == LANG_ENGLISH;
}

#endif // (_WIN32 || _WIN64) && CHECKRSRCLANG


// This code originated from the functions LoadIntlDll and EbGetIntlDllFileName
// from the file misc\host.cpp. We want the satellite Dll's to have the
// same naming conventions.
static BOOL FGenerateDllName(__out_ecount(cchDllName) LPWSTR pwszDllName, UINT cchDllName, LCID lcid)
{
    wchar_t wszAbbrevLangName[6]; // abbreviated language name according to ISO 639

    if (GetLocaleInfoW(lcid, LOCALE_SABBREVLANGNAME | LOCALE_NOUSEROVERRIDE,
        wszAbbrevLangName, 4) == 0)
    {
        return FALSE;
    }

    // for Chinese-Taiwan, and Portuguese-Brazil we use 2 letter although
    // they are the default country/region
    // get the abbreviated language name through the host-langid
    WORD langID = LANGIDFROMLCID(lcid);
    WORD subLangID = (WORD)SUBLANGID(langID);
    WORD primLangID = (WORD)PRIMARYLANGID(langID);
    int cchIntl = 3;

    if (subLangID == SUBLANG_NEUTRAL ||
        subLangID == SUBLANG_DEFAULT && primLangID != LANG_CHINESE &&
        primLangID != LANG_PORTUGUESE)
    {
        cchIntl = 2;
    }
    wszAbbrevLangName[cchIntl] = '\0';

    wcscpy_s(pwszDllName, cchDllName, g_wszPrefix);
    wcscat_s(pwszDllName, cchDllName, wszAbbrevLangName);
#if _WIN32 || _WIN64
    wcscat_s(pwszDllName, cchDllName, L".dll");
#else
#error Neither _WIN32, nor _WIN64 is defined
#endif

    return TRUE;
}


static HANDLE HlibLoadDll(LCID lcid, BOOL * pfUnload)
{
    AssertMem(pfUnload);

#if !MAC
    wchar_t wszName[MAX_PATH + 1];
    wchar_t wszPath[MAX_PATH + 1];
    int cchName;
    int cchPath;
    PWSTR pwszT;
    HANDLE hlib = NULL;

    *pfUnload = TRUE;

    // We only need to load the resource dll if the language of the resources
    // built into this dll don't match those needed by the user.
    if (FPrimaryLangExists((WORD)PRIMARYLANGID(LANGIDFROMLCID(lcid))))
    {
        *pfUnload = FALSE;
        return g_hInstance;
    }

    // Get the name of the language Dll.
    if (!FGenerateDllName(wszName, ARRAYSIZE(wszName), lcid))
        return NULL;
    cchName = (int)wcslen(wszName);

    // get the main dll's path
    cchPath = GetModuleFileNameW((HMODULE)g_hInstance, wszPath, _countof(wszPath) - 1);
    wszPath[cchPath] = L'\0';
    if (cchPath != 0)
    {
        pwszT = wcsrchr(wszPath, L'\\');
        if ((NULL != pwszT) && (wszPath + _countof(wszPath) - pwszT - 2 >= cchName))
        {
            // Replace the main dll's file name with the new dll name and load it.
            wcscpy_s(pwszT + 1, ARRAYSIZE(wszPath)-(pwszT+1-wszPath), wszName);
            hlib = LoadLibraryExW(wszPath, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE);
        }
    }

    // Couldn't load the the dll from the path of the main dll.
    // Just search the standard path for the dll.
    if (NULL == hlib)
    {
        hlib = LoadLibraryExW(wszName, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE);
    }
    return hlib;
#else   // !MAC
#if !SOURCERELEASE
    // UNDONE MAC: [EricLi] 30 Dec 1997
    // UNDONE MAC: How do you load a DLL on a Mac?
#endif // !SOURCERELEASE
    return NULL;
#endif // !MAC
}

