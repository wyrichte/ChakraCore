//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class JDTypeInfo
{
public:
    JDTypeInfo();

    void Clear();
    bool IsValid() const;

    ULONG64 GetModBase() const;
    ULONG GetTypeId() const;
    ULONG GetSize() const;
    bool IsPointerType() const;
    bool IsBitField() const;
    ULONG GetBitOffset() const;
    ULONG GetBitLength() const;

    static JDTypeInfo GetVoidPointerType();
    static JDTypeInfo FromExtRemoteTyped(ExtRemoteTyped const& remoteTyped);
private:
    friend class JDTypeCache;
    friend class FieldInfoCache;

    // Only JDTypeCache and FieldInfoCache can create JDTypeInfo
    JDTypeInfo(ULONG64 modBase, ULONG typeId, ULONG size, bool isPointerType, ULONG bitOffset, ULONG bitLength);

    ULONG64 m_ModBase;
    ULONG m_TypeId;
    ULONG m_size;
    ULONG m_bitOffset;
    ULONG m_bitLength;
    bool m_isPointerType;
    bool m_isValid;
};