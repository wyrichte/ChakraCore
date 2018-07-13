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

    static JDTypeInfo GetVoidPointerType();
    static JDTypeInfo FromExtRemoteTyped(ExtRemoteTyped const& remoteTyped);
private:
    friend class JDTypeCache;
    friend class FieldInfoCache;

    // Only JDTypeCache and FieldInfoCache can create JDTypeInfo
    JDTypeInfo(ULONG64 modBase, ULONG typeId, ULONG size, bool isPointerType);

    ULONG64 m_ModBase;
    ULONG m_TypeId;
    ULONG m_size;
    bool m_isPointerType;
    bool m_isValid;
};