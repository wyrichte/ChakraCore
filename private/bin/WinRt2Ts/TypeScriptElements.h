//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "MetadataString.h"
#include "XmlDocReference.h"

template <typename T>
class auto_ptr_vector : public std::vector<const T*>
{
public:
    void insert(auto_ptr_vector<T>&& otherVector)
    {
        vector::insert(this->end(), otherVector.begin(), otherVector.end());
        otherVector.clear();
    }

    void push_back(std::auto_ptr<T>&& elem)
    {
        vector::push_back(elem.release());
    }

    void push_back(const T* elem)
    {
        vector::push_back(elem);
    }

    ~auto_ptr_vector()
    {
        for (auto element : *this)
        {
            delete element;
        }
    }
};

template <typename T>
std::auto_ptr<T>&& move(std::auto_ptr<T>& lValue)
{
    return static_cast<auto_ptr<T>&&>(lValue);
}

template <typename T>
auto_ptr_vector<T>&& move(auto_ptr_vector<T>& lValue)
{
    return static_cast<auto_ptr_vector<T>&&>(lValue);
}

class TypeScriptType;

class Documentable
{
public:
    const XmlDocReference& GetDocumentation() const
    {
        return m_documentation;
    }

    void AttachDocumentation(XmlDocReference doc)
    {
        m_documentation = doc;
    }

private:
    XmlDocReference m_documentation;
};

class TypeScriptPropertySignature : public Documentable
{
public:
    const MetadataString& GetName() const;
    const TypeScriptType& GetType() const;
    bool IsOptional() const;

    static std::auto_ptr<TypeScriptPropertySignature> Make(MetadataString name, std::auto_ptr<TypeScriptType>&& type, bool optional = false);

private:
    const MetadataString m_name;
    const std::auto_ptr<TypeScriptType> m_type;
    const bool m_optional;

private:
    TypeScriptPropertySignature(MetadataString name, std::auto_ptr<TypeScriptType> type, bool optional);
};

class TypeScriptIndexSignature
{
public:
    const TypeScriptType& GetIndexType() const;
    const TypeScriptType& GetType() const;

    static std::auto_ptr<TypeScriptIndexSignature> Make(std::auto_ptr<TypeScriptType>&& indexType, std::auto_ptr<TypeScriptType>&& type);

private:
    const std::auto_ptr<TypeScriptType> m_indexType;
    const std::auto_ptr<TypeScriptType> m_type;

private:
    TypeScriptIndexSignature(std::auto_ptr<TypeScriptType>&& indexType, std::auto_ptr<TypeScriptType>&& type);
};

class TypeScriptParameter
{
public:
    const MetadataString& GetName() const;
    const MetadataString* GetStringLiteralOrNull() const;
    const TypeScriptType* GetTypeOrNull() const;

    static std::auto_ptr<TypeScriptParameter> Make(MetadataString name, std::auto_ptr<TypeScriptType>&& type);
    static std::auto_ptr<TypeScriptParameter> Make(MetadataString name, MetadataString literal);

private:
    const MetadataString m_name;
    const std::auto_ptr<MetadataString> m_literal;
    const std::auto_ptr<TypeScriptType> m_type;

private:
    TypeScriptParameter(MetadataString name, MetadataString* literal, TypeScriptType* type);
};

class TypeScriptParameterList : public Documentable
{
public:
    const auto_ptr_vector<TypeScriptParameter>& GetParameters() const;
    const MetadataString* GetRestParameterNameOrNull() const;

    static std::auto_ptr<TypeScriptParameterList> Make(auto_ptr_vector<TypeScriptParameter>&& parameters);
    static std::auto_ptr<TypeScriptParameterList> Make(MetadataString restParameterName);

private:
    const auto_ptr_vector<TypeScriptParameter> m_parameters;
    const std::auto_ptr<MetadataString> m_restParameterName;

private:
    TypeScriptParameterList(auto_ptr_vector<TypeScriptParameter>&& parameters, MetadataString* restParameterName);
};

