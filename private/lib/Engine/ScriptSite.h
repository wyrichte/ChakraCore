//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

const ulong kcbosMaskTestPoll   = 0x001F;
const ulong kluTickPollInterval = 500; // 2 times per second.

class Js::ModuleRoot;
typedef RecyclerFastAllocator<RefCountedHostVariant, FinalizableObjectBits> RefCountedHostVariantAllocator;

#if defined(PROFILE_EXEC) || defined(PROFILE_MEM)
class ProfileOnLoadCallBack : public Js::JavascriptFunction
{
public:
    static bool AttachEvent(ScriptSite * scriptSite);

protected:
    DEFINE_VTABLE_CTOR(ProfileOnLoadCallBack, JavascriptFunction);
    DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(ProfileOnLoadCallBack);

private:
    static Js::FunctionInfo functionInfo;
    ProfileOnLoadCallBack(ScriptSite * scriptSite);

    static Js::Var EntryProfileOnLoadCallBack(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);

    ScriptSite * scriptSite;
};
#endif

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    class IASDDebugObjectHelper;
#endif

class ScriptSite
{
    friend class JavascriptDispatch;
    friend class HostDispatch;
    friend class QueryContinuePoller;
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    friend class DebugObject;
#endif
public:
    typedef JsUtil::BaseDictionary<void *, JavascriptDispatch*, ArenaAllocator, PrimeSizePolicy, RecyclerPointerComparer> DispatchMap;
private:

    volatile long refCount;
    ScriptEngine* scriptEngine;
    IActiveScriptDirect* scriptDirect;

    // keep the IActiveScript alive if Close() is not called, and there is outstanding script javascriptdispatch.
    IActiveScript* activeScriptKeepAlive;
    // number of active JavascriptDispatch created from current ScriptSite.
    ULONG outstandingDispatchCount;

    // modules are list of ModuleRoots that map to incoming
    // named item. it will be the scope object in scoped lookup
    // generally the list is not very long and we don't need to have a
    // dictionary.
    // global object is explicitly pinned; this is used to pin all the moduleroots,
    // and do the lookup from byte code as needed.
    RecyclerRootPtr<JsUtil::List<Js::ModuleRoot*>> moduleRoots;

    // Global scope. Note that we don't need a separate global module
    // the global object itself is enough.
    JsUtil::List<HostDispatch*, ArenaAllocator>* globalDispatches;

    // Mapping from JS object to JS Dispatch to ensure COM identity
    // required by the host.
    DispatchMap* dispatchMap;
    LIST_ENTRY hostDispatchListHead;
    LIST_ENTRY javascriptDispatchListHead;

    RefCountedHostVariantAllocator refCountedHostVariantAllocator;
    BOOL refCountedAllocatorInitialized;
    ArenaAllocator* hostAllocator;
    Js::ScriptContext* scriptSiteContext;
    Recycler* recycler;

    // Caller stuff.
    DispatchExCaller *currentDispatchExCaller;
    IUnknown *m_punkCaller;
    ThreadContextId threadId;
    BOOL isClosed;

    DWORD ticksPerPoll;
    IActiveScriptSiteInterruptPoll *m_ppoll;
    IActiveScriptDirectSite* m_IASDSite;

    enum
    {
        kgrfbrkBreak =
            APPBREAKFLAG_DEBUGGER_BLOCK |
            APPBREAKFLAG_DEBUGGER_HALT |
            APPBREAKFLAG_STEP,
        kgrfbrkGlobalBreak =
            APPBREAKFLAG_DEBUGGER_BLOCK |
            APPBREAKFLAG_DEBUGGER_HALT
    };

    APPBREAKFLAGS m_grfbrk;
    BOOL m_fAllowBreakpoints;
    BOOL m_fAbort;
    void * m_pvMinorSession;

    ThreadContext* threadContext;

#if DBG_DUMP
    uint allocId;
    uint parentAllocId;
    wchar_t const * wszLocation;
#endif
#if DBG_DUMP || defined(PROFILE_EXEC)
#ifdef PROFILE_EXEC
    BOOL isProfileCreated;
#endif

