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

bool FieldInfoCache::HasField(ExtRemoteTyped& object, char const * fieldName)
{
    FieldInfoCache& fieldInfoCache = GetExtension()->fieldInfoCache;
    Key key = { object.m_Typed.ModBase, object.m_Typed.TypeId, fieldName };
    auto i = fieldInfoCache.cache.find(key);
    if (i != fieldInfoCache.cache.end())
    {
        return i->second.IsValid();
    }

    if (!object.HasField(fieldName))
    {
        fieldInfoCache.cache[key] = Value();
        return false;
    }

    // Note: We don't cache any result here if object has the Field. Typically after
    // HasField() check the caller will GetField() with the same literal fieldName string.
    // GetField() will populate the cache. Future HasField() could see the cache and
    // return true.
    return true;
}

JDRemoteTyped FieldInfoCache::GetField(JDRemoteTyped& object, char const * fieldName)
{
    const char* end = strchr(fieldName, '.');
    JDRemoteTyped field;

    FieldInfoCache& fieldInfoCache = GetExtension()->fieldInfoCache;
    Key key = { object.m_Typed.ModBase, object.m_Typed.TypeId, fieldName };
    auto i = fieldInfoCache.cache.find(key);
    if (i != fieldInfoCache.cache.end() && i->second.IsValid())
    {
        field = JDRemoteTyped(i->second.GetModBase(), i->second.GetTypeId(),
            (object.m_Typed.Tag == SymTagPointerType ? object.GetPtr() : object.m_Offset) + i->second.GetFieldOffset());
    }
    else
    {
        ExtRemoteTyped derefObject = object.m_Typed.Tag == SymTagPointerType ? object.Dereference() : object;
        FieldNamePart name(fieldName, end); // Only query field name part before dot
        field = derefObject.Field(name);
        ExtRemoteTyped pointerToField = field.GetPointerTo();       // Forces "field" to be populate correctly.

        // TODO: Don't cache field that are size 1, because they may be bit fields, and I don't know how to distingish them
        if (field.m_Typed.Size != 1)
        {
            Value value(field.m_Typed.ModBase, field.m_Typed.TypeId, (ULONG)(field.m_Offset - derefObject.m_Offset));
            if (value.IsValid())
            {
                fieldInfoCache.cache[key] = value;
            }
        }
    }

    return end ? GetField(field, end + 1) : field;
}
