//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

RemoteVar::RemoteVar(ULONG64 var) : var(var)
{

}

bool RemoteVar::DoInt32Var() 
{ 
    return g_Ext->m_PtrSize == 8;  // 64-bit use int32 var
}

bool RemoteVar::DoFloatVar()
{ 
    return g_Ext->m_PtrSize == 8;   // 64-bit use float var
}

bool RemoteVar::TryGetTaggedIntVar(int* value)
{
    if (GetExtension()->GetTaggedInt31Usage()) // IE9
    {
        if (var & 1)
        {
            *value = ((int)var) >> 1;
            return true;
        }
    }
    else
    {
        if (DoInt32Var())
        {
            if ((var >> 48) == 1)
            {
                *value = (int)var;
                return true;
            }
        }
        else
        {
            if (var & 1)
            {
                *value = ((int)var) >> 1;
                return true;
            }
        }
    }

    return false;
}

bool RemoteVar::TryGetFloatVar(double* value)
{
    if (DoFloatVar())
    {
        if ((uint64)var >> 50)
        {
            *(uint64*)value = var ^ FloatTag_Value;
            return true;
        }
    }
    return false;
}


void RemoteVar::Print(bool printSlotIndex, int depth)
{
    int intValue;
    double dblValue;

    if (TryGetTaggedIntVar(&intValue)) {
        g_Ext->Out("TaggedInt 0n%d\n", intValue);
        return;
    }
    else if (TryGetFloatVar(&dblValue)) {
        g_Ext->Out("FloatVar %f\n", dblValue);
        return;
    }
    RemoteRecyclableObject(var).Print(printSlotIndex, depth);    
}