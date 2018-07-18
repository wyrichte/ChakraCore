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
        bool operator<(FieldInfoCache::Key const& other) const
        {
            return this->m_TypeId < other.m_TypeId ||
                (this->m_TypeId == other.m_TypeId && (this->m_fieldName < other.m_fieldName ||
                    this->m_fieldName == other.m_fieldName && this->m_ModBase < other.m_ModBase));
        }
    };

    
    struct TypeKey
    {
        ULONG64 m_ModBase;
        ULONG m_TypeId;

        bool operator<(TypeKey const& other) const
        {
            return m_TypeId < other.m_TypeId
                || (m_TypeId == other.m_TypeId) && m_ModBase < other.m_ModBase;
        }

        operator size_t() const
        {
            return (size_t)(m_ModBase >> 4) ^ (size_t)m_TypeId;
        }
    };
    struct FieldInfo : public JDTypeInfo
    {
        FieldInfo() : m_fieldOffset((ULONG)-1)
        {

        }

        FieldInfo(ULONG64 modBase, ULONG typeId, ULONG size, ULONG fieldOffset, bool isPointerType, ULONG bitOffset, ULONG bitLength)
            : JDTypeInfo(modBase, typeId, size, isPointerType, bitOffset, bitLength), m_fieldOffset(fieldOffset)
        {

        }

        FieldInfo(FieldInfo value, ULONG startOffset)
            : JDTypeInfo(value), m_fieldOffset(value.m_fieldOffset + startOffset)
        {

        }

        ULONG GetFieldOffset() const { return m_fieldOffset; }        
    private:
        ULONG m_fieldOffset;
    };


    typedef stdext::hash_map<std::string, FieldInfo> FieldMap;
    stdext::hash_map<TypeKey, FieldMap *> fieldInfo;

    // First level cache that assume the field name pointer is a const string in the image, and we can avoid the string compare
    stdext::hash_map<Key, FieldInfo> cache;
   
    FieldMap * EnsureFieldMap(JDRemoteTyped& object);
    FieldMap * EnsureFieldMap(ULONG64 containerModBase, ULONG containerTypeId);
    FieldMap * CreateFieldMap(ULONG64 containerModBase, ULONG containerTypeId);
    FieldInfo GetFieldInfo(JDRemoteTyped& object, char const * fieldName, char const * fieldNameEnd);
    
public:
    static bool HasField(JDRemoteTyped& object, char const * field);
    static JDRemoteTyped GetField(JDRemoteTyped& object, char const * field);

    void Clear();
};