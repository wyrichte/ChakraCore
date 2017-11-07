//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

#ifdef JD_PRIVATE
#define ENABLE_MARK_OBJ 0
#define ENABLE_UI_SERVER 0
class RootPointers;
#include <guiddef.h>
#include "RecyclerObjectGraph.h"
#include "RemoteRecyclableObject.h"
#include "RemoteUtf8SourceInfo.h"
#include "RemoteFunctionInfo.h"
#include "RemoteFunctionBody.h"
#include "RemoteJavascriptFunction.h"
#include "RemoteScriptFunction.h"
#include "RemoteInterpreterStackFrame.h"
#include "RemoteScriptContext.h"
#include "RemoteThreadContext.h"
#endif

#include <map>
#include "FieldInfoCache.h"
#include "RecyclerObjectTypeInfo.h"
#ifdef JD_PRIVATE
#include "JDByteCodeCachedData.h"
#endif

#include "JDTypeCache.h"

enum CommandOutputType
{
    NormalOutputType,
    VerboseOutputType,
    SummaryOutputType,
    WebJdOutputType         // Future use
};

struct PageAllocatorStats
{
    ULONG64 count;
    ULONG64 totalByteCount;
};

template<typename T>
class AutoBuffer
{
    T* m_ptr;
    ULONG m_allocatedElementCount;
public:
    AutoBuffer() : m_ptr(nullptr), m_allocatedElementCount(0) {}
    ~AutoBuffer()
    {
        Reset();
    }
    operator T*() { return m_ptr; }
    void EnsureCapacity(ULONG requiredElements)
    {
        if (m_allocatedElementCount < requiredElements)
        {
            Reset();
            m_ptr = new T[requiredElements];
            m_allocatedElementCount = requiredElements;
        }
    }
private:
    void Reset()
    {
        delete[] m_ptr;
        m_ptr = nullptr;
        m_allocatedElementCount = 0;
    }
};

class MphCmdsWrapper
{
public :
    struct AutoMPHCmd
    {
        MphCmdsWrapper* wrapper;
        AutoMPHCmd(MphCmdsWrapper* w) :wrapper(w)
        {
            wrapper->InitializeForMPH();
        }
        ~AutoMPHCmd(){ wrapper->inMPHCmd = false; }
    };
public:
    bool inMPHCmd;
    char* tridentModule;
    char* memGCModule;
    void InitializeForMPH();

};

class AutoCppExpressionSyntax
{
public:
    AutoCppExpressionSyntax(IDebugControl5* control) :
        control(control)
    {
        if (SUCCEEDED(control->GetExpressionSyntax(&exprSyntaxFlags)))
        {
            control->SetExpressionSyntaxByName("c++");
        }
        else
        {
            control = NULL;
        }
    }

    ~AutoCppExpressionSyntax()
    {
        if (control != NULL)
        {
            control->SetExpressionSyntax(exprSyntaxFlags);
        }
    }

private:
    ULONG exprSyntaxFlags;
    IDebugControl5* control;
};


class EXT_CLASS_BASE : public ExtExtension
#ifdef JD_PRIVATE
    , public DummyTestGroup
#endif
#ifdef MPH_CMDS
    , public MphCmdsWrapper
#endif
{
protected:
    static HRESULT PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, REFIID iid, LPVOID* ppunk);
    void IfFailThrow(HRESULT hr, PCSTR msg = NULL);

public:
    EXT_CLASS_BASE();
    ~EXT_CLASS_BASE() { ClearCache(); }
    bool PreferDML();

