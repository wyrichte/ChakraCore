//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "TypeScriptElements.h"

class IndentingWriter
{
public:
    IndentingWriter(std::wostream& stream);

    void WriteLine(const std::wstring& line);

    void Indent();
    void Unindent();

private:
    IndentingWriter(std::wostream& stream, int indent);

private:
    std::wostream& m_stream;
    int m_indent;
};

class TypeScriptEmitter
{
public:
    TypeScriptEmitter(IndentingWriter& writer, Metadata::IStringConverter& converter);

    void OpenAmbientNamespace(const MetadataString namespaceName);
    void OpenNamespace(const MetadataString namespaceName);
    void CloseNamespace();

    void EmitInterfaceDeclaration(const TypeScriptInterfaceDeclaration& interfaceDeclaration);
    void EmitEnumDeclaration(const TypeScriptEnumDeclaration& enumDeclaration);
    void EmitClassDeclaration(const TypeScriptClassDeclaration& classDeclaration);

    std::wstring FormatTypeName(const MetadataString name);
    std::wstring FormatIdentifier(const MetadataString name);
    std::wstring FormatPropertyName(const MetadataString name);
    std::wstring FormatType(const TypeScriptType& type);
    std::wstring FormatParameterList(const TypeScriptParameterList& parameters);
    std::wstring FormatPropertyDeclaration(bool isStatic, const TypeScriptPropertySignature& signature);
    std::wstring FormatMethodDeclaration(bool isStatic, const TypeScriptMethodSignature& method);
    std::wstring FormatConstructorDeclaration(const TypeScriptParameterList& parameters);
    std::wstring FormatIndexDeclaration(const TypeScriptIndexSignature& indexSignature);
    std::wstring FormatCallSignature(const TypeScriptCallSignature& callSignature);
    std::wstring FormatExtendsClause(const TypeScriptTypeList& types);
    std::wstring FormatImplementsClause(const TypeScriptTypeList& types);
    std::wstring FormatStringLiteral(const MetadataString name);

private:
    IndentingWriter& m_writer;
    Metadata::IStringConverter& m_converter;
    std::vector<std::wstring> m_disallowedIdentifierNames;
};