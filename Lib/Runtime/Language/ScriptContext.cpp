//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include <StdAfx.h>
#include "BackEndAPI.h"
#if PROFILE_DICTIONARY
#include "DictionaryStats.h"
#endif
#ifdef F_JSETW
#include <IERESP_mshtml.h>
#include "microsoft-scripting-jscript9.internalevents.h"
#endif

#define DEFINE_OBJECT_NAME(object) const wchar_t *pwszObjectName = L#object;

#define REGISTER_OBJECT(object)\
    if (FAILED(hr = Register##object()))\
    {\
    return hr; \
    }\

#define REG_LIB_FUNC_CORE(pwszObjectName, pwszFunctionName, functionPropertyId, entryPoint)\
    if (FAILED(hr = RegisterLibraryFunction(pwszObjectName, pwszFunctionName, functionPropertyId, entryPoint)))\
    {\
    return hr; \
    }\

#define REG_OBJECTS_DYNAMIC_LIB_FUNC(pwszFunctionName, nFuncNameLen, entryPoint) {\
    Js::PropertyRecord const * propRecord; \
    GetOrAddPropertyRecord(pwszFunctionName, nFuncNameLen, &propRecord); \
    REG_LIB_FUNC_CORE(pwszObjectName, pwszFunctionName, propRecord->GetPropertyId(), entryPoint)\
}

#define REG_LIB_FUNC(pwszObjectName, functionPropertyId, entryPoint)\
    REG_LIB_FUNC_CORE(pwszObjectName, L#functionPropertyId, PropertyIds::##functionPropertyId, entryPoint)\

#define REG_OBJECTS_LIB_FUNC(functionPropertyId, entryPoint)\
    REG_LIB_FUNC(pwszObjectName, functionPropertyId, entryPoint)\

#define REG_OBJECTS_LIB_FUNC2(functionPropertyId, pwszFunctionPropertyName, entryPoint)\
    REG_LIB_FUNC_CORE(pwszObjectName, pwszFunctionPropertyName, PropertyIds::##functionPropertyId, entryPoint)\

#define REG_GLOBAL_LIB_FUNC(functionPropertyId, entryPoint)\
    REG_LIB_FUNC(NULL, functionPropertyId, entryPoint)\

#define REG_GLOBAL_CONSTRUCTOR(functionPropertyId)\
    REG_GLOBAL_LIB_FUNC(functionPropertyId, Javascript##functionPropertyId##::NewInstance)\

#define REGISTER_ERROR_OBJECT(functionPropertyId)\
    REG_GLOBAL_LIB_FUNC(functionPropertyId, JavascriptError::New##functionPropertyId##Instance)\
    REG_LIB_FUNC(L#functionPropertyId, toString, JavascriptError::EntryToString)\

namespace Js
{
    ScriptContext * ScriptContext::New(ThreadContext * threadContext)
    {
        AutoPtr<ScriptContext> scriptContext(HeapNew(ScriptContext, threadContext));
        scriptContext->InitializeAllocations();
        return scriptContext.Detach();
    }

    void ScriptContext::Delete(ScriptContext* scriptContext)
    {
        HeapDelete(scriptContext);
    }

    ScriptContext::ScriptContext(ThreadContext* threadContext) :
        ScriptContextBase(),
        interpreterArena(nullptr),
        dynamicFunctionReference(nullptr),
        moduleSrcInfoCount(0),
        // Regex globals
#if ENABLE_REGEX_CONFIG_OPTIONS
        regexStatsDatabase(0),
        regexDebugWriter(0),
#endif
        trigramAlphabet(nullptr),
        regexStacks(nullptr),
        arrayMatchInit(false),
        config(threadContext->IsOptimizedForManyInstances()),
        backgroundParser(nullptr),
#if ENABLE_NATIVE_CODEGEN
        nativeCodeGen(nullptr),
#endif
        threadContext(threadContext),
        scriptStartEventHandler(nullptr),
        scriptEndEventHandler(nullptr),
#ifdef FAULT_INJECTION
        disposeScriptByFaultInjectionEventHandler(nullptr),
#endif
        integerStringMap(nullptr),
        guestArena(nullptr),
        dbgRegisterFunction(nullptr),
        raiseMessageToDebuggerFunctionType(nullptr),
        transitionToDebugModeIfFirstSourceFn(nullptr),
        lastTimeZoneUpdateTickCount(0),        
        sourceSize(0),
        deferredBody(false),
        isScriptContextActuallyClosed(false),
        isInvalidatedForHostObjects(false),
        fastDOMenabled(false),
        directHostTypeId(TypeIds_GlobalObject),
        isPerformingNonreentrantWork(false),
        isDiagnosticsScriptContext(false),
        m_enumerateNonUserFunctionsOnly(false),
        recycler(threadContext->EnsureRecycler()),
        CurrentThunk(DefaultEntryThunk),
        CurrentCrossSiteThunk(CrossSite::DefaultThunk),
        DeferredParsingThunk(DefaultDeferredParsingThunk),
        DeferredDeserializationThunk(DefaultDeferredDeserializeThunk),
        m_pBuiltinFunctionIdMap(nullptr),
        diagnosticArena(nullptr),
        hostScriptContext(nullptr),
        scriptEngineHaltCallback(nullptr),
        interpreterThunkEmitter(nullptr),
#ifdef ASMJS_PLAT
        asmJsInterpreterThunkEmitter(nullptr),
        asmJsCodeGenerator(nullptr),
#endif
        generalAllocator(L"SC-General", threadContext->GetPageAllocator(), Throw::OutOfMemory),
#ifdef TELEMETRY
        telemetryAllocator(L"SC-Telemetry", threadContext->GetPageAllocator(), Throw::OutOfMemory),
#endif
        dynamicProfileInfoAllocator(L"SC-DynProfileInfo", threadContext->GetPageAllocator(), Throw::OutOfMemory),
#ifdef SEPARATE_ARENA
        sourceCodeAllocator(L"SC-Code", threadContext->GetPageAllocator(), Throw::OutOfMemory),
        regexAllocator(L"SC-Regex", threadContext->GetPageAllocator(), Throw::OutOfMemory),
#endif
#ifdef NEED_MISC_ALLOCATOR
        miscAllocator(L"GC-Misc", threadContext->GetPageAllocator(), Throw::OutOfMemory),
#endif
        inlineCacheAllocator(L"SC-InlineCache", threadContext->GetPageAllocator(), Throw::OutOfMemory),
        isInstInlineCacheAllocator(L"SC-IsInstInlineCache", threadContext->GetPageAllocator(), Throw::OutOfMemory),
        hasRegisteredInlineCache(false),
        hasRegisteredIsInstInlineCache(false),
        entryInScriptContextWithInlineCachesRegistry(nullptr),
        entryInScriptContextWithIsInstInlineCachesRegistry(nullptr),
        registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext(nullptr),
        cache(nullptr),
        bindRefChunkCurrent(nullptr),
        bindRefChunkEnd(nullptr),
        firstInterpreterFrameReturnAddress(nullptr),
        builtInLibraryFunctions(nullptr),
        isWeakReferenceDictionaryListCleared(false),
        referencesSharedDynamicSourceContextInfo(false)
#if DBG
        , isInitialized(false)
        , isCloningGlobal(false)
        , bindRef(nullptr)
#endif
#ifdef REJIT_STATS
        , rejitStatsMap(nullptr)
#endif
#ifdef TELEMETRY
        , telemetry(nullptr)
#endif
#ifdef INLINE_CACHE_STATS
        , cacheDataMap(nullptr)
#endif
#ifdef FIELD_ACCESS_STATS
        , fieldAccessStatsByFunctionNumber(nullptr)
#endif
        , authoringData(nullptr)
        , copyOnWriteMap(nullptr)
        , webWorkerId(Js::Constants::NonWebWorkerContextId)
        , url(L"")
        , startupComplete(false)
        , isEnumeratingRecyclerObjects(false)
#ifdef EDIT_AND_CONTINUE
        , activeScriptEditQuery(nullptr)
#endif
        , heapEnum(nullptr)
#ifdef RECYCLER_PERF_COUNTERS
        , bindReferenceCount(0)
#endif
        , nextPendingClose(nullptr)
        , debuggerMode(DebuggerMode::NotDebugging)
        , m_fTraceDomCall(FALSE)
        , domFastPathIRHelperMap(nullptr)
        , nativeModules(nullptr)
        , intConstPropsOnGlobalObject(nullptr)
        , intConstPropsOnGlobalUserObject(nullptr)
#ifdef MUTATORS
        , sourceMutator(nullptr)
#endif
#ifdef ARRLOG
        , logTable(nullptr)
#endif
#ifdef PROFILE_STRINGS
        , stringProfiler(nullptr)
#endif
    {
       // This may allocate memory and cause exception, but it is ok, as we all we have done so far
       // are field init and those dtor will be called if exception occurs
       threadContext->EnsureAndAddToDiagnostic();

       // Don't use throwing memory allocation in ctor, as exception in ctor doesn't cause the dtor to be called
       // potentially causing memory leaks
       BEGIN_NO_EXCEPTION;

#ifdef RUNTIME_DATA_COLLECTION
        createTime = time(nullptr);
#endif

#ifdef BGJIT_STATS
        interpretedCount = maxFuncInterpret = funcJITCount = bytecodeJITCount = interpretedCallsHighPri = jitCodeUsed = funcJitCodeUsed = loopJITCount = speculativeJitCount = 0;
#endif

#ifdef PROFILE_TYPES
        convertNullToSimpleCount = 0;
        convertNullToSimpleDictionaryCount = 0;
        convertNullToDictionaryCount = 0;
        convertDeferredToDictionaryCount = 0;
        convertDeferredToSimpleDictionaryCount = 0;
        convertSimpleToDictionaryCount = 0;
        convertSimpleToSimpleDictionaryCount = 0;
        convertPathToDictionaryCount1 = 0;
        convertPathToDictionaryCount2 = 0;
        convertPathToDictionaryCount3 = 0;
        convertPathToDictionaryCount4 = 0;
        convertPathToSimpleDictionaryCount = 0;
        convertSimplePathToPathCount = 0;
        convertSimpleDictionaryToDictionaryCount = 0;
        convertSimpleSharedDictionaryToNonSharedCount = 0;
        convertSimpleSharedToNonSharedCount = 0;
        simplePathTypeHandlerCount = 0;
        pathTypeHandlerCount = 0;
        promoteCount = 0;
        cacheCount = 0;
        branchCount = 0;
        maxPathLength = 0;
        memset(typeCount, 0, sizeof(typeCount));
        memset(instanceCount, 0, sizeof(instanceCount));
#endif

#ifdef PROFILE_OBJECT_LITERALS
        objectLiteralInstanceCount = 0;
        objectLiteralPathCount = 0;
        memset(objectLiteralCount, 0, sizeof(objectLiteralCount));
        objectLiteralSimpleDictionaryCount = 0;
        objectLiteralMaxLength = 0;
        objectLiteralPromoteCount = 0;
        objectLiteralCacheCount = 0;
        objectLiteralBranchCount = 0;
#endif
#if DBG_DUMP
        byteCodeDataSize = 0;
        byteCodeAuxiliaryDataSize = 0;
        byteCodeAuxiliaryContextDataSize = 0;
        memset(byteCodeHistogram, 0, sizeof(byteCodeHistogram));
#endif

        memset(propertyStrings, 0, sizeof(PropertyStringMap*)* 80);

#if DBG || defined(RUNTIME_DATA_COLLECTION)
        this->allocId = threadContext->GetUnreleasedScriptContextCount();
#endif
#if DBG
        this->hadProfiled = false;
#endif
#if DBG_DUMP
        forinCache = 0;
        forinNoCache = 0;
#endif

        callCount = 0;

        threadContext->GetHiResTimer()->Reset();

#ifdef PROFILE_EXEC
        profiler = nullptr;
        isProfilerCreated = false;
        disableProfiler = false;
        ensureParentInfo = false;
#endif

#ifdef PROFILE_MEM
        profileMemoryDump = true;
#endif

        m_pProfileCallback = nullptr;
        m_pProfileCallback2 = nullptr;
        m_inProfileCallback = FALSE;
        CleanupDocumentContext = nullptr;

        // Do this after all operations that may cause potential exceptions
        threadContext->RegisterScriptContext(this);
        numberAllocator.Initialize(this->GetRecycler());

#if DEBUG
        m_iProfileSession = -1;
#endif
#ifdef LEAK_REPORT
        this->urlRecord = nullptr;
        this->isRootTrackerScriptContext = false;
#endif

        PERF_COUNTER_INC(Basic, ScriptContext);
        PERF_COUNTER_INC(Basic, ScriptContextActive);

        END_NO_EXCEPTION;
    }

    void ScriptContext::InitializeAllocations()
    {
        //Language service uses a predetermined ES6 mode, and silently falls back to ES5 in case Windows.Globalization.dll is missing.
        if (BinaryFeatureControl::LanguageService())
        {
            this->charClassifier = Anew(GeneralAllocator(), CharClassifier, Js::CharClassifierModes::ES6, true);
        }
        else
        {
            this->charClassifier = Anew(GeneralAllocator(), CharClassifier, this);
        }

        this->valueOfInlineCache = AllocatorNewZ(InlineCacheAllocator, GetInlineCacheAllocator(), InlineCache);
        this->toStringInlineCache = AllocatorNewZ(InlineCacheAllocator, GetInlineCacheAllocator(), InlineCache);

#ifdef REJIT_STATS
        if (PHASE_STATS1(Js::ReJITPhase))
        {
            rejitReasonCounts = AnewArrayZ(GeneralAllocator(), uint, NumRejitReasons);
            bailoutReasonCounts = Anew(GeneralAllocator(), BailoutStatsMap, GeneralAllocator());
        }
#endif

#ifdef TELEMETRY
        this->telemetry = Anew(this->TelemetryAllocator(), ScriptContextTelemetry, *this);
#endif

#ifdef MUTATORS
        if (Js::Configuration::Global.flags.IsEnabled(Js::MutatorsFlag))
        {
            sourceMutator = Anew(MiscAllocator(), SourceMutator, this);
        }
#endif

#ifdef ARRLOG
        if (Js::Configuration::Global.flags.ArrayLog)
        {
            logTable = Anew(MiscAllocator(), UIntHashTable<ArrLogRec*>, MiscAllocator());
        }
#endif
#ifdef PROFILE_STRINGS
        if (Js::Configuration::Global.flags.ProfileStrings)
        {
            stringProfiler = Anew(MiscAllocator(), StringProfiler, threadContext->GetPageAllocator());
        }
#endif
        intConstPropsOnGlobalObject = Anew(GeneralAllocator(), PropIdSetForConstProp, GeneralAllocator());
        intConstPropsOnGlobalUserObject = Anew(GeneralAllocator(), PropIdSetForConstProp, GeneralAllocator());
    }
#ifdef BODLOG
    void PrintBod(int key, FunctionBody* bod, FILE* fp, void* ignore2) {
        fprintf(fp, "function %ls calls %d native 0x%x\n", bod->GetDisplayName(), bod->GetCallCount(), bod->NativeEntryPoint);
        if (bod->GetCallCount()>0) {

            const wchar_t* osrc = bod->GetSource();
            int len = bod->Length();
            wchar_t* src = (wchar_t*)calloc(2, len + 1);
            js_memcpy_s(src, (len + 1) * sizeof(wchar_t), osrc, len * 2);
            src[len] = L'\0';
            fprintf(fp, "source:\n");
            fprintf(fp, "%ls\n", src);
            free(src);
        }
    }


    void PrintBod2(FunctionBody* bod, FILE* fp, int total, int runningTotal, int remCount) {
        if (total>0) {
            fprintf(fp, "function %ls calls %d/%d(%4.2f) total calls so far %d/%d(%4.2f) remaining fn count %d native 0x%x\n",
                bod->GetDisplayName(), bod->GetCallCount(), total, (double)bod->GetCallCount() / total,
                runningTotal, total, (double)runningTotal / total, remCount, bod->NativeEntryPoint);
        }
        else {
            fprintf(fp, "function %ls calls %d/%d native 0x%x\n", bod->GetDisplayName(), bod->GetCallCount(), total,
                bod->NativeEntryPoint);
        }
        if (bod->GetCallCount()>0) {
            const wchar_t* osrc = bod->GetSource();
            int len = bod->Length();
            wchar_t* src = (wchar_t*)calloc(2, len + 1);
            js_memcpy_s(src, (len + 1) * sizeof(wchar_t), osrc, len * 2);
            src[len] = L'\0';
            fprintf(fp, "source:\n");
            fprintf(fp, "%ls\n", src);
            free(src);
        }
    }

    int __cdecl compareFnBod(void* context, const void* a, const void* b) {
        FunctionBody* abod = *(FunctionBody**)a;
        FunctionBody* bbod = *(FunctionBody**)b;
        return (abod->GetCallCount() - bbod->GetCallCount());
    }

    FILE* _bodlogfp = NULL;
#endif

    void ScriptContext::EnsureClearDebugDocument()
    {
        if (this->sourceList)
        {
            this->sourceList->Map([=](uint i, RecyclerWeakReference<Js::Utf8SourceInfo>* sourceInfoWeakRef) {
                Js::Utf8SourceInfo* sourceInfo = sourceInfoWeakRef->Get();
                if (sourceInfo)
                {
                    sourceInfo->ClearDebugDocument();
                }
            });
        }
    }

    void ScriptContext::ShutdownClearSourceLists()
    {
        if (this->sourceList)
        {
            // In the unclean shutdown case, we might not have destroyed the script context when
            // this is called- in which case, skip doing this work and simply release the source list
            // so that it doesn't show up as a leak. Since we're doing unclean shutdown, it's ok to
            // skip cleanup here for expediency.
            if (this->isClosed)
            {
                this->MapFunction([this](Js::FunctionBody* functionBody) {
                    Assert(functionBody->GetScriptContext() == this);
                    functionBody->CleanupSourceInfo(true);
                });
            }

            EnsureClearDebugDocument();

            // Don't need the source list any more so ok to release
            this->sourceList.Unroot(this->GetRecycler());
        }

        if (this->calleeUtf8SourceInfoList)
        {
            this->calleeUtf8SourceInfoList.Unroot(this->GetRecycler());
        }
    }

    ScriptContext::~ScriptContext()
    {
        // Take etw rundown lock on this thread context. We are going to change/destroy this scriptContext.
        AutoCriticalSection autocs(GetThreadContext()->GetEtwRundownCriticalSection());

        // TODO: Can we move this on Close()?
        ClearHostScriptContext();

        threadContext->UnregisterScriptContext(this);

        // Only call RemoveFromPendingClose if we are in a pending close state.
        if (isClosed && !isScriptContextActuallyClosed)
        {
            threadContext->RemoveFromPendingClose(this);
        }

        this->isClosed = true;
        bool closed = Close(true);

        diagProbesContainer.RemoveMutationBreakpointListIfNeeded();

        // JIT may access number allocator. Need to close the script context first,
        // which will close the native code generator and abort any current job on this generator.
        numberAllocator.Uninitialize();

        ShutdownClearSourceLists();

        if (regexStacks)
        {
            Adelete(RegexAllocator(), regexStacks);
            regexStacks = null;
        }

        if (javascriptLibrary != null)
        {
            javascriptLibrary->scriptContext = null;
            javascriptLibrary = null;
            if (closed)
            {
                // if we just closed, we haven't unpin the object yet.
                // We need to null out the script context in the global object first
                // before we unpin the global object so that script context dtor doesn't get called twice

#if ENABLE_NATIVE_CODEGEN
                Assert(this->IsClosedNativeCodeGenerator());
#endif
                this->recycler->RootRelease(globalObject);
                if (BinaryFeatureControl::LanguageService())
                {
                    globalObject = null;
                }
            }

        }

        if (this->backgroundParser != nullptr)
        {
            BackgroundParser::Delete(this->backgroundParser);
            this->backgroundParser = nullptr;
        }

#if ENABLE_NATIVE_CODEGEN
        if (this->nativeCodeGen != null)
        {
            DeleteNativeCodeGenerator(this->nativeCodeGen);
            nativeCodeGen = NULL;
        }
#endif

        if (nativeModules != nullptr)
        {
            EachNativeModule(
                [=](Js::NativeModule *nativeModule) -> void
            {
                if (nativeModule->loadedInMemory)
                {
                    HeapDeleteArray(nativeModule->exportCount, nativeModule->exports);
#if defined(_M_X64) || defined(_M_ARM32_OR_ARM64)
                    HeapDeleteArray(nativeModule->pdataCount, nativeModule->pdataTable);
                    if (nativeModule->functionTableHandle)
                    {
                        NtdllLibrary::Instance->DeleteGrowableFunctionTable(nativeModule->functionTableHandle);
                        nativeModule->functionTableHandle = nullptr;
                    }
#endif
                    VirtualFree(nativeModule->textSection, nativeModule->textSectionSize, MEM_DECOMMIT | MEM_RELEASE);
                }
                HeapDelete(nativeModule);
            });

            nativeModules->Clear();
            HeapDelete(nativeModules);
            nativeModules = nullptr;
        }

        if (this->interpreterThunkEmitter != null)
        {
            HeapDelete(interpreterThunkEmitter);
            this->interpreterThunkEmitter = NULL;
        }

#ifdef ASMJS_PLAT
        if (this->asmJsInterpreterThunkEmitter != nullptr)
        {
            HeapDelete(asmJsInterpreterThunkEmitter);
            this->asmJsInterpreterThunkEmitter = nullptr;
        }

        if (this->asmJsCodeGenerator != null)
        {
            HeapDelete(asmJsCodeGenerator);
            this->asmJsCodeGenerator = NULL;
        }
#endif

        if (diagnosticArena)
        {
            HeapDelete(diagnosticArena);
        }

        if (this->hasRegisteredInlineCache)
        {
            // TODO (PersistentInlineCaches): It really isn't necessary to clear inline caches in all script contexts.
            // Since this script context is being destroyed, the inline cache arena will also go away and release its
            // memory back to the page allocator.  Thus, we cannot leave this script context's inline caches on the
            // thread context's invalidation lists.  However, it should suffice to remove this script context's caches
            // without touching other script contexts' caches.  We could call some form of RemoveInlineCachesFromInvalidationLists()
            // on the inline cache allocator, which would walk all inline caches and zap values pointed to by strongRef.

            // clear out all inline caches to remove our proto inline caches from the thread context
            threadContext->ClearInlineCaches();
            Assert(!this->hasRegisteredInlineCache);
            Assert(this->entryInScriptContextWithInlineCachesRegistry == null);
        }
        else if (this->entryInScriptContextWithInlineCachesRegistry != null)
        {
            // UnregisterInlineCacheScriptContext may throw, set up the correct state first
            ScriptContext ** entry = this->entryInScriptContextWithInlineCachesRegistry;
            this->entryInScriptContextWithInlineCachesRegistry = null;
            threadContext->UnregisterInlineCacheScriptContext(entry);
        }

        if (this->hasRegisteredIsInstInlineCache)
        {
            // clear out all inline caches to remove our proto inline caches from the thread context
            threadContext->ClearIsInstInlineCaches();
            Assert(!this->hasRegisteredIsInstInlineCache);
            Assert(this->entryInScriptContextWithIsInstInlineCachesRegistry == null);
        }
        else if (this->entryInScriptContextWithInlineCachesRegistry != null)
        {
            // UnregisterInlineCacheScriptContext may throw, set up the correct state first
            ScriptContext ** entry = this->entryInScriptContextWithInlineCachesRegistry;
            this->entryInScriptContextWithInlineCachesRegistry = null;
            threadContext->UnregisterIsInstInlineCacheScriptContext(entry);
        }

        // In case there is something added to the list between close and dtor, just reset the list again
        this->weakReferenceDictionaryList.Reset();

        PERF_COUNTER_DEC(Basic, ScriptContext);
    }

    void ScriptContext::SetUrl(BSTR bstrUrl)
    {
        // Assumption: this method is never called multiple times
        Assert(this->url != null && wcslen(this->url) == 0);

        charcount_t length = SysStringLen(bstrUrl) + 1; // Add 1 for the NULL.

        wchar_t* urlCopy = AnewArray(this->GeneralAllocator(), wchar_t, length);
        js_memcpy_s(urlCopy, (length - 1) * sizeof(wchar_t), bstrUrl, (length - 1) * sizeof(wchar_t));
        urlCopy[length - 1] = L'\0';

        this->url = urlCopy;
#ifdef LEAK_REPORT
        if (Js::Configuration::Global.flags.IsEnabled(Js::LeakReportFlag))
        {
            this->urlRecord = LeakReport::LogUrl(urlCopy, this->globalObject);
        }
#endif
    }

    uint ScriptContext::GetNextSourceContextId()
    {
        Assert(this->cache);

        Assert(this->cache->sourceContextInfoMap ||
            this->cache->dynamicSourceContextInfoMap);

        uint nextSourceContextId = 0;

        if (this->cache->sourceContextInfoMap)
        {
            nextSourceContextId = this->cache->sourceContextInfoMap->Count();
        }

        if (this->cache->dynamicSourceContextInfoMap)
        {
            nextSourceContextId += this->cache->dynamicSourceContextInfoMap->Count();
        }

        return nextSourceContextId + 1;
    }

    // Do most of the Close() work except the final release which could delete the scriptContext.
    void ScriptContext::InternalClose()
    {
        this->PrintStats();

        isScriptContextActuallyClosed = true;

        PERF_COUNTER_DEC(Basic, ScriptContextActive);

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(L"MemoryTrace: ScriptContext Close\n");
            Output::Flush();
        }
#endif
#if F_JSETW
        EventWriteJSCRIPT_HOST_SCRIPT_CONTEXT_CLOSE(this);
#endif

        HRESULT hr = S_OK;
        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
        {
            DynamicProfileInfo::Save(this);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);

#if DBG_DUMP || defined(DYNAMIC_PROFILE_STORAGE) || defined(RUNTIME_DATA_COLLECTION)
        this->ClearDynamicProfileList();
#endif

#if ENABLE_NATIVE_CODEGEN
        if (nativeCodeGen != nullptr)
        {
            Assert(!isInitialized || this->globalObject != nullptr);
            CloseNativeCodeGenerator(this->nativeCodeGen);
        }
#endif

        if (this->sourceList)
        {
            bool hasFunctions = false;
            this->sourceList->MapUntil([&hasFunctions](int, RecyclerWeakReference<Utf8SourceInfo>* sourceInfoWeakRef) -> bool
            {
                Utf8SourceInfo* sourceInfo = sourceInfoWeakRef->Get();
                if (sourceInfo)
                {
                    hasFunctions = sourceInfo->HasFunctions();
                }

                return hasFunctions;
            });

            if (hasFunctions)
            {
                // We still need to walk through all the function bodies and call cleanup
                // because otherwise ETW events might not get fired if a GC doesn't happen
                // and the thread context isn't shut down cleanly (process detach case)
                this->MapFunction([this](Js::FunctionBody* functionBody) {
                    Assert(functionBody->GetScriptContext() == this);
                    functionBody->Cleanup(/* isScriptContextClosing */ true);
                });
            }
        }

        EtwTrace::LogSourceUnloadEvents(this);

        this->GetThreadContext()->SubSourceSize(this->GetSourceSize());

        if (this->interpreterThunkEmitter != nullptr)
        {
            this->interpreterThunkEmitter->Close();
        }

#ifdef ASMJS_PLAT
        if (this->asmJsInterpreterThunkEmitter != nullptr)
        {
            this->asmJsInterpreterThunkEmitter->Close();
        }
#endif

        // Stop profiling if present
        DeRegisterProfileProbe(S_OK, nullptr);

        // Need to print this out beform the native code gen is deleted
        // which will delete the codegenProfiler

#ifdef PROFILE_EXEC
        if (Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag))
        {
            if (isProfilerCreated)
            {
                this->ProfilePrint();
            }

            if (profiler != nullptr)
            {
                profiler->Release();
                profiler = nullptr;
            }
        }
#endif


        // Release this only after native code gen is shut down, as there may be
        // profile info allocated from the SourceDynamicProfileManager arena.
        // The first condition might not be true if the dynamic functions have already been freed by the time
        // ScriptContext closes
        if (referencesSharedDynamicSourceContextInfo)
        {
            // For the host provided dynamic code, we may not have added any dynamic context to the dynamicSourceContextInfoMap
            Assert(this->GetDynamicSourceContextInfoMap() != nullptr);
            this->GetThreadContext()->ReleaseSourceDynamicProfileManagers(this->GetUrl());
        }

        RECYCLER_PERF_COUNTER_SUB(BindReference, bindReferenceCount);

        if (this->interpreterArena)
        {
            ReleaseInterpreterArena();
            interpreterArena = nullptr;
        }

        if (this->guestArena)
        {
            ReleaseGuestArena();
            guestArena = nullptr;
            cache = nullptr;
            bindRefChunkCurrent = nullptr;
            bindRefChunkEnd = nullptr;
        }

        builtInLibraryFunctions = nullptr;

        pActiveScriptDirect = nullptr;

        isWeakReferenceDictionaryListCleared = true;
        this->weakReferenceDictionaryList.Clear(this->GeneralAllocator());

        // This can be null if the script context initialization threw
        // and InternalClose gets called in the destructor code path
        if (javascriptLibrary != nullptr)
        {
            javascriptLibrary->Uninitialize();
        }

        if (registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext != nullptr)
        {
            // UnregisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext may throw, set up the correct state first
            ScriptContext ** registeredScriptContext = registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext;
            ClearPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesCaches();
            Assert(registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext == nullptr);
            threadContext->UnregisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext(registeredScriptContext);
        }

        threadContext->ReleaseScriptContextFromDiagnostic();
    }

    bool ScriptContext::Close(bool inDestructor)
    {
        if (isScriptContextActuallyClosed)
            return false;

        // Limit the lock scope. We require the same lock in ~ScriptContext(), which may be called next.
        {
            // Take etw rundown lock on this thread context. We are going to change this scriptContext.
            AutoCriticalSection autocs(GetThreadContext()->GetEtwRundownCriticalSection());
            InternalClose();
        }

        if (!inDestructor && globalObject != nullptr)
        {
            //A side effect of releasing globalObject that this script context could be deleted, so the release call here
            //must be the last thing in close.
#if ENABLE_NATIVE_CODEGEN
            Assert(this->IsClosedNativeCodeGenerator());
#endif
            GetRecycler()->RootRelease(globalObject);
            if (BinaryFeatureControl::LanguageService())
            {
                globalObject = nullptr;
            }
        }

        // A script context closing is a signal to the thread context that it
        // needs to do an idle GC independent of what the heuristics are
        this->threadContext->SetForceOneIdleCollection();

        return true;
    }

#ifdef PROFILE_TYPES
    void ScriptContext::ProfileTypes()
    {
        Output::Print(L"===============================================================================\n");
        Output::Print(L"Types Profile\n");
        Output::Print(L"-------------------------------------------------------------------------------\n");
        Output::Print(L"Dynamic Type Conversions:\n");
        Output::Print(L"    Null to Simple                 %8d\n", convertNullToSimpleCount);
        Output::Print(L"    Deferred to SimpleMap          %8d\n", convertDeferredToSimpleDictionaryCount);
        Output::Print(L"    Simple to Map                  %8d\n", convertSimpleToDictionaryCount);
        Output::Print(L"    Simple to SimpleMap            %8d\n", convertSimpleToSimpleDictionaryCount);
        Output::Print(L"    Path to SimpleMap (set)        %8d\n", convertPathToDictionaryCount1);
        Output::Print(L"    Path to SimpleMap (delete)     %8d\n", convertPathToDictionaryCount2);
        Output::Print(L"    Path to SimpleMap (attribute)  %8d\n", convertPathToDictionaryCount3);
        Output::Print(L"    Path to SimpleMap              %8d\n", convertPathToSimpleDictionaryCount);
        Output::Print(L"    SimplePath to Path             %8d\n", convertSimplePathToPathCount);
        Output::Print(L"    Shared SimpleMap to non-shared %8d\n", convertSimpleSharedDictionaryToNonSharedCount);
        Output::Print(L"    Deferred to Map                %8d\n", convertDeferredToDictionaryCount);
        Output::Print(L"    Path to Map (accessor)         %8d\n", convertPathToDictionaryCount4);
        Output::Print(L"    SimpleMap to Map               %8d\n", convertSimpleDictionaryToDictionaryCount);
        Output::Print(L"    Path Cache Hits                %8d\n", cacheCount);
        Output::Print(L"    Path Branches                  %8d\n", branchCount);
        Output::Print(L"    Path Promotions                %8d\n", promoteCount);
        Output::Print(L"    Path Length (max)              %8d\n", maxPathLength);
        Output::Print(L"    SimplePathTypeHandlers         %8d\n", simplePathTypeHandlerCount);
        Output::Print(L"    PathTypeHandlers               %8d\n", pathTypeHandlerCount);
        Output::Print(L"\n");
        Output::Print(L"Type Statistics:                   %8s   %8s\n", L"Types", L"Instances");
        Output::Print(L"    Undefined                      %8d   %8d\n", typeCount[TypeIds_Undefined], instanceCount[TypeIds_Undefined]);
        Output::Print(L"    Null                           %8d   %8d\n", typeCount[TypeIds_Null], instanceCount[TypeIds_Null]);
        Output::Print(L"    Boolean                        %8d   %8d\n", typeCount[TypeIds_Boolean], instanceCount[TypeIds_Boolean]);
        Output::Print(L"    Integer                        %8d   %8d\n", typeCount[TypeIds_Integer], instanceCount[TypeIds_Integer]);
        Output::Print(L"    Number                         %8d   %8d\n", typeCount[TypeIds_Number], instanceCount[TypeIds_Number]);
        Output::Print(L"    String                         %8d   %8d\n", typeCount[TypeIds_String], instanceCount[TypeIds_String]);
        Output::Print(L"    Object                         %8d   %8d\n", typeCount[TypeIds_Object], instanceCount[TypeIds_Object]);
        Output::Print(L"    Function                       %8d   %8d\n", typeCount[TypeIds_Function], instanceCount[TypeIds_Function]);
        Output::Print(L"    Array                          %8d   %8d\n", typeCount[TypeIds_Array], instanceCount[TypeIds_Array]);
        Output::Print(L"    Date                           %8d   %8d\n", typeCount[TypeIds_Date], instanceCount[TypeIds_Date] + instanceCount[TypeIds_WinRTDate]);
        Output::Print(L"    Symbol                         %8d   %8d\n", typeCount[TypeIds_Symbol], instanceCount[TypeIds_Symbol]);
        Output::Print(L"    RegEx                          %8d   %8d\n", typeCount[TypeIds_RegEx], instanceCount[TypeIds_RegEx]);
        Output::Print(L"    Error                          %8d   %8d\n", typeCount[TypeIds_Error], instanceCount[TypeIds_Error]);
        Output::Print(L"    Proxy                          %8d   %8d\n", typeCount[TypeIds_Proxy], instanceCount[TypeIds_Proxy]);
        Output::Print(L"    BooleanObject                  %8d   %8d\n", typeCount[TypeIds_BooleanObject], instanceCount[TypeIds_BooleanObject]);
        Output::Print(L"    NumberObject                   %8d   %8d\n", typeCount[TypeIds_NumberObject], instanceCount[TypeIds_NumberObject]);
        Output::Print(L"    StringObject                   %8d   %8d\n", typeCount[TypeIds_StringObject], instanceCount[TypeIds_StringObject]);
        Output::Print(L"    SymbolObject                   %8d   %8d\n", typeCount[TypeIds_SymbolObject], instanceCount[TypeIds_SymbolObject]);
        Output::Print(L"    GlobalObject                   %8d   %8d\n", typeCount[TypeIds_GlobalObject], instanceCount[TypeIds_GlobalObject]);
        Output::Print(L"    Enumerator                     %8d   %8d\n", typeCount[TypeIds_Enumerator], instanceCount[TypeIds_Enumerator]);
        Output::Print(L"    ExtensionEnumerator            %8d   %8d\n", typeCount[TypeIds_ExtensionEnumerator], instanceCount[TypeIds_ExtensionEnumerator]);
        Output::Print(L"    Int8Array                      %8d   %8d\n", typeCount[TypeIds_Int8Array], instanceCount[TypeIds_Int8Array]);
        Output::Print(L"    Uint8Array                     %8d   %8d\n", typeCount[TypeIds_Uint8Array], instanceCount[TypeIds_Uint8Array]);
        Output::Print(L"    Uint8ClampedArray              %8d   %8d\n", typeCount[TypeIds_Uint8ClampedArray], instanceCount[TypeIds_Uint8ClampedArray]);
        Output::Print(L"    Int16Array                     %8d   %8d\n", typeCount[TypeIds_Int16Array], instanceCount[TypeIds_Int16Array]);
        Output::Print(L"    Int16Array                     %8d   %8d\n", typeCount[TypeIds_Uint16Array], instanceCount[TypeIds_Uint16Array]);
        Output::Print(L"    Int32Array                     %8d   %8d\n", typeCount[TypeIds_Int32Array], instanceCount[TypeIds_Int32Array]);
        Output::Print(L"    Uint32Array                    %8d   %8d\n", typeCount[TypeIds_Uint32Array], instanceCount[TypeIds_Uint32Array]);
        Output::Print(L"    Float32Array                   %8d   %8d\n", typeCount[TypeIds_Float32Array], instanceCount[TypeIds_Float32Array]);
        Output::Print(L"    Float64Array                   %8d   %8d\n", typeCount[TypeIds_Float64Array], instanceCount[TypeIds_Float64Array]);
        Output::Print(L"    DataView                       %8d   %8d\n", typeCount[TypeIds_DataView], instanceCount[TypeIds_DataView]);
        Output::Print(L"    ModuleRoot                     %8d   %8d\n", typeCount[TypeIds_ModuleRoot], instanceCount[TypeIds_ModuleRoot]);
        Output::Print(L"    HostObject                     %8d   %8d\n", typeCount[TypeIds_HostObject], instanceCount[TypeIds_HostObject]);
        Output::Print(L"    VariantDate                    %8d   %8d\n", typeCount[TypeIds_VariantDate], instanceCount[TypeIds_VariantDate]);
        Output::Print(L"    HostDispatch                   %8d   %8d\n", typeCount[TypeIds_HostDispatch], instanceCount[TypeIds_HostDispatch]);
        Output::Print(L"    Arguments                      %8d   %8d\n", typeCount[TypeIds_Arguments], instanceCount[TypeIds_Arguments]);
        Output::Print(L"    ActivationObject               %8d   %8d\n", typeCount[TypeIds_ActivationObject], instanceCount[TypeIds_ActivationObject]);
        Output::Print(L"    Map                            %8d   %8d\n", typeCount[TypeIds_Map], instanceCount[TypeIds_Map]);
        Output::Print(L"    Set                            %8d   %8d\n", typeCount[TypeIds_Set], instanceCount[TypeIds_Set]);
        Output::Print(L"    WeakMap                        %8d   %8d\n", typeCount[TypeIds_WeakMap], instanceCount[TypeIds_WeakMap]);
        Output::Print(L"    WeakSet                        %8d   %8d\n", typeCount[TypeIds_WeakSet], instanceCount[TypeIds_WeakSet]);
        Output::Print(L"    ArrayIterator                  %8d   %8d\n", typeCount[TypeIds_ArrayIterator], instanceCount[TypeIds_ArrayIterator]);
        Output::Print(L"    MapIterator                    %8d   %8d\n", typeCount[TypeIds_MapIterator], instanceCount[TypeIds_MapIterator]);
        Output::Print(L"    SetIterator                    %8d   %8d\n", typeCount[TypeIds_SetIterator], instanceCount[TypeIds_SetIterator]);
        Output::Print(L"    StringIterator                 %8d   %8d\n", typeCount[TypeIds_StringIterator], instanceCount[TypeIds_StringIterator]);
        Output::Print(L"    Generator                      %8d   %8d\n", typeCount[TypeIds_Generator], instanceCount[TypeIds_Generator]);
#if !DBG
        Output::Print(L"    ** Instance statistics only available on debug builds...\n");
#endif
        Output::Flush();
    }
#endif


#ifdef PROFILE_OBJECT_LITERALS
    void ScriptContext::ProfileObjectLiteral()
    {
        Output::Print(L"===============================================================================\n");
        Output::Print(L"    Object Lit Instances created.. %d\n", objectLiteralInstanceCount);
        Output::Print(L"    Object Lit Path Types......... %d\n", objectLiteralPathCount);
        Output::Print(L"    Object Lit Simple Map......... %d\n", objectLiteralSimpleDictionaryCount);
        Output::Print(L"    Object Lit Max # of properties %d\n", objectLiteralMaxLength);
        Output::Print(L"    Object Lit Promote count...... %d\n", objectLiteralPromoteCount);
        Output::Print(L"    Object Lit Cache Hits......... %d\n", objectLiteralCacheCount);
        Output::Print(L"    Object Lit Branch count....... %d\n", objectLiteralBranchCount);

        for (int i = 0; i < TypePath::MaxPathTypeHandlerLength; i++)
        {
            if (objectLiteralCount[i] != 0)
            {
                Output::Print(L"    Object Lit properties [ %2d] .. %d\n", i, objectLiteralCount[i]);
            }
        }

        Output::Flush();
    }
#endif

    //
    // Regex helpers
    //

#if ENABLE_REGEX_CONFIG_OPTIONS
    UnifiedRegex::RegexStatsDatabase* ScriptContext::GetRegexStatsDatabase()
    {
        if (regexStatsDatabase == 0)
        {
            ArenaAllocator* allocator = MiscAllocator();
            regexStatsDatabase = Anew(allocator, UnifiedRegex::RegexStatsDatabase, allocator);
        }
        return regexStatsDatabase;
    }

    UnifiedRegex::DebugWriter* ScriptContext::GetRegexDebugWriter()
    {
        if (regexDebugWriter == 0)
        {
            ArenaAllocator* allocator = MiscAllocator();
            regexDebugWriter = Anew(allocator, UnifiedRegex::DebugWriter);
        }
        return regexDebugWriter;
    }
#endif

    RegexPatternMruMap* ScriptContext::GetDynamicRegexMap() const
    {
        Assert(!isScriptContextActuallyClosed);
        Assert(guestArena);
        Assert(cache);
        Assert(cache->dynamicRegexMap);

        return cache->dynamicRegexMap;
    }

    void ScriptContext::SetTrigramAlphabet
        (__in_xcount(regex::TrigramAlphabet::AlphaCount) char* alpha
        , __in_xcount(regex::TrigramAlphabet::AsciiTableSize) char* alphaBits)
    {
        int i;
        if (trigramAlphabet == null) {
            ArenaAllocator* alloc = RegexAllocator();
            trigramAlphabet = AnewStruct(alloc, UnifiedRegex::TrigramAlphabet);
        }
        for (i = 0; i < UnifiedRegex::TrigramAlphabet::AsciiTableSize; i++) {
            trigramAlphabet->alphaBits[i] = UnifiedRegex::TrigramAlphabet::BitsNotInAlpha;
        }
        for (i = 0; i < UnifiedRegex::TrigramAlphabet::AlphaCount; i++) {
            trigramAlphabet->alpha[i] = alpha[i];
            trigramAlphabet->alphaBits[alpha[i]] = alphaBits[alpha[i]];
        }
        trigramAlphabet->InitTrigramMap();
    }

    UnifiedRegex::RegexStacks *ScriptContext::RegexStacks()
    {
        UnifiedRegex::RegexStacks * stacks = regexStacks;
        if (stacks)
        {
            return stacks;
        }
        return AllocRegexStacks();
    }

    UnifiedRegex::RegexStacks * ScriptContext::AllocRegexStacks()
    {
        Assert(this->regexStacks == null);
        UnifiedRegex::RegexStacks * stacks = Anew(RegexAllocator(), UnifiedRegex::RegexStacks, threadContext->GetPageAllocator());
        this->regexStacks = stacks;
        return stacks;
    }

    UnifiedRegex::RegexStacks *ScriptContext::SaveRegexStacks()
    {
        Assert(regexStacks);

        const auto saved = regexStacks;
        regexStacks = null;
        return saved;
    }

    void ScriptContext::RestoreRegexStacks(UnifiedRegex::RegexStacks *const stacks)
    {
        Assert(stacks);
        Assert(stacks != regexStacks);

        if (regexStacks)
        {
            Adelete(RegexAllocator(), regexStacks);
        }
        regexStacks = stacks;
    }

    Js::TempArenaAllocatorObject* ScriptContext::GetTemporaryAllocator(LPCWSTR name)
    {
        return this->threadContext->GetTemporaryAllocator(name);
    }

    void ScriptContext::ReleaseTemporaryAllocator(Js::TempArenaAllocatorObject* tempAllocator)
    {
        AssertMsg(tempAllocator != null, "tempAllocator should not be null");

        this->threadContext->ReleaseTemporaryAllocator(tempAllocator);
    }

    Js::TempGuestArenaAllocatorObject* ScriptContext::GetTemporaryGuestAllocator(LPCWSTR name)
    {
        return this->threadContext->GetTemporaryGuestAllocator(name);
    }

    void ScriptContext::ReleaseTemporaryGuestAllocator(Js::TempGuestArenaAllocatorObject* tempGuestAllocator)
    {
        AssertMsg(tempGuestAllocator != null, "tempAllocator should not be null");

        this->threadContext->ReleaseTemporaryGuestAllocator(tempGuestAllocator);
    }

    void ScriptContext::InitializePreGlobal()
    {
        this->guestArena = this->GetRecycler()->CreateGuestArena(L"Guest", Throw::OutOfMemory);
#if DBG_DUMP || defined(DYNAMIC_PROFILE_STORAGE) || defined(RUNTIME_DATA_COLLECTION)
        if (DynamicProfileInfo::NeedProfileInfoList())
        {
            this->profileInfoList.Root(RecyclerNew(this->GetRecycler(), SListBase<DynamicProfileInfo *>), recycler);
        }
#endif

        {
            AutoCriticalSection critSec(this->threadContext->GetEtwRundownCriticalSection());
            this->cache = AnewStructZ(guestArena, Cache);
        }

        this->cache->rootPath = TypePath::New(recycler);
        this->cache->dynamicRegexMap =
            RegexPatternMruMap::New(
            recycler,
            REGEX_CONFIG_FLAG(DynamicRegexMruListSize) <= 0 ? 16 : REGEX_CONFIG_FLAG(DynamicRegexMruListSize));

        SourceContextInfo* sourceContextInfo = RecyclerNewStructZ(this->GetRecycler(), SourceContextInfo);
        sourceContextInfo->dwHostSourceContext = Js::Constants::NoHostSourceContext;
        sourceContextInfo->isHostDynamicDocument = false;
        sourceContextInfo->sourceContextId = Js::Constants::NoSourceContext;
        this->cache->noContextSourceContextInfo = sourceContextInfo;

        SRCINFO* srcInfo = RecyclerNewStructZ(this->GetRecycler(), SRCINFO);
        srcInfo->sourceContextInfo = this->cache->noContextSourceContextInfo;
        srcInfo->moduleID = kmodGlobal;
        this->cache->noContextGlobalSourceInfo = srcInfo;

        this->backgroundParser = BackgroundParser::New(this);

#if ENABLE_NATIVE_CODEGEN
        if (BinaryFeatureControl::NativeCodeGen())
        {
            // Create the native code gen before the profiler
            this->nativeCodeGen = NewNativeCodeGenerator(this);
        }
#endif

#ifdef PROFILE_EXEC
        this->CreateProfiler();
#endif

#ifdef FIELD_ACCESS_STATS
        this->fieldAccessStatsByFunctionNumber = RecyclerNew(this->recycler, FieldAccessStatsByFunctionNumberMap, recycler);
        BindReference(this->fieldAccessStatsByFunctionNumber);
#endif

        this->operationStack = Anew(GeneralAllocator(), JsUtil::Stack<Var>, GeneralAllocator());

        Tick::InitType();
    }

    void ScriptContext::Initialize()
    {
        SmartFPUControl defaultControl;

        InitializePreGlobal();

        InitializeGlobalObject();

        InitializePostGlobal(false);
    }

    void ScriptContext::InitializePostGlobal(bool initializingCopy)
    {
        if (!initializingCopy)
            // In CopyOnWriteCopy() this is performed early because it is required to create a copy-on-write function.
            this->diagProbesContainer.Initialize(this);

        AssertMsg(this->CurrentThunk == DefaultEntryThunk, "Creating non default thunk while initializing");
        AssertMsg(this->DeferredParsingThunk == DefaultDeferredParsingThunk, "Creating non default thunk while initializing");
        AssertMsg(this->DeferredDeserializationThunk == DefaultDeferredDeserializeThunk, "Creating non default thunk while initializing");

        if (!sourceList)
        {
            AutoCriticalSection critSec(threadContext->GetEtwRundownCriticalSection());
            sourceList.Root(RecyclerNew(this->GetRecycler(), SourceList, this->GetRecycler()), this->GetRecycler());
        }

        interpreterThunkEmitter = HeapNew(InterpreterThunkEmitter, this->GetThreadContext()->GetAllocationPolicyManager(),
            SourceCodeAllocator(), Js::InterpreterStackFrame::InterpreterThunk);

#ifdef ASMJS_PLAT
        asmJsInterpreterThunkEmitter = HeapNew(InterpreterThunkEmitter, this->GetThreadContext()->GetAllocationPolicyManager(),
            SourceCodeAllocator(), Js::InterpreterStackFrame::InterpreterAsmThunk);
#endif

        EtwTrace::LogScriptContextLoadEvent(this);
        EventWriteJSCRIPT_HOST_SCRIPT_CONTEXT_START(this);

#ifdef PROFILE_EXEC
        if (profiler != null)
        {
            this->threadContext->GetRecycler()->SetProfiler(profiler->GetProfiler(), profiler->GetBackgroundRecyclerProfiler());
        }
#endif

#if DBG
        // We aren't going to be passing in a number to check range of -dump:LibInit, that will be done by Intl/Promise
        // This is just to force init Intl code if dump:LibInit has been passed
        if (CONFIG_ISENABLED(DumpFlag) && Js::Configuration::Global.flags.Dump.IsEnabled(Js::JsLibInitPhase))
        {
#ifdef ENABLE_INTL_OBJECT
            this->javascriptLibrary->GetEngineInterfaceObject()->DumpIntlByteCode(this);
#endif
            this->javascriptLibrary->GetEngineInterfaceObject()->DumpPromiseByteCode(this);
        }

        isInitialized = TRUE;
#endif
    }


#ifdef ASMJS_PLAT
    AsmJsCodeGenerator* ScriptContext::InitAsmJsCodeGenerator()
    {
        if( !asmJsCodeGenerator )
        {
            asmJsCodeGenerator = HeapNew( AsmJsCodeGenerator, this );
        }
        return asmJsCodeGenerator;
    }
#endif
    void ScriptContext::MarkForClose()
    {
        SaveStartupProfileAndRelease(true);
        this->isClosed = true;

#ifdef LEAK_REPORT
        if (this->isRootTrackerScriptContext)
        {
            this->GetThreadContext()->ClearRootTrackerScriptContext(this);
        }
#endif

        if (!threadContext->IsInScript())
        {
            Close(FALSE);
        }
        else
        {
            threadContext->AddToPendingScriptContextCloseList(this);
        }
    }

    void ScriptContext::InitializeGlobalObject()
    {
        GlobalObject * localGlobalObject = GlobalObject::New(this);
        GetRecycler()->RootAddRef(localGlobalObject);

        // Assigned the global Object after we have successfully AddRef (in case of OOM)
        globalObject = localGlobalObject;
        globalObject->Initialize(this);
    }

    ArenaAllocator* ScriptContext::AllocatorForDiagnostics()
    {
        if (!diagnosticArena)
        {
            diagnosticArena = HeapNew(ArenaAllocator, L"Diagnostic", GetThreadContext()->GetDiagnosticPageAllocator(), Throw::OutOfMemory);
        }

        return diagnosticArena;
    }

    void ScriptContext::PushObject(Var object)
    {
        operationStack->Push(object);
    }

    Var ScriptContext::PopObject()
    {
        return operationStack->Pop();
    }

    BOOL ScriptContext::CheckObject(Var object)
    {
        return operationStack->Contains(object);
    }

    void ScriptContext::SetHostScriptContext(HostScriptContext *  hostScriptContext)
    {
        Assert(this->hostScriptContext == null);
        this->hostScriptContext = hostScriptContext;
#ifdef PROFILE_EXEC
        this->ensureParentInfo = true;
#endif
    }

    //
    // Enables jscript9diag to get the HaltCallBack pointer that is implemented by
    // the ScriptEngine.
    //
    void ScriptContext::SetScriptEngineHaltCallback(HaltCallback* scriptEngine)
    {
        Assert(this->scriptEngineHaltCallback == NULL);
        Assert(scriptEngine != NULL);
        this->scriptEngineHaltCallback = scriptEngine;
    }

    void ScriptContext::ClearHostScriptContext()
    {
        if (this->hostScriptContext != null)
        {
            this->hostScriptContext->Delete();
#ifdef PROFILE_EXEC
            this->ensureParentInfo = false;
#endif
        }
    }

    IActiveScriptProfilerHeapEnum* ScriptContext::GetHeapEnum()
    {
        Assert(this->GetThreadContext());
        return this->GetThreadContext()->GetHeapEnum();
    }

    void ScriptContext::SetHeapEnum(IActiveScriptProfilerHeapEnum* newHeapEnum)
    {
        Assert(this->GetThreadContext());
        this->GetThreadContext()->SetHeapEnum(newHeapEnum);
    }

    void ScriptContext::ClearHeapEnum()
    {
        Assert(this->GetThreadContext());
        this->GetThreadContext()->ClearHeapEnum();
    }

    BOOL ScriptContext::VerifyAlive(BOOL isJSFunction, ScriptContext* requestScriptContext)
    {
        if (isClosed)
        {
            if (!requestScriptContext)
            {
                requestScriptContext = this;
            }

            if (!GetThreadContext()->RecordImplicitException())
            {
                return FALSE;
            }
            if (isJSFunction)
            {
                Js::JavascriptError::MapAndThrowError(requestScriptContext, JSERR_CantExecute);
            }
            else
            {
                Js::JavascriptError::MapAndThrowError(requestScriptContext, E_ACCESSDENIED);
            }
        }
        return TRUE;
    }

    void ScriptContext::VerifyAliveWithHostContext(BOOL isJSFunction, HostScriptContext* requestHostScriptContext)
    {
        if (requestHostScriptContext)
        {
            VerifyAlive(isJSFunction, requestHostScriptContext->GetScriptContext());
        }
        else
        {
            Assert(!GetHostScriptContext()->HasCaller());
            VerifyAlive(isJSFunction, NULL);
        }
    }


    PropertyRecord const * ScriptContext::GetPropertyName(PropertyId propertyId)
    {
        return threadContext->GetPropertyName(propertyId);
    }

    PropertyRecord const * ScriptContext::GetPropertyNameLocked(PropertyId propertyId)
    {
        return threadContext->GetPropertyNameLocked(propertyId);
    }

    void ScriptContext::InitPropertyStringMap(int i)
    {
        propertyStrings[i] = AnewStruct(GeneralAllocator(), PropertyStringMap);
        memset(propertyStrings[i]->strLen2, 0, sizeof(PropertyString*)* 80);
    }


    PropertyString* ScriptContext::AddPropertyString2(const Js::PropertyRecord* propString)
    {
        const wchar_t* buf = propString->GetBuffer();
        const uint i = PropertyStringMap::PStrMapIndex(buf[0]);
        if (propertyStrings[i] == NULL)
        {
            InitPropertyStringMap(i);
        }
        const uint j = PropertyStringMap::PStrMapIndex(buf[1]);
        if (propertyStrings[i]->strLen2[j] == NULL && !isClosed)
        {
            propertyStrings[i]->strLen2[j] = GetLibrary()->CreatePropertyString(propString, this->GeneralAllocator());
            this->TrackPid(propString);
        }
        return propertyStrings[i]->strLen2[j];
    }

    PropertyString* ScriptContext::CachePropertyString2(const PropertyRecord* propString)
    {
        Assert(propString->GetLength() == 2);
        const wchar_t* propertyName = propString->GetBuffer();
        if ((propertyName[0] <= 'z') && (propertyName[1] <= 'z') && (propertyName[0] >= '0') && (propertyName[1] >= '0') && ((propertyName[0] > '9') || (propertyName[1] > '9')))
        {
            return AddPropertyString2(propString);
        }
        return NULL;
    }


    PropertyString* ScriptContext::GetPropertyString(PropertyId propertyId)
    {
        PropertyStringCacheMap* propertyStringMap = this->GetLibrary()->EnsurePropertyStringMap();

        PropertyString *string;
        RecyclerWeakReference<PropertyString>* stringReference;
        if (propertyStringMap->TryGetValue(propertyId, &stringReference))
        {
            string = stringReference->Get();
            if (string != null)
            {
                return string;
            }
        }

        const Js::PropertyRecord* propertyName = this->GetPropertyName(propertyId);
        string = this->GetLibrary()->CreatePropertyString(propertyName);
        propertyStringMap->Item(propertyId, recycler->CreateWeakReferenceHandle(string));

        return string;
    }

    void ScriptContext::InvalidatePropertyStringCache(PropertyId propertyId, Type* type)
    {
        PropertyStringCacheMap* propertyStringMap = this->javascriptLibrary->GetPropertyStringMap();
        if (propertyStringMap != null)
        {
            PropertyString *string = null;
            RecyclerWeakReference<PropertyString>* stringReference;
            if (propertyStringMap->TryGetValue(propertyId, &stringReference))
            {
                string = stringReference->Get();
            }
            if (string)
            {
                PropertyCache const* cache = string->GetPropertyCache();
                if (cache->type == type)
                {
                    string->ClearPropertyCache();
                }
            }
        }
    }

    void ScriptContext::CleanupWeakReferenceDictionaries()
    {
        if (!isWeakReferenceDictionaryListCleared)
        {
            SListBase<JsUtil::IWeakReferenceDictionary*>::Iterator iter(&this->weakReferenceDictionaryList);

            while (iter.Next())
            {
                JsUtil::IWeakReferenceDictionary* weakReferenceDictionary = iter.Data();

                weakReferenceDictionary->Cleanup();
            }
        }
    }

    JavascriptString* ScriptContext::GetIntegerString(Var aValue)
    {
        return this->GetIntegerString(TaggedInt::ToInt32(aValue));
    }

    JavascriptString* ScriptContext::GetIntegerString(uint value)
    {
        if (value <= INT_MAX)
        {
            return this->GetIntegerString((int)value);
        }
        return TaggedInt::ToString(value, this);
    }

    JavascriptString* ScriptContext::GetIntegerString(int value)
    {
        // Optimize for 0-9
        if (0 <= value && value <= 9)
        {
            return GetLibrary()->GetCharStringCache().GetStringForCharA('0' + static_cast<char>(value));
        }

        if (this->integerStringMap == NULL)
        {
            this->integerStringMap = Anew(this->GeneralAllocator(), LargeUIntHashTable<JavascriptString *>,
                this->GeneralAllocator());
        }

        JavascriptString *string = this->integerStringMap->Lookup(value);

        if (string == NULL)
        {
            // Add the string to hash table cache
            // Don't add if table is getting too full.  We'll be holding on to
            // too many strings, and table lookup will become too slow.
            if (this->integerStringMap->IsDenserThan<10>())
            {
                // Use recycler memory
                string = TaggedInt::ToString(value, this);
            }
            else
            {
                wchar_t stringBuffer[20];

                TaggedInt::ToBuffer(value, stringBuffer, _countof(stringBuffer));
                string = JavascriptString::NewCopySzFromArena(stringBuffer, this, this->GeneralAllocator());
                this->integerStringMap->Add(value, string);
            }
        }

        return string;
    }

    void ScriptContext::CheckEvalRestriction()
    {
        HRESULT hr = S_OK;
        Var domError = nullptr;
        HostScriptContext* hostScriptContext = this->GetHostScriptContext();

        BEGIN_LEAVE_SCRIPT(this)
        {
            if (!FAILED(hr = hostScriptContext->CheckEvalRestriction()))
            {
                return;
            }

            hr = hostScriptContext->HostExceptionFromHRESULT(hr, &domError);
        }
        END_LEAVE_SCRIPT(this);

        if (FAILED(hr))
        {
            Js::JavascriptError::MapAndThrowError(this, hr);
        }

        if (domError != nullptr)
        {
            JavascriptExceptionOperators::Throw(domError, this);
        }

        AssertMsg(false, "We should have thrown by now.");
        Js::JavascriptError::MapAndThrowError(this, E_FAIL);
    }

    JavascriptFunction* ScriptContext::LoadScript(const wchar_t* script, SRCINFO const * pSrcInfo, CompileScriptException * pse, bool isExpression, bool disableDeferredParse, bool isForNativeCode, Utf8SourceInfo** ppSourceInfo, const wchar_t *rootDisplayName)
    {
        if (pSrcInfo == null)
        {
            pSrcInfo = this->cache->noContextGlobalSourceInfo;
        }

        Assert(!this->threadContext->IsScriptActive());
        Assert(pse != null);
        try
        {
            AUTO_NESTED_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
            Js::AutoDynamicCodeReference dynamicFunctionReference(this);

            // Convert to UTF8 and then load that
            size_t length = wcslen(script);
            if (length > UINT_MAX)
            {
                Js::Throw::OutOfMemory();
            }

            // Allocate memory for the UTF8 output buffer.
            // We need at most 3 bytes for each Unicode code point.
            // The + 1 is to include the terminating NUL.
            // Nit:  Technically, we know that the NUL only needs 1 byte instead of
            // 3, but that's difficult to express in a SAL annotation for "EncodeInto".
            size_t cbUtf8Buffer = (length + 1) * 3;

            LPUTF8 utf8Script = RecyclerNewArrayLeafTrace(this->GetRecycler(), utf8char_t, cbUtf8Buffer);

            Assert(length < MAXLONG);
            size_t cbNeeded = utf8::EncodeIntoAndNullTerminate(utf8Script, script, static_cast<charcount_t>(length));

#if DBG_DUMP
            if (Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::ParsePhase) && Configuration::Global.flags.Verbose)
            {
                Output::Print(L"Loading script.\n"
                    L"  Unicode (in bytes)    %u\n"
                    L"  UTF-8 size (in bytes) %u\n"
                    L"  Expected savings      %d\n", length * sizeof(wchar_t), cbNeeded, length * sizeof(wchar_t)-cbNeeded);
            }
#endif

            // Free unused bytes
            Assert(cbNeeded + 1 <= cbUtf8Buffer);
            *ppSourceInfo = Utf8SourceInfo::New(this, utf8Script, length, cbNeeded, pSrcInfo);

            //
            // Parse and execute the JavaScript file.
            //
            HRESULT hr;
            Parser parser(this);

            SourceContextInfo * sourceContextInfo = pSrcInfo->sourceContextInfo;

            // Invoke the parser, passing in the global function name, which we will then run to execute
            // the script.
            // This is global function called from jc or scriptengine::parse, in both case we can return the value to the caller.
            ULONG grfscr = fscrGlobalCode | (isExpression ? fscrReturnExpression : 0) | (isForNativeCode? fscrIsNativeCode : 0);
            if (!disableDeferredParse && (length > Parser::GetDeferralThreshold(sourceContextInfo->sourceDynamicProfileManager)))
            {
                grfscr |= fscrDeferFncParse;
            }

            if (PHASE_FORCE1(Js::EvalCompilePhase))
            {
                // pretend it is eval
                grfscr |= fscrEval;
            }

            ParseNodePtr parseTree;
            hr = parser.ParseCesu8Source(&parseTree, utf8Script, cbNeeded, grfscr, pse, &sourceContextInfo->nextLocalFunctionId,
                sourceContextInfo);
            (*ppSourceInfo)->SetParseFlags(grfscr);

            if (FAILED(hr) || parseTree == null || (grfscr & fscrStmtCompletion) != 0)
            {
                return null;
            }

            Assert(length < MAXLONG);
            uint sourceIndex = this->SaveSourceNoCopy(*ppSourceInfo, static_cast<charcount_t>(length), /*isCesu8*/ true);
            JavascriptFunction * pFunction = GenerateRootFunction(parseTree, sourceIndex, &parser, grfscr, pse, rootDisplayName);
            if (pFunction != nullptr && this->IsProfiling())
            {
                RegisterScript(pFunction->GetFunctionProxy());
            }
            return pFunction;
        }
        catch (Js::OutOfMemoryException)
        {
            pse->ProcessError(null, E_OUTOFMEMORY, null);
            return null;
        }
        catch (Js::StackOverflowException)
        {
            pse->ProcessError(null, VBSERR_OutOfStack, null);
            return null;
        }
    }

    JavascriptFunction* ScriptContext::LoadScript(LPCUTF8 script, size_t cb, SRCINFO const * pSrcInfo, CompileScriptException * pse, bool isExpression, bool disableDeferredParse, bool isForNativeCode, Utf8SourceInfo** ppSourceInfo, const wchar_t *rootDisplayName)
    {
        if (pSrcInfo == null)
        {
            pSrcInfo = this->cache->noContextGlobalSourceInfo;
        }

        Assert(!this->threadContext->IsScriptActive());
        Assert(pse != null);
        try
        {
            AUTO_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
            Js::AutoDynamicCodeReference dynamicFunctionReference(this);

            //
            // Parse and execute the JavaScript file.
            //
            HRESULT hr;
            Parser parser(this);
            SourceContextInfo * sourceContextInfo = pSrcInfo->sourceContextInfo;
            // Invoke the parser, passing in the global function name, which we will then run to execute
            // the script.
            ULONG grfscr = fscrGlobalCode | (isExpression ? fscrReturnExpression : 0) | (isForNativeCode? fscrIsNativeCode : 0);
            if (!disableDeferredParse && (cb > Parser::GetDeferralThreshold(sourceContextInfo->sourceDynamicProfileManager)))
            {
                grfscr |= fscrDeferFncParse;
            }

            if (PHASE_FORCE1(Js::EvalCompilePhase))
            {
                // pretend it is eval
                grfscr |= fscrEval;
            }

#if DBG_DUMP
            if (Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::ParsePhase) && Configuration::Global.flags.Verbose)
            {
                size_t length = utf8::ByteIndexIntoCharacterIndex(script, cb, utf8::doAllowThreeByteSurrogates);
                Output::Print(L"Direct UTF-8 parsing.\n"
                    L"  Would have expanded into:   %u (in bytes)\n"
                    L"  UTF-8 size (in bytes):      %u (in bytes)\n"
                    L"  Expected savings:           %d (in bytes)\n", length * sizeof(wchar_t), cb, length * sizeof(wchar_t)-cb);
            }
#endif
            ParseNodePtr parseTree;
            hr = parser.ParseUtf8Source(&parseTree, script, cb, grfscr, pse, &sourceContextInfo->nextLocalFunctionId,
                sourceContextInfo);

            if (FAILED(hr) || parseTree == null)
            {
                return null;
            }

            // We do not own the memory passed into DefaultLoadScriptUtf8. We need to save it so we copy the memory.
            *ppSourceInfo = Utf8SourceInfo::New(this, script, parser.GetSourceIchLim(), cb, pSrcInfo);
            (*ppSourceInfo)->SetParseFlags(grfscr);
            uint sourceIndex = this->SaveSourceNoCopy(*ppSourceInfo, parser.GetSourceIchLim(), /* isCesu8*/ false);

            JavascriptFunction * pFunction = GenerateRootFunction(parseTree, sourceIndex, &parser, grfscr, pse, rootDisplayName);
            if (pFunction != nullptr && this->IsProfiling())
            {
                RegisterScript(pFunction->GetFunctionProxy());
            }
            return pFunction;
        }
        catch (Js::OutOfMemoryException)
        {
            pse->ProcessError(null, E_OUTOFMEMORY, null);
            return null;
        }
        catch (Js::StackOverflowException)
        {
            pse->ProcessError(null, VBSERR_OutOfStack, null);
            return null;
        }
    }

    JavascriptFunction* ScriptContext::GenerateRootFunction(ParseNodePtr parseTree, uint sourceIndex, Parser* parser, ulong grfscr, CompileScriptException * pse, const wchar_t *rootDisplayName)
    {
        HRESULT hr;

        // Get the source code to keep it alive during the bytecode generation process
        LPCUTF8 source = this->GetSource(sourceIndex)->GetSource(L"ScriptContext::GenerateRootFunction");
        Assert(source != null); // Source should not have been reclaimed by now

        // Generate bytecode and native code
        ParseableFunctionInfo* body = NULL;
        hr = GenerateByteCode(parseTree, grfscr, this, &body, sourceIndex, false, parser, pse);
        this->GetSource(sourceIndex)->SetByteCodeGenerationFlags(grfscr);
        if (FAILED(hr))
        {
            return null;
        }

        body->SetDisplayName(rootDisplayName);
        body->SetIsTopLevel(true);

        JavascriptFunction* rootFunction = javascriptLibrary->CreateScriptFunction(body);

        return rootFunction;
    }

    BOOL ScriptContext::ReserveStaticTypeIds(__in int first, __in int last)
    {
        return threadContext->ReserveStaticTypeIds(first, last);
    }

    TypeId ScriptContext::ReserveTypeIds(int count)
    {
        return threadContext->ReserveTypeIds(count);
    }

    TypeId ScriptContext::CreateTypeId()
    {
        return threadContext->CreateTypeId();
    }

    void ScriptContext::OnScriptStart(bool isRoot, bool isForcedEnter, bool isScript)
    {
        if (this->scriptStartEventHandler != null && ((isRoot && threadContext->GetCallRootLevel() == 1) || isForcedEnter))
        {
            diagProbesContainer.isForcedToEnterScriptStart = false;

            this->scriptStartEventHandler(this);
        }

#ifdef ENABLE_NATIVE_CODEGEN
        //Blue 5491: Only start codegen if isScript. Avoid it if we are not really starting script and called from risky region such as catch handler.
        if (isScript)
        {
            NativeCodeGenEnterScriptStart(this->GetNativeCodeGenerator());
        }
#endif
    }

    void ScriptContext::OnScriptEnd(bool isRoot, bool isForcedEnd)
    {
        if ((isRoot && threadContext->GetCallRootLevel() == 1) || isForcedEnd)
        {
            if (this->scriptEndEventHandler != null)
            {
                this->scriptEndEventHandler(this);
            }
        }
    }

#ifdef FAULT_INJECTION
    void ScriptContext::DisposeScriptContextByFaultInjection() {
        if (this->disposeScriptByFaultInjectionEventHandler != null)
        {
            this->disposeScriptByFaultInjectionEventHandler(this);
        }
    }
#endif

    template <bool stackProbe, bool leaveForHost>
    bool ScriptContext::LeaveScriptStart(void * frameAddress)
    {
        ThreadContext * threadContext = this->threadContext;
        if (!threadContext->IsScriptActive())
        {
            // we should have enter always these days.
            Assert(FALSE);
            return false;
        }

        // Make sure the host function will have at least 32k of stack available.
        if (stackProbe)
        {
            threadContext->ProbeStack(Js::Constants::MinStackCallout, this);
        }
        else
        {
            AssertMsg(ExceptionCheck::HasStackProbe(), "missing stack probe");
        }

        threadContext->LeaveScriptStart<leaveForHost>(frameAddress);
        return true;
    }

    template <bool leaveForHost>
    void ScriptContext::LeaveScriptEnd(void * frameAddress)
    {
        this->threadContext->LeaveScriptEnd<leaveForHost>(frameAddress);
    }

    // explicit instantiations
    template bool ScriptContext::LeaveScriptStart<true, true>(void * frameAddress);
    template bool ScriptContext::LeaveScriptStart<true, false>(void * frameAddress);
    template bool ScriptContext::LeaveScriptStart<false, true>(void * frameAddress);
    template void ScriptContext::LeaveScriptEnd<true>(void * frameAddress);
    template void ScriptContext::LeaveScriptEnd<false>(void * frameAddress);

    bool ScriptContext::EnsureInterpreterArena(ArenaAllocator **ppAlloc)
    {
        bool fNew = false;
        if (this->interpreterArena == null)
        {
            this->interpreterArena = this->GetRecycler()->CreateGuestArena(L"Interpreter", Throw::OutOfMemory);
            fNew = true;
        }
        *ppAlloc = this->interpreterArena;
        return fNew;
    }

    void ScriptContext::ReleaseInterpreterArena()
    {
        AssertMsg(this->interpreterArena, "No interpreter arena to release");
        if (this->interpreterArena)
        {
            this->GetRecycler()->DeleteGuestArena(this->interpreterArena);
            this->interpreterArena = NULL;
        }
    }


    void ScriptContext::ReleaseGuestArena()
    {
        AssertMsg(this->guestArena, "No guest arena to release");
        if (this->guestArena)
        {
            this->GetRecycler()->DeleteGuestArena(this->guestArena);
            this->guestArena = NULL;
        }
    }

    void ScriptContext::SetScriptStartEventHandler(ScriptContext::EventHandler eventHandler)
    {
        AssertMsg(this->scriptStartEventHandler == null, "Do not support multi-cast yet");
        this->scriptStartEventHandler = eventHandler;
    }
    void ScriptContext::SetScriptEndEventHandler(ScriptContext::EventHandler eventHandler)
    {
        AssertMsg(this->scriptEndEventHandler == null, "Do not support multi-cast yet");
        this->scriptEndEventHandler = eventHandler;
    }

#ifdef FAULT_INJECTION
    void ScriptContext::SetDisposeDisposeByFaultInjectionEventHandler(ScriptContext::EventHandler eventHandler)
    {
        AssertMsg(this->disposeScriptByFaultInjectionEventHandler == null, "Do not support multi-cast yet");
        this->disposeScriptByFaultInjectionEventHandler = eventHandler;
    }
#endif

    bool ScriptContext::SaveSourceCopy(Utf8SourceInfo* const sourceInfo, int cchLength, bool isCesu8, uint * index)
    {
        HRESULT hr = S_OK;
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            *index = this->SaveSourceCopy(sourceInfo, cchLength, isCesu8);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
        return hr == S_OK;
    }

    uint ScriptContext::SaveSourceCopy(Utf8SourceInfo* sourceInfo, int cchLength, bool isCesu8)
    {
        Utf8SourceInfo* newSource = Utf8SourceInfo::Clone(this, sourceInfo);

        return SaveSourceNoCopy(newSource, cchLength, isCesu8);
    }


    Utf8SourceInfo* ScriptContext::CloneSourceCrossContext(Utf8SourceInfo* crossContextSourceInfo, SRCINFO const* srcInfo)
    {
        return Utf8SourceInfo::CloneNoCopy(this, crossContextSourceInfo, srcInfo);
    }


    uint ScriptContext::SaveSourceNoCopy(Utf8SourceInfo* sourceInfo, int cchLength, bool isCesu8)
    {
        Assert(sourceInfo->GetScriptContext() == this);
        if(this->IsInDebugMode() && sourceInfo->debugModeSource == nullptr && !sourceInfo->debugModeSourceIsEmpty)
        {
            sourceInfo->SetInDebugMode(true);
        }

        RecyclerWeakReference<Utf8SourceInfo>* sourceWeakRef = this->GetRecycler()->CreateWeakReferenceHandle<Utf8SourceInfo>(sourceInfo);
        sourceInfo->SetIsCesu8(isCesu8);

        return sourceList->SetAtFirstFreeSpot(sourceWeakRef);
    }

    void ScriptContext::CloneSources(ScriptContext* sourceContext)
    {
        sourceContext->sourceList->Map([=](int index, RecyclerWeakReference<Utf8SourceInfo>* sourceInfo)
        {
            Utf8SourceInfo* info = sourceInfo->Get();
            if (info)
            {
                CloneSource(info);
            }
        });
    }

    uint ScriptContext::CloneSource(Utf8SourceInfo* info)
    {
        return this->SaveSourceCopy(info, info->GetCchLength(), info->GetIsCesu8());
    }

    Utf8SourceInfo* ScriptContext::GetSource(uint index)
    {
        Assert(this->sourceList->IsItemValid(index)); // This assert should be a subset of info != null- if info was null, in the last collect, we'd have invalidated the item
        Utf8SourceInfo* info = this->sourceList->Item(index)->Get();
        Assert(info != null); // Should still be alive if this method is being called
        return info;
    }

    bool ScriptContext::IsItemValidInSourceList(int index)
    {
        return (index < this->sourceList->Count()) && this->sourceList->IsItemValid(index);
    }

    void ScriptContext::RecordException(JavascriptExceptionObject * exceptionObject, bool propagateToDebugger)
    {
        Assert(this->threadContext->GetRecordedException() == null || GetThreadContext()->HasUnhandledException());
        this->threadContext->SetRecordedException(exceptionObject, propagateToDebugger);
#if DBG
        exceptionObject->FillStackBackTrace();
#endif
    }

    void ScriptContext::RethrowRecordedException(JavascriptExceptionObject::HostWrapperCreateFuncType hostWrapperCreateFunc)
    {
        bool considerPassingToDebugger = false;
        JavascriptExceptionObject * exceptionObject = this->GetAndClearRecordedException(&considerPassingToDebugger);
        if (hostWrapperCreateFunc)
        {
            exceptionObject->SetHostWrapperCreateFunc(exceptionObject->GetScriptContext() != this ? hostWrapperCreateFunc : null);
        }
        JavascriptExceptionOperators::RethrowExceptionObject(exceptionObject, this, considerPassingToDebugger);
    }

    Js::JavascriptExceptionObject * ScriptContext::GetAndClearRecordedException(bool *considerPassingToDebugger)
    {
        JavascriptExceptionObject * exceptionObject = this->threadContext->GetRecordedException();
        Assert(exceptionObject != null);
        if (considerPassingToDebugger)
        {
            *considerPassingToDebugger = this->threadContext->GetPropagateException();
        }
        this->threadContext->SetRecordedException(null);
        return exceptionObject;
    }

    bool ScriptContext::IsInEvalMap(FastEvalMapString const& key, BOOL isIndirect, ScriptFunction **ppFuncScript)
    {
        EvalCacheDictionary *dict = isIndirect ? this->cache->indirectEvalCacheDictionary : this->cache->evalCacheDictionary;
        if (dict == null)
        {
            return false;
        }
#ifdef PROFILE_EVALMAP
        if (Configuration::Global.flags.ProfileEvalMap)
        {
            charcount_t len = key.str.GetLength();
            if (dict->TryGetValue(key, ppFuncScript))
            {
                Output::Print(L"EvalMap cache hit:\t souce size = %d\n", len);
            }
            else
            {
                Output::Print(L"EvalMap cache miss:\t souce size = %d\n", len);
            }
        }
#endif

        // If eval map cleanup is false, to preserve existing behavior, add it to the eval map mru list
        bool success = dict->TryGetValue(key, ppFuncScript);

        if (success)
        {
            dict->NotifyAdd(key);
#ifdef VERBOSE_EVAL_MAP
#if DBG
            dict->DumpKeepAlives();
#endif
#endif
        }

        return success;
    }

    void ScriptContext::BeginDynamicFunctionReferences()
    {
        if (this->dynamicFunctionReference == null)
        {
            this->dynamicFunctionReference = RecyclerNew(this->recycler, FunctionReferenceList, this->recycler);
            this->BindReference(this->dynamicFunctionReference);
            this->dynamicFunctionReferenceDepth = 0;
        }

        this->dynamicFunctionReferenceDepth++;
    }

    void ScriptContext::EndDynamicFunctionReferences()
    {
        Assert(this->dynamicFunctionReference != null);

        this->dynamicFunctionReferenceDepth--;

        if (this->dynamicFunctionReferenceDepth == 0)
        {
            this->dynamicFunctionReference->Clear();
        }
    }

    void ScriptContext::RegisterDynamicFunctionReference(FunctionProxy* func)
    {
        Assert(this->dynamicFunctionReferenceDepth > 0);
        this->dynamicFunctionReference->Push(func);
    }

    void ScriptContext::AddToEvalMap(FastEvalMapString const& key, BOOL isIndirect, ScriptFunction *pFuncScript)
    {
        EvalCacheDictionary *dict = isIndirect ? this->cache->indirectEvalCacheDictionary : this->cache->evalCacheDictionary;
        if (dict == null)
        {
            EvalCacheTopLevelDictionary* evalTopDictionary = RecyclerNew(this->recycler, EvalCacheTopLevelDictionary, this->recycler);
            dict = RecyclerNew(this->recycler, EvalCacheDictionary, evalTopDictionary, recycler);
            if (isIndirect)
            {
                this->cache->indirectEvalCacheDictionary = dict;
            }
            else
            {
                this->cache->evalCacheDictionary = dict;
            }
        }

        dict->Add(key, pFuncScript);
    }

    bool ScriptContext::IsInNewFunctionMap(EvalMapString const& key, ParseableFunctionInfo **ppFuncBody)
    {
        if (this->cache->newFunctionCache == null)
        {
            return false;
        }

        // If eval map cleanup is false, to preserve existing behavior, add it to the eval map mru list
        bool success = this->cache->newFunctionCache->TryGetValue(key, ppFuncBody);
        if (success)
        {
            this->cache->newFunctionCache->NotifyAdd(key);
#ifdef VERBOSE_EVAL_MAP
#if DBG
            this->cache->newFunctionCache->DumpKeepAlives();
#endif
#endif
        }

        return success;
    }

    void ScriptContext::AddToNewFunctionMap(EvalMapString const& key, ParseableFunctionInfo *pFuncBody)
    {
        if (this->cache->newFunctionCache == null)
        {
            this->cache->newFunctionCache = RecyclerNew(this->recycler, NewFunctionCache, this->recycler);
        }
        this->cache->newFunctionCache->Add(key, pFuncBody);
    }


    void ScriptContext::EnsureSourceContextInfoMap()
    {
        if (this->cache->sourceContextInfoMap == null)
        {
            this->cache->sourceContextInfoMap = RecyclerNew(this->GetRecycler(), SourceContextInfoMap, this->GetRecycler());
        }
    }

    void ScriptContext::EnsureDynamicSourceContextInfoMap()
    {
        if (this->cache->dynamicSourceContextInfoMap == null)
        {
            this->cache->dynamicSourceContextInfoMap = RecyclerNew(this->GetRecycler(), DynamicSourceContextInfoMap, this->GetRecycler());
        }
    }

    SourceContextInfo* ScriptContext::GetSourceContextInfo(uint hash)
    {
        SourceContextInfo * sourceContextInfo;
        if (this->cache->dynamicSourceContextInfoMap && this->cache->dynamicSourceContextInfoMap->TryGetValue(hash, &sourceContextInfo))
        {
            return sourceContextInfo;
        }
        return null;
    }

    SourceContextInfo* ScriptContext::CreateSourceContextInfo(uint hash, DWORD_PTR hostSourceContext)
    {
        EnsureDynamicSourceContextInfoMap();
        if (this->GetSourceContextInfo(hash) != null)
        {
            return const_cast<SourceContextInfo*>(this->cache->noContextSourceContextInfo);
        }

        if (this->cache->dynamicSourceContextInfoMap->Count() > INMEMORY_CACHE_MAX_PROFILE_MANAGER)
        {
            OUTPUT_TRACE(Js::DynamicProfilePhase, L"Max of dynamic script profile info reached.\n");
            return const_cast<SourceContextInfo*>(this->cache->noContextSourceContextInfo);
        }

        // NB: This is capped so we can continue allocating in the arena
        SourceContextInfo * sourceContextInfo = RecyclerNewStructZ(this->GetRecycler(), SourceContextInfo);
        sourceContextInfo->sourceContextId = this->GetNextSourceContextId();
        sourceContextInfo->dwHostSourceContext = hostSourceContext;
        sourceContextInfo->isHostDynamicDocument = true;
        sourceContextInfo->hash = hash;
        sourceContextInfo->sourceDynamicProfileManager = this->threadContext->GetSourceDynamicProfileManager(this->GetUrl(), hash, &referencesSharedDynamicSourceContextInfo);

        // For the host provided dynamic code (if hostSourceContext is not NoHostSourceContext), do not add to dynamicSourceContextInfoMap
        if (hostSourceContext == Js::Constants::NoHostSourceContext)
        {
            this->cache->dynamicSourceContextInfoMap->Add(hash, sourceContextInfo);
        }
        return sourceContextInfo;
    }

    //
    // Makes a copy of the URL to be stored in the map.
    //
    SourceContextInfo * ScriptContext::CreateSourceContextInfo(DWORD_PTR sourceContext, wchar_t const * url, size_t len,
        IActiveScriptDataCache* profileDataCache, wchar_t const * sourceMapUrl /*= NULL*/, size_t sourceMapUrlLen /*= 0*/)
    {
        // Take etw rundown lock on this thread context. We are going to init/add to sourceContextInfoMap.
        AutoCriticalSection autocs(GetThreadContext()->GetEtwRundownCriticalSection());

        EnsureSourceContextInfoMap();
        Assert(this->GetSourceContextInfo(sourceContext, profileDataCache) == null);
        SourceContextInfo * sourceContextInfo = RecyclerNewStructZ(this->GetRecycler(), SourceContextInfo);
        sourceContextInfo->sourceContextId = this->GetNextSourceContextId();
        sourceContextInfo->dwHostSourceContext = sourceContext;
        sourceContextInfo->isHostDynamicDocument = false;
        sourceContextInfo->sourceDynamicProfileManager = null;

        if (url != null)
        {
            sourceContextInfo->url = CopyString(url, len, this->SourceCodeAllocator());
            EtwTrace::LogSourceModuleLoadEvent(this, sourceContext, url);
        }
        if (sourceMapUrl != null && sourceMapUrlLen != 0)
        {
            sourceContextInfo->sourceMapUrl = CopyString(sourceMapUrl, sourceMapUrlLen, this->SourceCodeAllocator());
        }

        if (!this->startupComplete)
        {
            sourceContextInfo->sourceDynamicProfileManager = SourceDynamicProfileManager::LoadFromDynamicProfileStorage(sourceContextInfo, this, profileDataCache);
            Assert(sourceContextInfo->sourceDynamicProfileManager != NULL);
        }

        this->cache->sourceContextInfoMap->Add(sourceContext, sourceContextInfo);
        return sourceContextInfo;
    }

    // static
    const wchar_t* ScriptContext::CopyString(const wchar_t* str, size_t charCount, ArenaAllocator* alloc)
    {
        size_t length = charCount + 1; // Add 1 for the NULL.
        wchar_t* copy = AnewArray(alloc, wchar_t, length);
        js_memcpy_s(copy, (length - 1) * sizeof(wchar_t), str, (length - 1) * sizeof(wchar_t));
        copy[length - 1] = L'\0';
        return copy;
    }

    SourceContextInfo *  ScriptContext::GetSourceContextInfo(DWORD_PTR sourceContext, IActiveScriptDataCache* profileDataCache)
    {
        if (sourceContext == Js::Constants::NoHostSourceContext)
        {
            return const_cast<SourceContextInfo*>(this->cache->noContextSourceContextInfo);
        }

        // We only init sourceContextInfoMap, don't need to lock.
        EnsureSourceContextInfoMap();
        SourceContextInfo * sourceContextInfo;
        if (this->cache->sourceContextInfoMap->TryGetValue(sourceContext, &sourceContextInfo))
        {
            if (profileDataCache &&
                sourceContextInfo->sourceDynamicProfileManager != null &&
                !sourceContextInfo->sourceDynamicProfileManager->IsProfileLoadedFromWinInet() &&
                !this->startupComplete)
            {
                bool profileLoaded = sourceContextInfo->sourceDynamicProfileManager->LoadFromProfileCache(profileDataCache, sourceContextInfo->url);
                if (profileLoaded)
                {
                    JSETW(EventWriteJSCRIPT_PROFILE_LOAD(sourceContextInfo->dwHostSourceContext, this));
                }
            }
            return sourceContextInfo;
        }
        return null;
    }

    SRCINFO const *
        ScriptContext::GetModuleSrcInfo(Js::ModuleID moduleID)
    {
            if (moduleSrcInfoCount <= moduleID)
            {
                uint newCount = moduleID + 4;  // Preallocate 4 more slots, moduleID don't usually grow much

                SRCINFO const ** newModuleSrcInfo = RecyclerNewArrayZ(this->GetRecycler(), SRCINFO const*, newCount);
                memcpy(newModuleSrcInfo, cache->moduleSrcInfo, sizeof(SRCINFO const *)* moduleSrcInfoCount);
                cache->moduleSrcInfo = newModuleSrcInfo;
                moduleSrcInfoCount = newCount;
                cache->moduleSrcInfo[0] = this->cache->noContextGlobalSourceInfo;
            }

            SRCINFO const * si = cache->moduleSrcInfo[moduleID];
            if (si == null)
            {
                SRCINFO * newSrcInfo = RecyclerNewStructZ(this->GetRecycler(), SRCINFO);
                newSrcInfo->sourceContextInfo = this->cache->noContextSourceContextInfo;
                newSrcInfo->moduleID = moduleID;
                cache->moduleSrcInfo[moduleID] = newSrcInfo;
                si = newSrcInfo;
            }
            return si;
    }

    void ScriptContext::UpdateTimeZoneInfo()
    {
        GetTimeZoneInformation(&timeZoneInfo);
        _tzset();
    }

#ifdef PROFILE_EXEC
    void
        ScriptContext::DisableProfiler()
    {
            disableProfiler = true;
    }

    Profiler *
        ScriptContext::CreateProfiler()
    {
            Assert(profiler == null);
            if (Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag))
            {
                this->profiler = NoCheckHeapNew(ScriptContextProfiler);
                this->profiler->Initialize(GetThreadContext()->GetPageAllocator(), threadContext->GetRecycler());

#ifdef ENABLE_NATIVE_CODEGEN
                CreateProfilerNativeCodeGen(this->nativeCodeGen, this->profiler);
#endif

                this->isProfilerCreated = true;
                Profiler * oldProfiler = this->threadContext->GetRecycler()->GetProfiler();
                this->threadContext->GetRecycler()->SetProfiler(this->profiler->GetProfiler(), this->profiler->GetBackgroundRecyclerProfiler());
                return oldProfiler;
            }
            return null;
    }

    void
        ScriptContext::SetRecyclerProfiler()
    {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag));
            AssertMsg(this->profiler != null, "Profiler tag is supplied but the profiler pointer is NULL");

            if (this->ensureParentInfo)
            {
                this->hostScriptContext->EnsureParentInfo();
                this->ensureParentInfo = false;
            }

            this->GetRecycler()->SetProfiler(this->profiler->GetProfiler(), this->profiler->GetBackgroundRecyclerProfiler());
    }

    void
        ScriptContext::SetProfilerFromScriptContext(ScriptContext * scriptContext)
    {
            // this function needs to be called before any code gen happens so
            // that access to codegenProfiler won't have concurrency issues
            if (Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag))
            {
                Assert(this->profiler != null);
                Assert(this->isProfilerCreated);
                Assert(scriptContext->profiler != null);
                Assert(scriptContext->isProfilerCreated);


                scriptContext->profiler->ProfileMerge(this->profiler);

                this->profiler->Release();
                this->profiler = scriptContext->profiler;
                this->profiler->AddRef();
                this->isProfilerCreated = false;

#ifdef ENABLE_NATIVE_CODEGEN
                SetProfilerFromNativeCodeGen(this->nativeCodeGen, scriptContext->GetNativeCodeGenerator());
#endif

                this->threadContext->GetRecycler()->SetProfiler(this->profiler->GetProfiler(), this->profiler->GetBackgroundRecyclerProfiler());
            }
    }

    void
        ScriptContext::ProfileBegin(Js::Phase phase)
    {
            AssertMsg((this->profiler != null) == Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag),
                "Profiler tag is supplied but the profiler pointer is NULL");
            if (this->profiler)
            {
                if (this->ensureParentInfo)
                {
                    this->hostScriptContext->EnsureParentInfo();
                    this->ensureParentInfo = false;
                }
                this->profiler->ProfileBegin(phase);
            }
    }

    void
        ScriptContext::ProfileEnd(Js::Phase phase)
    {
            AssertMsg((this->profiler != null) == Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag),
                "Profiler tag is supplied but the profiler pointer is NULL");
            if (this->profiler)
            {
                this->profiler->ProfileEnd(phase);
            }
    }

    void
        ScriptContext::ProfileSuspend(Js::Phase phase, Js::Profiler::SuspendRecord * suspendRecord)
    {
            AssertMsg((this->profiler != null) == Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag),
                "Profiler tag is supplied but the profiler pointer is NULL");
            if (this->profiler)
            {
                this->profiler->ProfileSuspend(phase, suspendRecord);
            }
    }

    void
        ScriptContext::ProfileResume(Js::Profiler::SuspendRecord * suspendRecord)
    {
            AssertMsg((this->profiler != null) == Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag),
                "Profiler tag is supplied but the profiler pointer is NULL");
            if (this->profiler)
            {
                this->profiler->ProfileResume(suspendRecord);
            }
    }

    void
        ScriptContext::ProfilePrint()
    {
            if (disableProfiler)
            {
                return;
            }

            Assert(profiler != null);
            recycler->EnsureNotCollecting();
            profiler->ProfilePrint(Js::Configuration::Global.flags.Profile.GetFirstPhase());
#ifdef ENABLE_NATIVE_CODEGEN
            ProfilePrintNativeCodeGen(this->nativeCodeGen);
#endif
    }
