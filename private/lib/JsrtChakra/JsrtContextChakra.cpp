//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "JsrtRuntime.h"
#include "JsrtComException.h"
#include "JsrtContext.h"
#include "jsrtprivate.h"
#include "ScriptProjectionHost.hxx"
#include "JsrtContextChakra.h"

//=============================================================================================
// These JsrtContext are divergent from the Core implementation so we can use JsrtContextChakra
//=============================================================================================
JsrtContext *JsrtContext::New(JsrtRuntime * runtime)
{
    JsrtContext * context = JsrtContextChakra::New(runtime);
    // Pin the jsrtContext instance to javascript library so it doesn't get collected pre-maturely.
    context->GetScriptContext()->GetLibrary()->PinJsrtContextObject(context);
    return context;
}

/* static */
bool JsrtContext::Is(void * ref)
{
    return VirtualTableInfo<JsrtContextChakra>::HasVirtualTable(ref);
}

void JsrtContext::OnScriptLoad(Js::JavascriptFunction * scriptFunction, Js::Utf8SourceInfo* utf8SourceInfo)
{
    ((JsrtContextChakra *)this)->OnScriptLoad(scriptFunction, utf8SourceInfo);
}

class JsrtDummyScriptSite sealed : public IActiveScriptSite, public IActiveScriptSiteDebug, public IActiveScriptSiteDebugHelper
{
private:
    ULONG refCount;
    IDebugApplication *debugApplication;

public:
    JsrtDummyScriptSite() :
        refCount(1),
        debugApplication(nullptr)
    {
    }

    ~JsrtDummyScriptSite()
    {
        ReleaseDebugApplication();
    }

    bool SetDebugApplication(IDebugApplication *debugApplication)
    {
        AssertMsg(this->debugApplication == nullptr, "Overiding debugApplication will result in a leak");

        if (this->debugApplication)
        {
            return false;
        }

        this->debugApplication = debugApplication;
        return true;
    }

