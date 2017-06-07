//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"
#include "Library\ModuleRoot.h"
#include "JavascriptTypedObjectSlotAccessorFunction.h"
#include "DOMFastPathInfo.h"
#include "ChakraHostScriptContext.h"
#include "ChakraHostDebugContext.h"
#include "Types\DeferredTypeHandler.h"
#include "Types\PathTypeHandler.h"
#include "Types\PropertyIndexRanges.h"
#include "Types\SimpleDictionaryPropertyDescriptor.h"
#include "Types\SimpleDictionaryTypeHandler.h"
#include "Library\ES5Array.h"
#include "ActiveScriptProfilerHeapEnum.h"

using namespace PlatformAgnostic;

#ifdef ENABLE_BASIC_TELEMETRY
#include "..\Telemetry\Telemetry.h"
#include "..\Telemetry\ScriptContextTelemetry.h"
#endif
#ifdef ENABLE_PROJECTION
#include "Library\EngineInterfaceObject.h"
#include "..\Projection\\WinRtPromiseEngineInterfaceExtensionObject.h"
#include "..\Projection\ProjectionExternalLibrary.h"
#include "..\Projection\ProjectionContext.h"
#endif

#define DEFINE_OBJECT_NAME(object) const char16 *pwszObjectName = _u(#object);

#define REGISTER_OBJECT(object)\
    if (FAILED(hr = Register##object(pScriptContext)))\
    {\
        return hr;\
    }\

#define REG_LIB_FUNC_CORE(pwszObjectName, pwszFunctionName, functionPropertyId, entryPoint)\
    if (FAILED(hr = pScriptContext->RegisterLibraryFunction(pwszObjectName, pwszFunctionName, functionPropertyId, entryPoint)))\
    {\
        return hr;\
    }\

#define REG_GLOBAL_LIB_FUNC_CORE(pwszFunctionName, functionPropertyId, entryPoint)\
    REG_LIB_FUNC_CORE(NULL, pwszFunctionName, functionPropertyId, entryPoint)\

#define REG_GLOBAL_DYNAMIC_LIB_FUNC(pwszFunctionName, nFuncNameLen, entryPoint) {\
    Js::PropertyRecord const * propRecord; \
    pScriptContext->GetOrAddPropertyRecord(pwszFunctionName, nFuncNameLen, &propRecord); \
    REG_GLOBAL_LIB_FUNC_CORE(pwszFunctionName, propRecord->GetPropertyId(), entryPoint)\
    }

#define REG_OBJECTS_DYNAMIC_LIB_FUNC(pwszFunctionName, entryPoint) {\
    Js::PropertyRecord const * propRecord ; \
    pScriptContext->GetOrAddPropertyRecord(pwszFunctionName, _countof(pwszFunctionName) - 1, &propRecord); \
    REG_LIB_FUNC_CORE(pwszObjectName, pwszFunctionName, propRecord->GetPropertyId(), entryPoint)\
    }

