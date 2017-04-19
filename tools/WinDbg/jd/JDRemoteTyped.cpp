//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "JDRemoteTyped.h"
#include "JDTypeCache.h"

JDRemoteTyped::JDRemoteTyped(ExtRemoteTyped const& remoteTyped)
    : ExtRemoteTyped(remoteTyped)
{
}

JDRemoteTyped::JDRemoteTyped(ULONG64 modBase, ULONG typeID, ULONG64 offset, bool ptrTo)
{
    Set(ptrTo, modBase, typeID, offset);
}

JDRemoteTyped::JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo)
    : ExtRemoteTyped(Type, Offset, PtrTo)
{
}

JDRemoteTyped::JDRemoteTyped(PCSTR Expr, ULONG64 Offset)
    : ExtRemoteTyped(Expr, Offset)
{

}

bool JDRemoteTyped::HasField(PCSTR field)
{
    return FieldInfoCache::HasField(*this, field);
}

JDRemoteTyped JDRemoteTyped::Field(PCSTR field)
{
    return FieldInfoCache::GetField(*this, field);
}

JDRemoteTyped JDRemoteTyped::ArrayElement(LONG64 index)
{
    return __super::ArrayElement(index);
}

JDRemoteTyped JDRemoteTyped::Cast(char const * typeName)
{
    return JDRemoteTyped::FromPtrWithType(this->GetPtr(), typeName);
}

JDRemoteTyped JDRemoteTyped::CastWithVtable(char const ** typeName)
{
    ULONG64 pointer;
    if (this->m_Typed.Tag != SymTagPointerType)
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