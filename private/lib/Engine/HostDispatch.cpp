/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"
#include "Library\JSON.h"

#include <mshtml.h>

#ifndef __IHTMLDOMConstructor_INTERFACE_DEFINED__
#define __IHTMLDOMConstructor_INTERFACE_DEFINED__
//This is a hack to build in Visual Studio where razzle environment is not present
//Hence we redefine the interface from mshtml.h
#include "IHTMLDOMConstructor.h"
#endif

#include <mshtml.h>

#ifndef __IHTMLDOMConstructor_INTERFACE_DEFINED__
#define __IHTMLDOMConstructor_INTERFACE_DEFINED__
//This is a hack to build in Visual Studio where razzle environment is not present
//Hence we redefine the interface from mshtml.h
#include "IHTMLDOMConstructor.h"
#endif

extern "C" PVOID _ReturnAddress(VOID);
#pragma intrinsic(_ReturnAddress)

HostDispatch * 
HostDispatch::Create(Js::ScriptContext * scriptContext, IDispatch *pdisp, BOOL tryTracker /*=TRUE*/)
{
    HRESULT hr;
    Assert(!scriptContext->GetThreadContext()->IsScriptActive());
    ITracker* tracker = NULL;
    HostVariant* hostVariant = NULL;
    VARIANT* varDispatch;    

    if (pdisp != NULL)
    {
        if (tryTracker)
        {
            AUTO_NO_EXCEPTION_REGION;
            hr = pdisp->QueryInterface(IID_ITrackerJS9, (void**)&tracker);

            if (SUCCEEDED(hr) && tracker)
            {
                tracker->GetTrackingAlias(&varDispatch);
                if (varDispatch != NULL)
                {
                    // Note that one HostVariant can be reused in multiple ScriptContext's.
                    // This is the reason for separating it from HostDispatch, which is a script object and
                    // associated with a single ScriptSite/ScriptContext. (See Eze 1371, 1446.)
                    Assert(varDispatch->punkVal != NULL);
                    hostVariant = CONTAINING_RECORD(varDispatch, HostVariant, varDispatch);
                    Assert(hostVariant->isTracked);
                    tracker->Release();
                    tracker = NULL;
                }
            }
        }
    }

    Recycler* recycler = scriptContext->GetRecycler();
    if (hostVariant == NULL)
    {
        hostVariant = RecyclerNewTrackedLeaf(recycler, HostVariant, pdisp, scriptContext);
        if (tracker != NULL)
        {
            // Some how mshtml expects us to hold on one more ref count in ITracker interface such
            // that it can track through all the javascriptdispatch objects. We'll release the refcount
            // when we finalize HostDispatch.
            hostVariant->SetupTracker(tracker);
            tracker->Release();
        }
    }

    ScriptSite* scriptSite = ScriptSite::FromScriptContext(scriptContext);
    return RecyclerNewFinalized(
        recycler,
        HostDispatch,        
        hostVariant,
        scriptSite->GetActiveScriptExternalLibrary()->GetHostDispatchType());
}

HostDispatch* 
HostDispatch::Create(Js::ScriptContext * scriptContext, VARIANT* variant)
{        
    Recycler* recycler = scriptContext->GetRecycler();
    Assert(!scriptContext->GetThreadContext()->IsScriptActive());
        
    HostVariant* hostVariant = RecyclerNewTrackedLeaf(recycler, HostVariant);
    
    HRESULT hr = S_OK;
    hr = hostVariant->Initialize(variant);

    if (FAILED(hr))
    {
        Js::JavascriptError::ThrowError(scriptContext, hr /* TODO-ERROR: _u("NEED MESSAGE") */);
    } 

    ScriptSite* scripSite = ScriptSite::FromScriptContext(scriptContext);
    HostDispatch* hostDispatch = RecyclerNewFinalized(
        recycler,
        HostDispatch,        
        hostVariant,
        scripSite->GetActiveScriptExternalLibrary()->GetHostDispatchType());
        
    return hostDispatch;
}


HostDispatch * HostDispatch::Create(Js::ScriptContext * scriptContext, ITracker *tracker)
{
    HostVariant* hostVariant;
    VARIANT* varDispatch;
    Recycler* recycler = scriptContext->GetRecycler();
    Assert(!scriptContext->GetThreadContext()->IsScriptActive());

    tracker->GetTrackingAlias(&varDispatch);

    if (varDispatch != NULL)
    {
        // Note that one HostVariant can be reused in multiple ScriptContext's.
        // This is the reason for separating it from HostDispatch, which is a script object and
        // associated with a single ScriptSite/ScriptContext. (See Eze 1371, 1446.)
        Assert(varDispatch->punkVal != NULL);
        hostVariant = CONTAINING_RECORD(varDispatch, HostVariant, varDispatch);
        Assert(hostVariant->isTracked);
    }
    else
    {
        hostVariant = RecyclerNewTrackedLeaf(recycler, HostVariant, tracker, scriptContext);
    }

    ScriptSite* scripSite = ScriptSite::FromScriptContext(scriptContext);
    return RecyclerNewFinalized(
        recycler,
        HostDispatch,        
        hostVariant,
        scripSite->GetActiveScriptExternalLibrary()->GetHostDispatchType());
}

HostDispatch * HostDispatch::Create(Js::ScriptContext * scriptContext, LPCOLESTR itemName)
{
    Assert(!scriptContext->GetThreadContext()->IsScriptActive());
    Recycler* recycler = scriptContext->GetRecycler();
    ScriptSite* scripSite = ScriptSite::FromScriptContext(scriptContext);

    return RecyclerNewFinalized(
        recycler,
        HostDispatch,        
        RecyclerNewTrackedLeaf(recycler, HostVariant, itemName),
        scripSite->GetActiveScriptExternalLibrary()->GetHostDispatchType());
}

HostDispatch::HostDispatch(HostVariant* hostVariant, Js::StaticType * type) :
    Js::RecyclableObject(type),
    cycleStack(nullptr)
{
    AssertMsg(hostVariant, "Attempt to create HostDispatch without HostVariant");
    scriptSite = ScriptSite::FromScriptContext(type->GetScriptContext());
    RefCountedHostVariantAllocator* allocator = scriptSite->GetRefCountedHostVariantAllocator();
 
    refCountedHostVariant = AllocatorNew(RefCountedHostVariantAllocator, allocator, RefCountedHostVariant, hostVariant);

    if (!scriptSite->IsClosed())
    {
        InsertHeadList(&scriptSite->hostDispatchListHead, &linkList);    
    }
    else
    {
        InitializeListHead(&linkList);
    }
}

HostDispatch::HostDispatch(RefCountedHostVariant* refCountedHostVariant, Js::StaticType * type) :
    Js::RecyclableObject(type),
    refCountedHostVariant(refCountedHostVariant),
    cycleStack(nullptr)
{
    AssertMsg(refCountedHostVariant, "Attempt to create HostDispatch without HostVariant");
    scriptSite = ScriptSite::FromScriptContext(type->GetScriptContext());
    refCountedHostVariant->AddRef();
    if (!scriptSite->IsClosed())
    {
        InsertHeadList(&scriptSite->hostDispatchListHead, &linkList);    
    }
    else
    {
        InitializeListHead(&linkList);
    }
}

BOOL HostDispatch::Is(Var instance)
{
    if (Js::JavascriptOperators::GetTypeId(instance) == Js::TypeIds_HostDispatch)
    {
        return TRUE;
    }
    return FALSE;
}

VARIANT * HostDispatch::GetVariant() const
{
    HostVariant* hostVariant = GetHostVariant();
    if (hostVariant)
    {
        return &(hostVariant->varDispatch);
    }

    return NULL;
}

HostVariant * HostDispatch::GetHostVariant() const
{
    return refCountedHostVariant->GetHostVariant();
}

BOOL HostDispatch::CanSupportIDispatchEx() const
{
    return GetHostVariant() && GetHostVariant()->supportIDispatchEx;
}

IDispatch*& HostDispatch::FastGetDispatchNoRef(HostVariant* hostVariant) 
{ 
    return hostVariant->varDispatch.pdispVal; 
}

// 
// This method checks and makes sure that hostVariant is defined and doesn't point to a NULL dispVal
//
inline HRESULT HostDispatch::GetHostVariantWrapper(__out HostVariant** ppOut)
{
    Assert(ppOut != NULL);

    if (this->GetHostVariant() == NULL)
    {
        // the hostVariant is cleaned up, it is coming from a closed site.
        return E_ACCESSDENIED;
    }

    (*ppOut) = this->GetHostVariant();

    return S_OK;
}

