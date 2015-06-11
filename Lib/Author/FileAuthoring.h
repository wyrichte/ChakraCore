//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once
namespace Js
{
    class CharClassifier;
    enum CharClassifierModes;
}
namespace Authoring
{
    class ExtensionStateContainer;
    class JsValueDoc;
    class PropertyFilter;

    class AutoSetAuthorFileContext
    {
    public:
        AutoSetAuthorFileContext(Js::ScriptContext *scriptContext, IAuthorFileContext * fileContext)
        {
            Assert(scriptContext != nullptr);
            m_threadContext = scriptContext->GetThreadContext();
            m_threadContext->SetAuthoringContext(fileContext);
        }
        ~AutoSetAuthorFileContext()
        {
            m_threadContext->SetAuthoringContext(nullptr);
        }
    private:
        ThreadContext *m_threadContext;
    };

    class FileAuthoring : public PhaseReporter, public SimpleComObjectWithAlloc<IAuthorFileAuthoring>
    {
    private:
        ArenaAllocator* m_contextAlloc;
        Js::ScriptContext *m_rootScriptContext;
        RefCountedPtr<ScriptContextPath> m_scriptContextPath;
        LeafScriptContext *m_leafScriptContext;
        ScriptContextAutoPtr m_parseOnlyScriptContext;
        Js::ScriptContext *m_scriptContext;
        AuthoringFactory *m_factory;

        RefCountedPtr<AuthoringFileHandle> m_primaryFile;
        ParseNodeTree m_primaryTree;

        IAuthorCompletionDiagnosticCallback* m_diagCallback;
        CriticalSection hurryLock;

        uint m_primarySourceIndex;
        uint m_hurrySkipped;

        bool m_hurryCalled;
        bool m_contextInitialized;
        bool m_parserOnlyContext;
        bool m_polluted;
        bool m_contextChanged;
        bool m_scriptContextPathComplete;
        bool m_shouldCopyOnGet;

        // Support QI of additional interfaces 
        HRESULT GetInterface(REFIID iid, __out void** ppvObject) override;

        template<typename TRoutine>
        HRESULT EnsureQuiescentContext(TRoutine routine);

        template<typename TDoneHandler>
        HRESULT GetSingleCompletion(ArenaAllocator* alloc, FileAuthoring* fileAuthoring, long offset, Js::InternalString* itemName, TDoneHandler doneHandler);

        JsUtil::List<Completions*, ArenaAllocator> _completionLists;

