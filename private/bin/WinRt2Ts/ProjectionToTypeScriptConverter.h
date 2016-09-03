//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "TypeScriptEmitter.h"
#include "ProjectionToTypeScriptError.h"
#include "XmlDocReferenceBuilder.h"

class NamespaceContext
{
public:
    NamespaceContext(Metadata::IStringConverter& converter);

    void PushNamespace(MetadataString namespaceNameId);
    void PopNamespace();

    MetadataString GetNameIdRelativeToCurrentContext(MetadataString fullyQualifiedNameId);

private:
    std::wstring m_currentNamespace;
    Metadata::IStringConverter& m_converter;
};

class ProjectionToTypeScriptConverter
{
public:
    ProjectionToTypeScriptConverter(ArenaAllocator* alloc, TypeScriptEmitter& emitter, IndentingWriter& writer, Metadata::IStringConverter& converter, XmlDocReferenceBuilder& docBuilder, bool emitAnyForUnresolvedTypes, bool suppressWarningsForUnresolvedWindowsTypes);
    void EmitTopLevelNamespace(MetadataStringId namespaceNameId, RtPROPERTIESOBJECT childProperties);

private:

    enum class FullyQualifiedNameBehavior
    {
        EmitAsIs,
        EmitAsOptional,
        AlsoEmitUnqualifiedMember
    };
    
    template <typename MakeMemberFunction, typename MakeAnyMemberFunction>
    void AddPropertyToMemberList(
        XmlDocReference doc,
        TypeScriptTypeMemberList* memberList,
        MetadataString propertyName,
        FullyQualifiedNameBehavior fullyQualifiedNameBehavior,
        MakeMemberFunction makeMember,
        MakeAnyMemberFunction makeAnyMember
    );

    struct DelegateDeclarations
    {
        std::auto_ptr<TypeScriptInterfaceDeclaration> interfaceDeclaration;
        std::auto_ptr<TypeScriptTypeAliasDeclaration> eventHandlerArgumentTypeAlias;
        std::auto_ptr<TypeScriptTypeAliasDeclaration> eventHandlerReturnTypeAlias;
    };

    void EmitSpecialTypes();
    void EmitNamespace(MetadataStringId namespaceNameId, RtPROPERTIESOBJECT childProperties);
    void EmitTypeDefinitions(RtPROPERTIESOBJECT properties);
    bool IsTypeVisible(RtTYPE type);
    bool AreAllParameterTypesVisible(RtPARAMETERS params);
    std::auto_ptr<TypeScriptEnumDeclaration> GetTypeScriptDeclarationForEnum(MetadataString enumName, RtENUM enumDefinition);
    std::auto_ptr<TypeScriptClassDeclaration> GetTypeScriptDeclarationForRuntimeClass(MetadataString runtimeClassName, RtRUNTIMECLASSCONSTRUCTOR rtClassConstructor);
    std::auto_ptr<TypeScriptInterfaceDeclaration> GetTypeScriptDeclarationForInterface(MetadataString interfaceName, RtRUNTIMEINTERFACECONSTRUCTOR interfaceConstructor);
    std::auto_ptr<TypeScriptInterfaceDeclaration> GetTypeScriptDeclarationForStruct(MetadataString structName, RtSTRUCTCONSTRUCTOR structConstructor);
    DelegateDeclarations GetTypeScriptDeclarationForDelegate(MetadataString delegateName, RtDELEGATECONSTRUCTOR delegateConstructor);
    std::auto_ptr<TypeScriptType> GetTypeScriptTypeDefinitionType(MetadataString name, regex::ImmutableList<RtTYPE>* parameters);
    std::auto_ptr<TypeScriptType> GetTypeScriptType(RtTYPE type);
    std::auto_ptr<TypeScriptType> GetTypeScriptArrayTypeOrNull(RtSPECIALIZATION specialization);
    std::auto_ptr<TypeScriptParameterList> GetTypeScriptParameters(RtPARAMETERS params);
    std::auto_ptr<TypeScriptType> GetTypeScriptParameterTypeOrNull(RtPARAMETERS params, size_t argIndex);
    std::auto_ptr<TypeScriptType> GetTypeScriptTupleTypeFromArguments(RtPARAMETERS params, size_t startIndex);
    std::auto_ptr<TypeScriptType> GetTypeScriptReturnType(RtPARAMETERS params);
    std::auto_ptr<TypeScriptTypeMemberList> GetTypeScriptMethodSignatures(MetadataString methodName, bool applyIndexOfSpecialization, RtMETHODSIGNATURE methodSignature, FullyQualifiedNameBehavior fullyQualifiedNameBehavior);
    std::auto_ptr<TypeScriptIndexSignature> GetTypeScriptIndexSignatureOrNull(RtSPECIALIZATION mapSpecialization);
    std::auto_ptr<TypeScriptTypeList> GetExtendedTypeScriptTypeList(regex::ImmutableList<RtINTERFACECONSTRUCTOR>* extendsInterfaces, RtSPECIALIZATION specialization, RtTYPE promiseResultType);
    std::auto_ptr<TypeScriptTypeList> GetImplementedTypeScriptTypeList(regex::ImmutableList<RtINTERFACECONSTRUCTOR>* implementedInterfaces);
    std::auto_ptr<TypeScriptTypeMemberList> GetPropertyAndMethodMembers(regex::ImmutableList<RtPROPERTY>* properties, bool applyIndexOfSpecialization, FullyQualifiedNameBehavior fullyQualifiedNameBehavior);
    std::auto_ptr<TypeScriptTypeMemberList> GetInterfaceMethodAndPropertyMembers(bool extendsArray, regex::ImmutableList<MetadataStringId>* metadataNameFilter, regex::ImmutableList<RtPROPERTY>* properties);
    std::auto_ptr<TypeScriptConstructorOverloads> GetConstructorOverloads(RtMETHODSIGNATURE methodSignature);
    std::auto_ptr<TypeScriptMethodSignature> GetEventListenerMethodSignatureOrNull(MetadataStringId methodName, RtEVENT event);
    std::auto_ptr<TypeScriptMethodSignature> GetGeneralEventListenerMethodSignature(const MetadataStringId methodName);
    std::auto_ptr<TypeScriptType> GetProjectedEventHandlerArgumentTypeBase(RtPARAMETERS params);
    std::auto_ptr<TypeScriptType> GetProjectedEventHandlerType(RtTYPE listenerType, MetadataString eventName);
    RtTYPE GetPromiseResultType(RtRUNTIMEINTERFACECONSTRUCTOR interfaceConstructor);
    LPCWSTR GetLastPartIfFullyQualified(LPCWSTR propertyName);
    std::auto_ptr<TypeScriptType> AppendToTypeName(std::auto_ptr<TypeScriptType>&& delegateType, LPCWSTR suffix);

private:
    ArenaAllocator* m_alloc;
    TypeScriptEmitter& m_emitter;
    Metadata::IStringConverter& m_converter;
    IndentingWriter& m_writer;
    MetadataStringId m_voidStringId;
    MetadataStringId m_indexOfStringId;
    MetadataStringId m_exclusiveToAttributeStringId;
    NamespaceContext m_namespaceContext;
    XmlDocReferenceBuilder& m_docBuilder;
    bool m_emitAnyForUnresolvedTypes;
    bool m_suppressWarningsForUnresolvedWindowsTypes;
};