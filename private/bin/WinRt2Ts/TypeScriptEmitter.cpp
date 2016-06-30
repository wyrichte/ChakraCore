//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "TypeScriptEmitter.h"

using namespace Metadata;
using namespace std;

IndentingWriter::IndentingWriter(wostream& stream) :
    m_stream(stream),
    m_indent(0)
{
}

void IndentingWriter::WriteLine(const wstring& line)
{
    for (int i = 0; i < m_indent; i++)
    {
        m_stream << L"    ";
    }
    m_stream << line << endl;
}

void IndentingWriter::Indent()
{
    m_indent++;
}

void IndentingWriter::Unindent()
{
    m_indent--;
}

IndentingWriter::IndentingWriter(wostream& stream, int indent) :
    m_indent(indent),
    m_stream(stream)
{
}

TypeScriptEmitter::TypeScriptEmitter(IndentingWriter& writer, IStringConverter& converter) :
    m_converter(converter),
    m_writer(writer)
{
    m_disallowedIdentifierNames.push_back(L"arguments");
    m_disallowedIdentifierNames.push_back(L"function");
}

void TypeScriptEmitter::OpenAmbientNamespace(const MetadataString namespaceName)
{
    m_writer.WriteLine(wstring(L"declare namespace ") + FormatIdentifier(namespaceName) + L" {");
    m_writer.Indent();
}

void TypeScriptEmitter::OpenNamespace(const MetadataString namespaceName)
{
    m_writer.WriteLine(wstring(L"namespace ") + FormatIdentifier(namespaceName) + L" {");
    m_writer.Indent();
}

void TypeScriptEmitter::CloseNamespace()
{
    m_writer.Unindent();
    m_writer.WriteLine(L"}");
}

void TypeScriptEmitter::EmitInterfaceDeclaration(const TypeScriptInterfaceDeclaration& interfaceDeclaration)
{
    m_writer.WriteLine(L"interface " + FormatType(interfaceDeclaration.GetTypeReference()) + FormatExtendsClause(interfaceDeclaration.GetExtends()) + L" {");
    m_writer.Indent();

    for (auto member : interfaceDeclaration.GetMembers().GetPropertySignatures())
    {
        m_writer.WriteLine(FormatPropertyDeclaration(false, *member));
    }

    for (auto member : interfaceDeclaration.GetMembers().GetMethodSignatures())
    {
        m_writer.WriteLine(FormatMethodDeclaration(false, *member));
    }

    for (auto member : interfaceDeclaration.GetMembers().GetIndexSignatures())
    {
        m_writer.WriteLine(FormatIndexDeclaration(*member));
    }

    for (auto member : interfaceDeclaration.GetMembers().GetCallSignatures())
    {
        m_writer.WriteLine(FormatCallSignature(*member));
    }

    m_writer.Unindent();
    m_writer.WriteLine(L"}");
}

void TypeScriptEmitter::EmitEnumDeclaration(const TypeScriptEnumDeclaration& enumDeclaration)
{
    m_writer.WriteLine(wstring(L"enum ") + FormatIdentifier(enumDeclaration.GetIdentifier()) + L" {");

    for (auto member : enumDeclaration.GetMemberNames())
    {
        m_writer.Indent();
        m_writer.WriteLine(FormatPropertyName(member) + L",");
        m_writer.Unindent();
    }

    m_writer.WriteLine(L"}");
}

void TypeScriptEmitter::EmitClassDeclaration(const TypeScriptClassDeclaration& classDeclaration)
{
    wstring line = (classDeclaration.IsAbstract() ? wstring(L"abstract ") : L"") + L"class " + FormatType(classDeclaration.GetTypeReference());

    line += FormatExtendsClause(classDeclaration.GetExtends()) + FormatImplementsClause(classDeclaration.GetImplements()) + L" {";

    m_writer.WriteLine(line);

    m_writer.Indent();

    // Constructor overloads
    for (auto overload : classDeclaration.GetConstructorOverloads().GetOverloadParameterLists())
    {
        m_writer.WriteLine(FormatConstructorDeclaration(*overload));
    }

    // Static members
    for (auto member : classDeclaration.GetStaticMembers().GetPropertySignatures())
    {
        m_writer.WriteLine(FormatPropertyDeclaration(true, *member));
    }

    for (auto member : classDeclaration.GetStaticMembers().GetMethodSignatures())
    {
        m_writer.WriteLine(FormatMethodDeclaration(true, *member));
    }

    for (auto member : classDeclaration.GetStaticMembers().GetIndexSignatures())
    {
        m_writer.WriteLine(FormatIndexDeclaration(*member));
    }

    for (auto member : classDeclaration.GetStaticMembers().GetCallSignatures())
    {
        m_writer.WriteLine(FormatCallSignature(*member));
    }

    // Instance members
    for (auto member : classDeclaration.GetInstanceMembers().GetPropertySignatures())
    {
        m_writer.WriteLine(FormatPropertyDeclaration(false, *member));
    }

    for (auto member : classDeclaration.GetInstanceMembers().GetMethodSignatures())
    {
        m_writer.WriteLine(FormatMethodDeclaration(false, *member));
    }

    for (auto member : classDeclaration.GetInstanceMembers().GetIndexSignatures())
    {
        m_writer.WriteLine(FormatIndexDeclaration(*member));
    }

    m_writer.Unindent();

    m_writer.WriteLine(L"}");
}