#define REG_LIB_FUNC(pwszObjectName, functionPropertyId, entryPoint)\
    REG_LIB_FUNC_CORE(pwszObjectName, _u(#functionPropertyId), Js::PropertyIds::##functionPropertyId, entryPoint)\

#define REG_OBJECTS_LIB_FUNC(functionPropertyId, entryPoint)\
    REG_LIB_FUNC(pwszObjectName, functionPropertyId, entryPoint)\

#define REG_GLOBAL_CONSTRUCTOR(functionPropertyId)\
    REG_LIB_FUNC(nullptr, functionPropertyId, Javascript##functionPropertyId##::NewInstance)\

#define REG_GLOBAL_LIB_FUNC(functionPropertyId, entryPoint)\
    REG_LIB_FUNC(NULL, functionPropertyId, entryPoint)\

#define REGISTER_ERROR_OBJECT(functionPropertyId)\
    REG_GLOBAL_LIB_FUNC(functionPropertyId, Js::JavascriptError::New##functionPropertyId##Instance)\
    REG_LIB_FUNC(_u(#functionPropertyId), toString, Js::JavascriptError::EntryToString)\

HRESULT ScriptSite::Create(
        __in ScriptEngine *activeScript,
        __in IActiveScriptSite *iActiveScriptSite,
        __in BOOL useArena,
        __out ScriptSite** ppScriptSite)
{
    IfNullReturnError(ppScriptSite, E_INVALIDARG);
    *ppScriptSite = nullptr;

    HRESULT hr;
    ScriptSite * scriptSite = HeapNewNoThrow(ScriptSite);
    if (scriptSite == nullptr)
    {
        hr = E_OUTOFMEMORY;
    }
    else if (FAILED(hr = scriptSite->Init(
        activeScript,
        iActiveScriptSite,
        useArena)))
    {
        scriptSite->Release();
    }
    else
    {
        *ppScriptSite = scriptSite;
    }
    return hr;
}

HRESULT ScriptSite::CheckEvalRestriction() const
{
    Assert(this->m_IASDSite != nullptr);

    return (this->m_IASDSite != nullptr) ? this->m_IASDSite->CheckEvalRestriction() : S_OK;
}

HRESULT ScriptSite::HostExceptionFromHRESULT(HRESULT hr, Var* outError) const
{
    Assert(this->m_IASDSite != nullptr);

    return this->m_IASDSite->HostExceptionFromHRESULT(hr, outError);
}

HRESULT ScriptSite::SetDispatchInvoke(Js::JavascriptMethod dispatchInvoke)
{
    GetActiveScriptExternalLibrary()->SetDispatchInvoke(dispatchInvoke);
    return NOERROR;
}

HRESULT ScriptSite::ArrayBufferFromExternalObject(__in Js::RecyclableObject *obj,
    __out Js::ArrayBuffer **ppArrayBuffer)
{
#ifdef ENABLE_PROJECTION
//    OUTPUT_TRACE(TypedArrayPhase, _u("Suspected Projection ArrayBuffer source - attempting to get buffer\n"));
    Projection::ProjectionContext* projectionContext = scriptEngine->GetProjectionContext();
    if (projectionContext && obj->IsExternal())
    {
        return ProjectionContext::ArrayBufferFromExternalObject(obj, ppArrayBuffer);
    }
    *ppArrayBuffer = nullptr;
    return S_FALSE;

#else
    *ppArrayBuffer = nullptr;
    return S_FALSE;
#endif
}

Js::JavascriptError* ScriptSite::CreateWinRTError(IErrorInfo* perrinfo, Js::RestrictedErrorStrings * proerrstr)
{
#ifdef ENABLE_PROJECTION
    Projection::ProjectionContext* projectionContext = GetScriptEngine()->GetProjectionContext();
    if (projectionContext != nullptr)
    {
        return projectionContext->GetProjectionExternalLibrary()->CreateWinRTError(perrinfo, proerrstr);
    }
#endif
    return GetScriptSiteContext()->GetLibrary()->CreateError();
}

ScriptSite::ScriptSite()
    : refCount(1)
    , scriptEngine(nullptr)
    , scriptSiteContext(nullptr)
    , globalDispatches(nullptr)
    , dispatchMap(nullptr)
    , m_pvMinorSession((void*)-1)
    , m_ppoll(nullptr)
    , scriptDirect(nullptr)
#ifdef PROFILE_EXEC
    ,isProfileCreated(FALSE)
#endif
    , currentDispatchExCaller(nullptr)
    , m_punkCaller(nullptr)
    , hostAllocator(nullptr)
    , refCountedAllocatorInitialized(FALSE)
    , isClosed(FALSE)
    , outstandingDispatchCount(0)
    , activeScriptKeepAlive(nullptr)
    , externalLibary(nullptr)
#if ENABLE_DEBUG_CONFIG_OPTIONS
    , debugObjectHelper(nullptr)
#endif
{
    InitializeListHead(&hostDispatchListHead);
    InitializeListHead(&javascriptDispatchListHead);
    ThreadContext::IncrementActiveScriptSiteCount();

#if DBG_DUMP || defined(PROFILE_EXEC)
    this->nextTopLevelScriptSite = nullptr;
    this->prevTopLevelScriptSite = nullptr;
    this->parentScriptSite = nullptr;
    this->windowHost = nullptr;
    this->hasParentInfo = false;
#endif
#if DBG_DUMP
    this->wszLocation = nullptr;
#endif
    DLLAddRef();
}

HRESULT ScriptSite::Init(
        __in ScriptEngine *scriptEngine,
        __in IActiveScriptSite *iActiveScriptSite,
        __in BOOL useArena)
{
    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT_NESTED
    {
        this->scriptEngine = scriptEngine;
        this->scriptSiteContext = this->scriptEngine->EnsureScriptContext();
        this->threadContext = this->GetScriptSiteContext()->GetThreadContext();
        this->recycler = this->GetThreadContext()->GetRecycler();
        this->InitializeExternalLibrary();

        ChakraHostScriptContext * jscript9HostContext = HeapNew(ChakraHostScriptContext, this);
        this->GetScriptSiteContext()->SetHostScriptContext(jscript9HostContext);
        this->scriptSiteContext->SetScriptEngineHaltCallback(this->GetScriptEngine());

        ChakraHostDebugContext * chakraHostDebugContext = HeapNew(ChakraHostDebugContext, this);
        this->GetScriptSiteContext()->GetDebugContext()->SetHostDebugContext(chakraHostDebugContext);

        this->GetScriptSiteContext()->SetScriptStartEventHandler(ScriptSite::ScriptStartEventHandler);
        this->GetScriptSiteContext()->SetScriptEndEventHandler(ScriptSite::ScriptEndEventHandler);
#ifdef FAULT_INJECTION
        this->GetScriptSiteContext()->SetDisposeDisposeByFaultInjectionEventHandler(ScriptSite::DisposeScriptByFaultInjectionEventHandler);
#endif

        refCountedHostVariantAllocator.Initialize(this->GetRecycler());
        refCountedAllocatorInitialized = TRUE;

        threadId = GetCurrentThreadContextId();
        Assert(SUCCEEDED(scriptEngine->ValidateBaseThread()));

#if DBG_DUMP
        this->allocId = this->GetThreadContext()->scriptSiteCount++;

        if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::HostPhase))
        {
            Output::Print(_u("%p> ScriptSite %d(%p): Initialized \n"), this->threadId, this->allocId, this);
        }
#endif

        this->m_ppoll = nullptr;
        // Start with default value. TODO: Make it configurable through IActiveScriptProperty.
        this->ticksPerPoll = scriptEngine->m_dwTicksPerPoll;
        if (SUCCEEDED(iActiveScriptSite->QueryInterface(IID_IActiveScriptSiteInterruptPoll, (void **)&m_ppoll)))
        {
            // This host supports QueryContinue, so create a polling object if the thread doesn't already have one.
            if (!this->threadContext->HasInterruptPoller())
            {
                HeapNew(QueryContinuePoller, this->threadContext);
            }
        }

        this->m_IASDSite = nullptr;
        if (FAILED(iActiveScriptSite->QueryInterface(__uuidof(IActiveScriptDirectSite), (void **)&m_IASDSite)))
        {
            this->m_IASDSite = nullptr;
        }

        // Now try to read JS_PROFILER envvar and activate profiling
        // if this engine isn't executing encoded scripts.
        // Even if this fails, we dont throw any error because that will
        // fail the initialization of the session itself. We let it fail
        // quietly.

        IActiveScriptProfilerCallback *pProfileCallback;
        HRESULT hrT = ScriptEngine::CheckForExternalProfiler(&pProfileCallback);

        if (SUCCEEDED(hrT))
        {
            hrT = scriptEngine->StartProfilingInternal(pProfileCallback, PROFILER_EVENT_MASK_TRACE_ALL_WITH_DOM, 0);
            pProfileCallback->Release();
            AssertMsg(SUCCEEDED(hrT), "Unable to create Profiler");
        }
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    if (FAILED(hr))
    {
        // When ScriptSite::Init fails, we don't call ScriptSite::Close.
        // We need to clean up what Close would have cleaned up

        ReleasePointer(m_ppoll);
        this->scriptEngine->scriptContext = nullptr;
        this->scriptEngine = nullptr;
        isClosed = true;

        if (scriptSiteContext != nullptr)
        {
            HeapDelete(scriptSiteContext);
        }

        Assert(moduleRoots == nullptr);
        Assert(globalDispatches == nullptr);
        Assert(currentDispatchExCaller == nullptr);
        Assert(m_punkCaller == nullptr);
    }
#ifdef ENABLE_BASIC_TELEMETRY
    else
    {
        // Log out any relevant telemetry data. This is done to make sure we have some telemetry for cases when Site Navigation is not frequent, like Nodejs etc.
        bool isJSRT = false;
        DWORD hostType = scriptEngine->GetHostType(); // 0 for JShost, 1 for browser, 2 for WWA/JSRT, 3 for webview in WWA/XAML
        if (threadContext != nullptr)
        {
            isJSRT = threadContext->IsJSRT();
        }
        if (g_TraceLoggingClient!=nullptr && (this->scriptEngine->fNonPrimaryEngine == 0 || isJSRT))
        {
            g_TraceLoggingClient->FireChakraInitTelemetry(hostType, isJSRT);
        }
    }

#endif

    return hr;
}

ScriptSite::~ScriptSite()
{
#if DBG_DUMP || defined(PROFILE_EXEC)
    bool dump = false;
#if DBG_DUMP
    dump = Js::Configuration::Global.flags.Trace.IsEnabled(Js::HostPhase);
#endif
#ifdef PROFILE_EXEC
    // profiler is always created if PROFILE_EXEC is defined.
    dump = dump || (Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag) && isProfileCreated);
#endif
    if (dump)
    {
        this->DumpSiteInfo(_u("Deleted"));
        Output::Flush();
    }
#if DBG_DUMP
    if (wszLocation != nullptr)
    {
        HeapDeleteArray(wcslen(wszLocation) + 1, wszLocation);
    }
    if (setHostObjectStackBackTrace)
    {
        setHostObjectStackBackTrace.Unroot(recycler);
    }
    if (reinitHostObjectStackBackTrace)
    {
        reinitHostObjectStackBackTrace.Unroot(recycler);
    }
#endif
#endif

    // let global object delete script context at the end.

    if (this->scriptDirect)
    {
        this->scriptDirect->Release();
    }
    AssertMsg(isClosed, "ScriptSite dtor without close?");
    AssertMsg(m_ppoll == nullptr, "m_ppoll not NULL at dtor");
    AssertMsg(moduleRoots == nullptr, "moduleRoots not NULL at dtor");
    AssertMsg(scriptEngine == nullptr, "scriptEngine not NULL at dtor");
    AssertMsg(m_punkCaller == nullptr, "m_ppoll not NULL at dtor");
    AssertMsg(currentDispatchExCaller == nullptr, "currentDispatchExCaller not NULL at dtor");

#ifdef PROFILE_EXEC
    if (this->nextTopLevelScriptSite != nullptr)
    {
        ScriptSite* topScriptSite = threadContext->GetTopLevelScriptSite();
        if (topScriptSite && topScriptSite->nextTopLevelScriptSite == topScriptSite)
        {
            Assert(this == topScriptSite);
            Assert(this->prevTopLevelScriptSite == this);
            threadContext->SetTopLevelScriptSite(nullptr);
        }
        else
        {
            if (this == topScriptSite)
            {
                threadContext->SetTopLevelScriptSite(this->nextTopLevelScriptSite);
            }
            this->prevTopLevelScriptSite->nextTopLevelScriptSite = this->nextTopLevelScriptSite;
            this->nextTopLevelScriptSite->prevTopLevelScriptSite = this->prevTopLevelScriptSite;
        }
    }
#endif
    if (refCountedAllocatorInitialized)
    {
        refCountedHostVariantAllocator.Uninitialize();
    }

    if (hostAllocator)
    {
        HeapDelete(hostAllocator);
    }

    DLLRelease();
    ThreadContext::DecrementActiveScriptSiteCount();
}

void ScriptSite::Close()
{
    if (isClosed)
    {
        return;
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.DoHeapEnumOnEngineShutdown)
    {
       EnumHeap();
    }
#endif

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
    // we are closing the script context which unpin the global object
    // So the window host dispatch might not be live any more
    this->windowHost = nullptr;
#endif

    BOOL isNonPrimaryEngine = scriptEngine->fNonPrimaryEngine;
    Recycler* recycler =  this->GetRecycler();

#ifdef ENABLE_BASIC_TELEMETRY
    {
        Js::ScriptContext* scriptContext = this->GetScriptSiteContext();
        // Log out any relevant telemetry data.
        bool isJSRT = false;
        ThreadContext* threadContext = scriptContext->GetThreadContext();
        DWORD hostType = scriptEngine->GetHostType(); // 0 for JShost, 1 for browser, 2 for WWA/JSRT, 3 for webview in WWA/XAML
        if (threadContext != nullptr)
        {
            isJSRT = threadContext->IsJSRT();
        }
        if (!isNonPrimaryEngine || isJSRT)
        {
            g_TraceLoggingClient->FireSiteNavigation(scriptContext->GetUrl(), scriptEngine->GetActivityId(), hostType, isJSRT);

            if (scriptContext->HasTelemetry())
            {
                scriptContext->GetTelemetry().OutputTraceLogging(scriptEngine->GetActivityId(), hostType, isJSRT);
            }
        }
    }
#endif

#ifdef ENABLE_PROJECTION
    Projection::ProjectionContext* projectionContext = scriptEngine->GetProjectionContext();
    if (projectionContext)
    {
        HRESULT hr = NOERROR;
        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED

        // Notify the host that the ScriptEngine is about to Close
        hr = projectionContext->MarkForClose();

        END_TRANSLATE_OOM_TO_HRESULT(hr);
        AssertMsg(SUCCEEDED(hr), "ProjectionContext Closing failed due to OOM.");
    }
#endif

    IActiveScriptProfilerHeapEnum* pHeapEnum = GetScriptSiteContext()->GetHeapEnum();
    if (pHeapEnum)
    {
        ActiveScriptProfilerHeapEnum* heapEnum = static_cast<ActiveScriptProfilerHeapEnum*>(pHeapEnum);
        Assert(heapEnum != nullptr);
        heapEnum->CloseHeapEnum();
    }

    // if Close is explicitly called, we don't need to keep the engine alive. we should
    // preserve the old behavior.
    if (activeScriptKeepAlive)
    {
        activeScriptKeepAlive->Release();
        activeScriptKeepAlive = nullptr;
    }

    // We need to unlink global object and module roots in script site close, so later on
    // recycler can release the IDispatch* held in these HostDispatch objects.
    // We should not release the pointers directly here.
    if (moduleRoots != 0)
    {
        for (int i = 0; i < moduleRoots->Count(); i++)
        {
            Js::ModuleRoot* moduleRoot = moduleRoots->Item(i);
            moduleRoot->SetHostObject(moduleRoot->GetModuleID(), nullptr);
        }
    }

    if (globalDispatches)
    {
        // We don't need to addref/release as named items are added as global properties.
        globalDispatches->Clear();
    }

    // In JSRT, global object can be disposed before JSRTContxt, and we'll AV here.
    // We need to clean up the global object to break potential circular reference. we don't
    // need to do that for jsrt.
    if (!GetScriptSiteContext()->GetThreadContext()->IsJSRT())
    {
        Js::GlobalObject* globalObject = GetScriptSiteContext()->GetGlobalObject();
        globalObject->SetHostObject(nullptr);
        globalObject->SetDirectHostObject(nullptr, nullptr);
        globalObject = nullptr;
    }
    if (moduleRoots)
    {
        moduleRoots.Unroot(recycler);
    }

    if (this->scriptEngine->scriptContext)
    {
        this->scriptEngine->DebugDocMarkForClose();
    }

    // scriptContext will be closed, and that may delete it.  null out the script context.
    // Since the script body map is kept alive by the script context, if it goes away, we can't use it on the script engine
    // Same with debug stack frame
    this->scriptEngine->RemoveScriptBodyMap();
    this->scriptEngine->CleanupHalt();
    if (this->scriptEngine->debugStackFrame)
    {
        HeapDelete(this->scriptEngine->debugStackFrame);
        this->scriptEngine->debugStackFrame = nullptr;
    }
    this->scriptEngine->scriptContext = nullptr;
    this->scriptEngine = nullptr;
    ReleasePointer(m_ppoll);
    ReleasePointer(m_IASDSite);

    // JavascriptDispatch objects only check scriptObject, so make sure they are completely cleared out prior
    // to our closure and release calls. If anohter call comes in through COM RPC that should disable the JavascriptDispatch
    // from forwarding to the underlying external object and its type operations.
    LIST_ENTRY* listEntry = RemoveHeadList(&javascriptDispatchListHead);
    while (listEntry != &javascriptDispatchListHead)
    {
        JavascriptDispatch* javascriptDispatch = CONTAINING_RECORD(listEntry, JavascriptDispatch, linkList);
        InitializeListHead(listEntry);
        javascriptDispatch->ResetToNULL();
        listEntry = RemoveHeadList(&javascriptDispatchListHead);
        // null out the reference on the stack for debug build before we call the recycler below
        javascriptDispatch = nullptr;
    }
    InitializeListHead(&javascriptDispatchListHead);
    listEntry = nullptr;

    // We must mark ourselves closed before releasing any cross thread objects. Otherwise we may get
    // calls on our type operations which were potentially already freed.
    isClosed = TRUE;

    // m_punkCaller can be across an apartment boundary and then later stashed into the currentDispatchExCaller so both pointers
    // must be freed once the ScriptSite is closed and after JavascriptDispatch is cleared
    ReleasePointer(m_punkCaller);
    ReleasePointer(currentDispatchExCaller);

    // Any HostDispatch could be across an apartment boundary and so we need to clear them after isClosed and after JavascriptDispatch
    listEntry = RemoveHeadList(&hostDispatchListHead);
    while (listEntry != &hostDispatchListHead)
    {
        HostDispatch* hostDispatch = CONTAINING_RECORD(listEntry, HostDispatch, linkList);
        InitializeListHead(listEntry);
        hostDispatch->Finalize(false);
        listEntry = RemoveHeadList(&hostDispatchListHead);
        // null out the reference on the stack for debug build before we call the recycler below
        hostDispatch = nullptr;
    }
    InitializeListHead(&hostDispatchListHead);
    listEntry = nullptr;

#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (debugObjectHelper != nullptr)
    {
        HeapDelete(debugObjectHelper);
    }
#endif
#ifdef PROFILE_EXEC
    isProfileCreated = GetScriptSiteContext()->IsProfilerCreated();
#endif

    HRESULT hr = NOERROR;
    // Mark this as nested because close of one script context could cause another script context to close -
    // causing re-entrancy to this method.
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        GetScriptSiteContext()->MarkForClose();
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    AssertMsg(SUCCEEDED(hr), "Closing failed due to OOM.");

    // ScriptContext might have been freed during MarkForClose, null it out.
    scriptSiteContext = nullptr;

    threadContext->SetIsScriptContextCloseGCPending();

#ifdef ENABLE_PROJECTION
#if DBG_DUMP
    threadContext->DumpProjectionContextMemoryStats(_u("Stats after scriptSite close"), true);
#endif
#endif
}

void ScriptSite::CreateModuleRoot(
    __in Js::ModuleID moduleID,
    __in IDispatch* dispatch)
{
    Assert(GetThreadContext()->IsScriptActive());

    // May throw

    Js::ModuleRoot* tempModule;

    Assert(kmodGlobal != moduleID);

    if (this->moduleRoots == nullptr)
    {
        moduleRoots.Root(RecyclerNew(recycler, JsUtil::List<Js::ModuleRoot*>, recycler,
            JsUtil::List<Js::ModuleRoot*>::DefaultIncrement), recycler);
    }

#if DEBUG
    for (int i = 0 ; i < moduleRoots->Count(); i++)
    {
        tempModule = moduleRoots->Item(i);
        if (tempModule->GetModuleID() == moduleID)
        {
            AssertMsg(FALSE, "try to add an existing module");
        }
    }
#endif
    tempModule = RecyclerNew(this->GetRecycler(), Js::ModuleRoot,
        externalLibary->GetModuleRootType());
    Js::ScriptContext * scriptContext = GetScriptSiteContext();
    HostObject* hostObject = RecyclerNew(this->GetRecycler(), HostObject, scriptContext, dispatch,
        GetActiveScriptExternalLibrary()->GetHostObjectType());
    tempModule->SetHostObject(moduleID, hostObject);
    moduleRoots->Add(tempModule);
}

HRESULT ScriptSite::GetModuleDispatch(
    __in Js::ModuleID moduleID,
    __out IDispatch** dispatch)
{
    IfNullReturnError(dispatch, E_INVALIDARG);
    *dispatch = nullptr;

    Js::ModuleRoot* tempModule;
    JavascriptDispatch* moduleDispatch;
    if (kmodGlobal == moduleID)
    {
        *dispatch =  GetGlobalObjectDispatch();
        return NOERROR;
    }

    if (moduleRoots)
    {
        for (int i = 0 ; i < moduleRoots->Count(); i++)
        {
            tempModule = moduleRoots->Item(i);
            if (tempModule->GetModuleID() == moduleID)
            {
                 moduleDispatch = JavascriptDispatch::Create<false>(tempModule);
                 return moduleDispatch->QueryInterface(__uuidof(IDispatch), (void**)dispatch);
            }
        }
    }

    // Note: the legacy engine returns E_OUTOFMEMORY here, apparently assuming that it
    // failed to allocate a name table.
    return E_OUTOFMEMORY;
}

// ScriptSite and ScriptEngine can have different life time if IActiveScriptDirect is not used
// as we can SetScriptSite to different site in IActiveScript. However we don't allow that
// if IActiveScriptDirect is used, so the life time of these two objects is the same. Various
// external objects hold on ref count to ScriptSite, while the host site (script holder in trident)
// holds reference to ScriptEngine. After we navigate away from a site, ScriptEngine will release
// all reference to ScriptSite, but ScriptSite needs to hold on to a reference to IActiveScriptDirect
// such that calls to object access won't fault after we navigate out.
// Once all exteranl objects are released, ScriptSite will be released and delete, and we can
// release the scriptEngine at this time.
void ScriptSite::SetupFastDOM(IActiveScriptDirect* scriptDirect)
{
    if (this->scriptDirect == nullptr)
    {
        this->scriptDirect = scriptDirect;
        this->scriptDirect->AddRef();
    }
    GetScriptSiteContext()->SetFastDOMenabled();
}

Js::ModuleRoot * ScriptSite::GetModuleRoot(
    __in Js::ModuleID moduleID)
{
    Assert(kmodGlobal != moduleID);

    if (moduleRoots != nullptr)
    {
        for (int i = 0 ; i < moduleRoots->Count(); i++)
        {
            Js::ModuleRoot* tempModule = moduleRoots->Item(i);
            if (tempModule->GetModuleID() == moduleID)
            {
                return tempModule;
            }
        }
    }

    return nullptr;
}


#define END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT(hr, scriptContext, pspCaller) \
END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
catch (const Js::JavascriptException& err) \
{ \
    Js::JavascriptExceptionObject* pError = err.GetAndClear(); \
    hr = HandleJavascriptException(pError, scriptContext, static_cast<IServiceProvider *>(pspCaller)); \
} \
CATCH_UNHANDLED_EXCEPTION(hr)

#define END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_NO_SP(hr, scriptContext) \
END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
catch (const Js::JavascriptException& err) \
{ \
    Js::JavascriptExceptionObject* pError = err.GetAndClear(); \
    if (this->GetThreadContext()->HasPreviousHostScriptContext()) \
    { \
        DispatchExCaller* pspCaller = nullptr; \
        HostScriptContext * hostScriptContext = this->GetThreadContext()->GetPreviousHostScriptContext(); \
        hostScriptContext->GetDispatchExCaller(reinterpret_cast<void **>(&pspCaller)); \
        hr = HandleJavascriptException(pError, scriptContext, static_cast<IServiceProvider *>(pspCaller)); \
        hostScriptContext->ReleaseDispatchExCaller(reinterpret_cast<void *>(pspCaller)); \
    } \
    else \
    { \
        hr = HandleJavascriptException(pError, scriptContext, nullptr); \
    } \
} \
CATCH_UNHANDLED_EXCEPTION(hr)

//-----------------------------------------------------------------------------------
//
// ScriptSite::AddExternalObject
//
// Add a named root object to the script object. Note that we need to be able to track
// multiple root objects on multiple script objects, retrieving the right one to
// search for global names in a given script.
//
//-----------------------------------------------------------------------------------

HRESULT ScriptSite::AddExternalObject(
    __in LPCOLESTR pszName,
    __in IDispatch *pdisp,
    __in long lwCookie)
{
    HRESULT hr = S_OK;
    HostDispatch *  hostDispatch = nullptr;
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        if (nullptr == pdisp && 0 != lwCookie)
        {
            // call host back to get the object on first reference
            hostDispatch = HostDispatch::Create(scriptContext, pszName);
        }
        else
        {
            hostDispatch = HostDispatch::Create(scriptContext, pdisp);
        }

        charcount_t nameLength = Js::JavascriptString::GetBufferLength(pszName);
        if (!Js::JavascriptOperators::SetGlobalPropertyNoHost(pszName, nameLength, hostDispatch, scriptContext))
        {
            hr = E_FAIL;
        }

    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
#pragma prefast(suppress: 6286, "conditional compilation")
    if (SUCCEEDED(hr) && (DBG_DUMP
#ifdef PROFILE_EXEC
        || Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag)
#endif
#ifdef PROFILE_MEM
        || MemoryProfiler::IsEnabled() || MemoryProfiler::IsTraceEnabled()
#endif
        ) && wcscmp(pszName, _u("window")) == 0)
    {
        SetupWindowHost(hostDispatch);
    }
#endif
    return hr;
}

HRESULT ScriptSite::AddDefaultDispatch(Js::ModuleID moduleID, IDispatch *dispatch)
{
    HRESULT hr = NOERROR;
    Js::ScriptContext * scriptContext = GetScriptSiteContext();
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        if (moduleID == kmodGlobal)
        {
            // Add to the global list.
            hr = AddGlobalDispatch(dispatch);
        }
        else
        {
            CreateModuleRoot(moduleID, dispatch);
        }
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
    return hr;
}


HRESULT ScriptSite::AddGlobalDispatch(
    __in IDispatch* dispatch)
{
    HRESULT hr = NOERROR;
    Js::ScriptContext * scriptContext = GetScriptSiteContext();
    if (this->globalDispatches == nullptr)
    {
        if (hostAllocator == nullptr)
        {
            hostAllocator = HeapNew(ArenaAllocator, _u("Global-Host"), this->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
        }
        globalDispatches = JsUtil::List<HostDispatch*, ArenaAllocator>::New(hostAllocator);
    }


    Js::GlobalObject* globalObject = scriptContext->GetGlobalObject();
    HostObject* hostObject = RecyclerNew(
        this->GetRecycler(),
        HostObject,
        scriptContext,
        dispatch,
        externalLibary->GetHostObjectType());

    if (globalObject->GetHostObject() == nullptr)
    {
        globalObject->SetHostObject(hostObject);
    }
    else
    {
        globalDispatches->Add(hostObject->GetHostDispatch());
        HostObject* currentHostObject = static_cast<HostObject*>(globalObject->GetHostObject());
        currentHostObject->SetNeedToCheckOtherItem();
    }
    return hr;
}

#ifdef ENABLE_HEAP_DUMPER
    HRESULT ScriptSite::RegisterDebugDumpHeap(Js::ScriptContext *pContext)
    {
        Js::JavascriptLibrary* library = pContext->GetLibrary();
        if (library->AddFunctionToLibraryObjectWithPropertyName(library->GetDebugObject(), _u("dumpHeap"), &DebugObject::EntryInfo::DumpHeap, 0))
        {
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }
#endif

HRESULT ScriptSite::HandleJavascriptException(Js::JavascriptExceptionObject* exceptionObject, Js::ScriptContext * scriptContext, IServiceProvider * pspCaller)
{
    HRESULT hr;
    hr = E_FAIL;
    Assert(!scriptContext->GetThreadContext()->IsScriptActive());

    // It's possible we weren't able to clone the OOM exception in which case, let's not even
    // bother reporting it, just return a failure HRESULT
    if (exceptionObject == scriptContext->GetThreadContext()->GetPendingOOMErrorObject())
    {
        return E_OUTOFMEMORY;
    }

    if (pspCaller != nullptr)
    {
        hr = ActiveScriptError::CanHandleException(scriptContext, exceptionObject, pspCaller);
    }

    if (FAILED(hr) && hr != SCRIPT_E_RECORDED && hr != SCRIPT_E_PROPAGATE)
    {
        hr = ReportError(exceptionObject, scriptContext);
    }
    return hr;
}

HRESULT ScriptSite::CallRootFunction(Js::RecyclableObject * function, Js::Arguments args, IServiceProvider * pspCaller, Var * result)
{
    Js::ScriptContext * scriptContext = function->GetScriptContext();
#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
    scriptContext->GetHostScriptContext()->EnsureParentInfo();
#endif

    HRESULT hr = S_OK;
    if (scriptContext->IsHeapEnumInProgress())
    {
        AssertMsg(false, "shouldn't call function while in heap enumeration");
        return E_UNEXPECTED;
    }

    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
    {
        *result = Js::JavascriptFunction::CallRootFunction(function, args, scriptContext, false);
    }
    TRANSLATE_EXCEPTION_TO_HRESULT_ENTRY(const Js::JavascriptException& err)
    {
        *result = scriptContext->GetLibrary()->GetUndefined();

        Js::JavascriptExceptionObject * exceptionObject = err.GetAndClear();
        hr = HandleJavascriptException(exceptionObject, scriptContext, pspCaller);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    if (FAILED(hr))
    {
        *result = scriptContext->GetLibrary()->GetUndefined();
    }

    return hr;
}

HRESULT ScriptSite::ReportError(Js::JavascriptExceptionObject * pError, Js::ScriptContext* scriptContext)
{
    HRESULT hr = NO_ERROR;
    ActiveScriptError * pase;
    Js::ScriptContext * errorScriptContext = scriptContext;

    if (SUCCEEDED(ActiveScriptError::CreateRuntimeError(pError, &hr, nullptr, errorScriptContext, &pase)))
    {
        if (!errorScriptContext->IsClosed()) //Report error only if errorScriptContext is not closed.
        {
            ScriptSite * errorScriptSite = ScriptSite::FromScriptContext(errorScriptContext);
            Assert(errorScriptSite != nullptr);
            ThreadContext* threadContext = errorScriptContext->GetThreadContext();
            threadContext->SetHasUnhandledException();
            threadContext->SetUnhandledExceptionObject(pError);
            if (NOERROR == errorScriptSite->GetScriptEngine()->OnScriptError((IActiveScriptError *) IACTIVESCRIPTERROR64 pase))
            {
                // there might be additional exception thrown during ReportError as trident run some additional js code.
                // There is nothing additional to report for those.
                threadContext->SetRecordedException(nullptr);
                hr = SCRIPT_E_REPORTED;
            }
            threadContext->SetUnhandledExceptionObject(nullptr);
            threadContext->ResetHasUnhandledException();
            pError->ClearStackTrace();
        }
        else
        {
            // The errorScriptContext could be anyone and since they have closed we don't have anyone to report the error to.
            // However, we can say to the caller that an error WAS reported and they can handle this appropriately.
            hr = SCRIPT_E_REPORTED;
        }
        pase->Release();
    }
    return hr;
}

HRESULT ScriptSite::ExternalToPrimitive(Js::DynamicObject * scriptObject, Js::JavascriptHint hint, Var * result, IServiceProvider * pspCaller)
{
    HRESULT hr = S_OK;
    Js::ScriptContext * scriptContext = scriptObject->GetScriptContext();

    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        *result = Js::JavascriptExternalConversion::ToPrimitive(scriptObject, hint, scriptContext);
        return hr;
    }
    END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT(hr, scriptContext, pspCaller);

    *result = scriptObject->GetLibrary()->GetUndefined();
    return hr;
}


HRESULT ScriptSite::ExternalGetPropertyReference(Js::DynamicObject* scriptObject, DISPID id, Js::Var* varMember, IServiceProvider * pspCaller)
{
    HRESULT hr = NO_ERROR;
    Js::ScriptContext * scriptContext = scriptObject->GetScriptContext();
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        if (Js::JavascriptExternalOperators::GetPropertyReference(scriptObject, id, varMember, scriptContext))
        {
            return hr;
        }
        hr = DISP_E_MEMBERNOTFOUND;
    }
    END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT(hr, scriptContext, pspCaller);

    *varMember = scriptObject->GetLibrary()->GetUndefined();
    return hr;
}



HRESULT ScriptSite::ExternalGetProperty(Js::DynamicObject* scriptObject, DISPID id, Js::Var* varMember, IServiceProvider * pspCaller)
{
    HRESULT hr = NO_ERROR;
    Js::ScriptContext * scriptContext = scriptObject->GetScriptContext();
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        if (Js::JavascriptExternalOperators::GetProperty(scriptObject, id, varMember, scriptContext))
        {
            return hr;
        }
        hr = DISP_E_MEMBERNOTFOUND;
    }
    END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT(hr, scriptContext, pspCaller);

    *varMember = scriptObject->GetLibrary()->GetUndefined();
    return hr;
}


HRESULT ScriptSite::ExternalSetProperty(Js::DynamicObject* scriptObject, DISPID id, Js::Var value, IServiceProvider * pspCaller)
{
    HRESULT hr = NO_ERROR;
    Js::ScriptContext * scriptContext = scriptObject->GetScriptContext();
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        Js::JavascriptExternalOperators::SetProperty(scriptObject, id, value, scriptContext);
    }
    END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT(hr, scriptContext, pspCaller);

    return hr;
}

HRESULT ScriptSite::ExternalToNumber(Js::Var instance, double * result)
{
    // Optimize for TaggedInt and double before falling back to ToNumber_Full
    if( Js::TaggedInt::Is(instance) )
    {
        *result = Js::TaggedInt::ToDouble(instance);
        return S_OK;
    }
    else if( Js::JavascriptNumber::Is(instance) )
    {
        *result = Js::JavascriptNumber::GetValue(instance);
        return S_OK;
    }
    else
    {
        return ExternalToNumberCanThrow(instance, result);
    }
}

HRESULT ScriptSite::ExternalToNumberCanThrow(Js::Var instance, double* result)
{
    HRESULT hr = VerifyStackOnEntry();
    if ( hr == S_OK )
    {
        Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
        Assert(scriptContext != nullptr && !scriptContext->IsClosed());

        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
        {
            *result = Js::JavascriptExternalConversion::ToNumber(instance, scriptContext);
            return hr;
        }
        END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_NO_SP(hr, scriptContext);
    }
    return hr;
}

HRESULT ScriptSite::ExternalToString(Js::Var instance, Js::JavascriptString ** result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);
    HRESULT hr = NO_ERROR;
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        *result = Js::JavascriptExternalConversion::ToString(instance, scriptContext);
        (*result)->GetSz();
        return hr;
    }
    END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_NO_SP(hr, scriptContext);
    *result = nullptr;
    return hr;
}

