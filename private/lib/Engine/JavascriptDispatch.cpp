/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"
#include "mshtmdid.h"
#include "JavascriptPixelArrayDispatch.h"
#include "DispIdHelper.h"
#include "var.h"
#include "typeinfobuilder.h"

#include "Library\JSON.h"
#include "Library\ArgumentsObject.h"

#include "Library\ForInObjectEnumerator.h"

// Initialization order
//  AB AutoSystemInfo
//  AD PerfCounter
//  AE PerfCounterSet
//  AM Output/Configuration
//  AN MemProtectHeap
//  AP DbgHelpSymbolManager
//  AQ CFGLogger
//  AR LeakReport
//  AS JavascriptDispatch/RecyclerObjectDumper
//  AT HeapAllocator/RecyclerHeuristic
#pragma warning(disable:4075)
#pragma init_seg(".CRT$XCAS")

template JavascriptDispatch* JavascriptDispatch::Create<true>(Js::DynamicObject* scriptObject);
template JavascriptDispatch* JavascriptDispatch::Create<false>(Js::DynamicObject* scriptObject);

///----------------------------------------------------------------------------
///
/// JavascriptDispatch
///  Use Create to make sure the allocation used is correct
///
///----------------------------------------------------------------------------
template <bool inScript>
JavascriptDispatch* JavascriptDispatch::Create(Js::DynamicObject* scriptObject)
{
    Js::ScriptContext * scriptContext = scriptObject->GetScriptContext();
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    ScriptSite::DispatchMap * dispMap = scriptSite->EnsureDispatchMap();
    JavascriptDispatch* jsdisp = nullptr;

    if (inScript)
    {
        Assert(scriptContext->GetThreadContext()->IsScriptActive());
    }
    else
    {
        Assert(!scriptContext->GetThreadContext()->IsScriptActive());
    }
    // the access denied check is done in the caller.
    if (scriptObject->IsExternal() && inScript)
    {
        Assert(!scriptContext->GetThreadContext()->IsDisableImplicitException());
        scriptContext->VerifyAlive();
    }

    if (!dispMap->TryGetValue(scriptObject, &jsdisp))
    {
        Js::CustomExternalObject * customExternalScriptObject = Js::JavascriptOperators::TryFromVar<Js::CustomExternalObject>(scriptObject);
        if (customExternalScriptObject)
        {
            jsdisp = customExternalScriptObject->GetCachedJavascriptDispatch();
            if (jsdisp)
            {
                AssertMsg(!jsdisp->isGCTracked, "JD wrapping CEO shouldn't be GC tracked");
                Assert(jsdisp->scriptSite == nullptr);
                Assert(jsdisp->scriptObject != nullptr);
                jsdisp->scriptSite = scriptSite;
                scriptSite->AddRef();
                scriptSite->AddDispatchCount();
#ifdef TRACK_JS_DISPATCH
                jsdisp->LogAlloc();
#endif
            }
        }

        if (jsdisp == nullptr)
        {
            // pixel array is only implemented as Uint8ClampedArray in edge mode.
            if (scriptObject->GetTypeId() == Js::TypeIds_Uint8ClampedArray)
            {
                // Pixel array has its own object that implements some extra interfaces
                jsdisp = RecyclerNewFinalizedClientTracked(scriptContext->GetRecycler(), JavascriptPixelArrayDispatch, scriptObject, scriptSite);
            }
            else
            {
                jsdisp = RecyclerNewFinalizedClientTracked(scriptContext->GetRecycler(), JavascriptDispatch, scriptObject, scriptSite);
            }
        }

        if (!scriptSite->IsClosed() && scriptContext->IsFastDOMEnabled() &&
            (Js::JavascriptFunction::Is(scriptObject) || Js::ExternalObject::Is(scriptObject)))
        {
            scriptSite->AddToJavascriptDispatchList(&(jsdisp->linkList));
            Assert(Js::JavascriptOperators::GetTypeId(scriptObject) != Js::TypeIds_HostDispatch);
        }
        else
        {
            InitializeListHead(&(jsdisp->linkList));
        }
        AssertMsg(!dispMap->ContainsKey((void*)scriptObject), "Duplicate dispatch map entries for a JS object");
        dispMap->Add((void*)scriptObject, jsdisp);
#if DBG
        jsdisp->isFinishCreated = true;
#endif
    }
    else
    {
        Assert(jsdisp != nullptr);
        Assert(jsdisp->GetObject() == scriptObject);
    }
    return jsdisp;
}

JavascriptDispatch::JavascriptDispatch(
   __in Js::DynamicObject* scriptObject,
   __in ScriptSite *inScriptSite)
   : refCount(0),
   scriptSite(inScriptSite),
   scriptObject(scriptObject),
   dispIdEnumerator(nullptr),
   isGCTracked(false),
   isFinalized(false),
   isInCall(false),
   dispIdPropertyStringMap(nullptr)
#if DBG
   ,isFinishCreated(false)
#endif
{
    Assert(inScriptSite->IsClosed() || inScriptSite->GetScriptSiteContext() == scriptObject->GetScriptContext());
    scriptSite->AddRef();
    scriptSite->AddDispatchCount();
#ifdef TRACK_JS_DISPATCH
    LogAlloc();
#endif
}

HRESULT JavascriptDispatch::QueryInterface(REFIID riid, void **ppvObj)
{
    // Any Thread.
    bool addRef = true;
    AssertThis();
    AssertMem(ppvObj);
    IfNullReturnError(ppvObj, E_INVALIDARG);

    // scriptObject can be NULL if we are dealing with an external object or function after
    // the script engine closes. If this is the case we should respond to IUnknown directly
    // instead of forwarding (change of identity to COM) and avoid our fallback to QueryObjectInterface.
    if (IID_IUnknown == riid && (scriptObject == nullptr || !scriptObject->IsExternal()))
    {
        *ppvObj = (IDispatchEx *)this;
    }
    else if (__uuidof(IDispatchEx) == riid)
    {
        *ppvObj = (IDispatchEx *)this;
    }
    else if (__uuidof(IDispatch) == riid )
    {
        *ppvObj = (IDispatch *)this;
    }
    else if(__uuidof(IJavascriptDispatchLocalProxy) == riid )
    {
        *ppvObj = (IJavascriptDispatchLocalProxy*)this;
    }
    else if (IID_IJavascriptDispatchRemoteProxy == riid)
    {
        *ppvObj = (IJavascriptDispatchRemoteProxy*)this;
    }
    else if (IID_ITrackerJS9 == riid)
    {
        *ppvObj = nullptr;
        return E_NOINTERFACE;
    }
    else if (IID_IJsArray == riid)
    {
        if (scriptObject != nullptr && Js::JavascriptArray::Is(scriptObject))
        {
            // Identity check for mshtml.dll.
            *ppvObj = nullptr;
            return E_NOTIMPL;
        }
        return E_NOINTERFACE;
    }
    else if (IID_IJsArguments == riid)
    {
        if (scriptObject != nullptr && Js::ArgumentsObject::Is(scriptObject))
        {
            // Identity check for mshtml.dll.
            *ppvObj = nullptr;
            return E_NOTIMPL;
        }
        return E_NOINTERFACE;
    }
    else if (__uuidof(IEnumVARIANT) == riid)
    {
        AssertMsg(false, "Enumerator is removed in Chakra.");
        *ppvObj = nullptr;
        return E_NOINTERFACE;
    }
    else
    {
        if (nullptr != scriptObject)
        {
            HRESULT hr = NOERROR;
#if DBG
            Js::ScriptContext* scriptContext = this->GetScriptContext();
#endif
            hr = QueryObjectInterface(riid, ppvObj);
            if (SUCCEEDED(hr))
            {
                // Don't need to add ref the javascript dispatch, we got the interface somewhere else
                addRef = false;
            }
            else
            {
                // If the external object is a DOM prototype object, the QI call will
                // be passed through back to us and fail will with E_NOINTERFACE.
                // If the QI is for IUnknown, we don't want it to fail. Instead, we'll
                // return our own IUnknown implementation.
                if (IID_IUnknown == riid && E_NOINTERFACE == hr)
                {
                    *ppvObj = (IDispatchEx *)this;
                    hr = S_OK;
                }
            }
            if (FAILED(hr))
            {
                *ppvObj = nullptr;
                VERIFYHRESULTBEFORERETURN(hr, scriptContext);
                return hr;
            }
        }
        else
        {
            *ppvObj = nullptr;
            return E_NOINTERFACE;
        }
    }
    if (addRef)
    {
        AddRef();
    }

    return S_OK;
}

