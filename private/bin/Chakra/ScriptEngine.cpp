//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "share.h"
#pragma hdrstop
#include <fcntl.h>
#ifdef ENABLE_JS_ETW
#include <IERESP_mshtml.h>
#include "microsoft-scripting-jscript9.internalevents.h"
#endif
#include "webplatform.h"
#include "edgejsStatic.h"
#include "GenerateByteCodeConfig.h"

#define USE_ARENA false    // make this true to disable the memory recycler.

#define Compile (this->*(CompileFunction))

HRESULT LoadLatestPDM(__in REFCLSID rclsid, __in REFIID riid, __out LPVOID* ppPDM);

EXTERN_C const GUID GUID_CUSTOM_CONFIRMOBJECTSAFETY;

const LCID k_lcidUSEnglish = 0x409;

// Be careful not to let this collide with activscp.h definitions
#define SCRIPTTEXT_FORCEEXECUTION 0x80000000
#define SCRIPTTEXT_ISSCRIPTLET    0x40000000

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#define RAISE_FATL_INTERNAL_ERROR_IFFAILED(hr) if (hr != S_OK) Js::Throw::FatalInternalError();
#else
#define RAISE_FATL_INTERNAL_ERROR_IFFAILED(hr) 
#endif

interface IDebugHelperExOld : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowserFromError(
        /* [in] */ IActiveScriptError __RPC_FAR *pase,
        /* [in] */ LPCOLESTR pszName,
        /* [in] */ IDebugApplicationThread __RPC_FAR *pdat,
        /* [in] */ IDebugFormatter __RPC_FAR *pdf,
        /* [out] */ IDebugProperty __RPC_FAR *__RPC_FAR *ppdp) = 0;
};

// Empty exception structure
const EXCEPINFO NoException = { 0, 0, nullptr, nullptr, nullptr, 0, nullptr, nullptr, 0 };

// Local functions:

ulong ComputeGrfscrUTF16(const void *pDelimiter)
{
    LPCOLESTR pszDelimiter = (LPCOLESTR) pDelimiter;
    ulong grfscr = 0;
    if (nullptr != pszDelimiter)
    {
        if (pszDelimiter[0] == OLESTR('"') && pszDelimiter[1] == OLESTR('\0'))
            grfscr |= fscrMapQuote;
        else if (0 == ostricmp(OLESTR("</script>"), pszDelimiter))
            grfscr |= fscrHtmlComments;
        else if (0 == ostricmp(OLESTR("STRIP EMBEDDED HTML COMMENTS"), pszDelimiter))
            grfscr |= fscrHtmlComments|fscrDoNotHandleTrailingHtmlComments;
    }
    return grfscr;
}

ulong ComputeGrfscrUTF8(const void * pDelimiter)
{
    const char *pszDelimiter = (const char *)pDelimiter;
    ulong grfscr = 0;
    if (nullptr != pszDelimiter)
    {
        if (pszDelimiter[0] == '"' && pszDelimiter[1] == '\0')
            grfscr |= fscrMapQuote;
        else if (0 == _stricmp("</script>", pszDelimiter))
            grfscr |= fscrHtmlComments;
        else if (0 == _stricmp("STRIP EMBEDDED HTML COMMENTS", pszDelimiter))
            grfscr |= fscrHtmlComments|fscrDoNotHandleTrailingHtmlComments;
    }
    return grfscr;
}

ulong ComputeGrfscrUTF8ForSerialization(const void * pDelimiter)
{
    ulong result = ComputeGrfscrUTF8(pDelimiter);
    return result | fscrNoPreJit;
}

ulong ComputeGrfscrUTF8ForNativeSerialization(const void * pDelimiter)
{
    ulong result = ComputeGrfscrUTF8(pDelimiter);
    return result | fscrNoPreJit | fscrNoDeferParse | fscrIsNativeCode;
}

ulong ComputeGrfscrUTF8ForNativeSerializationDeserialization(const void * pDelimiter)
{
    ulong result = ComputeGrfscrUTF8(pDelimiter);
    // We don't want to allow proxies since we're going to generate all these functions right now
    return result | fscrIsNativeCode;
}

ulong ComputeGrfscrUTF8ForDeserialization(const void * pDelimiter)
{
    ulong result = ComputeGrfscrUTF8(pDelimiter);

    if(CONFIG_FLAG(CreateFunctionProxy))
    {
        result = result | fscrAllowFunctionProxy;
    }
    return result;
}

ulong ComputeGrfscrUTF8ForNativeDeserialization(const void * pDelimiter)
{
    ulong result = ComputeGrfscrUTF8(pDelimiter);
    if(CONFIG_FLAG(CreateFunctionProxy))
    {
        result = result | fscrAllowFunctionProxy;
    }
    return result | fscrIsNativeCode;
}

#if _WIN32 || _WIN64
template <class T>
charcount_t GetLengthExcludingHTMLCommentSuffix(const T * pszSrc, charcount_t cchSrc, Js::ScriptContext *scriptContext)
{
    Assert(cchSrc > 0);
    if (cchSrc < 1 || nullptr == pszSrc)
        return cchSrc;

    charcount_t len = cchSrc - 1;
    while (len > 3 && scriptContext->GetCharClassifier()->IsWhiteSpace(pszSrc[len]))
        len--;

    if (len < 3 ||
        '>' != pszSrc[len--] ||
        '-' != pszSrc[len--] ||
        '-' != pszSrc[len--]
    )
    {
        return cchSrc;
    }

    //saw a --> delimiter
    //now run back until there is an EOL, a // or a <!--
    while (len > 0)
    {
        if ('\n' == pszSrc[len] || '\r' == pszSrc[len])
        {
            len++; // include the EOL
            break; // saw an EOL
        }
        if (len >= 1 && '/' == pszSrc[len-1] && '/' == pszSrc[len])
        {
            len--; // exclude the first // of a // comment.
            break; //saw a //
        }
        if (len >= 3 &&
            '<' == pszSrc[len-3] &&
            '!' == pszSrc[len-2] &&
            '-' == pszSrc[len-1] &&
            '-' == pszSrc[len]
        )
        {
            len -= 3; // exclude the <!-- --> comment.
            break; //saw a <!--
        }
        len--;
    }

    return len;
}
#else
#error Neither  _WIN32, nor _WIN64 is defined
#endif

ScriptEngine::ScriptEngine(REFIID riidLanguage, LPCOLESTR pszLanguageName)
    : ScriptEngineBase()
    , m_riidLanguage(riidLanguage)
    , m_pszLanguageName(pszLanguageName)
    , scriptAllocator(nullptr)
    , CompileFunction(&ScriptEngine::DefaultCompile)
    , m_dwTicksPerPoll(DefaultTicksPerPoll)
    , m_fIsEvalRestrict(false)
    , m_fAllowWinRTConstructor(false)
    , m_activityID(GUID_NULL)
{
#if DEBUG
    //    dprintf("New %s script engine -> %p\n", g_pstrDLLName, this);
#endif // DEBUG
    m_lwCookieCount         = 0;
    m_refCount                  = 1;
    m_ssState               = SCRIPTSTATE_UNINITIALIZED;
    m_pActiveScriptSite     = nullptr;
    m_activeScriptDirectHost = nullptr;
    m_dwBaseThread          = NOBASETHREAD;
    m_fPersistLoaded        = FALSE;                         // Not loaded yet
    m_stsThreadState        = SCRIPTTHREADSTATE_NOTINSCRIPT; // Not running in a script
    m_cNesting              = 0;                             // Count of times nested in script
    m_moduleIDNext               = 1;                             // Next module number. 0 is global!
    m_lcidUser              = GetDefaultLocale();    // Default locale
    m_lcidUserDefault       = m_lcidUser;
    m_fIsValidCodePage      = TRUE;
    m_codepage              = GetACP();

    m_cbMinStackHost        = Js::Constants::MinStackHost;
    m_excepinfoInterrupt    = NoException;       // If interrupt raised, exception information
    pendingCloneSource      = nullptr;
    // Debugger
    m_scriptSiteDebug       = nullptr;
    m_pNonDebugDocFirst     = nullptr;
    m_pda                   = nullptr;
    m_debugApplicationThread= nullptr;
    m_debugHelper           = nullptr;
    m_debugFormatter        = nullptr;

    m_pcpAppEvents          = nullptr;
    m_dwAppAdviseID         = 0;
    m_dwSnifferCookie       = 0;

    m_fDumbHost = TRUE;
    fKeepEngineAlive = FALSE;
    fSetThreadDescription = FALSE;        
    m_isHostInDebugMode = FALSE;
    m_hasFailedToAttachDetach = FALSE;
    m_isFirstSourceCompile = TRUE;
    // End Debugger

    m_isDiagnosticsOM = FALSE;

    m_fIsPseudoDisconnected = FALSE;  // True if pretending to be disconnected
    m_fClearingDebugDocuments = FALSE;

    m_pglbod                = nullptr;
    eventHandlers                = nullptr;
    eventSinks            = nullptr;
    jsOps=nullptr;

    m_fNoHostSecurityManager = TRUE;
    m_fNoINETSecurityManager = TRUE;
    m_psitehostsecman        = nullptr;
    m_pinetsecman            = nullptr;

    //m_pvLastReportedScriptBody = nullptr;

#if ECMACP
    m_fECMACP               = FALSE;
#endif // ECMACP

    hostType = SCRIPTHOSTTYPE_DEFAULT;
    fCanOptimizeGlobalLookup = FALSE;
    fNonPrimaryEngine = FALSE;
    webWorkerID = Js::Constants::NonWebWorkerContextId;

    scriptContext = nullptr;
    isCloned = FALSE;
    wasScriptDirectEnabled = FALSE;
    scriptBodyMap = nullptr;
    debugStackFrame = nullptr;

    wasBinaryVerified = TRUE;

#ifdef ENABLE_PROJECTION
    projectionContext = nullptr;
#endif

    DLLAddRef(); // One DLL reference for each existing script engine

#ifdef TELEMETRY
    ScriptEngineTelemetry::Initialize();
#endif
}

ScriptEngine::~ScriptEngine()
{
    Assert(m_refCount == 0);
#if DEBUG
    //    dprintf("Deleting %s script engine %p\n", g_pstrDLLName, this);
#endif // DEBUG
    Assert(fKeepEngineAlive || (m_ssState == SCRIPTSTATE_CLOSED || m_ssState == SCRIPTSTATE_UNINITIALIZED));

#ifdef TELEMETRY
    ScriptEngineTelemetry::Cleanup();
#endif

    Close();

    DeleteCriticalSection(&m_csInterrupt);
    FreeExcepInfo(&m_excepinfoInterrupt);

    DLLRelease(); // One DLL reference for each existing script engine
}

BOOL ScriptEngine::InitializeThreadBound()
{
    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        ThreadContext *threadContext = ThreadBoundThreadContextManager::EnsureContextForCurrentThread();

        if (threadContext == nullptr)
        {
            return false;
        }

        hr = this->Initialize(threadContext);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr == S_OK;
}

HRESULT ScriptEngine::Initialize(ThreadContext * threadContext)
{
    Assert(threadContext != nullptr);
    HRESULT hr = NOERROR;
    this->threadContext = threadContext;
    m_dwSafetyOptions = INTERFACE_USES_DISPEX;

    // starting from Vista InitializeCriticalSection does not raise exception
    InitializeCriticalSection(&m_csInterrupt); // "Interrupt" critical section
    scriptAllocator = HeapNew(ArenaAllocator, L"ScriptEngine", this->threadContext->GetPageAllocator(), Js::Throw::OutOfMemory);
    eventHandlers = JsUtil::List<BaseEventHandler*, ArenaAllocator>::New(scriptAllocator);
    eventSinks = JsUtil::List<EventSink*, ArenaAllocator>::New(scriptAllocator);
    return hr;
}

Js::ScriptContext *
ScriptEngine::EnsureScriptContext()
{
    if (this->scriptContext != nullptr)
    {
        return this->scriptContext;
    }

    ThreadContext* localThreadContext = threadContext;

    AutoPtr<Js::ScriptContext> newScriptContext(Js::ScriptContext::New(localThreadContext));

    AssertMem(localThreadContext->Diagnostics);

    newScriptContext->DispatchDefaultInvoke = DispMemberProxy::DefaultInvoke;
    newScriptContext->DispatchProfileInoke = DispMemberProxy::ProfileInvoke;

    newScriptContext->SetRaiseMessageToDebuggerFunction(ScriptEngine::RaiseMessageToDebugger);
    newScriptContext->SetTransitionToDebugModeIfFirstSourceFn(ScriptEngine::TransitionToDebugModeIfFirstSource);
    bool forceDiagMode = CONFIG_FLAG(ForceDiagnosticsMode);

    // For now if the debugging Script option in IE is enabled then we use the interpreter
    if (forceDiagMode || this->IsDebuggerEnvironmentAvailable(/*requery*/ true))
    {
        if (!Js::Configuration::Global.EnableJitInDebugMode())
        {
            newScriptContext->ForceNoNative();
        }

        newScriptContext->SetInDebugMode();
        newScriptContext->SetDbgRegisterFunction(ScriptEngine::DbgRegisterFunction);
    }
    else if (this->CanRegisterDebugSources())
    {
        // We're in source rundown mode.
        newScriptContext->SetInSourceRundownMode();
        newScriptContext->SetDbgRegisterFunction(ScriptEngine::DbgRegisterFunction);
    }

    // Initialize with WebWorker State
    newScriptContext->webWorkerId = webWorkerID;
    newScriptContext->SetIsDiagnosticsScriptContext(!!m_isDiagnosticsOM);

    if (SCRIPTHOSTTYPE_DEFAULT != this->GetHostType())
    {
        // Let jscript.config be able to override the host type.
        if (!Js::Configuration::Global.flags.IsEnabled(Js::HostTypeFlag))
        {
            newScriptContext->SetHostType(this->GetHostType());
        }
    }
    // Let jscript.config be able to override the host type.
    if (!Js::Configuration::Global.flags.IsEnabled(Js::WinRTConstructorAllowedFlag))
    {
        newScriptContext->SetWinRTConstructorAllowed(this->m_fAllowWinRTConstructor == TRUE);
    }
    newScriptContext->SetCanOptimizeGlobalLookupFlag(fCanOptimizeGlobalLookup);

    newScriptContext->Initialize();

    this->scriptContext = newScriptContext.Detach();
    this->scriptContext->GetDocumentContext = ScriptEngine::GetDebugDocumentContextFromHostPosition;
    this->scriptContext->CleanupDocumentContext = ScriptEngine::CleanupDocumentContextList;

    Js::JavascriptLibrary *library = scriptContext->GetLibrary();
    Assert(library != nullptr);

    library->GetEvalFunctionObject()->SetEntryPoint(m_fIsEvalRestrict ? &Js::GlobalObject::EntryEvalRestrictedMode : &Js::GlobalObject::EntryEval);
    library->GetFunctionConstructor()->SetEntryPoint(m_fIsEvalRestrict ? &Js::JavascriptFunction::NewInstanceRestrictedMode : &Js::JavascriptFunction::NewInstance);

    return this->scriptContext;
}


// *** IUnknown Methods ***
STDMETHODIMP ScriptEngine::QueryInterface(
    /* [in]  */ REFIID riid,
    /* [out] */ void **ppvObj)
{
    VALIDATE_WRITE_POINTER(ppvObj, void *);
    HRESULT hr;
    hr = __super::QueryInterface(riid, ppvObj);
    if (SUCCEEDED(hr))
    {
        return hr;
    }
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = static_cast<IUnknown*>(static_cast<IActiveScript*>(this));
        AddRef();
        return NOERROR;
    }
    QI_IMPL(IID_IActiveScript, IActiveScript);
    QI_IMPL(__uuidof(IActiveScriptGarbageCollector), IActiveScriptGarbageCollector);
#ifdef ENABLE_PROJECTION
#ifdef WIN8_COMPAT
    if (Js::Configuration::Global.flags.Win8Compat)
    {
        QI_IMPL(__uuidof(IActiveScriptProjection), IActiveScriptProjectionWin8);
    }
#endif
    QI_IMPL(__uuidof(IActiveScriptProjection), IActiveScriptProjection);
    QI_IMPL(__uuidof(IPrivateScriptProjection), IPrivateScriptProjection);
#endif
    QI_IMPL(IID_IActiveScriptProperty, IActiveScriptProperty);
    QI_IMPL(IID_IVariantChangeType, IVariantChangeType);
    QI_IMPL(IID_IRemoteDebugApplicationEvents, IRemoteDebugApplicationEvents);
    QI_IMPL(IID_IObjectSafety, IObjectSafety);
    QI_IMPL(__uuidof(IActiveScriptByteCode), IActiveScriptByteCode);
    QI_IMPL(__uuidof(IActiveScriptNativeCode), IActiveScriptNativeCode);
    QI_IMPL(__uuidof(IActiveScriptLifecycleEventSink), IActiveScriptLifecycleEventSink);
    QI_IMPL(__uuidof(IDiagnosticsContextHelper), IDiagnosticsContextHelper);
    QI_IMPL(__uuidof(IActiveScriptDebugAttach), IActiveScriptDebugAttach);
    QI_IMPL(__uuidof(IScriptInvocationContextSubscriber), IScriptInvocationContextSubscriber);
    QI_IMPL(__uuidof(IActiveScriptDirectAsyncCausality), IActiveScriptDirectAsyncCausality);

#if !_WIN64 || USE_32_OR_64_BIT
    QI_IMPL(IID_IActiveScriptParse32, IActiveScriptParse32);
    QI_IMPL(IID_IActiveScriptParseProcedure2_32, IActiveScriptParseProcedure);
    QI_IMPL(__uuidof(IActiveScriptParseUTF832), IActiveScriptParseUTF832);
#endif // !_WIN64 || USE_32_OR_64_BIT

#if _WIN64 || USE_32_OR_64_BIT
    QI_IMPL(IID_IActiveScriptParse64, IActiveScriptParse);
    QI_IMPL(IID_IActiveScriptParseProcedure2_64, IActiveScriptParseProcedure);
    QI_IMPL(__uuidof(IActiveScriptParseUTF864), IActiveScriptParseUTF864);
#endif // _WIN64 || USE_32_OR_64_BIT

    // Profiler Interface
    QI_IMPL(IID_IActiveScriptProfilerControl, IActiveScriptProfilerControl);
    QI_IMPL(IID_IActiveScriptProfilerControl2, IActiveScriptProfilerControl);
    if (GetScriptContext())
    {
        QI_IMPL(__uuidof(IActiveScriptProfilerControl3), IActiveScriptProfilerControl);
        QI_IMPL(__uuidof(IActiveScriptProfilerControl4), IActiveScriptProfilerControl);
        QI_IMPL(__uuidof(IActiveScriptProfilerControl5), IActiveScriptProfilerControl);
#ifdef ENABLE_HEAP_DUMPER
        {
            QI_IMPL(__uuidof(IHeapDumper), IHeapDumper);
        }
#endif
    }

#ifdef EDIT_AND_CONTINUE
    QI_IMPL(__uuidof(IActiveScriptEdit), IActiveScriptEdit);
#endif

#if !_WIN64 || USE_32_OR_64_BIT
    QI_IMPL(IID_IActiveScriptDebug32, IActiveScriptDebug32);
#endif // !_WIN64 || USE_32_OR_64_BIT

#if _WIN64 || USE_32_OR_64_BIT
    QI_IMPL(IID_IActiveScriptDebug64, IActiveScriptDebug64);
#endif // _WIN64 || USE_32_OR_64_BIT

    QI_IMPL(IID_IActiveScriptStats, IActiveScriptStats);
    *ppvObj = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ScriptEngine::AddRef(void)
{
    return ScriptEngineBase::AddRef();
}

STDMETHODIMP_(ULONG) ScriptEngine::Release(void)
{
    return ScriptEngineBase::Release();
}



// Test hook for the profiler
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
HRESULT ScriptEngine::StartScriptProfiling(__in IActiveScriptProfilerCallback * profilerObject, __in DWORD dwEventMask, __in DWORD dwContext)
{
    HRESULT hr = E_INVALIDARG;
    if (profilerObject)
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            hr = this->StartProfilingInternal(profilerObject, dwEventMask, dwContext);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
    }
    return hr;
}
#endif

// *** IActiveScriptProfilerControl ***
STDMETHODIMP ScriptEngine::StartProfiling(
    __RPC__in REFCLSID clsidProfilerObject,
    __in DWORD dwEventMask,
    __in DWORD dwContext)
{
    HRESULT hr = E_FAIL;

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        IActiveScriptProfilerCallback *pProfileCallback = nullptr;
        hr = CoCreateInstance(
            clsidProfilerObject,
            nullptr,
            CLSCTX_INPROC_SERVER,
            __uuidof(IActiveScriptProfilerCallback),
            (void **)&pProfileCallback);

        if (SUCCEEDED(hr))
        {
            hr = this->StartProfilingInternal(pProfileCallback, dwEventMask, dwContext);
            pProfileCallback->Release();
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    return hr;
}

HRESULT ScriptEngine::CheckForExternalProfiler(IActiveScriptProfilerCallback **ppProfileCallback)
{
    // guid format - '{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}'
    WCHAR guid[39] = {0};
    CLSID clsid = {0};
    HRESULT hr = E_FAIL;

    DWORD dwResult = GetEnvironmentVariableW(L"JS_PROFILER", guid, 39);
    if (0 != dwResult)
    {
        AssertMsg(dwResult <= 39, "Environment variable JS_PROFILER should be of the format '{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}' (braces should also be present)");

        hr = IIDFromString (guid, &clsid);
        AssertMsg(SUCCEEDED(hr), "Environment variable JS_PROFILER is not a valid GUID");

        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(
                clsid,
                nullptr,
                CLSCTX_INPROC_SERVER,
                __uuidof(IActiveScriptProfilerCallback),
                (void **)ppProfileCallback);

            AssertMsg(SUCCEEDED(hr), "Unable to create Profiler");
        }
    }

    return hr;
}

HRESULT ScriptEngine::StartProfilingInternal(
    __in IActiveScriptProfilerCallback *pProfileCallback,
    __in DWORD dwEventMask,
    __in DWORD dwContext)
{
#ifdef ENABLE_NATIVE_CODEGEN

    OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptEngine::StartProfilingInternal\n");

    // Profiler only from the script engine thread
    if (GetCurrentThreadId() != m_dwBaseThread)
    {
        return E_FAIL;
    }

    HRESULT hr = this->scriptContext->RegisterProfileProbe(pProfileCallback, dwEventMask, dwContext, ScriptSite::RegisterExternalLibrary, DispMemberProxy::ProfileInvoke);

    if (FAILED(hr))
    {
        return hr;
    }

    this->CompileFunction = &ScriptEngine::ProfileModeCompile;

    return hr;

#else // !ENABLE_NATIVE_CODEGEN
    return E_NOTIMPL;
#endif
}

STDMETHODIMP ScriptEngine::SetProfilerEventMask(
    __in DWORD dwEventMask)
{
    OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptEngine::SetProfilerEventMask\n");

    // Profiler only from the script engine thread
    if (GetCurrentThreadId() != m_dwBaseThread)
    {
        return E_FAIL;
    }

    return this->scriptContext->SetProfileEventMask(dwEventMask);
}

STDMETHODIMP ScriptEngine::StopProfiling(
    __in HRESULT hrShutdownReason)
{
#ifdef ENABLE_NATIVE_CODEGEN
    OUTPUT_TRACE(Js::ScriptProfilerPhase, L"ScriptEngine::StopProfiling\n");
    // Profiler only from the script engine thread
    if (GetCurrentThreadId() != m_dwBaseThread)
    {
        return E_FAIL;
    }

    this->CompileFunction = &ScriptEngine::DefaultCompile;
    return this->scriptContext->DeRegisterProfileProbe(hrShutdownReason, DispMemberProxy::DefaultInvoke);
#else
    return E_FAIL;
#endif
}

// *** IActiveScriptProfilerControl2 ***
STDMETHODIMP ScriptEngine::CompleteProfilerStart()
{
    return E_NOTIMPL;
}

STDMETHODIMP ScriptEngine::PrepareProfilerStop()
{
    return E_NOTIMPL;
}

HRESULT ScriptEngine::CreateHeapEnum(
    ActiveScriptProfilerHeapEnum **ppEnum,
    bool preEnumHeap2,
    PROFILER_HEAP_ENUM_FLAGS enumFlags /*= PROFILER_HEAP_ENUM_FLAGS::PROFILER_HEAP_ENUM_FLAGS_NONE*/)
{
    HRESULT hr = S_OK;

    Recycler* recycler = threadContext->GetRecycler();
    Assert(recycler);
    recycler->CollectNow<CollectNowForceInThread>();

    ActiveScriptProfilerHeapEnum *pHeapEnum = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        pHeapEnum = HeapNew(ActiveScriptProfilerHeapEnum, *recycler, *this, scriptAllocator, preEnumHeap2, enumFlags);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    if (SUCCEEDED(hr))
    {
        hr = pHeapEnum->Init();
        if (SUCCEEDED(hr))
        {
            *ppEnum = pHeapEnum;
            return hr;
        }
    }
    if (pHeapEnum)
    {
        pHeapEnum->Release();
    }
    return hr;
}

// *** IActiveScriptProfilerControl3 ***
STDMETHODIMP ScriptEngine::EnumHeap(IActiveScriptProfilerHeapEnum** ppEnum)
{
    return EnumHeap2(PROFILER_HEAP_ENUM_FLAGS::PROFILER_HEAP_ENUM_FLAGS_NONE, ppEnum);
}

#ifdef ENABLE_HEAP_DUMPER
// *** IHeapDumper ***
STDMETHODIMP ScriptEngine::DumpHeap(const WCHAR* outputFile, HeapDumperObjectToDumpFlag objectsToDump, BOOL minimalDump, BOOL dumpAllEngines)
{
    HRESULT hr = S_OK;
    IFFAILRET(VerifyOnEntry());

    if (! outputFile)
    {
        return E_POINTER;
    }

    // if file already opened, don't set it here. Just ignore this one.
    if (! Output::GetOutputFile())
    {
        HRESULT hr = ConfigParser::s_moduleConfigParser.SetOutputFile(outputFile, L"wt");
        if (FAILED(hr))
        {
            return hr;
        }
    }

#ifndef ENABLE_DEBUG_CONFIG_OPTIONS
    ScriptSite::RegisterDebugDumpHeap(scriptContext);
#endif
    if (objectsToDump != HeapDumperObjectToDumpFlag::HeapDumperNoDumpRegisterOnly)
    {
        BEGIN_ENTER_SCRIPT(scriptContext, /*doCleanup*/ true, /*isCallRoot*/ true, /*hasCaller*/ false)
        {
            HeapDumper heapDumper(*this, (HeapDumperObjectToDumpFlag)objectsToDump, minimalDump);
            heapDumper.DumpHeap();
        }
        END_ENTER_SCRIPT(hr);
    }
    return hr;
}
#endif

// *** IActiveScriptEdit ***
#ifdef EDIT_AND_CONTINUE
STDMETHODIMP ScriptEngine::QueryEdit(
    /* [size_is][in] */ ScriptEditRequest *requests,
    /* [in] */ ULONG count,
    /* [out] */ IScriptEditQuery **ppQueryResult)
{
    return this->DispatchOnApplicationThread([&]() -> HRESULT
    {
        HRESULT hr = S_OK;

        if (!m_scriptEdit)
        {
            CComPtr<Js::ScriptEdit> spScriptEdit;
            IFFAILRET(Js::ScriptEdit::CreateInstance(&spScriptEdit));
            IFFAILRET(spScriptEdit->Init(this));

            m_scriptEdit = spScriptEdit;
        }

        return m_scriptEdit->QueryEdit(requests, count, ppQueryResult);
    });
}
#endif

// *** IActiveScriptProfilerControl4 ***
STDMETHODIMP ScriptEngine::SummarizeHeap(PROFILER_HEAP_SUMMARY* pHeapSummary)
{
    HRESULT hr = S_OK;
    IFFAILRET(VerifyOnEntry());

    ActiveScriptProfilerHeapEnum* pHeapEnum = nullptr;
    hr = CreateHeapEnum(&pHeapEnum, /* preEnumHeap2 */ true);

    if (SUCCEEDED(hr))
    {
        hr = pHeapEnum->Summarize(pHeapSummary);
    }
    if (pHeapEnum)
    {
        pHeapEnum->Release();
    }

    return hr;
}

// *** IActiveScriptProfilerControl5 ***
STDMETHODIMP ScriptEngine::EnumHeap2(PROFILER_HEAP_ENUM_FLAGS enumFlags, IActiveScriptProfilerHeapEnum** ppEnum)
{
    HRESULT hr = S_OK;
    IFFAILRET(VerifyOnEntry());

    ActiveScriptProfilerHeapEnum *pHeapEnum = nullptr;
    hr = CreateHeapEnum(&pHeapEnum, /* preEnumHeap2 */ false, enumFlags);
    if (SUCCEEDED(hr))
    {
        *ppEnum = pHeapEnum;
    }
    return hr;
}

const wchar_t *ScriptEngine::GetDispatchFunctionNameAndContext(Js::JavascriptFunction *pFunction, Js::ScriptContext **ppFunctionScriptContext)
{
    DispMemberProxy* pDispMemberProxy = (DispMemberProxy*)pFunction;

    *ppFunctionScriptContext = pDispMemberProxy->GetScriptSite()->GetScriptSiteContext();
    return pDispMemberProxy->GetName();
}

// begin debugging section

// IActiveScriptDebug

// === IActiveScriptDebug ===
STDMETHODIMP ScriptEngine::GetScriptTextAttributes(__RPC__in_ecount_full(uNumCodeChars) LPCOLESTR pstrCode, ULONG uNumCodeChars,
                                                   __RPC__in LPCOLESTR pstrDelimiter, DWORD dwFlags,
                                                   __RPC__inout_ecount_full(uNumCodeChars) SOURCE_TEXT_ATTR *prgsta)
{
    // Can be called even when no debugger
    if (GetScriptSiteHolder())
    {
        Parser ps(GetScriptSiteHolder()->GetScriptSiteContext());
        return ps.GetTextAttribs(pstrCode, uNumCodeChars, prgsta, uNumCodeChars, dwFlags, ComputeGrfscrUTF16(pstrDelimiter));
    }
    return E_FAIL;
}

HRESULT ScriptEngine::GetScriptTextAttributesUTF8(LPCUTF8 pchCode, ULONG cb,
                                                  LPCOLESTR pstrDelimiter, ULONG cch, DWORD dwFlags,
                                                  SOURCE_TEXT_ATTR *prgsta)
{
    if (GetScriptSiteHolder())
    {
        Parser ps(GetScriptSiteHolder()->GetScriptSiteContext());
        return ps.GetTextAttribsUTF8(pchCode, cb, prgsta, cch, dwFlags, ComputeGrfscrUTF16(pstrDelimiter));
    }
    return E_FAIL;
}

STDMETHODIMP ScriptEngine::GetScriptletTextAttributes(__RPC__in_ecount_full(cch) LPCOLESTR pstrCode, ULONG cch,
                                                      __RPC__in LPCOLESTR pstrDelimiter, DWORD dwFlags,
                                                      __RPC__inout_ecount_full(cch) SOURCE_TEXT_ATTR *prgsta)
{
    // Can be called even when no debugger
    if (GetScriptSiteHolder())
    {
        Parser ps(GetScriptSiteHolder()->GetScriptSiteContext());
        return ps.GetTextAttribs(pstrCode, cch, prgsta, cch, dwFlags, ComputeGrfscrUTF16(pstrDelimiter));
    }
    return E_FAIL;
}

bool ScriptEngine::CanHalt(Js::InterpreterHaltState* haltState)
{
    // This is registered as the callback for inline breakpoints.
    // We decide here if we are at a reasonable stop location that has source code.
    Assert(haltState->IsValid());

    Js::FunctionBody* pCurrentFuncBody = haltState->GetFunction();
    int byteOffset = haltState->GetCurrentOffset();
    Js::FunctionBody::StatementMap* map = pCurrentFuncBody->GetMatchingStatementMapFromByteCode(byteOffset, false);

    // Resolve the dummy ret code.
    return map != nullptr && (!pCurrentFuncBody->GetIsGlobalFunc() || !Js::FunctionBody::IsDummyGlobalRetStatement(&map->sourceSpan));
}

void ScriptEngine::DispatchHalt(Js::InterpreterHaltState* haltState)
{
    BREAKRESUMEACTION resumeAction = BREAKRESUMEACTION_CONTINUE;
    ERRORRESUMEACTION errorResumeAction = ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller;
    BREAKREASON br = BREAKREASON_BREAKPOINT;
    bool isBreakpoint = true;
    bool isException = false;

    if (GetScriptSiteHolder() == nullptr)
    {
        // Engine is in closed state.
        // Return with resumeaction CONTINUE.
        return;
    }

    // Additional mitigation for webworker,
    // Check if the script has got terminated, by polling the QueryContinue service.
    if (Js::STOP_EXCEPTIONTHROW == haltState->stopType && webWorkerID != Js::Constants::NonWebWorkerContextId)
    {
        if (QueryContinuePoller::GetInterruptPollState(GetScriptSiteHolder()) == E_ABORT)
        {
            // The host has aborted the script, there is no reason we should dispatch the exception to debugger.
            return;
        }
    }

    // Stop on exception should take higher precedence over any other break.
    if (Js::STOP_EXCEPTIONTHROW == haltState->stopType 
        || Js::STOP_INLINEBREAKPOINT == haltState->stopType 
        || Js::STOP_BREAKPOINT == haltState->stopType
        || Js::STOP_MUTATIONBREAKPOINT == haltState->stopType
        || !GetScriptSiteHolder()->FOneTimeBreak(&br))
    {
        isBreakpoint = false;
        switch(haltState->stopType)
        {
        case Js::STOP_MUTATIONBREAKPOINT:
            br = BREAKREASON_MUTATION_BREAKPOINT;
            isBreakpoint = true;
            break;
        case Js::STOP_BREAKPOINT:
        case Js::STOP_ASYNCBREAK:  // We don't reach here except Hybrid debugging - in that case behave as if breakpoint.
            br = BREAKREASON_BREAKPOINT;
            isBreakpoint = true;
            break;
        case Js::STOP_STEPCOMPLETE:
            if (m_remoteDbgThreadId == NOBASETHREAD || m_remoteDbgThreadId == m_dwBaseThread)
            {
                br = BREAKREASON_STEP;
            }
            else
            {
                // If the current thread is not stepping thread, then do not break.
                return;
            }
            isBreakpoint = true;
            break;
        case Js::STOP_INLINEBREAKPOINT:
            br = BREAKREASON_LANGUAGE_INITIATED;
            isBreakpoint = true;
            break;
        case Js::STOP_EXCEPTIONTHROW:
            AssertMsg(haltState->exceptionObject, "exception Object is Missing");
            isException = true;
            break;
        default:
            AssertMsg(false,"Unexpected haltState");

        }
    }
    HRESULT hResBreak = E_FAIL;
    if (isBreakpoint)
    {
        hResBreak = DbgHandleBreakpoint(br, &resumeAction);
    }
    else if (isException)
    {
        ActiveScriptError* scriptError = nullptr;
        HRESULT hrError;

        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            AutoCOMPtr<CDebugStackFrame> stackFrame(new CDebugStackFrame());
            if (stackFrame != nullptr && S_OK != stackFrame->Init(GetScriptSiteHolder(),0) )
            {
                stackFrame.Release();
            }

            if (S_OK == ActiveScriptError::CreateRuntimeError(haltState->exceptionObject, &hrError, stackFrame, nullptr, &scriptError))
            {
                BOOL callOnScriptError;
                AutoCOMPtr<IActiveScriptSite> spActiveScriptSite(this->m_pActiveScriptSite);
                AutoSetDispatchHaltFlag autoSetDispatchHaltFlag(scriptContext, threadContext);

        #if DBG
                void *frameAddress = _AddressOfReturnAddress();
                threadContext->Diagnostics->SetDispatchHaltFrameAddress(frameAddress);
        #endif

                DbgSetAllowUserToRecoverTab(spActiveScriptSite, false);

                // An extra addref to the site so that they won't get deleted while on the debugger thread.
                // Also, keep a local variable for the site as that might be null if script engine is closed
                AutoCOMPtr<ScriptSite> spScriptSite(this->GetScriptSiteHolder());

                hResBreak = m_pda->HandleRuntimeError(scriptError, spActiveScriptSite, &resumeAction, &errorResumeAction, &callOnScriptError);

                DbgSetAllowUserToRecoverTab(spActiveScriptSite, true);

                haltState->exceptionObject->SetDebuggerHasLogged(!callOnScriptError);

                if (errorResumeAction == ERRORRESUMEACTION_SkipErrorStatement)
                {
                    haltState->exceptionObject->SetDebuggerSkip(true);
                }

                scriptError->Release();
            }
        }
        END_LEAVE_SCRIPT(scriptContext);
    }

    if (hResBreak == S_OK)
    {
        ScriptEngine::HandleResumeAction(haltState, resumeAction);
    }

    // The break could have failed, but we should execute as usual, since debugging should not affect the normal script execution in this case.
    // The step controller is already de-activated so no need to special handle anything.
}

/*static*/
void ScriptEngine::HandleResumeAction(Js::InterpreterHaltState* haltState, BREAKRESUMEACTION resumeAction)
{
    Js::ScriptContext* scriptContext = haltState->framePointers->Peek()->GetScriptContext();
    if (resumeAction == BREAKRESUMEACTION_ABORT)
    {
        /// In this case we need to abort script execution entirely and also stop working on any breakpoint for this engine.
        ///
        scriptContext->diagProbesContainer.RemoveAllProbes();
        scriptContext->diagProbesContainer.UninstallInlineBreakpointProbe(NULL);
        throw Js::ScriptAbortException();
    }
    else
    {
        scriptContext->GetThreadContext()->Diagnostics->stepController.HandleResumeAction(haltState, resumeAction);
    }
}

