
//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "activprof.h"

#if DBG || ENABLE_REGEX_CONFIG_OPTIONS || defined(MUTATORS) || defined(ARRLOG) || defined(PROFILE_STRINGS)
#define NEED_MISC_ALLOCATOR
#endif

#define BuiltInFunctionsScriptId 0

class NativeCodeGenerator;
class BackgroundParser;
struct IActiveScriptDirect;
namespace Js
{
    class ScriptContext;
    class ScriptEditQuery;
    class MutationBreakpoint;
}

// Created for every source buffer passed by trident this structure has
// all the details.
class SRCINFO
{
    // TODO:
    // We currently don't free SRCINFO object so we don't want to add extra variables here.
    // When we do make it freeable and will be able to allocate more than one per Module,
    // we can move variables m_isGlobalFunc and m_isEval from FunctionBody.cpp here.
public:
    SourceContextInfo * sourceContextInfo;
    ULONG dlnHost;             // Line number passed to ParseScriptText
    ULONG ulColumnHost;        // Column number on the line where the parse script text started
    ULONG lnMinHost;           // Line offset of first host-supplied line
    ULONG ichMinHost;          // Range of host supplied characters
    ULONG ichLimHost;
    ULONG ulCharOffset;        // Char offset of the source text relative to the document. (Populated using IActiveScriptContext)
    Js::ModuleID moduleID;
    ULONG grfsi;

    static SRCINFO* Copy(Recycler* recycler, const SRCINFO* srcInfo)
    {
        SRCINFO* copySrcInfo = RecyclerNew(recycler, SRCINFO, *srcInfo);
        return copySrcInfo;
    }

    SRCINFO* Clone(Js::ScriptContext* scriptContext) const;
};

class AuthoringData;

class AuthoringCallbacks
{
public:
    virtual void Parsing() = 0;
    virtual void GeneratingByteCode() = 0;
    virtual void Executing() = 0;
    virtual void Progress() = 0;
    virtual void PreparingEval(charcount_t length) = 0;
    virtual Js::RecyclableObject *GetMissingPropertyResult(Js::ScriptContext *scriptContext, Js::RecyclableObject *instance, Js::PropertyId id, Js::TypeId typeId) = 0;
    virtual Js::RecyclableObject *GetMissingItemResult(Js::ScriptContext *scriptContext, Js::RecyclableObject *instance, uint32 index, Js::TypeId typeId) = 0;
    virtual Js::RecyclableObject *GetMissingParameterValue(Js::ScriptContext *scriptContext, Js::JavascriptFunction *function, uint32 paramIndex) = 0;
    virtual Js::RecyclableObject *GetTrackingKey(Js::ScriptContext *scriptContext, Js::Var value, Js::TypeId typeId) = 0;
    virtual Js::Var GetCallerName(Js::ScriptContext *scriptContext, int fileId, int offset) = 0;
    virtual Js::Var GetTrackingValue(Js::ScriptContext *scriptContext, Js::RecyclableObject *value) = 0;
    virtual bool HasThisStmt(Js::ScriptContext *scriptContext, Js::JavascriptFunction *function) = 0;
    virtual Js::Var GetExecutingScriptFileName(Js::ScriptContext *scriptContext) = 0;
    virtual bool CopyOnGet() = 0;
    virtual int GetFileIdOfSourceIndex(Js::ScriptContext *scriptContext, int sourceIndex) = 0;
    virtual DWORD_PTR GetAuthorSource(int sourceIndex, Js::ScriptContext *scriptContext) = 0;
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    virtual void LogFunctionStart(Js::ScriptContext *scriptContext, Js::FunctionBody * functionBody) = 0;
    virtual void LogFunctionEnd(Js::ScriptContext *scriptContext) = 0;
    virtual bool IsCallGraphEnabled() const = 0;
    virtual void SetCallGraph(bool enable) = 0;
#endif
};

struct CustomExternalObjectOperations
{
    size_t offsetOfOperationsUsage;
    DWORD operationFlagEquals;
    DWORD operationFlagStrictEquals;
};

enum ExternalJitData
{
    ExternalJitData_CustomExternalObjectOperations
};

class HostScriptContext
{
public:
    HostScriptContext(Js::ScriptContext* inScriptContext) { this->scriptContext = inScriptContext; }
    virtual void Delete() = 0;
    virtual HRESULT GetPreviousHostScriptContext(__deref_out HostScriptContext** ppUnkCaller) = 0;
    virtual HRESULT PushHostScriptContext() = 0;
    virtual void PopHostScriptContext() = 0;

    virtual HRESULT SetCaller(IUnknown *punkNew, IUnknown **ppunkPrev) = 0;
    virtual HRESULT GetDispatchExCaller(__deref_out void** dispatchExCaller) = 0;
    virtual void ReleaseDispatchExCaller(__in void* dispatchExCaler) = 0;
    virtual Js::ModuleRoot * GetModuleRoot(int moduleID) = 0;
    virtual HRESULT CheckCrossDomainScriptContext(__in Js::ScriptContext* scriptContext) = 0;

    virtual HRESULT GetHostContextUrl(__in DWORD_PTR hostSourceContext, __out BSTR& pUrl) = 0;
    virtual BOOL HasCaller() = 0;
    virtual void CleanDynamicCodeCache() = 0;
    virtual HRESULT VerifyDOMSecurity(Js::ScriptContext* targetContext, Js::Var obj) = 0;
    virtual Js::JavascriptMethod GetSimpleSlotAccessCrossSiteThunk() = 0;

    virtual HRESULT CheckEvalRestriction() = 0;
    virtual HRESULT HostExceptionFromHRESULT(HRESULT hr, Js::Var* outError) = 0;

    virtual HRESULT GetExternalJitData(ExternalJitData id, void *data) = 0;

    Js::ScriptContext* GetScriptContext() { return scriptContext; }

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
    virtual void EnsureParentInfo(Js::ScriptContext* scriptContext = NULL) = 0;
#endif
private:
    Js::ScriptContext* scriptContext;
};

namespace Js
{

#pragma pack(push, 1)
    struct StackFrameInfo
    {
        StackFrameInfo() { }
        StackFrameInfo(DWORD_PTR _scriptContextID
            , UINT32 _sourceLocationLineNumber
            , UINT32 _sourceLocationColumnNumber
            , UINT32 _methodIDOrNameIndex
            , UINT8 _isFrameIndex)
            : scriptContextID(_scriptContextID)
            , sourceLocationLineNumber(_sourceLocationLineNumber)
            , sourceLocationColumnNumber(_sourceLocationColumnNumber)
            , methodIDOrNameIndex(_methodIDOrNameIndex)
            , isFrameIndex(_isFrameIndex)
        { }

        DWORD_PTR scriptContextID;
        UINT32 sourceLocationLineNumber;
        UINT32 sourceLocationColumnNumber;
        UINT32 methodIDOrNameIndex;
        UINT8  isFrameIndex;
    };
#pragma pack(pop)

    class ProjectionConfiguration
    {
    public:
        ProjectionConfiguration() : targetVersion(0)
        {
        }

        DWORD GetTargetVersion() const { return this->targetVersion; }
        void SetTargetVersion(DWORD version) { this->targetVersion = version; }

        bool IsTargetWindows8() const           { return this->targetVersion == NTDDI_WIN8; }
        bool IsTargetWindowsBlueOrLater() const { return this->targetVersion >= NTDDI_WINBLUE; }

    private:
        DWORD targetVersion;
    };

    class ScriptConfiguration
    {
    public:
        ScriptConfiguration(const bool isOptimizedForManyInstances) :            
            HostType(Configuration::Global.flags.HostType),
            WinRTConstructorAllowed(Configuration::Global.flags.WinRTConstructorAllowed),
            NoNative(Configuration::Global.flags.NoNative),
            isOptimizedForManyInstances(isOptimizedForManyInstances)
        {
        }

        // Version
        // TODO: Revisit this and check what other flags are required
        bool SupportsES3()           const { return true; }
        bool SupportsES3Extensions() const { return true; }        
                                
        Number GetHostType() const    // Returns one of enum HostType values (see ConfigFlagsTable.h).
        {
            AssertMsg(this->HostType >= HostTypeMin && this->HostType <= HostTypeMax, "HostType value is out of valid range.");
            return this->HostType;
        }