// ------------------------------------------------------------------------------------------------
// jd private commands: Wrap anything not meant to be published under JD_PRIVATE.
// ------------------------------------------------------------------------------------------------
#ifdef JD_PRIVATE
public:
    friend class ObjectInfoHelper;
    friend class RecyclerObjectGraph;

    template <bool slist> 
    friend class RemoteListIterator;

    virtual void OnSessionInaccessible(ULONG64) override;
    virtual void __thiscall Uninitialize() override;

    bool PrintProperty(ULONG64 name, ULONG64 value, ULONG64 value1 = 0, int depth = 0);
    bool GetUsingInlineSlots(ExtRemoteTyped& typeHandler);
    void Out(_In_ PCSTR fmt, ...);
    void Out(_In_ PCWSTR fmt, ...);
    void Dbg(_In_ PCSTR fmt, ...);
    void Dbg(_In_ PCWSTR fmt, ...);
    bool IsJScript9();

    bool DumpPossibleSymbol(RecyclerObjectGraph::GraphImplNodeType* node, bool makeLink = true, bool showScriptContext = false);
    bool DumpPossibleSymbol(ULONG64 address, bool makeLink = true, bool showScriptContext = false);
    bool DumpPossibleExternalSymbol(JDRemoteTyped object, char const * typeName, bool makeLink = true, bool showScriptContext = false);

    HRESULT CppEvalExprU64NoThrow(_In_ PCSTR Str, _Out_ ULONG64 &result)
    {
        AutoCppExpressionSyntax cppSyntax(m_Control5);
        HRESULT Status;
        DEBUG_VALUE FullVal;

        if ((Status = m_Control->
            Evaluate(Str, DEBUG_VALUE_INT64, &FullVal, NULL)) == S_OK)
        {
            result = FullVal.I64;
        }

        return Status;
    }

    class PropertyNameReader
    {
    private:
        ExtRemoteTyped m_buffer;
        ULONG m_count;
        ULONG _none;
        ULONG _maxBuiltIn;
        
    public:
        
        PropertyNameReader(RemoteThreadContext threadContext);

        ULONG Count() const { return m_count; }
        ULONG GetPropertyIdByIndex(ULONG i) const { return _none + i; }
        ULONG64 GetNameByIndex(ULONG i);
        ULONG64 GetNameByPropertyId(ULONG propertyId);

        std::string GetNameStringByPropertyId(ULONG propertyId);
    };

    bool HasMemoryNS();
    PCSTR FillModule(PCSTR fmt); // results share one buffer
    PCSTR FillModule2(PCSTR fmt); // results share one buffer
    PCSTR FillModuleV(PCSTR fmt, ...);

    PCSTR FillModuleAndMemoryNS(PCSTR fmt);
    PCSTR GetSmallHeapBlockTypeName();
    PCSTR GetSmallHeapBucketTypeName();

    bool CheckTypeName(PCSTR typeName, ULONG* typeId = nullptr);
    PCSTR GetPageAllocatorType();
    PCSTR GetSegmentType();
    PCSTR GetPageSegmentType();

    ExtRemoteTyped GetThreadContextFromObject(ExtRemoteTyped& obj);

    ULONG64 GetEnumValue(const char* enumName, bool useMemoryNamespace, ULONG64 defaultValue = -1);

    bool InChakraModule(ULONG64 address);

#define DEFINE_BLOCKTYPE_ENUM_ACCESSOR(name)\
    ULONG64 enum_##name() \
    { \
        return (this->inMPHCmd)? \
            this->recyclerCachedData.GetMPHBlockTypeEnum##name() : \
            this->recyclerCachedData.GetBlockTypeEnum##name(); \
    }

    BLOCKTYPELIST(DEFINE_BLOCKTYPE_ENUM_ACCESSOR);
#undef DEFINE_BLOCKTYPE_ENUM_ACCESSOR

    
    FieldInfoCache fieldInfoCache;
    RecyclerCachedData recyclerCachedData;
    RecyclerObjectTypeInfo::Cache recyclerObjectTypeInfoCache;    
    CachedTypeInfo m_AuxPtrsFix16;
    CachedTypeInfo m_AuxPtrsFix32;
    RemoteThreadContext::Info remoteThreadContextInfo;
    ULONG64 chakraModuleBaseAddress;
    ULONG64 chakraModuleEndAddress;

    void DetectFeatureBySymbol(Nullable<bool>& feature, PCSTR symbol);
    bool PageAllocatorHasExtendedCounters();

    bool IsJITServer();

#ifdef JD_PRIVATE
    JDByteCodeCachedData const& GetByteCodeCachedData()
    {
        byteCodeCachedData.Ensure();
        return byteCodeCachedData;
    }
