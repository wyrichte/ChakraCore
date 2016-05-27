//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once
#include "stdafx.h"

ExtRemoteTyped ExtRemoteTypedUtil::GetTeb()
{
    return ExtRemoteTyped(g_Ext->Is32On64() ? "((ntdll!_TEB32 *)@$TEB->NtTib.ExceptionList)" : "@$TEB");
}

ULONG64 ExtRemoteTypedUtil::GetAsPointer(ExtRemoteTyped object)
{
#if _M_X64
    if (g_Ext->m_PtrSize == 4)
    {
        return (ULONG64)object.GetUlong();
    }
    else
#endif
    {
        return object.GetPtr();
    }
}

ULONG64 ExtRemoteTypedUtil::Count(ExtRemoteTyped object, PCSTR field)
{
    ULONG64 count = 0;
    LinkListForEach(object, field, 
        [&count](ExtRemoteTyped& object) 
        { 
            count++; 
            return false; 
        });
    return count;
}

ULONG64 ExtRemoteTypedUtil::TaggedCount(ExtRemoteTyped object, PCSTR field)
{
    ULONG64 count = 0;
    LinkListForEachTagged(object, field,
        [&count](ExtRemoteTyped& object)
        {
            count++;
            return false;
        });
    return count;
}

ULONG64 ExtRemoteTypedUtil::GetSizeT(ExtRemoteTyped data)
{
    if (data.GetTypeSize() == 8)
    {
        return data.GetUlong64();
    }
    return data.GetUlong();
}
