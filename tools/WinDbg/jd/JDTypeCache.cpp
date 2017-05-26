//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "JDTypeCache.h"

JDTypeCache::JDTypeCache()
{
    isOverrideAddedToVtableTypeNameMap = false;
}

void JDTypeCache::Clear()
{
    this->vtableTypeIdMap.clear();
    for (auto i = this->vtableTypeNameMap.begin(); i != this->vtableTypeNameMap.end(); i++)
    {
        delete (*i).second;
    }
    this->vtableTypeNameMap.clear();
}

JDRemoteTyped JDTypeCache::Cast(LPCSTR typeName, ULONG64 original)
{
    JDTypeCache& typeCache = GetExtension()->typeCache;
    if (original == 0)
    {
        return JDRemoteTyped("(void *)0");
    }
    auto i = typeCache.cacheTypeInfoCache.find(typeName);

    if (i == typeCache.cacheTypeInfoCache.end())
    {
        CHAR typeNameBuffer[1024];
        sprintf_s(typeNameBuffer, "(%s!%s*)@$extin", GetExtension()->GetModuleName(), typeName);
        JDRemoteTyped result(typeNameBuffer, original);
        ExtRemoteTyped deref = result.Dereference();
        typeCache.cacheTypeInfoCache.insert(std::make_pair(typeName, std::make_pair(deref.m_Typed.ModBase, deref.m_Typed.TypeId))).first;
        return result;
    }

    return JDRemoteTyped((*i).second.first, (*i).second.second, original, true);
}


bool JDTypeCache::CastWithVtable(ULONG64 objectAddress, JDRemoteTyped& result, char const** typeName)
{
    if (typeName)
    {
        *typeName = nullptr;
    }

    ULONG64 vtbleAddr = 0;
    RecyclerCachedData& recyclerCachedData = GetExtension()->recyclerCachedData;
    if (recyclerCachedData.IsCachedDebuggeeMemoryEnabled())
    {
        RemoteHeapBlock * heapBlock = recyclerCachedData.FindCachedHeapBlock(objectAddress);
        if (heapBlock)
        {
            RemoteHeapBlock::AutoDebuggeeMemory data(heapBlock, objectAddress, GetExtension()->m_PtrSize);
            char const * rawData = data;
            vtbleAddr = GetExtension()->m_PtrSize == 8 ? *(ULONG64 const *)rawData : (ULONG64)*(ULONG const *)rawData;
        }
        else
        {
            ULONG read;
            if (sizeof(vtbleAddr) >= g_Ext->m_PtrSize && FAILED(g_Ext->m_Data->ReadVirtual(objectAddress, &vtbleAddr, g_Ext->m_PtrSize, &read)) && read != g_Ext->m_PtrSize)
            {
                return false;
            }            
        }
    }
    else
    {
        ULONG read;
        if (sizeof(vtbleAddr) >= g_Ext->m_PtrSize && FAILED(g_Ext->m_Data->ReadVirtual(objectAddress, &vtbleAddr, g_Ext->m_PtrSize, &read)) && read != g_Ext->m_PtrSize)
        {
            return false;
        }
    }

    if (!(vtbleAddr % 4 == 0 && GetExtension()->InChakraModule(vtbleAddr)))
    {
        // Not our vtable
        return false;
    }

    JDTypeCache& typeCache = GetExtension()->typeCache;
    if (typeCache.vtableTypeIdMap.find(vtbleAddr) != typeCache.vtableTypeIdMap.end())
    {
        std::pair<ULONG64, ULONG> vtableTypeId = typeCache.vtableTypeIdMap[vtbleAddr];
        result.Set(true, vtableTypeId.first, vtableTypeId.second, objectAddress);
        if (typeName)
        {
            *typeName = typeCache.GetTypeNameFromVTablePointer(vtbleAddr);
        }

        return true;
    }

    char const * localTypeName = typeCache.GetTypeNameFromVTablePointer(vtbleAddr);
    if (localTypeName != nullptr)
    {
        if (typeName)
        {
            *typeName = localTypeName;
        }

        ULONG64 modBase;
        ULONG typeId;
        if (SUCCEEDED(GetExtension()->m_Symbols3->GetSymbolModule(localTypeName, &modBase))
            && SUCCEEDED(GetExtension()->m_Symbols3->GetTypeId(modBase, localTypeName, &typeId)))
        {
            result.Set(true, modBase, typeId, objectAddress);
            typeCache.vtableTypeIdMap[vtbleAddr] = std::pair<ULONG64, ULONG>(modBase, typeId);
            return true;
        }
    }

    return false;
}


static BOOL HasMultipleSymbolCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    (*(uint *)UserContext)++;
    return TRUE;
}

