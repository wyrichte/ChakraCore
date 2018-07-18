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
    std::set<FieldMap *> fieldMaps;
    for (auto i : fieldInfo)
    {
        fieldMaps.insert(i.second);
    }
    fieldInfo.clear();
    for (auto i : fieldMaps)
    {
        delete i;
    }
}

bool FieldInfoCache::HasField(JDRemoteTyped& object, char const * fieldName)
{
    const char* fieldNameEnd = strchr(fieldName, '.');
    FieldInfoCache& fieldInfoCache = GetExtension()->fieldInfoCache;
    FieldInfoCache::FieldInfo fieldInfo = fieldInfoCache.GetFieldInfo(object, fieldName, fieldNameEnd);
    if (fieldNameEnd == nullptr || !fieldInfo.IsValid())
    {
        return fieldInfo.IsValid();
    }
    if (fieldInfo.IsBitField())
    {
        // Bit fields shouldn't have more subfield
        return false;
    }
    JDRemoteTyped field = JDRemoteTyped(fieldInfo, (object.IsPointerType() ? object.GetPtr() : object.GetOffset()) + fieldInfo.GetFieldOffset());
    return HasField(field, fieldNameEnd + 1);
}

FieldInfoCache::FieldMap * FieldInfoCache::CreateFieldMap(ULONG64 containerModBase, ULONG containerTypeId)
{
    FieldMap * fieldMap = new FieldMap();
    ULONG64 handle;
    if (FAILED(g_Ext->m_System->GetCurrentProcessHandle(&handle)))
    {
        g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Unable get current process handle");
    }

    ULONG count;
    if (!SymGetTypeInfo((HANDLE)handle, containerModBase, containerTypeId, TI_GET_CHILDRENCOUNT, &count))
    {
        g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Unable to get count of children of type");
    }

    std::auto_ptr<TI_FINDCHILDREN_PARAMS> params((TI_FINDCHILDREN_PARAMS *)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * count));
    params.get()->Count = count;
    params.get()->Start = 0;
    if (!SymGetTypeInfo((HANDLE)handle, containerModBase, containerTypeId, TI_FINDCHILDREN, params.get()))
    {
        g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Unable to get children of type");
    }

    for (ULONG i = 0; i < count; i++)
    {
        ULONG childId = params.get()->ChildId[i];
        DWORD childSymTag;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_SYMTAG, &childSymTag))
        {
            g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Unable to get field sym tag");
        }

        if (childSymTag != SymTagData && childSymTag != SymTagBaseClass)
        {
            continue;
        }

        ULONG fieldTypeId;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_TYPEID, &fieldTypeId))
        {
            g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Unable to get field type");
        }

        ULONG fieldOffset;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_OFFSET, &fieldOffset))
        {
            continue;
        }

        if (childSymTag == SymTagBaseClass)
        {
            FieldInfoCache::FieldMap * baseClassFields = EnsureFieldMap(containerModBase, fieldTypeId);
            for (auto i : *baseClassFields)
            {
                fieldMap->insert(std::make_pair(i.first, FieldInfo(i.second, fieldOffset)));
            }
            continue;
        }

        ULONG64 fieldSize;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, fieldTypeId, TI_GET_LENGTH, &fieldSize))
        {
            g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Unable to get field size");
        }


        ULONG symTag;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, fieldTypeId, TI_GET_SYMTAG, &symTag))
        {
            g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Unable to get field type sym tag");
        }

        DWORD bitPos = 0;
        ULONG64 bitLength = 0;
        if (SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_BITPOSITION, &bitPos))
        {
            if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_LENGTH, &bitLength))
            {
                g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Can't cache bit field length");
            }
        }

        WCHAR * name;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_SYMNAME, &name))
        {
            g_Ext->ThrowLastError("FieldInfoCache::CreateFieldMap - Can't get anem of field");
        }
        char fieldName[1024];
        _snprintf(fieldName, _countof(fieldName), "%S", name);
        LocalFree(name);
        fieldMap->insert(std::make_pair(fieldName, FieldInfo(containerModBase, fieldTypeId, (ULONG)fieldSize, fieldOffset, symTag == SymTagPointerType, bitPos, (DWORD)bitLength)));
    }
    return fieldMap;
}

FieldInfoCache::FieldMap * FieldInfoCache::EnsureFieldMap(ULONG64 containerModBase, ULONG containerTypeId)
{
    TypeKey typeKey = { containerModBase, containerTypeId };
    auto i = fieldInfo.find(typeKey);
    if (i != fieldInfo.end())
    {
        return i->second;
    }
    FieldInfoCache::FieldMap * newFieldMap = CreateFieldMap(containerModBase, containerTypeId);
    fieldInfo[typeKey] = newFieldMap;
    return newFieldMap;
}
FieldInfoCache::FieldMap * FieldInfoCache::EnsureFieldMap(JDRemoteTyped& object)
{
    if (object.IsPointerType())
    {
        TypeKey typeKey = { object.GetModBase(), object.GetTypeId() };
        auto i = fieldInfo.find(typeKey);
        if (i != fieldInfo.end())
        {
            return i->second;
        }

        JDRemoteTyped derefObject = object.Dereference();
        if (derefObject.IsPointerType())
        {
            // pointer of pointer can't do field operations
            return nullptr;
        }
        FieldInfoCache::FieldMap * newFieldMap = EnsureFieldMap(derefObject.GetModBase(), derefObject.GetTypeId());
        fieldInfo[typeKey] = newFieldMap;       // pointer type shares the field Map of the type
    }
    return EnsureFieldMap(object.GetModBase(), object.GetTypeId());
}

JDRemoteTyped FieldInfoCache::GetField(JDRemoteTyped& object, char const * fieldName)
{
    FieldInfoCache& fieldInfoCache = GetExtension()->fieldInfoCache;
    const char* fieldNameEnd = strchr(fieldName, '.');
    FieldInfoCache::FieldInfo fieldInfo = fieldInfoCache.GetFieldInfo(object, fieldName, fieldNameEnd);

    if (!fieldInfo.IsValid())
    {
        g_Ext->ThrowLastError("Field doesn't exist");
    }

    if (fieldInfo.IsBitField())
    {
        // We can't get back the ExtRemoteTyped for a bit field from the JDTypeInfo.
        // Until we get rid of the reliance of ExtRemoteType, we can't use the cached information
        return object.GetExtRemoteTyped().Field(fieldName);
    }

    JDRemoteTyped field = JDRemoteTyped(fieldInfo , (object.IsPointerType() ? object.GetPtr() : object.GetOffset()) + fieldInfo.GetFieldOffset());
    return fieldNameEnd ? GetField(field, fieldNameEnd + 1) : field;
}

FieldInfoCache::FieldInfo FieldInfoCache::GetFieldInfo(JDRemoteTyped& object, char const * fieldName, char const * fieldNameEnd)
{
    Key key = { object.GetModBase(), object.GetTypeId(), fieldName };
    auto i = this->cache.find(key);    
    if (i != this->cache.end())
    {
        return i->second;
    }

    FieldInfoCache::FieldInfo fieldInfo;
    FieldInfoCache::FieldMap * fieldMap = this->EnsureFieldMap(object);
    if (fieldMap != nullptr)
    {
        FieldNamePart name(fieldName, fieldNameEnd); // Only query field name part before dot
        auto i = fieldMap->find((char const *)name);
        if (i != fieldMap->end())
        {
            fieldInfo = i->second;
        }
    }

    this->cache.insert(std::make_pair(key, fieldInfo));
    return fieldInfo;
}