    ScriptSite * prevTopLevelScriptSite;
    ScriptSite * nextTopLevelScriptSite;
    ScriptSite * parentScriptSite;
    Js::RecyclableObject * windowHost;
    bool hasParentInfo;
    bool isCloned;
    StackBackTrace * setHostObjectStackBackTrace;
    StackBackTrace * reinitHostObjectStackBackTrace;
    static const int StackToSkip = 2;
    static const int StackTraceDepth = 40;
#endif
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    IASDDebugObjectHelper* debugObjectHelper;
#endif

protected:
    ScriptSite();
    ~ScriptSite();
    HRESULT Init(
        __in ScriptEngine *scriptEngine,
        __in IActiveScriptSite *iActiveScriptSite,
        __in BOOL useArena);


public:
    /////////////////////////
    static HRESULT Create(
        __in ScriptEngine *scriptEngine,
        __in IActiveScriptSite *pss,
        __in BOOL useArena,
        __out ScriptSite **scriptSite);

    // Close is called by ScriptEngine when the ScriptEngine is Reset or Closed.
    void Close(void);

    HRESULT CheckEvalRestriction() const;
    HRESULT HostExceptionFromHRESULT(HRESULT hr, Var* outError) const;
    HRESULT GetExternalJitData(ExternalJitData id, void *data) const;

    // Reference Counting
    void AddRef(void) { InterlockedIncrement(&refCount); }
    void Release(void) { if (0 == InterlockedDecrement(&refCount)) delete this; }

    // Getting a module binder.
    Js::ModuleRoot * GetModuleRoot(
        __in Js::ModuleID moduleID);

    HRESULT GetModuleDispatch(
        __in Js::ModuleID moduleID,
        __out IDispatch** dispatch);

    Js::ScriptContext* GetScriptSiteContext() const { return scriptSiteContext; }
    ThreadContext *GetThreadContext() const { return threadContext; }
    Recycler* GetRecycler() const { return recycler; }
    IDispatch* GetGlobalObjectDispatch() ;
    JsUtil::List<HostDispatch*, ArenaAllocator>* GetGlobalDispatches() const { return globalDispatches; }
    IActiveScriptSiteInterruptPoll * RetrieveInterruptPoll() {IActiveScriptSiteInterruptPoll* poll = m_ppoll; m_ppoll = null; return poll; }
    void SetInterruptPoll(IActiveScriptSiteInterruptPoll* poll) {m_ppoll = poll; }
    DWORD GetTicksPerPoll() const { return ticksPerPoll; }
    void SetTicksPerPoll(DWORD ticks) { ticksPerPoll = ticks; }
    ArenaAllocator* HostAllocator();

    DispatchMap * EnsureDispatchMap();
    DispatchMap * GetDispatchMap() { return this->dispatchMap; }
    RefCountedHostVariantAllocator * GetRefCountedHostVariantAllocator() { return &refCountedHostVariantAllocator; }

    // adding objects to the ModuleRoot
    HRESULT AddExternalObject(
        __in LPCOLESTR pszName,
        __in IDispatch *pdisp,
        __in long lwCookie);

    // add external IDispatch to matching ModuleRoot or global dispatch list.
    HRESULT AddDefaultDispatch(Js::ModuleID mod, IDispatch* pdisp);

    // Code entry points
    HRESULT Execute(__in Js::RecyclableObject *pScrObj, __in Js::Arguments* args, __in IServiceProvider * pspCaller, __out_opt Js::Var* varResult);

    HRESULT GetDispatchExCaller(__out DispatchExCaller **dispatchExCaller);
    void ReleaseDispatchExCaller(__in DispatchExCaller *dispatchExCaller);

    HRESULT GetPreviousHostScriptContext(__out HostScriptContext** hostScriptContext);

    HRESULT PushHostScriptContext(HostScriptContext* hostScriptContext);
    void PopHostScriptContext();

    ULONG AddDispatchCount();
    ULONG ReleaseDispatchCount();

    void SetCaller(__in IUnknown *punkNew, __deref_out_opt IUnknown **ppunkPrev);

