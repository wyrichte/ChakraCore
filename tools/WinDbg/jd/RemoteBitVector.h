//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

class RemoteBitVector
{
public:
    // Supports BVFixed or BVStatic
    RemoteBitVector() {};
    RemoteBitVector(ExtRemoteTyped bitVector);
    bool Test(ULONG64 bit, ULONG64 * bvUnitAddress);

    static RemoteBitVector FromBVFixedPointer(ULONG64 bvFixedAddress);
    static ULONG64 GetBVFixedAllocSize(ULONG64 len);
private:
    ExtRemoteTyped bv;
};