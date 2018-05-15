//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

RemoteEntryPoint::RemoteEntryPoint()
    : entryPoint("(void *)0")
{

}

RemoteEntryPoint::RemoteEntryPoint(JDRemoteTyped entryPoint)
    : entryPoint(entryPoint.GetExtRemoteTyped())
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
    char const * state = entryPoint.Field("state").GetSimpleValue();
    if (ENUM_EQUAL(state, CodeGenRecorded) || ENUM_EQUAL(state, CodeGenDone))
    {
        JDRemoteTyped nativePointData = this->GetNativePointData();
        ULONG64 nativeAddress = nativePointData.Field("nativeAddress").GetPtr();
        if (codeAddress < nativeAddress)
        {
            return false;
        }
        ULONG64 codeSize = nativePointData.Field("codeSize").GetPtr();
        return codeAddress < nativeAddress + codeSize;
    }
    return false;
}

ULONG
RemoteEntryPoint::GetFrameHeight()
{
    return this->GetNativePointData().Field("frameHeight").GetUlong();
}

bool
RemoteEntryPoint::HasInlinees()
{
    return GetFrameHeight() > 0;
}

JDRemoteTyped
RemoteEntryPoint::GetNativePointData()
{
    if (entryPoint.HasField("nativeEntryPointData"))
    {
        return entryPoint.Field("nativeEntryPointData");
    }

    // Before commit 861fb1b74125aa4c2363e83af90b9b85974720a0, all the fields in nativeEntryPointData was in the entr point itself
    return entryPoint;
}