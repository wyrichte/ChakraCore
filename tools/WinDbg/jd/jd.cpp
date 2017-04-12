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
        "Js::AuxPtrsFix<enum Js::FunctionProxy::AuxPointerType,32,3>", false),
#ifdef JD_PRIVATE
   recyclerCachedData(this)
#endif
{
#ifdef JD_PRIVATE
    m_moduleName[0] = '\0';
    m_unitTestMode = false;
    m_uiServerString[0] = '\0';
    m_gcNS[0] = '\1';
    m_isCachedHasMemoryNS = false;
    m_hasMemoryNS = false;
    m_isOverrideAddedToVtableTypeNameMap = false;
#endif
}

EXT_COMMAND(ldsym,
    "Load JavaScript symbols",
    "{p;x;path;Path to jscript9 debugger helper}")
{
    // Find jscript9 module
    ULONG index = DEBUG_ANY_ID;
    ULONG64 base;
    IfFailThrow(FindJScriptModuleByName<JD_IS_PUBLIC>(m_Symbols, &index, &base),
        "Failed to find jscript9 module in the process");

    WCHAR moduleName[MAX_PATH], imageName[MAX_PATH];
    IfFailThrow(m_Symbols3->GetModuleNameStringWide(DEBUG_MODNAME_MODULE, index, base, moduleName, _countof(moduleName), NULL),
        "Failed to find jscript9 module name");
    if (HasArg("p"))
    {
        CA2W path = GetArgStr("p", false);
        wcscpy_s(imageName, path);
    }
    else
    {
        IfFailThrow(m_Symbols3->GetModuleNameStringWide(DEBUG_MODNAME_IMAGE, index, base, imageName, _countof(imageName), NULL),
            "Failed to find jscript9 module path");
    }
    Out(_u("Use %s\n"), imageName);

    const CLSID CLSID_JScript9DAC = { 0x197060cb, 0x5efb, 0x4a53, 0xb0, 0x42, 0x93, 0x9d, 0xbb, 0x31, 0x62, 0x7c };
    CComPtr<IScriptDAC> pDAC;
    IfFailThrow(PrivateCoCreate(imageName, CLSID_JScript9DAC, IID_PPV_ARGS(&pDAC)),
        "Failed to create jscript9 debug helper object; ensure the platform architecture of the debugger and jsd matches that of the process being debugged, and the version and path of jscript9 being loaded by jsd is correct.");

    CComPtr<ScriptDebugSite> pScriptDebugSite;
    IfFailThrow(ComObject<ScriptDebugSite>::CreateInstance(&pScriptDebugSite));
    IfFailThrow(pScriptDebugSite->Init(moduleName, this, m_Symbols, m_Symbols3, m_Data, m_Data4));

    IfFailThrow(pDAC->LoadScriptSymbols(pScriptDebugSite));
}

static RemoteNullTypeHandler s_nullTypeHandler;
static RemoteSimpleTypeHandler s_simpleTypeHandler;
static RemoteSimplePathTypeHandler s_simplePathTypeHandler;
static RemotePathTypeHandler s_pathTypeHandler;