#endif
private:

#ifdef JD_PRIVATE
    JDByteCodeCachedData byteCodeCachedData;
#endif

protected:
    void DumpBlock(ExtRemoteTyped block, LPCSTR desc, LPCSTR sizeField, int index);
    void DisplayLargeHeapBlockInfo(ExtRemoteTyped& largeHeapBlock);
    void DisplaySmallHeapBlockInfo(ExtRemoteTyped& smallHeapBlock, RemoteRecycler recycler);
    void DisplayPageAllocatorInfo(JDRemoteTyped pageAllocator, CommandOutputType outputType = NormalOutputType);
    void DisplaySegmentList(PCSTR strListName, ExtRemoteTyped segmentList, PageAllocatorStats& stats, CommandOutputType outputType = NormalOutputType, bool pageSegment = true);

    bool HasType(const char* moduleName, const char* typeName);
    PCSTR GetModuleName();
    PCSTR GetMemoryNS();
 
    ExtRemoteTyped GetInternalStringBuffer(ExtRemoteTyped internalString);
    ExtRemoteTyped GetPropertyName(ExtRemoteTyped propertyNameListEntry);
    ULONG GetPropertyIdNone(ExtRemoteTyped& propertyNameListBuffer);

    void PrintScriptContextUrl(RemoteScriptContext scriptContext, bool showAll, bool showLink);
    void PrintThreadContextUrl(RemoteThreadContext threadContext, bool showAll, bool showLink, bool isCurrentThreadContext = false);
    void PrintAllUrl(bool showAll, bool showLink);

    void PrintScriptContextSourceInfos(RemoteScriptContext scriptContext, bool printOnlyCount, bool printSourceContextInfo);
    void PrintThreadContextSourceInfos(RemoteThreadContext threadContext, bool printOnlyCount, bool printSourceContextInfo, bool isCurrentThreadContext = false);
    void PrintAllSourceInfos(bool printOnlyCount, bool printSourceContextInfo);    

    bool IsInt31Var(ULONG64 var, int* value);
    bool IsTaggedIntVar(ULONG64 var, int* value);
    bool IsFloatVar(ULONG64 var, double* value);

    void PrintVar(ULONG64 var, int depth = 0);
    void PrintProperties(ULONG64 var, int depth = 0);
    void PrintSimpleVarValue(ExtRemoteTyped& obj);
    std::string GetRemoteVTableName(PCSTR type);
    std::string GetTypeNameFromVTable(PCSTR vtablename);
public: // TODO (doilij) reorganize public member (this being public is needed for CSVX)
    std::string GetTypeNameFromVTable(ULONG64 vtableAddress);
protected:
    std::string GetTypeNameFromVTableOfObject(ULONG64 objectAddress);    
    ULONG64 GetRemoteVTable(PCSTR type);
    RemoteTypeHandler* GetTypeHandler(ExtRemoteTyped& obj, ExtRemoteTyped& typeHandler);

    void DumpStackTraceEntry(ULONG64 addr, AutoBuffer<char16>& buf);

    // jscript9diag
    void EnsureJsDebug(PCWSTR jscript9diagPath = nullptr);
    void CreateDebugProcess(IJsDebugProcess** ppDebugProcess);
    void CreateStackWalker(IJsDebugStackWalker** ppStackWalker);
    void EnsureStackFrame(int frame = -1);
    void Print(IJsDebugProperty* prop, int radix, int maxDepth = 3);
    void Print(IJsDebugProperty* prop, PCWSTR fmt, int radix, int depth, int maxDepth);
    
    bool DoInt32Var() const { return this->m_PtrSize == 8; };  // 64-bit use int32 var
    bool DoFloatVar() const { return this->m_PtrSize == 8; };  // 64-bit use float var

    bool IsMinidumpDebugging()
    {
        ULONG Class = 0;
        ULONG Qualifier = 0;
        this->m_Control6->GetDebuggeeType(&Class, &Qualifier);
        return (Qualifier & DEBUG_USER_WINDOWS_DUMP)
            || (Qualifier & DEBUG_USER_WINDOWS_SMALL_DUMP);
    }

    void ClearCache()
    {
        this->byteCodeCachedData.Clear();
        this->recyclerCachedData.Clear();
        this->typeCache.Clear();
        this->chakraModuleBaseAddress = 0;
        this->chakraModuleEndAddress = 0;
    }

    static RemoteRecycler GetRecycler(ULONG64 recyclerPtr, RemoteThreadContext * threadContext = nullptr);

    template <bool predecessorsMode = false>
    void PredSuccImpl();
