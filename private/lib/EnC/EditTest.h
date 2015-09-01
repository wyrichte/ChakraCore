//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once
#ifdef EDIT_AND_CONTINUE
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

namespace Js
{
    class ScriptParseTree;

    class EditTest
    {
    public:
        class EntryInfo
        {
        public:
            static Js::FunctionInfo LoadTextFile;
            static Js::FunctionInfo LCS;
            static Js::FunctionInfo Ast;
            static Js::FunctionInfo AstDiff;
        };

        static Var LoadTextFile(RecyclableObject* function, CallInfo callInfo, ...);
        static Var LCS(RecyclableObject* function, CallInfo callInfo, ...);
        static Var Ast(RecyclableObject* function, CallInfo callInfo, ...);
        static Var AstDiff(RecyclableObject* function, CallInfo callInfo, ...);

    private:
        static ScriptParseTree* Parse(EditAllocator* alloc, JavascriptString* str, ScriptContext* scriptContext);
        static Var ParseTreeToObject(ArenaAllocator* alloc, ParseNodePtr parseTree, ScriptContext* scriptContext);

        template <class Allocator, class TreeComparer>
        static DynamicObject* GenDiff(Allocator* alloc, TreeComparer comparer, ParseNodePtr rootA, ParseNodePtr rootB, ScriptContext* scriptContext);

        template <class Allocator>
        class AstDumpContext
        {
        private:
            struct NodeObj
            {
                ParseNodePtr node;
                DynamicObject* obj;

                NodeObj() {}
                NodeObj(ParseNodePtr node, DynamicObject* obj) : node(node), obj(obj) {}
            };
            typedef JsUtil::Stack<NodeObj, Allocator> NodeObjStack;

            Allocator* alloc;
            ScriptContext* scriptContext;
            NodeObjStack path;
            Var root;

            enum NodePropertyIds { TYPE, CH_MIN, CH_LIM, CHILDREN, MAX };
            PropertyId propertyIds[NodePropertyIds::MAX];

        public:
            AstDumpContext(Allocator* alloc, ScriptContext* scriptContext);

            Var GetRoot() const { return root; }
            Allocator* GetAllocator() const { return alloc; }
            ScriptContext* GetScriptContext() const { return scriptContext; }

            DynamicObject* NewObject(ParseNode* pnode) const;
            void Preorder(ParseNode* pnode);
            void Postorder(ParseNode* pnode);
        };

        template <class Allocator>
        class EditDumpContext
        {
        private:
            AstDumpContext<Allocator> astDumpContext;

            enum EditPropertyIds { KIND, OLD_NODE, NEW_NODE, MAX };
            PropertyId propertyIds[EditPropertyIds::MAX];

        public:
            EditDumpContext(Allocator* alloc, ScriptContext* scriptContext);

            DynamicObject* NewObject(ParseNode* pnode) const { return astDumpContext.NewObject(pnode); }
            DynamicObject* NewObject(const Edit<ParseNodePtr>& edit) const;
        };

    };

} // namespace Js

#endif
#endif