void ScriptEngine::CleanupHalt()
{
    if (this->debugStackFrame)
    {
        for (int i = 0; i < this->debugStackFrame->Count(); i++)
        {
            this->debugStackFrame->Item(i)->Release();
        }

        this->debugStackFrame->Clear();
    }
}

bool ScriptEngine::IsInClosedState()
{
    // m_fClearingDebugDocuments : is set when it goes to PDM (in the middle of close), and re-enter again to the engine and then the ScriptEngine is closed.
    //  So it is safe to bail out on m_fClearingDebugDocuments
    return this->GetScriptSiteHolder() == nullptr || this->GetScriptSiteHolder()->IsClosed() || this->m_fClearingDebugDocuments;
}

bool ScriptEngine::CanAllowBreakpoints()
{
    if (this->GetScriptSiteHolder())
    {
        return this->GetScriptSiteHolder()->FAllowBreakpoints() ? true : false;
    }

    return false;
}

HRESULT STDMETHODCALLTYPE ScriptEngine::GetFunctionName(_In_ Var instance, _Out_ BSTR * pBstrName)
{
    if (pBstrName == nullptr || instance == nullptr)
    {
        return E_POINTER;
    }

    *pBstrName = nullptr;

    if (GetScriptSiteHolder() == nullptr)
    {
        return E_ACCESSDENIED;
    }

    // Only function object supported
    if (!Js::JavascriptFunction::Is(instance))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    Js::JavascriptFunction * jsFunction = Js::JavascriptFunction::FromVar(instance);

    // For the bound function type, get the actual function.
    if (jsFunction->IsBoundFunction())
    {
        jsFunction = ((Js::BoundFunction *)jsFunction)->GetTargetFunction();
        if (jsFunction == nullptr)
        {
            return hr;
        }
    }

    Js::FunctionProxy *pFBody = jsFunction->GetFunctionProxy();
    if (pFBody != nullptr)
    {
        const wchar_t * pwzName = pFBody->EnsureDeserialized()->GetExternalDisplayName();
        if (pwzName != nullptr)
        {
            *pBstrName = ::SysAllocString(pwzName);
        }

        hr = S_OK;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngine::GetFunctionContext(_In_ Var instance, _Out_ IUnknown ** ppDebugDocumentContext)
{
    if (ppDebugDocumentContext == nullptr || instance == nullptr)
    {
        return E_POINTER;
    }

    *ppDebugDocumentContext = nullptr;

    if (GetScriptSiteHolder() == nullptr)
    {
        return E_ACCESSDENIED;
    }

    // Only function object supported
    if (!Js::JavascriptFunction::Is(instance))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    Js::JavascriptFunction * jsFunction = Js::JavascriptFunction::FromVar(instance);

    // For the bound function type, get the actual function.
    if (jsFunction->IsBoundFunction())
    {
        jsFunction = ((Js::BoundFunction *)jsFunction)->GetTargetFunction();
        if (jsFunction == nullptr)
        {
            return hr;
        }
    }

    // Might not have deserialized a function at this point
    if (jsFunction->GetFunctionProxy() != null)
    {
        hr = S_OK;

        // On few scenario event handlers are deferredparsed, undefer it.
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED(scriptContext, false)
        {
            jsFunction->GetFunctionProxy()->EnsureDeserialized()->Parse();
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

        if (hr != S_OK)
        {
            return hr;
        }
    }

    Js::FunctionBody *pFBody = jsFunction->GetFunctionBody();
    if (pFBody == nullptr)
    {
        return E_FAIL;
    }

    IDebugDocumentContext * pDebugDocumentContext = nullptr;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        hr = GetDebugDocumentContextFromHostPosition(scriptContext, pFBody, &pDebugDocumentContext);
        if (hr == S_OK)
        {
            *ppDebugDocumentContext = static_cast<IUnknown *>(pDebugDocumentContext);
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngine::GetFunctionIds(
    _In_ Var instance, _Out_ UINT32* pFunctionId, _Out_ DWORD_PTR* pScriptContextId)
{
    if (pScriptContextId == nullptr || pFunctionId == nullptr || instance == nullptr)
    {
        return E_POINTER;
    }

    *pFunctionId = 0;
    *pScriptContextId = 0;

    if (GetScriptSiteHolder() == nullptr)
    {
        return E_ACCESSDENIED;
    }

    // Only function object supported
    if (!Js::JavascriptFunction::Is(instance))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    Js::JavascriptFunction * jsFunction = Js::JavascriptFunction::FromVar(instance);

    // For the bound function type, get the actual function.
    if (jsFunction->IsBoundFunction())
    {
        jsFunction = ((Js::BoundFunction *)jsFunction)->GetTargetFunction();
        if (jsFunction == nullptr)
        {
            return hr;
        }
    }

    Js::FunctionProxy *pFBody = jsFunction->GetFunctionProxy();
    if (pFBody != nullptr)
    {
        (*pFunctionId) = EtwTrace::GetFunctionId(pFBody);
        (*pScriptContextId) = (DWORD_PTR)pFBody->GetScriptContext();
        hr = S_OK;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngine::GetFunctionInfo(
    _In_ Var instance,
    _Out_opt_ BSTR* pBstrName,
    _Out_opt_ UINT32* pLine,
    _Out_opt_ UINT32* pColumn,
    _Out_opt_ UINT32* pCchLength)
{
    if (instance == nullptr)
    {
        return E_POINTER;
    }

    if (pBstrName != nullptr)
    {
        *pBstrName = nullptr;
    }

    if (pLine != nullptr)
    {
        *pLine = 0;
    }

    if (pColumn != nullptr)
    {
        *pColumn = 0;
    }

    if (pCchLength != nullptr)
    {
        *pCchLength = 0;
    }

    if (GetScriptSiteHolder() == nullptr)
    {
        return E_ACCESSDENIED;
    }

    // Only function object supported
    if (!Js::JavascriptFunction::Is(instance))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    Js::JavascriptFunction * jsFunction = Js::JavascriptFunction::FromVar(instance);

    // For the bound function type, get the actual function.
    if (jsFunction->IsBoundFunction())
    {
        jsFunction = ((Js::BoundFunction *)jsFunction)->GetTargetFunction();
        if (jsFunction == nullptr)
        {
            return hr;
        }
    }

    Js::FunctionProxy *proxy = jsFunction->GetFunctionProxy();
    if (proxy != nullptr)
    {
        Js::ParseableFunctionInfo * pFBody = proxy->EnsureDeserialized();
        if (pBstrName != nullptr)
        {
            *pBstrName = nullptr;
            const wchar_t * pwzName = pFBody->GetExternalDisplayName();
            if (pwzName != nullptr)
            {
                *pBstrName = ::SysAllocString(pwzName);
            }
        }

        if (pLine != nullptr)
        {
            (*pLine) = pFBody->GetLineNumber();
        }

        if (pColumn != nullptr)
        {
            (*pColumn) = pFBody->GetColumnNumber();
        }

        if (pCchLength != nullptr)
        {
            (*pCchLength) = pFBody->LengthInChars();
        }

        hr = S_OK;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngine::TraceAsyncOperationStarting(
    _In_ GUID* platformId,
    _In_ LPCWSTR operationName,
    _In_ AsyncCausality_LogLevel logLevel,
    _Out_ UINT64* pOperationId)
{
    HRESULT hr = S_OK;

    CHECK_POINTER(pOperationId);
    *pOperationId = 0;

    IFFAILRET(VerifyOnEntry());

    if (!AsyncDebug::IsAsyncDebuggingEnabled(this->GetScriptContext()))
    {
        return E_NOTIMPL;
    }

    *pOperationId = AsyncDebug::GetNextAsyncOperationId();

    if (platformId)
    {
        return AsyncDebug::HostWrapperForTraceOperationCreation(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel), *platformId, *pOperationId, operationName);
    }
    else
    {
        return AsyncDebug::HostWrapperForTraceOperationCreation(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel), *pOperationId, operationName);
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngine::TraceAsyncCallbackStarting(
    _In_ GUID* platformId,
    _In_ UINT64 operationId,
    _In_ AsyncCausality_CallbackType workType,
    _In_ AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;

    IFFAILRET(VerifyOnEntry());

    if (!AsyncDebug::IsAsyncDebuggingEnabled(this->GetScriptContext()))
    {
        return E_NOTIMPL;
    }

    if (platformId)
    {
        return AsyncDebug::WrapperForTraceSynchronousWorkStart(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel), *platformId, operationId, static_cast<AsyncDebug::AsyncCallbackType>(workType));
    }
    else
    {
        return AsyncDebug::WrapperForTraceSynchronousWorkStart(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel), operationId, static_cast<AsyncDebug::AsyncCallbackType>(workType));
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngine::TraceAsyncCallbackCompleted(
    _In_ AsyncCausality_CallbackType workType,
    _In_ AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;

    IFFAILRET(VerifyOnEntry());

    if (!AsyncDebug::IsAsyncDebuggingEnabled(this->GetScriptContext()))
    {
        return E_NOTIMPL;
    }

    return AsyncDebug::WrapperForTraceSynchronousWorkCompletion(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel));
}

HRESULT STDMETHODCALLTYPE ScriptEngine::UpdateAsyncCallbackRelation(
    _In_ GUID* platformId,
    _In_ UINT64 operationId,
    _In_ AsyncCausality_RelationType relation,
    _In_ AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;

    IFFAILRET(VerifyOnEntry());

    if (!AsyncDebug::IsAsyncDebuggingEnabled(this->GetScriptContext()))
    {
        return E_NOTIMPL;
    }

    if (relation < AsyncCausality_RelationType_AssignDelegate || relation > AsyncCausality_RelationType_Last)
    {
        return E_INVALIDARG;
    }

    if (platformId)
    {
        return AsyncDebug::WrapperForTraceOperationRelation(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel), *platformId, operationId, static_cast<AsyncDebug::AsyncCallbackStatus>(relation));
    }
    else
    {
        return AsyncDebug::WrapperForTraceOperationRelation(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel), operationId, static_cast<AsyncDebug::AsyncCallbackStatus>(relation));
    }
}

HRESULT STDMETHODCALLTYPE ScriptEngine::TraceAsyncOperationCompleted(
    _In_ GUID* platformId,
    _In_ UINT64 operationId,
    _In_ AsyncCausality_OperationStatus status,
    _In_ AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;

    IFFAILRET(VerifyOnEntry());

    if (!AsyncDebug::IsAsyncDebuggingEnabled(this->GetScriptContext()))
    {
        return E_NOTIMPL;
    }

    if (status < AsyncCausality_OperationStatus_Started || status > AsyncCausality_OperationStatus_Last)
    {
        return E_INVALIDARG;
    }

    if (platformId)
    {
        return AsyncDebug::WrapperForTraceOperationCompletion(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel), *platformId, operationId, static_cast<AsyncDebug::AsyncOperationStatus>(status));
    }
    else
    {
        return AsyncDebug::WrapperForTraceOperationCompletion(this->GetScriptContext(), static_cast<AsyncDebug::LogLevel>(logLevel), operationId, static_cast<AsyncDebug::AsyncOperationStatus>(status));
    }
}

HRESULT ScriptEngine::GetDebugStackFrame(JsUtil::List<CDebugStackFrame *, ArenaAllocator> ** ppdbgFrame)
{
    HRESULT hr = S_OK;
    if (this->debugStackFrame == nullptr)
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            this->debugStackFrame = JsUtil::List<CDebugStackFrame *, ArenaAllocator>::New(this->scriptContext->AllocatorForDiagnostics());
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
    }

    *ppdbgFrame = debugStackFrame;
    return hr;
}

STDMETHODIMP ScriptEngine::EnumCodeContextsOfPosition(
#if _WIN64 || USE_32_OR_64_BIT
    DWORDLONG dwSourceContext,
#endif
#if !_WIN64 || USE_32_OR_64_BIT
    DWORD dwSourceContext,
#endif
    ULONG uCharacterOffset,
    ULONG uNumChars,
    IEnumDebugCodeContexts **ppescc
    )
{
    return DebugApiWrapper( [=] {
        CHECK_POINTER(ppescc);
        *ppescc = nullptr;

        if (m_fClearingDebugDocuments)
        {
            // We already cleared debug documents, we cannot provide the code-context.
            return E_FAIL;
        }

        // The script engine can be closed on a separate thread so we need to lock to ensure the
        // debug document stays around.
        AutoCriticalSection autoCriticalSection(this->threadContext->GetEtwRundownCriticalSection());

        if (IsInClosedState() || this->scriptContext == nullptr)
        {
            // Found scenario, where the debugger is looking for the context but by the time the scriptContext is not alive.
            // Just bailing out here.
            return E_FAIL;
        }

        if (!this->IsDebuggerEnvironmentAvailable())
        {
            return E_UNEXPECTED;
        }

        DWORD_PTR dwSourceCookie = (DWORD_PTR)dwSourceContext;

        ScriptDebugDocument* pDebugDocument = nullptr;

        this->MapDebugDocumentUntil([&](ScriptDebugDocument* scriptDocument) {
            if (scriptDocument->GetDebugSourceCookie() == dwSourceCookie)
            {
                pDebugDocument = scriptDocument;
                return true;
            }
            return false;
        });

        if (pDebugDocument != nullptr)
        {
            return pDebugDocument->EnumCodeContextsOfHostPosition(uCharacterOffset, uNumChars, ppescc);
        }

        return E_FAIL;
    });
}

HRESULT ScriptEngine::DbgHandleBreakpoint(BREAKREASON br, BREAKRESUMEACTION* pBra)
{
    HRESULT hr = E_FAIL;
    if (DebugHelper::IsScriptEngineClosed(this, &hr))
    {
        // Engine is in closed state.
        return hr;
    }

    Js::ScriptContext* scriptContext = this->GetScriptSiteHolder()->GetScriptSiteContext();
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        AutoSetDispatchHaltFlag autoSetDispatchHaltFlag(scriptContext, threadContext);

#if DBG
        void *frameAddress = _AddressOfReturnAddress();
        threadContext->Diagnostics->SetDispatchHaltFrameAddress(frameAddress);
#endif

        // Cache locally since HandleBreakPoint can cause script engine to close
        AutoCOMPtr<IActiveScriptSite> spActiveScriptSite(this->m_pActiveScriptSite);
        DbgSetAllowUserToRecoverTab(spActiveScriptSite, false);

        // An extra addref to the site so that they won't get deleted while on the debugger thread.
        // Also, keep a local variable for the site as that might be null if script engine is closed
        AutoCOMPtr<ScriptSite> spScriptSite(this->GetScriptSiteHolder());
        
        hr = m_pda->HandleBreakPoint(br, pBra);

        DbgSetAllowUserToRecoverTab(spActiveScriptSite, true);
    }
    END_LEAVE_SCRIPT(scriptContext);

    return hr;
}

void ScriptEngine::DbgSetAllowUserToRecoverTab(IActiveScriptSite* scriptSite, bool fAllowUserToRecoverTab)
{
    IOleCommandTarget * pOleCommandTarget = nullptr;
    if (SUCCEEDED(scriptSite->QueryInterface(&pOleCommandTarget)))
    {
        VARIANT varIn;
        varIn.vt = VT_BOOL;
        varIn.boolVal = (fAllowUserToRecoverTab ? VARIANT_TRUE : VARIANT_FALSE);

        const GUID cgidScriptSite = CGID_ScriptSite;        
        HRESULT hr = pOleCommandTarget->Exec(&cgidScriptSite, CMDID_SCRIPTSITE_ALLOWRECOVERY, 0, &varIn, nullptr);
        Unused(hr);     // Good to leave around for debugging
        pOleCommandTarget->Release();
    }
}

HRESULT
    ScriptEngine::DbgRegisterFunction(Js::ScriptContext * scriptContext, Js::FunctionBody * functionBody, LPCWSTR title)
{
    Assert(functionBody);
    return DbgRegisterFunction(scriptContext, functionBody, functionBody->GetHostSourceContext(), title);
}

/*static*/
HRESULT ScriptEngine::DbgRegisterFunction(Js::ScriptContext * scriptContext, Js::FunctionBody * functionBody, DWORD_PTR dwDebugSourceContext, LPCWSTR title)
{
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    Assert(scriptSite);

    ScriptEngine *pEngine = scriptSite->GetScriptEngine();

    // No one actually checks the hresult returned by this function
    // There are two callers to this function- eval, which handles the OOM
    // and FunctionBody::CheckAndRegisterFuncToDiag which also handles OOM
    CScriptBody* pbody = HeapNew(CScriptBody, functionBody, pEngine, functionBody->GetUtf8SourceInfo());

    HRESULT hr = pEngine->DbgRegisterScriptBlock(pbody, dwDebugSourceContext, title);

    pbody->Release();
    return hr;
}


// === Internal Debugger Methods ===
HRESULT ScriptEngine::DbgRegisterScriptBlock(CScriptBody *pbody)
{
    return this->DbgRegisterScriptBlock(pbody, pbody->GetHostSourceContext(), nullptr);
}

HRESULT ScriptEngine::DbgRegisterScriptBlock(CScriptBody *pbody, DWORD_PTR dwDebugSourceContext, LPCWSTR title)
{
    AssertMem(pbody);
    Assert(pbody->GetUtf8SourceInfo());

    // If we're not in source rundown or debug mode, we don't have to register anything.
    // Or if we are working with Intl; don't have to register it's source for debugging purposes.
    if (!this->CanRegisterDebugSources() || pbody->GetUtf8SourceInfo()->GetIsLibraryCode())
    {
        return NOERROR;
    }

    const SRCINFO * srcInfo = pbody->GetUtf8SourceInfo()->GetSrcInfo();

    RegisterDebugDocument(pbody, srcInfo->sourceContextInfo->IsDynamic() ? title : srcInfo->sourceContextInfo->url, dwDebugSourceContext);

    // Register this to probe container as well, but only dynamic code.
    if(srcInfo->sourceContextInfo->IsDynamic())
    {
        scriptContext->diagProbesContainer.RegisterContextToDiag(pbody->GetSecondaryHostSourceContext(), scriptContext->AllocatorForDiagnostics());
    }

    return S_OK;
}

HRESULT ScriptEngine::GetDebugSiteCoreNoRef(IActiveScriptSiteDebug **pscriptSiteDebug)
{
    Assert(!m_fDumbHost);
    AssertMem(pscriptSiteDebug);

    // SetScriptSite and SetState are setup so that m_fDubmHost is set to
    // true whenever m_scriptSiteDebug is nullptr.
    AssertMem(m_pActiveScriptSite);

    if (nullptr == m_scriptSiteDebug)
    {
        HRESULT hr;
        if (FAILED(hr = m_pActiveScriptSite->QueryInterface(IID_IActiveScriptSiteDebug,(void **)&m_scriptSiteDebug)))
        {
            m_fDumbHost = TRUE;
            return hr;
        }
    }
    *pscriptSiteDebug = m_scriptSiteDebug;
    return NOERROR;
}


HRESULT ScriptEngine::DebugDocMarkForClose()
{
    this->MapDebugDocument( [] (ScriptDebugDocument* document){
        document->MarkForClose();
    });
    return S_OK;
}

HRESULT ScriptEngine::NonDbgGetSourceDocumentFromHostContext(
    Js::FunctionBody *pFunctionBody,
    CScriptSourceDocumentText **ppdoc
    )
{
    AssertMem(ppdoc);
    Assert(pFunctionBody != nullptr);
    Assert(!pFunctionBody->GetScriptContext()->IsInDebugMode() || pFunctionBody->GetUtf8SourceInfo()->GetIsLibraryCode());

    Assert(pFunctionBody->IsDynamicScript() || pFunctionBody->GetSecondaryHostSourceContext() == Js::Constants::NoHostSourceContext);

    if (pFunctionBody->IsDynamicScript())
    {
        // TODO : can we figure out from where the source definition originated. might be tricky as arguments to eval can be constructed code
        return E_FAIL;
    }

    *ppdoc = nullptr;
    for (CScriptNonDebugDocumentText *pdoc = (CScriptNonDebugDocumentText *)m_pNonDebugDocFirst; pdoc != nullptr; pdoc = (CScriptNonDebugDocumentText *)pdoc->GetNext())
    {
        if (pdoc->GetSourceIndex() == pFunctionBody->GetSourceIndex())
        {
            pdoc->AddRef();
            *ppdoc = pdoc;
            return NOERROR;
        }
    }

    // Doesnt exit, so create one:
    CScriptNonDebugDocumentText *pdoc = HeapNewNoThrow(CScriptNonDebugDocumentText);
    IFNULLMEMRET(pdoc);

    BSTR bstrUrl = ::SysAllocString(pFunctionBody->GetSourceName());

    HRESULT hr = S_OK;
    if (FAILED(hr = pdoc->Init(this, pFunctionBody, bstrUrl)))
    {
        ::SysFreeString(bstrUrl);
        pdoc->Close();
        pdoc->Release();
        return hr;
    }

    pdoc->Link(&m_pNonDebugDocFirst); // link the document to our list
    pdoc->AddRef(); // extra AddRef so this doesn't get free'd by caller
    *ppdoc = pdoc;

    return hr;
}

ScriptDebugDocument * ScriptEngine::FindDebugDocument(SourceContextInfo * pInfo)
{
    ScriptDebugDocument *pDebugDocument = nullptr;
    this->MapUTF8SourceInfoUntil([&](Js::Utf8SourceInfo * sourceInfo) -> bool
    {
        if (sourceInfo->GetSourceContextInfo() == pInfo && sourceInfo->HasScriptDebugDocument())
        {
            pDebugDocument = static_cast<ScriptDebugDocument*>(sourceInfo->GetScriptDebugDocument());
            return true;
        }
        return false;
    });

    return pDebugDocument;
}

void ScriptEngine::RegisterDebugDocument(CScriptBody *pBody, const wchar_t * title, DWORD_PTR dwDebugSourceContext)
{
    Assert(this->CanRegisterDebugSources());

    ScriptDebugDocument* pDebugDocument = HeapNewNoThrow(ScriptDebugDocument, pBody, dwDebugSourceContext);
    if(pDebugDocument)
    {
        // Register will call AddText which might set breakpoints - so setup the utf8SourceInfo with the script debug document
        pBody->GetUtf8SourceInfo()->SetScriptDebugDocument(pDebugDocument);

        HRESULT hr = pDebugDocument->Register(title);
        if(FAILED(hr))
        {
            pBody->GetUtf8SourceInfo()->ClearDebugDocument();
        }
    }
}

HRESULT ScriptEngine::GetDocumentContextFromPosition(
    DWORD_PTR   dwSourceContext,
    ULONG       uCharacterOffset,
    ULONG       uNumChars,
    IDebugDocumentContext ** ppsc
    )
{
    HRESULT hr;
    IActiveScriptSiteDebug *scriptSiteDebug;
    if (SUCCEEDED(hr = GetDebugSiteNoRef(&scriptSiteDebug)))
    {
        hr = scriptSiteDebug->GetDocumentContextFromPosition(dwSourceContext,
            uCharacterOffset, uNumChars, ppsc);
    }
    return hr;
}

HRESULT ScriptEngine::GetDebugDocumentContextFromHostPosition(
    Js::ScriptContext           *pContext,
    Js::FunctionBody            *pFunctionBody,
    IDebugDocumentContext       **ppDebugDocumentContext)
{
    // The library code should not be register to the PDM, as they will not be shown to the user.
    if (pContext->IsInDebugOrSourceRundownMode() && !pFunctionBody->GetUtf8SourceInfo()->GetIsLibraryCode())
    {
        Js::Utf8SourceInfo *pUtf8SourceInfo = pFunctionBody->GetUtf8SourceInfo();
        ScriptDebugDocument *pDebugDocument = nullptr;
        if (pUtf8SourceInfo->HasScriptDebugDocument())
        {
            pDebugDocument = static_cast<ScriptDebugDocument*>(pUtf8SourceInfo->GetScriptDebugDocument());
        }
        else if (pUtf8SourceInfo->IsDynamic())  // This could be the case when current dynamic document is not registered to the PDM.
        { 
            // Register this document at this point.
            pFunctionBody->CheckAndRegisterFuncToDiag(pContext);
            if (pUtf8SourceInfo->HasScriptDebugDocument())
            {
                pDebugDocument = static_cast<ScriptDebugDocument*>(pUtf8SourceInfo->GetScriptDebugDocument());
            }
        }

        if (pDebugDocument == nullptr)
        {
            // Note that debugDocument can be NULL if it had failed to register with PDM in ScriptDebugDocument::Register
            return E_FAIL;
        }
        return pDebugDocument->GetDocumentContext(pFunctionBody->StartInDocument(), pFunctionBody->LengthInChars(), ppDebugDocumentContext);
    }

    ScriptEngine *pScriptEngine = ScriptSite::FromScriptContext(pContext)->GetScriptEngine();
    // Get the Document text in Non debug mode.
    CScriptSourceDocumentText *pSourceDocumentText = nullptr;
    HRESULT hr = pScriptEngine->NonDbgGetSourceDocumentFromHostContext(pFunctionBody, &pSourceDocumentText);

    if (SUCCEEDED(hr))
    {
        Assert(pFunctionBody != nullptr);

        hr = pSourceDocumentText->GetContextOfPosition(
            pFunctionBody->StartInDocument(),
            pFunctionBody->LengthInChars(),
            ppDebugDocumentContext);

        pSourceDocumentText->Release();
    }

    return hr;
}

// This function will be called from the script context when the sourceList gets changed.
HRESULT ScriptEngine::CleanupDocumentContextList(Js::ScriptContext *pScriptContext)
{
    Assert(pScriptContext);
    if (pScriptContext->IsClosed())
    {
        // Sources will already be cleaned.
        return E_FAIL;
    }

    ScriptEngine *pScriptEngine = ScriptSite::FromScriptContext(pScriptContext)->GetScriptEngine();
    Assert(pScriptEngine);

    if (!pScriptEngine->IsDebuggerEnvironmentAvailable())
    {
        pScriptEngine->CleanupDocumentContextListInternal(pScriptContext);
    }
    return S_OK;
}

HRESULT ScriptEngine::CleanupDocumentContextListInternal(Js::ScriptContext *pScriptContext)
{
    // Walk-thru entire list of nondebug documents and remove those whose sourceindex in the sourceList got removed.

    CScriptNonDebugDocumentText *pdoc = (CScriptNonDebugDocumentText *)m_pNonDebugDocFirst;
    while (pdoc != nullptr)
    {
        CScriptNonDebugDocumentText *tempPdoc = (CScriptNonDebugDocumentText *)pdoc->GetNext();

        if (!pScriptContext->IsItemValidInSourceList(pdoc->GetSourceIndex()))
        {
            pdoc->Close();
            pdoc->Release();
        }

        pdoc = tempPdoc;
    }

    return S_OK;
}

static BOOL FUserWantsDebugger(void)
{
    LONG  lErr;
    BOOL  fRet      = FALSE;
    HKEY  hk        = nullptr;
    DWORD dwType;
    DWORD dwDebug   = 0;
    DWORD dwSize    = sizeof(DWORD);
    LPCWSTR pszKey = L"Software\\Microsoft\\Windows Script\\Settings";
    LPCWSTR pszVal = L"JITDebug";

    lErr = RegOpenKeyExW(HKEY_CURRENT_USER, pszKey, 0, KEY_READ, &hk);
    if (NOERROR != lErr)
        goto LError;

    lErr = RegQueryValueEx(hk, pszVal, nullptr, &dwType, (LPBYTE)&dwDebug, &dwSize);
    if (ERROR_SUCCESS == lErr)
        fRet = (0 != dwDebug);

LError:
    REGCLOSE(hk);
    return fRet;
}

// === IDebugStackFrameSnifferEx ===
STDMETHODIMP ScriptEngine::EnumStackFrames(IEnumDebugStackFrames **ppedsf)
{
    return EnumStackFramesEx32(0, ppedsf);
}

STDMETHODIMP ScriptEngine::EnumStackFramesEx32(DWORD dwSpMin, IEnumDebugStackFrames **ppedsf)
{
    return DebugApiWrapper( [=] {
        if (!ppedsf)
        {
            return E_INVALIDARG;
        }
        *ppedsf = nullptr;

        if (GetCurrentThreadId() != m_dwBaseThread || IsInClosedState())
        {
            return E_FAIL;
        }

        if (!IsDebuggerEnvironmentAvailable())
        {
            return E_UNEXPECTED;
        }

        if (!scriptContext || !threadContext->Diagnostics->isAtDispatchHalt)
        {
            // Because jscript engine didn't halt the engine. So we will not be able to generate the stack.
            return E_FAIL;
        }

#if DBG
        // Validates if there are no javascript frame after we have broken to the debugger.
        threadContext->Diagnostics->ValidateDebugAPICall();
#endif 

        CEnumDebugStackFrames * pedsf = new CEnumDebugStackFrames(dwSpMin, this->GetScriptSiteHolder());

        if (pedsf == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        if (FAILED(pedsf->Init()))
        {
            pedsf->Release();
            return E_OUTOFMEMORY;
        }

        *ppedsf = pedsf;
        return S_OK;
    });
}

#if _WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::EnumStackFramesEx64(DWORDLONG dwSpMin, IEnumDebugStackFrames64 **ppedsf)
{
    Assert(false);
    return E_NOTIMPL;
}
#endif // _WIN64 || USE_32_OR_64_BIT

STDMETHODIMP ScriptEngine::OnConnectDebugger(IApplicationDebugger *pad)
{
    return NOERROR;
}
STDMETHODIMP ScriptEngine::OnDisconnectDebugger(void)
{
    return NOERROR;
}
STDMETHODIMP ScriptEngine::OnSetName(LPCOLESTR pstrName)
{
    return NOERROR;
}
STDMETHODIMP ScriptEngine::OnDebugOutput(LPCOLESTR pstr)
{
    return S_FALSE;
}
STDMETHODIMP ScriptEngine::OnClose(void)
{
    return NOERROR;
}
STDMETHODIMP ScriptEngine::OnEnterBreakPoint(IRemoteDebugApplicationThread *prdat)
{
    return NOERROR;
}
STDMETHODIMP ScriptEngine::OnLeaveBreakPoint(IRemoteDebugApplicationThread *prdat)
{
    return NOERROR;
}
STDMETHODIMP ScriptEngine::OnCreateThread(IRemoteDebugApplicationThread *prdat)
{
    return NOERROR;
}
STDMETHODIMP ScriptEngine::OnDestroyThread(IRemoteDebugApplicationThread *prdat)
{
    return NOERROR;
}


HRESULT ScriptEngine::SetBreakFlagChange(APPBREAKFLAGS abf, IRemoteDebugApplicationThread* prdatSteppingThread, bool fDuringSetupDebugApp)
{
    Assert(GetScriptSiteHolder() != nullptr && GetScriptSiteHolder()->GetScriptSiteContext() != nullptr && !GetScriptSiteHolder()->IsClosed());

    BOOL fIsOurThread = FALSE;
    m_remoteDbgThreadId = NOBASETHREAD;
    if (nullptr != prdatSteppingThread)
    {
        if (SUCCEEDED(prdatSteppingThread->GetSystemThreadId(&m_remoteDbgThreadId)))
        {
            fIsOurThread = (m_remoteDbgThreadId == m_dwBaseThread);
        }
    }
    GetScriptSiteHolder()->SetAppBreakFlags(abf, fIsOurThread);

    Js::ScriptContext* scriptContext=GetScriptSiteHolder()->GetScriptSiteContext();

    if (APPBREAKFLAG_DEBUGGER_HALT&abf && !threadContext->Diagnostics->isAtDispatchHalt)
    {
        scriptContext->diagProbesContainer.AsyncActivate(this);
        if (Js::Configuration::Global.EnableJitInDebugMode())
        {
            threadContext->GetDebuggingFlags()->SetForceInterpreter(true);
        }
    }
    else if (!fDuringSetupDebugApp)
    {
        if (Js::Configuration::Global.EnableJitInDebugMode())
        {
            threadContext->GetDebuggingFlags()->SetForceInterpreter(false);
        }
        scriptContext->diagProbesContainer.AsyncDeactivate();
    }

    if (fIsOurThread && (abf & APPBREAKFLAG_STEP))
    {
        // This is to mention that the last action was stepping, hence update the state of the diagnostics accordingly.
        Assert(scriptContext);
        scriptContext->diagProbesContainer.UpdateStep(fDuringSetupDebugApp);
    }
    else if (!fDuringSetupDebugApp)
    {
        Assert(scriptContext);
        scriptContext->diagProbesContainer.DeactivateStep();
    }

    return NOERROR;
}

// === IRemoteDebugApplicationEvents ===
STDMETHODIMP ScriptEngine::OnBreakFlagChange(APPBREAKFLAGS abf, IRemoteDebugApplicationThread *prdatSteppingThread)
{
    return DebugApiWrapper([=]() -> HRESULT {
        if (!IsInClosedState())
        {
            if (!IsDebuggerEnvironmentAvailable())
            {
                // TODO: Find out what happens in the source rundown mode when a break flag change occurs.  Might need to handle
                // this in the source rundown case as well.
                return E_UNEXPECTED;
            }

            if (GetScriptSiteHolder()->GetScriptSiteContext() != nullptr && !GetScriptSiteHolder()->IsClosed())
            {
                if (!GetScriptSiteHolder()->GetScriptSiteContext()->IsInDebugMode())
                {
                    AssertMsg(false, "Why do we get this event when we are not in debug mode?");
                    return E_UNEXPECTED;
                }

                return SetBreakFlagChange(abf, prdatSteppingThread, false);
            }
        }
        return NOERROR;
    });
}

// If any of the DbgCreateBrowser* methods are called from the debugger thread,
// the caller (currently all relevant code is in debugger\namedbp.cpp) should
// have called ScriptEngine::NamedBPEnter to ensure that the state remains usable
// during the call's execution.

HRESULT ScriptEngine::DbgCreateBrowserFromCodeContext(
    IDebugCodeContext * pcc,
    LPCOLESTR           pstrName,
    IDebugProperty **   ppdp
    )
{
    AssertMem(pcc);
    AssertPsz(pstrName);
    AssertMem(ppdp);
    Assert(IsDebuggerEnvironmentAvailable());

    HRESULT hr;
    IFFAILRET(EnsureBrowserMembers());
    AssertMem(m_debugHelper);
    AssertMem(m_debugApplicationThread);
    AssertMem(m_debugFormatter);
    IDebugHelperEx * pdhex;
    if (SUCCEEDED(hr = m_debugHelper->QueryInterface(_uuidof(IDebugHelperEx), (void **)&pdhex)))
    {
        hr = pdhex->CreatePropertyBrowserFromCodeContext(pcc, pstrName, m_debugApplicationThread, ppdp);
        pdhex->Release();
    }
    return hr;
}

HRESULT ScriptEngine::DbgCreateBrowserFromError(
    IActiveScriptError * pase,
    LPCOLESTR            pstrName,
    IDebugProperty **    ppdp
    )
{
    AssertMem(pase);
    AssertPsz(pstrName);
    AssertMem(ppdp);
    Assert(IsDebuggerEnvironmentAvailable());
    HRESULT hr;
    if (FAILED(hr = EnsureBrowserMembers()))
        return hr;
    AssertMem(m_debugHelper);
    AssertMem(m_debugApplicationThread);
    AssertMem(m_debugFormatter);
    IDebugHelperEx *pdhex;
    if (SUCCEEDED(hr = m_debugHelper->QueryInterface(_uuidof(IDebugHelperEx), (void **)&pdhex)))
    {
        hr = pdhex->CreatePropertyBrowserFromError(pase, pstrName, m_debugApplicationThread, m_debugFormatter, ppdp);
        pdhex->Release();
    }
    else
    {
        IDebugHelperExOld *pdhexOld;
        if (SUCCEEDED(hr = m_debugHelper->QueryInterface(IID_IDebugHelperExOld, (void **)&pdhexOld)))
        {
            // We pass in nullptr for the IDebugFormatter because the interface
            // changed between versions and the original code in the V3 engines
            // never passed in the formatter (so the code is untested).
            pdhexOld->CreatePropertyBrowserFromError(pase, pstrName, m_debugApplicationThread, nullptr, ppdp);
            pdhexOld->Release();
        }
    }
    return hr;
}


void ScriptEngine::FetchTopLevelFunction(JsUtil::List<Js::FunctionBody *, ArenaAllocator>* pFunctions, Js::Utf8SourceInfo * sourceInfo)
{
    Assert(pFunctions != nullptr);
    Assert(sourceInfo != nullptr);

    HRESULT hr = S_OK;
    //
    // Get functionbodies which are distinctly parseable, ie. they are not enclosed in any other function (finding out root node of the sub-tree, in which root node is
    // not enclosed in any other available function) this is by walking over all function and comparing their range.
    // functions and store in them in the temp container, by comparing their source range.

    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        pFunctions->Clear();

        sourceInfo->MapFunctionUntil([&] (Js::FunctionBody* pFuncBody) -> bool
        {
            if (pFuncBody->GetIsGlobalFunc())
            {
                if (pFuncBody->IsFakeGlobalFunc(pFuncBody->GetGrfscr()))
                {
                    // This is created due to 'Function' code or deferred parsed functions, there is nothing to re-compile in this function
                    // as this is just a place-holder/fake function.

                    Assert(pFuncBody->GetByteCode() == NULL);

                    return false;
                }

                if(!pFuncBody->GetIsTopLevel())
                {
                    return false;
                }

                // If global function, there is no need to find out any other functions.

                pFunctions->Clear();
                pFunctions->Add(pFuncBody);
                return true;
            }

            if (pFuncBody->IsFunctionParsed())
            {
                bool isNeedToAdd = true;
                for (int i = 0; i < pFunctions->Count(); i++)
                {
                    Js::FunctionBody *currentFunction = pFunctions->Item(i);
                    if (currentFunction != nullptr)
                    {
                        if (currentFunction->StartInDocument() > pFuncBody->StartInDocument() || !currentFunction->EndsAfter(pFuncBody->StartInDocument()))
                        {
                            if (pFuncBody->StartInDocument() <= currentFunction->StartInDocument() && pFuncBody->EndsAfter(currentFunction->StartInDocument()))
                            {
                                // The stored item has got the parent, remove current Item
                                pFunctions->Item(i, nullptr);
                            }
                        }
                        else
                        {
                            // Parent (the enclosing function) is already in the list
                            isNeedToAdd = false;
                            break;
                        }
                    }
                }

                if (isNeedToAdd)
                {
                    pFunctions->Add(pFuncBody);
                }
            }
            return false;
        });
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    Assert(hr == S_OK);
}

BOOL ScriptEngine::IsHostInDebugMode()
{
    return m_isHostInDebugMode;
}

// Queries the host to see if it is set to debug mode or not.
void ScriptEngine::CheckHostInDebugMode()
{
    Assert(this->m_pActiveScriptSite);
    AssertMsg(!m_isHostInDebugMode, "Check for host being in debug mode only if it's not already in debug mode" );

    HRESULT hr = S_OK;
    AssertMsg(this->m_dwBaseThread == GetCurrentThreadId(), "CheckHostInDebugMode invoked in debugger thread!");
    CComPtr<IActiveScriptSiteDebugHelper> activeScriptSiteDebugHelper;
    hr = this->m_pActiveScriptSite->QueryInterface(
        __uuidof(IActiveScriptSiteDebugHelper),
        reinterpret_cast<void**>(&activeScriptSiteDebugHelper));

    if (FAILED(hr))
    {
        // If the host doesn't implement the interface, we assume debug mode for the host.
        m_isHostInDebugMode = true;
    }
    else
    {
        Assert(activeScriptSiteDebugHelper);

        BOOL isInDebugMode = FALSE;
        hr = activeScriptSiteDebugHelper->IsInDebugMode(&isInDebugMode);
        if (FAILED(hr))
        {
            Assert(false);
        }
        if(m_isHostInDebugMode != isInDebugMode)
        {
            OUTPUT_TRACE(Js::DebuggerPhase, L"Host transitioned debug mode from %s to %s \n", IsTrueOrFalse(m_isHostInDebugMode),  IsTrueOrFalse(isInDebugMode));
        }
        m_isHostInDebugMode = isInDebugMode;
    }
}

STDMETHODIMP ScriptEngine::OnDebuggerAttached(__in ULONG pairCount, /* [size_is][in] */__RPC__in_ecount_full(pairCount) SourceContextPair *pSourceContextPairs)
{
    OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::OnDebuggerAttached: start %p\n", this);

    if (IsInClosedState() || this->scriptContext == nullptr)
    {
        return E_FAIL;
    }

    if (this->scriptContext->IsRunningScript())
    {
        OUTPUT_TRACE(Js::DebuggerPhase, L"Changing debug mode from within script is not allowed\n");
        m_hasFailedToAttachDetach = TRUE;
        return E_FAIL;
    }

    if (this->scriptContext->IsInDebugMode())
    {
        // We should be in the non-debug mode to perform dynamic attach, o/w we assume that we have done the attach for this engine already.
        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::OnDebuggerAttached already in the debug mode\n");
        return S_OK;
    }

    // Restore the flag to say that nothing went wrong.
    m_hasFailedToAttachDetach = FALSE;

    // Get into debug mode.
    if (!IsDebuggerEnvironmentAvailable(/*requery*/ true))
    {
        // Unable to get into debug mode.
        Assert(FALSE);
        return E_FAIL;
    }

    Js::StepController* stepController = &this->scriptContext->GetThreadContext()->Diagnostics->stepController;
    if (stepController->IsActive())
    {
        AssertMsg(stepController->GetActivatedContext() == nullptr, "StepController should not be active when we attach.");
        stepController->Deactivate(); // Defense in depth
    }

    bool shouldPerformSourceRundown = false;
    if (this->scriptContext->GetDebuggerMode() == Js::DebuggerMode::NotDebugging)
    {
        // Today we do source rundown as a part of attach to support VS attaching without
        // first calling PerformSourceRundown.  PerformSourceRundown will be called once
        // by F12 prior to attaching.
        this->scriptContext->SetInSourceRundownMode();

        // Need to perform rundown only once.
        shouldPerformSourceRundown = true;
    }

    if (shouldPerformSourceRundown)
    {
        // Set the function to register sources.
        this->scriptContext->SetDbgRegisterFunction(ScriptEngine::DbgRegisterFunction);
    }

    // Rundown on all exisiting functions and change their thunks so that they will go to debug mode once they are called.

    HRESULT hr = S_OK;

    Js::ScriptContext* scriptContext = this->scriptContext;
    if ((hr = scriptContext->OnDebuggerAttached()) == S_OK)
    {
        // Disable QC while functions are re-parsed as this can be time consuming 
        AutoDisableInterrupt autoDisableInterrupt(this->threadContext->GetInterruptPoller(), true);

        if ((hr = this->RundownSourcesAndReparse(pairCount, pSourceContextPairs, shouldPerformSourceRundown, /*shouldReparseFunctions*/ true)) == S_OK)
        {
            HRESULT hr2 = scriptContext->GetLibrary()->EnsureReadyIfHybridDebugging(); // Prepare library if hybrid debugging attach
            Assert(hr2 != E_FAIL);   // Ommiting hresult
        }

        if (this->scriptContext != nullptr && !this->scriptContext->IsClosed())
        {
            // Still do the pass on the function's entrypoint to reflect its state with the functionbody's entrypoint.
            this->scriptContext->UpdateRecyclerFunctionEntryPointsForDebugger();
        }
    }
    else
    {
        // Let's find out on what conditions it fails
        RAISE_FATL_INTERNAL_ERROR_IFFAILED(hr);
    }

    // A successful attach.
    OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::OnDebuggerAttached: done %p, hr = 0x%X\n", this, hr);

    return hr;
}

STDMETHODIMP ScriptEngine::OnDebuggerDetached()
{
    OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::OnDebuggerDetached: start %p\n", this);

    if (m_pda == nullptr)
    {
        // Debug mode should have been enabled.
        Assert(false);
        return E_UNEXPECTED;
    }

    if (IsInClosedState() || this->scriptContext == NULL)
    {
        Assert(false);
        return E_FAIL;
    }

    if (this->scriptContext->IsRunningScript())
    {
        OUTPUT_TRACE(Js::DebuggerPhase, L"Changing debug mode from within script is not allowed\n");
        m_hasFailedToAttachDetach = TRUE;
        return E_FAIL;
    }

    if (!this->scriptContext->IsInDebugMode())
    {
        // We should be in the debug mode to perform dynamic detach, o/w we assume that we have done the detach for this engine already.
        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::OnDebuggerDetached already in the non-debug mode\n");
        return S_OK;
    }

    Assert(m_isHostInDebugMode);

    // Now mark host in non-debug mode. (If any of prior tests rejects OnDebuggerDetach, we'll keep flag
    // m_isHostInDebugMode == true. There might be discrepency between script engine and host if we are
    // in debug mode, but when m_isHostInDebugMode == true, we won't check host and at least keep script
    // engine in debug mode consistently. See ScriptEngine::IsDebuggerEnvironmentAvailable)
    m_isHostInDebugMode = false;

    HRESULT hr = S_OK;

    Js::StepController* stepController = &this->scriptContext->GetThreadContext()->Diagnostics->stepController;
    if (stepController->IsActive())
    {
        // Normally step controller is deactivated on start of dispatch (step, async break, exception, etc),
        // and in the beginning of interpreter loop we check for step complete (can cause check whether current bytecode belong to stmt).
        // But since it holds to functionBody/statementMaps, we have to deactivate it as func bodies are going away/reparsed.
        stepController->Deactivate();
    }

    // Go through all existing functions and change their thunks back to using non-debug mode versions when called
    // and notify the script context that the debugger has detached to allow it to revert the runtime to the proper
    // state (JIT enabled).
    if ((hr = this->scriptContext->OnDebuggerDetached()) == S_OK)
    {
        // Move the debugger into source rundown mode.
        this->scriptContext->SetInSourceRundownMode();

        // Disable QC while functions are re-parsed as this can be time consuming 
        AutoDisableInterrupt autoDisableInterrupt(this->threadContext->GetInterruptPoller(), true);

        // Force a reparse so that indirect function caches are updated.
        hr = this->RundownSourcesAndReparse(0, nullptr, /*shouldPerformSourceRundown*/ false, /*shouldReparseFunctions*/ true);
        // Let's find out on what conditions it fails
        RAISE_FATL_INTERNAL_ERROR_IFFAILED(hr);

        // Still do the pass on the function's entrypoint to reflect its state with the functionbody's entrypoint.
        scriptContext->UpdateRecyclerFunctionEntryPointsForDebugger();

        // Restore the flag to say that nothing went wrong.
        m_hasFailedToAttachDetach = FALSE;
    }
    else
    {
        // Let's find out on what conditions it fails
        RAISE_FATL_INTERNAL_ERROR_IFFAILED(hr);
    }
   OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::OnDebuggerDetached: done %p, hr = 0x%X\n", this, hr);
   return hr;
}

// IScriptInvocationContextSubscriber

STDMETHODIMP ScriptEngine::PushInvocationContext(__in SCRIPT_INVOCATION_CONTEXT_TYPE contextType,
                                     __in_opt IUnknown * pContextObject,
                                     __in_opt LPCWSTR contextDescription,
                                     __out DWORD * pCookie)
{
    return E_NOTIMPL;
}

STDMETHODIMP ScriptEngine::PopInvocationContext(__in DWORD cookie)
{
    return E_NOTIMPL;
}

// Create an ordered flat list of sources to reparse. Caller of a source should be added to the list before we add the source itself
void ScriptEngine::WalkAndAddUtf8SourceInfo(Js::Utf8SourceInfo* sourceInfo, JsUtil::List<Js::Utf8SourceInfo *, Recycler, false, Js::CopyRemovePolicy, RecyclerPointerComparer> *utf8SourceInfoList)
{
    Js::Utf8SourceInfo* callerUtf8SourceInfo = sourceInfo->GetCallerUtf8SourceInfo();
    if (callerUtf8SourceInfo)
    {
        Js::ScriptContext* callerScriptContext = callerUtf8SourceInfo->GetScriptContext();
        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::WalkAndAddUtf8SourceInfo scriptEngine 0x%p, scriptContext 0x%p, sourceInfo 0x%p, callerUtf8SourceInfo 0x%p, sourceInfo scriptContext 0x%p, callerUtf8SourceInfo scriptContext 0x%p\n",
            this, scriptContext, sourceInfo, callerUtf8SourceInfo, sourceInfo->GetScriptContext(), callerScriptContext);
        
        if (sourceInfo->GetScriptContext() == callerScriptContext)
        {
            WalkAndAddUtf8SourceInfo(callerUtf8SourceInfo, utf8SourceInfoList);
        }
        else if (!callerScriptContext->IsInDebugOrSourceRundownMode())
        {
            // The caller scriptContext is not in run down/debug mode so let's save the relationship so that we can reparent callees afterwards 
            callerScriptContext->AddCalleeSourceInfoToList(sourceInfo);
        }
    }
    if (!utf8SourceInfoList->Contains(sourceInfo))
    {
        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::WalkAndAddUtf8SourceInfo Adding to utf8SourceInfoList scriptEngine 0x%p, scriptContext 0x%p, sourceInfo 0x%p, sourceInfo scriptContext 0x%p\n",
            this, scriptContext, sourceInfo, sourceInfo->GetScriptContext());
#if DBG
        bool found = false;
        this->MapUTF8SourceInfoUntil([&](Js::Utf8SourceInfo * sourceInfoTemp) -> bool
        {
            if (sourceInfoTemp == sourceInfo)
            {
                found = true;
            }
            return found;
        });
        AssertMsg(found, "Parented eval feature have extra source");
#endif
        utf8SourceInfoList->Add(sourceInfo);
    }
}

// Performs a rundown of sources and reparses function bodies (both steps optional).
HRESULT ScriptEngine::RundownSourcesAndReparse(ULONG pairCount, SourceContextPair* pSourceContextPairs, bool shouldPerformSourceRundown, bool shouldReparseFunctions)
{
    OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::RundownSourcesAndReparse scriptEngine 0x%p, scriptContext 0x%p, pairCount %d, shouldPerformSourceRundown %d, shouldReparseFunctions %d\n",
        this, scriptContext, pairCount, shouldPerformSourceRundown, shouldReparseFunctions);

    Js::TempArenaAllocatorObject *tempAllocator = nullptr;
    JsUtil::List<Js::FunctionBody *, ArenaAllocator>* pFunctionsToRegister = nullptr;
    JsUtil::List<Js::Utf8SourceInfo *, Recycler, false, Js::CopyRemovePolicy, RecyclerPointerComparer>* utf8SourceInfoList = nullptr;

    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    ThreadContext* threadContext = scriptContext->GetThreadContext();
    tempAllocator = threadContext->GetTemporaryAllocator(L"debuggerAlloc");
    pFunctionsToRegister = JsUtil::List<Js::FunctionBody*, ArenaAllocator>::New(tempAllocator->GetAllocator());
    utf8SourceInfoList = JsUtil::List<Js::Utf8SourceInfo *, Recycler, false, Js::CopyRemovePolicy, RecyclerPointerComparer>::New(scriptContext->GetRecycler());
    this->MapUTF8SourceInfoUntil([&](Js::Utf8SourceInfo * sourceInfo) -> bool
    {
        WalkAndAddUtf8SourceInfo(sourceInfo, utf8SourceInfoList);
        return false;
    });
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    if (hr != S_OK)
    {
        Assert(FALSE);
        return hr;
    }

    utf8SourceInfoList->MapUntil([&](int index, Js::Utf8SourceInfo * sourceInfo) -> bool
    {
        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::RundownSourcesAndReparse scriptEngine 0x%p, scriptContext 0x%p, sourceInfo 0x%p, HasScriptDebugDocument %d\n",
            this, scriptContext, sourceInfo, sourceInfo->HasScriptDebugDocument());
        if (sourceInfo->GetIsLibraryCode())
        {
            // Not putting the internal library code to the debug mode, but need to reinitialize execution mode limits of each
            // function body upon debugger detach, even for library code at the moment.
            if(shouldReparseFunctions)
            {
                sourceInfo->MapFunction([](Js::FunctionBody *const pFuncBody)
                {
                    if(pFuncBody->IsFunctionParsed())
                    {
                        pFuncBody->ReinitializeExecutionModeAndLimits();
                    }
                });
            }
            return false;
        }

        Assert(sourceInfo->GetSrcInfo() && sourceInfo->GetSrcInfo()->sourceContextInfo);

#if DBG
        if (shouldPerformSourceRundown)
        {
            // We shouldn't have a debug document if we're running source rundown for the first time.
            Assert(!sourceInfo->HasScriptDebugDocument());
        }
#endif // DBG

        DWORD_PTR dwDebugHostSourceContext = Js::Constants::NoHostSourceContext;

        if (shouldPerformSourceRundown)
        {
            for (ULONG i = 0; i < pairCount; i++)
            {
                if (pSourceContextPairs[i].dwHostSourceContext == sourceInfo->GetHostSourceContext() && pSourceContextPairs[i].ulCharOffset == sourceInfo->GetSrcInfo()->ulCharOffset)
                {
                    dwDebugHostSourceContext = pSourceContextPairs[i].dwDebugHostSourceContext;
                    break;
                }
            }
#if DBG
            if (dwDebugHostSourceContext != Js::Constants::NoHostSourceContext)
            {
                // then it must be hostmanaged.
                Assert(sourceInfo->IsHostManagedSource());
            }
            else
            {
                Assert(!sourceInfo->IsHostManagedSource());
            }
#endif
        }
        FetchTopLevelFunction(pFunctionsToRegister, sourceInfo);
        if (pFunctionsToRegister->Count() == 0)
        {
            // This could happen if there are no functions to re-compile.
            return false;
        }

        if (!fSetThreadDescription)
        {
            SetThreadDescription(sourceInfo->GetSourceContextInfo()->url); // the HRESULT is omitted.
        }

        bool fHasDoneSourceRundown = false;
        for (int i = 0; i < pFunctionsToRegister->Count(); i++)
        {
            Js::FunctionBody* pFuncBody = pFunctionsToRegister->Item(i);
            if (pFuncBody == nullptr)
            {
                continue;
            }

            if (shouldReparseFunctions)
            {
                if (this->scriptContext == nullptr)
                {
                    // scriptContext can be closed in previous call 
                    hr = E_FAIL;
                    return true;
                }

                BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED(scriptContext, false)
                {
                    pFuncBody->Parse();
                    // This is the first call to the function, ensure dynamic profile info
                    pFuncBody->EnsureDynamicProfileInfo();
                }
                END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

                if (hr != S_OK)
                {
                    break;
                }
            }

            if (!fHasDoneSourceRundown && shouldPerformSourceRundown)
            {
                BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
                {
                    DbgRegisterFunction(scriptContext, pFuncBody, dwDebugHostSourceContext, pFuncBody->GetSourceName());
                }
                END_TRANSLATE_OOM_TO_HRESULT(hr);

                fHasDoneSourceRundown = true;
            }
        }

        if(shouldReparseFunctions)
        {
            sourceInfo->MapFunction([](Js::FunctionBody *const pFuncBody)
            {
                if(pFuncBody->IsFunctionParsed())
                {
                    pFuncBody->ReinitializeExecutionModeAndLimits();
                }
            });
        }

        return false;
    });

    if (scriptContext != nullptr && !scriptContext->IsClosed())
    {
        if (shouldPerformSourceRundown && scriptContext->HaveCalleeSources())
        {
            scriptContext->MapCalleeSources([=](Js::Utf8SourceInfo* calleeSourceInfo)
            {
                if (calleeSourceInfo && calleeSourceInfo->HasScriptDebugDocument())
                {
                    ScriptDebugDocument* document = static_cast<ScriptDebugDocument*>(calleeSourceInfo->GetScriptDebugDocument());
                    document->ReParentToCaller();
                }
            });
        }
    }
    threadContext->ReleaseTemporaryAllocator(tempAllocator);

    return hr;
}