HRESULT HostDispatch::EnsureDispatch()
{
    HRESULT hr;

    HostVariant* pHostVariant = NULL;
    IfFailedReturn(GetHostVariantWrapper(&pHostVariant));

    if (FastGetDispatchNoRef(pHostVariant) == NULL)
    {
        return E_ACCESSDENIED;
    }

    if (pHostVariant->isUnknown)
    {
        return E_UNEXPECTED;
    }

    if (pHostVariant->varDispatch.vt != VT_LPWSTR)
    {
        return S_OK;
    }

    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    if (NULL == scriptEngine)
    {
        return E_UNEXPECTED;
    }
    LPCOLESTR itemName = pHostVariant->varDispatch.bstrVal;   

    Js::ScriptContext* scriptContext = this->GetScriptContext();
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        IActiveScriptSite* iactiveScripteSite;
        IUnknown* itemUnknown = NULL;
        IDispatch* dispatch = NULL;
        hr = scriptEngine->GetIActiveScriptSite(&iactiveScripteSite);
        if (SUCCEEDED(hr))
        {
            hr = iactiveScripteSite->GetItemInfo(itemName, SCRIPTINFO_IUNKNOWN, &itemUnknown, NULL);
            if (SUCCEEDED(hr))
            {
                hr = itemUnknown->QueryInterface(__uuidof(IDispatch), (void **)&dispatch);
                if (SUCCEEDED(hr) && !dispatch)
                {
                    hr = E_NOINTERFACE;
                }
                if (SUCCEEDED(hr))
                {
                    itemUnknown->Release();
                    pHostVariant->varDispatch.vt = VT_DISPATCH;
                    if (dispatch->QueryInterface(__uuidof(IDispatchEx), (void**)&pHostVariant->varDispatch.pdispVal) == S_OK
                        && FastGetDispatchNoRef(pHostVariant))
                    {
                        dispatch->Release();
                        pHostVariant->supportIDispatchEx = true;
                        pHostVariant->isUnknown = false;
                    }
                    else
                    {
                        pHostVariant->supportIDispatchEx = false;
                        pHostVariant->isUnknown = false;
                        pHostVariant->varDispatch.pdispVal = dispatch;
                    }
                }
                else
                {
                    pHostVariant->varDispatch.vt = VT_UNKNOWN;
                    pHostVariant->varDispatch.punkVal = itemUnknown;
                    pHostVariant->isUnknown = true;
                    pHostVariant->supportIDispatchEx = false;
                }
            }
        }
    }
    END_LEAVE_SCRIPT(scriptContext);    
    return hr;
}

HRESULT HostDispatch::CallInvokeHandler(InvokeFunc func, DISPID id, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei)
{
#ifdef FAULT_INJECTION
    if (Js::Configuration::Global.flags.FaultInjection >= 0)
    {
        return (this->*func)(id, wFlags, pdp, pvarRes, pei);
    }
#endif

    if (this->scriptSite->GetScriptSiteContext()->GetThreadContext()->GetAbnormalExceptionCode() != 0)
    {
        UnexpectedExceptionHandling_fatal_error();
    }

    HRESULT hr = 0;

    // mark volatile, because otherwise VC will incorrectly optimize away load in the finally block
    volatile uint32 exceptionCode = 0;
    EXCEPTION_POINTERS exceptionInfo = {0};
    __try
    {
        __try
        {
            hr = (this->*func)(id, wFlags, pdp, pvarRes, pei);
        }
        __except (exceptionInfo = *GetExceptionInformation(), exceptionCode = GetExceptionCode(), EXCEPTION_CONTINUE_SEARCH)
        {
            Assert(UNREACHED);
        }
    }
    __finally
    {
        // ensure that there is no EH across this boundary, as that can lead to bad state (e.g. destructors not called)
        if (exceptionCode != 0 && AbnormalTermination() && !IsDebuggerPresent())
        {
            this->scriptSite->GetScriptSiteContext()->GetThreadContext()->SetAbnormalExceptionCode(exceptionCode);
            this->scriptSite->GetScriptSiteContext()->GetThreadContext()->SetAbnormalExceptionRecord(&exceptionInfo);
        }
    }
    return hr;
}

HRESULT HostDispatch::CallInvokeExInternal(DISPID id, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei)
{
    AUTO_NO_EXCEPTION_REGION;

    HRESULT hr;
    HostVariant* pHostVariant = nullptr;
    IfFailedReturn(GetHostVariantWrapper(&pHostVariant));

    IDispatchEx *pDispEx = (IDispatchEx*)GetDispatchNoRef();
    DispatchExCaller* pdc = nullptr;
    if (pDispEx == NULL)
    {
        return E_ACCESSDENIED;
    }

    hr = this->scriptSite->GetDispatchExCaller(&pdc);
    if (SUCCEEDED(hr))
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
        LCID lcid = scriptEngine->GetInvokeVersion();
        
        pDispEx->AddRef();
            
        // The custom marshaler in dispex.dll has a bug that causes the DISPATCH_CONSTRUCT flag
        // to be lost. In IE10+ mode we use PrivateInvokeEx to work around that bug.
        if (wFlags & DISPATCH_CONSTRUCT)
        {
            IJavascriptDispatchRemoteProxy * remoteProxy = nullptr;
            hr = pDispEx->QueryInterface(IID_IJavascriptDispatchRemoteProxy, (void **)&remoteProxy);
            if (SUCCEEDED(hr))
            {
                hr = remoteProxy->PrivateInvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pdc);
                remoteProxy->Release();
            }
            else
            {
                hr = pDispEx->InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pdc);
            }
        }
        else
        {
            hr = pDispEx->InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pdc);
        }
            
        pDispEx->Release();

        this->scriptSite->ReleaseDispatchExCaller(pdc);
    }

    return hr;
}

#pragma strict_gs_check(push, on)
HRESULT HostDispatch::CallInvokeEx(DISPID id, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei)
{
    Js::ScriptContext* scriptContext = this->GetScriptContext();
    HRESULT hr;

    BEGIN_LEAVE_SCRIPT_SAVE_FPU_CONTROL(scriptContext)
    {
        hr = CallInvokeHandler(&HostDispatch::CallInvokeExInternal, id, wFlags, pdp, pvarRes, pei);
    }
    END_LEAVE_SCRIPT_RESTORE_FPU_CONTROL(scriptContext);

    return hr;
}
#pragma strict_gs_check(pop)

HRESULT HostDispatch::CallInvokeInternal(DISPID id, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei)
{
    AUTO_NO_EXCEPTION_REGION;

    HRESULT hr;
    UINT uArgErr;
    IDispatch* pdisp = GetDispatchNoRef();
    if (pdisp == NULL)
    {
        return E_ACCESSDENIED;
    }

    pdisp->AddRef();

    hr = pdisp->Invoke(id, IID_NULL, 0x409/*lcid*/, wFlags, pdp, pvarRes, pei, &uArgErr);

    pdisp->Release();

    return hr;
}

#pragma strict_gs_check(push, on)
HRESULT HostDispatch::CallInvoke(DISPID id, WORD wFlags, DISPPARAMS * pdp, VARIANT * pvarRes, EXCEPINFO * pei)
{
    Js::ScriptContext* scriptContext = this->GetScriptContext();
    HRESULT hr;

    BEGIN_LEAVE_SCRIPT_SAVE_FPU_CONTROL(scriptContext)
    {
        hr = CallInvokeHandler(&HostDispatch::CallInvokeInternal, id, wFlags, pdp, pvarRes, pei);
    }
    END_LEAVE_SCRIPT_RESTORE_FPU_CONTROL(scriptContext);

    return hr;
}
#pragma strict_gs_check(pop)

BOOL HostDispatch::IsGetDispIdCycle(const char16* name)
{
    StackNode* current = cycleStack;
    while(current)
    {
        if (wcscmp(current->Name, name) == 0)
        {
            BOOL isReallyCycle = FALSE;

            // We only want to prevent GetDispId cycle in the same thread (some legacy HTC issue).
            // If this is really a GetDispId cycle, the caller should be us and supports IJavascriptDispatchLocalProxy.
            Js::ScriptContext* scriptContext = this->GetScriptContext();
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                IUnknown* punkCaller = GetScriptSite()->GetCurrentCallerNoRef();
                if (punkCaller != nullptr)
                {
                    IJavascriptDispatchLocalProxy* pJavascriptDispatchProxy = nullptr;
                    if (SUCCEEDED(punkCaller->QueryInterface(&pJavascriptDispatchProxy)))
                    {
                        pJavascriptDispatchProxy->Release();
                        isReallyCycle = TRUE;
                    }
                }
            }
            END_LEAVE_SCRIPT(scriptContext);

            return isReallyCycle;
        }
        current = current->Next;
    }
    return FALSE;
}

BOOL HostDispatch::GetDispIdForProperty(LPCWSTR psz, DISPID *pid)
{
    // Get the DISPID for the named property on the current instance, handling delayed evaluation,
    // VT_UNKNOWN, and GetDispID cycles.

    HRESULT hr;

    if (FAILED(hr = EnsureDispatch()))
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    HostVariant* hostVariant = GetHostVariant();
    // Ok to access hostVariant directly- EnsureDispatch has verified it exists
    if (hostVariant->isUnknown)
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    if (IsGetDispIdCycle(psz))
    {
        return FALSE;
    }

    if (hostVariant->supportIDispatchEx)
    {
        hr = this->GetDispID(psz, fdexNameCaseSensitive, pid);    
    }
    else
    {
        hr = this->GetIDsOfNames(psz, pid);
    }

    if (SUCCEEDED(hr))
    {
        return TRUE;
    }

    if (DISP_E_UNKNOWNNAME == hr)
    {
        return FALSE;
    }

    Assert(hr != DISP_E_EXCEPTION);
    HandleDispatchError(hr, nullptr);
}