ULONG JavascriptDispatch::AddRef(void)
{
    ULONG currentRefCount = InterlockedIncrement(&refCount);

    if (currentRefCount == 1)
    {
        HRESULT hr = NOERROR;
        // AddRef could be called both in scriptActive and !isScriptActive
        // but we won't throw javascriptexception here, so we don't need to
        // catch javascriptexceptions.
        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
        {
            scriptSite->GetRecycler()->RootAddRef(this);
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        if (FAILED(hr))
        {
            JavascriptDispatch_OOM_fatal_error((ULONG_PTR)this);
            AssertMsg(FALSE, "cannot continue execution");
        }

    }
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
    if (Js::Configuration::Global.flags.LeakStackTrace)
    {
        Assert(trackNode != nullptr);
        StackBackTraceNode::Prepend(&NoCheckHeapAllocator::Instance, trackNode->refCountStackBackTraces,
            StackBackTrace::Capture(&NoCheckHeapAllocator::Instance, StackToSkip, StackTraceDepth));
    }
#endif
    return currentRefCount;
}

void JavascriptDispatch::RemoveFromDispatchMap()
{
    if (scriptObject != nullptr)
    {
        ScriptSite::DispatchMap * dispMap = scriptSite->GetDispatchMap();
        bool removed = false;
        if (dispMap != nullptr)
        {
            removed = dispMap->Remove(scriptObject);
        }
        AssertMsg(removed || !isFinishCreated, "Failed to remove a JS Dispatch from the map");
    }
}

void JavascriptDispatch::Finalize(bool isShutdown)
{
    Assert(isShutdown || refCount == 0);
    Assert(scriptSite != nullptr || (!isGCTracked && refCount == 0));
    isFinalized = true;
    if (scriptSite != nullptr)
    {
        Assert(isGCTracked || isShutdown || !isFinishCreated);
        RemoveFromDispatchMap();
    }
}

void JavascriptDispatch::Dispose(bool isShutdown)
{
    Assert(isShutdown || refCount == 0);
    Assert(scriptSite != nullptr || (!isGCTracked && refCount == 0));
    Assert(isFinalized);
    if (scriptSite != nullptr)
    {
        Assert(isGCTracked || isShutdown || !isFinishCreated);
        if (!scriptSite->isClosed)
        {
            RemoveEntryList(&linkList);
        }
        scriptSite->ReleaseDispatchCount();
        scriptSite->Release();
#ifdef TRACK_JS_DISPATCH
        LogFree(isShutdown);
#endif
    }
}

ULONG JavascriptDispatch::Release(void)
{
    AssertMsg(refCount != 0, "invalid release of JavascriptDispatch");
    Assert(isFinishCreated);

    ULONG currentCount = InterlockedDecrement(&refCount);

    if (currentCount == 0)
    {
        // By default we always AddRef the scriptsite and scriptObject. If we are GC tracked, we don't need to
        // hold on to them directly anymore. GC will get hold of them.
        Recycler* recycler = scriptSite->GetRecycler();

        if (!isGCTracked)
        {
            // normal code path here.

            RemoveFromDispatchMap();

            if (!scriptSite->isClosed)
            {
                RemoveEntryList(&linkList);
            }
            scriptSite->ReleaseDispatchCount();
            scriptSite->Release();
            scriptSite = nullptr;
#ifdef TRACK_JS_DISPATCH
            LogFree();
#endif
        }
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
        else if (Js::Configuration::Global.flags.LeakStackTrace)
        {
            Assert(trackNode != nullptr);
            StackBackTraceNode::DeleteAll(&NoCheckHeapAllocator::Instance, trackNode->refCountStackBackTraces);
        }
#endif
        // Make sure we unpin after we clean up everything else.
        // Otherwise, this javascript dispatch might have been swept during Recycler::Release
        // because JavascriptDsipatch::Release is on a vtable offset by 4 and so the this pointer
        // is not the start of the object.
        recycler->RootRelease(this);
    }
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
    else if (Js::Configuration::Global.flags.LeakStackTrace)
    {
        Assert(trackNode != nullptr);
        StackBackTraceNode::Prepend(&NoCheckHeapAllocator::Instance, trackNode->refCountStackBackTraces,
            StackBackTrace::Capture(&NoCheckHeapAllocator::Instance, StackToSkip, StackTraceDepth));
    }
#endif 
    return currentCount;
}


HRESULT JavascriptDispatch::GetTypeInfoCount(UINT *pctinfo)
{
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    HRESULT hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);

    *pctinfo = 1;
    return NOERROR;
}

HRESULT JavascriptDispatch::GetTypeInfo(UINT iti, LCID lcid, ITypeInfo **ppti)
{
    *ppti = nullptr;
    if (0 != iti)
        return DISP_E_BADINDEX;

    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    HRESULT hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);

    AutoActiveCallPointer autoActiveCallPointer(this);
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        hr = GetTypeInfoWithScriptEnter(iti, lcid, ppti);
    }
    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

    return hr;
}

HRESULT JavascriptDispatch::GetTypeInfoWithScriptEnter(UINT iti, LCID lcid, ITypeInfo **ppti)
{
    HRESULT hr = E_FAIL;
    AutoReleasePtr<TypeInfoBuilder> typeinfoBuild;
    Js::ScriptContext* scriptContext = this->GetScriptContext();

    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
    {

        hr = TypeInfoBuilder::Create(_u("JScriptTypeInfo"), lcid, &typeinfoBuild);
        if (SUCCEEDED(hr))
        {
            hr = typeinfoBuild->AddJavascriptObject(scriptObject);
        }
        if (SUCCEEDED(hr))
        {
            hr = typeinfoBuild->GetTypeInfo(ppti);
        }
    }
    END_JS_RUNTIME_CALL(scriptContext)
    return hr;
}

//----------------------------------------------------------------------------------
//
// JavascriptDispatch::GetIDsOfNames
//
// Get the field index of the given name. (Note that we only search for the first name we're given.
// This is compatible with legacy behavior.)
// If the field index is known and is present on our script object's type, pass it back.
// Otherwise, fail with DISP_E_UNKNOWNNAME.
//
//----------------------------------------------------------------------------------

HRESULT JavascriptDispatch::GetIDsOfNames(REFIID riid, __in_ecount(cpsz) LPOLESTR *prgpsz, UINT cpsz, LCID lcid, __in_ecount(cpsz) DISPID *prgid)
{
    UINT i;
    HRESULT hr = NOERROR;
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);

    if (riid != IID_NULL)
    {
        return DISP_E_UNKNOWNINTERFACE;
    }

    if (cpsz < 1)
    {
        return DISP_E_UNKNOWNNAME;
    }

    AutoActiveCallPointer autoActiveCallPointer(this);
    for (i = 0; i < cpsz; i++)
    {
        prgid[i] = DISPID_UNKNOWN;
    }

    // Even though the contract for IDispatch::GetIdsOfNames specifies it to be case insensitive,
    // legacy Jscript implements this as a case sensitive comparison. To ensure
    // compatibility, we have chosen to implement it as case sensitive for all non-external
    // javascript objects.
    // See WOOB 1125860 for more details.
    BSTR bstrName = ::SysAllocString(prgpsz[0]);
    hr = (bstrName != nullptr) ? S_OK : E_OUTOFMEMORY;
    IfFailedReturn(hr);

    if (!scriptObject->IsExternal())
    {
        hr = GetDispID(bstrName, fdexNameCaseSensitive, &prgid[0]);
    }
    else
    {
        hr = GetDispID(bstrName, fdexNameCaseInsensitive, &prgid[0]);
    }

    ::SysFreeString(bstrName);
    return hr;
}

//----------------------------------------------------------------------------------
//
// JavascriptDispatch::Invoke
//
// IDispatch::Invoke simply maps onto IDispatchEx::InvokeEx, which does the real work.
//
//----------------------------------------------------------------------------------

HRESULT JavascriptDispatch::Invoke(DISPID id, REFIID riid, LCID lcid, WORD wFlags,
                                   DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, UINT *puArgErr)
{
    if (IID_NULL != riid)
    {
        if (nullptr != pvarRes)
        {
            VariantInit(pvarRes);
        }
        if (nullptr != pei)
        {
            memset(pei, 0, sizeof(*pei));
        }
        return DISP_E_UNKNOWNINTERFACE;
    }
    return InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, nullptr);
}

HRESULT JavascriptDispatch::OP_InitPropertyWithScriptEnter(Var instance, PropertyId propertyId, Var newValue)
{
    HRESULT hr = NOERROR;
    Js::ScriptContext* scriptContext = this->GetScriptContext();
    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
    {
        hr = (Js::JavascriptOperators::OP_InitProperty(instance, propertyId, newValue) ? S_OK : E_FAIL);
    }
    END_JS_RUNTIME_CALL(scriptContext);
    return hr;
}

//----------------------------------------------------------------------------------
//
// JavascriptDispatch::GetDispID
//
// Similar to GetIDsOfNames, except that we accept a flag that forces us to create
// the given field on our object if it does not yet exist.
//
//----------------------------------------------------------------------------------

HRESULT JavascriptDispatch::GetDispID(BSTR bstr, DWORD grfdex, DISPID *pid)
{
    HRESULT hr = DISP_E_UNKNOWNNAME;

    AssertThis();
    AssertMem(pid);

    *pid = DISPID_UNKNOWN;
    int nameLength = bstr ? (int)wcslen(bstr) : 0;
    Js::PropertyId propertyId = Js::Constants::NoProperty;
    uint32 indexVal;

    HRESULT hr1;
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr1 = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr1);

    Assert(scriptObject != nullptr);
    AutoActiveCallPointer autoActiveCallPointer(this);

    BOOL isPropertyId = TRUE;

    // Don't let ModuleRoot defer to GlobalObject here. We don't want the host to request a global
    // property from the ModuleRoot; we want to get it from the GlobalObject directly. Enforce fdexnameInternal
    Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(scriptObject);
    if (typeId == Js::TypeIds_ModuleRoot || typeId == Js::TypeIds_GlobalObject)
    {
        grfdex |= fdexNameInternal;
    }
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        Js::PropertyRecord const * propertyRecord = nullptr;
        hr = GetPropertyIdWithFlagWithScriptEnter(bstr, grfdex, &propertyId, &indexVal, &isPropertyId, &propertyRecord);
        if (SUCCEEDED(hr))
        {
            if (propertyRecord)
            {
                Assert(propertyRecord->GetPropertyId() == propertyId);
                CachePropertyId(propertyRecord, isPropertyId);
            }
        }
        else
        {
            if(grfdex & fdexNameEnsure)
            {
                Js::ScriptContext * scriptContext = this->GetScriptContext();
                if (Js::JavascriptOperators::TryConvertToUInt32(bstr, nameLength, &indexVal) &&
                    (indexVal != Js::JavascriptArray::InvalidIndex))
                {
                    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
                    Js::JavascriptOperators::SetItem(scriptObject, scriptObject, indexVal, scriptContext->GetLibrary()->GetNull(), scriptContext);
                    END_JS_RUNTIME_CALL(scriptContext);
                    scriptContext->GetOrAddPropertyRecord(bstr, nameLength, &propertyRecord);
                    CachePropertyId(propertyRecord);
                    propertyId = propertyRecord->GetPropertyId();
                }
                else
                {
                    if (propertyId == Js::Constants::NoProperty)
                    {
                        // the type handler should keep the propertyid alive. We don't need to cache the property here.
                        scriptContext->GetOrAddPropertyRecord(bstr, nameLength, &propertyRecord);
                        propertyId = propertyRecord->GetPropertyId();
                    }

                    OP_InitPropertyWithScriptEnter(scriptObject, propertyId, scriptContext->GetLibrary()->GetNull());
                }
                hr = S_OK;
            }
            else
            {
                propertyId = (Js::PropertyId)DISPID_UNKNOWN;
            }
        }
    }
    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);

    if (SUCCEEDED(hr))
    {
        *pid = (DISPID) propertyId;

        if (scriptObject->IsExternal() && isPropertyId)
        {
            // We need to offset our DISPID return value to be in a known range for external objects.
            // This is so that callers who go directly to InvokeEx can continue to use known DISPIDs in the host namespace
            // Note that we don't do this if return value isn't really a propertyId, but rather a legacy DISPID.
            // See comment in GetPropertyIdWithFlag, where we fallback to the IDispatchExInternal
            (*pid) = PropertyIdToExpandoDispId(*pid);
        }
    }

    return hr;
}