// Tells the script engine to perform a rundown of sources and enter source rundown mode.
// This method may only be called once per ScriptEngine instance.
//
// Params :
//      pairCount           : Size of the pSourceContextPairs array.
//
//      pSourceContextPairs : An array of pairs, which is allocated at the host and contains the old cookie/sourcecontext
//                            and its corresponding new cookie/sourcecookie for each script block.
//
// Returns S_OK upon success (currently returns E_NOTIMPL).
STDMETHODIMP ScriptEngine::PerformSourceRundown(__in ULONG pairCount, /* [size_is][in] */__RPC__in_ecount_full(pairCount) SourceContextPair *pSourceContextPairs)
{
    //wprintf(L"Performing Source Rundown\n");
    if (IsInClosedState() || this->scriptContext == nullptr)
    {
        return E_FAIL;
    }

    if (!this->scriptContext->IsInNonDebugMode())
    {
        // We should be in the non-debug mode to perform source rundown, o/w we assume that we have done the rundown for this engine already.
        OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::PerformSourceRundown already in the %s mode\n", this->scriptContext->IsInSourceRundownMode() ? L"SourceRundown" : L"Debug");
        return S_OK;
    }

    HRESULT hr = S_OK;

    // Ensure PDM is loaded.
    if (!this->CanRegisterDebugSources())
    {
        Assert(FALSE);
        return E_FAIL;
    }

    // Move the debugger into source rundown mode.
    scriptContext->SetInSourceRundownMode();

    // Set the function to register sources.
    this->scriptContext->SetDbgRegisterFunction(ScriptEngine::DbgRegisterFunction);

    hr = this->RundownSourcesAndReparse(pairCount, pSourceContextPairs, /*shouldPerformSourceRundown*/ true, /*shouldReparseFunctions*/ false);

    // Successful source rundown.
    return hr;
}

HRESULT ScriptEngine::DbgCreateBrowserFromProperty(VARIANT *pvar,
                                                   IDebugSetValueCallback *psetvalue, LPCOLESTR pstrName,
                                                   IDebugProperty **ppdp)
{
    AssertMem(pvar);
    //AssertMem(psetvalue);
    AssertPsz(pstrName);
    AssertMem(ppdp);
    Assert(IsDebuggerEnvironmentAvailable());
    HRESULT hr;
    if (FAILED(hr = EnsureBrowserMembers()))
        return hr;
    AssertMem(m_debugHelper);
    AssertMem(m_debugApplicationThread);
    AssertMem(m_debugFormatter);
    IDebugHelperEx *pdhex;
    if (SUCCEEDED(hr = m_debugHelper->QueryInterface(_uuidof(IDebugHelperEx), (void **)&pdhex)))
    {
        hr = pdhex->CreateWriteablePropertyBrowser(pvar, pstrName, m_debugApplicationThread, m_debugFormatter, psetvalue, ppdp);
        pdhex->Release();
    }
    else
    {
        // We pass in nullptr for the IDebugFormatter because the interface
        // changed between versions and the original code in the V3 engines
        // never passed in the formatter (so the code is untested).
        hr = m_debugHelper->CreatePropertyBrowserEx(pvar, pstrName, m_debugApplicationThread, nullptr, ppdp);
    }
    return hr;
}

// Set the description to a thread if the current engine is associated with a web worker, this description will further be used by the tools while at break mode.
HRESULT ScriptEngine::SetThreadDescription(__in LPCWSTR url)
{
    if (webWorkerID == Js::Constants::NonWebWorkerContextId)
    {
        fSetThreadDescription = TRUE;

        // Not a web worker
        return E_FAIL;
    }

    if (scriptContext->IsInNonDebugMode())
    {
        return E_UNEXPECTED;
    }

    Assert(fSetThreadDescription == FALSE);
    fSetThreadDescription = TRUE;

    if (m_debugApplicationThread != nullptr)
    {
        size_t length = 11; // 10 to accommodate int_max and 1 for termination.

        if (url != nullptr)
        {
            length += (wcslen(url) + 2); // 2 for separator.
        }

        WCHAR * threadDescription = HeapNewNoThrowArrayZ(WCHAR, length);
        if (threadDescription != nullptr)
        {
            if (url != nullptr)
            {
                swprintf_s(threadDescription, length, OLESTR("%u##%s"), webWorkerID, url);
            }
            else
            {
                swprintf_s(threadDescription, length, OLESTR("%u"), webWorkerID);
            }

            m_debugApplicationThread->SetDescription(threadDescription); // No need to check HRESULT.

            HeapDeleteArray(length, threadDescription);

            return S_OK;
        }
    }

    return E_FAIL;
}

// === Internal Debugger Methods ===
HRESULT ScriptEngine::SetupNewDebugApplication(void)
{
    AssertMem(m_pda);
    Assert(nullptr == m_pcpAppEvents);

    HRESULT hr;
    IFFAILRET(m_pda->GetCurrentThread(&m_debugApplicationThread));

    // Connect up to the debug application events.
    IConnectionPointContainer *pcpc;
    if (SUCCEEDED(hr = m_pda->QueryInterface(IID_IConnectionPointContainer, (void **)&pcpc)))
    {
        hr = pcpc->FindConnectionPoint(IID_IRemoteDebugApplicationEvents, &m_pcpAppEvents);
        pcpc->Release();
    }
    IFFAILRET(hr);

    hr = m_pcpAppEvents->Advise((IRemoteDebugApplicationEvents *)this, &m_dwAppAdviseID);
    if (FAILED(hr))
    {
        m_pcpAppEvents->Release();
        m_pcpAppEvents = nullptr;
        return hr;
    }

    APPBREAKFLAGS abf;
    IRemoteDebugApplicationThread *prdatSteppingThread;
    if (SUCCEEDED(hr = m_pda->GetBreakFlags(&abf, &prdatSteppingThread)))
    {
        SetBreakFlagChange(abf, prdatSteppingThread, true);
        if (nullptr != prdatSteppingThread)
            prdatSteppingThread->Release();

        hr = m_pda->AddStackFrameSniffer(
#if _WIN64
            static_cast<IDebugStackFrameSniffer *>(static_cast<IDebugStackFrameSnifferEx64 *>(this)),
#else // _WIN64
            static_cast<IDebugStackFrameSniffer *>(static_cast<IDebugStackFrameSnifferEx32 *>(this)),
#endif // _WIN64
            &m_dwSnifferCookie);
        if (SUCCEEDED(hr))
        {
            m_fStackFrameSnifferAdded = TRUE;
            hr = m_pda->AddGlobalExpressionContextProvider(
                (IProvideExpressionContexts *)this,
                &m_dwExprContextProviderCookie);
            if (SUCCEEDED(hr))
                m_fExprContextProviderAdded = TRUE;
        }
        Js::ScriptContext* scriptContext=GetScriptSiteHolder()->GetScriptSiteContext();
        scriptContext->diagProbesContainer.InitializeInlineBreakEngine(this);

        // This is to get the script debugger options, such as FirstChance Exception
        IRemoteDebugApplication110 *pDebugApp110 = nullptr;
        if (m_pda->QueryInterface(__uuidof(IRemoteDebugApplication110), (void **)&pDebugApp110) == S_OK)
        {
            scriptContext->diagProbesContainer.InitializeForScriptOption(pDebugApp110);
        }
    }

    if (FAILED(hr))
    {
        UNADVISERELEASE(m_pcpAppEvents, m_dwAppAdviseID);
    }
    return hr;
}

void ScriptEngine::ResetDebugger(void)
{
    if (nullptr != m_pda)
    {
        if (m_fStackFrameSnifferAdded)
        {
            m_pda->RemoveStackFrameSniffer(m_dwSnifferCookie);
            m_fStackFrameSnifferAdded = FALSE;
        }

        if (m_fExprContextProviderAdded)
        {
            m_pda->RemoveGlobalExpressionContextProvider(m_dwExprContextProviderCookie);
            m_fExprContextProviderAdded = FALSE;
        }

        RELEASEPTR(m_debugApplicationThread);
        RELEASEPTR(m_debugHelper);
        RELEASEPTR(m_debugFormatter);
        RELEASEPTR(m_pda);
    }
    Assert(nullptr == m_pda);
    Assert(nullptr == m_debugApplicationThread);
    Assert(nullptr == m_debugHelper);
    Assert(nullptr == m_debugFormatter);
    UNADVISERELEASE(m_pcpAppEvents,m_dwAppAdviseID);

    m_fDumbHost = TRUE;
    RELEASEPTR(m_scriptSiteDebug);
}

HRESULT ScriptEngine::EnsureBrowserMembers(void)
{
    // We lock it here for the case when this method is called from the
    // script thread. The script thread will not have locked because
    // resets can only happen from the script thread. We need to lock
    // here so that the the script thread doesn't attempt to create the
    // variables while the debugger thread is.
    if (!NamedBPEnter())
        return E_FAIL;
    Assert(IsDebuggerEnvironmentAvailable());

    HRESULT hr = NOERROR;

    Assert(m_debugApplicationThread);
    if (nullptr == m_debugFormatter)
    {
        ComDebugFormatter * pdf;
        if (FAILED(hr = ComDebugFormatter::Create(&pdf)))
            goto LDone;
        m_debugFormatter = (IDebugFormatter *)pdf;
    }
    if (nullptr == m_debugHelper)
    {
        if (ShouldUseLocalPDM())
        {
            hr = LoadLatestPDM(CLSID_DebugHelper, IID_IDebugHelper, (void**)&m_debugHelper);
        }
        else
        {
            hr = CoCreateInstance(CLSID_DebugHelper, nullptr, CLSCTX_SERVER, IID_IDebugHelper, (void **)&m_debugHelper);
        }
    }
LDone:
    NamedBPLeave();
    return hr;
}

HRESULT ScriptEngine::GetDefaultDebugApplication(IDebugApplication **ppda)
{
    AssertMem(ppda);
    HRESULT hr;
    IProcessDebugManager *ppdm;
    IDebugApplication *pda = nullptr;
    
    if (ShouldUseLocalPDM())
    {
        if (FAILED(hr = LoadLatestPDM(CLSID_ProcessDebugManager, IID_IProcessDebugManager, (void**)&ppdm)))
        {
            goto LNoDebugger;
        }
    }
    else
    {
        if (FAILED(hr = CoCreateInstance(CLSID_ProcessDebugManager, nullptr, CLSCTX_ALL, IID_IProcessDebugManager, (void **)&ppdm)))
        {
            goto LNoDebugger;
        }
    }
    hr = ppdm->GetDefaultApplication(&pda);
    ppdm->Release();
    if (SUCCEEDED(hr))
    {
        *ppda = pda;
        return NOERROR;
    }
LNoDebugger:
    return hr;
}

HRESULT ScriptEngine::GetDebugApplicationCoreNoRef(IDebugApplication **ppda)
{
    AssertMem(ppda);
    if (m_pda)
    {
        *ppda = m_pda;
        return S_OK;
    }

    AssertMem(m_pActiveScriptSite);

    HRESULT hr;
    IActiveScriptSiteDebug * scriptSiteDebug;
    if (SUCCEEDED(GetDebugSiteNoRef(&scriptSiteDebug)))
    {
        // Smart host - if this fails, host wants debugging disabled.
        if (FAILED(hr = scriptSiteDebug->GetApplication(&m_pda)))
            goto LNoDebugger;
    }
    else
    {
        // Dumb host - get application from pdm.
        if (!FUserWantsDebugger())
        {
            hr = E_FAIL;
            goto LNoDebugger;
        }
        if (FAILED(hr = GetDefaultDebugApplication(&m_pda)))
        {
            goto LNoDebugger;
        }
    }

    if (SUCCEEDED(hr = SetupNewDebugApplication()))
    {
        *ppda = m_pda;
        return NOERROR;
    }

LNoDebugger:
    if (nullptr != m_pda)
    {
        RELEASEPTR(m_debugApplicationThread);
        RELEASEPTR(m_debugHelper);
        RELEASEPTR(m_debugFormatter);
        RELEASEPTR(m_pda);
    }
    return hr;
}

BOOL ScriptEngine::IsDebuggerEnvironmentAvailable(bool requery)
{
    if(this->CanRegisterDebugSources())
    {
        if (requery && !m_hasFailedToAttachDetach && !m_isHostInDebugMode)
        {
            CheckHostInDebugMode();
        }
        return this->IsHostInDebugMode();
    }
    return FALSE;
}

HRESULT ScriptEngine::TransitionToDebugModeIfFirstSource(Js::Utf8SourceInfo* utf8SourceInfo)
{
    HRESULT hr = S_OK;
    OUTPUT_TRACE(Js::DebuggerPhase, L"ScriptEngine::TransitionToDebugModeIfFirstSource scriptEngine 0x%p, scriptContext 0x%p, m_isFirstSourceCompile %d\n", this, scriptContext, m_isFirstSourceCompile);
    if (m_isFirstSourceCompile)
    {
        if (!m_hasFailedToAttachDetach)
        {
            // We will not transition the state if we have failed to attach or detach.
            if (CONFIG_FLAG(ForceDiagnosticsMode) || this->IsDebuggerEnvironmentAvailable(/*requery*/ true))
            {
                scriptContext->InitializeDebugging(ScriptEngine::DbgRegisterFunction);

                //Utf8SourceInfo is not part of scriptContext by this point, need to manually put it into debug mode if it's not
                if (utf8SourceInfo && !utf8SourceInfo->IsInDebugMode())
                {
                    utf8SourceInfo->SetInDebugMode(true);
                }

                hr = this->scriptContext->GetLibrary()->EnsureReadyIfHybridDebugging(); // Prepare library if started hybrid debugging
            }
            else if (this->CanRegisterDebugSources())
            {
                this->scriptContext->SetInSourceRundownMode();
                this->scriptContext->SetDbgRegisterFunction(ScriptEngine::DbgRegisterFunction);
            }
        }
        m_isFirstSourceCompile = FALSE;

        // Let's remove the transition function from script context - so that Eval call does not come all the way over here.
        this->scriptContext->SetTransitionToDebugModeIfFirstSourceFn(nullptr);
    }
    return hr;
}

// Checks whether or not sources can be registered (PDM is loaded).
bool ScriptEngine::CanRegisterDebugSources()
{
    if (nullptr != m_pda)
        return TRUE;
    if (nullptr == m_pActiveScriptSite)
        return FALSE;
    IDebugApplication *pda;
    return SUCCEEDED(GetDebugApplicationCoreNoRef(&pda));
}

// end debugging section

// *** IActiveScript Methods ***
STDMETHODIMP ScriptEngine::SetScriptSite(IActiveScriptSite *activeScriptSite)
{
    HRESULT hr;
    VALIDATE_INTERFACE_POINTER(activeScriptSite, IActiveScriptSite);
    if (nullptr != m_pActiveScriptSite)
        return E_UNEXPECTED; // Site already set
    m_dwBaseThread = GetCurrentThreadId();
#ifdef TODO_EVENTS
    g_JSEventTracer.Initialize();
#endif
    Assert(nullptr == GetScriptSiteHolder());
    if (wasScriptDirectEnabled)
    {
        return E_UNEXPECTED;
    }

    hr = ScriptSite::Create(this, activeScriptSite, USE_ARENA, &scriptSiteHolder);
    if (FAILED(hr))
    {
        m_dwBaseThread = NOBASETHREAD;
        return hr;
    }

    scriptSiteHolder->GetScriptSiteContext()->SetActiveScriptDirect(static_cast<IActiveScriptDirect*>(this));
    scriptSiteHolder->SetTicksPerPoll(m_dwTicksPerPoll);

    m_pActiveScriptSite = activeScriptSite;
    m_pActiveScriptSite->AddRef();

    CComBSTR bstrUrl;
    if (SUCCEEDED(this->GetUrl(&bstrUrl)))
    {
        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
        {
            scriptContext->SetUrl(bstrUrl);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            if ((Js::Configuration::Global.flags.PrintRunTimeDataCollectionTrace) && (false == fNonPrimaryEngine))
            {
                // Primary engine is created for top level Page, if start page is set to about:blank/about:Tabs, first primary engine AllocId is 1.
                Output::Print(L"ScriptEngine::SetScriptSite - Navigation Started [ScriptContext AllocId=%d] [Url=%s]\n", scriptContext->allocId, bstrUrl);
            }
#endif
#if DBG_DUMP
            scriptSiteHolder->SetUrl(bstrUrl);
#endif
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);

    }
#ifdef LEAK_REPORT
    else if (Js::Configuration::Global.flags.IsEnabled(Js::LeakReportFlag))
    {
        // HACK HACK: heuristically figure out which one is the root tracker script engine
        // and force close on it
        LCID lcidTemp;
        if (activeScriptSite->GetLCID(&lcidTemp) == E_NOTIMPL)
        {
            threadContext->SetRootTrackerScriptContext(scriptContext);
            scriptContext->urlRecord = LeakReport::LogUrl(L"<CRootTracker?>", scriptContext->GetGlobalObject());
        }
        else
        {
            scriptContext->urlRecord = LeakReport::LogUrl(L"<unknown>", scriptContext->GetGlobalObject());
        }
    }

    if (scriptContext->urlRecord != nullptr)
    {
        scriptContext->urlRecord->scriptEngine = this;
    }
#endif

    m_fNoHostSecurityManager = FALSE;
    m_fNoINETSecurityManager = FALSE;

    LCID lcid;
    if (SUCCEEDED(activeScriptSite->GetLCID(&lcid)))
    {
        if (SetCurrentLocale(lcid))
        {
            // SetCurrentLocale may have mapped the lcid.
            m_lcidUserDefault = m_lcidUser;
        }
    }

    // If we are a clone or are recovering from reset then now is the time to
    // register our named items:
    RegisterNamedItems();

    if (this->pendingCloneSource)
    {
        // Clone the script bodies that have been waiting for named items to be registered
        // in the cloned engine.
        Assert(this->IsCloned());
        hr = this->pendingCloneSource->CloneScriptBodies(this);
        this->pendingCloneSource = nullptr;
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (m_fPersistLoaded)
        ChangeScriptState(SCRIPTSTATE_INITIALIZED);

    m_fDumbHost = FALSE;

    if (IsDebuggerEnvironmentAvailable(/*requery*/true))
    {
        if (scriptContext->IsInDebugMode() && (!Js::Configuration::Global.EnableJitInDebugMode()))
        {
            scriptContext->ForceNoNative();
        }
        scriptContext->InitializeDebugging(ScriptEngine::DbgRegisterFunction);
    }

    if (scriptContext->IsInDebugOrSourceRundownMode())
    {
        // Register all script blocks
        DisableInterrupts();
        long cbod;
        if (nullptr != m_pglbod && 0 < (cbod = m_pglbod->Cv()))
        {
            for (long ibod = 0; ibod < cbod; ++ibod)
            {
                BOD * pbod = (BOD *)m_pglbod->PvGet(ibod);
                Assert(pbod->grfbod == (fbodPersist | fbodRun));
                AssertMem(pbod->pbody);
                // ignore errors
                DbgRegisterScriptBlock(pbod->pbody);
            }
        }
        EnableInterrupts();
    }

    return NOERROR;
}

STDMETHODIMP ScriptEngine::GetScriptSite(REFIID iid, VOID **ppvSiteObject)
{
    VALIDATE_WRITE_POINTER(ppvSiteObject, void *);
    *ppvSiteObject = nullptr;
    if (nullptr == m_pActiveScriptSite)
        return S_FALSE;
    // AddRef is done by QI
    return m_pActiveScriptSite->QueryInterface(iid, ppvSiteObject);
}

STDMETHODIMP ScriptEngine::SetScriptState(SCRIPTSTATE ss)
{
    HRESULT hr;
    HRESULT hrT;

    if (ss != m_ssState &&
        (SCRIPTSTATE_UNINITIALIZED == m_ssState ||
        SCRIPTSTATE_CLOSED        == m_ssState))
    {
        return E_UNEXPECTED;
    }

    hr = NOERROR;
    if (ss != m_ssState)
    {
        switch (ss)
        {
        case SCRIPTSTATE_INITIALIZED:
            switch (m_ssState)
            {
            case SCRIPTSTATE_CONNECTED:
            case SCRIPTSTATE_DISCONNECTED:
                Stop();
            case SCRIPTSTATE_STARTED:
                // REVIEW : handle failure of Reset
                hr = Reset(FALSE);
                ChangeScriptState(SCRIPTSTATE_INITIALIZED);
                hr = S_OK;
                break;
            }
            break;

        case SCRIPTSTATE_STARTED:
            switch (m_ssState)
            {
            case SCRIPTSTATE_CONNECTED:
            case SCRIPTSTATE_DISCONNECTED:
                Stop();
            case SCRIPTSTATE_INITIALIZED:
                hr = ExecutePendingScripts();
                if (SCRIPT_E_REPORTED == hr)
                    hr = S_OK;
                // Even if the execution failed we are still in started state.
                ChangeScriptState(SCRIPTSTATE_STARTED);
                break;
            }
            break;

        case SCRIPTSTATE_CONNECTED:
            hr = S_OK;
            switch (m_ssState)
            {
            case SCRIPTSTATE_INITIALIZED:
                hr = ExecutePendingScripts();
                if (SCRIPT_E_REPORTED == hr)
                    hr = S_OK;
                // Fall through
            case SCRIPTSTATE_STARTED:
                hrT = Run();
                if (FAILED(hrT))
                    hr = hrT;
                break;
            case SCRIPTSTATE_DISCONNECTED:
                Reconnect();
                hr = S_OK;
                // The reconnect should never fail when called from here.
                break;
            }
            break;

        case SCRIPTSTATE_DISCONNECTED:
            hr = PseudoDisconnect();
            break;

        case SCRIPTSTATE_CLOSED:
            hr = Close();
            break;

        case SCRIPTSTATE_UNINITIALIZED:
            switch (m_ssState)
            {
            case SCRIPTSTATE_CONNECTED:
            case SCRIPTSTATE_DISCONNECTED:
                Stop();
            case SCRIPTSTATE_STARTED:
            case SCRIPTSTATE_INITIALIZED:
                // Forget named item values and debugging documents
                DebugDocMarkForClose();
                Reset(TRUE);
                Assert(nullptr == GetScriptSiteHolder());
                ChangeScriptState(SCRIPTSTATE_UNINITIALIZED);
                // Forget the script site:
                RELEASEPTR(m_pActiveScriptSite);
                m_dwBaseThread = NOBASETHREAD; // Forget the current thread:
                hr = S_OK;
                break;
            }
            break;

        default:
            ;
        }
    }

    return hr;
}

HRESULT ScriptEngine::RegisterNamedItems(void)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    NamedItem *pnid;

    for (pnid = m_NamedItemList.First(); pnid != nullptr; pnid = pnid->pnidNext)
    {
        if (!pnid->fRegistered)
        {
            hr2 = RegisterNamedItem(pnid);
            if (FAILED(hr2))
                hr = hr2;
        }
    }

    return hr;
}