class TypeScriptConstructorOverloads
{
public:
    const auto_ptr_vector<TypeScriptParameterList>& GetOverloadParameterLists() const;

    auto_ptr_vector<TypeScriptParameterList>&& MoveOverloadParameterLists();

    static std::auto_ptr<TypeScriptConstructorOverloads> Make();
    static std::auto_ptr<TypeScriptConstructorOverloads> Make(auto_ptr_vector<TypeScriptParameterList>&& overloadParameterLists);

private:
    auto_ptr_vector<TypeScriptParameterList> m_overloadParameterLists;

private:
    TypeScriptConstructorOverloads(auto_ptr_vector<TypeScriptParameterList>&& overloadParameterLists);
};

class TypeScriptCallSignature : public Documentable
{
public:
    const TypeScriptParameterList& GetParameters() const;
    const TypeScriptType& GetReturnType() const;

    static std::auto_ptr<TypeScriptCallSignature> Make(std::auto_ptr<TypeScriptParameterList>&& parameters, std::auto_ptr<TypeScriptType>&& returnType);

private:
    const std::auto_ptr<TypeScriptParameterList> m_parameters;
    const std::auto_ptr<TypeScriptType> m_returntype;

private:
    TypeScriptCallSignature(std::auto_ptr<TypeScriptParameterList>&& parameters, std::auto_ptr<TypeScriptType>&& returnType);
};

class TypeScriptMethodSignature : public Documentable
{
public:
    const MetadataString& GetMethodName() const;
    const TypeScriptCallSignature& GetCallSignature() const;
    const TypeScriptType* GetOriginalReturnTypeCommentOrNull() const;
    bool IsOptional() const;

    static std::auto_ptr<TypeScriptMethodSignature> Make(MetadataString methodName, std::auto_ptr<TypeScriptParameterList>&& parameters, std::auto_ptr<TypeScriptType>&& returnType, bool optional);
    static std::auto_ptr<TypeScriptMethodSignature> Make(MetadataString methodName, std::auto_ptr<TypeScriptParameterList>&& parameters, std::auto_ptr<TypeScriptType>&& returnType, std::auto_ptr<TypeScriptType>&& originalReturnTypeComment);

private:
    const MetadataString m_methodName;
    const bool m_optional;
    const std::auto_ptr<TypeScriptCallSignature> m_callSignature;
    const std::auto_ptr<TypeScriptType> m_originalReturnTypeComment;

private:
    TypeScriptMethodSignature(MetadataString methodName, std::auto_ptr<TypeScriptParameterList>&& parameters, std::auto_ptr<TypeScriptType>&& returnType, std::auto_ptr<TypeScriptType>&& originalReturnTypeComment, bool optional);
};

class TypeScriptTypeList
{
public:
    const auto_ptr_vector<TypeScriptType>& GetTypes() const;

    static std::auto_ptr<TypeScriptTypeList> Make(auto_ptr_vector<TypeScriptType>&& types);

private:
    const auto_ptr_vector<TypeScriptType> m_types;

private:
    TypeScriptTypeList(auto_ptr_vector<TypeScriptType>&& types);
};

class TypeScriptTypeMemberList
{
public:
    const auto_ptr_vector<TypeScriptPropertySignature>& GetPropertySignatures() const;
    const auto_ptr_vector<TypeScriptMethodSignature>& GetMethodSignatures() const;
    const auto_ptr_vector<TypeScriptCallSignature>& GetCallSignatures() const;
    const auto_ptr_vector<TypeScriptIndexSignature>& GetIndexSignatures() const;

    static std::auto_ptr<TypeScriptTypeMemberList> Make();