BOOL HostDispatch::EnsureDispIdForProperty(LPCWSTR psz, DISPID *pid)
{
    // Get or add the DISPID for the named property on the current instance, handling delayed evaluation,
    // and VT_UNKNOWN, but ignoring GetDispID cycles.

    HRESULT hr;

    if (FAILED(hr = EnsureDispatch()))
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    HostVariant* hostVariant = GetHostVariant();
    // Ok to access hostVariant directly- EnsureDispatch has verified it exists
    if (hostVariant->isUnknown)
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    if (hostVariant->supportIDispatchEx)
    {
        hr = this->GetDispID(psz, fdexNameCaseSensitive | fdexNameEnsure, pid);
    }
    else
    {
        hr = this->GetIDsOfNames(psz, pid);
    }

    if (SUCCEEDED(hr))
    {
        return TRUE;
    }

    HandleDispatchError(hr, nullptr);
}

HRESULT HostDispatch::GetDispID(LPCWSTR psz, ULONG flags, DISPID *pid)
{
    HRESULT hr;

    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();

    if (scriptEngine == NULL)
    {
        return DISP_E_UNKNOWNNAME;
    }

    IfFailedReturn(EnsureDispatch());
    // Ok to access hostVariant directly since EnsureDispatch makes sure it's there
    IDispatchEx *pDispEx = (IDispatchEx*)GetDispatchNoRef();
    if (pDispEx == NULL)
    {
        return E_ACCESSDENIED;
    }

    flags |= (scriptEngine->GetInvokeVersion() << 28);

    Js::ScriptContext* scriptContext = this->GetScriptContext();
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        StackNode nextStack(psz, cycleStack);
        cycleStack = &nextStack;

        // psz has the memory layout of a BSTR
        Assert(SysStringLen((BSTR)psz) == wcslen(psz));
        hr = pDispEx->GetDispID((BSTR) psz, flags, pid);

        Assert(cycleStack == &nextStack);
        cycleStack = cycleStack->Next;
    }
    END_LEAVE_SCRIPT(scriptContext);

    return hr;
}

HRESULT HostDispatch::GetIDsOfNames(LPCWSTR psz, DISPID* pid)
{
    HRESULT hr;

    IfFailedReturn(EnsureDispatch());

    Js::ScriptContext* scriptContext = this->GetScriptContext();
    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        return E_ACCESSDENIED;
    }

    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        StackNode nextStack(psz, cycleStack);
        cycleStack = &nextStack;

        // Ok to access hostVariant directly- EnsureDispatch has verified it exists
        hr = pDispatch->GetIDsOfNames(IID_NULL, (LPOLESTR*)&psz, 1, 0x409, pid);

        Assert(cycleStack == &nextStack);
        cycleStack = cycleStack->Next;
    }
    END_LEAVE_SCRIPT(scriptContext);

    return hr;
}

BOOL HostDispatch::HasProperty(const char16 *psz)
{
    // See whether the instance has this property by trying to get its DISPID.

    DISPID       id = DISPID_UNKNOWN;

    return this->GetDispIdForProperty(psz, &id);
}

BOOL HostDispatch::GetValueByDispId(DISPID id, Js::Var *pValue)
{
    DISPPARAMS   dispEmpty = {0};
    HRESULT      hr;
    VARIANT      varValue;
    EXCEPINFO    ei;

    // Clear the excepinfo.
    memset(&ei, 0, sizeof(ei));

    Js::ScriptContext * scriptContext = this->GetScriptContext();    

    *pValue = scriptContext->GetLibrary()->GetUndefined();

    HostVariant* pHostVariant = NULL;
    hr = GetHostVariantWrapper(&pHostVariant);
    if (FAILED(hr))
    {
        HandleDispatchError(hr, nullptr);
    }
    
    if (pHostVariant->supportIDispatchEx)
    {
        VariantInit(&varValue);
        hr = this->CallInvokeEx(id, DISPATCH_PROPERTYGET, &dispEmpty, &varValue, &ei);
    }
    else
    {
        VariantInit(&varValue);
        hr = this->CallInvoke(id, DISPATCH_PROPERTYGET, &dispEmpty, &varValue, &ei);
    }

    // This is a special case where IE can return MEMBERNOTFOUND in Invoke
    // while GetDispID succeeded. With this check IE9 behaviors
    // similar to FF/chrome. 
    // There are apps depend on the IE8 specific behavior. Let's keep the behavior 
    // consistent with other browers in IE9 mode, and preserve IE8 behavior in IE8 mode. 
    // We already have the catch statement in different places where we don't throw,
    // like  HostDispatch::Equals, add_helper etc. 
    if (hr == DISP_E_MEMBERNOTFOUND)
    {
        return FALSE;
    }

    if (SUCCEEDED(hr))
    {
        Js::Var var;
        DispatchHelper::VariantPropertyFlag propertyFlag = DispatchHelper::VariantPropertyFlag::IsReturnValue;
        hr = DispatchHelper::MarshalVariantToJsVarWithLeaveScript(&varValue, &var, scriptContext, propertyFlag);
        VariantClear(&varValue);
        if (SUCCEEDED(hr))
        {
            *pValue = var;
            return TRUE;
        }
    }

    HandleDispatchError(hr, &ei);
}

BOOL HostDispatch::GetValue(const char16 * psz, Js::Var *pValue)
{
    DISPID       id = DISPID_UNKNOWN;

    if (!this->GetDispIdForProperty(psz, &id))
    {
        *pValue = this->GetLibrary()->GetUndefined();
        return FALSE;
    }

    return this->GetValueByDispId(id, pValue);
}

void HostDispatch::GetReferenceByDispId(DISPID id, Js::Var *pValue, const char16 *name)
{
    // Given an instance and a DISPID, create an lvalue reference (DispMemberProxy).

    Js::ScriptContext *scriptContext = this->GetScriptContext();

    HostVariant* pHostVariant = NULL;
    HRESULT hr = GetHostVariantWrapper(&pHostVariant);
    if (FAILED(hr)) 
    {
        HandleDispatchError(hr, nullptr);
    }
    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        HandleDispatchError(E_ACCESSDENIED, nullptr);
    }

    ScriptSite* scriptSite = ScriptSite::FromScriptContext(scriptContext);
    DispMemberProxy* proxy = RecyclerNewFinalized(
        scriptContext->GetRecycler(),
        DispMemberProxy,
        RecyclerNewTrackedLeaf(scriptContext->GetRecycler(), HostVariant, pDispatch),
        id,
        scriptSite->GetActiveScriptExternalLibrary()->GetDispMemberProxyType(),
        name);
    *pValue = proxy;
}

BOOL HostDispatch::GetPropertyReference(const char16 * psz, Js::Var *pValue)
{
    DISPID       id = DISPID_UNKNOWN;
    Js::ScriptContext* scriptContext = this->GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        *pValue = this->GetScriptContext()->GetLibrary()->GetUndefined();
        return TRUE;
    }

    if (!this->GetDispIdForProperty(psz, &id))
    {
        *pValue = scriptContext->GetLibrary()->GetUndefined();
        return FALSE;
    }

    this->GetReferenceByDispId(id, pValue, psz);

    return TRUE;
}

BOOL HostDispatch::PutValueByDispId(DISPID id, Js::Var value)
{
    AssertInScript();

    DISPID       idNamed = DISPID_PROPERTYPUT;
    DISPPARAMS   dp;
    HRESULT      hr;
    EXCEPINFO    ei;
    WORD         wFlags = DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF;
    VARIANT      varValue;

    hr = DispatchHelper::MarshalJsVarToVariantNoThrowWithLeaveScript(value, &varValue, this->GetScriptContext());
    if (FAILED(hr))
    {
        return FALSE;
    }

    // Clear the excepinfo.
    memset(&ei, 0, sizeof(ei));

    dp.cArgs = 1;
    dp.cNamedArgs = 1;
    dp.rgvarg = &varValue;
    if (varValue.vt != VT_DISPATCH)
    {
        wFlags = DISPATCH_PROPERTYPUT;
    }
    dp.rgdispidNamedArgs = &idNamed;

    HostVariant* pHostVariant = nullptr;
    hr = GetHostVariantWrapper(&pHostVariant);
    if (FAILED(hr)) HandleDispatchError(hr, nullptr);

    if (pHostVariant->supportIDispatchEx)
    {
        hr = this->CallInvokeEx(id, wFlags, &dp, nullptr, &ei);
    }
    else
    {
        hr = this->CallInvoke(id, wFlags, &dp, NULL, &ei);
    }

    VariantClear(&varValue);
    if (SUCCEEDED(hr))
    {
        return TRUE;
    }
    HandleDispatchError(hr, &ei);
}

BOOL HostDispatch::PutValue(const char16 * psz, Js::Var value)
{
    DISPID       id = DISPID_UNKNOWN;

    if (!this->EnsureDispIdForProperty(psz, &id))
    {
        return FALSE;
    }

    return this->PutValueByDispId(id, value);
}

