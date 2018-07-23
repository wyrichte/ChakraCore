//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once
class FieldInfoCache;
class JDTypeInfo
{
public:
    ULONG64 GetModBase() const;
    ULONG GetTypeId() const;
    ULONG GetSize() const;
    bool IsPointerType() const;
    bool IsBitField() const;
    ULONG GetBitOffset() const;
    ULONG GetBitLength() const;
    JDTypeInfo * GetDerefType();
    FieldInfoCache * GetFieldInfoCache();

    static JDTypeInfo * GetVoidPointerType();
    static JDTypeInfo * FromExtRemoteTyped(ExtRemoteTyped& remoteTyped);

    operator size_t() const;
    bool operator<(JDTypeInfo const& other) const;
private:
    friend class JDTypeCache;
    
    // Only JDTypeCache can create JDTypeInfo
    JDTypeInfo(ULONG64 modBase, ULONG typeId, ULONG size, ULONG symTag, ULONG bitOffset, ULONG bitLength);
    JDTypeInfo(JDTypeInfo&) = delete;    
    ~JDTypeInfo();

    JDTypeInfo * EnsureDerefType();

    friend class FieldInfoCache;
    FieldInfoCache * EnsureFieldInfoCache();


    const ULONG64 m_ModBase;
    const ULONG m_TypeId;
    const ULONG m_bitOffset;

    const ULONG m_bitLength;

    const ULONG m_size;
    const ULONG m_symTag;
    JDTypeInfo * m_derefType;
    FieldInfoCache * m_fieldInfoCache;
};