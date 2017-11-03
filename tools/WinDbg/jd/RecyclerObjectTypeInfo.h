//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

class RecyclerObjectTypeInfo
{
public:
    RecyclerObjectTypeInfo(char const * typeName, char const * typeNameOrField, bool hasVtable, bool isPropagated, ULONG64 javascriptLibrary)
    {
        memset(this, 0, sizeof(RecyclerObjectTypeInfo));
        this->typeName = typeName;
        this->typeNameOrField = typeNameOrField;
        this->hasVtable = hasVtable;
        this->isPropagated = isPropagated;
        this->javascriptLibrary = javascriptLibrary;
    }

    bool HasVtable() const { return hasVtable; }
    bool IsPropagated() const { return isPropagated; }
    const char * GetTypeName() const { return typeName; }
    const char * GetTypeNameOrField() const { return typeNameOrField; }   
    ULONG64 GetAssociatedJavascriptLibrary() const { return javascriptLibrary; }

    class Cache
    {
    public:
        ~Cache();
        RecyclerObjectTypeInfo * GetRecyclerObjectTypeInfo(char const * typeName, char const * typeNameOrField, bool hasVtable, bool isPropagated, ULONG64 javascriptLibrary);
    private:
        class HashCompare
        {
        public:
            stdext::hash_compare<const char *> comp;
            static const size_t bucket_size = 4;
            static const size_t min_buckets = 8;
            size_t operator()(const RecyclerObjectTypeInfo * _Key) const
            {
                return stdext::_Hash_value((char *)_Key, (char *)(_Key + 1));
            }
            bool operator()(
                const RecyclerObjectTypeInfo * _Key1,
                const RecyclerObjectTypeInfo * _Key2
                ) const
            {
                return memcmp(_Key1, _Key2, sizeof(RecyclerObjectTypeInfo)) == -1;
            }
        };
        HashSet<RecyclerObjectTypeInfo *, HashCompare> typeInfoSet;
    };
private:
    const char * typeName;
    const char * typeNameOrField;
    ULONG64 javascriptLibrary;
    bool hasVtable;
    bool isPropagated;
};