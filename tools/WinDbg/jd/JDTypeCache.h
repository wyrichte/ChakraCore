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

    static bool HasMultipleSymbol(ULONG64 address);
    static JDRemoteTyped Cast(LPCSTR typeName, ULONG64 original);
    static bool CastWithVtable(ULONG64 address, JDRemoteTyped& result, char const ** typeName = nullptr);
    char const * GetTypeNameFromVTablePointer(ULONG64 vtableAddr);

    std::map<ULONG64, std::pair<ULONG64, ULONG>> vtableTypeIdMap;
    std::map<ULONG64, std::string *> vtableTypeNameMap;
    stdext::hash_map<LPCSTR, std::pair<ULONG64, ULONG>> cacheTypeInfoCache;
    bool isOverrideAddedToVtableTypeNameMap;
};