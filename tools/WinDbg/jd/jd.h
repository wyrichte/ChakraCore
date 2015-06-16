//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#ifdef JD_PRIVATE
#define ENABLE_MARK_OBJ 0
#define ENABLE_UI_SERVER 0
class RootPointers;
#include <guiddef.h>
#include "RemoteThreadContext.h"
#endif

#include <map>

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

class EXT_CLASS_BASE : public ExtExtension
#ifdef JD_PRIVATE
    , public DummyTestGroup
#endif
{
protected:
    static HRESULT PrivateCoCreate(LPCWSTR strModule, REFCLSID rclsid, REFIID iid, LPVOID* ppunk);
    void IfFailThrow(HRESULT hr, PCSTR msg = NULL);

public:
    EXT_CLASS_BASE();

// ------------------------------------------------------------------------------------------------
// jd private commands: Wrap anything not meant to be published under JD_PRIVATE.
// ------------------------------------------------------------------------------------------------
#ifdef JD_PRIVATE
public:
    friend class HeapBlockHelper;
    friend class RecyclerObjectGraph;

    template <bool slist> 
    friend class RemoteListIterator;

    friend void ScanArena(ULONG64 arena, RootPointers& rootPointerManager);
    friend RootPointers ComputeRoots(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, ExtRemoteTyped* threadContext, bool dump);
    friend RootPointers ComputeRoots(EXT_CLASS_BASE* ext, ExtRemoteTyped recycler, ExtRemoteTyped threadContext, bool dump);
    friend bool IsUsingDebugPinRecord(EXT_CLASS_BASE* ext);
    
    virtual void __thiscall Uninitialize() override;

    static ULONG64 Count(ExtRemoteTyped head, PCSTR field);
    static ULONG64 TaggedCount(ExtRemoteTyped head, PCSTR field);
    static ULONG64 GetSizeT(ExtRemoteTyped data);
    bool PrintProperty(ULONG64 name, ULONG64 value, ULONG64 value1 = 0, int depth = 0);
    bool GetUsingInlineSlots(ExtRemoteTyped& typeHandler);
    void Out(_In_ PCSTR fmt, ...);
    void Out(_In_ PCWSTR fmt, ...);

    class PropertyNameReader
    {
    private:
        EXT_CLASS_BASE* m_ext;
        ExtRemoteTyped m_buffer;
        ULONG m_count;
        ULONG _none;

    public:
        PropertyNameReader(EXT_CLASS_BASE* ext, ExtRemoteTyped threadContext);

        ULONG Count() const { return m_count; }
        ULONG GetPropertyIdByIndex(ULONG i) const { return _none + i; }
        ULONG64 GetNameByIndex(ULONG i);
        ULONG64 GetNameByPropertyId(ULONG propertyId);
    };

    PCSTR FillModule(PCSTR fmt); // results share one buffer
    PCSTR FillModule2(PCSTR fmt); // results share one buffer
    PCSTR FillModuleV(PCSTR fmt, ...);

    PCSTR FillModuleAndMemoryNS(PCSTR fmt);
    PCSTR FillMemoryNS(PCSTR fmt);

    bool CheckTypeName(PCSTR typeName, ULONG* typeId = nullptr);
    PCSTR GetPageAllocatorType();
    PCSTR GetSegmentType();
    PCSTR GetPageSegmentType();

    ExtRemoteTyped GetThreadContextFromObject(ExtRemoteTyped& obj);        
    ExtRemoteTyped Cast(LPCSTR typeName, ULONG64 original);
    ExtRemoteTyped CastWithVtable(ExtRemoteTyped original, std::string* typeName = nullptr);
    std::string GetTypeName(ExtRemoteTyped& offset, bool includeModuleName = false);
    ULONG64 GetEnumValue(const char* enumName, ULONG64 default = -1);

#define ENUM(name)\
    ULONG64 enum_##name(){ \
        static ULONG64 s##name = this->GetEnumValue(#name); \
        return(s##name); \
    }
    // dynamic read HeapBlock enums
    ENUM(SmallNormalBlockType);
    ENUM(SmallLeafBlockType);
    ENUM(SmallFinalizableBlockType);
    ENUM(SmallNormalBlockWithBarrierType);
    ENUM(SmallFinalizableBlockWithBarrierType);
    ENUM(MediumNormalBlockType);
    ENUM(MediumLeafBlockType);
    ENUM(MediumFinalizableBlockType);
    ENUM(MediumNormalBlockWithBarrierType);
    ENUM(MediumFinalizableBlockWithBarrierType);
    ENUM(LargeBlockType);
    ENUM(SmallBlockTypeCount);
    ENUM(BlockTypeCount);

    RemoteThreadContext::Info remoteThreadContextInfo;
    void DetectFeatureBySymbol(Nullable<bool>& feature, PCSTR symbol);
    bool PageAllocatorHasExtendedCounters();
protected:
    size_t GetBVFixedAllocSize(int length);
    void DumpBlock(ExtRemoteTyped block, LPCSTR desc, LPCSTR sizeField, int index);
    bool TestFixed(ULONG64 bitVector, int index, ExtRemoteTyped& bvUnit);
    void DisplayLargeHeapBlockInfo(ExtRemoteTyped& largeHeapBlock);
    void DisplaySmallHeapBlockInfo(ExtRemoteTyped& smallHeapBlock, ExtRemoteTyped recycler);
    void DisplayPageAllocatorInfo(ExtRemoteTyped pageAllocator, CommandOutputType outputType = NormalOutputType);
    void DisplaySegmentList(PCSTR strListName, ExtRemoteTyped segmentList, PageAllocatorStats& stats, CommandOutputType outputType = NormalOutputType, bool pageSegment = true);

    PCSTR GetModuleName();
    PCSTR GetMemoryNS();

    ExtRemoteTyped GetTlsEntryList();
  
    ExtRemoteTyped GetRecycler(ULONG64 optionalRecyclerAddress);
    ExtRemoteTyped GetInternalStringBuffer(ExtRemoteTyped internalString);
    ExtRemoteTyped GetPropertyName(ExtRemoteTyped propertyNameListEntry);
    ULONG GetPropertyIdNone(ExtRemoteTyped& propertyNameListBuffer);

    void PrintScriptContextUrl(ExtRemoteTyped scriptContext);
    void PrintThreadContextUrl(ExtRemoteTyped threadContext, bool isCurrentThreadContext = false);
    void PrintAllUrl();

    void PrintScriptContextSourceInfos(ExtRemoteTyped scriptContext, bool printOnlyCount, bool printSourceContextInfo);
    void PrintThreadContextSourceInfos(ExtRemoteTyped threadContext, bool printOnlyCount, bool printSourceContextInfo, bool isCurrentThreadContext = false);
    void PrintAllSourceInfos(bool printOnlyCount, bool printSourceContextInfo);

    HRESULT CheckAndPrintJSFunction(ExtRemoteData firstArg, ULONG64 ebp, ULONG64 eip, int frameNumber);
    void PrintReferencedPids(ExtRemoteTyped scriptContext, ExtRemoteTyped threadContext);    

    bool IsInt31Var(ULONG64 var, int* value);
    bool IsTaggedIntVar(ULONG64 var, int* value);
    bool IsFloatVar(ULONG64 var, double* value);

    void PrintVar(ULONG64 var, int depth = 0);
    void PrintProperties(ULONG64 var, int depth = 0);
    void PrintSimpleValue(ExtRemoteTyped& obj);
    std::string GetRemoteVTableName(PCSTR type);
    ULONG64 GetRemoteVTable(PCSTR type);
    RemoteTypeHandler* GetTypeHandler(ExtRemoteTyped& obj, ExtRemoteTyped& typeHandler);

    
    void DumpStackTraceEntry(ULONG64 addr, AutoBuffer<wchar_t>& buf);

    // jscript9diag
    void EnsureJsDebug(PCWSTR jscript9diagPath = nullptr);
    void CreateDebugProcess(IJsDebugProcess** ppDebugProcess);
    void CreateStackWalker(IJsDebugStackWalker** ppStackWalker);
    void EnsureStackFrame(int frame = -1);
    void Print(IJsDebugProperty* prop, int radix, int maxDepth = 3);
    void Print(IJsDebugProperty* prop, PCWSTR fmt, int radix, int depth, int maxDepth);
    void ValidateEvaluateFullName(const JsDebugPropertyInfo& info, int radix);
    
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
    JD_PRIVATE_COMMAND_METHOD(var2);

    JD_PRIVATE_COMMAND_METHOD(url);
    JD_PRIVATE_COMMAND_METHOD(sourceInfos);
    JD_PRIVATE_COMMAND_METHOD(jstack);
    JD_PRIVATE_COMMAND_METHOD(gcstats);
    JD_PRIVATE_COMMAND_METHOD(hbstats);
    JD_PRIVATE_COMMAND_METHOD(jsobjectstats);
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
    JD_PRIVATE_COMMAND_METHOD(arrseg);
    JD_PRIVATE_COMMAND_METHOD(memstats);
    JD_PRIVATE_COMMAND_METHOD(findpage);

    // jdbackend commands
    JD_PRIVATE_COMMAND_METHOD(irinstr);
    JD_PRIVATE_COMMAND_METHOD(irfunc);

    // jdbytecode commands
    JD_PRIVATE_COMMAND_METHOD(bc);

    // jscript9diag commands
    JD_PRIVATE_COMMAND_METHOD(stack);
    JD_PRIVATE_COMMAND_METHOD(frame);
    JD_PRIVATE_COMMAND_METHOD(eval);
    JD_PRIVATE_COMMAND_METHOD(utmode);
    JD_PRIVATE_COMMAND_METHOD(jsstream);
    
    // Language service commands
    JD_PRIVATE_COMMAND_METHOD(lsprimarytext);
    JD_PRIVATE_COMMAND_METHOD(lscontextpath);

    MPH_COMMAND_METHOD(mpheap);
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

#ifdef JD_PRIVATE
class ExtRemoteString
{
public:
    ExtRemoteString(__in const ExtRemoteTyped &parent, PCSTR pszFieldName);

    bool IsNull();
    PCWSTR GetString();
    ExtRemoteTyped& F() { return m_parent; }

private:
    ExtRemoteTyped m_parent;
    WCHAR m_szBuffer[1024];
    PCSTR m_strFieldName;
};

std::string GetSymbolForOffset(EXT_CLASS_BASE* ext, ULONG64 offset);
ULONG64 GetPointerAtAddress(ULONG64 offset);
ULONG64 GetAsPointer(ExtRemoteTyped object);
int GuidToString(GUID& guid, LPSTR strGuid, int cchStrSize);
EXT_CLASS_BASE* GetExtension();
void ReplacePlaceHolders(PCSTR holder, std::string value, std::string& cmd);

template <typename Fn>
static bool LinkListForEach(ExtRemoteTyped list, char const * next, Fn fn)
{
    ExtRemoteTyped curr = list;
    while (curr.GetPtr() != 0)
    {
        if (fn(curr))
        {
            return true;
        }
        curr = curr.Field(next);
    }
    return false;
}

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