HRESULT JavascriptDispatch::GetPropertyIdWithFlagWithScriptEnter(__in BSTR bstrName, DWORD grfdex, __out Js::PropertyId* propertyId, uint32* indexVal, __out BOOL* pIsPropertyId, __out Js::PropertyRecord const ** propertyRecord)
{
    HRESULT hr = NOERROR;
    Js::ScriptContext* scriptContext = this->GetScriptContext();
    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
    {
        hr = GetPropertyIdWithFlag(bstrName, grfdex, propertyId, indexVal, pIsPropertyId, propertyRecord);
    }
    END_JS_RUNTIME_CALL(scriptContext);
    return hr;
}

HRESULT JavascriptDispatch::GetPropertyIdWithFlag(__in BSTR bstrName, DWORD grfdex, __out Js::PropertyId* propertyId, uint32* indexVal,
    __out BOOL* pIsPropertyId, __out Js::PropertyRecord const ** propertyRecord)
{
    Js::PropertyId newPropertyId = Js::Constants::NoProperty;
    *propertyId = Js::Constants::NoProperty;
    *propertyRecord = nullptr;
    HRESULT hr = DISP_E_UNKNOWNNAME;
    Js::ScriptContext* scriptContext = this->GetScriptContext();

    *pIsPropertyId = TRUE;

    int nameLength = bstrName ? (int)wcslen(bstrName) : 0;
    if (bstrName == nullptr)
    {
    }
    else if (Js::JavascriptOperators::TryConvertToUInt32(bstrName, nameLength, indexVal) &&
        (*indexVal != Js::JavascriptArray::InvalidIndex))
    {
        if (Js::JavascriptOperators::HasItem(scriptObject, *indexVal))
        {
            // the object don't hold index property Id, we need to cache it. return the property record.
            scriptContext->GetOrAddPropertyRecord(bstrName, nameLength, propertyRecord);
            newPropertyId = (*propertyRecord)->GetPropertyId();
            hr = S_OK;
        }
    }
    else if (grfdex & fdexNameCaseInsensitive)
    {
        // DOM objects use deferred type system initialization, so property IDs may not be provided yet.
        // Before we proceed with the lookup we need to force the undeferral which will happen when we call object on HasProperty.
        if (scriptObject->IsExternal())
        {
            Js::JavascriptOperators::OP_HasProperty(scriptObject, Js::Constants::NoProperty, scriptContext);
        }

        JsUtil::List<const RecyclerWeakReference<Js::PropertyRecord const>*>* list = scriptContext->FindPropertyIdNoCase((LPWSTR)bstrName, nameLength);
        if (list != nullptr)
        {
            AssertMsg(list->Count() > 0, "The list must contain at least one entry or else it would not have been added to the propertyMapNoCase dictionary.");

            Js::PropertyRecord const* caseSensitivePropertyRecord = nullptr;
            scriptContext->FindPropertyRecord((LPCWSTR) bstrName, nameLength, &caseSensitivePropertyRecord);

            if (caseSensitivePropertyRecord != nullptr)
            {
                // We want to prefer the case sensitive matched string so lets walk through the list and
                // if we find it, move it to the front of the list
                for (int i = 0; i < list->Count(); i++)
                {
                    const RecyclerWeakReference<Js::PropertyRecord const>* currentPropertyStringWeakRef = list->Item(i);
                    Js::PropertyRecord const * currentPropertyRecord = currentPropertyStringWeakRef->Get();
                    if (currentPropertyRecord == nullptr)
                    {
                        continue;
                    }

                    // If the property record found is the same as the case sensitive one,
                    // this item needs to move to the head of the list
                    if (currentPropertyRecord == caseSensitivePropertyRecord)
                    {
                        // Only swap if it's not already at the front
                        if (i != 0)
                        {
                            const RecyclerWeakReference<Js::PropertyRecord const>* temp = list->Item(0);

                            list->SetExistingItem(0, currentPropertyStringWeakRef);
                            list->SetExistingItem(i, temp);
                        }
                        break;
                    }
                }
            }

            // This loop prefers properties that were added first. This is done by comparing the property
            // index of each item. Prototype properties are given lower priority because they will not have a
            // property index on the current object.

            // bestIndex is used for two things: ignore propertyId without real property,
            // if there are existing property, find the one with lowest index, ie, added first.
            Js::PropertyIndex bestIndex = Js::Constants::UShortMaxValue;
            Assert(Js::Constants::UShortMaxValue == Js::Constants::NoSlot);
            for (int i = 0; i < list->Count(); i++)
            {
                const RecyclerWeakReference<Js::PropertyRecord const>* currentPropertyStringWeakRef = list->Item(i);
                Js::PropertyRecord const * currentPropertyRecord = currentPropertyStringWeakRef->Get();
                if (currentPropertyRecord == nullptr)
                {
                    continue;
                }

                Js::PropertyId currentPropertyId = currentPropertyRecord->GetPropertyId();
                Js::PropertyIndex currentIndex = scriptObject->GetPropertyIndex(currentPropertyId);
                if (currentIndex >= 0 && currentIndex < bestIndex)
                {
                    hr = S_OK;
                    bestIndex = currentIndex;
                    newPropertyId = currentPropertyId;
                }
                else if (newPropertyId == Js::Constants::NoProperty)
                {
                    if(grfdex & fdexNameInternal)
                    {
                        // look only on the local scope, do not reach for the dispatch host
                        AssertMsg(Js::JavascriptOperators::GetTypeId(scriptObject) == Js::TypeIds_ModuleRoot ||
                            Js::JavascriptOperators::GetTypeId(scriptObject) == Js::TypeIds_GlobalObject,
                            "Bad type, root object assumed");
                        if (Js::JavascriptConversion::PropertyQueryFlagsToBoolean(scriptObject->DynamicObject::HasPropertyQuery(currentPropertyId)))
                        {
                            newPropertyId = currentPropertyId;
                            hr = S_OK;
                        }
                    }
                    else if (Js::JavascriptOperators::OP_HasProperty(scriptObject, currentPropertyId, scriptContext))
                    {
                        newPropertyId = currentPropertyId;
                        hr = S_OK;
                    }
                }
            }
        }
        if (DISP_E_UNKNOWNNAME == hr && scriptObject->IsExternal())
        {
            // ATTENTION. in case insensitive lookup, aka VBScript, we shouldn't blindly add non-found properties to
            // our property name list. If we are being call from VB, and the script object is from DOM, we should
            // just delegate the call to DOM's Dispatch method and let them handle this.
            // We need this code because in fastDOM, JavascriptDispatch is being used as the default IDispatch interface
            // for the DOM objects as well, and we are in the front line to do delegations between script holders, rather
            // than just being scanned as one of the script holders.
            IDispatchEx* pDispEx;

            /* REVIEW: Do we need to handle exception here? */
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                if (SUCCEEDED(scriptObject->QueryObjectInterface(IID_IDispatchExInternal, (void**)&pDispEx)))
                {
                    hr = pDispEx->GetDispID(bstrName, grfdex, (DISPID*)&newPropertyId);
                    if (SUCCEEDED(hr))
                    {
                        // Notify our call chain that this is not a propertyId, but a legacy DISPID
                        *pIsPropertyId = FALSE;
                    }

                    pDispEx->Release();
                }
            }
            END_LEAVE_SCRIPT(scriptContext);
        }
    }
    else
    {
        Js::PropertyRecord const * localPropertyRecord;
        scriptContext->GetOrAddPropertyRecord(bstrName, nameLength, &localPropertyRecord);
        newPropertyId = localPropertyRecord->GetPropertyId();
        if(grfdex & fdexNameInternal)
        {
            // look only on the local scope, do not reach for the dispatch host
            Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(scriptObject);
            AssertMsg(Js::JavascriptOperators::GetTypeId(scriptObject) == Js::TypeIds_ModuleRoot ||
                Js::JavascriptOperators::GetTypeId(scriptObject) == Js::TypeIds_GlobalObject,
                "Bad type, root object assumed");
            if (Js::JavascriptConversion::PropertyQueryFlagsToBoolean(scriptObject->DynamicObject::HasPropertyQuery(newPropertyId)))
            {
                hr = S_OK;
            }
            else if (typeId == Js::TypeIds_GlobalObject)
            {
                if (Js::JavascriptOperators::OP_HasProperty(scriptObject->GetPrototype(), newPropertyId, scriptContext))
                {
                    hr = S_OK;
                }
            }
        }
        else
        {
            Js::GlobalObject* globalObject = scriptObject->GetLibrary()->GetGlobalObject();
            if (scriptObject->IsExternal() &&
                globalObject &&
                scriptObject == globalObject->GetSecureDirectHostObject() &&
                scriptContext->CanOptimizeGlobalLookup())
            {
                if (globalObject->HasProperty(newPropertyId))
                {
                    // The property is either global property or DOM property. the propertyid should either
                    // have been in the tracked list (js property), or should be added to it (dom element),
                    // as global element name cannot change.
                    // After close, HasProperty can succeed only for local jscript properties, so we don't need
                    // to add track (type handler should cache it).
                    if (!scriptContext->IsClosed())
                    {
                        scriptContext->TrackPid(localPropertyRecord);
                    }
                    hr = S_OK;
                }
            }
            else
            {
                if (Js::JavascriptOperators::OP_HasProperty(scriptObject, newPropertyId, scriptContext))
                {
                    // The property might be in a host dispatch object, better cache the property Id
                    *propertyRecord = localPropertyRecord;
                    hr = S_OK;
                }
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        *propertyId = newPropertyId;
    }
    return hr;
}

HRESULT JavascriptDispatch::CreateSafeArrayOfProperties(__out VARIANT* pvarRes)
{
    HRESULT hr = NOERROR;
    if (nullptr == pvarRes)
    {
        return NOERROR;
    }

    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        hr = CreateSafeArrayOfPropertiesWithScriptEnter(pvarRes);
    }
    TRANSLATE_EXCEPTION_TO_HRESULT_ENTRY(const Js::JavascriptException& err)
    {
        ActiveScriptError * pase = nullptr;
        Js::JavascriptExceptionObject * pError = err.GetAndClear();
        Js::ScriptContext * errorScriptContext = pError->GetScriptContext() ? pError->GetScriptContext() : this->GetScriptContext();
        if (SUCCEEDED(ActiveScriptError::CreateRuntimeError(pError, &hr, nullptr, errorScriptContext, &pase)))
        {
            if (errorScriptContext != nullptr && !errorScriptContext->IsClosed())
            {
                ScriptSite * errorScriptSite = ScriptSite::FromScriptContext(errorScriptContext);
                Assert(errorScriptSite != nullptr);
                if (NOERROR == errorScriptSite->GetScriptEngine()->OnScriptError((IActiveScriptError *) IACTIVESCRIPTERROR64 pase))
                {
                    hr = SCRIPT_E_REPORTED;
                }
            }
            pase->Release();
        }
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    return hr;
}

class AutoSafeArray
{
public:
    AutoSafeArray(SAFEARRAY * safeArray = nullptr) : safeArray(safeArray) {}
    ~AutoSafeArray()
    {
        if (safeArray != nullptr)
        {
            SafeArrayDestroy(safeArray);
        }
    }
    operator SAFEARRAY *() { return safeArray; }
    SAFEARRAY * Detach() { SAFEARRAY * ret = safeArray; safeArray = nullptr; return ret; }
    SAFEARRAY * operator->() { Assert(safeArray != nullptr); return safeArray; }
private:
    SAFEARRAY * safeArray;
};

HRESULT JavascriptDispatch::CreateSafeArrayOfPropertiesWithScriptEnter(VARIANT* pvarRes)
{
    HRESULT hr = S_FALSE;
    Js::ScriptContext* scriptContext = this->GetScriptContext();

    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
    {
        uint32 length;
        length = Js::JavascriptConversion::ToUInt32(Js::JavascriptOperators::OP_GetLength(scriptObject, scriptContext), scriptContext);
        // simulate old engine behavior
        if (length > 1000000)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            Js::Var propertyVar;
            VARIANT* variants;
            SAFEARRAYBOUND arrayBound;

            arrayBound.cElements = length;
            arrayBound.lLbound = 0;
            AutoSafeArray localSafeArray(SafeArrayCreate(VT_VARIANT, 1, &arrayBound));
            if (nullptr == localSafeArray)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                hr = E_FAIL;
                variants = (VARIANT*)localSafeArray->pvData;
                uint32 i = 0;
                for (; i < length; i++)
                {
                    Js::PropertyRecord const * propertyRecord = nullptr;
                    Js::JavascriptOperators::GetPropertyIdForInt(i, scriptContext, &propertyRecord);
                    propertyVar = Js::JavascriptOperators::GetProperty(scriptObject, propertyRecord->GetPropertyId(), scriptContext);
                    hr = DispatchHelper::MarshalJsVarToVariantNoThrowWithLeaveScript(propertyVar, &variants[i], scriptContext);
                    if (FAILED(hr))
                    {
                        break;
                    }
                }

                if (i == length)
                {
                    hr = S_OK;
                    pvarRes->vt = VTE_ARRAY;
                    pvarRes->parray = localSafeArray.Detach();
                }
            }
        }
    }
    END_JS_RUNTIME_CALL(scriptContext)

    return hr;
}