HRESULT ScriptSite::ExternalToInt32(Js::Var instance, int * result)
{
    // Try to handle the optimal case of a tagged int before falling back to external conversion.
    if (Js::TaggedInt::Is(instance))
    {
        *result = Js::TaggedInt::ToInt32(instance);
        return S_OK;
    }
    else
    {
        return ExternalToInt32CanThrow(instance, result);
    }
}

HRESULT ScriptSite::ExternalToInt32CanThrow(Js::Var instance, int * result)
{
    HRESULT hr = VerifyStackOnEntry();
    if ( hr == S_OK )
    {
        Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
        Assert(scriptContext != nullptr && !scriptContext->IsClosed());

        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
        {
            *result = Js::JavascriptExternalConversion::ToInt32(instance, scriptContext);
            return S_OK;
        }
        END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_NO_SP(hr, scriptContext);
        *result = 0;
    }
    return hr;
}


HRESULT ScriptSite::ExternalToInt64(Js::Var instance, __int64 * result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    // Try to handle the optimal case of a tagged int before falling back to external conversion.
    if (Js::TaggedInt::Is(instance))
    {
        *result = Js::TaggedInt::ToInt32(instance);
        return S_OK;
    }

    HRESULT hr = NO_ERROR;
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        *result = Js::JavascriptExternalConversion::ToInt64(instance, scriptContext);
        return S_OK;
    }
    END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_NO_SP(hr, scriptContext);
    *result = 0;
    return hr;
}