BOOL HostDispatch::GetAccessors(const char16 * name, Js::Var* getter, Js::Var* setter, Js::ScriptContext * requestContext)
{
    HRESULT hr = S_OK;
    BSTR propName = NULL;
    IHTMLDOMConstructor *pDomConst = NULL;
    VARIANT varGetter;
    VARIANT varSetter;
    BOOL bgetset = FALSE;
    if (nullptr == getter || nullptr == setter)
    {
        return FALSE;
    }

    if (requestContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        *setter = requestContext->GetLibrary()->GetNull();
        *getter = requestContext->GetLibrary()->GetNull();
        requestContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }

    if (FAILED(hr = EnsureDispatch()))
    {
        return FALSE;
    }

    VariantInit(&varGetter);
    VariantInit(&varSetter);
    *setter = NULL;
    *getter = NULL;
    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        return FALSE;
    }
    
    BEGIN_LEAVE_SCRIPT(requestContext)
    {
        propName = SysAllocString(name);
        if (propName != NULL)
        {
            hr = pDispatch->QueryInterface(IID_IHTMLDomConstructor, (void**)&pDomConst);

            if (SUCCEEDED(hr))
            {
                hr = pDomConst->LookupGetter(propName, &varGetter);
                if (SUCCEEDED(hr))
                {
                    hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(&varGetter, getter, requestContext);
                    VariantClear(&varGetter);
                    if (SUCCEEDED(hr))
                    {
                        bgetset = TRUE;
                    }
                }
                else
                {
                    // we'll try to find setter even if we can't find getter. 
                    hr = NOERROR;
                }
            }
        
            if (SUCCEEDED(hr))
            {
                hr = pDomConst->LookupSetter(propName, &varSetter);
                if (SUCCEEDED(hr))
                {
                    hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(&varSetter, setter, requestContext);
                    VariantClear(&varSetter);
                    if (SUCCEEDED(hr))
                    {
                        bgetset = TRUE;
                    }
                }
                else
                {
                    hr = NOERROR;
                }
            }


            if (NULL != pDomConst)
            {
                pDomConst->Release();
            }

            SysFreeString(propName);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        
    }
    END_LEAVE_SCRIPT(requestContext);   
    
    /* REVIEW: throw exception for failed HRESULT? */
    return bgetset;
}

void HostDispatch::ThrowIfCannotDefineProperty(Js::PropertyId propId, Js::PropertyDescriptor descriptor)
{
    Js::ScriptContext* scriptContext = this->GetScriptContext();

    // Do the same in all modes for compatibility and consistency. In IE9 std mode we may have
    // remote objects wrapped in HostDispatch and we don't support ES5 on remote objects.

    IHTMLDOMConstructor *pDomConst = NULL;
    if (FAILED(this->QueryObjectInterfaceInScript(IID_IHTMLDomConstructor,(void**)&pDomConst)))
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), VBSERR_ActionNotSupported);
    }

    pDomConst->Release();
    if (descriptor.GetterSpecified() || descriptor.SetterSpecified())
    {
        if (descriptor.ConfigurableSpecified() && !descriptor.IsConfigurable())
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidAttributeFalse, _u("configurable"));
        }
        if (descriptor.EnumerableSpecified() && descriptor.IsEnumerable())
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidAttributeTrue, _u("enumerable"));
        }

        Assert(!descriptor.WritableSpecified());  // Not allowed by JavascriptOperators::ToPropertyDescriptor
    }
    else
    {
        if (!descriptor.ValueSpecified() || Js::JavascriptOperators::GetTypeId(descriptor.GetValue()) == Js::TypeIds_Undefined)
        {
            return;
        }
        if (descriptor.ConfigurableSpecified() && !descriptor.IsConfigurable())
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidAttributeFalse, _u("configurable"));
        }
        if (descriptor.EnumerableSpecified() && !descriptor.IsEnumerable())
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidAttributeFalse, _u("enumerable"));
        }
        if (descriptor.WritableSpecified() && !descriptor.IsWritable())
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidAttributeFalse, _u("writable"));
        }
    }
}

void HostDispatch::ThrowIfCannotGetOwnPropertyDescriptor(Js::PropertyId propId)
{
    Js::ScriptContext* scriptContext = this->GetScriptContext();

    // Do the same in all modes for compatibility and consistency. In IE9 std mode we may have
    // remote objects wrapped in HostDispatch and we don't support ES5 on remote objects.

    IHTMLDOMConstructor *pDomConst = NULL;        
    if (FAILED(this->QueryObjectInterfaceInScript(IID_IHTMLDomConstructor,(void**)&pDomConst)))
    {
        Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_ActionNotSupported);
    }

    pDomConst->Release();
}

BOOL HostDispatch::GetDefaultPropertyDescriptor(Js::PropertyDescriptor& descriptor)
{
    return false;
}

BOOL HostDispatch::SetAccessors(const char16 * name, Js::Var getter, Js::Var setter)
{
    AssertInScript();

    HRESULT hr = S_OK;
  
    Js::ScriptContext* scriptContext = this->GetScriptContext();

    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }

    if (FAILED(hr = EnsureDispatch()))
    {
        return FALSE;
    }
    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        return FALSE;
    }

    if (getter == nullptr && setter == nullptr)
    {
        return FALSE;
    }
    
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        BSTR propName = NULL;
        VARIANT varGetter;
        VARIANT varSetter;

        // REVIEW: The special case for IHTMLDOMConstructor is IE8 only?  Remove?
        IHTMLDOMConstructor *pDomConst = NULL;

        VariantInit(&varGetter);
        VariantInit(&varSetter);

        if (nullptr != getter)
        {
            hr = DispatchHelper::MarshalJsVarToVariantNoThrow(getter, &varGetter, scriptContext);            
        }

        if (SUCCEEDED(hr) && nullptr != setter)
        {
            hr = DispatchHelper::MarshalJsVarToVariantNoThrow(setter, &varSetter, scriptContext);           
        }
       
        if (SUCCEEDED(hr))
        {
            propName = SysAllocString(name);
            if (propName == NULL)
            {
                hr = E_OUTOFMEMORY;            
            }
            else
            {
                hr = pDispatch->QueryInterface(IID_IHTMLDomConstructor, (void**)&pDomConst);
                if (SUCCEEDED(hr) && getter != nullptr)
                {
                    hr = pDomConst->DefineGetter(propName, &varGetter);
                }
                if (SUCCEEDED(hr) && setter != nullptr)
                {
                    hr = pDomConst->DefineSetter(propName, &varSetter);
                }                
            }
        }

        SysFreeString(propName);
        VariantClear(&varGetter);
        VariantClear(&varSetter);

        if (NULL != pDomConst)
        {
            pDomConst->Release();
        }
    }
    END_LEAVE_SCRIPT(scriptContext);      

    if (hr == E_OUTOFMEMORY)
    {
        Js::Throw::OutOfMemory();
    }
    return S_OK == hr;
}

Js::PropertyQueryFlags HostDispatch::HasItemQuery(uint32 index)
{
    // Reject implicit call
    ThreadContext * threadContext = GetScriptContext()->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return Js::PropertyQueryFlags::Property_Found;
    }
    char16 buffer[22];

    ::_i64tow_s(index, &buffer[2], (sizeof(buffer)-sizeof(DWORD))/sizeof(char16), 10);

    ((DWORD*)buffer)[0] = (DWORD)wcslen(&buffer[2]) * sizeof(WCHAR);

    return Js::JavascriptConversion::BooleanToPropertyQueryFlags(HasProperty(&buffer[2]));
}

Js::PropertyQueryFlags HostDispatch::GetItemReferenceQuery(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, Js::ScriptContext* requestContext)
{
    // Reject implicit call
    ThreadContext * threadContext = GetScriptContext()->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        *value = requestContext->GetLibrary()->GetNull();
        return Js::PropertyQueryFlags::Property_NotFound;
    }
    char16 buffer[22];

    ::_i64tow_s(index, &buffer[2], (sizeof(buffer)-sizeof(DWORD))/sizeof(char16), 10);

    ((DWORD*)buffer)[0] = (DWORD)wcslen(&buffer[2]) * sizeof(WCHAR);

    return Js::JavascriptConversion::BooleanToPropertyQueryFlags(GetPropertyReference(&buffer[2], value));
}

Js::PropertyQueryFlags HostDispatch::GetItemQuery(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, Js::ScriptContext* requestContext)
{
    // Reject implicit call
    ThreadContext * threadContext = GetScriptContext()->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        *value = requestContext->GetLibrary()->GetNull();
        return Js::PropertyQueryFlags::Property_NotFound;
    }
    char16 buffer[22];

    ::_i64tow_s(index, &buffer[2], (sizeof(buffer)-sizeof(DWORD))/sizeof(char16), 10);

    ((DWORD*)buffer)[0] = (DWORD)wcslen(&buffer[2]) * sizeof(WCHAR);

    return Js::JavascriptConversion::BooleanToPropertyQueryFlags(GetValue(&buffer[2], value));
}

BOOL HostDispatch::SetItem(__in uint32 index, __in Js::Var value, __in Js::PropertyOperationFlags flags)
{
    // Reject implicit call
    ThreadContext * threadContext = GetScriptContext()->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    char16 buffer[22];

    ::_i64tow_s(index, &buffer[2], (sizeof(buffer)-sizeof(DWORD))/sizeof(char16), 10);

    ((DWORD*)buffer)[0] = (DWORD)wcslen(&buffer[2]) * sizeof(WCHAR);

    return PutValue(&buffer[2], value);
}