Js::ScriptContext *
JavascriptDispatch::GetScriptContext()
{
    Assert(scriptObject != nullptr);
    Js::ScriptContext* scriptContext = scriptObject->GetScriptContext();
    Assert(scriptSite->IsClosed() || scriptSite->GetScriptSiteContext() == scriptContext);
    return scriptContext;
}

//----------------------------------------------------------------------------------
//
// JavascriptDispatch::InvokeEx
//
// Invoke the action described by wFlags on either our own script object (id <= 0)
// or on the member that is identified by a positive id value.
//
//----------------------------------------------------------------------------------

HRESULT JavascriptDispatch::InvokeEx(
                                     DISPID              id,
                                     LCID                lcid,
                                     WORD                wFlags,
                                     DISPPARAMS *        pdp,
                                     VARIANT *           pvarRes,
                                     EXCEPINFO *         pei,
                                     IServiceProvider *  pspCaller)
{
    HRESULT         hr = S_OK;

    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);

    Js::ScriptContext* scriptContext = this->GetScriptContext();
    AutoActiveCallPointer autoActiveCallPointer(this);

    // Initialize out parameters.
    if (pvarRes != nullptr)
    {
        VariantInit(pvarRes);
    }

    if (pei != nullptr)
    {
        memset(pei, 0, sizeof(*pei));
    }

    // Validate in parameters.
    if (pdp == nullptr || pdp->cArgs < pdp->cNamedArgs)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (!(wFlags & k_dispAll) ||
            (wFlags & ~k_dispAll) ||
            ((wFlags & k_dispCallOrGet) && (wFlags & ~k_dispCallOrGet)) ||
            ((wFlags & k_dispPutOrPutRef) && (wFlags & ~k_dispPutOrPutRef)) ||
            ((wFlags & DISPATCH_CONSTRUCT) && (wFlags & ~DISPATCH_CONSTRUCT)))
        {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr))
    {
        AddRef();
        AutoCallerPointer callerPointer(scriptSite, pspCaller);

        bool fExternalInvoke = false;
        if (scriptObject->IsExternal())
        {
            if (IsExpandoDispId(id))
            {
                // Subtract out the known offset to get us back into the internal propertyId range.
                id = ExpandoDispIdToPropertyId(id);
            }
            else if (id != DISPID_VALUE)
            {
                fExternalInvoke = true;

                // Fall back to the internal implementation of IDispatch for custom external types to find known DISPIDs.
                // We need a mechanism to use the internal IDispatch implementation for InvokeEx with known dispids.
                // A binary caller may simply InvokeEx without first using GetDispID so the JavascriptDispatch will not find the property.
                // As a fallback, we can QueryObjectInterface for this special IID looking for the internal IDispatchEx to try the InvokeEx on.
                IDispatchEx* pDispEx;
                if (SUCCEEDED(QueryObjectInterface(IID_IDispatchExInternal, (void**)&pDispEx)))
                {
                    hr = pDispEx->InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);
                    pDispEx->Release();
                }
            }
        }

        if (SUCCEEDED(hr) && !fExternalInvoke)
        {
            if (id > 0)
            {
                hr = InvokeOnMember(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);
            }
            else
            {
                const DISPID DISPID_GET_SAFEARRAY = -2700L;
                if (id == DISPID_GET_SAFEARRAY)
                {
                    hr = CreateSafeArrayOfProperties(pvarRes);
                }
                else
                {
                    hr = InvokeOnSelf(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);
                }
            }
        }


#if defined(PROFILE_EXEC) || defined(PROFILE_MEM)
        bool doPrintProfile = false;
#ifdef PROFILE_EXEC
        doPrintProfile = doPrintProfile || Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag);
#endif
#ifdef PROFILE_MEM
        if (MemoryProfiler::IsTraceEnabled())
        {
            doPrintProfile = true;
        }
#endif
        if (doPrintProfile && scriptSite->GetParentScriptSite() == nullptr &&
            Js::JavascriptFunction::Is(this->scriptObject))
        {
            Js::JavascriptFunction *func = Js::JavascriptFunction::FromVar(this->scriptObject);
            // We are profiling, so we can afford a check if the function is deserialized
            if ((wFlags & k_dispCallOrGet) && func->GetFunctionProxy()
                && wcscmp(func->GetFunctionProxy()->EnsureDeserialized()->GetDisplayName(), _u("onload")) == 0)
            {
                scriptSite->DumpSiteInfo(_u("OnLoad event"));
#ifdef PROFILE_EXEC
                if (Js::Configuration::Global.flags.IsEnabled(Js::ProfileFlag))
                {
                    scriptSite->GetScriptSiteContext()->ProfilePrint();
                }
#endif
#ifdef PROFILE_MEM
                if (MemoryProfiler::IsTraceEnabled())
                {
                    MemoryProfiler::PrintCurrentThread();
#ifdef PROFILE_RECYCLER_ALLOC
                    if (Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::AllPhase)
                        || Js::Configuration::Global.flags.TraceMemory.IsEnabled(Js::RunPhase))
                    {

                        scriptSite->GetScriptSiteContext()->GetRecycler()->PrintAllocStats();
                    }
#endif
                }
#endif
            
            }
            
        }
#endif
        if (FAILED(hr) && !fExternalInvoke && hr != SCRIPT_E_REPORTED && hr != SCRIPT_E_RECORDED  && hr != SCRIPT_E_PROPAGATE && pei)
        {
            pei->scode = GetScode(MapHr(hr));
            hr = DISP_E_EXCEPTION;
        }
        Release();
    }

    VERIFYHRESULTBEFORERETURN(hr, scriptContext);
    return hr;
}