#endif

    inline void ScriptContext::CoreSetProfileEventMask(DWORD dwEventMask)
    {
        AssertMsg(m_pProfileCallback != NULL, "Assigning the event mask when there is no callback");
        m_dwEventMask = dwEventMask;
        m_fTraceFunctionCall = (dwEventMask & PROFILER_EVENT_MASK_TRACE_SCRIPT_FUNCTION_CALL);
        m_fTraceNativeFunctionCall = (dwEventMask & PROFILER_EVENT_MASK_TRACE_NATIVE_FUNCTION_CALL);

        m_fTraceDomCall = (dwEventMask & PROFILER_EVENT_MASK_TRACE_DOM_FUNCTION_CALL);
    }

    HRESULT ScriptContext::RegisterProfileProbe(IActiveScriptProfilerCallback *pProfileCallback, DWORD dwEventMask, DWORD dwContext, RegisterExternalLibraryType RegisterExternalLibrary, JavascriptMethod dispatchInvoke)
    {
        if (m_pProfileCallback != NULL)
        {
            return ACTIVPROF_E_PROFILER_PRESENT;
        }

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::RegisterProfileProbe\n");
        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"Info\nThunks Address :\n");
        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"DefaultEntryThunk : 0x%08X, CrossSite::DefaultThunk : 0x%08X, DefaultDeferredParsingThunk : 0x%08X\n", DefaultEntryThunk, CrossSite::DefaultThunk, DefaultDeferredParsingThunk);
        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ProfileEntryThunk : 0x%08X, CrossSite::ProfileThunk : 0x%08X, ProfileDeferredParsingThunk : 0x%08X, ProfileDeferredDeserializeThunk : 0x%08X,\n", ProfileEntryThunk, CrossSite::ProfileThunk, ProfileDeferredParsingThunk, ProfileDeferredDeserializeThunk);
        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptType :\n");
        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"PROFILER_SCRIPT_TYPE_USER : 0, PROFILER_SCRIPT_TYPE_DYNAMIC : 1, PROFILER_SCRIPT_TYPE_NATIVE : 2, PROFILER_SCRIPT_TYPE_DOM : 3\n");

        HRESULT hr = pProfileCallback->Initialize(dwContext);
        if (SUCCEEDED(hr))
        {
            m_pProfileCallback = pProfileCallback;
            pProfileCallback->AddRef();
            CoreSetProfileEventMask(dwEventMask);
            if (m_fTraceDomCall)
            {
                if (FAILED(pProfileCallback->QueryInterface(&m_pProfileCallback2)))
                {
                    m_fTraceDomCall = FALSE;
                }
            }

            if (webWorkerId != Js::Constants::NonWebWorkerContextId)
            {
                IActiveScriptProfilerCallback3 * pProfilerCallback3;
                if (SUCCEEDED(pProfileCallback->QueryInterface(&pProfilerCallback3)))
                {
                    pProfilerCallback3->SetWebWorkerId(webWorkerId);
                    pProfilerCallback3->Release();
                    // Omitting the HRESULT since it is up to the callback to make use of the webworker information.
                }
            }

#if DEBUG
            StartNewProfileSession();
#endif

            NativeCodeGenerator *pNativeCodeGen = this->GetNativeCodeGenerator();
            AutoOptionalCriticalSection autoAcquireCodeGenQueue(GetNativeCodeGenCriticalSection(pNativeCodeGen));

            this->SetProfileMode(TRUE);
            SetProfileModeNativeCodeGen(pNativeCodeGen, TRUE);

            // Register builtin functions
            if (m_fTraceNativeFunctionCall)
            {
                hr = this->RegisterBuiltinFunctions(RegisterExternalLibrary);
                if (FAILED(hr))
                {
                    return hr;
                }
            }

            this->RegisterAllScripts();

            // Set the dispatch profiler:
            this->SetDispatchProfile(TRUE, dispatchInvoke);

            // Update the function objects currently present in there.
            this->SetFunctionInRecyclerToProfileMode();
        }

        return hr;
    }

    HRESULT ScriptContext::SetProfileEventMask(DWORD dwEventMask)
    {
        if (m_pProfileCallback == NULL)
        {
            return ACTIVPROF_E_PROFILER_ABSENT;
        }

        return ACTIVPROF_E_UNABLE_TO_APPLY_ACTION;
    }

    HRESULT ScriptContext::DeRegisterProfileProbe(HRESULT hrReason, JavascriptMethod dispatchInvoke)
    {
        if (m_pProfileCallback == NULL)
        {
            return ACTIVPROF_E_PROFILER_ABSENT;
        }

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::DeRegisterProfileProbe\n");

        // Aquire the code gen working queue - we are going to change the thunks
        NativeCodeGenerator *pNativeCodeGen = this->GetNativeCodeGenerator();
        Assert(pNativeCodeGen);
        {
            AutoOptionalCriticalSection lock(GetNativeCodeGenCriticalSection(pNativeCodeGen));

            this->SetProfileMode(FALSE);
            SetProfileModeNativeCodeGen(pNativeCodeGen, FALSE);

            // Unset the dispatch profiler:
            if (dispatchInvoke != nullptr)
            {
                this->SetDispatchProfile(FALSE, dispatchInvoke);
            }
        }

        m_inProfileCallback = TRUE;
        HRESULT hr = m_pProfileCallback->Shutdown(hrReason);
        m_inProfileCallback = FALSE;
        m_pProfileCallback->Release();
        m_pProfileCallback = NULL;

        if (m_pProfileCallback2 != NULL)
        {
            m_pProfileCallback2->Release();
            m_pProfileCallback2 = NULL;
        }

#if DEBUG
        StopProfileSession();
#endif

        return hr;
    }

    void ScriptContext::SetProfileMode(BOOL fSet)
    {
        if (fSet)
        {
            AssertMsg(m_pProfileCallback != NULL, "In profile mode when there is no call back");
            this->CurrentThunk = ProfileEntryThunk;
            this->CurrentCrossSiteThunk = CrossSite::ProfileThunk;
            this->DeferredParsingThunk = ProfileDeferredParsingThunk;
            this->DeferredDeserializationThunk = ProfileDeferredDeserializeThunk;
            this->globalObject->EvalHelper = &Js::GlobalObject::ProfileModeEvalHelper;
#if DBG
            this->hadProfiled = true;
#endif
        }
        else
        {
            this->CurrentThunk = DefaultEntryThunk;
            this->CurrentCrossSiteThunk = CrossSite::DefaultThunk;
            this->DeferredParsingThunk = DefaultDeferredParsingThunk;
            this->globalObject->EvalHelper = &Js::GlobalObject::DefaultEvalHelper;

            // In Debug mode/Fast F12 library is still needed for built-in wrappers.
            if (!(this->IsInDebugMode() && this->IsExceptionWrapperForBuiltInsEnabled()))
            {
                this->javascriptLibrary->SetProfileMode(FALSE);
            }
        }
    }

    HRESULT ScriptContext::RegisterScript(Js::FunctionProxy * proxy, BOOL fRegisterScript /*default TRUE*/)
    {
        if (m_pProfileCallback == NULL)
        {
            return ACTIVPROF_E_PROFILER_ABSENT;
        }

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::RegisterScript, fRegisterScript : %s, IsFunctionDefer : %s\n", IsTrueOrFalse(fRegisterScript), IsTrueOrFalse(proxy->IsDeferred()));

        AssertMsg(proxy != NULL, "Function body cannot be null when calling reporting");
        AssertMsg(proxy->GetScriptContext() == this, "wrong script context while reporting the function?");

        if (fRegisterScript)
        {
            // Register the script to the callback.
            // REVIEW: do we really need to undefer everything?
            HRESULT hr = proxy->EnsureDeserialized()->Parse()->ReportScriptCompiled();
            if (FAILED(hr))
            {
                return hr;
            }
        }

        return !proxy->IsDeferred() ? proxy->GetFunctionBody()->RegisterFunction(false) : S_OK;
    }

    HRESULT ScriptContext::RegisterAllScripts()
    {
        AssertMsg(m_pProfileCallback != NULL, "Called register scripts when we dont have profile callback");

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::RegisterAllScripts started\n");

        // TODO: Once Utf8SourceInfo can generate the debug document text without requiring a function body,
        // this code can be considerably simplified to doing the following:
        // - scriptContext->MapScript() : Fire script compiled for each script
        // - scriptContext->MapFunction(): Fire function compiled for each function
        this->MapScript([](Utf8SourceInfo* sourceInfo)
        {
            FunctionBody* functionBody = sourceInfo->GetAnyParsedFunction();
            if (functionBody)
            {
                functionBody->ReportScriptCompiled();
            }
        });

        // FunctionCompiled events for all functions.
        this->MapFunction([](Js::FunctionBody* pFuncBody)
        {
            if (!pFuncBody->GetIsTopLevel() && pFuncBody->GetIsGlobalFunc())
            {
                // This must be the dummy function, generated due to the deferred parsing.
                return;
            }

            pFuncBody->RegisterFunction(TRUE, TRUE); // Ignore potential failure (worst case is not profiling).
        });

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::RegisterAllScripts ended\n");
        return S_OK;
    }

    // Shuts down and recreates the native code generator.  This is used when
    // attaching and detaching the debugger in order to clear the list of work
    // items that are pending in the JIT job queue.
    // Alloc first and then free so that the native code generator is at a different address
    HRESULT ScriptContext::RecreateNativeCodeGenerator()
    {
        NativeCodeGenerator* oldCodeGen = this->nativeCodeGen;

        HRESULT hr = S_OK;
        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
            this->nativeCodeGen = NewNativeCodeGenerator(this);
        SetProfileModeNativeCodeGen(this->GetNativeCodeGenerator(), this->IsProfiling());
        END_TRANSLATE_OOM_TO_HRESULT(hr);

        // Delete the native code generator and recreate so that all jobs get cleared properly
        // and re-jitted.
        CloseNativeCodeGenerator(oldCodeGen);
        DeleteNativeCodeGenerator(oldCodeGen);

        return hr;
    }

    HRESULT ScriptContext::OnDebuggerAttached()
    {
        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptContext::OnDebuggerAttached: start 0x%p\n", this);

        return OnDebuggerAttachedDetached(/*attach*/ true);

        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptContext::OnDebuggerAttached: done %p\n", this);
    }

    // Reverts the script context state back to the state before debugging began.
    HRESULT ScriptContext::OnDebuggerDetached()
    {
        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptContext::OnDebuggerDetached: start %p\n", this);

        return OnDebuggerAttachedDetached(/*attach*/ false);

        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptContext::OnDebuggerDetached: done %p\n", this);
    }

    HRESULT ScriptContext::OnDebuggerAttachedDetached(bool attach)
    {

        // notify threadContext that debugger is attaching so do not do expire
        struct AutoRestore
        {
            AutoRestore(ThreadContext* threadContext)
                :threadContext(threadContext)
            {
                this->threadContext->SetDebuggerAttaching(true);
            }
            ~AutoRestore()
            {
                this->threadContext->SetDebuggerAttaching(false);
            }

        private:
            ThreadContext* threadContext;

        } autoRestore(this->GetThreadContext());

        if (BinaryFeatureControl::LanguageService() || !Js::Configuration::Global.EnableJitInDebugMode())
        {
            if (attach)
            {
                // Now force nonative, so the job will not be put in jit queue.
                ForceNoNative();
            }
            else
            {
                // Take the runtime out of interpreted mode so the JIT
                // queue can be exercised.
                this->ForceNative();
            }
        }

        // Invalidate all the caches.
        this->threadContext->InvalidateAllProtoInlineCaches();
        this->threadContext->InvalidateAllStoreFieldInlineCaches();
        this->threadContext->InvalidateAllIsInstInlineCaches();

        if (!attach)
        {
            this->UnRegisterDebugThunk();

            // Remove all breakpoint probes
            this->diagProbesContainer.RemoveAllProbes();
        }

        HRESULT hr = S_OK;
        if (!CONFIG_FLAG(ForceDiagnosticsMode))
        {
            // Recreate the native code generator so that all pending
            // JIT work items will be cleared.
            hr = RecreateNativeCodeGenerator();
            if (FAILED(hr))
            {
                return hr;
            }
            if (attach)
            {
                // We need to transition to debug mode after the NativeCodeGenerator is cleared/closed. Since the NativeCodeGenerator will be working on a different thread - it may
                // be checking on the DebuggerState (from ScriptContext) while emitting code.
                SetInDebugMode();
                UpdateNativeCodeGeneratorForDebugMode(this->nativeCodeGen);
            }
        }
        else if (attach)
        {
            SetInDebugMode();
        }

        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
        {
            // Remap all the function entry point thunks.
            this->sourceList->Map([=](uint i, RecyclerWeakReference<Js::Utf8SourceInfo>* sourceInfoWeakRef) {
                Js::Utf8SourceInfo* sourceInfo = sourceInfoWeakRef->Get();
                if (sourceInfo) {
                    sourceInfo->SetInDebugMode(attach);

                    if (!sourceInfo->GetIsLibraryCode())
                    {
                        sourceInfo->MapFunction([](Js::FunctionBody* functionBody) {
                            functionBody->SetEntryToDeferParseForDebugger();
                        });
                    }
                    else
                    {
                        sourceInfo->MapFunction([](Js::FunctionBody* functionBody) {
                            functionBody->ResetEntryPoint();
                        });
                    }
                }

            });
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);

        if (FAILED(hr))
        {
            return hr;
        }

        if (attach)
        {
            this->RegisterDebugThunk();
        }


#if DBG_DUMP || defined(DYNAMIC_PROFILE_STORAGE) || defined(RUNTIME_DATA_COLLECTION)
        // Reset the dynamic profile list
        if (this->profileInfoList)
        {
            this->profileInfoList->Reset();
        }
#endif
        return hr;
    }

    // We use ProfileThunk under debugger.
    void ScriptContext::RegisterDebugThunk(bool calledDuringAttach /*= true*/)
    {
        if (this->IsExceptionWrapperForBuiltInsEnabled())
        {
            this->CurrentThunk = ProfileEntryThunk;
            this->CurrentCrossSiteThunk = CrossSite::ProfileThunk;
            SetProfileModeNativeCodeGen(this->GetNativeCodeGenerator(), TRUE);

            // Set library to profile mode so that for built-ins all new instances of functions
            // are created with entry point set to the ProfileThunk.
            this->javascriptLibrary->SetProfileMode(true);
            this->javascriptLibrary->SetDispatchProfile(true, DispatchProfileInoke);
            if (!calledDuringAttach)
            {
                m_fTraceDomCall = TRUE; // This flag is always needed in DebugMode to wrap external functions with DebugProfileThunk
                // Update the function objects currently present in there.
                this->SetFunctionInRecyclerToProfileMode(true/*enumerateNonUserFunctionsOnly*/);
            }
        }
    }

    void ScriptContext::UnRegisterDebugThunk()
    {
        if (!this->IsProfiling() && this->IsExceptionWrapperForBuiltInsEnabled())
        {
            this->CurrentThunk = DefaultEntryThunk;
            this->CurrentCrossSiteThunk = CrossSite::DefaultThunk;
            SetProfileModeNativeCodeGen(this->GetNativeCodeGenerator(), FALSE);

            if (!this->IsProfiling())
            {
                this->javascriptLibrary->SetProfileMode(false);
                this->javascriptLibrary->SetDispatchProfile(false, DispatchDefaultInvoke);
            }
        }
    }

    HRESULT ScriptContext::RegisterBuiltinFunctions(RegisterExternalLibraryType RegisterExternalLibrary)
    {
        Assert(m_pProfileCallback != NULL);

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::RegisterBuiltinFunctions\n");

        HRESULT hr = S_OK;
        // TODO : create new profiler arena and allocate in it
        if (m_pBuiltinFunctionIdMap == NULL)
        {
            // Anew throws if it OOMs, so the caller into this function needs to handle that exception
            m_pBuiltinFunctionIdMap = Anew(GeneralAllocator(), BuiltinFunctionIdDictionary,
                GeneralAllocator(), 17);
        }

        this->javascriptLibrary->SetProfileMode(TRUE);

        if (FAILED(hr = OnScriptCompiled(BuiltInFunctionsScriptId, PROFILER_SCRIPT_TYPE_NATIVE, NULL)))
        {
            return hr;
        }

        // Register functions directly in global scope
        REG_GLOBAL_LIB_FUNC(eval, GlobalObject::EntryEval);
        REG_GLOBAL_LIB_FUNC(parseInt, GlobalObject::EntryParseInt);
        REG_GLOBAL_LIB_FUNC(parseFloat, GlobalObject::EntryParseFloat);
        REG_GLOBAL_LIB_FUNC(isNaN, GlobalObject::EntryIsNaN);
        REG_GLOBAL_LIB_FUNC(isFinite, GlobalObject::EntryIsFinite);
        REG_GLOBAL_LIB_FUNC(decodeURI, GlobalObject::EntryDecodeURI);
        REG_GLOBAL_LIB_FUNC(decodeURIComponent, GlobalObject::EntryDecodeURIComponent);
        REG_GLOBAL_LIB_FUNC(encodeURI, GlobalObject::EntryEncodeURI);
        REG_GLOBAL_LIB_FUNC(encodeURIComponent, GlobalObject::EntryEncodeURIComponent);
        REG_GLOBAL_LIB_FUNC(escape, GlobalObject::EntryEscape);
        REG_GLOBAL_LIB_FUNC(unescape, GlobalObject::EntryUnEscape);

        if (config.SupportsES3Extensions())
        {
            REG_GLOBAL_LIB_FUNC(ScriptEngine, GlobalObject::EntryScriptEngine);
            REG_GLOBAL_LIB_FUNC(ScriptEngineMajorVersion, GlobalObject::EntryScriptEngineMajorVersion);
            REG_GLOBAL_LIB_FUNC(ScriptEngineMinorVersion, GlobalObject::EntryScriptEngineMinorVersion);
            REG_GLOBAL_LIB_FUNC(ScriptEngineBuildVersion, GlobalObject::EntryScriptEngineBuildVersion);
            REG_GLOBAL_LIB_FUNC(CollectGarbage, GlobalObject::EntryCollectGarbage);
        }

        // Register constructors, prototypes and objects in global
        REGISTER_OBJECT(Object);
        REGISTER_OBJECT(Array);
        REGISTER_OBJECT(Boolean);
        REGISTER_OBJECT(Date);
        REGISTER_OBJECT(Function);
        REGISTER_OBJECT(Math);
        REGISTER_OBJECT(Number);
        REGISTER_OBJECT(String);
        REGISTER_OBJECT(RegExp);
        REGISTER_OBJECT(JSON);
        REGISTER_OBJECT(PixelArray);

        if (config.IsES6MapEnabled())
        {
            REGISTER_OBJECT(Map);
        }

        if (config.IsES6SetEnabled())
        {
            REGISTER_OBJECT(Set);
        }

        if (config.IsES6WeakMapEnabled())
        {
            REGISTER_OBJECT(WeakMap);
        }

        if (config.IsES6WeakSetEnabled())
        {
            REGISTER_OBJECT(WeakSet);
        }

        if (config.IsES6SymbolEnabled())
        {
            REGISTER_OBJECT(Symbol);
        }

        if (config.IsES6IteratorsEnabled())
        {
            REGISTER_OBJECT(ArrayIterator);
            REGISTER_OBJECT(MapIterator);
            REGISTER_OBJECT(SetIterator);
            REGISTER_OBJECT(StringIterator);
            REGISTER_OBJECT(EnumeratorIterator);
        }

        if (config.IsES6TypedArrayExtensionsEnabled())
        {
            REGISTER_OBJECT(TypedArray);
        }

        if (config.IsES6PromiseEnabled())
        {
            REGISTER_OBJECT(Promise);
        }

        if (config.IsES6ProxyEnabled())
        {
            REGISTER_OBJECT(Proxy);
            REGISTER_OBJECT(Reflect);
        }

#ifdef IR_VIEWER
        if (Js::Configuration::Global.flags.IsEnabled(Js::IRViewerFlag))
        {
            REGISTER_OBJECT(IRViewer);
        }
#endif /* IR_VIEWER */

        // Error Constructors and prototypes
        REGISTER_ERROR_OBJECT(Error);
        REGISTER_ERROR_OBJECT(EvalError);
        REGISTER_ERROR_OBJECT(RangeError);
        REGISTER_ERROR_OBJECT(ReferenceError);
        REGISTER_ERROR_OBJECT(SyntaxError);
        REGISTER_ERROR_OBJECT(TypeError);
        REGISTER_ERROR_OBJECT(URIError);

        if (config.IsWinRTEnabled())
        {
            REGISTER_ERROR_OBJECT(WinRTError);
        }

        // External Library
        if (RegisterExternalLibrary != NULL)
        {
            (*RegisterExternalLibrary)(this);
        }

        return hr;
    }

    void ScriptContext::SetFunctionInRecyclerToProfileMode(bool enumerateNonUserFunctionsOnly/* = false*/)
    {
        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::SetFunctionInRecyclerToProfileMode started (m_fTraceDomCall : %s)\n", IsTrueOrFalse(m_fTraceDomCall));

        // Mark this script context isEnumeratingRecyclerObjects
        AutoEnumeratingRecyclerObjects enumeratingRecyclerObjects(this);

        m_enumerateNonUserFunctionsOnly = enumerateNonUserFunctionsOnly;

        this->recycler->EnumerateObjects(JavascriptLibrary::EnumFunctionClass, &ScriptContext::RecyclerEnumClassEnumeratorCallback);

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::SetFunctionInRecyclerToProfileMode ended\n");
    }

    void ScriptContext::UpdateRecyclerFunctionEntryPointsForDebugger()
    {
        // Mark this script context isEnumeratingRecyclerObjects
        AutoEnumeratingRecyclerObjects enumeratingRecyclerObjects(this);

        this->recycler->EnumerateObjects(JavascriptLibrary::EnumFunctionClass, &ScriptContext::RecyclerFunctionCallbackForDebugger);
    }

    /*static*/
    void ScriptContext::RecyclerFunctionCallbackForDebugger(void *address, size_t size)
    {
        JavascriptFunction *pFunction = (JavascriptFunction *)address;

        ScriptContext* scriptContext = pFunction->GetScriptContext();
        if (scriptContext == nullptr || scriptContext->IsClosed())
        {
            // Can't enumerate from closed scriptcontext
            return;
        }

        if (!scriptContext->IsEnumeratingRecyclerObjects())
        {
            return; // function not from enumerating script context
        }

        // Wrapped function are not allocated with the EnumClass bit
        Assert(pFunction->GetFunctionInfo() != &JavascriptExternalFunction::EntryInfo::WrappedFunctionThunk);

        JavascriptMethod entryPoint = pFunction->GetEntryPoint();
        FunctionInfo * info = pFunction->GetFunctionInfo();
        FunctionProxy * proxy = info->GetFunctionProxy();
        if (proxy != info)
        {
            // Not a script function or, the thunk can deal with moving to the function body
            Assert(proxy == null || entryPoint == DefaultDeferredParsingThunk || entryPoint == ProfileDeferredParsingThunk
                || entryPoint == DefaultDeferredDeserializeThunk || entryPoint == ProfileDeferredDeserializeThunk ||
                entryPoint == CrossSite::DefaultThunk || entryPoint == CrossSite::ProfileThunk);

            // Replace entry points for built-ins/external/winrt functions so that we can wrap them with try-catch for "continue after exception".
            if (!pFunction->IsScriptFunction() && IsExceptionWrapperForBuiltInsEnabled(scriptContext))
            {
                if (scriptContext->IsInDebugMode())
                {
                    // We are attaching.
                    // For built-ins, WinRT and DOM functions which are already in recycler, change entry points to route to debug/profile thunk.
                    ScriptContext::SetEntryPointToProfileThunk(pFunction);
                }
                else
                {
                    // We are detaching.
                    // For built-ins, WinRT and DOM functions which are already in recycler, restore entry points to original.
                    if (!scriptContext->IsProfiling())
                    {
                        ScriptContext::RestoreEntryPointFromProfileThunk(pFunction);
                    }
                    // If we are profiling, don't change anything.
                }
            }

            return;
        }

        if (!proxy->IsFunctionBody())
        {
            // REVIEW: why we still have function that is still deferred?
            return;
        }
        Assert(pFunction->IsScriptFunction());

        // Excluding the internal library code, which is not debugg-able already
        if (!proxy->GetUtf8SourceInfo()->GetIsLibraryCode())
        {
            // Reset the constructor cache to default, so that it will not pick up the cached type, created before debugging.
            // Look bug: 301517
            pFunction->ResetConstructorCacheToDefault();
        }

        /*if(ScriptFunctionWithInlineCache::Is(pFunction) && ScriptFunctionWithInlineCache::FromVar(pFunction)->HasInlineCachesFromFunctionBody())
        {
        ScriptFunctionWithInlineCache* pFunctionWithInlineCache = ScriptFunctionWithInlineCache::FromVar(pFunction);
        pFunctionWithInlineCache->SetInlineCachesFromFunctionBody();
        }*/
        if (ScriptFunctionWithInlineCache::Is(pFunction))
        {
            ScriptFunctionWithInlineCache::FromVar(pFunction)->ClearInlineCacheOnFunctionObject();
        }

        /*Assert(entryPoint != DefaultDeferredParsingThunk && entryPoint != ProfileDeferredParsingThunk
        && entryPoint != DefaultDeferredDeserializeThunk && entryPoint != ProfileDeferredDeserializeThunk);*/

        // We should have force parsed the function, and have a function body
        FunctionBody * pBody = proxy->GetFunctionBody();

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (scriptContext->IsInDebugMode())
        {
            if (!(proxy->GetUtf8SourceInfo()->GetIsLibraryCode() || pBody->IsByteCodeDebugMode()))
            {
                // Identifing if any function escaped for not being in debug mode. (This can be removed as a part of TFS : 935011)
                Throw::FatalInternalError();
            }
        }
#endif

        ScriptFunction * scriptFunction = ScriptFunction::FromVar(pFunction);
        JavascriptMethod newEntryPoint;
        if (CrossSite::IsThunk(entryPoint))
        {
            // Can't change from cross-site to non-cross-site, but still need to update the e.p.info on ScriptFunctionType.
            newEntryPoint = entryPoint;
        }
        else
        {
            newEntryPoint = pBody->GetDirectEntryPoint(pBody->GetDefaultFunctionEntryPointInfo());
        }

        scriptFunction->ChangeEntryPoint(pBody->GetDefaultFunctionEntryPointInfo(), newEntryPoint);
    }

    void ScriptContext::RecyclerEnumClassEnumeratorCallback(void *address, size_t size)
    {
        // TODO: we are assuming its function because for now we are enumerating only on functions
        // In future if the RecyclerNewEnumClass is used of Recyclable objects or Dynamic object, we would need a check if it is function
        JavascriptFunction *pFunction = (JavascriptFunction *)address;

        ScriptContext* scriptContext = pFunction->GetScriptContext();
        if (scriptContext == nullptr || scriptContext->IsClosed())
        {
            // Can't enumerate from closed scriptcontext
            return;
        }

        if (!scriptContext->IsEnumeratingRecyclerObjects())
        {
            return; // function not from enumerating script context
        }

        if (!scriptContext->IsTraceDomCall() && (pFunction->IsExternalFunction() || pFunction->IsWinRTFunction()))
        {
            return;
        }

        if (scriptContext->IsEnumerateNonUserFunctionsOnly() && pFunction->IsScriptFunction())
        {
            return;
        }

        // Wrapped function are not allocated with the EnumClass bit
        Assert(pFunction->GetFunctionInfo() != &JavascriptExternalFunction::EntryInfo::WrappedFunctionThunk);

        JavascriptMethod entryPoint = pFunction->GetEntryPoint();
        FunctionProxy *proxy = pFunction->GetFunctionProxy();

        if (proxy != NULL)
        {
#if ENABLE_DEBUG_CONFIG_OPTIONS
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif

            OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::RecyclerEnumClassEnumeratorCallback\n");
            OUTPUT_TRACE(Js::ScriptProfilerPhase, L"\tFunctionProxy : 0x%08X, FunctionNumber : %s, DeferredParseAttributes : %d, EntryPoint : 0x%08X (IsIntermediateCodeGenThunk : %s, isNative : %s)\n",
                (DWORD_PTR)proxy, proxy->GetDebugNumberSet(debugStringBuffer), proxy->GetAttributes(), (DWORD_PTR)entryPoint, IsTrueOrFalse(IsIntermediateCodeGenThunk(entryPoint)), IsTrueOrFalse(scriptContext->IsNativeAddress(entryPoint)));

            FunctionInfo * info = pFunction->GetFunctionInfo();
            if (proxy != info)
            {
                // The thunk can deal with moving to the function body
                Assert(entryPoint == DefaultDeferredParsingThunk || entryPoint == ProfileDeferredParsingThunk
                    || entryPoint == DefaultDeferredDeserializeThunk || entryPoint == ProfileDeferredDeserializeThunk
                    || entryPoint == CrossSite::DefaultThunk || entryPoint == CrossSite::ProfileThunk);

                Assert(!proxy->IsDeferred());
                Assert(proxy->GetFunctionBody()->GetProfileSession() == proxy->GetScriptContext()->GetProfileSession());

                return;
            }


#ifdef ENABLE_NATIVE_CODEGEN
            if (!IsIntermediateCodeGenThunk(entryPoint) && entryPoint != DynamicProfileInfo::EnsureDynamicProfileInfoThunk)
#endif
            {
                OUTPUT_TRACE(Js::ScriptProfilerPhase, L"\t\tJs::ScriptContext::GetProfileModeThunk : 0x%08X\n", (DWORD_PTR)Js::ScriptContext::GetProfileModeThunk(entryPoint));

                ScriptFunction * scriptFunction = ScriptFunction::FromVar(pFunction);
                scriptFunction->ChangeEntryPoint(proxy->GetDefaultEntryPointInfo(), Js::ScriptContext::GetProfileModeThunk(entryPoint));

                OUTPUT_TRACE(Js::ScriptProfilerPhase, L"\tUpdated entrypoint : 0x%08X (isNative : %s)\n", (DWORD_PTR)pFunction->GetEntryPoint(), IsTrueOrFalse(scriptContext->IsNativeAddress(entryPoint)));
            }
        }
        else
        {
            ScriptContext::SetEntryPointToProfileThunk(pFunction);
        }
    }

    // static
    void ScriptContext::SetEntryPointToProfileThunk(JavascriptFunction* function)
    {
        JavascriptMethod entryPoint = function->GetEntryPoint();
        if (entryPoint == Js::CrossSite::DefaultThunk)
        {
            function->SetEntryPoint(Js::CrossSite::ProfileThunk);
        }
        else if (entryPoint != Js::CrossSite::ProfileThunk && entryPoint != ProfileEntryThunk)
        {
            function->SetEntryPoint(ProfileEntryThunk);
        }
    }

    // static
    void ScriptContext::RestoreEntryPointFromProfileThunk(JavascriptFunction* function)
    {
        JavascriptMethod entryPoint = function->GetEntryPoint();
        if (entryPoint == Js::CrossSite::ProfileThunk)
        {
            function->SetEntryPoint(Js::CrossSite::DefaultThunk);
        }
        else if (entryPoint == ProfileEntryThunk)
        {
            function->SetEntryPoint(function->GetFunctionInfo()->GetOriginalEntryPoint());
        }
    }

    ScriptContext *ScriptContext::CopyOnWriteCopy(void *initContext, void(*init)(void *, ScriptContext *))
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();
        SmartFPUControl defaultControl;

        // Create the script context copy.
        ThreadContext *threadContext = GetThreadContext();
        AutoPtr<ScriptContext> fork = ScriptContext::New(threadContext);
        fork->recycler = this->recycler;

        fork->config.CopyFrom(config);

        fork->InitializePreGlobal();
        // Clone the source list
        // This must be done before making a copy of the global object since making a copy-on-write
        // function relies on this being correct.
        // This will be released in the script context's destructor
        // Can't just bindref since the script context's destructor relies on it existing for walking the function list
        fork->sourceList.Root(RecyclerNew(fork->GetRecycler(), SourceList, fork->GetRecycler()), fork->GetRecycler());



        // Keep this alive till the global object has been cloned
        int numSources = this->sourceList->Count();
        Utf8SourceInfo** pinnedSourceInfo = RecyclerNewArrayZ(this->recycler, Js::Utf8SourceInfo*, numSources);

        // We can't share sources any more since they have function bodies in them
        // Instead, we clone the source info without making a copy of the source, so that each
        // script context still has it's own function body collection
        for (int i = 0; i < numSources; i++)
        {
            if (sourceList->IsItemValid(i))
            {
                RecyclerWeakReference<Utf8SourceInfo>* sourceInfoWeakRef = sourceList->Item(i);

                if (sourceInfoWeakRef->Get())
                {
                    Utf8SourceInfo* prevScriptContextSourceInfo = sourceInfoWeakRef->Get();
                    Utf8SourceInfo* newSourceInfo = fork->CloneSourceCrossContext(prevScriptContextSourceInfo);

                    sourceInfoWeakRef = recycler->CreateWeakReferenceHandle(newSourceInfo);
                    pinnedSourceInfo[i] = newSourceInfo;
                }

                fork->sourceList->Add(sourceInfoWeakRef);
            }
        }

        // This must be done early as well because it is also required to make a copy-on-write
        // function.
        fork->diagProbesContainer.Initialize(fork);

        if (init)
            init(initContext, fork);