static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler0_11("Js::SimpleDictionaryTypeHandlerBase<unsigned short,Js::PropertyRecord const *,0>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler1_11("Js::SimpleDictionaryTypeHandlerBase<unsigned short,Js::PropertyRecord const *,1>");
static RemoteSimpleDictionaryTypeHandler<INT> s_simpleDictionaryTypeHandlerLarge0_11("Js::SimpleDictionaryTypeHandlerBase<int,Js::PropertyRecord const *,0>");
static RemoteSimpleDictionaryTypeHandler<INT> s_simpleDictionaryTypeHandlerLarge1_11("Js::SimpleDictionaryTypeHandlerBase<int,Js::PropertyRecord const *,1>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler0("Js::SimpleDictionaryTypeHandlerBase<unsigned short,0>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler1("Js::SimpleDictionaryTypeHandlerBase<unsigned short,1>");
static RemoteSimpleDictionaryTypeHandler<USHORT> s_simpleDictionaryTypeHandler9("Js::SimpleDictionaryTypeHandlerBase<unsigned short>"); // IE9
static RemoteDictionaryTypeHandler<USHORT> s_dictionaryTypeHandler0("Js::DictionaryTypeHandlerBase<unsigned short>");

static RemoteTypeHandler* s_typeHandlers[] =
{
    &s_nullTypeHandler,
    &s_simpleTypeHandler,
    &s_simplePathTypeHandler,
    &s_pathTypeHandler,

    &s_simpleDictionaryTypeHandler0_11,
    &s_simpleDictionaryTypeHandler1_11,
    &s_simpleDictionaryTypeHandlerLarge0_11,
    &s_simpleDictionaryTypeHandlerLarge1_11,
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
        ThrowStatus(hr, msg);
    }
}

// ---- Begin jd private commands implementation --------------------------------------------------
#ifdef JD_PRIVATE
// ------------------------------------------------------------------------------------------------
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
#ifdef MPH_CMDS
    if (this->inMPHCmd)
    {
        return this->memGCModule;
    }
#endif 

    if (m_moduleName[0] == '\0') {
        ULONG index = DEBUG_ANY_ID;
        ULONG64 base;
        IfFailThrow(FindJScriptModuleByName<JD_IS_PUBLIC>(m_Symbols, &index, &base),
            "Failed to find jscript9 module in the process");

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

PCSTR EXT_CLASS_BASE::GetSmallHeapBlockTypeName()
{
    if (HasMemoryNS())
    {
        return FillModule("%s!Memory::SmallHeapBlockT<SmallAllocationBlockAttributes>");
    }
    else
    {
        return FillModule("%s!SmallHeapBlock");
    }
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

    if (arg != 0)
    {
        threadContext = ExtRemoteTyped(FillModule("(%s!ThreadContext*)@$extin"), arg);
    }
    else
    {
        ULONG threadId = 0;
        this->m_System4->GetCurrentThreadId(&threadId);
        Out("Current thread: %d\n", threadId);

        // @$teb resolves to the 64-bit TEB in the 64-bit process regardless of the bit-ness of the debuggee,
        // so don't rely on the TEB to get the threadContext.
        threadContext = RemoteThreadContext::GetCurrentThreadContext().GetExtRemoteTyped();
    }

    if (threadContext.GetPtr() == 0)
    {
        Out("ThreadContext not found on this thread- is Chakra loaded on this thread?\n");
    }
    else
    {
        threadContext.OutFullValue();
    }
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


EXT_CLASS_BASE::PropertyNameReader::PropertyNameReader(EXT_CLASS_BASE* ext, RemoteThreadContext threadContext)
{
    m_ext = ext;
    _maxBuiltIn = 0;
    if (threadContext.GetExtRemoteTyped().GetPtr() != 0)
    {
        if (!ext->m_newPropertyMap.HasValue())
        {

            // We're using the new property map logic if the propertyNameList isn't there.

            ext->m_newPropertyMap = !threadContext.GetExtRemoteTyped().HasField("propertyNameList");
        }

        if (ext->m_newPropertyMap)
        {
            ExtRemoteTyped propertyMap = threadContext.GetExtRemoteTyped().Field("propertyMap");
            m_buffer = propertyMap.Field("entries");
            m_count = propertyMap.Field("count").GetUlong();
            _none = m_ext->GetPropertyIdNone(m_buffer);
        }
        else
        {
            ExtRemoteTyped propertyNameList = threadContext.GetExtRemoteTyped().Field("propertyNameList");
            m_buffer = propertyNameList.Field("buffer");
            m_count = propertyNameList.Field("count").GetUlong();
            _none = m_ext->GetPropertyIdNone(m_buffer);
        }
    }
    else
    {
        m_count = 0;
        _none = 0;

        // Try to infer some info the slow way
        for (int i = 0; i < 20; i++)
        {
            ExtRemoteTyped prop(ext->FillModule("(%s!Js::PropertyIds::_E)@$extin"), i);
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
                ExtRemoteTyped prop(ext->FillModule("(%s!Js::PropertyIds::_E)@$extin"), midCountJSOnlyProperty);

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
    return m_ext->GetPropertyName(m_buffer[i]).GetPtr();
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
        if (m_ext->IsJITServer())
        {
            if (propertyId < _maxBuiltIn)
            {
                return JDUtil::GetEnumString(ExtRemoteTyped(m_ext->FillModule("(%s!Js::PropertyIds::_E)@$extin"), propertyId));
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
    "{;e,o,d=0;propertyId;The propertyId to lookup}{t;b;;Pointer is a thread context}{;e,o,d=0;pointer;Script or Thread context to print url}")
{
    ULONG propertyId = static_cast<ULONG>(GetUnnamedArgU64(0));
    ULONG64 pointer = GetUnnamedArgU64(1);

    ExtRemoteTyped threadContext;
    if (pointer == 0)
    {
        threadContext = RemoteThreadContext::GetCurrentThreadContext().GetExtRemoteTyped();
    }
    else if (HasArg("t"))
    {
        threadContext = ExtRemoteTyped(FillModule("(%s!ThreadContext*)@$extin"), pointer);
    }
    else
    {
        threadContext = ExtRemoteTyped(FillModule("(%s!Js::ScriptContext*)@$extin"), pointer).Field("threadContext");
    }

    PropertyNameReader propertyNameReader(this, threadContext);
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


EXT_CLASS_BASE::TaggedIntUsage EXT_CLASS_BASE::GetTaggedIntUsage()
{
    if (!m_taggedIntUsage.HasValue())
    {
        ULONG64 offset;
        m_taggedIntUsage = SUCCEEDED(m_Symbols->GetOffsetByName(FillModule("%s!Js::TaggedInt::Divide"), &offset)) ?
        TaggedInt_TaggedInt : TaggedInt_Int31;
    }
    return m_taggedIntUsage;
}

bool EXT_CLASS_BASE::IsInt31Var(ULONG64 var, int* value)
{
    if (!DoInt32Var())
    {
        if (GetTaggedIntUsage() == TaggedInt_Int31) // IE9
        {
            if (var & 1)
            {
                *value = ((int)var) >> 1;
                return true;
            }
        }
    }
    return false;
}
bool EXT_CLASS_BASE::IsTaggedIntVar(ULONG64 var, int* value)
{
    if (GetTaggedIntUsage() == TaggedInt_TaggedInt)
    {
        if (DoInt32Var())
        {
            if ((var >> 48) == 1)
            {
                *value = (int)var;
                return true;
            }
        }
        else
        {
            if (var & 1)
            {
                *value = ((int)var) >> 1;
                return true;
            }
        }
    }

    return false;
}
bool EXT_CLASS_BASE::IsFloatVar(ULONG64 var, double* value)
{
    if (DoFloatVar())
    {
        if ((uint64)var >> 50)
        {
            *(uint64*)value = var ^ FloatTag_Value;
            return true;
        }
    }
    return false;
}

JD_PRIVATE_COMMAND(var,
    "Print var",
    "{;s;var;Expresion that evaluates to var}")
{
    ExtRemoteTyped varTyped(GetUnnamedArgStr(0));
    ULONG64 var = varTyped.GetPtr();

    PrintVar(var, 0);
}

void EXT_CLASS_BASE::PrintVar(ULONG64 var, int depth)
{
    int intValue;
    double dblValue;
    if (IsInt31Var(var, &intValue)) {
        Out("Int31 0n%d\n", intValue);
        return;
    } else if (IsTaggedIntVar(var, &intValue)) {
        Out("TaggedInt 0n%d\n", intValue);
        return;
    } else if (IsFloatVar(var, &dblValue)) {
        Out("FloatVar %f\n", dblValue);
        return;
    }
    std::string className = GetTypeNameFromVTableOfObject(var);
    if (className.empty())
    {
        this->ThrowLastError("Pointer doesn't have a valid vtable");
    }

    ExtRemoteTyped obj(className.c_str(), var, true);
    ExtRemoteTyped typeId = obj.Field("type.typeId");

    const char* typeIdStr = JDUtil::GetEnumString(typeId);
    if (depth == 0)
    {
        std::string encodedClassName = JDUtil::EncodeDml(className.c_str());
        Dml("%s * <link cmd=\"dt %s 0x%p\">0x%p</link> (%s)", JDUtil::StripModuleName(encodedClassName.c_str()),
            encodedClassName.c_str(), var, var, typeIdStr);
        DumpPossibleExternalSymbol(obj, className.c_str());
        Out("\n");
    }       

    if(strcmp(typeIdStr, "TypeIds_Undefined") == 0)
    {
        Out("undefined\n");
        return; // done
    }
    else if (strcmp(typeIdStr, "TypeIds_Null") == 0)
    {
        Out("null\n");
        return; // done
    }
    else if (strcmp(typeIdStr, "TypeIds_Boolean") == 0)
    {        
        Out(obj.Field("value").GetW32Bool() ? "true\n" : "false\n");
        return; // done
    }        
    else if (strcmp(typeIdStr, "TypeIds_Number") == 0)
    {        
        obj.Field("m_value").OutFullValue();
        return; // done
    }
    else if (strcmp(typeIdStr, "TypeIds_String") == 0)
    {        
        Out("\"%mu\"\n", obj.Field("m_pszValue").GetPtr());
        return; // done
    }
    else if (strcmp(typeIdStr, "TypeIds_StringObject") == 0)
    {        
        if (depth == 0)
        {            
            ExtRemoteTyped value = obj.Field("value");
            PrintSimpleVarValue(value);
        }
        else
        {
            PrintSimpleVarValue(obj);
        }
    }
    else if (strcmp(typeIdStr, "TypeIds_Function") == 0)
    {        
        if (depth == 0)
        {           
            RemoteFunctionInfo functionInfo(obj.Field("functionInfo"));
            if (functionInfo.HasBody())
            {
                RemoteFunctionBody functionBody = functionInfo.GetFunctionBody();
                Out(_u("  [FunctionBody] "));
                functionBody.PrintNameAndNumberWithLink(this);
                Out(_u(" "));
                functionBody.PrintByteCodeLink(this);
                Out("\n");
            }
            else
            {
                std::string symbol = GetSymbolForOffset(this, functionInfo.GetOriginalEntryPoint());
                if (!symbol.empty())
                {
                    Out("  [NativeEntry] %s", symbol.c_str());
                }
                else
                {
                    obj.Field("functionInfo").OutFullValue();
                }
            }
        }
        else
        {
            PrintSimpleVarValue(obj);
        }
    }
    else if (strcmp(typeIdStr, "TypeIds_Array") == 0)
    {        
        if (depth == 0)
        {
            obj.Field("head").OutFullValue();
        }
        else
        {
            PrintSimpleVarValue(obj);
        }
    }
    else
    {        
        if (depth != 0)
        {
            PrintSimpleVarValue(obj);
        }
    }

    if (depth == 0)
    {
        ExtRemoteTyped prototype = obj.Field("type.prototype");
        Out("\n[prototype] ");
        PrintSimpleVarValue(prototype);
        Out("[properties] ");
        PrintProperties(var, depth + 1);
    }
}

void EXT_CLASS_BASE::PrintSimpleVarValue(ExtRemoteTyped& obj)
{
    std::string typeName = this->GetTypeNameFromVTableOfObject(obj.GetPtr());
    PCSTR typeNameString;
    if (!typeName.empty())
    {
        typeNameString = JDUtil::StripModuleName(typeName.c_str());
    }
    else
    {
        typeNameString = obj.GetTypeName();
    }    
    std::string encodedTypeName = JDUtil::EncodeDml(typeNameString);
    Dml("<link cmd=\"!jd.var 0x%p\">%s * 0x%p</link> (%s)\n", obj.GetPtr(), encodedTypeName.c_str(), obj.GetPtr(),
        JDUtil::GetEnumString(obj.Field("type.typeId")));
}

class ObjectPropertyDumper : public ObjectPropertyListener
{
private:
    EXT_CLASS_BASE* m_ext;
    int m_depth;
    TypeHandlerPropertyNameReader* m_propertyNameReader;

public:
    ObjectPropertyDumper(EXT_CLASS_BASE* ext, int depth, TypeHandlerPropertyNameReader* propertyNameReader)
        : m_ext(ext), m_depth(depth), m_propertyNameReader(propertyNameReader)
    {
    }

    virtual void Enumerate(ExtRemoteTyped& name, ULONG64 value, ULONG64 value1) const override
    {
        ULONG64 pName = m_propertyNameReader->GetPropertyName(name);
        m_ext->PrintProperty(pName, value, value1, m_depth);
    }

    static void Enumerate(ExtRemoteTyped& obj, RemoteTypeHandler* typeHandler, TypeHandlerPropertyNameReader* reader, EXT_CLASS_BASE* ext, int depth)
    {
        ObjectPropertyDumper dumper(ext, depth, reader);
        typeHandler->EnumerateProperties(obj, dumper);
    }
};

void EXT_CLASS_BASE::PrintProperties(ULONG64 var, int depth)
{
    ExtRemoteTyped obj(FillModule("(%s!Js::DynamicObject*)@$extin"), var);
    ExtRemoteTyped type(FillModule("(%s!Js::DynamicType*)@$extin"), obj.Field("type").GetPtr());
    ExtRemoteTyped typeHandler = type.Field("typeHandler");

    RemoteTypeHandler* pRemoteTypeHandler = GetTypeHandler(obj, typeHandler);

    if (depth == 1)
    {
        std::string typeName;
        PCSTR typeHandlerName = "Unknown handler type";
        if (pRemoteTypeHandler)
        {
            typeHandlerName = pRemoteTypeHandler->GetName();
        }
        else
        {
            typeName = this->GetTypeNameFromVTableOfObject(typeHandler.GetPtr());
            if (!typeName.empty())
            {
                typeHandlerName = JDUtil::StripModuleName(typeName.c_str());
            }
        }
        Out("%s * ", pRemoteTypeHandler ? pRemoteTypeHandler->GetName() : typeHandlerName);
        typeHandler.OutSimpleValue();
        Out("\n");
    }

    if (pRemoteTypeHandler)
    {
        if (m_usingPropertyRecordInTypeHandlers)
        {
            TypeHandlerPropertyRecordNameReader reader;
            ObjectPropertyDumper::Enumerate(obj, pRemoteTypeHandler, &reader, this, depth);
        }
        else
        {
            TypeHandlerPropertyIdNameReader reader(this, GetThreadContextFromObject(obj));
            ObjectPropertyDumper::Enumerate(obj, pRemoteTypeHandler, &reader, this, depth);
        }
    }
}

bool EXT_CLASS_BASE::PrintProperty(ULONG64 name, ULONG64 value, ULONG64 value1, int depth)
{
    // indent
    for (int i = 0; i < depth; i++)
    {
        Out("   ");
    }

    Out("%-12mu : ", name);
    try
    {
        PrintVar(value, depth);
    }
    catch (ExtException)
    {
        Out("%p <ERROR: Not a valid Var>\n", value);
    }

    if (value1)
    {
        PrintProperty(name, value1, 0, depth);
    }
    return true;
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

static BOOL HasMultipleSymbolCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    (*(uint *)UserContext)++;
    return TRUE;
}

bool EXT_CLASS_BASE::HasMultipleSymbol(ULONG64 address)
{
    uint count = 0;
    ULONG64 handle;
    HRESULT hr = this->m_System4->GetCurrentProcessHandle(&handle);
    if (SUCCEEDED(hr))
    {
        SymEnumSymbolsForAddr((HANDLE)handle, address, HasMultipleSymbolCallback, &count);
        return count != 1;
    }
    return false;
}

char const * EXT_CLASS_BASE::GetTypeNameFromVTablePointer(ULONG64 vtableAddr)
{
    if (!m_isOverrideAddedToVtableTypeNameMap)
    {
        m_isOverrideAddedToVtableTypeNameMap = true;

        // In release build, various vtable is ICF'ed.  Here are the overrides

        // Js::LiteralString vtable may be ICF'ed with Js::BufferStringBuilder::WritableString or Js::SingleCharString.
        // It is preferable to always report Js::LiteralString for all 3.  Enter the offset into the vtableTypeNameMap
        std::string vtableSymbolName = this->GetRemoteVTableName("Js::LiteralString");
        ULONG64 offset;
        if (this->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
        {
            auto newString = new std::string(std::string(m_moduleName) + "!Js::LiteralString"); 
            vtableTypeNameMap[offset] = newString;
        }

        // Js::DynamicObject vtable may be ICF'ed with Js::WebAssemlbyInstance
        // It is preferable to always report Js::DynamicObject for both.  Enter the offset into the vtableTypeNameMap
        vtableSymbolName = this->GetRemoteVTableName("Js::DynamicObject");
        if (this->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
        {
            auto newString = new std::string(std::string(m_moduleName) + "!Js::DynamicObject");
            vtableTypeNameMap[offset] = newString;
        }

        // Js::JavascriptDate vtable may be ICF'ed with Js::JavascriptDateWinRTDate
        // It is preferable to always report Js::JavascriptDate for both.  Enter the offset into the vtableTypeNameMap
        vtableSymbolName = this->GetRemoteVTableName("Js::JavascriptDate");
        if (this->GetSymbolOffset(vtableSymbolName.c_str(), true, &offset))
        {
            auto newString = new std::string(std::string(m_moduleName) + "!Js::JavascriptDate"); 
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
        if (this->GetOffsetSymbol(vtableAddr, &vtableName, &displacement) && displacement == 0)
        {
            int len = (int)(strlen(vtableName.GetBuffer()) - strlen("::`vftable'"));
            if (len > 0 && strcmp(vtableName.GetBuffer() + len, "::`vftable'") == 0)
            {
                if (HasMultipleSymbol(vtableAddr))
                {
                    Warn("\rWARNING: ICF vtable used: %p %s                                          \n", vtableAddr, vtableName.GetBuffer());
                }

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

std::string EXT_CLASS_BASE::GetTypeNameFromVTable(ULONG64 vtableAddress)
{
    std::string vtablename = GetSymbolForOffset(this, vtableAddress);
    if (vtablename.empty())
    {
        return std::string();
    }
    return GetTypeNameFromVTable(vtablename.c_str());
}

std::string EXT_CLASS_BASE::GetTypeNameFromVTableOfObject(ULONG64 objectAddress)
{
    return GetTypeNameFromVTable(ExtRemoteData(objectAddress, this->m_PtrSize).GetPtr());
}

ULONG64 EXT_CLASS_BASE::GetRemoteVTable(PCSTR type)
{
    auto symbol = GetRemoteVTableName(type);

    ULONG64 vtable;
    if (FAILED(m_Symbols->GetOffsetByName(symbol.c_str(), &vtable)))
    {
        vtable = 0;
    }
    return vtable;
}


// Get RemoteTypeHandler for a DynamicObject
RemoteTypeHandler* EXT_CLASS_BASE::GetTypeHandler(ExtRemoteTyped& obj, ExtRemoteTyped& typeHandler)
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
    }

    ULONG64 vtable = ExtRemoteTyped("(void**)@$extin", typeHandler.GetPtr()).Dereference().GetPtr();
    auto iter = m_typeHandlers.find(vtable);
    if (iter != m_typeHandlers.end())
    {
        iter->second->Set(this, m_moduleName, typeHandler);
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
        iter2->second->Set(this, m_moduleName, typeHandler);
        return iter2->second;
    }

    return NULL;
}

ExtRemoteTyped EXT_CLASS_BASE::GetThreadContextFromObject(ExtRemoteTyped& obj)
{
    auto type = obj.Field("type");

    if (!m_usingLibraryInType.HasValue())
    {
        m_usingLibraryInType = type.HasField("javascriptLibrary");
    }

    return m_usingLibraryInType ?
        type.Field("javascriptLibrary.scriptContext.threadContext") : type.Field("globalObject.scriptContext.threadContext");
}

void EXT_CLASS_BASE::PrintScriptContextUrl(ExtRemoteTyped scriptContext, bool showAll, bool showLink)
{
    if (scriptContext.Field("isScriptContextActuallyClosed").GetStdBool())
    {
        Out("C");
    }
    else if (scriptContext.Field("isClosed").GetStdBool())
    {
        Out("M");
    }
    else
    {
        ExtRemoteTyped hostScriptContextField = scriptContext.Field("hostScriptContext");
        if (hostScriptContextField.GetPtr())
        {
            ExtRemoteTyped hostScriptContext = this->CastWithVtable(hostScriptContextField);
            try
            {
                bool fPrimaryEngine;
                ExtRemoteTyped fNonPrimaryEngine = hostScriptContext.Field("scriptSite").Field("scriptEngine").Field("fNonPrimaryEngine");
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
        Dml("<link cmd=\"?? (Js::ScriptContext*)0x%p\">0x%p</link>", scriptContext.GetPtr(), scriptContext.GetPtr());
    }
    else
    {
        scriptContext.OutSimpleValue();
    }
    Out(" ");

    if (showAll)
    {
        ExtRemoteTyped javascriptLibrary = scriptContext.Field("javascriptLibrary");
        ExtRemoteTyped globalObject = javascriptLibrary.Field("globalObject");
        if (showLink)
        {
            ULONG64 javascriptLibraryPtr = javascriptLibrary.GetPtr();
            Dml("<link cmd=\"?? (Js::JavascriptLibrary*)0x%p\">0x%p</link> <link cmd=\"!jd.traceroots 0x%p\">></link>", javascriptLibraryPtr, javascriptLibraryPtr, javascriptLibraryPtr);
        }
        else
        {
            javascriptLibrary.OutSimpleValue();
        }
        Out(" ");
        ULONG64 globalObjectPtr = globalObject.GetPtr();
        if (globalObjectPtr != 0)
        {
            if (showLink)
            {
                Dml("<link cmd=\"!jd.var 0x%p\">0x%p</link> <link cmd=\"!jd.traceroots 0x%p\">></link>", globalObjectPtr, globalObjectPtr, globalObjectPtr);
            }
            else
            {
                globalObject.OutSimpleValue();
            }
        }
        else
        {
            Out("          ");
            if (g_Ext->m_PtrSize != 4)
            {
                Out("        ");
            }
            if (showLink)
            {
                Out("  ");
            }
        }
        Out(" ");
    }

    if (scriptContext.HasField("url"))
    {
        Out("%mu", scriptContext.Field("url").GetPtr());
    }
    else
    {
        ExtRemoteTyped omWindowProxy;
        ExtRemoteTyped globalObject = scriptContext.Field("globalObject");
        ExtRemoteTyped directHostObject = globalObject.Field("directHostObject");

        try
        {
            if (directHostObject.GetPtr()) {
                // IE9 mode
                ExtRemoteTyped customExternalObject("(Js::CustomExternalObject*)@$extin", directHostObject.GetPtr());
                omWindowProxy = ExtRemoteTyped("(mshtml!COmWindowProxy**)@$extin",
                    customExternalObject[1UL].GetPointerTo().GetPtr()).Dereference();
            } else {
                // IE8 mode
                ExtRemoteTyped hostObject("(HostObject*)@$extin", globalObject.Field("hostObject").GetPtr());

                ExtRemoteTyped tearoffThunk("(mshtml!TEAROFF_THUNK*)@$extin",
                    hostObject.Field("hostDispatch").Field("refCountedHostVariant").Field("hostVariant").Field("varDispatch").Field("pdispVal").GetPtr());
                omWindowProxy = ExtRemoteTyped("(mshtml!COmWindowProxy*)@$extin",
                    tearoffThunk.Field("pvObject1").GetPtr());
            }

            omWindowProxy.Field("_pCWindow").Field("_pMarkup").Field("_pHtmCtx").Field("_pDwnInfo").Field("_cusUri").Field("m_LPWSTRProperty").OutSimpleValue();
        }
        catch (...)
        {

        }
    }
    Out("\n");
}

void EXT_CLASS_BASE::PrintThreadContextUrl(ExtRemoteTyped threadContext, bool showAll, bool showLink, bool isCurrentThreadContext)
{
    bool found = false;
    
    if (threadContext.HasField("currentThreadId"))
    {
        ULONG threadId = threadContext.Field("currentThreadId").GetUlong();
        ULONG id;
        if (FAILED(this->m_System4->GetThreadIdBySystemId(threadId, &id)))
        {
            Out("xx");
        }
        else
        {
            Out("%2x", id);
        }
        Out(" ThreadId: %04x ", threadId);
    }
    Out("ThreadContext: 0x%p Recycler : 0x%p", threadContext.GetPtr(), threadContext.Field("recycler").GetPtr());

    if (isCurrentThreadContext)
    {
        Out(" (current)");
    }
    Out("\n");

    ExtRemoteTyped scriptContext = threadContext.Field("scriptContextList");
    while (scriptContext.GetPtr())
    {
        if (!found)
        {
            found = true;
            if (scriptContext.GetTypeSize() == 4)
            {
                // 32-bit
                Out(showAll? (showLink? "  ScrContext %-12s %-12s URL\n" : "  ScrContext %-10s %-10s URL\n") :"  ScrContext URL\n", "Library", "GlobalObj");
            }
            else
            {
                // 64-bit
                Out(showAll ? (showLink ? "  ScrContext %-20s %-20s URL\n" : "  ScrContext %-18s %-18s URL\n") : "  ScrContext URL\n", "Library", "GlobalObj");
            }

        }
        PrintScriptContextUrl(scriptContext, showAll, showLink);
        scriptContext = scriptContext.Field("next");
    }
}

void EXT_CLASS_BASE::PrintAllUrl(bool showAll, bool showLink)
{

    ULONG64 currentThreadContextPtr = 0;
    RemoteThreadContext remoteThreadContext;
    if (RemoteThreadContext::TryGetCurrentThreadContext(remoteThreadContext))
    {
        currentThreadContextPtr = remoteThreadContext.GetExtRemoteTyped().GetPtr();
    }
    
    RemoteThreadContext::ForEach([this, currentThreadContextPtr, showAll, showLink](RemoteThreadContext threadContext)
    {
        ExtRemoteTyped threadContextExtRemoteTyped = threadContext.GetExtRemoteTyped();
        PrintThreadContextUrl(threadContextExtRemoteTyped, showAll, showLink, threadContextExtRemoteTyped.GetPtr() == currentThreadContextPtr);
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
    }
    else if (HasArg("t"))
    {
        PrintThreadContextUrl(ExtRemoteTyped("(ThreadContext *)@$extin", pointer), showAll, showLink);
    }
    else
    {
        PrintScriptContextUrl(ExtRemoteTyped("(Js::ScriptContext *)@$extin", pointer), showAll, showLink);
    }
}


void EXT_CLASS_BASE::PrintScriptContextSourceInfos(ExtRemoteTyped scriptContext, bool printOnlyCount, bool printSourceContextInfo)
{
    if (scriptContext.Field("isScriptContextActuallyClosed").GetStdBool())
    {
        // Not considering this script context.
    }
    else if (scriptContext.Field("isClosed").GetStdBool())
    {
        // Not considering this script context.
    }
    else
    {
        Out("ScriptContext : ");
        scriptContext.OutSimpleValue();
        Out("\n");
        if (scriptContext.HasField("sourceList"))
        {
            ExtRemoteTyped sourceList = scriptContext.Field("sourceList").Field("ptr");
            if (sourceList.GetPtr() != 0)
            {
                ExtRemoteTyped buffer = sourceList.Field("buffer");
                if (buffer.GetPtr() != 0)
                {
                    ULONG count = sourceList.Field("count").GetUlong();
                    Out("Total sourceInfos : %d\n", count);
                    if (!printOnlyCount)
                    {
                        for (ULONG i = 0; i < count; i++)
                        {
                            ExtRemoteTyped sourceInfoWeakRef = buffer[i];
                            if ((sourceInfoWeakRef.GetPtr() & 1) == 0)
                            {
                                ULONG64 strongRef = sourceInfoWeakRef.Field("strongRef").GetPtr();
                                if (strongRef != 0)
                                {
                                    ExtRemoteTyped utf8SourceInfo = ExtRemoteTyped("(Js::Utf8SourceInfo*)@$extin", strongRef);
                                    Out("Utf8SourceInfo : [%d]\n", i);
                                    utf8SourceInfo.OutFullValue();
                                    if (printSourceContextInfo)
                                    {
                                        ExtRemoteTyped sourceContextInfo = utf8SourceInfo.Field("m_srcInfo").Field("sourceContextInfo");
                                        Out("SourceContextInfo : \n");
                                        sourceContextInfo.OutFullValue();
                                    }
                                }
                            }
                        }
                    }
                }

            }
        }
    }
    Out("\n");
}

void EXT_CLASS_BASE::PrintThreadContextSourceInfos(ExtRemoteTyped threadContext, bool printOnlyCount, bool printSourceContextInfo, bool isCurrentThreadContext)
{
    Out("ThreadContext: 0x%p \n", threadContext.GetPtr());

    ExtRemoteTyped scriptContext = threadContext.Field("scriptContextList");
    while (scriptContext.GetPtr())
    {
        PrintScriptContextSourceInfos(scriptContext, printOnlyCount, printSourceContextInfo);
        scriptContext = scriptContext.Field("next");
    }
}

void EXT_CLASS_BASE::PrintAllSourceInfos(bool printOnlyCount, bool printSourceContextInfo)
{
    ULONG64 currentThreadContextPtr = 0;
    RemoteThreadContext currentThreadContext;
    if (!RemoteThreadContext::TryGetCurrentThreadContext(currentThreadContext))
    {
        Out("Cannot find current thread context\n");
    }

    currentThreadContextPtr = currentThreadContext.GetExtRemoteTyped().GetPtr();
    RemoteThreadContext::ForEach([this, currentThreadContextPtr, printOnlyCount, printSourceContextInfo](RemoteThreadContext threadContext)
    {
        ExtRemoteTyped threadContextExtRemoteTyped = threadContext.GetExtRemoteTyped();
        PrintThreadContextSourceInfos(threadContextExtRemoteTyped, printOnlyCount, printSourceContextInfo, threadContextExtRemoteTyped.GetPtr() == currentThreadContextPtr);
        Out("--------------------------------------------------------------------\n");
        return false;
    });

}

JD_PRIVATE_COMMAND(sourceInfos,
    "Print all utf8source info'es",
    "{t;b;;Pointer is a thread context}"
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
    }
    else if (HasArg("t"))
    {
        PrintThreadContextSourceInfos(ExtRemoteTyped("(ThreadContext *)@$extin", pointer), printOnlyCount, printSourceContextInfo);
    }
    else
    {
        PrintScriptContextSourceInfos(ExtRemoteTyped("(Js::ScriptContext *)@$extin", pointer), printOnlyCount, printSourceContextInfo);
    }
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


JD_PRIVATE_COMMAND(jstack,
    "Print JS Stack. This is untested, and works only if all modules in the stack have FPO turned off",
    "{v;b,o;verbose;Dump ChildEBP and RetAddr information}"
    "{all;b,o;all;Dump full mixed mode stack- useful if stack has JITted functions}"
    "{;e,o,d=0;rbp;starting rbp}")
{
    const bool verbose = HasArg("v");
    const bool dumpFull = HasArg("all");

    ULONG64 rsp = 0;
    ULONG64 rbp = this->GetUnnamedArgU64(0);
    ULONG64 rip = 0;
    HRESULT hr;

    if (rbp == 0)
    {
        hr = this->m_Registers->GetStackOffset(&rsp);
        if (FAILED(hr))
        {
            ThrowLastError("Unable to load rsp\n");
        }
        hr = this->m_Registers->GetFrameOffset(&rbp);
        if (FAILED(hr))
        {
            ThrowLastError("Unable to load rbp\n");
        }
        hr = this->m_Registers->GetInstructionOffset(&rip);
        if (FAILED(hr))
        {
            ThrowLastError("Unable to load rip\n");
        }
    }

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
    ULONG frameNumber = 0;
    while (rbp != 0)
    {
        if (rip != 0)
        {
            CHAR nameBuffer[1024];
            ULONG nameSize;
            ULONG64 offset = 0;

            hr = m_Symbols->GetNearNameByOffset(
                rip,
                0,
                nameBuffer,
                sizeof(nameBuffer),
                &nameSize,
                &offset);

            if (SUCCEEDED(hr))
            {
                DEBUG_STACK_FRAME frames[100];
                ULONG filled;
                hr = this->m_Control5->GetStackTrace(rbp, rsp, rip, frames, _countof(frames), &filled);
                if (FAILED(hr))
                {
                    ThrowLastError("Unable to get stack frame");
                }

                ULONG i = 0;
                ULONG64 lastBailoutLayoutX64 = 0;
                bool lastWasBailoutOnX64 = false;
                bool hasSeenBailoutOnX64 = false;
                bool firstBailoutOnX64 = false;
                while (i < filled)
                {
                    rsp = frames[i].StackOffset;
                    rbp = frames[i].FrameOffset;
                    rip = frames[i].InstructionOffset;
                    ULONG64 ripRet = frames[i].ReturnOffset;

                    hr = m_Symbols->GetNearNameByOffset(
                        rip,
                        0,
                        nameBuffer,
                        sizeof(nameBuffer),
                        &nameSize,
                        &offset);

                    if (FAILED(hr))
                    {
                        // HEURITICS fixing up unwinding to JIT Frame from BailOutRecord::Bailout
                        if (lastWasBailoutOnX64)
                        {
                            if (lastBailoutLayoutX64 != 0)
                            {
                                rbp = lastBailoutLayoutX64 - 2 * ptrSize;
                            }
                            else if (firstBailoutOnX64)
                            {
                                ExtRemoteTyped bailOutRecord = *ExtRemoteTyped(this->FillModule("(%s!BailOutRecord **)@$extin"), rsp);
                                rbp = bailOutRecord.Field("globalBailOutRecordTable.registerSaveSpace").ArrayElement(5).GetPtr();  // 6 is RBP reg number
                            }
                        }
                        break;
                    }

                    if (ptrSize == 8)
                    {
                        lastWasBailoutOnX64 = false;
                        if (strcmp(nameBuffer, this->FillModule("%s!BailOutRecord::BailOutHelper")) == 0 && i + 1 < filled)
                        {                            
                            lastBailoutLayoutX64 = ExtRemoteData(frames[i + 1].StackOffset, ptrSize).GetPtr();
                        }
                        else if (strcmp(nameBuffer, this->FillModule("%s!BailOutRecord::BailOut")) == 0)
                        {
                            lastWasBailoutOnX64 = true;
                            firstBailoutOnX64 = !hasSeenBailoutOnX64;
                            hasSeenBailoutOnX64 = true;
                        }
                    }


                    Out("%02x", frameNumber);
                    if (verbose)
                    {
                        Out(" %p", rsp);
                        Out(" %p", ripRet);
                    }
                    Out(" %s+0x%x\n", nameBuffer, offset);

                    frameNumber++;
                    i++;
                }
                if (i == filled)
                {
                    break;
                }
            }
        }


        ULONG64 ripRet;
        if (!interpreterStackFrame.IsNull() && interpreterStackFrame.GetReturnAddress() == rip)
        {
            // If the interpreterStackFrame is from bailout, assume the stackwalker is right.
            bool isFromBailout = interpreterStackFrame.IsFromBailout();
            if (!isFromBailout)
            {
                // This is an interpreter thunk frame
                if (ptrSize == 8)
                {
                    // AMD64 stack size is InterpreterThunkEmitter::StackAllocSize = 0x28 
                    // So return address is at 0x28 and we will just fake rbp as 0x28 - 0x8;
                    rbp = rsp + 0x20;
                }
                else if (rbp <= rsp)
                {
                    // HEURSTIC: Assume the callee save rbp if we dn't have a valid rbp
                    ExtRemoteData childEbp(rsp - 2 * ptrSize, ptrSize);
                    rbp = childEbp.GetPtr();
                }
            }

            ExtRemoteData returnAddress(rbp + ptrSize, ptrSize);
            ripRet = returnAddress.GetPtr();

            Out("%02x", frameNumber);
            if (verbose)
            {
                Out(" %p", rsp);
                Out(" %p", ripRet);
            }

            Out(" js!");
            interpreterStackFrame.GetScriptFunction().PrintNameAndNumberWithLink(this);
            Out(" [");
            if (isFromBailout)
            {
                Out("JIT, Bailout ");
            }
           
            Dml("Interpreter <link cmd=\"?? (%s!Js::InterpreterStackFrame *)0x%p\">0x%p</link>]", this->GetModuleName(), interpreterStackFrame.GetAddress(), interpreterStackFrame.GetAddress());
            Out("\n");

            interpreterStackFrame = interpreterStackFrame.GetPreviousFrame();
        }
        else
        {
            if (rbp <= rsp)
            {
                // HEURSTIC: Assume the callee save rbp if we dn't have a valid rbp
                ExtRemoteData previousChildFramePointer(rsp - 2 * ptrSize, ptrSize);
                rbp = previousChildFramePointer.GetPtr();
            }

            ExtRemoteData returnAddress(rbp + ptrSize, ptrSize);
            ripRet = returnAddress.GetPtr();
            
            Out("%02x", frameNumber);
            if (verbose)
            {
                Out(" %p", rsp);
                Out(" %p", ripRet);
            }

            
            ExtRemoteData firstArg(rbp + ptrSize * 2, ptrSize);
            char const * typeName;
            JDRemoteTyped firstArgCasted = this->CastWithVtable(firstArg.GetPtr(), &typeName);
            bool isFunctionObject = false;
            if (typeName != nullptr && firstArgCasted.HasField("type"))
            {
                JDRemoteTyped type = firstArgCasted.Field("type");
                if (type.HasField("typeId") && ENUM_EQUAL(type.Field("typeId").GetSimpleValue(), TypeIds_Function))
                {
                    isFunctionObject = true;
                    Out(" js!");
                    RemoteScriptFunction(firstArgCasted).PrintNameAndNumberWithLink(this);
                    Out(" [JIT]\n");
                }
            }
            if (!isFunctionObject)
            {
                Out(" <unknown>\n");
            }
        }

        ExtRemoteData childEbp(rbp, ptrSize);
        rsp = rbp + 2 * ptrSize;
        rbp = childEbp.GetPtr();
        rip = ripRet;
        frameNumber++;
    }
    if (!interpreterStackFrame.IsNull())
    {
        Out("WARNING: Interpreter stack frame unmatched");
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

    int numThreadContexts = 0;
    ulong scriptThreadId = 0;
    RemoteThreadContext::ForEach([&scriptThreadId, &numThreadContexts, this](RemoteThreadContext threadContext)
    {
        ulong threadContextThreadId = 0;

        ULONG64 threadContextAddress = threadContext.GetExtRemoteTyped().GetPtr();
        if (threadContext.TryGetDebuggerThreadId(&threadContextThreadId))
        {
            this->Dml("<link cmd=\"~%us\">Thread context: <b>%p</b></link>\n", threadContextThreadId, threadContextAddress);
        }
        else
        {
            this->Out("Thread context: %p\n", threadContextAddress);
        }

        numThreadContexts++;
        scriptThreadId = threadContextThreadId;
        return false;
    });

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
        Dml("<link cmd=\"dt Js::SparseArraySegmentBase 0x%p\">%p</link>", segPtr, segPtr, segPtr);		// I don't know why I need to pass it in 3 times, but it works :(
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

        ExtRemoteTyped hostScriptContextField = scriptContext.GetHostScriptContext();
        if (hostScriptContextField.GetPtr())
        {
            ExtRemoteTyped hostScriptContext = this->CastWithVtable(hostScriptContextField);
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

// ---- End jd private commands implementation ----------------------------------------------------
#endif //JD_PRIVATE
// ------------------------------------------------------------------------------------------------