    IUnknown* GetCurrentCallerNoRef() { return m_punkCaller; }
    ThreadContextId GetThreadId() { return threadId; }

    BOOL IsClosed() const { return isClosed; }
    void SetupFastDOM(IActiveScriptDirect* scriptDirect);
    void AddToJavascriptDispatchList(LIST_ENTRY* newEntry) {InsertHeadList(&javascriptDispatchListHead, newEntry); }
    HRESULT CheckCrossDomainScriptContext(__in Js::ScriptContext* scriptContext);

    Js::JavascriptFunction* GetDefaultGetter(JavascriptTypeId typeId, PropertyId nameId, unsigned int slotIndex);
    Js::JavascriptFunction* GetDefaultSetter(JavascriptTypeId typeId, PropertyId nameId, unsigned int slotIndex);
    HRESULT VerifyDOMSecurity(Js::ScriptContext* targetScriptContext, Js::Var obj);

    template <typename Fn>
    static void ReportJavascriptDispatchObjectsHelper(JavascriptDispatch* obj, Fn* callback)
    {
        (*callback)(obj);
    }

    template <typename Fn>
    void ReportJavascriptDispatchObjects(Fn callback) {
        if (IsListEmpty(&javascriptDispatchListHead)) return;
        LIST_ENTRY* listEntry = &javascriptDispatchListHead;
        do
        {
            listEntry = NextEntryList(listEntry);
            ReportJavascriptDispatchObjectsHelper<Fn>(CONTAINING_RECORD(listEntry, JavascriptDispatch, linkList), &callback);
        } while (! IsEntryListTail(&javascriptDispatchListHead, listEntry));
    }

    static __inline ScriptSite* FromScriptDirect(IActiveScriptDirect* scriptDirect)
    {
        ScriptEngine* scriptEngine;
        scriptEngine = static_cast<ScriptEngine*>(scriptDirect);
        return scriptEngine->GetScriptSiteHolder();
    }

    ////////////////////////////////


    // Getters
    ScriptEngine *GetScriptEngine(void) { return scriptEngine; }
    LCID GetUserLocale(void);

    void RecordExcepInfoAndClear(EXCEPINFO *pei, HRESULT *phr);

    void SetAbort(BOOL fAbort) { m_fAbort = fAbort; }
    HRESULT HandleAbort(void)
    {
        if (!m_fAbort || !FInDebuggerCallback())
            return NOERROR;
        return RecordError(VBSERR_CantContinue);
    }

    HRESULT SetAppBreakFlags(APPBREAKFLAGS grfbrk, BOOL fIncludeThreadFlags)
    {
        if (!fIncludeThreadFlags)
        {
            grfbrk &= ~APPBREAKFLAG_STEP;
        }
        m_grfbrk = grfbrk;

        // If we're not in a callback from the debugger, we should allow
        // breakpoints as usual. If we are in a callback from the debugger
        // we should disable them unless the debugger explicitly turns
        // them on by setting DEBUG_TEXT_ALLOWBREAKPOINTS.
        m_fAllowBreakpoints = !FInDebuggerCallback();
        return NOERROR;
    }
    BOOL FOneTimeBreak(BREAKREASON *pbr)
    {
        if (!(m_grfbrk & kgrfbrkBreak))
            return FALSE;

        if (m_grfbrk & APPBREAKFLAG_DEBUGGER_HALT)
            *pbr = BREAKREASON_DEBUGGER_HALT;
        else if (m_grfbrk & APPBREAKFLAG_DEBUGGER_BLOCK)
            *pbr = BREAKREASON_DEBUGGER_BLOCK;
        else
            *pbr = BREAKREASON_STEP;
        return TRUE;
    }

    BOOL FInDebuggerCallback(void) { return 0 != (m_grfbrk & APPBREAKFLAG_IN_BREAKPOINT); }
    void SetAllowBreakpoints(BOOL fAllow) { m_fAllowBreakpoints = fAllow; }
    BOOL FAllowBreakpoints(void) { return m_fAllowBreakpoints; }

    void FreeError(void);
    static HRESULT ReportError(HRESULT hr);
    HRESULT RecordError(HRESULT hr);

