//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"
#pragma hdrstop

HRESULT ScriptEngine::AddEventSinks(ScriptEngine *pos, IDispatch *pdisp)
{
    ulong iti;
    ulong cti;
    EventSink *psink;
    HRESULT hr;
    IProvideClassInfo *pci = NULL;

    ITypeInfo *ptiCo = NULL;

    IProvideMultipleClassInfo *pmci = NULL;

    // First QI for IProvideMultipleClassInfo otherwise use ProvideClassInfo.
    if (SUCCEEDED(hr = pdisp->QueryInterface(_uuidof(IProvideMultipleClassInfo),
        (void **)&pmci)))
    {
        AssertMem(pmci);
        if (FAILED(hr = pmci->GetMultiTypeInfoCount(&cti)))
            goto LFail;
        if (0 == cti)
        {
            hr = HR(E_NOINTERFACE);
            goto LFail;
        }
    }
    else
    {
        if (NULL == ptiCo)
        {
            if (FAILED(hr = pdisp->QueryInterface(__uuidof(IProvideClassInfo), (void **)&pci)))
                goto LFail;
            AssertMem(pci);
            if (FAILED(hr = pci->GetClassInfo(&ptiCo)))
                goto LFail;
        }
        AssertMem(ptiCo);
        cti = 1;
    }

    // for each type info create an event sink object and add it to the list
    for (iti = 0; iti < cti; iti++)
    {
        // if there is a multiple class info interface then get the next
        // coclass type info
        if (NULL != pmci)
        {
            DWORD flags;

            // release the previous one
            if (NULL != ptiCo)
            {
                ptiCo->Release();
                ptiCo = NULL;
            }
            hr = pmci->GetInfoOfIndex(iti, MULTICLASSINFO_GETTYPEINFO,
                &ptiCo, &flags, NULL, NULL, NULL);
            if (FAILED(hr))
                continue;
        }

        Assert(ptiCo != NULL);
        // Create the event sink.
        hr = EventSink::Create(&psink, pos, pdisp, ptiCo);
        if (FAILED(hr))
            continue;

        // add it to the list
        eventSinks->Add(psink);
    }

LFail:
    // clean up
    if (NULL != pmci)
        pmci->Release();
    if (NULL != pci)
        pci->Release();
    if (NULL != ptiCo)
        ptiCo->Release();

    return hr;
}


/***************************************************************************
EventSink implementation.
***************************************************************************/
EventSink::EventSink()
{
    DLLAddRef();
    m_refCount = 1;
    m_pos = NULL;

    eventInfoes = NULL;
    m_iid = IID_IUnknown;
    m_dwCookie = 0;
    m_pdisp = NULL;
    m_pconn = NULL;
}


EventSink::~EventSink()
{
    ScriptSite* scriptSite = m_pos->GetScriptSiteHolder();
    // Unlink from the linked list.
    m_pos->RemoveEventSinks(this);

    // Unadvise. Should this ever happen?
    if (NULL != m_pconn)
        Disconnect();

    if (m_pdisp != NULL)
        m_pdisp->Release();

    for (int i = 0; i < eventInfoes->Count(); i++)
    {
        EventInfo* eventInfo = eventInfoes->Item(i);
        if (eventInfo->bstrName != NULL)
        {
            SysFreeString(eventInfo->bstrName);
        }
        if (eventInfo->scriptObject != NULL)
        {
            eventInfo->scriptObject.Unroot(scriptSite->GetRecycler());
        }
    }
    eventInfoes->Clear();

    DLLRelease();
}


HRESULT EventSink::Create(EventSink **ppsink, ScriptEngine *pos,
                           IDispatch* pdisp, ITypeInfo *ptiCoClass)
{
    AssertMem(ppsink);
    AssertMem(pos);
    AssertMem(pdisp);
    AssertMem(ptiCoClass);

    HRESULT hr;
    EventSink *psink;

    if (NULL == ptiCoClass)
    {
        return E_FAIL;
    }

    if (NULL == (psink = new EventSink()))
        return HR(E_OUTOFMEMORY);
    hr = psink->Init(pos, pdisp, ptiCoClass);
    if (FAILED(hr))
    {
        psink->Release();
        return hr;
    }
    *ppsink = psink;
    return NOERROR;
}