BOOL HostDispatch::DeleteItem(uint32 index, Js::PropertyOperationFlags flags)
{
    // Reject implicit call
    ThreadContext * threadContext = GetScriptContext()->GetThreadContext();
    if (threadContext->IsDisableImplicitCall())
    {
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return FALSE;
    }
    char16 buffer[22];

    ::_i64tow_s(index, &buffer[2], (sizeof(buffer)-sizeof(DWORD))/sizeof(char16), 10);

    ((DWORD*)buffer)[0] = (DWORD)wcslen(&buffer[2]) * sizeof(WCHAR);

    return DeleteProperty(&buffer[2]);
}

BOOL HostDispatch::DeletePropertyByDispId(DISPID id)
{
    HRESULT hr;
    if (FAILED(hr = EnsureDispatch()))
    {
        HandleDispatchError(hr, nullptr);
    }

    HostVariant* hostVariant = GetHostVariant();
    // Ok to use hostVariant directly- EnsureDispatch makes sure it exists
    if (!hostVariant->supportIDispatchEx)
    {
        return FALSE;
    }

    IDispatchEx *pDispEx = (IDispatchEx*)GetDispatchNoRef();
    if (pDispEx == NULL)
    {
        return FALSE;
    }

    Js::ScriptContext* scriptContext = this->GetScriptContext();
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = pDispEx->DeleteMemberByDispID(id);
        
        Assert(hr != DISP_E_EXCEPTION);

        if (hr == DISP_E_UNKNOWNNAME)
        {
            // This is the code that a wrapped JSDispatch will return if the member doesn't exist.
            // If the VT_DISPATCH is really one of ours, return "true" rather than throwing.
            IJavascriptDispatchLocalProxy *pProxy = nullptr;

            // Need to requery hostVariant since DeleteMemberByDispId might have freed the script engine
            HostVariant* pHostVariant = nullptr;
            if (SUCCEEDED(GetHostVariantWrapper(&pHostVariant)))
            {
                pDispEx = (IDispatchEx*)pHostVariant->GetDispatchNoRef();
                if (pDispEx != NULL)
                {
                    if(SUCCEEDED(pDispEx->QueryInterface(__uuidof(IJavascriptDispatchLocalProxy), (void**)&pProxy)) && pProxy)
                    {
                        hr = NOERROR;  // we don't want to override the original error code if QI failed here. 
                        pProxy->Release();
                    }
                }
                else
                {
                    hr = E_ACCESSDENIED;
                }
            }
        }
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (SUCCEEDED(hr))
    {
        // S_FALSE indicates that the member couldn't be deleted, which in JS equates to "false".
        return hr == S_OK;
    }
    HandleDispatchError(hr, nullptr);
}

BOOL HostDispatch::DeleteProperty(const char16 * psz)
{
    IDispatchEx *pDispEx = nullptr;    
    HRESULT hr;
    if (FAILED(hr = EnsureDispatch()))
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    HostVariant* hostVariant = GetHostVariant();
    // Ok to use hostVariant directly since EnsureDispatch verifies it exists
    if (hostVariant->isUnknown)
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    if (!hostVariant->supportIDispatchEx)
    {
        return FALSE;
    }

    pDispEx = (IDispatchEx*)GetDispatchNoRef();
    ScriptEngine* scriptEngine = this->scriptSite->GetScriptEngine();
    if ((nullptr == scriptEngine) || (nullptr == pDispEx))
    {
        return FALSE;
    }

    Js::ScriptContext* scriptContext = this->GetScriptContext();
    
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        BSTR bstr = SysAllocString(psz);
        if (bstr != nullptr)
        {
            hr = pDispEx->DeleteMemberByName(bstr, fdexNameCaseSensitive | ((scriptEngine->GetInvokeVersion() & 0xF) << 28));
            SysFreeString(bstr);            
        }          
        else
        {
            hr = E_OUTOFMEMORY;
        }

        Assert(hr != DISP_E_EXCEPTION);

        if (hr == DISP_E_UNKNOWNNAME)
        {
            // This is the code that a wrapped JSDispatch will return if the member doesn't exist.
            // If the VT_DISPATCH is really one of ours, return "true" rather than throwing.
            IJavascriptDispatchLocalProxy *pProxy = nullptr;

            // Need to requery HostVariant since DeleteMemberByName might have released the script engine
            HostVariant* pHostVariant = nullptr;
            if (SUCCEEDED(GetHostVariantWrapper(&pHostVariant)))
            {
                pDispEx = (IDispatchEx*)GetDispatchNoRef();
                if (pDispEx == NULL)
                {
                    hr = E_ACCESSDENIED;
                }
                else
                {
                    if(SUCCEEDED(pDispEx->QueryInterface(__uuidof(IJavascriptDispatchLocalProxy), (void**)&pProxy)) && pProxy)
                    {
                        hr = NOERROR;  // we don't want to override the original error code if QI failed here. 
                        pProxy->Release();
                    }
                }
            }
        }
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (SUCCEEDED(hr))
    {
        // S_FALSE indicates that the member couldn't be deleted, which in JS equates to "false".
        return hr == S_OK;
    }
    HandleDispatchError(hr, nullptr);
}

BOOL HostDispatch::ToString(Js::Var* value, Js::ScriptContext* scriptContext) 
{
    // Here we will call the actual ToString() method on the object
    // for dom objects we get different results when we call GetDefaultValue and ToString()
    // this function exists for compat functionality.
    Js::Var result;
    if (GetPropertyReference(scriptContext->GetPropertyName(Js::PropertyIds::toString)->GetBuffer(), &result))
    {
        Js::RecyclableObject* func = Js::RecyclableObject::FromVar(result);
        Js::Var values[1];
        Js::CallInfo info(Js::CallFlags_Value, 1);
        Js::Arguments args(info, values);
        values[0] = this;
        *value = Js::JavascriptFunction::CallFunction<true>(func, func->GetEntryPoint(), args);
        return true;
    }

    return this->GetDefaultValue(Js::JavascriptHint::HintString, value, false);
}

Js::Var HostDispatch::Invoke(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    RUNTIME_ARGUMENTS(args, callInfo);
    AssertMsg(args.Info.Count >0, "bad this argument in Invoke");

    HostDispatch* _this = (HostDispatch*)function;
    return _this->InvokeByDispId(args, DISPID_VALUE);
}