        HRESULT ValidateInternalHandles(__in_ecount(count) AuthoringFileHandle **handles, int count);
        HRESULT CollectSources(bool primaryOnly);
        void ReleasePrimaryContext();
        void ForgetContext();
        void CleanupParseOnlyContext();
        bool IsValidParseOnlyContext() { return m_scriptContext && m_parserOnlyContext; }
        void DecommitUnusedPages();
        HRESULT UpdatePrimary(bool fullContext);
        HRESULT ApplyContext();
        bool IsScriptContextPathValid();
        bool IsScriptContextValid();
        HRESULT AllocateContext();
        HRESULT EnsureContext();
        HRESULT ExecuteToBreakpoint(int loadLocation, Js::RegSlot slot, Js::FunctionBody *exprContainer, Js::FunctionBody *executingFunction, bool isRoot, Js::DynamicObject* environment, AuthorCompletionDiagnosticFlags *diagFlags, bool isFinalPass, Js::Var *var);
        void ExecuteRootFunction(AuthoringProbe& authorProbe, Js::JavascriptFunction *jsFunc, Js::Arguments& arguments, Js::Var *result, bool doNotReportPhase = false);
        HRESULT Execute(ParseNode *expr, AuthorCompletionDiagnosticFlags *diagFlags, bool isFinalPass, Js::Var *var);
        bool IdentifierLikeNode(ParseNodePtr pnode);
        bool IsNameNode(ParseNodePtr pnode);
        HRESULT RewriteExecutionTree(ParseNodePtr current);
        HRESULT GetValueOf(ArenaAllocator *localArena, ParseNodePtr pnode, AuthorCompletionDiagnosticFlags *diagFlags, Js::Var& value);
        HRESULT GetValueAt(long offset, ArenaAllocator *localArena, AuthorCompletionDiagnosticFlags &diagFlags, Js::Var& value, bool leftOfDot);
        bool IsExceptionObject(ArenaAllocator *alloc, Js::ScriptContext* scriptContext, Js::Var& var);
        static bool IsUndefinedObject(Js::ScriptContext* scriptContext, Js::Var value);
        static bool IsNullObject(Js::ScriptContext* scriptContext, Js::Var value);
        void AddPropertyCompletion(Completions *completions, Js::Var value, Js::RecyclableObject *object, Js::RecyclableObject *root, Js::PropertyId id, AuthorScope defaultScope);
        HRESULT AddCompletions(Js::Var value, Completions *completions, bool ownOnly, bool excludeObjectPrototype, bool excludeConstructor, bool checkWith, AuthorScope defaultScope, PropertyFilter isPropertyNameExcluded);
        HRESULT AddCompletionsImpl(Js::Var value, Completions *completions, bool ownOnly, bool excludeObjectPrototype, bool excludeConstructor, bool checkWith, AuthorScope defaultScope, PropertyFilter isPropertyNameExcluded);
        HRESULT CreateScopeRecord(ArenaAllocator *localArena, Parser *parser, ParseNodeCursor *cursor, __out ParseNodePtr *scopeRecord);
        HRESULT AddSymbolsAt(long offset, ArenaAllocator *localArena, Completions *completions);
        HRESULT AddSymbolsOf(ArenaAllocator *localArena, ParseNodeCursor *cursor, Completions *completions);
        HRESULT GetSymbolCompletions(long offset, ArenaAllocator *localArena, Completions *completions, __out Js::Var* symbolValue);
        HRESULT GetObjectLiteralCompletions(long offset, ArenaAllocator *localArena, Completions *completions, bool* literalCompletionAvailable);
        HRESULT GetLabelCompletions(long offset, ArenaAllocator *localArena, Completions *completions);
        void GetSymOf(tokens tk, _Outptr_result_buffer_(*cch) OLECHAR** ppSz, _Out_ ulong* cch);
        void AddReservedWordList(Completions *completions, __in_ecount(wordsLen) tokens* words, size_t wordsLen);
        HRESULT AddReservedWordsFor(ArenaAllocator *localArena, charcount_t offset, Completions *completions);
        AuthoringFileHandle* GetAuthoringFileByIndex(uint sourceIndex, Js::ScriptContext *scriptContext = nullptr);
        AuthoringFileHandle* GetAuthoringFileBySourceInfo(Js::Utf8SourceInfo * sourceInfo);
        HRESULT FindFunctionSource(ArenaAllocator *alloc, Js::JavascriptFunction* &func, AuthoringFileHandle* &file);
        HRESULT AddStaticFieldsDoc(ArenaAllocator* alloc, Js::JavascriptFunction* func);
        HRESULT GetFuncDocComments(ArenaAllocator* alloc, Js::JavascriptFunction* func, FunctionDocComments** funcDoc);
        static void ForceRegisterLoad(ParseNode* callNode, Parser* parser);
        HRESULT TestAbort();
        static bool IsCallOrNew(ParseNode* node);
        static bool IsFuncDeclOrProg(ParseNode* node);
        bool GetArgumentLimit(ParseNode* node, charcount_t offset, int& argIndex);
        bool FindArgument(ParseNode* node, charcount_t offset, int& argIndex);
        int GetArgumentIndex(ParseNode* callNode, long offset);
        HRESULT GetFunctionHelp(LPCWSTR funcAlias, __in Js::JavascriptFunction* func, Js::RecyclableObject* parentObject, __inout_opt AuthorDiagStatus *diagStatus, __out IAuthorFunctionHelp **result);
        HRESULT GetFunctionHelp(LPCWSTR name, AuthoringFileHandle* srcFile, Js::JavascriptFunction *function, Js::RecyclableObject* parentObject, __inout AuthorDiagStatus *diagStatus, __out IAuthorFunctionHelp **result);
        HRESULT ResolveDocCommentRef(ArenaAllocator* alloc, JsValueDoc* docCommentRef, AuthorScope scope, LPCWCHAR fieldName, __out JsValueDoc*& doc, __out AuthoringFileHandle*& file);
        static AuthorType GetAuthorType(ParseNode* node);
        static AuthorType GetAuthorType(Js::TypeId typeId);
        HRESULT GetFunctionValueAt(long offset, ArenaAllocator *localArena, Js::Var& callTargetValue, Js::RecyclableObject*& parentObject);
        void InstallProbe(AuthoringProbe* authorProbe);
        void UnistallProbe(AuthoringProbe* authorProbe);
        template<typename TOperation>
        void WithProbe(TOperation operation, Js::RegSlot slot, Js::FunctionBody *exprContainer, bool enableLoopGuardsOnAsyncBreak = true, int breakpointLocation = -1, IAuthorCompletionDiagnosticCallback *diagCallback = nullptr, bool isFinalPass = false);
        HRESULT GetHostType(Js::HostType *hostType);
        LanguageServiceExtension::CompletionRangeMode GetCompletionRangeMode(uint offset);
        bool IsCursorInComment(ParseNodeCursor* cursor);
        bool IsCompletionSupported(ParseNodeCursor* cursor, LanguageServiceExtension::CompletionRangeMode* completionRangeMode);
        bool IsImplicitFunctionHelpSupported(ParseNodeCursor* cursor);
        Js::Var UnwrapUndefined(Js::Var value, bool *isTrackingValue = nullptr);
        bool TryToGetLocationFromFieldDefinition(ArenaAllocator* arena, Js::RecyclableObject* parentObject, Js::InternalString* fieldName, int* fileId, long* sourceOffset);
        bool TryToGetLocationFromObjectDefinitionLocation(ArenaAllocator* arena, Js::RecyclableObject* instance, int* fileId, long* sourceOffset);
        bool TryToGetLocationInformationFromFunctionParseInformation(ArenaAllocator* arena, Js::RecyclableObject* functionObject, int* fileId, long* sourceOffset);

