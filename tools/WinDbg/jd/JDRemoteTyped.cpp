//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "JDRemoteTyped.h"
#include "JDTypeCache.h"

JDRemoteTyped::JDRemoteTyped() :
    useExtRemoteTyped(true)
{    
}

JDRemoteTyped::JDRemoteTyped(ExtRemoteTyped const& remoteTyped)
    : extRemoteTyped(remoteTyped), useExtRemoteTyped(true)
{
}

JDRemoteTyped::JDRemoteTyped(ULONG64 modBase, ULONG typeId, ULONG64 offset, bool ptrTo)
    : useExtRemoteTyped(true)
{
    extRemoteTyped.Set(ptrTo, modBase, typeId, offset);
}

JDRemoteTyped::JDRemoteTyped(JDTypeInfo const& typeInfo, ULONG64 offset)
    : useExtRemoteTyped(false), typeInfo(typeInfo), offset(offset), isDataValid(false)
{    
}

JDRemoteTyped::JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo)
    : extRemoteTyped(Type, Offset, PtrTo), useExtRemoteTyped(true)
{
}

JDRemoteTyped::JDRemoteTyped(PCSTR Expr, ULONG64 Offset)
    : extRemoteTyped(Expr, Offset), useExtRemoteTyped(true)
{
}

bool JDRemoteTyped::HasField(PCSTR field)
{
    return FieldInfoCache::HasField(*this, field);
}

JDRemoteTyped JDRemoteTyped::Field(PCSTR field)
{
    return FieldInfoCache::GetField(*this, field);
}

JDRemoteTyped JDRemoteTyped::BitField(PCSTR field)
{
    // Don't cache bit field accesses
    return GetExtRemoteTyped().Field(field);
}

JDRemoteTyped JDRemoteTyped::ArrayElement(LONG64 index)
{
    return GetExtRemoteTyped().ArrayElement(index);
}

JDRemoteTyped JDRemoteTyped::Cast(char const * typeName)
{
    return JDRemoteTyped::FromPtrWithType(this->GetPtr(), typeName);
}

JDRemoteTyped JDRemoteTyped::CastWithVtable(char const ** typeName)
{
    ULONG64 pointer;
    if (!this->IsPointerType())
    {
        pointer = this->GetPointerTo().GetPtr();
    }
    else
    {
        pointer = this->GetPtr();
    }

    JDRemoteTyped result;
    if (pointer && JDTypeCache::CastWithVtable(pointer, result, typeName))
    {
        return result;
    }
    return *this;
}

JDRemoteTyped JDRemoteTyped::NullPtr()
{
    return JDRemoteTyped("(void *)@$extin", 0);    
}

JDRemoteTyped JDRemoteTyped::FromPtrWithType(ULONG64 address, char const * typeName)
{
    return JDTypeCache::Cast(typeName, address);
}

JDRemoteTyped JDRemoteTyped::FromPtrWithVtable(ULONG64 address, char const ** typeName)
{
    JDRemoteTyped result;
    if (!JDTypeCache::CastWithVtable(address, result, typeName))
    {
        result = JDRemoteTyped("(void *)@$extin", address);
    }
    return result;
}

ULONG64 JDRemoteTyped::GetSizeT()
{
    return ExtRemoteTypedUtil::GetSizeT(GetExtRemoteTyped());
}

char const * JDRemoteTyped::GetEnumString()
{
    return JDUtil::GetEnumString(GetExtRemoteTyped());
}

// Redirection
ExtRemoteTyped& JDRemoteTyped::GetExtRemoteTyped()
{
    if (!useExtRemoteTyped)
    {
        extRemoteTyped.Set(false, this->typeInfo.GetModBase(), this->typeInfo.GetTypeId(), offset);
        useExtRemoteTyped = true;
    }
    return extRemoteTyped;
}

ExtRemoteTyped JDRemoteTyped::Dereference()
{
    return GetExtRemoteTyped().Dereference();
}

ExtRemoteTyped JDRemoteTyped::GetPointerTo()
{
    return GetExtRemoteTyped().GetPointerTo();
}

ExtRemoteTyped JDRemoteTyped::operator[](_In_ LONG Index)
{
    return GetExtRemoteTyped()[Index];
}

