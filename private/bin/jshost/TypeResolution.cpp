//+----------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Purpose: Enable metadata type resolution for Windows and third-party types.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

#define METADATA_FILE_EXTENSION _u(".winmd")

static const UINT32 MAX_TYPE_NAME = 512;

HRESULT ActiveScriptDirectHost::ValidateNameFormat(__in const HSTRING hstrFullName)
{
    HRESULT hr = S_OK;
    PCWSTR pszFullName = nullptr;
    UINT32 cFullName = 0;

    if (hstrFullName == nullptr)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        // If the input string is dot-separated, then make sure that the component to the left
        // and the component to the right of the last dot are non-empty.
        if (SUCCEEDED(hr))
        {
            pszFullName = m_WinRTStringLibrary.WindowsGetStringRawBuffer(hstrFullName, &cFullName);

            char16 *pszLastDot = wcsrchr(pszFullName, _u('.'));
            if (pszLastDot != nullptr)
            {
                const size_t cTypeName = wcslen(pszLastDot) - 1;
                const size_t cNamespace = cFullName - cTypeName - 1;
                if ((cNamespace == 0) || (cTypeName == 0) || (cFullName > MAX_TYPE_NAME))
                {
                    hr = RO_E_METADATA_INVALID_TYPE_FORMAT;
                }
            }
        }
    }

    return hr;
}