HRESULT ScriptSite::ExternalToUInt64(Js::Var instance, unsigned __int64 * result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    // Try to handle the optimal case of a tagged int before falling back to external conversion.
    if (Js::TaggedInt::Is(instance))
    {
        *result = Js::TaggedInt::ToInt32(instance);
        return S_OK;
    }

    HRESULT hr = NO_ERROR;
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        *result = Js::JavascriptExternalConversion::ToUInt64(instance, scriptContext);
        return S_OK;
    }
    END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_NO_SP(hr, scriptContext);
    *result = 0;
    return hr;
}

HRESULT ScriptSite::ExternalToBoolean(Js::Var instance, BOOL * result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    HRESULT hr = NO_ERROR;
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        *result = Js::JavascriptExternalConversion::ToBoolean(instance, scriptContext);
        return S_OK;
    }
    END_TRANSLATE_EXCEPTION_AND_REPORT_ERROROBJECT_TO_HRESULT_NO_SP(hr, scriptContext);
    *result = FALSE;
    return hr;
}

HRESULT ScriptSite::ExternalBOOLToVar(BOOL b, Js::Var * result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    // This runtime operation don't throw, so we can just call it directly
    *result = Js::JavascriptBoolean::ToVar(b, scriptContext);
    return S_OK;
}

HRESULT ScriptSite::ExternalInt32ToVar(int32 i, Js::Var * result)
{
    if ( !Js::TaggedInt::IsOverflow(i) )
    {
        *result = Js::TaggedInt::ToVarUnchecked(i);
        return S_OK;
    }
    else
    {
        return ExternalInt32ToVarCanThrow(i, result);
    }
}

HRESULT ScriptSite::ExternalInt32ToVarCanThrow(int32 i, Js::Var * result)
{
    // TODO: temporary disable the check; trident should reenable
    // this after they remove the usage of this API in heapenum
    HRESULT hr = VerifyStackOnEntry(TRUE);
    if ( hr == S_OK )
    {
        Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
        Assert(scriptContext != nullptr);

        *result = nullptr;
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            // This runtime operation only throw OOM
            *result = Js::JavascriptNumber::ToVar(i, scriptContext);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
    }
    return hr;
}