ExtRemoteTyped JDRemoteTyped::operator[](_In_ ULONG Index)
{
    return GetExtRemoteTyped()[Index];
}

ExtRemoteTyped JDRemoteTyped::operator[](_In_ LONG64 Index)
{
    return GetExtRemoteTyped()[Index];
}

ExtRemoteTyped JDRemoteTyped::operator[](_In_ ULONG64 Index)
{
    return GetExtRemoteTyped()[Index];
}

char const * JDRemoteTyped::GetTypeName()
{
    return GetExtRemoteTyped().GetTypeName();
}

char const * JDRemoteTyped::GetSimpleValue()
{
    return GetExtRemoteTyped().GetSimpleValue();
}

ULONG JDRemoteTyped::GetTypeSize()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.GetTypeSize();        
    }
    return this->typeInfo.GetSize();
}

BOOL JDRemoteTyped::GetW32Bool()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<BOOL>();
    }
    return GetExtRemoteTyped().GetW32Bool();
}

bool JDRemoteTyped::GetStdBool()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<bool>();
    }
    return GetExtRemoteTyped().GetStdBool();
}

CHAR JDRemoteTyped::GetChar()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<CHAR>();
    }
    return GetExtRemoteTyped().GetChar();
}

UCHAR JDRemoteTyped::GetUchar()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<UCHAR>();
    }
    return GetExtRemoteTyped().GetUchar();
}

USHORT JDRemoteTyped::GetUshort()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<USHORT>();
    }
    return GetExtRemoteTyped().GetUshort();
}

LONG JDRemoteTyped::GetLong()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<LONG>();
    }
    return GetExtRemoteTyped().GetLong();
}

ULONG JDRemoteTyped::GetUlong()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<ULONG>();
    }
    return GetExtRemoteTyped().GetUlong();
}

ULONG64 JDRemoteTyped::GetPtr()
{
    if (!useExtRemoteTyped)
    {
        return g_Ext->m_PtrSize == 8 ? EnsureData<ULONG64>() : EnsureData<ULONG>();
    }
    return GetExtRemoteTyped().GetPtr();
}

template <typename T>
T JDRemoteTyped::EnsureData()
{
    CompileAssert(sizeof(T) <= sizeof(data));
    if (isDataValid)
    {
        return *(T *)&data;
    }

    if (this->typeInfo.GetSize() != sizeof(T))
    {
        g_Ext->ThrowLastError("JDRemoteTyped: Size mismatch");
    }

    RecyclerCachedData& recyclerCachedData = GetExtension()->recyclerCachedData;
    if (recyclerCachedData.IsCachedDebuggeeMemoryEnabled())
    {
        RemoteHeapBlock * heapBlock = recyclerCachedData.FindCachedHeapBlock(this->offset);
        if (heapBlock)
        {
            RemoteHeapBlock::AutoDebuggeeMemory memory(heapBlock, this->offset, GetExtension()->m_PtrSize);
            char const * rawData = memory;
            data = (ULONG64)*(T const *)rawData;
            isDataValid = true;
            return *(T *)&data;
        }
    }

    HRESULT hr;
    ULONG bytesRead;
    if (FAILED(hr = g_Ext->m_Data4->ReadVirtual(this->offset, &data, sizeof(T), &bytesRead)))
    {
        g_Ext->ThrowStatus(hr, "Failed to GetPtr");
    }

    if (bytesRead != sizeof(T))
    {
        g_Ext->ThrowLastError("Unable to read");
    }
    isDataValid = true;
    return *(T *)&data;

}

// For FieldInfoCache
ULONG64 JDRemoteTyped::GetModBase()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.m_Typed.ModBase;
    }
    return this->typeInfo.GetModBase();
}

ULONG JDRemoteTyped::GetTypeId()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.m_Typed.TypeId;
    }
    return this->typeInfo.GetTypeId();
}

bool JDRemoteTyped::IsPointerType()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.m_Typed.Tag == SymTagPointerType;
    }
    return this->typeInfo.IsPointerType();
}

ULONG64 JDRemoteTyped::GetOffset()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.m_Offset;
    }
    return this->offset;
}