public:
    template<typename T>
    T GetNumberValue(ExtRemoteTyped var)
    {
        Assert(var.GetTypeSize() <= m_PtrSize);
        return (T)var.GetData(var.GetTypeSize());
    }


protected:
    char m_moduleName[16]; // jc or jscript9, access through GetModuleName()
    char m_fillModuleBuffer[1024]; // one temp buffer
    char m_uiServerString[40]; // UI Server session GUID string
    char m_gcNS[16];

    //
    // Detect some features dynamically so that this extension supports many jscript versions.
    //
    
    Nullable<bool> m_newPropertyMap;                // IE11 version of property map implementation
    Nullable<bool> m_usingWeakRef;                  // If the build uses WeakRef
    Nullable<ULONG> m_propertyIdNone;               // What's the PropertyIds::_none value
    Nullable<bool> m_usingInlineSlots;              // If the build uses inline slots
    Nullable<bool> m_usingLibraryInType;            // If the build uses javascriptLibrary in Type (older builds use globalObject)
    Nullable<bool> m_usingPropertyRecordInTypeHandlers;   // If the build uses PropertyRecord* in type handlers
    Nullable<bool> m_pageAllocatorHasExtendedCounters; // If the build has extended counter for page allocators
    Nullable<bool> m_isJITServer;

    enum TaggedIntUsage {
        TaggedInt_Int31,    // IE9 uses Int31
        TaggedInt_TaggedInt // IE10 uses Int32Var/FloatVar on amd64
    };
    Nullable<TaggedIntUsage> m_taggedIntUsage;
    TaggedIntUsage GetTaggedIntUsage();

    stdext::hash_map<ULONG64, RemoteTypeHandler*> m_typeHandlers;
    stdext::hash_map<std::string, RemoteTypeHandler*> m_typeHandlersByName;

    // jscript9diag
    CComPtr<IJsDebug2> m_jsDebug;
    CComPtr<IJsDebugFrame> m_jsFrame;
    CComPtr<IDebugSymbols5> m_symbols5;
    int m_jsFrameNumber;
    bool m_unitTestMode;

    bool m_isCachedHasMemoryNS;
    bool m_hasMemoryNS;

    friend class JDTypeCache;
    JDTypeCache typeCache;    
    
#endif //JD_PRIVATE
};

#ifdef JD_PRIVATE_CMDS
#define JD_PRIVATE_COMMAND(_Name, _Desc, _Args) \
    EXT_COMMAND(_Name, _Desc, _Args)
#else
#define JD_PRIVATE_COMMAND(_Name, _Desc, _Args) \
    void EXT_CLASS::##_Name()
#endif

#ifdef MPH_CMDS
#define MPH_COMMAND(_Name, _Desc, _Args) \
    EXT_COMMAND(_Name, _Desc, _Args)
#else
#define MPH_COMMAND(_Name, _Desc, _Args) \
    void EXT_CLASS::##_Name()
#endif

#define JD_PRIVATE_COMMAND_METHOD EXT_COMMAND_METHOD
#define MPH_COMMAND_METHOD EXT_COMMAND_METHOD

class EXT_CLASS : public EXT_CLASS_BASE
{
public:
    EXT_COMMAND_METHOD(ldsym);

public:
    JD_PRIVATE_COMMAND_METHOD(stst);
    JD_PRIVATE_COMMAND_METHOD(prop);
    JD_PRIVATE_COMMAND_METHOD(var);
    JD_PRIVATE_COMMAND_METHOD(fb);
    JD_PRIVATE_COMMAND_METHOD(diagvar);