//----------------------------------------------------------------------------------
//
// JavascriptDispatch::InvokeBuiltInOperation
//
// Invoke a built in operation on the script object
//
//----------------------------------------------------------------------------------
HRESULT JavascriptDispatch::InvokeBuiltInOperation(
    BuiltInOperation    operation,
    WORD                wFlags,
    DISPPARAMS *        pdp,
    VARIANT *           pVarRes,
    EXCEPINFO *         pei,
    IServiceProvider *  pspCaller)
{
    HRESULT hr = NOERROR;
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);

    // Initialize out parameters.
    if (pVarRes != nullptr)
    {
        VariantInit(pVarRes);
    }
    if (pei != nullptr)
    {
        memset(pei, 0, sizeof(*pei));
    }

    // Validate in parameters.
    if (pdp == nullptr || pdp->cArgs < pdp->cNamedArgs)
    {
        hr = E_INVALIDARG;
    }

    Js::ScriptContext * scriptContext = GetScriptContext();
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    Js::DynamicObject * parentObject = nullptr;
    Js::JavascriptMethod entryPoint = nullptr;

    switch (operation)
    {
#define BUILT_IN_OPERATION(o, p, f) \
    case o:\
        parentObject = library->p; \
        entryPoint = f.GetOriginalEntryPoint(); \
        break;
#include "BuiltInOperations.h"
#undef BUILT_IN_OPERATION

    default:
        AssertMsg(FALSE, "Unrecognized BuiltInOperation value");
    }

    if (!parentObject || !entryPoint)
    {
        return E_INVALIDARG;
    }

    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        if (parentObject->HasDeferredTypeHandler())
        {
            parentObject->HasProperty(Js::Constants::NoProperty);
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr)

    Js::JavascriptFunction * builtInFunction = scriptContext->GetBuiltInLibraryFunction(entryPoint);
    if (builtInFunction == nullptr)
    {
        return JSERR_NeedFunction;
    }

    Js::Arguments arguments(0, nullptr);
    if (scriptSite->IsClosed())
    {
        return JSERR_CantExecute;
    }

    Js::Var thisPointer = Js::JavascriptOperators::RootToThisObject(this->scriptObject, scriptContext);
    hr = DispatchHelper::MarshalDispParamToArgumentsNoThrowNoScript(pdp, thisPointer, scriptContext, builtInFunction, &arguments);
    if (SUCCEEDED(hr))
    {
        Js::Var varResult = nullptr;
        hr = scriptSite->Execute(builtInFunction, &arguments, pspCaller, &varResult);
        if (SUCCEEDED(hr) && pVarRes)
        {
            hr = DispatchHelper::MarshalJsVarToVariantNoThrow(varResult, pVarRes, scriptContext);
        }
    }

    if (FAILED(hr) && hr != SCRIPT_E_REPORTED && hr != SCRIPT_E_RECORDED  && hr != SCRIPT_E_PROPAGATE && pei)
    {
        pei->scode = GetScode(MapHr(hr));
        hr = DISP_E_EXCEPTION;
    }

    return hr;
}

//----------------------------------------------------------------------------------
//
// JavascriptDispatch::PrivateInvokeEx
//
// Private version of InvokeEx with more room for flag values and doesn't go through
// dispex.dll
//
//----------------------------------------------------------------------------------
HRESULT JavascriptDispatch::PrivateInvokeEx(
    DISPID              id,
    LCID                lcid,
    DWORD               dwFlags,
    DISPPARAMS *        pdp,
    VARIANT *           pvarRes,
    EXCEPINFO *         pei,
    IServiceProvider *  pspCaller)
{
    // For now we only use PrivateInvokeEx to bypass the bug in dispex.dll that drops
    // the DISPATCH_CONSTRUCT flag during marshaling so it's OK to just call InvokeEx.
    return InvokeEx(id, lcid, LOWORD(dwFlags), pdp, pvarRes, pei, pspCaller);
}

//----------------------------------------------------------------------------------
//
// JavascriptDispatch::InvokeOnMember
//
// Handles actions invoked by the caller/host on a member of our script object, identified
// by the id.
//
//----------------------------------------------------------------------------------

