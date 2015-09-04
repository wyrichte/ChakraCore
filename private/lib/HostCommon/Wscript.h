/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#include "edgescriptdirect.h"

// The Site will implement below function to Provide Debug Attach/Detach functionality,
class DiagnosticsHelperSite
{
public:
    virtual HRESULT SetDebugAttachFunc(IDispatch* dispatch) = 0;
    virtual HRESULT SetDebugDetachFunc(IDispatch* dispatch) = 0;
    virtual HRESULT SetProfilerStartFunc(IDispatch* dispatch) = 0;
    virtual HRESULT SetProfilerStopFunc(IDispatch* dispatch) = 0;
};

class Dispatch : public IDispatchEx
{
public:
    Dispatch();
    virtual ~Dispatch() { }
    
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObj);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT * /*pctinfo*/) { return E_NOTIMPL; };
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT /*itinfo*/, LCID /*lcid*/, ITypeInfo ** /*pptinfo*/) { return E_NOTIMPL; };
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(__in REFIID riid, __in_ecount(cpsz) LPOLESTR * prgpsz, __in UINT cpsz, __in LCID lcid, __inout_ecount(cpsz) DISPID * prgid);
    virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID id, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei, UINT * puArgErr);

    virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID(DISPID /*id*/) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName(BSTR /*name*/, DWORD /*grfdex*/) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetDispID(BSTR /*bstrName*/, DWORD /*grfdex*/, DISPID * /*pid*/) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetMemberName(DISPID /*id*/, BSTR * /*pbstrName*/) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetMemberProperties(DISPID /*id*/, DWORD /*grfdexFetch*/, DWORD * /*pgrfdex*/) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent(IUnknown ** /*ppunk*/) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetNextDispID(DWORD /*grfdex*/, DISPID /*id*/, DISPID * /*pid*/) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE InvokeEx(DISPID /*id*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS * /*pdp*/, VARIANT * /*pVarRes*/, EXCEPINFO * /*pei*/, IServiceProvider * /*pspCaller*/) { return E_NOTIMPL; }

private:
    ULONG refCount;
};

typedef HRESULT (*GetActiveScriptCallback)(IServiceProvider* pspCaller,  IActiveScript** activeScript);
typedef void (*NotifyCallback)();
typedef void(*AttachDetachCallback)(UINT cmdId, IDispatch * function);

// WScript.Echo callback function for the Jsrt APIs.
JsValueRef EchoCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

class WScriptDispatch : public Dispatch
{
public:
    WScriptDispatch(IActiveScript* activeScript,
        GetActiveScriptCallback pfGetActiveScriptCallback,
        NotifyCallback pfSetKeepAliveCallback = nullptr,
        NotifyCallback pfQuitCallback = nullptr,
        AttachDetachCallback pfAttachDetachCallback = nullptr);

    ~WScriptDispatch();
    enum DISPIDs
    {
        DISPID_Echo = 1,
        DISPID_Quit = 2,
        DISPID_LoadScriptFile = 3,
        DISPID_GetWorkingSet = 4,
        DISPID_SetKeepAlive = 5,
        DISPID_Attach = 6,
        DISPID_Detach = 7,
        DISPID_StartProfiling = 8,
        DISPID_StopProfiling = 9,
        DISPID_PerformSourceRundown = 10,
        DISPID_MaxValue = DISPID_PerformSourceRundown
    };

    HRESULT STDMETHODCALLTYPE GetDispID(BSTR bstrName, DWORD grfdex, DISPID * pid) override;
    HRESULT STDMETHODCALLTYPE GetMemberName(DISPID id, BSTR * pbstrName) override;
    HRESULT STDMETHODCALLTYPE GetNextDispID(DWORD grfdex, DISPID id, DISPID * pid) override;
    HRESULT STDMETHODCALLTYPE InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS * pdp, VARIANT * pVarRes, EXCEPINFO * pei, IServiceProvider * pspCaller) override;

    static HRESULT Initialize(IActiveScript * activeScript);
    void SetDiagnosticsHelperSite(DiagnosticsHelperSite *helpersSite)
    {
        Assert(m_DiagnosticsHelperSite == NULL);
        m_DiagnosticsHelperSite = helpersSite;
    }

    HRESULT Echo(FILE * stream, DISPPARAMS * pdp, IServiceProvider * pspCaller);

    HRESULT DebugAttach(DISPPARAMS * pdp);
    HRESULT DebugDetach(DISPPARAMS * pdp);
    HRESULT DebugSourceRundown();

    HRESULT ScriptProfilerStart(DISPPARAMS * pdp);
    HRESULT ScriptProfilerStop(DISPPARAMS * pdp);

    class EchoDispatch : public Dispatch
    {
    public:
        EchoDispatch(WScriptDispatch *container) : m_container(container) {}
        HRESULT STDMETHODCALLTYPE InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS * pdp, VARIANT * pVarRes, EXCEPINFO * pei, IServiceProvider * pspCaller) override;
    private:
        WScriptDispatch* m_container;
    };

