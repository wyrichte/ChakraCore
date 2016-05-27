/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"
#pragma hdrstop

HRESULT ScriptEngine::RegisterNamedEventHandler(
    __in NamedItem *pnid,
    __in_opt LPCOLESTR pszSubItem, 
    __in LPCOLESTR pszEvt, 
    DWORD dwFlags, 
    __in CScriptBody *pbody)
{

    NamedEventHandler *eventHandler;
    HRESULT hr;

    if (FAILED(hr = NamedEventHandler::Create(&eventHandler, this, pnid, pszSubItem,
        pszEvt, pbody, 0 != (dwFlags & SCRIPTTEXT_ISPERSISTENT))))
    {
        return hr;
    }

    eventHandlers->Add(eventHandler);

    if (SCRIPTSTATE_CONNECTED == m_ssState)
    {
        // We're already connected, so make sure we cover this event.
        SinkEventsOfNamedItems(eventHandlers->Count() - 1);
        hr = ConnectEventHandlers();
    }

    return hr;
}


// If we are not in running state yet, we shouldn't connect the event sink
// but once in running state, we should connect the sink right away.
// SinkEvents will make sure all events are connected.
HRESULT ScriptEngine::SinkEventsOfNamedItems(long eventHandlerID)
{
    Assert(eventHandlerID >= 0);
    IDispatch* dispatch;

    if (eventHandlers == nullptr)
    {
        return S_OK;
    }

    if (eventHandlers == nullptr || eventHandlers->Count() <= eventHandlerID)
    {
        return NOERROR;
    }

    HRESULT hr = NOERROR;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        for (int j = eventHandlerID; j < eventHandlers->Count(); j++)
        {
            int currentSinkItem = 0;
            BOOL eventHandlerFound = FALSE;
            BaseEventHandler* eventHandler = eventHandlers->Item(j);
            dispatch = eventHandler->GetDispatch();
            for (currentSinkItem = 0; currentSinkItem < eventSinks->Count(); currentSinkItem++)
            {
                if (dispatch == eventSinks->Item(currentSinkItem)->GetDispatch())
                {
                    eventHandlerFound = TRUE;
                    break;
                }
            }
            if (!eventHandlerFound)
            {
                Assert(currentSinkItem == eventSinks->Count());
                HRESULT hrT = AddEventSinks(this, dispatch);
                Assert(FAILED(hrT) || eventSinks->Item(eventSinks->Count()-1)->GetDispatch() == dispatch);
            }

            for (int i = 0; i < eventSinks->Count(); i++)
            {
                if (dispatch == eventSinks->Item(i)->GetDispatch())
                {
                    Js::DynamicObject * dynamicObject = eventHandler->GetScriptObject();
                    if (dynamicObject == nullptr)
                    {
                        return E_ACCESSDENIED;
                    }
                    eventSinks->Item(i)->RegisterEventHandler(eventHandler->GetEventHandlerName(), dynamicObject );
                }
            }
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}


/***************************************************************************
Named Item based event handler.
***************************************************************************/
NamedEventHandler::NamedEventHandler(void)
{
    refCount = 1;
    m_pos = nullptr;
    m_pnid = nullptr;
    m_pszSubItem = nullptr;
    eventHandlerName = nullptr;
    scriptBody = nullptr;
    m_pdisp = nullptr;
    shouldPersist = false;
    m_fTried = false;
}


NamedEventHandler::~NamedEventHandler(void)
{
    if (nullptr != m_pszSubItem)
        free((LPVOID)m_pszSubItem);
    if (nullptr != eventHandlerName)
        free((LPVOID)eventHandlerName);
    if (nullptr != scriptObject)
    {
        ScriptSite* scriptSite = m_pos->GetScriptSiteHolder();
        scriptObject.Unroot(scriptSite->GetRecycler());
    }
    if (nullptr != scriptBody)
        scriptBody->Release();
    if (nullptr != m_pdisp)
        m_pdisp->Release();
}


HRESULT NamedEventHandler::Create(
                                  __in NamedEventHandler **ppneh, 
                                  __in ScriptEngine *pos,
                                  __in NamedItem *pnid, 
                                  __in LPCOLESTR pszSubItem, 
                                  __in LPCOLESTR pszEvt, 
                                  __in CScriptBody *pbody,
                                  __in BOOL fPersist)
{
    AssertMem(ppneh);
    AssertMem(pos);
    AssertMem(pnid);
    AssertPszN(pszSubItem);
    AssertPsz(pszEvt);
    AssertMem(pbody);

    HRESULT hr;

    if (nullptr == (*ppneh = HeapNewNoThrow(NamedEventHandler)))
        return HR(E_OUTOFMEMORY);
    if (FAILED(hr = (*ppneh)->Init(pos, pnid, pszSubItem, pszEvt, pbody,
        fPersist)))
    {
        (*ppneh)->Release();
        *ppneh = nullptr;
        return hr;
    }
    return NOERROR;
}


HRESULT NamedEventHandler::Init(ScriptEngine *pos, NamedItem *pnid,
                                LPCOLESTR pszSubItem, LPCOLESTR pszEvt, CScriptBody *pbody, BOOL fPersist)
{
    AssertMem(pos);
    AssertMem(pnid);
    AssertPszN(pszSubItem);
    AssertPsz(pszEvt);
    AssertMem(pbody);

    m_pos = pos;
    m_pnid = pnid;
    if (nullptr == pszSubItem)
        m_pszSubItem = nullptr;
    else if (nullptr == (m_pszSubItem = ostrdup(pszSubItem)))
        return HR(E_OUTOFMEMORY);
    if (NULL == (eventHandlerName = ostrdup(pszEvt)))
        return HR(E_OUTOFMEMORY);
    scriptBody = pbody;
    scriptBody->AddRef();
    shouldPersist = (fPersist != FALSE);

    return NOERROR;
}


IDispatch* NamedEventHandler::GetDispatch(void)
{
    if (m_pdisp != nullptr)
    {
        return m_pdisp;
    }
    m_fTried = true;

    if (FAILED(m_pos->GetObjectOfItem(&m_pdisp, m_pnid, m_pszSubItem)))
        m_pdisp = nullptr;

    return m_pdisp;
}


Js::DynamicObject* NamedEventHandler::GetScriptObject(void)
{
    ScriptSite *scriptSite;

    if (nullptr != scriptObject)
        return scriptObject;

    if (nullptr == (scriptSite = m_pos->GetScriptSiteHolder()))
        return nullptr;

    this->scriptObject.Root(scriptBody->CreateEntryPoint(scriptSite), scriptSite->GetRecycler());    
    return scriptObject;
}


HRESULT NamedEventHandler::Reset(void)
{
    if (nullptr != scriptBody)
    {
        scriptBody->Release();
        scriptBody = nullptr;
    }
    if (nullptr != m_pdisp)
    {
        m_pdisp->Release();
        m_pdisp = nullptr;
    }

    if (nullptr != scriptObject)
    {
        ScriptSite* scriptSite = m_pos->GetScriptSiteHolder();
        scriptObject.Unroot(scriptSite->GetRecycler());        
    }

    return NOERROR;
}


HRESULT NamedEventHandler::Clone(__in ScriptEngine *pos, __out BaseEventHandler **ppeh)
{
    AssertMem(ppeh);
    AssertMem(pos);
    Assert(shouldPersist);

    NamedEventHandler *eventHandlerNew;
    HRESULT hr;

    *ppeh = nullptr;
    if (nullptr == (eventHandlerNew = HeapNewNoThrow(NamedEventHandler)))
        return E_OUTOFMEMORY;

    eventHandlerNew->m_pos = pos;
    if (nullptr == (eventHandlerNew->m_pnid = pos->FindNamedItem(m_pnid->bstrItemName)))
    {
        hr = HR(E_UNEXPECTED);
        goto LFail;
    }
    if (nullptr == m_pszSubItem)
    {
        eventHandlerNew->m_pszSubItem = nullptr;
    }
    else if (nullptr == (eventHandlerNew->m_pszSubItem = ostrdup(m_pszSubItem)))
    {
        hr = HR(E_OUTOFMEMORY);
        goto LFail;
    }
    if (nullptr == (eventHandlerNew->eventHandlerName = ostrdup(eventHandlerName)))
    {
        hr = HR(E_OUTOFMEMORY);
        goto LFail;
    }
    if (nullptr == (eventHandlerNew->scriptBody = scriptBody->Clone(pos)))
    {
        hr = HR(E_OUTOFMEMORY);
        goto LFail;
    }
    eventHandlerNew->shouldPersist = true;

    *ppeh = eventHandlerNew;
    return NOERROR;
LFail:
    eventHandlerNew->Release();
    return hr;

}