HRESULT JavascriptDispatch::InvokeOnMember(
    DISPID              id,
    LCID                lcid,
    WORD                wFlags,
    DISPPARAMS *        pdp,
    VARIANT *           pVarRes,
    EXCEPINFO *         pei,
    IServiceProvider *  pspCaller)
{
    HRESULT         hr = S_OK;
    IDispatchEx     *pDispEx = nullptr;
    VARIANT         *pVarValue = nullptr;
    Js::Var        varMember = nullptr;
    Js::Var        value = nullptr;

    Js::JavascriptLibrary* library = scriptObject->GetLibrary();

    // This should be checked already
    Assert(scriptObject != nullptr);

    // Examine wFlags to figure out the action we need to perform.

    if ((wFlags & DISPATCH_PROPERTYGET) && 0 != pdp->cArgs ||
        (wFlags & DISPATCH_METHOD) && !(wFlags & DISPATCH_PROPERTYGET) ||
        (wFlags & k_dispPutOrPutRef) && 1 != pdp->cArgs ||
        (wFlags & DISPATCH_CONSTRUCT))
    {
        // Call the method identified by id.

        // Use id to get the member. If it doesn't exist, error out rather than trying to call it.
        // (This matches v5.8 behavior.)

        if ((wFlags & (DISPATCH_METHOD | k_dispPutOrPutRef)) && library->GetScriptContext()->IsFastDOMEnabled())
        {
            hr = scriptSite->ExternalGetPropertyReference(scriptObject, id, &varMember, pspCaller);
        }
        else
        {
            // TODO: Determine whether this code path can be simplified. Apparently, this is a special case for host dispatch
            // and removing this would require remarshaling the disp param.
            hr = scriptSite->ExternalGetProperty(scriptObject, id, &varMember, pspCaller);
        }
        if (FAILED(hr))
        {
            return hr;
        }

        Js::Var varResult;

        // The member is one of our own script functions, so execute it directly.
        // If we have no "this" pointer in the params, provide one.
        // It is possible to have a property of the Javascript object to have
        // a property that is HostDispatch.
        if (Js::JavascriptOperators::GetTypeId(varMember) == Js::TypeIds_HostDispatch)
        {
            if (scriptSite->IsClosed())
            {
                // The session has closed already.
                return E_ACCESSDENIED;
            }

            HostDispatch* hostDispatch = (HostDispatch*)varMember;
            DISPPARAMS newDispParam;
            Js::ScriptContext* scriptContext = GetScriptContext();

            if (!DispatchHelper::DispParamsContainThis(pdp))
            {
                newDispParam.cArgs = pdp->cArgs + 1;
                newDispParam.cNamedArgs = pdp->cNamedArgs + 1;
                newDispParam.rgvarg = (VARIANT*)alloca(
                    ((newDispParam.cArgs * sizeof(VARIANT)) + (newDispParam.cNamedArgs * sizeof(DISPID))));
                newDispParam.rgdispidNamedArgs = (DISPID*)((BYTE*)newDispParam.rgvarg +
                    (newDispParam.cArgs * sizeof(VARIANT)));

                hr = DispatchHelper::MarshalJsVarToVariantNoThrow(this->scriptObject, &newDispParam.rgvarg[0], scriptContext);
                if (SUCCEEDED(hr) && pdp->cArgs != 0)
                {
                    js_memcpy_s(&newDispParam.rgvarg[1], (newDispParam.cArgs - 1) * sizeof(VARIANT), pdp->rgvarg, pdp->cArgs * sizeof(VARIANT));
                }
                newDispParam.rgdispidNamedArgs[0] = DISPID_THIS;
                if (pdp->cNamedArgs != 0)
                {
                    js_memcpy_s(&newDispParam.rgdispidNamedArgs[1], (newDispParam.cNamedArgs - 1) * sizeof(DISPID), pdp->rgdispidNamedArgs, pdp->cNamedArgs * sizeof(DISPID));
                }
                pdp = &newDispParam;
            }
            if (SUCCEEDED(hr))
            {
                BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false);
                {
                    hr = hostDispatch->InvokeMarshaled(0, wFlags, pdp, pVarRes, pei);
                }
                END_JS_RUNTIME_CALL(scriptContext);
                if (pdp == &newDispParam)
                {
                    Assert(newDispParam.rgvarg[0].vt == VT_DISPATCH);
                    VariantClear(&newDispParam.rgvarg[0]);
                }
                // The javascript dispatch could be an entry point for a call originating from another thread (cross-tab scenario)
                // Let's ensure that the caller's threadContext records the exception if the current threadContext has one recorded.
                if((hr == SCRIPT_E_PROPAGATE || hr == SCRIPT_E_RECORDED) && scriptContext->GetThreadContext()->GetRecordedException() != nullptr)
                {
                    hr = scriptSite->HandleJavascriptException(scriptContext->GetAndClearRecordedException(), scriptContext, pspCaller);
                }
           }
        }
        else
        {
            if (varMember == nullptr ||
                !Js::RecyclableObject::Is(varMember))
            {
                return JSERR_NeedFunction;
            }
            Js::RecyclableObject *obj = Js::RecyclableObject::FromVar(varMember);
            Js::Arguments arguments(0, nullptr);
            ScriptSite* targetScriptSite = nullptr;
            IfFailedReturn(GetTargetScriptSite(obj,&targetScriptSite));
            if (targetScriptSite->IsClosed())
            {
                return JSERR_CantExecute;
            }

            Js::ScriptContext* scriptContext = GetScriptContext();
            Js::ScriptContext* targetScriptContext = targetScriptSite->GetScriptSiteContext();
            Js::Var thisPointer = Js::JavascriptOperators::RootToThisObject(this->scriptObject, targetScriptContext);
            hr = DispatchHelper::MarshalDispParamToArgumentsNoThrowNoScript(pdp, thisPointer, targetScriptContext, obj, &arguments);
            if (wFlags & DISPATCH_CONSTRUCT)
            {
                arguments.Info.Flags = (Js::CallFlags)(arguments.Info.Flags | Js::CallFlags_New);
            }
            if (wFlags & k_dispPutOrPutRef)
            {
                AssertMsg(false, "This scenario is no longer supported");
            }

            if (SUCCEEDED(hr))
            {
                hr = targetScriptSite->Execute(obj, &arguments, pspCaller, &varResult);
                if (SUCCEEDED(hr) && pVarRes)
                {
                    hr = DispatchHelper::MarshalJsVarToVariantNoThrow(varResult, pVarRes, scriptContext);
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }
    }
    else if (wFlags & DISPATCH_PROPERTYGET)
    {
        // Get the value of the member.

        if (0 != pdp->cArgs || 0 != pdp->cNamedArgs)
        {
            return E_INVALIDARG;
        }

        // Use id to get the member.

        varMember = library->GetUndefined();
        hr = scriptSite->ExternalGetProperty(scriptObject, id, &varMember, pspCaller);
        if (FAILED(hr))
        {
            return hr;
        }

        if (pVarRes)
        {
            hr = DispatchHelper::MarshalJsVarToVariantNoThrow(varMember, pVarRes, GetScriptContext());
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }
    else
    {
        //
        // Put the given member value.
        //
        Assert(wFlags & k_dispPutOrPutRef);

        // Make sure that if we're doing a put, the required put param exists.
        if (!scriptObject->IsExternal() &&
            (0 == pdp->cNamedArgs || DISPID_PROPERTYPUT != pdp->rgdispidNamedArgs[0]))
        {
            return DISP_E_PARAMNOTOPTIONAL;
        }

        if (1 != pdp->cArgs ||
            (!scriptObject->IsExternal() && 1 != pdp->cNamedArgs))
        {
            return E_INVALIDARG;
        }

        // Get the default value of the IDispatch if the putref flag is not set.
        if (!(wFlags & DISPATCH_PROPERTYPUTREF) &&
            pdp->rgvarg->vt == VT_DISPATCH)
        {
            IDispatch   *pDisp = pdp->rgvarg->pdispVal;
            DISPPARAMS  dispEmpty = {0};
            VARIANT     varValue;

            // WOOB 1116700
            // in objectfallback.js, msxml can call into JavascriptDispatch InvokeEx with pdp != NULL
            // but pDisp NULL, when mshtml is passivating. We can't do much at this time anyhow.
            if (pDisp == nullptr)
            {
                return E_FAIL;
            }

            pVarValue = &varValue;
            VariantInit(pVarValue);

            hr = pDisp->QueryInterface(__uuidof(IDispatchEx), (void**)&pDispEx);
            if (SUCCEEDED(hr) && pDispEx)
            {
                if (scriptSite)
                {
                    ScriptEngine *scriptEngine = scriptSite->GetScriptEngine();
                    if (scriptEngine == nullptr)
                    {
                        return E_UNEXPECTED;
                    }
                    lcid = scriptEngine->GetInvokeVersion();
                }
                hr = pDispEx->InvokeEx(DISPID_VALUE, lcid, DISPATCH_PROPERTYGET, &dispEmpty, pVarValue, pei, pspCaller);
                pDispEx->Release();
            }
            else
            {
                hr = pDisp->Invoke(0, IID_NULL, lcid, DISPATCH_PROPERTYGET, &dispEmpty, pVarValue, pei, nullptr);
            }

            if (FAILED(hr))
            {
                VariantClear(pVarValue);

                // If we failed to obtain value from the passed in IDispatch we should still fallback and set the property
                // to the IDispatch pointer itself.
                // This is at least valid for the following VBScript construct:
                //   foo.onevent = GetRef(VBSFunction)
                // In this case property onevent will be set to IDispatch for the VBS function, and getting DISPID_VALUE will fail
                // because VBS method and only works with DISPATCH_METHOD flag. Note that providing that flag would cause method to
                // be invoked, which would be wrong.
                // So the only correct thing to do here is to just set the value.

                hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(pdp->rgvarg, &value, GetScriptContext());
            }
            else
            {
                hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(pVarValue, &value, GetScriptContext());
                VariantClear(pVarValue);
            }
        }
        else
        {
            hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(pdp->rgvarg, &value, GetScriptContext());
        }


        if (FAILED(hr))
        {
            return hr;
        }

        hr = scriptSite->ExternalSetProperty(scriptObject, id, value, pspCaller);
    }

    return hr;
}

//-----------------------------------------------------------------------------------
//
// JavascriptDispatch::InvokeOnSelf
//
// Perform the given action on the script object itself or IDispatch method. We
// only support DISPID_GET_SAFEARRAY dispid as the same in old engine.
//
//-----------------------------------------------------------------------------------
HRESULT JavascriptDispatch::InvokeOnSelf(
    DISPID              id,
    LCID                lcid,
    WORD                wFlags,
    DISPPARAMS *        pdp,
    VARIANT *           pVarRes,
    EXCEPINFO *         pei,
    IServiceProvider *  pspCaller)
{
    HRESULT         hr = S_OK;

    // Hold a reference to the global object o prevent it from getting freed if the engine
    // is shutdown during this invoke.
    Js::JavascriptLibrary * library = scriptObject->GetLibrary();

    // Verify that there is at most 1 named arg, the "this" pointer.

    if (pdp->cNamedArgs > 1 ||
        (pdp->cNamedArgs == 1 && !DispatchHelper::DispParamsContainThis(pdp)))
    {
        return DISP_E_MEMBERNOTFOUND;
    }

    if (id != 0)
    {
        return DISP_E_MEMBERNOTFOUND;
    }

    if (scriptSite->GetScriptEngine() == nullptr)
    {
        return E_UNEXPECTED;
    }
    // Default case: id == 0
    if (((pdp->cArgs == 1 && DispatchHelper::DispParamsContainThis(pdp)) ||
          (pdp->cArgs == 0 && !DispatchHelper::DispParamsContainThis(pdp))) &&
        (wFlags & DISPATCH_PROPERTYGET))
    {
        // Do property get

        Js::Var varValue = nullptr;
        Js::JavascriptHint hint = Js::JavascriptHint::None;

        hr = scriptSite->ExternalToPrimitive(this->scriptObject, hint, &varValue, pspCaller);

        if (SUCCEEDED(hr) && (pVarRes != nullptr))
        {
            hr = DispatchHelper::MarshalJsVarToVariantNoThrow(varValue, pVarRes, library->GetScriptContext());
        }
    }
    else if (wFlags & k_dispCallOrConstruct)
    {
        // Do call/construct
        Js::Var varResult = nullptr;
        Js::Var thisPointer = this->GetScriptContext()->GetLibrary()->GetNull();
        Js::Arguments arguments(0, nullptr);
        hr = DispatchHelper::MarshalDispParamToArgumentsNoThrowNoScript(pdp, thisPointer, this->GetScriptContext(), scriptObject, &arguments);
        if (wFlags & DISPATCH_CONSTRUCT)
        {
            arguments.Info.Flags = (Js::CallFlags)(arguments.Info.Flags | Js::CallFlags_New);
        }

        if (SUCCEEDED(hr))
        {
            hr = scriptSite->Execute(scriptObject, &arguments, pspCaller, &varResult);
            if (SUCCEEDED(hr) && (pVarRes != nullptr))
            {
                hr = DispatchHelper::MarshalJsVarToVariantNoThrow(varResult, pVarRes, library->GetScriptContext());
            }
        }
    }
    else
    {
        return DISP_E_MEMBERNOTFOUND;
    }

    return hr;
}

HRESULT JavascriptDispatch::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
    HRESULT hr = E_FAIL;
    Js::PropertyId propertyId;
    uint32 indexVal;
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);

    AutoActiveCallPointer autoActiveCallPointer(this);
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        BOOL isPropertyId = TRUE;
        Js::PropertyRecord const * propertyRecord = nullptr;
        hr = GetPropertyIdWithFlagWithScriptEnter(bstrName,  grfdex, &propertyId, &indexVal, &isPropertyId, &propertyRecord);
        if (SUCCEEDED(hr))
        {
            hr = Js::JavascriptExternalOperators::DeleteProperty(this->scriptObject, propertyId, this->GetScriptContext())? S_OK : S_FALSE;
        }
    }
    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

    return hr;
}

HRESULT JavascriptDispatch::DeleteMemberByDispID(DISPID id)
{
    HRESULT hr = S_FALSE;
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);
    Js::ScriptContext* scriptContext = this->GetScriptContext();

    AutoActiveCallPointer autoActiveCallPointer(this);
    if (scriptObject->IsExternal())
    {
        if (IsExpandoDispId(id))
        {
            // Subtract out the known offset to get us back into the internal propertyId range.
            id = ExpandoDispIdToPropertyId(id);
        }
        else
        {
            // Fall back to the internal implementation of IDispatch for custom external types to find known DISPIDs.
            // We need a mechanism to use the internal IDispatch implementation for DeleteMemberByDispID with known dispids.
            // A binary caller may simply DeleteMemberByDispID without first using GetDispID so the JavascriptDispatch will not find the property.
            // As a fallback, we can QueryObjectInterface for this special IID looking for the internal IDispatchEx to try the InvokeEx on.
            IDispatchEx* pDispEx;
            if (SUCCEEDED(QueryObjectInterface(IID_IDispatchExInternal, (void**)&pDispEx)))
            {
                hr = pDispEx->DeleteMemberByDispID(id);
                pDispEx->Release();
                return hr;
            }
        }
    }

    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        hr = Js::JavascriptExternalOperators::DeleteProperty(this->scriptObject, id, scriptContext)? S_OK : S_FALSE;
    }
    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
    VERIFYHRESULTBEFORERETURN(hr, scriptContext);
    return hr;
}

HRESULT JavascriptDispatch::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
    *pgrfdex = 0;
    return NOERROR;
}

HRESULT JavascriptDispatch::GetMemberName(DISPID id, BSTR *pbstr)
{
    HRESULT hr = NOERROR;
    IfNullReturnError(pbstr, E_INVALIDARG);
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);

    AutoActiveCallPointer autoActiveCallPointer(this);
    if (scriptObject->IsExternal())
    {
        if (IsExpandoDispId(id))
        {
            // Subtract out the known offset to get us back into the internal propertyId range.
            id = ExpandoDispIdToPropertyId(id);
        }
        else
        {
            // Fall back to the internal implementation of IDispatch for custom external types to find known DISPIDs.
            // We need a mechanism to use the internal IDispatch implementation for GetMemberName with known dispids.
            // A binary caller may simply GetMemberName without first using GetDispID so the JavascriptDispatch will not find the property.
            // As a fallback, we can QueryObjectInterface for this special IID looking for the internal IDispatchEx to try the InvokeEx on.
            IDispatchEx* pDispEx;
            if (SUCCEEDED(QueryObjectInterface(IID_IDispatchExInternal, (void**)&pDispEx)))
            {
                hr = pDispEx->GetMemberName(id, pbstr);
                pDispEx->Release();
                return hr;
            }
        }
    }

    Js::PropertyRecord const* propertyName = this->GetScriptContext()->GetPropertyName(id);
    if ((propertyName == nullptr) ||
        (propertyName->GetLength() == 0) ||
        (propertyName->GetBuffer() == nullptr))
    {
        return E_INVALIDARG;
    }

    *pbstr = SysAllocStringLen(propertyName->GetBuffer(), (uint)propertyName->GetLength()); // Using SysAllocStringLen for BSTR
    if (nullptr == *pbstr)
    {
        return E_OUTOFMEMORY;
    }
    return NOERROR;
}