    void AddMembers(std::auto_ptr<TypeScriptTypeMemberList>&& members);
    void AddMember(std::auto_ptr<TypeScriptPropertySignature>&& member);
    void AddMember(std::auto_ptr<TypeScriptMethodSignature>&& member);
    void AddMember(std::auto_ptr<TypeScriptCallSignature>&& member);
    void AddMember(std::auto_ptr<TypeScriptIndexSignature>&& member);
    bool ContainsMember(const TypeScriptPropertySignature& member);
    bool ContainsMember(const TypeScriptMethodSignature& member);

private:
    auto_ptr_vector<TypeScriptPropertySignature> m_propertySignatures;
    auto_ptr_vector<TypeScriptMethodSignature> m_methodSignatures;
    auto_ptr_vector<TypeScriptCallSignature> m_callSignatures;
    auto_ptr_vector<TypeScriptIndexSignature> m_indexSignatures;

private:
    TypeScriptTypeMemberList() {}
};

class TypeScriptEnumMember : public Documentable
{
public:
    const MetadataString& GetIdentifier() const;

    static std::auto_ptr<TypeScriptEnumMember> Make(MetadataString memberName);

private: 
    const MetadataString m_identifier;

private:
    TypeScriptEnumMember(MetadataString memberName);
};

class TypeScriptEnumDeclaration : public Documentable
{
public:
    const MetadataString& GetIdentifier() const;
    const auto_ptr_vector<TypeScriptEnumMember>& GetMembers() const;

    void AppendMember(MetadataString memberName, XmlDocReference doc);

    static std::auto_ptr<TypeScriptEnumDeclaration> Make(MetadataString identifier);

private:
    const MetadataString m_identifier;
    auto_ptr_vector<TypeScriptEnumMember> m_members;

private:
    TypeScriptEnumDeclaration(MetadataString identifier);
};

class TypeScriptInterfaceDeclaration : public Documentable
{
public:
    const TypeScriptType& GetTypeReference() const;
    const TypeScriptTypeList& GetExtends() const;
    const TypeScriptTypeMemberList& GetMembers() const;

    static std::auto_ptr<TypeScriptInterfaceDeclaration> Make(MetadataString identifier);
    static std::auto_ptr<TypeScriptInterfaceDeclaration> Make(std::auto_ptr<TypeScriptType>&& typeReference);
    static std::auto_ptr<TypeScriptInterfaceDeclaration> Make(std::auto_ptr<TypeScriptType>&& typeReference, std::auto_ptr<TypeScriptTypeList>&& extends, std::auto_ptr<TypeScriptTypeMemberList>&& members);

    template <class T>
    void AddMember(T&& member)
    {
        m_members->AddMember(move(member));
    }

private:
    std::auto_ptr<TypeScriptType> m_typeReference;
    std::auto_ptr<TypeScriptTypeList> m_extends;
    std::auto_ptr<TypeScriptTypeMemberList> m_members;

private:
    TypeScriptInterfaceDeclaration();
};

class TypeScriptClassDeclaration : public Documentable
{
public:
    const TypeScriptType& GetTypeReference() const;
    bool IsAbstract() const;
    const TypeScriptTypeList& GetExtends() const;
    const TypeScriptTypeList& GetImplements() const;
    const TypeScriptConstructorOverloads& GetConstructorOverloads() const;
    const TypeScriptTypeMemberList& GetStaticMembers() const;
    const TypeScriptTypeMemberList& GetInstanceMembers() const;

    static std::auto_ptr<TypeScriptClassDeclaration> Make(
        std::auto_ptr<TypeScriptType>&& typeReference,
        bool isAbstract,
        std::auto_ptr<TypeScriptTypeList>&& extends,
        std::auto_ptr<TypeScriptTypeList>&& implements,
        std::auto_ptr<TypeScriptConstructorOverloads>&& constructorOverloads,
        std::auto_ptr<TypeScriptTypeMemberList>&& staticMembers,
        std::auto_ptr<TypeScriptTypeMemberList>&& instanceMembers
        );

private:
    std::auto_ptr<TypeScriptType> m_typeReference;
    bool m_isAbstract;
    std::auto_ptr<TypeScriptTypeList> m_extends;
    std::auto_ptr<TypeScriptTypeList> m_implements;
    std::auto_ptr<TypeScriptConstructorOverloads> m_constructorOverloads;
    std::auto_ptr<TypeScriptTypeMemberList> m_staticMembers;
    std::auto_ptr<TypeScriptTypeMemberList> m_instanceMembers;

private:
    TypeScriptClassDeclaration() {}
};

