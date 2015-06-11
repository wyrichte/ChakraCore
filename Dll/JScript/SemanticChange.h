//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once
#ifdef EDIT_AND_CONTINUE

namespace Js
{
    // Use an allocator alias to simplify if we want to change to a different allocator.
    typedef Recycler EditAllocator;
#define EditNew(alloc, T, ...) AllocatorNew(EditAllocator, alloc, T, __VA_ARGS__)
#define EditNewArray(alloc, T, count) AllocatorNewArray(EditAllocator, alloc, T, count)
#define EditNewArrayLeaf(alloc, T, count) AllocatorNewArrayLeaf(EditAllocator, alloc, T, count)
#define EditDelete(alloc, obj) AllocatorDelete(EditAllocator, alloc, obj)
#define EditDeleteArray(alloc, count, obj) AllocatorDeleteArray(EditAllocator, alloc, count, obj)

    class ScriptDiff;

    //-----------------------------------------------------------------------------
    // EnC semantic change kinds.
    //-----------------------------------------------------------------------------
    enum class SemanticChangeKind
    {
        None, // Invalid

        // Insert a new function.
        InsertFunction,

        // Delete an existing function.
        DeleteFunction,

        // Update an existing function.
        UpdateFunction,

        // Move an existing function, no real change.
        MoveFunction,
    };

    //-----------------------------------------------------------------------------
    // Represents a semantic change in EnC.
    //-----------------------------------------------------------------------------
    class SemanticChange
    {
    public:
        typedef TreeMatch<FunctionTreeComparer<EditAllocator>, EditAllocator> TreeMatchType;
        typedef EditScript<FunctionTreeComparer<EditAllocator>, EditAllocator> EditScriptType;
        typedef JsUtil::List<SemanticChange, EditAllocator, /*isLeaf*/true> SemanticChangeList;

    private:
        SemanticChangeKind m_kind;

        ParseNodePtr m_oldNode;
        ParseNodePtr m_newNode;
        FunctionBody* m_oldFunctionBody;

        SemanticChange(SemanticChangeKind kind, ParseNodePtr oldNode, ParseNodePtr newNode, FunctionBody* oldFunctionBody) :
            m_kind(kind), m_oldNode(oldNode), m_newNode(newNode), m_oldFunctionBody(oldFunctionBody)
        {}

    public:
        SemanticChange() :
            m_kind(SemanticChangeKind::None)
        {}

        static SemanticChange InsertFunction(ParseNodePtr newNode)
        {
            return SemanticChange(SemanticChangeKind::InsertFunction, /*oldNode*/nullptr, newNode, /*oldFunctionBody*/nullptr);
        }

        static SemanticChange DeleteFunction(ParseNodePtr oldNode, FunctionBody* oldFunctionBody)
        {
            return SemanticChange(SemanticChangeKind::DeleteFunction, oldNode, /*newNode*/nullptr, oldFunctionBody);
        }

        static SemanticChange UpdateFunction(ParseNodePtr oldNode, ParseNodePtr newNode, FunctionBody* oldFunctionBody)
        {
            return SemanticChange(SemanticChangeKind::UpdateFunction, oldNode, newNode, oldFunctionBody);
        }

        static SemanticChange MoveFunction(ParseNodePtr oldNode, ParseNodePtr newNode, FunctionBody* oldFunctionBody)
        {
            return SemanticChange(SemanticChangeKind::MoveFunction, oldNode, newNode, oldFunctionBody);
        }

        SemanticChangeKind Kind() const { return m_kind; }
        ParseNodePtr OldNode() const { return m_oldNode; }
        ParseNodePtr NewNode() const { return m_newNode; }
        FunctionBody* OldFunctionBody() const { return m_oldFunctionBody; }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        void Trace(const ScriptDiff& diff) const;
#endif
    };

    //-----------------------------------------------------------------------------
    // SemanticChangeAnalyzer analyzes semantic changes from ScriptDiff.
    //-----------------------------------------------------------------------------
    class SemanticChangeAnalyzer
    {
    private:
        typedef Edit<ParseNodePtr> Edit;

        const ScriptDiff& m_diff;

        // Note that we may have 3 different functionIds for the same function:
        //        Runtime:
        //            oldFunction->functionId        -- oldCompileFunctionId
        //        OldTree recompiled for diffing:
        //            oldNode->functionId            -- newCompileFunctionId
        //        NewTree
        //            newNode->functionId
        //
        // Map old-source functions between old compile and new compile:
        //        oldFunction->functionId + m_functionIdDelta == oldNode->functionId
        //
        int m_functionIdDelta;

        // list of FunctionEdit
        JsUtil::List<Edit, EditAllocator, /*isLeaf*/true> m_functionEdits;

        // oldNode->functionId  ->  index to m_functionEdits
        //      Used to find corresponding Edit (if any) for runtime functions.
        //      With Edit we have oldNode, newNode, and we can find corresponding newFunctionBody from newNode.
        JsUtil::BaseDictionary<LocalFunctionId, int, EditAllocator> m_functionIdToEditIndex;

        // oldNode->functionId  ->  oldNode
        //      Used to find corresponding oldNode for runtime functions when no Edit exists. e.g. not changed.
        //      With oldNode we can find matching newNode and corresponding newFunctionBody.
        JsUtil::BaseDictionary<LocalFunctionId, ParseNodePtr, ForceLeafAllocator<EditAllocator>::AllocatorType> m_functionIdToNode;

        // Next m_functionEdits index to check/emit InsertFunction.
        //      Used to order InsertFunction vs. other semantic changes.
        uint m_nextAddFunctionEditIndex;

    private:
        SemanticChangeAnalyzer(EditAllocator* alloc, const ScriptDiff& diff);

        FunctionBody* OldRoot() const;
        void ProcessEdits();
        void MapOldTreeFunctionNodes();
        void Analyze(SemanticChange::SemanticChangeList* semanticChanges);
        void Analyze(SemanticChange::SemanticChangeList* semanticChanges, FunctionBody* top);
        void MoveTree(SemanticChange::SemanticChangeList* semanticChanges, FunctionBody* top);
        void AnalyzeUpdateTree(SemanticChange::SemanticChangeList* semanticChanges, FunctionBody* top, const Edit& edit);
        void CheckAddFunctionEdits(SemanticChange::SemanticChangeList* semanticChanges, uint maxEditIndex);

        static bool AreSameClosure(FunctionBody* before, FunctionBody* after);
        static bool AreEquivalent(const ScopeObjectChain* before, const ScopeObjectChain* after);
        static bool AreEquivalent(const DebuggerScope* before, const DebuggerScope* after);
        static bool IsTopLevelDeclaration(ParseNodePtr node);

        //
        // Map direct nested functions of a parent function.
        //
        template <class Func>
        static void MapNestedFunctions(FunctionBody* parent, const Func& func)
        {
            for (uint i = 0; i < parent->GetNestedCount(); i++)
            {
                func(parent->GetNestedFunc(i)->GetFunctionBody());
            }
        }

        //
        // Map a function and all nested descendant functions.
        //
        template <class Func>
        static void MapTree(FunctionBody* root, const Func& func)
        {
            func(root);
            MapNestedFunctions(root, [&](FunctionBody* f)
            {
                MapTree(f, func);
            });
        }

    public:
        static SemanticChange::SemanticChangeList* AnalyzeSemanticChanges(EditAllocator* alloc, const ScriptDiff& diff);
    };

} // namespace Js

#endif