bool DoesFileExist(__in PCWSTR pszFilePath)
{
    bool fFileExists = true;
    DWORD dwFileAttributes;
    dwFileAttributes = GetFileAttributes(pszFilePath);

    if ((dwFileAttributes == INVALID_FILE_ATTRIBUTES) ||
        (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        fFileExists = false;
    }

    return fFileExists;
}

HRESULT ActiveScriptDirectHost::FindTypeInMetaDataFile(
    __in IMetaDataDispenserEx *pMetaDataDispenser,
    __in PCWSTR pszFullName,
    __in PCWSTR pszCandidateFilePath,
    __in TYPE_RESOLUTION_OPTIONS resolutionOptions,
    __deref_out_opt IMetaDataImport2 **ppMetaDataImport,
    __out_opt mdTypeDef *pmdTypeDef)
{
    HRESULT hr = S_OK;
    IMetaDataImport2 *pMetaDataImport = nullptr;

    // Clear output parameters.
    if (ppMetaDataImport != nullptr)
    {
        *ppMetaDataImport = nullptr;
    }
    if (pmdTypeDef != nullptr)
    {
        *pmdTypeDef = mdTypeDefNil;
    }

    hr = pMetaDataDispenser->OpenScope(
        pszCandidateFilePath,
        (ofRead | ofNoTransform),
        IID_IMetaDataImport2,
        reinterpret_cast<IUnknown **>(&pMetaDataImport));

    if (SUCCEEDED(hr))
    {
        const size_t cFullName = wcslen(pszFullName);
        char16 pszRetrievedName[MAX_TYPE_NAME];
        HCORENUM hEnum = nullptr;
        mdTypeDef rgTypeDefs[32];
        ULONG cTypeDefs;
        bool fEntryFound = false;

        while ((hr == S_OK) && (fEntryFound == false))
        {
            hr = pMetaDataImport->EnumTypeDefs(
                &hEnum,
                rgTypeDefs,
                ARRAYSIZE(rgTypeDefs),
                &cTypeDefs);

            if ((hr == S_FALSE) || FAILED(hr))
            {
                break;
            }

            for (UINT32 iTokenIndex = 0; iTokenIndex < cTypeDefs; ++iTokenIndex)
            {
                hr = pMetaDataImport->GetTypeDefProps(
                    rgTypeDefs[iTokenIndex],
                    pszRetrievedName,
                    ARRAYSIZE(pszRetrievedName),
                    nullptr,
                    nullptr,
                    nullptr
                    );

                if (FAILED(hr))
                {
                    break;
                }

                if ((resolutionOptions & TRO_RESOLVE_TYPE) && (wcscmp(pszRetrievedName, pszFullName) == 0))
                {
                    fEntryFound = true;
                    if (pmdTypeDef != nullptr)
                    {
                        *pmdTypeDef = rgTypeDefs[iTokenIndex];
                    }
                    if (ppMetaDataImport != nullptr)
                    {
                        *ppMetaDataImport = pMetaDataImport;
                    }
                    break;
                }
                else 
                if (resolutionOptions & TRO_RESOLVE_NAMESPACE)
                {
                    // Check whether the name is a namespace rather than a type.
                    const size_t cRetrievedName = wcslen(pszRetrievedName);
                    if (cRetrievedName > cFullName)
                    {
                        const char16 *pch = nullptr;
                        pch = wcsstr(pszRetrievedName, pszFullName);
                        if ((pch != nullptr) &&
                            (pch == pszRetrievedName) &&
                            (pszRetrievedName[cFullName] == _u('.')))
                        {
                            fEntryFound = true;
                            hr = RO_E_METADATA_NAME_IS_NAMESPACE;
                            break;
                        }
                    }
                }
            }
        }

        if (hr == S_FALSE)
        {
            hr = RO_E_METADATA_NAME_NOT_FOUND;
        }

        if (FAILED(hr) || (ppMetaDataImport == nullptr))
        {
            pMetaDataImport->Release();
            pMetaDataImport = nullptr;
        }
    }

    return hr;
}

HRESULT ActiveScriptDirectHost::FindTypeInDirectory(
    __in IMetaDataDispenserEx *pMetaDataDispenser,
    __in PCWSTR pszFullName,
    __in PCWSTR pszDirectoryPath,
    __out_opt HSTRING *phstrMetaDataFilePath,
    __deref_out_opt IMetaDataImport2 **ppMetaDataImport,
    __out_opt mdTypeDef *pmdTypeDef)
{
    HRESULT hr = RO_E_METADATA_NAME_NOT_FOUND;

    char16 pszCandidateFilePath[MAX_PATH + 1] = {0};
    char16 pszCandidateFileName[MAX_PATH + 1] = {0};
    StringCchCopy(pszCandidateFileName, ARRAYSIZE(pszCandidateFileName), pszFullName);

    bool fFileExists = false;
    char16 *pszLastDot;

    // To resolve type Windows.B.C, first check if Windows.B.C is a type or
    // namespace in the first existing WinMD file, in this order:
    // 1. Windows.B.C.WinMD
    // 2. Windows.B.WinMD
    // 3. Windows.WinMD
    do
    {
        pszLastDot = nullptr;

        StringCchCopy(pszCandidateFilePath, ARRAYSIZE(pszCandidateFilePath), pszDirectoryPath);
        StringCchCat(pszCandidateFilePath, ARRAYSIZE(pszCandidateFilePath), pszCandidateFileName);
        StringCchCat(pszCandidateFilePath, ARRAYSIZE(pszCandidateFilePath), METADATA_FILE_EXTENSION);
        fFileExists = DoesFileExist(pszCandidateFilePath);

        if (fFileExists)
        {
            hr = FindTypeInMetaDataFile(
                pMetaDataDispenser,
                pszFullName,
                pszCandidateFilePath,
                TRO_RESOLVE_TYPE_AND_NAMESPACE,
                ppMetaDataImport,
                pmdTypeDef);

            if (SUCCEEDED(hr))
            {
                if (phstrMetaDataFilePath != nullptr)
                {
                    Assert(m_WinRTStringLibrary.IsAvailable());

                    hr = m_WinRTStringLibrary.WindowsCreateString(
                        pszCandidateFilePath,
                        static_cast<UINT32>(wcslen(pszCandidateFilePath)),
                        phstrMetaDataFilePath);
                }
            }

            // Stop looking in the "upward direction" since we hit the first
            // existing WinMD file.
            break;
        }
        else
        {
            hr = RO_E_METADATA_NAME_NOT_FOUND;

            pszLastDot = wcsrchr(pszCandidateFileName, '.');
            if (pszLastDot != nullptr)
            {
                *pszLastDot = '\0';
            }
        }
    } while (pszLastDot != nullptr);

    // If name was not found when searching in the "upward direction", then
    // the name might be a namespace name in a down-level file.
    if (hr == RO_E_METADATA_NAME_NOT_FOUND)
    {
        char16 pszFilePathSearchTemplate[MAX_PATH + 1] = {0};
        WIN32_FIND_DATA fd;
        HANDLE hFindFile;

        StringCchCopy(pszFilePathSearchTemplate, ARRAYSIZE(pszFilePathSearchTemplate), pszDirectoryPath);
        StringCchCat(pszFilePathSearchTemplate, ARRAYSIZE(pszFilePathSearchTemplate), pszFullName);
        StringCchCat(pszFilePathSearchTemplate, ARRAYSIZE(pszFilePathSearchTemplate), _u(".*"));
        StringCchCat(pszFilePathSearchTemplate, ARRAYSIZE(pszFilePathSearchTemplate), METADATA_FILE_EXTENSION);

        // Search in all files in the directory whose name begin with the input string.
        hFindFile = FindFirstFile(pszFilePathSearchTemplate, &fd);

        if (hFindFile != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    continue;
                }

                StringCchCopy(pszFilePathSearchTemplate, ARRAYSIZE(pszFilePathSearchTemplate), pszDirectoryPath);
                StringCchCat(pszFilePathSearchTemplate, ARRAYSIZE(pszFilePathSearchTemplate), fd.cFileName);

                hr = FindTypeInMetaDataFile(
                    pMetaDataDispenser,
                    pszFullName,
                    pszFilePathSearchTemplate,
                    TRO_RESOLVE_NAMESPACE,
                    ppMetaDataImport,
                    pmdTypeDef);

                Assert(hr != S_OK); // We are only searching for namespace name and not type name in the "downward direction".
                if (hr == RO_E_METADATA_NAME_IS_NAMESPACE)
                {
                    break;
                }
            } while (FindNextFile(hFindFile, &fd));

            FindClose(hFindFile);
        }
    }

    return hr;
}