Js::Var HostDispatch::InvokeByDispId(Js::Arguments args, DISPID id, BOOL fIsPut)
{
    AssertInScript();

    bool fNew = FALSE; //_this->ReadAndResetCallingFromNew();
    Js::RecyclableObject* thisArg=(Js::RecyclableObject*)args.Values[0];
    Js::ScriptContext* scriptContext = this->GetScriptContext();
    VARIANT *pvarDisplay = nullptr;

    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        return scriptContext->GetLibrary()->GetNull();
    }
    if (FAILED(EnsureDispatch()))
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    HostVariant* hostVariant = GetHostVariant();
    // Ok to use hostVariant directly- EnsureDispatch has verified its existence
    if (hostVariant->isUnknown)
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }
    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    if (fIsPut)
    {
        Assert(args.Info.Flags == CallFlags_None);
        if (args.Info.Count < 3)
        {
            // The v5.8 behavior throws an exception if there is no input parameter to the call. So:
            // x(0) = 1     -- does not throw
            // x() = 1      -- throws
            // We mimic this by checking here for 3 args: "this", input param, and assigned value.
            // This could be done at compile time, but it seems cleaner just to emit the call in the byte code
            // and let the runtime take care of issuing an error.

            Js::JavascriptError::ThrowReferenceError(this->GetScriptContext(), JSERR_CantAsgCall);
        }
    }
    else if (args.Info.Flags & CallFlags_New)
    {
        fNew = TRUE;
        Assert(thisArg->GetPropertyCount()==0 && thisArg->GetTypeId() != Js::TypeIds_HostDispatch);

        if (!hostVariant->supportIDispatchEx)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
    }

    HRESULT hr = S_OK;
    EXCEPINFO ei;
    DISPPARAMS dp;
    VARIANT   varRes;
    WORD      invokeFlags;

    // Clear the excepinfo.
    memset(&ei, 0, sizeof(ei));

    DISPID dispIdNamed;

    Js::Var result    = nullptr;

    // TODO (yongqu): check against large argument number to avoid stack overflow???

    // When calling into host via DispID, we only need to pass in this pointer
    // when we are trying to create a new object. We need to strip out this pointer
    // when calling into an existing DOM object, whether it support IDispatchEx or not.
    if (fNew)
    {
        Assert(hostVariant->supportIDispatchEx);

        dp.cNamedArgs = 1;
        dispIdNamed = DISPID_THIS;
        dp.rgdispidNamedArgs = &dispIdNamed;
        dp.cArgs = args.Info.Count;
        dp.rgvarg = (VARIANT*)_alloca(sizeof(VARIANT)*dp.cArgs);

        invokeFlags = DISPATCH_CONSTRUCT;

        // TODO: This check will not support non-global objects that point to default dispatches,
        // e.g., binder objects for non-global modules. Once we support those, we'll need to have a
        // more general way of identifying them.
        // Guarantee params include "this".
        // Retrieve "this" explicitly from arg 0, since we may really be looking for a named host object.
        if (thisArg == scriptContext->GetGlobalObject())
        {
            HostDispatch* hostDispatch = ((HostObject*)scriptContext->GetGlobalObject()->GetHostObject())->GetHostDispatch();
            thisArg = static_cast<RecyclableObject*>(hostDispatch);
        }
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = DispatchHelper::MarshalJsVarToVariantNoThrow(thisArg, dp.rgvarg, scriptContext);
            if (SUCCEEDED(hr))
            {
                if (dp.cArgs > 1)
                {
                    hr = DispatchHelper::MarshalJsVarsToVariantsNoThrow(&args.Values[1], &dp.rgvarg[1], dp.cArgs - 1);
                }
                if (FAILED(hr))
                {
                    VariantClear(dp.rgvarg);
                }
            }
        }
        END_LEAVE_SCRIPT(scriptContext)
    }
    else
    {
        // strip out this pointer when just calling the dispid.
        // but not for cross context calls
        BOOL keepThis = FALSE;
        
        if(!fIsPut)
        {
            if (Js::DynamicObject::Is(thisArg) && ! hostVariant->IsIDispatch())
            {
                keepThis = TRUE;
            }
            else
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    IJavascriptDispatchLocalProxy *pProxy = nullptr;
                    if(SUCCEEDED(pDispatch->QueryInterface(__uuidof(IJavascriptDispatchLocalProxy), (void**)&pProxy)) && pProxy)
                    {
                        pProxy->Release();
                        keepThis = TRUE;
                    }
                    else
                    {
                        IJavascriptDispatchRemoteProxy *pRemoteProxy = nullptr;
                        if(SUCCEEDED(pDispatch->QueryInterface(IID_IJavascriptDispatchRemoteProxy, (void**)&pRemoteProxy)) && pRemoteProxy)
                        {
                            pRemoteProxy->Release();
                            keepThis = TRUE;
                        }
                    }
                }
                END_LEAVE_SCRIPT(scriptContext);
            }
        }

        dp.cArgs = keepThis? args.Info.Count : (args.Info.Count - 1);
        dp.rgvarg = (VARIANT*)_alloca(sizeof(VARIANT)*dp.cArgs);

        VARIANT *rgvarg = dp.rgvarg;
        long argCount = dp.cArgs;
        if ((args.Info.Flags & CallFlags_ExtraArg) && ((args.Info.Flags & CallFlags_CallPut) == 0))
        {
            if (keepThis && argCount < 2)
            {
                Js::Throw::InternalError();
            }

            // We're calling eval (or at least we think we are) so use special to logic to marshal the last argument
            // (which is a frame display struct and not a Var). We'll put it in a SAFEARRAY, so allocate one on
            // the stack right here.
            Js::FrameDisplay *pDisplay = (Js::FrameDisplay*)args.Values[args.Info.Count - 1];
            uint length = pDisplay->GetLength();

            pvarDisplay = keepThis ? &dp.rgvarg[1] : &dp.rgvarg[0];
            SAFEARRAY safeArray = {0};
            safeArray.pvData = _alloca(sizeof(VARIANT)*length);
            safeArray.cDims = 1;
            safeArray.cbElements = length;
            pvarDisplay->vt = VT_SAFEARRAY;
            pvarDisplay->parray = &safeArray;

            // Marshal the scopes into the array data and make the following code ignore this arg.
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = DispatchHelper::MarshalFrameDisplayToVariantNoScript(pDisplay, (VARIANT*)safeArray.pvData);
            }
            END_LEAVE_SCRIPT(scriptContext)
            argCount--;
            rgvarg++;
        }

        if (SUCCEEDED(hr))
        {
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                if (keepThis)
                {
                    if (SUCCEEDED(hr = DispatchHelper::MarshalJsVarToVariantNoThrow(args.Values[0], dp.rgvarg, scriptContext)))
                    {
                        hr = DispatchHelper::MarshalJsVarsToVariantsNoThrow(&args.Values[1], &rgvarg[1], argCount - 1);
                    }
                }
                else
                {
                    hr = DispatchHelper::MarshalJsVarsToVariantsNoThrow(&args.Values[1], rgvarg, argCount);
                }
            }
            END_LEAVE_SCRIPT(scriptContext)
        }

        if (fIsPut)
        {
            //keep this is excluded here
            // The assigned value is last in the operand list, so first in the (reversed) disp params.
            dp.cNamedArgs = 1;
            dispIdNamed = DISPID_PROPERTYPUT;
            dp.rgdispidNamedArgs = &dispIdNamed;
            if (dp.rgvarg[0].vt == VT_DISPATCH)
            {
                invokeFlags = DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF;
            }
            else
            {
                invokeFlags = DISPATCH_PROPERTYPUT;
            }
        }
        else
        {
            if(keepThis)
            {
                dp.cNamedArgs = 1;
                dispIdNamed = DISPID_THIS;
                dp.rgdispidNamedArgs = &dispIdNamed;
            }
            else
            {
                dp.cNamedArgs = 0;
                dp.rgdispidNamedArgs = nullptr;
            }

            invokeFlags = DISPATCH_METHOD;
            int carg = keepThis ? dp.cArgs-1 : dp.cArgs;
            if (carg > 0 && (args.Info.Flags & CallFlags_Value))
            {
                invokeFlags |= DISPATCH_PROPERTYGET;
            }
        }
    }

    if (FAILED(hr))
    {
        result = scriptContext->GetLibrary()->GetUndefined();
    }
    else
    {
        VARIANT* pvarRes = nullptr;
        BOOL hasReturnValue = args.Info.Flags != CallFlags_None && ! (args.Info.Flags & CallFlags_NotUsed);
        if (hasReturnValue)
        {
            VariantInit(&varRes);
            pvarRes = &varRes;
        }

        hr = InvokeMarshaled(id, invokeFlags, &dp, pvarRes, &ei);
        if (pvarDisplay != nullptr)
        {
            // We have a frame display that we marshaled to a SAFEARRAY so we could pass it to eval.
            // Release all the array contents here.
            Assert(pvarDisplay->vt == VT_SAFEARRAY);
            SAFEARRAY *pArray = pvarDisplay->parray;
            Assert(pArray && pArray->cDims == 1);
            for (unsigned int i = 0; i < pArray->cbElements; i++)
            {
                VariantClear(&((VARIANT*)pArray->pvData)[i]);
            }
            // Mark the VT_SAFEARRAY as cleared so it's not double-released below.
            // (Could avoid calling VariantClear on it below altogether, but that would require a check in the loop,
            // and this is a rare case.)
            pvarDisplay->vt = VT_EMPTY;
        }
        for (unsigned int i = 0; i < dp.cArgs; i++)
        {
            VariantClear(&dp.rgvarg[i]);
        }

        if (SUCCEEDED(hr))
        {
            result = scriptContext->GetLibrary()->GetNull();
            if (hasReturnValue)
            {
                DispatchHelper::VariantPropertyFlag propertyFlag = DispatchHelper::VariantPropertyFlag::IsReturnValue;
                hr = DispatchHelper::MarshalVariantToJsVarWithLeaveScript(&varRes, &result, scriptContext, propertyFlag);
                VariantClear(&varRes);

                if (FAILED(hr))
                {
                    result = nullptr;
                }
            }
        }
    }

    if (result != nullptr)
        return result;

    HandleDispatchError(hr, &ei);
}

HRESULT HostDispatch::InvokeMarshaled( DISPID id, WORD invokeFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei)
{
    HRESULT hr = S_OK;
    HostVariant* pHostVariant = nullptr;
    IfFailedReturn(GetHostVariantWrapper(&pHostVariant));
    
    if (pHostVariant->supportIDispatchEx)
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
        if (nullptr == scriptEngine)
        {
            if (nullptr != pvarRes)
            {
                pvarRes->vt = VT_EMPTY;
            }
            return S_OK;
        }

        hr = this->CallInvokeEx(id, invokeFlags, pdp, pvarRes, pei);
    }
    else
    {
        AssertMsg((invokeFlags & DISPATCH_CONSTRUCT) == 0, "invalid call");
        hr = this->CallInvoke(id, invokeFlags, pdp, pvarRes, pei);
    }
    return hr;
}

BOOL HostDispatch::GetDefaultValue(Js::JavascriptHint hint, Js::Var* value, BOOL throwException)
{
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    ThreadContext * threadContext = scriptContext->GetThreadContext();

    // Reject implicit call
    if (threadContext->IsDisableImplicitCall())
    {        
        *value = scriptContext->GetLibrary()->GetNull();
        threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
        return TRUE;
    }

    VARIANT      varValue;
    DISPPARAMS   dp = {0};
    EXCEPINFO    ei;
    HRESULT hr;
    WORD wFlags;

    // Clear the excepinfo.
    memset(&ei, 0, sizeof(ei));
    VariantInit(&varValue);

    if (FAILED(hr = EnsureDispatch()))
    {
        return FALSE;
    }

    HostVariant* hostVariant = GetHostVariant();

    // Ok to access hostVariant directly since EnsureDispatch has verified it exists
    if (hostVariant->isUnknown)
    {
        Js::JavascriptError::ThrowTypeError(this->GetScriptContext(), JSERR_NeedFunction);
    }

    wFlags = DISPATCH_PROPERTYGET;

    if (hostVariant->supportIDispatchEx)
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
        if (nullptr == scriptEngine)
        {
            return FALSE;
        }
        hr = this->CallInvokeEx(DISPID_VALUE, wFlags, &dp, &varValue, &ei);
    }
    else
    {
        hr = this->CallInvoke(DISPID_VALUE, wFlags, &dp, &varValue, &ei);
    }

    if (SUCCEEDED(hr))
    {
        DispatchHelper::VariantPropertyFlag propertyFlag = DispatchHelper::VariantPropertyFlag::IsReturnValue;
        hr = DispatchHelper::MarshalVariantToJsVarWithLeaveScript(&varValue, value, scriptContext, propertyFlag);
        VariantClear(&varValue);
        if (SUCCEEDED(hr))
        {
            return TRUE;
        }
    }

    if (throwException)
    {
        HandleDispatchError(hr, &ei);
    }
    else
    {
        // clean up the threadcontext if we don't rethrow.
        if (hr == SCRIPT_E_RECORDED || hr == SCRIPT_E_PROPAGATE)
        {
            scriptContext->GetAndClearRecordedException();
        }
    }

    return FALSE;
}