    // When one script engine calls another, this function is used to
    // propagate any recorded error from the callee's session to the
    // caller's session.
    void PropagateRecordedError(ScriptSite *scriptSite, HRESULT *phr);

    HRESULT PollHalt(void);
    HRESULT HandleHalt(void);
    void GetStatementCount(ulong *pluHi, ulong *pluLo);

    static ScriptSite * FromScriptContext(Js::ScriptContext *);

    static HRESULT HandleJavascriptException(Js::JavascriptExceptionObject* exceptionObject, Js::ScriptContext * scriptContext, IServiceProvider * pspCaller = NULL);
    static HRESULT CallRootFunction(Js::JavascriptFunction * function, Js::Arguments args, IServiceProvider * pspCaller, Var * result);
    static HRESULT ExternalToPrimitive(Js::DynamicObject * obj, Js::JavascriptHint hint, Var * result, IServiceProvider * pspCaller = NULL);
    static HRESULT ExternalGetPropertyReference(Js::DynamicObject* scriptObject, DISPID id, Js::Var* varMember, IServiceProvider * pspCaller = NULL);
    static HRESULT ExternalGetProperty(Js::DynamicObject* scriptObject, DISPID id, Js::Var* varMember, IServiceProvider * pspCaller = NULL);
    static HRESULT ExternalSetProperty(Js::DynamicObject* scriptObject, DISPID id, Js::Var value, IServiceProvider * pspCaller = NULL);
#ifdef ENABLE_HEAP_DUMPER
    static HRESULT RegisterDebugDumpHeap(Js::ScriptContext *pContext);
#endif
    HRESULT ExternalToString(Js::Var instance, Js::JavascriptString ** result);
    HRESULT ExternalToNumber(Js::Var instance, double * result);
    HRESULT ExternalToNumberCanThrow(Js::Var instance, double * result);
    HRESULT ExternalToInt32(Js::Var instance, int * result);
    HRESULT ExternalToInt32CanThrow(Js::Var instance, int * result);
    HRESULT ExternalToInt64(Js::Var instance, __int64 * result);
    HRESULT ExternalToUInt64(Js::Var instance, unsigned __int64 * result);
    HRESULT ExternalToBoolean(Js::Var instance, BOOL * result);
    HRESULT ExternalToDate(Js::Var instance, double * result);
    HRESULT ExternalBOOLToVar(BOOL b, Js::Var * result);
    HRESULT ExternalInt32ToVar(int32 i, Js::Var * result);
    HRESULT ExternalInt32ToVarCanThrow(int32 i, Js::Var * result);
    HRESULT ExternalUInt64ToVar(unsigned __int64 i, Js::Var * result);
    HRESULT ExternalInt64ToVar(__int64 i, Js::Var * result);
    HRESULT ExternalDoubleToVar(double d, Js::Var * result);
    HRESULT ExternalDateToVar(double d, Js::Var * result);
    HRESULT ExternalSYSTEMTIMEToVar(SYSTEMTIME* pst, Js::Var * result);

    static HRESULT RegisterExternalLibrary(Js::ScriptContext *pScriptContext);
    static HRESULT CheckIsSiteAliveCallback(ScriptSite* scriptSite);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    IASDDebugObjectHelper* GetDebugObjectHelper() const { return debugObjectHelper; }
    void SetDebugObjectHelper(IASDDebugObjectHelper* helper) { debugObjectHelper = helper; }
#endif

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
    void SetupWindowHost(Js::RecyclableObject * hostObj);
    void EnsureParentInfo(Js::ScriptContext* scriptContext = NULL);
    void EnsureParentInfoWithScriptEnter();
    void SetUrl(BSTR bstrUrl);
    ScriptSite * GetParentScriptSite() { EnsureParentInfo(); return parentScriptSite; }
    void CaptureSetHostObjectTrace();
    void CaptureReinitHostObjectTrace();
#endif

private:
    HRESULT AddGlobalDispatch(__in IDispatch* dispatch);

    void CreateModuleRoot(
        __in Js::ModuleID moduleID,
        __in IDispatch* dispatch);