        bool IsErrorStackTraceEnabled()         const { return CONFIG_FLAG(errorStackTrace); }
        bool IsTypedArrayEnabled()              const { return true; }
        bool Is__proto__Enabled()               const { return CONFIG_FLAG(__proto__); }
        bool IsES6MapEnabled()                  const { return CONFIG_FLAG(Map); }
        bool IsES6SetEnabled()                  const { return CONFIG_FLAG(Set); }
        bool IsES6WeakMapEnabled()              const { return CONFIG_FLAG(WeakMap); }
        bool IsDefineGetterSetterEnabled()      const { return CONFIG_FLAG(DefineGetterSetter); }
        bool IsIntlEnabled() const;
        bool IsKhronosInteropEnabled()          const { return CONFIG_FLAG(KhronosInterop); }
        bool IsES6ArrayUseConstructorEnabled()  const { return CONFIG_FLAG_RELEASE(ES6ArrayUseConstructor); }
        bool IsES6ClassAndExtendsEnabled()      const { return CONFIG_FLAG_RELEASE(ES6Classes) || BinaryFeatureControl::LanguageService(); } // Need to remove the LanguageService check once the feature is enabled by default
        bool IsES6DateParseFixEnabled()         const { return CONFIG_FLAG_RELEASE(ES6DateParseFix); }
        bool IsES6DefaultArgsEnabled()          const { return CONFIG_FLAG_RELEASE(ES6DefaultArgs); }
        bool IsES6DestructuringEnabled()        const { return CONFIG_FLAG_RELEASE(ES6Destructuring); }
        bool IsES6FunctionNameEnabled()         const { return CONFIG_FLAG_RELEASE(ES6FunctionName); }
        bool IsES6FunctionNameFullEnabled()     const { return CONFIG_FLAG_RELEASE(ES6FunctionNameFull); }
        bool IsES6GeneratorsEnabled()           const { return CONFIG_FLAG_RELEASE(ES6Generators); }
        bool IsES6IteratorsEnabled()            const { return CONFIG_FLAG_RELEASE(ES6Iterators); }
        bool IsES6IsConcatSpreadableEnabled()   const { return CONFIG_FLAG_RELEASE(ES6IsConcatSpreadable); }
        bool IsES6LambdaEnabled()               const { return CONFIG_FLAG_RELEASE(ES6Lambda); }
        bool IsES6MathExtensionsEnabled()       const { return CONFIG_FLAG_RELEASE(ES6Math); }
        bool IsES6ObjectExtensionsEnabled()     const { return CONFIG_FLAG_RELEASE(ES6Object); }
        bool IsES6NumberExtensionsEnabled()     const { return CONFIG_FLAG_RELEASE(ES6Number); }
        bool IsES6NumericLiteralEnabled()       const { return CONFIG_FLAG_RELEASE(ES6NumericLiterals); }
        bool IsES6ObjectLiteralsEnabled()       const { return CONFIG_FLAG_RELEASE(ES6ObjectLiterals); }
        bool IsES6PromiseEnabled()              const { return CONFIG_FLAG_RELEASE(ES6Promise); }
        bool IsES6ProxyEnabled()                const { return CONFIG_FLAG_RELEASE(ES6Proxy); }
        bool IsES6RestEnabled()                 const { return CONFIG_FLAG_RELEASE(ES6Rest); }
        bool IsES6SpreadEnabled()               const { return CONFIG_FLAG_RELEASE(ES6Spread); }
        bool IsES6StringExtensionsEnabled()     const { return CONFIG_FLAG_RELEASE(ES6String); }
        bool IsES6StringPrototypeFixEnabled()   const { return CONFIG_FLAG_RELEASE(ES6StringPrototypeFixes); }
        bool IsES6StringTemplateEnabled()       const { return CONFIG_FLAG_RELEASE(ES6StringTemplate); }
        bool IsES6PrototypeChain()              const { return CONFIG_FLAG_RELEASE(ES6PrototypeChain); }
        bool IsES6SuperEnabled()                const { return CONFIG_FLAG_RELEASE(ES6Super) || BinaryFeatureControl::LanguageService(); } // Need to remove the LanguageService check once the feature is enabled by default
        bool IsES6SymbolEnabled()               const { return CONFIG_FLAG_RELEASE(ES6Symbol); }
        bool IsES6ToPrimitiveEnabled()          const { return CONFIG_FLAG_RELEASE(ES6ToPrimitive); }
        bool IsES6ToStringTagEnabled()          const { return CONFIG_FLAG_RELEASE(ES6ToStringTag); }
        bool IsES6TypedArrayExtensionsEnabled() const { return CONFIG_FLAG_RELEASE(ES6TypedArrayExtensions); }
        bool IsES6UnicodeExtensionsEnabled()    const { return CONFIG_FLAG_RELEASE(ES6Unicode); }
        bool IsES6UnscopablesEnabled()          const { return CONFIG_FLAG_RELEASE(ES6Unscopables); }
        bool IsES6WeakSetEnabled()              const { return CONFIG_FLAG_RELEASE(ES6WeakSet); }
        bool IsES6RegExChangesEnabled()         const { return CONFIG_FLAG_RELEASE(ES6RegExChanges); }
        bool SkipSplitOnNoResult()              const { return CONFIG_FLAG_RELEASE(SkipSplitOnNoResult); }
        bool AreWinRTDelegatesInterfaces()      const { return CONFIG_FLAG(WinRTDelegateInterfaces); }
        bool IsWinRTAdaptiveAppsEnabled()       const { return CONFIG_FLAG_RELEASE(WinRTAdaptiveApps); }

        void ForceNoNative() { this->NoNative = true; }
        void ForceNative() { this->NoNative = false; }
        bool IsNoNative() const { return this->NoNative; }

        void SetHostType(long hostType) { this->HostType = hostType; }
        void SetWinRTConstructorAllowed(bool allowed) { this->WinRTConstructorAllowed = allowed; }
        void SetCanOptimizeGlobalLookupFlag(BOOL f){ this->fCanOptimizeGlobalLookup = f;}
        BOOL CanOptimizeGlobalLookup() const { return this->fCanOptimizeGlobalLookup;}
        bool IsOptimizedForManyInstances() const { return isOptimizedForManyInstances; }        
        bool IsBlockScopeEnabled() const { return true; }
        bool IsLetAndConstEnabled() const { return CONFIG_FLAG(LetConst); }
        bool BindDeferredPidRefs() const { return IsLetAndConstEnabled(); }        
        void CopyFrom(ScriptConfiguration& other) { this->HostType = other.HostType; this->WinRTConstructorAllowed = other.WinRTConstructorAllowed; this->NoNative = other.NoNative; this->fCanOptimizeGlobalLookup = other.fCanOptimizeGlobalLookup; this->projectionConfiguration = other.projectionConfiguration; }

        ProjectionConfiguration const * GetProjectionConfig() const
        {
#ifdef ENABLE_PROJECTION
            return &projectionConfiguration;
#else
            return nullptr;
#endif
        }
        void SetProjectionTargetVersion(DWORD version)
        {
#ifdef ENABLE_PROJECTION
            projectionConfiguration.SetTargetVersion(version);
#endif
        }

        bool IsWinRTEnabled()           const { return (GetHostType() == Js::HostTypeApplication) || (GetHostType() == Js::HostTypeWebview); }

        bool IsWinRTConstructorAllowed() const { return (GetHostType() != Js::HostTypeWebview) || this->WinRTConstructorAllowed; }

    private:

        // Per script configurations        
        Number HostType;    // One of enum HostType values (see ConfigFlagsTable.h).
        bool WinRTConstructorAllowed;  // whether allow constructor in webview host type. Also note that this is not a security feature.
        bool NoNative;
        BOOL fCanOptimizeGlobalLookup;
        const bool isOptimizedForManyInstances;        

#ifdef ENABLE_PROJECTION
        ProjectionConfiguration projectionConfiguration;
#endif
    };

    struct ScriptEntryExitRecord
    {
        BOOL hasCaller : 1;
        BOOL hasReentered : 1;
#if DBG_DUMP
        BOOL isCallRoot : 1;
#endif
#if DBG || defined(PROFILE_EXEC)
        BOOL leaveForHost : 1;
#endif
#if DBG
        BOOL leaveForAsyncHostOperation : 1;
#endif
#ifdef CHECK_STACKWALK_EXCEPTION
        BOOL ignoreStackWalkException: 1;
#endif
        Js::ImplicitCallFlags savedImplicitCallFlags;

        void * returnAddrOfScriptEntryFunction;
        void * frameIdOfScriptExitFunction; // the frameAddres in x86, the return address in amd64/arm_soc
        ScriptContext * scriptContext;
        struct ScriptEntryExitRecord * next;

#if defined(_M_IX86) && defined(DBG)
        void * scriptEntryFS0;
#endif
#ifdef EXCEPTION_CHECK
        ExceptionType handledExceptionType;
#endif
    };

    static const unsigned int EvalMRUSize = 15;
    typedef JsUtil::BaseDictionary<DWORD_PTR, SourceContextInfo *, Recycler, PowerOf2SizePolicy> SourceContextInfoMap;
    typedef JsUtil::BaseDictionary<uint, SourceContextInfo *, Recycler, PowerOf2SizePolicy> DynamicSourceContextInfoMap;

    typedef JsUtil::BaseDictionary<EvalMapString, ScriptFunction*, RecyclerNonLeafAllocator, PrimeSizePolicy> SecondLevelEvalCache;
    typedef TwoLevelHashRecord<FastEvalMapString, ScriptFunction*, SecondLevelEvalCache, EvalMapString> EvalMapRecord;
    typedef JsUtil::Cache<FastEvalMapString, EvalMapRecord*, RecyclerNonLeafAllocator, PrimeSizePolicy, JsUtil::MRURetentionPolicy<FastEvalMapString, EvalMRUSize>, FastEvalMapStringComparer> EvalCacheTopLevelDictionary;
    typedef SList<Js::FunctionProxy*, Recycler> FunctionReferenceList;
    typedef JsUtil::Cache<EvalMapString, ParseableFunctionInfo*, RecyclerNonLeafAllocator, PrimeSizePolicy, JsUtil::MRURetentionPolicy<EvalMapString, EvalMRUSize>> NewFunctionCache;
    typedef JsUtil::BaseDictionary<ParseableFunctionInfo*, ParseableFunctionInfo*, Recycler, PrimeSizePolicy, RecyclerPointerComparer> ParseableFunctionInfoMap;
    // This is the dictionary used by script context to cache the eval.
    typedef TwoLevelHashDictionary<FastEvalMapString, ScriptFunction*, EvalMapRecord, EvalCacheTopLevelDictionary, EvalMapString> EvalCacheDictionary;

    struct PropertyStringMap 
    {        
        PropertyString* strLen2[80];

        __inline static uint PStrMapIndex(wchar_t ch)
        {
            Assert(ch >= '0' && ch <= 'z');
            return ch - '0';
        }
    };

    typedef JsUtil::BaseDictionary<Js::FunctionInfo*, IR::JnHelperMethod, ArenaAllocator, PowerOf2SizePolicy> DOMFastPathIRHelperMap;

    // valid if object!= NULL
    struct EnumeratedObjectCache {
        static const int kMaxCachedPropStrings=16;
        DynamicObject* object;
        DynamicType* type;
        PropertyString* propertyStrings[kMaxCachedPropStrings];
        int validPropStrings;
    };

    // Holder for all cached pointers. These are allocated on a guest arena
    // ensuring they cause the related objects to be pinned.
    struct Cache
    {
        JavascriptString * lastNumberToStringRadix10String;
        EnumeratedObjectCache enumObjCache;
        JavascriptString * lastUtcTimeFromStrString;
        TypePath* rootPath;
        EvalCacheDictionary* evalCacheDictionary;
        EvalCacheDictionary* indirectEvalCacheDictionary;
        NewFunctionCache* newFunctionCache;
        RegexPatternMruMap *dynamicRegexMap;
        ParseableFunctionInfoMap *copyOnWriteParseableFunctionInfoMap;
        SourceContextInfoMap* sourceContextInfoMap;   // maps host provided context cookie to the URL of the script buffer passed.
        DynamicSourceContextInfoMap* dynamicSourceContextInfoMap;
        SourceContextInfo* noContextSourceContextInfo;
        SRCINFO* noContextGlobalSourceInfo;
        SRCINFO const ** moduleSrcInfo;
    };

