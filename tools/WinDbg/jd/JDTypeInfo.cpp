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

JDTypeInfo JDTypeInfo::GetVoidPointerType()
{
    return JDTypeInfo(0, 0, g_Ext->m_PtrSize, true);
}

JDTypeInfo JDTypeInfo::FromExtRemoteTyped(ExtRemoteTyped const& remoteTyped)
{
    return JDTypeInfo(remoteTyped.m_Typed.ModBase, remoteTyped.m_Typed.TypeId, remoteTyped.m_Typed.Size, (remoteTyped.m_Typed.Tag == SymTagPointerType));
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