    JD_PRIVATE_COMMAND_METHOD(url);
    JD_PRIVATE_COMMAND_METHOD(sourceInfos);
    JD_PRIVATE_COMMAND_METHOD(jstack);
    JD_PRIVATE_COMMAND_METHOD(hbstats);
    JD_PRIVATE_COMMAND_METHOD(jsobjectstats);
    JD_PRIVATE_COMMAND_METHOD(jsobjectnodes);
    JD_PRIVATE_COMMAND_METHOD(pagealloc);
    JD_PRIVATE_COMMAND_METHOD(hbm);
    JD_PRIVATE_COMMAND_METHOD(swb);
    JD_PRIVATE_COMMAND_METHOD(findblock);
    JD_PRIVATE_COMMAND_METHOD(drpids);
    JD_PRIVATE_COMMAND_METHOD(oi);
    JD_PRIVATE_COMMAND_METHOD(count);
    JD_PRIVATE_COMMAND_METHOD(findref);
    JD_PRIVATE_COMMAND_METHOD(showpinned);
    JD_PRIVATE_COMMAND_METHOD(showroots);
    JD_PRIVATE_COMMAND_METHOD(markmap);
#if ENABLE_MARK_OBJ
    JD_PRIVATE_COMMAND_METHOD(markobj);
#endif
    JD_PRIVATE_COMMAND_METHOD(predecessors);
    JD_PRIVATE_COMMAND_METHOD(successors);
    JD_PRIVATE_COMMAND_METHOD(traceroots);
    JD_PRIVATE_COMMAND_METHOD(arenaroots);
    JD_PRIVATE_COMMAND_METHOD(savegraph);
    JD_PRIVATE_COMMAND_METHOD(slist);
    JD_PRIVATE_COMMAND_METHOD(dlist);
    JD_PRIVATE_COMMAND_METHOD(bv);
    JD_PRIVATE_COMMAND_METHOD(dumptrace);
    JD_PRIVATE_COMMAND_METHOD(recycler);
    JD_PRIVATE_COMMAND_METHOD(tc);
    JD_PRIVATE_COMMAND_METHOD(showblockinfo);
#if ENABLE_UI_SERVER
    EXT_COMMAND_METHOD(uiserver);
#endif
    JD_PRIVATE_COMMAND_METHOD(jsdisp);
    JD_PRIVATE_COMMAND_METHOD(warnicf);
    JD_PRIVATE_COMMAND_METHOD(arrseg);
    JD_PRIVATE_COMMAND_METHOD(memstats);
    JD_PRIVATE_COMMAND_METHOD(findpage);

    // jdbackend commands
    JD_PRIVATE_COMMAND_METHOD(irinstr);
    JD_PRIVATE_COMMAND_METHOD(irfunc);

    // jdbytecode commands
    JD_PRIVATE_COMMAND_METHOD(bc);

    // jscript9diag commands
    JD_PRIVATE_COMMAND_METHOD(diagstack);
    JD_PRIVATE_COMMAND_METHOD(diagframe);
    JD_PRIVATE_COMMAND_METHOD(diageval);
    JD_PRIVATE_COMMAND_METHOD(utmode);
    JD_PRIVATE_COMMAND_METHOD(jsstream);

    MPH_COMMAND_METHOD(mpheap);
};

#ifdef JD_PRIVATE

std::string GetSymbolForOffset(ULONG64 offset);
ULONG64 GetPointerAtAddress(ULONG64 offset);
int GuidToString(GUID& guid, LPSTR strGuid, int cchStrSize);
EXT_CLASS_BASE* GetExtension();
void ReplacePlaceHolders(PCSTR holder, std::string value, std::string& cmd);


template <typename Fn>
static bool SListForEach(ExtRemoteTyped list,  Fn fn)
{
    ExtRemoteTyped curr = list.Field("next");
    while (curr.Field("base").GetPtr() != list.GetPtr())
    {
        if (fn(curr.Field("node.data")))
        {
            return true;
        }
        curr = curr.Field("base.next");
    }
    return false;
}
#endif //JD_PRIVATE
