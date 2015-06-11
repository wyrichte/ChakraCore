//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{

    // Indicates that the context is missing a required script. 
    // This is triggered if an asynchronous request for a script loading results in a file that is not in the context. 
    // The host has added a new file to the context, and as a result the context is not complete.
    const int hresNonQuiescentContext = 0xF0000001;

    class ScriptContextManager;

    class PhaseReporter
    {
    protected:
        IAuthorFileContext *m_fileContext;
        RefCountedPtr<AuthoringServicesInfo> m_authoringServicesInfo;
        RefCountedPtr<IAuthorFileHandle> m_executionFile;

        // m_currentProbe and m_executingContext are accessed from a separate thread during calls to Hurry(), 
        // so any accesses need to be guarded by the critical section.
        CriticalSection hurryCriticalSection;
        AuthoringProbe *m_currentProbe;
        Js::ScriptContext *m_executingContext;

        int m_phaseId;
        bool m_abortCalled;
        bool m_progress;
        bool m_inPrepareEval;
#ifdef DEBUG
#ifdef VERBOSE_LOGGING
        Js::HiResTimer debugTimer;
        double lastPhaseChange;
#endif
#endif
    private:
        AuthorFileAuthoringPhase m_phase;
    public:
        PhaseReporter(IAuthorFileContext *fileContext, AuthoringServicesInfo *authoringServicesInfo) : m_fileContext(fileContext), 
            m_authoringServicesInfo(authoringServicesInfo), m_phase(afpDormant), m_abortCalled(false), m_currentProbe(nullptr), m_phaseId(0), 
            m_executionFile(nullptr), m_executingContext(nullptr), m_progress(false), m_inPrepareEval(false) {}

        bool WasAbortCalled() { return m_abortCalled; }

        IAuthorFileContext *FileContext() { return m_fileContext; }

        void Phase(AuthorFileAuthoringPhase phase)
        {
            // If we are executing, an eval is no longer preparing.
            if (phase == afpExecuting)
                m_inPrepareEval = false;

            if (phase != m_phase && m_fileContext && !m_inPrepareEval) 
            {
#ifdef DEBUG
#ifdef VERBOSE_LOGGING
                double now = debugTimer.Now();
                Output::Print(L"From %d to %d, (%g)\n", m_phase, phase, now - lastPhaseChange);
                lastPhaseChange = now;
#endif
#endif
                if (m_executingContext && m_executingContext->GetThreadContext()->IsScriptActive())
                {
                    // Exit script if we are in an active one before calling the host
                    BEGIN_LEAVE_SCRIPT(m_executingContext)
                    {
                        m_fileContext->FileAuthoringPhaseChanged(phase, ++m_phaseId, m_executionFile);
                    }
                    END_LEAVE_SCRIPT(m_executingContext);
                }
                else
                {
                    m_fileContext->FileAuthoringPhaseChanged(phase, ++m_phaseId, m_executionFile);
                }
                m_phase = phase;
                m_progress = false;
            }
        }

        void Progress() { m_progress = true; }
        void InPrepareEval() { m_inPrepareEval = true; }

        ScriptContextManager *GetScriptContextManager();
        PhaseReporter *GetActiveReporter();
        void Executing(AuthoringProbe *probe, Js::ScriptContext *scriptContext, bool doReportPhase = true);
        void DoneExecuting(bool doReportPhase = true);
        void SetExecutionFile(IAuthorFileHandle *handle);
        IAuthorFileHandle* GetExecutionFile();
        void ClearExecutionFile();
    };

    // Helper class to ensure Executing and DoneExecuting are called.
    class AutoPhaseReporterExecuting
    {
    public:
        AutoPhaseReporterExecuting(PhaseReporter *phaseReporter, AuthoringProbe *probe, Js::ScriptContext *scriptContext, bool doReportPhase = true)
        : m_phaseReporter(phaseReporter), m_doReportPhase(doReportPhase)
        {
            Assert(m_phaseReporter != nullptr);
            m_phaseReporter->Executing(probe, scriptContext, m_doReportPhase);
        }
        ~AutoPhaseReporterExecuting()
        {
            m_phaseReporter->DoneExecuting(m_doReportPhase);
        }
    private:
        PhaseReporter *m_phaseReporter;
        bool m_doReportPhase;
    };

    class ScriptContextPath;

    enum AuthoringFileHandleKind 
    { 
        AuthoringFileHandleKind_Normal, 
        AuthoringFileHandleKind_Transient,
        AuthoringFileHandleKind_Helper, 
    };

    __interface RememberNodeCommentsCallback
    {
    public:
        virtual void run(CommentTable* thisPtr, int commentTableIndex) = 0;
    };

    template<typename THandler>
    class RememberNodeCommentsCallbackImpl : public RememberNodeCommentsCallback
    {
    public:
        RememberNodeCommentsCallbackImpl(THandler handler) : m_handler(handler)
        {
        }

        virtual void run(CommentTable* thisPtr, int commentTableIndex)
        {
            this->m_handler(thisPtr, commentTableIndex);
        }
    private:
        THandler m_handler;
    };

    class AuthoringFileHandle : public SimpleComObjectWithAlloc<IAuthorFileHandle>
    {
    private:
        AuthoringFactory *m_factory;
        RefCountedPtr<IAuthorFile> m_file;
        MessageSet *m_messages;
        ScriptContextPath *m_scriptContextPaths;
        RefCountedPtr<CommentTable> m_commentTable;
        int m_fileId;
        AuthoringFileHandleKind m_kind;
        bool m_disableRewrite;
        bool m_inDeferredParsing;

        // True for transient AuthoringFileHandle instances which are usually created to represent a source file of a function 
        // created via Function ctor, like: new Function('arg', 'return arg*arg').
        // When true, the source should not be saved in ScriptContext since the lifetime of the source buffer is short. 
        bool m_transient;

        // Allocated using the ArenaAllocator of the base class.
        RefCountedPtr<AuthoringFileText> m_text; 

    public:
        AuthoringFileHandle(AuthoringFactory *factory, IAuthorFile *file, AuthoringFileHandleKind kind = AuthoringFileHandleKind_Normal) :
          SimpleComObjectWithAlloc<IAuthorFileHandle>(factory->GetAuthoringScriptContext()->GetThreadContext()->GetPageAllocator(), L"ls:AuthFileHandle"),
              m_factory(factory), 
              m_file(file),
              m_messages(NULL),
              m_text(nullptr),
              m_scriptContextPaths(nullptr),
              m_kind(kind),
              m_disableRewrite(kind == AuthoringFileHandleKind_Helper),
              m_transient(kind == AuthoringFileHandleKind_Transient),
              m_fileId(NewFileId()),
              m_inDeferredParsing(false) { }

        void OnDelete() override;

        bool ValidateInstance(AuthoringFactory *factory) { return m_factory == factory; }
        CommentTable *GetCommentTable();
        AuthoringFileText* Text() { return m_text; }
        int FileId() { return m_fileId; }
        int GetLength() { long length = 0;  m_file->GetLength(&length); return length; };

        bool IsSourceAtIndex(Js::ScriptContext *scriptContext, uint sourceIndex);
        bool IsSourceMatched(Js::Utf8SourceInfo * sourceInfo);
        bool IsTransient() { return m_transient; }
        bool IsDisableRewrite() { return m_disableRewrite; }
        bool IsLibraryCode() const { return m_kind == AuthoringFileHandleKind_Helper; }
        void InvalidateContexts();
        void AddContext(ScriptContextPath *scriptContextPath);
        void RemoveContext(ScriptContextPath *scriptContextPath);
        void ForgetBuffer();
        void GetDisplayName(BSTR *name) { m_file->GetDisplayName(name); }
        bool GetIsInDeferredParsingMode() const { return m_inDeferredParsing; }

        template<typename THandler>
        bool RememberNodeComments(ArenaAllocator* alloc, int searchLimOverride, ParseNode* node, ParseNode* parentNode, ParseNode* previousNode, bool searchPreviousNode, THandler handler)
        {
            RememberNodeCommentsCallbackImpl<THandler> wrapper(handler);
            return this->RememberNodeCommentsImpl(alloc, searchLimOverride, node, parentNode, previousNode, searchPreviousNode, (RememberNodeCommentsCallback&)wrapper);
        }

        bool RememberSingleNodeComments(ArenaAllocator* alloc, int searchLimOverride, ParseNode* node, ParseNode* parentNode, ParseNode* previousNode, bool searchPreviousNode, int location)
        {
            return this->RememberNodeComments(alloc, searchLimOverride, node, parentNode, previousNode, searchPreviousNode, [&](CommentTable* commentTable, int commentTableIndex)
            {
                commentTable->Associate(location ? location : node->ichMin, commentTableIndex);
            });
        }

        CommentBuffer* GetLastRememberedComments(ArenaAllocator *alloc, CommentType commentType);
        CommentBuffer* GetNodeComments(ArenaAllocator* alloc, charcount_t ichLocation, CommentType commentType);
        CommentBuffer* GetFunctionComments(ArenaAllocator* alloc, charcount_t functionStartPosition, charcount_t lcurlyPosition, charcount_t firstStatementPosition, charcount_t functionEndPosition, CommentType commentType);
        CommentBuffer* GetFunctionComments(ArenaAllocator* alloc, Js::ScriptContext *scriptContext, Js::JavascriptFunction *function, CommentType commentType);
        CommentBuffer* GetFunctionComments(ArenaAllocator* alloc, ParseNode* pnodeFnc, LanguageServiceExtension *extensions, CommentType commentType);

        charcount_t GetFunctionNameLocation(ArenaAllocator* alloc, Js::ScriptContext *scriptContext, Js::JavascriptFunction *function);     
        void ProcessDeferredParseTree(Js::ScriptContext *ScriptContext, PhaseReporter *reporter, Parser *parser, ParseNodePtr pnodeProg, bool isPrimaryFile);
        static void ProcessEvalTree(Js::ScriptContext *ScriptContext, PhaseReporter *reporter, Parser *parser, ParseNodePtr pnodeProg);

        // Enumerates the arguments of a function and also returns the minimum location of where to begin looking for the function's
        // metadata if the function contains a comment before the first token. This value can then be passed to GetFunctionComments
        // to retrieve the comment.
        template<typename THandler>
        void ForEachArgument(ArenaAllocator* alloc, Js::ScriptContext *scriptContext, Js::JavascriptFunction *function, THandler handler,
            __out charcount_t *functionStartPosition  = nullptr,
            __out charcount_t *lcurlyPosition         = nullptr, 
            __out charcount_t *firstStatementPosition = nullptr,
            __out charcount_t *functionEndPosition    = nullptr)
        {
            bool positionRequested = (functionStartPosition != nullptr);

            // If position is requested, the pointers for storing all the positions must be non-null.
            Assert(!positionRequested || functionStartPosition  != nullptr);
            Assert(!positionRequested || lcurlyPosition         != nullptr);
            Assert(!positionRequested || firstStatementPosition != nullptr);
            Assert(!positionRequested || functionEndPosition    != nullptr);

            Parser parser(scriptContext);
            ParseNode *funcDecl = ParseFunctionHeader(&parser, scriptContext, function);
            if (funcDecl) 
            {
                for (ParseNode *arg = funcDecl->sxFnc.pnodeArgs; arg; arg = arg->sxVar.pnodeNext)
                {
                    handler(arg, false /*not rest param*/);
                }
                if (funcDecl->sxFnc.pnodeRest != nullptr)
                {
                    handler(funcDecl->sxFnc.pnodeRest, true /*rest param*/);
                }

                if (positionRequested)
                {
                    *functionStartPosition = funcDecl->ichMin;
                    *lcurlyPosition = parser.GetLanguageServiceExtension()->LCurly(funcDecl);
                    *firstStatementPosition = funcDecl->ichLim;
                    *functionEndPosition = funcDecl->ichLim;
                }
            }
            else if (positionRequested)
            {
                *functionStartPosition = 0;
                *lcurlyPosition = 0;
                *firstStatementPosition = 0;
                *functionEndPosition = 0;
            }
        }

        AuthoringFileHandle* CreateSnapshot();

        STDMETHOD(FileChanged)(int changesLength, AuthorFileChange* changes);
        STDMETHOD(GetMessageSet)(IAuthorMessageSet **messages);
        STDMETHOD(Close)();
        STDMETHOD(get_FileId)(__out long *fileId);

        HRESULT UpdateParseTree(PhaseReporter *reporter, Js::ScriptContext *scriptContext, bool enableDeferredParsing, ParseNodeTree &tree, bool isPrimaryFile, __inout uint *sourceIndex);
        HRESULT AddGuardsAndApplyComments(Js::ScriptContext *scriptContext, ParseNodeTree *tree, bool isPrimaryFile, long cursorPos = -1, bool suppressThrows = false);
        HRESULT GenerateBody(PhaseReporter *reporter, ParseNodeTree *tree, Js::ScriptContext *scriptContext, uint sourceIndex, Js::FunctionBody **result);
        HRESULT Apply(PhaseReporter *reporter, Js::ScriptContext *scriptContext, ScriptContextPath *owner, ScriptContextPath *parent, bool isPrimaryFile, ParseNodeTree *existingTree = nullptr, uint fileIndex = 0);
        HRESULT ApplyAsyncRequests(ArenaAllocator* alloc, PhaseReporter *reporter, Js::ScriptContext *scriptContext, ScriptContextPath *owner);
        HRESULT GetAsyncRequests(ArenaAllocator* alloc, PhaseReporter *reporter, Js::ScriptContext *scriptContext, ScriptContextPath *owner, JsUtil::List<Js::RecyclableObject*, ArenaAllocator>* loadedScripts);
        HRESULT ExecuteAsyncRequests(ArenaAllocator* alloc, PhaseReporter *reporter, Js::ScriptContext *scriptContext, ScriptContextPath *owner, JsUtil::List<Js::RecyclableObject*, ArenaAllocator>* loadedScripts);
        HRESULT AuthoringFileHandle::EnsureAsyncRequests(PhaseReporter *reporter, AsyncRequestList* dependentAsyncRequests);

    private:
        HRESULT ApplyCommentsRewrite(PageAllocator* pageAlloc, ParseNodeTree *tree, long cursorPos = -1);
        HRESULT FetchText(Js::ScriptContext* scriptContext);
        HRESULT CreateParseTree(Js::ScriptContext *scriptContext);
        static int NewFileId();
        ParseNode *ParseFunctionHeader(__in Parser *parser, __in Js::ScriptContext *scriptContext, __in Js::JavascriptFunction *function);
        static void CallFunctionIgnoreExceptions(Js::JavascriptFunction* func, Js::RecyclableObject* thisObj, Js::ScriptContext* scriptContext, PhaseReporter* phaseReporter, Js::Var arg0 = nullptr, Js::Var arg1 = nullptr);
        static void CallFunctionIgnoreExceptions(LPCWSTR functionName, Js::RecyclableObject* thisObj, Js::ScriptContext* scriptContext, PhaseReporter* phaseReporter, Js::Var arg0 = nullptr, Js::Var arg1 = nullptr);
        static void InvokeSetTimeoutCalls(PhaseReporter* phaseReporter, Js::ScriptContext* scriptContext);
        bool RememberNodeCommentsImpl(ArenaAllocator* alloc, int searchLimOverride, ParseNode* node, ParseNode* parentNode, ParseNode* previousNode, bool searchPreviousNode, RememberNodeCommentsCallback& handler);
    };


}