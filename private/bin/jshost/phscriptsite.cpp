/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "WscriptFastDom.h"
#include "SCA.h"
#include "mshtmhst.h"
#include "delegatewrapper.h"
#include "guids.h"
#include "Core\BasePtr.h"
#include "Memory\AutoPtr.h"
#include "hostsysinfo.h"

LPVOID UTF8BoundaryTestBuffer = nullptr;
SimpleSourceMapper *UTF8SourceMapper = nullptr;

struct ByteCodeInfo
{
    byte *byteCodes;
    unsigned long size;
};

static std::map<std::wstring,ByteCodeInfo,std::less<std::wstring>> byteCodeFileMap;

const IID IID_IActiveScriptByteCode =   {0xBF70A42D,0x05C9,0x4858,0xAD,0xCA,0x40,0x73,0x2A,0xF3,0x2C,0xD6}; 

JsFile::JsFile()
{
}

JsFile::~JsFile()
{
    if (m_debugDocumentHelper)
        m_debugDocumentHelper->Detach();
}

DWORD JsHostActiveScriptSite::nextFreeOnShutdownSlot = 0;
LPVOID JsHostActiveScriptSite::freeOnShutdown[JsHostActiveScriptSite::freeOnShutdownCount];
DWORD JsHostActiveScriptSite::nextUnmapOnShutdownSlot = 0;
JsHostActiveScriptSite::MappingInfo JsHostActiveScriptSite::unmapOnShutdown[JsHostActiveScriptSite::freeOnShutdownCount];

JsHostActiveScriptSite::JsHostActiveScriptSite()
    : refCount (1),
      jsHostScriptSiteCookie(0),
      activeScriptCookie(0),
      byteCodeGenCookie(0),
      globalObjectIDispatchExCookie(0),
      javascriptDispatchCookie(0),
      m_WinRTLibrary(NULL),
      m_WinRTStringLibrary(NULL),
      m_WinRTTypeResolutionLibrary(NULL),
      m_dwNextSourceCookie(0),
      m_dwCurrentSourceCookie(0),
      nextDeleteSimpleSourceMappersOnShutDown(0),
      wasClosed(FALSE),
      m_didSourceRundown(false),
      m_isHostInDebugMode(false),
      m_isDiagnosticsScriptSite(false),
      isEvalAllowed(FALSE),
      lastException(nullptr),
      delegateErrorHandling(false),
      activeScript(NULL),
      domainId(0)
{
}

JsHostActiveScriptSite::~JsHostActiveScriptSite()
{
    if (javascriptDispatchCookie)
    {
        CoRevokeClassObject(javascriptDispatchCookie);
    }

    if (jsHostScriptSiteCookie)
    {
        git->RevokeInterfaceFromGlobal(jsHostScriptSiteCookie);
    }

    if (activeScriptCookie)
    {
        git->RevokeInterfaceFromGlobal(activeScriptCookie);
    }

    if (byteCodeGenCookie)
    {
        git->RevokeInterfaceFromGlobal(byteCodeGenCookie);
    }

    if (globalObjectIDispatchExCookie)
    {
        git->RevokeInterfaceFromGlobal(globalObjectIDispatchExCookie);
    }

    if (m_WinRTLibrary)
    {
        delete m_WinRTLibrary;
        m_WinRTLibrary = NULL;
    }

    if (m_WinRTStringLibrary)
    {
        delete m_WinRTStringLibrary;
        m_WinRTStringLibrary = NULL;
    }

    if (m_WinRTTypeResolutionLibrary)
    {
        delete m_WinRTTypeResolutionLibrary;
        m_WinRTTypeResolutionLibrary = nullptr;
    }

    if (this->lastException)
    {
        delete this->lastException;
        this->lastException = nullptr;
    }
}

void JsHostActiveScriptSite::RegisterSimpleSourceMapperToDeleteOnShutdown(SimpleSourceMapper* mapper)
{
    Assert(nextDeleteSimpleSourceMappersOnShutDown < freeOnShutdownCount);
    if (nextDeleteSimpleSourceMappersOnShutDown < freeOnShutdownCount)
    {
        deleteSimpleSourceMappersOnShutDown[nextDeleteSimpleSourceMappersOnShutDown++] = mapper;
    }
}

void JsHostActiveScriptSite::RegisterToFreeOnShutdown(LPVOID mem)
{
    AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);

    Assert(nextFreeOnShutdownSlot<freeOnShutdownCount);
    if (nextFreeOnShutdownSlot<freeOnShutdownCount)
    {
        freeOnShutdown[nextFreeOnShutdownSlot++] = mem;
    }
}

void JsHostActiveScriptSite::RegisterToUnmapOnShutdown(MappingInfo info)
{
    AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);

    Assert(nextUnmapOnShutdownSlot<freeOnShutdownCount);
    if (nextUnmapOnShutdownSlot<freeOnShutdownCount)
    {
        unmapOnShutdown[nextUnmapOnShutdownSlot++] = info;
    }
}

void JsHostActiveScriptSite::RegisterScriptDir(DWORD_PTR sourceContext, char16* const fullDir)
{
    char16 dir[_MAX_PATH];
    scriptDirMap[sourceContext] = std::wstring(GetDir(fullDir, dir));
}

char16* JsHostActiveScriptSite::GetDir(LPCWSTR fullPath, __out char16* const fullDir)
{
    char16 dir[_MAX_DIR];
    _wsplitpath_s(fullPath, fullDir, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0);
    wcscat_s(fullDir, _MAX_PATH, dir);
    return fullDir;
}

HRESULT JsHostActiveScriptSite::CreateScriptEngine(bool isPrimaryEngine)
{
    IActiveScriptParse* activeScriptParse = NULL;
    IClassFactory * jscriptClassFactory = NULL;
    IDispatch * globalObjectDispatch = NULL;
    IDispatchEx * globalObjectDispatchEx = NULL;
    CComPtr<IActiveScriptProperty> activeScriptProperty;
    CComPtr<IActiveScriptDirect> activeScriptDirect;
    HRESULT hr = NOERROR;

    hr = CoInitializeEx(NULL, HostSystemInfo::SupportsOnlyMultiThreadedCOM() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED); 
    if (FAILED(hr))
    {
        return hr;
    }

    if (!jscriptLibrary)
    {
        hr = CoCreateInstance(CLSID_Chakra, NULL, CLSCTX_INPROC_SERVER, _uuidof(IActiveScript), (LPVOID*)&activeScript);
        IfFailedGo(hr);
    }
    else
    {
        if (!JScript9Interface::SupportsDllGetClassObjectCallback())
        {
            hr = E_NOINTERFACE;
            goto LReturn;
        }

        hr = JScript9Interface::DllGetClassObject(CLSID_Chakra, __uuidof(IClassFactory), (LPVOID*)&jscriptClassFactory);
        IfFailedGo(hr);

        hr = jscriptClassFactory->CreateInstance(NULL, _uuidof(IActiveScript), (LPVOID*)&activeScript);
        IfFailedGo(hr);
    }

    hr = activeScript->QueryInterface(__uuidof(IActiveScriptProperty), (LPVOID*)&activeScriptProperty);
    IfFailedGo(hr);

    if (IsDiagnosticsScriptSite())
    {
        hr = activeScriptProperty->SetProperty(SCRIPTPROP_DIAGNOSTICS_OM, nullptr, nullptr);
        IfFailedGo(hr);
    }

    if (!isPrimaryEngine)
    {
        // Indicates whether to make the engine primary or not. For primary engine a full GC occurs when the script site is closed.
        VARIANT var = { 0 };
        var.vt = VT_I4;
        var.lVal = 1;
        hr = activeScriptProperty->SetProperty(SCRIPTPROP_NONPRIMARYENGINE, nullptr, &var);
        IfFailedGo(hr);
    }

    if (HostConfigFlags::flags.EvalRestrictionIsEnabled)
    {
        // This puts the ScriptEngine into eval restricted mode, which means each execution
        // of eval will check for permission by calling IActiveScriptDirectSite::CheckEvalRestriction
        VARIANT varValue;
        varValue.vt = VT_BOOL;
        varValue.boolVal = VARIANT_TRUE; 
        hr = activeScriptProperty->SetProperty(SCRIPTPROP_EVAL_RESTRICTION, nullptr, &varValue);
        IfFailedGo(hr);
        this->isEvalAllowed = HostConfigFlags::flags.EvalIsAllowed;
    }
    else
    {
        AssertMsg(!HostConfigFlags::flags.EvalIsAllowedIsEnabled, "If EvalRestriction is disabled, this flag does noething.");
    }

    hr = activeScript->SetScriptSite(this);
    IfFailedGo(hr);

    hr = activeScript->QueryInterface(__uuidof(IActiveScriptParse), (LPVOID*)&activeScriptParse);
    IfFailedGo(hr);

    hr = activeScriptParse->InitNew();
    IfFailedGo(hr);

    hr = activeScript->SetScriptState(SCRIPTSTATE_STARTED);
    IfFailedGo(hr);

    hr = activeScript->GetScriptDispatch(NULL, &globalObjectDispatch);
    IfFailedGo(hr);

    hr = globalObjectDispatch->QueryInterface(__uuidof(IDispatchEx), (void**)&globalObjectDispatchEx);
    IfFailedGo(hr);

    hr = git->RegisterInterfaceInGlobal(activeScript, IID_IActiveScript, &activeScriptCookie);
    IfFailedGo(hr);

    hr = git->RegisterInterfaceInGlobal(globalObjectDispatchEx, __uuidof(IDispatchEx), &globalObjectIDispatchExCookie);
    IfFailedGo(hr);

    hr = GetActiveScriptDirect(&activeScriptDirect);
    IfFailedGo(hr);

    if (HostConfigFlags::flags.OOPJIT)
    {
        hr = activeScriptDirect->SetJITConnectionInfo(JITProcessManager::GetRpcProccessHandle(), nullptr /*serverSecurityDescriptor*/, JITProcessManager::GetRpcConnectionId());
        IfFailedGo(hr);
    }

LReturn:
    if (FAILED(hr))
    {
        if (activeScript)
        {
            activeScript->SetScriptState(SCRIPTSTATE_CLOSED);
        }
    }

    if (globalObjectDispatchEx)
    {
        globalObjectDispatchEx->Release();
    }

    if (globalObjectDispatch)
    {
        globalObjectDispatch->Release();
    }

    if (activeScriptParse)
    {
        activeScriptParse->Release();
    }

    if (activeScript)
    {
        activeScript->Release();
    }

    if (jscriptClassFactory)
    {
        jscriptClassFactory->Release();
    }

    CoUninitialize();

    return hr;
}

HRESULT JsHostActiveScriptSite::StartScriptProfiler()
{    
    HRESULT hr = S_OK;
    CComPtr<IActiveScriptDirect> activeScriptDirect;
    hr = GetActiveScriptDirect(&activeScriptDirect);
    if (SUCCEEDED(hr))
    {
        CComPtr<ScriptProfiler> profilerObject;
        profilerObject.p = new ScriptProfiler(); 

        hr = JScript9Interface::StartScriptProfiling(activeScriptDirect, (IActiveScriptProfilerCallback *) profilerObject.p, PROFILER_EVENT_MASK_TRACE_ALL_WITH_DOM, JsHostActiveScriptSite::s_profilerCookie);
    }

    return hr;
}

HRESULT JsHostActiveScriptSite::StopScriptProfiler()
{    
    HRESULT hr = S_OK;
    CComPtr<IActiveScriptDirect> activeScriptDirect;
    hr = GetActiveScriptDirect(&activeScriptDirect);
    IfFailedGo(hr);

    hr = JScript9Interface::StopScriptProfiling(activeScriptDirect);
LReturn:
    return hr;
}

HRESULT JsHostActiveScriptSite::GetActiveScriptDirect(IActiveScriptDirect ** activeScriptDirect)
{
    HRESULT hr = S_OK;
    CComPtr<IActiveScript> activeScript;
    hr = GetActiveScript(&activeScript);
    if (SUCCEEDED(hr))
    {
        hr = activeScript->QueryInterface(__uuidof(IActiveScriptDirect), (void**)activeScriptDirect);
    }

    return hr;
}


HRESULT JsHostActiveScriptSite::CheckPerformSourceRundown()
{
    HRESULT hr = S_OK;
    DiagnosticsHelper * diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
    Assert(diagnosticsHelper->m_shouldPerformSourceRundown);

    if (!this->m_didSourceRundown)
    {
        if (!HostConfigFlags::flags.Targeted)
        {
            // Without this, the automatic setting breakpoint will not work.
            HostConfigFlags::flags.Auto = true;
        }

        // Time to attach the debugger to the host dynamically.
        HostConfigFlags::flags.DebugLaunch = SysAllocString(_u(""));

        hr = diagnosticsHelper->InitializeDebugging();
        IfFailedGo(hr);

        CComPtr<IActiveScript> activeScript;
        hr = GetActiveScript(&activeScript);
        IfFailedGo(hr);

        CComPtr<IActiveScriptDirect> activeScriptDirect;

        hr = activeScript->QueryInterface(__uuidof(IActiveScriptDirect), (void**)&activeScriptDirect);
        IfFailedGo(hr);

        CComPtr<IActiveScriptDebugAttach> scriptDynamicAttach;

        hr = activeScript->QueryInterface(__uuidof(IActiveScriptDebugAttach), (void**)&scriptDynamicAttach);
        IfFailedGo(hr);

        std::vector<SourceContextPair> cookiePairs;
        if (HostConfigFlags::flags.HostManagedSource)
        {
            this->RegisterDebugDocuments(activeScript, cookiePairs);
        }

        size_t cCookiePairs = cookiePairs.size();
        AssertMsg(cCookiePairs <= ULONG_MAX, "Dataloss while downcasting size of cookiePairs from size_t to ULONG.");
        ULONG ulCookiePairs = static_cast<ULONG>(cCookiePairs);
        // Now need to do rundown of the source context.
        if (ulCookiePairs > 0)
        {
            SourceContextPair * pairs = new SourceContextPair[ulCookiePairs];
            this->UpdateFileMapTable(cookiePairs, pairs);

            DebuggerController::Log(_u("Performing source rundown with %d cookie pairs.\n"), ulCookiePairs);
            hr = scriptDynamicAttach->PerformSourceRundown(ulCookiePairs, pairs);

            // Now remove the cookie pairs 
            delete [] pairs;
        }
        else
        {
            DebuggerController::Log(_u("Performing source rundown with no cookie pairs.\n"));
            hr = scriptDynamicAttach->PerformSourceRundown(0, NULL);
        }

        Assert(SUCCEEDED(hr));

        m_didSourceRundown = true;
    }

LReturn:
    return hr;
}