#if DBG
        fork->isCloningGlobal = true;
#endif

        // Clone the global object including the library.
        GlobalObject *forkedGlobalObject = globalObject->MakeCopyOnWriteObject(fork);
        fork->globalObject = forkedGlobalObject;
        fork->RecordCopyOnWrite(globalObject, forkedGlobalObject);

#if DBG
        fork->isCloningGlobal = false;
#endif

        fork->InitializePostGlobal(true);

        return fork.Detach();
    }

    Var ScriptContext::CopyTrackingValue(Var value, TypeId valueType)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        auto originalTrackingKey = RecyclableObject::FromVar(value);
        auto originalScriptContext = originalTrackingKey->GetScriptContext();
        auto originalLibraryValue = valueType == TypeIds_Undefined ? originalScriptContext->GetLibrary()->GetUndefined() : originalScriptContext->GetLibrary()->GetNull();
        if (originalTrackingKey != originalLibraryValue && originalScriptContext->authoringData && originalScriptContext->authoringData->Callbacks())
        {
            RecyclableObject *thisTrackingKey;

            EnsureCopyOnWriteMap();
            if (this->copyOnWriteMap->TryGetValue(originalTrackingKey, &thisTrackingKey))
                return thisTrackingKey;
            else
            {
                auto originalValue = originalScriptContext->authoringData->Callbacks()->GetTrackingValue(originalScriptContext, originalTrackingKey);
                if (originalValue && !Js::JavascriptOperators::IsUndefinedOrNullType(Js::JavascriptOperators::GetTypeId(originalValue)))
                {
                    auto thisValue = this->CopyOnWrite(originalValue);
                    thisTrackingKey = this->authoringData->Callbacks()->GetTrackingKey(this, thisValue, TypeIds_Undefined);
                    RecordCopyOnWrite(originalTrackingKey, thisTrackingKey);
                    return thisTrackingKey;
                }
            }
        }
        return valueType == TypeIds_Undefined ? GetLibrary()->GetUndefined() : GetLibrary()->GetNull();
    }

    Var ScriptContext::CopyOnWrite(Var value)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();
        auto valueType = JavascriptOperators::GetTypeId(value);
        switch (valueType)
        {
        case TypeIds_UndeclBlockVar:
            return this->GetLibrary()->GetUndeclBlockVar();
        case TypeIds_Enumerator:
        case TypeIds_ExtensionEnumerator:
        case TypeIds_HostDispatch:
            return value;
        case TypeIds_Boolean:
        case TypeIds_Integer:
        case TypeIds_Int64Number:
        case TypeIds_Number:
        case TypeIds_UInt64Number:
        {
            if (TaggedNumber::Is(value))
                return value;

            Js::RecyclableObject *obj = Js::RecyclableObject::FromVar(value);
            if (obj->GetScriptContext() == this)
                return value;

            // Since these are relatively small values with values comparison semantics,
            if (valueType == TypeIds_Boolean)
                return obj->CloneToScriptContext(this);
            else
                return Js::JavascriptNumber::CloneToScriptContext(value, this);
        }

        case TypeIds_Symbol:
        {
            return JavascriptSymbol::FromVar(value)->CloneToScriptContext(this);
        }

        case TypeIds_String:
        {
            JavascriptString *str = JavascriptString::FromVar(value);
            if (str->GetLength() < 16)
            {
                // Only remember non-trivial strings.
                if (str->GetScriptContext() == this)
                    return value;
                // Since these are relatively small values with values comparison semantics,
                return str->CloneToScriptContext(this);
            }

            Js::RecyclableObject *objValue = Js::RecyclableObject::FromVar(value);
            if (objValue->GetScriptContext() == this)
                return value;

            Js::RecyclableObject *copyObj;

            EnsureCopyOnWriteMap();
            if (!this->copyOnWriteMap->TryGetValue(objValue, &copyObj))
            {
                copyObj = objValue->CloneToScriptContext(this);
                RecordCopyOnWrite(objValue, copyObj);
            }
            return copyObj;
        }

        case TypeIds_Null:
            if (BinaryFeatureControl::LanguageService() && this->authoringData && this->authoringData->Callbacks())
                return CopyTrackingValue(value, valueType);
            return GetLibrary()->GetNull();

        case TypeIds_Undefined:
            if (BinaryFeatureControl::LanguageService() && this->authoringData && this->authoringData->Callbacks())
                return CopyTrackingValue(value, valueType);
            return GetLibrary()->GetUndefined();

        default:
        {
            DynamicObject *objValue = DynamicObject::FromVar(value);
            RecyclableObject *copyObj;
            EnsureCopyOnWriteMap();
            if (!this->copyOnWriteMap->TryGetValue(objValue, &copyObj))
            {
                copyObj = objValue->MakeCopyOnWriteObject(this);
                RecordCopyOnWrite(objValue, copyObj);
            }
            return copyObj;
        }

        }
    }

    void ScriptContext::EnsureCopyOnWriteMap()
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        if (!this->copyOnWriteMap)
        {
            ArenaAllocator *alloc = this->GetGuestArena();
            this->copyOnWriteMap = Anew(alloc, InstanceMap, alloc, 1);
        }
    }

    void ScriptContext::RecordCopyOnWrite(RecyclableObject *originalValue, RecyclableObject *copiedValue)
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        EnsureCopyOnWriteMap();
        this->copyOnWriteMap->Item(originalValue, copiedValue);
    }

    void ScriptContext::RecordFunctionClone(ParseableFunctionInfo* originalBody, ParseableFunctionInfo* clonedBody)
    {
        VERIFY_COPY_ON_WRITE_ENABLED();

        RecordCopyOnWrite((RecyclableObject *)originalBody, (RecyclableObject *)clonedBody);
    }

    ParseableFunctionInfo* ScriptContext::CopyFunction(ParseableFunctionInfo* functionInfo)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        if (!functionInfo) return nullptr;

        if (!this->cache->copyOnWriteParseableFunctionInfoMap)
        {
            this->cache->copyOnWriteParseableFunctionInfoMap = RecyclerNew(this->GetRecycler(), ParseableFunctionInfoMap, this->GetRecycler(), 1);
        }

        ParseableFunctionInfo *result;
        if (!this->cache->copyOnWriteParseableFunctionInfoMap->TryGetValue(functionInfo, &result))
        {
            if (functionInfo->IsDeferredParseFunction())
            {
                result = functionInfo->Clone(this);
            }
            else
            {
                result = functionInfo->GetFunctionBody()->Clone(this);
            }

            this->cache->copyOnWriteParseableFunctionInfoMap->Item(functionInfo, result);

            auto scopeInfo = result->GetScopeInfo();
            if (scopeInfo)
            {
                // Ensure the pids in the scopeInfo and parent scopeInfos are tracked.
                scopeInfo->EnsurePidTracking(this);
                auto parent = scopeInfo->GetParent();
                while (parent)
                {
                    if (parent->GetScopeInfo())
                    {
                        parent->GetScopeInfo()->EnsurePidTracking(this);
                        parent = parent->GetScopeInfo()->GetParent();
                    }
                    else
                        break;
                }
            }
        }

        return result;
    }

    UnifiedRegex::RegexPattern *ScriptContext::CopyPattern(UnifiedRegex::RegexPattern *pattern)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        if (!pattern) return nullptr;

        EnsureCopyOnWriteMap();
        UnifiedRegex::RegexPattern *result;

        if (!this->copyOnWriteMap->TryGetValue((Js::RecyclableObject *)pattern, (Js::RecyclableObject **)&result))
        {
            result = pattern->CopyToScriptContext(this);
            copyOnWriteMap->Item((Js::RecyclableObject *)pattern, (Js::RecyclableObject *)result);
        }

        return result;
    }

    Js::Var ScriptContext::GetTrackingValue(Js::RecyclableObject *value)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        if (authoringData && authoringData->Callbacks())
        {
            return authoringData->Callbacks()->GetTrackingValue(this, value);
        }

        return nullptr;
    }

    JavascriptMethod ScriptContext::GetProfileModeThunk(JavascriptMethod entryPoint)
    {
#ifdef ENABLE_NATIVE_CODEGEN
        Assert(!IsIntermediateCodeGenThunk(entryPoint));
#endif
        if (entryPoint == DefaultDeferredParsingThunk || entryPoint == ProfileDeferredParsingThunk)
        {
            return ProfileDeferredParsingThunk;
        }
        if (entryPoint == DefaultDeferredDeserializeThunk || entryPoint == ProfileDeferredDeserializeThunk)
        {
            return ProfileDeferredDeserializeThunk;
        }

        if (CrossSite::IsThunk(entryPoint))
        {
            return CrossSite::ProfileThunk;
        }
        return ProfileEntryThunk;
    }