STDMETHODIMP ScriptEngine::ResetNamedItems(void)
{
    m_moduleIDNext = 0;
    m_NamedItemList.Reset();
    m_moduleIDNext = 1 + m_NamedItemList.FindHighestModuleID();
    m_lwCookieCount = 0;
    return S_OK;
}

void ScriptEngine::FreeEventSinks(void)
{
    if (nullptr == eventSinks)
    {
        return;
    }
    for (int i = 0; i < eventSinks->Count(); i++)
    {
        eventSinks->Item(i)->Release();
    }
    eventSinks->Clear();
    eventSinks = nullptr;
}

STDMETHODIMP ScriptEngine::Reset(BOOL fFull)
{
    HRESULT hr;
    HRESULT hrT;

    if (fFull)
    {
        m_lcidUser              = GetDefaultLocale();
        m_lcidUserDefault       = m_lcidUser;
    }

    m_codepage = GetACP(); // Default codepage
    m_fIsValidCodePage = TRUE;

    // free the event sinks
    FreeEventSinks();

    if (nullptr != eventHandlers)
    {
        long i;
        BaseEventHandler *eventHandler;

        for (i = eventHandlers->Count(); i-- > 0; )
        {
            eventHandler = eventHandlers->Item(i);
            Assert(eventHandler != nullptr);
            if (!eventHandler->ShouldPersist())
            {
                eventHandler->Release();
                eventHandlers->Remove(eventHandler);
            }
            else
                eventHandler->Reset();
        }
        eventHandlers = nullptr;
    }

    this->RemoveScriptBodyMap();
    
    this->CleanupHalt();    // Release alll the debug stack frames in the list before null out the list.
    this->debugStackFrame = nullptr; // Memory for debugStackFrame will be reclaimed when we release the script site and the script context below

    // Forget external references, and delete non-persistent named items:
    hr = ResetNamedItems();
    Assert(SUCCEEDED(hr));
    // ResetNamedItems never fails.

    // Free the session object
    if (nullptr != GetScriptSiteHolder())
    {
        Assert(GetScriptSiteHolder()->GetScriptSiteContext() == this->scriptContext);
        // The script site has taken ownership of the script context;
        this->scriptContext = nullptr;
        GetScriptSiteHolder()->Close();
        GetScriptSiteHolder()->Release();
        SetScriptSiteHolder(nullptr);
    }


    // if script engine is created but SetScriptSite is not called. unlikely to
    // happen in trident.
    if (this->scriptContext != nullptr)
    {
        if(fFull)
        {
            this->MapDebugDocument( [] (ScriptDebugDocument* document){
                document->CloseDocument();
            });
        }
        this->scriptContext = nullptr;
    }


    // Throw away non-persisting code blocks and mark the remaining blocks
    // as needing to run.
    DisableInterrupts();
    if (nullptr != m_pglbod)
    {
        long ibod;
        BOD *pbod;

        for (ibod = m_pglbod->Cv(); ibod-- > 0; )
        {
            pbod = (BOD *)m_pglbod->PvGet(ibod);
            if (pbod->grfbod & fbodPersist)
            {
                pbod->grfbod = fbodPersist | fbodRun;
                pbod->pbody->ClearAllBreakPoints();
            }
            else
            {
                pbod->pbody->Release();
                m_pglbod->Delete(ibod);
            }
        }
    }
    EnableInterrupts();
    if (fFull)
    {
        ResetSecurity();
        ResetDebugger();

        CScriptSourceDocumentText *pdoc;
        // remove all non debugging documents
        while (nullptr != (pdoc = m_pNonDebugDocFirst))
        {
            pdoc->Close();
            // Extra Release, since we did an extra AddRef (in NonDbgGetSourceDocumentFromHostContext)
            pdoc->Release();
            Assert(pdoc != m_pNonDebugDocFirst);
        }
    }
    else
    {
        // Create the new session object and re-register named items
        hrT = ScriptSite::Create(this, m_pActiveScriptSite, USE_ARENA, &scriptSiteHolder);
        if (FAILED(hrT) && SUCCEEDED(hr))
            hr = hrT;
        hrT = RegisterNamedItems();
        if (FAILED(hrT) && SUCCEEDED(hr))
            hr = hrT;
    }
    if (scriptAllocator != nullptr)
    {
        HeapDelete(scriptAllocator);
        scriptAllocator = nullptr;
    }

    return hr;
}

HRESULT ScriptEngine::CloseInternal()
{
    BOD bod;
    HRESULT hr;

    switch(m_ssState)
    {
    case SCRIPTSTATE_CONNECTED:
    case SCRIPTSTATE_DISCONNECTED:
        IFFAILRET(ValidateBaseThread());
        Stop();
    case SCRIPTSTATE_INITIALIZED:
    case SCRIPTSTATE_STARTED:
        IFFAILRET(ValidateBaseThread());
        // Forget external references, and delete non-persistent named items:
        hr = ResetNamedItems();
        Assert(SUCCEEDED(hr)); // ResetNamedItems never fails.

        if (nullptr != this->scriptContext)
        {
            m_fClearingDebugDocuments = TRUE;
            this->scriptContext->EnsureClearDebugDocument();
            this->scriptContext->diagProbesContainer.UninstallInlineBreakpointProbe(this); // ScriptEngine closed, uninstall probe
            this->scriptContext->SetDbgRegisterFunction(nullptr); // Removing the registration function
        }

        if (nullptr != GetScriptSiteHolder())
        {
            GetScriptSiteHolder()->Close();
        }
        Reset(TRUE);
    case SCRIPTSTATE_UNINITIALIZED:
        ChangeScriptState(SCRIPTSTATE_CLOSED);
        // Free up named item list

        m_NamedItemList.DestroyAll();

        if (m_pActiveScriptSite)
        {
            m_pActiveScriptSite->Release();
            m_pActiveScriptSite = nullptr;
        }

        // Free the session object
        if (nullptr != GetScriptSiteHolder())
        {
            Js::ScriptContext* scriptSiteContext =  GetScriptSiteHolder()->GetScriptSiteContext();
            if(scriptSiteContext)
            {
                scriptSiteContext->SetActiveScriptDirect(NULL);
                scriptSiteContext->diagProbesContainer.Close();
            }
            Assert(GetScriptSiteHolder()->GetScriptSiteContext() == this->scriptContext);
            // The script site has taken ownership of the script context;
            this->scriptContext = nullptr;

            GetScriptSiteHolder()->Close();
            GetScriptSiteHolder()->Release();
            SetScriptSiteHolder(nullptr);
        }

        if (m_activeScriptDirectHost != nullptr)
        {
            m_activeScriptDirectHost->Release();
            m_activeScriptDirectHost = nullptr;
        }

        ResetSecurity();
        ResetDebugger();

        CScriptSourceDocumentText *pdoc;
        // remove all non debugging documents
        while (nullptr != (pdoc = m_pNonDebugDocFirst))
        {
            pdoc->Close();
            Assert(pdoc != m_pNonDebugDocFirst);
        }

    case SCRIPTSTATE_CLOSED:
        ;
    }

    // Free the event sinks.
    FreeEventSinks();

    // Free all event handlers.
    BaseEventHandler *eventHandler;

    if (eventHandlers != nullptr)
    {
        for (int i = eventHandlers->Count() -1; i >=0; i--)
        {
            eventHandler = eventHandlers->Item(i);
            eventHandler->Release();
        }
        eventHandlers->Clear();
        eventHandlers = nullptr;
    }

    if (scriptAllocator != nullptr)
    {
        HeapDelete(scriptAllocator);
        scriptAllocator = nullptr;
    }

    if (nullptr != jsOps)
    {
        jsOps->Release();
        jsOps = nullptr;
    }

    // Free all code bodies
    if (nullptr != m_pglbod)
    {
        while (m_pglbod->FPop(&bod))
            bod.pbody->Release();
        m_pglbod->Release();
        m_pglbod = nullptr;
    }

    if (this->scriptContext)
    {
        Assert(!this->scriptContext->IsRunningScript());
        this->scriptContext = nullptr;
    }

    return S_OK;
}

STDMETHODIMP ScriptEngine::Close(void)
{
    if (this->scriptContext != nullptr && this->scriptContext->IsInDebugMode())
    {
        // Lock since enumeration of code contexts (via ScriptEngine::EnumCodeContextsOfPosition()) occurs
        // on a different thread and we don't want enumeration to proceed as the engine is being closed.
        AutoCriticalSection autoCriticalSection(this->threadContext->GetEtwRundownCriticalSection());
        return this->CloseInternal();
    }
    else
    {
        return this->CloseInternal();
    }
}

HRESULT ScriptEngine::RegisterNamedItemHasCode(NamedItem *pnid)
{
    HRESULT hr = S_OK;
    if (pnid->dwFlags & SCRIPTITEM_NOCODE)
        return E_UNEXPECTED;

    if (pnid->fHasCode)
        return NOERROR;
    pnid->fHasCode = TRUE;

    // The global item was already registered with RegisterNamedItem
    if (pnid->moduleID == kmodGlobal)
        return NOERROR;

    if (!(pnid->dwFlags & SCRIPTITEM_CODEONLY))
    {
        IDispatch * pdisp;

        IFFAILRET(GetObjectOfItem(&pdisp, pnid));
        if (nullptr != pdisp)
        {
            // set the default dispatch for the module:
            hr = AddDefaultDispatch(pnid->moduleID, pdisp);
            pdisp->Release();
        }
    }
    return hr;
}

HRESULT ScriptEngine::RegisterNamedItem(NamedItem *pnid)
{
    // Register the item as a global name:

    HRESULT hr = S_OK;
    IDispatch *pdisp = nullptr;

    if (pnid->fRegistered)
        return NOERROR;
    BOOL fIsGlobalObject = pnid->moduleID == kmodGlobal;

    // If this is the global module object, or it has code
    // behind it, we have to instantiate it now:
    if (fIsGlobalObject || pnid->fHasCode)
    {
        hr = GetObjectOfItem(&pdisp, pnid);
        if (FAILED(hr))
        {
            pdisp = nullptr;
            // This hr can be ignored
            hr = S_OK;
        }
    }

    // If the object is visible, add it to the binder:
    if (pnid->dwFlags & SCRIPTITEM_ISVISIBLE)
    {
        pnid->lwCookie = ++m_lwCookieCount;
        if (m_lwCookieCount == 0)
        {
            m_lwCookieCount--;
            hr = E_FAIL;
        }
        else
        {
            hr = GetScriptSiteHolder()->AddExternalObject(pnid->bstrItemName, pdisp, m_lwCookieCount);
        }
    }

    if (SUCCEEDED(hr))
    {
        // If the object is the global object, or it has code behind it, set
        // the default dispatch for the module:
        if ((pnid->fHasCode || fIsGlobalObject) && pdisp)
        {
            hr = AddDefaultDispatch(pnid->moduleID, pdisp);
        }

        if (SUCCEEDED(hr))
        {
            pnid->fRegistered = TRUE;
        }
    }

    RELEASEPTR(pdisp);
    return hr;
}

STDMETHODIMP ScriptEngine::AddNamedItem(LPCOLESTR pcszName, DWORD dwFlags)
{
    HRESULT hr;

    VALIDATE_STRING(pcszName);
    IFFAILRET(ValidateBaseThread());

    NamedItem * pnid = m_NamedItemList.Find(pcszName);
    if (pnid == nullptr)
    {
        // Create a new named item
        pnid = new NamedItem;
        IFNULLMEMRET(pnid);
        pnid->bstrItemName = SysAllocString(pcszName);
        if (pnid->bstrItemName == nullptr)
        {
            delete pnid;
            return E_OUTOFMEMORY;
        }
        pnid->dwFlags = dwFlags;
        if (dwFlags & SCRIPTITEM_GLOBALMEMBERS)
            pnid->moduleID = kmodGlobal;
        else
            pnid->moduleID = m_moduleIDNext++;

        // Register the item as a global name:
        hr = RegisterNamedItem(pnid);
        if (FAILED(hr))
        {
            delete pnid;
            return hr;
        }
        m_NamedItemList.AddFirst(pnid);
    }
    else
    {
        // We are re-adding an existing named item
        if (dwFlags != pnid->dwFlags)
            return E_INVALIDARG; // No changes allowed to existing named items
    }
    return S_OK;
}

STDMETHODIMP ScriptEngine::AddTypeLib(REFGUID rguidTypeLib, DWORD dwMajor, DWORD dwMinor, DWORD dwFlags)
{
    AssertMsg(FALSE, "Not implemented");
    return E_NOTIMPL;
}

STDMETHODIMP ScriptEngine::GetScriptDispatch(LPCOLESTR pcszItemName, IDispatch **ppdisp)
{
    HRESULT     hr;
    Js::ModuleID       moduleID = kmodGlobal;
    NamedItem * pnid;

    // As global script methods are defined, a dispid is allocated and added
    // to the macro IDispatch. This IDispatch is returned here.
    VALIDATE_WRITE_POINTER(ppdisp, IDispatch *);
    *ppdisp = nullptr;

    IFFAILRET(ValidateBaseThread());

    // Figure out what module this goes in. Assume global module if directHostObject is set
    // and "window" named item is not.
    if (pcszItemName && pcszItemName[0] != WIDE('\0'))
    {
        pnid = m_NamedItemList.Find(pcszItemName);
        if (!pnid)
        {
            if (scriptContext->GetGlobalObject()->GetDirectHostObject() == nullptr)
            {
                DebugPrintf((L"Unable to find named item for script\n"));
                return E_INVALIDARG;
            }
        }
        else
        {
            moduleID = pnid->moduleID;
        }
    }

    // Get the IDispatch for this module:
    if (nullptr == GetScriptSiteHolder())
        return E_UNEXPECTED;


    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if (moduleID == kmodGlobal)
        {
            *ppdisp = GetScriptSiteHolder()->GetGlobalObjectDispatch();
        }
        else
        {
            hr = GetScriptSiteHolder()->GetModuleDispatch(moduleID, ppdisp);
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

STDMETHODIMP ScriptEngine::GetScriptState(SCRIPTSTATE *pssState)
{
    VALIDATE_WRITE_POINTER(pssState, SCRIPTSTATE);
    *pssState = m_ssState;
    return S_OK;
}

STDMETHODIMP ScriptEngine::GetCurrentScriptThreadID(SCRIPTTHREADID *pstidThread)
{
    VALIDATE_WRITE_POINTER(pstidThread, SCRIPTTHREADID);
    *pstidThread = GetCurrentThreadId();
    return S_OK;
}

STDMETHODIMP ScriptEngine::GetScriptThreadID(DWORD dwWin32ThreadId, SCRIPTTHREADID *pstidThread)
{
    VALIDATE_WRITE_POINTER(pstidThread, SCRIPTTHREADID);
    *pstidThread = dwWin32ThreadId;
    return S_OK;
}

STDMETHODIMP ScriptEngine::GetScriptThreadState(SCRIPTTHREADID stidThread, SCRIPTTHREADSTATE *pstsState)
{
    if (stidThread == SCRIPTTHREADID_CURRENT)
        stidThread = GetCurrentThreadId();
    else if (stidThread == SCRIPTTHREADID_BASE)
        stidThread = m_dwBaseThread;

    VALIDATE_WRITE_POINTER(pstsState, SCRIPTTHREADSTATE);
    *pstsState = SCRIPTTHREADSTATE_NOTINSCRIPT;
    if (m_dwBaseThread == NOBASETHREAD)
        return E_UNEXPECTED;
    if (stidThread != m_dwBaseThread)
        return E_INVALIDARG;
    *pstsState = m_stsThreadState;
    return S_OK;
}

STDMETHODIMP ScriptEngine::InterruptScriptThread(SCRIPTTHREADID stidThread, const EXCEPINFO *pexcepinfo, DWORD /* dwFlags */)
{
    return E_NOTIMPL;
}

STDMETHODIMP ScriptEngine::Clone(IActiveScript **ppscript)
{
    HRESULT         hr = NOERROR;
    ScriptEngine *  oleScriptNew;

    CHECK_POINTER(ppscript);
    *ppscript = nullptr;

#if DEBUG
    //    dprintf("Cloning %s script engine %p\n", g_pstrDLLName, this);
#endif // DEBUG

    if (m_ssState == SCRIPTSTATE_CLOSED)
        return E_UNEXPECTED;

    // Create a new script instance:
    oleScriptNew = HeapNewNoThrow(ScriptEngine, m_riidLanguage, m_pszLanguageName);
    IFNULLMEMRET(oleScriptNew);
#if ECMACP
    oleScriptNew->SetECMACP(m_fECMACP); // propagate language variant
#endif // ECMACP
    if (!oleScriptNew->InitializeThreadBound())
    {
        delete oleScriptNew;
        return E_OUTOFMEMORY;
    }

    DisableInterrupts();

    // If we're loaded, so is our child.  That way, all he has to do is
    // set his site to become initialized.
    oleScriptNew->m_fPersistLoaded = m_fPersistLoaded;

    IFFAILGO(m_NamedItemList.Clone(&oleScriptNew->m_NamedItemList));
    oleScriptNew->m_moduleIDNext = 1 + oleScriptNew->m_NamedItemList.FindHighestModuleID();

    Js::ScriptContext * newScriptContext = nullptr;
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {

        newScriptContext = oleScriptNew->EnsureScriptContext();
#ifdef PROFILE_EXEC
        // Use the profiler that we cloned from
        newScriptContext->SetProfilerFromScriptContext(this->scriptContext);
#endif
#ifdef PROFILE_MEM
        newScriptContext->DisableProfileMemoryDumpOnDelete();
#endif
        newScriptContext->CloneSources(this->scriptContext);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    if (FAILED(hr))
    {
        goto LReturn;
    }

    hr = NOERROR;

LReturn:
    EnableInterrupts();
    if (FAILED(hr))
    {
        oleScriptNew->Release();
    }
    else
    {
        oleScriptNew->SetIsCloned(this);
#ifdef PROFILE_EXEC
        oleScriptNew->originalScriptSite = this->GetScriptSiteHolder();
#endif
        *ppscript = (IActiveScript *)oleScriptNew;
    }
    return hr;
}

HRESULT
    ScriptEngine::CloneScriptBodies(ScriptEngine *oleScriptNew)
{
    HRESULT         hr = NOERROR;
    long            ibod, cbod;
    BOD             bod;

    // Clone the code blocks
    if (nullptr != m_pglbod && 0 < (cbod = m_pglbod->Cv()))
    {
        if (nullptr == oleScriptNew->m_pglbod &&
            nullptr == (oleScriptNew->m_pglbod = HeapNewNoThrow(GL,sizeof(BOD))))
        {
            FAILGO(E_OUTOFMEMORY);
        }

        try
        {
            AUTO_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
            for (ibod = 0; ibod < cbod; ++ibod)
            {
                m_pglbod->Get(ibod, &bod);
                if (!(bod.grfbod & fbodPersist))
                    continue;
                bod.grfbod = fbodPersist | fbodRun;
                bod.pbody  = bod.pbody->Clone(oleScriptNew);
                IFNULLMEMGO(bod.pbody);
                if (!oleScriptNew->m_pglbod->FAdd(&bod))
                {
                    bod.pbody->Release();
                    bod.pbody = nullptr;
                    FAILGO(E_OUTOFMEMORY);
                }
            }
        }
        catch(Js::OutOfMemoryException)
        {
            FAILGO(E_OUTOFMEMORY);
        }
        catch (Js::StackOverflowException)
        {
            FAILGO(HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW));
        }
    }

    // Clone persistent event handlers.
    if (eventHandlers->Count() > 0)
    {
        try
        {
            AUTO_HANDLED_EXCEPTION_TYPE(ExceptionType_OutOfMemory);
            BaseEventHandler * eventHandler;
            BaseEventHandler * eventHandlerNew;
            for (int i  = 0; i < eventHandlers->Count(); i++)
            {
                eventHandler = eventHandlers->Item(i);

                if (!eventHandler->ShouldPersist())
                    continue;
                IFFAILGO(eventHandler->Clone(oleScriptNew, &eventHandlerNew));
                oleScriptNew->eventHandlers->Add(eventHandlerNew);
            }
        }
        catch(Js::OutOfMemoryException)
        {
            FAILGO(E_OUTOFMEMORY);
        }
    }

LReturn:
    return hr;
}

// *** IActiveScriptParse Methods ***

STDMETHODIMP ScriptEngine::InitNew(void)
{
    if (m_fPersistLoaded)
        return E_UNEXPECTED;            // Already loaded
    m_fPersistLoaded = TRUE;
    if (m_pActiveScriptSite != nullptr)
        ChangeScriptState(SCRIPTSTATE_INITIALIZED);
    return S_OK;
}

#if !_WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::AddScriptlet(
    /* [in]  */ LPCOLESTR pcszDefaultName,
    /* [in]  */ LPCOLESTR pcszCode,
    /* [in]  */ LPCOLESTR pcszItemName,
    /* [in]  */ LPCOLESTR pcszSubItemName,
    /* [in]  */ LPCOLESTR pcszEventName,
    /* [in]  */ LPCOLESTR pcszDelimiter,
    /* [in]  */ DWORD    dwSourceContext,
    /* [in]  */ ULONG    ulStartingLineNumber,
    /* [in]  */ DWORD    dwFlags,
    /* [out] */ BSTR      *pbstrName,
    /* [out] */ EXCEPINFO *pexcepinfo)
{
    return AddScriptletCore(
        pcszDefaultName, pcszCode, pcszItemName, pcszSubItemName,
        pcszEventName, pcszDelimiter, (DWORD_PTR)dwSourceContext,
        ulStartingLineNumber, dwFlags, pbstrName, pexcepinfo);
}
#endif // !_WIN64 || USE_32_OR_64_BIT

#if _WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::AddScriptlet(
    /* [in]  */ LPCOLESTR pcszDefaultName,
    /* [in]  */ LPCOLESTR pcszCode,
    /* [in]  */ LPCOLESTR pcszItemName,
    /* [in]  */ LPCOLESTR pcszSubItemName,
    /* [in]  */ LPCOLESTR pcszEventName,
    /* [in]  */ LPCOLESTR pcszDelimiter,
    /* [in]  */ DWORDLONG dwSourceContext,
    /* [in]  */ ULONG    ulStartingLineNumber,
    /* [in]  */ DWORD    dwFlags,
    /* [out] */ BSTR      *pbstrName,
    /* [out] */ EXCEPINFO *pexcepinfo)
{
    return AddScriptletCore(
        pcszDefaultName, pcszCode, pcszItemName, pcszSubItemName,
        pcszEventName, pcszDelimiter, (DWORD_PTR)dwSourceContext,
        ulStartingLineNumber, dwFlags, pbstrName, pexcepinfo);
}
#endif // _WIN64 || USE_32_OR_64_BIT

HRESULT ScriptEngine::AddScriptletCore(
    /* [in]  */ LPCOLESTR /*pcszDefaultName*/,
    /* [in]  */ LPCOLESTR   pcszCode,
    /* [in]  */ LPCOLESTR   pcszItemName,
    /* [in]  */ LPCOLESTR   pcszSubItemName,
    /* [in]  */ LPCOLESTR   pcszEventName,
    /* [in]  */ LPCOLESTR   pcszDelimiter,
    /* [in]  */ DWORD_PTR   dwSourceContext,
    /* [in]  */ ULONG       ulStartingLineNumber,
    /* [in]  */ DWORD       dwFlags,
    /* [out] */ BSTR      * pbstrName,
    /* [out] */ EXCEPINFO * pexcepinfo
    )
{
    HRESULT         hr;
    ulong   grfscr  = fscrNil;
    ULONG           lnMinHost;
    ULONG           ichMinHost;
    ULONG           ichLimHost;
    Js::ModuleID           moduleID;
    CScriptBody *   pbody     = nullptr;
    Js::FunctionBody* pFuncBody = nullptr;
    LPOLESTR        pszFunc   = nullptr;
    LPOLESTR        pszTitle  = nullptr;
    LPOLESTR        pszEvt    = nullptr;
    LPCOLESTR       pcszCodeT = pcszCode;
    NamedItem *     pnid;
    BuildString     bs, procNameBuilder;

    SETRETVAL(pexcepinfo, NoException);
    SETRETVAL(pbstrName, nullptr);

    IFFAILRET(ValidateBaseThread());
    VALIDATE_STRING(pcszCodeT);
    VALIDATE_STRING(pcszItemName);
    AssertPszN(pcszSubItemName);
    VALIDATE_STRING(pcszEventName);

    dwFlags &= SCRIPTTEXT_ALL_FLAGS;
    dwFlags |= SCRIPTTEXT_ISSCRIPTLET; // this is a scriptlet
    grfscr |= ComputeGrfscrUTF16(pcszDelimiter);
    grfscr |= (fscrImplicitThis | fscrImplicitParents);

    // If we are started, force immediate execution.
    if (0 == (SCRIPTTEXT_DELAYEXECUTION & dwFlags) &&
        (m_ssState == SCRIPTSTATE_STARTED ||
        m_ssState == SCRIPTSTATE_CONNECTED ||
        m_ssState == SCRIPTSTATE_DISCONNECTED)
        )
        dwFlags |= SCRIPTTEXT_FORCEEXECUTION;
    else
        dwFlags &= (~SCRIPTTEXT_FORCEEXECUTION);

    pnid = m_NamedItemList.Find(pcszItemName);
    if (!pnid)
    {
        DebugPrintf((L"Unable to find named item for event source\n"));
        FAILGO(E_INVALIDARG);
    }
    bs.AppendSz(OLESTR("function "));
    procNameBuilder.AppendSz(pcszItemName);
    if (nullptr != pcszSubItemName)
    {
        procNameBuilder.AppendCh('.');
        procNameBuilder.AppendSz(pcszSubItemName);
    }
    procNameBuilder.AppendSz(OLESTR("_"));
    procNameBuilder.AppendSz(pcszEventName);
    LPOLESTR procName = procNameBuilder.PszReset();

    bs.AppendSz(procName);
    if (nullptr == ostrchr(pcszEventName, OLESTR('(')))
    {
        // No param list - add an empty one.
        bs.AppendSz(OLESTR("()"));
    }
    else
    {
        // the function name doesn't include the parameter
        OLECHAR* leftParenthesis = ostrchr(procName, OLESTR('('));
        Assert(leftParenthesis != nullptr);
        *leftParenthesis = '\0';
    }
    bs.AppendSz(OLESTR("\n{\n"));
    lnMinHost  = CountNewlines(bs.PszCur());
    ichMinHost = bs.CchCur();
    bs.AppendSz(pcszCodeT);
    ichLimHost = bs.CchCur();
    bs.AppendSz(OLESTR("\n}\n"));
    uint pszFuncLen = bs.CchCur();
    if (bs.FError() || nullptr == (pszFunc = bs.PszReset()))
        FAILGO(E_OUTOFMEMORY);

    if (!pnid->fHasCode)
        IFFAILGO(RegisterNamedItemHasCode(pnid));
    moduleID = pnid->moduleID;

    { // block
        SourceContextInfo* sourceContextInfo = nullptr;
        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
        {
            sourceContextInfo = this->GetSourceContextInfo(dwSourceContext, pszFuncLen, /*isDynamicDocument*/ FALSE, /*sourceMapUrl=*/ NULL, /*profileDataCache=*/ nullptr);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
        IFFAILGO(hr);

        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ ulStartingLineNumber,
            /* ulColumnHost        */ 0,
            /* lnMinHost           */ lnMinHost,
            /* ichMinHost          */ ichMinHost,
            /* ichLimHost          */ ichLimHost,
            /* ulCharOffset        */ 0,                             // Validate this
            /* mod                 */ moduleID,
            /* grfsi               */ fsiScriptlet
        };
        if (dwFlags & SCRIPTPROC_HOSTMANAGESSOURCE && (dwSourceContext != Js::Constants::NoHostSourceContext))
        {
            si.grfsi |= fsiHostManaged;
        }

        CompileScriptException  se;

        BOOL fUsedExisting = FALSE;
        Js::Utf8SourceInfo* sourceInfo = nullptr;
        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            Js::AutoDynamicCodeReference dynamicFunctionReference(scriptContext);

            hr = Compile(pszFunc, ostrlen(pszFunc), grfscr, &si, pszTitle, &se, &pbody, &pFuncBody,
                fUsedExisting, &sourceInfo /* no source to free here */, &ScriptEngine::CompileUTF16);
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr)

            if (FAILED(hr))
            {
                if (SCRIPT_E_RECORDED == hr)
                {
                    hr = ReportCompilerError(&si, &se, pexcepinfo, sourceInfo);
                }
                goto LReturn;
            }
            if ( SCRIPTPROC_ISXDOMAIN == ( dwFlags & SCRIPTPROC_ISXDOMAIN ))
            {
                sourceInfo->SetIsXDomain();
            }
    }

    //
    // Register the event handler.
    //
    { // If the event name has extra gunk, strip it off.
        const Js::CharClassifier* charClassifier = scriptContext->GetCharClassifier();
        const OLECHAR * pchMin  =  charClassifier->SkipWhiteSpace(pcszEventName);
        const OLECHAR * pchTemp = charClassifier->SkipWhiteSpace(pchMin);
        long            cch     = (long)(pchTemp - pchMin);
        Assert(cch >= 0 && cch <= LONG_MAX);
        if (pcszEventName != pchMin || 0 != pchMin[cch])
        {
            pszEvt = (OLECHAR *)malloc((cch + 1) * sizeof(OLECHAR));
            IFNULLMEMGO(pszEvt);
            js_memcpy_s(pszEvt, (cch + 1) * sizeof(OLECHAR), pchMin, cch * sizeof(OLECHAR));
            pszEvt[cch] = '\0';
            pcszEventName = pszEvt;
        }
    }
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        hr = RegisterNamedEventHandler(pnid, pcszSubItemName, pcszEventName, dwFlags, pbody);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr)

LReturn:
    RELEASEPTR(pbody);
    FREEPTR(pszFunc);
    FREEPTR(pszTitle);
    FREEPTR(pszEvt);
    return hr;
}

HRESULT ScriptEngine::SerializeByteCodes(DWORD dwSourceCodeLength, BYTE *utf8Code, IUnknown *punkContext, DWORD_PTR dwSourceContext, ComputeGrfscrFunction ComputeGrfscr, EXCEPINFO *pexcepinfo, BYTE **byteCode, DWORD *pdwByteCodeSize)
{
    HRESULT hr = S_OK;

    BOOL fUsedExisting = FALSE;
    hr = ParseScriptTextCore(
            (void *)utf8Code,
            nullptr, // pcszItemName
            punkContext, // punkContext
            nullptr, // pcszDelimiter
            dwSourceContext,
            0, // ulStartingLineNumber
            SCRIPTTEXT_DELAYEXECUTION, // dwFlags
            false,
            dwSourceCodeLength,
            &ScriptEngine::CompileUTF8,
            ComputeGrfscr,
            fUsedExisting,
            nullptr,
            pexcepinfo
            );

    if (FAILED(hr))
    {
        return hr;
    }

    if (m_pglbod == nullptr || m_pglbod->Cv() != 1)
    {
        Assert(false);
        return E_FAIL;
    }

    DWORD dwFlags = 0;
    {
        CComPtr<IGenerateByteCodeConfig> generateByteCodeConfig;
        if (punkContext != nullptr && SUCCEEDED(punkContext->QueryInterface(IID_IGenerateByteCodeConfig, (void **)&generateByteCodeConfig)))
        {
            dwFlags = generateByteCodeConfig->GetFlags();
        }
    }

    BOD *body = (BOD*)(m_pglbod->PvGet(0));
    Js::FunctionBody *function = body->pbody->GetRootFunction();

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"ByteCodeSerializer");
    hr = Js::ByteCodeSerializer::SerializeToBuffer(scriptContext, tempAllocator, dwSourceCodeLength, utf8Code, 0, nullptr, function, function->GetHostSrcInfo(), true, byteCode, pdwByteCodeSize, dwFlags);
    END_TEMP_ALLOCATOR(tempAllocator, scriptContext);
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    body->pbody->Release();
    m_pglbod->Release();
    m_pglbod = nullptr;

    return FAILED(hr) ? hr : S_OK;
}

HRESULT ScriptEngine::DeserializeByteCodes(DWORD dwByteCodeSize, BYTE *byteCode, IActiveScriptByteCodeSource* sourceProvider, IUnknown *punkContext, DWORD_PTR dwSourceContext, ComputeGrfscrFunction ComputeGrfscr, bool execute, Js::NativeModule *nativeModule, EXCEPINFO *pexcepinfo)
{
    // Byte code execution not allowed under debugger.
    if (IsDebuggerEnvironmentAvailable(/*requery*/ true))
    {
        OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::DeserializeByteCodes: Failed because under debugger.\n");
        return E_FAIL;
    }

    BOOL fUsedExisting = FALSE;
    BYTE * bytesAndSourceAndModule[] = {byteCode,  (BYTE *)sourceProvider, (BYTE *)nativeModule};


    HRESULT hr = ParseScriptTextCore(
            (void*)bytesAndSourceAndModule,
            nullptr, // pcszItemName
            punkContext, // punkContext
            nullptr, // pcszDelimiter
            dwSourceContext,
            0, // ulStartingLineNumber,
            (execute ? 0 : SCRIPTTEXT_DELAYEXECUTION), // dwFlags
            /* allowDeferredParse */ false,
            dwByteCodeSize,
            &ScriptEngine::CompileByteCodeBuffer,
            ComputeGrfscr,
            fUsedExisting,
            nullptr,
            pexcepinfo
            );

    OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::DeserializeByteCodes: HR=0x%08X.\n", hr);
    return hr;
}

C_ASSERT(SCRIPT_E_CANT_GENERATE==Js::ByteCodeSerializer::CantGenerate);
C_ASSERT(SCRIPT_E_INVALID_BYTECODE==Js::ByteCodeSerializer::InvalidByteCode);

