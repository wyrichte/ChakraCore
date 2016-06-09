//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "Errors.h"

class ProjectionToTypeScriptError : public ErrorBase
{
protected:
    ProjectionToTypeScriptError(unsigned int code) : ErrorBase(code) {}
};

class UnrecognizedTypeError : public ProjectionToTypeScriptError
{
public:
    UnrecognizedTypeError(MetadataString typeName) :
        m_typeName(typeName),
        ProjectionToTypeScriptError(ErrorCodeUnrecognizedType)
    {
    }

    std::wstring Description() override
    {
        return std::wstring(L"Unrecognized type: ") + m_typeName.ToString();
    }

private:
    MetadataString m_typeName;
};

class UnexpectedPropertyError : public ProjectionToTypeScriptError
{
public:
    UnexpectedPropertyError(MetadataString propertyName, ProjectionModel::PropertyType type) :
        m_propertyName(propertyName),
        m_propertyType(type),
        ProjectionToTypeScriptError(ErrorCodeUnexpectedProperty)
    {
    }

    std::wstring Description() override
    {
        wchar_t formattedIntBuf[100];
        swprintf_s(formattedIntBuf, L"%d", m_propertyType);
        return std::wstring(L"Unexpected type of property: ") + m_propertyName.ToString() + L" type: " + formattedIntBuf;
    }

private:
    MetadataString m_propertyName;
    ProjectionModel::PropertyType m_propertyType;
};