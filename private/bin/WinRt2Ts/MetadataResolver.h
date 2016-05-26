//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include <map>
#include <memory>
#include <vector>
#include <string>

#include "wrl.h"

#include "ProjectionModel.h"

class StringConverter : public Metadata::IStringConverter
{
public:
    MetadataStringId IdOfString(LPCWSTR sz) override;
    LPCWSTR StringOfId(MetadataStringId id) override;

private:
    std::vector<const wchar_t*> projectionTypeIdToString;
    std::map<std::wstring, MetadataStringId> stringToProjectionTypeId;
};

class MetadataResolver : public ProjectionModel::ITypeResolver
{
public:
    MetadataResolver(
        ArenaAllocator* allocator,
        Metadata::IStringConverter* converter,
        IMetaDataDispenser* dispenser,
        std::vector<std::wstring> winmds,
        bool enableVersioningAllAssemblies,
        bool enableVersioningWindowsAssemblies
        );

    HRESULT ResolveTypeName(MetadataStringId typeId, LPCWSTR typeName, Metadata::TypeDefProperties ** typeDef) override;
    Js::DelayLoadWinRtRoParameterizedIID *GetRoParameterizedIIDDelayLoad() override;

    std::vector<Metadata::Assembly*> assemblies;

private:
    Metadata::IStringConverter* converter;
    IMetaDataDispenser* dispenser;
    std::map<MetadataStringId, Metadata::TypeDefProperties*> typeDefs;
    Js::DelayLoadWinRtRoParameterizedIID roParameterizedIIDDelayLoad;
};