//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class ASTMutatingVisitor
    {
        template <typename TCallback>
        struct ParseNodeMutatingVisitor
        {
        public:
            typedef TCallback Context;

            template <typename TCallback>
            class MutatorWalkerPolicy
            {
            public:
                typedef bool ResultType;
                typedef TCallback Context;
                inline bool DefaultResult() { return true; }
                inline bool ContinueWalk(bool value) { return value; }
                inline bool WalkNode(ParseNode *node, Context context) { return true; }
                inline bool WalkListNode(ParseNode *node, Context context) { return true; }
                inline bool WalkFirstChild(ParseNode *&node, Context context) { return context(node); }
                inline bool WalkSecondChild(ParseNode *&node, Context context) { return context(node);  }
                inline bool WalkNthChild(ParseNode* pnodeParent, ParseNode *&node, Context context) { return context(node); }
                inline void WalkReference(ParseNode **ppnode, Context context) { }
            };

            inline bool Preorder(ParseNode *node, Context context)
            {
                ParseNodeWalker< MutatorWalkerPolicy<TCallback> > walker;
                return walker.Walk(node, context);
            }

            inline void Inorder(ParseNode *node, Context context) { }
            inline void Midorder(ParseNode *node, Context context) { }
            inline void Postorder(ParseNode *node, Context context) { }
            inline void InList(ParseNode *pnode, Context context) { }
            inline void PassReference(ParseNode **ppnode, Context context) { }
        };
    public:
        template <typename TCallback>
        static void Visit(ParseNodePtr tree, TCallback callback)
        {
            ParseNodeVisitor< ParseNodeMutatingVisitor<TCallback> > visitor;
            visitor.Visit(tree, callback);
        }
    };

    class ASTHelpers
    {
        template <typename THandler>
        struct LambdaVisitor : VisitorPolicyBase<THandler>
        {
            bool Preorder(ParseNodePtr pnode, THandler handler)
            {
                if(pnode) return handler(pnode);
                return true;
            }

            void InList(ParseNodePtr pnode, THandler handler)
            {
                if(pnode)
                    handler(pnode);
            }
        };

        template <typename TPreorder, typename TPostorder>
        struct LambdaVisitor2Context
        {
            TPreorder preorder;
            TPostorder postorder;

            LambdaVisitor2Context(TPreorder preorder,  TPostorder postorder): preorder(preorder), postorder(postorder) { }
        };

        template <typename TPreorder, typename TPostorder>
        struct LambdaVisitor2 : VisitorPolicyBase<LambdaVisitor2Context<TPreorder, TPostorder> *>
        {
            bool Preorder(ParseNodePtr pnode, Context context)
            {
                if (pnode) return context->preorder(pnode);
                return true;
            }

            void Postorder(ParseNodePtr pnode, Context context)
            {
                if (pnode) context->postorder(pnode);
            }
        };

    public:
        static bool IsArgument(ParseNodePtr pnodeFnc, ParseNodePtr pnodeVar);

        template <typename THandler>
        static void Visit(ParseNodePtr node, THandler handler)
        {
            ParseNodeVisitor<LambdaVisitor<THandler>> visitor;
            visitor.Visit(node, handler);
        } 

        template <typename TPreorder, typename TPostorder>
        static void Visit(ParseNodePtr pnode, TPreorder preorder, TPostorder postorder)
        {
            typedef ParseNodeVisitor<LambdaVisitor2<TPreorder, TPostorder>> Visitor;
            Visitor visitor;
            LambdaVisitor2Context<TPreorder, TPostorder> context(preorder, postorder);

            visitor.Visit(pnode, &context);
        }

        template <typename THandler>
        static void ForEachArg(ParseNodePtr args, THandler handler)
        {
            auto node = args;
            while(node)
            {
                Assert(node->nop == knopVarDecl);
                handler(node);
                node = node->sxVar.pnodeNext;
            }
        }

        // Traverses statements in the current function only. 
        template <typename THandler>
        static void TraverseStatements(ParseNodePtr node, THandler handler)
        {
LLoop:      // gotos to this label are tail-call optimizations. 
            // Each optimization assigns to node what would have been passed in as a parameter to node then jumps here.
            if (node)
            {
                if (node->IsStatement())
                    handler(node);
                switch (node->nop)
                {
                case knopList:
                    TraverseStatements(node->sxBin.pnode1, handler);
                    node = node->sxBin.pnode2;
                    goto LLoop;
                case knopFor:
                    node = node->sxFor.pnodeBody;
                    goto LLoop;
                case knopIf:
                    TraverseStatements(node->sxIf.pnodeTrue, handler);
                    node = node->sxIf.pnodeFalse;
                    goto LLoop;
                case knopWhile:
                case knopDoWhile:
                    node = node->sxWhile.pnodeBody;
                    goto LLoop;
                case knopForIn:
                case knopForOf:
                    node = node->sxForInOrForOf.pnodeBody;
                    goto LLoop;
                case knopBlock:
                    node = node->sxBlock.pnodeStmt;
                    goto LLoop;
                case knopWith:
                    node = node->sxWith.pnodeBody;
                    goto LLoop;
                case knopSwitch:
                    node = node->sxSwitch.pnodeCases;
                    goto LLoop;
                case knopCase:
                case knopDefaultCase:
                    node = node->sxCase.pnodeBody;
                    goto LLoop;
                case knopTryCatch:
                    TraverseStatements(node->sxTryCatch.pnodeTry, handler);
                    node = node->sxTryCatch.pnodeCatch;
                    goto LLoop;
                case knopCatch:
                    node = node->sxCatch.pnodeBody;
                    goto LLoop;
                case knopTry:
                    node = node->sxTry.pnodeBody;
                    goto LLoop;
                case knopFinally:
                    node = node->sxFinally.pnodeBody;
                    goto LLoop;
                case knopTryFinally:
                    TraverseStatements(node->sxTryFinally.pnodeTry, handler);
                    node = node->sxTryFinally.pnodeFinally;
                    goto LLoop;
                }
            }
        }

        class List
        {
        public:
            // THandler prototype: void THandler(ParseNodePtr current)
            template <typename THandler>
            static void ForEach(ParseNodePtr list, THandler handler)
            {
                Traverse(list, [&](ParseNodePtr node) -> bool
                { 
                    handler(node); 
                    // Always continue
                    return true; 
                },
                false // includeListNodes
                );
            }

            // THandler prototype: bool THandler(ParseNodePtr current)
            template <typename THandler>
            static void Traverse(ParseNodePtr list, THandler handler, bool includeListNodes)
            {
                bool keepGoing = true;
                auto current = list;
                while(current && keepGoing)
                {
                    if(current->nop == knopList)
                    {
                        // Call handler with the child node
                        if(current->sxBin.pnode1)
                            keepGoing = handler(current->sxBin.pnode1);
                        // Call handler with the list node itself if required
                        if(keepGoing && includeListNodes)
                            keepGoing = handler(current);
                        // Advance to the next node
                        current = current->sxBin.pnode2;
                    }
                    else
                    {
                        // The last node
                        handler(current);
                        current = nullptr;
                    }
                }
            }

            static ParseNodePtr* Find(ParseNodePtr& current, ParseNodePtr nodeToFind);           
            static uint Count(ParseNodePtr nodes);
        };

        class Scope
        {
        public:
            template <typename THandler>
            static void ForEach(ParseNodePtr* ppnodeScopes, THandler handler)
            {
                Assert(ppnodeScopes);

                auto ppnodeNextScope = ppnodeScopes;
                while(*ppnodeNextScope)
                {
                    Assert(IsScopeNode(*ppnodeNextScope));
                    handler(ppnodeNextScope);
                    // The next scope could have been mutated in the last call so make sure we 
                    // check before we proceed
                    if (*ppnodeNextScope)
                        ppnodeNextScope = GetNextRef(*ppnodeNextScope);
                }
            }

            static bool IsScopeNode(ParseNodePtr node);
            static ParseNodePtr* GetScopesRef(ParseNodePtr scopeNode);
            static ParseNodePtr* GetNextRef(ParseNodePtr scopeNode);
            static uint GetNestedCount(ParseNodePtr scopeNode);
            static void SetNestedCount(Parser* parser, ParseNodePtr fncNode, uint nestedCount);
            static void Append(ParseNodePtr scopeNode, ParseNodePtr scopeToAppend);
            static void Prepend(ParseNodePtr scopeNode, ParseNodePtr scopeToPrepend);
            static void Remove(ParseNodePtr scopeNode, ParseNodePtr scopeToRemove);
            template <typename THandler>
            static void TraverseRecursive(ParseNodePtr pnodeScopeNode, ParseNodePtr pnodeParentFunction, THandler handler)
            {
                Assert(pnodeScopeNode && IsScopeNode(pnodeScopeNode));
                Assert(pnodeParentFunction == nullptr || (pnodeParentFunction->nop == knopProg || pnodeParentFunction->nop == knopFncDecl));

                if (pnodeScopeNode->nop == knopProg || pnodeScopeNode->nop == knopFncDecl)
                    pnodeParentFunction = pnodeScopeNode;

                ForEach(ASTHelpers::Scope::GetScopesRef(pnodeScopeNode), [&](ParseNodePtr* ppnodeNextScope){
                    handler(/*ppnodeCurrentScope*/ ppnodeNextScope, /*pnodeParentScope*/ pnodeScopeNode, /*pnodeParentFunction*/ pnodeParentFunction);
                    if (*ppnodeNextScope)
                        TraverseRecursive(*ppnodeNextScope, pnodeParentFunction, handler);
                });
            }

            template <typename THandler>
            static void TraverseBlocksExcept(ParseNodePtr pnodeScopeNode, ParseNodePtr exceptionScopeNode, THandler handler)
            {
                Assert(pnodeScopeNode && pnodeScopeNode->nop != knopFncDecl);
                Assert(IsScopeNode(exceptionScopeNode));
                
                ForEach(ASTHelpers::Scope::GetScopesRef(pnodeScopeNode), [&](ParseNodePtr* ppnodeNextScope) {
                    if (*ppnodeNextScope != exceptionScopeNode)
                    {
                        handler(*ppnodeNextScope);
                        if ((*ppnodeNextScope)->nop != knopFncDecl)
                            TraverseBlocksExcept(*ppnodeNextScope, exceptionScopeNode, handler);
                    }
                });
            }
        };
    };

    class PidCache
    {
        Parser*   _parser;
        IdentPtr _undefined;
        IdentPtr _eval;
    public:
        PidCache(Parser* parser) : _parser(parser), _undefined(nullptr), _eval(nullptr)
        {
            Assert(_parser);
        }
        virtual ~PidCache() { }
        IdentPtr undefined() { return GetPid(L"undefined", _undefined); }
        IdentPtr eval() 
        { 
            if(!_eval)
                GetPid(L"eval", _eval)->SetIsEval(); 
            return _eval;
        }
    protected:
        IdentPtr GetPid(LPCWSTR name, IdentPtr& pid)
        {
            if(!pid) pid = _parser->CreatePid(name, static_cast<charcount_t>(wcslen(name)));
            return pid;
        }
    };

    class ASTBuilderBase
    {
        Parser*      _parser;
        ParseNodePtr _fncNode;
        ParseNodePtr _currentScope;
        ParseNodePtr _blockNode;
        charcount_t _min;
        charcount_t _lim;
    protected:
        ASTBuilderBase(Parser* parser, ParseNodePtr fncNode, ParseNodePtr blockNode = nullptr);
        virtual ~ASTBuilderBase();
        virtual PidCache* GetPidCache() = 0;
    public:
        void SetFunction(ParseNodePtr fncNode);
        void SetScope(ParseNodePtr fncNode);
        void SetExtent(charcount_t min, charcount_t lim = 0);
        void SetExtentAs(ParseNodePtr node);
        charcount_t Min();
        charcount_t Lim();
        IdentPtr Pid(LPCWSTR value);
        ParseNodePtr Undefined();
        ParseNodePtr Assign(ParseNodePtr target, ParseNodePtr expr);
        ParseNodePtr Name(LPCWSTR name);
        ParseNodePtr Name(IdentPtr name);
        ParseNodePtr String(LPCWSTR value);
        ParseNodePtr String(IdentPtr value);
        ParseNodePtr Int(int value);
        ParseNodePtr Array(ParseNodePtr items);
        ParseNodePtr Object(ParseNodePtr membersList);
        ParseNodePtr Member(LPCWSTR name, ParseNodePtr value);
        ParseNodePtr Member(IdentPtr name, ParseNodePtr value);
        ParseNodePtr Dot(ParseNodePtr left, ParseNodePtr right);
        ParseNodePtr Call(ParseNodePtr target, ParseNodePtr list);
        ParseNodePtr New(ParseNodePtr target, ParseNodePtr list);
        ParseNodePtr Var(LPCWSTR name, ParseNodePtr initExpr);
        ParseNodePtr Var(IdentPtr name, ParseNodePtr initExpr);
        ParseNodePtr Let(LPCWSTR name, ParseNodePtr initExpr);
        ParseNodePtr Let(IdentPtr name, ParseNodePtr initExpr);
        ParseNodePtr Block(ParseNodePtr body = nullptr);
        ParseNodePtr If(ParseNodePtr cond, ParseNodePtr ifTrue, ParseNodePtr ifFalse);
        ParseNodePtr Return(ParseNodePtr expr);
        ParseNodePtr This();
        ParseNodePtr True();
        ParseNodePtr False();
        ParseNodePtr EndCode();
        ParseNodePtr Boolean(bool value);
        ParseNodePtr Optional(ParseNodePtr node);
        ParseNodePtr List(
            ParseNodePtr arg0 = nullptr, 
            ParseNodePtr arg1 = nullptr, 
            ParseNodePtr arg2 = nullptr, 
            ParseNodePtr arg3 = nullptr, 
            ParseNodePtr arg4 = nullptr, 
            ParseNodePtr arg5 = nullptr,
            ParseNodePtr arg6 = nullptr,
            ParseNodePtr arg7 = nullptr);
        ParseNodePtr Param(IdentPtr name);
        ParseNodePtr Function(LPCWSTR name, ParseNodePtr body, ParseNodePtr scopes, ParseNodePtr argList);
        void Append(ParseNodePtr statement);
        void Append(ParseNodePtr& node, ParseNodePtr nodeToAppend);
        void Prepend(ParseNodePtr nodeToPrepend);
        void Prepend(ParseNodePtr& node, ParseNodePtr nodeToPrepend);
        bool Replace(ParseNodePtr node, ParseNodePtr replaceWith);
        bool Insert(ParseNodePtr& listStart, ParseNodePtr after, ParseNodePtr nodeToInsert);
    private:
        ParseNodePtr ListNode(ParseNodePtr node1, ParseNodePtr node2);
        ParseNodePtr CallNode(OpCode nop, ParseNodePtr target, ParseNodePtr args);
        void ValidateTree();
    };

    template<typename TPidCache = PidCache>
    class ASTBuilder : public ASTBuilderBase
    {
        TPidCache    _defaultPidCache;
        TPidCache*   _pidCache;
    public:
        ASTBuilder(Parser* parser, ParseNodePtr fncNode, TPidCache* pids = nullptr, ParseNodePtr blockNode = nullptr)
            : ASTBuilderBase(parser, fncNode, blockNode), _defaultPidCache(parser)
        {
            _pidCache = pids ? pids : &_defaultPidCache;
        }
        TPidCache& Pids() { return *_pidCache; }
    private:
        PidCache* GetPidCache() override { return _pidCache; }
    };
}
