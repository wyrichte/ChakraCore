//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "MetadataResolver.h"
#include "Winrt2TsErrors.h"

using namespace Metadata;
using namespace Microsoft::WRL;
using namespace ProjectionModel;
using namespace std;

static bool IsWindowsAssembly(LPCWSTR assemblyName)
{
    wstring fileName(assemblyName);
    auto lastSlash = fileName.rfind(L"\\");
    if (lastSlash != wstring::npos)
    {
        fileName.erase(0, lastSlash + 1);
    }
    transform(fileName.begin(), fileName.end(), fileName.begin(), towlower);

    return fileName.compare(0, ARRAYSIZE(L"windows.") - 1, L"windows.") == 0;
}

ComPtr<IMetaDataImport2> ImportAssembly(IMetaDataDispenser * dispenser, LPCWSTR filename)
{
    ComPtr<IMetaDataImport2> import;


    HRESULT hr = dispenser->OpenScope(filename, (ofRead | ofNoTransform), IID_IMetaDataImport2, (IUnknown**)(import.GetAddressOf()));
    if (FAILED(hr))
    {
        throw WinmdReadError(hr, filename);
    }

    return import;
}

MetadataResolver::MetadataResolver(
    ArenaAllocator* allocator,
    IStringConverter* converter,
    IMetaDataDispenser* dispenser,
    vector<wstring> winmds,
    bool enableVersioningAllAssemblies,
    bool enableVersioningWindowsAssemblies
    ) :
    dispenser(dispenser),
    converter(converter)
{
    roParameterizedIIDDelayLoad.EnsureFromSystemDirOnly();

    // Copy into an additional vector to remove duplicates
    vector<wstring> uniqueWinmds;

    for (auto& winmd : winmds)
    {
        if (find(uniqueWinmds.begin(), uniqueWinmds.end(), winmd) == uniqueWinmds.end())
        {
            uniqueWinmds.push_back(winmd);

            auto import = ImportAssembly(dispenser, winmd.c_str());

            auto assembly = Anew(allocator, Assembly, import.Get(), converter, allocator, enableVersioningAllAssemblies || enableVersioningWindowsAssemblies && IsWindowsAssembly(winmd.c_str()));

            assemblies.push_back(assembly);
        }
    }
}

HRESULT MetadataResolver::ResolveTypeName(MetadataStringId typeId, LPCWSTR typeName, Metadata::TypeDefProperties ** typeDef)
{
    auto foundTypeDef = typeDefs.find(typeId);
    if (foundTypeDef != typeDefs.end())
    {
        *typeDef = foundTypeDef->second;
    }
    else
    {
        LPCWSTR typeDefName = typeName;
        if (!typeDefName)
        {
            typeDefName = converter->StringOfId(typeId);
        }

        for (auto& assembly : assemblies)
        {
            *typeDef = const_cast<Metadata::TypeDefProperties *>(assembly->FindTopLevelTypeDefByName(typeDefName));
            if (*typeDef)
            {
                typeDefs[typeId] = *typeDef;
                return S_OK;
            }
        }

        // Add null to dictionary so that we don't try to find this missing type again.
        typeDefs[typeId] = nullptr;
        return E_FAIL;
    }

    return *typeDef ? S_OK : E_FAIL;
}

Js::DelayLoadWinRtRoParameterizedIID* MetadataResolver::GetRoParameterizedIIDDelayLoad()
{
    return &roParameterizedIIDDelayLoad;
}

MetadataStringId StringConverter::IdOfString(LPCWSTR sz)
{
    MetadataStringId typeId = MetadataStringIdNil;

    // nullptr maps to MetadataStringIdNil

    if (sz)
    {
        auto foundTypeId = stringToProjectionTypeId.find(sz);
        if (foundTypeId == stringToProjectionTypeId.end())
        {
            typeId = static_cast<int>(MetadataStringIdNil + projectionTypeIdToString.size() + 1);

            auto mapElement = stringToProjectionTypeId.insert(make_pair(sz, typeId));

            // Push the string pointer into the vector, the string is owned by the map
            projectionTypeIdToString.push_back(mapElement.first->first.c_str());

            Assert(stringToProjectionTypeId.size() == projectionTypeIdToString.size());
        }
        else
        {
            typeId = foundTypeId->second;
        }
    }

    return typeId;
}

LPCWSTR StringConverter::StringOfId(MetadataStringId id)
{
    // MetadataStringIdNil maps to nullptr

    if (id == MetadataStringIdNil)
    {
        return nullptr;
    }
    else
    {
        size_t idx = id - MetadataStringIdNil - 1;
        return projectionTypeIdToString[idx];
    }
}