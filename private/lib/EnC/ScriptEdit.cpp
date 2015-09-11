//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "EnCPch.h"
#ifdef EDIT_AND_CONTINUE
#include "ActiveScriptError.h"
#include "ByteCode\ByteCodeAPI.h"
namespace Js
{
    ScriptEdit::ScriptEdit() :
        m_scriptEngine(nullptr),
        m_allocator(nullptr),
        m_baselineGeneration(0)
    {
    }

    HRESULT ScriptEdit::Init(ScriptEngine* scriptEngine)
    {
        m_scriptEngine = scriptEngine;

        m_allocator = scriptEngine->GetScriptContext()->GetThreadContext()->GetRecycler();
        Assert(m_allocator);

        return S_OK;
    }

    void ScriptEdit::FinalDestruct()
    {
        __super::FinalDestruct();
    }

    STDMETHODIMP ScriptEdit::QueryEdit(
        /* [size_is][in] */ ScriptEditRequest *requests,
        /* [in] */ ULONG count,
        /* [out] */ IScriptEditQuery **ppQueryResult)
    {
        HRESULT hr = S_OK;
        OUTPUT_TRACE(Phase::ENCPhase, L"QueryEdit: %d requests\n", count);

        CComPtr<ScriptEditQuery> spScriptEditQuery;
        IFFAILRET(ScriptEditQuery::CreateInstance(&spScriptEditQuery));
        IFFAILRET(spScriptEditQuery->Init(this));

        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            hr = spScriptEditQuery->ProcessQuery(requests, count);
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

        if (SUCCEEDED(hr))
        {
            hr = spScriptEditQuery.QueryInterface(ppQueryResult);
        }

        OUTPUT_FLUSH();
        return hr;
    }

    ScriptEditQuery::ScriptEditQuery()
    {
    }

    HRESULT ScriptEditQuery::Init(ScriptEdit* scriptEdit)
    {
        m_scriptEdit = scriptEdit;

        ScriptEngine* scriptEngine = scriptEdit->GetScriptEngine();
        scriptEngine->AddRef();
        m_scriptEngine.Attach(scriptEngine); // Keep the reference

        // Records current baseline generation. Can CommitEdit only if script hasn't changed since this query.
        m_baselineGeneration = scriptEdit->GetBaselineGeneration();

        m_canApply = false;

        return S_OK;
    }

    void ScriptEditQuery::FinalDestruct()
    {
        // This is needed because this object might potentially be released on debugger thread.
        this->m_scriptEngine->DispatchOnApplicationThread([&]{
            EditAllocator* alloc = m_scriptEdit->GetAllocator();

            if (m_scriptDiffs != nullptr)
            {
                m_scriptDiffs->Map([=](int, ScriptDiff* diff)
                {
                    EditDelete(alloc, diff);
                });

                EditDelete(alloc, (ScriptDiffList*)m_scriptDiffs);
                m_scriptDiffs.Unroot(alloc);
            }

            ClearScriptEditActions();

            return S_OK;
        });
    }

    STDMETHODIMP ScriptEditQuery::CanApply(
        /* [out] */ BOOL *pCanApply)
    {
        HRESULT hr = S_OK;

        IfNullReturnError(pCanApply, E_POINTER);
        *pCanApply = m_canApply ? TRUE : FALSE;
        
        return hr;
    }

    STDMETHODIMP ScriptEditQuery::CommitEdit()
    {
        if (!m_canApply)
        {
            // This edit isn't supported. Client shouldn't commit unsupported edit.
            return E_FAIL;
        }

        if (m_baselineGeneration != m_scriptEdit->GetBaselineGeneration())
        {
            // Stale query. Script has already changed. Client shouldn't commit stale edit.
            return E_FAIL;
        }

        return m_scriptEngine->DispatchOnApplicationThread([this]()
        {
            HRESULT hr = S_OK;

            BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
            {
                // Script diff -> Script edit actions
                CreateScriptEditActions();

                // Enum recycler function objects
                ScriptContext* scriptContext = m_scriptEngine->GetScriptContext();
                {
                    ScriptContext::AutoScriptEditEnumFunctions autoEnumerating(scriptContext, this);

                    Recycler* recycler = scriptContext->GetRecycler();
                    recycler->EnumerateObjects(JavascriptLibrary::EnumFunctionClass, RecyclerEnumClassCallback);
                }

                // Commit
                CommitScriptEditActions();
            }
            END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

            ClearScriptEditActions();

            // Prevent stale commits. Even on failure we might have some sources committed.
            m_scriptEdit->IncBaselineGeneration();

            return hr;
        });
    }

