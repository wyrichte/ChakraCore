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
    JDRemoteTyped(ULONG64 modBase, ULONG typeID, ULONG64 offset);
    JDRemoteTyped(ExtRemoteTyped remoteTyped);
    JDRemoteTyped Field(PCSTR name) const;
};