bool JDTypeCache::HasMultipleSymbol(ULONG64 address)
{
    uint count = 0;
    ULONG64 handle;
    HRESULT hr = GetExtension()->m_System4->GetCurrentProcessHandle(&handle);
    if (SUCCEEDED(hr))
    {
        SymEnumSymbolsForAddr((HANDLE)handle, address, HasMultipleSymbolCallback, &count);
        return count != 1;
    }
    return false;
}

char const * JDTypeCache::GetTypeNameFromVTablePointer(ULONG64 vtableAddr)
{
    if (!isOverrideAddedToVtableTypeNameMap)
    {
        isOverrideAddedToVtableTypeNameMap = true;

        char const * moduleName = GetExtension()->GetModuleName();
        // In release build, various vtable is ICF'ed.  Here are the overrides

        // Js::LiteralString vtable may be ICF'ed with Js::BufferStringBuilder::WritableString or Js::SingleCharString.
        // It is preferable to always report Js::LiteralString for all 3.  Enter the offset into the vtableTypeNameMap
        std::string vtableSymbolName = GetExtension()->GetRemoteVTableName("Js::LiteralString");
        ULONG64 offset;
        if (GetExtension()->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
        {
            auto newString = new std::string(std::string(moduleName) + "!Js::LiteralString");
            vtableTypeNameMap[offset] = newString;
        }

        // Js::DynamicObject vtable may be ICF'ed with Js::WebAssemblyInstance
        // It is preferable to always report Js::DynamicObject for both.  Enter the offset into the vtableTypeNameMap
        vtableSymbolName = GetExtension()->GetRemoteVTableName("Js::DynamicObject");
        if (GetExtension()->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
        {
            auto newString = new std::string(std::string(moduleName) + "!Js::DynamicObject");
            vtableTypeNameMap[offset] = newString;
        }

        // Js::JavascriptDate vtable may be ICF'ed with Js::JavascriptDateWinRTDate
        // It is preferable to always report Js::JavascriptDate for both.  Enter the offset into the vtableTypeNameMap
        vtableSymbolName = GetExtension()->GetRemoteVTableName("Js::JavascriptDate");
        if (GetExtension()->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
        {
            auto newString = new std::string(std::string(moduleName) + "!Js::JavascriptDate");
            vtableTypeNameMap[offset] = newString;
        }

        // Js::RuntimeFunction vtable may be ICF'ed with Js::JavascriptPromise*Function and JavascriptTypedObjectSlotAccessorFunction
        // It is preferable to always report Js::RuntimeFunction for all of them.  Enter the offset into the vtableTypeNameMap
        vtableSymbolName = GetExtension()->GetRemoteVTableName("Js::RuntimeFunction");
        if (GetExtension()->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
        {
            auto newString = new std::string(std::string(moduleName) + "!Js::RuntimeFunction");
            vtableTypeNameMap[offset] = newString;
        }

        // Js::RecyclableObject vtable may be ICF'ed with Js::ThrowErrorObject and Js::UndeclaredBlockVariable
        // It is preferable to always report Js::RecyclableObject for all of them.  Enter the offset into the vtableTypeNameMap
        vtableSymbolName = GetExtension()->GetRemoteVTableName("Js::RecyclableObject");
        if (GetExtension()->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
        {
            auto newString = new std::string(std::string(moduleName) + "!Js::RecyclableObject");
            vtableTypeNameMap[offset] = newString;
        }
    }

    auto i = vtableTypeNameMap.find(vtableAddr);
    if (i != vtableTypeNameMap.end())
    {
        if ((*i).second)
        {
            return (*i).second->c_str();
        }
        return nullptr;
    }

    ExtBuffer<char> vtableName;
    try
    {
        ULONG64 displacement;
        if (GetExtension()->GetOffsetSymbol(vtableAddr, &vtableName, &displacement) && displacement == 0)
        {
            int len = (int)(strlen(vtableName.GetBuffer()) - strlen("::`vftable'"));
            if (len > 0 && strcmp(vtableName.GetBuffer() + len, "::`vftable'") == 0)
            {
                vtableName.GetBuffer()[len] = '\0';

                auto newString = new std::string(vtableName.GetBuffer());
                // Actual type name in expression shouldn't have __ptr64 in them
                JDUtil::ReplaceString(*newString, " __ptr64", "");
                vtableTypeNameMap[vtableAddr] = newString;
                return newString->c_str();
            }
        }
    }
    catch (...)
    {
    }
    return nullptr;
}

void JDTypeCache::WarnICF()
{
    bool warned = false;
    for (auto i = vtableTypeNameMap.begin(); i != vtableTypeNameMap.end(); i++)
    {
        if (HasMultipleSymbol(i->first))
        {
            warned = true;
            ExtWarn("WARNING: ICF vtable used: %p %s\n", i->first, i->second->c_str());
        }
    }
    if (!warned)
    {
        ExtOut("No ICF vtable detected");
    }
}