    void ScriptEditQuery::CreateScriptEditActions()
    {
        EditAllocator* alloc = m_scriptEdit->GetAllocator();

        Assert(!m_scriptEditActionMap);
        m_scriptEditActionMap.Root(EditNew(alloc, ScriptEditActionMap, alloc), alloc);

        m_scriptDiffs->Map([&](int, ScriptDiff* diff)
        {
            AutoAllocatorObjectPtr<ScriptEditAction, EditAllocator> scriptEditAction(
                EditNew(alloc, ScriptEditAction, diff),
                alloc);
            scriptEditAction->Prepare();

            m_scriptEditActionMap->Add(diff->OldTree().GetUtf8SourceInfo(), scriptEditAction);
            scriptEditAction.Detach();
        });
    }

    void ScriptEditQuery::CommitScriptEditActions()
    {
        if (m_scriptEditActionMap)
        {
            m_scriptEditActionMap->Map([&](Utf8SourceInfo*, ScriptEditAction* scriptEditAction)
            {
                scriptEditAction->Commit();
            });
        }
    }

    void ScriptEditQuery::ClearScriptEditActions()
    {
        if (m_scriptEditActionMap)
        {
            EditAllocator* alloc = m_scriptEdit->GetAllocator();

            m_scriptEditActionMap->Map([&](Utf8SourceInfo*, ScriptEditAction* scriptEditAction)
            {
                EditDelete(alloc, scriptEditAction);
            });

            EditDelete(alloc, (ScriptEditActionMap*)m_scriptEditActionMap);
            m_scriptEditActionMap.Unroot(alloc);
        }
    }

    void ScriptEditQuery::EnumFunction(JavascriptFunction* function)
    {
        Utf8SourceInfo* utf8SourceInfo = function->GetFunctionProxy()->GetUtf8SourceInfo();
        ScriptEditAction* scriptEditAction = m_scriptEditActionMap->Lookup(utf8SourceInfo, /*default*/nullptr);
        if (scriptEditAction)
        {
            scriptEditAction->CheckRecyclerFunction(function);
        }
    }

    void ScriptEditQuery::RecyclerEnumClassCallback(void *address, size_t size)
    {
        // TODO: we are assuming its function because for now we are enumerating only on functions
        // In future if the RecyclerNewEnumClass is used of Recyclable objects or Dynamic object, we would need a check if it is function
        Assert(JavascriptFunction::Is(address));
        JavascriptFunction* function = (JavascriptFunction*)address;

        if (!function->IsScriptFunction())
        {
            return; // We are only interested in script functions for EnC
        }

        ScriptContext* scriptContext = function->GetScriptContext();
        ScriptEditQuery* activeScriptEditQuery = scriptContext->GetActiveScriptEditQuery();
        if (!activeScriptEditQuery)
        {
            return; // Ignore unrelated scriptContexts
        }

        activeScriptEditQuery->EnumFunction(function);
    }

    STDMETHODIMP ScriptEditQuery::GetResultCount(
        /* [out] */ ULONG* count)
    {
        HRESULT hr = S_OK;

        IfNullReturnError(count, E_POINTER);
        *count = static_cast<ULONG>(m_scriptDiffs->Count());

        return hr;
    }

