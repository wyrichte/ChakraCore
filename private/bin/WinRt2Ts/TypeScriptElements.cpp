//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "TypeScriptElements.h"

using namespace std;

const MetadataString& TypeScriptPropertySignature::GetName() const
{
    return m_name;
}

const TypeScriptType& TypeScriptPropertySignature::GetType() const
{
    return *m_type;
}

auto_ptr<TypeScriptPropertySignature> TypeScriptPropertySignature::Make(MetadataString name, auto_ptr<TypeScriptType>&& type)
{
    return auto_ptr<TypeScriptPropertySignature>(new TypeScriptPropertySignature(name, type));
}

TypeScriptPropertySignature::TypeScriptPropertySignature(MetadataString name, auto_ptr<TypeScriptType> type) :
    m_name(name),
    m_type(type.release())
{
}

const MetadataString& TypeScriptType::GetName() const
{
    return m_name;
}

bool TypeScriptType::IsArray() const
{
    return m_type == Type::Array;
}

bool TypeScriptType::IsObject() const
{
    return m_type == Type::Object;
}

bool TypeScriptType::IsFunction() const
{
    return m_type == Type::Function;
}

const auto_ptr_vector<TypeScriptType>& TypeScriptType::GetTypeArguments() const
{
    return m_typeArguments;
}

const auto_ptr_vector<TypeScriptPropertySignature>& TypeScriptType::GetObjectTypeMembers() const
{
    return m_objectTypeMembers;
}

const TypeScriptParameterList& TypeScriptType::GetFunctionParameters() const
{
    return *m_functionParameters;
}

const TypeScriptType& TypeScriptType::GetFunctionReturnType() const
{
    return *m_functionReturnType;
}

auto_ptr<TypeScriptType> TypeScriptType::Make(MetadataString name)
{
    return auto_ptr<TypeScriptType>(new TypeScriptType(name, Type::PredefinedOrReference, auto_ptr_vector<TypeScriptType>(), auto_ptr_vector<TypeScriptPropertySignature>(), auto_ptr<TypeScriptParameterList>(), auto_ptr<TypeScriptType>()));
}

auto_ptr<TypeScriptType> TypeScriptType::MakeTypeReference(MetadataString name, auto_ptr_vector<TypeScriptType>&& typeArguments)
{
    return auto_ptr<TypeScriptType>(new TypeScriptType(name, Type::PredefinedOrReference, move(typeArguments), auto_ptr_vector<TypeScriptPropertySignature>(), auto_ptr<TypeScriptParameterList>(), auto_ptr<TypeScriptType>()));
}

auto_ptr<TypeScriptType> TypeScriptType::MakeTypeReference(MetadataString name, auto_ptr<TypeScriptType>&& typeArgument)
{
    auto_ptr_vector<TypeScriptType> typeArguments;
    typeArguments.push_back(typeArgument.release());
    return auto_ptr<TypeScriptType>(new TypeScriptType(name, Type::PredefinedOrReference, move(typeArguments), auto_ptr_vector<TypeScriptPropertySignature>(), auto_ptr<TypeScriptParameterList>(), auto_ptr<TypeScriptType>()));
}

auto_ptr<TypeScriptType> TypeScriptType::MakeObjectType(auto_ptr_vector<TypeScriptPropertySignature>&& objectTypeMembers)
{
    return auto_ptr<TypeScriptType>(new TypeScriptType(MetadataString(), Type::Object, auto_ptr_vector<TypeScriptType>(), move(objectTypeMembers), auto_ptr<TypeScriptParameterList>(), auto_ptr<TypeScriptType>()));
}

auto_ptr<TypeScriptType> TypeScriptType::MakeArrayType(auto_ptr<TypeScriptType>&& type)
{
    auto newType = type;
    const_cast<Type&>(newType->m_type) = Type::Array;
    return newType;
}

auto_ptr<TypeScriptType> TypeScriptType::MakeFunctionType(auto_ptr<TypeScriptParameterList>&& parameters, auto_ptr<TypeScriptType>&& returnType)
{
    return auto_ptr<TypeScriptType>(new TypeScriptType(MetadataString(), Type::Function, auto_ptr_vector<TypeScriptType>(), auto_ptr_vector<TypeScriptPropertySignature>(), move(parameters), move(returnType)));
}

TypeScriptType::TypeScriptType(
    MetadataString name, 
    Type type,
    auto_ptr_vector<TypeScriptType>&& typeArguments, 
    auto_ptr_vector<TypeScriptPropertySignature>&& objectTypeMembers,
    auto_ptr<TypeScriptParameterList>&& functionParameters,
    auto_ptr<TypeScriptType>&& functionReturnType
    ) :
    m_name(name),
    m_type(type),
    m_typeArguments(typeArguments),
    m_objectTypeMembers(objectTypeMembers),
    m_functionParameters(functionParameters),
    m_functionReturnType(functionReturnType)
{
    typeArguments.clear();
    objectTypeMembers.clear();
}