#if _M_IX86
    __declspec(naked)
        Var ScriptContext::ProfileModeDeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
            // Register functions
            __asm
            {
                push ebp
                    mov ebp, esp
                    lea eax, [esp + 8]
                    push eax
                    call ScriptContext::ProfileModeDeferredParse
                    pop ebp
                    // Although we don't restore ESP here on WinCE, this is fine because script profiler is not shipped for WinCE.
                    jmp eax
            }
    }
#elif defined(_M_X64) || defined(_M_ARM32_OR_ARM64)
    // Do nothing: the implementation of ScriptContext::ProfileModeDeferredParsingThunk is declared (appropriately decorated) in
    // Language\amd64\amd64_Thunks.asm and Language\arm\arm_Thunks.asm and Language\arm64\arm64_Thunks.asm respectively.
#else
    Var ScriptContext::ProfileModeDeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
        Js::Throw::NotImplemented();
        return null;
    }
#endif

    Js::JavascriptMethod ScriptContext::ProfileModeDeferredParse(ScriptFunction ** functionRef)
    {
#if ENABLE_DEBUG_CONFIG_OPTIONS
        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::ProfileModeDeferredParse FunctionNumber : %s, startEntrypoint : 0x%08X\n", (*functionRef)->GetFunctionProxy()->GetDebugNumberSet(debugStringBuffer), (*functionRef)->GetEntryPoint());

        BOOL fParsed = FALSE;
        JavascriptMethod entryPoint = Js::JavascriptFunction::DeferredParseCore(functionRef, fParsed);

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"\t\tIsParsed : %s, updatedEntrypoint : 0x%08X\n", IsTrueOrFalse(fParsed), entryPoint);

        //To get the scriptContext we only need the functionProxy
        FunctionProxy *pRootBody = (*functionRef)->GetFunctionProxy();
        ScriptContext *pScriptContext = pRootBody->GetScriptContext();
        if (pScriptContext->IsProfiling() && !pRootBody->GetFunctionBody()->HasFunctionCompiledSent())
        {
            pScriptContext->RegisterScript(pRootBody, FALSE /*fRegisterScript*/);
        }

        // We can come to this function even though we have stopped profiling.
        Assert(!pScriptContext->IsProfiling() || (*functionRef)->GetFunctionBody()->GetProfileSession() == pScriptContext->GetProfileSession());

        return entryPoint;
    }

