//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------

class CachedTypeInfo
{
public:
    CachedTypeInfo(char const * typeName, bool memoryNS, bool isChakra = true);
    CachedTypeInfo(char const * typeName32, char const * typeName64, bool memoryNS, bool isChakra = true);
    CachedTypeInfo(char const * (*GetTypeNameFunc)());
    JDRemoteTyped Cast(ULONG64 address);

    char const * const GetTypeName();
    std::string GetFullTypeName();

    void Clear();
private:
    void EnsureCached();
    void EnsureTypeName();

    std::string typeName32;
    std::string typeName64;
    char const * (*GetTypeNameFunc)();
    bool isChakra;
    bool memoryNS;
    std::string fullTypeName;
    ULONG64 modBase;
    ULONG typeId;
};

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------