    HRESULT CheckIsSiteAlive();
    HRESULT VerifyStackOnEntry(BOOL allowedInHeapEnum = FALSE);

    void InitializeExternalLibrary();
    void InitializeTypes();
    void InitializeDebugObject();
    void InitializeDiagnosticsScriptObject();

#if defined(EDIT_AND_CONTINUE) && defined(ENABLE_DEBUG_CONFIG_OPTIONS)
    void InitializeEditTest();
#endif

    static void __cdecl InitializeDebugObjectType(Js::DynamicObject* mathObject, Js::DeferredTypeHandlerBase * typeHandler, Js::DeferredInitializeMode mode);

    static void ScriptStartEventHandler(Js::ScriptContext * scriptContext);
    static void ScriptEndEventHandler(Js::ScriptContext * scriptContext);

#ifdef FAULT_INJECTION
    static void DisposeScriptByFaultInjectionEventHandler(Js::ScriptContext * scriptContext);
#endif

    static inline HRESULT RegisterDebug(Js::ScriptContext *pContext);

    static HRESULT ReportError(Js::JavascriptExceptionObject * pError, Js::ScriptContext* requestContext);
    void EnsureDefaultDOMAccessors(unsigned int slotIndex);


#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
    void DumpSiteInfo(wchar_t const * message, wchar_t const * message2 = null);
    friend class ProfileOnLoadCallBack;
#endif
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void EnumHeap();
#endif
};

class AutoCallerPointer
{
public:
    AutoCallerPointer(ScriptSite* scriptSite, IUnknown* newCaller) :
      scriptSite(scriptSite),
      library(NULL),
      previousCaller(NULL)
    {
        if (!scriptSite->IsClosed())
        {
            library = scriptSite->GetScriptSiteContext()->GetLibrary();
        }
        scriptSite->SetCaller(newCaller, &originalCaller);
    }
    ~AutoCallerPointer()
    {
        scriptSite->SetCaller(originalCaller, &previousCaller);
        RELEASEPTR(previousCaller);
        RELEASEPTR(originalCaller);
    }
private:
    Js::JavascriptLibrary* library;
    IUnknown* originalCaller;
    IUnknown* previousCaller;
    ScriptSite* scriptSite;
};

class JScript9HostScriptContext sealed : public HostScriptContext
{
public:
    JScript9HostScriptContext(ScriptSite * scriptSite)
        : HostScriptContext(scriptSite->GetScriptSiteContext())
    {
        this->scriptSite = scriptSite;
        scriptSite->AddRef();
    }
    ~JScript9HostScriptContext()
    {
        scriptSite->Release();
    }
    virtual void Delete()
    {
        HeapDelete(this);
    }

    HRESULT GetPreviousHostScriptContext(__deref_out HostScriptContext** previousScriptSite)
    {
        return scriptSite->GetPreviousHostScriptContext(previousScriptSite);
    }

    HRESULT SetCaller(IUnknown *punkNew, IUnknown **ppunkPrev)
    {
        scriptSite->SetCaller(punkNew, ppunkPrev);
        return NOERROR;
    }

    BOOL HasCaller()
    {
        return scriptSite->GetCurrentCallerNoRef() != NULL;
    }

    HRESULT PushHostScriptContext()
    {
        return scriptSite->PushHostScriptContext(this);
    }

    void PopHostScriptContext()
    {
        scriptSite->PopHostScriptContext();
    }

    HRESULT GetDispatchExCaller(__deref_out void** dispatchExCaller)
    {
        return scriptSite->GetDispatchExCaller((DispatchExCaller**)dispatchExCaller);
    }

    void ReleaseDispatchExCaller(__in void* dispatchExCaller)
    {
        scriptSite->ReleaseDispatchExCaller((DispatchExCaller*)dispatchExCaller);
    }

    Js::ModuleRoot * GetModuleRoot(int moduleID)
    {
        Assert(moduleID != kmodGlobal);
        return scriptSite->GetModuleRoot(moduleID);
    }

    HRESULT CheckCrossDomainScriptContext(__in Js::ScriptContext* scriptContext) override
    {
        return scriptSite->CheckCrossDomainScriptContext(scriptContext);
    }