#if _M_IX86
    __declspec(naked)
        Var ScriptContext::ProfileModeDeferredDeserializeThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
            // Register functions
            __asm
            {
                push ebp
                    mov ebp, esp
                    push[esp + 8]
                    call ScriptContext::ProfileModeDeferredDeserialize
                    pop ebp
                    // Although we don't restore ESP here on WinCE, this is fine because script profiler is not shipped for WinCE.
                    jmp eax
            }
    }
#elif defined(_M_X64) || defined(_M_ARM32_OR_ARM64)
    // Do nothing: the implementation of ScriptContext::ProfileModeDeferredDeserializeThunk is declared (appropriately decorated) in
    // Language\amd64\amd64_Thunks.asm and Language\arm\arm_Thunks.asm respectively.
#else
    Var ScriptContext::ProfileModeDeferredDeserializeThunk(RecyclableObject* function, CallInfo callInfo, ...)
    {
        Js::Throw::NotImplemented();
        return null;
    }
#endif

    Js::JavascriptMethod ScriptContext::ProfileModeDeferredDeserialize(ScriptFunction *function)
    {
#if ENABLE_DEBUG_CONFIG_OPTIONS
        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::ProfileModeDeferredDeserialize FunctionNumber : %s\n", function->GetFunctionProxy()->GetDebugNumberSet(debugStringBuffer));

        JavascriptMethod entryPoint = Js::JavascriptFunction::DeferredDeserialize(function);

        //To get the scriptContext; we only need the functionproxy
        FunctionProxy *pRootBody = function->GetFunctionProxy();
        ScriptContext *pScriptContext = pRootBody->GetScriptContext();
        if (pScriptContext->IsProfiling() && !pRootBody->GetFunctionBody()->HasFunctionCompiledSent())
        {
            pScriptContext->RegisterScript(pRootBody, FALSE /*fRegisterScript*/);
        }

        // We can come to this function even though we have stopped profiling.
        Assert(!pScriptContext->IsProfiling() || function->GetFunctionBody()->GetProfileSession() == pScriptContext->GetProfileSession());

        return entryPoint;
    }

    BOOL ScriptContext::GetProfileInfo(
        JavascriptFunction* function,
        PROFILER_TOKEN &scriptId,
        PROFILER_TOKEN &functionId)
    {
        BOOL fCanProfile = (m_pProfileCallback != nullptr && m_fTraceFunctionCall);
        if (!fCanProfile)
        {
            return FALSE;
        }

        Js::FunctionInfo* functionInfo = function->GetFunctionInfo();
        if (functionInfo->GetAttributes() & FunctionInfo::DoNotProfile)
        {
            return FALSE;
        }

        Js::FunctionBody * functionBody = functionInfo->GetFunctionBody();
        if (functionBody == nullptr)
        {
            functionId = GetFunctionNumber(functionInfo->GetOriginalEntryPoint());
            if (functionId == -1)
            {
                // Dom Call
                return m_fTraceDomCall && (m_pProfileCallback2 != nullptr);
            }
            else
            {
                // Builtin function
                scriptId = BuiltInFunctionsScriptId;
                return m_fTraceNativeFunctionCall;
            }
        }
        else if (!functionBody->GetUtf8SourceInfo()->GetIsLibraryCode() || functionBody->IsPublicLibraryCode()) // user script or public library code
        {
            scriptId = (PROFILER_TOKEN)functionBody->GetUtf8SourceInfo()->GetSourceInfoId();
            functionId = functionBody->GetFunctionNumber();
            return TRUE;
        }

        return FALSE;
    }

    bool ScriptContext::IsForceNoNative()
    {
        bool forceNoNative = false;
        if (this->GetDebuggerMode() != Js::DebuggerMode::NotDebugging)
        {
            forceNoNative = this->IsInterpreted();
        }
        else if (BinaryFeatureControl::LanguageService() || !Js::Configuration::Global.EnableJitInDebugMode())
        {
            forceNoNative = true;
            this->ForceNoNative();
        }
        return forceNoNative;
    }

    void ScriptContext::InitializeDebugging(DbgRegisterFunctionType dbgRegisterFunction)
    {
        if (!this->IsInDebugMode()) // If we already in debug mode, we would have done below changes already.
        {
            this->SetInDebugMode();
            this->SetDbgRegisterFunction(dbgRegisterFunction);
            if (this->IsInDebugMode())
            {
                // Note: for this we need final IsInDebugMode and NativeCodeGen initialized,
                //       and inside EnsureScriptContext, which seems appropriate as well,
                //       it's too early as pdm is not registered, thus IsDebuggerEnvironmentAvailable is false.
                this->RegisterDebugThunk(false/*calledDuringAttach*/);

                // TODO: for launch scenario for external and WinRT functions it might be too late to register debug thunk here,
                //       as we need the thunk registered before FunctionInfo's for built-ins, that may throw, are created.
                //       Need to verify. If that's the case, one way would be to enumerate and fix all external/winRT thunks here.
            }
        }
        Assert(this->dbgRegisterFunction == dbgRegisterFunction);
    }

    // Combined profile/debug wrapper thunk.
    // - used when we profile to send profile events
    // - used when we debug, only used for built-in functions
    // - used when we profile and debug
    Var ScriptContext::DebugProfileProbeThunk(RecyclableObject* callable, CallInfo callInfo, ...)
    {
        RUNTIME_ARGUMENTS(args, callInfo);

        JavascriptFunction* function = JavascriptFunction::FromVar(callable);
        ScriptContext* scriptContext = function->GetScriptContext();
        PROFILER_TOKEN scriptId = -1;
        PROFILER_TOKEN functionId = -1;
        bool functionEnterEventSent = false;

        const bool isProfilingUserCode = scriptContext->GetThreadContext()->IsProfilingUserCode();
        const bool isUserCode = !function->IsLibraryCode();
        wchar_t *pwszExtractedFunctionName = NULL;
        const wchar_t *pwszFunctionName = NULL;
        HRESULT hrOfEnterEvent = S_OK;

        // We can come here when profiling is not on
        // eg. User starts profiling, we update all thinks and then stop profiling - we dont update thunk
        // So we still get this call
        const bool fProfile = (isUserCode || isProfilingUserCode) // Only report user code or entry library code
            && scriptContext->GetProfileInfo(function, scriptId, functionId);

        if (fProfile)
        {
            Js::FunctionBody *pBody = function->GetFunctionBody();
            if (pBody != nullptr && !pBody->HasFunctionCompiledSent())
            {
                pBody->RegisterFunction(false/*changeThunk*/);
            }

#if DEBUG
            { // scope

                Assert(scriptContext->IsProfiling());

                if (pBody && pBody->GetProfileSession() != pBody->GetScriptContext()->GetProfileSession())
                {
                    wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
                    OUTPUT_TRACE_DEBUGONLY(Js::ScriptProfilerPhase, L"ScriptContext::ProfileProbeThunk, ProfileSession does not match (%d != %d), functionNumber : %s, functionName : %s\n",
                        pBody->GetProfileSession(), pBody->GetScriptContext()->GetProfileSession(), pBody->GetDebugNumberSet(debugStringBuffer), pBody->GetDisplayName());
                }
                AssertMsg(pBody == NULL || pBody->GetProfileSession() == pBody->GetScriptContext()->GetProfileSession(), "Function info wasnt reported for this profile session");
            }
#endif

            if (functionId == -1)
            {
                Var sourceString = function->GetSourceString();

                // SourceString will be null for the Js::BoundFunction, don't throw Enter/Exit notification in that case.
                if (sourceString != NULL)
                {
                    if (TaggedInt::Is(sourceString))
                    {
                        PropertyId nameId = TaggedInt::ToInt32(sourceString);
                        pwszFunctionName = scriptContext->GetPropertyString(nameId)->GetSz();
                    }
                    else
                    {
                        // it is string because user had called in toString extract name from it
                        Assert(JavascriptString::Is(sourceString));
                        const wchar_t *pwszToString = ((JavascriptString *)sourceString)->GetSz();
                        const wchar_t *pwszNameStart = wcsstr(pwszToString, L" ");
                        const wchar_t *pwszNameEnd = wcsstr(pwszToString, L"(");
                        if (pwszNameStart == nullptr || pwszNameEnd == nullptr || ((int)(pwszNameEnd - pwszNameStart) <= 0))
                        {
                            int len = ((JavascriptString *)sourceString)->GetLength() + 1;
                            pwszExtractedFunctionName = new wchar_t[len];
                            wcsncpy_s(pwszExtractedFunctionName, len, pwszToString, _TRUNCATE);
                        }
                        else
                        {
                            int len = (int)(pwszNameEnd - pwszNameStart);
                            AssertMsg(len > 0, "Allocating array with zero or negative length?");
                            pwszExtractedFunctionName = new wchar_t[len];
                            wcsncpy_s(pwszExtractedFunctionName, len, pwszNameStart + 1, _TRUNCATE);
                        }
                        pwszFunctionName = pwszExtractedFunctionName;
                    }

                    functionEnterEventSent = true;
                    Assert(pwszFunctionName != NULL);
                    hrOfEnterEvent = scriptContext->OnDispatchFunctionEnter(pwszFunctionName);
                }
            }
            else
            {
                hrOfEnterEvent = scriptContext->OnFunctionEnter(scriptId, functionId);
            }

            scriptContext->GetThreadContext()->SetIsProfilingUserCode(isUserCode); // Update IsProfilingUserCode state
        }

        Var aReturn = NULL;
        JavascriptMethod origEntryPoint = function->GetFunctionInfo()->GetOriginalEntryPoint();

        __try
        {
            Assert(!function->IsScriptFunction() || function->GetFunctionProxy());

            // No need to wrap script functions, also can't if the wrapper is already on the stack.
            // Treat "library code" script functions, such as Intl, as built-ins:
            // use the wrapper when calling them, and do not reset the wrapper when calling them.
            bool isDebugWrapperEnabled = scriptContext->IsInDebugMode() && IsExceptionWrapperForBuiltInsEnabled(scriptContext);
            bool useDebugWrapper =
                isDebugWrapperEnabled &&
                function->IsLibraryCode() &&
                !AutoRegisterIgnoreExceptionWrapper::IsRegistered(scriptContext->GetThreadContext());

            OUTPUT_VERBOSE_TRACE(Js::DebuggerPhase, L"DebugProfileProbeThunk: calling function: %s isWrapperRegistered=%d useDebugWrapper=%d\n",
                function->GetFunctionInfo()->HasBody() ? function->GetFunctionBody()->GetDisplayName() : L"built-in/library", AutoRegisterIgnoreExceptionWrapper::IsRegistered(scriptContext->GetThreadContext()), useDebugWrapper);

            if (scriptContext->IsInDebugMode())
            {
                scriptContext->diagProbesContainer.StartRecordingCall();
            }

            if (useDebugWrapper)
            {
                // For native use wrapper and bail out on to ignore exception.
                // Extract try-catch out of hot path in normal profile mode (presense of try-catch in a function is bad for perf).
                aReturn = ProfileModeThunk_DebugModeWrapper(function, scriptContext, origEntryPoint, args);
            }
            else
            {
                if (isDebugWrapperEnabled && !function->IsLibraryCode())
                {
                    // We want to ignore exception and continue into closest user/script function down on the stack.
                    // Thus, if needed, reset the wrapper for the time of this call,
                    // so that if there is library/helper call after script function, it will use try-catch.
                    // Can't use smart/destructor object here because of __try__finally.
                    ThreadContext* threadContext = scriptContext->GetThreadContext();
                    bool isOrigWrapperPresent = threadContext->GetDebuggingFlags()->IsBuiltInWrapperPresent();
                    if (isOrigWrapperPresent)
                    {
                        threadContext->GetDebuggingFlags()->SetIsBuiltInWrapperPresent(false);
                    }
                    __try
                    {
                        aReturn = JavascriptFunction::CallFunction<true>(function, origEntryPoint, args);
                    }
                    __finally
                    {
                        threadContext->GetDebuggingFlags()->SetIsBuiltInWrapperPresent(isOrigWrapperPresent);
                    }
                }
                else
                {
                    // Can we update return address to a thunk that sends Exit event and then jmp to entry instead of Calling it.
                    // Saves stack space and it might be something we would be doing anyway for handling profile.Start/stop
                    // which can come anywhere on the stack.
                    aReturn = JavascriptFunction::CallFunction<true>(function, origEntryPoint, args);
                }
            }
        }
        __finally
        {
            if (fProfile)
            {
                if (hrOfEnterEvent != ACTIVPROF_E_PROFILER_ABSENT)
                {
                    if (functionId == -1)
                    {
                        // Check whether we have sent the Enter event or not.
                        if (functionEnterEventSent)
                        {
                            scriptContext->OnDispatchFunctionExit(pwszFunctionName);
                            if (pwszExtractedFunctionName != NULL)
                            {
                                delete[]pwszExtractedFunctionName;
                            }
                        }
                    }
                    else
                    {
                        scriptContext->OnFunctionExit(scriptId, functionId);
                    }
                }

                scriptContext->GetThreadContext()->SetIsProfilingUserCode(isProfilingUserCode); // Restore IsProfilingUserCode state
            }

            if (scriptContext->IsInDebugMode())
            {
                scriptContext->diagProbesContainer.EndRecordingCall(aReturn, function);
            }
        }

        return aReturn;
    }

    // Part of ProfileModeThunk which is called in debug mode (debug or debug & profile).
    Var ScriptContext::ProfileModeThunk_DebugModeWrapper(JavascriptFunction* function, ScriptContext* scriptContext, JavascriptMethod entryPoint, Arguments& args)
    {
        AutoRegisterIgnoreExceptionWrapper autoWrapper(scriptContext->GetThreadContext());

        Var aReturn = HelperOrLibraryMethodWrapper<true>(scriptContext, [=] {
            return JavascriptFunction::CallFunction<true>(function, entryPoint, args);
        });

        return aReturn;
    }

    HRESULT ScriptContext::OnScriptCompiled(PROFILER_TOKEN scriptId, PROFILER_SCRIPT_TYPE type, IUnknown *pIDebugDocumentContext)
    {
        // TODO : can we do a delay send of these events or can we send a event before doing all this stuff that could calculate overhead?
        Assert(m_pProfileCallback != NULL);

        OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::OnScriptCompiled scriptId : %d, ScriptType : %d\n", scriptId, type);

        HRESULT hr = S_OK;

        if ((type == PROFILER_SCRIPT_TYPE_NATIVE && m_fTraceNativeFunctionCall) ||
            (type != PROFILER_SCRIPT_TYPE_NATIVE && m_fTraceFunctionCall))
        {
            m_inProfileCallback = TRUE;
            hr = m_pProfileCallback->ScriptCompiled(scriptId, type, pIDebugDocumentContext);
            m_inProfileCallback = FALSE;
        }
        return hr;
    }

    HRESULT ScriptContext::OnFunctionCompiled(
        PROFILER_TOKEN functionId,
        PROFILER_TOKEN scriptId,
        const WCHAR *pwszFunctionName,
        const WCHAR *pwszFunctionNameHint,
        IUnknown *pIDebugDocumentContext)
    {
        Assert(m_pProfileCallback != NULL);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (scriptId != BuiltInFunctionsScriptId || Js::Configuration::Global.flags.Verbose)
        {
            OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::OnFunctionCompiled scriptId : %d, functionId : %d, FunctionName : %s, FunctionNameHint : %s\n", scriptId, functionId, pwszFunctionName, pwszFunctionNameHint);
        }
#endif

        HRESULT hr = S_OK;

        if ((scriptId == BuiltInFunctionsScriptId && m_fTraceNativeFunctionCall) ||
            (scriptId != BuiltInFunctionsScriptId && m_fTraceFunctionCall))
        {
            m_inProfileCallback = TRUE;
            hr = m_pProfileCallback->FunctionCompiled(functionId, scriptId, pwszFunctionName, pwszFunctionNameHint, pIDebugDocumentContext);
            m_inProfileCallback = FALSE;
        }
        return hr;
    }

    HRESULT ScriptContext::OnFunctionEnter(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId)
    {
        if (m_pProfileCallback == NULL)
        {
            return ACTIVPROF_E_PROFILER_ABSENT;
        }

        OUTPUT_VERBOSE_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::OnFunctionEnter scriptId : %d, functionId : %d\n", scriptId, functionId);

        HRESULT hr = S_OK;

        if ((scriptId == BuiltInFunctionsScriptId && m_fTraceNativeFunctionCall) ||
            (scriptId != BuiltInFunctionsScriptId && m_fTraceFunctionCall))
        {
            m_inProfileCallback = TRUE;
            hr = m_pProfileCallback->OnFunctionEnter(scriptId, functionId);
            m_inProfileCallback = FALSE;
        }
        return hr;
    }

    HRESULT ScriptContext::OnFunctionExit(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId)
    {
        if (m_pProfileCallback == NULL)
        {
            return ACTIVPROF_E_PROFILER_ABSENT;
        }

        OUTPUT_VERBOSE_TRACE(Js::ScriptProfilerPhase, L"ScriptContext::OnFunctionExit scriptId : %d, functionId : %d\n", scriptId, functionId);

        HRESULT hr = S_OK;

        if ((scriptId == BuiltInFunctionsScriptId && m_fTraceNativeFunctionCall) ||
            (scriptId != BuiltInFunctionsScriptId && m_fTraceFunctionCall))
        {
            m_inProfileCallback = TRUE;
            hr = m_pProfileCallback->OnFunctionExit(scriptId, functionId);
            m_inProfileCallback = FALSE;
        }
        return hr;
    }

    HRESULT ScriptContext::FunctionExitSenderThunk(PROFILER_TOKEN functionId, PROFILER_TOKEN scriptId, ScriptContext *pScriptContext)
    {
        return pScriptContext->OnFunctionExit(scriptId, functionId);
    }

    HRESULT ScriptContext::FunctionExitByNameSenderThunk(const wchar_t *pwszFunctionName, ScriptContext *pScriptContext)
    {
        return pScriptContext->OnDispatchFunctionExit(pwszFunctionName);
    }

    Js::PropertyId ScriptContext::GetFunctionNumber(JavascriptMethod entryPoint)
    {
        return (m_pBuiltinFunctionIdMap == NULL) ? -1 : m_pBuiltinFunctionIdMap->Lookup(entryPoint, -1);
    }

    HRESULT ScriptContext::RegisterLibraryFunction(const wchar_t *pwszObjectName, const wchar_t *pwszFunctionName, Js::PropertyId functionPropertyId, JavascriptMethod entryPoint)
    {
#if DEBUG
        const wchar_t *pwszObjectNameFromProperty = const_cast<wchar_t *>(GetPropertyName(functionPropertyId)->GetBuffer());
        if (GetPropertyName(functionPropertyId)->IsSymbol())
        {
            // The spec names functions whose property is a well known symbol as the description from the symbol
            // wrapped in square brackets, so verify by skipping past first bracket
            Assert(!wcsncmp(pwszFunctionName + 1, pwszObjectNameFromProperty, wcslen(pwszObjectNameFromProperty)));
            Assert(wcslen(pwszFunctionName) == wcslen(pwszObjectNameFromProperty) + 2);
        }
        else
        {
            Assert(!wcscmp(pwszFunctionName, pwszObjectNameFromProperty));
        }
        Assert(m_pBuiltinFunctionIdMap != NULL);
#endif

        // Create the propertyId as object.functionName if it is not global function
        // the global functions would be recognized by just functionName
        // eg. with functionName, toString, depending on objectName, it could be Object.toString, or Date.toString
        wchar_t szTempName[70];
        if (pwszObjectName != NULL)
        {
            // Create name as "object.function"
            swprintf_s(szTempName, 70, L"%s.%s", pwszObjectName, pwszFunctionName);
            functionPropertyId = GetOrAddPropertyIdTracked(szTempName, wcslen(szTempName));
        }

        Js::PropertyId cachedFunctionId;
        bool keyFound = m_pBuiltinFunctionIdMap->TryGetValue(entryPoint, &cachedFunctionId);

        if (keyFound)
        {
            // Entry point is already in the map
            if (cachedFunctionId != functionPropertyId)
            {
                // This is the scenario where we could be using same function for multiple builtin functions
                // eg. Error.toString, WinRTError.toString etc.
                // We would ignore these extra entrypoints because while profiling, identifying which object's toString is too costly for its worth
                return S_OK;
            }

            // else is the scenario where map was created by earlier profiling session and we are yet to send function compiled for this session
        }
        else
        {
#if DBG
            m_pBuiltinFunctionIdMap->MapUntil([&](JavascriptMethod, Js::PropertyId propertyId) -> bool
            {
                if (functionPropertyId == propertyId)
                {
                    Assert(false);
                    return true;
                }
                return false;
            });
#endif

            // throws, this must always be in a function that handles OOM
            m_pBuiltinFunctionIdMap->Add(entryPoint, functionPropertyId);
        }

        // Use name with "Object." if its not a global function
        if (pwszObjectName != NULL)
        {
            return OnFunctionCompiled(functionPropertyId, BuiltInFunctionsScriptId, szTempName, NULL, NULL);
        }
        else
        {
            return OnFunctionCompiled(functionPropertyId, BuiltInFunctionsScriptId, pwszFunctionName, NULL, NULL);
        }
    }

    HRESULT ScriptContext::RegisterObject()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Object);

        DEFINE_OBJECT_NAME(Object);

        REG_OBJECTS_LIB_FUNC(defineProperty, JavascriptObject::EntryDefineProperty);
        REG_OBJECTS_LIB_FUNC(getOwnPropertyDescriptor, JavascriptObject::EntryGetOwnPropertyDescriptor);

        REG_OBJECTS_LIB_FUNC(defineProperties, JavascriptObject::EntryDefineProperties);
        REG_OBJECTS_LIB_FUNC(create, JavascriptObject::EntryCreate);
        REG_OBJECTS_LIB_FUNC(seal, JavascriptObject::EntrySeal);
        REG_OBJECTS_LIB_FUNC(freeze, JavascriptObject::EntryFreeze);
        REG_OBJECTS_LIB_FUNC(preventExtensions, JavascriptObject::EntryPreventExtensions);
        REG_OBJECTS_LIB_FUNC(isSealed, JavascriptObject::EntryIsSealed);
        REG_OBJECTS_LIB_FUNC(isFrozen, JavascriptObject::EntryIsFrozen);
        REG_OBJECTS_LIB_FUNC(isExtensible, JavascriptObject::EntryIsExtensible);

        REG_OBJECTS_LIB_FUNC(getPrototypeOf, JavascriptObject::EntryGetPrototypeOf);
        REG_OBJECTS_LIB_FUNC(keys, JavascriptObject::EntryKeys);
        REG_OBJECTS_LIB_FUNC(getOwnPropertyNames, JavascriptObject::EntryGetOwnPropertyNames);

        REG_OBJECTS_LIB_FUNC(setPrototypeOf, JavascriptObject::EntrySetPrototypeOf);

        if (config.IsES6SymbolEnabled())
        {
            REG_OBJECTS_LIB_FUNC(getOwnPropertySymbols, JavascriptObject::EntryGetOwnPropertySymbols);
        }

        REG_OBJECTS_LIB_FUNC(hasOwnProperty, JavascriptObject::EntryHasOwnProperty);
        REG_OBJECTS_LIB_FUNC(propertyIsEnumerable, JavascriptObject::EntryPropertyIsEnumerable);
        REG_OBJECTS_LIB_FUNC(isPrototypeOf, JavascriptObject::EntryIsPrototypeOf);
        REG_OBJECTS_LIB_FUNC(toLocaleString, JavascriptObject::EntryToLocaleString);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptObject::EntryToString);
        REG_OBJECTS_LIB_FUNC(valueOf, JavascriptObject::EntryValueOf);

        if (config.IsDefineGetterSetterEnabled())
        {
            REG_OBJECTS_LIB_FUNC(__defineGetter__, JavascriptObject::EntryDefineGetter);
            REG_OBJECTS_LIB_FUNC(__defineSetter__, JavascriptObject::EntryDefineSetter);
        }

        if (config.IsES6ObjectExtensionsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(is, JavascriptObject::EntryIs);
            REG_OBJECTS_LIB_FUNC(assign, JavascriptObject::EntryAssign);
        }

        return hr;
    }

    HRESULT ScriptContext::RegisterArray()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Array);

        DEFINE_OBJECT_NAME(Array);

        REG_OBJECTS_LIB_FUNC(isArray, JavascriptArray::EntryIsArray);
        REG_OBJECTS_LIB_FUNC(concat, JavascriptArray::EntryConcat);
        REG_OBJECTS_LIB_FUNC(join, JavascriptArray::EntryJoin);
        REG_OBJECTS_LIB_FUNC(pop, JavascriptArray::EntryPop);
        REG_OBJECTS_LIB_FUNC(push, JavascriptArray::EntryPush);
        REG_OBJECTS_LIB_FUNC(reverse, JavascriptArray::EntryReverse);
        REG_OBJECTS_LIB_FUNC(shift, JavascriptArray::EntryShift);
        REG_OBJECTS_LIB_FUNC(slice, JavascriptArray::EntrySlice);
        REG_OBJECTS_LIB_FUNC(sort, JavascriptArray::EntrySort);
        REG_OBJECTS_LIB_FUNC(splice, JavascriptArray::EntrySplice);
        REG_OBJECTS_LIB_FUNC(toLocaleString, JavascriptArray::EntryToLocaleString);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptArray::EntryToString);
        REG_OBJECTS_LIB_FUNC(unshift, JavascriptArray::EntryUnshift);
        REG_OBJECTS_LIB_FUNC(indexOf, JavascriptArray::EntryIndexOf);
        REG_OBJECTS_LIB_FUNC(every, JavascriptArray::EntryEvery);
        REG_OBJECTS_LIB_FUNC(filter, JavascriptArray::EntryFilter);
        REG_OBJECTS_LIB_FUNC(forEach, JavascriptArray::EntryForEach);
        REG_OBJECTS_LIB_FUNC(lastIndexOf, JavascriptArray::EntryLastIndexOf);
        REG_OBJECTS_LIB_FUNC(map, JavascriptArray::EntryMap);
        REG_OBJECTS_LIB_FUNC(reduce, JavascriptArray::EntryReduce);
        REG_OBJECTS_LIB_FUNC(reduceRight, JavascriptArray::EntryReduceRight);
        REG_OBJECTS_LIB_FUNC(some, JavascriptArray::EntrySome);

        if (config.IsES6StringExtensionsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(find, JavascriptArray::EntryFind);
            REG_OBJECTS_LIB_FUNC(findIndex, JavascriptArray::EntryFindIndex);
        }

        if (config.IsES6IteratorsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(entries, JavascriptArray::EntryEntries)
            REG_OBJECTS_LIB_FUNC(keys, JavascriptArray::EntryKeys)
            REG_OBJECTS_LIB_FUNC(values, JavascriptArray::EntryValues)
            // _symbolIterator is just an alias for values on Array.prototype so do not register it as its own function
        }

        if (config.IsES6TypedArrayExtensionsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(fill, JavascriptArray::EntryFill)
            REG_OBJECTS_LIB_FUNC(copyWithin, JavascriptArray::EntryCopyWithin)
            REG_OBJECTS_LIB_FUNC(from, JavascriptArray::EntryFrom);
            REG_OBJECTS_LIB_FUNC(of, JavascriptArray::EntryOf);
        }

        return hr;
    }

    HRESULT ScriptContext::RegisterBoolean()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Boolean);

        DEFINE_OBJECT_NAME(Boolean);

        REG_OBJECTS_LIB_FUNC(valueOf, JavascriptBoolean::EntryValueOf);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptBoolean::EntryToString);

        return hr;
    }

    HRESULT ScriptContext::RegisterDate()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Date);

        DEFINE_OBJECT_NAME(Date);

        REG_OBJECTS_LIB_FUNC(parse, JavascriptDate::EntryParse);
        REG_OBJECTS_LIB_FUNC(now, JavascriptDate::EntryNow);
        REG_OBJECTS_LIB_FUNC(UTC, JavascriptDate::EntryUTC);

        REG_OBJECTS_LIB_FUNC(getDate, JavascriptDate::EntryGetDate);
        REG_OBJECTS_LIB_FUNC(getDay, JavascriptDate::EntryGetDay);
        REG_OBJECTS_LIB_FUNC(getFullYear, JavascriptDate::EntryGetFullYear);
        REG_OBJECTS_LIB_FUNC(getHours, JavascriptDate::EntryGetHours);
        REG_OBJECTS_LIB_FUNC(getMilliseconds, JavascriptDate::EntryGetMilliseconds);
        REG_OBJECTS_LIB_FUNC(getMinutes, JavascriptDate::EntryGetMinutes);
        REG_OBJECTS_LIB_FUNC(getMonth, JavascriptDate::EntryGetMonth);
        REG_OBJECTS_LIB_FUNC(getSeconds, JavascriptDate::EntryGetSeconds);
        REG_OBJECTS_LIB_FUNC(getTime, JavascriptDate::EntryGetTime);
        REG_OBJECTS_LIB_FUNC(getTimezoneOffset, JavascriptDate::EntryGetTimezoneOffset);
        REG_OBJECTS_LIB_FUNC(getUTCDate, JavascriptDate::EntryGetUTCDate);
        REG_OBJECTS_LIB_FUNC(getUTCDay, JavascriptDate::EntryGetUTCDay);
        REG_OBJECTS_LIB_FUNC(getUTCFullYear, JavascriptDate::EntryGetUTCFullYear);
        REG_OBJECTS_LIB_FUNC(getUTCHours, JavascriptDate::EntryGetUTCHours);
        REG_OBJECTS_LIB_FUNC(getUTCMilliseconds, JavascriptDate::EntryGetUTCMilliseconds);
        REG_OBJECTS_LIB_FUNC(getUTCMinutes, JavascriptDate::EntryGetUTCMinutes);
        REG_OBJECTS_LIB_FUNC(getUTCMonth, JavascriptDate::EntryGetUTCMonth);
        REG_OBJECTS_LIB_FUNC(getUTCSeconds, JavascriptDate::EntryGetUTCSeconds);
        if (config.SupportsES3Extensions() && config.GetHostType() != HostTypeApplication)
        {
            REG_OBJECTS_LIB_FUNC(getVarDate, JavascriptDate::EntryGetVarDate);
        }
        REG_OBJECTS_LIB_FUNC(getYear, JavascriptDate::EntryGetYear);
        REG_OBJECTS_LIB_FUNC(setDate, JavascriptDate::EntrySetDate);
        REG_OBJECTS_LIB_FUNC(setFullYear, JavascriptDate::EntrySetFullYear);
        REG_OBJECTS_LIB_FUNC(setHours, JavascriptDate::EntrySetHours);
        REG_OBJECTS_LIB_FUNC(setMilliseconds, JavascriptDate::EntrySetMilliseconds);
        REG_OBJECTS_LIB_FUNC(setMinutes, JavascriptDate::EntrySetMinutes);
        REG_OBJECTS_LIB_FUNC(setMonth, JavascriptDate::EntrySetMonth);
        REG_OBJECTS_LIB_FUNC(setSeconds, JavascriptDate::EntrySetSeconds);
        REG_OBJECTS_LIB_FUNC(setTime, JavascriptDate::EntrySetTime);
        REG_OBJECTS_LIB_FUNC(setUTCDate, JavascriptDate::EntrySetUTCDate);
        REG_OBJECTS_LIB_FUNC(setUTCFullYear, JavascriptDate::EntrySetUTCFullYear);
        REG_OBJECTS_LIB_FUNC(setUTCHours, JavascriptDate::EntrySetUTCHours);
        REG_OBJECTS_LIB_FUNC(setUTCMilliseconds, JavascriptDate::EntrySetUTCMilliseconds);
        REG_OBJECTS_LIB_FUNC(setUTCMinutes, JavascriptDate::EntrySetUTCMinutes);
        REG_OBJECTS_LIB_FUNC(setUTCMonth, JavascriptDate::EntrySetUTCMonth);
        REG_OBJECTS_LIB_FUNC(setUTCSeconds, JavascriptDate::EntrySetUTCSeconds);
        REG_OBJECTS_LIB_FUNC(setYear, JavascriptDate::EntrySetYear);
        REG_OBJECTS_LIB_FUNC(toDateString, JavascriptDate::EntryToDateString);
        REG_OBJECTS_LIB_FUNC(toISOString, JavascriptDate::EntryToISOString);
        REG_OBJECTS_LIB_FUNC(toJSON, JavascriptDate::EntryToJSON);
        REG_OBJECTS_LIB_FUNC(toLocaleDateString, JavascriptDate::EntryToLocaleDateString);
        REG_OBJECTS_LIB_FUNC(toLocaleString, JavascriptDate::EntryToLocaleString);
        REG_OBJECTS_LIB_FUNC(toLocaleTimeString, JavascriptDate::EntryToLocaleTimeString);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptDate::EntryToString);
        REG_OBJECTS_LIB_FUNC(toTimeString, JavascriptDate::EntryToTimeString);
        REG_OBJECTS_LIB_FUNC(toUTCString, JavascriptDate::EntryToUTCString);
        REG_OBJECTS_LIB_FUNC(toGMTString, JavascriptDate::EntryToGMTString);
        REG_OBJECTS_LIB_FUNC(valueOf, JavascriptDate::EntryValueOf);

        return hr;
    }

    HRESULT ScriptContext::RegisterFunction()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Function);

        DEFINE_OBJECT_NAME(Function);

        REG_OBJECTS_LIB_FUNC(apply, JavascriptFunction::EntryApply);
        REG_OBJECTS_LIB_FUNC(bind, JavascriptFunction::EntryBind);
        REG_OBJECTS_LIB_FUNC(call, JavascriptFunction::EntryCall);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptFunction::EntryToString);

        if (config.IsES6ClassAndExtendsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(toMethod, JavascriptFunction::EntryToMethod);
        }

        return hr;
    }

    HRESULT ScriptContext::RegisterMath()
    {
        HRESULT hr = S_OK;

        DEFINE_OBJECT_NAME(Math);

        REG_OBJECTS_LIB_FUNC(abs, Math::Abs);
        REG_OBJECTS_LIB_FUNC(acos, Math::Acos);
        REG_OBJECTS_LIB_FUNC(asin, Math::Asin);
        REG_OBJECTS_LIB_FUNC(atan, Math::Atan);
        REG_OBJECTS_LIB_FUNC(atan2, Math::Atan2);
        REG_OBJECTS_LIB_FUNC(ceil, Math::Ceil);
        REG_OBJECTS_LIB_FUNC(cos, Math::Cos);
        REG_OBJECTS_LIB_FUNC(exp, Math::Exp);
        REG_OBJECTS_LIB_FUNC(floor, Math::Floor);
        REG_OBJECTS_LIB_FUNC(log, Math::Log);
        REG_OBJECTS_LIB_FUNC(max, Math::Max);
        REG_OBJECTS_LIB_FUNC(min, Math::Min);
        REG_OBJECTS_LIB_FUNC(pow, Math::Pow);
        REG_OBJECTS_LIB_FUNC(random, Math::Random);
        REG_OBJECTS_LIB_FUNC(round, Math::Round);
        REG_OBJECTS_LIB_FUNC(sin, Math::Sin);
        REG_OBJECTS_LIB_FUNC(sqrt, Math::Sqrt);
        REG_OBJECTS_LIB_FUNC(tan, Math::Tan);

        if (config.IsES6MathExtensionsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(log10, Math::Log10);
            REG_OBJECTS_LIB_FUNC(log2, Math::Log2);
            REG_OBJECTS_LIB_FUNC(log1p, Math::Log1p);
            REG_OBJECTS_LIB_FUNC(expm1, Math::Expm1);
            REG_OBJECTS_LIB_FUNC(cosh, Math::Cosh);
            REG_OBJECTS_LIB_FUNC(sinh, Math::Sinh);
            REG_OBJECTS_LIB_FUNC(tanh, Math::Tanh);
            REG_OBJECTS_LIB_FUNC(acosh, Math::Acosh);
            REG_OBJECTS_LIB_FUNC(asinh, Math::Asinh);
            REG_OBJECTS_LIB_FUNC(atanh, Math::Atanh);
            REG_OBJECTS_LIB_FUNC(hypot, Math::Hypot);
            REG_OBJECTS_LIB_FUNC(trunc, Math::Trunc);
            REG_OBJECTS_LIB_FUNC(sign, Math::Sign);
            REG_OBJECTS_LIB_FUNC(cbrt, Math::Cbrt);
            REG_OBJECTS_LIB_FUNC(imul, Math::Imul);
            REG_OBJECTS_LIB_FUNC(clz32, Math::Clz32);
            REG_OBJECTS_LIB_FUNC(fround, Math::Fround);
        }

        return hr;
    }

    HRESULT ScriptContext::RegisterNumber()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Number);

        DEFINE_OBJECT_NAME(Number);

        if (config.IsES6NumberExtensionsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(isNaN, JavascriptNumber::EntryIsNaN);
            REG_OBJECTS_LIB_FUNC(isFinite, JavascriptNumber::EntryIsFinite);
            REG_OBJECTS_LIB_FUNC(isInteger, JavascriptNumber::EntryIsInteger);
            REG_OBJECTS_LIB_FUNC(isSafeInteger, JavascriptNumber::EntryIsSafeInteger);
        }

        REG_OBJECTS_LIB_FUNC(toExponential, JavascriptNumber::EntryToExponential);
        REG_OBJECTS_LIB_FUNC(toFixed, JavascriptNumber::EntryToFixed);
        REG_OBJECTS_LIB_FUNC(toPrecision, JavascriptNumber::EntryToPrecision);
        REG_OBJECTS_LIB_FUNC(toLocaleString, JavascriptNumber::EntryToLocaleString);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptNumber::EntryToString);
        REG_OBJECTS_LIB_FUNC(valueOf, JavascriptNumber::EntryValueOf);

        return hr;
    }

    HRESULT ScriptContext::RegisterString()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(String);

        DEFINE_OBJECT_NAME(String);

        REG_OBJECTS_LIB_FUNC(fromCharCode, JavascriptString::EntryFromCharCode);

        if (config.IsES6UnicodeExtensionsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(fromCodePoint, JavascriptString::EntryFromCodePoint);
            REG_OBJECTS_LIB_FUNC(codePointAt, JavascriptString::EntryCodePointAt);
            REG_OBJECTS_LIB_FUNC(normalize, JavascriptString::EntryNormalize);
        }

        REG_OBJECTS_LIB_FUNC(indexOf, JavascriptString::EntryIndexOf);
        REG_OBJECTS_LIB_FUNC(lastIndexOf, JavascriptString::EntryLastIndexOf);
        REG_OBJECTS_LIB_FUNC(replace, JavascriptString::EntryReplace);
        REG_OBJECTS_LIB_FUNC(search, JavascriptString::EntrySearch);
        REG_OBJECTS_LIB_FUNC(slice, JavascriptString::EntrySlice);
        REG_OBJECTS_LIB_FUNC(charAt, JavascriptString::EntryCharAt);
        REG_OBJECTS_LIB_FUNC(charCodeAt, JavascriptString::EntryCharCodeAt);
        REG_OBJECTS_LIB_FUNC(concat, JavascriptString::EntryConcat);
        REG_OBJECTS_LIB_FUNC(localeCompare, JavascriptString::EntryLocaleCompare);
        REG_OBJECTS_LIB_FUNC(match, JavascriptString::EntryMatch);
        REG_OBJECTS_LIB_FUNC(split, JavascriptString::EntrySplit);
        REG_OBJECTS_LIB_FUNC(substring, JavascriptString::EntrySubstring);
        REG_OBJECTS_LIB_FUNC(substr, JavascriptString::EntrySubstr);
        REG_OBJECTS_LIB_FUNC(toLocaleLowerCase, JavascriptString::EntryToLocaleLowerCase);
        REG_OBJECTS_LIB_FUNC(toLocaleUpperCase, JavascriptString::EntryToLocaleUpperCase);
        REG_OBJECTS_LIB_FUNC(toLowerCase, JavascriptString::EntryToLowerCase);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptString::EntryToString);
        REG_OBJECTS_LIB_FUNC(toUpperCase, JavascriptString::EntryToUpperCase);
        REG_OBJECTS_LIB_FUNC(trim, JavascriptString::EntryTrim);
        REG_OBJECTS_LIB_FUNC(valueOf, JavascriptString::EntryValueOf);
        if (config.SupportsES3Extensions() && config.GetHostType() != HostTypeApplication)
        {
            REG_OBJECTS_LIB_FUNC(anchor, JavascriptString::EntryAnchor);
            REG_OBJECTS_LIB_FUNC(big, JavascriptString::EntryBig);
            REG_OBJECTS_LIB_FUNC(blink, JavascriptString::EntryBlink);
            REG_OBJECTS_LIB_FUNC(bold, JavascriptString::EntryBold);
            REG_OBJECTS_LIB_FUNC(fixed, JavascriptString::EntryFixed);
            REG_OBJECTS_LIB_FUNC(fontcolor, JavascriptString::EntryFontColor);
            REG_OBJECTS_LIB_FUNC(fontsize, JavascriptString::EntryFontSize);
            REG_OBJECTS_LIB_FUNC(italics, JavascriptString::EntryItalics);
            REG_OBJECTS_LIB_FUNC(link, JavascriptString::EntryLink);
            // TODO: Small is defined using Entry2 and it has Small as the propertyId and small as name
            // For some reason if i make it ENTRY(small) instead of ENTRY2(Small, L"small") it overflows with error
            //REG_OBJECTS_LIB_FUNC(Small, JavascriptString::EntrySmall);
            REG_OBJECTS_DYNAMIC_LIB_FUNC(L"small", 5, JavascriptString::EntrySmall);
            REG_OBJECTS_LIB_FUNC(strike, JavascriptString::EntryStrike);
            REG_OBJECTS_LIB_FUNC(sub, JavascriptString::EntrySub);
            REG_OBJECTS_LIB_FUNC(sup, JavascriptString::EntrySup);
        }

        if (config.IsES6StringExtensionsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(repeat, JavascriptString::EntryRepeat);
            REG_OBJECTS_LIB_FUNC(startsWith, JavascriptString::EntryStartsWith);
            REG_OBJECTS_LIB_FUNC(endsWith, JavascriptString::EntryEndsWith);
            REG_OBJECTS_LIB_FUNC(includes, JavascriptString::EntryIncludes);
            REG_OBJECTS_LIB_FUNC(trimLeft, JavascriptString::EntryTrimLeft);
            REG_OBJECTS_LIB_FUNC(trimRight, JavascriptString::EntryTrimRight);

        }

        if (config.IsES6StringTemplateEnabled())
        {
            REG_OBJECTS_LIB_FUNC(raw, JavascriptString::EntryRaw);
        }

        if (config.IsES6IteratorsEnabled())
        {
            REG_OBJECTS_LIB_FUNC2(_symbolIterator, L"[Symbol.iterator]", JavascriptString::EntrySymbolIterator);
        }

        return hr;
    }

    HRESULT ScriptContext::RegisterRegExp()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(RegExp);

        DEFINE_OBJECT_NAME(RegExp);

        REG_OBJECTS_LIB_FUNC(exec, JavascriptRegExp::EntryExec);
        REG_OBJECTS_LIB_FUNC(test, JavascriptRegExp::EntryTest);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptRegExp::EntryToString);
        // This is deprecated. Should be guarded with appropriate version flag.
        REG_OBJECTS_LIB_FUNC(compile, JavascriptRegExp::EntryCompile);

        return hr;
    }

    HRESULT ScriptContext::RegisterJSON()
    {
        HRESULT hr = S_OK;

        DEFINE_OBJECT_NAME(JSON);

        REG_OBJECTS_LIB_FUNC(stringify, JSON::Stringify);
        REG_OBJECTS_LIB_FUNC(parse, JSON::Parse);
        return hr;
    }

    HRESULT ScriptContext::RegisterWeakMap()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(WeakMap);

        DEFINE_OBJECT_NAME(WeakMap);

        REG_OBJECTS_LIB_FUNC2(delete_, L"delete", JavascriptWeakMap::EntryDelete);
        REG_OBJECTS_LIB_FUNC(get, JavascriptWeakMap::EntryGet);
        REG_OBJECTS_LIB_FUNC(has, JavascriptWeakMap::EntryHas);
        REG_OBJECTS_LIB_FUNC(set, JavascriptWeakMap::EntrySet);

        return hr;
    }

    HRESULT ScriptContext::RegisterWeakSet()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(WeakSet);

        DEFINE_OBJECT_NAME(WeakSet);

        REG_OBJECTS_LIB_FUNC(add, JavascriptWeakSet::EntryAdd);
        REG_OBJECTS_LIB_FUNC2(delete_, L"delete", JavascriptWeakSet::EntryDelete);
        REG_OBJECTS_LIB_FUNC(has, JavascriptWeakSet::EntryHas);

        return hr;
    }

    HRESULT ScriptContext::RegisterMap()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Map);

        DEFINE_OBJECT_NAME(Map);

        REG_OBJECTS_LIB_FUNC(clear, JavascriptMap::EntryClear);
        REG_OBJECTS_LIB_FUNC2(delete_, L"delete", JavascriptMap::EntryDelete);
        REG_OBJECTS_LIB_FUNC(forEach, JavascriptMap::EntryForEach);
        REG_OBJECTS_LIB_FUNC(get, JavascriptMap::EntryGet);
        REG_OBJECTS_LIB_FUNC(has, JavascriptMap::EntryHas);
        REG_OBJECTS_LIB_FUNC(set, JavascriptMap::EntrySet);

        if (config.IsES6IteratorsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(entries, JavascriptMap::EntryEntries);
            REG_OBJECTS_LIB_FUNC(keys, JavascriptMap::EntryKeys);
            REG_OBJECTS_LIB_FUNC(values, JavascriptMap::EntryValues);
        }

        return hr;
    }

    HRESULT ScriptContext::RegisterSet()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Set);

        DEFINE_OBJECT_NAME(Set);

        REG_OBJECTS_LIB_FUNC(add, JavascriptSet::EntryAdd);
        REG_OBJECTS_LIB_FUNC(clear, JavascriptSet::EntryClear);
        REG_OBJECTS_LIB_FUNC2(delete_, L"delete", JavascriptSet::EntryDelete);
        REG_OBJECTS_LIB_FUNC(forEach, JavascriptSet::EntryForEach);
        REG_OBJECTS_LIB_FUNC(has, JavascriptSet::EntryHas);

        if (config.IsES6IteratorsEnabled())
        {
            REG_OBJECTS_LIB_FUNC(entries, JavascriptSet::EntryEntries);
            REG_OBJECTS_LIB_FUNC(values, JavascriptSet::EntryValues);
        }

        return hr;
    }

    HRESULT ScriptContext::RegisterSymbol()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Symbol);

        DEFINE_OBJECT_NAME(Symbol);

        REG_OBJECTS_LIB_FUNC(valueOf, JavascriptSymbol::EntryValueOf);
        REG_OBJECTS_LIB_FUNC(toString, JavascriptSymbol::EntryToString);
        REG_OBJECTS_LIB_FUNC2(for_, L"for", JavascriptSymbol::EntryFor);
        REG_OBJECTS_LIB_FUNC(keyFor, JavascriptSymbol::EntryKeyFor);

        return hr;
    }

    HRESULT ScriptContext::RegisterArrayIterator()
    {
        HRESULT hr = S_OK;
        // Array Iterator has no global constructor

        DEFINE_OBJECT_NAME(Array Iterator);

        REG_OBJECTS_LIB_FUNC(next, JavascriptArrayIterator::EntryNext);
        REG_OBJECTS_LIB_FUNC2(_symbolIterator, L"[Symbol.iterator]", JavascriptArrayIterator::EntrySymbolIterator);

        return hr;
    }

    HRESULT ScriptContext::RegisterMapIterator()
    {
        HRESULT hr = S_OK;
        // Map Iterator has no global constructor

        DEFINE_OBJECT_NAME(Map Iterator);

        REG_OBJECTS_LIB_FUNC(next, JavascriptMapIterator::EntryNext);
        REG_OBJECTS_LIB_FUNC2(_symbolIterator, L"[Symbol.iterator]", JavascriptMapIterator::EntrySymbolIterator);

        return hr;
    }

    HRESULT ScriptContext::RegisterSetIterator()
    {
        HRESULT hr = S_OK;
        // Set Iterator has no global constructor

        DEFINE_OBJECT_NAME(Set Iterator);

        REG_OBJECTS_LIB_FUNC(next, JavascriptSetIterator::EntryNext);
        REG_OBJECTS_LIB_FUNC2(_symbolIterator, L"[Symbol.iterator]", JavascriptSetIterator::EntrySymbolIterator);

        return hr;
    }

    HRESULT ScriptContext::RegisterStringIterator()
    {
        HRESULT hr = S_OK;
        // String Iterator has no global constructor

        DEFINE_OBJECT_NAME(String Iterator);

        REG_OBJECTS_LIB_FUNC(next, JavascriptStringIterator::EntryNext);
        REG_OBJECTS_LIB_FUNC2(_symbolIterator, L"[Symbol.iterator]", JavascriptStringIterator::EntrySymbolIterator);

        return hr;
    }

    HRESULT ScriptContext::RegisterEnumeratorIterator()
    {
        HRESULT hr = S_OK;
        // Enumerator Iterator has no global constructor

        DEFINE_OBJECT_NAME(Enumerator Iterator);

        REG_OBJECTS_LIB_FUNC(next, JavascriptEnumeratorIterator::EntryNext);
        REG_OBJECTS_LIB_FUNC2(_symbolIterator, L"[Symbol.iterator]", JavascriptEnumeratorIterator::EntrySymbolIterator);

        return hr;
    }

    HRESULT ScriptContext::RegisterTypedArray()
    {
        HRESULT hr = S_OK;
        // TypedArray has no named global constructor

        DEFINE_OBJECT_NAME(TypedArray);

        REG_OBJECTS_LIB_FUNC(from, TypedArrayBase::EntryFrom);
        REG_OBJECTS_LIB_FUNC(of, TypedArrayBase::EntryOf);
        REG_OBJECTS_LIB_FUNC(set, TypedArrayBase::EntrySet);
        REG_OBJECTS_LIB_FUNC(subarray, TypedArrayBase::EntrySubarray);
        REG_OBJECTS_LIB_FUNC(copyWithin, TypedArrayBase::EntryCopyWithin);
        REG_OBJECTS_LIB_FUNC(every, TypedArrayBase::EntryEvery);
        REG_OBJECTS_LIB_FUNC(fill, TypedArrayBase::EntryFill);
        REG_OBJECTS_LIB_FUNC(filter, TypedArrayBase::EntryFilter);
        REG_OBJECTS_LIB_FUNC(find, TypedArrayBase::EntryFind);
        REG_OBJECTS_LIB_FUNC(findIndex, TypedArrayBase::EntryFindIndex);
        REG_OBJECTS_LIB_FUNC(forEach, TypedArrayBase::EntryForEach);
        REG_OBJECTS_LIB_FUNC(indexOf, TypedArrayBase::EntryIndexOf);
        REG_OBJECTS_LIB_FUNC(join, TypedArrayBase::EntryJoin);
        REG_OBJECTS_LIB_FUNC(lastIndexOf, TypedArrayBase::EntryLastIndexOf);
        REG_OBJECTS_LIB_FUNC(map, TypedArrayBase::EntryMap);
        REG_OBJECTS_LIB_FUNC(reduce, TypedArrayBase::EntryReduce);
        REG_OBJECTS_LIB_FUNC(reduceRight, TypedArrayBase::EntryReduceRight);
        REG_OBJECTS_LIB_FUNC(reverse, TypedArrayBase::EntryReverse);
        REG_OBJECTS_LIB_FUNC(slice, TypedArrayBase::EntrySlice);
        REG_OBJECTS_LIB_FUNC(some, TypedArrayBase::EntrySome);
        REG_OBJECTS_LIB_FUNC(sort, TypedArrayBase::EntrySort);

        return hr;
    }

    HRESULT ScriptContext::RegisterPromise()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Promise);

        DEFINE_OBJECT_NAME(Promise);

        REG_OBJECTS_LIB_FUNC(all, JavascriptPromise::EntryAll);
        REG_OBJECTS_LIB_FUNC2(catch_, L"catch", JavascriptPromise::EntryCatch);
        REG_OBJECTS_LIB_FUNC(race, JavascriptPromise::EntryRace);
        REG_OBJECTS_LIB_FUNC(resolve, JavascriptPromise::EntryResolve);
        REG_OBJECTS_LIB_FUNC(then, JavascriptPromise::EntryThen);

        return hr;
    }

    HRESULT ScriptContext::RegisterProxy()
    {
        HRESULT hr = S_OK;
        REG_GLOBAL_CONSTRUCTOR(Proxy);

        DEFINE_OBJECT_NAME(Proxy);

        REG_OBJECTS_LIB_FUNC(revocable, JavascriptProxy::EntryRevocable);
        return hr;
    }

    HRESULT ScriptContext::RegisterReflect()
    {
        HRESULT hr = S_OK;
        DEFINE_OBJECT_NAME(Reflect);

        REG_OBJECTS_LIB_FUNC(defineProperty, JavascriptReflect::EntryDefineProperty);
        REG_OBJECTS_LIB_FUNC(deleteProperty, JavascriptReflect::EntryDeleteProperty);
        REG_OBJECTS_LIB_FUNC(enumerate, JavascriptReflect::EntryEnumerate);
        REG_OBJECTS_LIB_FUNC(get, JavascriptReflect::EntryGet);
        REG_OBJECTS_LIB_FUNC(getOwnPropertyDescriptor, JavascriptReflect::EntryGetOwnPropertyDescriptor);
        REG_OBJECTS_LIB_FUNC(getPrototypeOf, JavascriptReflect::EntryGetPrototypeOf);
        REG_OBJECTS_LIB_FUNC(has, JavascriptReflect::EntryHas);
        REG_OBJECTS_LIB_FUNC(isExtensible, JavascriptReflect::EntryIsExtensible);
        REG_OBJECTS_LIB_FUNC(ownKeys, JavascriptReflect::EntryOwnKeys);
        REG_OBJECTS_LIB_FUNC(preventExtensions, JavascriptReflect::EntryPreventExtensions);
        REG_OBJECTS_LIB_FUNC(set, JavascriptReflect::EntrySet);
        REG_OBJECTS_LIB_FUNC(setPrototypeOf, JavascriptReflect::EntrySetPrototypeOf);
        REG_OBJECTS_LIB_FUNC(apply, JavascriptReflect::EntryApply);
        REG_OBJECTS_LIB_FUNC(construct, JavascriptReflect::EntryConstruct);
        return hr;
    }


    HRESULT ScriptContext::RegisterGenerator()
    {
        HRESULT hr = S_OK;
        DEFINE_OBJECT_NAME(Generator);

        REG_OBJECTS_LIB_FUNC(next, JavascriptGenerator::EntryNext);
        REG_OBJECTS_LIB_FUNC(return_, JavascriptGenerator::EntryReturn);
        REG_OBJECTS_LIB_FUNC(throw_, JavascriptGenerator::EntryThrow);
        REG_OBJECTS_LIB_FUNC2(_symbolIterator, L"[Symbol.iterator]", JavascriptGenerator::EntrySymbolIterator);

        return hr;
    }

