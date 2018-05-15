//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "CachedTypeInfo.h"

CachedTypeInfo::CachedTypeInfo(char const * typeName, bool memoryNS, bool isChakra) :
    typeName32(typeName), typeName64(typeName), memoryNS(memoryNS), isChakra(isChakra), GetTypeNameFunc(nullptr)
{}

CachedTypeInfo::CachedTypeInfo(char const * typeName32, char const * typeName64, bool memoryNS, bool isChakra) :
    typeName32(typeName32), typeName64(typeName64), memoryNS(memoryNS), isChakra(isChakra), GetTypeNameFunc(nullptr)
{}

CachedTypeInfo::CachedTypeInfo(char const * (*GetTypeNameFunc)()) :
    memoryNS(false), isChakra(false), GetTypeNameFunc(GetTypeNameFunc)
{}

void CachedTypeInfo::Clear()
{
    typeInfo.Clear();
    fullTypeName.clear();
}

char const * const CachedTypeInfo::GetTypeName()
{
    EnsureTypeName();
    return g_Ext->IsCurMachine32() ? typeName32.c_str() : typeName64.c_str();
}

std::string CachedTypeInfo::GetFullTypeName()
{
    EnsureCached();
    return fullTypeName;
}

JDRemoteTyped CachedTypeInfo::Cast(ULONG64 address)
{
    EnsureCached();

    return JDRemoteTyped(typeInfo, address);
}

void CachedTypeInfo::EnsureTypeName()
{
    if (GetTypeNameFunc != nullptr)
    {
        typeName32 = GetTypeNameFunc();
        typeName64 = typeName32;
        GetTypeNameFunc = nullptr;
    }
}

void CachedTypeInfo::EnsureCached()
{
    if (typeInfo.IsValid())
    {
        return;
    }

    if (memoryNS)
    {
        std::string typeStringFormat("%s!%s");
        typeStringFormat += GetTypeName();
        fullTypeName = GetExtension()->FillModuleAndMemoryNS(typeStringFormat.c_str());
    }
    else if (isChakra)
    {
        std::string typeStringFormat("%s!");
        typeStringFormat += GetTypeName();
        fullTypeName = GetExtension()->FillModule(typeStringFormat.c_str());
    }
    else 
    {
        fullTypeName = GetTypeName();
    }

    ExtRemoteTyped remoteTyped = ExtRemoteTyped(fullTypeName.c_str(), 0, false);
    typeInfo.Set(remoteTyped);
}
