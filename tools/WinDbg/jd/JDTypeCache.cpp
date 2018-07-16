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
    this->vtableTypeInfoMap.clear();
    for (auto i : this->vtableTypeNameMap)
    {
        delete i.second;
    }
    this->vtableTypeNameMap.clear();
    for (auto i : this->typeNames)
    {
        delete i;
    }
    this->typeNames.clear();
    this->cacheTypeInfoCache.clear();
    this->chakraCacheTypeInfoCache.clear();
}

JDTypeInfo JDTypeCache::GetCachedTypeInfo(char const * typeName)
{
    // NOTE: This function assume typeName is not a pointer type.
    // This is either called from a vtable derived type name, or JDTypeCache::Cast
    // which already check if it is pointer type.
    auto i = this->cacheTypeInfoCache.find(typeName);

    if (i != this->cacheTypeInfoCache.end())
    {
        return i->second;
    }

    ULONG64 modBase;
    ULONG typeId;
    ULONG size;
    if (SUCCEEDED(GetExtension()->m_Symbols3->GetSymbolModule(typeName, &modBase))
        && SUCCEEDED(GetExtension()->m_Symbols3->GetTypeId(modBase, typeName, &typeId))
        && SUCCEEDED(GetExtension()->m_Symbols3->GetTypeSize(modBase, typeId, &size)))
    {
        JDTypeInfo typeInfo = JDTypeInfo(modBase, typeId, size, false);
        this->cacheTypeInfoCache[typeName] = typeInfo;
        return typeInfo;
    }

    g_Ext->Err("ERROR: Unable to get type info '%s'\n", typeName);
    g_Ext->ThrowLastError("Fatal error in JDTypeCache::GetCachedTypeInfo");
}

char const * JDTypeCache::AddTypeName(char const * typeName)
{
    char const * name = _strdup(typeName);
    typeNames.push_back(name);
    return name;
}

char const * JDTypeCache::GetTypeName(JDTypeInfo const& typeInfo, bool isPtrTo)
{
    JDTypeCache& typeCache = GetExtension()->typeCache;
    NameKey nameKey = { typeInfo.GetModBase(), typeInfo.GetTypeId(), isPtrTo };
    auto i = typeCache.typeNameMap.find(nameKey);
    if (i != typeCache.typeNameMap.end())
    {
        return i->second;
    }

    char const * foundName = nullptr;
    if (isPtrTo)
    {
        NameKey nameKey = { typeInfo.GetModBase(), typeInfo.GetTypeId(), false };
        auto i = typeCache.typeNameMap.find(nameKey);
        if (i != typeCache.typeNameMap.end())
        {
            foundName = i->second;
        }
    }
    
    if (foundName == nullptr)
    {
        char originalTypeName[1024];
        ULONG originalTypeNameSize;
        if (SUCCEEDED(GetExtension()->m_Symbols3->GetTypeName(typeInfo.GetModBase(), typeInfo.GetTypeId(), originalTypeName, _countof(originalTypeName), &originalTypeNameSize)))
        {
            NameKey nameKey = { typeInfo.GetModBase(), typeInfo.GetTypeId(), false };
            char const * typeName = typeCache.AddTypeName(originalTypeName);
            typeCache.typeNameMap[nameKey] = typeName;
            foundName = typeName;
        }
        else
        {
            g_Ext->ThrowLastError("Unable to get type name");
        }
    }

    if (isPtrTo)
    {
        std::string ptrToName = foundName;
        if (ptrToName[ptrToName.length() - 1] == '*')
        {
            ptrToName += '*';
        }
        else
        {
            ptrToName += " *";
        }

        NameKey nameKey = { typeInfo.GetModBase(), typeInfo.GetTypeId(), true };
        foundName = typeCache.AddTypeName(ptrToName.c_str());
        typeCache.typeNameMap[nameKey] = foundName;
    }
    return foundName;
    
}

JDRemoteTyped JDTypeCache::Cast(LPCSTR typeName, ULONG64 original)
{
    if (original == 0)
    {
        return JDRemoteTyped::NullPtr();
    }

    JDTypeCache& typeCache = GetExtension()->typeCache;
    auto i = typeCache.chakraCacheTypeInfoCache.find(typeName);
    if (i != typeCache.chakraCacheTypeInfoCache.end())
    {
        return JDRemoteTyped(i->second, original, true);
    }

    JDTypeInfo typeInfo;
    // REVIEW: this check will produce false positive if the pointer type is in the template paramater.
    // but it will still give the right answer.
    // The only consequence is that  it won't use the cacheTypeInfoCache, but it will still be cached
    // in chakraCacheTypeInfoCAche   
    if (strchr(typeName, '*') != nullptr)
    {
        // A pointer type gives different answer with IDebugSymbol::GetTypeId, 
        // which doesn't allow the type to do ArrayElement access
        // Use ExtRemoteTyped to get the type info of the pointer of the type and then dereference.
        CHAR typeNameBuffer[1024];
        sprintf_s(typeNameBuffer, "(%s!%s*)@$extin", GetExtension()->GetModuleName(), typeName);
        ExtRemoteTyped result(typeNameBuffer, original);
        ExtRemoteTyped deref = result.Dereference();
        typeInfo = JDTypeInfo::FromExtRemoteTyped(deref);
    }
    else
    {
        std::string localTypeName = GetExtension()->GetModuleName();
        localTypeName += "!";
        localTypeName += typeName;
        typeInfo = typeCache.GetCachedTypeInfo(typeCache.AddTypeName(localTypeName.c_str()));
    }
    typeCache.chakraCacheTypeInfoCache[typeName] = typeInfo;
    return JDRemoteTyped(typeInfo, original, true);
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
    auto i = typeCache.vtableTypeInfoMap.find(vtbleAddr);
    if (i != typeCache.vtableTypeInfoMap.end())
    {
        result = JDRemoteTyped(i->second, objectAddress, true);
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

        JDTypeInfo typeInfo = typeCache.GetCachedTypeInfo(localTypeName);
        typeCache.vtableTypeInfoMap[vtbleAddr] = typeInfo;
        result = JDRemoteTyped(typeInfo, objectAddress, true);
        return true;
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

void JDTypeCache::EnsureOverrideAddedToVtableTypeNameMap()
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

            std::string crossSiteName = std::string("Js::CrossSiteObject<") + name + ">";
            std::string crossSiteVtableSymbolName = GetExtension()->GetRemoteVTableName(crossSiteName.c_str());
            if (GetExtension()->GetSymbolOffset(crossSiteVtableSymbolName.c_str(), true, &offset))
            {
                auto newString = new std::string(std::string(moduleName) + "!" + crossSiteName);
                vtableTypeNameMap[offset] = newString;
            }
        }
    }
}
char const * JDTypeCache::GetTypeNameFromVTablePointer(ULONG64 vtableAddr)
{
    EnsureOverrideAddedToVtableTypeNameMap();

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
    vtableTypeNameMap[vtableAddr] = nullptr;
    return nullptr;
}

void JDTypeCache::WarnICF()
{
    bool warned = false;
    for (auto i : vtableTypeNameMap)
    {
        if (i.second && HasMultipleSymbol(i.first))
        {
            warned = true;
            ExtWarn("WARNING: ICF vtable used: %p %s\n", i.first, i.second->c_str());
        }
    }
    if (!warned)
    {
        ExtOut("No ICF vtable detected");
    }
}