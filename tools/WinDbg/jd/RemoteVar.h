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

    bool IsRecyclableObject();
    bool IsUndefined();
    bool IsNull();

    void Print(bool printSlotIndex, int depth);
    void PrintLink(char const * link = nullptr);
private:
    ULONG64 var;

    static bool DoInt32Var();
    static bool DoFloatVar();
};