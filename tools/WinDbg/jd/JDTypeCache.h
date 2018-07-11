//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

// Helper class for JDRemoteTyped to cache type data, only used by JDRemoteTyped, and instantiated by the extension
class JDTypeCache
{
public:
    JDTypeCache();
    void Clear();

    void WarnICF();

private:
    friend class JDRemoteTyped;

    // Function used by JDRemoteTyped
    static bool HasMultipleSymbol(ULONG64 address);
    static JDRemoteTyped Cast(LPCSTR typeName, ULONG64 original);
    static bool CastWithVtable(ULONG64 address, JDRemoteTyped& result, char const ** typeName = nullptr);
    static char const * GetTypeName(JDTypeInfo const& typeInfo, bool isPtrTo);

    // Internal functions
    char const * GetTypeNameFromVTablePointer(ULONG64 vtableAddr);
    void EnsureOverrideAddedToVtableTypeNameMap();
    char const * AddTypeName(char const* typeName);
    JDTypeInfo GetCachedTypeInfo(char const * typeName);

    std::map<ULONG64, JDTypeInfo> vtableTypeInfoMap;
    std::map<ULONG64, std::string *> vtableTypeNameMap;
    stdext::hash_map<LPCSTR, JDTypeInfo> cacheTypeInfoCache;
    stdext::hash_map<LPCSTR, JDTypeInfo> chakraCacheTypeInfoCache;

    struct NameKey
    {
        ULONG64 modBase;
        ULONG typeId;
        bool ptrTo;

        bool operator<(NameKey const& other) const
        {
            return modBase < other.modBase || typeId < other.typeId || ptrTo < other.ptrTo;
        }
    };
    std::map<NameKey, char const *> typeNameMap;
    std::list<char const *> typeNames;
    bool isOverrideAddedToVtableTypeNameMap;
};