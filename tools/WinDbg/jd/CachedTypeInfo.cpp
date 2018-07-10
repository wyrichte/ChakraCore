//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "CachedTypeInfo.h"

CachedTypeInfo::CachedTypeInfo(char const * typeName, bool memoryNS, bool isChakra, bool isPtrTo) :
    typeName32(typeName), typeName64(typeName), memoryNS(memoryNS), isChakra(isChakra), GetTypeNameFunc(nullptr), isPtrTo(isPtrTo)
{}

CachedTypeInfo::CachedTypeInfo(char const * typeName32, char const * typeName64, bool memoryNS, bool isChakra, bool isPtrTo) :
    typeName32(typeName32), typeName64(typeName64), memoryNS(memoryNS), isChakra(isChakra), GetTypeNameFunc(nullptr), isPtrTo(isPtrTo)
{}

CachedTypeInfo::CachedTypeInfo(char const * (*GetTypeNameFunc)()) :
    memoryNS(false), isChakra(false), GetTypeNameFunc(GetTypeNameFunc)
{}

void CachedTypeInfo::Clear()
{
    typeInfo.Clear();
}

char const * const CachedTypeInfo::GetTypeName()
{
    EnsureTypeName();
    return g_Ext->IsCurMachine32() ? typeName32.c_str() : typeName64.c_str();
}

JDRemoteTyped CachedTypeInfo::Cast(ULONG64 address)
{
    EnsureCached();
    return JDRemoteTyped(typeInfo, address, isPtrTo);
}

ULONG CachedTypeInfo::GetSize()
{
    Assert(!isPtrTo);
    EnsureCached();
    return typeInfo.GetSize();
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

    std::string fullTypeName;
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

    if (isPtrTo)
    {
        std::string expr = "(";
        expr += fullTypeName + " *)@$extin";
        ExtRemoteTyped remoteTyped = ExtRemoteTyped(expr.c_str(), 0);
        typeInfo = JDTypeInfo::FromExtRemoteTyped(remoteTyped);
    }
    else
    {
        ExtRemoteTyped remoteTyped = ExtRemoteTyped(fullTypeName.c_str(), 0, false);
        typeInfo = JDTypeInfo::FromExtRemoteTyped(remoteTyped);
    }
    
}
