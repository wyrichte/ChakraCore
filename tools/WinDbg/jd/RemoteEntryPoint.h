//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class RemoteEntryPoint
{
public:
    RemoteEntryPoint();
    RemoteEntryPoint(JDRemoteTyped entryPoint);
    
    ULONG64 GetPtr();
    bool IsInNativeAddressRange(ULONG64 nativeAddress);
    bool HasInlinees();
    ULONG GetFrameHeight();    
private:
    JDRemoteTyped GetNativePointData();
    JDRemoteTyped entryPoint;
};