HRESULT JsHostActiveScriptSite::CheckEvalRestriction()
{
    return this->isEvalAllowed ? S_OK : E_ACCESSDENIED;
}

HRESULT JsHostActiveScriptSite::HostExceptionFromHRESULT(HRESULT hrParam, Var *outError)
{
    HRESULT hr = S_OK;
    if (hrParam != E_ACCESSDENIED)
    {
        return E_INVALIDARG;
    }

    if (outError != nullptr)
    {

        CComPtr<IActiveScriptDirect> activeScriptDirect;
        hr = activeScript->QueryInterface(__uuidof(IActiveScriptDirect), (void **)&activeScriptDirect);
        IfFailedGo(hr);

        return activeScriptDirect->CreateErrorObject(JsErrorType::JavascriptError, E_ACCESSDENIED, _u("Security Error"), outError);
    }

LReturn:
    return hr;
}

HRESULT JsHostActiveScriptSite::RegisterDebugDocuments(CComPtr<IActiveScript>& activeScript, std::vector<SourceContextPair>& cookiePairs)
{
    // Now walk thru currently available document and put them in the debug mode.
    HRESULT hr = S_OK;
    std::map<DWORD_PTR,JsFile>::iterator it;
    for (it = m_fileMap.begin(); it != m_fileMap.end(); ++it)
    {
        if ((*it).second.m_debugDocumentHelper == NULL && (*it).second.m_sourceContent.c_str() != nullptr)
        {
            CComPtr<IDebugDocumentHelper> debugDocumentHelper;

            DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();

            hr = diagnosticsHelper->CreateDocumentHelper(&debugDocumentHelper);
            IfFailedGo(hr);

            TCHAR szShortName[255];
            GetShortNameFromUrl((*it).second.m_fileName.c_str(), szShortName, 255);

            hr = debugDocumentHelper->Init(diagnosticsHelper->m_debugApplication, szShortName,  (*it).second.m_fileName.c_str(), TEXT_DOC_ATTR_READONLY);
            IfFailedGo(hr);

            // Attach to NULL says it's a global document
            hr = debugDocumentHelper->Attach(NULL);
            IfFailedGo(hr);

            // right now it is stored all the time.
            debugDocumentHelper->AddUnicodeText((*it).second.m_sourceContent.c_str());
            size_t ctCharsScript = (*it).second.m_sourceContent.length();
            AssertMsg(ctCharsScript <= ULONG_MAX, "Data loss while downcasting ctCharsScript from size_t to ULONG.");
            DWORD_PTR dwCookie;
            // The script is the entire file .. unlike html where a script block can be a subset.
            hr = debugDocumentHelper->DefineScriptBlock(0, static_cast<ULONG>(ctCharsScript), activeScript, FALSE, &dwCookie);
            IfFailedGo(hr);

            (*it).second.m_debugDocumentHelper = debugDocumentHelper;
            SourceContextPair pair;
            pair.dwHostSourceContext = (*it).first;
            pair.ulCharOffset = 0;
            pair.dwDebugHostSourceContext = dwCookie;

            cookiePairs.push_back(pair);
        }
    }

LReturn:
    return hr;
}

void JsHostActiveScriptSite::UpdateFileMapTable(std::vector<SourceContextPair>& cookiePairs, SourceContextPair* outPairArray)
{
    std::vector<SourceContextPair>::iterator it;
    int i = 0;
    for (it = cookiePairs.begin(); it != cookiePairs.end(); ++it, i++)
    {
        outPairArray[i] = (*it);

        // Update the map table, so that it can debug context.
        m_fileMap[outPairArray->dwDebugHostSourceContext] = m_fileMap[outPairArray->dwHostSourceContext];
    }
}


HRESULT JsHostActiveScriptSite::CheckDynamicAttach()
{
    HRESULT hr = S_OK;
    if (this->m_isDiagnosticsScriptSite)
    {
        // Not exercising the Debug attach.
        return hr;
    }
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();

    {
        if (!HostConfigFlags::flags.Targeted)
        {
            // Without this, the automatic setting breakpoint will not work.
            HostConfigFlags::flags.Auto = true;
        }

        // Time to attach the debugger to the host dynamically.
        HostConfigFlags::flags.DebugLaunch = SysAllocString(_u(""));

        hr = diagnosticsHelper->InitializeDebugging();
        IfFailedGo(hr);


        CComPtr<IActiveScript> activeScript;
        hr = GetActiveScript(&activeScript);
        IfFailedGo(hr);

        CComPtr<IActiveScriptDebugAttach> scriptDynamicAttach;

        hr = activeScript->QueryInterface(__uuidof(IActiveScriptDebugAttach), (void**)&scriptDynamicAttach);
        IfFailedGo(hr);

        std::vector<SourceContextPair> cookiePairs;
        if (!m_didSourceRundown && HostConfigFlags::flags.HostManagedSource)
        {
            this->RegisterDebugDocuments(activeScript, cookiePairs);
        }

        size_t cCookiePairs = cookiePairs.size();
        AssertMsg(cCookiePairs <= ULONG_MAX, "Dataloss while downcasting size of cookiePairs from size_t to ULONG.");
        ULONG ulCookiePairs = static_cast<ULONG>(cCookiePairs);
        if(!m_isHostInDebugMode)
        {
            m_isHostInDebugMode = true;
            // Now need to do rundown of the source context.
            if (ulCookiePairs > 0)
            {
                SourceContextPair * pairs = new SourceContextPair[ulCookiePairs];
                this->UpdateFileMapTable(cookiePairs, pairs);

                DebuggerController::Log(_u("Attaching the debugger with %d cookie pairs.\n"), ulCookiePairs);
                hr = scriptDynamicAttach->OnDebuggerAttached(ulCookiePairs, pairs);
                m_didSourceRundown = true;

                // Now remove the cookie pairs 
                delete [] pairs;
            }
            else
            {
                DebuggerController::Log(_u("Performing attach with no cookie pairs.\n"));
                hr = scriptDynamicAttach->OnDebuggerAttached(0, NULL);
            }


            IfFailedGo_NoLog(hr);

            Assert(diagnosticsHelper->m_debugger);
            diagnosticsHelper->m_debugger->OnDebuggerAttachedCompleted();
        }
    }
LReturn:
    return hr;
}

HRESULT JsHostActiveScriptSite::CheckDynamicDetach()
{
    HRESULT hr = S_OK;
    if (this->m_isDiagnosticsScriptSite)
    {
        // Not exercising the Debug Detach.
        return hr;
    }

    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();

    CComPtr<IActiveScript> activeScript;
    hr = GetActiveScript(&activeScript);
    IfFailedGo(hr);

    if(m_isHostInDebugMode)
    {
        m_isHostInDebugMode = false;

        CComPtr<IActiveScriptDebugAttach> scriptDynamicAttach;

        hr = activeScript->QueryInterface(__uuidof(IActiveScriptDebugAttach), (void**)&scriptDynamicAttach);
        IfFailedGo(hr);

        // Detach the debugger.
        DebuggerController::Log(_u("Detaching the debugger.\n"));
        hr = scriptDynamicAttach->OnDebuggerDetached();
        IfFailedGo_NoLog(hr);

        // Turn off setting of breakpoints in the debugger since we're no longer in debug mode.
        Assert(diagnosticsHelper->m_debugger);
        diagnosticsHelper->m_debugger->OnDebuggerDetachedCompleted();
    }

LReturn:
    return hr;
}

HRESULT JsHostActiveScriptSite::InitializeDebugDocument(LPCWSTR scriptCode, BSTR filename)
{
    HRESULT hr = S_OK;
    CComPtr<IActiveScript> activeScript;
    CComPtr<IDebugDocumentHelper> debugDocumentHelper;
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();

    hr = diagnosticsHelper->CreateDocumentHelper(&debugDocumentHelper);
    IfFailedGo(hr);

    TCHAR szShortName[255];
    GetShortNameFromUrl(filename, szShortName, 255);

    hr = debugDocumentHelper->Init(diagnosticsHelper->m_debugApplication, szShortName,  filename, TEXT_DOC_ATTR_READONLY);
    IfFailedGo(hr);

    // Attach to NULL says it's a global document
    hr = debugDocumentHelper->Attach(NULL);
    IfFailedGo(hr);

    int ctCharsScript = (int) wcslen(scriptCode);

    hr = GetActiveScript(&activeScript);
    IfFailedGo(hr);

    // The script is the entire file .. unlike html where a script block can be a subset.
    hr = debugDocumentHelper->DefineScriptBlock(0,ctCharsScript,activeScript, FALSE, &m_dwNextSourceCookie);
    IfFailedGo(hr);

    m_dwCurrentSourceCookie = m_dwNextSourceCookie;
    m_fileMap[m_dwNextSourceCookie].m_sourceContent = scriptCode; // This will be used in the OnEnterScript
    m_fileMap[m_dwNextSourceCookie].m_debugDocumentHelper = debugDocumentHelper;
    m_fileMap[m_dwNextSourceCookie].m_fileName = filename;

LReturn:
    return hr;
}

HRESULT JsHostActiveScriptSite::RegisterCrossThreadInterface()
{
    if (javascriptDispatchCookie != 0)
    {
        return S_FALSE;
    }

    HRESULT hr = S_OK;

    IUnknown * punk = NULL;
    hr = JScript9Interface::DllGetClassObject(IID_IJavascriptDispatchRemoteProxy, IID_IUnknown, (void **)&punk);
    if (SUCCEEDED(hr))
    {
        hr = CoRegisterClassObject(IID_IJavascriptDispatchRemoteProxy, punk, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &javascriptDispatchCookie);
        if (SUCCEEDED(hr))
        {
            hr = CoRegisterPSClsid(IID_IJavascriptDispatchRemoteProxy, IID_IJavascriptDispatchRemoteProxy);
        }
        punk->Release();
    }

    return hr;
}

HRESULT JsHostActiveScriptSite::InitializeProjection()
{
    HRESULT hr = S_OK;
    bool projectionHostSet = false;

    IActiveScriptProjection* activeScriptProjection = NULL;
    IActiveScriptDirect * activeScriptDirect = NULL;

    ActiveScriptDirectHost* host = new ActiveScriptDirectHost();

    IActiveScript * activeScript;
    hr = GetActiveScript(&activeScript);
    IfFailedGo(hr);

    hr = activeScript->QueryInterface(__uuidof(IActiveScriptDirect), (void**)&activeScriptDirect);
    IfFailedGo(hr);

    // Setup the ABI projection
    hr = activeScript->QueryInterface(__uuidof(IActiveScriptProjection), (LPVOID*)&activeScriptProjection);
    IfFailedGo(hr);

    DelegateWrapper* delegateWrapper = nullptr;
    if (HostConfigFlags::flags.EnableDelegateWrapper)
    {
        delegateWrapper = new DelegateWrapper();
    }
    // JsHost will always pass version NTDDI_WIN8. You can use the -TargetWinRTVersion flag to specify a different version.
    hr = activeScriptProjection->SetProjectionHost(host, HostConfigFlags::flags.Configurable, NTDDI_WIN8, delegateWrapper);
    IfFailedGo(hr);
    projectionHostSet = true;

    // Reserve the namespaces that we know about
    hr = activeScriptProjection->ReserveNamespace(_u("JSTest"), false);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    hr = activeScriptProjection->ReserveNamespace(_u("Windows.Devices.Printers"), false);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    hr = activeScriptProjection->ReserveNamespace(_u("Animal"), false);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    hr = activeScriptProjection->ReserveNamespace(_u("Animals"), false);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    hr = activeScriptProjection->ReserveNamespace(_u("Fabrikam.Kitchen"), false);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    hr = activeScriptProjection->ReserveNamespace(_u("BPTTest"), false);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    hr = activeScriptProjection->ReserveNamespace(_u("Winery"), false);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    hr = activeScriptProjection->ReserveNamespace(_u("DevTests"), true);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    hr = activeScriptProjection->ReserveNamespace(_u("MetadataError"), false);
    if (hr == E_INVALIDARG) 
    {
        // dup reserve namespace registration override - can happen when multiple WScript.LoadScriptFile present
        hr = S_OK;
    }
    IfFailedGo(hr);

    m_WinRTLibrary = new DelayLoadWinRt();
    m_WinRTStringLibrary = new Js::DelayLoadWinRtString();
    m_WinRTTypeResolutionLibrary = new DelayLoadWinRtTypeResolution();

    TestUtilities::Initialize(activeScriptDirect, m_WinRTLibrary, m_WinRTStringLibrary);

LReturn:
    if (activeScriptProjection)
    {
        activeScriptProjection->Release();
    }

    if (host)
    {
        if (projectionHostSet)
        {
            host->Release();
        }
        if (FAILED(hr))
        {
            host->Close();
        }
    }

    if (activeScriptDirect)
    {
        activeScriptDirect->Release();
    }

    if (activeScript)
    {
        activeScript->Release();
    }

    return hr;
}

HRESULT JsHostActiveScriptSite::StopScriptEngine()
{
    HRESULT hr = S_OK;

    IActiveScript * activeScript;
    hr = GetActiveScript(&activeScript);
    if (SUCCEEDED(hr))
    {
        SCRIPTSTATE ss;
        hr = activeScript->GetScriptState(&ss);
        if (SUCCEEDED(hr) && ss != SCRIPTSTATE_CLOSED)
        {
            hr = activeScript->SetScriptState(SCRIPTSTATE_CLOSED);
        }

        activeScript->Release();
    }

    return hr;
}

struct WriteByteCodesProcParam
{
    JsHostActiveScriptSite * scriptSite;
    HRESULT hrResult;
    DWORD lengthBytes;
    BYTE * contentsRaw;
    DWORD_PTR dwSourceCookie;
    LPCWSTR bcFullPath;
    HANDLE completeEvent;
};

