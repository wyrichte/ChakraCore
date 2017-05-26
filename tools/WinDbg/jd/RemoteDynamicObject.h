//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteDynamicObject : public RemoteRecyclableObject
{
public:
    RemoteDynamicObject(RemoteRecyclableObject const& o);
    
    JDRemoteTyped GetDynamicType();
    void PrintProperties(bool printSlotIndex, int depth);
private:
    static void PrintProperty(bool printSlotIndex, ULONG64 name, LONG slot, ULONG64 value, LONG slot1, ULONG64 value1, int depth);
    friend class ObjectPropertyDumper;
};