//
// dwSourceCodeLength: Length of the source code without the nullptr terminating character
//
HRESULT ScriptEngine::GenerateByteCodeBuffer(
    /* [in] */ DWORD dwSourceCodeLength,
    /* [size_is][in] */ __RPC__in_ecount_full(dwSourceCodeLength) BYTE *utf8Code,
    /* [in] */ __RPC__in_opt IUnknown *punkContext,
    /* [in] */ DWORD_PTR dwSourceContext,
    /* [in] */ __RPC__in EXCEPINFO *pexcepinfo,
    /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pdwByteCodeSize) BYTE **byteCode,
    /* [out] */ __RPC__out DWORD *pdwByteCodeSize)
{
    OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::GenerateByteCodeBuffer\n");

    CHECK_POINTER(byteCode);
    *byteCode = nullptr;
    if (pdwByteCodeSize != nullptr)
    {
        *pdwByteCodeSize = 0;
    }

// Temporarily turn on flags for APPX team
#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.GenerateByteCodeBufferReturnsCantGenerate)
    {
        return SCRIPT_E_CANT_GENERATE;
    }
#endif

    if(Js::Configuration::Global.flags.IncludeNativeCodeWithSerializedByteCodes)
    {
        // Don't include the source in the PE because the byte code API will provide it at deserialization time anyway.
        return SerializeNativeCode(dwSourceCodeLength, utf8Code, 0, nullptr, punkContext, dwSourceContext, false, pexcepinfo, byteCode, pdwByteCodeSize);
    }

    return SerializeByteCodes(dwSourceCodeLength, utf8Code, punkContext, dwSourceContext, ComputeGrfscrUTF8ForSerialization, pexcepinfo, byteCode, pdwByteCodeSize);
}

HRESULT ScriptEngine::ExecuteByteCodeBuffer(
    /* [in] */ DWORD dwByteCodeSize,
    /* [size_is][in] */ __RPC__in_ecount_full(dwByteCodeSize) BYTE *byteCode,
    /* [in] */ IActiveScriptByteCodeSource *pbyteCodeSource,
    /* [in] */ __RPC__in_opt IUnknown *punkContext,
    /* [in] */ DWORD_PTR dwSourceContext,
    /* [out] */ __RPC__out EXCEPINFO *pexcepinfo)
{
    OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::ExecuteByteCodeBuffer\n");

    if (pexcepinfo != nullptr)
    {
        ZeroMemory(pexcepinfo, sizeof(EXCEPINFO));
    }

#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.ExecuteByteCodeBufferReturnsInvalidByteCode)
    {
        OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::ExecuteByteCodeBuffer: Forced failure.\n");
        return SCRIPT_E_INVALID_BYTECODE;
    }
#endif

    if(Js::Configuration::Global.flags.IncludeNativeCodeWithSerializedByteCodes)
    {
        return ExecuteNativeCodeInMemoryBuffer(dwByteCodeSize, byteCode, pbyteCodeSource, punkContext, dwSourceContext, pexcepinfo);
    }
    else if (dwByteCodeSize > sizeof(IMAGE_DOS_HEADER))
    {
        IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *) byteCode;

        if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
        {
            // If we've gotten a native code image but we haven't been told to accept it, reject it
            return SCRIPT_E_INVALID_BYTECODE;
        }
    }

    return DeserializeByteCodes(dwByteCodeSize, byteCode, pbyteCodeSource, punkContext, dwSourceContext, ComputeGrfscrUTF8ForDeserialization, true, nullptr, pexcepinfo);
}

HRESULT ScriptEngine::SerializeNativeCode(DWORD dwSourceCodeLength, BYTE * utf8Code, DWORD dwFunctionTableLength, BYTE * functionTable, IUnknown *punkContext, DWORD_PTR dwSourceContext, bool includeSource, EXCEPINFO *pexcepinfo, BYTE ** nativeCode, DWORD * pdwNativeCodeSize)
{
    BYTE *byteCodeBuffer = nullptr;
    DWORD dwByteCodeSize = 0;
    HRESULT hr = S_OK;
    Js::FunctionBody *rootFunction;

#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.GenerateNativeCodeBufferReturnsCantGenerate)
    {
        return SCRIPT_E_CANT_GENERATE;
    }
#endif

    BOOL fUsedExisting = FALSE;
    hr = ParseScriptTextCore(
        (void *) utf8Code,
        nullptr, // pcszItemName
        punkContext, // punkContext
        nullptr, // pcszDelimiter
        dwSourceContext,
        0, // ulStartingLineNumber
        SCRIPTTEXT_DELAYEXECUTION, // dwFlags
        false,
        dwSourceCodeLength,
        &ScriptEngine::CompileUTF8,
        ComputeGrfscrUTF8ForNativeSerialization,
        fUsedExisting,
        nullptr,
        pexcepinfo
        );

    if (FAILED(hr))
    {
        return hr;
    }

    if (m_pglbod == nullptr || m_pglbod->Cv() != 1)
    {
        Assert(false);
        return E_FAIL;
    }

    BOD *body = (BOD*)(m_pglbod->PvGet(0));

    rootFunction = body->pbody->GetRootFunction();
    body->pbody->Release();
    m_pglbod->Release();
    m_pglbod = nullptr;

    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"ByteCodeSerializer");
    hr = Js::ByteCodeSerializer::SerializeToBuffer(scriptContext, tempAllocator, dwSourceCodeLength, utf8Code, dwFunctionTableLength, functionTable, rootFunction, rootFunction->GetHostSrcInfo(), true, &byteCodeBuffer, &dwByteCodeSize, GENERATE_BYTE_CODE_FOR_NATIVE);
    GenerateAllFunctionsForSerialization(scriptContext->GetNativeCodeGenerator(), rootFunction, includeSource ? utf8Code : nullptr, includeSource ? dwSourceCodeLength : 0, byteCodeBuffer, dwByteCodeSize, dwFunctionTableLength, functionTable, nativeCode, pdwNativeCodeSize);
    END_TEMP_ALLOCATOR(tempAllocator, scriptContext);
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    CoTaskMemFree(byteCodeBuffer);
    return hr;
}

HRESULT ScriptEngine::GenerateNativeCodeBuffer(
    /* [in] */ DWORD dwSourceCodeLength,
    /* [in, size_is] */ __RPC__in_ecount_full(dwSourceCodeLength) BYTE * utf8Code,
    /* [in] */ DWORD dwFunctionTableLength,
    /* [in, size_is] */ __RPC__in_ecount_full(dwFunctionTableLength) BYTE * functionTable,
    /* [in] */ __RPC__in_opt IUnknown  *punkContext,
    /* [in] */ DWORD_PTR dwSourceContext,
    /* [in] */ __RPC__in EXCEPINFO *pexcepinfo,
    /* [out, size_is] */ __RPC__deref_out_ecount_full_opt(*pdwNativeCodeSize) BYTE ** nativeCode,
    /* [out] */ DWORD * pdwNativeCodeSize)
{
    OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::GenerateNativeCodeBuffer\n");

    IfNullReturnError(nativeCode, E_INVALIDARG);
    *nativeCode = nullptr;
    if (pdwNativeCodeSize != nullptr)
    {
        *pdwNativeCodeSize = 0;
    }

#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.PartiallySerializeNativeCode && !dwFunctionTableLength)
    {
        const int functionCount = 1024;
        BYTE partialFunctionTable[functionCount];
        for (dwFunctionTableLength = 0; dwFunctionTableLength < functionCount; dwFunctionTableLength++)
        {
            partialFunctionTable[dwFunctionTableLength] = (dwFunctionTableLength % 2) == 0;
        }
        functionTable = partialFunctionTable;
    }
#endif

    return SerializeNativeCode(dwSourceCodeLength, utf8Code, dwFunctionTableLength, functionTable, punkContext, dwSourceContext, true, pexcepinfo, nativeCode, pdwNativeCodeSize);
}

bool GetSerializedResource(HMODULE module, int identifier, BYTE **resource, DWORD *resourceSize)
{
    HRSRC resourceHandle = FindResourceEx(module, MAKEINTRESOURCE(identifier), MAKEINTRESOURCE(0xC0DE), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

    if (resourceHandle == nullptr)
    {
        return false;
    }

    if (resourceSize != nullptr)
    {
        *resourceSize = SizeofResource(module, resourceHandle);

        if (*resourceSize == 0)
        {
            return false;
        }
    }

    HGLOBAL resourceGlobalHandle = LoadResource(module, resourceHandle);

    if (resourceGlobalHandle == nullptr)
    {
        return false;
    }

    *resource = (BYTE *)LockResource(resourceGlobalHandle);

    if (*resource == nullptr)
    {
        return false;
    }

    return true;
}

HRESULT ScriptEngine::ExecuteNativeCodeModule(
    /* [in] */ LPCWSTR moduleName,
    /* [in] */ __RPC__in_opt IUnknown  *punkContext,
    /* [in] */ DWORD_PTR dwSourceContext,
    /* [out] */ __RPC__out EXCEPINFO *pexcepinfo)
{
    HRESULT hr = S_OK;

    OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::ExecuteNativeCodeModule\n");

    BYTE *sourceCode = nullptr;
    DWORD sourceCodeSize = 0;
    BYTE *byteCode = nullptr;
    DWORD byteCodeSize = 0;
    BYTE *nativeMap = nullptr;
    BYTE *nativeThrowMap = nullptr;
    Js::NativeModule *nativeModule = nullptr;

#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.ExecuteNativeCodeBufferReturnsInvalidByteCode)
    {
        OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::ExecuteNativeCodeModule: Forced failure.\n");
        return SCRIPT_E_INVALID_BYTECODE;
    }
#endif

    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT

    if (pexcepinfo != nullptr)
    {
        ZeroMemory(pexcepinfo, sizeof(EXCEPINFO));
    }

    HMODULE module = GetModuleHandle(moduleName);

    IfNullReturnError(module, SCRIPT_E_INVALID_NATIVE_CODE);

    // Load the various things we're going to need from the resources
    if (!GetSerializedResource(module, NativeResourceIds::SourceCodeResourceNameId, &sourceCode, &sourceCodeSize) ||
        !GetSerializedResource(module, NativeResourceIds::ByteCodeResourceNameId, &byteCode, &byteCodeSize) ||
        !GetSerializedResource(module, NativeResourceIds::NativeMapResourceNameId, &nativeMap, nullptr) ||
        !GetSerializedResource(module, NativeResourceIds::NativeThrowMapResourceNameId, &nativeThrowMap, nullptr))
    {
        return SCRIPT_E_INVALID_NATIVE_CODE;
    }

    // Have we already loaded this module? If so, we can skip some stuff.
    if (!scriptContext->TryGetNativeModule((BYTE *)module, &nativeModule))
    {
        IMAGE_NT_HEADERS *ntHeaders = (IMAGE_NT_HEADERS *)((BYTE *)module + ((IMAGE_DOS_HEADER *)module)->e_lfanew);

        nativeModule = HeapNewNoThrow(Js::NativeModule);

        if (nativeModule == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        nativeModule->base = (BYTE *)module;
        // This is imprecise, but doesn't hurt anything
        nativeModule->code = (BYTE *)module;
        // This is imprecise, but doesn't hurt anything
        nativeModule->codeSize = ntHeaders->OptionalHeader.SizeOfImage;
        nativeModule->exports = (DWORD *)((BYTE *)module + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + sizeof(IMAGE_EXPORT_DIRECTORY));
        nativeModule->exportCount = ((IMAGE_EXPORT_DIRECTORY *)((BYTE *)module + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress))->NumberOfFunctions;
        nativeModule->nativeMap = nativeMap;
        nativeModule->nativeThrowMap = nativeThrowMap;
        nativeModule->imageBase = ntHeaders->OptionalHeader.ImageBase;
        // These pointers are only used for in-memory loads
        nativeModule->textHeader = nullptr;
        nativeModule->relocHeader = nullptr;
        nativeModule->textSection = nullptr;
        nativeModule->textSectionSize = 0;
#if _M_X64 || _M_ARM
        nativeModule->pdataTable = nullptr;
        nativeModule->pdataCount = 0;
        nativeModule->functionTableHandle = nullptr;
#endif
        nativeModule->loadedInMemory = false;
        scriptContext->AddNativeModule((BYTE *)module, nativeModule);
    }

    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    if (SUCCEEDED(hr))
    {
        hr = DeserializeByteCodes(byteCodeSize, byteCode, new SimpleSourceMapper(sourceCode, sourceCodeSize), punkContext, dwSourceContext, ComputeGrfscrUTF8ForNativeDeserialization, true, nativeModule, pexcepinfo);
        // We don't hook up the actual native code here, we wait until function deserialization time.
    }

    return hr;
}

#if _M_X64 || _M_ARM
bool ExtractHeaders(BYTE *nativeCode, size_t *imageBase, IMAGE_SECTION_HEADER **textSectionHeader, IMAGE_SECTION_HEADER **relocSectionHeader, IMAGE_SECTION_HEADER **rsrcSectionHeader, IMAGE_SECTION_HEADER **edataSectionHeader, IMAGE_SECTION_HEADER **pdataSectionHeader, IMAGE_SECTION_HEADER **xdataSectionHeader)
#else
bool ExtractHeaders(BYTE *nativeCode, size_t *imageBase, IMAGE_SECTION_HEADER **textSectionHeader, IMAGE_SECTION_HEADER **relocSectionHeader, IMAGE_SECTION_HEADER **rsrcSectionHeader, IMAGE_SECTION_HEADER **edataSectionHeader)
#endif
{
    BYTE *current = nativeCode;
    IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *)current;

    *textSectionHeader = nullptr;
    *relocSectionHeader = nullptr;
    *rsrcSectionHeader = nullptr;
    *edataSectionHeader = nullptr;
#if _M_X64 || _M_ARM
    *pdataSectionHeader = nullptr;
    *xdataSectionHeader = nullptr;
#endif

    // Check to make sure we've got a PE, since we might be getting a bytecode file instead.
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE ||
        dosHeader->e_lfanew != sizeof(IMAGE_DOS_HEADER))
    {
        OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::ExecuteInMemoryNativeCodeBuffer: Invalid DOS header.\n");
        return false;
    }

    current += sizeof(IMAGE_DOS_HEADER);
#if _M_X64
    IMAGE_NT_HEADERS64 *ntHeaders = (IMAGE_NT_HEADERS64 *)current;
#else
    IMAGE_NT_HEADERS32 *ntHeaders = (IMAGE_NT_HEADERS32 *)current;
#endif

    // We're only going to check the NT signature and machine to make sure they
    // match. That should be good enough.
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::ExecuteInMemoryNativeCodeBuffer: Invalid NT header.\n");
        return false;
    }

#if _M_IX86
    if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
#elif _M_X64
    if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
#elif _M_ARM
    if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_ARMNT)
#endif
    {
        OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::ExecuteInMemoryNativeCodeBuffer: Invalid machine type.\n");
        return false;
    }

    if (ntHeaders->OptionalHeader.MajorLinkerVersion != PE_MAJOR_VERSION ||
        ntHeaders->OptionalHeader.MinorLinkerVersion != PE_MINOR_VERSION)
    {
        OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::ExecuteInMemoryNativeCodeBuffer: Bad PE version.\n");
        return false;
    }

    *imageBase = ntHeaders->OptionalHeader.ImageBase;

    // Now let's identify the sections.
#if _M_X64
    current += sizeof(IMAGE_NT_HEADERS64);
#else
    current += sizeof(IMAGE_NT_HEADERS32);
#endif

    for (int headerIndex = 0; headerIndex < ntHeaders->FileHeader.NumberOfSections; headerIndex++)
    {
        IMAGE_SECTION_HEADER *currentHeader = (IMAGE_SECTION_HEADER *)current;

        if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == currentHeader->VirtualAddress)
        {
            *relocSectionHeader = (IMAGE_SECTION_HEADER *)current;
        }
        else if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress == currentHeader->VirtualAddress)
        {
            *rsrcSectionHeader = (IMAGE_SECTION_HEADER *)current;
        }
#if _M_X64 || _M_ARM
        else if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress == currentHeader->VirtualAddress)
        {
            *pdataSectionHeader = (IMAGE_SECTION_HEADER *)current;
        }
#endif
        else if ((currentHeader->Characteristics & IMAGE_SCN_CNT_CODE) != 0)
        {
            *textSectionHeader = (IMAGE_SECTION_HEADER *)current;
        }
        else if (strcmp((const char *)currentHeader->Name, ".edata") == 0)
        {
            Assert((currentHeader->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) != 0);
            *edataSectionHeader = (IMAGE_SECTION_HEADER *)current;
        }
#if _M_X64 || _M_ARM
        else if (strcmp((const char *)currentHeader->Name, ".xdata") == 0)
        {
            Assert((currentHeader->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) != 0);
            *xdataSectionHeader = (IMAGE_SECTION_HEADER *)current;
        }
#endif

        current += sizeof(IMAGE_SECTION_HEADER);
    }

    return true;
}

bool ExtractResources(BYTE *rsrcStart, IMAGE_SECTION_HEADER *rsrcHeader, BYTE **byteCode, DWORD *byteCodeSize, AutoCOMPtr<IActiveScriptByteCodeSource>& sourceProvider, BYTE **nativeMap, DWORD *nativeMapSize, BYTE **nativeThrowMap, DWORD *nativeThrowMapSize, bool sourceProviderIsEmpty)
{
    BYTE *current = rsrcStart;

    // Skip the type directory
    current += sizeof(IMAGE_RESOURCE_DIRECTORY) + sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);

    // Name directory
    IMAGE_RESOURCE_DIRECTORY *nameDirectory = (IMAGE_RESOURCE_DIRECTORY *)current;
    current += sizeof(IMAGE_RESOURCE_DIRECTORY);

    for (int resourceIndex = 0; resourceIndex < nameDirectory->NumberOfIdEntries; resourceIndex++)
    {
        IMAGE_RESOURCE_DIRECTORY_ENTRY *nameDirectoryEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)current;

        if (!sourceProviderIsEmpty && nameDirectoryEntry->Id == NativeResourceIds::SourceCodeResourceNameId)
        {
            // If we were given code, then we shouldn't find one in the image.
            if ((IActiveScriptByteCodeSource *)(sourceProvider) != nullptr)
            {
                return false;
            }

            // Indirect through language directory
            IMAGE_RESOURCE_DATA_ENTRY *sourceCodeDataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)(rsrcStart + ((IMAGE_RESOURCE_DIRECTORY_ENTRY *)(rsrcStart + nameDirectoryEntry->OffsetToDirectory + sizeof(IMAGE_RESOURCE_DIRECTORY)))->OffsetToData);
            SimpleSourceMapper * sourceMapper = new SimpleSourceMapper(rsrcStart + sourceCodeDataEntry->OffsetToData - rsrcHeader->VirtualAddress, sourceCodeDataEntry->Size);
            sourceMapper->AddRef();
            *(&sourceProvider) = sourceMapper;
        }
        else if (nameDirectoryEntry->Id == NativeResourceIds::ByteCodeResourceNameId)
        {
            // Indirect through language directory
            IMAGE_RESOURCE_DATA_ENTRY *byteCodeDataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)(rsrcStart + ((IMAGE_RESOURCE_DIRECTORY_ENTRY *)(rsrcStart + nameDirectoryEntry->OffsetToDirectory + sizeof(IMAGE_RESOURCE_DIRECTORY)))->OffsetToData);
            *byteCode = rsrcStart + byteCodeDataEntry->OffsetToData - rsrcHeader->VirtualAddress;
            *byteCodeSize = byteCodeDataEntry->Size;
        }
        else if (nameDirectoryEntry->Id == NativeResourceIds::NativeMapResourceNameId)
        {
            // Indirect through language directory
            IMAGE_RESOURCE_DATA_ENTRY *nativeMapDataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)(rsrcStart + ((IMAGE_RESOURCE_DIRECTORY_ENTRY *)(rsrcStart + nameDirectoryEntry->OffsetToDirectory + sizeof(IMAGE_RESOURCE_DIRECTORY)))->OffsetToData);
            *nativeMap = rsrcStart + nativeMapDataEntry->OffsetToData - rsrcHeader->VirtualAddress;
            *nativeMapSize = nativeMapDataEntry->Size;
        }
        else if (nameDirectoryEntry->Id == NativeResourceIds::NativeThrowMapResourceNameId)
        {
            // Indirect through language directory
            IMAGE_RESOURCE_DATA_ENTRY *nativeThrowMapDataEntry = (IMAGE_RESOURCE_DATA_ENTRY *)(rsrcStart + ((IMAGE_RESOURCE_DIRECTORY_ENTRY *)(rsrcStart + nameDirectoryEntry->OffsetToDirectory + sizeof(IMAGE_RESOURCE_DIRECTORY)))->OffsetToData);
            *nativeThrowMap = rsrcStart + nativeThrowMapDataEntry->OffsetToData - rsrcHeader->VirtualAddress;
            *nativeThrowMapSize = nativeThrowMapDataEntry->Size;
        }

        current += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
    }

    // We should have code one way or another.
    return sourceProviderIsEmpty || ((IActiveScriptByteCodeSource *)(sourceProvider) != nullptr);
}

void FillInExportTable(_Out_writes_(numberOfFunctions) DWORD *exportTable, DWORD numberOfFunctions, _In_reads_(numberOfFunctions) DWORD *originalExportTable, IMAGE_SECTION_HEADER *textHeader)
{
    for (uint index = 0; index < numberOfFunctions; index++)
    {
        exportTable[index] = originalExportTable[index] - textHeader->VirtualAddress;
    }
}

#if _M_X64 || _M_ARM
void SetupExceptionBlock(size_t imageBase, BYTE *nativeCode, BYTE *textSection, DWORD *exportTable, IMAGE_SECTION_HEADER *textHeader, IMAGE_SECTION_HEADER *pdataHeader, IMAGE_SECTION_HEADER *xdataHeader, RUNTIME_FUNCTION *pdataTable, unsigned pdataCount, void **functionTableHandle)
{
    Assert(AutoSystemInfo::Data.IsWin8OrLater());

    RUNTIME_FUNCTION *originalPdataTable = (RUNTIME_FUNCTION *)(nativeCode + pdataHeader->PointerToRawData);
    BYTE *xdataBase = textSection + textHeader->SizeOfRawData;

    for (uint index = 0; index < pdataCount; index++)
    {
        RUNTIME_FUNCTION *pdata = &pdataTable[index];
        pdata->BeginAddress = (size_t)(originalPdataTable[index].BeginAddress - textHeader->VirtualAddress);

#if _M_X64
        DWORD functionOffset = exportTable[index];
        size_t functionSize = exportTable[index + 1] - exportTable[index];
        BYTE *xdata =
            xdataBase +
            (originalPdataTable[index].UnwindInfoAddress - xdataHeader->VirtualAddress);

        Assert(pdata->BeginAddress == functionOffset);
        pdata->EndAddress = functionOffset + (DWORD)functionSize;
        pdata->UnwindInfoAddress = (DWORD)(xdata - textSection);
        Assert((size_t)(xdata - textSection) < UINT_MAX);
        Assert(pdata->UnwindInfoAddress % 2 == 0);
#else
        pdata->UnwindData = originalPdataTable[index].UnwindData;
        if (pdata->Flag != 1)
        {
            pdata->UnwindData = pdata->UnwindData - xdataHeader->VirtualAddress + xdataBase - textSection;
        }
#endif
    }

    DWORD oldProtect;
    BOOL result = VirtualProtect(xdataBase, xdataHeader->SizeOfRawData, PAGE_READWRITE, &oldProtect);
    Assert(result);

    memcpy_s(xdataBase, xdataHeader->SizeOfRawData, nativeCode + xdataHeader->PointerToRawData, xdataHeader->SizeOfRawData);

    result = VirtualProtect(xdataBase, xdataHeader->SizeOfRawData, PAGE_EXECUTE_READ, &oldProtect);
    Assert(result);

    DWORD status = 
        NtdllLibrary::Instance->AddGrowableFunctionTable(
            functionTableHandle, 
            pdataTable, 
            pdataCount, 
            pdataCount, 
            (ULONG_PTR)textSection, 
            (ULONG_PTR)(textSection + textHeader->SizeOfRawData));
    Js::Throw::CheckAndThrowOutOfMemory(NT_SUCCESS(status));
}
#endif


HRESULT ScriptEngine::LoadInMemoryBuffer(BYTE *nativeCode, AutoCOMPtr<IActiveScriptByteCodeSource>& sourceProvider, BYTE **loadedByteCode, DWORD *loadedByteCodeSize, Js::NativeModule **loadedNativeModule, bool sourceProviderIsEmpty)
{
    HRESULT hr = S_OK;

    BYTE *byteCode = nullptr;
    DWORD byteCodeSize = 0;
    BYTE *nativeMap = nullptr;
    DWORD nativeMapSize = 0;
    BYTE *nativeThrowMap = nullptr;
    DWORD nativeThrowMapSize = 0;
    Js::NativeModule *nativeModule = nullptr;

    size_t imageBase = 0;

    IMAGE_SECTION_HEADER *textSectionHeader = nullptr;
    IMAGE_SECTION_HEADER *relocSectionHeader = nullptr;
    IMAGE_SECTION_HEADER *rsrcSectionHeader = nullptr;
    IMAGE_SECTION_HEADER *edataSectionHeader = nullptr;
#if _M_X64 || _M_ARM
    IMAGE_SECTION_HEADER *pdataSectionHeader = nullptr;
    IMAGE_SECTION_HEADER *xdataSectionHeader = nullptr;
#endif

    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT_NESTED

#if _M_X64 || _M_ARM
    if (!ExtractHeaders(nativeCode, &imageBase, &textSectionHeader, &relocSectionHeader, &rsrcSectionHeader, &edataSectionHeader, &pdataSectionHeader, &xdataSectionHeader))
#else
    if (!ExtractHeaders(nativeCode, &imageBase, &textSectionHeader, &relocSectionHeader, &rsrcSectionHeader, &edataSectionHeader))
#endif
    {
        return SCRIPT_E_INVALID_BYTECODE;
    }

    if (!ExtractResources(nativeCode + rsrcSectionHeader->PointerToRawData, rsrcSectionHeader, &byteCode, &byteCodeSize, sourceProvider, &nativeMap, &nativeMapSize, &nativeThrowMap, &nativeThrowMapSize, sourceProviderIsEmpty))
    {
        return SCRIPT_E_INVALID_BYTECODE;
    }

    if (!scriptContext->TryGetNativeModule(nativeCode, &nativeModule))
    {
        // We haven't seen this one before. We need to simulate the DLL load process
        nativeModule = HeapNewNoThrow(Js::NativeModule);

        if (nativeModule == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        IMAGE_EXPORT_DIRECTORY *exportDirectory = (IMAGE_EXPORT_DIRECTORY *) (nativeCode + edataSectionHeader->PointerToRawData);
        DWORD *exportTable = HeapNewNoThrowArray(DWORD, exportDirectory->NumberOfFunctions);

        if (exportTable == nullptr)
        {
            HeapDelete(nativeModule);
            return E_OUTOFMEMORY;
        }

        FillInExportTable(exportTable, exportDirectory->NumberOfFunctions, (DWORD *) ((BYTE *) exportDirectory + sizeof(IMAGE_EXPORT_DIRECTORY)), textSectionHeader);

#if _M_X64 || _M_ARM
#if _M_X64
        unsigned pdataCount = exportDirectory->NumberOfFunctions - 1;
#else
        unsigned pdataCount = ((IMAGE_NT_HEADERS32 *) (nativeCode + sizeof(IMAGE_DOS_HEADER)))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size / sizeof(RUNTIME_FUNCTION);
#endif
        RUNTIME_FUNCTION *pdataTable = HeapNewNoThrowArray(RUNTIME_FUNCTION, pdataCount);

        if (pdataTable == nullptr)
        {
            HeapDelete(nativeModule);
            HeapDeleteArray(exportDirectory->NumberOfFunctions, exportTable);
            return E_OUTOFMEMORY;
        }
#endif

        size_t textSectionSize =
            textSectionHeader->SizeOfRawData 
#if _M_X64 || _M_ARM
                + xdataSectionHeader->SizeOfRawData
#endif
                ;
        BYTE *textSection = (BYTE *)VirtualAlloc(
            nullptr,
            textSectionSize,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_READONLY);

        if (textSection == nullptr)
        {
            HeapDelete(nativeModule);
            HeapDeleteArray(exportDirectory->NumberOfFunctions, exportTable);
#if _M_X64 || _M_ARM
            HeapDeleteArray(pdataCount, pdataTable);
#endif
            return E_OUTOFMEMORY;
        }

#if _M_X64 || _M_ARM
        void *functionTableHandle;
        SetupExceptionBlock(imageBase, nativeCode, textSection, exportTable, textSectionHeader, pdataSectionHeader, xdataSectionHeader, pdataTable, pdataCount, &functionTableHandle);
#endif

        nativeModule->base = nativeCode;
        nativeModule->code = nativeCode + textSectionHeader->PointerToRawData;
        nativeModule->codeSize = textSectionHeader->SizeOfRawData;
        nativeModule->exports = exportTable;
        nativeModule->exportCount = exportDirectory->NumberOfFunctions;
        nativeModule->nativeMap = nativeMap;
        nativeModule->nativeThrowMap = nativeThrowMap;
        nativeModule->imageBase = imageBase;
        nativeModule->textHeader = textSectionHeader;
        nativeModule->relocHeader = relocSectionHeader;
        nativeModule->textSection = textSection;
        nativeModule->textSectionSize = textSectionSize;
#if _M_X64 || _M_ARM
        nativeModule->pdataTable = pdataTable;
        nativeModule->pdataCount = pdataCount;
        nativeModule->functionTableHandle = functionTableHandle;
#endif
        nativeModule->loadedInMemory = true;
        scriptContext->AddNativeModule(nativeCode, nativeModule);
    }

    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    if (loadedByteCode != nullptr)
    {
        *loadedByteCode = byteCode;
    }

    if (loadedByteCodeSize != nullptr)
    {
        *loadedByteCodeSize = byteCodeSize;
    }

    *loadedNativeModule = nativeModule;
    return hr;
}

HRESULT ScriptEngine::ExecuteNativeCodeInMemoryBuffer(DWORD dwNativeCodeSize, BYTE *nativeCode, IActiveScriptByteCodeSource* sourceProvider, IUnknown *punkContext, DWORD_PTR dwSourceContext, EXCEPINFO *pexcepinfo)
{
    HRESULT hr = S_OK;

    // What we're going to do here is simulate what the OS would do when loading
    // a PE. This allows testing native code serialization in situations where
    // people are already compiled against the byte code serialization API. Sadly,
    // the OS provides no way to load a DLL in memory.

    IfNullReturnError(nativeCode, SCRIPT_E_INVALID_BYTECODE);

#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.ExecuteNativeCodeBufferReturnsInvalidByteCode)
    {
        OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::ExecuteNativeCodeInMemoryBuffer: Forced failure.\n");
        return SCRIPT_E_INVALID_BYTECODE;
    }
#endif

    // If sourceProvider is null, which it may be, this won't do anything.
    // The purpose of this is to ensure we have at least added one ref count, and removed the one we added at the end of this method
    AutoCOMPtr<IActiveScriptByteCodeSource> autoSourceProvider(sourceProvider);

    BYTE *byteCode;
    DWORD byteCodeSize;
    Js::NativeModule *nativeModule;

    hr = LoadInMemoryBuffer(nativeCode, autoSourceProvider, &byteCode, &byteCodeSize, &nativeModule);

    if (SUCCEEDED(hr))
    {
        hr = DeserializeByteCodes(byteCodeSize, byteCode, autoSourceProvider, punkContext, dwSourceContext, ComputeGrfscrUTF8ForNativeDeserialization, true, nativeModule, pexcepinfo);
    }

    OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::ExecuteInMemoryNativeCodeBuffer: HR=0x%08X.\n", hr);
    return hr;
}

HRESULT ScriptEngine::ExecuteNativeCodeInMemoryBuffer(
    /* [in] */ DWORD dwNativeCodeSize,
    /* [size_is][in] */ __RPC__in_ecount_full(dwByteCodeSize) BYTE *nativeCode,
    /* [in] */ __RPC__in_opt IUnknown *punkContext,
    /* [in] */ DWORD_PTR dwSourceContext,
    /* [out] */ __RPC__out EXCEPINFO *pexcepinfo)
{
    OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::ExecuteNativeCodeInMemoryBuffer\n");

    if (pexcepinfo != nullptr)
    {
        ZeroMemory(pexcepinfo, sizeof(EXCEPINFO));
    }

    return ExecuteNativeCodeInMemoryBuffer(dwNativeCodeSize, nativeCode, nullptr, punkContext, dwSourceContext, pexcepinfo);
}

#if !_WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::ParseScriptText(
    /* [in]  */ LPCOLESTR pcszCode,
    /* [in]  */ LPCOLESTR pcszItemName,
    /* [in]  */ IUnknown  *punkContext,
    /* [in]  */ LPCOLESTR pcszDelimiter,
    /* [in]  */ DWORD     dwSourceContext,
    /* [in]  */ ULONG     ulStartingLineNumber,
    /* [in]  */ DWORD     dwFlags,
    /* [out] */ VARIANT   *pvarResult,
    /* [out] */ EXCEPINFO *pexcepinfo)
{
    BOOL fUsedExisting = FALSE;
    HRESULT hr = ParseScriptTextCore(
        (void *)pcszCode,
        pcszItemName,
        punkContext,
        pcszDelimiter,
        (DWORD_PTR)dwSourceContext,
        ulStartingLineNumber,
        dwFlags,
        /* allowDeferredParse */ true,
        ostrlen(pcszCode),
        &ScriptEngine::CompileUTF16,
        ComputeGrfscrUTF16,
        fUsedExisting,
        pvarResult,
        pexcepinfo
        );

    return hr;
}
#endif // !_WIN64 || USE_32_OR_64_BIT

#if _WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::ParseScriptText(
    /* [in]  */ LPCOLESTR   pcszCode,
    /* [in]  */ LPCOLESTR   pcszItemName,
    /* [in]  */ IUnknown  * punkContext,
    /* [in]  */ LPCOLESTR   pcszDelimiter,
    /* [in]  */ DWORDLONG   dwSourceContext,
    /* [in]  */ ULONG       ulStartingLineNumber,
    /* [in]  */ DWORD       dwFlags,
    /* [out] */ VARIANT   * pvarResult,
    /* [out] */ EXCEPINFO * pexcepinfo)
{
    BOOL fUsedExisting = FALSE;
    return ParseScriptTextCore(
        (void *)pcszCode,
        pcszItemName,
        punkContext,
        pcszDelimiter,
        (DWORD_PTR)dwSourceContext,
        ulStartingLineNumber,
        dwFlags,
        /* allowDeferredParse */ true,
        ostrlen(pcszCode),
        &ScriptEngine::CompileUTF16,
        ComputeGrfscrUTF16,
        fUsedExisting,
        pvarResult,
        pexcepinfo
        );
}
#endif // _WIN64 || USE_32_OR_64_BIT

#if !_WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::ParseScriptText(
    /* [in]  */ BYTE *    pcszCode,
    /* [in]  */ ULONG     ulCodeOffset,
    /* [in]  */ DWORD     dwLength,
    /* [in]  */ LPCOLESTR pcszItemName,
    /* [in]  */ IUnknown  *punkContext,
    /* [in]  */ const BYTE * pcszDelimiter,
    /* [in]  */ DWORD     dwSourceContext,
    /* [in]  */ ULONG     ulStartingLineNumber,
    /* [in]  */ DWORD     dwFlags,
    /* [out] */ VARIANT   *pvarResult,
    /* [out] */ EXCEPINFO *pexcepinfo)
{
    Assert(pcszCode != nullptr);
    Assert(dwLength >= ulCodeOffset);

    LPCUTF8 pszCode = pcszCode + ulCodeOffset;
    DWORD cbCode = dwLength - (DWORD)ulCodeOffset;
    BOOL fUsedExisting = FALSE;
    HRESULT hr;

    // Add the code to delete into engines deleting list

    Js::Utf8SourceInfo* pScriptSourceInfo = nullptr;
    hr = ParseScriptTextCore(
        (void *)pszCode,
        pcszItemName,
        punkContext,
        pcszDelimiter,
        (DWORD_PTR)dwSourceContext,
        ulStartingLineNumber,
        dwFlags,
        /* allowDeferredParse */ true,
        cbCode,
        &ScriptEngine::CompileUTF8,
        ComputeGrfscrUTF8,
        fUsedExisting,
        pvarResult,
        pexcepinfo,
        &pScriptSourceInfo
        );

    if (pScriptSourceInfo == nullptr || fUsedExisting)
    {
        //delete [] pcszCode;
        HeapFree(GetProcessHeap(), 0, pcszCode);
    }
    else
    {
        pScriptSourceInfo->SetTridentBuffer(pcszCode);
    }

    return hr;
}
#endif // !_WIN64 || USE_32_OR_64_BIT