        void ResetHurryAndAbort()
        {
            this->SetHurryCalled(false);
            m_hurrySkipped = 0;
            m_abortCalled = false;
        }

        void SetHurryCalled(bool hurryCalled);

    public:
        HRESULT GetInternalHandle(IAuthorFileHandle *externalHandle, AuthoringFileHandle *&internalHandle);

        AuthoringFileHandle* GetAuthoringFileById(int fileId);

        HRESULT TestHasActiveCursors()
        {
            return m_primaryFile && m_primaryTree.HasActiveCursors() ? E_INVALIDARG : S_OK;
        }

        static HRESULT ResolveDocCommentRef(ArenaAllocator* alloc, FileAuthoring* fileAuthoring, ScriptContextPath *contextPath, JsValueDoc *docCommentRef, AuthorScope scope, LPCWCHAR fieldName, __out JsValueDoc *&doc, __out AuthoringFileHandle *& file);

        static Js::CharClassifier *lsCharClassifier;

        static Js::CharClassifier *GetLSCharClassifier();

    public:
        bool IsHurryCalled();

        FileAuthoring(AuthoringFactory *factory, AuthoringServicesInfo *authoringServicesInfo, Js::ScriptContext *scriptContext, IAuthorFileContext *fileContext);

        Js::ScriptContext *GetScriptContext() { return m_scriptContext; }
        AuthoringFileHandle* GetPrimaryFile() { return m_primaryFile; }
        ParseNodeTree* GetPrimaryTree() { return &m_primaryTree; }

        ExtensionStateContainer* ExtensionsState();

        BOOL IsValidMissingValueContext(Js::ScriptContext * scriptContext) { return scriptContext == m_scriptContext && IsScriptContextValid(); }

