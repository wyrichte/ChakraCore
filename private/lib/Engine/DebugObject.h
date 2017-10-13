//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include "Psapi.h"

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
//
// Debug/FreTest Javascript object to exercise the disposable
// object codepath in the GC
//
class DebugDisposableObject: public Js::RecyclableObject
{
public:
    DEFINE_VTABLE_CTOR(DebugDisposableObject, RecyclableObject);

    DebugDisposableObject(Js::DynamicType* type, Js::ScriptContext* scriptContext, bool collectOnDispose, uint bytesToAllocateOnDispose, bool allocateLeaf, uint allocationCount);
    virtual void Dispose(bool isShutdown) override;

    virtual void Finalize(bool isShutdown) override
    {
    }

private:
    bool collectOnDispose;
    bool allocateLeaf;
    uint bytesToAllocateOnDispose;
    uint allocationCount;
};

class DebugFuncExecutorInDisposeObject : public Js::DynamicObject
{
public:
    DebugFuncExecutorInDisposeObject(Js::DynamicType* type, Js::JavascriptFunction* functionToCall, Js::Var* args, ushort numberOfArgs);
    virtual void Dispose(bool isShutdown) override;

protected:
    DEFINE_VTABLE_CTOR(DebugFuncExecutorInDisposeObject, DynamicObject);
    DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(DebugFuncExecutorInDisposeObject);

private:
    Js::JavascriptFunction* functionToCall;
    Js::Var* args;
    ushort numberOfArgs;
    bool areAllParametersPinned = false;
};
#endif

class DebugObject
{

public:    
    class EntryInfo
    {
    public:
        static Js::FunctionInfo Write;
        static Js::FunctionInfo WriteLine;
        static Js::FunctionInfo GetterSetNonUserCodeExceptions;
        static Js::FunctionInfo SetterSetNonUserCodeExceptions;
        static Js::FunctionInfo GetterDebuggerEnabled;
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        static Js::FunctionInfo GetWorkingSet;
        static Js::FunctionInfo SourceDebugBreak;
        static Js::FunctionInfo InvokeFunction;
        static Js::FunctionInfo GetHostInfo;
        static Js::FunctionInfo GetMemoryInfo;
        static Js::FunctionInfo GetTypeHandlerName;
        static Js::FunctionInfo GetArrayType;
        static Js::FunctionInfo DumpHeap;
        static Js::FunctionInfo CreateDebugDisposableObject;
        static Js::FunctionInfo IsInJit;
        static Js::FunctionInfo GetCurrentSourceInfo;
        static Js::FunctionInfo GetLineOfPosition;
        static Js::FunctionInfo GetPositionOfLine;
        static Js::FunctionInfo AddFTLProperty;
        static Js::FunctionInfo AddLazyFTLProperty;
        static Js::FunctionInfo CreateTypedObject;
        static Js::FunctionInfo CreateProjectionArrayBuffer;
        static Js::FunctionInfo EmitStackTraceEvent;
        static Js::FunctionInfo GetTypeInfo;
        static Js::FunctionInfo ParseFunction;
        static Js::FunctionInfo SetAutoProxyName;
        static Js::FunctionInfo DisableAutoProxy;
        static Js::FunctionInfo CreateDebugFuncExecutorInDisposeObject;
        static Js::FunctionInfo DetachAndFreeObject;
        static Js::FunctionInfo IsAsmJSModule;
        static Js::FunctionInfo Enable;
        static Js::FunctionInfo DisableImplicitCalls;
        static Js::FunctionInfo EnableImplicitCalls;
#else
#ifdef ENABLE_HEAP_DUMPER
        static Js::FunctionInfo DumpHeap;
#endif
#endif

#if JS_PROFILE_DATA_INTERFACE
        static Js::FunctionInfo GetProfileDataObject;
#endif

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        // for controlling fault injection inside JS code
        static Js::FunctionInfo GetterFaultInjectionCookie;
        static Js::FunctionInfo SetterFaultInjectionCookie;
#endif
    };