HRESULT EventSink::Init(ScriptEngine *pos, IDispatch* pdisp, ITypeInfo *ptiCo)
{
    AssertMem(pos);
    AssertMem(pdisp);
    AssertMem(ptiCo);

    HRESULT hr;

    // Set the script reference.
    m_pos = pos;

    // Save pdisp.
    m_pdisp = pdisp;
    m_pdisp->AddRef();
    eventInfoes = EventInfoList::New(pos->GetScriptAllocator());        
    // Allocate the event set.
    hr = CreateEventSet(ptiCo);

    return hr;
}


HRESULT EventSink::CreateEventSet(ITypeInfo *ptiCo)
{
    AssertMem(ptiCo);

    int ievt;
    uint cbstr;
    HRESULT hr;
    FUNCDESC *pfdesc = NULL;
    TYPEATTR *ptattr = NULL;
    ITypeInfo *ptiSrc = NULL;

    ptiCo->AddRef();

    // Get the source type info.
    if (FAILED(hr = GetSourceTypeInfo(ptiCo, &ptiSrc)))
        goto Error;
    if (FAILED(hr = ptiSrc->GetTypeAttr(&ptattr)))
        goto Error;

    if (ptattr->typekind != TKIND_DISPATCH)
    {
        hr = HR(E_NOINTERFACE);
        goto Error;
    }

    // Squirrel away the IID for later use by GetConnectionPoint.
    m_iid = ptattr->guid;

    // Capture the total events.
    int funcCount = ptattr->cFuncs;
    if (funcCount == 0)
    {
        hr = HR(E_NOINTERFACE);
        goto Error;
    }

    // Populate the event table.
    for (ievt = 0; ievt < funcCount; ++ievt)
    {
        EventInfo* eventInfo = AnewStruct(m_pos->GetScriptAllocator(), EventInfo);
        if (NULL == eventInfo)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
        memset(eventInfo, 0, sizeof(EventInfo));

        if (FAILED(hr = ptiSrc->GetFuncDesc(ievt, &pfdesc)))
            goto Error;
        eventInfo->dispID = pfdesc->memid;

        if (FAILED(hr = ptiSrc->GetNames(eventInfo->dispID, &eventInfo->bstrName, 1, &cbstr)))
            goto Error;
        ptiSrc->ReleaseFuncDesc(pfdesc);
        eventInfoes->Add(eventInfo);
        pfdesc = NULL;
    }
    hr = NOERROR;
    Sort();

Error:
    if (ptiSrc)
    {
        if (pfdesc)
            ptiSrc->ReleaseFuncDesc(pfdesc);
        if (ptattr)
            ptiSrc->ReleaseTypeAttr(ptattr);
        ptiSrc->Release();
    }
    if (ptiCo != NULL)
        ptiCo->Release();

    return hr;
}


HRESULT EventSink::FindEvent(__in DISPID id, __deref_out EventInfo **outEventInfo)
{
    IfNullReturnError(outEventInfo, E_POINTER);
    *outEventInfo = nullptr;

    // TODO (yongqu): do binary search if perf is an issue here. need to implement sort first.
    for (int i = 0; i < eventInfoes->Count(); i++)
    {
        if (eventInfoes->Item(i)->dispID == id)
        {
            *outEventInfo = eventInfoes->Item(i);
            return NOERROR;
        }
    }

    return DISP_E_MEMBERNOTFOUND;
}


void EventSink::Sort(void)
{
    // TODO (yongqu): implement if we have perf issue here. 
    //eventInfoes->Sort();
}


