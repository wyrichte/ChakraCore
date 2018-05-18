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
        return JDRemoteTyped::NullPtr();
    }
    auto i = typeCache.cacheTypeInfoCache.find(typeName);

    if (i == typeCache.cacheTypeInfoCache.end())
    {
        CHAR typeNameBuffer[1024];
        sprintf_s(typeNameBuffer, "(%s!%s*)@$extin", GetExtension()->GetModuleName(), typeName);
        JDRemoteTyped result(typeNameBuffer, original);
        JDRemoteTyped deref = result.Dereference();
        typeCache.cacheTypeInfoCache.insert(std::make_pair(typeName, std::make_pair(deref.GetModBase(), deref.GetTypeId()))).first;
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

    if (!(vtbleAddr % 4 == 0 && (GetExtension()->InChakraModule(vtbleAddr) || GetExtension()->InEdgeModule(vtbleAddr))))
    {
        // Not our vtable
        return false;
    }

    JDTypeCache& typeCache = GetExtension()->typeCache;
    if (typeCache.vtableTypeIdMap.find(vtbleAddr) != typeCache.vtableTypeIdMap.end())
    {
        std::pair<ULONG64, ULONG> vtableTypeId = typeCache.vtableTypeIdMap[vtbleAddr];
        result = JDRemoteTyped(vtableTypeId.first, vtableTypeId.second, objectAddress, true);
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
            result = JDRemoteTyped(modBase, typeId, objectAddress, true);
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

// In release build, various vtable is ICF'ed.  Here are the overrides
static char const * PreloadVtableNames[]
{
    "Js::LiteralString",        // Js::BufferStringBuilder::WritableString, Js::SingleCharString
    "Js::DynamicObject",        // Js::WebAssemblyInstance
    "Js::JavascriptDate",       // Js::JavascriptDateWinRTDate
    "Js::RuntimeFunction",      // Js::JavascriptPromise*Function, JavascriptTypedObjectSlotAccessorFunction
    "Js::RecyclableObject",     // Js::ThrowErrorObject, Js::UndeclaredBlockVariable
    "Js::ScriptFunction",       // Js::AsmJsScriptFunction
    "Js::CustomExternalObject", // Projection::ArrayObjectInstance
    "Js::JavascriptArray",      // Js::JavascriptNativeArray

};
char const * JDTypeCache::GetTypeNameFromVTablePointer(ULONG64 vtableAddr)
{
    if (!isOverrideAddedToVtableTypeNameMap)
    {
        isOverrideAddedToVtableTypeNameMap = true;

        char const * moduleName = GetExtension()->GetModuleName();
        
        for (int i = 0; i < _countof(PreloadVtableNames); i++)
        {
            char const * name = PreloadVtableNames[i];
            std::string vtableSymbolName = GetExtension()->GetRemoteVTableName(name);
            ULONG64 offset;
            if (GetExtension()->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
            {
                auto newString = new std::string(std::string(moduleName) + "!" + name);
                vtableTypeNameMap[offset] = newString;
            }
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