class TypeScriptTypeAliasDeclaration
{
public:
    const TypeScriptType& GetTypeReference() const;
    const TypeScriptType& GetRightHandTypeReference() const;

    static std::auto_ptr<TypeScriptTypeAliasDeclaration> Make(std::auto_ptr<TypeScriptType>&& typeReference, std::auto_ptr<TypeScriptType>&& rightHandTypeReference);

private:
    const std::auto_ptr<TypeScriptType> m_typeReference;
    const std::auto_ptr<TypeScriptType> m_rightHandTypeReference;

private:
    TypeScriptTypeAliasDeclaration(std::auto_ptr<TypeScriptType>&& typeReference, std::auto_ptr<TypeScriptType>&& rightHandTypeReference);
};

class TypeScriptType
{
public:
    const MetadataString& GetName() const;
    bool IsArray() const;
    bool IsObject() const;
    bool IsFunction() const;
    bool IsTuple() const;
    const auto_ptr_vector<TypeScriptType>& GetTypeArguments() const;
    const auto_ptr_vector<TypeScriptPropertySignature>& GetObjectTypeMembers() const;
    const TypeScriptParameterList& GetFunctionParameters() const;
    const TypeScriptType& GetFunctionReturnType() const;
    const TypeScriptType* GetIntersectionTypeOrNull() const;

    static std::auto_ptr<TypeScriptType> Make(MetadataString name);
    static std::auto_ptr<TypeScriptType> MakeTypeReference(MetadataString name, auto_ptr_vector<TypeScriptType>&& typeArguments);
    static std::auto_ptr<TypeScriptType> MakeTypeReference(MetadataString name, std::auto_ptr<TypeScriptType>&& typeArgument);
    static std::auto_ptr<TypeScriptType> MakeObjectType(auto_ptr_vector<TypeScriptPropertySignature>&& objectTypeMembers);
    static std::auto_ptr<TypeScriptType> MakeArrayType(std::auto_ptr<TypeScriptType>&& type);
    static std::auto_ptr<TypeScriptType> RenameType(std::auto_ptr<TypeScriptType>&& type, MetadataString newName);
    static std::auto_ptr<TypeScriptType> IntersectTypes(std::auto_ptr<TypeScriptType>&& type, std::auto_ptr<TypeScriptType>&& intersectionType);
    static std::auto_ptr<TypeScriptType> MakeFunctionType(std::auto_ptr<TypeScriptParameterList>&& parameters, std::auto_ptr<TypeScriptType>&& returnType);
    static std::auto_ptr<TypeScriptType> MakeTupleType(auto_ptr_vector<TypeScriptType>&& typeArguments);

private:
    enum class Type
    {
        PredefinedOrReference,
        Array,
        Object,
        Function,
        Tuple
    };

    const MetadataString m_name;
    const Type m_type;
    const std::auto_ptr<TypeScriptParameterList> m_functionParameters;
    const std::auto_ptr<TypeScriptType> m_functionReturnType;
    const auto_ptr_vector<TypeScriptType> m_typeArguments;
    const auto_ptr_vector<TypeScriptPropertySignature> m_objectTypeMembers;
    std::auto_ptr<TypeScriptType> m_intersection;

private:
    TypeScriptType(MetadataString name, Type type, auto_ptr_vector<TypeScriptType>&& typeArguments, auto_ptr_vector<TypeScriptPropertySignature>&& objectTypeMembers, std::auto_ptr<TypeScriptParameterList>&& functionParameterList, std::auto_ptr<TypeScriptType>&& returnType);
};