    void ReleaseDebugApplication()
    {
        if (debugApplication)
        {
            debugApplication->Release();
            debugApplication = nullptr;
        }
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObject)
    {
        if (ppvObject == nullptr)
        {
            return E_POINTER;
        }

        if (IsEqualIID(riid, IID_IUnknown))
        {
            *ppvObject = static_cast<IActiveScriptSite *>(this);
            this->AddRef();
            return S_OK;
        }

        if (IsEqualIID(riid, IID_IActiveScriptSite))
        {
            *ppvObject = static_cast<IActiveScriptSite *>(this);
            this->AddRef();
            return S_OK;
        }

        if (IsEqualIID(riid, IID_IActiveScriptSiteDebug))
        {
            *ppvObject = static_cast<IActiveScriptSiteDebug *>(this);
            this->AddRef();
            return S_OK;
        }

        if (IsEqualIID(riid, __uuidof(IActiveScriptSiteDebugHelper)))
        {
            *ppvObject = static_cast<IActiveScriptSiteDebugHelper *>(this);
            this->AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return ++this->refCount;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG result = --this->refCount;

        if (result == 0)
        {
            HeapDelete(this);
        }

        return result;
    }

    // IActiveScriptSite
    STDMETHODIMP GetLCID(LCID * lcid) { return E_NOTIMPL; }
    STDMETHODIMP GetItemInfo(LPCOLESTR name, DWORD returnMask, IUnknown ** item, ITypeInfo ** typeInfo) { return TYPE_E_ELEMENTNOTFOUND; }
    STDMETHODIMP GetDocVersionString(BSTR * version) { return E_NOTIMPL; }
    STDMETHODIMP OnScriptTerminate(const VARIANT *result, const EXCEPINFO *excepinfo) { return S_OK; }
    STDMETHODIMP OnStateChange(SCRIPTSTATE scriptState) { return S_OK; }
    STDMETHODIMP OnScriptError(IActiveScriptError *scriptError) { return S_OK; }
    STDMETHODIMP OnEnterScript() { return S_OK; }
    STDMETHODIMP OnLeaveScript() { return S_OK; }

    // IActiveScriptSiteDebug
    // NOTE: We don't implement GetDocumentContextFromPosition because JSRT doesn't support host managed sources.
    // If we do at some point support it, we'll need an implementation of this method.
    STDMETHODIMP GetDocumentContextFromPosition(DWORD_PTR dwSourceContext, ULONG uCharacterOffset, ULONG uNumChars, IDebugDocumentContext** ppsc) { return E_NOTIMPL; }
    STDMETHODIMP GetApplication(IDebugApplication**  ppda)
    {
        IfNullReturnError(ppda, E_INVALIDARG);
        IfNullReturnError(debugApplication, E_NOTIMPL);

        *ppda = debugApplication;
        (*ppda)->AddRef();

        return S_OK;
    }
    STDMETHODIMP GetRootApplicationNode(IDebugApplicationNode**  ppdanRoot) { return E_NOTIMPL; }
    STDMETHODIMP OnScriptErrorDebug(IActiveScriptErrorDebug* pErrorDebug, BOOL* pfEnterDebugger, BOOL* pfCallOnScriptErrorWhenContinuing) { return E_NOTIMPL; }

    // IActiveScriptSiteDebugHelper
    STDMETHODIMP IsInDebugMode(BOOL *pfDebugMode)
    {
        IfNullReturnError(pfDebugMode, E_INVALIDARG);
        *pfDebugMode = TRUE;
        return S_OK;
    }
    STDMETHODIMP GetApplicationNode(IDebugApplicationNode** ppdan) { return E_NOTIMPL; }
};


JsrtContextChakra::JsrtContextChakra(JsrtRuntime * runtime) :
    JsrtContext(runtime),
    hasProjectionHost(false)
{
    ThreadContext * threadContext = runtime->GetThreadContext();

    this->scriptEngine = HeapNew(ScriptEngine, CLSID_Chakra, g_pszLangName);
    JsrtComException::ThrowIfFailed(this->scriptEngine->Initialize(threadContext));

    // we dont need to force a GC at scriptsite close. it's relatively lightweight,
    // and we can let JsCollectGarbage called by user.

    this->scriptEngine->SetNonPrimaryEngine(TRUE);

    VARIANT variant;
    HRESULT hr;

#if DBG
    hr = this->scriptEngine->GetProperty(SCRIPTPROP_INVOKEVERSIONING, nullptr, &variant);
    Assert(SUCCEEDED(hr) && variant.vt == VT_I4 && variant.iVal == SCRIPTLANGUAGEVERSION_5_12);
#endif

    variant.vt = VT_I4;
    variant.lVal = SCRIPTHOSTTYPE_APPLICATION;
    hr = this->scriptEngine->SetProperty(SCRIPTPROP_HOSTTYPE, nullptr, &variant);
    Assert(SUCCEEDED(hr));

    InitSite(runtime);

    SetScriptContext(this->scriptEngine->GetScriptContext());
    Link();
    PinCurrentJsrtContext();

    this->projectionDelegateWrapper = nullptr;
}

/* static */
JsrtContextChakra *JsrtContextChakra::New(JsrtRuntime * runtime)
{
    return RecyclerNewFinalizedLeaf(runtime->GetThreadContext()->EnsureRecycler(), JsrtContextChakra, runtime);
}

void JsrtContextChakra::InitSite(JsrtRuntime *runtime)
{
    scriptSite = HeapNew(JsrtDummyScriptSite);
    JsrtComException::ThrowIfFailed(this->scriptEngine->SetScriptSite(scriptSite));

    if (runtime->GetThreadContext()->NoJIT())
    {
        Js::ScriptContext * scriptContext = this->scriptEngine->GetScriptContext();
        scriptContext->ForceNoNative();
    }

    JsrtComException::ThrowIfFailed(this->scriptEngine->InitNew());
    JsrtComException::ThrowIfFailed(this->scriptEngine->SetScriptState(SCRIPTSTATE_STARTED));

    // Now try to read JS_PROFILER envvar and activate profiling.
    // Even if this fails, we dont throw any error because that will
    // fail the initialization of the session itself. We let it fail
    // queitly.

    IActiveScriptProfilerCallback *pProfileCallback;
    HRESULT hr = ScriptEngine::CheckForExternalProfiler(&pProfileCallback);

    if (SUCCEEDED(hr))
    {
        hr = scriptEngine->GetScriptContext()->RegisterProfileProbe(
            pProfileCallback,
            PROFILER_EVENT_MASK_TRACE_ALL_WITH_DOM,
            0,
            nullptr,
            DispMemberProxy::ProfileInvoke);
        AssertMsg(SUCCEEDED(hr), "Unable to create Profiler");
    }
}

void JsrtContextChakra::Dispose(bool isShutdown)
{
    if (this->scriptEngine != nullptr)
    {
        if (this->scriptSite != nullptr)
        {
            this->scriptSite->Release();
            this->scriptSite = nullptr;
        }

        this->scriptEngine->Close();
        this->scriptEngine->Release();
        this->scriptEngine = nullptr;

        if (this->projectionDelegateWrapper != nullptr)
        {
            this->projectionDelegateWrapper->Release();
            this->projectionDelegateWrapper = nullptr;
        }
        SetScriptContext(nullptr);
        Unlink();
    }
}

bool JsrtContextChakra::SetDebugApplication(IDebugApplication *debugApplication)
{
    return scriptSite->SetDebugApplication(debugApplication);
}

JsErrorCode JsrtContextChakra::SetProjectionDelegateWrapper(_In_ IDelegateWrapper *delegateWrapper)
{
    if (delegateWrapper == nullptr)
    {
        return JsErrorNullArgument;
    }

    if (this->hasProjectionHost)
    {
        return JsErrorInvalidArgument;
    }

    if (this->projectionDelegateWrapper != nullptr)
    {
        return JsErrorInvalidArgument;
    }

    delegateWrapper->AddRef();
    this->projectionDelegateWrapper = delegateWrapper;
    return JsNoError;
}

void JsrtContextChakra::ReleaseDebugApplication()
{
    scriptSite->ReleaseDebugApplication();
}

JsErrorCode JsrtContextChakra::ReserveWinRTNamespace(_In_z_ const wchar_t* nameSpace)
{
    HRESULT hr = NOERROR;
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    IActiveScriptDirect* activeScriptDirect = scriptContext->GetActiveScriptDirect();
    if (activeScriptDirect == nullptr)
    {
        Assert(FALSE);
        return JsErrorCannotStartProjection;
    }
    CComPtr<IActiveScriptProjection> activeScriptProjection = nullptr;
    hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScriptProjection), (void**)&activeScriptProjection);

    if (SUCCEEDED(hr))
    {
        if (!hasProjectionHost)
        {
            CComPtr<IActiveScript> activeScript;
            hr = activeScriptDirect->QueryInterface(&activeScript);
            if (SUCCEEDED(hr))
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = CreateScriptProjectionHost(
                        activeScript,
                        activeScriptProjection,
                        nullptr,  // applicationObjectsToExpose
                        0);       // applicationObjectsToExposeCount

                    if (SUCCEEDED(hr))
                    {
                        // Reset delegate wrapper to ours, even if this->projectionDelegateWrapper == nullptr.
                        CComPtr<IPrivateScriptProjection> privateScriptProjection;
                        hr = activeScriptProjection->QueryInterface(&privateScriptProjection);
                        if (SUCCEEDED(hr))
                        {
                            hr = privateScriptProjection->ResetDelegateWrapper(this->projectionDelegateWrapper);
                        }
                    }
                }
                END_LEAVE_SCRIPT(scriptContext)
            }

            hasProjectionHost = SUCCEEDED(hr);
        }

        if (SUCCEEDED(hr))
        {
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = activeScriptProjection->ReserveNamespace(nameSpace, /*isExtensible*/false);
            }
            END_LEAVE_SCRIPT(scriptContext)
        }

        if (SUCCEEDED(hr))
        {
            // TODO: have a flag to set different jsrt specific features.
            ScriptSite::FromScriptContext(scriptContext)->GetScriptEngine()->GetProjectionContext()->SetIgnoreWebHidden(TRUE);
        }
    }
    if (FAILED(hr))
    {
        return JsErrorCannotStartProjection;
    }
    return JsNoError;
}


