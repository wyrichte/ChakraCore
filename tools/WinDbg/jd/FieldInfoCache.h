//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class FieldInfoCache
{
public:
    FieldInfoCache(ULONG64 containerModBase, ULONG containerTypeId);

    class FieldInfo
    {
    public:
        bool IsValid() { return m_typeInfo != nullptr; }
        JDTypeInfo * GetTypeInfo() const { return m_typeInfo; }
        ULONG GetFieldOffset() const { return m_fieldOffset; }

    private:
        friend class FieldInfoCache;
        FieldInfo() : m_typeInfo(nullptr), m_fieldOffset((ULONG)-1) {}
        FieldInfo(JDTypeInfo * typeInfo, ULONG fieldOffset) : m_typeInfo(typeInfo), m_fieldOffset(fieldOffset) {}
        FieldInfo(FieldInfo value, ULONG startOffset) : m_typeInfo(value.m_typeInfo), m_fieldOffset(value.m_fieldOffset + startOffset) {}

        JDTypeInfo * const m_typeInfo;
        ULONG const m_fieldOffset;
    };

    FieldInfo GetFieldInfo(char const * fieldName, char const * fieldNameEnd);
private:
    struct Key
    {
        char const * m_fieldName;

        size_t GetHash() const
        {
            return ((size_t)m_fieldName) >> 4;
        }
        operator size_t() const
        {
            return GetHash();
        }
        bool operator<(FieldInfoCache::Key const& other) const
        {
            return this->m_fieldName < other.m_fieldName;
        }
    };
    

    // First level cache that assume the field name pointer is a const string in the image, and we can avoid the string compare
    stdext::hash_map<Key, FieldInfo> cache;

    // Field amp from the debug information
    stdext::hash_map<std::string, FieldInfo> fieldMap;

    void PopulateFieldInfo(ULONG64 containerModBase, ULONG containerTypeId);
};