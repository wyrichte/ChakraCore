//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "JDRemoteTyped.h"
#include "JDTypeCache.h"

JDRemoteTyped::JDRemoteTyped(ExtRemoteTyped const& remoteTyped)
    : extRemoteTyped(remoteTyped)
{
}

JDRemoteTyped::JDRemoteTyped(ULONG64 modBase, ULONG typeID, ULONG64 offset, bool ptrTo)
{
    extRemoteTyped.Set(ptrTo, modBase, typeID, offset);
}

JDRemoteTyped::JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo)
    : extRemoteTyped(Type, Offset, PtrTo)
{
}

JDRemoteTyped::JDRemoteTyped(PCSTR Expr, ULONG64 Offset)
    : extRemoteTyped(Expr, Offset)
{
}

bool JDRemoteTyped::HasField(PCSTR field)
{
    return FieldInfoCache::HasField(GetExtRemoteTyped(), field);
}

JDRemoteTyped JDRemoteTyped::Field(PCSTR field)
{
    return FieldInfoCache::GetField(GetExtRemoteTyped(), field);
}

JDRemoteTyped JDRemoteTyped::BitField(PCSTR field)
{
    // Don't cache bit field accesses
    return GetExtRemoteTyped().Field(field);
}

JDRemoteTyped JDRemoteTyped::ArrayElement(LONG64 index)
{
    return GetExtRemoteTyped().ArrayElement(index);
}

JDRemoteTyped JDRemoteTyped::Cast(char const * typeName)
{
    return JDRemoteTyped::FromPtrWithType(this->GetPtr(), typeName);
}

JDRemoteTyped JDRemoteTyped::CastWithVtable(char const ** typeName)
{
    ULONG64 pointer;
    if (extRemoteTyped.m_Typed.Tag != SymTagPointerType)
    {
        pointer = this->GetPointerTo().GetPtr();
    }
    else
    {
        pointer = this->GetPtr();
    }

    JDRemoteTyped result;
    if (pointer && JDTypeCache::CastWithVtable(pointer, result, typeName))
    {
        return result;
    }
    return *this;
}

JDRemoteTyped JDRemoteTyped::FromPtrWithType(ULONG64 address, char const * typeName)
{
    return JDTypeCache::Cast(typeName, address);
}

JDRemoteTyped JDRemoteTyped::FromPtrWithVtable(ULONG64 address, char const ** typeName)
{
    JDRemoteTyped result;
    if (!JDTypeCache::CastWithVtable(address, result, typeName))
    {
        result = JDRemoteTyped("(void *)@$extin", address);
    }
    return result;
}

ULONG64 JDRemoteTyped::GetSizeT()
{
    return ExtRemoteTypedUtil::GetSizeT(GetExtRemoteTyped());
}

char const * JDRemoteTyped::GetEnumString()
{
    return JDUtil::GetEnumString(GetExtRemoteTyped());
}

ExtRemoteTyped& JDRemoteTyped::GetExtRemoteTyped()
{
    return extRemoteTyped;
}

ExtRemoteTyped JDRemoteTyped::Dereference()
{
    return GetExtRemoteTyped().Dereference();
}

ExtRemoteTyped JDRemoteTyped::GetPointerTo()
{
    return GetExtRemoteTyped().GetPointerTo();
}

ExtRemoteTyped JDRemoteTyped::operator[](_In_ LONG Index)
{
    return GetExtRemoteTyped()[Index];
}

ExtRemoteTyped JDRemoteTyped::operator[](_In_ ULONG Index)
{
    return GetExtRemoteTyped()[Index];
}

ExtRemoteTyped JDRemoteTyped::operator[](_In_ LONG64 Index)
{
    return GetExtRemoteTyped()[Index];
}

ExtRemoteTyped JDRemoteTyped::operator[](_In_ ULONG64 Index)
{
    return GetExtRemoteTyped()[Index];
}

char const * JDRemoteTyped::GetTypeName()
{
    return GetExtRemoteTyped().GetTypeName();
}

char const * JDRemoteTyped::GetSimpleValue()
{
    return GetExtRemoteTyped().GetSimpleValue();
}

ULONG JDRemoteTyped::GetTypeSize()
{
    return GetExtRemoteTyped().GetTypeSize();
}

BOOL JDRemoteTyped::GetW32Bool()
{
    return GetExtRemoteTyped().GetW32Bool();
}

bool JDRemoteTyped::GetStdBool()
{
    return GetExtRemoteTyped().GetStdBool();
}

CHAR JDRemoteTyped::GetChar()
{
    return GetExtRemoteTyped().GetChar();
}

UCHAR JDRemoteTyped::GetUchar()
{
    return GetExtRemoteTyped().GetUchar();
}

USHORT JDRemoteTyped::GetUshort()
{
    return GetExtRemoteTyped().GetUshort();
}

LONG JDRemoteTyped::GetLong()
{
    return GetExtRemoteTyped().GetLong();
}

ULONG JDRemoteTyped::GetUlong()
{
    return GetExtRemoteTyped().GetUlong();
}

ULONG64 JDRemoteTyped::GetPtr()
{
    return GetExtRemoteTyped().GetPtr();
}