class DispIdEnumerator
{
private:
    Js::ForInObjectEnumerator enumerator;
    Js::Var currentIndex;
public:
    DispIdEnumerator(Js::RecyclableObject * scriptObject, Js::ScriptContext * scriptContext)
        : enumerator(scriptObject, scriptContext), currentIndex(nullptr)
    {
    }
    void Clear() 
    { 
        this->currentIndex = nullptr;
        enumerator.Clear(); 
    }
    void Initialize(Js::RecyclableObject * scriptObject, Js::ScriptContext * scriptContext)
    {
        this->currentIndex = nullptr;
        enumerator.Initialize(scriptObject, scriptContext);
    }    
    bool MoveNext()
    {
        Js::PropertyId propertyId;
        currentIndex = enumerator.MoveAndGetNext(propertyId);
        return currentIndex != nullptr;
    }
    Js::ScriptContext * GetScriptContext() const { return enumerator.GetScriptContext(); }
    Var GetCurrentIndex() const { return currentIndex; }
};

Js::PropertyId JavascriptDispatch::GetEnumeratorCurrentPropertyId()
{
    if (!dispIdEnumerator)
    {
        return Js::Constants::NoProperty;
    }
    Var stringIndex = dispIdEnumerator->GetCurrentIndex();
    Js::ScriptContext * scriptContext = this->GetScriptContext();
    if (stringIndex != nullptr)
    {
        Js::JavascriptString* name = Js::JavascriptString::FromVar(stringIndex);
        Js::PropertyRecord const * propertyRecord = nullptr;
        scriptContext->GetOrAddPropertyRecord(name->GetString(), name->GetLength(), &propertyRecord);
        Js::PropertyId pid = propertyRecord->GetPropertyId();

        if (!Js::IsInternalPropertyId(pid))
        {
            return pid;
        }
    }

    return Js::Constants::NoProperty;
}

void JavascriptDispatch::CreateDispIdEnumerator()
{
    dispIdEnumerator = RecyclerNew(scriptSite->GetRecycler(), DispIdEnumerator, scriptObject, this->GetScriptContext());
}

HRESULT JavascriptDispatch::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
    HRESULT hr = E_FAIL;
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);
    *pid = 0;
#if DBG
    Js::ScriptContext* scriptContext = this->GetScriptContext();
#endif
    AutoActiveCallPointer autoActiveCallPointer(this);
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
    {
        hr = GetNextDispIDWithScriptEnter(grfdex, id, pid);
    }
    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
    VERIFYHRESULTBEFORERETURN(hr, scriptContext);

    return hr;
}

HRESULT JavascriptDispatch::GetNextDispIDWithScriptEnter(DWORD grfdex, DISPID id, DISPID *pid)
{
    HRESULT hr = NOERROR;
    Js::ScriptContext* scriptContext = this->GetScriptContext();
    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
    {
        if (dispIdEnumerator == nullptr || dispIdEnumerator->GetScriptContext() != scriptContext) CreateDispIdEnumerator();

        Js::PropertyId currentPropertyId;
        if (DISPID_STARTENUM == id)
        {
            dispIdEnumerator->Clear();
            dispIdEnumerator->Initialize(scriptObject, scriptContext);
        }
        else
        {
            DISPID propId = id;
            if (scriptObject->IsExternal())
            {
                propId = ExpandoDispIdToPropertyId(propId);
            }

            currentPropertyId = GetEnumeratorCurrentPropertyId();
            if ( Js::Constants::NoProperty == currentPropertyId || currentPropertyId != propId )
            {
                dispIdEnumerator->Clear();
                dispIdEnumerator->Initialize(scriptObject, scriptContext);

                BOOL found = false;
                while (dispIdEnumerator->MoveNext())
                {
                    currentPropertyId = GetEnumeratorCurrentPropertyId();
                    if (Js::Constants::NoProperty != currentPropertyId && currentPropertyId == propId  &&
                        ((grfdex & fdexEnumAll) || scriptObject->IsEnumerable(currentPropertyId)) )
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    hr = S_FALSE;
                }
            }
        }

        if (NOERROR == hr)
        {
            hr = S_FALSE;
            while (dispIdEnumerator->MoveNext())
            {
                currentPropertyId = GetEnumeratorCurrentPropertyId();
                if (Js::Constants::NoProperty != currentPropertyId &&
                    ((grfdex & fdexEnumAll) || scriptObject->IsEnumerable(currentPropertyId)))
                {
                    *pid = currentPropertyId;

                    if (scriptObject->IsExternal())
                    {
                        // We need to offset our DISPID return value to be in a known range for external objects.
                        // This is so that callers who go directly to InvokeEx can continue to use known DISPIDs in the host namespace
                        (*pid) = PropertyIdToExpandoDispId(*pid);
                    }
                    hr = NOERROR;
                    break;
                }
            }
        }
    }
    END_JS_RUNTIME_CALL(scriptContext);

    return hr;
}

HRESULT JavascriptDispatch::GetNameSpaceParent(IUnknown **ppunk)
{
    HRESULT hr = NOERROR;
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);
    AutoActiveCallPointer autoActiveCallPointer(this);
    IfNullReturnError(ppunk, E_INVALIDARG);
    *ppunk = nullptr;
#if DBG
    Js::ScriptContext* scriptContext = this->GetScriptContext();
#endif

    if (scriptObject->IsExternal())
    {
        // Fall back to the internal implementation of IDispatch for GetNameSpaceParent.
        IDispatchEx* pDispEx;
        if (SUCCEEDED(QueryObjectInterface(IID_IDispatchExInternal, (void**)&pDispEx)))
        {
            hr = pDispEx->GetNameSpaceParent(ppunk);
            pDispEx->Release();
        }
    }
    VERIFYHRESULTBEFORERETURN(hr, scriptContext);

    return hr;
}



HRESULT JavascriptDispatch::GetTypeId(int* typeId)
{
    HRESULT hr = E_FAIL;
    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);
    *typeId = Js::JavascriptOperators::GetTypeId(scriptObject);
    return S_OK;
}

BOOL JavascriptDispatch::HasInstanceWithScriptEnter(Var instance)
{
    BOOL result = false;

    Js::ScriptContext* scriptContext = this->GetScriptContext();
    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
    {
         result = scriptObject->HasInstance(instance, scriptContext);;
    }
    END_JS_RUNTIME_CALL(scriptContext);
    return result;
}

