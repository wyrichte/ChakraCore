//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "FieldInfoCache.h"

JDTypeInfo::JDTypeInfo(ULONG64 modBase, ULONG typeId, ULONG size, ULONG symTag, ULONG bitOffset, ULONG bitLength)
    : m_ModBase(modBase), m_TypeId(typeId), m_size(size), m_symTag(symTag), m_bitOffset(bitOffset), m_bitLength(bitLength), m_derefType(nullptr), m_fieldInfoCache(nullptr)
{

}

JDTypeInfo::~JDTypeInfo()
{
    if (!this->IsPointerType())
    {
        if (this->m_fieldInfoCache)
        {
            delete this->m_fieldInfoCache;
        }
    }
}

JDTypeInfo::operator size_t() const
{
    return ((size_t)this->m_TypeId ^ (size_t)(this->m_ModBase >> 8) + this->m_bitOffset + this->m_bitLength);
}

bool JDTypeInfo::operator<(JDTypeInfo const& other) const
{
    return this->m_TypeId < other.m_TypeId
        || (this->m_TypeId == other.m_TypeId
            && (this->m_ModBase < other.m_ModBase
                || (this->m_ModBase == other.m_ModBase
                    && (this->m_bitOffset < other.m_bitOffset
                        || (this->m_bitOffset == other.m_bitOffset
                            && (this->m_bitLength < other.m_bitLength))))));
}

JDTypeInfo * JDTypeInfo::GetVoidPointerType()
{
    return JDTypeCache::GetTypeInfo(0, 0, g_Ext->m_PtrSize, SymTagPointerType, 0, 0);
}

JDTypeInfo * JDTypeInfo::FromExtRemoteTyped(ExtRemoteTyped& remoteTyped)
{
    JDTypeInfo * typeInfo = JDTypeCache::GetTypeInfo(remoteTyped.m_Typed.ModBase, remoteTyped.m_Typed.TypeId, remoteTyped.m_Typed.Size, remoteTyped.m_Typed.Tag, 0, 0);
    if (typeInfo->IsPointerType() && typeInfo->m_derefType == nullptr)
    {
        // Pointer type from ExtRemoteType can be synethic, we need to gather all the information now
        ExtRemoteTyped deref = remoteTyped.Dereference();
        typeInfo->m_derefType = JDTypeInfo::FromExtRemoteTyped(deref);
    }
    return typeInfo;
}


ULONG64 JDTypeInfo::GetModBase() const
{
    return m_ModBase;
}

ULONG JDTypeInfo::GetTypeId() const
{
    return m_TypeId;
}

ULONG JDTypeInfo::GetSize() const
{
    return m_size;
}

bool JDTypeInfo::IsPointerType() const
{
    return m_symTag == SymTagPointerType;
}

bool JDTypeInfo::IsBitField() const
{
    return m_bitLength != 0;
}

ULONG JDTypeInfo::GetBitOffset() const
{
    return m_bitOffset;
}

ULONG JDTypeInfo::GetBitLength() const
{
    return m_bitLength;
}

JDTypeInfo * JDTypeInfo::GetDerefType()
{
    return this->IsPointerType() ? EnsureDerefType() : nullptr;
}

FieldInfoCache * JDTypeInfo::GetFieldInfoCache()
{
    return EnsureFieldInfoCache();
}

JDTypeInfo * JDTypeInfo::EnsureDerefType()
{
    Assert(this->IsPointerType());
    JDTypeInfo * derefType = this->m_derefType;
    if (derefType == nullptr)
    {
        ULONG64 handle;
        if (FAILED(g_Ext->m_System->GetCurrentProcessHandle(&handle)))
        {
            g_Ext->ThrowLastError("JDTypeInfo::EnsureDerefType - Unable get current process handle");
        }
        ULONG64 containerModBase = this->GetModBase();
        ULONG containerTypeId = this->GetTypeId();
        ULONG typeId;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, containerTypeId, TI_GET_TYPE, &typeId))
        {
            g_Ext->ThrowLastError("JDTypeInfo::EnsureDerefType - Unable get type id");
        }
        ULONG64 typeSize;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, typeId, TI_GET_LENGTH, &typeSize))
        {
            g_Ext->ThrowLastError("JDTypeInfo::EnsureDerefType - Unable to get type size");
        }
        ULONG symTag;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, typeId, TI_GET_SYMTAG, &symTag))
        {
            g_Ext->ThrowLastError("JDTypeInfo::EnsureDerefType - Unable to get type sym tag");
        }

        derefType = JDTypeCache::GetTypeInfo(containerModBase, typeId, (ULONG)typeSize, symTag, 0, 0);
        this->m_derefType = derefType;
    }
    return derefType;
}

FieldInfoCache * JDTypeInfo::EnsureFieldInfoCache()
{
    if (this->IsPointerType())
    {
        JDTypeInfo * derefType = this->EnsureDerefType();
        if (derefType->IsPointerType())
        {
            // pointer of a pointer don't have field maps
            return nullptr;
        }
        return derefType->EnsureFieldInfoCache();
    }
    FieldInfoCache * fieldInfoCache = this->m_fieldInfoCache;
    if (fieldInfoCache == nullptr)
    {
        fieldInfoCache = new FieldInfoCache(this->GetModBase(), this->GetTypeId());
        this->m_fieldInfoCache = fieldInfoCache;
    }
    return fieldInfoCache;
}