#include "stdafx.h"
#include "JDRemoteTyped.h"
//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

JDRemoteTyped::JDRemoteTyped(ExtRemoteTyped remoteTyped)
    : ExtRemoteTyped(remoteTyped)
{
}

JDRemoteTyped::JDRemoteTyped(ULONG64 modBase, ULONG typeID, ULONG64 offset)
{
    Set(false, modBase, typeID, offset);
}

JDRemoteTyped::JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo)
    : ExtRemoteTyped(Type, Offset, PtrTo)
{
}

JDRemoteTyped::JDRemoteTyped(PCSTR Expr, ULONG64 Offset)
    : ExtRemoteTyped(Expr, Offset)
{

}

JDRemoteTyped JDRemoteTyped::Field(PCSTR field) const
{
    return FieldInfoCache::GetField(*this, field);
}
