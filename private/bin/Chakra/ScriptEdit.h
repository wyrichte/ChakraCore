//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once
#ifdef EDIT_AND_CONTINUE

namespace Js
{
    class ScriptEdit;
    class ScriptParseTree;
    class ScriptDiff;
    class ScriptEditAction;

    //-----------------------------------------------------------------------------
    // Dispatch a call synchronously in given thread through PDM.
    //-----------------------------------------------------------------------------
    template <class Fn>
    HRESULT Dispatch(IDebugBitCorrectApplicationThread* pThread, const Fn& fn)
    {
        class DebugThreadCallDispatcher :
            public ComObjectBase<IDebugThreadCall, DebugThreadCallDispatcher>
        {
        private:
            const Fn* m_fn;

        public:
            void Init(const Fn* fn)
            {
                m_fn = fn;
            }

            STDMETHOD(ThreadCallHandler)(
                DWORD_PTR dwParam1,
                DWORD_PTR dwParam2,
                DWORD_PTR dwParam3)
            {
                return (*m_fn)();
            }
        };

        CComPtr<DebugThreadCallDispatcher> spDispatcher;
        HRESULT hr = DebugThreadCallDispatcher::CreateInstance(&spDispatcher);
        if (SUCCEEDED(hr))
        {
            spDispatcher->Init(&fn);
            hr = pThread->SynchronousCallIntoThread(spDispatcher, 0, 0, 0);
        }
        return hr;
    }
    
    //-----------------------------------------------------------------------------
    // ScriptEdit provides EnC service. Only one instance is created/owned by ScriptEngine.
    //-----------------------------------------------------------------------------
    class ScriptEdit :
        public ComObjectBase<IActiveScriptEdit, ScriptEdit>
    {
    private:
        ScriptEngine* m_scriptEngine;   // Owner no-ref reference. This object is only created/owned by ScriptEngine.
        EditAllocator* m_allocator;    // Allocator for AST etc. for this session
        ULONG m_baselineGeneration;     // A counter to validate CommitEdit. CommitEdit is only allowed on the same baseline
                                        // generation when the edit query was made. Each CommitEdit increments this counter.
        
    public:
        ScriptEdit();
        HRESULT Init(ScriptEngine* scriptEngine);
        void FinalDestruct();

        //
        // IActiveScriptEdit
        //
        STDMETHOD(QueryEdit)(
            /* [size_is][in] */ ScriptEditRequest *requests,
            /* [in] */ ULONG count,
            /* [out] */ IScriptEditQuery **ppQueryResult);
    
        ScriptEngine* GetScriptEngine() const { return m_scriptEngine; }
        ScriptContext* GetScriptContext() const { return m_scriptEngine->GetScriptContext(); }
        ULONG GetBaselineGeneration() const { return m_baselineGeneration; }
        EditAllocator* GetAllocator() const { return m_allocator; }

        void IncBaselineGeneration() { m_baselineGeneration++; }
    };

    //-----------------------------------------------------------------------------
    // Edit query result. Describes if an edit request is supported, enumerate errors if any, and
    // provides other services needed by EnC client.
    //-----------------------------------------------------------------------------
    class ScriptEditQuery :
        public ComObjectBase<IScriptEditQuery, ScriptEditQuery>
    {
    private:
        CComPtr<ScriptEdit> m_scriptEdit;       // The ScriptEdit instance that created this query
        CComPtr<ScriptEngine> m_scriptEngine;   // Keep the ScriptEngine alive
        ULONG m_baselineGeneration;             // The ScriptEdit baseline generation when this query was made
        bool m_canApply;

        typedef JsUtil::List<ScriptDiff*, EditAllocator> ScriptDiffList;
        RecyclerRootPtr<ScriptDiffList> m_scriptDiffs;     // Owns every ScriptDiff in the list

        typedef JsUtil::BaseDictionary<Utf8SourceInfo*, ScriptEditAction*, EditAllocator> ScriptEditActionMap;
        RecyclerRootPtr<ScriptEditActionMap> m_scriptEditActionMap; // oldUtf8SourceInfo* -> ScriptEditAction*

        static void RecyclerEnumClassCallback(void *address, size_t size);
        void EnumFunction(JavascriptFunction* function);

        void CreateScriptEditActions();
        void CommitScriptEditActions();
        void ClearScriptEditActions();

        PREVENT_COPY(ScriptEditQuery); // Disable copy constructor and operator=

    public:
        ScriptEditQuery();
        HRESULT Init(ScriptEdit* scriptEdit);
        void FinalDestruct();

        //
        // IActiveScriptEdit
        //
        STDMETHOD(CanApply)(
            /* [out] */ BOOL *pCanApply);

        STDMETHOD(CommitEdit)();
        
        STDMETHOD(GetResultCount)(
            /* [out] */ ULONG *count);

        STDMETHOD(GetResult)(
            /* [in] */ ULONG index,
            /* [out] */ ScriptEditResult *result);

        ScriptEdit* GetScriptEdit() const { return m_scriptEdit; }
        ScriptEngine* GetScriptEngine() const { return m_scriptEngine; }

        HRESULT ProcessQuery(ScriptEditRequest *requests, ULONG count);
        HRESULT ProcessQuery(const ScriptEditRequest& request);
        HRESULT ProcessQuery(const ScriptEditRequest& request, Utf8SourceInfo* utf8SourceInfo, _In_reads_(newLen) WCHAR* newFullText, ULONG newLen);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        void Trace(const ScriptEditRequest& r) const;
#endif
    };