const TypeScriptType& TypeScriptIndexSignature::GetIndexType() const
{
    return *m_indexType;
}

const TypeScriptType& TypeScriptIndexSignature::GetType() const
{
    return *m_type;
}

auto_ptr<TypeScriptIndexSignature> TypeScriptIndexSignature::Make(auto_ptr<TypeScriptType>&& indexType, auto_ptr<TypeScriptType>&& type)
{
    return auto_ptr<TypeScriptIndexSignature>(new TypeScriptIndexSignature(move(indexType), move(type)));
}

TypeScriptIndexSignature::TypeScriptIndexSignature(auto_ptr<TypeScriptType>&& indexType, auto_ptr<TypeScriptType>&& type) :
    m_indexType(indexType.release()),
    m_type(type.release())
{
}

const MetadataString& TypeScriptParameter::GetName() const
{
    return m_name;
}

const MetadataString* TypeScriptParameter::GetStringLiteralOrNull() const
{
    return m_literal.get();
}

const TypeScriptType* TypeScriptParameter::GetTypeOrNull() const
{
    return m_type.get();
}

auto_ptr<TypeScriptParameter> TypeScriptParameter::Make(MetadataString name, auto_ptr<TypeScriptType>&& type)
{
    return auto_ptr<TypeScriptParameter>(new TypeScriptParameter(name, nullptr, type.release()));
}

auto_ptr<TypeScriptParameter> TypeScriptParameter::Make(MetadataString name, MetadataString literal)
{
    return auto_ptr<TypeScriptParameter>(new TypeScriptParameter(name, new MetadataString(literal), nullptr));
}

TypeScriptParameter::TypeScriptParameter(MetadataString name, MetadataString* literal, TypeScriptType* type) :
    m_name(name),
    m_literal(literal),
    m_type(type)
{
}

const auto_ptr_vector<TypeScriptParameter>& TypeScriptParameterList::GetParameters() const
{
    return m_parameters;
}

const MetadataString* TypeScriptParameterList::GetRestParameterNameOrNull() const
{
    return m_restParameterName.get();
}

auto_ptr<TypeScriptParameterList> TypeScriptParameterList::Make(auto_ptr_vector<TypeScriptParameter>&& parameters)
{
    return auto_ptr<TypeScriptParameterList>(new TypeScriptParameterList(move(parameters), nullptr));
}

auto_ptr<TypeScriptParameterList> TypeScriptParameterList::Make(MetadataString restParameterName)
{
    return auto_ptr<TypeScriptParameterList>(new TypeScriptParameterList(auto_ptr_vector<TypeScriptParameter>(), new MetadataString(restParameterName)));
}

TypeScriptParameterList::TypeScriptParameterList(auto_ptr_vector<TypeScriptParameter>&& parameters, MetadataString* restParameterName) :
    m_parameters(parameters),
    m_restParameterName(restParameterName)
{
    parameters.clear();
}

const auto_ptr_vector<TypeScriptParameterList>& TypeScriptConstructorOverloads::GetOverloadParameterLists() const
{
    return m_overloadParameterLists;
}

auto_ptr_vector<TypeScriptParameterList>&& TypeScriptConstructorOverloads::MoveOverloadParameterLists()
{
    return move(m_overloadParameterLists);
}

auto_ptr<TypeScriptConstructorOverloads> TypeScriptConstructorOverloads::Make()
{
    return auto_ptr<TypeScriptConstructorOverloads>(new TypeScriptConstructorOverloads(auto_ptr_vector<TypeScriptParameterList>()));
}

auto_ptr<TypeScriptConstructorOverloads> TypeScriptConstructorOverloads::Make(auto_ptr_vector<TypeScriptParameterList>&& overloadParameterLists)
{
    return auto_ptr<TypeScriptConstructorOverloads>(new TypeScriptConstructorOverloads(move(overloadParameterLists)));
}

TypeScriptConstructorOverloads::TypeScriptConstructorOverloads(auto_ptr_vector<TypeScriptParameterList>&& overloadParameterLists) :
    m_overloadParameterLists(move(overloadParameterLists))
{
    overloadParameterLists.clear();
}

const TypeScriptParameterList& TypeScriptCallSignature::GetParameters() const
{
    return *m_parameters;
}

const TypeScriptType& TypeScriptCallSignature::GetReturnType() const
{
    return *m_returntype;
}