    HRESULT GetHostContextUrl(__in DWORD_PTR hostSourceContext, __out BSTR& pUrl) override
    {
        return scriptSite->GetScriptEngine()->GetHostContextUrl(hostSourceContext, &pUrl);
    }

    void CleanDynamicCodeCache() override
    {
        if (!scriptSite->IsClosed()) {
            scriptSite->GetScriptEngine()->CleanScriptBodyMap();
        }
    }

    HRESULT VerifyDOMSecurity(Js::ScriptContext* targetContext, Js::Var obj) override
    {
        return scriptSite->VerifyDOMSecurity(targetContext, obj);
    }

    Js::JavascriptMethod GetSimpleSlotAccessCrossSiteThunk() override
    {
        return DOMFastPathInfo::CrossSiteSimpleSlotAccessorThunk;
    }

    HRESULT CheckEvalRestriction() override
    {
        return scriptSite->CheckEvalRestriction();
    }

    HRESULT HostExceptionFromHRESULT(HRESULT hr, Js::Var* outError) override
    {
        return scriptSite->HostExceptionFromHRESULT(hr, outError);
    }

    HRESULT GetExternalJitData(ExternalJitData id, void *data) override
    {
        return scriptSite->GetExternalJitData(id, data);
    }

#if DBG_DUMP || defined(PROFILE_EXEC) || defined(PROFILE_MEM)
    void EnsureParentInfo(Js::ScriptContext* scriptContext = NULL) override
    {
        return scriptSite->EnsureParentInfo(scriptContext);
    }
#endif

    ScriptSite * GetScriptSite() { return scriptSite; }
private:
    ScriptSite * scriptSite;
};

class JScript9HostDebugContext sealed : public HostDebugContext
{
public:
    JScript9HostDebugContext(ScriptSite * scriptSite)
        : HostDebugContext(scriptSite->GetScriptSiteContext())
    {
        this->scriptSite = scriptSite;
        scriptSite->AddRef();
    }
    ~JScript9HostDebugContext()
    {
        scriptSite->Release();
    }

    virtual void Delete()
    {
        HeapDelete(this);
    }

    DWORD_PTR GetHostSourceContext(Js::Utf8SourceInfo * sourceInfo)
    {
        DWORD_PTR dwDebugHostSourceContext = Js::Constants::NoHostSourceContext;

        ScriptEngine* scriptEngine = this->GetScriptSite()->GetScriptEngine();
        SourceContextPair* pSourceContextPairs = scriptEngine->GetSourceContextPairs();

        for (ULONG i = 0; i < scriptEngine->GetSourceContextPairCount(); i++)
        {
            if (pSourceContextPairs[i].dwHostSourceContext == sourceInfo->GetHostSourceContext() &&
                pSourceContextPairs[i].ulCharOffset == sourceInfo->GetSrcInfo()->ulCharOffset)
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
        return dwDebugHostSourceContext;
    }

    HRESULT SetThreadDescription(__in LPCWSTR url)
    {
        HRESULT hr = S_OK;
        ScriptEngine * scriptEngine = this->GetScriptSite()->GetScriptEngine();
        if (scriptEngine->IsSetThreadDescription())
        {
            hr = scriptEngine->SetThreadDescription(url);
        }
        return hr;
    }

    HRESULT DbgRegisterFunction(Js::ScriptContext * scriptContext, Js::FunctionBody * functionBody, DWORD_PTR dwDebugSourceContext, LPCWSTR title)
    {
        return this->GetScriptSite()->GetScriptEngine()->DbgRegisterFunction(scriptContext, functionBody, dwDebugSourceContext, title);
    }

    void ReParentToCaller(Js::Utf8SourceInfo* sourceInfo)
    {
        if (sourceInfo && sourceInfo->HasScriptDebugDocument())
        {
            ScriptDebugDocument* document = static_cast<ScriptDebugDocument*>(sourceInfo->GetScriptDebugDocument());
            document->ReParentToCaller();
        }
    }

    ScriptSite * GetScriptSite() { return scriptSite; }
private:
    ScriptSite * scriptSite;
};
