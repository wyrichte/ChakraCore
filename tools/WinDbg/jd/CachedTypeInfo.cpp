//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "CachedTypeInfo.h"

CachedTypeInfo::CachedTypeInfo(char const * typeName, bool memoryNS, bool isChakra) :
    typeName32(typeName), typeName64(typeName), memoryNS(memoryNS), isChakra(isChakra), modBase(0), typeId(0), GetTypeNameFunc(nullptr)
{}

CachedTypeInfo::CachedTypeInfo(char const * typeName32, char const * typeName64, bool memoryNS, bool isChakra) :
    typeName32(typeName32), typeName64(typeName64), memoryNS(memoryNS), isChakra(isChakra), modBase(0), typeId(0), GetTypeNameFunc(nullptr)
{}

CachedTypeInfo::CachedTypeInfo(char const * (*GetTypeNameFunc)()) :
    memoryNS(false), isChakra(false), GetTypeNameFunc(GetTypeNameFunc), modBase(0), typeId(0)
{}

void CachedTypeInfo::Clear()
{
    modBase = 0;
    typeId = 0;
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

    return JDRemoteTyped(modBase, typeId, address);
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
    if (typeId != 0)
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
    modBase = remoteTyped.m_Typed.ModBase;
    typeId = remoteTyped.m_Typed.TypeId;
}