auto_ptr<TypeScriptCallSignature> TypeScriptCallSignature::Make(auto_ptr<TypeScriptParameterList>&& parameters, auto_ptr<TypeScriptType>&& returnType)
{
    return auto_ptr<TypeScriptCallSignature>(new TypeScriptCallSignature(move(parameters), move(returnType)));
}

TypeScriptCallSignature::TypeScriptCallSignature(auto_ptr<TypeScriptParameterList>&& parameters, auto_ptr<TypeScriptType>&& returnType) :
    m_parameters(parameters.release()),
    m_returntype(returnType.release())
{
}

const MetadataString& TypeScriptMethodSignature::GetMethodName() const
{
    return m_methodName;
}

const TypeScriptCallSignature& TypeScriptMethodSignature::GetCallSignature() const
{
    return *m_callSignature;
}

const TypeScriptType* TypeScriptMethodSignature::GetOriginalReturnTypeCommentOrNull() const
{
    return m_originalReturnTypeComment.get();
}

auto_ptr<TypeScriptMethodSignature> TypeScriptMethodSignature::Make(MetadataString methodName, auto_ptr<TypeScriptParameterList>&& parameters, auto_ptr<TypeScriptType>&& returnType)
{
    return auto_ptr<TypeScriptMethodSignature>(new TypeScriptMethodSignature(methodName, move(parameters), move(returnType), auto_ptr<TypeScriptType>()));
}

auto_ptr<TypeScriptMethodSignature> TypeScriptMethodSignature::Make(MetadataString methodName, auto_ptr<TypeScriptParameterList>&& parameters, auto_ptr<TypeScriptType>&& returnType, auto_ptr<TypeScriptType>&& originalReturnTypeComment)
{
    return auto_ptr<TypeScriptMethodSignature>(new TypeScriptMethodSignature(methodName, move(parameters), move(returnType), move(originalReturnTypeComment)));
}

TypeScriptMethodSignature::TypeScriptMethodSignature(MetadataString methodName, auto_ptr<TypeScriptParameterList>&& parameters, auto_ptr<TypeScriptType>&& returnType, auto_ptr<TypeScriptType>&& originalReturnTypeComment) :
    m_methodName(methodName),
    m_callSignature(TypeScriptCallSignature::Make(move(parameters), move(returnType))),
    m_originalReturnTypeComment(originalReturnTypeComment.release())
{
}

const auto_ptr_vector<TypeScriptType>& TypeScriptTypeList::GetTypes() const
{
    return m_types;
}

auto_ptr<TypeScriptTypeList> TypeScriptTypeList::Make(auto_ptr_vector<TypeScriptType>&& types)
{
    return auto_ptr<TypeScriptTypeList>(new TypeScriptTypeList(move(types)));
}

TypeScriptTypeList::TypeScriptTypeList(auto_ptr_vector<TypeScriptType>&& types) :
    m_types(types)
{
    types.clear();
}

const auto_ptr_vector<TypeScriptPropertySignature>& TypeScriptTypeMemberList::GetPropertySignatures() const
{
    return m_propertySignatures;
}

const auto_ptr_vector<TypeScriptMethodSignature>& TypeScriptTypeMemberList::GetMethodSignatures() const
{
    return m_methodSignatures;
}

const auto_ptr_vector<TypeScriptCallSignature>& TypeScriptTypeMemberList::GetCallSignatures() const
{
    return m_callSignatures;
}

const auto_ptr_vector<TypeScriptIndexSignature>& TypeScriptTypeMemberList::GetIndexSignatures() const
{
    return m_indexSignatures;
}

auto_ptr<TypeScriptTypeMemberList> TypeScriptTypeMemberList::Make()
{
    return auto_ptr<TypeScriptTypeMemberList>(new TypeScriptTypeMemberList());
}

void TypeScriptTypeMemberList::AddMembers(auto_ptr<TypeScriptTypeMemberList>&& members)
{
    m_propertySignatures.insert(move(members->m_propertySignatures));
    m_methodSignatures.insert(move(members->m_methodSignatures));
    m_callSignatures.insert(move(members->m_callSignatures));
    m_indexSignatures.insert(move(members->m_indexSignatures));
}

void TypeScriptTypeMemberList::AddMember(auto_ptr<TypeScriptPropertySignature>&& member)
{
    m_propertySignatures.push_back(member.release());
}

void TypeScriptTypeMemberList::AddMember(auto_ptr<TypeScriptMethodSignature>&& member)
{
    m_methodSignatures.push_back(member.release());
}

void TypeScriptTypeMemberList::AddMember(auto_ptr<TypeScriptCallSignature>&& member)
{
    m_callSignatures.push_back(member.release());
}

