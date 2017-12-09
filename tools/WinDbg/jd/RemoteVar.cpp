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

    if (TryGetTaggedIntVar(&intValue))
    {
        if (GetExtension()->PreferDML() && depth != 0)
        {
            g_Ext->Dml("<link cmd=\"!jd.var 0x%p\">[TaggedInt] 0n%d</link>", this->var, intValue);
        }
        else
        {
            g_Ext->Out("[TaggedInt] 0n%d (0x%p)", intValue, this->var);
        }       
        return;
    }
    if (TryGetFloatVar(&dblValue))
    {
        if (GetExtension()->PreferDML() && depth != 0)
        {
            g_Ext->Dml("<link cmd=\"!jd.var 0x%p\">[TaggedInt] 0n%d</link>", this->var, intValue);
        }
        else
        {
            g_Ext->Out("[FloatVar] %f (0x%p)", intValue, this->var);
        }
        return;
    }
    RemoteRecyclableObject(var).Print(printSlotIndex, depth);
}

void RemoteVar::PrintLink(char const * link)
{
    if (GetExtension()->PreferDML())
    {
        if (link != nullptr)
        {
            g_Ext->Dml("<link cmd=\"!jd.var 0x%p\">%s</link>", this->var, link);
        }
        else
        {
            g_Ext->Dml("<link cmd=\"!jd.var 0x%p\">0x%p</link>", this->var, this->var);
        }
    }
    else
    {
        if (link != nullptr)
        {
            g_Ext->Out("%s=0x%p", link, this->var);
        }
        else
        {
            g_Ext->Out("0x%p", this->var);
        }
    }
}

bool RemoteVar::IsRecyclableObject()
{
    int intValue;
    double dblValue;
    if (TryGetTaggedIntVar(&intValue)) {
        return false;
    }
    if (TryGetFloatVar(&dblValue)) {
        return false;
    }
    return true;
}

bool RemoteVar::IsUndefined()
{
    return IsRecyclableObject() && RemoteRecyclableObject(var).IsUndefined();
}

bool RemoteVar::IsNull()
{
    return IsRecyclableObject() && RemoteRecyclableObject(var).IsNull();
}