    STDMETHODIMP ScriptEditQuery::GetResult(
        /* [in] */ ULONG index,
        /* [out] */ ScriptEditResult *result)
    {
        HRESULT hr = S_OK;

        IfNullReturnError(result, E_POINTER);
        memset(result, 0, sizeof(ScriptEditResult));

        if (index < static_cast<ULONG>(m_scriptDiffs->Count()))
        {
            ScriptDiff* diff = m_scriptDiffs->Item(index);

            if (diff->OldTree().GetUtf8SourceInfo()->HasDebugDocument())
            {
                ((ScriptDebugDocument*)diff->OldTree().GetUtf8SourceInfo()->GetDebugDocument())->QueryDocumentText(&result->oldDebugDocumentText);
            }
            
            if (diff->NewTree().GetUtf8SourceInfo()->HasDebugDocument())
            {
                ScriptDebugDocument* newTreeScriptDebugDocument = (ScriptDebugDocument*)diff->NewTree().GetUtf8SourceInfo()->GetDebugDocument();

                if (newTreeScriptDebugDocument->HasDocumentText())
                {
                    newTreeScriptDebugDocument->QueryDocumentText(&result->newDebugDocumentText);
                }
            }
            if (diff->NewTree().HasError())
            {
                AutoCOMPtr<ActiveScriptError> activeScriptError;
                Utf8SourceInfo* utf8SourceInfo = diff->NewTree().GetUtf8SourceInfo();
                IfFailGo(ActiveScriptError::CreateCompileError(
                    utf8SourceInfo->GetSrcInfo(), diff->NewTree().GetError(), utf8SourceInfo, &activeScriptError));
                IfFailGo(activeScriptError->GetCompileErrorInfo(&result->message, &result->line, &result->column));
                result->line++; // Convert to 1-based
                result->column++;
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }

    Error:
        return hr;
    }

    HRESULT ScriptEditQuery::ProcessQuery(ScriptEditRequest *requests, ULONG count)
    {
        Assert(!m_scriptDiffs);
        EditAllocator* alloc = m_scriptEdit->GetAllocator();
        m_scriptDiffs.Root(EditNew(alloc, ScriptDiffList, alloc, count), alloc);

        HRESULT hr = S_OK;
        while (count > 0)
        {
            IfFailGo(ProcessQuery(requests[--count]));
        }

        const bool hasError = m_scriptDiffs->MapUntil([](int, ScriptDiff* diff)
        {
            return diff->NewTree().HasError();
        });
                
        m_canApply = !hasError;

    Error:
        return hr;
    }

    HRESULT ScriptEditQuery::ProcessQuery(const ScriptEditRequest& request)
    {
        HRESULT hr = S_OK;
        EditAllocator* alloc = m_scriptEdit->GetAllocator();

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        Trace(request);
#endif
        IDebugDocumentText* debugDocumentText = request.debugDocumentText;
        const ULONG editEnd = request.editTextSpan.start + request.editTextSpan.length;
        if (debugDocumentText == nullptr
            || editEnd < request.editTextSpan.start // overflow
            || (request.newText == nullptr && request.newTextLength > 0) // invalid newText
            || request.newTextLength >= INT_MAX)
        {
            IfFailGo(E_INVALIDARG);
        }

        const bool exists = m_scriptDiffs->MapUntil([=](int, ScriptDiff* diff)
        {
            if (diff->OldTree().GetUtf8SourceInfo()->HasDebugDocument())
            {
                ScriptDebugDocument* scriptDebugDocument = (ScriptDebugDocument*)diff->OldTree().GetUtf8SourceInfo()->GetDebugDocument();
                if (scriptDebugDocument->GetDocumentText() == debugDocumentText)
                {
                    return true;
                }
            }
            return false;
        });
        if (exists)
        {
            IfFailGo(E_INVALIDARG);
        }

        Utf8SourceInfo* requestUtf8SourceInfo = nullptr;
        m_scriptEngine->GetScriptContext()->MapScript([&](Utf8SourceInfo* utf8SourceInfo)
        {
            if (utf8SourceInfo->HasDebugDocument())
            {
                ScriptDebugDocument* scriptDebugDocument = (ScriptDebugDocument*)utf8SourceInfo->GetDebugDocument();
                if (scriptDebugDocument->HasDocumentText() && scriptDebugDocument->GetDocumentText() == debugDocumentText)
                {
                    requestUtf8SourceInfo = utf8SourceInfo;
                }
                //TODO: MapUntil
            }
        });
        if (!requestUtf8SourceInfo)
        {
            IfFailGo(E_INVALIDARG);
        }

        // Old full text length
        ULONG oldLen;
        IfFailGo(debugDocumentText->GetSize(nullptr, &oldLen));
        if (editEnd > oldLen)
        {
            IfFailGo(E_INVALIDARG);
        }

        // New full text length
        ULONG newLen = oldLen - request.editTextSpan.length;
        IfFailGo(ULongAdd(newLen, request.newTextLength, &newLen));

        {
            // Assemble new unicode full text
            AutoAllocatorArrayPtr<WCHAR, ForceLeafAllocator<EditAllocator>::AllocatorType> buffer(
                EditNewArrayLeaf(alloc, WCHAR, newLen), newLen, alloc);
            ULONG cur = 0;

            ULONG len = request.editTextSpan.start;
            IfFailGo(debugDocumentText->GetText(0, buffer + cur, nullptr, nullptr, len));
            cur += len;

            len = request.newTextLength;
            if (len > 0)
            {
                js_wmemcpy_s(buffer + cur, newLen - cur, request.newText, len);
                cur += len;
            }

            len = oldLen - editEnd;
            IfFailGo(debugDocumentText->GetText(editEnd, buffer + cur, nullptr, nullptr, len));
            cur += len;

            Assert(cur == newLen);
            // No need to null terminate. We will null terminate when encoding into utf8.

            IfFailGo(ProcessQuery(request, requestUtf8SourceInfo, buffer, newLen));
            buffer.Detach(); // Unicode source text is transferred to ScriptDiff
        }

    Error:
        return hr;
    }

    HRESULT ScriptEditQuery::ProcessQuery(const ScriptEditRequest& request, Utf8SourceInfo* utf8SourceInfo, _In_reads_(newLen) WCHAR* newFullText, ULONG newLen)
    {
        HRESULT hr = S_OK;
        EditAllocator* alloc = m_scriptEdit->GetAllocator();

        ScriptContext* scriptContext = m_scriptEngine->GetScriptContext();
        AutoAllocatorObjectPtr<ScriptParseTree, EditAllocator> oldTree(
            EditNew(alloc, ScriptParseTree, alloc, utf8SourceInfo, scriptContext),
            alloc);
        AutoAllocatorObjectPtr<ScriptParseTree, EditAllocator> newTree(
            EditNew(alloc, ScriptParseTree, alloc, newFullText, newLen, utf8SourceInfo->GetSrcInfo(), utf8SourceInfo->GetParseFlags(), utf8SourceInfo->GetByteCodeGenerationFlags(), scriptContext),
            alloc);

        AutoAllocatorObjectPtr<ScriptDiff, EditAllocator> diff(ScriptDiff::Diff(m_scriptEdit, oldTree, newTree, newFullText), alloc);
        oldTree.Detach();
        newTree.Detach();

        m_scriptDiffs->Add(diff);
        diff.Detach();

        return hr;
    }

    ScriptParseTree::ScriptParseTree(EditAllocator* alloc, PCWSTR source, ULONG length, const SRCINFO* srcInfo, ULONG parseFlags, ULONG byteCodeGenerationFlags, ScriptContext* scriptContext) :
        m_parser(scriptContext),
        m_hasError(false)
    {
        // Decode newFullText into utf8
        const charcount_t cchSource = length;
        const size_t cbUtf8Buffer = (static_cast<size_t>(cchSource)+1) * 3;
        AutoAllocatorArrayPtr<utf8char_t, ForceLeafAllocator<EditAllocator>::AllocatorType> utf8Buffer(
            EditNewArrayLeaf(alloc, utf8char_t, cbUtf8Buffer), cbUtf8Buffer, alloc);
        const size_t cbSource = utf8::EncodeIntoAndNullTerminate(utf8Buffer, source, cchSource);

        if (!srcInfo)
        {
            srcInfo = scriptContext->GetModuleSrcInfo(kmodGlobal);
        }
        Utf8SourceInfo* sourceInfo = Utf8SourceInfo::New(scriptContext, utf8Buffer, cchSource, cbSource, srcInfo);
        sourceInfo->SetParseFlags(parseFlags);
        sourceInfo->SetByteCodeGenerationFlags(byteCodeGenerationFlags);
        Parse(alloc, sourceInfo);
    }

    ScriptParseTree::ScriptParseTree(EditAllocator* alloc, Utf8SourceInfo* utf8SourceInfo, ScriptContext* scriptContext) :
        m_parser(scriptContext),
        m_hasError(false)
    {
        Parse(alloc, utf8SourceInfo);
    }

    ScriptParseTree::~ScriptParseTree()
    {
        Recycler* recycler = m_parser.GetScriptContext()->GetRecycler();
        m_utf8SourceInfo.Unroot(recycler);
    }

    void ScriptParseTree::Parse(EditAllocator* alloc, Utf8SourceInfo* utf8SourceInfo)
    {
        ScriptContext* scriptContext = m_parser.GetScriptContext();

        HRESULT hrParser = S_OK;
        {
            const SRCINFO* srcInfo = utf8SourceInfo->GetSrcInfo();
            SourceContextInfo* sourceContextInfo = srcInfo->sourceContextInfo;
            ULONG grfscr = utf8SourceInfo->GetParseFlags();
            
            // avoid defer parse
            grfscr &= ~fscrDeferFncParse;
            grfscr |= fscrNoAsmJs;

            m_functionIdBegin = sourceContextInfo->nextLocalFunctionId; // Save begin functionId

            hrParser = m_parser.ParseSourceWithOffset(&m_parseTree,
                utf8SourceInfo->GetSource(), /*offset*/0, utf8SourceInfo->GetCbLength(), /*cchOffset*/0, utf8SourceInfo->GetIsCesu8(),
                grfscr, &m_parseException, &sourceContextInfo->nextLocalFunctionId, /*lineNumber*/0, sourceContextInfo,
                /*functionInfo*/nullptr, /*reparse*/true);
        }

        if (FAILED(hrParser))
        {
            if (hrParser != SCRIPT_E_RECORDED)
            {
                JavascriptError::MapAndThrowError(scriptContext, hrParser);
            }

            this->m_hasError = true;
        }
        else
        {
            FixParentLinks(m_parseTree, alloc); //TODO: Consider setting parent links at parse time
        }

        // Pin utf8SourceInfo while this ScriptParseTree is alive
        Recycler* recycler = scriptContext->GetRecycler();
        m_utf8SourceInfo.Root(utf8SourceInfo, recycler);
    }

    void ScriptParseTree::GenerateByteCode(ScriptContext* scriptContext, _Outptr_ ParseableFunctionInfo** root)
    {
        // This keeps function bodies generated by the byte code alive till we return
        Js::AutoDynamicCodeReference dynamicFunctionReference(scriptContext);

        // Following adds to scriptContext->sourceList. If something goes wrong, we may clear the added entry actively.
        // Since the sourceList is a weakref list, it is ok to let GC clean it up.
        //
        uint sourceIndex = scriptContext->SaveSourceNoCopy(m_utf8SourceInfo, m_utf8SourceInfo->GetCchLength(), m_utf8SourceInfo->GetIsCesu8());
        HRESULT hrCodeGen;
        CompileScriptException se;
        {            
            ULONG grfscr = this->m_utf8SourceInfo->GetByteCodeGenerationFlags();
            hrCodeGen = ::GenerateByteCode(m_parseTree, grfscr, scriptContext, root, sourceIndex, /*forceNoNative*/false, &m_parser, &se);
        }

        if (FAILED(hrCodeGen))
        {
            Assert(hrCodeGen == SCRIPT_E_RECORDED);
            hrCodeGen = se.ei.scode;
            se.Free();
            if (hrCodeGen == VBSERR_OutOfStack)
            {
                JavascriptError::ThrowStackOverflowError(scriptContext);
            }
            JavascriptError::MapAndThrowError(scriptContext, hrCodeGen);
        }
    }

    ScriptDiff::ScriptDiff(ScriptEdit* scriptEdit, ScriptParseTree* oldTree, ScriptParseTree* newTree, PCWSTR newFullText, TreeMatchType* match, EditScriptType* editScript) :
        m_scriptEdit(scriptEdit),
        m_oldTree(oldTree),
        m_newTree(newTree),
        m_newFullText(newFullText),
        m_match(match),
        m_editScript(editScript),
        m_semanticChanges(nullptr)
    {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        TraceAstDiff();
#endif
    }

    void ScriptDiff::AnalyzeSemanticChanges()
    {
        Assert(!m_semanticChanges); // Not already analyzed

        if (m_match) // We have match only if no syntax error
        {
            // Generate the new tree, needed for analyzing closure changes
            ScriptContext* scriptContext = m_scriptEdit->GetScriptContext();
            ParseableFunctionInfo* root = nullptr;
            NewTree().GenerateByteCode(scriptContext, &root);
            m_newRoot.Root(root->GetFunctionBody(), scriptContext->GetRecycler()); // Keep root functionBody alive. Will UnPin in destructor

            EditAllocator* alloc = m_scriptEdit->GetAllocator();
            m_semanticChanges = SemanticChangeAnalyzer::AnalyzeSemanticChanges(alloc, *this);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            TraceSemanticChanges();
#endif
        }
    }

    ScriptDiff::~ScriptDiff()
    {
        EditAllocator* alloc = m_scriptEdit->GetAllocator();
        if (m_newFullText)
        {
            EditDeleteArray(alloc, m_newTree->GetUtf8SourceInfo()->GetCchLength(), m_newFullText);
        }

        EditDelete(alloc, m_oldTree);
        EditDelete(alloc, m_newTree);

        if (m_newRoot)
        {
            m_newRoot.Unroot(m_scriptEdit->GetScriptContext()->GetRecycler());
        }

        if (m_match != nullptr) { EditDelete(alloc, m_match); }
        if (m_editScript != nullptr) { EditDelete(alloc, m_editScript); }
        if (m_semanticChanges != nullptr) { EditDelete(alloc, m_semanticChanges); }
    }

    ScriptDiff* ScriptDiff::Diff(ScriptEdit* scriptEdit, ScriptParseTree* oldTree, ScriptParseTree* newTree, PCWSTR newFullText)
    {
        EditAllocator* alloc = scriptEdit->GetAllocator();
        FunctionTreeComparer<EditAllocator> comparer(alloc);

        ScriptDiff* diff = nullptr;

        AssertMsg(!oldTree->HasError(), "The old tree should not have any parse errors");

        if (newTree->HasError())
        {
            diff = EditNew(alloc, ScriptDiff, scriptEdit, oldTree, newTree, newFullText, /*match*/nullptr, /*editScript*/nullptr);
        }
        else
        {
            AutoAllocatorObjectPtr<TreeMatchType, EditAllocator> match(
                EditNew(alloc, TreeMatchType, alloc, oldTree->GetParseTree(), newTree->GetParseTree(), comparer),
                alloc);

            AutoAllocatorObjectPtr<EditScriptType, EditAllocator> editScript(
                EditNew(alloc, EditScriptType, alloc, *match),
                alloc);

            AutoAllocatorObjectPtr<ScriptDiff, EditAllocator> tmpDiff(
                EditNew(alloc, ScriptDiff, scriptEdit, oldTree, newTree, newFullText, match, editScript), alloc);

            tmpDiff->AnalyzeSemanticChanges(); // May OOM, so only detach after this succeeds

            match.Detach();
            editScript.Detach();
            diff = tmpDiff.Detach();
        }

        return diff;
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void ScriptEditQuery::Trace(const ScriptEditRequest& r) const
    {
        if (Js::Configuration::Global.flags.Trace.IsEnabled(Phase::ENCPhase))
        {
            if (r.debugDocumentText)
            {
                CComBSTR name;
                PCWSTR pName = SUCCEEDED(r.debugDocumentText->GetName(DOCUMENTNAMETYPE_APPNODE, &name)) ? name : L"[GetName failed]";

                WCHAR text[64];
                ULONG maxLen = min(r.editTextSpan.length, static_cast<ULONG>(_countof(text) - 1));
                PCWSTR pText = SUCCEEDED(r.debugDocumentText->GetText(r.editTextSpan.start, text, nullptr, nullptr, maxLen)) ? text : L"[GetText failed]";
                text[maxLen] = L'\0';

                WCHAR newText[64];
                maxLen = min(r.newTextLength, static_cast<ULONG>(_countof(newText) - 1));
                wcsncpy_s(newText, r.newText, maxLen);
                newText[maxLen] = L'\0';

                OUTPUT_TRACE(Phase::ENCPhase, L"%s (%d,%d):\n%s...\n===>\n%s...\n", pName, r.editTextSpan.start, r.editTextSpan.length, pText, newText);
            }
        }
    }

    void ScriptDiff::TraceAstDiff() const
    {
        if (Js::Configuration::Global.flags.Trace.IsEnabled(Phase::ENCPhase))
        {
            if (this->m_editScript != nullptr)
            {
                OUTPUT_TRACE(Phase::ENCPhase, L"=== AST Diffing ===\n");
                LPCUTF8 oldSource = m_oldTree->GetUtf8Source();
                LPCUTF8 newSource = m_newTree->GetUtf8Source();

                // WARNING: Following names must be in sync with enum EditKind (rarely changed)
                static PCWSTR s_editKinds[] = { L"None", L"Update", L"Insert", L"Delete", L"Move", L"Reorder" };

                m_editScript->Edits().Map([&](int, const Edit<ParseNodePtr>& edit)
                {
                    PCWSTR editLabel = s_editKinds[edit.Kind()];
                    switch (edit.Kind())
                    {
                    case EditKind::Delete:
                        Trace(editLabel, edit.OldNode(), oldSource);
                        break;

                    case EditKind::Insert:
                        Trace(editLabel, edit.NewNode(), newSource);
                        break;

                    case EditKind::Update:
                    case EditKind::Move:
                    case EditKind::Reorder:
                        Trace(editLabel, edit.OldNode(), oldSource, edit.NewNode(), newSource);
                        break;

                    default:
                        OUTPUT_TRACE(Phase::ENCPhase, editLabel);
                        break;
                    }
                });
            }
            else 
            {
                OUTPUT_TRACE(Phase::ENCPhase, L"=== Parse error ===\n");
            }
        }
    }

    void ScriptDiff::TraceSemanticChanges() const
    {
        if (Js::Configuration::Global.flags.Trace.IsEnabled(Phase::ENCPhase))
        {
            OUTPUT_TRACE(Phase::ENCPhase, L"=== Semantic Changes ===\n");
            m_semanticChanges->Map([&](int, const SemanticChange& change)
            {
                change.Trace(*this);
            });
        }
    }

    void ScriptDiff::Trace(PCWSTR label, ParseNodePtr pnode, LPCUTF8 source, ParseNodePtr pNewNode, LPCUTF8 newSource) const
    {
        utf8char_t buffer[32];
        GetNodeStr(buffer, _countof(buffer), pnode, source);

        if (pNewNode)
        {
            utf8char_t newBuffer[32];
            GetNodeStr(newBuffer, _countof(newBuffer), pNewNode, newSource);
            OUTPUT_TRACE(Phase::ENCPhase, L"%s: %s %S -> %s %S\n", label,
                GetParseNodeName(pnode), buffer, GetParseNodeName(pNewNode), newBuffer);
        }
        else
        {
            OUTPUT_TRACE(Phase::ENCPhase, L"%s: %s %S\n", label, GetParseNodeName(pnode), buffer);
        }
    }

    void ScriptDiff::GetNodeStr(_Out_writes_z_(maxLen) utf8char_t* buffer, size_t maxLen, ParseNodePtr pnode, LPCUTF8 source)
    {
        maxLen = min(static_cast<size_t>(pnode->ichLim - pnode->ichMin), maxLen - 1);
        js_memcpy_s(buffer, maxLen, source + pnode->ichMin, maxLen);
        buffer[maxLen] = '\0';

        // Replace \r \n with . for easier read
        utf8char_t* p = buffer;
        while (*p)
        {
            if (*p == '\r' || *p == '\n')
            {
                *p = '.';
            }
            p++;
        }
    }

    static PCWSTR const s_parseNodeNames[] = {
#define PTNODE(nop,sn,pc,nk,ok,json) OLESTR(json),
#include "ptlist.h"
#undef PTNODE
        L"INVALID_PARSE_NODE_NAME" // dummy entry to work around intellisense warning
    };

    PCWSTR ScriptDiff::GetParseNodeName(ParseNodePtr pnode)
    {
        return s_parseNodeNames[pnode->nop];
    }
#endif // ENABLE_DEBUG_CONFIG_OPTIONS


    ScriptEditAction::ScriptEditAction(ScriptDiff* diff) :
        m_diff(diff),
        m_editActions(m_diff->GetScriptEdit()->GetAllocator()),
        m_updateFunctionActions(m_diff->GetScriptEdit()->GetAllocator())
    {
    }

    ScriptEditAction::~ScriptEditAction()
    {
    }

    //
    // Prepare edit actions for this source diff.
    //
    void ScriptEditAction::Prepare()
    {
        //
        // Walk semantic changes, prepare edit actions
        //
        m_diff->SemanticChanges().Map([this](int, const SemanticChange& semanticChange)
        {
            ParseNodePtr functionDeclarationNode = nullptr;
            switch (semanticChange.Kind())
            {
            case SemanticChangeKind::InsertFunction:
                functionDeclarationNode = semanticChange.NewNode();
                Assert(functionDeclarationNode);
                Assert(functionDeclarationNode->parent);
                // Nested function are not added to global
                // var funcVar = func() will also not add to global
                if (functionDeclarationNode->parent->nop == knopProg) 
                {
                    if (functionDeclarationNode->sxFnc.pid != nullptr) // Anonymous functions are not added to global
                    {
                        AddAddGlobal(semanticChange);
                    }
                }
                break;
            case SemanticChangeKind::DeleteFunction:
                break;

            case SemanticChangeKind::UpdateFunction:
            case SemanticChangeKind::MoveFunction:
                AddUpdateFunction(semanticChange);
                break;

            default:
                Assert(false);
                break;
            }
        });
    }

    //
    // Create an UpdateFunction action to update a functionBody.
    //
    void ScriptEditAction::AddUpdateFunction(const SemanticChange& semanticChange)
    {
        EditAllocator* alloc = m_diff->GetScriptEdit()->GetAllocator();

        AutoAllocatorObjectPtr<UpdateFunction, EditAllocator> action(
            EditNew(alloc, UpdateFunction, this, semanticChange),
            alloc);
        m_editActions.Add(action);
        UpdateFunction* updateFunction = action.Detach();

        // Register this edit action to receive function object enumeration
        m_updateFunctionActions.Add(semanticChange.OldFunctionBody()->GetLocalFunctionId(), updateFunction);
    }

    //
    // Create an UpdateGlobal action to update global with a new JavascriptFunction.
    //
    void ScriptEditAction::AddAddGlobal(const SemanticChange& semanticChange)
    {
        EditAllocator* alloc = m_diff->GetScriptEdit()->GetAllocator();

        AutoAllocatorObjectPtr<AddGlobal, EditAllocator> action(
            EditNew(alloc, AddGlobal, this, semanticChange),
            alloc);
        m_editActions.Add(action);
        action.Detach();
    }


    //
    // Check a function object to see if we have applicable edit actions, if yes prepare accordingly.
    //
    void ScriptEditAction::CheckRecyclerFunction(JavascriptFunction* function)
    {
        Assert(function->GetFunctionProxy()->GetUtf8SourceInfo() == m_diff->OldTree().GetUtf8SourceInfo());

        UpdateFunction* updateFunction = m_updateFunctionActions.Lookup(function->GetFunctionInfo()->GetLocalFunctionId(), /*default*/nullptr);
        if (updateFunction)
        {
            updateFunction->AddFunction(function);
        }
    }

    //
    // Commit the changes in this source.
    //
    void ScriptEditAction::Commit()
    {
        Utf8SourceInfo* newUtf8SourceInfo = this->m_diff->NewTree().GetUtf8SourceInfo();

        // Update PDM first before making runtime changes. This may fail.
        {
            HRESULT hr = S_OK;
            ScriptEngine* scriptEngine = this->GetScriptEdit()->GetScriptEngine();
            
            CComPtr<CScriptBody> scriptBody;
            scriptBody.Attach(HeapNew(CScriptBody, m_diff->NewRoot(), scriptEngine, newUtf8SourceInfo));

            // TODO: scriptBody life time?
            Assert(scriptEngine->CanRegisterDebugSources());
            if (FAILED(hr = scriptEngine->DbgRegisterScriptBlock(scriptBody)))
            {
                JavascriptError::MapAndThrowError(scriptEngine->GetScriptContext(), hr);
            }
        }

        // Runtime updates, no failure allowed
        BEGIN_NO_EXCEPTION
        {
            // Commit all edit actions in this source
            m_editActions.CommitAll();
        }
        END_NO_EXCEPTION;
    }

    ScriptEditAction::UpdateFunction::UpdateFunction(ScriptEditAction* owner, const SemanticChange& semanticChange) :
        m_ownerScriptEditAction(owner),
        m_semanticChange(semanticChange),
        m_functionList(owner->GetScriptEdit()->GetAllocator())
    {
    }

    void ScriptEditAction::UpdateFunction::AddFunction(JavascriptFunction* function)
    {
        Assert(function->GetFunctionInfo()->GetLocalFunctionId() == m_semanticChange.OldFunctionBody()->GetLocalFunctionId());
        m_functionList.Add(function);
    }

    void ScriptEditAction::UpdateFunction::Commit()
    {
        FunctionBody* newBody = m_ownerScriptEditAction->GetScriptDiff()->NewTree().GetUtf8SourceInfo()->FindFunction(
            m_semanticChange.NewNode()->sxFnc.functionId);
        Assert(newBody);

        // Change these functionObject to use new functionBody
        m_functionList.Map([&](int, JavascriptFunction* function)
        {
            ScriptFunction* scriptFunction = ScriptFunction::FromVar(function);
            FunctionEntryPointInfo* newEntryPointInfo = newBody->GetDefaultFunctionEntryPointInfo();
            scriptFunction->UpdateThunkEntryPoint(newEntryPointInfo, newBody->GetDirectEntryPoint(newEntryPointInfo));
            function->SetFunctionInfo(newBody);

            if (ScriptFunctionWithInlineCache::Is(function))
            {
                ScriptFunctionWithInlineCache::FromVar(function)->ClearInlineCacheOnFunctionObject();
            }
        });
    }

    ScriptEditAction::AddGlobal::AddGlobal(ScriptEditAction* owner, const SemanticChange& semanticChange) :
        m_ownerScriptEditAction(owner),
        m_semanticChange(semanticChange)
    {
    }

    void ScriptEditAction::AddGlobal::Commit()
    {
        ScriptContext* scriptContext = m_ownerScriptEditAction->GetScriptEdit()->GetScriptContext();
        ParseNodePtr functionNode = m_semanticChange.NewNode();

        FunctionBody* newBody = m_ownerScriptEditAction->GetScriptDiff()->NewTree().GetUtf8SourceInfo()->FindFunction(functionNode->sxFnc.functionId);
        Assert(newBody);

        GlobalObject* globalObject = scriptContext->GetGlobalObject();
        Assert(globalObject);

        JavascriptLibrary* library = scriptContext->GetLibrary();
        Assert(library);

        HRESULT hr;
        BEGIN_TRANSLATE_OOM_TO_HRESULT
            BEGIN_JS_RUNTIME_CALL_NOT_SCRIPT(scriptContext)
                PropertyRecord const * propertyRecord;
                scriptContext->GetOrAddPropertyRecord(functionNode->sxFnc.pid->Psz(), functionNode->sxFnc.pid->Cch(), &propertyRecord);
                PropertyId newFunctionPropertyId = propertyRecord->GetPropertyId();
                if (!globalObject->HasProperty(newFunctionPropertyId)) 
                {
                    ScriptFunction* function = library->CreateScriptFunction(newBody);
                    globalObject->SetProperty(newFunctionPropertyId, function, PropertyOperation_None, /* PropertyValueInfo = */ NULL);
                }
            END_JS_RUNTIME_CALL(scriptContext)
        END_TRANSLATE_OOM_TO_HRESULT(hr)
    }
} // namespace Js

#endif