HRESULT ScriptSite::ExternalInt64ToVar(__int64 value, Js::Var* result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    if ( !Js::TaggedInt::IsOverflow(value) )
    {
        *result = Js::TaggedInt::ToVarUnchecked((int)value);
        return S_OK;
    }

    HRESULT hr = S_OK;
    *result = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        // This runtime operation only throw OOM
        *result = Js::JavascriptTypedNumber<__int64>::ToVar(value, scriptContext);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT ScriptSite::ExternalUInt64ToVar(unsigned __int64 value, Js::Var* result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    if ( !Js::TaggedInt::IsOverflow(value) )
    {
        *result = Js::TaggedInt::ToVarUnchecked((int)value);
        return S_OK;
    }

    HRESULT hr = S_OK;
    *result = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        // This runtime operation only throw OOM
        *result = Js::JavascriptTypedNumber<unsigned __int64>::ToVar(value, scriptContext);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}


HRESULT ScriptSite::ExternalDoubleToVar(double d, Js::Var * result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    HRESULT hr = S_OK;
    *result = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        // This runtime operation only throw OOM
        *result = Js::JavascriptNumber::ToVarWithCheck(d, scriptContext);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT ScriptSite::ExternalToDate(Js::Var instance, double * result)
{
    if(!Js::JavascriptDate::Is(instance))
    {
        return E_INVALIDARG;
    }

    *result = Js::JavascriptDate::FromVar(instance)->GetTime();
    return S_OK;
}

HRESULT ScriptSite::ExternalDateToVar(double d, Js::Var * result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    HRESULT hr = S_OK;
    *result = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        *result = scriptContext->GetLibrary()->CreateDate(d);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT ScriptSite::ExternalSYSTEMTIMEToVar(SYSTEMTIME* pst, Js::Var * result)
{
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
    Assert(scriptContext != nullptr);

    HRESULT hr = S_OK;
    *result = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        *result = scriptContext->GetLibrary()->CreateDate(pst);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT ScriptSite::Execute(__in Js::RecyclableObject *pScrObj, __in Js::Arguments* args, __in IServiceProvider * pspCaller, __out_opt Js::Var* varResult)
{
    Js::Var                        atomResult;
    HRESULT                         hr = S_OK;

    if (varResult != nullptr)
    {
        *varResult = nullptr;
    }

    if (this->scriptEngine == nullptr)
    {
        // The session has closed already.
        return E_UNEXPECTED;
    }

    // Call through DynamicObject and put the result (if any) in pVarRes.
    if (pScrObj == nullptr ||
        !Js::JavascriptConversion::IsCallable(pScrObj))
    {
        return JSERR_NeedFunction;
    }

    SmartFPUControl smartFpuControl;
    if (smartFpuControl.HasErr())
    {
        return smartFpuControl.GetErr();
    }

    Js::ScriptContext* functionScriptContext = pScrObj->GetScriptContext();
    Js::RecyclableObject * callableObj = pScrObj;
    if (functionScriptContext != this->GetScriptSiteContext())
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        callableObj = Js::RecyclableObject::FromVar(Js::CrossSite::MarshalVar(this->GetScriptSiteContext(), pScrObj));
        END_TRANSLATE_OOM_TO_HRESULT(hr);
    }
    if (FAILED(hr))
    {
        return hr;
    }
    uint argCount = args->Info.Count;
    AssertMsg((args->Info.Flags & CallFlags_CallPut) == 0, "This is NOT expected.");
    if (args->Info.Flags & CallFlags_ExtraArg)
    {
        Assert(argCount >= 1);
        argCount--;
    }
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT
            for (unsigned int i = 0; i < argCount; i++)
            {
                args->Values[i] = Js::CrossSite::MarshalVar(functionScriptContext, args->Values[i]);
            }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
    }
    if (FAILED(hr))
    {
        return hr;
    }
    Assert(functionScriptContext == this->GetScriptSiteContext()
        || callableObj->IsExternal()
        || Js::CrossSite::IsThunk(callableObj->GetEntryPoint()));
    hr = ScriptSite::CallRootFunction(callableObj, *args, pspCaller, &atomResult);

    if (varResult != nullptr)
    {
        *varResult = atomResult;
    }

    smartFpuControl.RestoreFPUControl();
    if (smartFpuControl.HasErr())
    {
        return smartFpuControl.GetErr();
    }

    return hr;
}

void ScriptSite::SetCaller(__in IUnknown *punkNew, __deref_out_opt IUnknown **ppunkPrev)
{
    *ppunkPrev = nullptr;

    AssertMemN(punkNew);

    *ppunkPrev = m_punkCaller;
    if (!isClosed)
    {
        m_punkCaller = punkNew;
        if (nullptr != m_punkCaller)
            m_punkCaller->AddRef();
    }
    else
    {
        // we don't need to restore the caller site
        // this happens during cleanup and we shouldn't have
        // more setcaller calls.
        Assert(m_punkCaller == NULL);
    }
    if (NULL != currentDispatchExCaller)
    {
        currentDispatchExCaller->Release();
        currentDispatchExCaller = NULL;
    }
}

HRESULT ScriptSite::GetPreviousHostScriptContext(__out HostScriptContext **dispatchExCaller)
{
    IfNullReturnError(dispatchExCaller, E_INVALIDARG);
    *dispatchExCaller = nullptr;

    ThreadContext *  threadContext = this->GetThreadContext();
    *dispatchExCaller = threadContext->GetPreviousHostScriptContext();
    return NOERROR;
}

HRESULT ScriptSite::PushHostScriptContext(HostScriptContext* hostScriptContxt)
{
    HRESULT hr = NOERROR;

    // Allocations can throw here.
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
    {
        ThreadContext *  threadContext = this->GetThreadContext();
        threadContext->PushHostScriptContext(hostScriptContxt);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
    return hr;
}


void ScriptSite::PopHostScriptContext()
{
    ThreadContext * threadContext = this->GetThreadContext();
    threadContext->PopHostScriptContext();
}

HRESULT ScriptSite::GetDispatchExCaller(__out DispatchExCaller **dispatchExCaller)
{
    IfNullReturnError(dispatchExCaller, E_INVALIDARG);
    *dispatchExCaller = nullptr;

    HRESULT hr;

    if (NULL == currentDispatchExCaller)
    {
        if (FAILED(hr = DispatchExCaller::Create(this, m_punkCaller, &currentDispatchExCaller)))
        {
            currentDispatchExCaller = NULL;
            return hr;
        }
        if (NULL != m_punkCaller)
        {
            m_punkCaller->Release();
            m_punkCaller = NULL;
        }
    }

    Assert(NULL == m_punkCaller);
    *dispatchExCaller = currentDispatchExCaller;
    currentDispatchExCaller = NULL;

    return NOERROR;
}

void ScriptSite::ReleaseDispatchExCaller(
    __in DispatchExCaller *dispatchExCaller)
{
    if (NULL != currentDispatchExCaller)
    {
        currentDispatchExCaller->Release();
        currentDispatchExCaller = NULL;
    }
    if (NULL != m_punkCaller)
    {
        m_punkCaller->Release();
        m_punkCaller = NULL;
    }

    currentDispatchExCaller = dispatchExCaller;
}


ScriptSite::DispatchMap * ScriptSite::EnsureDispatchMap(void)
{
    AssertCanHandleOutOfMemory();
    if (this->dispatchMap == nullptr)
    {
        if (hostAllocator == NULL)
        {
            hostAllocator = HeapNew(ArenaAllocator, _u("Global-Host"), this->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
        }
        this->dispatchMap = Anew(hostAllocator, DispatchMap, hostAllocator, 17);
    }
    return this->dispatchMap;
}


LCID ScriptSite::GetUserLocale(void)
{
    if (NULL != scriptEngine)
        return scriptEngine->GetUserLcid();
    return GetUserDefaultLCID();
}

void ScriptSite::RecordExcepInfoAndClear(EXCEPINFO *pei, HRESULT *phr)
{
    AssertMsg(FALSE, "Not Implemented");
}

ScriptSite *
ScriptSite::FromHostScriptContext(HostScriptContext * hostScriptContext)
{
    return ((ChakraHostScriptContext *)hostScriptContext)->GetScriptSite();
}

ScriptSite *
ScriptSite::FromScriptContext(Js::ScriptContext * scriptContext)
{
    return ScriptSite::FromHostScriptContext(scriptContext->GetHostScriptContext());
}

void
ScriptSite::ScriptStartEventHandler(Js::ScriptContext * scriptContext)
{
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    if (scriptEngine)
    {
        scriptEngine->OnEnterScript();
    }
}

void
ScriptSite::ScriptEndEventHandler(Js::ScriptContext * scriptContext)
{
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    if (scriptEngine)
    {
        scriptEngine->OnLeaveScript();
    }
}

#ifdef FAULT_INJECTION
void
ScriptSite::DisposeScriptByFaultInjectionEventHandler(Js::ScriptContext * scriptContext) {
    ScriptSite *scriptSite = ScriptSite::FromScriptContext(scriptContext);
    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    if (scriptEngine)
    {

        HRESULT hr = scriptEngine->SetScriptState(SCRIPTSTATE::SCRIPTSTATE_CLOSED);
        if (FAILED(hr))
        {
            AssertMsg(FALSE, "Call to SetScriptState() failed.");
        }
        else
        {
            SCRIPTSTATE ss;
            scriptEngine->GetScriptState(&ss);
            AssertMsg(ss == SCRIPTSTATE_CLOSED, "SetScriptState failed to set the state to SCRIPTSTATE_CLOSED.");
        }
    }
}
#endif

HRESULT ScriptSite::CheckIsSiteAliveCallback(ScriptSite* scriptSite)
{
    return scriptSite->CheckIsSiteAlive();
}

HRESULT ScriptSite::CheckIsSiteAlive()
{
    if (scriptEngine == NULL)
    {
        return E_ACCESSDENIED;
    }
    return NOERROR;
}

void
ScriptSite::InitializeExternalLibrary()
{
    EnsureExternalLibrary();
    externalLibary->Initialize(GetScriptSiteContext()->GetLibrary());

    InitializeDebugObject();
#if defined(EDIT_AND_CONTINUE) && defined(ENABLE_DEBUG_CONFIG_OPTIONS)
    InitializeEditTest();
#endif
}

void ScriptSite::EnsureExternalLibrary()
{
    if (externalLibary == nullptr)
    {
        Js::ScriptContext * scriptContext = this->GetScriptSiteContext();
        Js::JavascriptLibrary* library = scriptContext->GetLibrary();
        externalLibary = RecyclerNew(library->GetRecycler(), ActiveScriptExternalLibrary);
    }
}

HRESULT ScriptSite::EnqueuePromiseTask(__in Js::Var taskVar)
{
    if (IsClosed())
    {
        return E_ACCESSDENIED;
    }

    IActiveScriptDirectHost* scriptHost = GetScriptEngine()->GetActiveScriptDirectHostNoRef();
    if (scriptHost != nullptr)
    {
        BEGIN_LEAVE_SCRIPT(scriptSiteContext)
        {
            scriptHost->EnqueuePromiseTask(taskVar);
        }
        END_LEAVE_SCRIPT(scriptSiteContext);
    }
    else
    {
        // This is a fallback for WScript
        PropertyId wscriptId = scriptSiteContext->GetOrAddPropertyIdTracked(_u("WScript"));
        PropertyId setTimeoutId = scriptSiteContext->GetOrAddPropertyIdTracked(_u("SetTimeout"));

        Js::Var global = scriptSiteContext->GetGlobalObject();
        Js::Var wscript;
        Js::Var setTimeout;
        Js::JavascriptFunction* hostCallback;

        // Try to load WScript.SetTimeout
        if (Js::JavascriptOperators::GetRootProperty(global, wscriptId, &wscript, scriptSiteContext) &&
            Js::RecyclableObject::Is(wscript) &&
            Js::JavascriptOperators::GetProperty(Js::RecyclableObject::FromVar(wscript), setTimeoutId, &setTimeout, scriptSiteContext) &&
            Js::JavascriptConversion::IsCallable(setTimeout))
        {
            hostCallback = Js::JavascriptFunction::FromVar(setTimeout);
        }
        else
        {
            hostCallback = scriptSiteContext->GetLibrary()->GetThrowerFunction();
        }

        CALL_FUNCTION(
            scriptSiteContext->GetThreadContext(),
            hostCallback,
            Js::CallInfo(Js::CallFlags::CallFlags_Value, 3),
            scriptSiteContext->GetLibrary()->GetUndefined(),
            taskVar,
            Js::JavascriptNumber::ToVar(0, scriptSiteContext));
    }
    return NOERROR;
}

HRESULT ScriptSite::FetchImportedModule(Js::ModuleRecordBase* referencingModule, LPCOLESTR specifier, Js::ModuleRecordBase** dependentModuleRecord)
{
    HRESULT hr = NOERROR;
    IActiveScriptDirectHost* scriptHost = GetScriptEngine()->GetActiveScriptDirectHostNoRef();
    if (scriptHost == nullptr)
    {
        hr = E_ACCESSDENIED;
    }
    else
    {
        Assert(!GetScriptSiteContext()->GetThreadContext()->IsScriptActive());
        BEGIN_NO_EXCEPTION
        hr = scriptHost->FetchImportedModule((ModuleRecord)referencingModule, specifier, wcslen(specifier), (ModuleRecord*)dependentModuleRecord);
        END_NO_EXCEPTION
    }
    return hr;
}

HRESULT ScriptSite::FetchImportedModuleFromScript(DWORD_PTR dwReferencingSourceContext, LPCOLESTR specifier, Js::ModuleRecordBase** dependentModuleRecord)
{
    HRESULT hr = NOERROR;
    IActiveScriptDirectHost* scriptHost = GetScriptEngine()->GetActiveScriptDirectHostNoRef();
    if (scriptHost == nullptr)
    {
        hr = E_ACCESSDENIED;
    }
    else
    {
        Assert(!GetScriptSiteContext()->GetThreadContext()->IsScriptActive());
        BEGIN_NO_EXCEPTION
        // TODO(suwc): switch to FetchImportedModuleFromScript once idl update is completed
        //hr = scriptHost->FetchImportedModuleFromScript(dwReferencingSourceContext, specifier, wcslen(specifier), (ModuleRecord*)dependentModuleRecord);
        hr = scriptHost->FetchImportedModule(nullptr, specifier, wcslen(specifier), (ModuleRecord*)dependentModuleRecord);
        END_NO_EXCEPTION
    }
    return hr;
}

HRESULT ScriptSite::NotifyHostAboutModuleReady(Js::ModuleRecordBase* referencingModule, Js::Var exceptionVar)
{
    HRESULT hr = NOERROR;
    IActiveScriptDirectHost* scriptHost = GetScriptEngine()->GetActiveScriptDirectHostNoRef();
    if (scriptHost == nullptr)
    {
        hr = E_ACCESSDENIED;
    }
    else
    {
        // we don't need this block if the host does not callback to scriptengine right away and use Promise/EnqueueTask instead.
        BEGIN_NO_EXCEPTION
        hr = scriptHost->NotifyModuleReady(referencingModule, exceptionVar);
        END_NO_EXCEPTION
    }
    return hr;
}

void ScriptSite::InitializeDebugObject()
{
    // TODO: move this to ActiveScriptExternalLibrary after move out Promise.
    Js::ScriptContext* scriptContext = this->GetScriptSiteContext();
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    Js::DynamicObject* debugObject = Js::DynamicObject::New(recycler,
        Js::DynamicType::New(scriptContext, Js::TypeId::TypeIds_Object, library->GetObjectPrototype(), nullptr,
        Js::DeferredTypeHandler<ScriptSite::InitializeDebugObjectType>::GetDefaultInstance()));

    library->EnsureDebugObject(debugObject);
}

void ScriptSite::InitializeDebugObjectType(Js::DynamicObject* debugObject, Js::DeferredTypeHandlerBase * typeHandler, Js::DeferredInitializeMode mode)
{
    typeHandler->Convert(debugObject, mode, 36);

    Js::ScriptContext* scriptContext = debugObject->GetScriptContext();
    Js::JavascriptLibrary* library = debugObject->GetLibrary();

    // Note: Any new function addition/deletion/modification should also be updated in ScriptSite::RegisterDebug
    // so that the update is in sync with profiler
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    library->AddMember(debugObject, Js::PropertyIds::EngineInterface, library->GetEngineInterfaceObject(), PropertyNone);
#endif

    library->AddFunctionToLibraryObject(debugObject, Js::PropertyIds::write, &DebugObject::EntryInfo::Write, 0);
    library->AddFunctionToLibraryObject(debugObject, Js::PropertyIds::writeln, &DebugObject::EntryInfo::WriteLine, 0);

    // Adding accessor to setNonUserCodeExceptions.
    library->SetDebugObjectNonUserAccessor(&DebugObject::EntryInfo::GetterSetNonUserCodeExceptions, &DebugObject::EntryInfo::SetterSetNonUserCodeExceptions);
    debugObject->SetAccessors(Js::PropertyIds::setNonUserCodeExceptions, library->GetDebugObjectNonUserGetterFunction(), library->GetDebugObjectNonUserSetterFunction());

    // Adding accessor to debuggerEnabled.
    library->SetDebugObjectDebugModeAccessor(&DebugObject::EntryInfo::GetterDebuggerEnabled);
    debugObject->SetAccessors(Js::PropertyIds::debuggerEnabled, library->GetDebugObjectDebugModeGetterFunction(), library->GetUndefined());
    debugObject->SetConfigurable(Js::PropertyIds::debuggerEnabled, FALSE);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

    if (!CONFIG_FLAG(DisableDebugObject))
    {
        library->SetDebugObjectFaultInjectionCookieGetterAccessor(&DebugObject::EntryInfo::GetterFaultInjectionCookie, &DebugObject::EntryInfo::SetterFaultInjectionCookie);
        Js::PropertyRecord const * faultInjectionCookiePropertyRecord;
        scriptContext->GetOrAddPropertyRecord(_u("faultInjectionCookie"), &faultInjectionCookiePropertyRecord);
        Js::PropertyId faultInjectionCookiePropertyId = faultInjectionCookiePropertyRecord->GetPropertyId();
        debugObject->SetAccessors(faultInjectionCookiePropertyId, library->GetDebugObjectFaultInjectionCookieGetterFunction(), library->GetDebugObjectFaultInjectionCookieSetterFunction());
        debugObject->SetAttributes(faultInjectionCookiePropertyId, PropertyWritable);

        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getWorkingSet"), &DebugObject::EntryInfo::GetWorkingSet, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("sourceDebugBreak"), &DebugObject::EntryInfo::SourceDebugBreak, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("invokeFunction"), &DebugObject::EntryInfo::InvokeFunction, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getMemoryInfo"), &DebugObject::EntryInfo::GetMemoryInfo, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getHostInfo"), &DebugObject::EntryInfo::GetHostInfo, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getTypeHandlerName"), &DebugObject::EntryInfo::GetTypeHandlerName, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getArrayType"), &DebugObject::EntryInfo::GetArrayType, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("createDebugDisposableObject"), &DebugObject::EntryInfo::CreateDebugDisposableObject, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("isInJit"), &DebugObject::EntryInfo::IsInJit, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getCurrentSourceInfo"), &DebugObject::EntryInfo::GetCurrentSourceInfo, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getLineOfPosition"), &DebugObject::EntryInfo::GetLineOfPosition, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getPositionOfLine"), &DebugObject::EntryInfo::GetPositionOfLine, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("createTypedObject"), &DebugObject::EntryInfo::CreateTypedObject, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("createProjectionArrayBuffer"), &DebugObject::EntryInfo::CreateProjectionArrayBuffer, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("addFTLProperty"), &DebugObject::EntryInfo::AddFTLProperty, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("emitStackTraceEvent"), &DebugObject::EntryInfo::EmitStackTraceEvent, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getTypeInfo"), &DebugObject::EntryInfo::GetTypeInfo, 1);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("parseFunction"), &DebugObject::EntryInfo::ParseFunction, 1);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("setAutoProxyName"), &DebugObject::EntryInfo::SetAutoProxyName, 1);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("disableAutoProxy"), &DebugObject::EntryInfo::DisableAutoProxy, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("createDebugFuncExecutorInDisposeObject"), &DebugObject::EntryInfo::CreateDebugFuncExecutorInDisposeObject, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("detachAndFreeObject"), &DebugObject::EntryInfo::DetachAndFreeObject, 1);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("isAsmJSModule"), &DebugObject::EntryInfo::IsAsmJSModule, 0);
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("enable"), &DebugObject::EntryInfo::Enable, 1);
    }
    if (!CONFIG_FLAG(DisableDebugObject) || CONFIG_FLAG(DumpHeap))
    {
        library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("dumpHeap"), &DebugObject::EntryInfo::DumpHeap, 0);
    }