wstring TypeScriptEmitter::FormatTypeName(const MetadataString name)
{
    return name.ToString();
}

wstring TypeScriptEmitter::FormatIdentifier(const MetadataString name)
{
    auto parameterName = wstring(name.ToString());

    // Disambiguate from reserved JS words if necessary
    if (find(m_disallowedIdentifierNames.begin(), m_disallowedIdentifierNames.end(), parameterName) != m_disallowedIdentifierNames.end())
    {
        parameterName = parameterName + L"_";
    }

    return parameterName;
}

wstring TypeScriptEmitter::FormatPropertyName(const MetadataString name)
{
    if (wcschr(name.ToString(), L'.') != nullptr)
    {
        return FormatStringLiteral(name);
    }
    else
    {
        return name.ToString();
    }
}

wstring TypeScriptEmitter::FormatType(const TypeScriptType& type)
{
    wstring out;
    if (type.IsObject())
    {
        out += L"{ ";
        for (auto objectMember : type.GetObjectTypeMembers())
        {
            out += FormatPropertyDeclaration(false, *objectMember);
            out += L" ";
        }
        out += L"}";
    }
    else if (type.IsFunction())
    {
        out += FormatParameterList(type.GetFunctionParameters());
        out += L" => ";
        out += FormatType(type.GetFunctionReturnType());
    }
    else
    {
        out += FormatTypeName(type.GetName());

        if (!type.GetTypeArguments().empty())
        {
            out += L"<";
            bool firstElement = true;
            for (auto param : type.GetTypeArguments())
            {
                if (firstElement)
                {
                    firstElement = false;
                }
                else
                {
                    out += L", ";
                }

                out += FormatType(*param);
            }

            out += L">";
        }

        if (type.IsArray())
        {
            out += L"[]";
        }
    }

    return out;
}

wstring TypeScriptEmitter::FormatParameterList(const TypeScriptParameterList& parameters)
{
    wstring out(L"(");

    bool firstElement = true;

    for (auto param : parameters.GetParameters())
    {
        if (firstElement)
        {
            firstElement = false;
        }
        else
        {
            out += L", ";
        }

        auto parameterName = FormatIdentifier(param->GetName());

        if (param->GetStringLiteralOrNull() == nullptr)
        {
            out += parameterName + L": " + FormatType(*param->GetTypeOrNull());
        }
        else
        {
            out += parameterName + L": " + FormatStringLiteral(*param->GetStringLiteralOrNull());
        }
    }

    if (parameters.GetRestParameterNameOrNull() != nullptr)
    {
        if (!firstElement)
        {
            out += L", ";
        }

        out += L"...";
        out += parameters.GetRestParameterNameOrNull()->ToString();
    }

    out += L")";

    return out;
}

wstring TypeScriptEmitter::FormatPropertyDeclaration(bool isStatic, const TypeScriptPropertySignature& signature)
{
    return (isStatic ? L"static " : L"") + FormatPropertyName(signature.GetName()) + L": " + FormatType(signature.GetType()) + L";";
}

wstring TypeScriptEmitter::FormatMethodDeclaration(bool isStatic, const TypeScriptMethodSignature& method)
{
    auto outString = (isStatic ? L"static " : L"") + FormatPropertyName(method.GetMethodName()) + FormatCallSignature(method.GetCallSignature()) + L";";

    if (method.GetOriginalReturnTypeCommentOrNull() != nullptr)
    {
        outString += L" /* " + FormatType(*method.GetOriginalReturnTypeCommentOrNull()) + L" */ ";
    }

    return outString;
}

wstring TypeScriptEmitter::FormatConstructorDeclaration(const TypeScriptParameterList& parameters)
{
    return L"constructor" + FormatParameterList(parameters) + L";";
}

wstring TypeScriptEmitter::FormatIndexDeclaration(const TypeScriptIndexSignature& indexSignature)
{
    return L"[index: " + FormatType(indexSignature.GetIndexType()) + L"]: " + FormatType(indexSignature.GetType()) + L";";
}

wstring TypeScriptEmitter::FormatCallSignature(const TypeScriptCallSignature& callSignature)
{
    return FormatParameterList(callSignature.GetParameters()) + L": " + FormatType(callSignature.GetReturnType());
}

wstring TypeScriptEmitter::FormatExtendsClause(const TypeScriptTypeList& types)
{
    wstring out;
    bool firstElement = true;
    for (auto type : types.GetTypes())
    {
        if (firstElement)
        {
            out += L" extends ";
            firstElement = false;
        }
        else
        {
            out += L", ";
        }

        out += FormatType(*type);
    }

    return out;
}

wstring TypeScriptEmitter::FormatImplementsClause(const TypeScriptTypeList& types)
{
    wstring out;
    bool firstElement = true;
    for (auto type : types.GetTypes())
    {
        if (firstElement)
        {
            out += L" implements ";
            firstElement = false;
        }
        else
        {
            out += L", ";
        }

        out += FormatType(*type);
    }

    return out;
}

wstring TypeScriptEmitter::FormatStringLiteral(const MetadataString name)
{
    return wstring(L"\"") + name.ToString() + L"\"";
}