        HRESULT ExecuteFunction(Js::JavascriptFunction *jsFunc, Js::Arguments& arguments, Js::Var *result, bool doNotDisableLoopGuard = false, bool doNotReportPhase = false);
        HRESULT ExecuteSetter(Js::RecyclableObject* obj, LPCWSTR propertyName, Js::Var value);
        HRESULT ExecuteGetter(Js::RecyclableObject* obj, LPCWSTR propertyName, Js::Var* result);

        AuthoringFileHandle* GetAuthoringFile(Js::ParseableFunctionInfo* jsFuncBody);
        AuthoringFileHandle* GetAuthoringFile(uint sourceIndex, Js::ScriptContext *scriptContext);
        uint GetSourceIndexOf(AuthoringFileHandle *file);
        uint GetSourceIndexOf(int fileId);
        int GetFileIdOf(uint sourceIndex, Js::ScriptContext *scriptContext = nullptr);
        bool ShouldCopyOnGet() { return m_shouldCopyOnGet; }

        HRESULT CreateSymbolHelp(Js::ScriptContext* scriptContext, Js::RecyclableObject* jsParentObj, Js::Var jsValue, LPCWSTR name, AuthorScope scope, AuthorType authorType,
            int fileId, charcount_t declarationPos, IAuthorSymbolHelp** result);

        virtual void OnDelete() override;
        void EngineClosing();

        void AddCompletionList(Completions* completions);
        void RemoveCompletionList(Completions* completions);

        void CreateCapturingProxyHolder(Js::ScriptContext* scriptContext, Js::FunctionBody* exprContainer, /* out */ Js::Var* capturingProxyHolder);
        void ExecuteFunctionWithCapturingProxy(Js::JavascriptFunction* function, int paramIndexOfCall, Js::JavascriptProxy* capturingProxy);

        FileAuthoring* CreateSnapshot();

        // IAuthorFileAuthoring interface
        STDMETHOD(Update)();
        STDMETHOD(GetRegions)(IAuthorRegionSet **result);
        STDMETHOD(GetTaskComments)(AuthorTaskCommentPrefix* prefixes, ULONG count, IAuthorTaskCommentSet **result);
        STDMETHOD(GetCompletions)(long offset, AuthorCompletionFlags flags, IAuthorCompletionSet **result);
        STDMETHOD(GetQuickInfo)(long offset, AuthorFileRegion *extent, IAuthorSymbolHelp **result);
        STDMETHOD(GetASTAsJSON)(BSTR *result);
        STDMETHOD(GetFunctionHelp)(__in long offset, __in AuthorFunctionHelpFlags flags, __out_ecount(1) DWORD *currentParameterIndex,
            __inout_opt AuthorFileRegion *extent, __out AuthorDiagStatus *diagStatus, __out IAuthorFunctionHelp **result);
        STDMETHOD(ContextChanged)();
        STDMETHOD(Hurry)(int phaseId);
        STDMETHOD(Abort)();
        STDMETHOD(GetASTCursor)(__out IAuthorParseNodeCursor **result);
        STDMETHOD(GetDefinitionLocation)(long offset, __out IAuthorFileHandle **fileHandle, __out long *location);
        STDMETHOD(GetReferences)(long offset, __out IAuthorReferenceSet **references);
        STDMETHOD(GetStructure)(__out IAuthorStructure **result);
        STDMETHOD(SetCompletionDiagnosticCallback)(__in IAuthorCompletionDiagnosticCallback *acdCallback);
        STDMETHOD(GetDiagnostics)(__out IAuthorDiagnostics **result);

        // Other methods
        STDMETHOD(GetCompletionHint)(long completionCookie, BSTR *result);        
        HRESULT CreateASTCursor(IAuthorParseNodeCursor **result);
        HRESULT SerializeTreeIntoJSON(TextBuffer *buffer, ArenaAllocator* alloc);
        STDMETHOD(HurryImpl)(int phaseId, bool isAbort);
        STDMETHOD(GetStructureImpl)(__out IAuthorStructure **result);
     };
}