    // Represents the different modes that the debugger can be placed into.
    enum DebuggerMode
    {
        // The debugger is not running so the engine can be running
        // in JITed mode.
        NotDebugging,

        // The debugger is not running but PDM has been created and
        // source rundown was performed to register script documents.
        SourceRundown,

        // The debugger is running which means that the engine is
        // running in interpreted mode.
        Debugging,
    };

    struct NativeModule
    {
        BYTE *base;
        BYTE *code;
        size_t codeSize;
        DWORD *exports;
        uint exportCount;
        BYTE *nativeMap;
        BYTE *nativeThrowMap;
        size_t imageBase;
        IMAGE_SECTION_HEADER *textHeader;
        IMAGE_SECTION_HEADER *relocHeader;
        BYTE *textSection;
        size_t textSectionSize;
#if defined(_M_X64) || defined(_M_ARM32_OR_ARM64)
        RUNTIME_FUNCTION *pdataTable;
        uint pdataCount;
        void *functionTableHandle;
#endif
        bool loadedInMemory;

        NativeModule() :
            base(nullptr),
            code(nullptr),
            codeSize(0),
            exports(nullptr),
            exportCount(0),
            nativeMap(nullptr),
            nativeThrowMap(nullptr),
            imageBase(0),
            textHeader(nullptr),
            relocHeader(nullptr),
            textSection(nullptr),
            textSectionSize(0),
#if defined(_M_X64) || defined(_M_ARM32_OR_ARM64)
            pdataTable(nullptr),
            pdataCount(0),
            functionTableHandle(nullptr),
#endif
            loadedInMemory(false)
        {
        }
    };

    class ScriptContext : public ScriptContextBase
    {
        friend class LowererMD;
        friend class RemoteScriptContext;
    public:
        static DWORD GetThreadContextOffset() { return offsetof(ScriptContext, threadContext); }
        static DWORD GetOptimizationOverridesOffset() { return offsetof(ScriptContext, optimizationOverrides); }
        static DWORD GetRecyclerOffset() { return offsetof(ScriptContext, recycler); }        
        static DWORD GetNumberAllocatorOffset() { return offsetof(ScriptContext, numberAllocator); }
        static DWORD GetAsmIntDbValOffset() { return offsetof(ScriptContext, retAsmIntDbVal); }

        ScriptContext *next;
        ScriptContext *prev;
        double retAsmIntDbVal; // stores the double & float result for Asm interpreter
#ifdef SIMD_JS_ENABLED
        AsmJsSIMDValue retAsmSimdVal; // stores raw simd result for Asm interpreter
        static DWORD GetAsmSimdValOffset() { return offsetof(ScriptContext, retAsmSimdVal); }
#endif
        ScriptContextOptimizationOverrideInfo optimizationOverrides;
        AuthoringData *authoringData;
        Js::Var GetTrackingValue(Js::RecyclableObject *value);

        Js::JavascriptMethod CurrentThunk;
        Js::JavascriptMethod CurrentCrossSiteThunk;
        Js::JavascriptMethod DeferredParsingThunk;
        Js::JavascriptMethod DeferredDeserializationThunk;
        Js::JavascriptMethod DispatchDefaultInvoke;
        Js::JavascriptMethod DispatchProfileInoke;

        typedef HRESULT (*GetDocumentContextFunction)(
            ScriptContext *pContext,
            Js::FunctionBody *pFunctionBody,
            IDebugDocumentContext **ppDebugDocumentContext);
        GetDocumentContextFunction GetDocumentContext;

        typedef HRESULT (*CleanupDocumentContextFunction)(ScriptContext *pContext);
        CleanupDocumentContextFunction CleanupDocumentContext;

        const ScriptContextBase* GetScriptContextBase() const { return static_cast<const ScriptContextBase*>(this); }

        bool IsUndeclBlockVar(Var var) const { return this->javascriptLibrary->IsUndeclBlockVar(var); }
        
        void TrackPid(const PropertyRecord* propertyRecord)
        {
            if (IsBuiltInPropertyId(propertyRecord->GetPropertyId()) || propertyRecord->IsBound())
            {
                return;
            }
                       
            if (-1 != this->GetLibrary()->EnsureReferencedPropertyRecordList()->AddNew(propertyRecord))
            {
                RECYCLER_PERF_COUNTER_INC(PropertyRecordBindReference);
            }            
        }
        void TrackPid(PropertyId propertyId)
        {
            if (IsBuiltInPropertyId(propertyId))
            {
                return;
            }
            const PropertyRecord* propertyRecord = this->GetPropertyName(propertyId);
            Assert(propertyRecord != null);
            this->TrackPid(propertyRecord);
        }

        bool IsTrackedPropertyId(Js::PropertyId propertyId)
        {
            if (IsBuiltInPropertyId(propertyId))
            {
                return true;
            }
            const PropertyRecord* propertyRecord = this->GetPropertyName(propertyId);
            Assert(propertyRecord != null);
            if (propertyRecord->IsBound())
            {
                return true;
            }
            JavascriptLibrary::ReferencedPropertyRecordHashSet * referencedPropertyRecords 
                = this->GetLibrary()->GetReferencedPropertyRecordList();
            return referencedPropertyRecords && referencedPropertyRecords->Contains(propertyRecord);           
        }

        void InvalidateHostObjects()
        {
            AssertMsg(!isClosed, "Host Object invalidation should occur before the engine is fully closed. Figure our how isClosed got set beforehand.");
            isInvalidatedForHostObjects = true;
        }
        bool IsInvalidatedForHostObjects()
        {
            return isInvalidatedForHostObjects;
        }

        void EmitStackTraceEvent(__in UINT64 operationID, __in USHORT maxFrameCount, bool emitV2AsyncStackEvent);

        void SetIsDiagnosticsScriptContext(bool set) { this->isDiagnosticsScriptContext = set; }
        bool IsDiagnosticsScriptContext() const { return this->isDiagnosticsScriptContext; }

        // Debugger methods.
        DebuggerMode GetDebuggerMode() const { return this->debuggerMode; }
        void SetDebuggerMode(DebuggerMode mode);
        bool IsInNonDebugMode() const { return this->GetDebuggerMode() == DebuggerMode::NotDebugging; }
        bool IsInSourceRundownMode() const { return this->GetDebuggerMode() == DebuggerMode::SourceRundown; }
        bool IsInDebugMode() const { return this->GetDebuggerMode() == DebuggerMode::Debugging; }   // TODO: Fast F12: assert that we are called from main thread.
        bool IsInDebugOrSourceRundownMode() const { return this->IsInDebugMode() || this->IsInSourceRundownMode(); }
        bool IsRunningScript() const { return this->threadContext->GetScriptEntryExit() != nullptr; }
        void SetInDebugMode() { this->SetDebuggerMode(DebuggerMode::Debugging); }
        void SetInSourceRundownMode() { this->SetDebuggerMode(DebuggerMode::SourceRundown); }

        typedef JsUtil::List<RecyclerWeakReference<Utf8SourceInfo>*, Recycler, false, Js::WeakRefFreeListedRemovePolicy> CalleeSourceList;
        RecyclerRootPtr<CalleeSourceList> calleeUtf8SourceInfoList;
        void AddCalleeSourceInfoToList(Utf8SourceInfo* sourceInfo);
        bool HaveCalleeSources() { return calleeUtf8SourceInfoList && !calleeUtf8SourceInfoList->Empty(); }

        template<class TMapFunction>
        void MapCalleeSources(TMapFunction map)
        {
            if (this->HaveCalleeSources())
            {
                calleeUtf8SourceInfoList->Map([&](uint i, RecyclerWeakReference<Js::Utf8SourceInfo>* sourceInfoWeakRef)
                {
                    if (calleeUtf8SourceInfoList->IsItemValid(i))
                    {
                        Js::Utf8SourceInfo* sourceInfo = sourceInfoWeakRef->Get();
                        map(sourceInfo);
                    }
                });
            }
            if (calleeUtf8SourceInfoList)
            {
                calleeUtf8SourceInfoList.Unroot(this->GetRecycler());
            }
        }

#ifdef ASMJS_PLAT
        inline AsmJsCodeGenerator* GetAsmJsCodeGenerator() const{return asmJsCodeGenerator;}
        AsmJsCodeGenerator* InitAsmJsCodeGenerator();
#endif

        bool IsExceptionWrapperForBuiltInsEnabled();
        bool IsEnumerateNonUserFunctionsOnly() const { return m_enumerateNonUserFunctionsOnly; }
        bool IsTraceDomCall() const { return !!m_fTraceDomCall; }
        static bool IsExceptionWrapperForBuiltInsEnabled(ScriptContext* scriptContext);
        static bool IsExceptionWrapperForHelpersEnabled(ScriptContext* scriptContext);

        InlineCache * GetValueOfInlineCache() const { return valueOfInlineCache;}
        InlineCache * GetToStringInlineCache() const { return toStringInlineCache; }

    private:
        PropertyStringMap* propertyStrings[80];        

        JavascriptFunction* GenerateRootFunction(ParseNodePtr parseTree, uint sourceIndex, Parser* parser, ulong grfscr, CompileScriptException * pse, const wchar_t *rootDisplayName);

        typedef void (*EventHandler)(ScriptContext *);
        ScriptContext ** entryInScriptContextWithInlineCachesRegistry;
        ScriptContext ** entryInScriptContextWithIsInstInlineCachesRegistry;
        ScriptContext ** registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext;

        typedef JsUtil::BaseDictionary<RecyclableObject *, RecyclableObject *, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> InstanceMap;
        InstanceMap *copyOnWriteMap;

        ArenaAllocator generalAllocator;
#ifdef TELEMETRY
        ArenaAllocator telemetryAllocator;
#endif

        ArenaAllocator dynamicProfileInfoAllocator;
        InlineCacheAllocator inlineCacheAllocator;
        IsInstInlineCacheAllocator isInstInlineCacheAllocator;

        ArenaAllocator* interpreterArena;
        ArenaAllocator* guestArena;

        ArenaAllocator* diagnosticArena;
        void ** bindRefChunkCurrent;
        void ** bindRefChunkEnd;