    //-----------------------------------------------------------------------------
    class ScriptParseTree
    {
    private:        
        RecyclerRootPtr<Utf8SourceInfo> m_utf8SourceInfo; // Utf8SourceInfo in recycler
        Parser m_parser;            // Parser owns ParseNodes. Needs to keep alive.
        ParseNodePtr m_parseTree;
        LocalFunctionId m_functionIdBegin; // The start functionId used by parse
        CompileScriptException m_parseException;
        bool m_hasError;

        void Parse(EditAllocator* alloc, Utf8SourceInfo* utf8SourceInfo);

        PREVENT_COPY(ScriptParseTree); // Disable copy constructor and operator=

    public:
        ScriptParseTree(EditAllocator* alloc, PCWSTR source, ULONG length, const SRCINFO* srcInfo, ULONG parseFlags, ULONG byteCodeGenerationFlags, ScriptContext* scriptContext);
        ScriptParseTree(EditAllocator* alloc, Utf8SourceInfo* utf8SourceInfo, ScriptContext* scriptContext);
        ~ScriptParseTree();
        bool HasError() const { return m_hasError;  }
        CompileScriptException* GetError() { return &m_parseException; }

        Utf8SourceInfo* GetUtf8SourceInfo() const { return m_utf8SourceInfo; }
        LPCUTF8 GetUtf8Source() const { return m_utf8SourceInfo->GetSource(); }
        ParseNodePtr GetParseTree() const { AssertMsg(!m_hasError, "One should not get the parse tree after parsing failure.");  return m_parseTree; }
        LocalFunctionId GetFunctionIdBegin() const { return m_functionIdBegin; }
        void GenerateByteCode(ScriptContext* scriptContext, _Outptr_ ParseableFunctionInfo** root);
    };

    //-----------------------------------------------------------------------------
    class ScriptDiff
    {
    public:
        typedef TreeMatch<FunctionTreeComparer<EditAllocator>, EditAllocator> TreeMatchType;
        typedef EditScript<FunctionTreeComparer<EditAllocator>, EditAllocator> EditScriptType;
        typedef SemanticChange::SemanticChangeList SemanticChangeList;

    private:
        CComPtr<ScriptEdit> m_scriptEdit;
        ScriptParseTree* m_oldTree;
        ScriptParseTree* m_newTree;

        PCWSTR m_newFullText; // Saved copy of new full text, needed at apply to update PDM
        RecyclerRootPtr<FunctionBody> m_newRoot; // Pinned new root FunctionBody

        TreeMatchType* m_match;
        EditScriptType* m_editScript;
        SemanticChangeList* m_semanticChanges;

        ScriptDiff(ScriptEdit* scriptEdit, ScriptParseTree* oldTree, ScriptParseTree* newTree, PCWSTR newFullText, TreeMatchType* match, EditScriptType* editScript);
        PREVENT_COPY(ScriptDiff); // Disable copy constructor and operator=

        void AnalyzeSemanticChanges();

    public:
        ~ScriptDiff();
        static ScriptDiff* Diff(ScriptEdit* scriptEdit, ScriptParseTree* oldTree, ScriptParseTree* newTree, PCWSTR newFullText);

        ScriptEdit* GetScriptEdit() const { return m_scriptEdit; }
        const ScriptParseTree& OldTree() const { return *m_oldTree; }
        const ScriptParseTree& NewTree() const { return *m_newTree; }
        PCWSTR NewFullText() const { return m_newFullText; }
        FunctionBody* NewRoot() const { return m_newRoot; }
        const TreeMatchType& Match() const { return *m_match; }
        const EditScriptType& EditScript() const { return *m_editScript; }
        const SemanticChangeList& SemanticChanges() const { return *m_semanticChanges; }

