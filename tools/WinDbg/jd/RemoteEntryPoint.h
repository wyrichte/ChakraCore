//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class RemoteEntryPoint
{
public:
    RemoteEntryPoint();
    RemoteEntryPoint(ExtRemoteTyped entryPoint);
    
    ULONG64 GetPtr();
    bool IsInNativeAddressRange(ULONG64 nativeAddress);
    bool HasInlinees();
    ULONG GetFrameHeight();
private:
    ExtRemoteTyped entryPoint;
};