        bool startupComplete; // Indicates if the heuristic startup phase for this script context is complete
        bool isInvalidatedForHostObjects;  // Indicates that we've invalidate all objects in the host so stop calling them.
        bool isEnumeratingRecyclerObjects; // Indicates this scriptContext is enumerating recycler objects. Used by recycler enumerating callbacks to filter out other unrelated scriptContexts.
        bool m_enumerateNonUserFunctionsOnly; // Indicates that recycler enumeration callback will consider only non-user functions (which are built-ins, external, winrt etc).

        ThreadContext* threadContext;
        TypeId  directHostTypeId;

        InlineCache * valueOfInlineCache;
        InlineCache * toStringInlineCache;
        
        typedef JsUtil::BaseHashSet<Js::PropertyId, ArenaAllocator> PropIdSetForConstProp;
        PropIdSetForConstProp * intConstPropsOnGlobalObject;
        PropIdSetForConstProp * intConstPropsOnGlobalUserObject;

        // Debugger fields.
        DebuggerMode debuggerMode;

        void * firstInterpreterFrameReturnAddress;
#ifdef SEPARATE_ARENA
        ArenaAllocator sourceCodeAllocator;
        ArenaAllocator regexAllocator;
#endif
#ifdef NEED_MISC_ALLOCATOR
        ArenaAllocator miscAllocator;
#endif

#if DBG
        SimpleHashTable<void *, uint> * bindRef;
        int m_iProfileSession;
#endif

#ifdef PROFILE_EXEC
        ScriptContextProfiler * profiler;
        bool isProfilerCreated;
        bool disableProfiler;
        bool ensureParentInfo;

        Profiler * CreateProfiler();
#endif
#ifdef PROFILE_MEM
        bool profileMemoryDump;
#endif
#ifdef PROFILE_STRINGS
        StringProfiler* stringProfiler;
#endif

public:
#ifdef PROFILE_TYPES
        int convertNullToSimpleCount;
        int convertNullToSimpleDictionaryCount;
        int convertNullToDictionaryCount;
        int convertDeferredToDictionaryCount;
        int convertDeferredToSimpleDictionaryCount;
        int convertSimpleToDictionaryCount;
        int convertSimpleToSimpleDictionaryCount;
        int convertPathToDictionaryCount1;
        int convertPathToDictionaryCount2;
        int convertPathToDictionaryCount3;
        int convertPathToDictionaryCount4;
        int convertPathToSimpleDictionaryCount;
        int convertSimplePathToPathCount;
        int convertSimpleDictionaryToDictionaryCount;
        int convertSimpleSharedDictionaryToNonSharedCount;
        int convertSimpleSharedToNonSharedCount;
        int simplePathTypeHandlerCount;
        int pathTypeHandlerCount;
        int promoteCount;
        int cacheCount;
        int branchCount;
        int maxPathLength;
        int typeCount[TypeIds_Limit];
        int instanceCount[TypeIds_Limit];
#endif
#ifdef  PROFILE_OBJECT_LITERALS
        int objectLiteralInstanceCount;
        int objectLiteralPathCount;
        int objectLiteralCount[TypePath::MaxPathTypeHandlerLength];
        int objectLiteralSimpleDictionaryCount;
        uint32 objectLiteralMaxLength;
        int objectLiteralPromoteCount;
        int objectLiteralCacheCount;
        int objectLiteralBranchCount;
#endif
#if DBG_DUMP
        uint byteCodeDataSize;
        uint byteCodeAuxiliaryDataSize;
        uint byteCodeAuxiliaryContextDataSize;
        uint byteCodeHistogram[OpCode::ByteCodeLast];
        uint32 forinCache;
        uint32 forinNoCache;
#endif
#if ARRLOG
        UIntHashTable<ArrLogRec*>* logTable;
#endif
#ifdef BGJIT_STATS
        uint interpretedCount;
        uint funcJITCount;
        uint loopJITCount;
        uint bytecodeJITCount;
        uint interpretedCallsHighPri;
        uint maxFuncInterpret;
        uint jitCodeUsed;
        uint funcJitCodeUsed;
        uint speculativeJitCount;
#endif

#ifdef REJIT_STATS
        // Used to store bailout stats
        typedef JsUtil::BaseDictionary<uint, uint, ArenaAllocator> BailoutStatsMap;

        struct RejitStats
        {
            uint *m_rejitReasonCounts;
            BailoutStatsMap* m_bailoutReasonCounts;

            uint  m_totalRejits;
            uint  m_totalBailouts;

            RejitStats(ScriptContext *scriptContext) : m_totalRejits(0), m_totalBailouts(0)
            {
                m_rejitReasonCounts = AnewArrayZ(scriptContext->GeneralAllocator(), uint, NumRejitReasons);
                m_bailoutReasonCounts = Anew(scriptContext->GeneralAllocator(), BailoutStatsMap, scriptContext->GeneralAllocator());
            }
        };

        void LogDataForFunctionBody(Js::FunctionBody *body, uint idx, bool isRejit);

        void LogRejit(Js::FunctionBody *body, uint reason);
        void LogBailout(Js::FunctionBody *body, uint kind);

        // Used to centrally collect stats for all function bodies.
        typedef JsUtil::WeaklyReferencedKeyDictionary<const Js::FunctionBody, RejitStats*> RejitStatsMap;
        RejitStatsMap* rejitStatsMap;

        BailoutStatsMap *bailoutReasonCounts;
        uint *rejitReasonCounts;
#endif
#ifdef TELEMETRY
        
    private:
        ScriptContextTelemetry* telemetry;
    public:
        ScriptContextTelemetry& GetTelemetry();

#endif
#ifdef INLINE_CACHE_STATS
        // Used to store inline cache stats

        struct CacheData
        {
            uint hits;
            uint misses;
            uint collisions;
            bool isGetCache;
            Js::PropertyId propertyId;

            CacheData() : hits(0), misses(0), collisions(0), isGetCache(false), propertyId(Js::PropertyIds::_none) { }
        };

        // This is a strongly referenced dictionary, since we want to know hit rates for dead caches.
        typedef JsUtil::BaseDictionary<const Js::PolymorphicInlineCache*, CacheData*, Recycler> CacheDataMap;
        CacheDataMap *cacheDataMap;

        void LogCacheUsage(Js::PolymorphicInlineCache *cache, bool isGet, Js::PropertyId propertyId, bool hit, bool collision);
#endif

#ifdef FIELD_ACCESS_STATS
        typedef SList<FieldAccessStatsPtr, Recycler> FieldAccessStatsList;

        struct FieldAccessStatsEntry
        {
            RecyclerWeakReference<FunctionBody>* functionBodyWeakRef;
            FieldAccessStatsList stats;

            FieldAccessStatsEntry(RecyclerWeakReference<FunctionBody>* functionBodyWeakRef, Recycler* recycler)
                : functionBodyWeakRef(functionBodyWeakRef), stats(recycler) {}
        };

        typedef JsUtil::BaseDictionary<uint, FieldAccessStatsEntry*, Recycler> FieldAccessStatsByFunctionNumberMap;

        FieldAccessStatsByFunctionNumberMap* fieldAccessStatsByFunctionNumber;

        void RecordFieldAccessStats(FunctionBody* functionBody, FieldAccessStatsPtr fieldAccessStats);
#endif

#ifdef MISSING_PROPERTY_STATS
        int missingPropertyMisses;
        int missingPropertyHits;
        int missingPropertyCacheAttempts;

        void RecordMissingPropertyMiss();
        void RecordMissingPropertyHit();
        void RecordMissingPropertyCacheAttempt();
#endif

        bool IsIntConstPropertyOnGlobalObject(Js::PropertyId propId);
        void TrackIntConstPropertyOnGlobalObject(Js::PropertyId propId);
        bool IsIntConstPropertyOnGlobalUserObject(Js::PropertyId propertyId);
        void TrackIntConstPropertyOnGlobalUserObject(Js::PropertyId propertyId);

        void AddNativeModule(BYTE *moduleBase, NativeModule *module);
        bool TryGetNativeModule(BYTE *moduleBase, NativeModule **nativeModule);
        template <typename TConditionalFunction>
        bool AnyNativeModule(TConditionalFunction function)
        {
            return nativeModules && nativeModules->AnyValue(function);
        }
        template <typename fn>
        void EachNativeModule(fn function)
        {
            if (nativeModules)
            {
                nativeModules->EachValue(function);
            }
        }

private:
        //
        // Regex globals
        //
#if ENABLE_REGEX_CONFIG_OPTIONS
        UnifiedRegex::DebugWriter* regexDebugWriter;
        UnifiedRegex::RegexStatsDatabase* regexStatsDatabase;
#endif
        UnifiedRegex::TrigramAlphabet* trigramAlphabet;
        UnifiedRegex::RegexStacks *regexStacks;

        FunctionReferenceList* dynamicFunctionReference;
        uint dynamicFunctionReferenceDepth;

        JsUtil::Stack<Var>* operationStack;
        Recycler* recycler;
        RecyclerJavascriptNumberAllocator numberAllocator;

        ScriptConfiguration config;
        CharClassifier *charClassifier;
        InterpreterThunkEmitter* interpreterThunkEmitter;
        BackgroundParser *backgroundParser;
#ifdef ASMJS_PLAT
        InterpreterThunkEmitter* asmJsInterpreterThunkEmitter;
        AsmJsCodeGenerator* asmJsCodeGenerator;
#endif
#if ENABLE_NATIVE_CODEGEN
        NativeCodeGenerator* nativeCodeGen;
#endif

        TIME_ZONE_INFORMATION timeZoneInfo;
        uint lastTimeZoneUpdateTickCount;
        DaylightTimeHelper daylightTimeHelper;

        HostScriptContext * hostScriptContext;
        HaltCallback* scriptEngineHaltCallback;
        EventHandler scriptStartEventHandler;
        EventHandler scriptEndEventHandler;
#ifdef FAULT_INJECTION    
        EventHandler disposeScriptByFaultInjectionEventHandler;
#endif

        LargeUIntHashTable<JavascriptString *> *integerStringMap;

        double lastNumberToStringRadix10;
        double lastUtcTimeFromStr;

        bool referencesSharedDynamicSourceContextInfo;

