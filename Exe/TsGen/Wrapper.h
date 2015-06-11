//---------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "settings.h"
#include "XmlReader.h"

using namespace regex;

namespace Wrapper
{
    struct MetadataContext : ProjectionModel::ITypeResolver, Metadata::IStringConverter
    {
        ArenaAllocator* alloc;
        IMetaDataDispenser* dispenser;
        typedef JsUtil::BaseDictionary<MetadataStringId, Metadata::Assembly*, ArenaAllocator, PrimeSizePolicy> TMetadata;
        typedef JsUtil::BaseDictionary<MetadataStringId, Metadata::TypeDefProperties*, ArenaAllocator, PrimeSizePolicy> TTypeDefs;
        typedef JsUtil::BaseDictionary<WCHAR*, MetadataStringId, ArenaAllocator, PrimeSizePolicy> TStringToProjectionTypeId;
        typedef JsUtil::BaseDictionary<MetadataStringId, WCHAR*, ArenaAllocator, PrimeSizePolicy> TProjectionTypeIdToString;
        TMetadata* metadata;
        TTypeDefs* typeDefs;
        TStringToProjectionTypeId* stringToProjectionTypeId;
        TProjectionTypeIdToString* projectionTypeIdToString;
        ImmutableList<Metadata::Assembly*>* assemblies;
        ImmutableList<Metadata::Assembly*>* referenceAssemblies;
        Js::DelayLoadWinRtRoParameterizedIID roParameterizedIIDDelayLoad;
        MetadataStringId nextTypeId;
        bool enableVersioningAllAssemblies;

        MetadataContext(_Inout_ ArenaAllocator* alloc, _Inout_ IMetaDataDispenser* dispenser, _In_ ImmutableList<LPCWSTR>* winmds, _In_ ImmutableList<LPCWSTR>* references, bool enableVersioningAllAssemblies);

        // Info:        Resolve a type name to a type def
        // Parameter:   typeName - the type name
        //              typeDef - receives the typeDef
        // Return:      S_OK if the type was found, E_FAIL if not
        HRESULT ResolveTypeName(MetadataStringId typeId, _In_ LPCWSTR typeName, _In_ Metadata::TypeDefProperties** typeDef) override;

        // Info:        Get the DelayLoadWinRtRoParameterizedIID
        Js::DelayLoadWinRtRoParameterizedIID* GetRoParameterizedIIDDelayLoad();

        // Info:        Get the string id for a given string.
        // Parameter:   sz - the string
        // Return:      The id of the string
        MetadataStringId IdOfString(_In_ LPCWSTR sz);

        // Info:        Get the string for a given string id.
        // Parameter:   id - the id
        // Return:      The string
        LPCWSTR StringOfId(MetadataStringId id);

        // Info:        Determine whether this assembly should be versioned
        // Parameter:   assemblyName - the name of the assembly
        // Return:      Whether or not this assembly should be versioned
        bool IsVersionedAssembly(_In_ LPCWSTR assemblyName);
    };

    class ObjectModel
    {
    public:
        Settings* settings = nullptr;
        ArenaAllocator* alloc = nullptr;
        RtASSIGNMENTSPACE expr = nullptr;
        ProjectionModel::ProjectionBuilder* builder = nullptr;
        MetadataContext* resolver = nullptr;
        XmlReader::XmlReader* xmlReader = nullptr;

        ObjectModel() {}
        ObjectModel(_In_ ArenaAllocator* a, _In_ MetadataContext* r, _In_ Settings* s, _In_ ProjectionModel::ProjectionBuilder* b);
    };
}