#include "stdafx.h"


ScriptDebugNodeSource::ScriptDebugNodeSource(void)
: m_refCount(0), m_ulSourceId(0), m_ulContainerSourceId(0), m_readyForInsertBp(false), m_hasFailedToSetBp(false)
{
}


ScriptDebugNodeSource::~ScriptDebugNodeSource(void)
{
}

HRESULT ScriptDebugNodeSource::QueryInterface(REFIID riid, void** ppvObject) 
{
    if (riid == _uuidof(IUnknown))
    {
        *ppvObject =  static_cast<IUnknown*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG ScriptDebugNodeSource::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG ScriptDebugNodeSource::Release()
{
    long currentCount = InterlockedDecrement(&m_refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

void ScriptDebugNodeSource::OnInsertText()
{
    // Notify about the insert text, so that the breakpoint can be resolved.
    m_spDebugger->OnInsertText(this);
}

void ScriptDebugNodeSource::Init(__in IDebugApplicationNode *pDebugApplicationNode, __in ULONG ulContainerSourceId, __in ULONG ulSourceId, __in Debugger *pDebugCore)
{
    if (pDebugApplicationNode == NULL || pDebugCore == NULL)
    {
        fwprintf(stdout, L"[FAILED] at ScriptDebugNodeSource::Init (Invalid arg)\n");
        return;
    }

    m_spDbgAppNode = pDebugApplicationNode;
    m_spDebugger = pDebugCore;
    m_ulSourceId = ulSourceId;
    m_ulContainerSourceId = ulContainerSourceId;

    FetchFileUrl();

    m_spSourceEvents.p= new SourceNodeEventSink();
    m_spSourceEvents.p->AddRef();

    m_spSourceEvents->Init(this, true);

    CComPtr<IDebugDocument> spDebugDocument;
    HRESULT hr = pDebugApplicationNode->GetDocument(&spDebugDocument);

    if (hr == S_OK)
    {
        // Find out if this is HTML or JS file

        hr = spDebugDocument->QueryInterface(__uuidof(IDebugDocumentText),(LPVOID*)&m_spDebugDocumentText.p);

        if (hr == S_OK)
        {
            // Create the event handler for Source text events
            m_spSourceTextEvents.p  = new SourceNodeEventSink();
            m_spSourceTextEvents.p->AddRef();
            m_spSourceTextEvents->Init(this, false);

            // Check if we can resolve pending breakpoints
            // Resolve pending breakpoint
        }
    }
}

void ScriptDebugNodeSource::FetchFileUrl()
{
    HRESULT hr = m_spDbgAppNode->GetName(DOCUMENTNAMETYPE_URL, &m_spUrlBstr);

    if(FAILED(hr))
    {
        hr = m_spDbgAppNode->GetName(DOCUMENTNAMETYPE_UNIQUE_TITLE, &m_spUrlBstr);

        if(FAILED(hr))
        {
            hr = m_spDbgAppNode->GetName(DOCUMENTNAMETYPE_APPNODE, &m_spUrlBstr);
        }
    }

    if (hr != S_OK)
    {
        fwprintf(stdout, L"[FAILED] to get file name at ScriptDebugNodeSource::FetchFileUrl\n");
    }
}

BSTR ScriptDebugNodeSource::GetUrl()
{
    return m_spUrlBstr.m_str;
}

HRESULT ScriptDebugNodeSource::GetPostionOfLine(ULONG ulLineNumber, ULONG *pcCharPos)
{
    if (m_spDebugger->IsAttached() && m_spDebugDocumentText != NULL)
    {
        IDebugDocumentText *pDebugDocumentText = m_spDebugDocumentText;
        pDebugDocumentText->AddRef();
        HRESULT hr = pDebugDocumentText->GetPositionOfLine(ulLineNumber, pcCharPos);
        pDebugDocumentText->Release();
        return hr;
    }

    fwprintf(stdout, L"[FAILED] at ScriptDebugNodeSource::GetPostionOfLine\n");

    return E_FAIL;
}

HRESULT ScriptDebugNodeSource::GetDebugCodeContext(ULONG ulLineNumber, ULONG ulColumnNumber, ULONG ulCharPosCount, IDebugCodeContext **ppDebugCodeContext)
{
    ULONG ulFirstCharPos;
    HRESULT hr = GetPostionOfLine(ulLineNumber, &ulFirstCharPos);

    if (hr != S_OK)
    {
        return hr;
    }

    ulFirstCharPos += ulColumnNumber;

    return GetDebugCodeContext(ulFirstCharPos, ulCharPosCount, ppDebugCodeContext);
}

HRESULT ScriptDebugNodeSource::GetDebugCodeContext(ULONG ulFirstCharPos, ULONG ulCharPosCount, IDebugCodeContext **ppDebugCodeContext)
{
    HRESULT hr = m_spDebugger->IsAttached() ? S_OK : E_FAIL;

    if (hr != S_OK)
    {
        return hr;
    }

    if (!m_spDebugDocumentText)
    {
        return E_FAIL;
    }

    IDebugDocumentText *pDebugDocumentText = m_spDebugDocumentText;
    pDebugDocumentText->AddRef();

    CComPtr<IDebugDocumentContext> spDebugDocumentContext;
    hr = pDebugDocumentText->GetContextOfPosition(ulFirstCharPos, ulCharPosCount,  &spDebugDocumentContext);
    if (hr == S_OK)
    {
        CComPtr<IEnumDebugCodeContexts> spEnumDebugCodeContexts;
        hr = spDebugDocumentContext->EnumCodeContexts(&spEnumDebugCodeContexts);

        if (hr == S_OK)
        {
            ULONG ulFetched;
            hr = spEnumDebugCodeContexts->Next(1, ppDebugCodeContext, &ulFetched);
        }
        else
        {
            // Allow setting bp for the next time.
            SetFailedToSetBp(true);
        }
    }

    pDebugDocumentText->Release();

    return hr;
}



HRESULT ScriptDebugNodeSource::SourceNodeEventSink::QueryInterface(REFIID riid, void** ppvObject) 
{
    if (riid == _uuidof(IUnknown))
    {
        *ppvObject =  static_cast<IUnknown*>(static_cast<IDebugApplicationNodeEvents*>(this));
    }
    else if (riid == _uuidof(IDebugApplicationNodeEvents))
    {
        *ppvObject =  static_cast<IDebugApplicationNodeEvents*>(this);
    }
    else if (riid == _uuidof(IDebugDocumentTextEvents))
    {
        *ppvObject =  static_cast<IDebugDocumentTextEvents*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG ScriptDebugNodeSource::SourceNodeEventSink::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG ScriptDebugNodeSource::SourceNodeEventSink::Release()
{
    long currentCount = InterlockedDecrement(&m_refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

void ScriptDebugNodeSource::SourceNodeEventSink::Init (ScriptDebugNodeSource *pSourceFile, bool isForNodeEvent)
{
    m_pSourceFile = pSourceFile;

    m_fIsForNodeEvent = isForNodeEvent;
    if (m_fIsForNodeEvent)
    {
        m_pSourceFile->GetDebugApplicationNode()->AddRef();
        ConnectSinkForNodeEvent();
        m_pSourceFile->AddRef();
    }
    else
    {
        if (FAILED(ConnectSinkForTextEvent(m_pSourceFile->GetDocumentText())))
        {
            m_pSourceFile = NULL;
        }
        else
        {
            m_pSourceFile->GetDocumentText()->AddRef();
            m_pSourceFile->AddRef();
        }
    }
}

ScriptDebugNodeSource::SourceNodeEventSink::~SourceNodeEventSink()
{
    if (m_fIsForNodeEvent)
    {
        onDetach();
    }
    else
    {
        onDestroy();
    }
}

void ScriptDebugNodeSource::DisconnectEventSinks()
{
    if (this->m_spSourceEvents)
    {
        this->m_spSourceEvents->DisconnectSink();
        this->m_spSourceEvents.Release();
    }
    if (this->m_spSourceTextEvents)
    {
        this->m_spSourceTextEvents->DisconnectSink();
        this->m_spSourceTextEvents.Release();
    }
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::ConnectSinkForNodeEvent()
{
    Assert(m_spConnectionPoint == NULL);

    HRESULT hr;

    // Get the IConnectionPointContainer for the node
    CComPtr<IConnectionPointContainer> spConnectionPointContainer;
    
    hr = m_pSourceFile->GetDebugApplicationNode()->QueryInterface(__uuidof(IConnectionPointContainer), (void**) &spConnectionPointContainer);

    if (hr == S_OK)
    {
        // Get the IDebugApplicationNodeEvents
        CComPtr<IDebugApplicationNodeEvents> spSink;
        hr = QueryInterface(__uuidof(IDebugApplicationNodeEvents), (void**) & spSink);

        if (hr == S_OK)
        {
            // Find the connection point for IDebugApplicationNodeEvents
            hr = spConnectionPointContainer->FindConnectionPoint(__uuidof(IDebugApplicationNodeEvents), &m_spConnectionPoint);
            if (hr == S_OK)
            {
                // Connect the sink
                hr = m_spConnectionPoint->Advise(spSink, &m_dwCookie);
                if (hr != S_OK)
                {
                    DisconnectSinkInternal();
                }
            }
        }
    }
    return hr;
}

void ScriptDebugNodeSource::SourceNodeEventSink::DisconnectSinkInternal()
{
    if (m_spConnectionPoint)
    {
        m_spConnectionPoint.Release();

        // Release all client references to this object
        CComPtr<IUnknown> spUnk;
        if (SUCCEEDED(QueryInterface(IID_IUnknown, (void**) & spUnk)))
        {
            // CoDisconnect in case the server has died
            CoDisconnectObject(spUnk, 0);
        }
    }
}

void ScriptDebugNodeSource::SourceNodeEventSink::DisconnectSink(void)
{
    if (m_spConnectionPoint)
    {
        m_spConnectionPoint->Unadvise(m_dwCookie);
        DisconnectSinkInternal();
    }
}


HRESULT ScriptDebugNodeSource::SourceNodeEventSink::ConnectSinkForTextEvent(__in IDebugDocumentText *pDebugDocumentText)
{
    Assert(m_spConnectionPoint == NULL);

    // Get the IConnectionPointContainer for the node
    CComPtr<IConnectionPointContainer> spConnectionPointContainer;
    HRESULT hr = m_pSourceFile->GetDocumentText()->QueryInterface(__uuidof(IConnectionPointContainer), (void**) & spConnectionPointContainer);

    if (SUCCEEDED(hr))
    {
        // Get the IDebugDocumentTextEvents
        CComPtr<IDebugDocumentTextEvents> spSink;
        hr = QueryInterface(__uuidof(IDebugDocumentTextEvents), (void**) & spSink);

        if (SUCCEEDED(hr))
        {
            // Find the connection point for IDebugDocumentTextEvents
            hr = spConnectionPointContainer->FindConnectionPoint(__uuidof(IDebugDocumentTextEvents), &m_spConnectionPoint);
            if (SUCCEEDED(hr))
            {
                // Connect the sink
                hr = m_spConnectionPoint->Advise(spSink, &m_dwCookie);
                if (FAILED(hr))
                {
                    DisconnectSinkInternal();
                }
            }
        }
    }

    return hr;
}


// IDebugApplicationNodeEvents
HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onAddChild(IDebugApplicationNode* prddpChild)
{
    if (m_pSourceFile->m_spDebugger->IsAttached())
    {
        m_pSourceFile->m_spDebugger->AddNode(prddpChild, m_pSourceFile->GetSourceId());
    }

    return S_OK;
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onRemoveChild(IDebugApplicationNode* prddpChild)
{
    m_pSourceFile->m_spDebugger->RemoveNode(prddpChild);
    return S_OK;
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onDetach()
{
    DisconnectSink();

    ScriptDebugNodeSource *pSourceFile = m_pSourceFile;
    m_pSourceFile = NULL;
    if (pSourceFile)
    {
        pSourceFile->GetDebugApplicationNode()->Release();
        pSourceFile->Release();
    }

    return S_OK;
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onAttach(IDebugApplicationNode* prddpParent)
{
    return S_OK;
}


// IDebugDocumentTextEvents
HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onDestroy() 
{ 
    DisconnectSink();

    Assert(!m_fIsForNodeEvent);

    // For removing re-entrancy problem
    ScriptDebugNodeSource *pSourceFile = m_pSourceFile;
    m_pSourceFile = NULL;
    if (pSourceFile)
    {
        pSourceFile->GetDocumentText()->Release();
        pSourceFile->Release();
    }

    return S_OK;
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onInsertText(ULONG cCharacterPosition, ULONG cNumToInsert)
{
    if (m_pSourceFile->m_spDebugger->IsAttached() && (cNumToInsert > 0 || m_pSourceFile->HasFailedToSetBp()))
    {
        m_pSourceFile->SetFailedToSetBp(false);
        m_pSourceFile->OnInsertText();
    }

    return S_OK;
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onRemoveText(ULONG cCharacterPosition, ULONG cNumToRemove)
{
    return S_OK;
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onReplaceText(ULONG cCharacterPosition, ULONG cNumToReplace)
{
    return S_OK;
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onUpdateTextAttributes(ULONG cCharacterPosition, ULONG cNumToUpdate)
{
    return S_OK;
}

HRESULT ScriptDebugNodeSource::SourceNodeEventSink::onUpdateDocumentAttributes(TEXT_DOC_ATTR textdocattr)
{
    return S_OK;
}