void TypeScriptTypeMemberList::AddMember(auto_ptr<TypeScriptIndexSignature>&& member)
{
    m_indexSignatures.push_back(member.release());
}

const MetadataString& TypeScriptEnumDeclaration::GetIdentifier() const
{
    return m_identifier;
}

const vector<MetadataString>& TypeScriptEnumDeclaration::GetMemberNames() const
{
    return m_memberNames;
}

auto_ptr<TypeScriptEnumDeclaration> TypeScriptEnumDeclaration::Make(MetadataString identifier)
{
    return auto_ptr<TypeScriptEnumDeclaration>(new TypeScriptEnumDeclaration(identifier));
}

void TypeScriptEnumDeclaration::AppendMember(MetadataString memberName)
{
    m_memberNames.push_back(memberName);
}

TypeScriptEnumDeclaration::TypeScriptEnumDeclaration(MetadataString identifier) :
    m_identifier(identifier)
{
}

const TypeScriptType& TypeScriptInterfaceDeclaration::GetTypeReference() const
{
    return *m_typeReference;
}

const TypeScriptTypeList& TypeScriptInterfaceDeclaration::GetExtends() const
{
    return *m_extends;
}

const TypeScriptTypeMemberList& TypeScriptInterfaceDeclaration::GetMembers() const
{
    return *m_members;
}

auto_ptr<TypeScriptInterfaceDeclaration> TypeScriptInterfaceDeclaration::Make(MetadataString identifier)
{
    auto object = auto_ptr<TypeScriptInterfaceDeclaration>(new TypeScriptInterfaceDeclaration());
    object->m_typeReference = TypeScriptType::Make(identifier);
    return object;
}

auto_ptr<TypeScriptInterfaceDeclaration> TypeScriptInterfaceDeclaration::Make(auto_ptr<TypeScriptType>&& typeReference)
{
    auto object = auto_ptr<TypeScriptInterfaceDeclaration>(new TypeScriptInterfaceDeclaration());
    object->m_typeReference = typeReference;
    return object;
}

auto_ptr<TypeScriptInterfaceDeclaration> TypeScriptInterfaceDeclaration::Make(auto_ptr<TypeScriptType>&& typeReference, auto_ptr<TypeScriptTypeList>&& extends, auto_ptr<TypeScriptTypeMemberList>&& members)
{
    auto object = auto_ptr<TypeScriptInterfaceDeclaration>(new TypeScriptInterfaceDeclaration());
    object->m_typeReference = typeReference;
    object->m_extends = extends;
    object->m_members = members;
    return object;
}

TypeScriptInterfaceDeclaration::TypeScriptInterfaceDeclaration() :
    m_members(TypeScriptTypeMemberList::Make()),
    m_extends(TypeScriptTypeList::Make(auto_ptr_vector<TypeScriptType>()))
{
}

const TypeScriptType& TypeScriptClassDeclaration::GetTypeReference() const
{
    return *m_typeReference;
}

bool TypeScriptClassDeclaration::IsAbstract() const
{
    return m_isAbstract;
}

const TypeScriptTypeList& TypeScriptClassDeclaration::GetExtends() const
{
    return *m_extends;
}

const TypeScriptTypeList& TypeScriptClassDeclaration::GetImplements() const
{
    return *m_implements;
}

const TypeScriptConstructorOverloads& TypeScriptClassDeclaration::GetConstructorOverloads() const
{
    return *m_constructorOverloads;
}

const TypeScriptTypeMemberList& TypeScriptClassDeclaration::GetStaticMembers() const
{
    return *m_staticMembers;
}

const TypeScriptTypeMemberList& TypeScriptClassDeclaration::GetInstanceMembers() const
{
    return *m_instanceMembers;
}

auto_ptr<TypeScriptClassDeclaration> TypeScriptClassDeclaration::Make(
    auto_ptr<TypeScriptType>&& typeReference,
    bool isAbstract,
    auto_ptr<TypeScriptTypeList>&& extends,
    auto_ptr<TypeScriptTypeList>&& implements,
    auto_ptr<TypeScriptConstructorOverloads>&& constructorOverloads,
    auto_ptr<TypeScriptTypeMemberList>&& staticMembers,
    auto_ptr<TypeScriptTypeMemberList>&& instanceMembers
    )
{
    auto object = auto_ptr<TypeScriptClassDeclaration>(new TypeScriptClassDeclaration());
    object->m_typeReference = typeReference;
    object->m_isAbstract = isAbstract;
    object->m_extends = extends;
    object->m_implements = implements;
    object->m_constructorOverloads = constructorOverloads;
    object->m_staticMembers = staticMembers;
    object->m_instanceMembers = instanceMembers;

    return object;
}