        ScriptParseTree& NewTree() { return *m_newTree; }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        void TraceAstDiff() const;
        void TraceSemanticChanges() const;
        void Trace(PCWSTR label, ParseNodePtr pnode, LPCUTF8 source, ParseNodePtr pNewNode = nullptr, LPCUTF8 newSource = nullptr) const;

        static void GetNodeStr(_Out_writes_z_(maxLen) utf8char_t* buffer, size_t maxLen, ParseNodePtr pnode, LPCUTF8 source);
        static PCWSTR GetParseNodeName(ParseNodePtr pnode);
#endif
    };

    //-----------------------------------------------------------------------------
    // Interface for common EnC edit action.
    //-----------------------------------------------------------------------------
    class IEditAction
    {
    public:
        virtual ~IEditAction() {} // Ensure correct destructor called for subclass

        // Prepare this edit action (alternatively can be done by constructor). May fail/throw/OOM.
        // Do not change any runtime state yet.
        void Prepare() {}

        // Commit this edit action. Should not fail/throw. In case we have to fail, must ensure
        // runtime in consistent state.
        //
        // Example: Suppose we have multiple edit requests. During Commit() some sources are
        //          successfully committed, then one source fail at updating PDM. This one failure
        //          should not affect runtime. Runtime continues with already committed sources.
        virtual void Commit() = 0;

        virtual void Free(EditAllocator* alloc) = 0;

        class EditActionList : public JsUtil::List<IEditAction*, EditAllocator>
        {
        public:
            EditActionList(EditAllocator* alloc) : JsUtil::List<IEditAction*, EditAllocator>(alloc) {}

            ~EditActionList()
            {
                EditAllocator* alloc = GetAllocator();
                Map([alloc](int, IEditAction* action)
                {
                    action->Free(alloc);
                });
            }

            // Commit all edit actions in this list
            void CommitAll() const
            {
                Map([](int, IEditAction* action)
                {
                    action->Commit();
                });
            }
        };
    };

    template <class T>
    class IEditActionImpl: public IEditAction
    {
    protected:
        T* pThis() { return static_cast<T*>(this); }

    public:
        virtual void Free(EditAllocator* alloc) override
        {
            EditDelete(alloc, pThis());
        }
    };

    //-----------------------------------------------------------------------------
    // Edit action for one script diff.
    //-----------------------------------------------------------------------------
    class ScriptEditAction : public IEditActionImpl<ScriptEditAction>
    {
    private:
        class UpdateFunction;
        typedef JsUtil::BaseDictionary<LocalFunctionId, UpdateFunction*, EditAllocator> UpdateFunctionMap;

        ScriptDiff* m_diff;

        IEditAction::EditActionList m_editActions;  // All edit actions for this source
        UpdateFunctionMap m_updateFunctionActions;  // Update function actions that need to collect function objects

        class UpdateFunction : public IEditActionImpl<UpdateFunction>
        {
        private:
            typedef JsUtil::List<JavascriptFunction*, EditAllocator> FunctionList;

            ScriptEditAction* m_ownerScriptEditAction;
            const SemanticChange& m_semanticChange;
            FunctionList m_functionList;

        public:
            UpdateFunction(ScriptEditAction* owner, const SemanticChange& semanticChange);

            void AddFunction(JavascriptFunction* function);
            virtual void Commit() override;
        };

        class AddGlobal : public IEditActionImpl<AddGlobal>
        {
        private:
            ScriptEditAction* m_ownerScriptEditAction;
            const SemanticChange& m_semanticChange;
        public:
            AddGlobal(ScriptEditAction* owner, const SemanticChange& semanticChange);
            virtual void Commit() override;
        };

        void AddUpdateFunction(const SemanticChange& semanticChange);
        void AddAddGlobal(const SemanticChange& semanticChange);

        PREVENT_COPY(ScriptEditAction); // Disable copy constructor and operator=

    public:
        ScriptEditAction(ScriptDiff* diff);
        ~ScriptEditAction();

        ScriptDiff* GetScriptDiff() const { return m_diff; }
        ScriptEdit* GetScriptEdit() const { return m_diff->GetScriptEdit(); }

        void Prepare();
        void CheckRecyclerFunction(JavascriptFunction* function);
        virtual void Commit() override;
    };

} // namespace Js

#endif