    static Js::Var EntryWrite(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryWriteLine(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetterSetNonUserCodeExceptions(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntrySetterSetNonUserCodeExceptions(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetterDebuggerEnabled(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    static Js::Var EntryGetWorkingSet(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntrySourceDebugBreak(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryInvokeFunction(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetHostInfo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetMemoryInfo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetTypeHandlerName(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetArrayType(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var DumpHeapInternal(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryCreateDebugDisposableObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryIsInJit(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetCurrentSourceInfo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetLineOfPosition(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetPositionOfLine(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryAddFTLProperty(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryAddLazyFTLProperty(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var AddFTLPropertyCommon(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryCreateTypedObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryCreateProjectionArrayBuffer(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryEmitStackTraceEvent(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryGetTypeInfo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryParseFunction(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntrySetAutoProxyName(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryDisableAutoProxy(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryCreateDebugFuncExecutorInDisposeObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var DetachAndFreeObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryIsAsmJSModule(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryEnable(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryDisableImplicitCalls(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
    static Js::Var EntryEnableImplicitCalls(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);

    static Js::Var DummyScriptMethod(Var method, CallInfo callInfo, Var* args) { return method; }
    static HRESULT __cdecl DummyInitializeMethod(Var instance) { return S_OK; }

    // Delay loaded to ensure that fre bits do not have a dependency on PSAPI.dll
    class PsapiLibrary : protected DelayLoadLibrary
    {
    private:
        typedef BOOL (WINAPI *PFnGetProcessMemoryInfo)(
            HANDLE Process,
            PPROCESS_MEMORY_COUNTERS ppsmemCounters,
            DWORD cb);
        PFnGetProcessMemoryInfo getProcessMemoryInfo;

    public:
        PsapiLibrary() : DelayLoadLibrary(),
            getProcessMemoryInfo(NULL)
        {
            this->EnsureFromSystemDirOnly();
        }

        LPCTSTR GetLibraryName() const { return _u("psapi.dll"); } 

        BOOL GetProcessMemoryInfo(
                HANDLE Process,
                PPROCESS_MEMORY_COUNTERS ppsmemCounters,
                DWORD cb)
        {
             if (m_hModule)
             {
                if (getProcessMemoryInfo == nullptr)
                {
                    getProcessMemoryInfo = (PFnGetProcessMemoryInfo)GetFunction("GetProcessMemoryInfo");
                    if (getProcessMemoryInfo == nullptr)
                    {
                        Assert(false);
                        return FALSE;
                    }
                }
                return getProcessMemoryInfo(Process, ppsmemCounters, cb);
             }
             Assert(false);
             return FALSE;
        }
    };
#else ENABLE_DEBUG_CONFIG_OPTIONS
#ifdef ENABLE_HEAP_DUMPER
    static Js::Var DumpHeap(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
#endif
#endif
#if JS_PROFILE_DATA_INTERFACE
    static Js::Var EntryGetProfileDataObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
#endif

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        static Js::Var EntryGetterFaultInjectionCookie(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
        static Js::Var EntrySetterFaultInjectionCookie(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
#endif

private:    
    static Js::Var WriteHelper(Js::RecyclableObject* function, Js::Arguments args, bool newLine);
    static Js::FunctionBody * GetCallerFunctionBody(Js::ScriptContext *scriptContext);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    static IASDDebugObjectHelper* EnsureDebugObjectHelper(Js::ScriptContext* scriptContext);
#endif
};
#ifdef ENABLE_HEAP_DUMPER

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
class IASDDebugObjectHelper 
{
public:
    IASDDebugObjectHelper(IActiveScriptDirect* scriptDirect) : scriptDirect(scriptDirect), externalTypeMap(nullptr) {}
    HTYPE EnsureType(JavascriptTypeId typeId, Js::PropertyId nameId, bool useDefaultTypeOperations);
    typedef JsUtil::BaseDictionary<JavascriptTypeId, HTYPE, Recycler, PowerOf2SizePolicy> ExternalTypeMap;
    
private:
    IActiveScriptDirect* scriptDirect;
    ExternalTypeMap* externalTypeMap;
};
#endif
class HeapDumper
{

    struct DumpArgs
    {
        HeapDumperObjectToDumpFlag dumpType;
        Var objectToDump;
        Var objectToSkip;
        bool printHeapEnum;
        bool printBaselineComparison;
        bool dumpRootsOnly;
        bool returnArray;
        PROFILER_HEAP_ENUM_FLAGS enumFlags;
        UINT maxDumpIndent;
        bool trimDirectoryNameFromFullPath;    // When property value is string and is a path, trim the dir part and leave hust file name part.
        DumpArgs() :
            dumpType(HeapDumperDumpNew), objectToDump(nullptr), objectToSkip(nullptr), printHeapEnum(true), printBaselineComparison(false),
            dumpRootsOnly(false), returnArray(false), enumFlags(PROFILER_HEAP_ENUM_FLAGS_RELATIONSHIP_SUBSTRINGS), maxDumpIndent(UINT_MAX), trimDirectoryNameFromFullPath(true) {}
    };

    struct TypeMap
    {
        ULONG count;
        ULONG space;
        LPWSTR name;
    }; 

    struct IndentBuffer
    {
        static const int maxIndent = 5000;
        static WCHAR buffer[];
        static UINT currIndent;
        static HRESULT Append(LPCWSTR, UINT&);
        static HRESULT Append(LPCWSTR);
        static void SetIndent(UINT indentAmount);
        static UINT CurrentIndent() { return currIndent; }
        static void Print();
    };


    ULONG FindObjectInSnapshot(DWORD_PTR obj);
    void DumpAddress(ULONG objIndex, bool newLine);
    HRESULT DumpSnapshotObject(DWORD_PTR address);
    HRESULT DumpRelationshipList(PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list, bool isIndexPropertyList, bool includeId);
    HRESULT DumpCollectionList(PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list);
    void DumpRelationshipFlags(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship);
    HRESULT DumpProperty(PROFILER_HEAP_OBJECT_RELATIONSHIP& elem);
    HRESULT DumpObjectInfo(ULONG objIndex);
    LPCWSTR GetNameFromId(PROFILER_HEAP_OBJECT_NAME_ID nameId);
    LPCWSTR GetStringValue(LPCWSTR propertyValue);
    bool IsNewStateAvailable(PROFILER_HEAP_OBJECT& obj);
    bool IsExternalObject(PROFILER_HEAP_OBJECT& obj);
    static bool HasArg(Js::Var args[], int argc, int i);

public:
    HeapDumper(ScriptEngine& scriptEngineIn, Js::Var args[], int argc);
    HeapDumper(ScriptEngine& scriptEngineIn, HeapDumperObjectToDumpFlag objectsToDump, BOOL minimalDump);
    ~HeapDumper();
    Js::Var DumpHeap();

private:
    static const USHORT profilerHeapObjectFlagsDumped = 0x8000;
    static TypeMap HeapDumper::typeMap[];
    ULONG numSnapshotElements;
    PROFILER_HEAP_OBJECT** pSnapshot;
    UINT maxPropertyId;
    LPCWSTR* pPropertyIdMap;
    ScriptEngine& scriptEngine;
    AutoReleasePtr<IActiveScriptProfilerHeapEnum> pEnum;
    DumpArgs dumpArgs;
};
#endif
