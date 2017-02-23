//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once
#include "stdafx.h"

RecyclerObjectTypeInfo::Cache::~Cache()
{
    typeInfoSet.MapAll([](RecyclerObjectTypeInfo * info)
    {
        delete info;
    });
}

RecyclerObjectTypeInfo * RecyclerObjectTypeInfo::Cache::GetRecyclerObjectTypeInfo(char const * typeName, char const * typeNameOrField, bool hasVtable, bool isPropagated, ULONG64 javascriptLibrary)
{
    RecyclerObjectTypeInfo tempTypeInfo(typeName, typeNameOrField, hasVtable, isPropagated, javascriptLibrary);
    RecyclerObjectTypeInfo * typeInfo = typeInfoSet.Get(&tempTypeInfo);
    if (typeInfo == nullptr)
    {
        typeInfo = new RecyclerObjectTypeInfo(tempTypeInfo);
        typeInfoSet.Add(typeInfo);
    }

    return typeInfo;
}