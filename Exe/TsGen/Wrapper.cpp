//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include <metahost.h>
#include <sdkddkver.h>
#include "wrapper.h"

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#define NTDDI_VERSION NTDDI_WIN8
#include <rometadata.h>

// Info:        Import a particular assembly from the dispenser
// Parameter:   dispenser - metadata dispenser
//              filename - filename of the assembly to read
// Return:      the importer
IMetaDataImport2* ImportAssembly(_Inout_ IMetaDataDispenser* dispenser, _In_ LPCWSTR filename)
{
    // TODO: Use factored out metadata reader
    IMetaDataImport2* import;

    auto hr = dispenser->OpenScope(filename, (ofRead | ofNoTransform), IID_IMetaDataImport2, (LPUNKNOWN*)&import);
    if (FAILED(hr))
    {
        UINT oldcp = GetConsoleOutputCP();

        if (0 != SetConsoleOutputCP(CP_UTF8))
        {
            int bufferSize = WideCharToMultiByte(CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL);
            char* fileNameUTF8 = new char[bufferSize];
            WideCharToMultiByte(CP_UTF8, 0, filename, -1, fileNameUTF8, bufferSize, NULL, NULL);
            wprintf(L"<ignored-missing-metadatafile filename='%S'/>\n", fileNameUTF8);
            delete[] fileNameUTF8;
        }
        else
        {
            wprintf(L"<ignored-missing-metadatafile filename='%S'/>\n", filename);
        }

        SetConsoleOutputCP(oldcp);

        return nullptr;
    }
    return import;
}




Wrapper::MetadataContext::MetadataContext(_Inout_ ArenaAllocator* alloc, _Inout_ IMetaDataDispenser* dispenser, _In_ ImmutableList<LPCWSTR>* winmds, _In_ ImmutableList<LPCWSTR>* references, bool enableVersioningAllAssemblies)
    : alloc(alloc), dispenser(dispenser), assemblies(nullptr), referenceAssemblies(nullptr), nextTypeId(1000), enableVersioningAllAssemblies(enableVersioningAllAssemblies)
{
    roParameterizedIIDDelayLoad.Ensure();

    metadata = Anew(alloc, TMetadata, alloc);
    typeDefs = Anew(alloc, TTypeDefs, alloc);
    stringToProjectionTypeId = Anew(alloc, TStringToProjectionTypeId, alloc, 0);
    projectionTypeIdToString = Anew(alloc, TProjectionTypeIdToString, alloc, 0);

    winmds->Iterate([&](LPCWSTR winmd) {
        auto key = IdOfString(winmd);
        if (!metadata->ContainsKey(key))
        {
            auto import = ImportAssembly(dispenser, winmd);
            if (import)
            {
                auto assembly = Anew(alloc, Metadata::Assembly, import, this, alloc, IsVersionedAssembly(winmd));
                metadata->Add(key, assembly);
                assemblies = assemblies->Prepend(assembly, alloc);
                referenceAssemblies = referenceAssemblies->Prepend(assembly, alloc);
            }
        }
    });
    references->Iterate([&](LPCWSTR winmd) {
        auto key = IdOfString(winmd);
        if (!metadata->ContainsKey(key))
        {
            auto import = ImportAssembly(dispenser, winmd);
            if (import)
            {
                auto assembly = Anew(alloc, Metadata::Assembly, import, this, alloc, IsVersionedAssembly(winmd));
                metadata->Add(key, assembly);
                referenceAssemblies = referenceAssemblies->Prepend(assembly, alloc);
            }
        }
    });
}


// Info:        Resolve a type name to a type def
// Parameter:   typeName - the type name
//              typeDef - receives the typeDef
// Return:      S_OK if the type was found, E_FAIL if not
HRESULT Wrapper::MetadataContext::ResolveTypeName(MetadataStringId typeId, _In_ LPCWSTR typeName, _In_ Metadata::TypeDefProperties** typeDef)
{
    auto key = typeId;
    if (!typeDefs->TryGetValue(key, typeDef))
    {
        auto references = referenceAssemblies;
        while (references)
        {
            LPCWSTR typeDefName = typeName;
            if (!typeDefName)
            {
                typeDefName = StringOfId(typeId);
            }
            auto reference = references->First();
            *typeDef = const_cast<Metadata::TypeDefProperties*>(reference->FindTopLevelTypeDefByName(typeDefName));
            if (*typeDef)
            {
                Js::VerifyCatastrophic((*typeDef)->id == key);
                typeDefs->Add(key, *typeDef);
                return S_OK;
            }
            references = references->GetTail();
        }

        // Error recovery path. Add null to dictionary so that we don't try to find this missing type again.
        typeDefs->Add(key, nullptr);
        return E_FAIL;
    }
    return *typeDef ? S_OK : E_FAIL;
}

// Info:        Get the DelayLoadWinRtRoParameterizedIID
Js::DelayLoadWinRtRoParameterizedIID* Wrapper::MetadataContext::GetRoParameterizedIIDDelayLoad()
{
    return &roParameterizedIIDDelayLoad;
}

// Info:        Get the string id for a given string.
// Parameter:   sz - the string
// Return:      The id of the string
MetadataStringId Wrapper::MetadataContext::IdOfString(_In_ LPCWSTR sz)
{
    auto key = const_cast<WCHAR*>(sz);
    MetadataStringId typeId = MetadataStringIdNil;
    if (sz) // nullptr <=> MetadataStringIdNil
    {
        if (!stringToProjectionTypeId->TryGetValue(key, &typeId))
        {
            typeId = nextTypeId;
            ++nextTypeId;
            size_t strLen = wcslen(sz) + 1;
            LPWSTR string = AnewArrayZ(alloc, wchar_t, strLen);
            wcscpy_s(string, strLen, sz);
            stringToProjectionTypeId->Add(string, typeId);
            projectionTypeIdToString->Add(typeId, string);
        }
    }
    return typeId;
}

// Info:        Get the string for a given string id.
// Parameter:   id - the id
// Return:      The string
LPCWSTR Wrapper::MetadataContext::StringOfId(MetadataStringId id)
{
    WCHAR* sz = nullptr;
    if (id != MetadataStringIdNil)
    {
        projectionTypeIdToString->TryGetValue(id, &sz);
    }
    return sz;

}

// Info:        Determine whether this assembly should be versioned
// Parameter:   assemblyName - the name of the assembly
// Return:      Whether or not this assembly should be versioned
bool Wrapper::MetadataContext::IsVersionedAssembly(_In_ LPCWSTR assemblyName)
{
    // If versioning is enabled for all assemblies, always return true
    if (enableVersioningAllAssemblies)
    {
        return true;
    }

    // Otherwise, versioning is only enabled for first-party Windows winmds (assemblies starting with "Windows.").
    LPCWSTR firstPartyWinmdPrefix = L"Windows.";
    size_t prefixCharCount = wcslen(firstPartyWinmdPrefix);

    if (wcslen(assemblyName) <= prefixCharCount)
    {
        return false;
    }

    return (_wcsnicmp(assemblyName, firstPartyWinmdPrefix, prefixCharCount) == 0);
}