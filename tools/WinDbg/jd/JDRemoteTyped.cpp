//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "JDRemoteTyped.h"
#include "JDTypeCache.h"
#include "FieldInfoCache.h"

JDRemoteTyped::JDRemoteTyped() :
    useExtRemoteTyped(true), typeInfo(nullptr)
{    
}

JDRemoteTyped::JDRemoteTyped(ExtRemoteTyped const& remoteTyped)
    : extRemoteTyped(remoteTyped), useExtRemoteTyped(true), typeInfo(nullptr)
{
}

JDRemoteTyped::JDRemoteTyped(JDTypeInfo * typeInfo, ULONG64 offset, bool ptrTo)
    : useExtRemoteTyped(false), typeInfo(typeInfo), isVoidPointer(false), isPtrTo(ptrTo)
{
    if (isPtrTo)
    {
        this->isDataValid = true;
        this->data = offset;
        this->offset = 0;
    }
    else
    {
        this->isDataValid = false;
        this->offset = offset;
    }
}

JDRemoteTyped::JDRemoteTyped(ULONG64 address)
    : useExtRemoteTyped(false), typeInfo(JDTypeInfo::GetVoidPointerType()), offset(0), isDataValid(true), isVoidPointer(true), data(address), isPtrTo(true)
{
}

JDRemoteTyped::JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo)
    : extRemoteTyped(Type, Offset, PtrTo), useExtRemoteTyped(true), typeInfo(nullptr)
{
}

JDRemoteTyped::JDRemoteTyped(PCSTR Expr, ULONG64 Offset)
    : extRemoteTyped(Expr, Offset), useExtRemoteTyped(true), typeInfo(nullptr)
{
}

JDRemoteTyped::JDRemoteTyped(PCSTR Expr)
    : extRemoteTyped(Expr), useExtRemoteTyped(true), typeInfo(nullptr)
{
}

JDTypeInfo *
JDRemoteTyped::GetTypeInfo()
{
    if (!useExtRemoteTyped)
    {
        return this->typeInfo;
    }

    if (this->typeInfo == nullptr)
    {
        this->typeInfo = JDTypeInfo::FromExtRemoteTyped(extRemoteTyped);
    }
    return this->typeInfo;
}

bool JDRemoteTyped::HasField(PCSTR fieldName)
{
    if (!useExtRemoteTyped && this->isVoidPointer)
    {
        return false;
    }

    const char* fieldNameEnd = strchr(fieldName, '.');
    FieldInfoCache::FieldInfo fieldInfo = this->GetTypeInfo()->GetFieldInfoCache()->GetFieldInfo(fieldName, fieldNameEnd);
    if (fieldNameEnd == nullptr || !fieldInfo.IsValid())
    {
        return fieldInfo.IsValid();
    }
    if (fieldInfo.GetTypeInfo()->IsBitField())
    {
        // Bit fields shouldn't have more subfield
        return false;
    }
    JDRemoteTyped field = JDRemoteTyped(fieldInfo.GetTypeInfo(), (this->IsPointerType() ? this->GetPtr() : this->GetOffset()) + fieldInfo.GetFieldOffset());
    return field.HasField(fieldNameEnd + 1);
}

JDRemoteTyped JDRemoteTyped::Field(PCSTR fieldName)
{
    const char* fieldNameEnd = strchr(fieldName, '.');
    FieldInfoCache::FieldInfo fieldInfo = this->GetTypeInfo()->GetFieldInfoCache()->GetFieldInfo(fieldName, fieldNameEnd);

    if (!fieldInfo.IsValid())
    {
        g_Ext->ThrowStatus(E_FAIL, "FieldInfoCache::GetField - Field '%s' doesn't exist", fieldName);
    }

    if (fieldInfo.GetTypeInfo()->IsBitField())
    {
        // We can't get back the ExtRemoteTyped for a bit field from the JDTypeInfo.
        // Until we get rid of the reliance of ExtRemoteType, we can't use the cached information
        return this->GetExtRemoteTyped().Field(fieldName);
    }

    JDRemoteTyped field = JDRemoteTyped(fieldInfo.GetTypeInfo(), (this->IsPointerType() ? this->GetPtr() : this->GetOffset()) + fieldInfo.GetFieldOffset());
    return fieldNameEnd ? field.Field(fieldNameEnd + 1) : field;
}

JDRemoteTyped JDRemoteTyped::ArrayElement(LONG64 index)
{
    if (!useExtRemoteTyped && this->isPtrTo && !this->isVoidPointer)
    {
        return JDRemoteTyped(this->typeInfo, this->data + this->typeInfo->GetSize() * index);
    }
    return GetExtRemoteTyped().ArrayElement(index);
}

JDRemoteTyped JDRemoteTyped::Cast(char const * typeName)
{
    return JDRemoteTyped::FromPtrWithType(this->GetPtr(), typeName);
}

ULONG64 JDRemoteTyped::GetObjectPtr()
{
    if (!this->IsPointerType())
    {
        return this->GetPointerTo().GetPtr();
    }
    return this->GetPtr();
}

JDRemoteTyped JDRemoteTyped::CastWithVtable(char const ** typeName)
{
    ULONG64 pointer = this->GetObjectPtr();

    JDRemoteTyped result;
    if (pointer && JDTypeCache::CastWithVtable(pointer, result, typeName))
    {
        return result;
    }
    return *this;
}

JDRemoteTyped JDRemoteTyped::NullPtr()
{
    return JDRemoteTyped::VoidPtr(0);
}

