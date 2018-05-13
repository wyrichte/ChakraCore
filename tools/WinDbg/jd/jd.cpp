//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

// EXT_DECLARE_GLOBALS must be used to instantiate
// the framework's assumed globals.
EXT_DECLARE_GLOBALS();

EXT_CLASS_BASE::EXT_CLASS_BASE() :
    m_AuxPtrsFix16("Js::AuxPtrsFix<enum Js::FunctionProxy::AuxPointerType,16,3>",
        "Js::AuxPtrsFix<enum Js::FunctionProxy::AuxPointerType,16,1>", false),
    m_AuxPtrsFix32("Js::AuxPtrsFix<enum Js::FunctionProxy::AuxPointerType,32,6>",
        "Js::AuxPtrsFix<enum Js::FunctionProxy::AuxPointerType,32,3>", false)
{
    m_moduleName[0] = '\0';
    m_unitTestMode = false;
    m_uiServerString[0] = '\0';
    m_gcNS[0] = '\1';
    m_isCachedHasMemoryNS = false;
    m_hasMemoryNS = false;
}

static RemoteNullTypeHandler s_nullTypeHandler;
static RemoteSimpleTypeHandler s_simpleTypeHandler("Js::SimpleTypeHandler");
static RemoteSimpleTypeHandler s_simpleTypeHandler1_11("Js::SimpleTypeHandler<1>");
static RemoteSimpleTypeHandler s_simpleTypeHandler2_11("Js::SimpleTypeHandler<2>");

static RemotePathTypeHandler s_simplePathTypeHandler("Js::SimplePathTypeHandler");
static RemotePathTypeHandler s_pathTypeHandler;

static RemotePathTypeHandler s_simplePathTypeHandlerNoAttr("Js::SimplePathTypeHandlerNoAttr");  // RS4+
static RemotePathTypeHandler s_simplePathTypeHandlerWithAttr("Js::SimplePathTypeHandlerWithAttr");  // RS4+
static RemotePathTypeHandler s_pathTypeHandlerNoAttr("Js::PathTypeHandlerNoAttr");  // RS4+
static RemotePathTypeHandler s_pathTypeHandlerWithAttr("Js::PathTypeHandlerWithAttr");  // RS4+