#ifdef SIMD_JS_ENABLED
    HRESULT ScriptContext::RegisterSIMD()
    {
        HRESULT hr = S_OK;

        DEFINE_OBJECT_NAME(SIMD);

        // Float32x4
        REG_OBJECTS_LIB_FUNC(float32x4, SIMDFloat32x4Lib::EntryFloat32x4);
        REG_OBJECTS_LIB_FUNC(zero, SIMDFloat32x4Lib::EntryZero);
        REG_OBJECTS_LIB_FUNC(splat, SIMDFloat32x4Lib::EntrySplat);
        REG_OBJECTS_LIB_FUNC(withX, SIMDFloat32x4Lib::EntryWithX);
        REG_OBJECTS_LIB_FUNC(withY, SIMDFloat32x4Lib::EntryWithY);
        REG_OBJECTS_LIB_FUNC(withZ, SIMDFloat32x4Lib::EntryWithZ);
        REG_OBJECTS_LIB_FUNC(withW, SIMDFloat32x4Lib::EntryWithW);
        REG_OBJECTS_LIB_FUNC(fromFloat64x2, SIMDFloat32x4Lib::EntryFromFloat64x2);
        REG_OBJECTS_LIB_FUNC(fromFloat64x2Bits, SIMDFloat32x4Lib::EntryFromFloat64x2Bits);
        REG_OBJECTS_LIB_FUNC(fromInt32x4, SIMDFloat32x4Lib::EntryFromInt32x4);
        REG_OBJECTS_LIB_FUNC(fromInt32x4Bits, SIMDFloat32x4Lib::EntryFromInt32x4Bits);
        REG_OBJECTS_LIB_FUNC(add, SIMDFloat32x4Lib::EntryAdd);
        REG_OBJECTS_LIB_FUNC(sub, SIMDFloat32x4Lib::EntrySub);
        REG_OBJECTS_LIB_FUNC(mul, SIMDFloat32x4Lib::EntryMul);
        REG_OBJECTS_LIB_FUNC(div, SIMDFloat32x4Lib::EntryDiv);
        REG_OBJECTS_LIB_FUNC(and, SIMDFloat32x4Lib::EntryAnd);
        REG_OBJECTS_LIB_FUNC(or, SIMDFloat32x4Lib::EntryOr);
        REG_OBJECTS_LIB_FUNC(xor, SIMDFloat32x4Lib::EntryXor);
        REG_OBJECTS_LIB_FUNC(min, SIMDFloat32x4Lib::EntryMin);
        REG_OBJECTS_LIB_FUNC(max, SIMDFloat32x4Lib::EntryMax);
        REG_OBJECTS_LIB_FUNC(scale, SIMDFloat32x4Lib::EntryScale);
        REG_OBJECTS_LIB_FUNC(abs, SIMDFloat32x4Lib::EntryAbs);
        REG_OBJECTS_LIB_FUNC(neg, SIMDFloat32x4Lib::EntryNeg);
        REG_OBJECTS_LIB_FUNC(not, SIMDFloat32x4Lib::EntryNot);
        REG_OBJECTS_LIB_FUNC(sqrt, SIMDFloat32x4Lib::EntrySqrt);
        REG_OBJECTS_LIB_FUNC(reciprocal, SIMDFloat32x4Lib::EntryReciprocal);
        REG_OBJECTS_LIB_FUNC(reciprocalSqrt, SIMDFloat32x4Lib::EntryReciprocalSqrt);
        REG_OBJECTS_LIB_FUNC(lessThan, SIMDFloat32x4Lib::EntryLessThan);
        REG_OBJECTS_LIB_FUNC(lessThanOrEqual, SIMDFloat32x4Lib::EntryLessThanOrEqual);
        REG_OBJECTS_LIB_FUNC(equal, SIMDFloat32x4Lib::EntryEqual);
        REG_OBJECTS_LIB_FUNC(notEqual, SIMDFloat32x4Lib::EntryNotEqual);
        REG_OBJECTS_LIB_FUNC(greaterThan, SIMDFloat32x4Lib::EntryGreaterThan);
        REG_OBJECTS_LIB_FUNC(greaterThanOrEqual, SIMDFloat32x4Lib::EntryGreaterThanOrEqual);
        REG_OBJECTS_LIB_FUNC(shuffle, SIMDFloat32x4Lib::EntryShuffle);
        REG_OBJECTS_LIB_FUNC(shuffleMix, SIMDFloat32x4Lib::EntryShuffleMix);
        REG_OBJECTS_LIB_FUNC(clamp, SIMDFloat32x4Lib::EntryClamp);
        REG_OBJECTS_LIB_FUNC(select, SIMDFloat32x4Lib::EntrySelect);
        // Float64x2
        REG_OBJECTS_LIB_FUNC(float64x2, SIMDFloat64x2Lib::EntryFloat64x2);
        REG_OBJECTS_LIB_FUNC(zero, SIMDFloat64x2Lib::EntryZero);
        REG_OBJECTS_LIB_FUNC(splat, SIMDFloat64x2Lib::EntrySplat);
        REG_OBJECTS_LIB_FUNC(withX, SIMDFloat64x2Lib::EntryWithX);
        REG_OBJECTS_LIB_FUNC(withY, SIMDFloat64x2Lib::EntryWithY);
        REG_OBJECTS_LIB_FUNC(fromFloat32x4, SIMDFloat64x2Lib::EntryFromFloat32x4);
        REG_OBJECTS_LIB_FUNC(fromFloat32x4Bits, SIMDFloat64x2Lib::EntryFromFloat32x4Bits);
        REG_OBJECTS_LIB_FUNC(fromInt32x4, SIMDFloat64x2Lib::EntryFromInt32x4);
        REG_OBJECTS_LIB_FUNC(fromInt32x4Bits, SIMDFloat64x2Lib::EntryFromInt32x4Bits);
        REG_OBJECTS_LIB_FUNC(add, SIMDFloat64x2Lib::EntryAdd);
        REG_OBJECTS_LIB_FUNC(sub, SIMDFloat64x2Lib::EntrySub);
        REG_OBJECTS_LIB_FUNC(mul, SIMDFloat64x2Lib::EntryMul);
        REG_OBJECTS_LIB_FUNC(div, SIMDFloat64x2Lib::EntryDiv);
        REG_OBJECTS_LIB_FUNC(min, SIMDFloat64x2Lib::EntryMin);
        REG_OBJECTS_LIB_FUNC(max, SIMDFloat64x2Lib::EntryMax);
        REG_OBJECTS_LIB_FUNC(scale, SIMDFloat64x2Lib::EntryScale);
        REG_OBJECTS_LIB_FUNC(abs, SIMDFloat64x2Lib::EntryAbs);
        REG_OBJECTS_LIB_FUNC(neg, SIMDFloat64x2Lib::EntryNeg);
        REG_OBJECTS_LIB_FUNC(sqrt, SIMDFloat64x2Lib::EntrySqrt);
        REG_OBJECTS_LIB_FUNC(reciprocal, SIMDFloat64x2Lib::EntryReciprocal);
        REG_OBJECTS_LIB_FUNC(reciprocalSqrt, SIMDFloat64x2Lib::EntryReciprocalSqrt);
        REG_OBJECTS_LIB_FUNC(lessThan, SIMDFloat64x2Lib::EntryLessThan);
        REG_OBJECTS_LIB_FUNC(lessThanOrEqual, SIMDFloat64x2Lib::EntryLessThanOrEqual);
        REG_OBJECTS_LIB_FUNC(equal, SIMDFloat64x2Lib::EntryEqual);
        REG_OBJECTS_LIB_FUNC(notEqual, SIMDFloat64x2Lib::EntryNotEqual);
        REG_OBJECTS_LIB_FUNC(greaterThan, SIMDFloat64x2Lib::EntryGreaterThan);
        REG_OBJECTS_LIB_FUNC(greaterThanOrEqual, SIMDFloat64x2Lib::EntryGreaterThanOrEqual);
        REG_OBJECTS_LIB_FUNC(shuffle, SIMDFloat64x2Lib::EntryShuffle);
        REG_OBJECTS_LIB_FUNC(shuffleMix, SIMDFloat64x2Lib::EntryShuffleMix);
        REG_OBJECTS_LIB_FUNC(clamp, SIMDFloat64x2Lib::EntryClamp);
        REG_OBJECTS_LIB_FUNC(select, SIMDFloat64x2Lib::EntrySelect);
        // Int32x4
        REG_OBJECTS_LIB_FUNC(int32x4, SIMDInt32x4Lib::EntryInt32x4);
        REG_OBJECTS_LIB_FUNC(zero, SIMDInt32x4Lib::EntryZero);
        REG_OBJECTS_LIB_FUNC(splat, SIMDInt32x4Lib::EntrySplat);
        REG_OBJECTS_LIB_FUNC(bool_, SIMDInt32x4Lib::EntryBool);
        REG_OBJECTS_LIB_FUNC(withX, SIMDInt32x4Lib::EntryWithX);
        REG_OBJECTS_LIB_FUNC(withY, SIMDInt32x4Lib::EntryWithY);
        REG_OBJECTS_LIB_FUNC(withZ, SIMDInt32x4Lib::EntryWithZ);
        REG_OBJECTS_LIB_FUNC(withW, SIMDInt32x4Lib::EntryWithW);
        REG_OBJECTS_LIB_FUNC(withFlagX, SIMDInt32x4Lib::EntryWithFlagX);
        REG_OBJECTS_LIB_FUNC(withFlagY, SIMDInt32x4Lib::EntryWithFlagY);
        REG_OBJECTS_LIB_FUNC(withFlagZ, SIMDInt32x4Lib::EntryWithFlagZ);
        REG_OBJECTS_LIB_FUNC(withFlagW, SIMDInt32x4Lib::EntryWithFlagW);
        REG_OBJECTS_LIB_FUNC(fromFloat64x2, SIMDInt32x4Lib::EntryFromFloat64x2);
        REG_OBJECTS_LIB_FUNC(fromFloat64x2Bits, SIMDInt32x4Lib::EntryFromFloat64x2Bits);
        REG_OBJECTS_LIB_FUNC(fromFloat32x4, SIMDInt32x4Lib::EntryFromFloat32x4);
        REG_OBJECTS_LIB_FUNC(fromFloat32x4Bits, SIMDInt32x4Lib::EntryFromFloat32x4Bits);
        REG_OBJECTS_LIB_FUNC(add, SIMDInt32x4Lib::EntryAdd);
        REG_OBJECTS_LIB_FUNC(sub, SIMDInt32x4Lib::EntrySub);
        REG_OBJECTS_LIB_FUNC(mul, SIMDInt32x4Lib::EntryMul);
        REG_OBJECTS_LIB_FUNC(and, SIMDInt32x4Lib::EntryAnd);
        REG_OBJECTS_LIB_FUNC(or,  SIMDInt32x4Lib::EntryOr);
        REG_OBJECTS_LIB_FUNC(xor, SIMDInt32x4Lib::EntryXor);
        REG_OBJECTS_LIB_FUNC(neg, SIMDInt32x4Lib::EntryNeg);
        REG_OBJECTS_LIB_FUNC(not, SIMDInt32x4Lib::EntryNot);
        REG_OBJECTS_LIB_FUNC(lessThan, SIMDInt32x4Lib::EntryLessThan);
        REG_OBJECTS_LIB_FUNC(equal, SIMDInt32x4Lib::EntryEqual);
        REG_OBJECTS_LIB_FUNC(greaterThan, SIMDInt32x4Lib::EntryGreaterThan);
        REG_OBJECTS_LIB_FUNC(shuffle, SIMDInt32x4Lib::EntryShuffle);
        REG_OBJECTS_LIB_FUNC(shuffleMix, SIMDInt32x4Lib::EntryShuffleMix);
        REG_OBJECTS_LIB_FUNC(shiftLeft, SIMDInt32x4Lib::EntryShiftLeft);
        REG_OBJECTS_LIB_FUNC(shiftRightLogical, SIMDInt32x4Lib::EntryShiftRightLogical);
        REG_OBJECTS_LIB_FUNC(shiftRightArithmetic, SIMDInt32x4Lib::EntryShiftRightArithmetic);
        REG_OBJECTS_LIB_FUNC(select, SIMDInt32x4Lib::EntrySelect);

        return hr;
    }
