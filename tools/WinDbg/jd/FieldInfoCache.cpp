//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

bool FieldInfoCache::HasField(ExtRemoteTyped& object, char const * fieldName)
{
    Assert(strchr(fieldName, '.') == nullptr);
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
    return true;
}

JDRemoteTyped FieldInfoCache::GetField(JDRemoteTyped& object, char const * fieldName)
{
    Assert(strchr(fieldName, '.') == nullptr);
    FieldInfoCache& fieldInfoCache = GetExtension()->fieldInfoCache;
    
    Key key = { object.m_Typed.ModBase, object.m_Typed.TypeId, fieldName };
    auto i = fieldInfoCache.cache.find(key);
    if (i != fieldInfoCache.cache.end() && i->second.IsValid())
    {
        return JDRemoteTyped(i->second.GetModBase(), i->second.GetTypeId(), 
            (object.m_Typed.Tag == SymTagPointerType? object.GetPtr() : object.m_Offset) + i->second.GetFieldOffset());
    }

    ExtRemoteTyped derefObject = object.m_Typed.Tag == SymTagPointerType ? object.Dereference() : object;
    JDRemoteTyped field = derefObject.Field(fieldName);
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
    return field;
}