#if _WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::ParseScriptText(
    /* [in]  */ BYTE *      pcszCode,
    /* [in]  */ ULONG       ulCodeOffset,
    /* [in]  */ DWORD       dwLength,
    /* [in]  */ LPCOLESTR   pcszItemName,
    /* [in]  */ IUnknown  * punkContext,
    /* [in]  */ const BYTE *   pcszDelimiter,
    /* [in]  */ DWORDLONG   dwSourceContext,
    /* [in]  */ ULONG       ulStartingLineNumber,
    /* [in]  */ DWORD       dwFlags,
    /* [out] */ VARIANT   * pvarResult,
    /* [out] */ EXCEPINFO * pexcepinfo)
{
    Assert(pcszCode != nullptr);
    Assert(dwLength >= ulCodeOffset);

    LPCUTF8 pszCode = pcszCode + ulCodeOffset;
    DWORD cbCode = dwLength - (DWORD)ulCodeOffset;
    BOOL fUsedExisting = FALSE;
    HRESULT hr;

    // Add the code to delete into engines deleting list

    Js::Utf8SourceInfo* pScriptSourceInfo = nullptr;

    hr = ParseScriptTextCore(
        (void *)pszCode,
        pcszItemName,
        punkContext,
        pcszDelimiter,
        (DWORD_PTR)dwSourceContext,
        ulStartingLineNumber,
        dwFlags,
        true,
        cbCode,
        &ScriptEngine::CompileUTF8,
        ComputeGrfscrUTF8,
        fUsedExisting,
        pvarResult,
        pexcepinfo,
        &pScriptSourceInfo
        );
    if (pScriptSourceInfo == nullptr || fUsedExisting)
    {
        //delete [] pcszCode;
        HeapFree(GetProcessHeap(), 0, pcszCode);
    }
    else
    {
        pScriptSourceInfo->SetTridentBuffer(pcszCode);
    }
    return hr;
}
#endif // _WIN64 || USE_32_OR_64_BIT

HRESULT ScriptEngine::ParseScriptTextCore(
    /* [in]  */ void *   pcszCode,
    /* [in]  */ LPCOLESTR   pcszItemName,
    /* [in]  */ IUnknown  * punkContext,
    /* [in]  */ const void *   pcszDelimiter,
    /* [in]  */ DWORD_PTR   dwSourceContext,
    /* [in]  */ ULONG       ulStartingLineNumber,
    /* [in]  */ DWORD       dwFlags,
    /* [in]  */ bool allowDeferredParse,
    /* [in]  */ size_t      len,
    /* [in]  */ CoreCompileFunction fnCoreCompile,
    /* [in]  */ ComputeGrfscrFunction ComputeGrfscr,
    /* [out] */ BOOL &fUsedExisiting,
    /* [out] */ VARIANT   * pvarResult,
    /* [out] */ EXCEPINFO * pexcepinfo,
    /* [out] */ Js::Utf8SourceInfo** ppSourceInfo
    )
{
    HRESULT     hr;
    Js::ModuleID       moduleID       = kmodGlobal;
    LPCOLESTR    pszTitle = nullptr;
    void * pcszCodeT = pcszCode;
    NamedItem * pnid      = nullptr;
    Js::FunctionBody* pFuncBody = nullptr;

#if defined(ONE_PAGE_SS)
    this->GetScriptSiteHolder()->SessionReset();
#endif //defined(ONE_PAGE_SS)

    IFFAILRET(ValidateBaseThread());

    if (pvarResult)
        VariantInit(pvarResult);
    SETRETVAL(pexcepinfo, NoException);
    SETRETVAL(ppSourceInfo, nullptr);

    AutoIdleDecommit autoIdleDecommit(scriptContext->GetRecycler());

    dwFlags &= SCRIPTTEXT_ALL_FLAGS;

    // If they ask for a result or we are started, force immediate execution
    if (nullptr != pvarResult ||
        0 == (SCRIPTTEXT_DELAYEXECUTION & dwFlags) &&
        (m_ssState == SCRIPTSTATE_STARTED ||
        m_ssState == SCRIPTSTATE_CONNECTED ||
        m_ssState == SCRIPTSTATE_DISCONNECTED)
        )
        dwFlags |= SCRIPTTEXT_FORCEEXECUTION;
    else
        dwFlags &= (~SCRIPTTEXT_FORCEEXECUTION);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    if (Js::Configuration::Global.flags.Off.IsEnabled(Js::RunPhase))
    {
        dwFlags &= (~SCRIPTTEXT_FORCEEXECUTION);
    }
#endif

    if (nullptr == pcszCodeT)
        hr = NOERROR;
    else
    {
        // Figure out what module this goes in
        if (pcszItemName && pcszItemName[0] != WIDE('\0'))
        {
            pnid = m_NamedItemList.Find(pcszItemName);
            if (!pnid)
            {
                if (scriptContext->GetGlobalObject()->GetDirectHostObject() == nullptr)
                {
                    DebugPrintf((L"Unable to find named item for script\n"));
                    FAILGO(E_INVALIDARG);
                }
            }
            else
            {
                moduleID = pnid->moduleID;
            }
        }

        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            if (nullptr != pcszItemName && wcscmp(pcszItemName, L"window") != 0)
            {
                ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
                Recycler* recycler = threadContext->GetRecycler();
                Js::StringBuilder<Recycler>* stringBuilder = Js::StringBuilder<Recycler>::New(recycler, 32);

                stringBuilder->AppendSz(pcszItemName);
                stringBuilder->Append(L' ');
                stringBuilder->AppendCppLiteral(L"script block");
                pszTitle = stringBuilder->Detach();
            }
            else
            {
                pszTitle = Js::Constants::GlobalCode;
            }

            if (pszTitle == nullptr)
            {
                FAILGO(E_OUTOFMEMORY);
            }
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr)
            IFFAILGO(hr);

        if (pnid && !pnid->fHasCode)
            IFFAILGO(RegisterNamedItemHasCode(pnid));

        Assert(len <= ULONG_MAX);

        ULONG ulColumn = 0;
        ULONG ulCharOffset = 0;
        CComPtr<IActiveScriptDataCache> spProfileDataCache;
        BOOL isDynamicDocument = FALSE;
        CComBSTR sourceMapUrl;
        if (punkContext != nullptr)
        {
            CComPtr<IActiveScriptContext> spActiveScriptContext;
            if (SUCCEEDED(punkContext->QueryInterface(&spActiveScriptContext)))
            {
                HRESULT hRes = spActiveScriptContext->GetLineColumn(nullptr, &ulColumn);
                Assert(SUCCEEDED(hRes));
                hRes = spActiveScriptContext->GetOffset(&ulCharOffset);
                Assert(SUCCEEDED(hRes));
                hRes = spActiveScriptContext->IsDynamicDocument(&isDynamicDocument);
                Assert(SUCCEEDED(hRes));

                // If there is no map, for script elements it returns S_OK and BSTR param receives null/empty, 
                // for non-script elements it returns E_NOTIMPL.
                hRes = spActiveScriptContext->GetSourceMapUrl(&sourceMapUrl);
                Assert(SUCCEEDED(hRes) || hRes == E_NOTIMPL);
            }

            if(CONFIG_FLAG(WininetProfileCache))
            {
                punkContext->QueryInterface(&spProfileDataCache);
            }
        }

        SourceContextInfo* sourceContextInfo = nullptr;
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            sourceContextInfo = this->GetSourceContextInfo(dwSourceContext, (uint)len, isDynamicDocument, sourceMapUrl, spProfileDataCache);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
        IFFAILGO(hr);

        if (!fSetThreadDescription)
        {
            SetThreadDescription(sourceContextInfo->url); // the HRESULT is omitted.
        }

        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ ulStartingLineNumber,
            /* ulColumnHost        */ ulColumn,
            /* lnMinHost           */ 0,
            /* ichMinHost          */ 0,
            /* ichLimHost          */ (ulong)len,
            /* ulCharOffset        */ ulCharOffset,
            /* mod                 */ moduleID,
            /* grfsi               */ 0
        };

        hr = CreateScriptBody(pcszCodeT, len, dwFlags, allowDeferredParse, &si, pcszDelimiter, pszTitle,
            fnCoreCompile, ComputeGrfscr, fUsedExisiting, &pFuncBody, ppSourceInfo, pexcepinfo);
    }

    if ((dwFlags & SCRIPTTEXT_FORCEEXECUTION) && SUCCEEDED(hr))
    {
        if (this->scriptContext && this->threadContext->IsInScript())
        {
            // This is to prepare diagnostics mode to fire EnterScriptStart/EnterScriptEnd to host. The reason for doing this
            // is to synchronize the document with PDM, which will further notify any debuggerApp for document change and document can resolve their breakpoints
            this->scriptContext->diagProbesContainer.PrepDiagForEnterScript();
        }

        hr = ExecutePendingScripts(pvarResult, pexcepinfo);
    }

LReturn:
    return hr;
}

#if !_WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::ParseProcedureText(
    /* [in]  */ LPCOLESTR   pstrCode,
    /* [in]  */ LPCOLESTR   pstrFormalParams,
    /* [in]  */ LPCOLESTR   pstrProcedureName,
    /* [in]  */ LPCOLESTR   pstrItemName,
    /* [in]  */ IUnknown  * punkContext,
    /* [in]  */ LPCOLESTR   pstrDelimiter,
    /* [in]  */ DWORD       dwSourceContext,
    /* [in]  */ ULONG       ulStartingLineNumber,
    /* [in]  */ DWORD       dwFlags,
    /* [out] */ IDispatch **ppdisp
    )
{
    return ParseProcedureTextCore(
        pstrCode,
        pstrFormalParams,
        pstrProcedureName,
        pstrItemName,
        punkContext,
        pstrDelimiter,
        (DWORD_PTR)dwSourceContext,
        ulStartingLineNumber,
        dwFlags,
        ppdisp,
        TRUE
        );
}
#endif // !_WIN64 || USE_32_OR_64_BIT

#if _WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ScriptEngine::ParseProcedureText(
    /* [in]  */ LPCOLESTR   pstrCode,
    /* [in]  */ LPCOLESTR   pstrFormalParams,
    /* [in]  */ LPCOLESTR   pstrProcedureName,
    /* [in]  */ LPCOLESTR   pstrItemName,
    /* [in]  */ IUnknown  * punkContext,
    /* [in]  */ LPCOLESTR   pstrDelimiter,
    /* [in]  */ DWORDLONG   dwSourceContext,
    /* [in]  */ ULONG       ulStartingLineNumber,
    /* [in]  */ DWORD       dwFlags,
    /* [out] */ IDispatch **ppdisp
    )
{
    return ParseProcedureTextCore(
        pstrCode,
        pstrFormalParams,
        pstrProcedureName,
        pstrItemName,
        punkContext,
        pstrDelimiter,
        (DWORD_PTR)dwSourceContext,
        ulStartingLineNumber,
        dwFlags,
        ppdisp,
        TRUE
        );
}
#endif // _WIN64 || USE_32_OR_64_BIT

HRESULT ScriptEngine::ParseProcedureTextCore(
    /* [in]  */ LPCOLESTR   pcszCode,
    /* [in]  */ LPCOLESTR   pcszParams,
    /* [in]  */ LPCOLESTR   pcszProc,
    /* [in]  */ LPCOLESTR   pcszItem,
    /* [in]  */ IUnknown  * punkContext,
    /* [in]  */ LPCOLESTR   pcszDelim,
    /* [in]  */ DWORD_PTR   dwSourceContext,
    /* [in]  */ ULONG       ulStartingLineNumber,
    /* [in]  */ DWORD       dwFlags,
    /* [out] */ IDispatch **ppdisp,
    /* [in]  */ BOOL        fReportError
    )
{
    HRESULT         hr;
    ulong           grfscr    = fscrNil;
    NamedItem *     pnid      = nullptr;
    LPOLESTR        pszSrc    = nullptr;
    LPOLESTR        pszTitle  = nullptr;
    LPCOLESTR       pszProcName = (nullptr == pcszProc || NULL == pcszProc[0]) ? OLESTR("anonymous") : pcszProc;
    LPCOLESTR       pcszCodeT = pcszCode;
    BuildString     bs;
    SRCINFO         si;
    CComPtr<IActiveScriptDataCache> spProfileDataCache;

    JavascriptDispatch* entryPointDispatch = nullptr;
    Js::DynamicObject * pfuncScript = nullptr;
    CComBSTR sourceMapUrl;

    CHECK_POINTER(ppdisp);
    *ppdisp = nullptr;

    IFFAILRET(VerifyOnEntry());

    AutoIdleDecommit autoIdleDecommit(scriptContext->GetRecycler());

    dwFlags &= SCRIPTPROC_ALL_FLAGS;
    IFEMPTYSETNULL(pcszParams);
    IFEMPTYSETNULL(pcszProc);
    IFEMPTYSETNULL(pcszItem);
    IFEMPTYSETNULL(pcszDelim);

    si.moduleID = kmodGlobal;
    if (nullptr != pcszItem)
    {
        pnid = m_NamedItemList.Find(pcszItem);
        if (nullptr == pnid)
        {
            if (scriptContext->GetGlobalObject()->GetDirectHostObject() == nullptr)
            {
                DebugPrintf((L"Unable to find named item for script\n"));
                FAILGO(E_INVALIDARG);
            }
        }
        else
        {
            si.moduleID = pnid->moduleID;
        }
    }

    if (pnid && !pnid->fHasCode)
        RegisterNamedItemHasCode(pnid);

    ULONG ulColumn = 0;
    ULONG ulCharOffset = 0;
    BOOL isDynamicDocument = FALSE;
    if (punkContext != nullptr)
    {
        CComPtr<IActiveScriptContext> spActiveScriptContext;
        if (SUCCEEDED(punkContext->QueryInterface(&spActiveScriptContext)))
        {
            HRESULT hRes = spActiveScriptContext->GetLineColumn(nullptr, &ulColumn);
            Assert(SUCCEEDED(hRes));
            hRes = spActiveScriptContext->GetOffset(&ulCharOffset);
            Assert(SUCCEEDED(hRes));
            hRes = spActiveScriptContext->IsDynamicDocument(&isDynamicDocument);
            Assert(SUCCEEDED(hRes));

            // If there is no map, for script elements it returns S_OK and BSTR param receives null/empty, 
            // for non-script elements it returns E_NOTIMPL.
            hRes = spActiveScriptContext->GetSourceMapUrl(&sourceMapUrl);
            Assert(SUCCEEDED(hRes) || hRes == E_NOTIMPL);
        }

        if(CONFIG_FLAG(WininetProfileCache))
        {
            punkContext->QueryInterface(&spProfileDataCache);
        }
    }

    si.dlnHost             = ulStartingLineNumber;
    si.ulColumnHost        = ulColumn;
    si.ulCharOffset        = ulCharOffset;
    si.grfsi = 0;

    if (dwFlags & SCRIPTTEXT_ISSCRIPTLET)
    {
        si.grfsi |= fsiScriptlet;
    }

    // Ideally If the source is host managed it will have a valid source context, otherwise debugging will go in
    // a weird state. There are some cases in the mshtml where they pass host managed source without a valid context.
    // for example dynamically generated code, setTimeout.
    // Point fixing here to allow to create node to have better debugging experience. If mshtml agrees on passing proper flag, below extra check can be removed.

    if ((dwFlags & SCRIPTPROC_HOSTMANAGESSOURCE) && (dwSourceContext != Js::Constants::NoHostSourceContext))
    {
        si.grfsi |= fsiHostManaged;
    }

    bs.AppendSz(OLESTR("function "));
    bs.AppendSz(pszProcName);
    if (nullptr != pcszParams)
    {
        bs.AppendCh('(');
        bs.AppendSz(pcszParams);
        bs.AppendSz(OLESTR(")\n"));
    }
    else
        bs.AppendSz(OLESTR("()\n"));
    bs.AppendSz(OLESTR("{\n"));
    if (dwFlags & SCRIPTPROC_ISEXPRESSION)
    {
        bs.AppendSz(OLESTR("return ( "));
    }

    si.lnMinHost    = CountNewlines(bs.PszCur());
    si.ichMinHost   = bs.CchCur();
    bs.AppendSz(pcszCodeT);
    si.ichLimHost   = bs.CchCur();

    if (dwFlags & SCRIPTPROC_ISEXPRESSION)
    {
        bs.AppendSz(OLESTR(" ) "));
    }
    bs.AppendSz(OLESTR("\n}\n"));
    uint pszSrcLen = bs.CchCur();
    if (bs.FError() || nullptr == (pszSrc = bs.PszReset()))
        FAILGO(E_OUTOFMEMORY);

    // compile it
    grfscr |= ComputeGrfscrUTF16(pcszDelim);

    if (dwFlags & SCRIPTPROC_IMPLICIT_THIS)
    {
        grfscr |= fscrImplicitThis;
    }
    if (dwFlags & SCRIPTPROC_IMPLICIT_PARENTS)
    {
        grfscr |= fscrImplicitParents;
    }

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        si.sourceContextInfo = this->GetSourceContextInfo(dwSourceContext, (uint)pszSrcLen, isDynamicDocument, sourceMapUrl, spProfileDataCache);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    IFFAILGO(hr);

    // Create the code body, passing the name of the procedure the caller specified.
    // This is the function we will wrap and return.
    Js::ScriptContext* scriptContext = GetScriptSiteHolder()->GetScriptSiteContext();
    {
        CompileScriptException  se;

        BOOL fUsedExisting = FALSE;
        Js::Utf8SourceInfo* sourceInfo = NULL;
        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            Js::AutoDynamicCodeReference dynamicFunctionReference(scriptContext);
            AutoCOMPtr<CScriptBody> pbody;
            Js::FunctionBody* funcBody;

            hr = Compile(pszSrc, ostrlen(pszSrc), grfscr, &si, pszTitle, &se, &pbody, &funcBody, fUsedExisting, &sourceInfo, &ScriptEngine::CompileUTF16);
            if (SUCCEEDED(hr))
            {
                if (!fUsedExisting)
                {
                    // Mark this function for special profiling registration
                    funcBody->SetIsTopLevel(true);
                }

                if ( SCRIPTPROC_ISXDOMAIN == ( dwFlags & SCRIPTPROC_ISXDOMAIN ))
                {
                    sourceInfo->SetIsXDomain();
                }

                pfuncScript = pbody->CreateEntryPoint(GetScriptSiteHolder());
                entryPointDispatch = JavascriptDispatch::Create<false>(pfuncScript);

                hr = entryPointDispatch->QueryInterface(IID_IDispatchEx, (void**)ppdisp);
                Assert(hr == S_OK);
            }
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr)


            if (FAILED(hr))
            {
                if (fReportError && SCRIPT_E_RECORDED == hr)
                {
                    hr = ReportCompilerError(&si, &se, nullptr, sourceInfo);
                }
            }
    }

LReturn:
    FREEPTR(pszSrc);

    return hr;
}

// *** Private Methods ***

STDMETHODIMP ScriptEngine::Run(void)
{
    HRESULT hr;

    IFFAILRET(ValidateBaseThread());
    if (m_ssState != SCRIPTSTATE_INITIALIZED &&
        m_ssState != SCRIPTSTATE_STARTED)
        return E_UNEXPECTED;

    IfFailedReturn(SinkEvents());

    ChangeScriptState(SCRIPTSTATE_CONNECTED);
    return NOERROR;
}

STDMETHODIMP ScriptEngine::Stop(void)
{
    HRESULT hr;

    IFFAILRET(ValidateBaseThread());
    if (m_ssState != SCRIPTSTATE_CONNECTED &&
        m_ssState != SCRIPTSTATE_DISCONNECTED)
        return E_UNEXPECTED;

    // This can be called in any thread.  We need to wait for all running
    // event handlers to complete, then disconnect from all connection points.
    // The following stuff should be done in the base thread!
    Disconnect();
    ChangeScriptState(SCRIPTSTATE_INITIALIZED);
    return S_OK;
}

STDMETHODIMP ScriptEngine::Disconnect()
{
    HRESULT hr;

    IFFAILRET(ValidateBaseThread());
    if (m_ssState != SCRIPTSTATE_CONNECTED &&
        m_ssState != SCRIPTSTATE_DISCONNECTED)
        return E_UNEXPECTED;

    DisconnectEventHandlers();

    m_fIsPseudoDisconnected = FALSE;
    ChangeScriptState(SCRIPTSTATE_DISCONNECTED);
    return S_OK;
}

STDMETHODIMP ScriptEngine::PseudoDisconnect(void)
{
    HRESULT hr;

    IFFAILRET(ValidateBaseThread());
    if (SCRIPTSTATE_CONNECTED != m_ssState)
        return E_UNEXPECTED;

    m_fIsPseudoDisconnected = TRUE;
    ChangeScriptState(SCRIPTSTATE_DISCONNECTED);
    return S_OK;
}

STDMETHODIMP ScriptEngine::Reconnect(void)
{
    HRESULT hr;

    IFFAILRET(ValidateBaseThread());
    if (m_ssState != SCRIPTSTATE_DISCONNECTED)
        return E_UNEXPECTED;

    // Connect to connection points
    if (m_fIsPseudoDisconnected)
        m_fIsPseudoDisconnected = FALSE;
    else
        ConnectEventHandlers();
    ChangeScriptState(SCRIPTSTATE_CONNECTED);
    return S_OK;
}

HRESULT ScriptEngine::SinkEvents(void)
{
    // Add new named items to the object model
    HRESULT hr;
    IfFailedReturn(SinkEventsOfNamedItems());

    // Connect the script to events
    return ConnectEventHandlers();
}

void ScriptEngine::NotifyScriptStateChange(SCRIPTSTATE ss)
{
    if (m_pActiveScriptSite)
        m_pActiveScriptSite->OnStateChange(ss);
}

void ScriptEngine::ChangeScriptState(SCRIPTSTATE ss)
{
    DisableInterrupts();
    m_ssState   = ss;
    NotifyScriptStateChange(ss);
    EnableInterrupts();
}

void ScriptEngine::OnEnterScript()
{
    if (GetCurrentThreadContextId() != ThreadContextTLSEntry::GetThreadContextId(this->threadContext))
        return;
    Assert(m_pActiveScriptSite != nullptr);
    Assert(GetScriptSiteHolder() != nullptr);

    DisableInterrupts();

    m_cNesting++;
    m_stsThreadState = SCRIPTTHREADSTATE_RUNNING;
    if (1 == m_cNesting)
        FreeExcepInfo(&m_excepinfoInterrupt);

    EnableInterrupts();
    Js::ScriptContext* scriptContext = this->GetScriptSiteHolder()->GetScriptSiteContext();

    // in fastDOM, we can get callback into script, and we are really leaving script here.
#ifdef EXCEPTION_CHECK
    AutoNestedHandledExceptionType autoNestedHandledExceptionType(ExceptionType_HasStackProbe);
#endif
    BEGIN_LEAVE_SCRIPT_NO_STACK_PROBE(scriptContext)
    {
        if (m_pActiveScriptSite)
        {
            m_pActiveScriptSite->OnEnterScript();
        }
    }
    END_LEAVE_SCRIPT(scriptContext);
}

void ScriptEngine::OnLeaveScript(void)
{
    if (GetCurrentThreadContextId() != ThreadContextTLSEntry::GetThreadContextId(this->threadContext))
        return;
    AssertMemN(m_pActiveScriptSite);
    AssertMemN(GetScriptSiteHolder());
    Assert(m_stsThreadState == SCRIPTTHREADSTATE_RUNNING);
    Assert(m_cNesting != 0);
#ifdef EXCEPTION_CHECK
    AutoNestedHandledExceptionType autoNestedHandledExceptionType(ExceptionType_HasStackProbe);
#endif

    // The code was moved to be called before insript flag was cleared in threadcontext. It is called
    // half way in ctor so we should avoid exceptions. should be safe as this is callrootlevel 0
    BEGIN_LEAVE_SCRIPT_NO_STACK_PROBE(scriptContext)
    {
        if (m_pActiveScriptSite)
        {
            m_pActiveScriptSite->OnLeaveScript();
        }
    }
    END_LEAVE_SCRIPT(scriptContext);

    DisableInterrupts();

    m_cNesting--;
    if (m_cNesting == 0)
        m_stsThreadState = SCRIPTTHREADSTATE_NOTINSCRIPT;

    EnableInterrupts();
}

HRESULT ScriptEngine::GetIActiveScriptSite(IActiveScriptSite **ppActiveScriptSite)
{
    *ppActiveScriptSite = m_pActiveScriptSite;
    return NOERROR;
}

HRESULT ScriptEngine::GetActiveScriptSiteWindow(IActiveScriptSiteWindow **ppassw)
{
    Assert(nullptr != m_pActiveScriptSite);
    return m_pActiveScriptSite->QueryInterface(IID_IActiveScriptSiteWindow, (void **)ppassw);
}

HRESULT ScriptEngine::GetActiveScriptSiteUIControl(IActiveScriptSiteUIControl ** ppuic)
{
    Assert(nullptr != m_pActiveScriptSite);
    return m_pActiveScriptSite->QueryInterface(IID_IActiveScriptSiteUIControl, (void **)ppuic);
}

NamedItem *ScriptEngine::FindNamedItem(LPCOLESTR pcszName)
{
    return m_NamedItemList.Find(pcszName);
}

HRESULT ScriptEngine::ConnectEventHandlers(void)
{
    HRESULT hr, hrFail = NOERROR;
    for (int i = 0; i < eventSinks->Count(); i++)
    {
        hr = eventSinks->Item(i)->Connect();
        if (FAILED(hr))
        {
            hrFail = hr;
        }
    }
    return hrFail;
}

HRESULT ScriptEngine::DisconnectEventHandlers(void)
{
    HRESULT hr, hrFail = NOERROR;
    for (int i = 0; i < eventSinks->Count(); i++)
    {
        hr = eventSinks->Item(i)->Disconnect();
        if (FAILED(hr))
        {
            hrFail = hr;
        }
    }
    return hrFail;
}

HRESULT ScriptEngine::AddDefaultDispatch(Js::ModuleID moduleID, IDispatch *pdisp)
{
    if (nullptr == GetScriptSiteHolder())
        return E_UNEXPECTED;
    return GetScriptSiteHolder()->AddDefaultDispatch(moduleID, pdisp);
}

HRESULT ScriptEngine::OnScriptError(IActiveScriptError *pase)
{
    if (nullptr == m_pActiveScriptSite)
    {
        return E_FAIL;
    }
    HRESULT hr;
    Assert(!threadContext->IsScriptActive());
    hr = m_pActiveScriptSite->OnScriptError(pase);
    return hr;
}

HRESULT ScriptEngine::GetObjectOfItem(IDispatch **ppdisp, NamedItem *pnid, LPCOLESTR pszSubItem)
{
    HRESULT     hr;
    IUnknown *  punk;

    CHECK_POINTER(ppdisp);
    *ppdisp = nullptr;
    if (nullptr != pnid->pdisp)
    {
        *ppdisp = pnid->pdisp;
        (*ppdisp)->AddRef();
        goto LHaveBase;
    }
    IFFAILRET(m_pActiveScriptSite->GetItemInfo(pnid->bstrItemName, SCRIPTINFO_IUNKNOWN, &punk, nullptr));
    // Get the object's IDispatch
    hr = punk->QueryInterface(IID_IDispatch, (void **)ppdisp);
    punk->Release();
    IFFAILRET(hr);

    pnid->pdisp = (*ppdisp);
    (*ppdisp)->AddRef();

LHaveBase:

    if (nullptr != pszSubItem)
    {
        AssertMsg(0, "not tested");
        DISPID      id;
        VARIANT     var;
        DISPPARAMS  dp;

        hr = (*ppdisp)->GetIDsOfNames(IID_NULL, (LPOLESTR *)&pszSubItem, 1,
            k_lcidUSEnglish, &id);
        if (FAILED(hr))
        {
            if (nullptr != pnid->bstrItemName &&
                0 == ostrcmp(pszSubItem, pnid->bstrItemName))
            {
                return NOERROR;
            }
            (*ppdisp)->Release();
            *ppdisp = nullptr;
            return hr;
        }

        var.vt = VT_EMPTY;
        memset(&dp, 0, sizeof(dp));
        hr = (*ppdisp)->Invoke(id, IID_NULL, k_lcidUSEnglish, DISPATCH_PROPERTYGET,
            &dp, &var, nullptr, nullptr);
        (*ppdisp)->Release();
        *ppdisp = nullptr;
        IFFAILRET(hr);
        if (var.vt == VT_DISPATCH)
        {
            *ppdisp = var.pdispVal;
            var.pdispVal = nullptr;
        }
        VariantClear(&var);
        if (nullptr == *ppdisp)
            return E_FAIL;
    }

    return NOERROR;
}

HRESULT ScriptEngine::CreateScriptBody(void * pszSrc, size_t len, DWORD dwFlags, bool allowDeferParse, SRCINFO *psi,
                                       const void *pszDelimiter, LPCOLESTR pszTitle,
                                       CoreCompileFunction fnCoreCompile, ComputeGrfscrFunction ComputeGrfscr,
                                       BOOL &fUsedExisting, Js::FunctionBody** ppFuncBody, Js::Utf8SourceInfo** ppSourceInfo, EXCEPINFO *pei, CScriptBody **ppbody)
{
    Assert(pszSrc != nullptr);
    AssertMem(psi);
    AssertPszN(pszDelimiter);
    AssertPszN(pszTitle);
    AssertMemN(ppbody);
    HRESULT hr;
    // Pass the "global code" flag to Generate so it returns the global function as the script body.
    ulong   grfscr  = fscrGlobalCode;
    BOD     bod     = { fbodRun, psi->moduleID, nullptr };

    // Can't have both "ISEXPRESSION" and "ISPERSISTENT"
    if ((dwFlags & (SCRIPTTEXT_ISEXPRESSION | SCRIPTTEXT_ISPERSISTENT)) ==
        (SCRIPTTEXT_ISEXPRESSION | SCRIPTTEXT_ISPERSISTENT))
    {
        return E_INVALIDARG;
    }

    grfscr |= ComputeGrfscr(pszDelimiter);
    if (dwFlags & SCRIPTTEXT_ISPERSISTENT)
        bod.grfbod |= fbodPersist;

    // We want to be able to debug global code. However we do not want to
    // keep around transient code. e.g. Code blocks generated from
    // sombody executing window.execScript in IE. So we compromise and
    // only keep around global code for smart hosts.
    // In cases where we don't want to keep the global code, we set the flag
    // indicating that the function body should be allocated in the recycler so that
    // it can be reclaimed when either Chakra or FastDom ceases to reference it
    // We keep this code around only if the debugger is enabled. If the debugger is not
    // enabled, we really don't have to keep code around unless we're using it (so GC keeps it alive)
    // or it needs to be cloned, in which case it's created with SCRIPTTEXT_ISPERSISTENT
    // If the debugger is enabled, we want to debug global code but not transient code
    if (IsDebuggerEnvironmentAvailable() && ((dwFlags & SCRIPTTEXT_HOSTMANAGESSOURCE) && ((psi->sourceContextInfo->dwHostSourceContext != Js::Constants::NoHostSourceContext))))
    {
        bod.grfbod |= fbodKeep;
    }

    // Flags on the sourceInfo that affect Document hosting
    // Ideally If the source is host managed it will have a valid source context, otherwise debugging will go in a weird state.
    // But there are some cases in the mshtml where we would get host managed source without a valid context.
    // for example dynamically generated code, using document.createElement or document.write
    // Point fixing here to allow to create node to have better debugging experience.
    // If mshtml agrees on passing proper flag, below extra check can be removed.
    if ((dwFlags & SCRIPTTEXT_HOSTMANAGESSOURCE) && (psi->sourceContextInfo->dwHostSourceContext != Js::Constants::NoHostSourceContext))
    {
        psi->grfsi |= fsiHostManaged;
    }
    if (dwFlags & SCRIPTTEXT_ISSCRIPTLET)
    {
        psi->grfsi |= fsiScriptlet;
    }

    if ((dwFlags & (SCRIPTTEXT_ISEXPRESSION | SCRIPTTEXT_FORCEEXECUTION)) ==
        (SCRIPTTEXT_ISEXPRESSION | SCRIPTTEXT_FORCEEXECUTION))
    {
        grfscr |= fscrReturnExpression;
        bod.grfbod |= fbodReturnExpression;
    }

    if (dwFlags & SCRIPTTEXT_ISSCRIPTLET)
    {
        grfscr |= (fscrImplicitThis | fscrImplicitParents);
    }

    ULONG deferParseThreshold = Parser::GetDeferralThreshold(psi->sourceContextInfo->sourceDynamicProfileManager);
    if (allowDeferParse && psi->ichLimHost > deferParseThreshold)
    {
        grfscr |= fscrDeferFncParse;
    }

#pragma prefast(suppress:6237, "The right hand side condition does not have any side effects.")
    if (CONFIG_FLAG(ForceSerialized) && !IsDebuggerEnvironmentAvailable())
    {
        grfscr &= ~fscrDeferFncParse;
    }

    if (dwFlags & SCRIPTTEXT_ISNONUSERCODE)
    {
        grfscr |= fscrIsLibraryCode;
    }

    // Create the code body, passing the name of the global function to the parser
    // to identify the function we want to wrap and return to the caller.
    CompileScriptException se;
    Js::Utf8SourceInfo* sourceInfo = NULL;
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        Js::AutoDynamicCodeReference dynamicFunctionReference(scriptContext);

        hr = Compile( pszSrc, len, grfscr, psi, pszTitle, &se, &bod.pbody, ppFuncBody, fUsedExisting, &sourceInfo, fnCoreCompile);
        SETRETVAL(ppSourceInfo, sourceInfo);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
    if (FAILED(hr))
    {
        if (SCRIPT_E_RECORDED == hr)
        {
            hr = ReportCompilerError(psi, &se, nullptr, sourceInfo);
        }
        return hr;
    }
    else
    {
        if ( SCRIPTTEXT_ISXDOMAIN == ( dwFlags & SCRIPTTEXT_ISXDOMAIN ))
        {
            sourceInfo->SetIsXDomain();
        }
    }

    DisableInterrupts();
    AssertMem(bod.pbody);


    if (nullptr != ppbody)
        *ppbody = bod.pbody;
    else
    {
        if (nullptr == m_pglbod)
        {
            m_pglbod = HeapNewNoThrow(GL,sizeof(BOD));
            IFNULLMEMGO(m_pglbod);
        }
        if(!m_pglbod->FAdd(&bod))
            FAILGO(E_OUTOFMEMORY);
    }

    // Either we've handed back the pbody (through ppbody) or we've queued
    // it up in m_glbod. We need to AddRef here it so that the release below
    // doesn't blow it away.
    bod.pbody->AddRef();

LReturn:
    EnableInterrupts();
    RELEASEPTR(bod.pbody);
    return hr;
}

HRESULT ScriptEngine::ExecutePendingScripts(VARIANT *pvarRes, EXCEPINFO *pei)
{
    HRESULT         hr = NOERROR;
    HRESULT         hrT = S_OK;
    BOD *           pbod;
    long            ibod;
    ulong           grfscr;
    CScriptBody *   pbody;
    Js::DynamicObject *pep = nullptr;

    if (nullptr != pvarRes)
        pvarRes->vt = VT_EMPTY;

    if (nullptr == GetScriptSiteHolder())
        return E_UNEXPECTED;

    if (nullptr == m_pglbod)
        return NOERROR;

    // The execution of script code may cause the host to call back
    // into the engines Close method which would clear scriptSite. In
    // this case, we abort execution.
    AutoCallerPointer callerPointer(GetScriptSiteHolder(), m_pActiveScriptSite);
    for (;;)
    {
        DisableInterrupts();

        // Get the next pbody to execute
        for (ibod = 0; ; ++ibod)
        {
            // We abort if the engine was closed (nullptr == scriptSite). The check
            // must occur before the ibod check because if the engine is
            // closed, m_pglbod will be nullptr as well.
            if (nullptr == GetScriptSiteHolder() || (hrT == E_OUTOFMEMORY) || ibod >= m_pglbod->Cv())
            {
                Assert(!( (hrT == E_OUTOFMEMORY) && (hr != DISP_E_EXCEPTION)));
                EnableInterrupts();
                return hr;
            }
            pbod = (BOD *)m_pglbod->PvGet(ibod);
            if (pbod->grfbod & fbodRun)
                break;
        }
        pbody = pbod->pbody;
        pbody->AddRef();

        // Get the global entry point.
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            pep = pbody->CreateEntryPoint(GetScriptSiteHolder());

            grfscr = (pbod->grfbod & fbodReturnExpression) &&
                (nullptr != pvarRes ? fscrReturnExpression : fscrNil);

            // Erase the flags
            ulong grfbodNewFlags =  pbod->grfbod & ~(fbodRun | fbodReturnExpression);

            if (!grfbodNewFlags)
            {
                pbody->GetRootFunction()->SetIsTopLevel(true);

                pbod->pbody->Release();
                m_pglbod->Delete(ibod);
            }
            pbod->grfbod = grfbodNewFlags;
        }
        END_TRANSLATE_OOM_TO_HRESULT(hrT);

        if (SUCCEEDED(hrT))
        {
            long cb;
            SRCINFO* srcInfo = ((SRCINFO*)(pbody->PvGetData(&cb)));
            BOOL newDefer = (srcInfo->grfsi & fsiDeferredParse) != 0;
            Js::ScriptContext* localScriptContext = scriptContext;
            BOOL fDefer = localScriptContext->SetDeferredBody(newDefer);

            EnableInterrupts();

            // Run the code.
            Js::Arguments arguments(0, nullptr);
            Js::Var varResult;
            hrT = GetScriptSiteHolder()->Execute(pep, &arguments, nullptr, &varResult);
            if (SUCCEEDED(hrT) && pvarRes != nullptr)
            {
                hr = DispatchHelper::MarshalJsVarToVariantNoThrow(varResult, pvarRes, localScriptContext);
            }
            localScriptContext->SetDeferredBody(fDefer);
        }
        else
        {
            EnableInterrupts();
        }
        if (FAILED(hrT) && SUCCEEDED(hr))
        {
            hr = hrT;
            Assert(hr != SCRIPT_E_RECORDED);
            if (SCRIPT_E_REPORTED != hr && pei)
            {
                ActiveScriptError::FillExcepInfo(hr, nullptr, pei);
                hr = DISP_E_EXCEPTION;
            }
        }
        pbody->Release();

        // Prevent looping if we are out of memory.
        if (hrT == E_OUTOFMEMORY && AutoSystemInfo::ShouldQCMoreFrequently())
        {
            return hr;
        }
    }
}

