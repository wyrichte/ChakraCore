//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class JDTypeInfo;

class JDRemoteTyped
{
public:
    JDRemoteTyped();
    JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo);
    JDRemoteTyped(PCSTR Expr, ULONG64 Offset);
    JDRemoteTyped(ULONG64 modBase, ULONG typeID, ULONG64 offset, bool ptrTo);    
    JDRemoteTyped(ExtRemoteTyped const& remoteTyped);
    JDRemoteTyped(JDTypeInfo const& typeInfo, ULONG64 offset);

    bool HasField(PCSTR name);
    JDRemoteTyped Field(PCSTR name);
    JDRemoteTyped ArrayElement(LONG64 index);

    JDRemoteTyped BitField(PCSTR name);

    static JDRemoteTyped NullPtr();
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
    bool useExtRemoteTyped;
};
