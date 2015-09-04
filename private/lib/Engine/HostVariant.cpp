/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"

HostVariant::HostVariant(IDispatch *pdisp, Js::ScriptContext* scriptContext) :
    isUnknown(FALSE),
    isTracked(FALSE)
{
    memset(&varDispatch, 0, sizeof(VARIANT));
    varDispatch.vt = VT_DISPATCH;
    supportIDispatchEx = FALSE;

    if (pdisp)
    {
        HRESULT hr;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = pdisp->QueryInterface(__uuidof(IDispatchEx), (void**)&this->varDispatch.pdispVal);
        }
        END_LEAVE_SCRIPT(scriptContext);
        
        if (hr == S_OK && this->varDispatch.pdispVal)
        {
            supportIDispatchEx = TRUE;
        }
        else
        {
            this->varDispatch.pdispVal = pdisp;
            pdisp->AddRef();
        }
    }
    else
    {
        this->varDispatch.pdispVal = NULL;
    }
#ifdef TRACK_DISPATCH
    LogAlloc();
#endif
}

HostVariant::HostVariant(IDispatch *pdisp) :
    isUnknown(FALSE),
    isTracked(FALSE)
{
    memset(&varDispatch, 0, sizeof(VARIANT));
    varDispatch.vt = VT_DISPATCH;
    supportIDispatchEx = FALSE;

    if (pdisp)
    {
        HRESULT hr;
        hr = pdisp->QueryInterface(__uuidof(IDispatchEx), (void**)&this->varDispatch.pdispVal);
        
        if (hr == S_OK && this->varDispatch.pdispVal)
        {
            supportIDispatchEx = TRUE;
        }
        else
        {
            this->varDispatch.pdispVal = pdisp;
            pdisp->AddRef();
        }
    }
    else
    {
        this->varDispatch.pdispVal = NULL;
    }
#ifdef TRACK_DISPATCH
    LogAlloc();
#endif
}


HostVariant::HostVariant(IDispatchEx *pdispex) :
    isUnknown(FALSE),
    supportIDispatchEx(TRUE),
    isTracked(FALSE)
{
    memset(&varDispatch, 0, sizeof(VARIANT));

    pdispex->AddRef();
    this->varDispatch.vt = VT_DISPATCH;
    this->varDispatch.pdispVal = pdispex;
#ifdef TRACK_DISPATCH
    LogAlloc();
#endif
}

HostVariant::HostVariant(HostVariant *origHostVariant, Js::ScriptContext* scriptContext) :
    isUnknown(origHostVariant->isUnknown),
    supportIDispatchEx(origHostVariant->supportIDispatchEx)
{
    memset(&varDispatch, 0, sizeof(VARIANT));

    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        // the original one came from VariantCopy; second one shouldn't fail
        AssertMsg(!isTracked, "shouldn't clone tracked hostvariant");
        // This is not really needed as recycler always zeroed out the memory. Just defend in depth.
        VariantInit(&varDispatch);
        HRESULT hr =  VariantCopy(&varDispatch, &origHostVariant->varDispatch);
        AssertMsg(SUCCEEDED(hr), "variantcopy second time failed??");
    }
    END_LEAVE_SCRIPT(scriptContext);
}

HostVariant::HostVariant(ITracker *tracker, Js::ScriptContext* scriptContext) :
    isUnknown(FALSE),
    supportIDispatchEx(TRUE),
    isTracked(TRUE)    
{
    memset(&varDispatch, 0, sizeof(VARIANT));

    tracker->AddRef();
    tracker->SetTrackingAlias(&varDispatch);
    this->varDispatch.vt = VT_DISPATCH;
    this->varDispatch.pdispVal = tracker;

#ifdef TRACK_DISPATCH
    LogAlloc();
#endif
}

// We just need to hold the reference, and do Equality checs.
HostVariant::HostVariant() :
    isUnknown(TRUE),
    isTracked(FALSE)
{
    VariantInit(&varDispatch);
}

HRESULT HostVariant::Initialize(VARIANT * variant)
{    
    Assert(!(variant->vt & VT_BYREF));
#if DBG
    ITracker* tracker = NULL;
    if (variant->vt == VT_UNKNOWN)
    {
        Assert(FAILED(variant->punkVal->QueryInterface(IID_ITrackerJS9, (void**)&tracker)));
        if (tracker != NULL)
        {
            tracker->Release();
        }
    }
#endif     

    HRESULT hr = VariantCopy(&varDispatch, variant);
    
#ifdef TRACK_DISPATCH
    LogAlloc();
#endif

    return hr;
}

