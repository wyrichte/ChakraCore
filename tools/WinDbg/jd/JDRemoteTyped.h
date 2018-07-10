//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class JDTypeInfo;

class JDRemoteTyped
{
public:
    JDRemoteTyped();
    JDRemoteTyped(PCSTR Expr);
    JDRemoteTyped(PCSTR Type, ULONG64 Offset, bool PtrTo);
    JDRemoteTyped(PCSTR Expr, ULONG64 Offset);    
    JDRemoteTyped(ExtRemoteTyped const& remoteTyped);

    JDRemoteTyped(ULONG64 modBase, ULONG typeID, ULONG64 offset, bool ptrTo);
    JDRemoteTyped(JDTypeInfo const& typeInfo, ULONG64 offset);

    bool HasField(PCSTR name);
    JDRemoteTyped Field(PCSTR name);
    JDRemoteTyped ArrayElement(LONG64 index);

    JDRemoteTyped BitField(PCSTR name);

    static JDRemoteTyped NullPtr();
    static JDRemoteTyped VoidPtr(ULONG64 address);
    static JDRemoteTyped FromPtrWithType(ULONG64 address, char const * typeName);
    static JDRemoteTyped FromPtrWithVtable(ULONG64 offset, const char ** typeName = nullptr);
    JDRemoteTyped CastWithVtable(const char ** typeName = nullptr);
    JDRemoteTyped Cast(const char * typeName);

    ULONG64 GetSizeT();
    char const * GetEnumString();

    // Redirects
    ExtRemoteTyped& GetExtRemoteTyped();
    JDRemoteTyped Dereference();
    JDRemoteTyped GetPointerTo();
    JDRemoteTyped operator[](_In_ LONG Index);
    JDRemoteTyped operator[](_In_ ULONG Index);
    JDRemoteTyped operator[](_In_ LONG64 Index);
    JDRemoteTyped operator[](_In_ ULONG64 Index);

    char const * GetTypeName();
    char const * GetSimpleValue();
    ULONG GetTypeSize();

    BOOL GetW32Bool();
    bool GetStdBool();
    CHAR GetChar();
    UCHAR GetUchar();
    SHORT GetShort();
    USHORT GetUshort();
    LONG GetLong();
    ULONG GetUlong();

    LONG64 GetLong64();
    ULONG64 GetUlong64();
    ULONG64 GetPtr();
    
    double GetDouble();

    PWCHAR GetString(ExtBuffer<WCHAR> * buffer);
private:
    JDRemoteTyped(ULONG64 address);

    friend class JDTypeCache;
    friend class FieldInfoCache;
    
    ULONG64 GetModBase();
    ULONG GetTypeId();
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
    bool isVoidPointer;
};
