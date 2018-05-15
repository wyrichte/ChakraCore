//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

JDTypeInfo::JDTypeInfo()
{
    Clear();
}
JDTypeInfo::JDTypeInfo(ULONG64 modBase, ULONG typeId, ULONG size, bool isPointerType)
    : m_ModBase(modBase), m_TypeId(typeId), m_size(size), m_isPointerType(isPointerType), m_isValid(true)
{

}

void JDTypeInfo::Set(ExtRemoteTyped const& remoteTyped)
{
    m_isValid = true;
    m_ModBase = remoteTyped.m_Typed.ModBase;
    m_TypeId = remoteTyped.m_Typed.TypeId;
    m_size = remoteTyped.m_Typed.Size;
    m_isPointerType = (remoteTyped.m_Typed.Tag == SymTagPointerType);
}

void JDTypeInfo::Clear()
{
    m_isValid = false;
    m_ModBase = 0;
    m_TypeId = 0;
    m_size = 0;
    m_isPointerType = false;
}
bool JDTypeInfo::IsValid() const
{
    return this->m_isValid;
}

ULONG64 JDTypeInfo::GetModBase() const
{
    return m_ModBase;
}

ULONG JDTypeInfo::GetTypeId() const
{
    return m_TypeId;
}

ULONG JDTypeInfo::GetSize() const
{
    return m_size;
}

bool JDTypeInfo::IsPointerType() const
{
    return m_isPointerType;
}