/***************************************************************************
Connect the event sink to the IConnectionPoint of the source.
***************************************************************************/
HRESULT EventSink::Connect(void)
{
    HRESULT hr;
    IConnectionPointContainer *pcpc = NULL;

    // check to see if we have already advised
    if (NULL != m_pconn)
        return NOERROR;

    // Attach ourself to the control.
    hr = m_pdisp->QueryInterface(__uuidof(IConnectionPointContainer), (void **)&pcpc);
    if (FAILED(hr))
    {
        DebugPrintf((_u("QI(IConnectionPointContainer) Failed\n")));
        goto Error;
    }

    hr = pcpc->FindConnectionPoint(m_iid, &m_pconn);
    if (FAILED(hr))
    {
        DebugPrintf((_u("FindConnectionPoint Failed\n")));
        m_pconn = NULL;
        goto Error;
    }

    hr = m_pconn->Advise((IUnknown*)this, &m_dwCookie);
    if (FAILED(hr))
    {
        m_pconn->Release();
        m_pconn = NULL;
        DebugPrintf((_u("Advise() Failed\n")));
        goto Error;
    }

    hr = NOERROR;

Error:
    // Release the connection point container.
    if (pcpc != NULL)
        pcpc->Release();

    return hr;
}


/***************************************************************************
Disconnect the event sink from the connection point.
***************************************************************************/
HRESULT EventSink::Disconnect(void)
{
    HRESULT hr;

    // If already disconnected (or never connected) return OK.
    if (m_pconn == NULL)
        return NOERROR;

    hr = m_pconn->Unadvise(m_dwCookie);
    if (FAILED(hr))
    {
        DebugPrintf((_u("Unadvise() Failed\n")));
        goto Error;
    }

    hr = NOERROR;

Error:
    // Release the connection point and null the pointer.
    m_pconn->Release();
    m_pconn = NULL;

    return hr;
}


