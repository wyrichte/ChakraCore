//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

// A class which is intermediate document provider, which actually delegates the call to the actual provider but changes the GetName implementation.

class ScriptDocumentProviderBridge sealed : public IDebugDocumentProvider
{
public :
    ScriptDocumentProviderBridge(IDebugDocumentProvider * pActualDocumentProvider, SourceContextInfo* sourceContextInfo);
    ~ScriptDocumentProviderBridge();

    // IUnknown:
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

    // IDebugDocumentProvider:
    STDMETHODIMP GetDocument(
        /* [out] */ IDebugDocument **ppssd);

    // IDebugDocumentInfo:
    STDMETHODIMP GetName(
        /* [in]  */ DOCUMENTNAMETYPE documentType,
        /* [out] */ BSTR *pbstrName);
    STDMETHODIMP GetDocumentClassId(CLSID *pclsidDocument);

private:
    long m_refCount;
    IDebugDocumentProvider * m_pActualDocumentProvider;
    SourceContextInfo* m_sourceContextInfo;
};

class CScriptBody;

// This class represents a debug document and provides routines to create and link sources to the PDM.
// In the debug mode, an instance will be created for each script block, even if the script block is host managed. (Which will help the CCodeContext to get it work done)
// For non-host managed blocks (eg. Dynamic code), it will create a IDebugDocumentHelper and do the DefineScriptBlock and AddUnicodeText

class ScriptDebugDocument sealed : public IUnknown, public Js::DebugDocument
{
public:
    ScriptDebugDocument(CScriptBody *pScriptBody, DWORD_PTR debugSourceContext);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);
    HRESULT ReParentToCaller();
    HRESULT Register(const char16 *title);

    HRESULT EnumCodeContextsOfHostPosition(ULONG uCharacterOffset, ULONG uNumChars, IEnumDebugCodeContexts **ppEnumCodeContext);
    HRESULT EnumCodeContextsOfPosition(ULONG uCharacterOffset, ULONG uNumChars, IEnumDebugCodeContexts **ppEnumCodeContext);

    HRESULT GetDocumentContext(ULONG uCharacterOffset, ULONG  uNumChars, IDebugDocumentContext **    ppDebugDocumentContext);
    DWORD_PTR GetDebugSourceCookie() const { return m_debugSourceCookie; }

    void CloseDocument();

    void MarkForClose();

    CScriptBody * GetScriptBody() const { return m_pScriptBody; }
    void GetFormattedTitle(LPCWSTR title, _Out_z_cap_(length) LPWSTR formattedTitle, int length);
    HRESULT DbgGetRootApplicationNode(IDebugApplicationNode **ppdan);

    virtual bool HasDocumentText() const override;
    virtual void SetDocumentText(void* document);
    void* GetDocumentText() const override;
    void QueryDocumentText(IDebugDocumentText** ppDebugDocumentText);

private:
    HRESULT AddText();
    Js::ScriptContext * GetScriptContext() const;
    bool IsScriptActive() const;
    HRESULT AttachNode(IDebugApplicationNode *pNode, IDebugApplicationNode *pParentNode);

private:
    long                    m_refCount;

    CComPtr<IDebugDocumentHelper> m_debugDocHelper;

    CScriptBody *           m_pScriptBody;

    void* m_documentText;    // A unique document ID, which debugger uses for the source file matching.

    // Populated from the IDebugDocuementHelper (When current source is not host managed)
    DWORD_PTR               m_debugSourceCookie;

    BOOL                    m_isMarkedClosed;
    bool                    m_isAttached;
};