        // We could delay the actual close after callRootLevel is 0.
        // this is to indicate the actual close is called once only.
        bool isScriptContextActuallyClosed;
#if DBG
        bool isInitialized;
        bool isCloningGlobal;
#endif
        bool fastDOMenabled;
        bool hasRegisteredInlineCache;
        bool hasRegisteredIsInstInlineCache;
        bool deferredBody;
        bool isPerformingNonreentrantWork;        
        bool isDiagnosticsScriptContext;   // mentions that current script context belongs to the diagnostics OM.

        size_t sourceSize;

        void CleanSourceListInternal(bool calledDuringMark);
        typedef JsUtil::List<RecyclerWeakReference<Utf8SourceInfo>*, Recycler, false, Js::FreeListedRemovePolicy> SourceList;
        RecyclerRootPtr<SourceList> sourceList;

        IActiveScriptProfilerHeapEnum* heapEnum;

        // Profiler Probes
        // In future these can be list of callbacks ?
        // Profiler Callback information
        IActiveScriptProfilerCallback *m_pProfileCallback;
        BOOL m_fTraceFunctionCall;
        BOOL m_fTraceNativeFunctionCall;
        DWORD m_dwEventMask;

        IActiveScriptProfilerCallback2 *m_pProfileCallback2;
        BOOL m_fTraceDomCall;
        BOOL m_inProfileCallback;
        
#if DBG_DUMP || defined(DYNAMIC_PROFILE_STORAGE) || defined(RUNTIME_DATA_COLLECTION)
        RecyclerRootPtr<SListBase<DynamicProfileInfo *>> profileInfoList;
#endif
#if DEBUG
        static int scriptContextCount;
#endif
#ifdef MUTATORS
        Js::SourceMutator *sourceMutator;
#endif

        // List of weak reference dictionaries. We'll walk through them
        // and clean them up post-collection
        // IWeakReferenceDictionary objects are added to this list by calling
        // RegisterWeakReferenceDictionary. If you use JsUtil::WeakReferenceDictionary,
        // which also exposes the IWeakReferenceDictionary interface, it'll
        // automatically register the dictionary on the script context
        SListBase<JsUtil::IWeakReferenceDictionary*> weakReferenceDictionaryList;
        bool isWeakReferenceDictionaryListCleared;

        typedef HRESULT (*DbgRegisterFunctionType)(ScriptContext *, FunctionBody *, LPCWSTR title);
        DbgRegisterFunctionType dbgRegisterFunction;

        typedef void(*RaiseMessageToDebuggerFunctionType)(ScriptContext *, DWORD, LPCWSTR, LPCWSTR);
        RaiseMessageToDebuggerFunctionType raiseMessageToDebuggerFunctionType;

        typedef void(*TransitionToDebugModeIfFirstSourceFn)(ScriptContext *, Utf8SourceInfo *);
        TransitionToDebugModeIfFirstSourceFn transitionToDebugModeIfFirstSourceFn;

        inline HRESULT RegisterObject();
        inline HRESULT RegisterArray();
        inline HRESULT RegisterBoolean();
        inline HRESULT RegisterDate();
        inline HRESULT RegisterFunction();
        inline HRESULT RegisterMath();
        inline HRESULT RegisterNumber();
        inline HRESULT RegisterString();
        inline HRESULT RegisterRegExp();
        inline HRESULT RegisterJSON();
        inline HRESULT RegisterPixelArray();
        inline HRESULT RegisterMap();
        inline HRESULT RegisterSet();
        inline HRESULT RegisterWeakMap();
        inline HRESULT RegisterWeakSet();
        inline HRESULT RegisterSymbol();
        inline HRESULT RegisterArrayIterator();
        inline HRESULT RegisterMapIterator();
        inline HRESULT RegisterSetIterator();
        inline HRESULT RegisterStringIterator();
        inline HRESULT RegisterEnumeratorIterator();
        inline HRESULT RegisterTypedArray();
        inline HRESULT RegisterPromise();
        inline HRESULT RegisterProxy();
        inline HRESULT RegisterReflect();
        inline HRESULT RegisterGenerator();
#ifdef SIMD_JS_ENABLED
        inline HRESULT RegisterSIMD();
#endif

        // Theoretically we can put this in ThreadContext; don't want to keep the dictionary forever, and preserve the possibility of
        // using JavascriptFunction as key.
        DOMFastPathIRHelperMap* domFastPathIRHelperMap;

#ifdef IR_VIEWER
        inline HRESULT RegisterIRViewer();
#endif /* IR_VIEWER */
                
        ScriptContext(ThreadContext* threadContext);
        void InitializeAllocations();
        void InitializePreGlobal();
        void InitializePostGlobal(bool initializingCopy);        

        // Source Info
        void EnsureSourceContextInfoMap();
        void EnsureDynamicSourceContextInfoMap();

        uint moduleSrcInfoCount;
#ifdef RUNTIME_DATA_COLLECTION
        time_t createTime;
#endif
        wchar_t const * url;

        void PrintStats();
        BOOL LeaveScriptStartCore(void * frameAddress, bool leaveForHost);

        void InternalClose();

    public:
        static const int kArrayMatchCh=72;
        static const int kMaxArrayMatchIndex=8192;
        short arrayMatchItems[kArrayMatchCh];
        bool arrayMatchInit;

#ifdef LEAK_REPORT
        LeakReport::UrlRecord * urlRecord;
        bool isRootTrackerScriptContext;
#endif

        DaylightTimeHelper *GetDaylightTimeHelper() { return &daylightTimeHelper; }

        bool IsClosed() const { return isClosed; }
        bool IsActuallyClosed() const { return isScriptContextActuallyClosed; }
        bool IsClosedNativeCodeGenerator() const 
        { 
            return !nativeCodeGen || ::IsClosedNativeCodeGenerator(nativeCodeGen);
        }
        
        void SetDirectHostTypeId(TypeId typeId) {directHostTypeId = typeId; }
        TypeId GetDirectHostTypeId() const { return directHostTypeId; }

        TypePath* GetRootPath() { return cache->rootPath; }

        DOMFastPathIRHelperMap* EnsureDOMFastPathIRHelperMap();

        wchar_t const * GetUrl() const { return url; }
        void SetUrl(BSTR bstr);
#ifdef RUNTIME_DATA_COLLECTION
        time_t GetCreateTime() const { return createTime; }
        uint GetAllocId() const { return allocId; }
#endif
        void InitializeArrayMatch()
        {
            if (!arrayMatchInit)
            {
                for (int i=0;i<kArrayMatchCh;i++)
                {
                    arrayMatchItems[i]= -1;
                }
                arrayMatchInit=true;
            }
        }

#ifdef HEAP_ENUMERATION_VALIDATION
        bool IsInitialized() { return this->isInitialized; }
#endif
        ProbeContainer diagProbesContainer;

        uint callCount;

        // Guest arena allocated cache holding references that need to be pinned.
        Cache* cache;

        // if the current context is for webworker
        DWORD webWorkerId;
       
        static ScriptContext * New(ThreadContext * threadContext);
        static void Delete(ScriptContext* scriptContext);

        ~ScriptContext();

#ifdef PROFILE_TYPES
        void ProfileTypes();
#endif

#ifdef PROFILE_OBJECT_LITERALS
        void ProfileObjectLiteral();
#endif

        //
        // Regex helpers
        //
#if ENABLE_REGEX_CONFIG_OPTIONS
        UnifiedRegex::RegexStatsDatabase* GetRegexStatsDatabase();
        UnifiedRegex::DebugWriter* GetRegexDebugWriter();
#endif
        RegexPatternMruMap* GetDynamicRegexMap() const;

        UnifiedRegex::TrigramAlphabet* GetTrigramAlphabet() { return trigramAlphabet; }

        void SetTrigramAlphabet(__in_xcount(regex::TrigramAlphabet::AlphaCount) char* alpha,
            __in_xcount(regex::TrigramAlphabet::AsciiTableSize) char* alphaBits);

        UnifiedRegex::RegexStacks *RegexStacks();
        UnifiedRegex::RegexStacks *AllocRegexStacks();
        UnifiedRegex::RegexStacks *SaveRegexStacks();
        void RestoreRegexStacks(UnifiedRegex::RegexStacks *const contStack);

        void InitializeGlobalObject();
        JavascriptLibrary* GetLibrary() const { return javascriptLibrary; }
        const JavascriptLibraryBase* GetLibraryBase() const { return javascriptLibrary->GetLibraryBase(); }
#if DBG
        BOOL IsCloningGlobal() const { return isCloningGlobal;}
#endif
        void PushObject(Var object);
        Var PopObject();
        BOOL CheckObject(Var object);

        inline bool IsHeapEnumInProgress() { return GetRecycler()->IsHeapEnumInProgress(); }

        bool IsInterpreted() { return config.IsNoNative(); }
        void ForceNoNative() { config.ForceNoNative(); }
        void ForceNative() { config.ForceNative(); }
        ScriptConfiguration const * GetConfig(void) const { return &config; }
        CharClassifier const * GetCharClassifier(void) const;

        ThreadContext * GetThreadContext() const { return threadContext; }

        TIME_ZONE_INFORMATION * GetTimeZoneInfo()
        {
            uint tickCount = GetTickCount();
            if (tickCount - lastTimeZoneUpdateTickCount > 1000)
            {
                UpdateTimeZoneInfo();
                lastTimeZoneUpdateTickCount = tickCount;
            }
            return &timeZoneInfo;
        }
        void UpdateTimeZoneInfo();

        static const int MaxEvalSourceSize = 400;

        bool IsInEvalMap(FastEvalMapString const& key, BOOL isIndirect, ScriptFunction **ppFuncScript);
        void AddToEvalMap(FastEvalMapString const& key, BOOL isIndirect, ScriptFunction *pFuncScript);

        template <typename TCacheType>
        void CleanDynamicFunctionCache(TCacheType* cacheType);        
        void CleanEvalMapCache(Js::EvalCacheTopLevelDictionary * cacheType);

        template <class TDelegate>
        void MapFunction(TDelegate mapper);

        template <class TDelegate>
        FunctionBody* FindFunction(TDelegate predicate);

        __inline bool EnableEvalMapCleanup() { return CONFIG_FLAG(EnableEvalMapCleanup); };
        void BeginDynamicFunctionReferences();
        void EndDynamicFunctionReferences();
        void RegisterDynamicFunctionReference(FunctionProxy* func);
        uint GetNextSourceContextId();

