//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class FieldInfoCache
{
private:
    
    struct Key
    {
        ULONG64 m_ModBase;
        ULONG m_TypeId;
        char const * m_fieldName;

        size_t GetHash() const
        {
            return ((size_t)m_TypeId ^ (size_t)(m_ModBase >> 4) ^ (((size_t)m_fieldName) >> 4) ^ _HASH_SEED);
        }
        operator size_t() const 
        {
            return GetHash();
        }
    };

    friend bool operator<(FieldInfoCache::Key const& a, FieldInfoCache::Key const& b);
    struct Value
    {
        ULONG m_fieldOffset;
        ULONG64 m_ModBase;
        ULONG m_TypeId;
    };

    stdext::hash_map<Key, Value> cache;
public:
    static bool HasField(ExtRemoteTyped& object, char const * field);
    static JDRemoteTyped GetField(JDRemoteTyped& object, char const * field);
};

inline bool operator<(FieldInfoCache::Key const& a, FieldInfoCache::Key const& b)
{
    return a.m_TypeId < b.m_TypeId ||
        (a.m_TypeId == b.m_TypeId && (a.m_fieldName < b.m_fieldName ||
            a.m_fieldName == b.m_fieldName && a.m_ModBase < b.m_ModBase));
}