#endif

#ifdef IR_VIEWER
    HRESULT ScriptContext::RegisterIRViewer()
    {
        HRESULT hr = S_OK;

        DEFINE_OBJECT_NAME(IRViewer);

        // TODO (t-doilij) move GlobalObject::EntryParseIR to JavascriptIRViewer::EntryParseIR
        REG_LIB_FUNC_CORE(pwszObjectName, L"parseIR", GetOrAddPropertyIdTracked(L"parseIR"), GlobalObject::EntryParseIR);
        REG_LIB_FUNC_CORE(pwszObjectName, L"functionList", GetOrAddPropertyIdTracked(L"functionList"), GlobalObject::EntryFunctionList);
        REG_LIB_FUNC_CORE(pwszObjectName, L"rejitFunction", GetOrAddPropertyIdTracked(L"rejitFunction"), GlobalObject::EntryRejitFunction);

        return hr;
    }
#endif /* IR_VIEWER */

    void ScriptContext::BindReference(void * addr)
    {
        Assert(!this->isClosed);
        Assert(this->guestArena);
        Assert(recycler->IsValidObject(addr));
#if DBG
        if (this->bindRef == NULL)
        {
            typedef SimpleHashTable<void *, uint> RelativeHashTable;
            bindRef = Anew(MiscAllocator(), RelativeHashTable, MiscAllocator());
        }
        uint refCount;
        Assert(!bindRef->TryGetValue(addr, &refCount));
        bindRef->Add(addr, 1);
#endif
        if (bindRefChunkCurrent == bindRefChunkEnd)
        {
            bindRefChunkCurrent = AnewArrayZ(this->guestArena, void *, ArenaAllocator::ObjectAlignment / sizeof(void *));
            bindRefChunkEnd = bindRefChunkCurrent + ArenaAllocator::ObjectAlignment / sizeof(void *);
        }
        Assert((bindRefChunkCurrent + 1) <= bindRefChunkEnd);
        *bindRefChunkCurrent = addr;
        bindRefChunkCurrent++;

#ifdef RECYCLER_PERF_COUNTERS
        this->bindReferenceCount++;
        RECYCLER_PERF_COUNTER_INC(BindReference);
#endif
    }

    HRESULT ScriptContext::RegisterPixelArray()
    {
        HRESULT hr = S_OK;

        // Due to naming inconsistency of CanvasPixelArray, the REG_GLOBAL_CONSTRUCTOR macro doesn't work here
        REG_LIB_FUNC(NULL, CanvasPixelArray, JavascriptPixelArray::NewInstance);

        return hr;
    }

#ifdef PROFILE_STRINGS
    StringProfiler* ScriptContext::GetStringProfiler()
    {
        return stringProfiler;
    }
#endif

    void ScriptContext::FreeLoopBody(void* address)
    {
        FreeNativeCodeGenAllocation(this, address);
    }

    void ScriptContext::FreeFunctionEntryPoint(Js::JavascriptMethod method)
    {
        FreeNativeCodeGenAllocation(this, method);
    }

    void ScriptContext::RegisterAsScriptContextWithInlineCaches()
    {
        if (this->entryInScriptContextWithInlineCachesRegistry == null)
        {
            DoRegisterAsScriptContextWithInlineCaches();
        }
    }

    void ScriptContext::DoRegisterAsScriptContextWithInlineCaches()
    {
        Assert(this->entryInScriptContextWithInlineCachesRegistry == null);
        // this call may throw OOM
        this->entryInScriptContextWithInlineCachesRegistry = threadContext->RegisterInlineCacheScriptContext(this);
    }

    void ScriptContext::RegisterAsScriptContextWithIsInstInlineCaches()
    {
        if (this->entryInScriptContextWithIsInstInlineCachesRegistry == null)
        {
            DoRegisterAsScriptContextWithIsInstInlineCaches();
        }
    }

    bool ScriptContext::IsRegisteredAsScriptContextWithIsInstInlineCaches()
    {
        return this->entryInScriptContextWithIsInstInlineCachesRegistry != null;
    }

    void ScriptContext::DoRegisterAsScriptContextWithIsInstInlineCaches()
    {
        Assert(this->entryInScriptContextWithIsInstInlineCachesRegistry == null);
        // this call may throw OOM
        this->entryInScriptContextWithIsInstInlineCachesRegistry = threadContext->RegisterIsInstInlineCacheScriptContext(this);
    }

    void ScriptContext::RegisterProtoInlineCache(InlineCache *pCache, PropertyId propId)
    {
        hasRegisteredInlineCache = true;
        threadContext->RegisterProtoInlineCache(pCache, propId);
    }

    void ScriptContext::InvalidateProtoCaches(const PropertyId propertyId)
    {
        threadContext->InvalidateProtoInlineCaches(propertyId);
        // Because setter inline caches get registered in the store field chain, we must invalidate that
        // chain whenever we invalidate the proto chain.
        threadContext->InvalidateStoreFieldInlineCaches(propertyId);
        threadContext->InvalidatePropertyGuards(propertyId);
        threadContext->InvalidateProtoTypePropertyCaches(propertyId);
    }

    void ScriptContext::InvalidateAllProtoCaches()
    {
        threadContext->InvalidateAllProtoInlineCaches();
        // Because setter inline caches get registered in the store field chain, we must invalidate that
        // chain whenever we invalidate the proto chain.
        threadContext->InvalidateAllStoreFieldInlineCaches();
        threadContext->InvalidateAllPropertyGuards();
        threadContext->InvalidateAllProtoTypePropertyCaches();
    }

    void ScriptContext::RegisterStoreFieldInlineCache(InlineCache *pCache, PropertyId propId)
    {
        hasRegisteredInlineCache = true;
        threadContext->RegisterStoreFieldInlineCache(pCache, propId);
    }

    void ScriptContext::InvalidateStoreFieldCaches(const PropertyId propertyId)
    {
        threadContext->InvalidateStoreFieldInlineCaches(propertyId);
        threadContext->InvalidatePropertyGuards(propertyId);
    }

    void ScriptContext::InvalidateAllStoreFieldCaches()
    {
        threadContext->InvalidateAllStoreFieldInlineCaches();
    }

    void ScriptContext::RegisterIsInstInlineCache(Js::IsInstInlineCache * cache, Js::Var function)
    {
        Assert(JavascriptFunction::FromVar(function)->GetScriptContext() == this);
        hasRegisteredIsInstInlineCache = true;
        threadContext->RegisterIsInstInlineCache(cache, function);
    }

#if DBG
    bool ScriptContext::IsIsInstInlineCacheRegistered(Js::IsInstInlineCache * cache, Js::Var function)
    {
        return threadContext->IsIsInstInlineCacheRegistered(cache, function);
    }
#endif

    void ScriptContext::CleanSourceListInternal(bool calledDuringMark)
    {
        bool fCleanupDocRequired = false;
        for (int i = 0; i < sourceList->Count(); i++)
        {
            if (this->sourceList->IsItemValid(i))
            {
                RecyclerWeakReference<Utf8SourceInfo>* sourceInfoWeakRef = this->sourceList->Item(i);
                Utf8SourceInfo* strongRef = null;

                if (calledDuringMark)
                {
                    strongRef = sourceInfoWeakRef->FastGet();
                }
                else
                {
                    strongRef = sourceInfoWeakRef->Get();
                }

                if (strongRef == null)
                {
                    this->sourceList->RemoveAt(i);
                    fCleanupDocRequired = true;
                }
            }
        }

        // If the sourceList got changed, in we need to refresh the nondebug document list in the profiler mode.
        if (fCleanupDocRequired && m_pProfileCallback != NULL)
        {
            Assert(CleanupDocumentContext != NULL);
            CleanupDocumentContext(this);
        }
    }

    void ScriptContext::ClearScriptContextCaches()
    {
        // Prevent reentrancy for the following work, which is not required to be done on every call to this function including
        // reentrant calls
        if (this->isPerformingNonreentrantWork)
        {
            return;
        }
        class AutoCleanup
        {
        private:
            ScriptContext *const scriptContext;

        public:
            AutoCleanup(ScriptContext *const scriptContext) : scriptContext(scriptContext)
            {
                scriptContext->isPerformingNonreentrantWork = true;
            }

            ~AutoCleanup()
            {
                scriptContext->isPerformingNonreentrantWork = false;
            }
        } autoCleanup(this);

        if (this->isScriptContextActuallyClosed)
        {
            return;
        }
        Assert(this->guestArena);
        Assert(this->cache);

        if (EnableEvalMapCleanup())
        {
            // The eval map is not re-entrant, so make sure it's not in the middle of adding an entry
            // Also, dont clean the eval map if the debugger is attached
            if (!this->IsInDebugMode())
            {
                if (this->cache->evalCacheDictionary != null)
                {
                    this->CleanDynamicFunctionCache<Js::EvalCacheTopLevelDictionary>(this->cache->evalCacheDictionary->GetDictionary());
                }
                if (this->cache->indirectEvalCacheDictionary != null)
                {
                    this->CleanDynamicFunctionCache<Js::EvalCacheTopLevelDictionary>(this->cache->indirectEvalCacheDictionary->GetDictionary());
                }
                if (this->cache->newFunctionCache != null)
                {
                    this->CleanDynamicFunctionCache<Js::NewFunctionCache>(this->cache->newFunctionCache);
                }
                if (this->hostScriptContext != null)
                {
                    this->hostScriptContext->CleanDynamicCodeCache();
                }

            }
        }

        if (REGEX_CONFIG_FLAG(DynamicRegexMruListSize) > 0)
        {
            GetDynamicRegexMap()->RemoveRecentlyUnusedItems();
        }

        // The language service expects the source index in the source list to never change
        // so to keep this invariant, we don't cleanup the source list during GC
        if (!BinaryFeatureControl::LanguageService())
        {
            CleanSourceListInternal(true);
        }
    }

void ScriptContext::ClearInlineCaches()
{
    Assert(this->entryInScriptContextWithInlineCachesRegistry != null);

    // For persistent inline caches, we assume here that all thread context's invalidation lists
    // will be reset, such that all invalidationListSlotPtr will get zeroed.  We will not be zeroing
    // this field here to preserve the free list, which uses the field to link caches together.
    GetInlineCacheAllocator()->ZeroAll();

    this->entryInScriptContextWithInlineCachesRegistry = null; // caller will remove us from the thread context

    this->hasRegisteredInlineCache = false;
}

void ScriptContext::ClearIsInstInlineCaches()
{
    Assert(entryInScriptContextWithIsInstInlineCachesRegistry != null);
    GetIsInstInlineCacheAllocator()->ZeroAll();

    this->entryInScriptContextWithIsInstInlineCachesRegistry = null; // caller will remove us from the thread context.

    this->hasRegisteredIsInstInlineCache = false;
}


#ifdef PERSISTENT_INLINE_CACHES
void ScriptContext::ClearInlineCachesWithDeadWeakRefs()
{
    // Review: I should be able to assert this here just like in ClearInlineCaches.
    Assert(this->entryInScriptContextWithInlineCachesRegistry != null);
    GetInlineCacheAllocator()->ClearCachesWithDeadWeakRefs(this->recycler);
    Assert(GetInlineCacheAllocator()->HasNoDeadWeakRefs(this->recycler));
}
#endif

void ScriptContext::RegisterConstructorCache(Js::PropertyId propertyId, Js::ConstructorCache* cache)
{
    this->threadContext->RegisterConstructorCache(propertyId, cache);
}

void ScriptContext::RegisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext()
{
    Assert(!IsClosed());

    if (registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext == null)
    {
        DoRegisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext();
    }
}

    void ScriptContext::DoRegisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext()
    {
        Assert(!IsClosed());
        Assert(registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext == null);

        // this call may throw OOM
        registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext = threadContext->RegisterPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext(this);
    }

    void ScriptContext::ClearPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesCaches()
    {
        Assert(registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext != null);
        javascriptLibrary->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();

        // Caller will unregister the script context from the thread context
        registeredPrototypeChainEnsuredToHaveOnlyWritableDataPropertiesScriptContext = null;
    }

    JavascriptString * ScriptContext::GetLastNumberToStringRadix10(double value)
    {
        if (value == lastNumberToStringRadix10)
        {
            return cache->lastNumberToStringRadix10String;
        }
        return null;
    }

    void
        ScriptContext::SetLastNumberToStringRadix10(double value, JavascriptString * str)
    {
            lastNumberToStringRadix10 = value;
            cache->lastNumberToStringRadix10String = str;
    }

    bool ScriptContext::GetLastUtcTimeFromStr(JavascriptString * str, double& dbl)
    {
        Assert(str != null);
        if (str != cache->lastUtcTimeFromStrString)
        {
            if (cache->lastUtcTimeFromStrString == null
                || !JavascriptString::Equals(str, cache->lastUtcTimeFromStrString))
            {
                return false;
            }
        }
        dbl = lastUtcTimeFromStr;
        return true;
    }

    void
        ScriptContext::SetLastUtcTimeFromStr(JavascriptString * str, double value)
    {
            lastUtcTimeFromStr = value;
            cache->lastUtcTimeFromStrString = str;
    }

#ifdef ENABLE_NATIVE_CODEGEN
    BOOL ScriptContext::IsNativeAddress(void * codeAddr)
    {
        PreReservedVirtualAllocWrapper *preReservedVirtualAllocWrapper = this->threadContext->GetPreReservedVirtualAllocator();
        if (preReservedVirtualAllocWrapper->IsPreReservedRegionPresent())
        {
            if (preReservedVirtualAllocWrapper->IsInRange(codeAddr))
            {
                Assert(!this->IsDynamicInterpreterThunk(codeAddr));
                return true;
            }
            else if (this->threadContext->IsAllJITCodeInPreReservedRegion())
            {
                return false;
            }
        }

        // Try locally first and then all script context on the thread
        //Slow path
        return IsNativeFunctionAddr(this, codeAddr) || this->threadContext->IsNativeAddress(codeAddr);
    }
#endif

    bool ScriptContext::SetDispatchProfile(bool fSet, JavascriptMethod dispatchInvoke)
    {
        if (!fSet)
        {
            this->javascriptLibrary->SetDispatchProfile(false, dispatchInvoke);
            return true;
        }
        else if (m_fTraceDomCall)
        {
            this->javascriptLibrary->SetDispatchProfile(true, dispatchInvoke);
            return true;
        }

        return false;
    }

    HRESULT ScriptContext::OnDispatchFunctionEnter(const WCHAR *pwszFunctionName)
    {
        if (m_pProfileCallback2 == NULL)
        {
            return ACTIVPROF_E_PROFILER_ABSENT;
        }

        HRESULT hr = S_OK;

        if (m_fTraceDomCall)
        {
            m_inProfileCallback = TRUE;
            hr = m_pProfileCallback2->OnFunctionEnterByName(pwszFunctionName, PROFILER_SCRIPT_TYPE_DOM);
            m_inProfileCallback = FALSE;
        }
        return hr;
    }

    HRESULT ScriptContext::OnDispatchFunctionExit(const WCHAR *pwszFunctionName)
    {
        if (m_pProfileCallback2 == NULL)
        {
            return ACTIVPROF_E_PROFILER_ABSENT;
        }

        HRESULT hr = S_OK;

        if (m_fTraceDomCall)
        {
            m_inProfileCallback = TRUE;
            hr = m_pProfileCallback2->OnFunctionExitByName(pwszFunctionName, PROFILER_SCRIPT_TYPE_DOM);
            m_inProfileCallback = FALSE;
        }
        return hr;
    }

    void ScriptContext::SetBuiltInLibraryFunction(JavascriptMethod entryPoint, JavascriptFunction* function)
    {
        if (!isClosed)
        {
            if (builtInLibraryFunctions == NULL)
            {
                Assert(this->recycler);

                builtInLibraryFunctions = RecyclerNew(this->recycler, BuiltInLibraryFunctionMap, this->recycler);
                BindReference(builtInLibraryFunctions);
            }

            builtInLibraryFunctions->Item(entryPoint, function);
        }
    }

    JavascriptFunction* ScriptContext::GetBuiltInLibraryFunction(JavascriptMethod entryPoint)
    {
        JavascriptFunction * function = NULL;
        if (builtInLibraryFunctions)
        {
            builtInLibraryFunctions->TryGetValue(entryPoint, &function);
        }
        return function;
    }

    DOMFastPathIRHelperMap* ScriptContext::EnsureDOMFastPathIRHelperMap()
    {
        if (domFastPathIRHelperMap == nullptr)
        {
            // Anew throws if it OOMs, so the caller into this function needs to handle that exception
            domFastPathIRHelperMap = Anew(GeneralAllocator(), DOMFastPathIRHelperMap,
                GeneralAllocator(), 17);    // initial capacity set to 17; unlikely to grow much bigger.
        }

        return domFastPathIRHelperMap;
    }

    void ScriptContext::AddDynamicProfileInfo(FunctionBody * functionBody, WriteBarrierPtr<DynamicProfileInfo>* dynamicProfileInfo)
    {
        Assert(functionBody->GetScriptContext() == this);
        Assert(functionBody->HasValidSourceInfo());

        DynamicProfileInfo * newDynamicProfileInfo = *dynamicProfileInfo;
        // If it is a dynamic script - we should create a profile info bound to the threadContext for its lifetime.
        SourceContextInfo* sourceContextInfo = functionBody->GetSourceContextInfo();
        SourceDynamicProfileManager* profileManager = sourceContextInfo->sourceDynamicProfileManager;
        if (sourceContextInfo->IsDynamic())
        {
            if (profileManager != null)
            {
                // There is an in-memory cache and dynamic profile info is coming from there
                if (newDynamicProfileInfo == null)
                {
                    newDynamicProfileInfo = DynamicProfileInfo::New(this->GetRecycler(), functionBody, true /* persistsAcrossScriptContexts */);
                    profileManager->UpdateDynamicProfileInfo(functionBody->GetLocalFunctionId(), newDynamicProfileInfo);
                    *dynamicProfileInfo = newDynamicProfileInfo;
                }
                profileManager->MarkAsExecuted(functionBody->GetLocalFunctionId());
                newDynamicProfileInfo->UpdateFunctionInfo(functionBody, this->GetRecycler());
            }
            else
            {
                if (newDynamicProfileInfo == null)
                {
                    newDynamicProfileInfo = functionBody->AllocateDynamicProfile();
                }
                *dynamicProfileInfo = newDynamicProfileInfo;
            }
        }
        else
        {
            if (newDynamicProfileInfo == null)
            {
                newDynamicProfileInfo = functionBody->AllocateDynamicProfile();
                *dynamicProfileInfo = newDynamicProfileInfo;
            }
            Assert(functionBody->interpretedCount == 0);
#if DBG_DUMP || defined(DYNAMIC_PROFILE_STORAGE) || defined(RUNTIME_DATA_COLLECTION)
            if (profileInfoList)
            {
                profileInfoList->Prepend(this->GetRecycler(), newDynamicProfileInfo);
            }
#endif
            if (!startupComplete)
            {
                Assert(profileManager);
                profileManager->MarkAsExecuted(functionBody->GetLocalFunctionId());
            }
        }
        Assert(*dynamicProfileInfo != null);
    }

    CharClassifier const * ScriptContext::GetCharClassifier(void) const
    {
        return this->charClassifier;
    }

    void ScriptContext::OnStartupComplete()
    {
        // Uncomment the assert below once bug 522912 is fixed.
        //AssertMsg(!startupComplete, "Startup complete - invoked twice?");
        JSETW(EventWriteJSCRIPT_ON_STARTUP_COMPLETE(this));

        SaveStartupProfileAndRelease();
    }

    void ScriptContext::SaveStartupProfileAndRelease(bool isSaveOnClose)
    {
        if (!startupComplete && this->cache->sourceContextInfoMap)
        {
            this->cache->sourceContextInfoMap->Map([&](DWORD_PTR dwHostSourceContext, SourceContextInfo* info)
            {
                Assert(info->sourceDynamicProfileManager);
                uint bytesWritten = info->sourceDynamicProfileManager->SaveToProfileCacheAndRelease(info);
                if (bytesWritten > 0)
                {
                    JSETW(EventWriteJSCRIPT_PROFILE_SAVE(info->dwHostSourceContext, this, bytesWritten, isSaveOnClose));
                    OUTPUT_TRACE(Js::DynamicProfilePhase, L"Profile saving succeeded\n");
                }
            });
        }
        startupComplete = true;
    }

    void ScriptContextBase::ClearGlobalObject()
    {
#if ENABLE_NATIVE_CODEGEN
        ScriptContext* scriptContext = static_cast<ScriptContext*>(this);
        Assert(scriptContext->IsClosedNativeCodeGenerator());
#endif
        globalObject = null;
        javascriptLibrary = null;
    }

    void ScriptContext::SetFastDOMenabled()
    {
        fastDOMenabled = true; Assert(globalObject->GetDirectHostObject() != NULL);
    }

    JavascriptMethod ScriptContext::GetNextDynamicAsmJsInterpreterThunk(PVOID* ppDynamicInterpreterThunk)
    {
#ifdef ASMJS_PLAT
        return (JavascriptMethod)this->asmJsInterpreterThunkEmitter->GetNextThunk(ppDynamicInterpreterThunk);
#else
        __debugbreak();
        return nullptr;
#endif
    }

    JavascriptMethod ScriptContext::GetNextDynamicInterpreterThunk(PVOID* ppDynamicInterpreterThunk)
    {
        return (JavascriptMethod)this->interpreterThunkEmitter->GetNextThunk(ppDynamicInterpreterThunk);
    }

    BOOL ScriptContext::IsDynamicInterpreterThunk(void* address)
    {
        return this->interpreterThunkEmitter->IsInRange(address);
    }

    void ScriptContext::ReleaseDynamicInterpreterThunk(BYTE* address, bool addtoFreeList)
    {
        this->interpreterThunkEmitter->Release(address, addtoFreeList);
    }

    bool ScriptContext::IsExceptionWrapperForBuiltInsEnabled()
    {
        return ScriptContext::IsExceptionWrapperForBuiltInsEnabled(this);
    }

    // static
    bool ScriptContext::IsExceptionWrapperForBuiltInsEnabled(ScriptContext* scriptContext)
    {
        Assert(scriptContext);
        return CONFIG_FLAG(EnableContinueAfterExceptionWrappersForBuiltIns);
    }

    bool ScriptContext::IsExceptionWrapperForHelpersEnabled(ScriptContext* scriptContext)
    {
        Assert(scriptContext);
        return  CONFIG_FLAG(EnableContinueAfterExceptionWrappersForHelpers);
    }

    void ScriptContextBase::SetGlobalObject(GlobalObject *globalObject)
    {
#if DBG
        ScriptContext* scriptContext = static_cast<ScriptContext*>(this);
        Assert(scriptContext->IsCloningGlobal() && !this->globalObject);
#endif
        this->globalObject = globalObject;
    }

    void ConvertKey(const FastEvalMapString& src, EvalMapString& dest)
    {
        dest.str = src.str;
        dest.strict = src.strict;
        dest.moduleID = src.moduleID;
        dest.hash = TAGHASH((hash_t)dest.str);
    }

    void ScriptContext::PrintStats()
    {
#if DBG_DUMP
        DynamicProfileInfo::DumpScriptContext(this);
#endif
#ifdef RUNTIME_DATA_COLLECTION
        DynamicProfileInfo::DumpScriptContextToFile(this);
#endif
#ifdef PROFILE_TYPES
        if (Configuration::Global.flags.ProfileTypes)
        {
            ProfileTypes();
        }
#endif

#ifdef PROFILE_OBJECT_LITERALS
        if (Configuration::Global.flags.ProfileObjectLiteral)
        {
            ProfileObjectLiteral();
        }
#endif

#ifdef PROFILE_STRINGS
        if (stringProfiler != null)
        {
            stringProfiler->PrintAll();
            Adelete(MiscAllocator(), stringProfiler);
            stringProfiler = null;
        }
#endif

#ifdef ARRLOG
        PrintArrLog(logTable);
#endif

#ifdef PROFILE_MEM
        if (profileMemoryDump && MemoryProfiler::IsTraceEnabled() && !BinaryFeatureControl::LanguageService())
        {
            MemoryProfiler::PrintAll();
#ifdef PROFILE_RECYCLER_ALLOC
            if (Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::AllPhase)
                || Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::RunPhase))
            {
                GetRecycler()->PrintAllocStats();
            }
