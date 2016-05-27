//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "JDRemoteTyped.h"

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