HRESULT ActiveScriptDirectHost::FindType(
    __in IMetaDataDispenserEx *pMetaDataDispenser,
    __in PCWSTR pszFullName,
    __out_opt HSTRING *phstrMetaDataFilePath,
    __deref_out_opt IMetaDataImport2 **ppMetaDataImport,
    __out_opt mdTypeDef *pmdTypeDef)
{
    HRESULT hr = S_OK;

    WCHAR pszBaseFolderPath[MAX_PATH+1];
    ExpandEnvironmentStrings(WIN_JSHOST_METADATA_BASE_PATH, pszBaseFolderPath, _countof(pszBaseFolderPath));

    if (SUCCEEDED(hr))
    {
        hr = FindTypeInDirectory(
            pMetaDataDispenser,
            pszFullName,
            pszBaseFolderPath,
            phstrMetaDataFilePath,
            ppMetaDataImport,
            pmdTypeDef);
    }

    return hr;
}

HRESULT ActiveScriptDirectHost::RoGetMetaDataFile(
    __in const HSTRING name,
    __in_opt IMetaDataDispenserEx *metaDataDispenser,
    __out_opt HSTRING *metaDataFilePath,
    __deref_out_opt IMetaDataImport2 **metaDataImport,
    __out_opt mdTypeDef *typeDefToken)
{
    HRESULT hr = S_OK;

    // Reset output parameters.
    if (metaDataFilePath != nullptr)
    {
        *metaDataFilePath = nullptr;
    }
    if (metaDataImport != nullptr)
    {
        *metaDataImport = nullptr;
    }
    if (typeDefToken != nullptr)
    {
        *typeDefToken =  mdTypeDefNil;
    }

    // The metadata reader object and the TypeDef token parameters are treated as a pair.
    if (((metaDataImport == nullptr) && (typeDefToken != nullptr)) ||
        ((metaDataImport != nullptr) && (typeDefToken == nullptr)))
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        hr = ValidateNameFormat(name);

        if (SUCCEEDED(hr))
        {
            IMetaDataDispenserEx *pMetaDataDispenser = nullptr;

            // The API uses the caller's passed-in metadata dispenser. If null, it
            // will create an instance of the metadata reader to dispense metadata files.
            if (metaDataDispenser == nullptr)
            {
                hr = CoCreateInstance(
                    CLSID_CorMetaDataDispenser,
                    nullptr,
                    CLSCTX_INPROC_SERVER,
                    IID_IMetaDataDispenserEx,
                    reinterpret_cast<void **>(&pMetaDataDispenser));
            }
            else
            {
                pMetaDataDispenser = metaDataDispenser;
            }

            if (SUCCEEDED(hr))
            {
                PCWSTR pszFullName = m_WinRTStringLibrary.WindowsGetStringRawBuffer(name, nullptr);

                hr = FindType(
                    pMetaDataDispenser,
                    pszFullName,
                    metaDataFilePath,
                    metaDataImport,
                    typeDefToken);

                // Release the reference only if we created the metadata reader.
                if ((metaDataDispenser == nullptr) && (pMetaDataDispenser != nullptr))
                {
                    pMetaDataDispenser->Release();
                    pMetaDataDispenser = nullptr;
                }
            }
        }
    }

    return hr;
}