#endif
        }
#endif
#if DBG_DUMP
        if (PHASE_STATS1(Js::ByteCodePhase))
        {
            Output::Print(L" Total Bytecode size: <%d, %d, %d> = %d\n",
                byteCodeDataSize,
                byteCodeAuxiliaryDataSize,
                byteCodeAuxiliaryContextDataSize,
                byteCodeDataSize + byteCodeAuxiliaryDataSize + byteCodeAuxiliaryContextDataSize);
        }

        if (Configuration::Global.flags.BytecodeHist)
        {
            Output::Print(L"ByteCode Histogram\n");
            Output::Print(L"\n");

            uint total = 0;
            uint unique = 0;
            for (int j = 0; j < (int)OpCode::ByteCodeLast; j++)
            {
                total += byteCodeHistogram[j];
                if (byteCodeHistogram[j] > 0)
                {
                    unique++;
                }
            }
            Output::Print(L"%9u                     Total executed ops\n", total);
            Output::Print(L"\n");

            uint max = UINT_MAX;
            double pctcume = 0.0;

            while (true)
            {
                uint upper = 0;
                int index = -1;
                for (int j = 0; j < (int)OpCode::ByteCodeLast; j++)
                {
                    if (OpCodeUtil::IsValidOpcode((OpCode)j) && byteCodeHistogram[j] > upper && byteCodeHistogram[j] < max)
                    {
                        index = j;
                        upper = byteCodeHistogram[j];
                    }
                }

                if (index == -1)
                {
                    break;
                }

                max = byteCodeHistogram[index];

                for (OpCode j = (OpCode)0; j < OpCode::ByteCodeLast; j++)
                {
                    if (OpCodeUtil::IsValidOpcode(j) && max == byteCodeHistogram[(int)j])
                    {
                        double pct = ((double)max) / total;
                        pctcume += pct;

                        Output::Print(L"%9u  %5.1lf  %5.1lf  %04x %s\n", max, pct * 100, pctcume * 100, j, OpCodeUtil::GetOpCodeName(j));
                    }
                }
            }
            Output::Print(L"\n");
            Output::Print(L"Unique opcodes: %d\n", unique);
        }

#endif

#ifdef BGJIT_STATS
        // We do not care about small script contexts without much activity - unless t
        if (PHASE_STATS1(Js::BGJitPhase) && (this->interpretedCount > 50 || Js::Configuration::Global.flags.IsEnabled(Js::ForceFlag)))
        {
            uint loopJitCodeUsed = 0;
            uint bucketSize1 = 20;
            uint bucketSize2 = 100;
            uint size1CutOffbucketId = 4;
            uint totalBuckets[15] = { 0 };
            uint nativeCodeBuckets[15] = { 0 };
            uint usedNativeCodeBuckets[15] = { 0 };
            uint rejits[15] = { 0 };
            uint zeroInterpretedFunctions = 0;
            uint oneInterpretedFunctions = 0;
            uint nonZeroBytecodeFunctions = 0;
            Output::Print(L"Script Context: 0x%p Url: %s\n", this, this->url);

            FunctionBody* anyFunctionBody = this->FindFunction([](FunctionBody* body) { return body != null; });

            if (anyFunctionBody)
            {
                OUTPUT_VERBOSE_STATS(Js::BGJitPhase, L"Function list\n");
                OUTPUT_VERBOSE_STATS(Js::BGJitPhase, L"===============================\n");
                OUTPUT_VERBOSE_STATS(Js::BGJitPhase, L"%-24s, %-8s, %-10s, %-10s, %-10s, %-10s, %-10s\n", L"Function", L"InterpretedCount", L"ByteCodeInLoopSize", L"ByteCodeSize", L"IsJitted", L"IsUsed", L"NativeCodeSize");

                this->MapFunction([&](FunctionBody* body)
                {
                    bool isNativeCode = false;

                    // Filtering interpreted count lowers a lot of noise
                    if (body->interpretedCount > 1 || Js::Configuration::Global.flags.IsEnabled(Js::ForceFlag))
                    {
                        body->MapEntryPoints([&](uint entryPointIndex, FunctionEntryPointInfo* entryPoint)
                        {
                            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
                            char rejit = entryPointIndex > 0 ? '*' : ' ';
                            isNativeCode = entryPoint->IsNativeCode() | isNativeCode;
                            OUTPUT_VERBOSE_STATS(Js::BGJitPhase, L"%-20s %16s %c, %8d , %10d , %10d, %-10s, %-10s, %10d\n",
                                body->GetExternalDisplayName(),
                                body->GetDebugNumberSet(debugStringBuffer),
                                rejit,
                                body->interpretedCount,
                                body->GetByteCodeInLoopCount(),
                                body->GetByteCodeCount(),
                                entryPoint->IsNativeCode() ? L"Jitted" : L"Interpreted",
                                body->GetNativeEntryPointUsed() ? L"Used" : L"NotUsed",
                                entryPoint->IsNativeCode() ? entryPoint->GetCodeSize() : 0);
                        });
                    }
                    if (body->interpretedCount == 0)
                    {
                        zeroInterpretedFunctions++;
                        if (body->GetByteCodeCount() > 0)
                        {
                            nonZeroBytecodeFunctions++;
                        }
                    }
                    else if (body->interpretedCount == 1)
                    {
                        oneInterpretedFunctions++;
                    }


                    // Generate a histogram using interpreted counts.
                    uint bucket;
                    uint intrpCount = body->interpretedCount;
                    if (intrpCount < 100)
                    {
                        bucket = intrpCount / bucketSize1;
                    }
                    else if (intrpCount < 1000)
                    {
                        bucket = size1CutOffbucketId  + intrpCount / bucketSize2;
                    }
                    else
                    {
                        bucket = _countof(totalBuckets) - 1;
                    }

                    // Explicitly assume that the bucket count is less than the following counts (which are all equal)
                    // This is because min will return _countof(totalBuckets) - 1 if the count exceeds _countof(totalBuckets) - 1.
                    __analysis_assume(bucket < _countof(totalBuckets));
                    __analysis_assume(bucket < _countof(nativeCodeBuckets));
                    __analysis_assume(bucket < _countof(usedNativeCodeBuckets));
                    __analysis_assume(bucket < _countof(rejits));

                    totalBuckets[bucket]++;
                    if (isNativeCode)
                    {
                        nativeCodeBuckets[bucket]++;
                        if (body->GetNativeEntryPointUsed())
                        {
                            usedNativeCodeBuckets[bucket]++;
                        }
                        if (body->HasRejit())
                        {
                            rejits[bucket]++;
                        }
                    }

                    body->MapLoopHeaders([&](uint loopNumber, LoopHeader* header)
                    {
                        wchar_t loopBodyName[256];
                        EtwTrace::GetLoopBodyName(body, header, loopBodyName, _countof(loopBodyName));
                        header->MapEntryPoints([&](int index, LoopEntryPointInfo * entryPoint)
                        {
                            if (entryPoint->IsNativeCode())
                            {
                                wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
                                char rejit = index > 0 ? '*' : ' ';
                                OUTPUT_VERBOSE_STATS(Js::BGJitPhase, L"%-20s %16s %c, %8d , %10d , %10d, %-10s, %-10s, %10d\n",
                                    loopBodyName,
                                    body->GetDebugNumberSet(debugStringBuffer),
                                    rejit,
                                    header->interpretCount,
                                    header->GetByteCodeCount(),
                                    header->GetByteCodeCount(),
                                    L"Jitted",
                                    entryPoint->IsUsed() ? L"Used" : L"NotUsed",
                                    entryPoint->GetCodeSize());
                                if (entryPoint->IsUsed())
                                {
                                    loopJitCodeUsed++;
                                }
                            }
                        });
                    });
                });
            }

            Output::Print(L"**  SpeculativelyJitted: %6d FunctionsJitted: %6d JittedUsed: %6d Usage:%f ByteCodesJitted: %6d JitCodeUsed: %6d Usage: %f \n",
                speculativeJitCount, funcJITCount, funcJitCodeUsed, ((float)(funcJitCodeUsed) / funcJITCount) * 100, bytecodeJITCount, jitCodeUsed, ((float)(jitCodeUsed) / bytecodeJITCount) * 100);
            Output::Print(L"** LoopJITCount: %6d LoopJitCodeUsed: %6d Usage: %f\n",
                loopJITCount, loopJitCodeUsed, ((float)loopJitCodeUsed / loopJITCount) * 100);
            Output::Print(L"** TotalInterpretedCalls: %6d MaxFuncInterp: %6d  InterpretedHighPri: %6d \n",
                interpretedCount, maxFuncInterpret, interpretedCallsHighPri);
            Output::Print(L"** ZeroInterpretedFunctions: %6d OneInterpretedFunctions: %6d ZeroInterpretedWithNonZeroBytecode: %6d \n ", zeroInterpretedFunctions, oneInterpretedFunctions, nonZeroBytecodeFunctions);
            Output::Print(L"** %-24s : %-10s %-10s %-10s %-10s %-10s\n", L"InterpretedCounts", L"Total", L"NativeCode", L"Used", L"Usage", L"Rejits");
            uint low = 0;
            uint high = 0;
            for (uint i = 0; i < _countof(totalBuckets); i++)
            {
                low = high;
                if (i <= size1CutOffbucketId)
                {
                    high = low + bucketSize1;
                }
                else if (i < (_countof(totalBuckets) - 1))
                {
                    high = low + bucketSize2;               }
                else
                {
                    high = 100000;
                }
                Output::Print(L"** %10d - %10d : %10d %10d %10d %7.2f %10d\n", low, high, totalBuckets[i], nativeCodeBuckets[i], usedNativeCodeBuckets[i], ((float)usedNativeCodeBuckets[i] / nativeCodeBuckets[i]) * 100, rejits[i]);
            }
            Output::Print(L"\n\n");
        }
#endif

#ifdef REJIT_STATS
        if (PHASE_STATS1(Js::ReJITPhase))
        {
            uint totalBailouts = 0;
            uint totalRejits = 0;
            WCHAR buf[256];

            // Dump bailout data.
            Output::Print(L"%-40s %6s\n", L"Bailout Reason,", L"Count");

            bailoutReasonCounts->Map([&totalBailouts](uint kind, uint val) {
                WCHAR buf[256];
                totalBailouts += val;
                if (val != 0)
                {
                    swprintf_s(buf, L"%S,", GetBailOutKindName((IR::BailOutKind)kind));
                    Output::Print(L"%-40s %6d\n", buf, val);
                }
            });


            Output::Print(L"%-40s %6d\n", L"TOTAL,", totalBailouts);
            Output::Print(L"\n\n");

            // Dump rejit data.
            Output::Print(L"%-40s %6s\n", L"Rejit Reason,", L"Count");
            for (uint i = 0; i < NumRejitReasons; ++i)
            {
                totalRejits += rejitReasonCounts[i];
                if (rejitReasonCounts[i] != 0)
                {
                    swprintf_s(buf, L"%S,", RejitReasonNames[i]);
                    Output::Print(L"%-40s %6d\n", buf, rejitReasonCounts[i]);
                }
            }
            Output::Print(L"%-40s %6d\n", L"TOTAL,", totalRejits);
            Output::Print(L"\n\n");

            // If in verbose mode, dump data for each FunctionBody
            if (Configuration::Global.flags.Verbose && rejitStatsMap != NULL)
            {
                // Aggregated data
                Output::Print(L"%-30s %14s %14s\n", L"Function (#),", L"Bailout Count,", L"Rejit Count");
                rejitStatsMap->Map([](Js::FunctionBody const *body, RejitStats *stats, RecyclerWeakReference<const Js::FunctionBody> const*) {
                    wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
                    for (uint i = 0; i < NumRejitReasons; ++i)
                        stats->m_totalRejits += stats->m_rejitReasonCounts[i];

                    stats->m_bailoutReasonCounts->Map([stats](uint kind, uint val) {
                        stats->m_totalBailouts += val;
                    });

                    WCHAR buf[256];

                    swprintf_s(buf, L"%s (%s),", body->GetExternalDisplayName(), (const_cast<Js::FunctionBody*>(body))->GetDebugNumberSet(debugStringBuffer)); //TODO Kount
                    Output::Print(L"%-30s %14d, %14d\n", buf, stats->m_totalBailouts, stats->m_totalRejits);

                });
                Output::Print(L"\n\n");

                // Per FunctionBody data
                rejitStatsMap->Map([](Js::FunctionBody const *body, RejitStats *stats, RecyclerWeakReference<const Js::FunctionBody> const *) {
                    wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
                    WCHAR buf[256];

                    swprintf_s(buf, L"%s (%s),", body->GetExternalDisplayName(), (const_cast<Js::FunctionBody*>(body))->GetDebugNumberSet(debugStringBuffer)); //TODO Kount
                    Output::Print(L"%-30s\n\n", buf);

                    // Dump bailout data
                    if (stats->m_totalBailouts != 0)
                    {
                        Output::Print(L"%10sBailouts:\n", L"");

                        stats->m_bailoutReasonCounts->Map([](uint kind, uint val) {
                            if (val != 0)
                            {
                                WCHAR buf[256];
                                swprintf_s(buf, L"%S,", GetBailOutKindName((IR::BailOutKind)kind));
                                Output::Print(L"%10s%-40s %6d\n", L"", buf, val);
                            }
                        });
                    }
                    Output::Print(L"\n");

                    // Dump rejit data.
                    if (stats->m_totalRejits != 0)
                    {
                        Output::Print(L"%10sRejits:\n", L"");
                        for (uint i = 0; i < NumRejitReasons; ++i)
                        {
                            if (stats->m_rejitReasonCounts[i] != 0)
                            {
                                swprintf_s(buf, L"%S,", RejitReasonNames[i]);
                                Output::Print(L"%10s%-40s %6d\n", L"", buf, stats->m_rejitReasonCounts[i]);
                            }
                        }
                        Output::Print(L"\n\n");
                    }
                });

            }
        }
#endif

#ifdef TELEMETRY
    {
        this->GetTelemetry().OutputTelemetry();
    }
#endif

#ifdef FIELD_ACCESS_STATS
    if (PHASE_STATS1(Js::ObjTypeSpecPhase))
    {
        FieldAccessStats globalStats;
        if (this->fieldAccessStatsByFunctionNumber != null)
        {
            this->fieldAccessStatsByFunctionNumber->Map([&globalStats](uint functionNumber, FieldAccessStatsEntry* entry)
            {
                FieldAccessStats functionStats;
                entry->stats.Map([&functionStats](FieldAccessStatsPtr entryPointStats)
                {
                    functionStats.Add(entryPointStats);
                });

                if (PHASE_VERBOSE_STATS1(Js::ObjTypeSpecPhase))
                {
                    FunctionBody* functionBody = entry->functionBodyWeakRef->Get();
                    const wchar_t* functionName = functionBody != null ? functionBody->GetDisplayName() : L"<unknown>";
                    Output::Print(L"FieldAccessStats: function %s (#%u): inline cache stats:\n", functionName, functionNumber);
                    Output::Print(L"    overall: total %u, no profile info %u\n", functionStats.totalInlineCacheCount, functionStats.noInfoInlineCacheCount);
                    Output::Print(L"    mono: total %u, empty %u, cloned %u\n",
                        functionStats.monoInlineCacheCount, functionStats.emptyMonoInlineCacheCount, functionStats.clonedMonoInlineCacheCount);
                    Output::Print(L"    poly: total %u (high %u, low %u), null %u, empty %u, ignored %u, disabled %u, equivalent %u, non-equivalent %u, cloned %u\n",
                        functionStats.polyInlineCacheCount, functionStats.highUtilPolyInlineCacheCount, functionStats.lowUtilPolyInlineCacheCount,
                        functionStats.nullPolyInlineCacheCount, functionStats.emptyPolyInlineCacheCount, functionStats.ignoredPolyInlineCacheCount, functionStats.disabledPolyInlineCacheCount,
                        functionStats.equivPolyInlineCacheCount, functionStats.nonEquivPolyInlineCacheCount, functionStats.clonedPolyInlineCacheCount);
                }

                globalStats.Add(&functionStats);
            });
        }

        Output::Print(L"FieldAccessStats: totals\n");
        Output::Print(L"    overall: total %u, no profile info %u\n", globalStats.totalInlineCacheCount, globalStats.noInfoInlineCacheCount);
        Output::Print(L"    mono: total %u, empty %u, cloned %u\n",
            globalStats.monoInlineCacheCount, globalStats.emptyMonoInlineCacheCount, globalStats.clonedMonoInlineCacheCount);
        Output::Print(L"    poly: total %u (high %u, low %u), null %u, empty %u, ignored %u, disabled %u, equivalent %u, non-equivalent %u, cloned %u\n",
            globalStats.polyInlineCacheCount, globalStats.highUtilPolyInlineCacheCount, globalStats.lowUtilPolyInlineCacheCount,
            globalStats.nullPolyInlineCacheCount, globalStats.emptyPolyInlineCacheCount, globalStats.ignoredPolyInlineCacheCount, globalStats.disabledPolyInlineCacheCount,
            globalStats.equivPolyInlineCacheCount, globalStats.nonEquivPolyInlineCacheCount, globalStats.clonedPolyInlineCacheCount);
    }
#endif

#ifdef MISSING_PROPERTY_STATS
    if (PHASE_STATS1(Js::MissingPropertyCachePhase))
    {
        Output::Print(L"MissingPropertyStats: hits = %d, misses = %d, cache attempts = %d.\n",
            this->missingPropertyHits, this->missingPropertyMisses, this->missingPropertyCacheAttempts);
    }
#endif


#ifdef INLINE_CACHE_STATS
        if (PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            Output::Print(L"%s,%s,%s,%s,%s,%s,%s,%s,%s\n", L"Function", L"Property", L"Kind", L"Accesses", L"Misses", L"Miss Rate", L"Collisions", L"Collision Rate", L"Slot Count");
            cacheDataMap->Map([this](Js::PolymorphicInlineCache const *cache, CacheData *data) {
                wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
                uint total = data->hits + data->misses;
                wchar_t const *propName = this->threadContext->GetPropertyName(data->propertyId)->GetBuffer();

                wchar funcName[1024];

                swprintf_s(funcName, L"%s (%s)", cache->functionBody->GetExternalDisplayName(), cache->functionBody->GetDebugNumberSet(debugStringBuffer));

                Output::Print(L"%s,%s,%s,%d,%d,%f,%d,%f,%d\n",
                    funcName,
                    propName,
                    data->isGetCache ? L"get" : L"set",
                    total,
                    data->misses,
                    static_cast<float>(data->misses) / total,
                    data->collisions,
                    static_cast<float>(data->collisions) / total,
                    cache->GetSize()
                    );
            });
        }
#endif

#if ENABLE_REGEX_CONFIG_OPTIONS
        if (regexStatsDatabase != 0)
            regexStatsDatabase->Print(GetRegexDebugWriter());
#endif
        OUTPUT_STATS(Js::EmitterPhase, L"Script Context: 0x%p Url: %s\n", this, this->url);
        OUTPUT_STATS(Js::EmitterPhase, L"  Total thread committed code size = %d\n", this->GetThreadContext()->GetCodeSize());

        OUTPUT_STATS(Js::ParsePhase, L"Script Context: 0x%p Url: %s\n", this, this->url);
        OUTPUT_STATS(Js::ParsePhase, L"  Total ThreadContext source size %d\n", this->GetThreadContext()->GetSourceSize());

        Output::Flush();
    }
    void ScriptContext::SetNextPendingClose(ScriptContext * nextPendingClose) {
        Assert(this->nextPendingClose == nullptr && nextPendingClose != nullptr);
        this->nextPendingClose = nextPendingClose;
    }

    bool ScriptContext::HasMutationBreakpoints()
    {
        return this->diagProbesContainer.HasMutationBreakpoints();
    }

    void ScriptContext::InsertMutationBreakpoint(Js::MutationBreakpoint *mutationBreakpoint)
    {
        this->diagProbesContainer.InsertMutationBreakpoint(mutationBreakpoint);
    }

#ifdef REJIT_STATS
    void ScriptContext::LogDataForFunctionBody(Js::FunctionBody *body, uint idx, bool isRejit)
    {
        if (rejitStatsMap == NULL)
        {
            rejitStatsMap = RecyclerNew(this->recycler, RejitStatsMap, this->recycler);
            BindReference(rejitStatsMap);
        }

        RejitStats *stats = NULL;
        if (!rejitStatsMap->TryGetValue(body, &stats))
        {
            stats = Anew(GeneralAllocator(), RejitStats, this);
            rejitStatsMap->Item(body, stats);
        }

        if (isRejit)
        {
            stats->m_rejitReasonCounts[idx]++;
        }
        else
        {
            if (!stats->m_bailoutReasonCounts->ContainsKey(idx))
            {
                stats->m_bailoutReasonCounts->Item(idx, 1);
            }
            else
            {
                uint val = stats->m_bailoutReasonCounts->Item(idx);
                ++val;
                stats->m_bailoutReasonCounts->Item(idx, val);
            }
        }
    }
    void ScriptContext::LogRejit(Js::FunctionBody *body, uint reason)
    {
        Assert(reason < NumRejitReasons);
        rejitReasonCounts[reason]++;

        if (Js::Configuration::Global.flags.Verbose)
        {
            LogDataForFunctionBody(body, reason, true);
        }
    }
    void ScriptContext::LogBailout(Js::FunctionBody *body, uint kind)
    {
        if (!bailoutReasonCounts->ContainsKey(kind))
        {
            bailoutReasonCounts->Item(kind, 1);
        }
        else
        {
            uint val = bailoutReasonCounts->Item(kind);
            ++val;
            bailoutReasonCounts->Item(kind, val);
        }

        if (Js::Configuration::Global.flags.Verbose)
        {
            LogDataForFunctionBody(body, kind, false);
        }
    }
#endif

#ifdef TELEMETRY
    ScriptContextTelemetry& ScriptContext::GetTelemetry()
    {
        return *this->telemetry;
    }
#endif

    // Sets the specified mode for the debugger.  The mode is used to inform
    // the runtime of whether or not functions should be JITed or interpreted
    // when they are defer parsed.
    // Note: Transitions back to NotDebugging are not allowed.  Once the debugger
    // is in SourceRundown or Debugging mode, it can only transition between those
    // two modes.
    void ScriptContext::SetDebuggerMode(DebuggerMode mode)
    {
        if (this->debuggerMode == mode)
        {
            // Already in this mode so return.
            return;
        }

        if (mode == DebuggerMode::NotDebugging)
        {
            AssertMsg(false, "Transitioning to non-debug mode is not allowed.");
            return;
        }

        this->debuggerMode = mode;
    }


#ifdef INLINE_CACHE_STATS
    void ScriptContext::LogCacheUsage(Js::PolymorphicInlineCache *cache, bool isGetter, Js::PropertyId propertyId, bool hit, bool collision)
    {
        if (cacheDataMap == NULL)
        {
            cacheDataMap = RecyclerNew(this->recycler, CacheDataMap, this->recycler);
            BindReference(cacheDataMap);
        }

        CacheData *data = NULL;
        if (!cacheDataMap->TryGetValue(cache, &data))
        {
            data = Anew(GeneralAllocator(), CacheData);
            cacheDataMap->Item(cache, data);
            data->isGetCache = isGetter;
            data->propertyId = propertyId;
        }

        Assert(data->isGetCache == isGetter);
        Assert(data->propertyId == propertyId);

        if (hit)
        {
            data->hits++;
        }
        else
        {
            data->misses++;
        }
        if (collision)
        {
            data->collisions++;
        }
    }
#endif

#ifdef FIELD_ACCESS_STATS
    void ScriptContext::RecordFieldAccessStats(FunctionBody* functionBody, FieldAccessStatsPtr fieldAccessStats)
    {
        Assert(fieldAccessStats != null);

        if (!PHASE_STATS1(Js::ObjTypeSpecPhase))
        {
            return;
        }

        FieldAccessStatsEntry* entry;
        if (!this->fieldAccessStatsByFunctionNumber->TryGetValue(functionBody->GetFunctionNumber(), &entry))
        {
            RecyclerWeakReference<FunctionBody>* functionBodyWeakRef;
            this->recycler->FindOrCreateWeakReferenceHandle(functionBody, &functionBodyWeakRef);
            entry = RecyclerNew(this->recycler, FieldAccessStatsEntry, functionBodyWeakRef, this->recycler);

            this->fieldAccessStatsByFunctionNumber->AddNew(functionBody->GetFunctionNumber(), entry);
        }

        entry->stats.Prepend(fieldAccessStats);
    }
#endif

#ifdef MISSING_PROPERTY_STATS
    void ScriptContext::RecordMissingPropertyMiss()
    {
        this->missingPropertyMisses++;
    }

    void ScriptContext::RecordMissingPropertyHit()
    {
        this->missingPropertyHits++;
    }

    void ScriptContext::RecordMissingPropertyCacheAttempt()
    {
        this->missingPropertyCacheAttempts++;
    }
#endif

    bool ScriptContext::TryGetNativeModule(BYTE *moduleBase, NativeModule **nativeModule)
    {
        return nativeModules && nativeModules->TryGetValue(moduleBase, nativeModule);
    }

    bool ScriptContext::IsIntConstPropertyOnGlobalObject(Js::PropertyId propId)
    {
        return intConstPropsOnGlobalObject->ContainsKey(propId);
    }

    void ScriptContext::TrackIntConstPropertyOnGlobalObject(Js::PropertyId propertyId)
    {
        intConstPropsOnGlobalObject->AddNew(propertyId);
    }

    bool ScriptContext::IsIntConstPropertyOnGlobalUserObject(Js::PropertyId propertyId)
    {
        return intConstPropsOnGlobalUserObject->ContainsKey(propertyId) != NULL;
    }

    void ScriptContext::TrackIntConstPropertyOnGlobalUserObject(Js::PropertyId propertyId)
    {
        intConstPropsOnGlobalUserObject->AddNew(propertyId);
    }

    void ScriptContext::AddNativeModule(BYTE *moduleBase, NativeModule *nativeModule)
    {
        if (!nativeModules || !nativeModules->ContainsKey(moduleBase))
        {
            if (!nativeModules)
            {
                nativeModules = new NativeModuleMap(&HeapAllocator::Instance);
            }
            nativeModules->Add(moduleBase, nativeModule);
        }
    }

    void ScriptContext::AddCalleeSourceInfoToList(Utf8SourceInfo* sourceInfo)
    {
        Assert(sourceInfo);

        RecyclerWeakReference<Js::Utf8SourceInfo>* sourceInfoWeakRef = nullptr;
        this->GetRecycler()->FindOrCreateWeakReferenceHandle(sourceInfo, &sourceInfoWeakRef);
        Assert(sourceInfoWeakRef);

        if (!calleeUtf8SourceInfoList)
        {
            Recycler *recycler = this->GetRecycler();
            calleeUtf8SourceInfoList.Root(RecyclerNew(recycler, CalleeSourceList, recycler), recycler);
        }

        if (!calleeUtf8SourceInfoList->Contains(sourceInfoWeakRef))
        {
            calleeUtf8SourceInfoList->Add(sourceInfoWeakRef);
        }
    }

    void ScriptContext::EmitStackTraceEvent(__in UINT64 operationID, __in USHORT maxFrameCount, bool emitV2AsyncStackEvent)
    {
        // If call root level is zero, there is no EntryExitRecord and the stack walk will fail.
        if (GetThreadContext()->GetCallRootLevel() == 0)
        {
            return;
        }

        Assert(EventEnabledJSCRIPT_STACKTRACE() || EventEnabledJSCRIPT_ASYNCCAUSALITY_STACKTRACE_V2() || PHASE_TRACE1(Js::StackFramesEventPhase));
        BEGIN_TEMP_ALLOCATOR(tempAllocator, this, L"StackTraceEvent")
        {
            JsUtil::List<StackFrameInfo, ArenaAllocator> stackFrames(tempAllocator);
            Js::JavascriptStackWalker walker(this);
            unsigned short nameBufferLength = 0;
            Js::StringBuilder<ArenaAllocator> nameBuffer(tempAllocator);
            nameBuffer.Reset();

            OUTPUT_TRACE(Js::StackFramesEventPhase, L"\nPosting stack trace via ETW:\n");

            ushort frameCount = walker.WalkUntil((ushort)maxFrameCount, [&](Js::JavascriptFunction* function, ushort frameIndex) -> bool
            {
                ULONG lineNumber = 0;
                LONG columnNumber = 0;
                UINT32 methodIdOrNameId = 0;
                UINT8 isFrameIndex = 0; // FALSE
                const WCHAR* name = nullptr;
                if (function->IsScriptFunction() && !function->IsLibraryCode())
                {
                    Js::FunctionBody * functionBody = function->GetFunctionBody();
                    functionBody->GetLineCharOffset(walker.GetByteCodeOffset(), &lineNumber, &columnNumber);
                    methodIdOrNameId = EtwTrace::GetFunctionId(functionBody);
                    name = functionBody->GetExternalDisplayName();
                }
                else
                {
                    if (function->IsScriptFunction())
                    {
                        name = function->GetFunctionBody()->GetExternalDisplayName();
                    }
                    else
                    {
                        name = walker.GetCurrentNativeLibraryEntryName();
                    }

                    ushort nameLen = AsyncDebug::ProcessNameAndGetLength(&nameBuffer, name);

                    methodIdOrNameId = nameBufferLength;

                    // Keep track of the current length of the buffer. The next nameIndex will be at this position (+1 for each '\\', '\"', and ';' character added above).
                    nameBufferLength += nameLen;

                    isFrameIndex = 1; // TRUE;
                }

                StackFrameInfo frame((DWORD_PTR)function->GetScriptContext(),
                    (UINT32)lineNumber,
                    (UINT32)columnNumber,
                    methodIdOrNameId,
                    isFrameIndex);

                OUTPUT_TRACE(Js::StackFramesEventPhase, L"Frame : (%s : %u) (%s), LineNumber : %u, ColumnNumber : %u\n",
                    (isFrameIndex == 1) ? (L"NameBufferIndex") : (L"MethodID"),
                    methodIdOrNameId,
                    name,
                    lineNumber,
                    columnNumber);

                stackFrames.Add(frame);

                return false;
            });

            Assert(frameCount == (ushort)stackFrames.Count());

            if (frameCount > 0) // No need to emit event if there are no script frames.
            {
                auto nameBufferString = nameBuffer.Detach();

                if (nameBufferLength > 0)
                {
                    // Account for the terminating null character.
                    nameBufferLength++;
                }

                if (emitV2AsyncStackEvent)
                {
                    JSETW(EventWriteJSCRIPT_ASYNCCAUSALITY_STACKTRACE_V2(operationID, frameCount, nameBufferLength, sizeof(StackFrameInfo), &stackFrames.Item(0), nameBufferString));
                }
                else
                {
                    JSETW(EventWriteJSCRIPT_STACKTRACE(operationID, frameCount, nameBufferLength, sizeof(StackFrameInfo), &stackFrames.Item(0), nameBufferString));
                }
            }
        }
        END_TEMP_ALLOCATOR(tempAllocator, this);

        OUTPUT_FLUSH();
    }

    bool ScriptConfiguration::IsIntlEnabled() const
    {
        // Intl is never enabled in the Language Service. The Language service includes Intl.js directly as a reference file and
        // initializes the Intl object with a mock version of the EngineInterfaceObject implemented in JS in IntlHelpers.js.
        // The Language Service should never use the actual runtime Intl object.
        return !BinaryFeatureControl::LanguageService() && Js::Configuration::Global.flags.Intl;
    }

} // End namespace Js

SRCINFO* SRCINFO::Clone(Js::ScriptContext* scriptContext) const
{
    SRCINFO* srcInfo;
    if (this->sourceContextInfo->dwHostSourceContext == Js::Constants::NoHostSourceContext  &&
        this->dlnHost == 0 && this->ulColumnHost == 0 && this->ulCharOffset == 0 &&
        this->ichMinHost == 0 && this->ichLimHost == 0 && this->grfsi == 0)
    {
        srcInfo = const_cast<SRCINFO*>(scriptContext->GetModuleSrcInfo(this->moduleID));
    }
    else
    {
        SourceContextInfo* sourceContextInfo = this->sourceContextInfo->Clone(scriptContext);
        srcInfo = SRCINFO::Copy(scriptContext->GetRecycler(), this);
        srcInfo->sourceContextInfo = sourceContextInfo;
    }
    return srcInfo;
}
