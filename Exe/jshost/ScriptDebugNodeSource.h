#pragma once

class ScriptDebugNodeSource : public IUnknown
{
public:
    ScriptDebugNodeSource(void);
    ~ScriptDebugNodeSource(void);

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    //
    // Helper functions
    BSTR GetUrl();

    HRESULT GetPostionOfLine(ULONG ulLineNumber, ULONG *pcCharPos);
    HRESULT GetDebugCodeContext(ULONG ulFirstCharPos, ULONG ulCharPosCount, IDebugCodeContext **ppDebugCodeContext);
    HRESULT GetDebugCodeContext(ULONG ulLineNumber, ULONG ulColumnNumber, ULONG ulCharPosCount, IDebugCodeContext **ppDebugCodeContext);

    IDebugApplicationNode * GetDebugApplicationNode() { return m_spDbgAppNode; }
    IDebugDocumentText * GetDocumentText() { return m_spDebugDocumentText; }
    ULONG  GetSourceId() const { return m_ulSourceId; }
    ULONG  GetContainerSourceId() const { return m_ulContainerSourceId; }

    ULONG GetTextLength() { return m_uCchText; }

    void OnInsertText();

    void Init(__in IDebugApplicationNode *pDebugApplicationNode, __in ULONG ulContainerSourceId, __in ULONG ulSourceId, __in Debugger *pDebugCore);

    bool ShouldParticipateInInsertBp() const { return m_readyForInsertBp; }
    void SetParticipateInInsertBp(bool set) { m_readyForInsertBp = set; }

    bool HasFailedToSetBp() const { return m_hasFailedToSetBp; }
    void SetFailedToSetBp(bool set) { m_hasFailedToSetBp = set; }

    void DisconnectEventSinks();

    // This implements the source node and source text update event sink
    class SourceNodeEventSink : public IDebugApplicationNodeEvents, public IDebugDocumentTextEvents
    {
    public:
        SourceNodeEventSink() 
            : m_refCount(0)
        { }

        ~SourceNodeEventSink();

        // IUnknown
        HRESULT STDMETHODCALLTYPE QueryInterface(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

        ULONG STDMETHODCALLTYPE AddRef();
        ULONG STDMETHODCALLTYPE Release();

        // IDebugApplicationNodeEvents
        HRESULT STDMETHODCALLTYPE onAddChild(IDebugApplicationNode* prddpChild);
        HRESULT STDMETHODCALLTYPE onRemoveChild(IDebugApplicationNode* prddpChild);
        HRESULT STDMETHODCALLTYPE onDetach();
        HRESULT STDMETHODCALLTYPE onAttach(IDebugApplicationNode* prddpParent);

        // IDebugDocumentTextEvents
        HRESULT STDMETHODCALLTYPE onDestroy();
        HRESULT STDMETHODCALLTYPE onInsertText(ULONG cCharacterPosition, ULONG cNumToInsert);
        HRESULT STDMETHODCALLTYPE onRemoveText(ULONG cCharacterPosition, ULONG cNumToRemove);
        HRESULT STDMETHODCALLTYPE onReplaceText(ULONG cCharacterPosition, ULONG cNumToReplace);
        HRESULT STDMETHODCALLTYPE onUpdateTextAttributes(ULONG cCharacterPosition, ULONG cNumToUpdate);
        HRESULT STDMETHODCALLTYPE onUpdateDocumentAttributes(TEXT_DOC_ATTR textdocattr);


        void Init(ScriptDebugNodeSource *pSourceFile, bool isForNodeEvent);

        HRESULT ConnectSinkForNodeEvent();
        HRESULT ConnectSinkForTextEvent(__in IDebugDocumentText *pDebugDocumentText);
        void DisconnectSink();

    private:
        void DisconnectSinkInternal();

    private:
        long                        m_refCount;

        ScriptDebugNodeSource *     m_pSourceFile;
        CComPtr<IConnectionPoint>   m_spConnectionPoint;
        DWORD                       m_dwCookie;
        bool                        m_fIsForNodeEvent;
    };


private :
    void FetchFileUrl();

private:
    long                            m_refCount;

    ULONG                           m_ulSourceId;
    ULONG                           m_ulContainerSourceId;
    CComPtr<IDebugApplicationNode>  m_spDbgAppNode;
    CComPtr<IDebugDocumentText>     m_spDebugDocumentText;

    CComPtr<Debugger>               m_spDebugger;

    bool                            m_readyForInsertBp;
    bool                            m_hasFailedToSetBp;

    // Pointer to the container
    CComBSTR                        m_spUrlBstr;
    CComBSTR                        m_spBstrSource;
    ULONG                           m_uCchText;

    CComPtr<SourceNodeEventSink>    m_spSourceEvents; // event sink for source node
    CComPtr<SourceNodeEventSink>    m_spSourceTextEvents; // event sink for source text change
};

