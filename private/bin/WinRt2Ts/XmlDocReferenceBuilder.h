//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "XmlDocReference.h"
#include <map>

class XmlDocReferenceBuilder
{
public:
    XmlDocReferenceBuilder(ArenaAllocator* alloc, std::map<Metadata::Assembly*, std::wstring>& assemblyToFullPath);

    XmlDocReference MakeForMethod(RtMETHODSIGNATURE methodSignature);
    XmlDocReference MakeForProperty(MetadataString propertyMetadataName);
    XmlDocReference MakeForField(MetadataString propertyMetadataName);
    XmlDocReference MakeForEventHandler(MetadataString propertyMetadataName);
    XmlDocReference MakeForCurrentType();

    void SetCurrentType(const Metadata::TypeDefProperties* currentTypeDef);

private:
    XmlDocReference XmlDocReferenceBuilder::MakeForMember(LPCWSTR prefix, LPCWSTR name);
    std::wstring XmlDocReferenceBuilder::GetXmlDocTypeName(RtTYPE type);

    struct
    {
        MetadataString containingAssemblyName;
        MetadataString containingTypeName;
    } m_context;

    ArenaAllocator* m_alloc;
    std::map<Metadata::Assembly*, std::wstring>& m_assemblyToFullPath;
};

