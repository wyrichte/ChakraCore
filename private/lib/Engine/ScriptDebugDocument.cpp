//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "EnginePch.h"

ScriptDocumentProviderBridge::ScriptDocumentProviderBridge(IDebugDocumentProvider * pActualDocumentProvider, SourceContextInfo* sourceContextInfo)
    : m_refCount(0), m_pActualDocumentProvider(pActualDocumentProvider), m_sourceContextInfo(sourceContextInfo)
{
    Assert(m_sourceContextInfo);
    AssertMsg(!(m_sourceContextInfo->IsDynamic() && m_sourceContextInfo->sourceMapUrl), 
        "Can't have dynamic sctipt with source map url, source maps are only for actual script files.");

    if (m_pActualDocumentProvider)
    {
        m_pActualDocumentProvider->AddRef();
    }
}

ScriptDocumentProviderBridge::~ScriptDocumentProviderBridge()
{
    if (m_pActualDocumentProvider)
    {
        m_pActualDocumentProvider->Release();
    }
}

HRESULT ScriptDocumentProviderBridge::QueryInterface(REFIID riid, void** ppvObject) 
{
    if (riid == _uuidof(IDebugDocumentProvider))
    {
        *ppvObject =  static_cast<IDebugDocumentProvider*>(this);
    }
    else if (riid == _uuidof(IDebugDocumentInfo))
    {
        *ppvObject =  static_cast<IDebugDocumentInfo*>(this);
    }
    else if (riid == _uuidof(IUnknown))
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

ULONG ScriptDocumentProviderBridge::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG ScriptDocumentProviderBridge::Release()
{
    long currentCount = InterlockedDecrement(&m_refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

HRESULT ScriptDocumentProviderBridge::GetDocument(IDebugDocument **ppssd)
{
    if (m_pActualDocumentProvider)
    {
        return m_pActualDocumentProvider->GetDocument(ppssd);
    }
    return E_FAIL;
}

HRESULT ScriptDocumentProviderBridge::GetName(DOCUMENTNAMETYPE documentType, BSTR *pbstrName)
{
    if (m_pActualDocumentProvider == NULL)
    {
        return E_FAIL;
    }
    if (pbstrName == nullptr)
    {
        return E_INVALIDARG;
    }

    *pbstrName = NULL;

    Assert(m_sourceContextInfo);
    if (m_sourceContextInfo->IsDynamic())
    {
        switch (documentType)
        {
            case DOCUMENTNAMETYPE_APPNODE:
            case DOCUMENTNAMETYPE_FILE_TAIL:
            case DOCUMENTNAMETYPE_TITLE:
                return m_pActualDocumentProvider->GetName(DOCUMENTNAMETYPE_APPNODE, pbstrName);

            case DOCUMENTNAMETYPE_UNIQUE_TITLE:
            case DOCUMENTNAMETYPE_URL:
                return m_pActualDocumentProvider->GetName(DOCUMENTNAMETYPE_URL, pbstrName);
        }
    }
    if (documentType == DOCUMENTNAMETYPE_SOURCE_MAP_URL)
    {
        if (m_sourceContextInfo->sourceMapUrl)
        {
            *pbstrName = SysAllocString(m_sourceContextInfo->sourceMapUrl);
        }
        // If there is no map available, we still return S_OK with pbstrName receiving NULL.
        return S_OK;    
    }

    return m_pActualDocumentProvider->GetName(documentType, pbstrName);
}

HRESULT ScriptDocumentProviderBridge::GetDocumentClassId(CLSID *pclsidDocument)
{
    if (m_pActualDocumentProvider)
    {
        return m_pActualDocumentProvider->GetDocumentClassId(pclsidDocument);
    }
    return E_FAIL;

}


ScriptDebugDocument::ScriptDebugDocument(CScriptBody *pScriptBody, DWORD_PTR debugSourceContext) : 
    DebugDocument(pScriptBody->GetUtf8SourceInfo(), 
                  pScriptBody->GetRootFunction() ? pScriptBody->GetRootFunction()->GetFunctionBody() : nullptr),
    m_refCount(1),
    m_pScriptBody(pScriptBody),
    m_debugSourceCookie(debugSourceContext),
    m_documentText(nullptr),
    m_isMarkedClosed(FALSE),
    m_isAttached(false)
{
    if (m_pScriptBody)
    {
        m_pScriptBody->AddRef();
    }
}

void ScriptDebugDocument::MarkForClose()
{
    m_isMarkedClosed = TRUE;
}

HRESULT ScriptDebugDocument::QueryInterface(REFIID riid, void** ppvObject) 
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

ULONG ScriptDebugDocument::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG ScriptDebugDocument::Release()
{
    long currentCount = InterlockedDecrement(&m_refCount);
    if (currentCount == 0)
    {
        HeapDelete(this);
    }
    return currentCount;
}

HRESULT ScriptDebugDocument::ReParentToCaller()
{
    HRESULT hr = S_OK;
    if (m_debugDocHelper && m_isAttached)
    {
        CComPtr<IDebugApplicationNode> pNode;
        if (m_debugDocHelper->GetDebugApplicationNode(&pNode) == S_OK)
        {
            Js::Utf8SourceInfo* callerUtfSourceInfo = m_pScriptBody->GetUtf8SourceInfo()->GetCallerUtf8SourceInfo();
            if (callerUtfSourceInfo && !callerUtfSourceInfo->IsHostManagedSource() && callerUtfSourceInfo->HasDebugDocument())
            {
                ScriptDebugDocument* callerDocument = static_cast<ScriptDebugDocument*>(callerUtfSourceInfo->GetDebugDocument());
                Assert(callerDocument && callerDocument->m_debugDocHelper && callerDocument->m_isAttached);
                if (callerDocument->m_debugDocHelper && callerDocument->m_isAttached)
                {
                    CComPtr<IDebugApplicationNode> pCallerNode;
                    if (callerDocument->m_debugDocHelper->GetDebugApplicationNode(&pCallerNode) == S_OK)
                    {
                        Assert(pCallerNode != nullptr);
                        // Saving the exception, as this code path could re-enter through jscript debugger APIs
                        BEGIN_NO_EXCEPTION
                        {
                            hr = m_debugDocHelper->Detach();
                        }
                        END_NO_EXCEPTION
                        if (SUCCEEDED(hr))
                        {
                            m_isAttached = false;
                            hr = AttachNode(pNode, pCallerNode);
                            if (SUCCEEDED(hr))
                            {
                                m_isAttached = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return hr;
}

//
// Order of registration is important as VS and other debuggers rely on this:
// For sources managed by script engine (not by host):
// 1. Initialize debug document helper
// 2. Store reference to IDebugDocumentText in Utf8SourceInfo (used by jscript9diag.dll for setting breakpoints etc)
// 3. Insert the text using debug document helper.
// 4. Attach the node to the hierearchy
//
HRESULT ScriptDebugDocument::Register(const char16 * title)
{
    Assert(!m_isMarkedClosed);
    Assert(m_debugDocHelper == NULL);

    ScriptEngine* scriptEngine = m_pScriptBody->GetScriptEngine();
    HRESULT hr = S_OK;
    Js::Utf8SourceInfo* utfSourceInfo = m_pScriptBody->GetUtf8SourceInfo();
    const SRCINFO * srcInfo = utfSourceInfo->GetSrcInfo();
    bool isDynamic = srcInfo->sourceContextInfo->IsDynamic();
    bool isHostManagedSource = utfSourceInfo->IsHostManagedSource();
    WCHAR * shortName = NULL;
    WCHAR *longName = NULL;
    CComPtr<IDebugApplicationNode> pParentNode;
    CComPtr<IDebugApplicationNode> pNode;
    CComPtr<IDebugDocumentContext> spDebugDocumentContext;
    const int nameSize = 255;
    if (!isHostManagedSource)
    {
        if (ShouldUseLocalPDM())
        {
            // CreateDebugDocumentHelper simply calls CoCreate on CLSID_CDebugDocumentHelper
            // When using local PDM we can't use CoCreate to get to this, instead
            // use LoadLatestPDM
            hr = LoadLatestPDM(__uuidof(CDebugDocumentHelper),IID_PPV_ARGS(&m_debugDocHelper));
        }
        else
        {
            hr = CoCreateInstance(__uuidof(CDebugDocumentHelper), NULL, CLSCTX_INPROC_SERVER, _uuidof(IDebugDocumentHelper), (LPVOID*)&m_debugDocHelper);
        }

        if (hr != S_OK)
        {
            return hr;
        }

        Assert(m_debugDocHelper);

        if(!title)
        {
            title = _u("script block");
        }

        if (isDynamic)
        {
            shortName = const_cast<char16*>(title);
            longName = HeapNewNoThrowArray(WCHAR, nameSize);
            IfNullReturnError(longName, E_OUTOFMEMORY);
            this->GetFormattedTitle(shortName, longName, nameSize);
        }
        else
        {
            longName = const_cast<char16*>(title);
            shortName = HeapNewNoThrowArray(WCHAR, nameSize);
            IfNullReturnError(shortName, E_OUTOFMEMORY);
            Js::FunctionBody::GetShortNameFromUrl(longName, shortName, nameSize);
        }

        IDebugApplication *pDebugApp = NULL;
        
        IfFailGo(scriptEngine->GetDebugApplicationCoreNoRef(&pDebugApp));
        
        // For dynamic stuff, no text attribute have defined, previously TEXT_DOC_ATTR_READONLY is always used.
        IfFailGo(m_debugDocHelper->Init(pDebugApp, shortName,  longName, isDynamic ? TEXT_DOC_ATTR_READONLY : TEXT_DOC_ATTR_TYPE_SCRIPT));

        IfFailGo(m_debugDocHelper->DefineScriptBlock(0, utfSourceInfo->GetCchLength(), scriptEngine, FALSE /*no scriptlet*/, &m_debugSourceCookie));
    } // !isHostManagedSource
    OUTPUT_TRACE(Js::DebuggerPhase, _u("ScriptDebugDocument::Register: Host managed source: %d, dwsourcecontext %p, Title: %s \n"), isHostManagedSource, srcInfo->sourceContextInfo->dwHostSourceContext, title);
    
    // Ensure Utf8SourceInfo has a reference to IDebugDocumentText before we add the text.
    if(SUCCEEDED(this->GetDocumentContext(srcInfo->ichMinHost, /*length*/ 1, &spDebugDocumentContext)))
    {
        CComPtr<IDebugDocument> spDebugDocument;
        if(SUCCEEDED(spDebugDocumentContext->GetDocument(&spDebugDocument)))
        {
            CComPtr<IDebugDocumentText> spDebugDocumentText;
            if(SUCCEEDED(spDebugDocument->QueryInterface(&spDebugDocumentText)))
            {
                ((ScriptDebugDocument*)utfSourceInfo->GetDebugDocument())->SetDocumentText(spDebugDocumentText);
            }
        }
    }

    if (!isHostManagedSource)
    {
        IfFailGo(AddText());
        
        if (m_debugDocHelper->GetDebugApplicationNode(&pNode) == S_OK)
        {
            /***********************************************************************************
                Root Application node - GetDebugApplicationCoreNoRef
                |- Top Most Url node - DbgGetRootApplicationNode
                    |- Included js file - 1
                        |- Dynamic scripts
                            |- eval code(1)
                                |- eval code(2) - eval called from above eval
                    |- Included js file - 2
                |- Dynamic scripts
                    |- eval code(3) - eval code which doesn't belong to page - like console eval
            ***********************************************************************************/

            Js::Utf8SourceInfo* callerUtfSourceInfo = utfSourceInfo->GetCallerUtf8SourceInfo();
            if (callerUtfSourceInfo) 
            {
                ScriptDebugDocument* parentDocument = (callerUtfSourceInfo->HasDebugDocument()) ? static_cast<ScriptDebugDocument*>(callerUtfSourceInfo->GetDebugDocument()) : nullptr;
                if (parentDocument && parentDocument->m_debugDocHelper)
                {
                    parentDocument->m_debugDocHelper->GetDebugApplicationNode(&pParentNode);
                }
                if (!pParentNode && isDynamic && !callerUtfSourceInfo->IsHostManagedSource())
                {
                    // Dynamic code which doesn't have a parent node should go to Root Application node (parent node of top most Url) so that F12 can show it under Dynamic scripts
                    IDebugApplication* application = nullptr;
                    HRESULT hr2 = scriptEngine->GetDebugApplicationCoreNoRef(&application);
                    AssertMsg(hr2 == S_OK, "Failed to get root application node");
                    application->GetRootNode(&pParentNode);
                }
            }
            
            if (!pParentNode)
            {
                // If we failed to find a parent default to top most node (node of top most Url)
                this->DbgGetRootApplicationNode(&pParentNode);
            }
            
            CComPtr<IDebugDocumentProvider> debugDocProvider;
            if (m_debugDocHelper->QueryInterface(&debugDocProvider) == S_OK)
            {
                // No matter if source map is present or not, we need to customize the document provider, so that we can intercept the GetName.
                // This way we have more control for case like no source map available and return deterministic HR (S_OK and null/empty BSTR).
                CComPtr<ScriptDocumentProviderBridge> spDocProviderBridge(
                    HeapNewNoThrow(ScriptDocumentProviderBridge, debugDocProvider, srcInfo->sourceContextInfo));
                if (spDocProviderBridge)
                {
                    pNode->SetDocumentProvider(spDocProviderBridge);
                }
                else
                {
                    pNode->SetDocumentProvider(debugDocProvider);
                }
            }
        }

        hr = AttachNode(pNode, pParentNode);
        if(SUCCEEDED(hr))
        {
            m_isAttached = true;
        }

    }

Error:
    if (isDynamic)
    {
        if(longName)
        {
            HeapDeleteArray(nameSize, longName);
        }
    }
    else
    {
        if(shortName)
        {
            HeapDeleteArray(nameSize, shortName);
        }
    }
    return hr;
}

HRESULT ScriptDebugDocument::AttachNode(IDebugApplicationNode *pNode, IDebugApplicationNode *pParentNode)
{
    // Attach goes to PDM (outside of scriptengine), and it is possible that during that time, the debugger execute the scripts
    // Guard that by saying that we are leaving script. However in some instances we are not in the script at all.
    HRESULT hr;
    if (IsScriptActive())
    {
        BEGIN_LEAVE_SCRIPT(GetScriptContext())
        {
            if (pNode && pParentNode)
            {
                hr = pNode->Attach(pParentNode);
            }
            else
            {
                hr = m_debugDocHelper->Attach(NULL);
            }
        }
        END_LEAVE_SCRIPT(GetScriptContext())
    }
    else
    {
        // Saving the exception state, as this code path could re-enter through jscript debugger APIs
        BEGIN_NO_EXCEPTION
        {
            if (pNode && pParentNode)
            {
                hr = pNode->Attach(pParentNode);
            }
            else
            {
                hr = m_debugDocHelper->Attach(NULL);
            }
        }
        END_NO_EXCEPTION
    }
    return hr;
}

void ScriptDebugDocument::CloseDocument()
{
    __super::CloseDocument();

    MarkForClose();

    if (m_pScriptBody)
    {
        m_pScriptBody->Release();
        m_pScriptBody = NULL;
    }

    if (m_debugDocHelper)
    {
        if(m_isAttached)
        {
            // Saving the exception, as this code path could re-enter through jscript debugger APIs
            BEGIN_NO_EXCEPTION
            {
                m_debugDocHelper->Detach();
            }
            END_NO_EXCEPTION
        }
        m_debugDocHelper.Release();
        m_debugDocHelper = NULL;
    }

    Release();
}

Js::ScriptContext * ScriptDebugDocument::GetScriptContext() const
{
    Assert(m_pScriptBody);
    return m_pScriptBody->GetScriptContext();
}

bool ScriptDebugDocument::IsScriptActive() const
{
    return GetScriptContext()->GetThreadContext()->IsScriptActive();
}

HRESULT ScriptDebugDocument::AddText()
{
    Assert(m_pScriptBody);
    Assert(!m_isMarkedClosed);

#if DBG
    if (m_debugDocHelper == nullptr)
    {
        Assert(m_pScriptBody->GetUtf8SourceInfo()->IsHostManagedSource());
    }
#endif

    if (m_debugDocHelper != nullptr && m_pScriptBody->GetUtf8SourceInfo() != nullptr)
    {
        Js::Utf8SourceInfo *utf8SourceInfo = m_pScriptBody->GetUtf8SourceInfo();
        if (utf8SourceInfo->HasSource()) // On Intl initialization failure the utf8Source can be nullptr
        {
            int32 cchLength = utf8SourceInfo->GetCchLength();

            AutoArrayPtr<char16> sourceContent(HeapNewNoThrowArray(char16, cchLength + 1), cchLength + 1);
            if (sourceContent != nullptr)
            {
                size_t cbLength = utf8SourceInfo->GetCbLength();
                utf8::DecodeOptions options = utf8SourceInfo->IsCesu8() ? utf8::doAllowThreeByteSurrogates : utf8::doDefault;
                LPCUTF8 source = utf8SourceInfo->GetSource();
                LPCUTF8 end = source + utf8::CharacterIndexToByteIndex(source, cbLength, cchLength, options);
                utf8::DecodeUnitsIntoAndNullTerminate(sourceContent, source, end, options);
                HRESULT hr = S_OK;
                // AddUnicodeText goes to PDM (outside of scriptengine), and it is possible that during that time, the debugger execute the scripts
                // Guard that by saying that we are leaving script. However in some instances we are not in the script at all.
                if (IsScriptActive())
                {
                    BEGIN_LEAVE_SCRIPT(GetScriptContext())
                    {
                        hr = m_debugDocHelper->AddUnicodeText(sourceContent);
                    }
                    END_LEAVE_SCRIPT(GetScriptContext())
                }
                else
                {
                    hr = m_debugDocHelper->AddUnicodeText(sourceContent);
                }

                return hr;
            }
        }
    }

    return E_FAIL;
}

HRESULT ScriptDebugDocument::EnumCodeContextsOfHostPosition(ULONG uCharacterOffset, ULONG uNumChars, IEnumDebugCodeContexts **ppEnumCodeContext)
{
    if (m_isMarkedClosed)
    {
        return E_FAIL;
    }

    Assert(ppEnumCodeContext);
    *ppEnumCodeContext = NULL;
    Assert(m_pScriptBody != NULL);
    Assert(m_pScriptBody->GetUtf8SourceInfo() != NULL);

    const SRCINFO * pSrcInfo = m_pScriptBody->GetUtf8SourceInfo()->GetSrcInfo();
    if (pSrcInfo)
    {
        return EnumCodeContextsOfPosition(uCharacterOffset + pSrcInfo->ichMinHost, uNumChars, ppEnumCodeContext);
    }

    return E_FAIL;
}

HRESULT ScriptDebugDocument::EnumCodeContextsOfPosition(ULONG uCharacterOffset, ULONG uNumChars, IEnumDebugCodeContexts **ppEnumCodeContext)
{
    Assert(ppEnumCodeContext);
    *ppEnumCodeContext = NULL;

    HRESULT hr;

    StatementSpan bos;
    bos.cch = (long)uNumChars;
    bos.ich = (long)uCharacterOffset;

    if (m_pScriptBody == NULL || !m_pScriptBody->GetStatementSpan(bos.ich, &bos))
    {
        return E_FAIL;
    }

    CCodeContext *codeContext = new CCodeContext(this, bos.ich, bos.cch);

    if (codeContext == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CEnumCodeContexts *enumCodeContexts = new CEnumCodeContexts;

    // An enumerator for the code context
    if (enumCodeContexts == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (FAILED(hr = enumCodeContexts->Init(codeContext)))
    {
        goto Exit;
    }

    *ppEnumCodeContext = enumCodeContexts;

    codeContext->Release();
    return NOERROR;

Exit:
    if (enumCodeContexts != NULL)
    {
        enumCodeContexts->Release();
    }

    if (codeContext != NULL)
    {
        codeContext->Release();
    }

    return hr;
}

HRESULT ScriptDebugDocument::GetDocumentContext(ULONG uCharacterOffset, ULONG  uNumChars, IDebugDocumentContext ** ppDebugDocumentContext)
{
    if (ppDebugDocumentContext == NULL)
    {
        return E_POINTER;
    }

    if (m_isMarkedClosed)
    {
        return E_FAIL;
    }

    if (m_debugDocHelper == NULL)
    {
        // this must be host managed.
        Assert(m_pScriptBody);
        const SRCINFO * srcInfo = m_pScriptBody->GetUtf8SourceInfo()->GetSrcInfo();
        Assert(srcInfo);

        if (m_pScriptBody->GetUtf8SourceInfo()->IsHostManagedSource())
        {
            ULONG ichStart = uCharacterOffset;
            ULONG ichEnd = uCharacterOffset + uNumChars;

            // Truncate range to host-owned portion

            if (ichStart > srcInfo->ichLimHost)
            {
                ichStart = srcInfo->ichLimHost;
            }
            if (ichStart < srcInfo->ichMinHost)
            {
                ichStart = srcInfo->ichMinHost;
            }
            if (ichEnd > srcInfo->ichLimHost)
            {
                ichEnd = srcInfo->ichLimHost;
            }
            if (ichEnd < ichStart)
            {
                ichEnd = ichStart;
            }

            return m_pScriptBody->GetScriptEngine()->GetDocumentContextFromPosition(
                m_debugSourceCookie,
                ichStart - srcInfo->ichMinHost,
                ichEnd - ichStart,
                ppDebugDocumentContext);
        }
        else
        {
            Assert(FALSE);
            return E_FAIL;
        }
    }

    // Managed by the engine, create the document context.

    HRESULT hr = E_FAIL;

    ULONG uStartOffset;

    hr = m_debugDocHelper->GetScriptBlockInfo(m_debugSourceCookie, NULL, &uStartOffset, NULL);
    if (SUCCEEDED(hr))
    {
        hr = m_debugDocHelper->CreateDebugDocumentContext(uStartOffset + uCharacterOffset, uNumChars, ppDebugDocumentContext);
    }

    return hr;
}

void ScriptDebugDocument::GetFormattedTitle(LPCWSTR title, _Out_z_cap_(length) LPWSTR formattedTitle, int length)
{
    ThreadContext *threadContext = m_pScriptBody->GetScriptContext()->GetThreadContext();
    int id = -1;
    if (wcscmp(title, Js::Constants::EvalCode) == 0)
    {
        id = threadContext->GetDebugManager()->GetNextId(Js::DebugManager::DFT_EvalCode);
    }
    else if (wcscmp(title, Js::Constants::FunctionCode) == 0)
    {
        id = threadContext->GetDebugManager()->GetNextId(Js::DebugManager::DFT_AnonymousCode);
    }
    else
    {
        id = threadContext->GetDebugManager()->GetNextId(Js::DebugManager::DFT_JScriptBlock);
    }
    swprintf_s(formattedTitle, length, OLESTR("%s (%d)"), title, id);
}

HRESULT ScriptDebugDocument::DbgGetRootApplicationNode(IDebugApplicationNode **ppdan)
{
    AssertMem(ppdan);
    HRESULT hr = NOERROR;

    // REVIEW: rokyu - can scriptSiteDebug->GetRootApplicationNode return a node
    // that is different from scriptSiteDebug->GetApplication->GetRootNode?
    IActiveScriptSiteDebug *scriptSiteDebug = nullptr;
    ScriptEngine* scriptEngine = m_pScriptBody->GetScriptEngine();

    if (SUCCEEDED(scriptEngine->GetDebugSiteNoRef(&scriptSiteDebug)))
    {
        CComPtr<IActiveScriptSiteDebugHelper> spScriptSiteDebugHelper;
        if (scriptSiteDebug->QueryInterface(&spScriptSiteDebugHelper) == S_OK)
        {
            hr = spScriptSiteDebugHelper->GetApplicationNode(ppdan);
        }
        else
        {
            hr = scriptSiteDebug->GetRootApplicationNode(ppdan);
        }
    }

    if(*ppdan == NULL)
    {
        IDebugApplication* application = nullptr;
        IfFailRet(scriptEngine->GetDebugApplicationCoreNoRef(&application));
        return application->GetRootNode(ppdan);
    }
    return hr;
}

bool ScriptDebugDocument::HasDocumentText() const
{
    return m_documentText != nullptr;
}

void ScriptDebugDocument::SetDocumentText(void* document)
{
    Assert(!HasDocumentText());
    m_documentText = document;
}

void* ScriptDebugDocument::GetDocumentText() const
{
    Assert(HasDocumentText());
    return m_documentText;
}

void ScriptDebugDocument::QueryDocumentText(IDebugDocumentText** ppDebugDocumentText)
{
    Assert(HasDocumentText());
    *ppDebugDocumentText = reinterpret_cast<IDebugDocumentText*>(m_documentText);
    (*ppDebugDocumentText)->AddRef();
}
