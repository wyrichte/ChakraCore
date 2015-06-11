//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

#if DBG_DUMP
void PrintPnodeWIndent(ParseNode *pnode, int indentAmt);
#endif

namespace Authoring
{
    namespace Names
    {
        const wchar_t asyncRequests[] = L"_$asyncRequests";
        const wchar_t complete[] = L"complete";
        const wchar_t charset[] = L"charset";
        const wchar_t currentTarget[] = L"currentTarget";
        const wchar_t load[] = L"load";
        const wchar_t OK[] = L"OK";
        const wchar_t onload[] = L"onload";
        const wchar_t onreadystatechange[] = L"onreadystatechange";
        const wchar_t readyState[] = L"readyState";
        const wchar_t status[] = L"status";
        const wchar_t statusText[] = L"statusText";
        const wchar_t src[] = L"src";
        const wchar_t srcElement[] = L"srcElement";
        const wchar_t readystatechange[] = L"readystatechange";
        const wchar_t responseText[] = L"responseText";
        const wchar_t _url[] = L"_$url";
        const wchar_t invokeImmediateSetTimeoutCalls[] = L"_$invokeImmediateSetTimeoutCalls";
        const wchar_t init[] = L"init";
        const wchar_t _asyncRequestList[] = L"_asyncRequestList";
    }

    // Number of code points to read at once when calling IAuthorFileReader::Read
    const int FILE_READ_CODE_POINTS = 1024 * 32;

    TYPE_STATS(AuthoringFileHandle, L"AuthoringFileHandle")

    ScriptContextManager *PhaseReporter::GetScriptContextManager()
    {
        return m_authoringServicesInfo->GetScriptContextManager();
    }

    PhaseReporter *PhaseReporter::GetActiveReporter()
    {
        auto result = GetScriptContextManager()->GetActivePhaseReporter();
        if (!result) return this;
        return result;
    }

    void PhaseReporter::Executing(AuthoringProbe *probe, Js::ScriptContext *scriptContext, bool doReportPhase/* = true*/)
    {
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendExecutionBegin);
        {
            AutoCriticalSection autocs(&hurryCriticalSection);
            m_currentProbe = probe;
            m_executingContext = scriptContext;
        }

