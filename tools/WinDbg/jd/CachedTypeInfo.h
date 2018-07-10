//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

class CachedTypeInfo
{
public:
    CachedTypeInfo(char const * typeName, bool memoryNS, bool isChakra = true, bool isPtrTo = false);
    CachedTypeInfo(char const * typeName32, char const * typeName64, bool memoryNS, bool isChakra = true, bool isPtrTo = false);
    CachedTypeInfo(char const * (*GetTypeNameFunc)());
    JDRemoteTyped Cast(ULONG64 address);

    ULONG GetSize();

    void Clear();
private:
    char const * const GetTypeName();

    void EnsureCached();
    void EnsureTypeName();

    std::string typeName32;
    std::string typeName64;
    char const * (*GetTypeNameFunc)();
    bool isChakra;
    bool memoryNS;
    bool isPtrTo;
    JDTypeInfo typeInfo;
};