        bool IsInNewFunctionMap(EvalMapString const& key, ParseableFunctionInfo **ppFuncBody);
        void AddToNewFunctionMap(EvalMapString const& key, ParseableFunctionInfo *pFuncBody);

        SourceContextInfo * GetSourceContextInfo(DWORD_PTR hostSourceContext, IActiveScriptDataCache* profileDataCache);
        SourceContextInfo * GetSourceContextInfo(uint hash);
        SourceContextInfo * CreateSourceContextInfo(uint hash, DWORD_PTR hostSourceContext);
        SourceContextInfo * CreateSourceContextInfo(DWORD_PTR hostSourceContext, wchar_t const * url, size_t len, 
            IActiveScriptDataCache* profileDataCache, wchar_t const * sourceMapUrl = nullptr, size_t sourceMapUrlLen = 0);

#if defined(LEAK_REPORT) || defined(CHECK_MEMORY_LEAK)
        void ClearSourceContextInfoMaps()
        {
          if (this->cache != null)
          {
              this->cache->sourceContextInfoMap = null;
              this->cache->dynamicSourceContextInfoMap = null;
              this->referencesSharedDynamicSourceContextInfo = false;
          }
        }
#endif

#if DBG_DUMP || defined(DYNAMIC_PROFILE_STORAGE) || defined(RUNTIME_DATA_COLLECTION)
        void ClearDynamicProfileList()
        {
            if (profileInfoList)
            {
                profileInfoList->Reset();
                profileInfoList.Unroot(this->recycler);
            }
        }

        SListBase<DynamicProfileInfo *> * GetProfileInfoList() { return profileInfoList; }
#endif

        SRCINFO const * GetModuleSrcInfo(Js::ModuleID moduleID);
        SourceContextInfoMap* GetSourceContextInfoMap()
        {
            return (this->cache ? this->cache->sourceContextInfoMap : null);
        }
        DynamicSourceContextInfoMap* GetDynamicSourceContextInfoMap()
        {
            return (this->cache ? this->cache->dynamicSourceContextInfoMap : null);
        }

        void SetFirstInterpreterFrameReturnAddress(void * returnAddress) { firstInterpreterFrameReturnAddress = returnAddress;}
        void *GetFirstInterpreterFrameReturnAddress() { return firstInterpreterFrameReturnAddress;}

        void CleanupWeakReferenceDictionaries();

        void Initialize();
        bool Close(bool inDestructor);
        void MarkForClose();
        void SetHostType(long hostType) { config.SetHostType(hostType); }
        void SetWinRTConstructorAllowed(bool allowed) { config.SetWinRTConstructorAllowed(allowed); }
        void SetCanOptimizeGlobalLookupFlag(BOOL f){ config.SetCanOptimizeGlobalLookupFlag(f);}
        BOOL CanOptimizeGlobalLookup(){ return config.CanOptimizeGlobalLookup();}        
        void SetProjectionTargetVersion(DWORD version) { config.SetProjectionTargetVersion(version); }

        bool IsClosed() { return isClosed; }
        bool IsFastDOMEnabled() { return fastDOMenabled; }
        void SetFastDOMenabled();
        BOOL VerifyAlive(BOOL isJSFunction = FALSE, ScriptContext* requestScriptContext = NULL);
        void VerifyAliveWithHostContext(BOOL isJSFunction, HostScriptContext* requestHostScriptContext);
        void AddFunctionBodyToPropIdMap(FunctionBody* body);

        void BindReference(void* addr);

        void InitPropertyStringMap(int i);        
        PropertyString* AddPropertyString2(const Js::PropertyRecord* propertyRecord);
        PropertyString* CachePropertyString2(const Js::PropertyRecord* propertyRecord);        
        inline PropertyString* GetPropertyString2(wchar_t ch1, wchar_t ch2);
        inline void FindPropertyRecord(__in LPCWSTR pszPropertyName, __in int propertyNameLength, PropertyRecord const** propertyRecord);
        inline JsUtil::List<const RecyclerWeakReference<Js::PropertyRecord const>*>* FindPropertyIdNoCase(__in LPCWSTR pszPropertyName, __in int propertyNameLength);

        inline void FindPropertyRecord(JavascriptString* pstName, PropertyRecord const** propertyRecord);
        PropertyRecord const * GetPropertyName(PropertyId propertyId);
        PropertyRecord const * GetPropertyNameLocked(PropertyId propertyId);
        inline void GetOrAddPropertyRecord(JsUtil::CharacterBuffer<WCHAR> const& propName, PropertyRecord const** propertyRecord);
        template <size_t N> void GetOrAddPropertyRecord(const wchar_t(&propertyName)[N], PropertyRecord const** propertyRecord);
        inline PropertyId GetOrAddPropertyIdTracked(JsUtil::CharacterBuffer<WCHAR> const& propName);
        template <size_t N> PropertyId GetOrAddPropertyIdTracked(const wchar_t(&propertyName)[N]);
        inline PropertyId GetOrAddPropertyIdTracked(__in_ecount(propertyNameLength) LPCWSTR pszPropertyName, __in int propertyNameLength);
        inline void GetOrAddPropertyRecord(__in_ecount(propertyNameLength) LPCWSTR pszPropertyName, __in int propertyNameLength, PropertyRecord const** propertyRecord);
        inline BOOL IsNumericPropertyId(PropertyId propertyId, uint32* value);

        inline void RegisterWeakReferenceDictionary(JsUtil::IWeakReferenceDictionary* weakReferenceDictionary);
        void ResetWeakReferenceDicitionaryList() { weakReferenceDictionaryList.Reset(); }

        BOOL ReserveStaticTypeIds(__in int first, __in int last);
        TypeId ReserveTypeIds(int count);
        TypeId CreateTypeId();

        WellKnownHostType GetWellKnownHostType(Js::TypeId typeId) { return threadContext->GetWellKnownHostType(typeId); }
        void SetWellKnownHostTypeId(WellKnownHostType wellKnownType, Js::TypeId typeId) { threadContext->SetWellKnownHostTypeId(wellKnownType, typeId); }

        JavascriptFunction* LoadScript(const wchar_t* script, SRCINFO const * pSrcInfo, CompileScriptException * pse, bool isExpression, bool disableDeferredParse, bool isForNativeCode, Utf8SourceInfo** ppSourceInfo, const wchar_t *rootDisplayName);
        JavascriptFunction* LoadScript(LPCUTF8 script, size_t cb, SRCINFO const * pSrcInfo, CompileScriptException * pse, bool isExpression, bool disableDeferredParse, bool isForNativeCode, Utf8SourceInfo** ppSourceInfo, const wchar_t *rootDisplayName);

        ArenaAllocator* GeneralAllocator() { return &generalAllocator; }

#ifdef TELEMETRY
        ArenaAllocator* TelemetryAllocator() { return &telemetryAllocator; }
#endif

#ifdef SEPARATE_ARENA
        ArenaAllocator* SourceCodeAllocator() { return &sourceCodeAllocator; }
        ArenaAllocator* RegexAllocator() { return &regexAllocator; }
#else
        ArenaAllocator* SourceCodeAllocator() { return &generalAllocator; }
        ArenaAllocator* RegexAllocator() { return &generalAllocator; }
#endif
#ifdef NEED_MISC_ALLOCATOR
        ArenaAllocator* MiscAllocator() { return &miscAllocator; }
#endif
        InlineCacheAllocator* GetInlineCacheAllocator() { return &inlineCacheAllocator; }
        IsInstInlineCacheAllocator* GetIsInstInlineCacheAllocator() { return &isInstInlineCacheAllocator; }
        ArenaAllocator* DynamicProfileInfoAllocator() { return &dynamicProfileInfoAllocator; }

        ArenaAllocator* AllocatorForDiagnostics();

        Js::TempArenaAllocatorObject* GetTemporaryAllocator(LPCWSTR name);
        void ReleaseTemporaryAllocator(Js::TempArenaAllocatorObject* tempAllocator);
        Js::TempGuestArenaAllocatorObject* GetTemporaryGuestAllocator(LPCWSTR name);
        void ReleaseTemporaryGuestAllocator(Js::TempGuestArenaAllocatorObject* tempAllocator);

        bool EnsureInterpreterArena(ArenaAllocator **);
        void ReleaseInterpreterArena();

        ArenaAllocator* GetGuestArena() const
        {
            return guestArena;
        }

        void ReleaseGuestArena();

        Recycler* GetRecycler() const { return recycler; }
        RecyclerJavascriptNumberAllocator * GetNumberAllocator() { return &numberAllocator; }
#if ENABLE_NATIVE_CODEGEN
        NativeCodeGenerator * GetNativeCodeGenerator() const { return nativeCodeGen; }
#endif
        BackgroundParser * GetBackgroundParser() const { return backgroundParser; }

        void OnScriptStart(bool isRoot, bool isForcedEnter, bool isScript);
        void OnScriptEnd(bool isRoot, bool isForcedEnd);

        template <bool stackProbe, bool leaveForHost>
        bool LeaveScriptStart(void * frameAddress);
        template <bool leaveForHost>
        void LeaveScriptEnd(void * frameAddress);

        HostScriptContext * GetHostScriptContext() const { return hostScriptContext; }
        void SetHostScriptContext(HostScriptContext *  hostScriptContext);
        void SetScriptEngineHaltCallback(HaltCallback* scriptEngine);
        void ClearHostScriptContext();

        IActiveScriptProfilerHeapEnum* GetHeapEnum();
        void SetHeapEnum(IActiveScriptProfilerHeapEnum* newHeapEnum);
        void ClearHeapEnum();

        void SetScriptStartEventHandler(EventHandler eventHandler);
        void SetScriptEndEventHandler(EventHandler eventHandler);
#ifdef FAULT_INJECTION
        void DisposeScriptContextByFaultInjection();
        void SetDisposeDisposeByFaultInjectionEventHandler(EventHandler eventHandler);
#endif
        EnumeratedObjectCache* GetEnumeratedObjectCache() { return &(cache->enumObjCache); }
        PropertyString* GetPropertyString(PropertyId propertyId);
        void InvalidatePropertyStringCache(PropertyId propertyId, Type* type);
        JavascriptString* GetIntegerString(Var aValue);
        JavascriptString* GetIntegerString(int value);
        JavascriptString* GetIntegerString(uint value);