BOOL HostDispatch::IsInstanceOf(Js::Var prototypeProxy)
{
    Assert(Js::JavascriptOperators::GetTypeId(this) == Js::TypeIds_HostDispatch);
    HRESULT hr;

    if (FAILED(hr = EnsureDispatch()))
    {
        HandleDispatchError(hr, nullptr);
    }

    Js::RecyclableObject* func = Js::RecyclableObject::FromVar(prototypeProxy);
    Js::Var values[2];
    Js::CallInfo info(Js::CallFlags_Value, 2);
    Js::Arguments args(info, values);
    values[0] = func;
    values[1] = this;
    Js::Var res = Js::JavascriptFunction::CallFunction<true>(func, func->GetEntryPoint(), args);
    Assert(Js::JavascriptBoolean::Is(res));
    return Js::JavascriptBoolean::FromVar(res)->GetValue();
}

__declspec(noreturn)
void HostDispatch::HandleDispatchError(HRESULT hr, EXCEPINFO* exceptInfo)
{
    HostDispatch::HandleDispatchError(this->GetScriptContext(), hr, exceptInfo);
}

__declspec(noreturn)
void HostDispatch::HandleDispatchError(Js::ScriptContext * scriptContext, HRESULT hr, EXCEPINFO* exceptInfo)
{
    Assert(!SUCCEEDED(hr));
    
    if (DISP_E_EXCEPTION == hr)
    {
        Assert(!scriptContext->HasRecordedException());
    
        Assert(exceptInfo != nullptr);
        if (nullptr != exceptInfo->pfnDeferredFillIn)
        {
            exceptInfo->pfnDeferredFillIn(exceptInfo);
            exceptInfo->pfnDeferredFillIn = nullptr;
        }
        if (exceptInfo->bstrDescription != nullptr)
        {
            // move the EXCEPINFO's description to a recycler allocation
            Assert(exceptInfo->bstrDescription);
            uint32 len = SysStringLen(exceptInfo->bstrDescription) + 1;
            char16 * allocatedString = RecyclerNewArrayLeaf(scriptContext->GetRecycler(), char16, len);
            wcscpy_s(allocatedString, len, exceptInfo->bstrDescription);

            // save hCode before we clear exceptInfo
            HRESULT hCode = exceptInfo->scode;

            // free exceptInfo's strings before we raise an exception, else we leak Source, Description, Helpfile
            FreeExcepInfo(exceptInfo);

            Js::JavascriptError::ThrowDispatchError(scriptContext, hCode, allocatedString);
        }
        hr = exceptInfo->scode;
    }
    
    // free other allocations
    if (nullptr != exceptInfo)
    {
        FreeExcepInfo(exceptInfo);
    }

    if (hr == SCRIPT_E_RECORDED || hr == SCRIPT_E_PROPAGATE)
    {
        // Rethrow
        scriptContext->RethrowRecordedException(HostDispatch::CreateDispatchWrapper);
    }

    Assert(!scriptContext->HasRecordedException());
    Js::JavascriptError::MapAndThrowError(scriptContext, hr);
}

HRESULT HostDispatch::QueryObjectInterface(REFIID riid, void** ppvObj)
{
    Js::ScriptContext* scriptContext = this->GetScriptContext();  
    HRESULT hr;
    IfFailedReturn(EnsureDispatch());
    Assert(!scriptContext->GetThreadContext()->IsScriptActive()); 

    if (!ppvObj)
    {
        return E_POINTER;
    }

    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        return E_ACCESSDENIED;
    }
    // Ok to directly access hostVariant since EnsureDispatch checked to see if it exists
    hr = pDispatch->QueryInterface(riid, ppvObj);

    if (SUCCEEDED(hr) && !(*ppvObj))
    {
        hr = E_NOINTERFACE; // protect from bad QI implementation
    }
    return hr;
}

HRESULT HostDispatch::QueryObjectInterfaceInScript(REFIID riid, void** ppvObj)
{
    Js::ScriptContext* scriptContext = this->GetScriptContext();  
    HRESULT hr;
    IfFailedReturn(EnsureDispatch());
    Assert(scriptContext->GetThreadContext()->IsScriptActive()); 

    if (!ppvObj)
    {
        return E_POINTER;
    }

    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        return E_ACCESSDENIED;
    }

    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = pDispatch->QueryInterface(riid, ppvObj);

        if (SUCCEEDED(hr) && !(*ppvObj))
        {
            hr = E_NOINTERFACE; // protect from bad QI implementation
        }
    }
    END_LEAVE_SCRIPT(scriptContext);
    return hr;
}

BOOL HostDispatch::GetRemoteTypeId(Js::TypeId* typeId)
{
    HRESULT hr;
    if (FAILED(hr = EnsureDispatch()))
    {
        return FALSE;
    }

    IDispatch* dispatch = GetDispatchNoRef();
    if (dispatch == nullptr)
    {
        return FALSE;
    }
    IJavascriptDispatchRemoteProxy* jscriptInfo = nullptr;

    Js::ScriptContext* scriptContext = this->GetScriptContext();  
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = dispatch->QueryInterface(IID_IJavascriptDispatchRemoteProxy, (void**)&jscriptInfo);
        if (SUCCEEDED(hr) && !jscriptInfo)
        {
            hr = E_NOINTERFACE;
        }

        if (SUCCEEDED(hr))
        {
            hr = jscriptInfo->GetTypeId((int*)typeId);
            jscriptInfo->Release();
        }
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (SUCCEEDED(hr))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void HostDispatch::RemoveFromPrototype(Js::ScriptContext * requestContext)
{
    return;
}

void HostDispatch::AddToPrototype(Js::ScriptContext * requestContext)
{
    return;
}

void HostDispatch::SetPrototype(RecyclableObject* newPrototype)
{
    Js::JavascriptError::ThrowTypeError(GetScriptContext(), VBSERR_ActionNotSupported);
}

BOOL HostDispatch::InvokeBuiltInOperationRemotely(Js::JavascriptMethod entryPoint, Js::Arguments args, Js::Var* result)
{
    HRESULT hr;
    Js::ScriptContext* scriptContext = GetScriptContext();
    if (scriptContext->GetThreadContext()->IsDisableImplicitCall())
    {                
        scriptContext->GetThreadContext()->AddImplicitCallFlags(Js::ImplicitCall_External);
        *result = scriptContext->GetLibrary()->GetNull();
        return TRUE;
    }

    if (FAILED(hr = EnsureDispatch()))
    {
        return FALSE;
    }

    // Map the entry point to a BuiltInOperation value
    BuiltInOperation operation;
    if (!GetBuiltInOperationFromEntryPoint(entryPoint, &operation))
    {
        return FALSE;
    }

    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        return FALSE;
    }
    
    // Make sure the object on the other side is a JavascriptDispatch object
    IJavascriptDispatchRemoteProxy* dispatchProxy;

    hr = HostDispatch::QueryInterfaceWithLeaveScript(pDispatch, IID_IJavascriptDispatchRemoteProxy, (void**)&dispatchProxy, scriptContext);
    if (FAILED(hr))
    {
        return FALSE;
    }

    // Construct the DISPPARAMS structure
    DISPPARAMS dp;
    memset(&dp, 0, sizeof(dp));
    DISPID dispIdThis = DISPID_THIS;
    if (args.Info.Count > 0)
    {
        dp.cArgs = args.Info.Count;
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispIdThis;
        dp.rgvarg = (VARIANT*)_alloca(sizeof(VARIANT)*dp.cArgs);
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = DispatchHelper::MarshalJsVarToVariantNoThrow(args.Values[0], dp.rgvarg, scriptContext);
            if (SUCCEEDED(hr))
            {
                hr = DispatchHelper::MarshalJsVarsToVariantsNoThrow(&args.Values[1], &dp.rgvarg[1], dp.cArgs - 1);
            }
        }
        END_LEAVE_SCRIPT(scriptContext)
    }
    
    EXCEPINFO ei;
    memset(&ei, 0, sizeof(ei));

    if (SUCCEEDED(hr))
    {
        VARIANT varRes;
        VariantInit(&varRes);

        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            DispatchExCaller* pdc = nullptr;
            hr = scriptSite->GetDispatchExCaller(&pdc);
            if (SUCCEEDED(hr))
            {
                hr = dispatchProxy->InvokeBuiltInOperation(operation, 0, &dp, &varRes, &ei, pdc);
                scriptSite->ReleaseDispatchExCaller(pdc);
            }

            for (uint32 i = 0; i < dp.cArgs; ++i)
            {
                VariantClear(&dp.rgvarg[i]);
            }

            if (SUCCEEDED(hr) && result)
            {
                DispatchHelper::VariantPropertyFlag propertyFlag = DispatchHelper::VariantPropertyFlag::IsReturnValue;
                hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(&varRes, result, scriptContext, propertyFlag);
            }
        }
        END_LEAVE_SCRIPT(scriptContext)

        VariantClear(&varRes);
    }

    dispatchProxy->Release();

    if (FAILED(hr))
    {
        if (result)
        {
            *result = nullptr;
        }

        // E_NOTIMPL means that the script context of the remote object is not
        // running in IE10 mode or higher. Instead of throwing an exception,
        // fail gracefully.
        if (hr == E_NOTIMPL)
        {
            return FALSE;
        }

        HandleDispatchError(hr, &ei);
    }

    return TRUE;
}