HRESULT ScriptEngine::GetInterruptInfo(EXCEPINFO * pexcepinfo)
{
    AssertMem(pexcepinfo);
    CopyException(pexcepinfo, &m_excepinfoInterrupt);
    return NOERROR;
}

HRESULT ScriptEngine::ChangeType(VARIANT *pvarDst, VARIANT *pvarSrc, LCID lcid, VARTYPE vtNew)
{
    // note: pvarDst can be the same with pvarSrc here
    HRESULT hr = S_OK, hrT = S_OK;
    VARIANT varSrc;
    VariantInit(&varSrc);

    IFFAILRET(ValidateBaseThread());
    if (nullptr == pvarDst || nullptr == pvarSrc)
        return E_POINTER;

    SmartFPUControl smartFpuControl;
    if (smartFpuControl.HasErr())
    {
        return smartFpuControl.GetErr();
    }

    if (nullptr == GetScriptSiteHolder())
        FAILGO(E_UNEXPECTED);

    if (pvarSrc->vt == vtNew)
    {
        hr = VariantCopyInd(pvarDst, pvarSrc);
        goto LReturn;
    }

    // If the source is the dispatch pointer we passed out, we should retrieve
    // the default value given our hint. Otherwise, defer to oleaut32 to do the type coercing.
    if (pvarSrc->vt == VT_DISPATCH)
    {
        IJavascriptDispatchLocalProxy*  jsProxy;
        hr = pvarSrc->pdispVal->QueryInterface(__uuidof(IJavascriptDispatchLocalProxy), (void **)&jsProxy);
        if(SUCCEEDED(hr))
        {

            JavascriptDispatch* javascriptDispatch = static_cast<JavascriptDispatch*>(jsProxy);
            Js::Var varValue;
            Js::JavascriptHint hint;
            hint = (vtNew == VT_BSTR) ? Js::JavascriptHint::HintString : Js::JavascriptHint::HintNumber;

            if (nullptr == javascriptDispatch->GetObject())
            {
                hr = E_ACCESSDENIED;
            }
            else
            {
                hr = GetScriptSiteHolder()->ExternalToPrimitive(javascriptDispatch->GetObject(), hint, &varValue);

                hrT = DispatchHelper::MarshalJsVarToVariantNoThrow(varValue, &varSrc, GetScriptSiteHolder()->GetScriptSiteContext());
                pvarSrc = &varSrc;
            }
            if (FAILED(hrT))
            {
                hr = hrT;
            }
            jsProxy->Release();
        }
        else
        {
            hr = DispatchHelper::GetDispatchValue(GetScriptSiteHolder(), pvarSrc->pdispVal, 0, &varSrc);
            pvarSrc = &varSrc;
        }

        if (pvarSrc->vt == vtNew)
        {
            hr = VariantCopyInd(pvarDst, pvarSrc);
            goto LReturn;
        }
    }


    if (SUCCEEDED(hr))
    {
        // There are too many difference between the default
        // oleaut hander behavior and jscript behavior.
        // Port the old behavior
        hrT = DispatchHelper::JscriptChangeType(pvarSrc, pvarDst, vtNew, scriptContext);
        Assert(FAILED(hrT) || pvarDst->vt == vtNew);
        if(FAILED(hrT))
        {
            VariantClear(pvarDst);
            hr = hrT;
        }
    }

LReturn:
    VariantClear(&varSrc);
    return hr;
}

HRESULT ScriptEngine::GetStat(DWORD stid, ULONG *pluHi, ULONG *pluLo)
{
    HRESULT hr;
    IFFAILRET(ValidateBaseThread());
    if (nullptr == pluHi || nullptr == pluLo)
    {
        return E_POINTER;
    }
    *pluHi = *pluLo = 0;

    switch (stid)
    {
    case SCRIPTSTAT_STATEMENT_COUNT:
        // Get the statement count from the ThreadContext's dedicated poller.
        if (GetScriptSiteHolder() == nullptr)
        {
            break;
        }
        if (threadContext)
        {
            InterruptPoller *poller = threadContext->GetInterruptPoller();
            if (poller)
            {
                poller->GetStatementCount(pluHi, pluLo);
                return NOERROR;
            }
        }
    }

    return E_NOTIMPL;
}

HRESULT ScriptEngine::GetStatEx(REFGUID guid, ULONG *pluHi, ULONG *pluLo)
{
    if (nullptr == pluHi || nullptr == pluLo)
    {
        return E_POINTER;
    }
    *pluHi = *pluLo = 0;
    return E_NOTIMPL;
}

HRESULT ScriptEngine::ResetStats(void)
{
    // Reset the statement count on the ThreadContext's poller.
    HRESULT hr;
    IFFAILRET(ValidateBaseThread());
    if (GetScriptSiteHolder())
    {
        if (threadContext)
        {
            InterruptPoller *poller = threadContext->GetInterruptPoller();
            if (poller)
            {
                poller->ResetStatementCount();
            }
        }
    }
    return NOERROR;
}

HRESULT ScriptEngine::ProfileModeCompile(
    __in void * pszSrc,
    __in size_t len,
    __in ulong grfscr,
    __in SRCINFO *srcInfo,
    __in LPCOLESTR pszTitle,
    __in_opt CompileScriptException *pse,
    __out CScriptBody **ppbody,
    __out Js::FunctionBody** ppFuncBody,
    __out BOOL &fUsedExisting,
    __out Js::Utf8SourceInfo** ppSourceInfo,
    __in CoreCompileFunction fnCoreCompile)
{
    CHECK_POINTER(ppbody);
    *ppbody = nullptr;
    CHECK_POINTER(ppFuncBody);
    *ppFuncBody = nullptr;
    fUsedExisting = false;
    CHECK_POINTER(ppSourceInfo);
    *ppSourceInfo = nullptr;

    // When we're profiling, might as well deserialize everything
    grfscr = (grfscr & (~fscrAllowFunctionProxy));

    HRESULT hr = DefaultCompile(pszSrc, len, grfscr, srcInfo, pszTitle, pse, ppbody, ppFuncBody, fUsedExisting, ppSourceInfo, fnCoreCompile);

    if (SUCCEEDED(hr) && !fUsedExisting)
    {
        if (webWorkerID != Js::Constants::NonWebWorkerContextId)
        {
            // Tells the host to register the webworker source to the PDM. The call to OnEnterScript will make call to the PDM's AddUnicodeText.
            // This is to ensure that source is registered before emitting script/function compiled event to the profiler.
            if (m_pActiveScriptSite)
            {
                m_pActiveScriptSite->OnEnterScript();
            }
        }

        this->scriptContext->RegisterScript((*ppbody)->GetRootFunction());
    }

    return hr;
}

HRESULT ScriptEngine::DefaultCompile(
    __in void * pszSrc,
    __in size_t len,
    __in ulong grfscr,
    __in SRCINFO *srcInfo,
    __in LPCOLESTR pszTitle,
    __in_opt CompileScriptException *pse,
    __out CScriptBody **ppbody,
    __out Js::FunctionBody** ppFuncBody,
    __out BOOL &fUsedExisting,
    __out Js::Utf8SourceInfo** ppSourceInfo,
    __in CoreCompileFunction fnCoreCompile)
{
    return (this->*fnCoreCompile)(pszSrc,len, grfscr, srcInfo, pszTitle, pse, ppbody, ppFuncBody, fUsedExisting, ppSourceInfo);
}

HRESULT ScriptEngine::CompileByteCodeBuffer(
    __in void * bytesAndSourceAndModule,
    __in size_t cbLength,
    __in ulong grfscr,
    __in SRCINFO *srcInfo,
    __in LPCOLESTR pszTitle,
    __in_opt CompileScriptException *pse,
    __out CScriptBody **ppbody,
    __out Js::FunctionBody** ppFuncBody,
    __out BOOL &fUsedExisting,
    __out Js::Utf8SourceInfo** pSourceInfo)
{
    Assert(!IsDebuggerEnvironmentAvailable());
    fUsedExisting = FALSE;
    Js::FunctionBody *rootFunction;
    HRESULT hr = S_OK;
    auto unpack = (BYTE**)bytesAndSourceAndModule;
    auto byteCode = unpack[0];

    auto sourceMapper = (IActiveScriptByteCodeSource *)unpack[1];   
    Js::ISourceHolder* sourceHolder = (Js::DynamicSourceHolder *)RecyclerNewFinalized(scriptContext->GetRecycler(), Js::DynamicSourceHolder, sourceMapper);
    Js::NativeModule * nativeModule = nullptr;
#ifdef ENABLE_NATIVE_CODE_SERIALIZATION
    nativeModule = (Js::NativeModule *)unpack[2];
    if (nativeModule && IS_JS_ETW(EventEnabledJSCRIPT_NATIVE_MODULE_LOAD()))
    {
        HCRYPTPROV hProv = NULL;
        HCRYPTHASH hHash = NULL;
        const DWORD hashSize = 20;
        DWORD cbHash = hashSize;
        BYTE pHash[hashSize];

        ZeroMemory(pHash, hashSize);
        hProv = this->threadContext->EnsureCryptoContext();
        if (hProv &&
            CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash) &&
            CryptHashData(hHash, sourceHolder->GetSource(L"ScriptEngine::CompileByteCodeBuffer - CryptHashData"), sourceHolder->GetByteLength(L"ScriptEngine::CompileByteCodeBuffer - CryptHashData"), 0) &&
            CryptGetHashParam(hHash, HP_HASHVAL, pHash, &cbHash, 0))
        {
            // Succeeded
        }

        if (hHash != NULL)
        {
            CryptDestroyHash(hHash);
        }
        JS_ETW(EventWriteJSCRIPT_NATIVE_MODULE_LOAD(nativeModule->base, pHash, srcInfo->sourceContextInfo->url, nativeModule->codeSize));
    }
#endif
    hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, grfscr, sourceHolder, scriptContext->AddHostSrcInfo(srcInfo), (byte*) byteCode, nativeModule, &rootFunction);

    if (FAILED(hr))
    {
        return hr;
    }

    // CompileByteCodeBuffer is called by ParseScriptText which handles OOM
    *ppbody = HeapNew(CScriptBody, rootFunction, this, rootFunction->GetUtf8SourceInfo());
    *ppFuncBody = rootFunction;
    return hr;
}

HRESULT ScriptEngine::CompileUTF16(
    __in void * pSrc,
    __in size_t cbLength,
    __in ulong grfscr,
    __in SRCINFO *srcInfo,
    __in LPCOLESTR pszTitle,
    __in_opt CompileScriptException *pse,
    __out CScriptBody **ppbody,
    __out Js::FunctionBody** ppFuncBody,
    __out BOOL &fUsedExisting,
    __out Js::Utf8SourceInfo** ppSourceInfo)
{
    HRESULT hr = NOERROR;
    LPCOLESTR pszSrc = (LPCOLESTR) pSrc;

    Assert(this->scriptContext == GetScriptSiteHolder()->GetScriptSiteContext());

    // We count the characters because the length received in srcInfo is unreliable.
    charcount_t stringLength = static_cast< charcount_t>(cbLength);

#if _WIN32 || _WIN64
    if (grfscr & fscrHtmlComments && !(grfscr & fscrDoNotHandleTrailingHtmlComments) && stringLength > 0)
    {
        stringLength = GetLengthExcludingHTMLCommentSuffix(pszSrc, stringLength, this->scriptContext);
    }
#else
#error Neither _WIN16, _WIN32, nor _WIN64 is defined
#endif

    // Convert the LPOLESTR buffer to a LPUTF8
    // Allocate we need at most 3 bytes for each wchar and a null terminator

    LPUTF8 pchUtf8Code = HeapNewNoThrowArray(utf8char_t, stringLength * 3 + 1 );

    if (nullptr == pchUtf8Code)
    {
        return E_OUTOFMEMORY;
    }

    AutoArrayPtr<utf8char_t> autoFreeSourceCode(pchUtf8Code, stringLength * 3 + 1);

    cbLength = utf8::EncodeIntoAndNullTerminate(pchUtf8Code, pszSrc, stringLength);

#if DBG_DUMP
    if (Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::ParsePhase) && Js::Configuration::Global.flags.Verbose)
    {
        Output::Print(L"Default Compile\n"
            L"  Tile:                   %s\n"
            L"  Unicode size (in bytes) %u\n"
            L"  UTF-8 size (in bytes)   %u\n"
            L"  Expected savings        %d\n", pszTitle != nullptr ? pszTitle : L"", stringLength * sizeof(wchar_t), cbLength, stringLength * sizeof(wchar_t) - cbLength);
    }
#endif

    // Reallocate the buffer to a smaller amount since it is probably 3 times larger than we needed.
    // Don't need to handle OOM here since this is currently always called from a try-catch block

    ENTER_PINNED_SCOPE(Js::Utf8SourceInfo, sourceInfo);
    sourceInfo = Js::Utf8SourceInfo::New(scriptContext, pchUtf8Code, stringLength, cbLength, srcInfo);
    if ((grfscr & fscrIsLibraryCode) != 0)
    {
        sourceInfo->SetIsLibraryCode();
    }

    Assert(utf8::CharsAreEqual(pszSrc, pchUtf8Code, stringLength, utf8::doAllowThreeByteSurrogates));

    // Compile the UTF8 source
    SETRETVAL(ppSourceInfo, sourceInfo);
    hr = CompileUTF8Core(sourceInfo, stringLength, grfscr, srcInfo, pszTitle, false, pse, ppbody, ppFuncBody, fUsedExisting);

    LEAVE_PINNED_SCOPE();

    return hr;
}

HRESULT ScriptEngine::CompileUTF8(
    __in void * pSrc,
    __in size_t cbLength,
    __in ulong grfscr,
    __in SRCINFO *srcInfo,
    __in LPCOLESTR pszTitle,
    __in_opt CompileScriptException *pse,
    __out CScriptBody **ppbody,
    __out Js::FunctionBody** ppFuncBody,
    __out BOOL &fUsedExisting,
    __out Js::Utf8SourceInfo** ppSourceInfo)
{
    // TODO : calculate cbLength? how
    LPCUTF8 pszSrc = (LPCUTF8) pSrc;
    HRESULT hr = ERROR_SUCCESS;

    Assert(this->scriptContext == GetScriptSiteHolder()->GetScriptSiteContext());

    // We count the characters because the length received in srcInfo is unreliable.
    charcount_t stringLength = static_cast< charcount_t>(cbLength);


#if _WIN32 || _WIN64
    if (grfscr & fscrHtmlComments && !(grfscr & fscrDoNotHandleTrailingHtmlComments) && stringLength > 0)
    {
        stringLength = GetLengthExcludingHTMLCommentSuffix(pszSrc, stringLength, this->scriptContext);
    }
#else
#error Neither _WIN16, _WIN32, nor _WIN64 is defined
#endif

    // Currently always called from a try-catch
    ENTER_PINNED_SCOPE(Js::Utf8SourceInfo, sourceInfo);

    sourceInfo = Js::Utf8SourceInfo::NewWithNoCopy(scriptContext, (LPUTF8) pSrc, stringLength, stringLength, srcInfo);
    if ((grfscr & fscrIsLibraryCode) != 0)
    {
        sourceInfo->SetIsLibraryCode();
    }
    SETRETVAL(ppSourceInfo, sourceInfo);

    hr = CompileUTF8Core(sourceInfo, stringLength, grfscr, srcInfo, pszTitle, true, pse, ppbody, ppFuncBody, fUsedExisting);

    LEAVE_PINNED_SCOPE();

    return hr;
}

HRESULT ScriptEngine::CompileUTF8Core(
    __in Js::Utf8SourceInfo* utf8SourceInfo,
    __in charcount_t cchLength,
    __in ulong grfscr,
    __in SRCINFO *srcInfo,
    __in LPCOLESTR pszTitle,
    __in BOOL fOriginalUTF8Code,
    __in_opt CompileScriptException *pse,
    __out CScriptBody **ppbody,
    __out Js::FunctionBody** ppFuncBody,
    __out BOOL &fUsedExisting)
{

    HRESULT hr = S_OK;
    Assert(this->scriptContext == GetScriptSiteHolder()->GetScriptSiteContext());

    // Normally we should transition to debug mode through attach call but if debugging is allowed in IE settings 
    // or page is refreshed when F12 is opened attach won't be called and we will directly go into debug mode.
    // EnsureScriptContext should take care of it but there are few cases when host is not in debug mode when we call EnsureScriptContext
    // so we will only transition to debug mode when first source is compiled. m_isFirstSourceCompile takes care of it
    hr = this->TransitionToDebugModeIfFirstSource(utf8SourceInfo);

    if (grfscr & fscrImplicitThis)
    {
        if (scriptBodyMap == nullptr)
        {
            scriptBodyMap = RecyclerNew(scriptContext->GetRecycler(), ScriptBodyDictionary, scriptContext->GetRecycler(), 2);

            // TODO: Unbind the map when the script context is destroyed
            this->scriptContext->BindReference(scriptBodyMap);
        }

        if (scriptBodyMap->TryGetValue(utf8SourceInfo, ppbody))
        {
            long srcLength;
            // only match when SRCINFO is consistent as well. In real life I don't expect
            // repeated Parseproceduretext coming from different moduleRoot.
            void* cachedSrcInfo = (*ppbody)->PvGetData(&srcLength);
            if (srcLength == sizeof(SRCINFO) && (memcmp(srcInfo, cachedSrcInfo, sizeof(SRCINFO)) ==0))
            {
                (*ppbody)->AddRef();
                (*ppFuncBody) = (*ppbody)->GetRootFunction();

                fUsedExisting = TRUE;
                return NOERROR;
            }

            //  Make sure we nullptr out this value, ParseProdureTextCore will call release whether
            // we return successful HR or not.
            *ppbody = nullptr;
        }

    }

    fUsedExisting = FALSE;

    Js::FunctionBody* pRootFunc = nullptr;

    uint sourceIndex = 0;
    LPCUTF8 pszSrc = utf8SourceInfo->GetSource(L"ScriptEngine::CompileUTF8Core");
    size_t cbLength = utf8SourceInfo->GetCbLength(L"ScriptEngine::CompileUTF8Core");
    // BLOCK
    {
        Parser ps(this->scriptContext);
        Js::ParseableFunctionInfo * func = nullptr;

        if (SUCCEEDED(hr))
        {
            ParseNodePtr parseTree;
            SourceContextInfo * sourceContextInfo = srcInfo->sourceContextInfo;
            if (fOriginalUTF8Code)
            {
                hr = ps.ParseUtf8Source(&parseTree, pszSrc, cbLength, grfscr, pse, &sourceContextInfo->nextLocalFunctionId,
                    sourceContextInfo);
                cchLength = ps.GetSourceIchLim();

                // Correcting total number of characters.
                utf8SourceInfo->SetCchLength(cchLength);
            }
            else
            {
                hr = ps.ParseCesu8Source(&parseTree, pszSrc, cbLength, grfscr, pse, &sourceContextInfo->nextLocalFunctionId,
                    sourceContextInfo);
            }
            utf8SourceInfo->SetParseFlags(grfscr);

            if (SUCCEEDED(hr))
            {
                bool isCesu8 = !fOriginalUTF8Code;
                sourceIndex = scriptContext->SaveSourceNoCopy(utf8SourceInfo, cchLength, isCesu8);
                hr = GenerateByteCode(parseTree, grfscr, scriptContext, &func, sourceIndex, scriptContext->IsForceNoNative(), &ps, pse);
                utf8SourceInfo->SetByteCodeGenerationFlags(grfscr);
            }
            else
            {
                // In case of syntax error, if we are in debug mode, put the utf8SourceInfo into debug mode.
                if (scriptContext->IsInDebugMode())
                {
                    if (!utf8SourceInfo->IsInDebugMode())
                    {
                        utf8SourceInfo->SetInDebugMode(true);
                    }
                }
            }
        }

        if (FAILED(hr))
        {
            // parsing failed
            Assert(pRootFunc == nullptr);
            if (SCRIPT_E_RECORDED == hr)
            {
                if (SUCCEEDED(HR(pse->ei.scode)))
                    pse->ei.scode = E_FAIL;
            }
            if (scriptContext->IsInDebugOrSourceRundownMode())
            {
                // Register the document with PDM to ensure critical errors like syntax errors are correctly reproted
                // in the debugger
                CScriptBody* scriptBody = HeapNewNoThrow(CScriptBody, /*functionBody*/ nullptr, this, utf8SourceInfo);
                if(scriptBody)
                {                    
                    DbgRegisterScriptBlock(scriptBody);       
                    // Always need to release the ref count of script body.  If DbgRegisterScriptBlock is successful, it will do its own AddRef
                    RELEASEPTR(scriptBody);
                }
            }

            return hr;
        }
        pRootFunc = func->GetFunctionBody();
        // Mark the particular source buffer to have deferred functions using a static deferral threshold and not one based on profile.
        if (ps.GetSourceLength() > Parser::GetDeferralThreshold(/*profileManager=*/ nullptr) )
        {
            srcInfo->grfsi |= fsiDeferredParse;
        }
    }

    if (!IsDebuggerEnvironmentAvailable() && CONFIG_FLAG(ForceSerialized))
    {
        auto srcInfo = pRootFunc->GetHostSrcInfo();
        if (srcInfo->moduleID == kmodGlobal)
        {
            byte * byteCode; // Note. DEBUG-Only, this buffer gets leaked. The current byte code cache gaurantee is that the buffer lives as long as the process.
            DWORD dwByteCodeSize;
            byte * nativeCode = nullptr;
            DWORD dwNativeCodeSize;
            Js::FunctionBody* deserializedFunction = nullptr;

            OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Forcing serialization.\n");
            BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"ByteCodeSerializer");
            hr = Js::ByteCodeSerializer::SerializeToBuffer(scriptContext, tempAllocator, cbLength, pszSrc, 0, nullptr, pRootFunc, srcInfo, true, &byteCode, &dwByteCodeSize);

            if (SUCCEEDED(hr))
            {
                ulong flags = 0;
                if (CONFIG_FLAG(IncludeNativeCodeWithSerializedByteCodes))
                {
                    flags = fscrIsNativeCode;
                }
                else if (CONFIG_FLAG(CreateFunctionProxy))
                {
                    flags = fscrAllowFunctionProxy;
                }

                srcInfo->sourceContextInfo->nextLocalFunctionId = pRootFunc->GetLocalFunctionId();
                hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, flags, pszSrc, srcInfo, byteCode, nullptr, &deserializedFunction, sourceIndex);

                if (SUCCEEDED(hr))
                {
                    if (CONFIG_FLAG(IncludeNativeCodeWithSerializedByteCodes))
                    {
                        Js::NativeModule *nativeModule;

                        OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Forcing native code serialization.\n");

                        GenerateAllFunctionsForSerialization(scriptContext->GetNativeCodeGenerator(), deserializedFunction, nullptr, 0, byteCode, dwByteCodeSize, 0, nullptr, &nativeCode, &dwNativeCodeSize);
                        // If sourceProvider is null, which it may be, this won't do anything.
                        // The purpose of this is to ensure we have at least added one ref count, and removed the one we added at the end of this method
                        AutoCOMPtr<IActiveScriptByteCodeSource> empty(nullptr);
                        hr = LoadInMemoryBuffer(nativeCode, empty, nullptr, nullptr, &nativeModule, true);

                        if (SUCCEEDED(hr))
                        {
                            flags = 0;

                            if (CONFIG_FLAG(CreateFunctionProxy))
                            {
                                flags = fscrAllowFunctionProxy;
                            }

                            srcInfo->sourceContextInfo->nextLocalFunctionId = pRootFunc->GetLocalFunctionId();
                            hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, flags, pszSrc, srcInfo, byteCode, nativeModule, &deserializedFunction, sourceIndex);

                            if (SUCCEEDED(hr))
                            {
                                OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Native serialization succeeded.\n");
                                pRootFunc = deserializedFunction;
                            }
                            else
                            {
                                OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Native serialization failed.\n");
                            }
                        }
                        else
                        {
                            OUTPUT_TRACE(Js::NativeCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Native serialization failed.\n");
                        }
                    }
                    else
                    {
                        OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Serialization succeeded.\n");
                        pRootFunc = deserializedFunction;
                    }
                }
                else
                {
                    OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Serialization failed.\n");
                }
            }
            else
            {
                OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Serialization failed.\n");
            }
            END_TEMP_ALLOCATOR(tempAllocator, scriptContext);
            OUTPUT_TRACE(Js::ByteCodeSerializationPhase, L"ScriptEngine::CompileUTF8Core: Forced serialization done.\n");
        }
    }

    // Set name hint first since this can throw, so we'd want to do this before the CScriptBody
    // is created since otherwise we'll leak the script body and with it the function etc.
    if (nullptr != pszTitle)
    {
        pRootFunc->SetDisplayName(pszTitle);
    }

    *ppFuncBody = pRootFunc;
    *ppbody = nullptr;

    // CompileUtf8Core is called by callers who wrap it in BEGIN_TRANSLATE_EXCEPTION_*
    // so we don't need to handle the exception here, we'll just have pbody be nullptr and
    // throw the error
    *ppbody = HeapNew(CScriptBody, pRootFunc, this, utf8SourceInfo);

    // Register it.
    if (scriptContext->IsInDebugOrSourceRundownMode())
    {
        if (FAILED(hr = DbgRegisterScriptBlock(*ppbody)))
        {
            RELEASEPTR(*ppbody);
            return hr;
        }
    }

    if ((grfscr & fscrImplicitThis) != 0)
    {
        Js::Utf8SourceInfo *sourceInfo = this->scriptContext->GetSource(sourceIndex);
        Assert(sourceInfo->GetCbLength(L"ScriptEngine::CompileUTF8Core") == cbLength);
        CScriptBody* oldBody;
        if (scriptBodyMap->TryGetValueAndRemove(utf8SourceInfo, &oldBody))
        {
            oldBody->Release();
        }

        scriptBodyMap->Add(utf8SourceInfo, *ppbody);
        (*ppbody)->AddRef();
    }

    return NOERROR;
}

SourceContextInfo * ScriptEngine::GetSourceContextInfo( DWORD_PTR hostSourceContext, uint hash, BOOL isDynamicDocument, BSTR sourceMapUrl, IActiveScriptDataCache* profileDataCache)
{
    SourceContextInfo * sourceContextInfo;
    if(!CONFIG_ISENABLED(Js::NoDynamicProfileInMemoryCacheFlag))
    {
        if(hostSourceContext == Js::Constants::NoHostSourceContext || isDynamicDocument)
        {
            return scriptContext->CreateSourceContextInfo(hash, hostSourceContext);
        }
    }

    sourceContextInfo = scriptContext->GetSourceContextInfo(hostSourceContext, profileDataCache);

    if (sourceContextInfo != nullptr)
    {
        return sourceContextInfo;
    }

    // The script context should have create a default SourceContextInfo for NoHostSourceContext
    Assert(hostSourceContext != Js::Constants::NoHostSourceContext);
    CComBSTR bstrUrl;

    GetHostContextUrl(hostSourceContext, &bstrUrl);
    return scriptContext->CreateSourceContextInfo(hostSourceContext, bstrUrl, SysStringLen(bstrUrl), profileDataCache, sourceMapUrl, SysStringLen(sourceMapUrl));
}

HRESULT ScriptEngine::GetHostContextUrl(__in DWORD_PTR hostSourceContext, __out BSTR* pUrl)
{
    AssertMsg(hostSourceContext != Js::Constants::NoHostSourceContext, "There is no URL available for dynamic scripts");

    HRESULT hr = S_OK;
    CHECK_POINTER(pUrl);
    *pUrl = nullptr;

    IOleCommandTarget *pOleCommandTarget;
    hr = m_pActiveScriptSite->QueryInterface(&pOleCommandTarget);
    if (FAILED(hr))
    {
        return hr;
    }

    VARIANT varIn;
    varIn.vt = VT_UI4 | VT_BYREF;
    varIn.pulVal = (ULONG *)hostSourceContext;

    const GUID cgidScriptSite = CGID_ScriptSite;
    VARIANT varOut = {};
    hr = pOleCommandTarget->Exec(&cgidScriptSite, CMDID_HOSTCONTEXT_URL, 0, &varIn, &varOut);
    pOleCommandTarget->Release();

    if(SUCCEEDED(hr))
    {
        *pUrl = varOut.bstrVal;
    }
    return hr;
}

HRESULT ScriptEngine::GetUrl(__out BSTR* pUrl)
{
    HRESULT hr = S_OK;
    *pUrl = nullptr;

    IOleCommandTarget *pOleCommandTarget;
    hr = m_pActiveScriptSite->QueryInterface(&pOleCommandTarget);
    if (FAILED(hr))
    {
        return hr;
    }

    const GUID cgidScriptSite = CGID_ScriptSite;
    VARIANT varOut = {};
    hr = pOleCommandTarget->Exec(&cgidScriptSite, CMDID_SCRIPTSITE_URL, 0, nullptr, &varOut);
    pOleCommandTarget->Release();

    if(SUCCEEDED(hr))
    {
        *pUrl = varOut.bstrVal;
    }
    return hr;
}

void ScriptEngine::CleanScriptBodyMap()
{
    if (nullptr != scriptBodyMap)
    {
        scriptBodyMap->Clean([](Js::Utf8SourceInfo * info, CScriptBody * scriptBody)
        {
            scriptBody->Release();            
        });
    }
}


void ScriptEngine::RemoveScriptBodyMap()
{
    if (nullptr != scriptBodyMap)
    {
        scriptBodyMap->CleanAll([](Js::Utf8SourceInfo * info, CScriptBody * scriptBody)
        {
            scriptBody->Release();
        });

        scriptBodyMap = nullptr;
    }
}