// Delay loaded IDispatch. This can happen when a named item is added but GetItemInfo returns NULL
// during initialization. We'll fill in the real IDispatch* when used.
// Reuse the VARAIANT to avoid additional memory usage.
// itemName is held in NamedItemList; no additional copying is needed.
HostVariant::HostVariant(LPCOLESTR itemName) :
    isUnknown(FALSE),
    isTracked(FALSE)
{
    memset(&varDispatch, 0, sizeof(VARIANT));
    varDispatch.vt = VT_LPWSTR;
    varDispatch.bstrVal = (BSTR)itemName;
#ifdef TRACK_DISPATCH
    LogAlloc();
#endif
}

void HostVariant::Finalize(bool isShutdown)
{
    if (!isShutdown && isTracked)
    {
        ITracker * tracker = (ITracker*) this->varDispatch.pdispVal;
        tracker->SetTrackingAlias(NULL);
        isTracked = FALSE;
    }
}

void HostVariant::Dispose(bool isShutdown)
{
    // If it was tracked before, Finalize should have been call 
    // and unregister the tracking alias    

    if (!isShutdown)
    {
        Assert(!isTracked);
        IDispatch * dispatch = FinalizeInternal();
        if (dispatch != null)
        {
            FinalizeDispatch(dispatch);
        }
    }
    else
    {
        LEAK_REPORT_PRINT(L"HostVariant %p: Finalize not called on shutdown (%s %p)\n", this,
            isUnknown? L"IUnknown" : this->varDispatch.vt == VT_LPWSTR? L"LPWSTR" : L"IDispatch",
            isUnknown? varDispatch.punkVal : this->varDispatch.vt == VT_LPWSTR? 0 :
                this->varDispatch.pdispVal
            );
    }
}

void HostVariant::FinalizeDispatch(IDispatch * dispatch)
{
    // dispose should only be executed outside of the script
    AssertNotInScript();

    ULONG count = dispatch->Release();
    (count);    
}

IDispatch * HostVariant::FinalizeInternal()
{  
    IDispatch* dispatch = null;
    if (isUnknown)
    {
        VariantClear(&varDispatch);
        return null;
    }

    if (this->varDispatch.vt == VT_LPWSTR)
    {
        this->varDispatch.bstrVal = NULL;
#ifdef TRACK_DISPATCH
        LogFree();
#endif
        return null;
    }

    if (this->varDispatch.pdispVal == NULL)
    {
        return null;
    }

    // If we are still tracked here, then the finalize is call from HostDispatch.
    // We can't release a tracked object yet, as the host variant might still 
    // be referenced by another HostDispatch.
    if (isTracked)
    {
        return null;
    }
   
    // We can come in here recursively from previous release call of the 
    // same object. Clear up the variant to prevent double release.
    dispatch = this->varDispatch.pdispVal;
    this->varDispatch.pdispVal = NULL;
        
    
#ifdef TRACK_DISPATCH
    LogFree();
#endif
#if DBG
    this->varDispatch.vt = VT_EMPTY;
#endif

    return dispatch;
}

void HostVariant::MarkTrackedObjects(Recycler * recycler)
{
    ITracker* tracker;
    if (isTracked)
    {
        tracker = (ITracker*)varDispatch.punkVal;        
        tracker->EnumerateTrackedObjects(recycler);
    }
}

void HostVariant::SetupTracker(ITracker* inTracker)
{
    inTracker->AddRef();
    if (varDispatch.punkVal)
    {
        varDispatch.punkVal->Release();
    }
    varDispatch.punkVal = inTracker;
    inTracker->SetTrackingAlias(&varDispatch);

    isTracked = TRUE;
}


#ifdef TRACK_DISPATCH
CriticalSection HostVariant::s_cs;
HostVariant::LinkList* HostVariant::allocatedLinkList = NULL;
int HostVariant::activeHostVariantCount = 0;

void
HostVariant::LogAlloc()
{
    if (Js::Configuration::Global.flags.TrackDispatch)
    {
        AutoCriticalSection autocs(&s_cs);
        HostVariant::activeHostVariantCount++;
        LinkList* currentItem = NoCheckHeapNewStruct(LinkList);
        currentItem->object = (void*)this;
        currentItem->next = allocatedLinkList;
        allocatedLinkList = currentItem;
        stackBackTrace = StackBackTrace::Capture(&NoCheckHeapAllocator::Instance, StackToSkip, StackTraceDepth);
    }
}

void
HostVariant::LogFree()
{
    if (Js::Configuration::Global.flags.TrackDispatch)
    {
        LinkList* current = allocatedLinkList;
        if (current != NULL && current->object == this)
        {
            allocatedLinkList = allocatedLinkList->next;
            delete current;
        }
        else
        {
            while (current != NULL)
            {
                if (current->next != NULL && current->next->object == this)
                {
                    LinkList* temp = current->next;
                    current->next = current->next->next;
                    delete temp;
                    break;
                }
                current = current->next;
            } ;
        }
        this->stackBackTrace->Delete(&NoCheckHeapAllocator::Instance);
        stackBackTrace = NULL;
        HostVariant::activeHostVariantCount--;
    }
}
#endif