//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class JDRemoteTyped : public ExtRemoteTyped
{
public:
    JDRemoteTyped() {};
    JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo);
    JDRemoteTyped(PCSTR Expr, ULONG64 Offset);
    JDRemoteTyped(ULONG64 modBase, ULONG typeID, ULONG64 offset, bool PtrTo = false);
    JDRemoteTyped(ExtRemoteTyped const& remoteTyped);

    bool HasField(PCSTR name);
    JDRemoteTyped Field(PCSTR name);
    JDRemoteTyped ArrayElement(LONG64 index);

};
