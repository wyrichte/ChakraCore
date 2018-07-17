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

void FieldInfoCache::Clear()
{
    cache.clear();
}

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

void FieldInfoCache::EnsureNotBitField(ULONG64 containerModBase, ULONG containerTypeId, ULONG offset)
{
    // This function is to verify that this field is not a bit field.
    // There is no API that tells you the characteristic of a field by it's offset. 
    // Instead, we  find the index of any field that has the same offset as the input offset, 
    // and then you check whether the fields at indices before and after that index are at different offsets
    // and if they're not, you assume it must be a bitfield.

    ULONG64 handle;
    if (FAILED(g_Ext->m_System->GetCurrentProcessHandle(&handle)))
    {
        g_Ext->ThrowLastError("FieldInfoCache::GetField - Unable get current process handle");
    }

    ULONG count;
    if (!SymGetTypeInfo((HANDLE)handle, containerModBase, containerTypeId, TI_GET_CHILDRENCOUNT, &count))
    {
        g_Ext->ThrowLastError("FieldInfoCache::GetField - Unable to get count of children of type");
    }

    std::auto_ptr<TI_FINDCHILDREN_PARAMS> params((TI_FINDCHILDREN_PARAMS *)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * count));
    params.get()->Count = count;
    params.get()->Start = 0;
    if (!SymGetTypeInfo((HANDLE)handle, containerModBase, containerTypeId, TI_FINDCHILDREN, params.get()))
    {
        g_Ext->ThrowLastError("FieldInfoCache::GetField - Unable to get children of type");
    }
    

    for (ULONG i = 0; i < count; i++)
    {
        ULONG childId = params.get()->ChildId[i];
        DWORD childSymTag;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_SYMTAG, &childSymTag))
        {
            g_Ext->ThrowLastError("FieldInfoCache::GetField - Unable to get field sym tag");
        }

        if (childSymTag != SymTagData)
        {
            continue;
        }

        ULONG fieldOffset;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_OFFSET, &fieldOffset))
        {
            continue;
        }

        if (fieldOffset < offset)
        {
            continue;
        }

        ULONG fieldTypeId;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_TYPEID, &fieldTypeId))
        {
            g_Ext->ThrowLastError("FieldInfoCache::GetField - Unable to get field type");
        }

        ULONG fieldSize;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, fieldTypeId, TI_GET_LENGTH, &fieldSize))
        {
            g_Ext->ThrowLastError("FieldInfoCache::GetField - Unable to get field size");
        }
        if (fieldOffset + fieldSize <= offset)
        {
            break;
        }

        ULONG symTag;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, fieldTypeId, TI_GET_SYMTAG, &symTag))
        {
            g_Ext->ThrowLastError("FieldInfoCache::GetField - Unable to get field type sym tag");
        }
       
        if (symTag == SymTagUDT)
        {
            EnsureNotBitField(containerModBase, fieldTypeId, offset - fieldOffset);
            return;
        }

        DWORD bitPos;
        if (SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_BITPOSITION, &bitPos))
        {
            g_Ext->ThrowLastError("FieldInfoCache::GetField - Can't cache bit field.  Use .BitField instead");
        }
        return;
    }

    g_Ext->ThrowLastError("FieldInfoCache::GetField - Field not found");
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
        ExtRemoteTyped derefObject = object.IsPointerType() ? object.GetExtRemoteTyped().Dereference() : object.GetExtRemoteTyped();
        FieldNamePart name(fieldName, end); // Only query field name part before dot
        ExtRemoteTyped tempField = derefObject.Field(name);
        ExtRemoteTyped pointerToField = tempField.GetPointerTo();       // Forces "field" to be populate correctly.

        ULONG fieldOffset = (ULONG)(tempField.m_Offset - derefObject.m_Offset);
        if (tempField.m_Typed.Size == 1)
        {
            EnsureNotBitField(derefObject.m_Typed.ModBase, derefObject.m_Typed.TypeId, fieldOffset);
        }

        Value value(tempField.m_Typed.ModBase, tempField.m_Typed.TypeId, tempField.m_Typed.Size, fieldOffset, tempField.m_Typed.Tag == SymTagPointerType);
        if (value.IsValid())
        {
            fieldInfoCache.cache[key] = value;
        }
        field = tempField;
    }

    return end ? GetField(field, end + 1) : field;
}
