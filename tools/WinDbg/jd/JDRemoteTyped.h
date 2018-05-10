//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class JDTypeInfo
{
public:
    JDTypeInfo() 
    {
        Clear();
    }
    JDTypeInfo(ULONG64 modBase, ULONG typeId, ULONG size, bool isPointerType)
        : m_ModBase(modBase), m_TypeId(typeId), m_size(size), m_isPointerType(isPointerType), m_isValid(true)
    {

    }

    void Set(ExtRemoteTyped const& remoteTyped)
    {
        m_isValid = true;
        m_ModBase = remoteTyped.m_Typed.ModBase;
        m_TypeId = remoteTyped.m_Typed.TypeId;
        m_size = remoteTyped.m_Typed.Size;
        m_isPointerType = remoteTyped.m_Typed.Tag == 14; // SymTagPointerType;
    }

    void Clear()
    {
        m_isValid = false;
        m_ModBase = 0;
        m_TypeId = 0;
        m_size = 0;
        m_isPointerType = false;
    }
    bool IsValid() const
    {
        return this->m_isValid;
    }

    ULONG64 GetModBase() const { return m_ModBase; }
    ULONG GetTypeId() const { return m_TypeId; }
    ULONG GetSize() const { return m_size; }
    bool IsPointerType() const { return m_isPointerType; }
private:
    ULONG64 m_ModBase;
    ULONG m_TypeId;
    ULONG m_size;
    bool m_isPointerType;
    bool m_isValid;
};

class JDRemoteTyped
{
public:
    JDRemoteTyped() {};    
    JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo);
    JDRemoteTyped(PCSTR Expr, ULONG64 Offset);
    JDRemoteTyped(ULONG64 modBase, ULONG typeID, ULONG64 offset, bool ptrTo);    
    JDRemoteTyped(ExtRemoteTyped const& remoteTyped);
    JDRemoteTyped(JDTypeInfo typeInfo, ULONG64 offset);

    bool HasField(PCSTR name);
    JDRemoteTyped Field(PCSTR name);
    JDRemoteTyped ArrayElement(LONG64 index);

    JDRemoteTyped BitField(PCSTR name);

    static JDRemoteTyped FromPtrWithType(ULONG64 address, char const * typeName);
    static JDRemoteTyped FromPtrWithVtable(ULONG64 offset, const char ** typeName = nullptr);
    JDRemoteTyped CastWithVtable(const char ** typeName = nullptr);
    JDRemoteTyped Cast(const char * typeName);

    ULONG64 GetSizeT();
    char const * GetEnumString();

    // Redirects
    ExtRemoteTyped& GetExtRemoteTyped();
    ExtRemoteTyped Dereference();
    ExtRemoteTyped GetPointerTo();
    ExtRemoteTyped operator[](_In_ LONG Index);
    ExtRemoteTyped operator[](_In_ ULONG Index);
    ExtRemoteTyped operator[](_In_ LONG64 Index);
    ExtRemoteTyped operator[](_In_ ULONG64 Index);

    char const * GetTypeName();
    char const * GetSimpleValue();
    ULONG GetTypeSize();

    BOOL GetW32Bool();
    bool GetStdBool();
    CHAR GetChar();
    UCHAR GetUchar();
    USHORT GetUshort();
    LONG GetLong();
    ULONG GetUlong();

    ULONG64 GetPtr();
    
private:
    friend class FieldInfoCache;
    
    ULONG64 GetModBase();
    ULONG GetTypeId();
    ULONG GetSize();
    bool IsPointerType();
    ULONG64 GetOffset();
    
    template <typename T>
    T EnsureData();

    ExtRemoteTyped extRemoteTyped;    
    JDTypeInfo typeInfo;
    ULONG64 offset;
    ULONG64 data;
    bool isDataValid;
    bool extRemoteTypedInitialized;
};