void JsrtContextChakra::OnScriptLoad(Js::JavascriptFunction * scriptFunction, Js::Utf8SourceInfo* utf8SourceInfo)
{
    if (scriptFunction != NULL)
    {
        // for telemetry purposes
#ifdef ENABLE_BASIC_TELEMETRY
        if (utf8SourceInfo != nullptr && g_TraceLoggingClient != nullptr)
        {
            JsrtRuntime* runtime = this->GetRuntime();
            if (runtime != nullptr)
            {
                const wchar_t* url = utf8SourceInfo->GetSrcInfo()->sourceContextInfo->url;
                g_TraceLoggingClient->TryLogNodePackage(runtime->GetThreadContext()->GetRecycler(), url);
            }
        }
#endif
        if (this->GetScriptEngine()->CanRegisterDebugSources() || this->GetScriptContext()->IsProfiling())
        {
            ScriptEngine * scriptEngine = this->GetScriptEngine();
            // It is safer to ensure it is deserialized at this point before createing a cscriptbody.
            if (CONFIG_FLAG(ForceSerialized) && scriptFunction->GetFunctionProxy() != null) {
                scriptFunction->GetFunctionProxy()->EnsureDeserialized();
            }
            CScriptBody* pbody = HeapNew(CScriptBody, scriptFunction->GetFunctionBody(), scriptEngine, utf8SourceInfo);
            if (this->GetScriptEngine()->CanRegisterDebugSources())
            {
                HRESULT hr = scriptEngine->DbgRegisterScriptBlock(pbody);
                if (FAILED(hr))
                {
                    pbody->Release();
                    JsrtComException::ThrowIfFailed(hr);
                }
            }

            if (this->GetScriptContext()->IsProfiling())
            {
                this->GetScriptContext()->RegisterScript(pbody->GetRootFunction());
            }
            pbody->Release();
        }
    }
}