#endif
#if JS_PROFILE_DATA_INTERFACE
    library->AddFunctionToLibraryObjectWithPropertyName(debugObject, _u("getProfileDataObject"), &DebugObject::EntryInfo::GetProfileDataObject, 0);
#endif

    if (CONFIG_FLAG(AsyncDebugging) && !CONFIG_FLAG(DisableDebugObject))
    {
        // Debug.MS_ASYNC_OP_STATUS_SUCCESS: number=1. Writable:false, Configurable:false, Enumerable:false
        library->AddMember(debugObject, Js::PropertyIds::MS_ASYNC_OP_STATUS_SUCCESS, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Completed, scriptContext), PropertyNone);

        // Debug.MS_ASYNC_OP_STATUS_CANCELED: number=2. Writable:false, Configurable:false, Enumerable:false
        library->AddMember(debugObject, Js::PropertyIds::MS_ASYNC_OP_STATUS_CANCELED, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Canceled, scriptContext), PropertyNone);

        // Debug.MS_ASYNC_OP_STATUS_ERROR: number=3. Writable:false, Configurable:false, Enumerable:false
        library->AddMember(debugObject, Js::PropertyIds::MS_ASYNC_OP_STATUS_ERROR, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Error, scriptContext), PropertyNone);

        // Debug.MS_ASYNC_CALLBACK_STATUS_ASSIGN_DELEGATE: number=0. Writable:false, Configurable:false, Enumerable:false
        library->AddMember(debugObject, Js::PropertyIds::MS_ASYNC_CALLBACK_STATUS_ASSIGN_DELEGATE, Js::JavascriptNumber::New(AsyncDebug::AsyncCallbackStatus_AssignDelegate, scriptContext), PropertyNone);

        // Debug.MS_ASYNC_CALLBACK_STATUS_JOIN: number=1. Writable:false, Configurable:false, Enumerable:false
        library->AddMember(debugObject, Js::PropertyIds::MS_ASYNC_CALLBACK_STATUS_JOIN, Js::JavascriptNumber::New(AsyncDebug::AsyncCallbackStatus_Join, scriptContext), PropertyNone);

        // Debug.MS_ASYNC_CALLBACK_STATUS_CHOOSEANY: number=2. Writable:false, Configurable:false, Enumerable:false
        library->AddMember(debugObject, Js::PropertyIds::MS_ASYNC_CALLBACK_STATUS_CHOOSEANY, Js::JavascriptNumber::New(AsyncDebug::AsyncCallbackStatus_ChooseAny, scriptContext), PropertyNone);

        // Debug.MS_ASYNC_CALLBACK_STATUS_CANCEL: number=3. Writable:false, Configurable:false, Enumerable:false
        library->AddMember(debugObject, Js::PropertyIds::MS_ASYNC_CALLBACK_STATUS_CANCEL, Js::JavascriptNumber::New(AsyncDebug::AsyncCallbackStatus_Cancel, scriptContext), PropertyNone);

        // Debug.MS_ASYNC_CALLBACK_STATUS_ERROR: number=4. Writable:false, Configurable:false, Enumerable:false
        library->AddMember(debugObject, Js::PropertyIds::MS_ASYNC_CALLBACK_STATUS_ERROR, Js::JavascriptNumber::New(AsyncDebug::AsyncCallbackStatus_Error, scriptContext), PropertyNone);

        // Debug.msTraceAsyncOperationStarting([operationName: string=""], [logLevel: number=1]):number. Writable:false, Configurable:false, Enumerable:false
        library->AddFunctionToLibraryObject(debugObject, Js::PropertyIds::msTraceAsyncOperationStarting, &AsyncDebug::EntryInfo::BeginAsyncOperation, 2);
        debugObject->SetAttributes(Js::PropertyIds::msTraceAsyncOperationStarting, PropertyNone);

        // Debug.msTraceAsyncCallbackStarting([asyncOperationId: number=-1], [workType: number=1], [logLevel: number=1]):undefined. Writable:false, Configurable:false, Enumerable:false
        library->AddFunctionToLibraryObject(debugObject, Js::PropertyIds::msTraceAsyncCallbackStarting, &AsyncDebug::EntryInfo::BeginAsyncCallback, 3);
        debugObject->SetAttributes(Js::PropertyIds::msTraceAsyncCallbackStarting, PropertyNone);

        // Debug.msTraceAsyncCallbackCompleted([logLevel: number=1]):undefined. Writable:false, Configurable:false, Enumerable:false
        library->AddFunctionToLibraryObject(debugObject, Js::PropertyIds::msTraceAsyncCallbackCompleted, &AsyncDebug::EntryInfo::CompleteAsyncCallback, 1);
        debugObject->SetAttributes(Js::PropertyIds::msTraceAsyncCallbackCompleted, PropertyNone);

        // Debug.msUpdateAsyncCallbackRelation([relatedAsyncOperationID: number=-1], [relationType: number=5], [logLevel: number=1]):undefined. Writable:false, Configurable:false, Enumerable:false
        library->AddFunctionToLibraryObject(debugObject, Js::PropertyIds::msUpdateAsyncCallbackRelation, &AsyncDebug::EntryInfo::UpdateAsyncCallbackStatus, 3);
        debugObject->SetAttributes(Js::PropertyIds::msUpdateAsyncCallbackRelation, PropertyNone);

        // Debug.msTraceAsyncOperationCompleted([asyncOperationID: number=-1], [status: number=1], [logLevel: number=1]):undefined. Writable:false, Configurable:false, Enumerable:false
        library->AddFunctionToLibraryObject(debugObject, Js::PropertyIds::msTraceAsyncOperationCompleted, &AsyncDebug::EntryInfo::CompleteAsyncOperation, 3);
        debugObject->SetAttributes(Js::PropertyIds::msTraceAsyncOperationCompleted, PropertyNone);
    }

}