#ifdef ENABLE_PROJECTION
HRESULT ScriptEngine::EnsureProjection()
{
    if (projectionContext != nullptr)
    {
        return S_OK;
    }

    if (IsInClosedState())
    {
        return E_ACCESSDENIED;
    }

    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    AutoPtr<ProjectionContext> newProjectionContext(HeapNewNoThrow(ProjectionContext, GetScriptSiteHolder(), threadContext));
    if (newProjectionContext == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = newProjectionContext->Initialize();
    if (SUCCEEDED(hr))
    {
        projectionContext = newProjectionContext.Detach();
    }

    return hr;
}

void ScriptEngine::ResetProjectionContext()
{
    if (projectionContext)
    {
        projectionContext->ClearCaches();
    }
}
#endif

HRESULT ScriptEngine::GetApplicationThread(IDebugApplicationThread** ppAppThread)
{
    AssertMsg(GetCurrentThreadId() == m_dwBaseThread, "GetApplicationThread() called on wrong thread");
    if (!ppAppThread)
    {
        return E_INVALIDARG;
    }

    *ppAppThread = m_debugApplicationThread;

    if(*ppAppThread)
    {
        (*ppAppThread)->AddRef();
        return S_OK;
    }

    return E_FAIL;
}

HRESULT ScriptEngine::GetBitCorrectApplicationThread(IDebugBitCorrectApplicationThread** ppAppThread)
{
    IfNullReturnError(ppAppThread, E_POINTER);

    // TODO: Handle thread conflicts accessing this field.
    if (m_debugApplicationThread)
    {
        return m_debugApplicationThread->QueryInterface(ppAppThread);
    }

    *ppAppThread = nullptr;
    return E_FAIL;
}

// === IObjectSafety ===
STDMETHODIMP ScriptEngine::GetInterfaceSafetyOptions(REFIID riid,
                                                     DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
    if (nullptr == pdwSupportedOptions || nullptr == pdwEnabledOptions)
        return E_POINTER;
    *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA |
        INTERFACE_USES_DISPEX;
#if _WIN32 || _WIN64
    *pdwSupportedOptions |= INTERFACE_USES_SECURITY_MANAGER;
#endif // _WIN32 || _WIN64
    *pdwEnabledOptions = m_dwSafetyOptions;
    return S_OK;
}

STDMETHODIMP ScriptEngine::SetInterfaceSafetyOptions(REFIID riid,
                                                     DWORD dwOptionsSetMask, DWORD dwEnabledOptions)
{
    // Attempt to set the interface setting options on this object.
    // Since these are assumed to be fixed, we basically just check
    // that the attempted settings are valid.
    if (m_ssState != SCRIPTSTATE_UNINITIALIZED &&
        m_ssState != SCRIPTSTATE_INITIALIZED ||
        (dwOptionsSetMask & ~(INTERFACESAFE_FOR_UNTRUSTED_DATA |
        INTERFACE_USES_DISPEX  |
        INTERFACE_USES_SECURITY_MANAGER)))
    {
        return E_FAIL;
    }

    dwEnabledOptions |= INTERFACE_USES_DISPEX;
    m_dwSafetyOptions &= ~dwOptionsSetMask;
    m_dwSafetyOptions |= dwOptionsSetMask & dwEnabledOptions;
    return S_OK;
}

// === IActiveScriptProperty ===

// See SetProperty for more comments.
STDMETHODIMP ScriptEngine::GetProperty(DWORD dwProperty, VARIANT *pvarIndex, VARIANT *pvarValue)
{
    IfNullReturnError(pvarValue, E_INVALIDARG);
    pvarValue->vt = VT_EMPTY;

    long lVersionInfo;

    switch (dwProperty)
    {
        // locale conversion is used in vb, but not in jscript.
    case SCRIPTPROP_CONVERSIONLCID:
        return E_NOTIMPL;

    case SCRIPTPROP_NAME:
        if (nullptr != pvarIndex)
            return E_INVALIDARG;
        pvarValue->bstrVal = SysAllocString(m_pszLanguageName);
        IFNULLMEMRET(pvarValue->bstrVal);
        pvarValue->vt = VT_BSTR;
        return NOERROR;

    case SCRIPTPROP_MAJORVERSION: lVersionInfo = SCRIPT_ENGINE_MAJOR_VERSION; goto LVersionInfo;
    case SCRIPTPROP_MINORVERSION: lVersionInfo = SCRIPT_ENGINE_MINOR_VERSION; goto LVersionInfo;
    case SCRIPTPROP_BUILDNUMBER : lVersionInfo = SCRIPT_ENGINE_BUILDNUMBER; goto LVersionInfo;
LVersionInfo:
        if (nullptr != pvarIndex)
            return E_INVALIDARG;
        pvarValue->vt   = VT_I4;
        pvarValue->lVal = lVersionInfo;
        return NOERROR;

    case SCRIPTPROP_HOSTSTACKREQUIRED:
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;
        if (nullptr != pvarIndex)
            return E_INVALIDARG;
        pvarValue->vt   = VT_I4;
        pvarValue->lVal = m_cbMinStackHost;
        return NOERROR;

    case SCRIPTPROP_INTEGERMODE:
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;

    case SCRIPTPROP_STRINGCOMPAREINSTANCE:
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;

    case SCRIPTPROP_CATCHEXCEPTION:
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;

    case SCRIPTPROP_HACK_FIBERSUPPORT:
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;

        // TODO (yongqu): this is optimization to reduce lookup cost in scoped lookup
        // where we know the obj is part of the global binder (like windows).
        // we only do scoped lookup in event handler, where we need to do the lookup
        // chain anyway. I don't think we need this feature now.
    case SCRIPTPROP_ABBREVIATE_GLOBALNAME_RESOLUTION:
        pvarValue->vt = VT_BOOL;
        pvarValue->boolVal = fCanOptimizeGlobalLookup ? VARIANT_TRUE : VARIANT_FALSE;
        return NOERROR;

    case SCRIPTPROP_INVOKEVERSIONING:
        if (nullptr != pvarIndex)
            return E_INVALIDARG;

        // Return the unclamped version which is stored in scriptVersion
        pvarValue->vt   = VT_I4;
        pvarValue->lVal = SCRIPTLANGUAGEVERSION_5_12;
        return NOERROR;

        // Get the host type: Browser(IE)/Application(WWA).
    case SCRIPTPROP_HOSTTYPE:
        if (nullptr != pvarIndex)
        {
            return E_INVALIDARG;
        }

        pvarValue->vt   = VT_I4;
        pvarValue->lVal = this->hostType;
        return NOERROR;

    case SCRIPTPROP_NONPRIMARYENGINE:
        if (nullptr != pvarIndex)
        {
            return E_INVALIDARG;
        }
        pvarValue->vt = VT_I4;
        pvarValue->lVal = (fNonPrimaryEngine != FALSE) ? 1 : 0;
        return NOERROR;

    case SCRIPTPROP_OPTIMIZE_FOR_MANY_INSTANCES:
        {
            if (nullptr != pvarIndex)
            {
                return E_INVALIDARG;
            }
            pvarValue->vt = VT_BOOL;
            ThreadContext *const threadContext = ThreadContext::GetContextForCurrentThread();
            Assert(threadContext); // script engine should be initialized before this
            pvarValue->boolVal = threadContext->IsOptimizedForManyInstances() ? VARIANT_TRUE : VARIANT_FALSE;
            return NOERROR;
        }

    case SCRIPTPROP_WEBWORKERID:
        {
            pvarValue->vt = VT_UI4;
            pvarValue->lVal = webWorkerID;
            return NOERROR;
        }
    case SCRIPTPROP_EMIE:
        {
            if (nullptr != pvarIndex)
            {
                return E_INVALIDARG;
            }
            pvarValue->vt = VT_BOOL;
            pvarValue->boolVal = VARIANT_FALSE;
            return NOERROR;
        }

    case SCRIPTPROP_DIAGNOSTICS_OM:
        AssertMsg(FALSE, "not expected");
        return E_NOTIMPL;

    case SCRIPTPROP_EVAL_RESTRICTION:
        {
            pvarValue->vt = VT_BOOL;
            pvarValue->boolVal = this->m_fIsEvalRestrict ? VARIANT_TRUE : VARIANT_FALSE;
            return NOERROR;
        }
    
    case SCRIPTPROP_ALLOW_WINRT_CONSTRUCTOR:
        {
            pvarValue->vt = VT_BOOL;
            pvarValue->boolVal = this->m_fAllowWinRTConstructor ? VARIANT_TRUE : VARIANT_FALSE;
            return NOERROR;
        }

    default:
        Assert(FALSE);
        break;
    }
    return E_NOTIMPL;
}

STDMETHODIMP ScriptEngine::SetProperty(DWORD dwProperty, VARIANT *pvarIndex, VARIANT *pvarValue)
{
    // we don't expect to have value for SCRIPTPROP_DIAGNOSTICS_OM
    if (nullptr == pvarValue && dwProperty != SCRIPTPROP_DIAGNOSTICS_OM)
        return E_INVALIDARG;
    switch (dwProperty)
    {
        // readonly props
    case SCRIPTPROP_NAME:
    case SCRIPTPROP_MAJORVERSION:
    case SCRIPTPROP_MINORVERSION:
    case SCRIPTPROP_BUILDNUMBER:
        return E_ACCESSDENIED;

        // TODO: (yongqu): This is a security feature in old engine where we will
        // make sure there is enough stack space as specified here before
        // calling into error handling code. (IActiveScriptErrorDebug*).
        // Maybe we still need this?
    case SCRIPTPROP_HOSTSTACKREQUIRED:
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;
        if (nullptr != pvarIndex)
            return E_INVALIDARG;
        if (VT_I4 != pvarValue->vt)
            return E_INVALIDARG;
        m_cbMinStackHost = (UINT)pvarValue->lVal;
        if (m_cbMinStackHost < Js::Constants::MinStackHost)
            m_cbMinStackHost = Js::Constants::MinStackHost;
        return NOERROR;

        // locale conversion is used in vb, but not in jscript.
    case SCRIPTPROP_CONVERSIONLCID:
        return E_NOTIMPL;

        // Safe devide functionality. When set, we'll do long division
        // and return NaN if divedend is 0 (default is long division).
        // not used in windows.
    case SCRIPTPROP_INTEGERMODE:
        // This flag can only be set if the engine is uninitialized.
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;

        // Override the BSTR comparison route. It is enabled in old engine,
        // but I don't see usage in the windows tree.
    case SCRIPTPROP_STRINGCOMPAREINSTANCE:
        // This flag can only be set if the engine is uninitialized.
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;
        if (SCRIPTSTATE_UNINITIALIZED != m_ssState)
            return E_UNEXPECTED;
        if (VT_UNKNOWN != pvarValue->vt)
            return E_INVALIDARG;
#if 0
        IActiveScriptStringCompare* pActiveScriptStrComp;
        pActiveScriptStrComp = nullptr;
        if (pvarValue->punkVal)
        {
            HRESULT hr = pvarValue->punkVal->QueryInterface(IID_IActiveScriptStringCompare, (void **)&pActiveScriptStrComp);
            if (FAILED(hr) || pActiveScriptStrComp == nullptr)
                return E_INVALIDARG;
        }
        if (nullptr != m_pActiveScriptStrComp)
        {
            m_pActiveScriptStrComp->Release();
            m_pActiveScriptStrComp = nullptr;
        }
        m_pActiveScriptStrComp = pActiveScriptStrComp;
#endif
        return NOERROR;

        // IIS is using this to catch a bunch of exceptions,
        // including some fatal exceptions. COM interfaces are
        // not supposed to raise excpetion, and from security
        // perspective we are not supposed to catch fatal exceptions
        // I think we shouldn't do this.
        // IIS doesn't appear to be checking return HR from the call
    case SCRIPTPROP_CATCHEXCEPTION:
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;

        // The code is enabled in old engine, but there is no usage
        // in the system. I don't think we need this.
    case SCRIPTPROP_HACK_FIBERSUPPORT:
        AssertMsg(FALSE, "not tested");
        return E_NOTIMPL;

        // TODO: this is optimization to reduce lookup cost in scoped lookup
        // where we know the Var object is part of the global binder (like windows).
        // We don't need this feature in new engine.
    case SCRIPTPROP_ABBREVIATE_GLOBALNAME_RESOLUTION:
        if (VT_BOOL != pvarValue->vt)
            return E_INVALIDARG;
        EXPECT_POINTER(GetScriptSiteHolder());
        fCanOptimizeGlobalLookup = (VARIANT_FALSE != pvarValue->boolVal);
        if(scriptContext)
        {
            scriptContext->SetCanOptimizeGlobalLookupFlag(fCanOptimizeGlobalLookup);
        }
        return NOERROR;

        // This is the Invoke Version information mshtml passes to the jscript engine, and we
        // need to pass it wFlags for GetDispID, GetNextDispID, InvokeEx,
        // DeleteMemberByName when IDispatchEx is supported in the interface.
        // We need to do the same thing for IDispatch* HostDispatch & JavascriptDispatch
        // interface pointer if QI IDispatchEx*  succeeded.
        // The flag is split into two now. The invokeVersion is used for calling out on IDispatchEx.
        // The scriptVersion is used to determine the types of quirks to support in the JScript
        // library and when dealing with certain return values from within HostDispatch. For the
        // invokeVersion mshtml wants to clamp to a maximum version of 8, but 9 and later is still
        // used to control various flags in the script engine through scriptVersion.
    case SCRIPTPROP_INVOKEVERSIONING:
        if ( nullptr != pvarIndex )
        {
            return E_INVALIDARG;
        }

        if ( VT_I4 != pvarValue->vt || pvarValue->lVal > 0xF )
        {
            return E_INVALIDARG;
        }

        if (pvarValue->lVal != SCRIPTLANGUAGEVERSION_5_12)
        {
            Version_Inconsistency_fatal_error();
        }
                
        return NOERROR;

        // Set the host type: Browser(IE)/Application(WWA).
    case SCRIPTPROP_HOSTTYPE:
        if ( nullptr != pvarIndex )
        {
            return E_INVALIDARG;
        }

        if ( VT_I4 != pvarValue->vt || pvarValue->lVal <= SCRIPTHOSTTYPE_DEFAULT || pvarValue->lVal > SCRIPTHOSTTYPE_MAX)
        {
            // Note: SCRIPTHOSTTYPE_DEFAULT is used to detect engines that didn't set the host type,
            // but it's not valid for real engine to set it, that's why '<='.
            return E_INVALIDARG;
        }       

        this->hostType = pvarValue->lVal;

        if (this->hostType == SCRIPTHOSTTYPE_APPLICATION || this->hostType == SCRIPTHOSTTYPE_WEBVIEW)
        {
            // The host is WinRT enabled, so enable WER exception support
            Js::Configuration::Global.flags.WERExceptionSupport = TRUE;
        }
        return NOERROR;

    case SCRIPTPROP_NONPRIMARYENGINE:
        if (nullptr != pvarIndex)
        {
            return E_INVALIDARG;
        }
        if ( VT_I4 != pvarValue->vt )
        {
            return E_INVALIDARG;
        }

        SetNonPrimaryEngine(pvarValue->lVal != 0);
        return NOERROR;

    case SCRIPTPROP_QUERYCONTINUE_TIMER:
        if (nullptr != pvarIndex)
        {
            return E_INVALIDARG;
        }
        if ( VT_I4 != pvarValue->vt )
        {
            return E_INVALIDARG;
        }

        m_dwTicksPerPoll = pvarValue->lVal;
        if(GetScriptSiteHolder())
        {
            GetScriptSiteHolder()->SetTicksPerPoll(m_dwTicksPerPoll);
        }

        return NOERROR;

    case SCRIPTPROP_OPTIMIZE_FOR_MANY_INSTANCES:
        {
            if ( nullptr != pvarIndex )
            {
                return E_INVALIDARG;
            }
            if ( VT_BOOL != pvarValue->vt )
            {
                return E_INVALIDARG;
            }

            ThreadContext *const threadContext = ThreadContext::GetContextForCurrentThread();
            Assert(threadContext); // script engine should be initialized before this
            threadContext->OptimizeForManyInstances(pvarValue->boolVal != VARIANT_FALSE);
            return NOERROR;
        }

    case SCRIPTPROP_WEBWORKERID:
        {
            if ( VT_UI4 != pvarValue->vt )
            {
                return E_INVALIDARG;
            }

            webWorkerID = pvarValue->lVal;

            if (this->scriptContext != nullptr)
            {
                this->scriptContext->webWorkerId = webWorkerID;
            }

            return NOERROR;
        }

    case SCRIPTPROP_HOSTKEEPALIVE:
        {
            if ( nullptr != pvarIndex )
            {
                return E_INVALIDARG;
            }
            if ( VT_BOOL != pvarValue->vt )
            {
                return E_INVALIDARG;
            }
            fKeepEngineAlive = (pvarValue->boolVal != VARIANT_FALSE);
            return NOERROR;
        }

    case SCRIPTPROP_WEBPLATFORM_VERSION:
        {
            if ( nullptr != pvarIndex )
            {
                return E_INVALIDARG;
            }
            if ( VT_UI8 != pvarValue->vt )
            {
                return E_INVALIDARG;
            }
            // Ignore web platform version, we don't use it currently.        
            return NOERROR;
        }
    case SCRIPTPROP_EMIE:
        {
            if ( nullptr != pvarIndex )
            {
                return E_INVALIDARG;
            }
            if ( VT_BOOL != pvarValue->vt )
            {
                return E_INVALIDARG;
            }
            if (pvarValue->boolVal != VARIANT_FALSE)
            {
                // EMIE not supported, Fail Fast?
                Version_Inconsistency_fatal_error();
            }            
            return NOERROR;
        }
    case SCRIPTPROP_DIAGNOSTICS_OM:
        {
            this->m_isDiagnosticsOM = TRUE;
            // This is called before SetScriptSite, so there won't be scriptcontext available.
            Assert(this->scriptContext == nullptr);
            return NOERROR;
        }
    case SCRIPTPROP_EVAL_RESTRICTION:
        {
            if ( nullptr != pvarIndex )
            {
                return E_INVALIDARG;
            }
            if ( VT_BOOL != pvarValue->vt )
            {
                return E_INVALIDARG;
            }
            
            this->m_fIsEvalRestrict = (pvarValue->boolVal != VARIANT_FALSE);

            Js::JavascriptMethod newFunctionFunc = &Js::JavascriptFunction::NewInstance;
            Js::JavascriptMethod evalFunc = &Js::GlobalObject::EntryEval;
            
            if (this->m_fIsEvalRestrict)
            {
                newFunctionFunc = &Js::JavascriptFunction::NewInstanceRestrictedMode;
                evalFunc = &Js::GlobalObject::EntryEvalRestrictedMode;
            }
            
            if (scriptContext != nullptr)
            {
                Js::JavascriptLibrary *library = scriptContext->GetLibrary();
                Assert(library != nullptr);

                library->GetEvalFunctionObject()->SetEntryPoint(evalFunc);
                library->GetFunctionConstructor()->SetEntryPoint(newFunctionFunc);
            }
            return NOERROR;
        }
    case SCRIPTPROP_ALLOW_WINRT_CONSTRUCTOR:
        {
            if (nullptr != pvarIndex)
            {
                return E_INVALIDARG;
            }
            if (VT_BOOL != pvarValue->vt)
            {
                return E_INVALIDARG;
            }

            this->m_fAllowWinRTConstructor = (pvarValue->boolVal != VARIANT_FALSE);

            return NOERROR;
        }
    default:
        break;
    }
    return E_NOTIMPL;
}

/*************************************************************************
*
* ScriptEngine::GetHostSecurityManager
*
* This function obtains an IInternetHostSecurityManager from the
* CALLER, and if the caller is not available it tries to get one
* from the SITE.
*
* You should Release() the security manager when you're done with it.
* Since the caller may vary from call to call there is no point in
* caching the pointer.
*
* IMPORTANT NOTE: This function uses the DexCaller object to do the
* QueryService on the caller and then the site if that fails.  This
* function depends on knowledge of the DexCaller internals.
*
************************************************************************/
HRESULT ScriptEngine::GetHostSecurityManager(IInternetHostSecurityManager **ppsecman)
{
    AssertMem(ppsecman);
    *ppsecman = nullptr;

    HRESULT     hr;
    DispatchExCaller * pdc = nullptr;

    if (nullptr == GetScriptSiteHolder())
    {
        AssertMsg(FALSE, "How did we get here with no session object?");
        return E_FAIL;
    }

    IFFAILGO(GetScriptSiteHolder()->GetDispatchExCaller(&pdc));
    hr = pdc->QSCaller(SID_SInternetHostSecurityManager, IID_IInternetHostSecurityManager, (void **) ppsecman);
    if (FAILED(hr))
    {
        IFFAILGO(pdc->QSSite(SID_SInternetHostSecurityManager, IID_IInternetHostSecurityManager, (void **) ppsecman));
    }

    hr = S_OK;

LReturn:
    if (nullptr != pdc)
        GetScriptSiteHolder()->ReleaseDispatchExCaller(pdc);
    return hr;
}

/*************************************************************************
*
* ScriptEngine::GetINETSecurityManagerNoRef
*
* This function obtains an IInternetSecurityManager from the
* script site.  If the site gives us no joy, we cocreate one.
*
* We cache the pointer, so there is no need to Release() it.
*
************************************************************************/
HRESULT ScriptEngine::GetINETSecurityManagerNoRef(IInternetSecurityManager **ppsecman)
{
    AssertMem(ppsecman);
    *ppsecman = nullptr;

    HRESULT hr;
    IServiceProvider * psp;

    if (m_fNoINETSecurityManager)
        return E_FAIL;

    if (nullptr != m_pinetsecman)
    {
        *ppsecman = m_pinetsecman;
        return S_OK;
    }

    hr = m_pActiveScriptSite->QueryInterface(IID_IServiceProvider, (void **)&psp);
    if (SUCCEEDED(hr))
    {
        hr = psp->QueryService(SID_SInternetSecurityManager,
            IID_IInternetSecurityManager, (void **)&m_pinetsecman);
        psp->Release();
    }

    if (FAILED(hr))
    {
        hr = CoCreateInstance(CLSID_InternetSecurityManager, nullptr, CLSCTX_INPROC_SERVER,
            IID_IInternetSecurityManager, (void **)&m_pinetsecman);
        if (FAILED(hr))
        {
            m_fNoINETSecurityManager = TRUE;
            return hr;
        }
    }

    *ppsecman = m_pinetsecman;
    return S_OK;
}

// === Protected Methods ===
void ScriptEngine::ResetSecurity(void)
{
    m_fNoHostSecurityManager = TRUE;
    m_fNoINETSecurityManager = TRUE;
    RELEASEPTR(m_psitehostsecman);
    RELEASEPTR(m_pinetsecman);

}

HRESULT ScriptEngine::SetObjectSafety(IObjectSafety *psafe, REFIID riid, DWORD dwMask, DWORD dwSet)
{
    return psafe->SetInterfaceSafetyOptions(riid, dwMask , dwSet);
}

/*************************************************************************
*
* ScriptEngine::GetSiteHostSecurityManagerNoRef
*
* This function obtains an IInternetHostSecurityManager from the script site.
*
* We cache the pointer, so there is no need to Release() it.
*
************************************************************************/
HRESULT ScriptEngine::GetSiteHostSecurityManagerNoRef(IInternetHostSecurityManager **ppsecman)
{
    AssertMem(ppsecman);
    *ppsecman = nullptr;
    if (m_fNoHostSecurityManager || nullptr == m_pActiveScriptSite)
        return E_FAIL;

    HRESULT hr;
    if (nullptr == m_psitehostsecman)
    {
        IServiceProvider * psp;
        hr = m_pActiveScriptSite->QueryInterface(IID_IServiceProvider, (void **)&psp);
        if (SUCCEEDED(hr))
        {
            hr = psp->QueryService(SID_SInternetHostSecurityManager,
                IID_IInternetHostSecurityManager, (void **)&m_psitehostsecman);
            psp->Release();
        }
        if (FAILED(hr))
            m_fNoHostSecurityManager = TRUE;
    }
    else
        hr = NOERROR;
    *ppsecman = m_psitehostsecman;
    return hr;
}

void ScriptEngine::ResetLocales(void)
{
    SetCurrentLocale(m_lcidUserDefault);
}

BOOL ScriptEngine::SetCurrentLocale(LCID lcid)
{
    // Map any special lcid's to their real values.
    switch (lcid)
    {
    case LOCALE_USER_DEFAULT:
        lcid = GetDefaultLocale();
        break;
        /*
        case LOCALE_SYSTEM_DEFAULT:
        lcid = GetSystemDefaultLCID();
        break;
        */
    default: //IE passes correct lcid got from GetUserDefaultLCID
        if (!IsValidLocale(lcid, LCID_INSTALLED))
            return FALSE;
    }

    // Setting the locale also sets the error locale and codepage
    WCHAR szLocale[6];
    UINT codepage;

    int ret = GetLocaleInfoW(lcid, LOCALE_IDEFAULTANSICODEPAGE, szLocale, 6);
    if (ret == 0)
    {
        return FALSE;
    }
    codepage = wcstoul(szLocale, nullptr, 10);
    m_fIsValidCodePage = ::IsValidCodePage(codepage);
    if( m_fIsValidCodePage )
        m_codepage = codepage;

    m_lcidUser = lcid;
    return TRUE;
}

BOOL ScriptEngine::SetCurrentCodePage(UINT codepage)
{
    if (!::IsValidCodePage(codepage))
        return FALSE;
    m_fIsValidCodePage = TRUE;
    m_codepage = codepage;
    return TRUE;
}

HRESULT ScriptEngine::GetLanguageInfo (BSTR * pbstrLang, GUID * pguidLang)
{
    SETRETVAL(pguidLang, m_riidLanguage);
    if (pbstrLang)
    {
        *pbstrLang = SysAllocString(m_pszLanguageName);
        IFNULLMEMRET(*pbstrLang);
    }
    return NOERROR;
}

STDMETHODIMP ScriptEngine::CollectGarbage(SCRIPTGCTYPE scriptgctype)
{
    HRESULT hr;

    // Pad CoCreates a bare script engine to call CollectGarbage to avoid stack pinning. Rather than write another
    // GC collection pass which ignores stack pins, which would sometimes be wrong, we instead simply call CollectGarbage
    // on this temporary engine. Since we don't call SetScriptSite our base thread is NOBASETHREAD. Allow this call to go
    // through since we'll follow up with getting the current thread context and won't use our ScriptEngine.
    if (m_dwBaseThread != NOBASETHREAD)
    {
        IFFAILRET(ValidateBaseThread());
    }

    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    if (nullptr == threadContext)
    {
        return E_INVALIDARG;
    }
    return JavascriptThreadService::CollectGarbage(threadContext->GetRecycler(), scriptgctype);
}


HRESULT STDMETHODCALLTYPE ScriptEngine::ParseInternal(
    __in LPWSTR scriptText,
    __out Var *scriptFunc)
{
    HRESULT hr = NOERROR;
    IfNullReturnError(scriptText, E_INVALIDARG);
    IfNullReturnError(scriptFunc, E_INVALIDARG);
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        *scriptFunc = nullptr;
        return hr;
    }

    // We are not executing the script here so we don't need to catch error.

    CompileScriptException se;
    Js::Utf8SourceInfo* sourceInfo;
    Js::JavascriptFunction* jsFunc = scriptContext->LoadScript(scriptText, null, &se, true /* isExpression */, false /* disableDeferredParse */, false, &sourceInfo, Js::Constants::UnknownScriptCode);

    // TODO: is this the right way to handle these parse error?
    if (jsFunc == nullptr)
    {
        SourceContextInfo* sourceContextInfo = nullptr;
        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
        {
            // Note: pass sourceMapUrl as null, as we don't have IActiveScriptContext for script strings for like setTimeout.
            sourceContextInfo = this->GetSourceContextInfo(Js::Constants::NoHostSourceContext, wcslen(scriptText), /*isDynamicDocument*/ FALSE , /*sourceMapUrl=*/NULL, /*profileDataCache=*/ nullptr);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ 0,
            /* ulColumnHost        */ 0,
            /* lnMinHost           */ 0,
            /* ichMinHost          */ 0,
            /* ichLimHost          */ 0,
            /* ulCharOffset        */ 0,
            /* mod                 */ 0,
            /* grfsi               */ 0
        };

        *scriptFunc = nullptr;
        if (SUCCEEDED(hr))
        {
            hr = ReportCompilerError(&si, &se, nullptr, sourceInfo);
        }
        return hr;
    }

    // TODO: Parse should be called with a parameter to indicate if this source is host managed
    // or runtime managed, and set it in srcinfo.  If host managed, the srccontext cookie should
    // also be set. For now this script is always runtime managed.
    if(this->IsDebuggerEnvironmentAvailable())
    {
        // if debugging, register this script block as eval code
        BEGIN_TRANSLATE_OOM_TO_HRESULT
        {
            // Assumption: LoadScript handles -ForceSerialized config option, therefore at this point we might have a defer deserialized top level function
        if (CONFIG_FLAG(ForceSerialized) && jsFunc->GetFunctionProxy() != null){
                jsFunc->GetFunctionProxy()->EnsureDeserialized();
            }
            jsFunc->GetFunctionBody()->CheckAndRegisterFuncToDiag(scriptContext);
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
    }

    *scriptFunc = jsFunc;
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngine::GetJavascriptOperationsInternal(
    __out IJavascriptOperations **operations)
{
    HRESULT hr = NOERROR;
    IfNullReturnError(operations, E_INVALIDARG);
    hr = VerifyOnEntry(TRUE);
    if (FAILED(hr))
    {
        *operations = nullptr;
        return hr;
    }
    if (this->jsOps == nullptr)
    {
        CJavascriptOperations* jsOps = HeapNewNoThrow(CJavascriptOperations, scriptContext);
        if (nullptr == jsOps)
        {
            return E_OUTOFMEMORY;
        }
        this->jsOps = jsOps;
        jsOps->AddRef();
    }
    return this->jsOps->QueryInterface(_uuidof(IJavascriptOperations), (void**)operations);
}


// Info:        Given an IUnknown, will QI the IUnknown for IInspectable, then convert it to a JavaScript Var
//              representing the JavaScript projection of the underlying IInspectable.
// Parameters:  unknown - The object to be converted. Must implement IInspectable but not IDispatch or IDispatchEx.
//              instance - A pointer to the resulting Var
// Returns:     ERROR_BAD_ENVIRONMENT for nullptr projectionContext
//              E_POINTER for nullptr parameters (unknown, instance)
//              E_INVALIDARG for IUnknown that can't be QI'd for IInspectable
//              E_FAIL for failure to marshal the obtained IInspectable
//              S_OK otherwise
HRESULT STDMETHODCALLTYPE ScriptEngine::InspectableUnknownToVarInternal(
    __in IUnknown* unknown,
    __out Var* instance)
{
#ifdef ENABLE_PROJECTION
    HRESULT hr = NOERROR;
    IfNullReturnError(projectionContext, HRESULT_FROM_WIN32(ERROR_BAD_ENVIRONMENT));
    IfNullReturnError(unknown, E_POINTER);
    IfNullReturnError(instance, E_POINTER);
    hr = VerifyOnEntry();
    if (FAILED(hr))
    {
        *instance = nullptr;
        return hr;
    }

    *instance = scriptContext->GetLibrary()->GetNull();

    CComPtr<IInspectable> inspectable;
    hr = unknown->QueryInterface(IID_IInspectable, (void**)&inspectable);
    if (FAILED(hr) || !inspectable)
    {
        return E_INVALIDARG;
    }
    IUnknown* result = nullptr;
    if (SUCCEEDED(unknown->QueryInterface(IID_IDispatch, (void**)&result)))
    {
        result->Release();
        return E_INVALIDARG;
    }
#if DBG
    result = nullptr;
    Assert(FAILED(unknown->QueryInterface(__uuidof(IDispatchEx), (void**)&result)));
    if (result)
    {
        result->Release();
    }
#endif
    CComPtr<IUnknown> unkptr;
    hr = unknown->QueryInterface(IID_IUnknown, (void**)&unkptr);
    if (FAILED(hr) || !unkptr)
    {
        return E_INVALIDARG;
    }
    hr = S_OK;

    BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, false)
    {
        Projection::ProjectionMarshaler marshal(Projection::ResourceCleanup::CalleeRetainsOwnership, this->GetProjectionContext(), false);
        *instance = marshal.TryReadInspectable(unkptr, inspectable);
    }
    END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

        if (SUCCEEDED(hr) && (Js::JavascriptOperators::GetTypeId(*instance) == Js::TypeIds_Undefined))
        {
            hr = E_FAIL;
        }
        return hr;
#else // ENABLE_PROJECTION
    return E_NOTIMPL;
#endif
}

// ==== ScriptEngine Implementation =============================================

inline HRESULT ScriptEngine::GetDebugSiteNoRef(IActiveScriptSiteDebug **pscriptSiteDebug)
{
    if (m_fDumbHost)
        return E_FAIL;
    return GetDebugSiteCoreNoRef(pscriptSiteDebug);
}

inline void ScriptEngine::EnableInterrupts(void)
{
    LeaveCriticalSection(&m_csInterrupt);
}

inline void ScriptEngine::DisableInterrupts (void)
{
    EnterCriticalSection(&m_csInterrupt);
}



IActiveScriptDirectHost* ScriptEngine::GetActiveScriptDirectHostNoRef()
{
    HRESULT hr = NOERROR;
    if (m_activeScriptDirectHost == nullptr)
    {
        hr = GetScriptSite(_uuidof(IActiveScriptDirectHost), (void**)&m_activeScriptDirectHost);
    }
    if (hr == S_OK)
    {
        return m_activeScriptDirectHost;
    }
    return nullptr;
}

const LPWSTR g_featureKeyName = L"Software\\Microsoft\\Internet Explorer\\JScript9";

LPWSTR JsUtil::ExternalApi::GetFeatureKeyName()
{
    return g_featureKeyName;
}

bool ConfigParserAPI::FillConsoleTitle(__ecount(cchBufferSize) LPWSTR buffer, size_t cchBufferSize, __in LPWSTR moduleName)
{
    swprintf_s(buffer, cchBufferSize, L"PID: %d - %s - %d.%d.%4d.%d", GetCurrentProcessId(), moduleName,
        SCRIPT_ENGINE_MAJOR_VERSION, SCRIPT_ENGINE_MINOR_VERSION, SCRIPT_ENGINE_PRODUCTBUILD, SCRIPT_ENGINE_BUILDNUMBER);

    return true;
}

void ConfigParserAPI::DisplayInitialOutput(__in LPWSTR moduleName)
{
    Output::Print(L"INIT: PID        : %d\n", GetCurrentProcessId());
    Output::Print(L"INIT: DLL Path   : %s\n", moduleName);
    Output::Print(L"INIT: Build      : %d.%d.%4d.%d", SCRIPT_ENGINE_MAJOR_VERSION, SCRIPT_ENGINE_MINOR_VERSION, SCRIPT_ENGINE_PRODUCTBUILD, SCRIPT_ENGINE_BUILDNUMBER);
#if defined(__BUILDMACHINE__)
#if defined(__BUILDDATE__)
#define B2(x,y) " (" #x "." #y ")"
#define B1(x,y) B2(x, y)
#define BUILD_MACHINE_TAG B1(__BUILDMACHINE__, __BUILDDATE__)
#else
#define B2(x) " built by: " #x
#define B1(x) B2(x)
#define BUILD_MACHINE_TAG B1(__BUILDMACHINE__)
#endif
    Output::Print(L"%S", BUILD_MACHINE_TAG);
#endif
}

HRESULT __stdcall JsVarAddRef(Var instance)
{
    HRESULT hr = NOERROR;
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    if (nullptr == threadContext)
    {
        AssertMsg(false, "JsVarAddRef called from wrong thread");
        return E_INVALIDARG;
    }
    Recycler* recycler = threadContext->GetRecycler();
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
#if DBG
        if (recycler->IsValidObject(instance))
#endif
        {
            recycler->RootAddRef(instance);
        }
        JS_ETW(EventWriteJSCRIPT_RECYCLER_EXTERNAL_ADDREF(instance));
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    // We are expecting only two HRESULTs here- either it succeded or it ran out of memory
    Assert(SUCCEEDED(hr) || hr == E_OUTOFMEMORY);

    // This method can return a failure HRESULT
    // Threshold TFS 112123 tracks making sure that FastDOM handles failure HRESULTs
    return hr;
}

HRESULT __stdcall JsVarRelease(Var instance)
{
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    if (nullptr == threadContext)
    {
        AssertMsg(false, "JsVarAddRef called from wrong thread");
        return E_INVALIDARG;
    }
    Recycler* recycler = threadContext->GetRecycler();
#if DBG
    if (recycler->IsValidObject(instance))
#endif
    {
        recycler->RootRelease(instance);
    }
    JS_ETW(EventWriteJSCRIPT_RECYCLER_EXTERNAL_RELEASE(instance));
    return NOERROR;
}

HRESULT __stdcall JsVarToScriptDirect(Var instance, IActiveScriptDirect** scriptDirectRef)
{
    HRESULT hr = E_FAIL;
    *scriptDirectRef = nullptr;
    if (Js::RecyclableObject::Is(instance))
    {
        Js::RecyclableObject* object = Js::RecyclableObject::FromVar(instance);
        Js::ScriptContext* scriptContext = object->GetScriptContext();
        // Verify that this is being called from the same thread.
        Assert(scriptContext->GetRecycler() == ThreadContext::GetContextForCurrentThread()->GetRecycler());
        if (scriptContext->IsClosed())
        {
            return E_ACCESSDENIED;
        }
        *scriptDirectRef = scriptContext->GetActiveScriptDirect();
        Assert(*scriptDirectRef != nullptr);
        (*scriptDirectRef)->AddRef();
        hr = S_OK;
    }
    return hr;
}

HRESULT __stdcall JsVarToExtension(Var instance,void** extensionRef)
{
    HRESULT hr = S_OK;
    *extensionRef = nullptr;
    if (instance == nullptr)
    {
        hr = E_INVALIDARG;
    }
    // In finalize time the type information might not be available anymore,
    // but the object vtbl should still be intact.
    if (Js::TaggedNumber::Is(instance) || !(Js::RecyclableObject::FromVar(instance))->IsExternalVirtual())
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *extensionRef = (void*)(((char*)instance) + sizeof(Js::CustomExternalObject));

    }
    return hr;
}

STDMETHODIMP ScriptEngine::OnEvent(_In_ EventId eventId, _In_ VARIANT* pvarArgs)
{
    switch (eventId)
    {
    case EventId_StartupComplete:
        GetScriptContext()->OnStartupComplete();
        return S_OK;
    case EventId_SuspendCleanupStart:
    case EventId_SuspendCleanupEnd:
        return E_INVALIDARG;
    };
    Assert(false);
    return E_NOTIMPL;
}


HRESULT ScriptEngine::ReportCompilerError(SRCINFO* srcInfo, CompileScriptException* se, EXCEPINFO * pexcepinfo, Js::Utf8SourceInfo* sourceInfo)
{
    HRESULT hr = NOERROR;
    // Try to report the error.
    ActiveScriptError *pase;
    if (FAILED(ActiveScriptError::CreateCompileError(srcInfo, se, sourceInfo, &pase)))
        return HR(se->ei.scode);

    // report the error as a debugger event
    if (m_pda != nullptr)
    {
        m_pda->FireDebuggerEvent(__uuidof(IRemoteDebugCriticalErrorEvent110),
            static_cast<IUnknown *>(static_cast<IRemoteDebugCriticalErrorEvent110 *>(pase)));
    }

    if (NOERROR == OnScriptError((IActiveScriptError *) IACTIVESCRIPTERROR64 pase))
    {
        hr = SCRIPT_E_REPORTED;
    }
    else
        se->GetError(&hr,pexcepinfo);
    pase->Release();
    return hr;
}

HRESULT STDMETHODCALLTYPE ScriptEngine::SetActivityId(__in const GUID* pActivityId)
{
    HRESULT hr = NOERROR;
    IFFAILRET(VerifyOnEntry());
    if (pActivityId != nullptr)
    {
        this->m_activityID = *pActivityId;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

/*static*/
void ScriptEngine::TransitionToDebugModeIfFirstSource(Js::ScriptContext *scriptContext, Js::Utf8SourceInfo *sourceInfo)
{
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (scriptSite)
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();

        if (scriptEngine != nullptr)
        {
            scriptEngine->TransitionToDebugModeIfFirstSource(sourceInfo); // Omitting the result of this function
        }
    }
}

/*static*/
void ScriptEngine::RaiseMessageToDebugger(Js::ScriptContext *scriptContext, DEBUG_EVENT_INFO_TYPE messageType, LPCWSTR message, LPCWSTR url)
{
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (scriptSite)
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();

        if (scriptEngine != nullptr)
        {
            IDebugApplication *debugApplication = scriptEngine->GetDebugApplication();
            if (debugApplication != nullptr)
            {                
                CComPtr<RemoteDebugInfoEvent> remoteDebugInfo(HeapNewNoThrow(RemoteDebugInfoEvent, messageType, message, url));
                debugApplication->FireDebuggerEvent(__uuidof(IRemoteDebugInfoEvent110),
                    static_cast<IUnknown *>(static_cast<IRemoteDebugInfoEvent110 *>(remoteDebugInfo)));
            }
        }
    }
}

RemoteDebugInfoEvent::RemoteDebugInfoEvent(DEBUG_EVENT_INFO_TYPE messageType, LPCWSTR message, LPCWSTR url, IDebugDocumentContext *context/*= nullptr*/)
: m_cRef(0), m_documentContext(context)
{
    m_messageType = messageType;

    if (message)
    {
        m_message.m_str = ::SysAllocString(message);
    }

    if (url)
    {
        m_url.m_str = ::SysAllocString(url);
    }
}

// === IUnknown ===
STDMETHODIMP RemoteDebugInfoEvent::QueryInterface(REFIID riid, void **ppv)
{
    CHECK_POINTER(ppv);

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IUnknown *)this;
    }
    else if (IsEqualIID(riid, __uuidof(IRemoteDebugInfoEvent110)))
    {
        *ppv = (IRemoteDebugInfoEvent110 *)this;
    }
    else
    {
        *ppv = nullptr;
        return HR(E_NOINTERFACE);
    }
    AddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) RemoteDebugInfoEvent::AddRef(void)
{
    return InterlockedIncrement((long *)&m_cRef);
}

STDMETHODIMP_(ULONG) RemoteDebugInfoEvent::Release(void)
{
    long ref;

    ref = InterlockedDecrement((long *)&m_cRef);
    if (ref == 0)
    {
        HeapDelete(this);
    }

    return ref;
}

STDMETHODIMP RemoteDebugInfoEvent::GetEventInfo(DEBUG_EVENT_INFO_TYPE* pMessageType,
    BSTR* pbstrMessage,
    BSTR* pbstrUrl,
    IDebugDocumentContext** ppLocation)
{
    if (pMessageType)
    {
        *pMessageType = m_messageType;
    }
    m_message.CopyTo(pbstrMessage);
    m_url.CopyTo(pbstrUrl);

    if (ppLocation )
    {
        if (m_documentContext)
        {
            m_documentContext->QueryInterface(ppLocation);
        }
        else
        {
            *ppLocation = nullptr;
        }
    }

    return S_OK;
}

#ifndef NO_PRIVATE_ISOS
//
// Private copy of IsOS_OneCoreUAP to satisfy dtbhost.lib
// This implementation is based on the implementation at inetcore\lib\common\ieisos_sp.cpp
//
bool
IsOs_OneCoreUAP()
{
    static bool s_fInitialized = false;
    static bool s_fIsOsOneCoreUAP = false;

    if (!s_fInitialized)
    {
        HMODULE hModNtDll = GetModuleHandle(L"ntdll.dll");
        if (hModNtDll == nullptr)
        {
            RaiseException(0, EXCEPTION_NONCONTINUABLE, 0, 0);
        }

        typedef void(*PFNRTLGETDEVICEFAMILYINFOENUM)(ULONGLONG*, ULONG*, ULONG*);
        PFNRTLGETDEVICEFAMILYINFOENUM pfnRtlGetDeviceFamilyInfoEnum =
            reinterpret_cast<PFNRTLGETDEVICEFAMILYINFOENUM>(GetProcAddress(hModNtDll, "RtlGetDeviceFamilyInfoEnum"));

        if (pfnRtlGetDeviceFamilyInfoEnum)
        {
            // True by default on Threshold except in the desktop/team/server cases
            s_fIsOsOneCoreUAP = true;

            ULONG ulPlatform;
            pfnRtlGetDeviceFamilyInfoEnum(nullptr /* uap info */, &ulPlatform, nullptr /* deviceClass */);
            if (ulPlatform == 3 || /* DEVICEFAMILYINFOENUM_DESKTOP */
                ulPlatform == 6 || /* DEVICEFAMILYINFOENUM_TEAM    */
                ulPlatform == 9)   /* DEVICEFAMILYINFOENUM_SERVER  */
            {
                // The only exceptions are desktop, team, and server which still have the legacy Win32 
                // binaries to support the desktop configuration.
                s_fIsOsOneCoreUAP = false;
            }
        }

        s_fInitialized = true;
    }

    return s_fIsOsOneCoreUAP;
}
#endif