/***************************************************************************
IUnknown methods.
***************************************************************************/
STDMETHODIMP EventSink::QueryInterface(REFIID riid, void **ppv)
{
    CHECK_POINTER(ppv);
    if (riid == IID_IUnknown ||
        riid == __uuidof(IDispatch) ||
        riid == m_iid)
    {
        *ppv = this;
        AddRef();
        return NOERROR;
    }
    *ppv = NULL;
    return HR(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) EventSink::AddRef(void)
{
    return InterlockedIncrement(&m_refCount);
}


STDMETHODIMP_(ULONG) EventSink::Release(void)
{

    long currentCount = InterlockedDecrement(&m_refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}


/***************************************************************************
IDispatch methods.
***************************************************************************/
STDMETHODIMP EventSink::GetTypeInfoCount(uint *pcti)
{
    *pcti = 0;
    return NOERROR;
}


STDMETHODIMP EventSink::GetTypeInfo(uint iti, LCID lcid, ITypeInfo **ppti)
{
    *ppti = NULL;
    return HR(DISP_E_BADINDEX);
}


STDMETHODIMP EventSink::GetIDsOfNames(REFIID riid, __in_ecount(cpsz) OLECHAR **prgpszNames,
                                       uint cpsz, LCID lcid, DISPID *prgdispid)
{
    AssertMsg(FALSE, "Unexpected call to EventSink::GetIDsOfNames");
    return HR(E_NOTIMPL);
}


STDMETHODIMP EventSink::Invoke(DISPID id, REFIID riid, LCID lcid,
                                ushort wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei,
                                uint *puArgErr)
{
    HRESULT hr;
    EventInfo *eventInfo;
    ScriptSite *scriptSite;
    ulong grfscr;

    grfscr = fscrImplicitThis | fscrImplicitParents;

    if (NULL != pvarRes)
        VariantInit(pvarRes);

    if (NULL == m_pos)
        return HR(E_UNEXPECTED);

    // If we are pseudo-disconnected, ignore the event.
    if (m_pos->IsPseudoDisconnected())
        return NOERROR;

    if (FAILED(hr = FindEvent(id, &eventInfo)))
        return hr;

    if (NULL == eventInfo->scriptObject)
        return HR(DISP_E_MEMBERNOTFOUND);

    scriptSite = m_pos->GetScriptSiteHolder();
    if (NULL == scriptSite)
        return HR(E_FAIL);

    // We we're invoked using IDispatch::Invoke - we don't know what the
    // caller is.
    AutoCallerPointer callerPointer(scriptSite, NULL);
    Js::Arguments arguments(0, NULL);
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();

    // Marshal the "this" pointer here.
    Js::Var thisVar = nullptr;
    VARIANT thisVariant;
    thisVariant.vt = VT_DISPATCH;
    thisVariant.pdispVal = this->GetDispatch();
    hr = DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(&thisVariant, &thisVar, scriptContext);
    if (SUCCEEDED(hr))
    {
        hr = DispatchHelper::MarshalDispParamToArgumentsNoThrowNoScript(pdp, thisVar, scriptContext, eventInfo->scriptObject, &arguments);

        if (SUCCEEDED(hr))
        {
            // firing event; doesn't need to return value.
            hr = scriptSite->Execute(eventInfo->scriptObject, &arguments, NULL, NULL);
        }    
    }

    return hr;
}


BOOL EventSink::RegisterEventHandler(__in LPCOLESTR eventName, __in Js::DynamicObject *scriptObject)
{
    ScriptSite* scriptSite = m_pos->GetScriptSiteHolder();
    Recycler * recycler = scriptSite->GetRecycler();
    for (int i = 0; i < eventInfoes->Count(); i++)
    {
        EventInfo* eventInfo = eventInfoes->Item(i);
        if (0 == ostricmp(eventInfo->bstrName, eventName))
        {
            if (eventInfo->scriptObject != NULL)
            {
                eventInfo->scriptObject.Unroot(recycler);                
            }
            eventInfo->scriptObject.Root(scriptObject, recycler);
            return TRUE;
        }
    }
    return FALSE;
}



//---------------------------------------------------------------------
// Return the typeinfo marked [source, default] that is contained
// by the given CoClass typeinfo.
//---------------------------------------------------------------------
HRESULT EventSink::GetSourceTypeInfo(ITypeInfo *ptiCo, ITypeInfo **pptinfoSource)
{
    int flags;
    HRESULT hr;
    TYPEATTR *ptattr;
    HREFTYPE hreftype;
    UINT iImplType, cImplTypes;

    *pptinfoSource = NULL;
    // Look it up.

    // ...make sure the typeinfo is a CoClass (consisting of a set of interfaces)
    if (FAILED(hr = ptiCo->GetTypeAttr(&ptattr)))
        return hr;
    AssertMsg(ptattr->typekind == TKIND_COCLASS, "Unexpected TypeInfo kind");

    cImplTypes = ptattr->cImplTypes;

    ptiCo->ReleaseTypeAttr(ptattr);

    // ...search through the set of interfaces in the CoClass
    for (iImplType = 0; iImplType < cImplTypes ; iImplType++)
    {
        if (FAILED(hr = ptiCo->GetImplTypeFlags(iImplType, &flags)))
            return hr;

        // Look for the [source, default] interface on the coclass
        // that isn't marked as restricted.
        if ((flags &
            (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE | IMPLTYPEFLAG_FRESTRICTED))
            == (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE))
        {
            // Get the handle of the implemented interface.
            if (FAILED(hr = ptiCo->GetRefTypeOfImplType(iImplType, &hreftype)))
                return hr;

            // Get the typeinfo implementing the interface.
            return ptiCo->GetRefTypeInfo(hreftype, pptinfoSource);
        }
    }

    return HR(E_FAIL);
}

bool EventSink::EventInfoComparer::Equals(EventSink::EventInfo *src, EventSink::EventInfo *dst)
{
    return (src->dispID == dst->dispID);
}

int EventSink::EventInfoComparer::Compare(EventSink::EventInfo *src, EventSink::EventInfo *dst)
{
    if (src->dispID < dst->dispID)
    {
        return -1;
    }
    if (src->dispID == dst->dispID)
    {
        return 0;
    }
    return 1;
}