#if defined(EDIT_AND_CONTINUE) && defined(ENABLE_DEBUG_CONFIG_OPTIONS)
#include "EditAndContinue.h"

void ScriptSite::InitializeEditTest()
{
    EditAndContinue::InitializeEditTest(this->GetScriptSiteContext());
}
#endif

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
#if DBG_DUMP
void
ScriptSite::SetUrl(BSTR bstrUrl)
{
    charcount_t length = SysStringLen(bstrUrl) + 1; // Add 1 for the NULL.

    char16* urlCopy = HeapNewArray(char16, length);
    js_memcpy_s(urlCopy, (length - 1) * sizeof(char16), bstrUrl, (length - 1) * sizeof(char16));
    urlCopy[length - 1] = _u('\0');
    this->wszLocation = urlCopy;
}
#endif

void
ScriptSite::DumpSiteInfo(char16 const * message, char16 const * message2)
{
    if (message2 == nullptr) { message2 = _u(""); }
#if DBG_DUMP
    Output::Print(_u("%p> ScriptSite %d(%p): %s%s\n\t"), this->threadId, this->allocId, this, message, message2);
    EnsureParentInfo();

    if (this->parentScriptSite != nullptr)
    {
        Output::Print(_u("%s ScriptSite %d(%p)"), _u("Parent"), this->parentAllocId, parentScriptSite);
    }
    else if (this->nextTopLevelScriptSite != nullptr)
    {
        Output::Print(_u("Top Level"));
    }
    else
    {
        Output::Print(_u("Unknown"));
    }

    if (this->wszLocation != nullptr)
    {
        Output::Print(_u(": URL: %s"),  this->wszLocation);
    }
    Output::Print(_u("\n"));
#else
    Output::Print(_u("%p> ScriptSite %p: %s%s\n"), this->threadId, this, message, message2);
#endif
}

#if defined(PROFILE_EXEC) || defined(PROFILE_MEM)
Js::FunctionInfo ProfileOnLoadCallBack::functionInfo(FORCE_NO_WRITE_BARRIER_TAG(ProfileOnLoadCallBack::EntryProfileOnLoadCallBack), Js::FunctionInfo::DoNotProfile);
ProfileOnLoadCallBack::ProfileOnLoadCallBack(ScriptSite * scriptSite)
    : Js::JavascriptFunction(
        scriptSite->GetScriptSiteContext()->GetLibrary()->CreateFunctionType(functionInfo.GetOriginalEntryPoint()),
        &functionInfo),
      scriptSite(scriptSite)
{
}

Var
ProfileOnLoadCallBack::EntryProfileOnLoadCallBack(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{

    ProfileOnLoadCallBack * callback = (ProfileOnLoadCallBack *)function;
    ScriptSite * scriptSite = callback->scriptSite;
    Js::RecyclableObject * hostDispatch = scriptSite->windowHost;
    Js::ScriptContext * scriptContext = scriptSite->GetScriptSiteContext();
    Assert(hostDispatch != nullptr);

    if (Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag)
#ifdef PROFILE_MEMORY
        || MemoryProfiler::IsTraceEnabled()
#endif
        )
    {
        Js::PropertyRecord const * propertyRecord;
        scriptContext->GetOrAddPropertyRecord(_u("document"), _countof(_u("document")) - 1, &propertyRecord);
        PropertyId propertyId = propertyRecord->GetPropertyId();
        Var aValue = Js::JavascriptOperators::GetProperty(hostDispatch, propertyId, scriptContext);
        if (Js::TaggedNumber::Is(aValue))
        {
            return false;
        }
        hostDispatch = Js::RecyclableObject::FromVar(aValue);

        scriptContext->GetOrAddPropertyRecord(_u("readyState"), _countof(_u("readyState")) - 1, &propertyRecord);
        propertyId = propertyRecord->GetPropertyId();
        aValue = Js::JavascriptOperators::GetProperty(hostDispatch, propertyId, scriptContext);
        char16 const * readyState = _u("unknown");
        if (Js::JavascriptString::Is(aValue))
        {
            readyState = Js::JavascriptString::FromVar(aValue)->GetSz();
        }

        scriptSite->DumpSiteInfo(_u("onreadystatechange event: "), readyState);

#ifdef PROFILE_EXEC
        scriptContext->ProfileEnd(Js::RunPhase);
        if (Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag))
        {
            scriptContext->ProfilePrint();
        }
        scriptContext->ProfileBegin(Js::RunPhase);
#endif
#ifdef PROFILE_MEM
        if (MemoryProfiler::IsTraceEnabled())
        {
            MemoryProfiler::PrintCurrentThread();
#ifdef PROFILE_RECYCLER_ALLOC
            if (Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::AllPhase)
                || Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::RunPhase))
            {
                scriptContext->GetRecycler()->PrintAllocStats();
            }
#endif
        }
#endif

        Output::Flush();
    }
    return scriptContext->GetLibrary()->GetUndefined();
}

bool
ProfileOnLoadCallBack::AttachEvent(ScriptSite * scriptSite)
{
    // document.attachEvent("onreadystatechange", callback)
    Js::RecyclableObject * hostDispatch = scriptSite->windowHost;
    Assert(hostDispatch != nullptr);

    Js::ScriptContext * scriptContext = scriptSite->GetScriptSiteContext();

    Js::PropertyRecord const * propertyRecord;
    scriptContext->GetOrAddPropertyRecord(_u("document"), _countof(_u("document")) - 1, &propertyRecord);
    PropertyId propertyId = propertyRecord->GetPropertyId();
    Var aValue = Js::JavascriptOperators::GetProperty(hostDispatch, propertyId, scriptContext);
    if (Js::TaggedNumber::Is(aValue))
    {
        return false;
    }
    hostDispatch = Js::RecyclableObject::FromVar(aValue);
    scriptContext->GetOrAddPropertyRecord(_u("attachEvent"), _countof(_u("attachEvent")) - 1, &propertyRecord);
    propertyId = propertyRecord->GetPropertyId();
    aValue = Js::JavascriptOperators::GetPropertyReference(hostDispatch, propertyId, scriptContext);
    if (Js::TaggedNumber::Is(aValue))
    {
        return false;
    }
    Js::Var values[3] = {
        hostDispatch,
        scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("onreadystatechange")),
        RecyclerNew(scriptContext->GetRecycler(), ProfileOnLoadCallBack, scriptSite)
    };
    Js::CallInfo info(Js::CallFlags_None, 3);
    Js::Arguments args(info, values);
    Js::RecyclableObject * function = Js::RecyclableObject::FromVar(aValue);

    Js::JavascriptFunction::CallFunction<true>(function, function->GetEntryPoint(), args);
    return true;
}
#endif

void
ScriptSite::EnsureParentInfo(Js::ScriptContext* requestContext)
{
    if (requestContext)
    {
        if (requestContext->GetThreadContext()->IsDisableImplicitCall())
        {
            requestContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
            return;
        }
        BEGIN_LEAVE_SCRIPT(requestContext)
        {
            EnsureParentInfoWithScriptEnter();
        }
        END_LEAVE_SCRIPT(requestContext);
    }
    else
    {
        EnsureParentInfoWithScriptEnter();
    }
}

void ScriptSite::EnsureParentInfoWithScriptEnter()
{
    if (hasParentInfo || !this->windowHost)
    {
        return;
    }
    hasParentInfo = true;
    Js::ScriptContext * scriptContext = this->GetScriptSiteContext();

    HRESULT hrIgnored;
    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        Var aValue;
        PropertyId propertyId;
        Js::PropertyRecord const * propertyRecord;
        Assert(this->parentScriptSite == nullptr);

        // check if this is a top level script site.
        scriptContext->GetOrAddPropertyRecord(_u("top"), _countof(_u("top")) - 1, &propertyRecord);
        propertyId = propertyRecord->GetPropertyId();

        // TODO: Catch exception for this call too?  We just got the host dispatch, so it should still be valid
        aValue = Js::JavascriptOperators::GetProperty(windowHost, propertyId, scriptContext);

        if (Js::JavascriptOperators::Equal(windowHost, aValue, scriptContext))
        {
            // this is top level
            ScriptSite* topScriptSite = threadContext->GetTopLevelScriptSite();
            if (topScriptSite)
            {
                this->prevTopLevelScriptSite = topScriptSite->prevTopLevelScriptSite;
                this->nextTopLevelScriptSite = topScriptSite;
                topScriptSite->prevTopLevelScriptSite->nextTopLevelScriptSite = this;
                topScriptSite->prevTopLevelScriptSite = this;
                threadContext->SetTopLevelScriptSite(this);
            }
            else
            {
                threadContext->SetTopLevelScriptSite(this);
                this->prevTopLevelScriptSite = this;
                this->nextTopLevelScriptSite = this;
            }
            bool eventAttached = false;
#ifdef PROFILE_EXEC

            if (Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag))
            {
                scriptContext->SetRecyclerProfiler();
                if (!ProfileOnLoadCallBack::AttachEvent(this))
                {
                    Output::Print(_u("Unable to set up on load event for profile print\n"));
                    Output::Flush();
                }
                eventAttached = true;
            }
#endif
#ifdef PROFILE_MEM
            if (!eventAttached && (MemoryProfiler::IsEnabled() || MemoryProfiler::IsTraceEnabled()))
            {
                if (!ProfileOnLoadCallBack::AttachEvent(this))
                {
                    Output::Print(_u("Unable to set up on load event for profile print\n"));
                    Output::Flush();
                }
            }
#endif
        }
        else if (threadContext->GetTopLevelScriptSite())
        {
            ScriptSite * current = threadContext->GetTopLevelScriptSite();
            do
            {
                if (current->windowHost != nullptr && Js::JavascriptOperators::Equal(aValue, current->windowHost, scriptContext))
                {
                    this->parentScriptSite = current;
#if DBG_DUMP
                    this->parentAllocId = this->parentScriptSite->allocId;
#endif
                    break;
                }
                current = current->nextTopLevelScriptSite;
            }
            while (current != threadContext->GetTopLevelScriptSite());

#ifdef PROFILE_EXEC
            if (this->parentScriptSite != nullptr && Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag))
            {
                this->GetScriptSiteContext()->SetProfilerFromScriptContext(this->parentScriptSite->GetScriptSiteContext());
            }
#endif
#ifdef PROFILE_MEM
            if (this->parentScriptSite != nullptr)
            {
                this->GetScriptSiteContext()->DisableProfileMemoryDumpOnDelete();
            }
#endif
        }

#if DBG_DUMP
        if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::HostPhase))
        {
            this->DumpSiteInfo(_u("Parent info infered"));
            Output::Flush();
        }
#endif
    }
 #pragma prefast(suppress: 28931, "hr is ignored");
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hrIgnored);
}

void
ScriptSite::SetupWindowHost(Js::RecyclableObject * hostObj)
{
    if (windowHost == hostObj)
    {
        return;
    }

    Assert(windowHost == nullptr);
    Assert(!hasParentInfo);

    windowHost = hostObj;

    // delay getting the parent information from top.
    // SetupWindowHost is called during script engine set up, and it may not be fully initialize
    // on the Trident side. Calling back to the DOM may confuse it and create another script enginee

#if DBG_DUMP
    if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::HostPhase))
    {
        if (this->hasParentInfo)
        {
            this->DumpSiteInfo(_u("'window' External Object Added"));
        }
        else
        {
            Output::Print(_u("%p> ScriptSite %d(%p): 'window' External Object Added, parent info defered\n\t"), this->threadId, this->allocId, this);
        }
        Output::Flush();
    }
