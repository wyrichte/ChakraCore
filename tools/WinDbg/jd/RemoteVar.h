//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class RemoteVar
{
public:
    RemoteVar(ULONG64 var);

    bool TryGetTaggedIntVar(int* value);
    bool TryGetFloatVar(double* value);

    void Print(bool printSlotIndex, int depth);
private:
    ULONG64 var;

    static bool DoInt32Var();
    static bool DoFloatVar();
};