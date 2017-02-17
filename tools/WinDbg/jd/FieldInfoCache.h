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
        Value(ULONG64 modBase = 0, ULONG typeId = 0, ULONG fieldOffset = (ULONG)-1)
            : m_ModBase(modBase), m_TypeId(typeId), m_fieldOffset(fieldOffset)
        {

        }
        
        bool IsValid() const
        {
            return this->m_fieldOffset != (ULONG)-1 && this->m_ModBase != 0 && this->m_TypeId != 0 && this->m_fieldOffset != -1;
        }

        ULONG64 GetModBase() const { return m_ModBase; }
        ULONG GetTypeId() const { return m_TypeId; }
        ULONG GetFieldOffset() const { return m_fieldOffset;  }
    private:

        ULONG64 m_ModBase;
        ULONG m_TypeId;
        ULONG m_fieldOffset;


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