#endif
}

void ScriptSite::CaptureSetHostObjectTrace()
{
    HRESULT hr = NOERROR;
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        if (this->setHostObjectStackBackTrace)
        {
            setHostObjectStackBackTrace.Unroot(recycler);
        }
        this->setHostObjectStackBackTrace.Root(StackBackTrace::Capture(recycler, ScriptSite::StackToSkip, ScriptSite::StackTraceDepth), recycler);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
}

void ScriptSite::CaptureReinitHostObjectTrace()
{
    HRESULT hr = NOERROR;
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        if (this->reinitHostObjectStackBackTrace)
        {
            this->reinitHostObjectStackBackTrace.Unroot(recycler);
        }
        this->reinitHostObjectStackBackTrace.Root(StackBackTrace::Capture(recycler, ScriptSite::StackToSkip, ScriptSite::StackTraceDepth), recycler);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
}
#endif

HRESULT ScriptSite::RegisterExternalLibrary(Js::ScriptContext *pScriptContext)
{
    HRESULT hr = S_OK;

    REGISTER_OBJECT(Debug);
    if (pScriptContext->GetConfig()->IsWinRTEnabled())
    {
        REGISTER_ERROR_OBJECT(WinRTError);
    }

    return hr;
}

HRESULT ScriptSite::RegisterDebug(Js::ScriptContext *pScriptContext)
{
    Assert(pScriptContext);
    HRESULT hr = S_OK;
    DEFINE_OBJECT_NAME(Debug);

    REG_OBJECTS_LIB_FUNC(write, DebugObject::EntryWrite);
    REG_OBJECTS_LIB_FUNC(writeln, DebugObject::EntryWriteLine);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("getWorkingSet"), DebugObject::EntryGetWorkingSet);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("sourceDebugBreak"), DebugObject::EntrySourceDebugBreak);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("invokeFunction"), DebugObject::EntryInvokeFunction);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("isInJit"), DebugObject::EntryIsInJit);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("getMemoryInfo"), DebugObject::EntryGetMemoryInfo);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("getHostInfo"), DebugObject::EntryGetHostInfo);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("getTypeHandlerName"), DebugObject::EntryGetTypeHandlerName);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("getArrayType"), DebugObject::EntryGetArrayType);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("dumpHeap"), DebugObject::DumpHeapInternal);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("createDebugDisposableObject"), DebugObject::EntryCreateDebugDisposableObject);
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("createDebugFuncExecutorInDisposeObject"), DebugObject::EntryCreateDebugFuncExecutorInDisposeObject);
#endif
#if JS_PROFILE_DATA_INTERFACE
    REG_OBJECTS_DYNAMIC_LIB_FUNC(_u("getProfileDataObject"), DebugObject::EntryGetProfileDataObject);
#endif

    if (CONFIG_FLAG(AsyncDebugging))
    {
        REG_OBJECTS_LIB_FUNC(msTraceAsyncOperationStarting, AsyncDebug::BeginAsyncOperation);
        REG_OBJECTS_LIB_FUNC(msTraceAsyncCallbackStarting, AsyncDebug::BeginAsyncCallback);
        REG_OBJECTS_LIB_FUNC(msTraceAsyncCallbackCompleted, AsyncDebug::CompleteAsyncCallback);
        REG_OBJECTS_LIB_FUNC(msUpdateAsyncCallbackRelation, AsyncDebug::UpdateAsyncCallbackStatus);
        REG_OBJECTS_LIB_FUNC(msTraceAsyncOperationCompleted, AsyncDebug::CompleteAsyncOperation);
    }

    return hr;
}

HRESULT ScriptSite::VerifyStackOnEntry(BOOL allowedInHeapEnum)
{
    if (!threadContext->IsStackAvailableNoThrow())
    {
        return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
    }
    else if (threadContext->GetRecycler()->IsHeapEnumInProgress() && !allowedInHeapEnum)
    {
        Assert(FALSE);
        return E_UNEXPECTED;
    }
    else
    {
        return S_OK;
    }
}

HRESULT ScriptSite::VerifyDOMSecurity(Js::ScriptContext* targetContext, Js::Var obj)
{
    HRESULT hr = NOERROR;
    ScriptSite* targetSite = ScriptSite::FromScriptContext(targetContext);
    IActiveScriptSite* srcIActiveScriptSite = NULL, *destIActiveScriptSite = NULL;
    if (IsClosed() || targetContext->IsClosed() || targetContext->IsInvalidatedForHostObjects())
    {
        return E_ACCESSDENIED;
    }
    hr = GetScriptEngine()->GetIActiveScriptSite(&srcIActiveScriptSite);
    if (SUCCEEDED(hr))
    {
        hr = targetSite->GetScriptEngine()->GetIActiveScriptSite(&destIActiveScriptSite);
    }
    if (SUCCEEDED(hr))
    {
        IActiveScriptDirectHost* scriptHost =  GetScriptEngine()->GetActiveScriptDirectHostNoRef();
        if (scriptHost == nullptr)
        {
            hr = E_ACCESSDENIED;
        }
        else
        {
            hr = scriptHost->ValidateCall(srcIActiveScriptSite, destIActiveScriptSite, obj);
        }
    }

    return hr;
}

// For CMDID_SCRIPTSITE_SID
#include <mshtmhst.h>

HRESULT ScriptSite::CheckCrossDomainScriptContext(__in Js::ScriptContext* remoteScriptContext)
{
    // For real cross domain throw failures we need to differentiate an actual SID failure versus a script
    // site not being available due to closure. While we do a top level script context closure check Trident
    // can also detach a site during navigation before the script site actually closes. In that case we won't
    // be able to get the command targets to eventually get the SIDs. Propagate E_FAIL for this case.
    HRESULT hrReturn = E_FAIL;

    // Refuse access if the script site or the originating script context is already closed
    if ( !IsClosed() && !remoteScriptContext->IsClosed() )
    {
        ScriptSite* remoteScriptSite = ScriptSite::FromScriptContext(remoteScriptContext);
        if ( remoteScriptSite )
        {
            IOleCommandTarget* pLocalCommandTarget = NULL;
            HRESULT hr = scriptEngine->GetScriptSite(__uuidof(IOleCommandTarget), (void**)&pLocalCommandTarget);
            if ( S_OK == hr )
            {
                VARIANTARG varLocalSid;
                VariantInit(&varLocalSid);

                hr = pLocalCommandTarget->Exec(&CGID_ScriptSite, CMDID_SCRIPTSITE_SID, 0, NULL, &varLocalSid);
                if ( S_OK == hr )
                {
                    IOleCommandTarget* pRemoteCommandTarget = NULL;
                    hr = remoteScriptSite->scriptEngine->GetScriptSite(__uuidof(IOleCommandTarget), (void**)&pRemoteCommandTarget);
                    if ( S_OK == hr )
                    {
                        VARIANTARG varRemoteSid;
                        VariantInit(&varRemoteSid);

                        hr = pRemoteCommandTarget->Exec(&CGID_ScriptSite, CMDID_SCRIPTSITE_SID, 0, NULL, &varRemoteSid);
                        if ( S_OK == hr )
                        {
                            // Here is where we determine real cross domain failure versus same domain.
                            if ( 0 == memcmp(V_BSTR(&varLocalSid), V_BSTR(&varRemoteSid), MAX_SIZE_SECURITY_ID) )
                            {
                                hrReturn = S_OK;
                            }
                            else
                            {
                                hrReturn = E_ACCESSDENIED;
                            }

                            VariantClear(&varRemoteSid);
                        }

                        pRemoteCommandTarget->Release();
                    }

                    VariantClear(&varLocalSid);
                }

                pLocalCommandTarget->Release();
            }
        }
    }

    return hrReturn;
}

// keep track of number of active JavascriptDispatch; release the keepalive engine
// when the count goes down to 0.
ULONG ScriptSite::ReleaseDispatchCount()
{
    ULONG currentCount = InterlockedDecrement(&outstandingDispatchCount);
    if (currentCount == 0 && activeScriptKeepAlive)
    {
        activeScriptKeepAlive->Release();
        activeScriptKeepAlive = NULL;
    }
    return currentCount;
}

// keep script engine alive as needed.
ULONG ScriptSite::AddDispatchCount()
{
    ULONG newCount = InterlockedIncrement(&outstandingDispatchCount);
    if (newCount == 1)
    {
        // if ShouldKeepAlive flag is set (via IActiveScriptProperty::SetProperty),
        // we should keep the script engine alive with outstanding JavascriptDispatch.
        // setup activeScriptKeepAlive with strong refcount to keep engine alive
        // as long as there is outstand JavascriptDispatch.
        if (scriptEngine && scriptEngine->ShouldKeepAlive())
        {
            HRESULT hr;
            Assert(activeScriptKeepAlive == NULL);
            hr = scriptEngine->QueryInterface(IID_IActiveScript, (void**)&activeScriptKeepAlive);
            Assert(SUCCEEDED(hr));
        }
    }
    return newCount;
}

IDispatch* ScriptSite::GetGlobalObjectDispatch()
{
    JavascriptDispatch* globalDispatch = JavascriptDispatch::Create<false>(this->GetScriptSiteContext()->GetGlobalObject());
    return globalDispatch->GetThis();
}

Js::JavascriptFunction* ScriptSite::GetDefaultSlotGetter(bool isObject, JavascriptTypeId typeId, PropertyId nameId, unsigned int slotIndex, ScriptMethod fallBack)
{
    Js::FunctionInfo* functionInfo = isObject ? ::DOMFastPathInfo::GetObjectGetterInfo(slotIndex) : ::DOMFastPathInfo::GetTypeGetterInfo(slotIndex);
    Js::JavascriptFunction* getterFunction = GetActiveScriptExternalLibrary()->CreateSlotGetterFunction(isObject, slotIndex, functionInfo, typeId, nameId, fallBack);
    getterFunction->SetPropertyWithAttributes(Js::PropertyIds::length, Js::TaggedInt::ToVarUnchecked(0), PropertyNone, NULL);

    return getterFunction;
}

Js::JavascriptFunction* ScriptSite::GetDefaultSlotSetter(bool isObject, JavascriptTypeId typeId, PropertyId nameId, unsigned int slotIndex, ScriptMethod fallBack)
{
    Js::FunctionInfo* functionInfo = isObject ? ::DOMFastPathInfo::GetObjectSetterInfo(slotIndex) : ::DOMFastPathInfo::GetTypeSetterInfo(slotIndex);
    Js::JavascriptFunction* setterFunction = GetActiveScriptExternalLibrary()->CreateSlotSetterFunction(isObject, slotIndex, functionInfo, typeId, nameId, fallBack);
    setterFunction->SetPropertyWithAttributes(Js::PropertyIds::length, Js::TaggedInt::ToVarUnchecked(1), PropertyNone, NULL);

    return setterFunction;
}

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
void ScriptSite::EnumHeap()
{
    IActiveScriptProfilerHeapEnum* pEnum = NULL;
    HRESULT hr = scriptEngine->EnumHeap(&pEnum);
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        return;
    }
    const size_t snapshotChunkSize = 1000;
    PROFILER_HEAP_OBJECT* snapshotChunk[snapshotChunkSize];
    do {
        ULONG numFetched = 0;
        hr = pEnum->Next(snapshotChunkSize, snapshotChunk, &numFetched);
        if (numFetched == 0 || FAILED(hr)) break;
    } while (TRUE);
    Assert(SUCCEEDED(hr));
    pEnum->Release();
}

#endif