JDRemoteTyped JDRemoteTyped::VoidPtr(ULONG64 address)
{
    return JDRemoteTyped(address);
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
        result = JDRemoteTyped::VoidPtr(address);
    }
    return result;
}

ULONG64 JDRemoteTyped::GetSizeT()
{
    if (this->GetTypeSize() == 8)
    {
        return this->GetUlong64();
    }
    return this->GetUlong();
}

char const * JDRemoteTyped::GetEnumString()
{
    // Trim out the enum numeric value
    char * valueStr = this->GetExtRemoteTyped().GetSimpleValue();
    char * endValueStr = strchr(valueStr, '(');
    if (endValueStr != nullptr)
    {
        *(endValueStr - 1) = 0;
    }
    return valueStr;
}

// Redirection
ExtRemoteTyped& JDRemoteTyped::GetExtRemoteTyped()
{
    if (!useExtRemoteTyped)
    {
        if (this->isVoidPointer)
        {
            extRemoteTyped.Set("(void *)@$extin", this->data);
        }
        else if (this->isPtrTo)
        {
            extRemoteTyped.Set(true, this->typeInfo->GetModBase(), this->typeInfo->GetTypeId(), this->data);
        }
        else
        {
            extRemoteTyped.Set(false, this->typeInfo->GetModBase(), this->typeInfo->GetTypeId(), this->offset);
        }
        useExtRemoteTyped = true;
    }
    return extRemoteTyped;
}

JDRemoteTyped JDRemoteTyped::Dereference()
{
    if (!useExtRemoteTyped && !this->isVoidPointer)
    {
        if (this->isPtrTo)
        {
            return JDRemoteTyped(typeInfo, this->data);
        }
        JDTypeInfo * derefType = typeInfo->GetDerefType();
        if (derefType)
        {
            return JDRemoteTyped(derefType, this->GetPtr());
        }
    }
    return GetExtRemoteTyped().Dereference();
}

JDRemoteTyped JDRemoteTyped::GetPointerTo()
{
    return GetExtRemoteTyped().GetPointerTo();
}

JDRemoteTyped JDRemoteTyped::operator[](_In_ LONG Index)
{
    return ArrayElement(Index);
}

JDRemoteTyped JDRemoteTyped::operator[](_In_ ULONG Index)
{
    return ArrayElement((LONG64)Index);
}

JDRemoteTyped JDRemoteTyped::operator[](_In_ LONG64 Index)
{
    return ArrayElement(Index);
}

JDRemoteTyped JDRemoteTyped::operator[](_In_ ULONG64 Index)
{
    if (Index > 0x7fffffffffffffffUI64)
    {
        g_Ext->ThrowRemote
        (HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW),
            "Array index too large");
    }
    return ArrayElement((LONG64)Index);
}

char const * JDRemoteTyped::GetTypeName()
{
    return GetExtRemoteTyped().GetTypeName();
}

char const * JDRemoteTyped::GetSimpleTypeName()
{
    if (!useExtRemoteTyped && !isVoidPointer)
    {
        return JDTypeCache::GetTypeName(this->typeInfo, this->isPtrTo);
    }
    return JDUtil::StripStructClass(GetTypeName());
}

char const * JDRemoteTyped::GetSimpleValue()
{
    return GetExtRemoteTyped().GetSimpleValue();
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

SHORT JDRemoteTyped::GetShort()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<SHORT>();
    }
    return GetExtRemoteTyped().GetShort();
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

LONG64 JDRemoteTyped::GetLong64()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<LONG64>();
    }
    return GetExtRemoteTyped().GetLong64();
}

ULONG64 JDRemoteTyped::GetUlong64()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<ULONG64>();
    }
    return GetExtRemoteTyped().GetUlong64();
}

ULONG64 JDRemoteTyped::GetPtr()
{
    if (!useExtRemoteTyped)
    {
        return g_Ext->m_PtrSize == 8 ? EnsureData<ULONG64>() : EnsureData<ULONG>();
    }
    return GetExtRemoteTyped().GetPtr();
}

double JDRemoteTyped::GetDouble()
{
    if (!useExtRemoteTyped)
    {
        return EnsureData<double>();
    }
    return GetExtRemoteTyped().GetDouble();
}

wchar_t * JDRemoteTyped::GetString(ExtBuffer<wchar_t> * buffer)
{
    return GetExtRemoteTyped().GetString(buffer);
}

template <typename T>
T JDRemoteTyped::EnsureData()
{
    CompileAssert(sizeof(T) <= sizeof(data));
    if (isDataValid)
    {
        return *(T *)&data;
    }

    Assert(!this->isPtrTo);
    if (this->typeInfo->GetSize() != sizeof(T))
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

    if (this->typeInfo->IsBitField())
    {
        data = (data >> this->typeInfo->GetBitOffset()) & ((1 << this->typeInfo->GetBitLength()) - 1);
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
    return this->typeInfo->GetModBase();
}

ULONG JDRemoteTyped::GetTypeId()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.m_Typed.TypeId;
    }
    return this->typeInfo->GetTypeId();
}

bool JDRemoteTyped::IsPointerType()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.m_Typed.Tag == SymTagPointerType;
    }
    return this->isPtrTo || this->typeInfo->IsPointerType();
}

ULONG JDRemoteTyped::GetTypeSize()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.GetTypeSize();
    }
    return this->isPtrTo ? g_Ext->m_PtrSize : this->typeInfo->GetSize();
}

ULONG64 JDRemoteTyped::GetOffset()
{
    if (useExtRemoteTyped)
    {
        return extRemoteTyped.m_Offset;
    }
    Assert(!this->isPtrTo);
    return this->offset;
}