static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler0_11("Js::SimpleDictionaryTypeHandlerBase<unsigned short,Js::PropertyRecord const *,0>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler1_11("Js::SimpleDictionaryTypeHandlerBase<unsigned short,Js::PropertyRecord const *,1>");
static RemoteSimpleDictionaryTypeHandler<INT> s_simpleDictionaryTypeHandlerLarge0_11("Js::SimpleDictionaryTypeHandlerBase<int,Js::PropertyRecord const *,0>");
static RemoteSimpleDictionaryTypeHandler<INT> s_simpleDictionaryTypeHandlerLarge1_11("Js::SimpleDictionaryTypeHandlerBase<int,Js::PropertyRecord const *,1>");

static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryUnorderedTypeHandler0_11("Js::SimpleDictionaryUnorderedTypeHandler<unsigned short,Js::PropertyRecord const *,0>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryUnorderedTypeHandler1_11("Js::SimpleDictionaryUnorderedTypeHandler<unsigned short,Js::PropertyRecord const *,1>");
static RemoteSimpleDictionaryTypeHandler<INT> s_simpleDictionaryUnorderedTypeHandlerLarge0_11("Js::SimpleDictionaryUnorderedTypeHandler<int,Js::PropertyRecord const *,0>");
static RemoteSimpleDictionaryTypeHandler<INT> s_simpleDictionaryUnorderedTypeHandlerLarge1_11("Js::SimpleDictionaryUnorderedTypeHandler<int,Js::PropertyRecord const *,1>");

static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryUnorderedStringTypeHandler0_11("Js::SimpleDictionaryUnorderedTypeHandler<unsigned short,Js::JavascriptString *,0>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryUnorderedStringTypeHandler1_11("Js::SimpleDictionaryUnorderedTypeHandler<unsigned short,Js::JavascriptString *,1>");
static RemoteSimpleDictionaryTypeHandler<INT> s_simpleDictionaryUnorderedStringTypeHandlerLarge0_11("Js::SimpleDictionaryUnorderedTypeHandler<int,Js::JavascriptString *,0>");
static RemoteSimpleDictionaryTypeHandler<INT> s_simpleDictionaryUnorderedStringTypeHandlerLarge1_11("Js::SimpleDictionaryUnorderedTypeHandler<int,Js::JavascriptString *,1>");

static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler0("Js::SimpleDictionaryTypeHandlerBase<unsigned short,0>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler1("Js::SimpleDictionaryTypeHandlerBase<unsigned short,1>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler9("Js::SimpleDictionaryTypeHandlerBase<unsigned short>"); // IE9
static RemoteDictionaryTypeHandler<USHORT> s_dictionaryTypeHandler0("Js::DictionaryTypeHandlerBase<unsigned short>");

static RemoteTypeHandler* s_typeHandlers[] =
{
    &s_nullTypeHandler,
    &s_simpleTypeHandler,
    &s_simpleTypeHandler1_11,
    &s_simpleTypeHandler2_11,
    &s_simplePathTypeHandler,
    &s_pathTypeHandler,

    &s_simplePathTypeHandlerNoAttr,
    &s_simplePathTypeHandlerWithAttr,
    &s_pathTypeHandlerNoAttr,
    &s_pathTypeHandlerWithAttr,

    &s_simpleDictionaryTypeHandler0_11,
    &s_simpleDictionaryTypeHandler1_11,
    &s_simpleDictionaryTypeHandlerLarge0_11,
    &s_simpleDictionaryTypeHandlerLarge1_11,
    &s_simpleDictionaryUnorderedTypeHandler0_11,
    &s_simpleDictionaryUnorderedTypeHandler1_11,
    &s_simpleDictionaryUnorderedTypeHandlerLarge0_11,
    &s_simpleDictionaryUnorderedTypeHandlerLarge1_11,
    &s_simpleDictionaryUnorderedStringTypeHandler0_11,
    &s_simpleDictionaryUnorderedStringTypeHandler1_11,
    &s_simpleDictionaryUnorderedStringTypeHandlerLarge0_11,
    &s_simpleDictionaryUnorderedStringTypeHandlerLarge1_11,
    &s_simpleDictionaryTypeHandler0,
    &s_simpleDictionaryTypeHandler1,
    &s_simpleDictionaryTypeHandler9, // IE9
    &s_dictionaryTypeHandler0
};

HRESULT EXT_CLASS_BASE::PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, REFIID iid, LPVOID* ppunk)
{
    typedef HRESULT(STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);

    HRESULT hr = NOERROR;
    CComPtr <IClassFactory> pClassFactory;
    FN_DllGetClassObject pProc = NULL;

    HINSTANCE hInstance = LoadLibraryEx(strModule, NULL, 0);
    IfNullGo(hInstance, E_FAIL);
    IfNullGo(pProc = (FN_DllGetClassObject)GetProcAddress(hInstance, "DllGetClassObject"), E_FAIL);
    IfFailGo(pProc(rclsid, __uuidof(IClassFactory), (LPVOID*)&pClassFactory));
    IfFailGo(pClassFactory->CreateInstance(NULL, iid, ppunk));

Error:
    return hr;
}

void EXT_CLASS_BASE::IfFailThrow(HRESULT hr, PCSTR msg)
{
    if (FAILED(hr))
    {
        ThrowStatus(hr, "%s", msg ? msg : "");
    }
}

bool EXT_CLASS_BASE::PreferDML()
{
    ULONG ulOptions = 0;
    return SUCCEEDED(m_Control->GetEngineOptions(&ulOptions)) && ((ulOptions & DEBUG_ENGOPT_PREFER_DML) != 0);
}

void
EXT_CLASS_BASE::OnSessionInaccessible(ULONG64)
{
    this->ClearCache();
}

bool EXT_CLASS_BASE::IsJScript9()
{
    return _stricmp(GetModuleName(), "jscript9") == 0;
}

// Get cached JS module name
PCSTR EXT_CLASS_BASE::GetModuleName()
{
    if (this->inMPHCmd)
    {
        return this->memGCModule;
    }

    if (m_moduleName[0] == '\0') {
        ULONG index = DEBUG_ANY_ID;
        ULONG64 base;
        IfFailThrow(FindJScriptModuleByName(m_Symbols, &index, &base),
            "Failed to find the jscript module in the process");

        if (FAILED(m_Symbols2->GetModuleNameString(
            DEBUG_MODNAME_MODULE, // get module name
            DEBUG_ANY_ID,         // use base
            base,
            m_moduleName,
            _countof(m_moduleName),
            NULL))) {
            ThrowLastError("Failed to get module name");
        }
    }

    return m_moduleName;
}

bool EXT_CLASS_BASE::HasMemoryNS()
{
    if (m_isCachedHasMemoryNS)
    {
        return m_hasMemoryNS;
    }

    if (IsJScript9())
    {
        m_hasMemoryNS = false;
        m_isCachedHasMemoryNS = true;
        return false;
    }

    const char* moduleName = GetModuleName();

    if (HasType(moduleName, "Memory::Recycler"))
    {
        m_hasMemoryNS = true;
        m_isCachedHasMemoryNS = true;
    }
    else
    {
        if (HasType(moduleName, "Recycler"))
        {
            m_hasMemoryNS = false;
            m_isCachedHasMemoryNS = true;
        }
        else
        {
            this->Err("Cannot find Recycler type, do you have symbol loaded?\n");
        }
    }

    return m_hasMemoryNS;
}

bool EXT_CLASS_BASE::HasType(const char* moduleName, const char* typeName)
{
    char symRecyclerType[256];
    ULONG symRecyclerTypeId = 0;
    sprintf_s(symRecyclerType, "%s!%s", moduleName, typeName);
    return this->m_Symbols2->GetSymbolTypeId(symRecyclerType, &symRecyclerTypeId, NULL) == S_OK;
}

PCSTR EXT_CLASS_BASE::GetMemoryNS()
{
    if (m_gcNS[0] == '\1')
    {
        if (HasMemoryNS())
        {
            strcpy_s(m_gcNS, "Memory::");
        }
        else
        {
            strcpy_s(m_gcNS, "");
        }
    }

    return m_gcNS;
}

// Fill a symbol name with module name. The result string uses my shared buffer.
PCSTR EXT_CLASS_BASE::FillModule(PCSTR fmt)
{
    sprintf_s(m_fillModuleBuffer, fmt, GetModuleName());
    return m_fillModuleBuffer;
}

// Same as FillModule but fills module name at 2 locations in fmt string.
PCSTR EXT_CLASS_BASE::FillModule2(PCSTR fmt)
{
    sprintf_s(m_fillModuleBuffer, fmt, GetModuleName(), GetModuleName());
    return m_fillModuleBuffer;
}

PCSTR EXT_CLASS_BASE::FillModuleV(PCSTR fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    _vsnprintf_s(m_fillModuleBuffer, _countof(m_fillModuleBuffer), _TRUNCATE, fmt, argptr);
    return m_fillModuleBuffer;
}

PCSTR EXT_CLASS_BASE::FillModuleAndMemoryNS(PCSTR fmt)
{
    sprintf_s(m_fillModuleBuffer, fmt, GetModuleName(), GetMemoryNS());
    return m_fillModuleBuffer;
}

PCSTR EXT_CLASS_BASE::GetSmallHeapBucketTypeName()
{
    if (HasMemoryNS())
    {
        return FillModule("%s!Memory::HeapBucketT<Memory::SmallNormalHeapBlockT<SmallAllocationBlockAttributes> >");
    }
    else
    {
        return FillModule("%s!HeapBucketT<SmallNormalHeapBlock>");
    }
}

bool EXT_CLASS_BASE::PageAllocatorHasExtendedCounters()
{
    /* Some version doesn't have an accurate commit number on the page allocator
    if (!m_pageAllocatorHasExtendedCounters.HasValue())
    {
        ExtRemoteTyped fakePageAllocator(this->FillModuleV("(%s!%s *)0", this->GetModuleName(), this->GetPageAllocatorType()));
        m_pageAllocatorHasExtendedCounters = fakePageAllocator.HasField("reservedBytes");
    }
    return m_pageAllocatorHasExtendedCounters;
    */
    return false;
}

bool EXT_CLASS_BASE::IsJITServer()
{
    if (!m_isJITServer.HasValue())
    {
        ExtRemoteTyped isJITServer(this->FillModule("%s!JITManager::s_jitManager.m_isJITServer"));
        m_isJITServer = isJITServer.GetStdBool();
    }

    return m_isJITServer;
}

// Detect a feature (code change) by checking a symbol
void EXT_CLASS_BASE::DetectFeatureBySymbol(Nullable<bool>& feature, PCSTR symbol)
{
    if (!feature.HasValue())
    {
        ULONG symbolTypeId;
        feature = SUCCEEDED(m_Symbols->GetSymbolTypeId(symbol, &symbolTypeId, NULL)) ? true : false;
    }
}

JD_PRIVATE_COMMAND(tc,
    "Dumps the given thread context or the thread context for the current thread",
    "{;e,o,d=0;threadctx;Thread context address}")
{
    ULONG64 arg = GetUnnamedArgU64(0);
    ExtRemoteTyped threadContext;

    if (arg == 0)
    {
        RemoteThreadContext::PrintAll();
        return;
    }
    threadContext = ExtRemoteTyped(FillModule("(%s!ThreadContext*)@$extin"), arg);
    threadContext.OutFullValue();
}

// Get a pointer to InternalString content buffer.
ExtRemoteTyped EXT_CLASS_BASE::GetInternalStringBuffer(ExtRemoteTyped internalString)
{
    if (!m_usingWeakRef.HasValue())
    {
        m_usingWeakRef = internalString.HasField("m_offset");
    }

    if (m_usingWeakRef)
    {
        if (internalString.GetPtr() == NULL) // WeakRef, released
        {
            return internalString;
        }
        ULONG offset = internalString.Field("m_offset").GetUchar();
        return internalString.Field("m_content")[offset].GetPointerTo();
    }
    else
    {
        return internalString.Field("m_content");
    }
}

// Get a property name internal string buffer from a ThreadContext::propertyNameList entry.
ExtRemoteTyped EXT_CLASS_BASE::GetPropertyName(ExtRemoteTyped propertyNameListEntry)
{
    if (m_newPropertyMap)
    {
        ExtRemoteTyped propertyRecord = propertyNameListEntry.Field("value");
        return propertyRecord[1UL].GetPointerTo();
    }
    else
    {
        // Newer builds use RecyclerWeakReference, older ones (IE9 RTM) use InternalString.
        if (!m_usingWeakRef.HasValue())
        {
            m_usingWeakRef = propertyNameListEntry.HasField("strongRef");
        }

        if (m_usingWeakRef)
        {
            ExtRemoteTyped propertyRecord = ExtRemoteTyped("(Js::PropertyRecord*)@$extin",
                propertyNameListEntry.Field("strongRef").GetPtr());
            return propertyRecord[1UL].GetPointerTo();
        }
        else
        {
            return GetInternalStringBuffer(propertyNameListEntry);
        }
    }
}

//
// Get the value of PropertyIds::_none. As we adding InternalPropertyIds, the _none base value keeps changing.
// Use an internal hack to get it. Our library initialization adds JavascriptLibrary::s_propertyNames first.
// The first propertyNameList entry is for "" and contains the initial PropertyId that equals PropertyIds::_none.
//
ULONG EXT_CLASS_BASE::GetPropertyIdNone(ExtRemoteTyped& propertyNameListBuffer)
{
    if (!m_propertyIdNone.HasValue())
    {
        if (m_newPropertyMap)
        {
            ExtRemoteTyped propertyMapEntry = propertyNameListBuffer[0UL];
            ExtRemoteTyped propertyRecord = propertyMapEntry.Field("value");
            m_propertyIdNone = propertyRecord.Field("pid").GetUlong();
        }
        else
        {
            ExtRemoteTyped propertyNameListEntry = propertyNameListBuffer[0UL];

            // Newer builds use RecyclerWeakReference, older ones (IE9 RTM) use InternalString.
            if (!m_usingWeakRef.HasValue())
            {
                m_usingWeakRef = propertyNameListEntry.HasField("strongRef");
            }

            if (m_usingWeakRef)
            {
                ExtRemoteTyped propertyRecord = ExtRemoteTyped("(Js::PropertyRecord*)@$extin",
                    propertyNameListEntry.Field("strongRef").GetPtr());

                if (propertyRecord.Field("byteCount").GetUlong() == 0)
                {
                    m_propertyIdNone = propertyRecord.Field("pid").GetUlong(); // Return the PropertyId for ""
                }
                else
                {
                    ThrowLastError("Error: Can't figure out initial PropertyId. This extension needs udpate.");
                }
            }
            else
            {
                m_propertyIdNone = 1; // This was used prior to WeakRef (IE9)
            }
        }
    }

    return m_propertyIdNone;
}


EXT_CLASS_BASE::PropertyNameReader::PropertyNameReader(RemoteThreadContext threadContext)
{
    _maxBuiltIn = 0;
    if (threadContext.GetPtr() != 0)
    {
        if (!GetExtension()->m_newPropertyMap.HasValue())
        {

            // We're using the new property map logic if the propertyNameList isn't there.

            GetExtension()->m_newPropertyMap = !threadContext.GetExtRemoteTyped().HasField("propertyNameList");
        }

        if (GetExtension()->m_newPropertyMap)
        {
            ExtRemoteTyped propertyMap = threadContext.GetExtRemoteTyped().Field("propertyMap");
            m_buffer = propertyMap.Field("entries");
            m_count = propertyMap.Field("count").GetUlong();
            _none = GetExtension()->GetPropertyIdNone(m_buffer);
        }
        else
        {
            ExtRemoteTyped propertyNameList = threadContext.GetExtRemoteTyped().Field("propertyNameList");
            m_buffer = propertyNameList.Field("buffer");
            m_count = propertyNameList.Field("count").GetUlong();
            _none = GetExtension()->GetPropertyIdNone(m_buffer);
        }
    }
    else
    {
        m_count = 0;
        _none = 0;

        // Try to infer some info the slow way
        for (int i = 0; i < 20; i++)
        {
            ExtRemoteTyped prop(GetExtension()->FillModule("(%s!Js::PropertyIds::_E)@$extin"), i);
            if (strcmp(JDUtil::GetEnumString(prop), "_none") == 0)
            {
                _none = i;
                break;
            }
        }

        if (_none != 0)
        {
            // Binary search the Js::PropertyIds::_countJSOnlyProperty
            int maxCountJSOnlyProperty = 2000;
            int minCountJSOnlyProperty = _none + 1;
            while (maxCountJSOnlyProperty >= minCountJSOnlyProperty)
            {
                int midCountJSOnlyProperty = (maxCountJSOnlyProperty - minCountJSOnlyProperty) / 2 + minCountJSOnlyProperty;
                ExtRemoteTyped prop(GetExtension()->FillModule("(%s!Js::PropertyIds::_E)@$extin"), midCountJSOnlyProperty);

                char const * name = JDUtil::GetEnumString(prop);
                if (strcmp(name, "_countJSOnlyProperty") == 0)
                {
                    _maxBuiltIn = midCountJSOnlyProperty;
                    break;
                }

                if (strncmp(name, "0n", 2) == 0)
                {
                    maxCountJSOnlyProperty = midCountJSOnlyProperty - 1;
                }
                else
                {
                    minCountJSOnlyProperty = midCountJSOnlyProperty + 1;
                }
            }
        }
    }
}

ULONG64 EXT_CLASS_BASE::PropertyNameReader::GetNameByIndex(ULONG i)
{
    return GetExtension()->GetPropertyName(m_buffer[i]).GetPtr();
}

ULONG64 EXT_CLASS_BASE::PropertyNameReader::GetNameByPropertyId(ULONG propertyId)
{
    if (propertyId >= _none)
    {
        propertyId -= _none;
        if (propertyId < m_count)
        {
            return GetNameByIndex(propertyId);
        }
    }

    return 0;
}

std::string EXT_CLASS_BASE::PropertyNameReader::GetNameStringByPropertyId(ULONG propertyId)
{
    ULONG64 propertyNameAddress = this->GetNameByPropertyId(propertyId);
    if (propertyNameAddress == 0)
    {
        char buffer[30];

        // Either this is a JIT server or it is an invalid property id
        if (GetExtension()->IsJITServer())
        {
            if (propertyId < _maxBuiltIn)
            {
                return JDUtil::GetEnumString(ExtRemoteTyped(GetExtension()->FillModule("(%s!Js::PropertyIds::_E)@$extin"), propertyId));
            }
            sprintf_s(buffer, "<PropId %d>", propertyId);
        }
        else
        {
            sprintf_s(buffer, "<INVALID PropId %d>", propertyId);
        }
        return buffer;
    }

    ExtRemoteTyped fieldName = ExtRemoteTyped("(char16 *)@$extin", propertyNameAddress);
    wchar tempBuffer2[1024];
    char tempBuffer[1024];
    sprintf_s(tempBuffer, "%S", tempBuffer2);
    return tempBuffer;
}

JD_PRIVATE_COMMAND(prop,
    "Print property name",
    "{;e,o,d=0;propertyId;The propertyId to lookup}{;e,o,d=0;pointer;Script or Thread context to print property name}")
{
    ULONG propertyId = static_cast<ULONG>(GetUnnamedArgU64(0));
    ULONG64 pointer = GetUnnamedArgU64(1);

    RemoteThreadContext threadContext;
    if (pointer == 0)
    {
        threadContext = RemoteThreadContext::GetCurrentThreadContext();
    }
    else if (!RemoteThreadContext::TryGetThreadContextFromAnyContextPointer(pointer, threadContext))
    {
        Err("ERROR: Pointer %p is not a ThreadContext or a ScriptContext\n", pointer);
        ThrowCommandHelp();
    }

    PropertyNameReader propertyNameReader(threadContext);
    if (propertyId)
    {
        ULONG64 pName = propertyNameReader.GetNameByPropertyId(propertyId);
        pName ? Out("%mu\n", pName) : Out("\n");
    }
    else
    {
        Out("ThreadContext 0x%p\n", threadContext.GetPtr());
        Out("PropertyId PropertyName\n");
        Out("---------- ------------\n");
        for (ULONG i = 0; i < propertyNameReader.Count(); i++)
        {
            ULONG currentPropertyId = propertyNameReader.GetPropertyIdByIndex(i);
            ULONG64 pName = propertyNameReader.GetNameByIndex(i);
            pName ? Out("    0n%-4d %mu\n", currentPropertyId, pName) : Out("    0n%-4d\n", currentPropertyId);
        }
    }
}


bool EXT_CLASS_BASE::GetTaggedInt31Usage()
{
    if (!m_taggedInt31Usage.HasValue())
    {
        ULONG64 offset;
        m_taggedInt31Usage = FAILED(m_Symbols->GetOffsetByName(FillModule("%s!Js::TaggedInt::Divide"), &offset));
    }
    return m_taggedInt31Usage;
}


JD_PRIVATE_COMMAND(var,
    "Print var",
    "{s;b,o;printSlotIndex;Print slot index}"
    "{;s;var;Expression that evaluates to var}")
{
    ExtRemoteTyped varTyped(GetUnnamedArgStr(0));
    RemoteVar var = varTyped.GetPtr();
    var.Print(this->HasArg("s"), 0);
    g_Ext->Out("\n");
}

bool EXT_CLASS_BASE::GetUsingInlineSlots(ExtRemoteTyped& typeHandler)
{
    if (!m_usingInlineSlots.HasValue())
    {
        m_usingInlineSlots = typeHandler.HasField("inlineSlotCapacity");
    }
    return m_usingInlineSlots;
}

bool EXT_CLASS_BASE::InChakraModule(ULONG64 address)
{
    if (chakraModuleBaseAddress == 0)
    {
        ULONG moduleIndex = 0;
        if (FAILED(g_Ext->m_Symbols3->GetModuleByModuleName(GetExtension()->GetModuleName(), 0, &moduleIndex, &chakraModuleBaseAddress)))
        {
            g_Ext->Err("Unable to get range for module '%s'. Is Chakra loaded?\n", GetExtension()->GetModuleName());
        }

        IMAGEHLP_MODULEW64 moduleInfo;
        g_Ext->GetModuleImagehlpInfo(chakraModuleBaseAddress, &moduleInfo);
        chakraModuleEndAddress = chakraModuleBaseAddress + moduleInfo.ImageSize;
    }

    return (address >= chakraModuleBaseAddress && address < chakraModuleEndAddress);
}

bool EXT_CLASS_BASE::InEdgeModule(ULONG64 address)
{
    if (edgeModuleBaseAddress == 0)
    {
        char const * edgeModule = "edgehtml";
        if (_stricmp(GetExtension()->GetModuleName(), "jscript9") == 0)
        {
            edgeModule = "mshtml";
        }

        ULONG moduleIndex = 0;
        if (FAILED(g_Ext->m_Symbols3->GetModuleByModuleName(edgeModule, 0, &moduleIndex, &edgeModuleBaseAddress)))
        {
            // Don't error in this case. Assume no edge module.
            edgeModuleBaseAddress = 1; 
            edgeModuleEndAddress = 1;
            return false;
        }

        IMAGEHLP_MODULEW64 moduleInfo;
        g_Ext->GetModuleImagehlpInfo(edgeModuleBaseAddress, &moduleInfo);
        edgeModuleEndAddress = edgeModuleBaseAddress + moduleInfo.ImageSize;
    }

    return (address >= edgeModuleBaseAddress && address < edgeModuleEndAddress);
}


// Get VTable of a type
std::string EXT_CLASS_BASE::GetRemoteVTableName(PCSTR type)
{
    return std::string(m_moduleName) + "!" + type + "::`vftable'";
}

std::string EXT_CLASS_BASE::GetTypeNameFromVTable(PCSTR vtablename)
{
    const char * vftableSuffix = "::`vftable'";
    const size_t vftableSuffixLen = _countof("::`vftable'") - 1;
    size_t len = strlen(vtablename);
    if (len > vftableSuffixLen && strcmp(vtablename + len - vftableSuffixLen, vftableSuffix) == 0)
    {
        return std::string(vtablename).substr(0, len - vftableSuffixLen);
    }
    return std::string();
}

std::string EXT_CLASS_BASE::GetTypeNameFromVTable(ULONG64 vtableAddress)
{
    std::string vtablename = GetSymbolForOffset(vtableAddress);
    if (vtablename.empty())
    {
        return std::string();
    }
    return GetTypeNameFromVTable(vtablename.c_str());
}

// Get RemoteTypeHandler for a DynamicObject
RemoteTypeHandler* EXT_CLASS_BASE::GetTypeHandler(ExtRemoteTyped& typeHandler)
{
    if (m_typeHandlersByName.empty())
    {
        // Collect all known TypeHandlers.
        //
        // PERF: m_Symbols->GetOffsetByName is expensive, use GetNameByOffset and string search instead
        //

        DetectFeatureBySymbol(m_usingPropertyRecordInTypeHandlers, FillModule("%s!Js::BuiltInPropertyRecords"));
        for (int i = 0; i < _countof(s_typeHandlers); i++)
        {
            RemoteTypeHandler* currTypeHandler = s_typeHandlers[i];
            // The list includes symbols on all builds. Some are not available.
            m_typeHandlersByName[GetRemoteVTableName(currTypeHandler->GetName())] = currTypeHandler;
        }

        // for 64 bit
        m_typeHandlersByName[GetRemoteVTableName("Js::SimpleDictionaryTypeHandlerBase<unsigned short,Js::PropertyRecord const * __ptr64,0>")] = &s_simpleDictionaryTypeHandler0_11;
        m_typeHandlersByName[GetRemoteVTableName("Js::SimpleDictionaryTypeHandlerBase<unsigned short,Js::PropertyRecord const * __ptr64,1>")] = &s_simpleDictionaryTypeHandler1_11;
        m_typeHandlersByName[GetRemoteVTableName("Js::SimpleDictionaryTypeHandlerBase<int,Js::PropertyRecord const * __ptr64,0>")] = &s_simpleDictionaryTypeHandlerLarge0_11;
        m_typeHandlersByName[GetRemoteVTableName("Js::SimpleDictionaryTypeHandlerBase<int,Js::PropertyRecord const * __ptr64,1>")] = &s_simpleDictionaryTypeHandlerLarge1_11;

        m_typeHandlersByName[GetRemoteVTableName("Js::SimpleDictionaryUnorderedTypeHandler<unsigned short,Js::PropertyRecord const * __ptr64,0>")] = &s_simpleDictionaryUnorderedTypeHandler0_11;
        m_typeHandlersByName[GetRemoteVTableName("Js::SimpleDictionaryUnorderedTypeHandler<unsigned short,Js::PropertyRecord const * __ptr64,1>")] = &s_simpleDictionaryUnorderedTypeHandler1_11;
        m_typeHandlersByName[GetRemoteVTableName("Js::SimpleDictionaryUnorderedTypeHandler<int,Js::PropertyRecord const * __ptr64,0>")] = &s_simpleDictionaryUnorderedTypeHandlerLarge0_11;
        m_typeHandlersByName[GetRemoteVTableName("Js::SimpleDictionaryUnorderedTypeHandler<int,Js::PropertyRecord const * __ptr64,1>")] = &s_simpleDictionaryUnorderedTypeHandlerLarge1_11;
    }

    ULONG64 vtable = ExtRemoteTyped("(void**)@$extin", typeHandler.GetPtr()).Dereference().GetPtr();
    auto iter = m_typeHandlers.find(vtable);
    if (iter != m_typeHandlers.end())
    {
        iter->second->Set(m_moduleName, typeHandler);
        return iter->second;
    }

    char nameBuffer[255];
    ULONG nameSize;
    HRESULT hr = m_Symbols3->GetNameByOffset(vtable, nameBuffer, sizeof(nameBuffer), &nameSize, NULL);
    if (FAILED(hr))
    {
        return NULL;
    }

    auto iter2 = m_typeHandlersByName.find(std::string(nameBuffer));
    if (iter2 != m_typeHandlersByName.end())
    {
        // Found the name, add it to the vtable map, so we don't have to do GetNameByOffset again for this vtable
        m_typeHandlers[vtable] = iter2->second;
        iter2->second->Set(m_moduleName, typeHandler);
        return iter2->second;
    }

    return NULL;
}

void EXT_CLASS_BASE::PrintScriptContextUrl(RemoteScriptContext scriptContext, bool showAll, bool showLink)
{
    if (scriptContext.IsScriptContextActuallyClosed())
    {
        Out("C");
    }
    else if (scriptContext.IsClosed())
    {
        Out("M");
    }
    else
    {
        JDRemoteTyped hostScriptContext = scriptContext.GetHostScriptContext();
        if (hostScriptContext.GetPtr())
        {
            try
            {
                bool fPrimaryEngine;
                JDRemoteTyped fNonPrimaryEngine = hostScriptContext.Field("scriptSite").Field("scriptEngine").Field("fNonPrimaryEngine");
                if (strcmp(fNonPrimaryEngine.GetTypeName(), "int") == 0)
                {
                    fPrimaryEngine = strcmp(fNonPrimaryEngine.GetSimpleValue(), "0n0") == 0;
                }
                else
                {
                    fPrimaryEngine = fNonPrimaryEngine.GetStdBool() == false;
                }
                Out(fPrimaryEngine? "P" : "N");
            }
            catch (...)
            {
                Out("E");
            }
        }
    }

    Out(" ");
    if (showLink)
    {
        if (PreferDML())
        {
            Dml("<link cmd=\"?? (Js::ScriptContext*)0x%p\">0x%p</link>", scriptContext.GetPtr(), scriptContext.GetPtr());
        }
        else
        {
            Out("0x%p /*\"?? (Js::ScriptContext*)0x%p\" to display*/", scriptContext.GetPtr(), scriptContext.GetPtr());
        }
    }
    else
    {
        Out("0x%p", scriptContext.GetPtr());
    }
    Out(" ");

    if (showAll)
    {
        JDRemoteTyped javascriptLibrary = scriptContext.GetJavascriptLibrary();
        ExtRemoteTyped globalObject = javascriptLibrary.Field("globalObject").GetExtRemoteTyped();
        ULONG64 javascriptLibraryPtr = javascriptLibrary.GetPtr();
        if (showLink)
        {
            if (PreferDML())
            {
                Dml("<link cmd=\"?? (Js::JavascriptLibrary*)0x%p\">0x%p</link> <link cmd=\"!jd.traceroots 0x%p\">&gt;</link>", javascriptLibraryPtr, javascriptLibraryPtr, javascriptLibraryPtr);
            }
            else
            {
                Out("0x%p /*\"?? (Js::JavascriptLibrary*)0x%p\" to display*/ > /*\"!jd.traceroots 0x%p\"to display*/", javascriptLibraryPtr, javascriptLibraryPtr, javascriptLibraryPtr);
            }
        }
        else
        {
            Out("0x%p", javascriptLibraryPtr);
        }
        Out(" ");
        ULONG64 globalObjectPtr = globalObject.GetPtr();
        if (globalObjectPtr != 0)
        {
            if (showLink)
            {
                if (PreferDML())
                {
                    Dml("<link cmd=\"!jd.var 0x%p\">0x%p</link> <link cmd=\"!jd.traceroots 0x%p\">&gt;</link>", globalObjectPtr, globalObjectPtr, globalObjectPtr);
                }
                else
                {
                    Out("0x%p /*\"!jd.var 0x%p\" to display*/ > /*\"!jd.traceroots 0x%p\" to display*/", globalObjectPtr, globalObjectPtr, globalObjectPtr);
                }
            }
            else
            {
                Out("0x%p", globalObjectPtr);
            }
        }
        else
        {
            Out("          ");
            if (g_Ext->m_PtrSize != 4)
            {
                Out("         ");
            }
            if (showLink)
            {
                Out("  ");
            }
        }
        Out(" ");
    }

    JDRemoteTyped url = scriptContext.GetUrl();
    if (url.GetPtr() != NULL)
    {
        Out("%mu", url.GetPtr());
    }
    Out("\n");
}

void EXT_CLASS_BASE::PrintThreadContextUrl(RemoteThreadContext threadContext, bool showAll, bool showLink, bool isCurrentThreadContext)
{
    bool found = false;

    ULONG threadId = (ULONG)-1;
    ULONG id;
    if (threadContext.TryGetDebuggerThreadId(&id, &threadId))
    {
        Out("%3d", id);
    }
    else
    {
        Out("???");
    }
    Out(" ThreadId: %04x ", threadId);
    Out("ThreadContext: 0x%p Recycler: 0x%p", threadContext.GetPtr(), threadContext.GetRecycler().GetPtr());

    if (isCurrentThreadContext)
    {
        Out(" (current)");
    }
    Out("\n");

    char const * headerFormat;
    if (this->m_PtrSize == 4)
    {
        // 32-bit
        headerFormat = (showAll ? (showLink ? "  %-12s %-12s %-12s URL\n" : "  %-10s %-10s %-10s URL\n") : "  %-12s URL\n");
    }
    else
    {
        // 64-bit
        headerFormat = (showAll ? (showLink ? "  %-18s %-20s %-20s URL\n" : "  %-18s %-18s %-18s URL\n") : "  %-18s URL\n");
    }

    ULONG scriptContextCount = 0;
    threadContext.ForEachScriptContext([&](RemoteScriptContext scriptContext)
    {
        if (!found)
        {
            found = true;
            Out(headerFormat, "ScrContext", "Library", "GlobalObj");
        }
        PrintScriptContextUrl(scriptContext, showAll, showLink);
        scriptContextCount++;
        return false;
    });

    Out("Count: %u\n", scriptContextCount);
}

void EXT_CLASS_BASE::PrintAllUrl(bool showAll, bool showLink)
{

    ULONG64 currentThreadContextPtr = 0;
    RemoteThreadContext remoteThreadContext;
    if (RemoteThreadContext::TryGetCurrentThreadContext(remoteThreadContext))
    {
        currentThreadContextPtr = remoteThreadContext.GetPtr();
    }

    RemoteThreadContext::ForEach([this, currentThreadContextPtr, showAll, showLink](RemoteThreadContext threadContext)
    {
        PrintThreadContextUrl(threadContext, showAll, showLink, threadContext.GetPtr() == currentThreadContextPtr);
        Out("--------------------------------------------------------------------\n");
        return false;
    });

}

JD_PRIVATE_COMMAND(url,
    "Print URLs",
    "{t;b;;Pointer is a thread context}"
    "{;e,o,d=0;pointer;Script or Thread context to print url}"
    "{a;b,o;Show associated pointers}"
    "{l;b,o;Show link}")
{
    ULONG64 pointer = GetUnnamedArgU64(0);
    const bool showAll = HasArg("a");
    const bool showLink = HasArg("l");

    if (pointer == 0)
    {
        PrintAllUrl(showAll, showLink);
        return;
    }
    RemoteThreadContext remoteThreadContext;
    if (RemoteThreadContext::TryGetThreadContextFromPointer(pointer, remoteThreadContext))
    {
        PrintThreadContextUrl(remoteThreadContext, showAll, showLink);
        return;
    }
    RemoteScriptContext remoteScriptContext;
    if (RemoteScriptContext::TryGetScriptContextFromPointer(pointer, remoteScriptContext))
    {
        PrintScriptContextUrl(remoteScriptContext, showAll, showLink);
        return;
    }

    Err("ERROR: Pointer %p is not a ThreadContext or ScriptContext\n", pointer);
    ThrowCommandHelp();
}


void EXT_CLASS_BASE::PrintScriptContextSourceInfos(RemoteScriptContext scriptContext, bool printOnlyCount, bool printSourceContextInfo)
{
    if (scriptContext.IsScriptContextActuallyClosed())
    {
        // Not considering this script context.
    }
    else if (scriptContext.IsClosed())
    {
        // Not considering this script context.
    }
    else
    {
        Out("ScriptContext : %p\n", scriptContext.GetPtr());
        scriptContext.ForEachUtf8SourceInfo([&](ULONG i, RemoteUtf8SourceInfo remoteUtf8SourceInfo)
        {
            Out("Utf8SourceInfo : [%d]\n", i);
            remoteUtf8SourceInfo.GetExtRemoteTyped().OutFullValue();
            if (printSourceContextInfo)
            {
                ExtRemoteTyped sourceContextInfo = remoteUtf8SourceInfo.GetSrcInfo().Field("sourceContextInfo").GetExtRemoteTyped();
                Out("SourceContextInfo : \n");
                sourceContextInfo.OutFullValue();
            }
            return false;
        });       
    }
    Out("\n");
}

void EXT_CLASS_BASE::PrintThreadContextSourceInfos(RemoteThreadContext threadContext, bool printOnlyCount, bool printSourceContextInfo, bool isCurrentThreadContext)
{
    Out("ThreadContext: 0x%p \n", threadContext.GetPtr());

    threadContext.ForEachScriptContext([&](RemoteScriptContext scriptContext)
    {
        PrintScriptContextSourceInfos(scriptContext, printOnlyCount, printSourceContextInfo);
        return false;
    });
}

void EXT_CLASS_BASE::PrintAllSourceInfos(bool printOnlyCount, bool printSourceContextInfo)
{
    ULONG64 currentThreadContextPtr = 0;
    RemoteThreadContext currentThreadContext;
    if (!RemoteThreadContext::TryGetCurrentThreadContext(currentThreadContext))
    {
        Out("Cannot find current thread context\n");
    }

    currentThreadContextPtr = currentThreadContext.GetPtr();
    RemoteThreadContext::ForEach([this, currentThreadContextPtr, printOnlyCount, printSourceContextInfo](RemoteThreadContext threadContext)
    {
        PrintThreadContextSourceInfos(threadContext, printOnlyCount, printSourceContextInfo, threadContext.GetPtr() == currentThreadContextPtr);
        Out("--------------------------------------------------------------------\n");
        return false;
    });

}

JD_PRIVATE_COMMAND(sourceInfos,
    "Print all utf8source info'es",
    "{;e,o,d=0;pointer;Script or Thread context to get source info'es}"
    "{c;b,o;;Print total count}"
    "{s;b,o;;Print sourceContextInfo as well}")
{
    ULONG64 pointer = GetUnnamedArgU64(0);
    const bool printOnlyCount = HasArg("c");
    const bool printSourceContextInfo = HasArg("s");

    if (pointer == 0)
    {
        PrintAllSourceInfos(printOnlyCount, printSourceContextInfo);
        return;
    }
    RemoteThreadContext remoteThreadContext;
    if (RemoteThreadContext::TryGetThreadContextFromPointer(pointer, remoteThreadContext))
    {
        PrintThreadContextSourceInfos(remoteThreadContext, printOnlyCount, printSourceContextInfo);
        return;
    }
    RemoteScriptContext remoteScriptContext;
    if (RemoteScriptContext::TryGetScriptContextFromPointer(pointer, remoteScriptContext))
    {
        PrintScriptContextSourceInfos(remoteScriptContext, printOnlyCount, printSourceContextInfo);
        return;
    }

    Err("ERROR: Pointer %p is not a ThreadContext or ScriptContext\n", pointer);
    ThrowCommandHelp();
}

JD_PRIVATE_COMMAND(count,
    "Count item in a list",
    "{;s;type;type of object}{;e;head;head of the list}{;s;next;next field name}")
{
    PCSTR type = GetUnnamedArgStr(0);
    ULONG64 head = GetUnnamedArgU64(1);
    PCSTR nextstr = GetUnnamedArgStr(2);

    char buffer[256];
    sprintf_s(buffer, "(%s *)@$extin", type);

    ExtRemoteTyped object(buffer, head);
    Out("%I64u\n", ExtRemoteTypedUtil::Count(object, nextstr));
}

JD_PRIVATE_COMMAND(dumpbuffer,
    "Outputs buffer content",
    "{;e,r;buffer; The buffer to dump}"\
    "{;e,o,d=0x10;length; Length of the buffer to dump}"\
    "{file;s,o;file; File to output the buffer, dump to console if missing}")
{
    ULONG64 address = GetUnnamedArgU64(0);
    ULONG length = (ULONG)GetUnnamedArgU64(1);
    ExtRemoteData ext(address, length);
    byte* buf = new byte[length];
    struct AutoCleanBuf
    {
        byte* buf;
        ~AutoCleanBuf() { delete[] buf; }
    } autoCleanBuf = { buf };
    ext.ReadBuffer(buf, length);

    if (HasArg("file"))
    {
        const char* filename = GetArgStr("file");
        FILE* outfile = fopen(filename, "wb");
        if (!outfile)
        {
            g_Ext->Out("Unable to open file %s for write\n", filename);
            return;
        }
        struct AutoClose
        {
            FILE* file;
            ~AutoClose() { fclose(file); }
        } autoClose = { outfile };
        fwrite(buf, sizeof(byte), length, outfile);
        int error = ferror(outfile);
        if (error != 0)
        {
            g_Ext->Out("There was an error while writing to file %s: %d\n", filename, error);
            return;
        }
    }
    else
    {
        for (ULONG i = 0; i < length; ++i)
        {
            UCHAR val = buf[i];
            g_Ext->Out("0x%x,", val);
        }
    }
}

class JDStackWalker
{
public:    
    JDStackWalker(ULONG64 initialRbp = 0);
    ~JDStackWalker();
    bool Next();
    CHAR * GetCurrentSymbol(ULONG64 * offset);
 
    
    ULONG64 GetRbp() const { return rbp; }
    ULONG64 GetRip() const { return rip; }
    ULONG64 GetAddressOfReturnAddress() { return nextFrameIndex < filled ? frames[nextFrameIndex].StackOffset - g_Ext->m_PtrSize : 0;  }

    void PrintFrameNumber(bool verbose);
private:
    ULONG GetFrameNumber() const { return frameNumber; }
    ULONG64 GetRsp() const { return rsp; }
    ULONG64 GetNextRip() const { return nextFrameIndex < filled ? frames[nextFrameIndex].InstructionOffset : 0; }

    ULONG64 rbp;
    ULONG64 rsp;
    ULONG64 rip;

    ULONG frameNumber;
    CHAR nameBuffer[1024];

    DEBUG_STACK_FRAME_EX * frames;
    ULONG filled;
    ULONG nextFrameIndex;

    static const unsigned int NumFrames = 0x1000;
};

JDStackWalker::JDStackWalker(ULONG64 initialRbp) :
    frames(nullptr)
{
    frameNumber = (ULONG)-1;
    nextFrameIndex = 0;
    ULONG64 startRsp;
    ULONG64 startRbp;
    ULONG64 startRip;

    if (initialRbp == 0)
    {
        HRESULT hr = g_Ext->m_Registers->GetStackOffset(&startRsp);
        if (FAILED(hr))
        {
            g_Ext->ThrowLastError("Unable to load rsp\n");
        }
        hr = g_Ext->m_Registers->GetFrameOffset(&startRbp);
        if (FAILED(hr))
        {
            g_Ext->ThrowLastError("Unable to load rbp\n");
        }
        hr = g_Ext->m_Registers->GetInstructionOffset(&startRip);
        if (FAILED(hr))
        {
            g_Ext->ThrowLastError("Unable to load rip\n");
        }
    }
    else
    {
        startRbp = initialRbp;
        startRsp = 0;
        startRip = 0;
    }
    
    frames = new DEBUG_STACK_FRAME_EX[NumFrames];
    HRESULT hr = g_Ext->m_Control5->GetStackTraceEx(startRbp, startRsp, startRip, frames, NumFrames, &filled);
    if (FAILED(hr) || filled == 0)
    {
        g_Ext->ThrowLastError("Unable to get stack frames");
    }
}

JDStackWalker::~JDStackWalker()
{
    if (frames)
    {
        delete[] frames;
    }
}

bool JDStackWalker::Next()
{
    if (nextFrameIndex >= filled)
    {
        return false;
    }

    rsp = frames[nextFrameIndex].StackOffset;
    rbp = frames[nextFrameIndex].FrameOffset;
    rip = frames[nextFrameIndex].InstructionOffset;    
    frameNumber++;
    nextFrameIndex++;
    return true;
}

CHAR * JDStackWalker::GetCurrentSymbol(ULONG64 *offset)
{
    ULONG nameSize;
    if (SUCCEEDED(g_Ext->m_Symbols->GetNearNameByOffset(
        rip,
        0,
        nameBuffer,
        sizeof(nameBuffer),
        &nameSize,
        offset)))
    {
        return nameBuffer;
    }
    return nullptr;
}

void JDStackWalker::PrintFrameNumber(bool verbose)
{
    if (GetExtension()->PreferDML())
    {
        g_Ext->Dml("<link cmd=\".frame 0x%02x\">%02x</link>", this->GetFrameNumber(), this->GetFrameNumber());
    }
    else
    {
        g_Ext->Out("%02x", this->GetFrameNumber());
    }
    if (verbose)
    {
        g_Ext->Out(" %p", this->GetRsp());
        g_Ext->Out(" %p", this->GetNextRip());
    }
}

static void DumpArgs(ULONG callCount, ULONG64 addressOfArguments)
{
    if (callCount == 0)
    {
        return;
    }

    const ULONG ptrSize = g_Ext->m_PtrSize;
    ExtRemoteData thisArg(addressOfArguments, ptrSize);
    RemoteVar thisVar(thisArg.GetPtr());
    if (!thisVar.IsUndefined())
    {
        g_Ext->Out(" ");
        thisVar.PrintLink("this");
    }

    g_Ext->Out(" (");
    for (ULONG i = 1; i < callCount; i++)
    {
        if (i != 1)
        {
            g_Ext->Out(", ");
        }
        ExtRemoteData arg(addressOfArguments + ptrSize * i, ptrSize);
        RemoteVar argVar(arg.GetPtr());
        argVar.Print(false, -1);
    }
    g_Ext->Out(")");
}

static void DumpInlineeFrames(ULONG64 frameAddr, bool dumpArgs, bool verbose)
{
    JDRemoteTyped frame(ExtRemoteTyped(GetExtension()->FillModule("(%s!InlinedFrameLayout *)@$extin"), frameAddr));
    ULONG argCount = (ULONG)frame.Field("callInfo").BitField("Count").GetSizeT();
    if (argCount == 0)
    {
        return;
    }
    ULONG64 arguments = frameAddr + frame.Dereference().GetTypeSize();
    // REVIEW: Recursion
    DumpInlineeFrames(arguments + argCount * g_Ext->m_PtrSize, dumpArgs, verbose);

    g_Ext->Out("   ");
    if (verbose)
    {
        if (g_Ext->m_PtrSize == 8)
        {
            g_Ext->Out("%34s", "");
        }
        else
        {
            g_Ext->Out("%18s", "");
        }
    }

    g_Ext->Out("js!");
    RemoteScriptFunction(frame.Field("function")).PrintNameAndNumberWithLink();
    g_Ext->Out(" [JIT inlined]");

    if (dumpArgs)
    {
        DumpArgs(argCount, arguments);
    }
    g_Ext->Out("\n");
};

JD_PRIVATE_COMMAND(jstack,
    "Print JS Stack. This is untested, and works only if all modules in the stack have FPO turned off",
    "{v;b,o;verbose;Dump ChildEBP and RetAddr information}"
    "{all;b,o;all;Dump full mixed mode stack- useful if stack has JITted functions}"
    "{args;b,o;all;Dump arguments to script functions}"
    "{;e,o,d=0;rbp;starting rbp}")
{
    const bool verbose = HasArg("v");
    const bool dumpFull = HasArg("all");
    const bool dumpArgs = HasArg("args");

    RemoteThreadContext threadContext = RemoteThreadContext::GetCurrentThreadContext();
    if (threadContext.GetCallRootLevel() == 0)
    {
        this->ThrowLastError("Script not running");
    }
    RemoteInterpreterStackFrame interpreterStackFrame = threadContext.GetLeafInterpreterStackFrame();

    Out(" #");
    const int ptrSize = this->m_PtrSize;
    if (verbose)
    {
        if (ptrSize == 8)
        {
            Out(" ChildESP         RetAddr");
        }
        else
        {
            Out(" ChildESP RetAddr");
        }
    }
    Out("\n");

    JDRemoteTyped nativeLibraryEntryRecordCurr = threadContext.GetNativeLibraryEntryRecord().Field("head");
    ULONG64 nativeLibraryAddressOfReturnAddressCurr = nativeLibraryEntryRecordCurr.GetPtr() != 0 ? nativeLibraryEntryRecordCurr.Field("addr").GetPtr() : 0;
    JDStackWalker stackWalker(this->GetUnnamedArgU64(0));
    while (stackWalker.Next())
    {
        ULONG64 offset = 0;
        CHAR * nameBuffer = stackWalker.GetCurrentSymbol(&offset);

        if (!dumpFull && nativeLibraryAddressOfReturnAddressCurr != 0 && nativeLibraryAddressOfReturnAddressCurr == stackWalker.GetAddressOfReturnAddress())
        {
            JDRemoteTyped function = nativeLibraryEntryRecordCurr.Field("function");
            JDRemoteTyped name = nativeLibraryEntryRecordCurr.Field("name");
            ExtBuffer<WCHAR> buffer;
            PWCHAR functionName = name.Dereference().GetString(&buffer);
            stackWalker.PrintFrameNumber(verbose);
            Out(" js!");
            if (this->PreferDML())
            {
                Dml(_u("<link cmd=\"!jd.var 0x%p\">%s</link>"), function.GetPtr(), functionName);
            }
            else
            {
                Out(_u("%s /*\"!jd.var 0x%p\" to display*/"), functionName, function.GetPtr());
            }
            Out(" [Native]");

            if (dumpArgs)
            {
                ExtRemoteTyped callInfo = nativeLibraryEntryRecordCurr.Field("callInfo").GetExtRemoteTyped();
                ULONG callCount = callInfo.Field("Count").GetUlong();

                // Skip the return address, and the 2 arguments function and callInfo
                ULONG64 argLocation = nativeLibraryAddressOfReturnAddressCurr + g_Ext->m_PtrSize * 3;
                DumpArgs(callCount, argLocation);
            }

            Out("\n");
            nativeLibraryEntryRecordCurr = nativeLibraryEntryRecordCurr.Field("next");
            nativeLibraryAddressOfReturnAddressCurr = nativeLibraryEntryRecordCurr.GetPtr() != 0 ? nativeLibraryEntryRecordCurr.Field("addr").GetPtr() : 0;
            continue;
        }

        // Native stack frames
        if (nameBuffer && strncmp(nameBuffer, "js!", 3) != 0)
        {
            if (dumpFull)
            {
                stackWalker.PrintFrameNumber(verbose);
                Out(" %s+0x%x\n", nameBuffer, offset);
            }
            continue;
        }

        // Interpreter stack frames
        if (!interpreterStackFrame.IsNull() && interpreterStackFrame.GetReturnAddress() == stackWalker.GetRip())
        {
            stackWalker.PrintFrameNumber(verbose);

            Out(" js!");
            interpreterStackFrame.GetScriptFunction().PrintNameAndNumberWithLink();
            Out(" [");

            if (interpreterStackFrame.IsFromBailout())
            {
                Out("JIT, Bailout ");
            }
            if (PreferDML())
            {
                Dml("Interpreter <link cmd=\"?? (%s!Js::InterpreterStackFrame *)0x%p\">0x%p</link>]", this->GetModuleName(), interpreterStackFrame.GetAddress(), interpreterStackFrame.GetAddress());
            }
            else
            {
                Out("Interpreter 0x%p /*\"?? (%s!Js::InterpreterStackFrame *)0x%p\" to display*/]", interpreterStackFrame.GetAddress(), this->GetModuleName(), interpreterStackFrame.GetAddress());
            }
            
            if (dumpArgs)
            {
                ExtRemoteTyped callInfo(GetExtension()->FillModule("(%s!Js::CallInfo *)@$extin"), stackWalker.GetRbp() + ptrSize * 3);
                ULONG callCount = callInfo.Field("Count").GetUlong();
                DumpArgs(callCount, stackWalker.GetRbp() + ptrSize * 4);
            }
            Out("\n");

            interpreterStackFrame = interpreterStackFrame.GetPreviousFrame();
            continue;
        }

        // JIT'ed stack frames
        ExtRemoteData firstArg(stackWalker.GetRbp() + ptrSize * 2, ptrSize);
        char const * typeName;
        JDRemoteTyped firstArgCasted = JDRemoteTyped::FromPtrWithVtable(firstArg.GetPtr(), &typeName);
        bool isFunctionObject = false;
        if (typeName != nullptr && firstArgCasted.HasField("type"))
        {
           JDRemoteTyped type = firstArgCasted.Field("type");
            if (type.HasField("typeId") && ENUM_EQUAL(type.Field("typeId").GetSimpleValue(), TypeIds_Function))
            {
                RemoteScriptFunction function(firstArgCasted.GetExtRemoteTyped());
                ExtRemoteTyped callInfo(GetExtension()->FillModule("(%s!Js::CallInfo *)@$extin"), stackWalker.GetRbp() + ptrSize * 3);
                ULONG callFlags = callInfo.Field("Flags").GetUlong();
                
                // Js::CallFlags_InternalFrame is not available in jscript9's PDBs so we hardcode the value
                // by default here, and in the Chakra case, pick up the one from the PDB in case it's different
                // from the jscript9 value, since the latter is not likely to change
                ULONG64 callFlagsInternalFrameValue = 0x80;

                if (!IsJScript9())
                {
                    callFlagsInternalFrameValue = ExtRemoteTyped(GetExtension()->FillModule("(%s!Js::CallFlags_InternalFrame)")).GetUlong();
                }

                bool isInternalFrame = callFlags & callFlagsInternalFrameValue;

                RemoteEntryPoint entryPoint = function.GetFunctionBody().GetEntryPointFromNativeAddress(stackWalker.GetRip());
                if (entryPoint.GetPtr())
                {
                    if (entryPoint.HasInlinees())
                    {
                        DumpInlineeFrames(stackWalker.GetRbp() - entryPoint.GetFrameHeight(), dumpArgs, verbose);
                    }

                    stackWalker.PrintFrameNumber(verbose);

                    isFunctionObject = true;
                    Out(" js!");
                    function.PrintNameAndNumberWithLink();

                    if (isInternalFrame)
                    {
                        Out(" [JIT LoopBody #%u]\n", interpreterStackFrame.GetCurrentLoopNum());
                    }
                    else
                    {

                        Out(" [JIT]");
                        if (dumpArgs)
                        {
                            ULONG callCount = callInfo.Field("Count").GetUlong();
                            DumpArgs(callCount, stackWalker.GetRbp() + ptrSize * 4);
                        }
                        Out("\n");
                    }
                }
            }
        }
        if (!isFunctionObject && dumpFull)
        {
            stackWalker.PrintFrameNumber(verbose);
            if (nameBuffer)
            {
                Out(" %s+0x%x\n", nameBuffer, offset);
            }
            else
            {
                Out(" <unknown %p>\n", stackWalker.GetRip());
            }
        }
    }

    if (!interpreterStackFrame.IsNull())
    {
        Out("WARNING: Interpreter stack frame unmatched\n");
    }
}

JD_PRIVATE_COMMAND(drpids,
    "Dumps the pids referenced in scriptContext",
    "")
{
    RemoteThreadContext::ForEach([this](RemoteThreadContext threadContext)
    {
        threadContext.GetExtRemoteTyped().OutTypeName();
        Out(" ");
        threadContext.GetExtRemoteTyped().OutSimpleValue();
        Out("\n");

        threadContext.ForEachScriptContext([this, &threadContext](RemoteScriptContext scriptContext)
        {
            scriptContext.PrintReferencedPids();
            return false;
        });
        Out("\n");
        return false;
    });
}

JD_PRIVATE_COMMAND(stst,
    "Switch to script thread- if there's only one script thread, it switches to it. If there are multiple, it dumps them all",
    "")
{
    if (!RemoteThreadContext::HasThreadId())
    {
        ThrowLastError("ERROR: this command not supported in Win7/Win8");
    }

    ulong scriptThreadId = 0;
    uint numThreadContexts = RemoteThreadContext::PrintAll(&scriptThreadId);
    if (numThreadContexts == 0)
    {
        this->Out("No script threads were found");
    }
    else if (numThreadContexts == 1)
    {
        if (scriptThreadId != 0)
        {
            this->Execute("~%us", scriptThreadId);
        }
        else
        {
            this->Out("Unable to get debugger thread Id");
        }
    }

}

JD_PRIVATE_COMMAND(arrseg,
    "Dumps the segments of a JavascriptArray",
    "{;e,o,d=0;array address;array to dump segments for}")
{

    ULONG64 arg = GetUnnamedArgU64(0);
    ExtRemoteTyped arr(FillModule("(%s!Js::JavascriptArray*)@$extin"), arg);
    ExtRemoteTyped seg = arr.Field("head");
    while (seg.GetPtr())
    {
        uint32 left = seg.Field("left").GetUlong();
        uint32 length = seg.Field("length").GetUlong();
        void * segPtr = (void *)seg.GetPtr();
        if (PreferDML())
        {
            Dml("<link cmd=\"dt Js::SparseArraySegmentBase 0x%p\">%p</link>", segPtr, segPtr);
        }
        else
        {
            Out("%p /*\"dt Js::SparseArraySegmentBase 0x%p\" to display*/", segPtr, segPtr);
        }
        Out(": %08x - %08x\n", left, left + length - 1);
        seg = seg.Field("next");
    }
}


JD_PRIVATE_COMMAND(bv,
    "Dumps bit vector",
    "{;x;bitvector;expression of a bitvector}")
{

    PCSTR arg = GetUnnamedArgStr(0);
    ExtRemoteTyped input = ExtRemoteTyped(arg);
    ExtRemoteTyped curr = input.Field("head");
    bool seen = false;
    Out("[");
    while (curr.GetPtr() != 0)
    {
        ExtRemoteTyped word = curr.Field("data.word");

        ULONG size = word.GetTypeSize();
        ULONG64 wordValue = (size == 4) ? word.GetUlong() : word.GetUlong64();
        LONG offset = curr.Field("startIndex").GetLong();
        for (ULONG64 i = 0; i < size * 8; i++)
        {
            if (wordValue & ((ULONG64)1 << i))
            {
                if (!seen)
                {
                    seen = true;
                }
                else
                {
                    Out(",");
                }
                Out("%d", offset + i);
            }
        }
        curr = curr.Field("next");
    }
    Out("]\n");
}

JD_PRIVATE_COMMAND(jsdisp,
    "Dumps JavascriptDispatch",
    "")
{
    RemoteThreadContext threadContext = RemoteThreadContext::GetCurrentThreadContext();
    threadContext.ForEachScriptContext([this](RemoteScriptContext scriptContext)
    {

        JDRemoteTyped hostScriptContext = scriptContext.GetHostScriptContext();
        if (hostScriptContext.GetPtr())
        {
            ExtRemoteTyped javascriptDispatch = ExtRemoteTyped(this->FillModule("(%s!JavascriptDispatch *)0"));
            ULONG64 offset = javascriptDispatch.GetFieldOffset("linkList");
            ExtRemoteTyped head = hostScriptContext.Field("scriptSite").Field("javascriptDispatchListHead").GetPointerTo();
            ExtRemoteTyped curr = head.Field("Flink");

            while (curr.GetPtr() != head.GetPtr())
            {
                javascriptDispatch = ExtRemoteTyped(this->FillModule("(%s!JavascriptDispatch *)@$extin"), curr.GetPtr() - offset);
                Out("%p %p %d\n", javascriptDispatch.GetPtr(), javascriptDispatch.Field("scriptObject").GetPtr(), javascriptDispatch.Field("isGCTracked").GetBoolean());
                curr = curr.Field("Flink");
            }
        }
        return false;
    });
}

JD_PRIVATE_COMMAND(warnicf,
    "Check and warn about ICF for vtable we have used",
    "")
{
    this->typeCache.WarnICF();
}

#if ENABLE_UI_SERVER
JD_PRIVATE_COMMAND(uiserver,
    "Starts the UI Server that can by connected to through the browser  (INCOMPLETE)",
    "")
{
    if (m_uiServerString[0] == '\0')
    {
        GUID guid;
        ::CoCreateGuid(&guid); // Check result?

        GuidToString(guid, m_uiServerString, sizeof(m_uiServerString));

        const char serverFormatString[] = "npipe:Pipe=WebJD_%s,IcfEnable";
        const int bufferSize = sizeof(serverFormatString) + sizeof(m_uiServerString) + 1;
        char serverOptions[bufferSize];

        ::sprintf_s(serverOptions, bufferSize, serverFormatString, m_uiServerString);
        Out("Starting UI Server with connection string is %s\n", serverOptions);
        HRESULT hr = g_ExtInstancePtr->m_Client->StartServer(serverOptions);

        if (FAILED(hr))
        {
            Out("Failed to start UI server- HRESULT 0x%x\n", hr);
        }

        // TODO: Spawn proxy server
    }
}
#endif

namespace Output
{
    void Print(LPCWSTR format, ...)
    {
        va_list args;
        WCHAR* buffer;
        va_start(args, format);
        int len = _vscwprintf(format, args) + 1;
        buffer = (WCHAR*)malloc(len * sizeof(WCHAR));
        vswprintf_s(buffer, len, format, args);
        g_ExtInstancePtr->Out(buffer);
        free(buffer);
    }
}
