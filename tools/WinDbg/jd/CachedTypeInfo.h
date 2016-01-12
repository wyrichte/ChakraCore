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
    CachedTypeInfo(char const * typeName, bool memoryNS, bool isChakra = false);
    CachedTypeInfo(char const * (*GetTypeNameFunc)());
    ExtRemoteTyped Cast(ULONG64 address);

    char const * const GetTypeName();
    std::string GetFullTypeName();

    void Clear();
private:
    void EnsureCached();
    void EnsureTypeName();

    std::string typeName;
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