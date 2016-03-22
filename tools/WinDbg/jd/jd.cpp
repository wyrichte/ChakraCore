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

    char symRecyclerType[256];
    ULONG symRecyclerTypeId = 0;
    sprintf_s(symRecyclerType, "%s!Memory::Recycler", GetModuleName());
    if (this->m_Symbols2->GetSymbolTypeId(symRecyclerType, &symRecyclerTypeId, NULL) == S_OK)
    {
        m_hasMemoryNS = true;
        m_isCachedHasMemoryNS = true;
    }
    else
    {
        sprintf_s(symRecyclerType, "%s!Recycler", GetModuleName());
        if (this->m_Symbols2->GetSymbolTypeId(symRecyclerType, &symRecyclerTypeId, NULL) == S_OK)
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

EXT_CLASS_BASE::PropertyNameReader::PropertyNameReader(EXT_CLASS_BASE* ext, ExtRemoteTyped threadContext)
{
    m_ext = ext;

    if (!ext->m_newPropertyMap.HasValue())
    {

        // We're using the new property map logic if the propertyNameList isn't there.

        ext->m_newPropertyMap = !threadContext.HasField("propertyNameList");
    }

    if (ext->m_newPropertyMap)
    {
        ExtRemoteTyped propertyMap = threadContext.Field("propertyMap");
        m_buffer = propertyMap.Field("entries");
        m_count = propertyMap.Field("count").GetUlong();
        _none = m_ext->GetPropertyIdNone(m_buffer);
    }
    else
    {
        ExtRemoteTyped propertyNameList = threadContext.Field("propertyNameList");
        m_buffer = propertyNameList.Field("buffer");
        m_count = propertyNameList.Field("count").GetUlong();
        _none = m_ext->GetPropertyIdNone(m_buffer);
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

JD_PRIVATE_COMMAND(prop,
    "Print property name",
    "{;e,o,d=0;propertyId;The propertyId to lookup}{t;b;;Pointer is a thread context}{;e,o,d=0;pointer;Script or Thread context to print url}")
{
    ULONG propertyId = static_cast<ULONG>(GetUnnamedArgU64(0));
    ULONG64 pointer = GetUnnamedArgU64(1);

    ExtRemoteTyped threadContext;
    if (pointer == 0)
    {
        ExtRemoteTyped teb("(ntdll!_TEB*)@$teb");
        threadContext = RemoteThreadContext::GetThreadContextFromTeb(teb).GetExtRemoteTyped();
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
            ULONG propertyId = propertyNameReader.GetPropertyIdByIndex(i);
            ULONG64 pName = propertyNameReader.GetNameByIndex(i);
            pName ? Out("    0n%-4d %mu\n", propertyId, pName) : Out("    0n%-4d\n", propertyId);
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
    if (depth == 0)
    {
        Dml("%s * <link cmd=\"dt %s 0x%p\">0x%p</link> (%s)\n", JDUtil::StripModuleName(className.c_str()),
            className.c_str(), var, var, JDUtil::GetEnumString(typeId));
    }       

    switch (typeId.GetUlong())
    {
    case TypeIds_Undefined:
    {
        Out("undefined\n");
    }
        return; // done

    case TypeIds_Null:
    {
        Out("null\n");
    }
        return; // done

    case TypeIds_Boolean:
    {        
        Out(obj.Field("value").GetW32Bool() ? "true\n" : "false\n");
    }
        return; // done

    case TypeIds_Number:
    {        
        obj.Field("m_value").OutFullValue();
    }
        return; // done

    case TypeIds_String:
    {        
        Out("\"%mu\"\n", obj.Field("m_pszValue").GetPtr());        
    }
        return; // done

    case TypeIds_StringObject:
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
        break;

    case TypeIds_Function:
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
        break;

    case TypeIds_Array:
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
        break;
        
    default:
    {        
        if (depth != 0)
        {
            PrintSimpleVarValue(obj);
        }
    }
        break;
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
    Dml("<link cmd=\"!jd.var 0x%p\">%s * 0x%p</link> (%s)\n", obj.GetPtr(), typeNameString, obj.GetPtr(),
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
            static TypeHandlerPropertyRecordNameReader reader;
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

char const * EXT_CLASS_BASE::GetTypeNameFromVTablePointer(ULONG64 vtableAddr)
{
    auto i = vtableTypeNameMap.find(vtableAddr);
    if (i != vtableTypeNameMap.end())
    {
        return (*i).second->c_str();
    }

    ExtBuffer<char> vtableName;
    try
    {
        if (this->GetOffsetSymbol(vtableAddr, &vtableName))
        {
            int len = (int)(strlen(vtableName.GetBuffer()) - strlen("::`vftable'"));
            if (len > 0 && strcmp(vtableName.GetBuffer() + len, "::`vftable'") == 0)
            {
                vtableName.GetBuffer()[len] = '\0';

                auto newString = new std::string(vtableName.GetBuffer());
                // Actual type name in expression shouldn't have __ptr64 in them
                JDUtil::ReplaceString(*newString, " __ptr64", "");
                vtableTypeNameMap[vtableAddr] = newString;
                return newString->c_str();;
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

        DetectFeatureBySymbol(m_usingPropertyRecordInTypeHandlers, FillModule("%s!Js::BuiltInPropertyRecords"));
        for (int i = 0; i < _countof(s_typeHandlers); i++)
        {
            RemoteTypeHandler* typeHandler = s_typeHandlers[i];
            // The list includes symbols on all builds. Some are not available.
            m_typeHandlersByName[GetRemoteVTableName(typeHandler->GetName())] = typeHandler;
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

void EXT_CLASS_BASE::PrintScriptContextUrl(ExtRemoteTyped scriptContext)
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
                if (strcmp(hostScriptContext.Field("scriptSite").Field("scriptEngine").Field("fNonPrimaryEngine").GetSimpleValue(), "0n0") == 0)
                {
                    Out("P");
                }
                else
                {
                    Out("N");
                }
            }
            catch (...)
            {
                Out("E");
            }
        }
    }

    Out(" ");
    scriptContext.OutSimpleValue();
    Out(" ");

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

void EXT_CLASS_BASE::PrintThreadContextUrl(ExtRemoteTyped threadContext, bool isCurrentThreadContext)
{
    bool found = false;
    
    ULONG threadId = threadContext.Field("currentThreadId").GetUlong();
    ULONG id;
    if (FAILED(this->m_System4->GetThreadIdBySystemId(threadId, &id)))
    {
        Out("xx ");
    }
    else
    {
        Out("%2x", id);
    }
    Out(" ThreadId: %04x ThreadContext: 0x%p Recycler: 0x%p", threadId, threadContext.GetPtr(), 
        threadContext.Field("recycler").GetPtr());    
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
                Out("  ScrContext URL\n");
            }
            else
            {
                // 64-bit
                Out("  ScriptContext       URL\n");
            }

        }
        PrintScriptContextUrl(scriptContext);
        scriptContext = scriptContext.Field("next");
    }
}

void EXT_CLASS_BASE::PrintAllUrl()
{

    ULONG64 currentThreadContextPtr = 0;
    RemoteThreadContext remoteThreadContext;
    if (RemoteThreadContext::TryGetCurrentThreadContext(remoteThreadContext))
    {
        currentThreadContextPtr = remoteThreadContext.GetExtRemoteTyped().GetPtr();
    }
    
    RemoteThreadContext::ForEach([this, currentThreadContextPtr](RemoteThreadContext threadContext)
    {
        ExtRemoteTyped threadContextExtRemoteTyped = threadContext.GetExtRemoteTyped();
        PrintThreadContextUrl(threadContextExtRemoteTyped, threadContextExtRemoteTyped.GetPtr() == currentThreadContextPtr);
        Out("--------------------------------------------------------------------\n");
        return false;
    });

}

JD_PRIVATE_COMMAND(url,
    "Print URLs",
    "{t;b;;Pointer is a thread context}{;e,o,d=0;pointer;Script or Thread context to print url}")
{
    ULONG64 pointer = GetUnnamedArgU64(0);

    if (pointer == 0)
    {
        PrintAllUrl();
    }
    else if (HasArg("t"))
    {
        PrintThreadContextUrl(ExtRemoteTyped("(ThreadContext *)@$extin", pointer));
    }
    else
    {
        PrintScriptContextUrl(ExtRemoteTyped("(Js::ScriptContext *)@$extin", pointer));
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
                            ULONG64 strongRef = buffer[i].Field("strongRef").GetPtr();
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
    try
    {
        ExtRemoteTyped teb("(ntdll!_TEB*)@$teb");
        ExtRemoteTyped currentThreadContext = RemoteThreadContext::GetThreadContextFromTeb(teb).GetExtRemoteTyped();
        currentThreadContextPtr = currentThreadContext.GetPtr();
    }
    catch (...)
    {
        Out("Cannot find current thread context\n");
    }

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

void EXT_CLASS_BASE::PrintFrameNumberWithLink(uint frameNumber)
{
    Dml("<link cmd=\".frame %x\">%02x</link>", frameNumber, frameNumber);
}

JD_PRIVATE_COMMAND(jstack,
    "Print JS Stack. This is untested, and works only if all modules in the stack have FPO turned off",
    "{v;b,o;verbose;Dump ChildEBP and RetAddr information}"
    "{all;b,o;all;Dump full mixed mode stack- useful if stack has JITted functions}")
{
    const bool verbose = HasArg("v");
    const bool dumpFull = HasArg("all");

    ULONG64 ebp = 0;
    ULONG64 eip = 0;
    HRESULT hr = this->m_Registers->GetFrameOffset(&ebp); // do something with hr?
    hr = this->m_Registers->GetInstructionOffset(&eip);

    const int psize = this->m_PtrSize;
    RemoteThreadContext threadContext = RemoteThreadContext::GetCurrentThreadContext();
    if (threadContext.GetCallRootLevel() == 0)
    {
        this->ThrowLastError("Script not running");
    }
    RemoteInterpreterStackFrame interpreterStackFrame = threadContext.GetLeafInterpreterStackFrame();

    int frameNumber = 0;

    Out(" #");
    if (verbose)
    {
        Out(" ChildEBP RetAddr");
    }
    Out("\n");
    while (ebp != 0)
    {
        ExtRemoteData childEbp(ebp, psize);
        ExtRemoteData returnAddress(ebp + 4, psize);

        if (!interpreterStackFrame.IsNull() && interpreterStackFrame.GetReturnAddress() == eip)
        {
            PrintFrameNumberWithLink(frameNumber);
            if (verbose)
            {                
                Out(" %08x", ebp);
                Out(" %08x", eip);
            }
            Out(" js!");
            interpreterStackFrame.GetScriptFunction().PrintNameAndNumberWithLink(this);
            Out("\n");

            interpreterStackFrame = interpreterStackFrame.GetPreviousFrame();
        }
        else
        {
            ExtRemoteData firstArg(ebp + 8, psize);
            // TODO: JIT'ed code
            if (dumpFull)
            {
                PrintFrameNumberWithLink(frameNumber);
                if (verbose)
                {                    
                    Out(" %08x", ebp);
                    Out(" %08x", eip);
                }

                CHAR nameBuffer[1024];
                ULONG nameSize;
                ULONG64 offset;

                hr = m_Symbols->GetNearNameByOffset(
                    eip,
                    0,
                    nameBuffer,
                    sizeof(nameBuffer),
                    &nameSize,
                    &offset);
                
                Out(" %s\n", SUCCEEDED(hr)? nameBuffer: "<unknown>");
            }
        }
        eip = returnAddress.GetPtr();
        ebp = childEbp.GetPtr();
        frameNumber++;
    }
    if (!interpreterStackFrame.IsNull())
    {
        Out("WARNING: Interpreter stack frame unmatched");
    }
}

void EXT_CLASS_BASE::PrintReferencedPids(ExtRemoteTyped scriptContext, ExtRemoteTyped threadContext)
{
    scriptContext.OutTypeName();
    Out(" ");
    scriptContext.OutSimpleValue();
    Out("\n");

    bool isReferencedPropertyRecords = !scriptContext.HasField("referencedPropertyIds");    
    ExtRemoteTyped referencedPidDictionary = isReferencedPropertyRecords? 
        scriptContext.Field("javascriptLibrary.referencedPropertyRecords") :
        scriptContext.Field("referencedPropertyIds");
    ExtRemoteTyped referencedPidDictionaryCount = referencedPidDictionary.Field("count");
    ExtRemoteTyped referencedPidDictionaryEntries = referencedPidDictionary.Field("entries");
    long pidCount = referencedPidDictionaryCount.GetLong();

    Out("Number of pids on scriptContext: %d\n", pidCount);

    PropertyNameReader propertyNameReader(this, threadContext);
    for (int i = 0; i < pidCount; i++)
    {
        ExtRemoteTyped entry = referencedPidDictionaryEntries.ArrayElement(i);
        long pid = entry.Field(isReferencedPropertyRecords ? "value.pid" : "value").GetLong();
        ExtRemoteTyped propertyName("(char16 *)@$extin", propertyNameReader.GetNameByPropertyId(pid));
        Out(_u("Pid: %d "), pid);
        propertyName.OutSimpleValue();
        Out(_u("\n"));
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

        threadContext.ForEachScriptContext([this, &threadContext](ExtRemoteTyped scriptContext)
        {
            PrintReferencedPids(scriptContext, threadContext.GetExtRemoteTyped());
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
    int numThreadContexts = 0;
    ulong scriptThreadId = 0;
    RemoteThreadContext::ForEach([&scriptThreadId, &numThreadContexts, this](RemoteThreadContext threadContext)
    {
        ulong threadContextSystemThreadId = threadContext.GetThreadId();
        ulong threadContextThreadId = 0;

        HRESULT hr = this->m_System4->GetThreadIdBySystemId(threadContextSystemThreadId, &threadContextThreadId);

        if (SUCCEEDED(hr))
        {
            ULONG64 threadContextAddress = threadContext.GetExtRemoteTyped().GetPtr();

            this->Dml("<link cmd=\"~%us\">Thread context: <b>%p</b></link>\n", threadContextThreadId, threadContextAddress);
            numThreadContexts++;
            if (numThreadContexts == 1)
            {
                scriptThreadId = threadContextThreadId;
            }
        }

        return false;
    });

    if (numThreadContexts == 0)
    {
        this->Out("No script threads were found");
    }
    else if (numThreadContexts == 1)
    {
        this->Execute("~%us", scriptThreadId);
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
    threadContext.ForEachScriptContext([this](ExtRemoteTyped scriptContext)
    {

        ExtRemoteTyped hostScriptContextField = scriptContext.Field("hostScriptContext");
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
                Out("%p %p %d\n", javascriptDispatch.GetPtr(), javascriptDispatch.Field("scriptObject").GetPtr(), javascriptDispatch.Field("isGCTracked").GetW32Bool());
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