        void CheckEvalRestriction();

        RecyclableObject* GetMissingPropertyResult(Js::RecyclableObject *instance, Js::PropertyId id);
        RecyclableObject* GetMissingItemResult(Js::RecyclableObject *instance, uint32 index);
        RecyclableObject* GetMissingParameterValue(Js::JavascriptFunction *function, uint32 paramIndex);
        RecyclableObject *GetNullPropertyResult(Js::RecyclableObject *instance, Js::PropertyId id);
        RecyclableObject *GetNullItemResult(Js::RecyclableObject *instance, uint32 index);
        bool GetCopyOnGetEnabled();

        bool HasRecordedException() const { return threadContext->GetRecordedException() != null; }
        Js::JavascriptExceptionObject * GetAndClearRecordedException(bool *considerPassingToDebugger = nullptr);
        void RecordException(Js::JavascriptExceptionObject * exceptionObject, bool propagateToDebugger = false);
        __declspec(noreturn) void RethrowRecordedException(JavascriptExceptionObject::HostWrapperCreateFuncType hostWrapperCreateFunc);

#ifdef ENABLE_NATIVE_CODEGEN
        BOOL IsNativeAddress(void * codeAddr);
#endif

        uint SaveSourceCopy(Utf8SourceInfo* sourceInfo, int cchLength, bool isCesu8);
        bool SaveSourceCopy(Utf8SourceInfo* const sourceInfo, int cchLength, bool isCesu8, uint * index);

        uint SaveSourceNoCopy(Utf8SourceInfo* sourceInfo, int cchLength, bool isCesu8);
        Utf8SourceInfo* CloneSourceCrossContext(Utf8SourceInfo* crossContextSourceInfo, SRCINFO const* srcInfo = NULL);

        void CloneSources(ScriptContext* sourceContext);
        Utf8SourceInfo* GetSource(uint sourceIndex);

        uint SourceCount() const { return (uint)sourceList->Count(); }
        void CleanSourceList() { CleanSourceListInternal(false); }
        SourceList* GetSourceList() const { return sourceList; }
        bool IsItemValidInSourceList(int index);

        template <typename TFunction>
        void MapScript(TFunction mapper)
        {
            this->sourceList->Map([mapper] (int, RecyclerWeakReference<Utf8SourceInfo>* sourceInfoWeakReference)
            {
                Utf8SourceInfo* strongRef = sourceInfoWeakReference->Get();

                if (strongRef)
                {
                    mapper(strongRef);
                }
            });
        }

#ifdef CHECK_STACKWALK_EXCEPTION
        void SetIgnoreStackWalkException() {threadContext->GetScriptEntryExit()->ignoreStackWalkException = true; }
#endif

        // This is used in the debugging scenario where the execution will go to the PDM and the PDM makes call to the engine again.
        // In that scenario we need to enforce the current EER to have 'hasCaller' property set, which will enable the stack walking across frames.
        // Do not call this directly, look for ENFORCE_ENTRYEXITRECORD_HASCALLER macro.
        void EnforceEERHasCaller() { threadContext->GetScriptEntryExit()->hasCaller = true; }

        void DbgRegisterFunction(Js::ParseableFunctionInfo * func, LPCWSTR title)
        {
            if (dbgRegisterFunction != null)
            {
                FunctionBody * functionBody;
                // REVIEW: kinda wasteful to defer parse and then parse again here.
                // Can DbgRegisterFunction accept a ParseableFunctionInfo?
                if (func->IsDeferredParseFunction())
                {
                    functionBody = func->Parse();
                }
                else
                {
                    functionBody = func->GetFunctionBody();
                }
                dbgRegisterFunction(this, functionBody, title);
            }
        }
        void SetDbgRegisterFunction(DbgRegisterFunctionType function)
        {
            dbgRegisterFunction = function;
        }

        void SetRaiseMessageToDebuggerFunction(RaiseMessageToDebuggerFunctionType function)
        {
            raiseMessageToDebuggerFunctionType = function;
        }

        void RaiseMessageToDebugger(DWORD messageType, LPCWSTR message, LPCWSTR url)
        {
            if (raiseMessageToDebuggerFunctionType != nullptr)
            {
                raiseMessageToDebuggerFunctionType(this, messageType, message, url);
            }
        }

        void SetTransitionToDebugModeIfFirstSourceFn(TransitionToDebugModeIfFirstSourceFn function)
        {
            transitionToDebugModeIfFirstSourceFn = function;
        }

        void TransitionToDebugModeIfFirstSource(Utf8SourceInfo *sourceInfo)
        {
            if (transitionToDebugModeIfFirstSourceFn != nullptr)
            {
                transitionToDebugModeIfFirstSourceFn(this, sourceInfo);
            }
        }

        void AddSourceSize(size_t sourceSize)
        {
            this->sourceSize += sourceSize;
            this->threadContext->AddSourceSize(sourceSize);
        }

        size_t GetSourceSize()
        {
            return this->sourceSize;
        }

        BOOL SetDeferredBody(BOOL set)
        {
            bool old = this->deferredBody;
            this->deferredBody = !!set;
            return old;
        }

        BOOL GetDeferredBody(void) const
        {
            return this->deferredBody;
        }

    public:
        void RegisterAsScriptContextWithInlineCaches();
        void RegisterAsScriptContextWithIsInstInlineCaches();
        bool IsRegisteredAsScriptContextWithIsInstInlineCaches();
        void FreeLoopBody(void* codeAddress);
        void FreeFunctionEntryPoint(Js::JavascriptMethod method);

    private:
        void DoRegisterAsScriptContextWithInlineCaches();
        void DoRegisterAsScriptContextWithIsInstInlineCaches();
        uint CloneSource(Utf8SourceInfo* info);
    public:
        void RegisterProtoInlineCache(InlineCache *pCache, PropertyId propId);
        void InvalidateProtoCaches(const PropertyId propertyId);
        void InvalidateAllProtoCaches();
        void RegisterStoreFieldInlineCache(InlineCache *pCache, PropertyId propId);
        void InvalidateStoreFieldCaches(const PropertyId propertyId);
        void InvalidateAllStoreFieldCaches();
        void RegisterIsInstInlineCache(Js::IsInstInlineCache * cache, Js::Var function);
#if DBG
        bool IsIsInstInlineCacheRegistered(Js::IsInstInlineCache * cache, Js::Var function);
#endif
        void ClearInlineCaches();
        void ClearIsInstInlineCaches();
#ifdef PERSISTENT_INLINE_CACHES
        void ClearInlineCachesWithDeadWeakRefs();
#endif
        void ClearScriptContextCaches();
        void RegisterConstructorCache(Js::PropertyId propertyId, Js::ConstructorCache* cache);

    public:
        void RegisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext();
    private:
        void DoRegisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext();
    public:
        void ClearPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesCaches();

    public:
        JavascriptString * GetLastNumberToStringRadix10(double value);
        void SetLastNumberToStringRadix10(double value, JavascriptString * str);
        bool GetLastUtcTimeFromStr(JavascriptString * str, double& dbl);
        void SetLastUtcTimeFromStr(JavascriptString * str, double value);
        bool IsNoContextSourceContextInfo(SourceContextInfo *sourceContextInfo) const
        {
            return sourceContextInfo == cache->noContextSourceContextInfo;
        }

        BOOL IsProfiling()
        {
            return (m_pProfileCallback != NULL);
        }

        BOOL IsInProfileCallback()
        {
            return m_inProfileCallback;
        }

#if DBG
        SourceContextInfo const * GetNoContextSourceContextInfo() const { return cache->noContextSourceContextInfo; }

        int GetProfileSession()
        {
            AssertMsg(m_pProfileCallback != NULL, "Asking for profile session when we arent in any");
            return m_iProfileSession;
        }

        void StartNewProfileSession()
        {
            AssertMsg(m_pProfileCallback != NULL, "New Session when the profiler isnt set to any callback");
            m_iProfileSession++;
        }

        void StopProfileSession()
        {
            AssertMsg(m_pProfileCallback == NULL, "How to stop when there is still the callback out there");
        }

        bool hadProfiled;
        bool HadProfiled() const { return hadProfiled; }
#endif

        SRCINFO *AddHostSrcInfo(SRCINFO const *pSrcInfo);

        inline void CoreSetProfileEventMask(DWORD dwEventMask);
        typedef HRESULT (*RegisterExternalLibraryType)(Js::ScriptContext *pScriptContext);
        HRESULT RegisterProfileProbe(IActiveScriptProfilerCallback *pProfileCallback, DWORD dwEventMask, DWORD dwContext, RegisterExternalLibraryType RegisterExternalLibrary, JavascriptMethod dispatchInvoke);
        HRESULT SetProfileEventMask(DWORD dwEventMask);
        HRESULT DeRegisterProfileProbe(HRESULT hrReason, JavascriptMethod dispatchInvoke);

        HRESULT RegisterScript(Js::FunctionProxy *pFunctionBody, BOOL fRegisterScript = TRUE);

        // Register static and dynamic scripts
        HRESULT RegisterAllScripts();

        // Iterate thru utf8sourceinfo and clear debug document if they are there.
        void EnsureClearDebugDocument();

        // To be called directly only when the thread context is shutting down
        void ShutdownClearSourceLists();

        inline HRESULT RegisterLibraryFunction(const wchar_t *pwszObjectName, const wchar_t *pwszFunctionName, Js::PropertyId functionPropertyId, JavascriptMethod entryPoint);

        HRESULT RegisterBuiltinFunctions(RegisterExternalLibraryType RegisterExternalLibrary);
        void RegisterDebugThunk(bool calledDuringAttach = true);
        void UnRegisterDebugThunk();

        void UpdateRecyclerFunctionEntryPointsForDebugger();
        void SetFunctionInRecyclerToProfileMode(bool enumerateNonUserFunctionsOnly = false);
        static void SetEntryPointToProfileThunk(JavascriptFunction* function);
        static void RestoreEntryPointFromProfileThunk(JavascriptFunction* function);

        static void RecyclerEnumClassEnumeratorCallback(void *address, size_t size);
        static void RecyclerFunctionCallbackForDebugger(void *address, size_t size);

        HRESULT RecreateNativeCodeGenerator();

