//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

RemoteEntryPoint::RemoteEntryPoint()
    : entryPoint("(void *)0")
{

}

RemoteEntryPoint::RemoteEntryPoint(ExtRemoteTyped entryPoint)
    : entryPoint(entryPoint)
{

}

ULONG64
RemoteEntryPoint::GetPtr()
{
    return entryPoint.GetPtr();
}

bool
RemoteEntryPoint::IsInNativeAddressRange(ULONG64 codeAddress)
{
    char * state = entryPoint.Field("state").GetSimpleValue();
    if (ENUM_EQUAL(state, CodeGenRecorded) || ENUM_EQUAL(state, CodeGenDone))
    {
        ULONG64 nativeAddress = entryPoint.Field("nativeAddress").GetPtr();
        if (codeAddress < nativeAddress)
        {
            return false;
        }
        ULONG64 codeSize = entryPoint.Field("codeSize").GetPtr();
        return codeAddress < nativeAddress + codeSize;
    }
    return false;
}

ULONG
RemoteEntryPoint::GetFrameHeight()
{
    return entryPoint.Field("frameHeight").GetUlong();
}

bool
RemoteEntryPoint::HasInlinees()
{
    return GetFrameHeight() > 0;
}