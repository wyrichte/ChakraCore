//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

// Extracts string between [start, end) to help decompose dotted field name
class FieldNamePart
{
private:
    char buf[128];
    const char* name;

public:
    FieldNamePart(const char* start, const char* end) : name(start)
    {
        if (end)
        {
            if (strncpy_s(buf, start, end - start) == 0)
            {
                name = buf;
            }
            else
            {
                GetExtension()->ThrowInvalidArg("Field name too long? %s", start);
            }
        }
    }

    operator const char*() const
    {
        return name;
    }
};

bool FieldInfoCache::HasField(JDRemoteTyped& object, char const * fieldName)
{
    FieldInfoCache& fieldInfoCache = GetExtension()->fieldInfoCache;
    Key key = { object.GetModBase(), object.GetTypeId(), fieldName };
    auto i = fieldInfoCache.cache.find(key);
    if (i != fieldInfoCache.cache.end())
    {
        return i->second.HasField();
    }

    bool hasField = object.GetExtRemoteTyped().HasField(fieldName);
    fieldInfoCache.cache[key] = Value(hasField);
    return hasField;
}

JDRemoteTyped FieldInfoCache::GetField(JDRemoteTyped& object, char const * fieldName)
{
    const char* end = strchr(fieldName, '.');
    JDRemoteTyped field;

    FieldInfoCache& fieldInfoCache = GetExtension()->fieldInfoCache;
    Key key = { object.GetModBase(), object.GetTypeId(), fieldName };
    auto i = fieldInfoCache.cache.find(key);
    if (i != fieldInfoCache.cache.end() && i->second.IsValid())
    {
        field = JDRemoteTyped(i->second, (object.IsPointerType() ? object.GetPtr() : object.GetOffset()) + i->second.GetFieldOffset());
    }
    else
    {
        ExtRemoteTyped derefObject = object.IsPointerType() ? object.Dereference() : object.GetExtRemoteTyped();
        FieldNamePart name(fieldName, end); // Only query field name part before dot
        ExtRemoteTyped tempField = derefObject.Field(name);
        ExtRemoteTyped pointerToField = tempField.GetPointerTo();       // Forces "field" to be populate correctly.

        // TODO: Don't cache field that are size 1, because they may be bit fields, and I don't know how to distingish them
        if (tempField.m_Typed.Size != 1)
        {
            Value value(tempField.m_Typed.ModBase, tempField.m_Typed.TypeId, tempField.m_Typed.Size, (ULONG)(tempField.m_Offset - derefObject.m_Offset), tempField.m_Typed.Tag == SymTagPointerType);
            if (value.IsValid())
            {
                fieldInfoCache.cache[key] = value;
            }
        }
        field = tempField;
    }

    return end ? GetField(field, end + 1) : field;
}