HRESULT JavascriptDispatch::HasInstance(VARIANT varInstance, BOOL * result, EXCEPINFO * pei, IServiceProvider * pspCaller)
{
    HRESULT hr = NOERROR;

    ThreadContextScope scope(this->scriptSite->GetThreadContext());
    hr = VerifyOnEntry(scope.IsValid());
    IfFailedReturn(hr);

    if (scriptSite->IsClosed())
    {
        return E_ACCESSDENIED;
    }

    if (pei != nullptr)
    {
        memset(pei, 0, sizeof(*pei));
    }

    Js::ScriptContext* scriptContext = GetScriptContext();

    Js::Var instance = nullptr;
    hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(&varInstance, &instance, scriptContext);
    if (SUCCEEDED(hr))
    {
        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
        {
            *result = HasInstanceWithScriptEnter(instance);
        }
        TRANSLATE_EXCEPTION_TO_HRESULT_ENTRY(const Js::JavascriptException& err)
        {
            Js::JavascriptExceptionObject * exceptionObject = err.GetAndClear();
            hr = scriptSite->HandleJavascriptException(exceptionObject, scriptContext, pspCaller);
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
    }

    if (FAILED(hr) && hr != SCRIPT_E_REPORTED && hr != SCRIPT_E_RECORDED  && hr != SCRIPT_E_PROPAGATE && pei)
    {
        pei->scode = GetScode(MapHr(hr));
        hr = DISP_E_EXCEPTION;
    }
    VERIFYHRESULTBEFORERETURN(hr, scriptContext);

    return hr;
}


void JavascriptDispatch::ResetToNULL()
{
    Assert(this->GetScriptContext()->IsFastDOMEnabled());
    if (this->GetScriptContext()->IsFastDOMEnabled())
    {
        ScriptSite::DispatchMap * dispMap = scriptSite->GetDispatchMap();
        dispMap->Remove(scriptObject);
        if (!IsInCall())
        {
            ResetContentToNULL();
        }
    }
}

HRESULT JavascriptDispatch::ResetToScriptSite(ScriptSite* newScriptSite)
{
    Assert(newScriptSite != scriptSite);
    Assert(Js::ExternalObject::Is(this->scriptObject));
    Assert(scriptObject->IsExternal());

    HRESULT hr = NOERROR;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        // Do the operation that can OOM first
        ScriptSite::DispatchMap * newDispatchMap = newScriptSite->EnsureDispatchMap();
        AssertMsg(!newDispatchMap->ContainsKey((void*)scriptObject), "Duplicate dispatch map entries for a JS object");
        newDispatchMap->Add((void*)scriptObject, this);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    if (FAILED(hr))
    {
        return hr;
    }

    ScriptSite::DispatchMap* oldDispatchMap = scriptSite->GetDispatchMap();
    Assert(oldDispatchMap);
    bool removed = oldDispatchMap->Remove(scriptObject);
    AssertMsg(removed, "Failed to remove a JS Dispatch from the map");
    if (!scriptSite->IsClosed())
    {
        RemoveEntryList(&linkList);
    }

    scriptSite->ReleaseDispatchCount();
    scriptSite->Release();
    scriptSite = newScriptSite;
    scriptSite->AddRef();
    scriptSite->AddDispatchCount();

    AssertMsg(!this->GetScriptContext()->IsClosed() && !scriptSite->IsClosed(), "the new site shouldn't be closed");
    newScriptSite->AddToJavascriptDispatchList(&linkList);


    return hr;
}

void JavascriptDispatch::SetAsGCTracked()
{
    if (!isGCTracked)
    {
        isGCTracked = true;
    }
}

BOOL JavascriptDispatch::VerifyCallingThread()
{
    if (scriptSite->GetThreadId() != GetCurrentThreadContextId())
    {
        return FALSE;
    }
    return TRUE;
}

HRESULT JavascriptDispatch::GetTargetScriptSite(__in Js::RecyclableObject* obj, __out ScriptSite** targetScriptSite)
{
    IfNullReturnError(targetScriptSite, E_INVALIDARG);
    *targetScriptSite = nullptr;

    Js::ScriptContext* targetScriptContext = obj->GetScriptContext();
    if (targetScriptContext == this->GetScriptContext())
    {
        *targetScriptSite = scriptSite;
        return NOERROR;
    }

    *targetScriptSite = ScriptSite::FromScriptContext(targetScriptContext);
    Assert(*targetScriptSite != nullptr);
    return NOERROR;
}

HRESULT JavascriptDispatch::QueryObjectInterface(REFIID iid, void** obj)
{
    HRESULT hr = NOERROR;
    hr = scriptObject->QueryObjectInterface(iid, obj);
    return hr;
}

AutoActiveCallPointer::~AutoActiveCallPointer()
{
    if (!wasInCall)
    {
        Assert(javascriptDispatch->IsInCall());
        javascriptDispatch->ResetIsInCall();

        // we don't need to zero out if the JavascriptDispatch is already released during the call.
        if (javascriptDispatch->scriptSite != nullptr && javascriptDispatch->scriptSite->IsClosed())
        {
            if ((Js::JavascriptFunction::Is(javascriptDispatch->scriptObject) || Js::ExternalObject::Is(javascriptDispatch->scriptObject)))
            {
                // if the scriptsite is closed during the call, we don't zero out
                // the nested object right away; instead we'll zero it out at this time.
                ScriptSite::DispatchMap * dispMap = javascriptDispatch->scriptSite->GetDispatchMap();
                dispMap->Remove(javascriptDispatch->scriptObject);
                javascriptDispatch->ResetContentToNULL();
            }
        }
    }
}

HRESULT JavascriptDispatch::ResetContentToNULL()
{
    // this is called when a site is closed. we should clean up the cache when the JavascriptDispatch is reset.
    Js::CustomExternalObject * customExternalScriptObject = Js::JavascriptOperators::TryFromVar<Js::CustomExternalObject>(scriptObject);
    if (customExternalScriptObject)
    {
        customExternalScriptObject->CacheJavascriptDispatch(nullptr);
    }
    scriptObject = nullptr;
    dispIdEnumerator = nullptr;
    return NOERROR;
}

HRESULT JavascriptDispatch::VerifyOnEntry(bool isValidThreadScope)
{
    // Called on wrong thread
    if (!isValidThreadScope)
    {
        return E_ACCESSDENIED;
    }

    if (!VerifyCallingThread())
    {
        return E_UNEXPECTED;
    }

    // In a case like user code appendChild() an iframe with onload handler, and removeChild before the document is
    // finished loading, the onload handler of the frame is not called, but jscript holds on to the only reference
    // to the iframe that keep it alive. In one last GC, the HostDispatch/HostVariant holding on to the iframe, as
    // well as the the JavascriptDispatch, are collectible. The JavascriptDispatch::Finalize is called, but not
    // Disposed/swept yet, but the function pointer held by the JavascriptDispatch is already swept. If between
    // the finalize and dispose phrase in GC, the document finished loading and the onload handler for the doomed
    // frame is called, we will av accessing the zeroed out javascriptfunction.
    // The solution is to treat finalized JavascriptDispatch as Disposed already. I'm not aware of similar problem
    // for other disposable objects.
    if (scriptObject == nullptr || isFinalized)
    {
        AssertMsg(isFinalized || scriptSite->IsClosed(), "ScriptContext not closed but script object is NULL");
        return E_ACCESSDENIED;
    }

    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();

    if (!threadContext->IsStackAvailableNoThrow())
    {
        return HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW);
    }
    if (threadContext->GetRecycler()->IsHeapEnumInProgress())
    {
        AssertMsg(false, "shouldn't call JavascriptDispatch while in heap enumeration");
        return E_UNEXPECTED;
    }
    return NOERROR;
}

void JavascriptDispatch::CachePropertyId(Js::PropertyRecord const * propertyRecord, BOOL isPropertyId)
{
    ThreadContext* threadContext = GetScriptContext()->GetThreadContext();
    Assert(propertyRecord->GetPropertyId() != Js::Constants::NoProperty);
    if (!dispIdPropertyStringMap)
    {
        this->dispIdPropertyStringMap = RecyclerNew(threadContext->GetRecycler(), Int32InternalStringMap, threadContext->GetRecycler(), 32);
    }

    this->dispIdPropertyStringMap->Item(propertyRecord->GetPropertyId(), propertyRecord);

    // Trident can GetDispID first, then repeat create JavascriptDispatch/call/release sequence.
    // this cause problem for nested element's fields. So for propertyIds that are created from 
    // CustomExternalObject, we'll keep them alive for the lifetime of CEO. type handler might not
    // hold reference to those property so we need to use cached javascriptdispatch
    Js::CustomExternalObject * customExternalScriptObject = Js::JavascriptOperators::TryFromVar<Js::CustomExternalObject>(scriptObject);
    if (customExternalScriptObject && isPropertyId)
    {
        customExternalScriptObject->CacheJavascriptDispatch(this);
    }


}


#ifdef TRACK_JS_DISPATCH
JavascriptDispatch::TrackNode* JavascriptDispatch::allocatedList = nullptr;
CriticalSection JavascriptDispatch::s_cs;

void
JavascriptDispatch::LogAlloc()
{
    bool doTrack = false;
#ifdef TRACK_DISPATCH
    doTrack = doTrack || Js::Configuration::Global.flags.TrackDispatch;
#endif
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
    doTrack = doTrack || Js::Configuration::Global.flags.LeakStackTrace;
#endif
    if (doTrack)
    {
        TrackNode * node = NoCheckHeapNewStruct(TrackNode);
        node->javascriptDispatch = this;        
        node->stackBackTrace = StackBackTrace::Capture(&NoCheckHeapAllocator::Instance, StackToSkip, StackTraceDepth);
        node->hadShutdown = false;
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
        node->refCountStackBackTraces = nullptr;
#endif
        AutoCriticalSection autocs(&s_cs);
        node->next = allocatedList;
        node->prev = nullptr;
        if (allocatedList != nullptr)
        {
            allocatedList->prev = node;
        }
        allocatedList = node;
        trackNode = node;
    }
    else
    {
        trackNode = nullptr;
    }
}

void
JavascriptDispatch::LogFree(bool isShutdown)
{
    if (trackNode != nullptr)
    {
        if (isShutdown)
        {
            trackNode->hadShutdown = true;
            return;
        }

        AutoCriticalSection autocs(&s_cs);
        if (trackNode->prev == nullptr)
        {
            Assert(trackNode == allocatedList);
            allocatedList = trackNode->next;
            if (allocatedList != nullptr)
            {
                allocatedList->prev = nullptr;
            }
        }
        else
        {
            trackNode->prev->next = trackNode->next;
            if (trackNode->next != nullptr)
            {
                trackNode->next->prev = trackNode->prev;
            }
        }

        trackNode->stackBackTrace->Delete(&NoCheckHeapAllocator::Instance);
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
        StackBackTraceNode::DeleteAll(&NoCheckHeapAllocator::Instance, trackNode->refCountStackBackTraces);
#endif
        NoCheckHeapDelete(trackNode);
        trackNode = nullptr;
    }
}
#endif
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
void 
JavascriptDispatch::PrintJavascriptRefCountStackTraces()
{
    AutoCriticalSection autocs(&s_cs);
    TrackNode * curr = allocatedList;
    while (curr != nullptr)
    {
        Output::Print(_u("%p\n"), curr->javascriptDispatch);
        Output::Print(_u(" Allocation Stack Trace:\n"));
        curr->stackBackTrace->Print();
        Output::Print(_u(" Ref count Stack Trace:\n"));
        StackBackTraceNode::PrintAll(curr->refCountStackBackTraces);
        curr = curr->next;
    }
}

class JavascriptDispatchLeakOutput
{
public:
    ~JavascriptDispatchLeakOutput();
};
JavascriptDispatchLeakOutput leakoutput;

JavascriptDispatchLeakOutput::~JavascriptDispatchLeakOutput()
{
    AutoCriticalSection autocs(&JavascriptDispatch::s_cs);
    if (!Js::Configuration::Global.flags.LeakStackTrace || JavascriptDispatch::allocatedList == nullptr)
    {
        return;
    }
#ifdef CHECK_MEMORY_LEAK
    if (Js::Configuration::Global.flags.CheckMemoryLeak)
    {
        Output::Print(_u("-------------------------------------------------------------------------------------\n"));
        Output::Print(_u("Leaked JavascriptDispatch"));
        Output::Print(_u("-------------------------------------------------------------------------------------\n"));
        JavascriptDispatch::PrintJavascriptRefCountStackTraces();
    }
#endif
#ifdef LEAK_REPORT
    if (Js::Configuration::Global.flags.IsEnabled(Js::LeakReportFlag))
    {
        LeakReport::StartSection(_u("Leaked JavascriptDispatch"));
        LeakReport::StartRedirectOutput();
        JavascriptDispatch::PrintJavascriptRefCountStackTraces();
        LeakReport::EndRedirectOutput();
        LeakReport::EndSection();
    }
#endif
}

#endif
