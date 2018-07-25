//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "FieldInfoCache.h"

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

FieldInfoCache::FieldInfoCache(ULONG64 containerModBase, ULONG containerTypeId)
{
    PopulateFieldInfo(containerModBase, containerTypeId);
}

void
FieldInfoCache::PopulateFieldInfo(ULONG64 containerModBase, ULONG containerTypeId)
{
    ULONG64 handle;
    if (FAILED(g_Ext->m_System->GetCurrentProcessHandle(&handle)))
    {
        g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Unable get current process handle");
    }

    ULONG count;
    if (!SymGetTypeInfo((HANDLE)handle, containerModBase, containerTypeId, TI_GET_CHILDRENCOUNT, &count))
    {
        g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Unable to get count of children of type");
    }

    std::auto_ptr<TI_FINDCHILDREN_PARAMS> params((TI_FINDCHILDREN_PARAMS *)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * count));
    params.get()->Count = count;
    params.get()->Start = 0;
    if (!SymGetTypeInfo((HANDLE)handle, containerModBase, containerTypeId, TI_FINDCHILDREN, params.get()))
    {
        g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Unable to get children of type");
    }

    for (ULONG i = 0; i < count; i++)
    {
        ULONG childId = params.get()->ChildId[i];
        DWORD childSymTag;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_SYMTAG, &childSymTag))
        {
            g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Unable to get field sym tag");
        }

        if (childSymTag != SymTagData && childSymTag != SymTagBaseClass)
        {
            continue;
        }

        ULONG fieldTypeId;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_TYPEID, &fieldTypeId))
        {
            g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Unable to get field type");
        }

        ULONG fieldOffset;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_OFFSET, &fieldOffset))
        {
            continue;
        }

        ULONG64 fieldSize;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, fieldTypeId, TI_GET_LENGTH, &fieldSize))
        {
            g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Unable to get field size");
        }

        ULONG symTag;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, fieldTypeId, TI_GET_SYMTAG, &symTag))
        {
            g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Unable to get field type sym tag");
        }

        if (childSymTag == SymTagBaseClass)
        {
            JDTypeInfo * baseTypeInfo = JDTypeCache::GetTypeInfo(containerModBase, fieldTypeId, (ULONG)fieldSize, symTag, 0, 0);
            FieldInfoCache * baseClassFields = baseTypeInfo->EnsureFieldInfoCache();
            for (auto i : baseClassFields->fieldMap)
            {
                fieldMap.insert(std::make_pair(i.first, FieldInfo(i.second, fieldOffset)));
            }
            continue;
        }

        DWORD bitPos = 0;
        ULONG64 bitLength = 0;
        if (SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_BITPOSITION, &bitPos))
        {
            if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_LENGTH, &bitLength))
            {
                g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Can't cache bit field length");
            }
        }

        WCHAR * name;
        if (!SymGetTypeInfo((HANDLE)handle, containerModBase, childId, TI_GET_SYMNAME, &name))
        {
            g_Ext->ThrowLastError("FieldInfoCache::PopulateFieldInfo - Can't get name of field");
        }
        char fieldName[1024];
        _snprintf(fieldName, _countof(fieldName), "%S", name);
        LocalFree(name);
        fieldMap.insert(std::make_pair(fieldName, FieldInfo(JDTypeCache::GetTypeInfo(containerModBase, fieldTypeId, (ULONG)fieldSize, symTag, bitPos, (DWORD)bitLength), fieldOffset)));
    }
}

FieldInfoCache::FieldInfo FieldInfoCache::GetFieldInfo(char const * fieldName, char const * fieldNameEnd)
{
    Key key = { fieldName };
    auto i = this->cache.find(key);    
    if (i != this->cache.end())
    {
        return i->second;
    }

    FieldNamePart name(fieldName, fieldNameEnd); // Only query field name part before dot
    auto j = this->fieldMap.find((char const *)name);
    if (j != this->fieldMap.end())
    {
        this->cache.insert(std::make_pair(key, j->second));
        return j->second;
    }

    this->cache.insert(std::make_pair(key, FieldInfo()));
    return FieldInfo();
}