private:
    HRESULT ValidateDispatch(DISPPARAMS * pdp, IDispatch** ppArgNoRef, bool optional = false);
    HRESULT ValidCall(DISPPARAMS * pdp, IDispatch** ppArgNoRef, bool optional = false);

    IActiveScript* m_activeScript;
    DiagnosticsHelperSite * m_DiagnosticsHelperSite;
    GetActiveScriptCallback m_pfGetActiveScriptCallback;
    NotifyCallback m_pfSetKeepAliveCallback;
    NotifyCallback m_pfQuitCallback;
    AttachDetachCallback m_pfAttachDetachCallback;
};

// LPCWSTR WorkingSetProc = L"var ws = new Object(); ws.workingSet = arguments[0]; ws.maxWorkingSet = arguments[1]; ws.pageFault = arguments[2]; ws.privateUsage = arguments[3]; return ws;";
template <typename T>
HRESULT GetWorkingSetFromActiveScript(T* activeScript, VARIANT* varResult)
{
    HRESULT hr = NOERROR;
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX memoryCounter;
    memoryCounter.cb = sizeof(memoryCounter);

    if (!GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&memoryCounter,sizeof(memoryCounter)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    IActiveScriptGarbageCollector* gc;
    if (SUCCEEDED(activeScript->QueryInterface(__uuidof(IActiveScriptGarbageCollector), (void**)&gc)))
    {
        gc->CollectGarbage(SCRIPTGCTYPE_NORMAL);
        gc->Release();
    }

    CComPtr<IDispatch> procDispatch;
    CComPtr<IActiveScriptParseProcedure> procedureParse;

#if _WIN64 || USE_32_OR_64_BIT
        hr = activeScript->QueryInterface(IID_IActiveScriptParseProcedure2_64, (void**)&procedureParse);
#endif
#if !_WIN64 || USE_32_OR_64_BIT
        hr = activeScript->QueryInterface(__uuidof(IActiveScriptParseProcedure2_32), (void**)&procedureParse);
#endif
    if (SUCCEEDED(hr))
    {
        hr = procedureParse->ParseProcedureText(
            L"var ws = new Object(); ws.workingSet = arguments[0]; ws.maxWorkingSet = arguments[1]; ws.pageFault = arguments[2]; ws.privateUsage = arguments[3]; return ws;",
            NULL, NULL, NULL, NULL, NULL, (DWORD)(-1), 0, 0, &procDispatch);
    }

    IfFailedReturn(hr);

    VariantInit(varResult);
    CComPtr<IDispatchEx> dispEx;
    EXCEPINFO ei;
    VARIANT args[5]; // this & other properties 
    IfFailedReturn(procDispatch->QueryInterface(__uuidof(IDispatchEx), (void**)&dispEx));
    memset(&ei, 0, sizeof(ei));
    DISPID dispIdNamed;
    DISPPARAMS dispParams;
    dispParams.cNamedArgs = 1;
    dispIdNamed = DISPID_THIS;
    dispParams.rgdispidNamedArgs = &dispIdNamed;
    dispParams.cArgs = 5;
    dispParams.rgvarg = args;
    args[0].vt = VT_DISPATCH;
    args[0].pdispVal = dispEx;
    args[1].vt = VT_R8;
    args[1].dblVal = (double)memoryCounter.PrivateUsage;
    args[2].vt = VT_R8;
    args[2].dblVal = (double)memoryCounter.PageFaultCount;
    args[3].vt = VT_R8;
    args[3].dblVal = (double)memoryCounter.PeakWorkingSetSize;
    args[4].vt = VT_R8;
    args[4].dblVal = (double)memoryCounter.WorkingSetSize;
    hr = dispEx->InvokeEx(0, 0x1, DISPATCH_METHOD | DISPATCH_PROPERTYGET, &dispParams, varResult, &ei, NULL);
    return hr;
}