VOID CALLBACK WriteByteCodesProc(ULONG_PTR param)
{
    auto parameters = (WriteByteCodesProcParam*)param;
    auto scriptSite = parameters->scriptSite;
    auto lengthBytes = parameters->lengthBytes;
    auto contentsRaw = parameters->contentsRaw;
    auto dwSourceCookie = parameters->dwSourceCookie;
    auto bcFullPath = parameters->bcFullPath;

    IActiveScript * activeScript = nullptr;
    IActiveScriptByteCode * byteCodeGen = nullptr;
    HANDLE fileHandle = nullptr;
    byte * buffer = nullptr;
    HRESULT hr = S_OK;
    DWORD bufferSize = 1;

    auto shutDown = [&]() {
        if (fileHandle) CloseHandle(fileHandle);
        if (buffer) CoTaskMemFree(buffer);
        if (byteCodeGen) byteCodeGen->Release();
        if (activeScript) activeScript->Release();
        if (scriptSite) scriptSite->Shutdown(scriptSite);
        if (scriptSite) scriptSite->Release();
        parameters->hrResult = hr;
        if (parameters->completeEvent)
        {
            SetEvent(parameters->completeEvent);
        }
    };

    hr = scriptSite->GetActiveScript(&activeScript);
    if (SUCCEEDED(hr))
    {
        hr = activeScript->QueryInterface(IID_IActiveScriptByteCode, (void**)&byteCodeGen);
        if (SUCCEEDED(hr))
        {
            // Create byte code file
            EXCEPINFO excepinfo = {0};

            hr = byteCodeGen->GenerateByteCodeBuffer(lengthBytes, (BYTE*)contentsRaw, nullptr, dwSourceCookie, &excepinfo, &buffer, &bufferSize);
            if (SUCCEEDED(hr))
            {
                DWORD written = 0;
                fileHandle = CreateFile(bcFullPath, GENERIC_WRITE, FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                auto writeSuccess = fileHandle != INVALID_HANDLE_VALUE ? WriteFile(fileHandle, buffer, bufferSize, &written, nullptr) : FALSE;
                if (!writeSuccess || written!=bufferSize)
                {
                    CloseHandle(fileHandle);
                    hr = E_FAIL;
                }
            }
        }
    }
    return shutDown();
}

HRESULT WriteByteCodes(HANDLE thread, JsHostActiveScriptSite * scriptSite, DWORD lengthBytes, BYTE * contentsRaw, DWORD_PTR dwSourceCookie, LPCWSTR bcFullPath)
{
    HRESULT hr = S_OK;

    DWORD threadId = GetThreadId(thread);
    if (!threadId)
    {
        hr = E_FAIL;
    }
    if (SUCCEEDED(hr))
    {
        if (threadId == GetCurrentThreadId())
        {
            Assert(0);
        }
        else
        {
            WriteByteCodesProcParam param;
            // Create the event that will be signalled when the APC is done
            param.completeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            param.scriptSite = scriptSite;
            param.lengthBytes = lengthBytes;
            param.contentsRaw = contentsRaw;
            param.dwSourceCookie = dwSourceCookie;
            param.bcFullPath = bcFullPath;
            param.hrResult = S_OK;

            if (!param.completeEvent)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
            {
                // Queue an APC to the target thread
                if (!QueueUserAPC(WriteByteCodesProc, thread, (ULONG_PTR)&param))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
                else
                {
                    switch (WaitForSingleObject(param.completeEvent, INFINITE))
                    {
                    case WAIT_OBJECT_0:
                        break;

                    case WAIT_FAILED:
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        break;
                    }
                }

                CloseHandle(param.completeEvent);
            }
            return param.hrResult;
        }
    }

    return hr;
}

#include "GenerateByteCodeConfig.h"
#include "..\..\..\core\lib\Runtime\ByteCode\ByteCodeSerializeFlags.h"
static CGenerateByteCodeConfig s_generateLibraryByteCodeHeaderConfig(GENERATE_BYTE_CODE_BUFFER_LIBRARY);
HRESULT PerformUTF8BoundaryTest(JsHostActiveScriptSite *scriptSite, DWORD lengthInBytes, BYTE *contentsRaw, DWORD_PTR dwSourceCookie)
{
    CComPtr<IActiveScript> activeScript(nullptr);
    CComPtr<IActiveScriptByteCode> byteCodeGen(nullptr);
    byte * byteCodeBuffer = nullptr;
    HRESULT hr = S_OK;
    DWORD byteCodeBufferSize = 1;
    EXCEPINFO excepinfo = {0};
    
    int bytesToOffsetBy = 4096 - (lengthInBytes % 4096);
    bytesToOffsetBy = bytesToOffsetBy == 4096 ? 0  : bytesToOffsetBy;
    int commitSize = lengthInBytes  + bytesToOffsetBy;
    int virtualAllocationSize = commitSize + 4096;
    
    IfFailGo(scriptSite->GetActiveScript(&activeScript));
    IfFailGo(activeScript->QueryInterface(IID_IActiveScriptByteCode, (void**)&byteCodeGen));

    UTF8BoundaryTestBuffer = VirtualAlloc(nullptr, virtualAllocationSize, MEM_RESERVE, PAGE_NOACCESS);
    UTF8BoundaryTestBuffer = VirtualAlloc(UTF8BoundaryTestBuffer, commitSize, MEM_COMMIT, PAGE_READWRITE);
    BYTE* offsetBuffer = (BYTE*)UTF8BoundaryTestBuffer + bytesToOffsetBy;
    memcpy_s(offsetBuffer, lengthInBytes, contentsRaw, lengthInBytes);

    UTF8SourceMapper = new SimpleSourceMapper();
    UTF8SourceMapper->Initialize(offsetBuffer, lengthInBytes);

    IfFailGo(byteCodeGen->GenerateByteCodeBuffer(lengthInBytes, (BYTE*)contentsRaw, nullptr, dwSourceCookie, &excepinfo, &byteCodeBuffer, &byteCodeBufferSize));
    IfFailGo(byteCodeGen->ExecuteByteCodeBuffer(byteCodeBufferSize, byteCodeBuffer, UTF8SourceMapper, nullptr, dwSourceCookie, &excepinfo));
        
Error:
    return hr;
}

HRESULT GenerateLibraryByteCodeHeader(JsHostActiveScriptSite * scriptSite, DWORD lengthBytes, BYTE * contentsRaw, DWORD_PTR dwSourceCookie, LPCWSTR bcFullPath, LPCWSTR libraryNameWide)
{
    IActiveScript * activeScript = nullptr;
    IActiveScriptByteCode * byteCodeGen = nullptr;
    HANDLE fileHandle = nullptr;
    byte * buffer = nullptr;
    HRESULT hr = S_OK;
    DWORD bufferSize = 1;
    EXCEPINFO excepinfo = {0};

    IfFailGo(scriptSite->GetActiveScript(&activeScript));
    IfFailGo(activeScript->QueryInterface(IID_IActiveScriptByteCode, (void**)&byteCodeGen));
    // Create byte code file    
    IfFailGo(byteCodeGen->GenerateByteCodeBuffer(lengthBytes, (BYTE*)contentsRaw, &s_generateLibraryByteCodeHeaderConfig, dwSourceCookie, &excepinfo, &buffer, &bufferSize));
    fileHandle = CreateFile(bcFullPath, GENERIC_WRITE, FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) IfFailGo(E_FAIL);

    DWORD written;

    // For validating the header file against the library file
    // We want to keep the MIT notice here because Intl.js ByteCode can, in theory, be produced using either ch.exe or jshost.exe.
    // No matter how it is generated, we want to make sure that the MIT License is definitely present in case we commit the result to Git.
    auto outputStr =
        "//-------------------------------------------------------------------------------------------------------\r\n"
        "// Copyright (C) Microsoft. All rights reserved.\r\n"
        "// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.\r\n"
        "//-------------------------------------------------------------------------------------------------------\r\n"
        "#if 0\r\n";
    if (!WriteFile(fileHandle, outputStr, static_cast<DWORD>(strlen(outputStr)), &written, nullptr)) IfFailGo(E_FAIL);
    if (!WriteFile(fileHandle, contentsRaw, lengthBytes, &written, nullptr)) IfFailGo(E_FAIL);
    if (lengthBytes < 2 || contentsRaw[lengthBytes - 2] != '\r' || contentsRaw[lengthBytes - 1] != '\n')
    {
        outputStr = "\r\n#endif\r\n";
    }
    else
    {
        outputStr = "#endif\r\n";
    }
    if (!WriteFile(fileHandle, outputStr, static_cast<DWORD>(strlen(outputStr)), &written, nullptr)) IfFailGo(E_FAIL);

    // Write out the bytecode
    outputStr = "namespace Js\r\n{\r\n    const char Library_Bytecode_";
    if (!WriteFile(fileHandle, outputStr, static_cast<DWORD>(strlen(outputStr)), &written, nullptr)) IfFailGo(E_FAIL);
    size_t convertedChars;
    char libraryNameNarrow[MAX_PATH + 1];
    if (wcstombs_s(&convertedChars, libraryNameNarrow, libraryNameWide, _TRUNCATE) != 0) IfFailGo(E_FAIL);
    if (!WriteFile(fileHandle, libraryNameNarrow, static_cast<DWORD>(strlen(libraryNameNarrow)), &written, nullptr)) IfFailGo(E_FAIL);
    outputStr = "[] = {\r\n/* 00000000 */";
    if (!WriteFile(fileHandle, outputStr, static_cast<DWORD>(strlen(outputStr)), &written, nullptr)) IfFailGo(E_FAIL);

    for (unsigned int i = 0; i < bufferSize; i++)
    {
        const DWORD scratchLen = 6;
        const DWORD commaSpaceLen = 2;
        const DWORD offsetLen = 17;
        char scratch[scratchLen];
        int num = _snprintf_s(scratch, scratchLen, " 0x%02X", buffer[i]);
        Assert(num == 5);
        if (!WriteFile(fileHandle, scratch, scratchLen - 1, &written, nullptr)) IfFailGo(E_FAIL);

        //Add a comma and a space if this is not the last item
        if (i < bufferSize - 1)
        {
            char commaSpace[commaSpaceLen];
            _snprintf_s(commaSpace, commaSpaceLen, ",");  // close quote, new line, offset and open quote
            if (!WriteFile(fileHandle, commaSpace, commaSpaceLen - 1, &written, nullptr)) IfFailGo(E_FAIL);
        }

        //Add a line break every 16 scratches, primarily so the compiler doesn't complain about the string being too long.
        //Also, won't add for the last scratch
        if (i % 16 == 15 && i < bufferSize - 1)
        {
            char offset[offsetLen];
            _snprintf_s(offset, offsetLen, "\r\n/* %08X */", i + 1);  // close quote, new line, offset and open quote
            if (!WriteFile(fileHandle, offset, offsetLen - 1, &written, nullptr)) IfFailGo(E_FAIL);
        }
    }
    outputStr = "};\r\n\r\n";
    if (!WriteFile(fileHandle, outputStr, static_cast<DWORD>(strlen(outputStr)), &written, nullptr)) IfFailGo(E_FAIL);

    outputStr = "}\r\n";
    if (!WriteFile(fileHandle, outputStr, static_cast<DWORD>(strlen(outputStr)), &written, nullptr)) IfFailGo(E_FAIL);

Error:
    if (fileHandle) CloseHandle(fileHandle);
    if (buffer) CoTaskMemFree(buffer);
    if (byteCodeGen) byteCodeGen->Release();
    if (activeScript) activeScript->Release();
    return hr;
}

HRESULT JsHostActiveScriptSite::LoadScriptFromFile(LPCWSTR scriptFilename, void** errorObject, bool isModuleCode)
{
    HRESULT hr = S_OK;
    LPCOLESTR contents = NULL;
    bool isUtf8 = false;
    LPCOLESTR contentsRaw = NULL;
    UINT lengthBytes = 0;
    bool usedUtf8 = false; // If we have used utf8 buffer (contentsRaw) to parse code, the buffer will be owned by script engine. Do not free it.
    char16 fullpath[_MAX_PATH];
    bool contentsFromModuleSourceMap = false;
    bool printFileOpenError = !HostConfigFlags::flags.MuteHostErrorMsgIsEnabled;

    auto moduleFromSourceMap = moduleSourceMap.find(scriptFilename);

    if (scriptFilename == nullptr)
    {
        scriptFilename = _u("script.js");
    }

    if (_wfullpath(fullpath, scriptFilename, _MAX_PATH) == nullptr)
    {
        fwprintf(stderr, _u("Out of memory"));
        IfFailGo(E_OUTOFMEMORY);
    }

    size_t len = wcslen(fullpath);
    if (isModuleCode && moduleFromSourceMap != moduleSourceMap.end())
    {
        // This is our string and it's not UTF8
        // Leave isUtf8 false, contentsRaw nullptr, and lengthBytes 0
        // TODO: Support different encodings?
        contents = moduleFromSourceMap->second.c_str();
        contentsFromModuleSourceMap = true;
    }
    else
    {
        RegisterScriptDir(m_dwNextSourceCookie, fullpath);
        hr = JsHostLoadScriptFromFile(fullpath, contents, &isUtf8, &contentsRaw, &lengthBytes, printFileOpenError);
        IfFailGo(hr);
    }
   
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
    if (HostConfigFlags::flags.HostManagedSource && (HostConfigFlags::flags.EnableDebug || diagnosticsHelper->m_debugger != NULL))
    {
        if (len != static_cast<int>(len))
        {
            IfFailGo(ERROR_BAD_PATHNAME);
        }
        CComBSTR bstr(static_cast<int>(len), fullpath);
        IfFailGo(InitializeDebugDocument(contents, bstr));
    }
    else
    {
        m_fileMap[m_dwNextSourceCookie].m_fileName.assign(fullpath, len);
        if (HostConfigFlags::flags.HostManagedSource)
        {
            m_fileMap[m_dwNextSourceCookie].m_sourceContent = ::SysAllocString(contents);
        }
    }

    if (isModuleCode)
    {
        if (isUtf8)
        {
            hr = LoadModuleFromString(isUtf8, scriptFilename, (UINT)wcslen(scriptFilename), contentsRaw, lengthBytes, errorObject, fullpath);
        }
        else
        {
            hr = LoadModuleFromString(isUtf8, scriptFilename, (UINT)wcslen(scriptFilename), contents, (UINT)wcslen(contents)*sizeof(char16), errorObject, fullpath);
        }
        goto Error;
    }

    if (isUtf8 && HostConfigFlags::flags.PerformUTF8BoundaryTestIsEnabled)
    {
        DWORD_PTR dwSourceCookie = this->AddUrl(fullpath);
        hr = PerformUTF8BoundaryTest(this, lengthBytes, (BYTE*)contentsRaw, dwSourceCookie);
    }
    else if (isUtf8 && HostConfigFlags::flags.GenerateLibraryByteCodeHeaderIsEnabled)
    {
        char16 * bcFullPath = NULL;
        WCHAR filename[_MAX_PATH];
        WCHAR ext[_MAX_EXT];
        _wsplitpath_s(fullpath, NULL, 0, NULL, 0, filename, _countof(filename), ext, _countof(ext));
        if (HostConfigFlags::flags.GenerateLibraryByteCodeHeader != NULL && *HostConfigFlags::flags.GenerateLibraryByteCodeHeader != _u('\0'))
        {
            bcFullPath = HostConfigFlags::flags.GenerateLibraryByteCodeHeader;
        }
        else
        {
#if _M_AMD64
            auto fnAppend = _u(".bc.64b.h");
#else
            auto fnAppend = _u(".bc.32b.h");
#endif
            auto bcFilenameLength = wcslen(fullpath) + wcslen(fnAppend); 
            bcFullPath = new char16[bcFilenameLength + 1];
            wcscpy_s(bcFullPath, bcFilenameLength + 1, fullpath);
            wcscat_s(bcFullPath, bcFilenameLength + 1, fnAppend); 
        }
        DWORD_PTR dwSourceCookie = this->AddUrl(fullpath);
        hr = GenerateLibraryByteCodeHeader(this, lengthBytes, (BYTE*)contentsRaw, dwSourceCookie, bcFullPath, filename);
        if (bcFullPath != HostConfigFlags::flags.GenerateLibraryByteCodeHeader)
        {
            delete [] bcFullPath;
        }
    }
    else if (isUtf8 && HostConfigFlags::flags.SerializedIsEnabled)
    {        
        // Byte code creation and consumption
        char16 bcFullPath[MAX_PATH];
        char16 filename[MAX_PATH];
        char16 combinedName[MAX_PATH];
        _wsplitpath_s(fullpath, nullptr, 0, nullptr, 0, filename, MAX_PATH, nullptr, 0);
        wcscpy_s(combinedName, MAX_PATH, HostConfigFlags::flags.Serialized);
        if (HostConfigFlags::flags.Serialized[0] != '\0')
        {
            wcscat_s(combinedName, MAX_PATH, _u("."));
        }
        wcscat_s(combinedName, MAX_PATH, filename);
        _wmakepath_s(bcFullPath, MAX_PATH, nullptr, nullptr, combinedName, _u("bc"));

        auto byteCodeMapEntry = byteCodeFileMap.find(bcFullPath);
        byte *byteCodes = nullptr;
        unsigned long fileSize = 0;

        if (byteCodeMapEntry != byteCodeFileMap.end())
        {
            byteCodes = byteCodeMapEntry->second.byteCodes;
            fileSize = byteCodeMapEntry->second.size;
        }

        if (HostConfigFlags::flags.RecreateByteCodeFile && byteCodes == nullptr)
        {
            HANDLE newThread;
            HANDLE terminateEvent;
            hr = CreateEngineThread(&newThread, &terminateEvent);
            if (SUCCEEDED(hr))
            {
                JsHostActiveScriptSite * scriptSite = NULL;

                // save debugging state and restore it - when doing bytecode generation we need to make sure we are not in debug mode
                BSTR debugLaunch = HostConfigFlags::flags.DebugLaunch;
                bool enableDebug = HostConfigFlags::flags.EnableDebug;
                HostConfigFlags::flags.DebugLaunch = NULL;
                HostConfigFlags::flags.EnableDebug = false;
                // This engine will be freed earlier than shutdown to ensure there is no sharing memory between threadcontexts
                hr = CreateNewEngine(newThread, &scriptSite, /*freeAtShutdown*/false, false/*actAsDiagnosticsHost*/, true /* always primary engine */, 0);
                
                if (SUCCEEDED(hr))
                {
                    DWORD_PTR dwSourceCookie = scriptSite->AddUrl(fullpath);
                    Assert(isUtf8);
                    hr = WriteByteCodes(newThread, scriptSite, lengthBytes, (BYTE*)contentsRaw, dwSourceCookie, bcFullPath);
                }
                SetEvent(terminateEvent);
                WaitForMultipleObjects(1, &newThread, FALSE, INFINITE);
                HostConfigFlags::flags.DebugLaunch = debugLaunch;
                HostConfigFlags::flags.EnableDebug = enableDebug;
            }
        }
        
        if (hr == SCRIPT_E_CANT_GENERATE)
        {
            fwprintf(stderr, _u("WARNING: Unable to generate byte code cache for %s\n"), filename);
            fflush(stderr);
            hr = LoadScriptFromString(contents, isUtf8 ? (BYTE*)contentsRaw : nullptr, lengthBytes, &usedUtf8);
        }
        else if (SUCCEEDED(hr))
        {
            // Open and map the byte code file.
            IActiveScript * activeScript = NULL;
            DWORD_PTR dwSourceCookie = m_dwNextSourceCookie++;

            hr = GetActiveScript(&activeScript);

            if (SUCCEEDED(hr))
            {
                IActiveScriptByteCode * byteCodeGen = NULL;
                hr = activeScript->QueryInterface(IID_IActiveScriptByteCode, (LPVOID*)&byteCodeGen);
                activeScript->Release();

                if (SUCCEEDED(hr) && byteCodes == nullptr)
                {
                    hr = E_FAIL;

                    auto handle = CreateFile(bcFullPath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (handle == INVALID_HANDLE_VALUE)
                    {
                        fwprintf(stderr, _u("ERROR: Unable to open byte code cache file %s\n"), bcFullPath);
                        fflush(stderr);
                    }
                    else
                    {
                        fileSize = GetFileSize(handle, nullptr);
                        auto mapping = CreateFileMapping(handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
                        if (mapping == NULL)
                        {
                            fwprintf(stderr, _u("ERROR: Unable to create file mapping for byte code cache file %s\n"), bcFullPath);
                            fflush(stderr);
                        }
                        else
                        {
                            byteCodes = (byte*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);

                            MappingInfo info = {handle, mapping, byteCodes};
                            RegisterToUnmapOnShutdown(info);

                            ByteCodeInfo byteCodeInfo;
                            byteCodeInfo.byteCodes = byteCodes;
                            byteCodeInfo.size = fileSize;
                            byteCodeFileMap[bcFullPath] = byteCodeInfo;
                        }
                    }
                }

                if (byteCodes == nullptr)
                {
                    fwprintf(stderr, _u("ERROR: Unable to map byte code cache file %s\n"), bcFullPath);
                    fflush(stderr);
                }
                else
                {
                    SimpleSourceMapper *sourceMapper = new SimpleSourceMapper ();
                    sourceMapper->Initialize((BYTE*)contentsRaw, lengthBytes);

                    EXCEPINFO excepinfo;
                    hr = byteCodeGen->ExecuteByteCodeBuffer(fileSize, byteCodes, sourceMapper, nullptr, dwSourceCookie, &excepinfo);
                    RegisterToFreeOnShutdown((void*)contentsRaw);
                    RegisterSimpleSourceMapperToDeleteOnShutdown(sourceMapper);
                    contentsRaw = nullptr;
                    byteCodeGen->Release();
                }
            }
        }
        else if (hr != SCRIPT_E_REPORTED)
        {
            fwprintf(stderr, _u("ERROR: Unexpected error: 0x%x\n"), hr);
            fflush(stderr);
        }
    }
    else
    {
        hr = LoadScriptFromString(contents, isUtf8 ? (BYTE*)contentsRaw : nullptr, lengthBytes, &usedUtf8, fullpath);
    }

Error:
    if (contentsRaw && !usedUtf8)
    {
        HeapFree(GetProcessHeap(), 0, (void*)contentsRaw);
    }

    if (!contentsFromModuleSourceMap && contents && (contents != contentsRaw))
    {
        HeapFree(GetProcessHeap(), 0, (void*)contents);
    }

    if (HostConfigFlags::flags.IgnoreScriptErrorCode)
    {
        return S_OK;
    }
    return hr;
}

DWORD_PTR JsHostActiveScriptSite::AddUrl(_In_z_ LPCWSTR url)
{
    DWORD_PTR dwSourceCookie = m_dwNextSourceCookie++;
    this->m_fileMap[dwSourceCookie].m_fileName = SysAllocString(url);
    return dwSourceCookie;
}

// TODO: make source code heapalloc.
HRESULT JsHostActiveScriptSite::LoadModuleFromString(bool isUtf8, 
    LPCWSTR filename, UINT fileNameLength, LPCWSTR contentRaw, UINT byteLength, void** errorObject, LPCWSTR fullName)
{
    HRESULT hr = S_OK;
    CComPtr<IActiveScript> activeScript;
    CComPtr<IActiveScriptDirect> activeScriptDirect;
    hr = GetActiveScript(&activeScript);
    if (SUCCEEDED(hr))
    {
        hr = activeScript->QueryInterface(IID_PPV_ARGS(&activeScriptDirect));
    }
    if (SUCCEEDED(hr))
    {
        DWORD_PTR dwSourceCookie = m_dwNextSourceCookie++;
    // TODO: handle nested module/create the dictionary for filename<->ModuleRecord mapping.
        ModuleRecord requestModule = nullptr;
        LPCWSTR moduleRecordKey = fullName ? fullName : filename;
        auto moduleRecordEntry = moduleRecordMap.find(std::wstring(moduleRecordKey));
        if (moduleRecordEntry == moduleRecordMap.end())
        {
            fwprintf(stderr, _u("module record for %s should have been created \n"), filename);
            return E_INVALIDARG;
        }
        requestModule = moduleRecordEntry->second;
        ParseModuleSourceFlags flags = (isUtf8 ? ParseModuleSourceFlags_DataIsUTF8 : ParseModuleSourceFlags_DataIsUTF16LE);
        hr = activeScriptDirect->ParseModuleSource(requestModule, nullptr, (void*)dwSourceCookie, (LPBYTE)contentRaw, byteLength, flags, 0, 0, 0, errorObject);

        if (FAILED(hr) && *errorObject != nullptr)
        {
            // This is alternative way to report error coming from ParseModuleSource. However here the call
            // comes from an external function call, and we want to just throw back out, from the caller.
            //IActiveScriptError* scriptError = nullptr;
            //hr = activeScriptDirect->CreateScriptErrorFromVar(*errorObject, &scriptError);
            //if (SUCCEEDED(hr))
            //{
            //    hr = OnScriptError(scriptError);
            //    scriptError->Release();
            //}
        }
    }
    return hr;
}

HRESULT JsHostActiveScriptSite::LoadScriptFromString(LPCOLESTR contents, _In_opt_bytecount_(cbBytes) LPBYTE pbUtf8, UINT cbBytes, _Out_opt_ bool* pUsedUtf8, char16 *fullPath)
{
    HRESULT hr = S_OK;

    if (pUsedUtf8)
    {
        *pUsedUtf8 = false;
    }

    CComPtr<IActiveScript> activeScript;
    hr = GetActiveScript(&activeScript);
    if (SUCCEEDED(hr))
    {
        CComPtr<IActiveScriptParseUTF8> activeScriptParseUTF8;
        CComPtr<IActiveScriptParse> activeScriptParse;

        if (!pbUtf8 || FAILED(hr = activeScript->QueryInterface(IID_PPV_ARGS(&activeScriptParseUTF8))))
        {
            hr = activeScript->QueryInterface(IID_PPV_ARGS(&activeScriptParse));
        }

        if (SUCCEEDED(hr))
        {
            // Increment m_dwNextSourceCookie first, so that if below ParseScriptText calls WScript.LoadScriptFile we'll use the correct next source cookie.
            DWORD_PTR dwSourceCookie = m_dwNextSourceCookie++;
            if (fullPath)
            {
                RegisterScriptDir(dwSourceCookie, fullPath);
            }

            EXCEPINFO excepinfo;
            DWORD dwFlag = SCRIPTTEXT_ISVISIBLE | (HostConfigFlags::flags.HostManagedSource ? SCRIPTTEXT_HOSTMANAGESSOURCE : 0);
            
            if (activeScriptParseUTF8)
            {
                hr = activeScriptParseUTF8->ParseScriptText(pbUtf8, 0, cbBytes, NULL, NULL, NULL, dwSourceCookie, 0, dwFlag, NULL, &excepinfo);
                if (pUsedUtf8)
                {
                    *pUsedUtf8 = true; // Script engine has taken over the utf8 buffer as Trident buffer
                }
            }
            else
            {
                hr = activeScriptParse->ParseScriptText(contents, NULL, NULL, NULL, dwSourceCookie, 0, dwFlag, NULL, &excepinfo);
            }
        }
    }

    return hr;
}

HRESULT JsHostActiveScriptSite::Create(JsHostActiveScriptSite ** scriptSiteOut, bool actAsDiagnosticsHost, bool isPrimary, WORD domainId)
{
    HRESULT hr = S_OK;

    JsHostActiveScriptSite * scriptSite = new JsHostActiveScriptSite();
    scriptSite->SetDomainId(domainId);
    if (actAsDiagnosticsHost)
    {
        scriptSite->SetDiagnosticsScriptSite(true);
    }

    // Create the script engine
    hr = scriptSite->CreateScriptEngine(isPrimary);
    if (SUCCEEDED(hr))
    {
        hr = git->RegisterInterfaceInGlobal((IJsHostScriptSite*)scriptSite, IID_IJsHostScriptSite, &scriptSite->jsHostScriptSiteCookie);

        // Initialize WScript
        if (SUCCEEDED(hr))
        {
            IActiveScript * activeScript = NULL;
            hr = scriptSite->GetActiveScript(&activeScript);
            if (SUCCEEDED(hr))
            {
                hr = WScriptFastDom::Initialize(activeScript);
                if (SUCCEEDED(hr))
                {
                    hr = SCA::Initialize(activeScript);
                    if (SUCCEEDED(hr))
                    {
                        hr = MockDomObjectManager::Initialize(activeScript);
                    }
                }
                activeScript->Release();
            }
        }
    }

    if (SUCCEEDED(hr) && (HostConfigFlags::flags.EnableDebug || HostConfigFlags::flags.DebugLaunch))
    {
        DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
        hr = diagnosticsHelper->InitializeDebugging(/*canSetBreakpoints*/true);
        if (SUCCEEDED(hr))
        {
            scriptSite->m_isHostInDebugMode = HostConfigFlags::flags.EnableDebug
                || diagnosticsHelper->IsHostInDebugMode(); // Can be false if DebuggerDetached
            scriptSite->m_didSourceRundown = scriptSite->m_isHostInDebugMode;
        }
    }

    // Register the object in GIT
    if (SUCCEEDED(hr))
    {
        *scriptSiteOut = scriptSite;
    }
    else
    {
        if (scriptSite)
        {
            Shutdown(scriptSite);
        }
    }

    return hr;
}

HRESULT JsHostActiveScriptSite::Shutdown(JsHostActiveScriptSite * scriptSite)
{
    Assert(scriptSite);
    if (scriptSite->wasClosed)
        return S_OK;

    HRESULT hr = S_OK;

    scriptSite->AddRef();
    scriptSite->StopScriptEngine();

    if (scriptSite->jsHostScriptSiteCookie)
    {
        hr = git->RevokeInterfaceFromGlobal(scriptSite->jsHostScriptSiteCookie);
    }

    scriptSite->wasClosed = TRUE;
    scriptSite->Release();

    return hr;
}

void JsHostActiveScriptSite::Terminate()
{
    AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);

    WScriptFastDom::ShutdownAll();

    for (DWORD slot = 0; slot < nextFreeOnShutdownSlot; ++slot)
    {
        HeapFree(GetProcessHeap(), 0, freeOnShutdown[slot]);
        freeOnShutdown[slot] = nullptr;
    }

    nextFreeOnShutdownSlot = 0;

    for (DWORD slot = 0; slot < nextUnmapOnShutdownSlot; ++slot)
    {
        UnmapViewOfFile(unmapOnShutdown[slot].base);
        CloseHandle(unmapOnShutdown[slot].fileMapping);
        CloseHandle(unmapOnShutdown[slot].fileHandle);
    }

    nextUnmapOnShutdownSlot = 0;
}

HRESULT JsHostActiveScriptSite::GetActiveScript(IActiveScript ** activeScript)
{
    return git->GetInterfaceFromGlobal(activeScriptCookie, IID_IActiveScript, (void**)activeScript);
}

HRESULT JsHostActiveScriptSite::GetByteCodeGenerator(IActiveScriptByteCode ** byteCodeGen)
{
    return git->GetInterfaceFromGlobal(byteCodeGenCookie, IID_IActiveScriptByteCode, (void**)byteCodeGen);
}

HRESULT JsHostActiveScriptSite::GetGlobalObjectDispatchEx(IDispatchEx ** globalObject)
{
    return git->GetInterfaceFromGlobal(globalObjectIDispatchExCookie, __uuidof(IDispatchEx), (void**)globalObject);
}

STDMETHODIMP JsHostActiveScriptSite::LoadScriptFile(LPCOLESTR filename)
{
    HRESULT hr = S_OK;

    IJsHostScriptSite * scriptSite = NULL;
    hr = git->GetInterfaceFromGlobal(jsHostScriptSiteCookie, IID_IJsHostScriptSite, (void**)&scriptSite);
    if (SUCCEEDED(hr))
    {
        if (scriptSite != this)
        {
            hr = scriptSite->LoadScriptFile(filename);
        }
        else
        {
            hr = LoadScriptFromFile(filename);
        }

        scriptSite->Release();
    }

    return hr;
}

STDMETHODIMP JsHostActiveScriptSite::LoadModuleFile(LPCOLESTR filename, BOOL useExistingModuleRecord, byte** errorObject, DWORD_PTR referenceModuleRecord)
{
    HRESULT hr = S_OK;

    CComPtr<IActiveScriptDirect> scriptDirect = nullptr;
    ModuleRecord moduleRecord = nullptr;
    std::wstring specifierFullPath;
    char16 fullPath[_MAX_PATH];
    if (referenceModuleRecord)
    {
        auto moduleDirEntry = moduleDirMap.find((ModuleRecord)referenceModuleRecord);
        if (moduleDirEntry != moduleDirMap.end())
        {
            specifierFullPath = moduleDirEntry->second;
        }
    }

    specifierFullPath += filename;
    if (_wfullpath(fullPath, specifierFullPath.c_str(), _MAX_PATH) == nullptr)
    {
        return E_FAIL;
    }

    auto moduleEntry = moduleRecordMap.find(fullPath);
    if (useExistingModuleRecord)
    {
        if (moduleEntry == moduleRecordMap.end())
        {
            fwprintf(stderr, _u("missing module record for \'%s\'\n"), filename);
            hr = E_INVALIDARG;
        }
        else
        {
            hr = GetActiveScriptDirect(&scriptDirect);
            if (SUCCEEDED(hr))
            {
                moduleRecord = moduleEntry->second;
            }
        }
    }
    else
    {
        if (moduleEntry != moduleRecordMap.end())
        {
            fwprintf(stderr, _u("ERROR: same module file was loaded multiple times %s\n"), filename);
            hr = E_INVALIDARG;
        }
        else
        {
            hr = GetActiveScriptDirect(&scriptDirect);
            if (SUCCEEDED(hr))
            {
                hr = scriptDirect->InitializeModuleRecord(nullptr, filename, (UINT)wcslen(filename), &moduleRecord);

                if (SUCCEEDED(hr))
                {
                    moduleRecordMap[fullPath] = moduleRecord;
                    char16 dir[_MAX_PATH];
                    moduleDirMap[moduleRecord] = std::wstring(GetDir(fullPath, dir));
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = LoadScriptFromFile(fullPath, (Var*)errorObject, true);
    }

    if (FAILED(hr) && moduleRecord != nullptr)
    {
        Var exceptionVar = nullptr;
        scriptDirect->ParseModuleSource(moduleRecord, nullptr, nullptr, nullptr, 0, ParseModuleSourceFlags_DataIsUTF8, 0, 0, 0, &exceptionVar);
    }

    return hr;
}

STDMETHODIMP JsHostActiveScriptSite::RegisterModuleSource(LPCOLESTR moduleIdentifier, LPCOLESTR script)
{
    char16 fullpath[_MAX_PATH];
    if (_wfullpath(fullpath, moduleIdentifier, _MAX_PATH) == nullptr)
    {
        fwprintf(stderr, _u("Out of memory"));
        return E_OUTOFMEMORY;
    }

    auto existingModuleSource = moduleSourceMap.find(fullpath);
    
    if (existingModuleSource != moduleSourceMap.end())
    {
        fwprintf(stderr, _u("Source for module identifier \"%s\" already registered.\n"), moduleIdentifier);
        return E_INVALIDARG;
    }

    moduleSourceMap[fullpath] = script;

    return S_OK;
}

STDMETHODIMP JsHostActiveScriptSite::LoadScript(LPCOLESTR script)
{
    HRESULT hr = S_OK;

    CComPtr<IJsHostScriptSite> scriptSite = NULL;
    hr = git->GetInterfaceFromGlobal(jsHostScriptSiteCookie, IID_IJsHostScriptSite, (void**)&scriptSite);
    if (SUCCEEDED(hr))
    {
        if (scriptSite != this)
        {
            hr = scriptSite->LoadScript(script);
        }
        else
        {
            bool usedUtf8 = true;
            size_t scriptContentLen = wcslen(script);
            // If scriptContent length trucates, reject the script
            if (scriptContentLen <= UINT_MAX)
            {
                hr = LoadScriptFromString(script, nullptr, 0, &usedUtf8);
            }
            else
            {
                hr = E_FAIL;
            }
        }

    }

    return hr;
}

STDMETHODIMP JsHostActiveScriptSite::LoadModule(LPCOLESTR script, byte** errorObject)
{
    HRESULT hr = S_OK;

    CComPtr<IJsHostScriptSite> scriptSite = NULL;
    hr = git->GetInterfaceFromGlobal(jsHostScriptSiteCookie, IID_IJsHostScriptSite, (void**)&scriptSite);
    if (SUCCEEDED(hr))
    {
        if (scriptSite != this)
        {
            hr = E_NOTIMPL;
        }
        else
        {
            ModuleRecord requestModuleRecord = nullptr;
            CComPtr<IActiveScriptDirect> scriptDirect = nullptr;
            hr = GetActiveScriptDirect(&scriptDirect);
            if (moduleRecordMap.find(script) != moduleRecordMap.end())
            {
                fwprintf(stderr, _u("same script got loaded as module more than once\n"));
                hr = E_INVALIDARG;
            }
            if (SUCCEEDED(hr))
            {
                hr = scriptDirect->InitializeModuleRecord(nullptr, script, (UINT)wcslen(script), &requestModuleRecord);
            }
            if (SUCCEEDED(hr))
            {
                moduleRecordMap[script] = requestModuleRecord;
                hr = LoadModuleFromString(false, script, (UINT)wcslen(script), script, (UINT)wcslen(script)*sizeof(WCHAR), (Var*)errorObject);
            }
        }
    }

    return hr;
}


STDMETHODIMP JsHostActiveScriptSite::QueryInterface(REFIID riid, void ** ppvObj)
{
    QI_IMPL(IID_IUnknown, IActiveScriptSite);
    QI_IMPL(IID_IActiveScriptSite, IActiveScriptSite);
    QI_IMPL(IID_IJsHostScriptSite, IJsHostScriptSite);
    QI_IMPL_INTERFACE(ISCAHost);
    QI_IMPL_INTERFACE(IHeapEnumHost);
    QI_IMPL_INTERFACE(IOleCommandTarget);
    QI_IMPL_INTERFACE(IActiveScriptSiteDebug);
    QI_IMPL_INTERFACE(IActiveScriptSiteDebugHelper);
    QI_IMPL_INTERFACE(IActiveScriptDirectSite);
    QI_IMPL_INTERFACE(IActiveScriptDirectHost);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) JsHostActiveScriptSite::AddRef(void)
{
    return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) JsHostActiveScriptSite::Release(void)
{
    LONG res = InterlockedDecrement(&refCount);

    if (res == 0)
    {
        delete this;
    }

    return res;
}

STDMETHODIMP JsHostActiveScriptSite::GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti)
{
    Assert(FALSE);
    return E_NOTIMPL;
}

STDMETHODIMP JsHostActiveScriptSite::OnEnterScript()
{
    if ((HostConfigFlags::flags.DebugLaunch || HostConfigFlags::flags.EnableDebug) && m_fileMap[m_dwCurrentSourceCookie].m_sourceContent.c_str() != NULL)
    {
        // We are in debug mode, do the unicode text from here.
        if (m_fileMap[m_dwCurrentSourceCookie].m_debugDocumentHelper != NULL)
        {
            m_fileMap[m_dwCurrentSourceCookie].m_debugDocumentHelper->AddUnicodeText(m_fileMap[m_dwCurrentSourceCookie].m_sourceContent.c_str());
            m_fileMap[m_dwCurrentSourceCookie].m_sourceContent.clear();
        }
    }
    return S_OK;
}

STDMETHODIMP JsHostActiveScriptSite::OnLeaveScript()
{
    if (HostConfigFlags::flags.EnableOnLeaveScript)
    {
        LoadScript(_u("__onLeaveScript()"));
    }
    return S_OK;
}

STDMETHODIMP JsHostActiveScriptSite::OnScriptError(IActiveScriptError * error)
{
    HRESULT hr = S_OK;

    if (m_onScriptErrorHelper.GetCallback())
    {
        return m_onScriptErrorHelper.GetCallback()(error, m_onScriptErrorHelper.GetContext());
    }

    // flush all buffer so that we can emit the error message in the right sequence
    _flushall();

    // <HACK>
    // To allow verification of the ActiveScriptError object for ScriptErrorTests
    char16 filename[_MAX_FNAME];
    char16 ext[_MAX_EXT];
    BSTR filenameFlag = NULL;
    hr = JScript9Interface::GetFileNameFlag(&filenameFlag);
    if (filenameFlag != NULL)
    {
        _wsplitpath_s(filenameFlag, NULL, 0, NULL, 0, filename, _MAX_FNAME, ext, _MAX_EXT);
        // Check if file name begins with specified ActiveScriptError test prefix
        if (!HostSystemInfo::SupportsOnlyMultiThreadedCOM()
            && wcsncmp(filename, _u("ActiveScriptError_"), wcslen(_u("ActiveScriptError_"))) == 0)
        {    
            IActiveScriptWinRTErrorDebug* debugEx = NULL;
            hr = error->QueryInterface(__uuidof(IActiveScriptWinRTErrorDebug), (void**)&debugEx);
            if (SUCCEEDED(hr))
            {
                AutoReleasePtr<IActiveScriptWinRTErrorDebug> autoReleaseDebugEx(debugEx);
                AutoBSTR autoReleaseRestrictedString;
                AutoBSTR autoReleaseRestrictedReference;
                AutoBSTR autoReleaseCapabilitySid;

                // Get and print restricted error string
                hr = debugEx->GetRestrictedErrorString(&autoReleaseRestrictedString);
                if (FAILED(hr))
                {
                    fwprintf(stderr, _u("An unknown error occured in call to GetRestrictedErrorString"));
                    fflush(stderr);
                    return hr;
                }
                wprintf(_u("Restricted Error String: %s\n"), (char16*)autoReleaseRestrictedString);

                // Get and print restricted error reference
                hr = debugEx->GetRestrictedErrorReference(&autoReleaseRestrictedReference);
                if (FAILED(hr))
                {
                    fwprintf(stderr, _u("An unknown error occured in call to GetRestrictedErrorReference"));
                    fflush(stderr);
                    return hr;
                }
                if (autoReleaseRestrictedReference)
                {
                    wprintf(_u("Non-null Restricted Error Reference\n"));
                }
                else
                {
                    wprintf(_u("Null Restricted Error Reference\n"));
                }

                // Get and print capability SID
                hr = debugEx->GetCapabilitySid(&autoReleaseCapabilitySid);
                if (FAILED(hr))
                {
                    fwprintf(stderr, _u("An unknown error occured in call to GetCapabilitySid"));
                    fflush(stderr);
                    return hr;
                }
                wprintf(_u("Capability SID: %s\n"), (char16 *)autoReleaseCapabilitySid);
            }

            IActiveScriptErrorEx* errorEx = NULL;
            hr = error->QueryInterface(__uuidof(IActiveScriptErrorEx), (void**)&errorEx);
            if (SUCCEEDED(hr))
            {
                AutoReleasePtr<IActiveScriptErrorEx> autoReleaseErrorEx(errorEx);
                ExtendedExceptionInfo excepInfo;

                memset(&excepInfo, 0, sizeof(excepInfo));

                // Get Extended Exception Info
                hr = errorEx->GetExtendedExceptionInfo(&excepInfo);

                if (FAILED(hr))
                {
                    fwprintf(stderr, _u("An unknown error occured in call to GetExtendedExceptionInfo"));
                    fflush(stderr);
                    return hr;
                }

                wprintf(_u("EXCEPINFO scode: 0x%X\n"), excepInfo.exceptionInfo.scode);
                wprintf(_u("Error Type: %s\n"), excepInfo.errorType.typeText);

                // Clean up Extended Exception Info
                if (excepInfo.exceptionInfo.bstrSource)
                    SysFreeString(excepInfo.exceptionInfo.bstrSource);
                if (excepInfo.exceptionInfo.bstrDescription)
                    SysFreeString(excepInfo.exceptionInfo.bstrDescription);
                if (excepInfo.exceptionInfo.bstrHelpFile)
                    SysFreeString(excepInfo.exceptionInfo.bstrHelpFile);
                memset(&excepInfo.exceptionInfo, 0, sizeof(excepInfo.exceptionInfo));

                wprintf(_u("Stack info:\n"));
                for (unsigned int i=0; i < excepInfo.callStack.frameCount; i++)
                {
                    wprintf(_u("frame %d: %s, line: %d, characterPosition: %d\n"), i, excepInfo.callStack.frames[i].functionName, 
                        excepInfo.callStack.frames[i].lineNumber, excepInfo.callStack.frames[i].characterPosition);
                    CoTaskMemFree((LPVOID)(excepInfo.callStack.frames[i].functionName));

                    if (excepInfo.callStack.frames[i].activeScriptDirect != nullptr)
                    {
                        excepInfo.callStack.frames[i].activeScriptDirect->Release();
                    }
                }
                if (excepInfo.callStack.frames)
                {
                    CoTaskMemFree((LPVOID)(excepInfo.callStack.frames));
                }
                excepInfo.callStack.frames = NULL;
                excepInfo.callStack.frameCount = 0;

                CoTaskMemFree((LPVOID)excepInfo.errorType.typeText);
                excepInfo.errorType.typeText = NULL;
            }  
        }
    }
    // </HACK>

    if (HostConfigFlags::flags.EnableExtendedErrorMessages) 
    {
        // HACK for long error messages
        IActiveScriptErrorEx* errorEx = NULL;
        hr = error->QueryInterface(__uuidof(IActiveScriptErrorEx), (void**)&errorEx);
        if (SUCCEEDED(hr))
        {
            AutoReleasePtr<IActiveScriptErrorEx> autoReleaseErrorEx(errorEx);
            ExtendedExceptionInfo excepInfo;

            memset(&excepInfo, 0, sizeof(excepInfo));

            // Get Extended Exception Info
            hr = errorEx->GetExtendedExceptionInfo(&excepInfo);

            if (FAILED(hr))
            {
                fwprintf(stderr, _u("An unknown error occured in call to GetExtendedExceptionInfo"));
                fflush(stderr);
                return hr;
            }

            // Catch the originating file error message and don't display it
            if (excepInfo.exceptionInfo.bstrDescription != nullptr
                && !(excepInfo.callStack.frameCount == 1
                    && excepInfo.errorType.typeNumber == JsErrorType::JavascriptError
                    && !wcscmp(excepInfo.exceptionInfo.bstrSource, _u("JavaScript runtime error"))
                    && !wcscmp(excepInfo.exceptionInfo.bstrDescription, _u("")))
                && !this->delegateErrorHandling)
            {
                fwprintf(stderr, _u("%s: %s\n"), MapExternalToES6ErrorText((JsErrorType)excepInfo.errorType.typeNumber), excepInfo.exceptionInfo.bstrDescription);
            }

            // Save it if it needs to be used by the host APIs later
            if (this->lastException != nullptr)
            {
                this->lastException->errorType = (JsErrorType)excepInfo.errorType.typeNumber;
                UINT length = SysStringLen(excepInfo.exceptionInfo.bstrDescription);
                this->lastException->description = new char16[length + 1];
                wcscpy_s(this->lastException->description, length + 1, excepInfo.exceptionInfo.bstrDescription);
                errorEx->GetThrownObject(&(this->lastException->thrownObject));
            }

            // Clean up Extended Exception Info
            if (excepInfo.exceptionInfo.bstrSource)
                SysFreeString(excepInfo.exceptionInfo.bstrSource);
            if (excepInfo.exceptionInfo.bstrDescription)
                SysFreeString(excepInfo.exceptionInfo.bstrDescription);
            if (excepInfo.exceptionInfo.bstrHelpFile)
                SysFreeString(excepInfo.exceptionInfo.bstrHelpFile);
            memset(&excepInfo.exceptionInfo, 0, sizeof(excepInfo.exceptionInfo));

            // If it's a ParseError there wont be any stack, display this message instead
            if (excepInfo.callStack.frameCount == 0 &&
                (excepInfo.errorType.typeNumber == JsErrorType::JavascriptParseError ||
                 excepInfo.errorType.typeNumber == JsErrorType::JavascriptReferenceError ||
                 excepInfo.errorType.typeNumber == JsErrorType::JavascriptSyntaxError))
            {
                DWORD sourceContext;
                ULONG lineNumber;
                LONG charPosition;
                hr = error->GetSourcePosition(&sourceContext, &lineNumber, &charPosition);
                if (SUCCEEDED(hr) && !this->delegateErrorHandling)
                {
                    if (NULL != filenameFlag)
                    {
                        if (HostConfigFlags::flags.BVT)
                        {
                            fwprintf(stderr, _u("\tat code (%s%s:%d:%d)\n"), filename, ext, lineNumber + 1, charPosition + 1);
                        }
                        else
                        {
                            fwprintf(stderr, _u("\tat code (%s:%d:%d)\n"), filenameFlag, lineNumber + 1, charPosition + 1);
                        }
                    }
                }
                if (FAILED(hr))
                {
                    fwprintf(stderr, _u("An unknown error occured."));
                    fflush(stderr);
                    return hr;
                }
            }

            for (unsigned int i = 0; i < excepInfo.callStack.frameCount; i++)
            {
                if (NULL != filenameFlag && !this->delegateErrorHandling)
                {
                    if (HostConfigFlags::flags.BVT)
                    {
                        fwprintf(stderr, _u("\tat %s (%s%s:%d:%d)\n"), excepInfo.callStack.frames[i].functionName, filename, ext,
                            excepInfo.callStack.frames[i].lineNumber + 1, excepInfo.callStack.frames[i].characterPosition + 1);
                    }
                    else
                    {
                        fwprintf(stderr, _u("\tat %s (%s:%d:%d)\n"), excepInfo.callStack.frames[i].functionName, filenameFlag,
                            excepInfo.callStack.frames[i].lineNumber + 1, excepInfo.callStack.frames[i].characterPosition + 1);
                    }
                }
                CoTaskMemFree((LPVOID)(excepInfo.callStack.frames[i].functionName));

                if (excepInfo.callStack.frames[i].activeScriptDirect != nullptr)
                {
                    excepInfo.callStack.frames[i].activeScriptDirect->Release();
                }
            }
            if (excepInfo.callStack.frames)
            {
                CoTaskMemFree((LPVOID)(excepInfo.callStack.frames));
            }
            excepInfo.callStack.frames = NULL;
            excepInfo.callStack.frameCount = 0;

            CoTaskMemFree((LPVOID)excepInfo.errorType.typeText);
            excepInfo.errorType.typeText = NULL;
        }
        if (FAILED(hr))
        {
            fwprintf(stderr, _u("An unknown error occured."));
            fflush(stderr);
            return hr;
        }
        fflush(stderr);
        return S_OK;
    }
    else
    {
        DWORD sourceContext;
        ULONG lineNumber;
        LONG charPosition;
        EXCEPINFO exceptionInfo = { 0 };
        hr = error->GetSourcePosition(&sourceContext, &lineNumber, &charPosition);
        if (SUCCEEDED(hr))
        {
            hr = error->GetExceptionInfo(&exceptionInfo);
        }
        if (FAILED(hr))
        {
            fwprintf(stderr, _u("An unknown error occured."));
            fflush(stderr);
            return hr;
        }
        if (NULL != filenameFlag)
        {
            if (HostConfigFlags::flags.BVT)
            {
                fwprintf(stderr, _u("%s%s(%i, %i) "), filename, ext, lineNumber + 1, charPosition + 1);
            }
            else
            {
                fwprintf(stderr, _u("%s(%i, %i) "), filenameFlag, lineNumber + 1, charPosition + 1);
            }
        }
        fwprintf(stderr, _u("%s: %s\n"), exceptionInfo.bstrSource, exceptionInfo.bstrDescription);
        fflush(stderr);
        return S_OK;
    }
}

LPCWSTR JsHostActiveScriptSite::MapExternalToES6ErrorText(JsErrorType externalErrorType)
{
    switch (externalErrorType)
    {
    case (JsErrorType::CustomError) :
        // Not a valid ES6 Error type
        return _u("CustomError");
    case (JsErrorType::JavascriptError) :
        // Not a valid ES6 Error type
        return _u("JavascriptError");
    case (JsErrorType::JavascriptEvalError) :
        return _u("EvalError");
    case (JsErrorType::JavascriptParseError) :
        // Technically this is a SyntaxError in ES6 language
        return _u("SyntaxError");
    case (JsErrorType::JavascriptRangeError) :
        return _u("RangeError");
    case (JsErrorType::JavascriptReferenceError) :
        return _u("ReferenceError");
    case (JsErrorType::JavascriptSyntaxError) :
        return _u("SyntaxError");
    case (JsErrorType::JavascriptTypeError) :
        return _u("TypeError");
    case (JsErrorType::JavascriptURIError) :
        return _u("URIError");
    case (JsErrorType::WinRTError) :
        // Not a valid ES6 Error type
        return _u("WinRTError");
    default:
        AssertMsg(false, "Unexpected JsErrorType");
        return NULL;
    }
}

STDMETHODIMP JsHostActiveScriptSite::GetDocumentContextFromPosition(
    DWORD_PTR                dwSourceContext,
    ULONG                    uCharacterOffset,
    ULONG                    uNumChars,
    IDebugDocumentContext**  ppsc)
{
    auto mapEntry = m_fileMap.find(dwSourceContext);
    if (mapEntry == m_fileMap.end())
    {
        return E_INVALIDARG;
    }
    JsFile* jsFile = &(mapEntry->second);
    return jsFile->m_debugDocumentHelper->CreateDebugDocumentContext(uCharacterOffset, uNumChars, ppsc);
}

STDMETHODIMP JsHostActiveScriptSite::GetApplication(IDebugApplication**  ppda)
{
    IfNullReturnError(ppda, E_INVALIDARG);
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();

    IfNullReturnError(diagnosticsHelper->m_debugApplication, E_NOTIMPL);

    *ppda = diagnosticsHelper->m_debugApplication;
    (*ppda)->AddRef();

    return S_OK;
}

STDMETHODIMP JsHostActiveScriptSite::GetRootApplicationNode(
   IDebugApplicationNode**  ppdanRoot)
{
    IfNullReturnError(ppdanRoot, E_INVALIDARG);
    *ppdanRoot = NULL; // All App Nodes should be top level

    if (m_fileMap[m_dwCurrentSourceCookie].m_debugDocumentHelper != NULL)
    {
        return m_fileMap[m_dwCurrentSourceCookie].m_debugDocumentHelper->GetDebugApplicationNode(ppdanRoot);
    }

    return E_FAIL;
}

STDMETHODIMP JsHostActiveScriptSite::GetApplicationNode(IDebugApplicationNode** ppdan)
{
    *ppdan = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP JsHostActiveScriptSite::OnScriptErrorDebug(
   IActiveScriptErrorDebug*  pErrorDebug,
   BOOL*                     pfEnterDebugger,
   BOOL*                     pfCallOnScriptErrorWhenContinuing)
{
    return E_NOTIMPL;
}

// Determines whether or not the host is in debug mode.
STDMETHODIMP JsHostActiveScriptSite::IsInDebugMode(BOOL *pfDebugMode)
{
    IfNullReturnError(pfDebugMode, E_INVALIDARG);
    *pfDebugMode = static_cast<BOOL>(m_isHostInDebugMode);
    return S_OK;
}

//
// ISCAHost::CreateObject
//
STDMETHODIMP JsHostActiveScriptSite::CreateObject(ISCAContext* context, SCATypeId typeId, Var *instance)
{
    IfNullReturnError(context, E_INVALIDARG);

    HRESULT hr = S_OK;

    CComPtr<IActiveScript> pActiveScript;
    ScriptDirect pScriptDirect;

    IfFailGo(GetActiveScript(&pActiveScript));
    IfFailGo(pScriptDirect.From(pActiveScript));

    switch (typeId)
    {
    case SCA_ImageDataObject:
        {
            IfFailGo(SCA::CreateImageDataObject(pScriptDirect, instance));
        }
        break;

    default:
        hr = E_FAIL;
    }

Error:
    return pScriptDirect.CheckRecordedException(hr);
}

//
// ISCAHost::DiscardProperties
//
STDMETHODIMP JsHostActiveScriptSite::DiscardProperties(ISCAContext* context, SCATypeId typeId, ISCAPropBag* propbag)
{
    return E_NOTIMPL;
}

//
// IHeapEnumHost::BeginEnumeration
//
STDMETHODIMP JsHostActiveScriptSite::BeginEnumeration()
{
    return S_OK;
}

//
// IOleCommandTarget interfaces
//
STDMETHODIMP JsHostActiveScriptSite::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
    return E_NOTIMPL;
}

STDMETHODIMP JsHostActiveScriptSite::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut)
{
    if (!IsEqualGUID(*pguidCmdGroup, CGID_ScriptSite))
    {
        return E_NOTIMPL;
    }
    switch (nCmdID)
    {
    case CMDID_SCRIPTSITE_URL:
        if (nCmdexecopt != 0 || pvaIn != NULL)
        {
            return E_INVALIDARG;
        }

        if (m_fileMap.size() == 0)
        {
            return E_FAIL;
        }

        pvaOut->vt = VT_BSTR;
        pvaOut->bstrVal = SysAllocString(m_fileMap.begin()->second.m_fileName.c_str());
        return S_OK;

    case CMDID_SCRIPTSITE_SID:
    {
        V_VT(pvaOut) = VT_BSTR;
        BYTE    abSID[MAX_SIZE_SECURITY_ID];
        DWORD   cbSID = (sizeof(abSID) / sizeof((abSID)[0]));
        memset(abSID, 0, cbSID);

        V_BSTR(pvaOut) = SysAllocStringLen(NULL, MAX_SIZE_SECURITY_ID);
        IfNullReturnError(V_BSTR(pvaOut), E_OUTOFMEMORY);
        memcpy_s(V_BSTR(pvaOut), MAX_SIZE_SECURITY_ID, abSID, MAX_SIZE_SECURITY_ID);
        memcpy_s(V_BSTR(pvaOut), MAX_SIZE_SECURITY_ID, &(this->domainId), sizeof(this->domainId));
        return S_OK;
    }
    case CMDID_HOSTCONTEXT_URL:

        if (nCmdexecopt != 0 || pvaIn == NULL || (pvaIn->vt != (VT_UI4 | VT_BYREF)))
        {
            return E_INVALIDARG;
        }

        auto jsFileIter = m_fileMap.find((DWORD_PTR)pvaIn->pulVal);

        if (jsFileIter == m_fileMap.end())
        {
            return E_INVALIDARG;
        }

        pvaOut->vt = VT_BSTR;
        pvaOut->bstrVal = SysAllocString(jsFileIter->second.m_fileName.c_str());

        return S_OK;
    
    }

    return E_NOTIMPL;
}

void __stdcall JsHostActiveScriptSite::EnqueuePromiseTask(Var task)
{
    // Push the callback
    WScriptFastDom::CallbackMessage *msg = new WScriptFastDom::CallbackMessage(0, task);
    WScriptFastDom::PushMessage(msg);
}

STDMETHODIMP JsHostActiveScriptSite::FetchImportedModuleHelper(
    /* [in] */ ModuleRecord referencingModule,
    /* [in] */ LPCWSTR specifier,
    /* [in] */ unsigned long specifierLength,
    /* [out] */ ModuleRecord *dependentModuleRecord,
    /* [in] */ LPCWSTR refdir)
{
    HRESULT hr;
    // TODO: implement the dictionary
    CComPtr<IActiveScriptDirect> activeScriptDirect;
    hr = GetActiveScriptDirect(&activeScriptDirect);
    ModuleRecord moduleRecord = nullptr;

    char16 fullPath[_MAX_PATH];
    std::wstring specifierFullPath = refdir ? refdir : _u("");
    specifierFullPath += specifier;
    if (_wfullpath(fullPath, specifierFullPath.c_str(), _MAX_PATH) == nullptr)
    {
        return JsErrorInvalidArgument;
    }

    auto moduleEntry = moduleRecordMap.find(fullPath);
    if (moduleEntry != moduleRecordMap.end())
    {
        *dependentModuleRecord = moduleEntry->second;
        return S_OK;
    }

    hr = activeScriptDirect->InitializeModuleRecord(referencingModule, specifier, specifierLength, &moduleRecord);
    if (SUCCEEDED(hr))
    {
        char16 dir[_MAX_PATH];
        moduleDirMap[moduleRecord] = std::wstring(GetDir(fullPath, dir));
        moduleRecordMap[fullPath] = moduleRecord;
        activeScriptDirect->SetModuleHostInfo(moduleRecord, ModuleHostInfo_HostDefined, (void *)specifier);
        WScriptFastDom::ModuleMessage* moduleMessage =
            WScriptFastDom::ModuleMessage::Create(referencingModule, specifier, activeScriptDirect);
        WScriptFastDom::PushMessage(moduleMessage);
        *dependentModuleRecord = moduleRecord;
    }
    return hr;
}

STDMETHODIMP JsHostActiveScriptSite::FetchImportedModule(
    /* [in] */ __RPC__in ModuleRecord referencingModule,
    /* [in] */ __RPC__in LPCWSTR specifier,
    /* [in] */ unsigned long specifierLength,
    /* [out] */ __RPC__deref_out_opt ModuleRecord *dependentModuleRecord)
{
    auto moduleDirEntry = moduleDirMap.find(referencingModule);
    if (moduleDirEntry != moduleDirMap.end())
    {
        std::wstring dir = moduleDirEntry->second;
        return FetchImportedModuleHelper(referencingModule, specifier, specifierLength, dependentModuleRecord, dir.c_str());
    }

    return FetchImportedModuleHelper(referencingModule, specifier, specifierLength, dependentModuleRecord);
}


STDMETHODIMP JsHostActiveScriptSite::FetchImportedModuleFromScript(
    /* [in] */ __RPC__in DWORD_PTR dwReferencingSourceContext,
    /* [in] */ __RPC__in LPCWSTR specifier,
    /* [in] */ unsigned long specifierLength,
    /* [out] */ __RPC__deref_out_opt ModuleRecord *dependentModuleRecord)
{
    auto scriptDirEntry = scriptDirMap.find(dwReferencingSourceContext);
    if (scriptDirEntry != scriptDirMap.end())
    {
        std::wstring dir = scriptDirEntry->second;
        return FetchImportedModuleHelper(nullptr, specifier, specifierLength, dependentModuleRecord, dir.c_str());
    }

    return FetchImportedModuleHelper(nullptr, specifier, specifierLength, dependentModuleRecord);
}

STDMETHODIMP JsHostActiveScriptSite::NotifyModuleReady(
    /* [in] */ __RPC__in ModuleRecord referencingModule,
    /* [in] */ __RPC__in Var exceptionVar)
{
    HRESULT hr = NOERROR;
    CComPtr<IActiveScriptDirect> activeScriptDirect;
    hr = GetActiveScriptDirect(&activeScriptDirect);
    if (SUCCEEDED(hr))
    {
        if (exceptionVar != nullptr)
        {
            if (!HostConfigFlags::flags.MuteHostErrorMsgIsEnabled)
            {
                IActiveScriptError* scriptError = nullptr;
                hr = activeScriptDirect->CreateScriptErrorFromVar(exceptionVar, &scriptError);
                if (SUCCEEDED(hr))
                {
                    OnScriptError(scriptError);
                    scriptError->Release();
                }
            }

            if (HostConfigFlags::flags.TraceHostCallbackIsEnabled)
            {
                LPCWSTR specifier = nullptr;
                activeScriptDirect->GetModuleHostInfo(referencingModule, ModuleHostInfo_HostDefined, (void**)&specifier);

                if (specifier != nullptr)
                {
                    wprintf(_u("NotifyModuleReadyCallback(exception) %s\n"), specifier);
                }
            }
        }
        else
        {
            WScriptFastDom::ModuleMessage* moduleMessage =
                WScriptFastDom::ModuleMessage::Create(referencingModule, nullptr, activeScriptDirect);
            WScriptFastDom::PushMessage(moduleMessage);
        }
    }

    return hr;
}

OperationUsage ScriptOperations::defaultUsage =
{
    OperationFlag_all,
    OperationFlag_none,
    OperationFlagsForNamespaceOrdering_none
};

ScriptOperations::ScriptOperations(ITypeOperations* defaultTypeOperations):
    refCount(1),
    defaultOperations(defaultTypeOperations)
{
}

ScriptOperations::~ScriptOperations()
{
    defaultOperations->Release();
}

HRESULT STDMETHODCALLTYPE ScriptOperations::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (riid == _uuidof(ITypeOperations) ||
        riid == IID_IUnknown)
    {
        *ppvObject =  (ITypeOperations*) this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE ScriptOperations::AddRef( void)
{
    return InterlockedIncrement(&refCount);
}

ULONG STDMETHODCALLTYPE ScriptOperations::Release( void)
{
    long currentCount = InterlockedDecrement(&refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetOperationUsage(
    /* [out] */ OperationUsage *usageRef)
{
    *usageRef = defaultUsage;
    return NOERROR;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::HasOwnProperty(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [out] */ BOOL *result)
{
    return defaultOperations->HasOwnProperty(scriptDirect,instance, propertyId, result);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetOwnProperty(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [out] */ Var *value,
    /* [out] */ BOOL *propertyPresent)
{
    return defaultOperations->GetOwnProperty(scriptDirect,instance, propertyId, value, propertyPresent);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetPropertyReference(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [out] */ Var *value,
    /* [out] */ BOOL *propertyPresent)
{
    return defaultOperations->GetPropertyReference(scriptDirect,instance, propertyId, value, propertyPresent);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::SetProperty(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [in] */ Var value,
    /* [out] */ BOOL *result)
{
    return defaultOperations->SetProperty(scriptDirect,instance, propertyId, value, result);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::DeleteProperty(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [out] */ BOOL *result)
{
    return defaultOperations->DeleteProperty(scriptDirect,instance, propertyId, result);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetOwnItem(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ Var index,
    /* [out] */ Var *value,
    /* [out] */ BOOL *itemPresent)
{
    return defaultOperations->GetOwnItem(scriptDirect,instance, index, value, itemPresent);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::SetItem(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ Var index,
    /* [in] */ Var value,
    /* [out] */ BOOL *result)
{
    return defaultOperations->SetItem(scriptDirect,instance, index, value, result);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::DeleteItem(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ Var index,
    /* [out] */ BOOL *result)
{
    return defaultOperations->DeleteItem(scriptDirect,instance, index, result);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetEnumerator(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [out] */ IVarEnumerator **enumerator)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::IsEnumerable(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [out] */ BOOL *result)
{
    return defaultOperations->IsEnumerable(scriptDirect,instance, propertyId, result);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::IsWritable(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [out] */ BOOL *result)
{
    return defaultOperations->IsWritable(scriptDirect,instance, propertyId, result);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::IsConfigurable(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [out] */ BOOL *result)
{
    return defaultOperations->IsConfigurable(scriptDirect,instance, propertyId, result);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::SetEnumerable(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [in] */ BOOL value)
{
    return defaultOperations->SetEnumerable(scriptDirect,instance, propertyId, value);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::SetWritable(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [in] */ BOOL value)
{
    return defaultOperations->SetWritable(scriptDirect,instance, propertyId, value);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::SetConfigurable(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [in] */ BOOL value)
{
    return defaultOperations->SetConfigurable(scriptDirect,instance, propertyId, value);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::SetAccessors(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [in] */ Var getter,
    /* [in] */ Var setter)
{
    return defaultOperations->SetAccessors(scriptDirect,instance, propertyId, getter, setter);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetSetter(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ PropertyId propertyId,
    /* [out] */ Var* setter,
    /* [out] */ ::DescriptorFlags* flags)
{
    return defaultOperations->GetSetter(scriptDirect,instance, propertyId, setter, flags);
}

HRESULT STDMETHODCALLTYPE ScriptOperations::Equals(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ Var other,
    /* [out] */ BOOL *result)
{
    *result = (instance == other);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::StrictEquals(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ Var other,
    /* [out] */ BOOL *result)
{
    *result = (instance == other);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::QueryObjectInterface(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var instance,
    /* [in] */ REFIID riid,
    /* [out] */ void** ppvObj)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetInitializer(
    /* [out] */ InitializeMethod * initializer,
    /* [out] */ int * initSlotCapacity,
    /* [out] */ BOOL * hasAccessors)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetFinalizer(
    /* [out] */ FinalizeMethod * finalizer)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::HasInstance(
    /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
    /* [in] */ Var constructor,
    /* [in] */ Var instance,
    /* [out] */ BOOL* result)
{
    *result = TRUE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ScriptOperations::GetNamespaceParent(
    /* [in] */ Var instance,
    /* [out] */ Var* namespaceParent)
{
    *namespaceParent = NULL;
    return E_NOTIMPL;
}

ActiveScriptDirectHost::ActiveScriptDirectHost()
{
    _refCount = 1;
    _closeWasCalled = false;

#if DBG
    // Determine the windows version
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
    _isWin8 = (osvi.dwBuildNumber >= 7800);
#endif
}

ActiveScriptDirectHost::~ActiveScriptDirectHost()
{
    Assert(true == _closeWasCalled);
}

STDMETHODIMP ActiveScriptDirectHost::GetTypeMetaDataInformation(LPCWSTR typeName, IUnknown** metaDataInformation, DWORD* typeDefToken, BOOL* isVersioned)
{
    if (NULL == metaDataInformation || NULL == isVersioned)
    {
        return E_POINTER;
    }
    *metaDataInformation = NULL;
    *isVersioned = false;
    HRESULT hr = S_OK;

    Assert(m_WinRTStringLibrary.IsAvailable());
    if (m_WinRTStringLibrary.IsAvailable())
    {
        Assert(_isWin8);

        HSTRING hstrTypeName = NULL;
        HSTRING hstrMetaDataFilePath = NULL;

        // OK for typeName to get truncated as it would pass incomplete typeName below which would give wrong results, but won't be security issue.
        m_WinRTStringLibrary.WindowsCreateString(typeName, static_cast<UINT32>(wcslen(typeName)), &hstrTypeName);

        IMetaDataImport2* pMetaDataImport = NULL;
        hr = RoGetMetaDataFile(hstrTypeName, NULL, &hstrMetaDataFilePath, &pMetaDataImport, (mdTypeDef*)typeDefToken);

        m_WinRTStringLibrary.WindowsDeleteString(hstrTypeName);

        if (SUCCEEDED(hr))
        {
            hr = pMetaDataImport->QueryInterface(IID_IUnknown, (void**)metaDataInformation);
        }

        if (NULL != pMetaDataImport)
        {
            pMetaDataImport->Release();
        }

        m_WinRTStringLibrary.WindowsDeleteString(hstrMetaDataFilePath);
    }
    else
    {
        wprintf(_u("ERROR: m_WinRTStringLibrary not available -- returning E_FAIL!\n"));
        return E_FAIL;
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    *isVersioned = (wcsncmp(typeName, _u("Windows."), wcslen(_u("Windows."))) == 0 || wcsncmp(typeName, _u("DevTests.ContractVersioned."), wcslen(_u("DevTests.ContractVersioned."))) == 0);
#else
    *isVersioned = (wcsncmp(typeName, _u("Windows."), wcslen(_u("Windows."))) == 0);
#endif

    // Test cases for bad metadata
    if (RO_E_METADATA_NAME_NOT_FOUND == hr)
    {
        if (0 == wcscmp(typeName, _u("MetadataError.InvalidMetadataFile")))
        {
            return RO_E_INVALID_METADATA_FILE;
        }
        else if (0 == wcscmp(typeName, _u("MetadataError.BadMetadata")))
        {
            return META_E_BADMETADATA;
        }
        else if (0 == wcscmp(typeName, _u("MetadataError.BadSignature")))
        {
            return META_E_BAD_SIGNATURE;
        }
        else if (0 == wcscmp(typeName, _u("MetadataError.InternalError")))
        {
            return CLDB_E_INTERNALERROR;
        }
        else if (0 == wcscmp(typeName, _u("MetadataError.OutOfMemory")))
        {
            return E_OUTOFMEMORY;
        }
        else if (0 == wcscmp(typeName, _u("MetadataError.Fail")))
        {
            return E_FAIL;
        }
    }

    return hr;
}

typedef HRESULT (STDAPICALLTYPE* FN_DllGetClassObject)(REFCLSID,REFIID, LPVOID*);

STDMETHODIMP ActiveScriptDirectHost::CreateTypeFactoryInstance(LPCWSTR typeName, IID factoryID, IUnknown** instance)
{
    if (NULL == instance)
    {
        return E_POINTER;
    }

    *instance = NULL;

    HRESULT hr = E_INVALIDARG;

    if (m_WinRTStringLibrary.IsAvailable())
    {
        // On Win8, use the WinRT apis
        IUnknown* pFactory;
        HSTRING hString = NULL;

        // OK for typeName to get truncated as it would pass incomplete typeName below which would give wrong results, but won't be security issue.
        m_WinRTStringLibrary.WindowsCreateString (typeName, static_cast<UINT32>(wcslen (typeName)), &hString);

        Assert(m_WinRTLibrary.IsAvailable());
        hr = m_WinRTLibrary.GetActivationFactory(hString, factoryID != GUID_NULL ? factoryID : __uuidof(IActivationFactory), (PVOID*)&pFactory);

        if (SUCCEEDED(hr))
        {
            *instance = pFactory;
        }

        m_WinRTStringLibrary.WindowsDeleteString(hString);
    }
    else
    {
        Assert(!_isWin8);
        WCHAR pszFilePath[MAX_PATH+1];
        ExpandEnvironmentStrings(WIN_JSHOST_METADATA_BASE_PATH, pszFilePath, MAX_PATH + 1);

        HINSTANCE hInstance = NULL;
        IClassFactory* pClassFactory;
        FN_DllGetClassObject pProc = NULL;
        void* punk;

        // The ABI type factories that we are aware of:
        // Animal.Animal
        if (0 == wcscmp(typeName, _u("Animal.Animal")))
        {
            wcscat_s(pszFilePath, MAX_PATH, _u("\\Animal.dll"));

            // Try to load the dll
            const CLSID CLSID_AnimalFactory = {0xA88BF705,0x491F,0x4FB1,0xB4,0xC4,0x00,0xA3,0x32,0x13,0x5B,0x05};
            const IID IID_IAnimalFactory = {0xF1DB40AE,0xDF57,0x4A1A,0x96,0x0A,0x9C,0x05,0xB7,0x0A,0x4E,0xB0};

            hInstance = LoadLibraryEx(pszFilePath, NULL, 0);
            if (hInstance != NULL)
            {
                pProc = (FN_DllGetClassObject) GetProcAddress(hInstance, "DllGetClassObject");
                if (pProc != NULL)
                {
                    pProc(CLSID_AnimalFactory, __uuidof(IClassFactory), (LPVOID*)&pClassFactory);
                    pClassFactory->CreateInstance(NULL, IID_IAnimalFactory, &punk);
                    *instance = (IUnknown*)punk;
                    (*instance)->AddRef();
                    pClassFactory->Release();
                    return S_OK;
                }
                else
                {
                    return HRESULT_FROM_WIN32(GetLastError());
                }
            }
            else
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }

        // Animals.Animal
        else if (0 == wcscmp(typeName, _u("Animals.Animal")))
        {
            wcscat_s(pszFilePath, MAX_PATH, _u("\\animalserver.dll"));

            // Try to load the dll
            const CLSID CLSID_AnimalFactory = {0xA88BF705,0x491F,0x4FB1,0xB4,0xC4,0x00,0xA3,0x32,0x13,0x5B,0x05};
            const IID IID_IAnimalFactory = {0xF1DB40AE,0xDF57,0x4A1A,0x96,0x0A,0x9C,0x05,0xB7,0x0A,0x4E,0xB0};

            hInstance = LoadLibraryEx(pszFilePath, NULL, 0);
            if (hInstance != NULL)
            {
                pProc = (FN_DllGetClassObject) GetProcAddress(hInstance, "DllGetClassObject");
                if (pProc != NULL)
                {
                    pProc(CLSID_AnimalFactory, __uuidof(IClassFactory), (LPVOID*)&pClassFactory);
                    pClassFactory->CreateInstance(NULL, IID_IAnimalFactory, &punk);
                    *instance = (IUnknown*)punk;
                    (*instance)->AddRef();
                    pClassFactory->Release();
                    return S_OK;
                }
                else
                {
                    return HRESULT_FROM_WIN32(GetLastError());
                }
            }
            else
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }


    return hr;
}

STDMETHODIMP ActiveScriptDirectHost::TypeInstanceCreated(IUnknown* instance)
{
    instance->AddRef(); // Make sure this is a valid IUnknown
    instance->Release();
    return S_OK;
}

STDMETHODIMP ActiveScriptDirectHost::Close()
{
    // Ensure that Close hasn't already been called
    Assert(!_closeWasCalled);
    _closeWasCalled = true;

    // Make sure that our ref count is 2: one for projectionhost,
    // and one for telemetryhost.
    Assert(2 == _refCount);
    return S_OK;
}

STDMETHODIMP ActiveScriptDirectHost::GetNamespaceChildren(
        __in LPCWSTR fullNamespace,
        __out DWORD *metaDataImportCount,
        __deref_out_ecount_full(*metaDataImportCount) IUnknown ***metaDataImport,
        __out DWORD *childrenNamespacesCount,
        __deref_out_ecount_full(*childrenNamespacesCount) LPWSTR **childrenNamespaces,
        __deref_out_ecount_full(*metaDataImportCount) BOOL **metaDataImportIsVersioned)
{
    IfNullReturnError(fullNamespace, E_INVALIDARG);
    IfNullReturnError(metaDataImportCount, E_POINTER);
    IfNullReturnError(metaDataImport, E_POINTER);
    IfNullReturnError(childrenNamespacesCount, E_POINTER);
    IfNullReturnError(childrenNamespaces, E_POINTER);
    IfNullReturnError(metaDataImportIsVersioned, E_POINTER);

    HSTRING hstrFullNamespace = NULL;
    // OK for fullNamespace/strPackageGraphDir to get truncated as it would pass incomplete fullNamespace/strPackageGraphDir below which would give wrong results, but won't be security issue.
    m_WinRTStringLibrary.WindowsCreateString(fullNamespace, static_cast<UINT32>(wcslen(fullNamespace)), &hstrFullNamespace);

    const DWORD cPackageGraphDirs = 1;
    HSTRING ahstrPackageGraphDirs[cPackageGraphDirs];

    WCHAR strPackageGraphDir[MAX_PATH+1];
    ExpandEnvironmentStrings(WIN_JSHOST_METADATA_BASE_PATH, strPackageGraphDir, _countof(strPackageGraphDir));

    m_WinRTStringLibrary.WindowsCreateString(strPackageGraphDir, static_cast<UINT32>(wcslen(strPackageGraphDir)), &(ahstrPackageGraphDirs[0]));

    DWORD cRetrievedMetaDataFilePaths = 0;
    HSTRING * phstrRetrievedMetaDataFilePaths;
    DWORD cRetrievedSubNamespaces = 0;
    HSTRING * phstrRetrievedSubNamespaces;

    HRESULT hr = m_WinRTTypeResolutionLibrary.RoResolveNamespace(
        hstrFullNamespace,
        ahstrPackageGraphDirs[0],
        cPackageGraphDirs,
        ahstrPackageGraphDirs,
        &cRetrievedMetaDataFilePaths,
        &phstrRetrievedMetaDataFilePaths,
        &cRetrievedSubNamespaces,
        &phstrRetrievedSubNamespaces);

    IfFailedReturn(hr);

    *childrenNamespacesCount = cRetrievedSubNamespaces;
    *childrenNamespaces = (LPWSTR *)CoTaskMemAlloc((*childrenNamespacesCount)*sizeof(LPCWSTR));
    for (DWORD i=0; i<cRetrievedSubNamespaces; i++)
    {
        LPCWSTR name = m_WinRTStringLibrary.WindowsGetStringRawBuffer(phstrRetrievedSubNamespaces[i], nullptr);
        LPWSTR subNamespaceCopy = (LPWSTR)CoTaskMemAlloc((wcslen(name)+1)*sizeof(char16));
        wcscpy_s(subNamespaceCopy, (wcslen(name)+1), name);
        (*childrenNamespaces)[i] = subNamespaceCopy;
        m_WinRTStringLibrary.WindowsDeleteString(phstrRetrievedSubNamespaces[i]);
    }

    *metaDataImportCount = cRetrievedMetaDataFilePaths;
    *metaDataImport = (IUnknown **)CoTaskMemAlloc((*metaDataImportCount)*sizeof(IUnknown*));

    CComPtr<IMetaDataDispenserEx> pMetaDataDispenser = nullptr;

    hr = MetaDataGetDispenser(
        CLSID_CorMetaDataDispenser,
        IID_IMetaDataDispenserEx,
        reinterpret_cast<void **>(&pMetaDataDispenser));

    IfFailedReturn(hr);

    for (DWORD i=0; i<cRetrievedMetaDataFilePaths; i++)
    {
        LPCWSTR pszFilePath = m_WinRTStringLibrary.WindowsGetStringRawBuffer(phstrRetrievedMetaDataFilePaths[i], nullptr);

        CComPtr<IUnknown> spUnk;
        IfFailedReturn(pMetaDataDispenser->OpenScope(pszFilePath, (ofRead | ofNoTransform), IID_IMetaDataImport2, &spUnk));
        IMetaDataImport2 * import=nullptr;
        IfFailedReturn(spUnk->QueryInterface(IID_IMetaDataImport2, reinterpret_cast<void **>(&import)));
        (*metaDataImport)[i] = import;

        m_WinRTStringLibrary.WindowsDeleteString(phstrRetrievedMetaDataFilePaths[i]);
    }

    BOOL isVersioned = FALSE;
    isVersioned = (wcsncmp(fullNamespace, _u("Windows"), wcslen(_u("Windows"))) == 0);
    *metaDataImportIsVersioned = (BOOL *)CoTaskMemAlloc((*metaDataImportCount)*sizeof(BOOL));
    for (DWORD metaDataImportIdx = 0; metaDataImportIdx < *metaDataImportCount; ++metaDataImportIdx)
    {
        (*metaDataImportIsVersioned)[metaDataImportIdx] = isVersioned;
    }


    CoTaskMemFree(phstrRetrievedMetaDataFilePaths);
    phstrRetrievedMetaDataFilePaths = nullptr;
    CoTaskMemFree(phstrRetrievedSubNamespaces);
    phstrRetrievedSubNamespaces = nullptr;

     m_WinRTStringLibrary.WindowsDeleteString(ahstrPackageGraphDirs[0]);
     m_WinRTStringLibrary.WindowsDeleteString(hstrFullNamespace);

    return S_OK;
}

// IUnknown members
STDMETHODIMP ActiveScriptDirectHost::QueryInterface(
/* [in]  */ REFIID riid,
/* [out] */ void **ppvObj)
{
    if (NULL == ppvObj)
    {
        return E_POINTER;
    }

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = static_cast<IUnknown*>(static_cast<IActiveScriptProjectionHost*>(this));
        AddRef();
        return NOERROR;
    }
    QI_IMPL(_uuidof(IActiveScriptProjectionHost), IActiveScriptProjectionHost);
    QI_IMPL_INTERFACE(IActiveScriptProjectionTelemetryHost);

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ActiveScriptDirectHost::AddRef(void)
{
    return InterlockedIncrement(&_refCount);
}

STDMETHODIMP_(ULONG) ActiveScriptDirectHost::Release(void)
{
    LONG res = InterlockedDecrement(&_refCount);

    if (res == 0)
    {
        delete this;
    }

    return res;
}

STDMETHODIMP ActiveScriptDirectHost::TelemetryIncrement(DWORD dataID, DWORD count)
{
    return S_OK;
}

// set the SQM data for given dataID; corresponds to calling SqmSet(HSESSION, DWORD, DWORD)
STDMETHODIMP ActiveScriptDirectHost::TelemetrySet(DWORD dataID, DWORD value)
{
    return S_OK;
}

HRESULT DelayLoadWinRt::ActivateInstance(HSTRING activatableClassId, __deref_out IInspectable ** instance)
{
    if (m_hModule)
    {
        if (m_pfnActivateInstance == NULL)
        {
            m_pfnActivateInstance = (PFNCActivateInstance)GetFunction("RoActivateInstance");
            if (m_pfnActivateInstance == NULL)
            {
                return E_UNEXPECTED;
            }
        }

        Assert(m_pfnActivateInstance != NULL);
        return m_pfnActivateInstance(activatableClassId, instance);
    }

    return E_NOTIMPL;
}

HRESULT DelayLoadWinRt::GetActivationFactory(__in HSTRING activatableClassId, __in REFIID id, __deref_out PVOID* factory)
{
    if (m_hModule)
    {
        if (m_pfnGetActivationFactory == NULL)
        {
            m_pfnGetActivationFactory = (PFNGetActivationFactory)GetFunction("RoGetActivationFactory");

            if (m_pfnGetActivationFactory == NULL)
            {
                return E_UNEXPECTED;
            }
        }

        Assert(m_pfnGetActivationFactory != NULL);
        return m_pfnGetActivationFactory(activatableClassId, id, factory);
    }

    return E_NOTIMPL;
}

HRESULT DelayLoadWinRtTypeResolution::RoResolveNamespace(
    __in_opt const HSTRING namespaceName,
    __in_opt const HSTRING windowsMetaDataPath,
    __in const DWORD packageGraphPathsCount,
    __in_opt const HSTRING *packageGraphPaths,
    __out DWORD *metaDataFilePathsCount,
    HSTRING **metaDataFilePaths,
    __out DWORD *subNamespacesCount,
    HSTRING **subNamespaces)
{
    if (m_hModule)
    {
        if (m_pfnRoResolveNamespace == NULL)
        {
            m_pfnRoResolveNamespace = (PFNCRoResolveNamespace)GetFunction("RoResolveNamespace");
            if (m_pfnRoResolveNamespace == NULL)
            {
                return E_UNEXPECTED;
            }
        }

        Assert(m_pfnRoResolveNamespace != NULL);
        return m_pfnRoResolveNamespace(namespaceName, windowsMetaDataPath, packageGraphPathsCount, packageGraphPaths,
            metaDataFilePathsCount, metaDataFilePaths, subNamespacesCount, subNamespaces);
    }

    return E_NOTIMPL;
}

