//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "XmlDocReferenceBuilder.h"

using namespace std;
using namespace ProjectionModel;
using namespace Metadata;

XmlDocReferenceBuilder::XmlDocReferenceBuilder(ArenaAllocator* alloc, map<Assembly*, wstring>& assemblyToFullPath) :
    m_alloc(alloc),
    m_assemblyToFullPath(assemblyToFullPath)
{
}

XmlDocReference XmlDocReferenceBuilder::MakeForMethod(RtMETHODSIGNATURE methodSignature)
{
    wstring methodId = MetadataString(AbiMethodSignature::From(methodSignature)->metadataNameId).ToString();

    auto inParams = methodSignature->GetParameters()->allParameters->Where([&](RtPARAMETER param) {
        // Don't include the return value
        return param->isIn || AbiParameter::From(param)->inParameterIndex > 0 || AbiParameter::From(param)->GetParameterOnStackCount() > 1;
    }, m_alloc);
    if (!inParams->IsEmpty())
    {
        methodId += L"(";
        inParams->IterateBetween(
            [&](RtPARAMETER param) { methodId += GetXmlDocTypeName(param->type); },
            [&](RtPARAMETER, RtPARAMETER) { methodId += L","; });
        methodId += L")";
    }

    return MakeForMember(L"M:", methodId.c_str());
}

XmlDocReference XmlDocReferenceBuilder::MakeForProperty(MetadataString propertyMetadataName)
{
    return MakeForMember(L"P:", propertyMetadataName.ToString());
}

XmlDocReference XmlDocReferenceBuilder::MakeForField(MetadataString propertyMetadataName)
{
    return MakeForMember(L"F:", propertyMetadataName.ToString());
}

XmlDocReference XmlDocReferenceBuilder::MakeForEventHandler(MetadataString propertyMetadataName)
{
    return MakeForMember(L"E:", propertyMetadataName.ToString());
}

XmlDocReference XmlDocReferenceBuilder::MakeForCurrentType()
{
    XmlDocReference doc;

    wstring id = L"T:";
    id += m_context.containingTypeName.ToString();

    doc.xmlDocId = MetadataString(id.c_str());
    doc.xmlDocFile = m_context.containingAssemblyName;

    return doc;
}

void XmlDocReferenceBuilder::SetCurrentType(const Metadata::TypeDefProperties* currentTypeDef)
{
    m_context.containingAssemblyName = m_assemblyToFullPath.find(const_cast<Assembly*>(&currentTypeDef->assembly))->second.c_str();
    m_context.containingTypeName = currentTypeDef->id;
}

XmlDocReference XmlDocReferenceBuilder::MakeForMember(LPCWSTR prefix, LPCWSTR name)
{
    XmlDocReference doc;

    wstring id = prefix;
    id += m_context.containingTypeName.ToString();
    id += L".";
    id += name;

    doc.xmlDocId = MetadataString(id.c_str());
    doc.xmlDocFile = m_context.containingAssemblyName;

    return doc;
}

wstring XmlDocReferenceBuilder::GetXmlDocTypeName(RtTYPE type)
{
    // The format of doc IDs are documented at http://msdn.microsoft.com/en-us/library/fsbx0t7x.aspx 

    if (TypeDefinitionType::Is(type))
    {
        auto tdt = TypeDefinitionType::From(type);
        auto typeName = MetadataString(tdt->typeDef->id).ToString();

        if (wcscmp(typeName, L"Windows.Foundation.PropertyValue") == 0
            || wcscmp(typeName, L"Windows.Foundation.IPropertyValue") == 0)
        {
            return L"System.Object";
        }

        if (wcscmp(typeName, L"Windows.Foundation.IReference`1") == 0)
        {
            Assert(tdt->genericParameters->Count() == 1);
            return GetXmlDocTypeName(tdt->genericParameters->First());
        }

        if (wcscmp(typeName, L"Windows.Foundation.IReferenceArray`1") == 0)
        {
            Assert(tdt->genericParameters->Count() == 1);
            return GetXmlDocTypeName(tdt->genericParameters->First()) + L"[]";
        }

        return typeName;
    }

    switch (type->typeCode)
    {
    case tcVoidType:
        return L"Void";
    case tcByRefType:
    {
        auto br = ByRefType::From(type);
        return GetXmlDocTypeName(br->pointedTo) + L"@";
    }
    case tcSystemGuidType:
        return L"System.Guid";
    case tcWindowsFoundationDateTimeType:
        return L"Windows.Foundation.DateTime";
    case tcWindowsFoundationTimeSpanType:
        return L"Windows.Foundation.TimeSpan";
    case tcWindowsFoundationEventRegistrationTokenType:
        return L"Windows.Foundation.EventRegistrationToken";
    case tcWindowsFoundationHResultType:
        return L"Windows.Foundation.HResult";
    case tcArrayType:
    {
        auto at = ArrayType::From(type);
        return GetXmlDocTypeName(at->elementType) + L"[]";
    }
    case tcGenericClassVarType:
    {
        auto gcv = GenericClassVarType::From(type);
        wchar_t index[32];
        swprintf_s(index, L"'T%d'", gcv->var->index);
        return index;
    }
    break;
    case tcMissingNamedType:
    {
        auto stn = MissingNamedType::From(type);
        return MetadataString(stn->fullTypeNameId).ToString();
    }
    case tcGenericParameterType:
    {
        auto gpt = GenericParameterType::From(type);
        wchar_t index[32];
        swprintf_s(index, L"`%d", gpt->properties->sequence);
        return index;
    }
    break;

    case tcMissingGenericInstantiationType:
    {
        auto gi = MissingGenericInstantiationType::From(type);
        return MetadataString(gi->parent->fullTypeNameId).ToString();
    }

    case tcBasicType:
        auto bt = BasicType::From(type);
        switch (bt->typeCor)
        {
        case ELEMENT_TYPE_STRING:
            return L"System.String";
        case ELEMENT_TYPE_CHAR:
            return L"System.Char";
        case ELEMENT_TYPE_BOOLEAN:
            return L"System.Boolean";
        case ELEMENT_TYPE_I1:
            return L"System.Int8";
        case ELEMENT_TYPE_U1:
            return L"System.Byte";
        case ELEMENT_TYPE_I2:
            return L"System.Int16";
        case ELEMENT_TYPE_U2:
            return L"System.UInt16";
        case ELEMENT_TYPE_I4:
            return L"System.Int32";
        case ELEMENT_TYPE_U4:
            return L"System.UInt32";
        case ELEMENT_TYPE_I8:
            return L"System.Int64";
        case ELEMENT_TYPE_U8:
            return L"System.UInt64";
        case ELEMENT_TYPE_R4:
            return L"System.Single";
        case ELEMENT_TYPE_R8:
            return L"System.Double";
        case ELEMENT_TYPE_OBJECT:
            return L"System.Object";
        case ELEMENT_TYPE_VAR:
            return L"T";
        }
    }

    return L"";
}