        HRESULT OnDebuggerAttached();
        HRESULT OnDebuggerDetached();
        HRESULT OnDebuggerAttachedDetached(bool attach);
        void InitializeDebugging(DbgRegisterFunctionType dbgRegisterFunction);
        bool IsForceNoNative();
        bool IsEnumeratingRecyclerObjects() const { return isEnumeratingRecyclerObjects; }

    private:
        class AutoEnumeratingRecyclerObjects
        {
        public:
            AutoEnumeratingRecyclerObjects(ScriptContext* scriptContext):
                m_scriptContext(scriptContext)
            {
                Assert(!m_scriptContext->IsEnumeratingRecyclerObjects());
                m_scriptContext->isEnumeratingRecyclerObjects = true;
            }

            ~AutoEnumeratingRecyclerObjects()
            {
                Assert(m_scriptContext->IsEnumeratingRecyclerObjects());
                m_scriptContext->isEnumeratingRecyclerObjects = false;
            }

        private:
            ScriptContext* m_scriptContext;
        };

#ifdef EDIT_AND_CONTINUE
    private:
        ScriptEditQuery* activeScriptEditQuery;

        void BeginScriptEditEnumFunctions(ScriptEditQuery* scriptEditQuery) { Assert(!activeScriptEditQuery); activeScriptEditQuery = scriptEditQuery; }
        void EndScriptEditEnumFunctions() { Assert(activeScriptEditQuery); activeScriptEditQuery = nullptr; }
    public:
        ScriptEditQuery* GetActiveScriptEditQuery() const { return activeScriptEditQuery; }

        class AutoScriptEditEnumFunctions
        {
        public:
            AutoScriptEditEnumFunctions(ScriptContext* scriptContext, ScriptEditQuery* scriptEditQuery) : m_scriptContext(scriptContext)
            {
                scriptContext->BeginScriptEditEnumFunctions(scriptEditQuery);
            }
            ~AutoScriptEditEnumFunctions() { m_scriptContext->EndScriptEditEnumFunctions(); }
        private:
            ScriptContext* m_scriptContext;
        };
#endif

    private:
        typedef JsUtil::BaseDictionary<JavascriptMethod, Js::PropertyId, ArenaAllocator, PrimeSizePolicy> BuiltinFunctionIdDictionary;
        BuiltinFunctionIdDictionary *m_pBuiltinFunctionIdMap;
        Js::PropertyId GetFunctionNumber(JavascriptMethod entryPoint);
        
        Var CopyTrackingValue(Var value, TypeId valueType);
        static const wchar_t* CopyString(const wchar_t* str, size_t charCount, ArenaAllocator* alloc);

    public:
        ScriptContext *CopyOnWriteCopy(void *initContext, void (*init)(void *, ScriptContext *));
        Var CopyOnWrite(Var value);
        void EnsureCopyOnWriteMap();
        void RecordCopyOnWrite(RecyclableObject *originalValue, RecyclableObject *copiedValue);
        UnifiedRegex::RegexPattern *CopyPattern(UnifiedRegex::RegexPattern *pattern);
        ParseableFunctionInfo *CopyFunction(ParseableFunctionInfo *body);
        void RecordFunctionClone(ParseableFunctionInfo* originalBody, ParseableFunctionInfo* clonedBody);

#ifdef DEBUG
        int CopyOnWriteTableSize() { return copyOnWriteMap ? copyOnWriteMap->Count() : 0; }
#endif

        JavascriptMethod GetNextDynamicAsmJsInterpreterThunk(PVOID* ppDynamicInterpreterThunk);
        JavascriptMethod GetNextDynamicInterpreterThunk(PVOID* ppDynamicInterpreterThunk);
        BOOL IsDynamicInterpreterThunk(void* address);
        void ReleaseDynamicInterpreterThunk(BYTE* address, bool addtoFreeList);

        void SetProfileMode(BOOL fSet);
        static JavascriptMethod GetProfileModeThunk(JavascriptMethod entryPoint);
        static Var ProfileModeThunk_DebugModeWrapper(JavascriptFunction* function, ScriptContext* scriptContext, JavascriptMethod entryPoint, Arguments& args);
        BOOL GetProfileInfo(
            JavascriptFunction* function,
            PROFILER_TOKEN &scriptId,
            PROFILER_TOKEN &functionId);
        static Var DebugProfileProbeThunk(RecyclableObject* function, CallInfo callInfo, ...);
        static JavascriptMethod ProfileModeDeferredParse(ScriptFunction **function);
        static Var ProfileModeDeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...);

        // Thunks for deferred deserialization of function bodies from the byte code cache
        static JavascriptMethod ProfileModeDeferredDeserialize(ScriptFunction* function);
        static Var ProfileModeDeferredDeserializeThunk(RecyclableObject* function, CallInfo callInfo, ...);

        HRESULT OnScriptCompiled(PROFILER_TOKEN scriptId, PROFILER_SCRIPT_TYPE type, IUnknown *pIDebugDocumentContext);
        HRESULT OnFunctionCompiled(
            PROFILER_TOKEN functionId,
            PROFILER_TOKEN scriptId,
            const WCHAR *pwszFunctionName,
            const WCHAR *pwszFunctionNameHint,
            IUnknown *pIDebugDocumentContext);
        HRESULT OnFunctionEnter(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId);
        HRESULT OnFunctionExit(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId);

        bool SetDispatchProfile(bool fSet, JavascriptMethod dispatchInvoke);
        HRESULT OnDispatchFunctionEnter(const WCHAR *pwszFunctionName);
        HRESULT OnDispatchFunctionExit(const WCHAR *pwszFunctionName);

        void OnStartupComplete();
        void SaveStartupProfileAndRelease(bool isSaveOnClose = false);

        static HRESULT FunctionExitSenderThunk(PROFILER_TOKEN functionId, PROFILER_TOKEN scriptId, ScriptContext *pScriptContext);
        static HRESULT FunctionExitByNameSenderThunk(const wchar_t *pwszFunctionName, ScriptContext *pScriptContext);

        void AddDynamicProfileInfo(FunctionBody * functionBody, WriteBarrierPtr<DynamicProfileInfo>* dynamicProfileInfo);
#if DBG || defined(RUNTIME_DATA_COLLECTION)
        uint allocId;
#endif

#ifdef MUTATORS
        Js::SourceMutator* GetSourceMutator() { return sourceMutator; }
#endif

#ifdef PROFILE_EXEC
        void DisableProfiler();
        void SetRecyclerProfiler();
        void SetProfilerFromScriptContext(ScriptContext * scriptContext);
        void ProfileBegin(Js::Phase);
        void ProfileEnd(Js::Phase);
        void ProfileSuspend(Js::Phase, Js::Profiler::SuspendRecord * suspendRecord);
        void ProfileResume(Js::Profiler::SuspendRecord * suspendRecord);
        void ProfilePrint();
        bool IsProfilerCreated() const { return isProfilerCreated; }
#endif

#ifdef PROFILE_MEM
        void DisableProfileMemoryDumpOnDelete() { profileMemoryDump = false; }
#endif

#ifdef PROFILE_STRINGS
        StringProfiler * GetStringProfiler(); // May be null if string profiling not enabled
#endif

    public:
        void SetBuiltInLibraryFunction(JavascriptMethod entryPoint, JavascriptFunction* function);
        JavascriptFunction* GetBuiltInLibraryFunction(JavascriptMethod entryPoint);

    private:
        typedef JsUtil::BaseDictionary<JavascriptMethod, JavascriptFunction*, Recycler, PowerOf2SizePolicy> BuiltInLibraryFunctionMap;
        BuiltInLibraryFunctionMap* builtInLibraryFunctions;

        typedef JsUtil::BaseDictionary<BYTE *, NativeModule *, HeapAllocator> NativeModuleMap;
        NativeModuleMap *nativeModules;

#ifdef RECYCLER_PERF_COUNTERS
        size_t bindReferenceCount;
#endif

        ScriptContext * nextPendingClose;
    public:
        void SetNextPendingClose(ScriptContext * nextPendingClose);
        inline ScriptContext * GetNextPendingClose() const { return nextPendingClose; }

        // Keep track of all breakpoints in order to properly clean up on debugger detach
        bool HasMutationBreakpoints();
        void InsertMutationBreakpoint(Js::MutationBreakpoint *mutationBreakpoint);
    };

    class AutoDynamicCodeReference
    {
    public:
        AutoDynamicCodeReference(ScriptContext* scriptContext):
          m_scriptContext(scriptContext)
          {
              scriptContext->BeginDynamicFunctionReferences();
          }

          ~AutoDynamicCodeReference()
          {
              m_scriptContext->EndDynamicFunctionReferences();
          }

    private:
        ScriptContext* m_scriptContext;
    };
}

#define BEGIN_TEMP_ALLOCATOR(allocator, scriptContext, name) \
    Js::TempArenaAllocatorObject *temp##allocator = scriptContext->GetTemporaryAllocator(name); \
    ArenaAllocator * allocator = temp##allocator->GetAllocator();

#define END_TEMP_ALLOCATOR(allocator, scriptContext) \
    scriptContext->ReleaseTemporaryAllocator(temp##allocator);

#define DECLARE_TEMP_ALLOCATOR(allocator) \
    Js::TempArenaAllocatorObject *temp##allocator = null; \
    ArenaAllocator * allocator = null;

#define ACQUIRE_TEMP_ALLOCATOR(allocator, scriptContext, name) \
    temp##allocator = scriptContext->GetTemporaryAllocator(name); \
    allocator = temp##allocator->GetAllocator();

#define RELEASE_TEMP_ALLOCATOR(allocator, scriptContext) \
    if (temp##allocator) \
    scriptContext->ReleaseTemporaryAllocator(temp##allocator);

#define DECLARE_TEMP_GUEST_ALLOCATOR(allocator) \
    Js::TempGuestArenaAllocatorObject *tempGuest##allocator = null; \
    ArenaAllocator * allocator = null;

#define ACQUIRE_TEMP_GUEST_ALLOCATOR(allocator, scriptContext, name) \
    tempGuest##allocator = scriptContext->GetTemporaryGuestAllocator(name); \
    allocator = tempGuest##allocator->GetAllocator();

#define RELEASE_TEMP_GUEST_ALLOCATOR(allocator, scriptContext) \
    if (tempGuest##allocator) \
    scriptContext->ReleaseTemporaryGuestAllocator(tempGuest##allocator);