        if (doReportPhase)
        {
            // Set the current phase reporter as the active reporter to be used by 
            // deferred parsing and deferred code generation.
            Assert(m_authoringServicesInfo->GetScriptContextManager()->GetActivePhaseReporter() == nullptr);
            m_authoringServicesInfo->GetScriptContextManager()->SetActivePhaseReporter(this);
            Phase(afpExecuting);
        }
    }

    void PhaseReporter::DoneExecuting(bool doReportPhase/* = true*/)
    {
        {
            AutoCriticalSection autocs(&hurryCriticalSection);
            m_currentProbe = nullptr;
            m_executingContext = nullptr;
        }

        if (doReportPhase)
        {
            m_authoringServicesInfo->GetScriptContextManager()->SetActivePhaseReporter(nullptr);
            Phase(afpDormant);
        }

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendExecutionEnd);
    }

    void PhaseReporter::SetExecutionFile(IAuthorFileHandle *handle)
    {
        m_executionFile.Assign(handle);
    }

    IAuthorFileHandle* PhaseReporter::GetExecutionFile()
    {
        return m_executionFile;
    }

    void PhaseReporter::ClearExecutionFile()
    {
        m_executionFile.ReleaseAndNull();
    }

    void AuthoringFileHandle::OnDelete()
    {
        Close();
        SimpleComObjectWithAlloc<IAuthorFileHandle>::OnDelete();
    }

    static int currentFileId = 0;
    int AuthoringFileHandle::NewFileId() 
    { 
        return ++currentFileId; 
    }

    CommentTable* AuthoringFileHandle::GetCommentTable()
    {
        if (m_commentTable == nullptr)
        {
            m_commentTable.Assign(CommentTable::Create(m_factory->GetAuthoringScriptContext()->GetThreadContext()->GetPageAllocator()));
        }

        return m_commentTable;
    }

    void AuthoringFileHandle::ForgetBuffer()
    {
        m_text.ReleaseAndNull();
        m_commentTable.ReleaseAndNull();
    }

    STDMETHODIMP AuthoringFileHandle::FileChanged(int changesLength, AuthorFileChange* changes)
    {
        STDMETHOD_PREFIX;

        if (changesLength)
        {
            ReleasePtr(m_messages);
            InvalidateContexts();
            ForgetBuffer();
        }

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP AuthoringFileHandle::GetMessageSet(IAuthorMessageSet **messages)
    {
        STDMETHOD_PREFIX;

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetMessageSetBegin);

        if (!m_messages)
            m_messages = new MessageSet(m_factory->GetAuthoringScriptContext()->GetThreadContext()->GetPageAllocator());

        *messages = m_messages;
        m_messages->AddRef();

        STDMETHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetMessageSetEnd);
        STDMETHOD_POSTFIX_CLEAN_END;
    }

    STDMETHODIMP AuthoringFileHandle::Close()
    {
        STDMETHOD_PREFIX;

        ReleasePtr(m_messages);
        m_file.ReleaseAndNull();
        InvalidateContexts();
        ForgetBuffer();

        STDMETHOD_POSTFIX;
    }

    STDMETHODIMP AuthoringFileHandle::get_FileId(__out long *fileId)
    {
        STDMETHOD_PREFIX;

        *fileId = (long)(m_fileId);

        STDMETHOD_POSTFIX;
    }

    HRESULT AuthoringFileHandle::FetchText(Js::ScriptContext* scriptContext)
    {
        Assert(scriptContext);

        IAuthorFileReader* reader = nullptr;
        ArenaAllocator tempAlloc(L"ls:temp", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory); 

        METHOD_PREFIX;
        ValidatePtr(m_file, E_FAIL);

        // Read the file from the host.

        // File length in bytes
        long fileLength;
        IfFailGo(m_file->GetLength(&fileLength));
        Validate(fileLength >= 0);

        OUTPUT_TRACE(Js::JSLSPhase, L"AuthoringFileHandle::FetchText : fileLength %d\n", fileLength);

        // Buffer length in code points. Add 1 for the null terminator.
        charcount_t bufferLenChars = fileLength + 1;

        // Allocate a buffer to hold the content of the file.
        auto buffer = String::Alloc(&tempAlloc, bufferLenChars);

        // Allocate a reader to the file for which this is a authoring service.
        IfFailGo(m_file->GetReader(&reader));

        // Code points read 
        long read = 0;
        charcount_t totalRead = 0;
        do
        {
            // Decrement by 1 since bufferLenChars includes the null terminator.
            auto available = bufferLenChars - totalRead - 1;
            IfFailGo(reader->Read(min((charcount_t)FILE_READ_CODE_POINTS, available), &buffer[totalRead], &read));
            Validate(read >= 0);
            totalRead += read;
            Validate(totalRead < bufferLenChars);
        }
        while(read);

        buffer[totalRead] = 0;

        // Save buffer information.
        m_text.Assign(AuthoringFileText::New(scriptContext, buffer, totalRead));

        METHOD_POSTFIX_CLEAN_START;
        if(reader)
        {
            reader->Close();
            ReleasePtr(reader);
        }
        METHOD_POSTFIX_CLEAN_END;
    }

    HRESULT AuthoringFileHandle::UpdateParseTree(PhaseReporter *reporter, Js::ScriptContext *scriptContext, bool enableDeferredParsing, ParseNodeTree &tree, bool isPrimaryFile, __inout uint *sourceIndex)
    {
        METHOD_PREFIX;

        Assert(sourceIndex);

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendParsingBegin);

        ValidatePtr(m_file, E_FAIL);

        if (!m_text || !tree.IsValidFor(scriptContext) || !tree.IsValidFor(m_text))
        {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            Js::HiResTimer timer;
            double start = timer.Now();
#endif
            if (!m_text)
            {
                IfFailGo(FetchText(scriptContext));
            }

            Assert(m_text);
            reporter->Phase(afpParsing);

            // Produce a parse tree for the file.
            auto parser = Anew(Alloc(), Parser, scriptContext);
            ReleasePtr(m_messages);
            m_messages = new MessageSet(m_factory->GetAuthoringScriptContext()->GetThreadContext()->GetPageAllocator());

            ParseNodePtr newNodeTree = nullptr;
            auto srcInfo = scriptContext->GetModuleSrcInfo(kmodGlobal);
            auto clearCommentTableOnError = false;

            // Only ask for comments if we don't already have a comment table.
            // The comment table might be filled in by a previous parse of this file in a different context. The comment table
            // would then already contain valid entries (since it is cleared if the file was modified) and might contain information
            // needed in a different context (such as the location of a field or function comment).
            if (!m_commentTable)
            {
                parser->SetCommentCallback(CommentTable::Add, GetCommentTable());

                // If we defer parse and have to back-out because we ran into an error, clear the comment table back to the empty state
                clearCommentTableOnError = true;
            }

            if (enableDeferredParsing)
            {
                // Deferred parsing does not perform error correction so if we get any errors we need to restart.
                HRESULT parseResult = parser->ParseCesu8Source(&newNodeTree, m_text->Buffer(), m_text->Length(), fscrStmtCompletion | fscrDeferFncParse, NULL, &srcInfo->sourceContextInfo->nextLocalFunctionId, srcInfo->sourceContextInfo);
                if (!SUCCEEDED(parseResult))
                    if ((parseResult > ERRnoMemory && parseResult < MWUNUSED_ENUM) ||  // MWUNUSED_ENUM is an enum value that is defined after the last error in the error enumeration defined in cmperr.h. 
                        (parseResult > VBSERR_None && parseResult < MWUNUSED_rtError)) // MWUNUSED_rtError is an enum value that is defined after the last error in the error enumeration defined in rterror.h. 
                    {
                        // We ran into a parsing error. Disable deferred parsing and parse again.
                        enableDeferredParsing = false;

                        // Free the older parser
                        ADeletePtr(Alloc(), parser);

                        // Create a new parser because the a parser object doesn't handle parsing twice.
                        parser = Anew(Alloc(), Parser, scriptContext);

                        if (clearCommentTableOnError)
                        {
                            m_commentTable.ReleaseAndNull();
                            parser->SetCommentCallback(CommentTable::Add, GetCommentTable());
                        }
                    }
                    else
                        IfFailGo(parseResult);
            }

            // This is checked again because we might have failed to the deferred parsing step and disabled deferred parsing.
            if (!enableDeferredParsing)
            {
                // Setting an error callback enables error recovery.
                parser->SetErrorCallback(authorErrorHandler, m_messages);

                IfFailGo(parser->ParseCesu8Source(&newNodeTree, m_text->Buffer(), m_text->Length(), fscrStmtCompletion, NULL, &srcInfo->sourceContextInfo->nextLocalFunctionId, srcInfo->sourceContextInfo));
            }

            Assert(newNodeTree);

            m_file->StatusChanged(m_messages->Count() != 0 ? (parser->HasUncertainStructure() ? AuthorFileStatus(afsErrors | afsInvalidRegions) : afsErrors) : afsNoMessages);
            tree.Initialize(parser, newNodeTree, m_text, Alloc());  
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            if (isPrimaryFile || CONFIG_FLAG(DumpCommentsFromReferencedFiles))
            {
                if (PHASE_TRACE1(Js::CommentTablePhase))
                {
                    CComBSTR name;
                    this->m_file->GetDisplayName(&name);
                    OUTPUT_TRACE(Js::CommentTablePhase, L"Displaying comments for : %ls\n", name);
                    this->m_commentTable->DumpCommentTable(this->m_text);
                }
            }
#endif
            if (parser->HasSubsumedFunction())
            {
                HoistSubsumedFunctions::Apply(Alloc(), &tree);
            }

            if(!m_transient)
            {
                // Important: Calling SaveSourceNoCopy() implies that m_buffer will live longer than scriptContext. If this is no longer the case,
                // this code must be changed to use SaveSourceCopy().
                *sourceIndex = scriptContext->SaveSourceNoCopy(m_text->GetSourceInfoForScriptContext(scriptContext), m_text->Length(), true);

                // Ensure the source file Utf8SourceInfo will not be collected. We call RecordCopyOnWrite because it adds the pointer as a key to a page 
                // that is in the recycler.
                auto scriptList = scriptContext->GetSourceList();
                auto sourceInfo = scriptList->Item(*sourceIndex)->Get();
                if (sourceInfo)
                    scriptContext->RecordCopyOnWrite((Js::RecyclableObject *)sourceInfo, (Js::RecyclableObject *)sourceInfo);
            }
            else
            {
                *sourceIndex = MAXUINT;
            }
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            double diff = timer.Now() - start;
            if (diff > 2.0) // greater then 2.0 msec should make sense
            {
                OUTPUT_TRACE(Js::JSLSStatsPhase, L"AuthoringFileHandle::UpdateParseTree  enableDeferredParsing : %ls, primary file : %ls, time spent %8.3f\n",
                    enableDeferredParsing ? L"yes" : L"no", isPrimaryFile ? L"yes" : L"no", diff);
            }
#endif
        }

        METHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendParsingEnd);
        METHOD_POSTFIX_CLEAN_END;
    }

    HRESULT AuthoringFileHandle::ApplyCommentsRewrite(PageAllocator* pageAlloc, ParseNodeTree *tree, long cursorPos) 
    {
        METHOD_PREFIX;

        AssertMsg(tree->IsValidFor(m_text), "Mismatch between file text and parse tree");

        if (!m_disableRewrite && !tree->IsMutated(ParseNodeTree::mutDocComments))
        {
            typedef ParseNodeVisitor<VisitorComposer<TL2(LevelLimiter, DocCommentsRewrite)> > RewritingVisitor;

            CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendApplyCommentsBegin);

            // Initialize the context for the composed visitor
            LevelLimiterContext levelLimiterContext;
            ArenaAllocator localAlloc(L"ls:DocCommentsRewrite", pageAlloc, Js::Throw::OutOfMemory);
            RewriteContext rewriteContext(&localAlloc, tree, this, this->FileId(), cursorPos);

            RewritingVisitor::VisitorContext context;
            context.h = &levelLimiterContext;
            context.t.h = &rewriteContext;

            RewritingVisitor visitor;
            visitor.Visit(tree->TreeRoot(), context);

            if (levelLimiterContext.treeTruncated)
                 tree->SetMutated(ParseNodeTree::mutLevelTruncation);
            tree->SetMutated(ParseNodeTree::mutDocComments);

            CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendApplyCommentsEnd);
        }

        METHOD_POSTFIX;
    }

    struct SuppressThrows : VisitorPolicyBase<bool>
    {
    protected:
        SuppressThrows() { }

        bool Preorder(ParseNode* pnode, bool active)
        {
            if (active && pnode->nop == knopThrow)
            {
                if (pnode->sxUni.pnode1)
                {
                    // Convert this to a unary plus instead of a throw
                    pnode->nop = knopPos;
                }
                else
                    pnode->nop = knopEmpty;
            }
            return true;
        }
    };

    HRESULT AuthoringFileHandle::AddGuardsAndApplyComments(Js::ScriptContext *scriptContext, ParseNodeTree *tree, bool isPrimaryFile, long cursorPos, bool suppressThrows)
    {
        METHOD_PREFIX;

        AssertMsg(tree->IsValidFor(m_text), "Mismatch between file text and parse tree");
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        Js::HiResTimer timer;
        double start = timer.Now();
#endif
        if (!m_disableRewrite)
        {
            // Rewrite the Doc Comments
            ApplyCommentsRewrite(scriptContext->GetThreadContext()->GetPageAllocator(), tree, cursorPos);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            double diff = timer.Now() - start;
            if (diff > 2.0) // greater then 2.0 msec should make sense
            {
                OUTPUT_TRACE(Js::JSLSStatsPhase, L"AuthoringFileHandle::AddGuardsAndApplyComments  - just ApplyCommentsRewrite time %8.3f for total comments : %d\n", timer.Now() - start, GetCommentTable()->Count());
            }
#endif

            // The following visitor will simultaneously execute CleanTree and AddLoopGuards.
            CleanTreeContext cleanContext = { tree->LanguageServiceExtension() };
            if (IsDisableRewrite())
            {
                // Just clean the tree for internal files.
                typedef ParseNodeVisitor<CleanTree> RewritingVisitor;

                RewritingVisitor visitor;
                visitor.Visit(tree->TreeRoot(), &cleanContext);
            }
            else
            {
                // The following visitor will simultaneously execute CleanTree and AddLoopGuards.
#ifdef DEBUG
                // TreeValidator was not previously added correctly. The following line is correct but now it reports errors in unit tests. Tracked by DEVDIV2:324908
                // typedef ParseNodeVisitor<VisitorComposer<TL4(CleanTree, AddLoopGuards, SuppressThrows, TreeValidator)> > RewritingVisitor;
                typedef ParseNodeVisitor<VisitorComposer<TL3(CleanTree, AddLoopGuards, SuppressThrows)> > RewritingVisitor;
#else
                typedef ParseNodeVisitor<VisitorComposer<TL3(CleanTree, AddLoopGuards, SuppressThrows)> > RewritingVisitor;
#endif

                // Initialize the context for the composed visitor
                RewritingVisitor::VisitorContext context;
                context.h = &cleanContext;
                context.t.h = tree->GetParser();
                context.t.t.h = suppressThrows;

                RewritingVisitor visitor;
                visitor.Visit(tree->TreeRoot(), context);
            }

            tree->SetMutated((ParseNodeTree::MutationKind)(ParseNodeTree::mutDocComments | ParseNodeTree::mutLoopGuards));
            tree->SetCleaned(); // record the tree was cleaned.
#if DBG_DUMP
            if (PHASE_TRACE1(Js::DocCommentParseTreePhase))
            {
                if (isPrimaryFile)
                {
                    PrintPnodeWIndent(tree->TreeRoot(), 0);
                }
            }
#endif
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        double diff = timer.Now() - start;
        if (diff > 2.0) // greater then 2.0 msec should make sense
        {
            OUTPUT_TRACE(Js::JSLSStatsPhase, L"AuthoringFileHandle::AddGuardsAndApplyComments  time %8.3f\n", timer.Now() - start);
        }
#endif
        METHOD_POSTFIX;
    }

    void AuthoringFileHandle::ProcessDeferredParseTree(Js::ScriptContext *scriptContext, PhaseReporter *reporter, Parser *parser, ParseNodePtr pnodeProg, bool isPrimaryFile)
    {
       if (reporter) reporter->Phase(afpPreparing);

        ParseNodeTree tree(Alloc());
        tree.Initialize(parser, pnodeProg, m_text, Alloc());
        m_inDeferredParsing = true;
        AddGuardsAndApplyComments(scriptContext, &tree, isPrimaryFile);
        m_inDeferredParsing = false;
        tree.ReleaseParser();
    }

    void AuthoringFileHandle::ProcessEvalTree(Js::ScriptContext *scriptContext, PhaseReporter *reporter, Parser *parser, ParseNodePtr pnodeProg)
    {
        if (reporter) reporter->Phase(afpPreparing);

        ParseNodeVisitor<AddLoopGuards> visitor;
        visitor.Visit(pnodeProg, parser);        
    }

    HRESULT AuthoringFileHandle::GenerateBody(PhaseReporter *reporter, ParseNodeTree *tree, Js::ScriptContext *scriptContext, uint sourceIndex, Js::FunctionBody **result)
    {
        METHOD_PREFIX;

        AssertMsg(tree->IsValidFor(m_text), "Mismatch between file text and parse tree");

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendByteCodeGenerationBegin);

        Js::AutoDynamicCodeReference dynamicFunctionReference(scriptContext);
        CompileScriptException se;

        ValidatePtr(m_file, E_FAIL);
        ValidateArg(result);

        if (tree->IsByteCodeGenerated())
        {
            // Clean the tree from a previous generation
            CleanTreeContext context = { tree->LanguageServiceExtension() };
            ParseNodeVisitor<CleanTree> visitor;
            visitor.Visit(tree->TreeRoot(), &context);
            tree->SetCleaned();
        }

        // Generate the function
        scriptContext->SetInDebugMode();
        scriptContext->GetThreadContext()->Diagnostics->languageServiceEnabled = true;

        reporter->Phase(afpPreparing);
        *result = NULL;
        Js::ParseableFunctionInfo * func = null;
        IfFailGo(GenerateByteCode(tree->TreeRoot(), fscrGlobalCode, scriptContext, &func, sourceIndex, true, tree->GetParser(), &se));        
        *result = func->GetFunctionBody();
        tree->SetByteCodeGenerated();

        METHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendByteCodeGenerationEnd);
        METHOD_POSTFIX_CLEAN_END;
    }

    HRESULT AuthoringFileHandle::Apply(PhaseReporter *reporter, Js::ScriptContext *scriptContext, ScriptContextPath *owner, ScriptContextPath *parent, bool isPrimaryFile, ParseNodeTree *existingTree, uint fileIndex)
    {
        ArenaAllocator tempAlloc(L"ls:temp", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory); 
        ParseNodeTree localTree(Alloc());
        ParseNodeTree *tree = existingTree;

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        Js::HiResTimer timer;
        double start = timer.Now();
        double bytecodetime = 0;
        {
            CComBSTR name;
            this->m_file->GetDisplayName(&name);
            OUTPUT_TRACE(Js::JSLSPhase, L"AuthoringFileHandle::Apply %ls, started @ absolute time %10.3f\n", name, start);
        }
#endif

        METHOD_PREFIX;

        Js::FunctionBody *body = NULL;

        ValidatePtr(m_file, E_FAIL);

        Assert(owner);

        uint sourceIndex = 0;

        if (reporter) reporter->SetExecutionFile(this);

        if (tree)
        {
            AssertMsg(tree->IsValidFor(m_text), "Mismatch between file text and parse tree");
            sourceIndex = fileIndex;
        }
        else
        {
            tree = &localTree;
            IfFailGo(UpdateParseTree(reporter, scriptContext, /* enableDeferParsing = */true, *tree, isPrimaryFile, &sourceIndex));
        }

        IfFailGo(reporter->WasAbortCalled() ? E_ABORT : S_OK);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

        start = timer.Now();
#endif

        IfFailGo(AddGuardsAndApplyComments(scriptContext, tree, isPrimaryFile));
        IfFailGo(reporter->WasAbortCalled() ? E_ABORT : S_OK);
        IfFailGo(GenerateBody(reporter, tree, scriptContext, sourceIndex, &body));
        IfFailGo(reporter->WasAbortCalled() ? E_ABORT : S_OK);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        bytecodetime = timer.Now() - start;
        start = timer.Now();
#endif

        {
            Assert(!LoopGuardsHelpers::LoopGuardsEnabled(scriptContext));

            // Install a probe to ignore exceptions
            AuthoringProbe authorProbe(0, body, true);
            scriptContext->diagProbesContainer.AddProbe(&authorProbe);
            scriptContext->diagProbesContainer.InitializeInlineBreakEngine(&authorProbe);
            if (parent)
                parent->InstallInlineBreakpointProbes(&authorProbe);

            // Apply the function by executing the global function.
            Js::JavascriptFunction *jsFunc = scriptContext->GetLibrary()->CreateScriptFunction(body);

            // Call the root function and run to the break-point if hit.
            Js::Var args[1];
            args[0] = scriptContext->GetGlobalObject();
            Js::Arguments jsArguments(0, args);
            Js::Var varResult;
            {
                AutoPhaseReporterExecuting autoPhaseReporterExecuting(reporter, &authorProbe, scriptContext);
                try
                {
                    // Intentionally ignore failures to execute. If we run into errors just get as far as we can.
                    // This mimics the behavior of loading broken file in IE but we will even try with files that
                    // IE will not load (such as files containing parse errors) because at design time they might
                    // contain information the user would expect in intellisense.
                    CallRootFunction(jsFunc, jsArguments, &varResult);
                }
                catch (ExecutionStop)
                {
                    if (authorProbe.AsyncBreakTriggered())
                    {
                        owner->m_contextFileHalted = true;
                    }
                }
            }

            OUTPUT_TRACE(Js::JSLSStatsPhase, L"AuthoringFileHandle::Apply bytecodegen time %8.3f and execution time %8.3f\n", bytecodetime, timer.Now() - start);

            // Although rare it is possible for an async breakpoint to be requested between the time the execution stopped 
            // and the exception was handled. This ensures that an async-break is not left over in this case.
            authorProbe.ClearAsyncBreak(scriptContext);

            // Uninstall the probe
            scriptContext->diagProbesContainer.RemoveProbe(&authorProbe);
            scriptContext->diagProbesContainer.UninstallInlineBreakpointProbe(&authorProbe);
            if (parent)
                parent->UninstallInlineBreakpointProbes(&authorProbe);

            Assert(!LoopGuardsHelpers::LoopGuardsEnabled(scriptContext));
        }

        // Invoke any immediate SetTimeout calls added by this file
        InvokeSetTimeoutCalls(reporter, scriptContext);

        // Apply OnLoad events for any async script requests this file may have requested
        owner->ClearDependentAsyncRequest();
        IfFailGo(ApplyAsyncRequests(&tempAlloc, reporter, scriptContext, owner));

#ifdef DEBUG
#ifdef VERBOSE_LOGGING
        Output::Print(L"Apply instruction executed: %d\n", InstructionsExecuted(scriptContext));
#endif
#endif

        METHOD_POSTFIX_CLEAN_START;
        if (reporter) reporter->ClearExecutionFile();
        METHOD_POSTFIX_CLEAN_END;
    }

    bool AuthoringFileHandle::IsSourceAtIndex(Js::ScriptContext *scriptContext, uint sourceIndex)
    {
        if (m_text && sourceIndex < scriptContext->SourceCount())
        {
            Js::Utf8SourceInfo *sourceInfo = scriptContext->GetSource(sourceIndex);
            return sourceInfo && sourceInfo->GetSource(L"AuthoringFileHandle::IsSourceAtIndex") == m_text->Buffer();
        }
        return false;
    }

    bool AuthoringFileHandle::IsSourceMatched(Js::Utf8SourceInfo * sourceInfo)
    {
        return m_text && sourceInfo && sourceInfo->GetSource(L"AuthoringFileHandle::IsSourceAtIndex") == m_text->Buffer();
    }

    void AuthoringFileHandle::InvalidateContexts()
    {
        ScriptContextPath *current = m_scriptContextPaths;

        while (current)
        {
            current->Invalidate();
            current = current->m_invalidationLink;
        }
    }

    void AuthoringFileHandle::AddContext(ScriptContextPath *scriptContextPath)
    {
        scriptContextPath->m_invalidationLink = m_scriptContextPaths;
        m_scriptContextPaths = scriptContextPath;
    }

    void AuthoringFileHandle::RemoveContext(ScriptContextPath *scriptContextPath)
    {
        ScriptContextPath **pcurrent = &m_scriptContextPaths;
        while (*pcurrent)
        {
            if (*pcurrent == scriptContextPath)
            {
                *pcurrent = scriptContextPath->m_invalidationLink;
                break;
            }
            else
                pcurrent = &(*pcurrent)->m_invalidationLink;
        }
    }

    HRESULT AuthoringFileHandle::EnsureAsyncRequests(PhaseReporter *reporter, AsyncRequestList *dependentAsyncRequestList)
    {
        STDMETHOD_PREFIX;

        Assert(dependentAsyncRequestList);

        auto dependentAsyncRequests = dependentAsyncRequestList->Items();
        Assert(dependentAsyncRequests);

        bool missingFiles = false;

        for (int i = 0, len = dependentAsyncRequests->Count(); i < len; ++i)
        {
            AuthorFileAsynchronousScriptStatus status;
            auto request = dependentAsyncRequests->Item(i);
            OUTPUT_TRACE(Js::JSLSPhase, L"EnsureAsyncRequests : Initiating VerifySciptInContext for url %ls\n", request.requestSource);

            IfFailGo(reporter->FileContext()->VerifyScriptInContext(CComBSTR(request.requestSource), CComBSTR(request.requestCharSet),CComBSTR(request.requestType), this, VARIANT_FALSE, &status));
            OUTPUT_TRACE(Js::JSLSPhase, L"EnsureAsyncRequests : VerifySciptInContext completed, status : %ls\n", status == afassAdded ? L"afassAdded" : (status == afassLoaded ? L"afassLoaded" : L"afassIgnored"));

            if (status == afassAdded)
                missingFiles = true;
        }

        if (missingFiles)
            hr = hresNonQuiescentContext;

        STDMETHOD_POSTFIX;
    }

    HRESULT AuthoringFileHandle::GetAsyncRequests(ArenaAllocator* alloc, PhaseReporter *reporter, Js::ScriptContext *scriptContext, ScriptContextPath *owner, JsUtil::List<Js::RecyclableObject*, ArenaAllocator>* loadedScripts)
    {
        STDMETHOD_PREFIX;

        Assert(alloc);
        Assert(reporter);
        Assert(scriptContext);
        Assert(loadedScripts);

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetAsyncRequestsBegin);

        Js::RecyclableObject* asyncRequests = JsHelpers::GetProperty<Js::RecyclableObject*>(scriptContext->GetGlobalObject(), Names::asyncRequests, alloc, scriptContext);
        Js::JavascriptArray* asyncRequestList = nullptr;
        if (asyncRequests)
            asyncRequestList = JsHelpers::GetProperty<Js::JavascriptArray*>(asyncRequests, Names::_asyncRequestList, alloc, scriptContext, true, true);

        if (asyncRequestList)
        { 
            bool missingFiles = false;

            for (uint32 i = 0 ; i < asyncRequestList->GetLength(); i++)
            {
                if (!asyncRequestList->HasItem(i))
                {
                    continue;
                }

                Js::RecyclableObject* asyncRequest = JsHelpers::GetArrayElement<Js::RecyclableObject*>(asyncRequestList, i, alloc);
                if (asyncRequest)
                {
                    LPCWSTR requestSource = JsHelpers::GetProperty<LPCWSTR>(asyncRequest, Names::src, alloc, scriptContext, true, true);

                    // skip tags with no source or text attributes
                    if (requestSource == nullptr || *requestSource == NULL)
                    {
                        continue;
                    }

                    LPCWSTR requestCharset = JsHelpers::GetProperty<LPCWSTR>(asyncRequest, Names::charset, alloc, scriptContext, true, true);
                    LPCWSTR requestType = JsHelpers::GetProperty<LPCWSTR>(asyncRequest, Names::type, alloc, scriptContext, true, true);

                    // only require the async request to be inserted before this file if we have an event to execute when the script is loaded
                    // otherwise, just adding it to the context list is sufficient.
                    bool insertBeforeSourceFile = JsHelpers::GetProperty<Js::JavascriptFunction*>(asyncRequest, Names::onload, alloc, scriptContext) || JsHelpers::GetProperty<Js::JavascriptFunction*>(asyncRequest, Names::onreadystatechange, alloc, scriptContext);

                    // call the host for each script found, to find if it is in the context or not
                    AuthorFileAsynchronousScriptStatus status;
                    OUTPUT_TRACE(Js::JSLSPhase, L"GetAsyncRequests : Initiating VerifySciptInContext for url %ls\n", requestSource);

                    HRESULT hres = reporter->FileContext()->VerifyScriptInContext(CComBSTR(requestSource), CComBSTR(requestCharset), CComBSTR(requestType), this, insertBeforeSourceFile ? VARIANT_TRUE : VARIANT_FALSE, &status);
                    OUTPUT_TRACE(Js::JSLSPhase, L"GetAsyncRequests : VerifySciptInContext completed, status : %ls\n", status == afassAdded ? L"afassAdded" : (status == afassLoaded ? L"afassLoaded" : L"afassIgnored"));

                    IfFailGo(hres);

                    if (status == AuthorFileAsynchronousScriptStatus::afassLoaded)
                    {
                        if (insertBeforeSourceFile)
                        {
                            // add the script to the list of loaded scripts to be processed later
                            loadedScripts->Add(asyncRequest);
                        }
                        else if (owner)
                        {
                            // if the requested script is after this file, let the owner scriptcontext know it has a dependent file
                            AsyncRequest request (requestSource, requestCharset, requestType);
                            owner->RecordDependentAsyncRequest(&request);
                        }
                    }
                    else if (status == AuthorFileAsynchronousScriptStatus::afassIgnored)
                    {
                        // skip this file
                        continue;
                    }
                    else
                    {
                        missingFiles = true;
                    }
                }
            }

            // call _$asyncRequests.init() to clear the list
            CallFunctionIgnoreExceptions(Names::init, asyncRequests, scriptContext, reporter);

            // check if any missing files were found
            if (missingFiles)
            {
                hr = hresNonQuiescentContext;
            }
        }

        STDMETHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetAsyncRequestsEnd);
        STDMETHOD_POSTFIX_CLEAN_END;
    }

    HRESULT AuthoringFileHandle::ExecuteAsyncRequests(ArenaAllocator* alloc, PhaseReporter *reporter, Js::ScriptContext *scriptContext, ScriptContextPath *owner, JsUtil::List<Js::RecyclableObject*, ArenaAllocator>* loadedScripts)
    {
        STDMETHOD_PREFIX;

        Assert(alloc);
        Assert(reporter);
        Assert(scriptContext);
        Assert(loadedScripts);

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendExecuteAsyncRequestsBegin);

        bool missingFiles = false;

        for(int i = 0; i < loadedScripts->Count(); i++)
        {
            Js::RecyclableObject* asyncRequest = loadedScripts->Item(i);

            Assert(asyncRequest);

            // set the request properties indicating a successful load
            JsHelpers::SetField(asyncRequest, Names::readyState, Names::complete, scriptContext);

            // find the event functions
            Js::JavascriptFunction* requestOnLoadFunction = JsHelpers::GetProperty<Js::JavascriptFunction*>(asyncRequest, Names::onload, alloc, scriptContext);
            Js::JavascriptFunction* requestOnReadyStateChangeFunction = JsHelpers::GetProperty<Js::JavascriptFunction*>(asyncRequest, Names::onreadystatechange, alloc, scriptContext);

            // execute the onLoad function(s)
            if (requestOnLoadFunction || requestOnReadyStateChangeFunction)
            {
                // create the event object used to call the events
                auto eventObject = JsHelpers::CreateObject(scriptContext);
                JsHelpers::SetField(eventObject, Names::srcElement, asyncRequest, scriptContext);
                JsHelpers::SetField(eventObject, Names::currentTarget, asyncRequest, scriptContext);

                // execute the onLoad function(s)
                if (requestOnLoadFunction)
                {
                    JsHelpers::SetField(eventObject, Names::type, Names::load, scriptContext);
                    CallFunctionIgnoreExceptions(requestOnLoadFunction, asyncRequest, scriptContext, reporter, Convert::ToVar(eventObject, scriptContext));
                }

                if (requestOnReadyStateChangeFunction && requestOnReadyStateChangeFunction != requestOnLoadFunction)
                {
                    JsHelpers::SetField(eventObject, Names::type, Names::readystatechange, scriptContext);
                    CallFunctionIgnoreExceptions(requestOnReadyStateChangeFunction, asyncRequest, scriptContext, reporter, Convert::ToVar(eventObject, scriptContext));
                }
            }

            // apply any async requests that resulted from onload events
            HRESULT hres = ApplyAsyncRequests(alloc, reporter, scriptContext, owner);
            if (hres == hresNonQuiescentContext)
            {
                missingFiles = true;
                continue;
            }
            else
            {
                IfFailGo(hres);
            }
        }

        // check if any missing files were found
        if (missingFiles)
        {
            hr = hresNonQuiescentContext;
        }

        STDMETHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendExecuteAsyncRequestsEnd);
        STDMETHOD_POSTFIX_CLEAN_END;
    }

    HRESULT AuthoringFileHandle::ApplyAsyncRequests(ArenaAllocator* alloc, PhaseReporter *reporter, Js::ScriptContext *scriptContext, ScriptContextPath *owner)
    {
        STDMETHOD_PREFIX;

        Assert(alloc);
        Assert(reporter);
        Assert(scriptContext);

        bool missingFiles = false;
        JsUtil::List<Js::RecyclableObject*, ArenaAllocator>* loadedScripts = JsUtil::List<Js::RecyclableObject*, ArenaAllocator>::New(alloc);

        hr = GetAsyncRequests(alloc, reporter, scriptContext, owner, loadedScripts);

        if (hr == hresNonQuiescentContext)
        {
            // some files were missing, keep going as we may find out we need more files
            missingFiles = true;
        }
        else
        {
            // unexpected failure occured
            IfFailGo(hr);
        }

        if (loadedScripts->Count() > 0)
        {
            hr = ExecuteAsyncRequests(alloc, reporter, scriptContext, owner, loadedScripts);
            if (hr == hresNonQuiescentContext)
            {
                // some files were missing, keep going as we may find out we need more files
                missingFiles = true;
            }
            else
            {
                // unexpected failure occured
                IfFailGo(hr);
            }
        }

        // check if any missing files were found
        if (missingFiles)
        {
            hr = hresNonQuiescentContext;
        }

        STDMETHOD_POSTFIX;
    }

    bool AuthoringFileHandle::RememberNodeCommentsImpl(ArenaAllocator* alloc, int searchLimOverride, ParseNode* node, ParseNode* parentNode, ParseNode* previousNode, bool searchPreviousNode, RememberNodeCommentsCallback& handler)
    {
        bool result = false;
        Assert(node != nullptr || (searchLimOverride != -1 && searchPreviousNode == false));

        if (parentNode != nullptr)
        {
            if (searchPreviousNode)
            {
                Assert(previousNode == nullptr);
                previousNode = GetPreviousNode(parentNode, node);
            }

            auto from = previousNode ? ActualLim(previousNode) : -1;
            if (from == -1)
            {
                // knopProg node->ichMin is the offset of the first child node, not including comments that can appear before that node.
                // In case of non-deferred parsing we adjust from to 0 to include the comments. 
                // In case of deferred parsing we shouldn't do it because knopProg only includes a subrange of a file with the function being parsed. 
                if (parentNode->nop == knopProg && !m_inDeferredParsing)
                {
                    from = 0;
                }
                else
                {
                    from = parentNode->ichMin;
                }
            }

            auto to = (searchLimOverride == -1 ? node->ichMin : (charcount_t)searchLimOverride);

            result = GetCommentTable()->AssociateCommentBefore(from, to, [&](CommentTable* commentTable, int commentTableIndex)
            {
                handler.run(commentTable, commentTableIndex);
            });
        }

        return result;
    }

    CommentBuffer* AuthoringFileHandle::GetLastRememberedComments(ArenaAllocator *alloc, CommentType commentType)
    {
        return GetCommentTable()->GetLastAssociatedComment(alloc, m_text, commentType);
    }

    CommentBuffer* AuthoringFileHandle::GetNodeComments(ArenaAllocator* alloc, charcount_t ichLocation, CommentType commentType)
    {
        return GetCommentTable()->GetCommentsBefore(alloc, m_text, ichLocation, commentType);
    }

    CommentBuffer* AuthoringFileHandle::GetFunctionComments(ArenaAllocator* alloc, charcount_t functionStartPosition, charcount_t lcurlyPosition, charcount_t firstStatementPosition, charcount_t functionEndPosition, CommentType commentType)
    {
        CommentBuffer* result;
        if (commentType == CommentType::commenttypeAnyDoc)
        {
            CommentBuffer* vsdoc = GetCommentTable()->GetFirstCommentBetween(alloc, m_text, lcurlyPosition, firstStatementPosition, CommentType::commenttypeVSDoc);
            if (vsdoc->GetBuffer() != nullptr)
            {
                // VSDoc takes priority to be backward compatible for any existent caller that expect the VSDoc, if it is written.
                return vsdoc;
            }

            // JSDoc related to function declaration should be coded before the function declaration.
            return GetCommentTable()->GetCommentsBefore(alloc, m_text, functionStartPosition, CommentType::commenttypeJSDoc);
        }
        else
        {
            return GetCommentTable()->GetFirstCommentBetween(alloc, m_text, lcurlyPosition, firstStatementPosition, commentType);
        }

        return result;
    }

    CommentBuffer* AuthoringFileHandle::GetFunctionComments(ArenaAllocator* alloc, Js::ScriptContext *scriptContext, Js::JavascriptFunction *function, CommentType commentType)
    {   
        charcount_t functionStartPosition;
        charcount_t lcurlyPosition;
        charcount_t firstStatementPosition;
        charcount_t  functionEndPosition;
        ForEachArgument(alloc, scriptContext, function, [](ParseNodePtr node, bool isRest) {}, &functionStartPosition, &lcurlyPosition, &firstStatementPosition, &functionEndPosition);

#ifdef DEBUG
        if (lcurlyPosition > 0)
        {
            // This code runs under debug, so we can add a safety check to ensure the function is deserialized
            Assert(function->GetFunctionProxy());
            auto body = function->GetFunctionProxy()->EnsureDeserialized();
            auto min = body->StartInDocument();
            Assert(lcurlyPosition >= min);
            auto lim = min + body->LengthInChars();
            Assert(firstStatementPosition <= lim);
        }
#endif

        return this->GetFunctionComments(alloc, functionStartPosition, lcurlyPosition, firstStatementPosition, functionEndPosition, commentType);
    }

    CommentBuffer* AuthoringFileHandle::GetFunctionComments(ArenaAllocator *alloc, ParseNodePtr pnodeFnc, LanguageServiceExtension *extensions, CommentType commentType)
    {   
        Assert(pnodeFnc && pnodeFnc->nop == knopFncDecl);

        auto body = pnodeFnc->sxFnc.pnodeBody;
        if (body)
        {
            auto min = extensions->LCurly(pnodeFnc);
            // Due to error correction, the function might not have a LCurly location.
            if (!min) min = pnodeFnc->ichMin;

            return this->GetFunctionComments(alloc, /* functionStartPosition = */ pnodeFnc->ichMin, /* lcurlyPosition = */ min, /* firstStatementPosition = */ body->ichMin, /* functionEndPosition = */ pnodeFnc->ichLim, commentType);
        }

        // Empty result
        return nullptr;
    }

    void ignoreErrorHandler(void *data, charcount_t position, charcount_t length, HRESULT errorCode) { }

    ParseNode* AuthoringFileHandle::ParseFunctionHeader(__in Parser *parser, __in Js::ScriptContext *scriptContext, __in Js::JavascriptFunction *function)
    {
        Assert(parser);
        Assert(scriptContext);
        Assert(function);

        ParseNode* tree = nullptr;
        Js::ParseableFunctionInfo* info = function->GetFunctionInfo()->GetParseableFunctionInfo();

        if (info)
        {
            auto isCesu8 = info->GetUtf8SourceInfo()->IsCesu8();
            auto offset = (info->IsDeferredParseFunction() ? info->StartOffset() : info->GetFunctionBody()->StartOffset());
            auto charOffset = info->StartInDocument();
            auto length = info->LengthInBytes();
            auto pszStart = info->GetStartOfDocument(L"AuthoringFileHandle::ParseFunctionHeader");
            CompileScriptException se;
            uint nextFunctionId = info->GetLocalFunctionId();
            ULONG flags = fscrDeferredFnc | fscrFunctionHeaderOnly | fscrStmtCompletion 
                | (info->GetIsClassMember() ? fscrDeferredClassMemberFnc : fscrNil);

            // Ensure errors are ignored
            parser->SetErrorCallback(ignoreErrorHandler, nullptr);

            // Parse just the function name and argument list. fscrDeferredFnc informs the parser that we are starting in potentially a nested function
            // and fscrFunctionHeaderOnly tells the parser to stop after the first function argument list.
            parser->ParseSourceWithOffset(&tree, pszStart, offset, length, charOffset, isCesu8, flags, &se, 
                &nextFunctionId, info->GetLineNumber(), info->GetSourceContextInfo(), info, false, false);

            if (tree && tree->nop == knopProg) 
            {
                auto body = tree->sxProg.pnodeBody;
                if (body && body->nop == knopList)
                {
                    auto fncDecl = body->sxBin.pnode1;
                    if (fncDecl && fncDecl->nop == knopFncDecl)
                        return fncDecl;
                }
            }
        }

        return nullptr;
    }
    
    charcount_t AuthoringFileHandle::GetFunctionNameLocation(ArenaAllocator* alloc, Js::ScriptContext *scriptContext, Js::JavascriptFunction *function)
    {
        Parser parser(scriptContext);            
        ParseNode *funcDecl = ParseFunctionHeader(&parser, scriptContext, function);
        if (funcDecl)
        {
            auto name = funcDecl->sxFnc.pnodeNames;
            // Ignore the error symbol that is inserted by error correction for the case "function() {...}"
            if (name && (name->nop != knopVarDecl || !InternalName(name->sxVar.pid)))
                return name->ichMin;
        }
        return 0;
    }

    AuthoringFileHandle* AuthoringFileHandle::CreateSnapshot()
    {
        auto snapshot = new AuthoringFileHandle(m_factory, m_file, m_kind);
        snapshot->m_fileId = m_fileId;
        snapshot->m_text.Assign(m_text);
        snapshot->m_commentTable.Assign(m_commentTable);
        return snapshot;
    }

    void AuthoringFileHandle::CallFunctionIgnoreExceptions(Js::JavascriptFunction* func, Js::RecyclableObject* thisObj, Js::ScriptContext* scriptContext, PhaseReporter* phaseReporter, Js::Var arg0, Js::Var arg1)
    {
        Assert(func);
        
        // Install a probe to to ignore exceptions
        // Make sure the function is parsed.        
        AuthoringProbe authorProbe(0, func->GetParseableFunctionInfo());
        scriptContext->diagProbesContainer.AddProbe(&authorProbe);
        scriptContext->diagProbesContainer.InitializeInlineBreakEngine(&authorProbe);

        {
            AutoPhaseReporterExecuting autoPhaseReporterExecuting(phaseReporter, &authorProbe, scriptContext);
            try
            {
                JsHelpers::CallFunction(func, thisObj, scriptContext, arg0, arg1);
            }
            catch (Js::JavascriptExceptionObject *)
            {
            }
            catch (Js::InternalErrorException)
            {
            }
            catch (Js::OutOfMemoryException)
            {
            }
            catch (Js::NotImplementedException)
            {
            }
            catch (ExecutionStop)
            {
            }
        }
        // Although rare it is possible for an async breakpoint to be requested between the time the execution stopped 
        // and the exception was handled. This ensures that an async-break is not left over in this case.
        authorProbe.ClearAsyncBreak(scriptContext);

        // Uninstall the probe
        scriptContext->diagProbesContainer.RemoveProbe(&authorProbe);
        scriptContext->diagProbesContainer.UninstallInlineBreakpointProbe(&authorProbe);
    }

    void AuthoringFileHandle::CallFunctionIgnoreExceptions(LPCWSTR functionName, Js::RecyclableObject* thisObj, Js::ScriptContext* scriptContext, PhaseReporter* phaseReporter, Js::Var arg0, Js::Var arg1)
    {
        auto func = JsHelpers::GetProperty<Js::JavascriptFunction*>(thisObj, functionName, nullptr, scriptContext);
        if (func)
            CallFunctionIgnoreExceptions(func, thisObj, scriptContext, phaseReporter);
    }

     void AuthoringFileHandle::InvokeSetTimeoutCalls(PhaseReporter* phaseReporter, Js::ScriptContext* scriptContext)
     {
         CallFunctionIgnoreExceptions(Names::invokeImmediateSetTimeoutCalls, scriptContext->GetGlobalObject(), scriptContext, phaseReporter);
     }
}
