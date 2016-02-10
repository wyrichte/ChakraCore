//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

JDRemoteTyped FieldInfoCache::GetField(JDRemoteTyped object, char const * fieldName)
{
    Assert(strchr(fieldName, '.') == nullptr);
    FieldInfoCache& fieldInfoCache = GetExtension()->fieldInfoCache;
    ExtRemoteTyped derefObject = object.m_Typed.Tag == SymTagPointerType ? object.Dereference() : object;
    Key key = { object.m_Typed.ModBase, object.m_Typed.TypeId, fieldName };
    auto i = fieldInfoCache.cache.find(key);
    if (i != fieldInfoCache.cache.end())
    {        
        return JDRemoteTyped((*i).second.m_ModBase, (*i).second.m_TypeId, derefObject.m_Offset + (*i).second.m_fieldOffset);
    }

    ExtRemoteTyped field = derefObject.Field(fieldName);
    ExtRemoteTyped pointerToField = field.GetPointerTo();

    Value value;
    value.m_fieldOffset = (ULONG)(field.m_Offset - derefObject.m_Offset);
    value.m_ModBase = field.m_Typed.ModBase;
    value.m_TypeId = field.m_Typed.TypeId;

    fieldInfoCache.cache[key] = value;
    return field;
}