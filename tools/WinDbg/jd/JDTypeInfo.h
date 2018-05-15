//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class JDTypeInfo
{
public:
    JDTypeInfo();
    JDTypeInfo(ULONG64 modBase, ULONG typeId, ULONG size, bool isPointerType);

    void Set(ExtRemoteTyped const& remoteTyped);

    void Clear();
    bool IsValid() const;

    ULONG64 GetModBase() const;
    ULONG GetTypeId() const;
    ULONG GetSize() const;
    bool IsPointerType() const;
private:
    ULONG64 m_ModBase;
    ULONG m_TypeId;
    ULONG m_size;
    bool m_isPointerType;
    bool m_isValid;
};