Js::DynamicObject* HostDispatch::GetRemoteObject()
{
    IJavascriptDispatchLocalProxy* jsProxy = nullptr;
    HostVariant* pHostVariant = nullptr;
    HRESULT hr;
    if (FAILED(hr = GetHostVariantWrapper(&pHostVariant)))
    {
        return nullptr;
    }

    IDispatch* pDispatch = GetDispatchNoRef();
    if (pDispatch == NULL)
    {
        return nullptr;
    }

    hr = HostDispatch::QueryInterfaceWithLeaveScript(pDispatch, __uuidof(IJavascriptDispatchLocalProxy), (void**)&jsProxy, GetScriptContext());
    if (FAILED(hr) || !jsProxy)
    {
        return nullptr;
    }

    JavascriptDispatch* javascriptDispatch = static_cast<JavascriptDispatch*>(jsProxy);
    Js::DynamicObject* dynamicObject = javascriptDispatch->GetObject();
    jsProxy->Release();
    if (NULL == dynamicObject)
    {
        HandleDispatchError(E_ACCESSDENIED, NULL);
    }
    return dynamicObject;
}

Var HostDispatch::CreateDispatchWrapper(Var object, Js::ScriptContext * sourceScriptContext, Js::ScriptContext * destScriptContext)
{
    Assert(sourceScriptContext != destScriptContext);
    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr;
    bool isScriptActive = sourceScriptContext->GetThreadContext()->IsScriptActive();
    if (isScriptActive)
    {
        hr = DispatchHelper::MarshalJsVarToVariantNoThrowWithLeaveScript(object, &vt, destScriptContext);
    }
    else
    {
        hr = DispatchHelper::MarshalJsVarToVariantNoThrow(object, &vt, destScriptContext);
    }
    
    if (SUCCEEDED(hr))
    {
        Var var;
        if (isScriptActive)
        {
            hr = DispatchHelper::MarshalVariantToJsVarWithLeaveScript(&vt, &var, destScriptContext);
        }
        else
        {
            hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(&vt, &var, destScriptContext);
        }
        
        if (SUCCEEDED(hr))
        {
            VariantClear(&vt);
            return var;
        }
    }
    VariantClear(&vt);
    HandleDispatchError(destScriptContext, hr, NULL);
}

void HostDispatch::Finalize(bool isShutdown)
{
    // Finalize can be called mulitple times (from recycler and from ScriptSite::Close)
    // Check if that is the case and don't do anything
    if (isShutdown)
    {
        return;
    }
    if (linkList.Flink == NULL)
    {
        return ;
    }
    RemoveEntryList(&linkList);
    linkList.Flink = NULL;
    // release the refcount to refCountedHostVariant. We reduce the
    // refcount in finalize, and when the refcount goes down to 0, the tie is
    // cut (refCountedHostVariant->hostVariant set to NULL), and we let recycler
    // do the dispose.
    refCountedHostVariant->Release();    
}

void HostDispatch::Dispose(bool isShutdown)
{
}

IDispatch* HostDispatch::GetDispatchNoRef()
{
    if (GetHostVariant() && FastGetDispatchNoRef(GetHostVariant()) != NULL)
    {
        if (FAILED(EnsureDispatch()))
        {
            return NULL;
        }
        return GetHostVariant()->GetDispatchNoRef();
    }
    else
    {
        return NULL;
    }
}

IDispatch* HostDispatch::GetDispatch()
{
    if (GetHostVariant() && FastGetDispatchNoRef(GetHostVariant()) != NULL)
    {
        if (FAILED(EnsureDispatch()))
        {
            return NULL;
        }
        return GetHostVariant()->GetDispatch();
    }
    else
    {
        return NULL;
    }
}

IDispatch* HostDispatch::GetIDispatchAddress()
{
    return GetHostVariant()->GetIDispatchAddress();
}

BOOL HostDispatch::EqualsHelper(HostDispatch *left, HostDispatch *right, BOOL *value, BOOL strictEqual)
{
    if (FAILED(left->EnsureDispatch()) || FAILED(right->EnsureDispatch()))
    {
        *value = FALSE;
        return FALSE;
    }

    if (FastGetDispatchNoRef(left->GetHostVariant()) == FastGetDispatchNoRef(right->GetHostVariant()))
    {
        *value = TRUE;
        return TRUE;
    }

    HRESULT hr;
    IUnknown* unknownLeft = NULL, *unknownRight = NULL;
    IObjectIdentity* objectIdentity = NULL;

    Js::ScriptContext* scriptContext = left->GetScriptContext();    
    hr = left->QueryObjectInterfaceInScript(IID_IUnknown, (void **)&unknownLeft);
    if (SUCCEEDED(hr))
    {
        hr = right->QueryObjectInterfaceInScript(IID_IUnknown, (void **)&unknownRight);
        if (SUCCEEDED(hr))
        {
            if (unknownLeft == unknownRight)
            {
                *value = TRUE;
            }
            else
            {
                if (strictEqual == FALSE)
                {
                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        hr = unknownLeft->QueryInterface(IID_IObjectIdentity, (void **)&objectIdentity);
                        if (SUCCEEDED(hr) && !objectIdentity)
                        {
                            hr = E_NOINTERFACE;
                        }

                        if (SUCCEEDED(hr))
                        {
                            *value = (NOERROR == objectIdentity->IsEqualObject(unknownRight));
                            objectIdentity->Release();
                        }
                    }
                    END_LEAVE_SCRIPT(scriptContext);
                }
                else
                {
                    *value = FALSE;
                }
            }
            unknownRight->Release();
        }
        unknownLeft->Release();
    }
    return SUCCEEDED(hr) ? TRUE : FALSE;
}

Js::RecyclableObject * HostDispatch::CloneToScriptContext(Js::ScriptContext* requestContext)
{
    HRESULT hr = EnsureDispatch();     
    if (FAILED(hr))
    {
        HandleDispatchError(hr, NULL);
    }

    Recycler* recycler = requestContext->GetRecycler();
    ScriptSite* requestSite = ScriptSite::FromScriptContext(requestContext);
    HostDispatch* newHostDispatch = RecyclerNewFinalized(
        recycler,
        HostDispatch,        
        refCountedHostVariant,
        requestSite->GetActiveScriptExternalLibrary()->GetHostDispatchType());
    
    return newHostDispatch;    

}

BOOL HostDispatch::GetBuiltInOperationFromEntryPoint(Js::JavascriptMethod entryPoint, BuiltInOperation* operation)
{
    Assert(operation != NULL);

    *operation = BuiltInOperation_Unknown;

    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();

    // Try to get the operation from the cache
    uint operationId;
    if (threadContext->GetBuiltInOperationIdFromEntryPoint(entryPoint, &operationId))
    {
        *operation = (BuiltInOperation)operationId;
        return TRUE;
    }

    // Initialize the cache if it hasn't been already
    if (!threadContext->IsEntryPointToBuiltInOperationIdCacheInitialized())
    {
        BOOL isInitialized = FALSE;
        __try
        {
#define BUILT_IN_OPERATION(o, p, f) threadContext->SetBuiltInOperationIdForEntryPoint(f.GetOriginalEntryPoint(), o);
#include "BuiltInOperations.h"
#undef BUILT_IN_OPERATION
            isInitialized = TRUE;
        }
        __finally
        {
            if (!isInitialized)
            {
                threadContext->ResetEntryPointToBuiltInOperationIdCache();
            }
        }

        if (threadContext->GetBuiltInOperationIdFromEntryPoint(entryPoint, &operationId))
        {
            *operation = (BuiltInOperation)operationId;
            return TRUE;
        }
    }

    // We should never reach here
    AssertMsg(FALSE, "This JavascriptMethod is not supported.");
    return FALSE;
}

HRESULT HostDispatch::QueryInterfaceWithLeaveScript(IUnknown* obj, REFIID iid, void** returnObj, Js::ScriptContext* scriptContext)
{
    HRESULT hr;
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = obj->QueryInterface(iid, returnObj);
    }
    END_LEAVE_SCRIPT(scriptContext);
    return hr;
}

#if DBG
Js::ScriptContext * HostDispatch::GetScriptContext()
{
    Js::ScriptContext * scriptContext = __super::GetScriptContext();
    Assert(scriptSite->IsClosed() || scriptContext == scriptSite->GetScriptSiteContext());
    return scriptContext;
}
#endif
