//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "StdAfx.h"
#ifdef EDIT_AND_CONTINUE

namespace Js
{
    SemanticChange::SemanticChangeList* SemanticChangeAnalyzer::AnalyzeSemanticChanges(EditAllocator* alloc, const ScriptDiff& diff)
    {
        SemanticChangeAnalyzer semanticChangeAnalyzer(alloc, diff);

        AutoAllocatorObjectPtr<SemanticChange::SemanticChangeList, EditAllocator> semanticChanges(
            SemanticChange::SemanticChangeList::New(alloc), alloc);
        semanticChangeAnalyzer.Analyze(semanticChanges);
        return semanticChanges.Detach();
    }

    SemanticChangeAnalyzer::SemanticChangeAnalyzer(EditAllocator* alloc, const ScriptDiff& diff) :
        m_diff(diff),
        m_functionEdits(alloc),
        m_functionIdToEditIndex(alloc),
        m_functionIdToNode(alloc)
    {
        // Old Utf8SourceInfo contains existing FunctionBodies. We just reparsed and need a way to associate
        // ParseNodes to existing FunctionBodies. One solution is to use local functionId. Figure out the delta:
        //
        //      oldFunctionBody.functionId + delta == parseNode.functionId
        //
        m_functionIdDelta = m_diff.OldTree().GetFunctionIdBegin() - OldRoot()->GetLocalFunctionId();
    }

    FunctionBody* SemanticChangeAnalyzer::OldRoot() const
    {
        const Utf8SourceInfo* oldUtf8SourceInfo = m_diff.OldTree().GetUtf8SourceInfo();
        ScriptDebugDocument* scriptDebugDocument = static_cast<ScriptDebugDocument*>(oldUtf8SourceInfo->GetScriptDebugDocument());
        if (!scriptDebugDocument)
        {
            Throw::OutOfMemory(); // scriptDebugDocument is null, likely previous Register failed
        }

        return scriptDebugDocument->GetScriptBody()->GetRootFunction();
    }

    //
    // Process edit script
    //
    void SemanticChangeAnalyzer::ProcessEdits()
    {
        m_diff.EditScript().Edits().Map([&](int, const Edit& edit)
        {
            switch (edit.Kind())
            {
            case EditKind::Insert:
                if (edit.NewNode()->nop == knopFncDecl)
                {
                    m_functionEdits.Add(edit);
                }
                break;

            case EditKind::Delete:
            case EditKind::Reorder:
            case EditKind::Update:
            case EditKind::Move:
                if (edit.OldNode()->nop == knopFncDecl)
                {
                    LocalFunctionId functionId = edit.OldNode()->sxFnc.functionId;
                    int editIndex = m_functionIdToEditIndex.Lookup(functionId, /*default*/-1);

                    if (editIndex < 0)
                    {
                        int editIndex = m_functionEdits.Add(edit);
                        m_functionIdToEditIndex.Add(functionId, editIndex);
                    }
                    else
                    {
                        // We may have multiple changes on a function (Reorder/Move + Update). When this happens record the change of higher priority.
                        const Edit& lastEdit = m_functionEdits.Item(editIndex);
                        if ((lastEdit.Kind() == EditKind::Reorder && edit.Kind() == EditKind::Update)   // Reorder, Update  ===> Update
                            || (lastEdit.Kind() == EditKind::Update && edit.Kind() == EditKind::Move))  // Update, Move     ===> Move
                        {
                            m_functionEdits.SetExistingItem(editIndex, edit);
                        }
                        else
                        {
                            Assert((lastEdit.Kind() == EditKind::Update && edit.Kind() == EditKind::Reorder)    // Update, Reorder  ===> Keep Update
                                || (lastEdit.Kind() == EditKind::Move && edit.Kind() == EditKind::Update));     // Move, Update     ===> Keep Move
                        }
                    }

                    // If move a nested funciton to top-level, simulate by a top-level Insert here. This guarantees an Insert of new top-level function.
                    if (edit.Kind() == EditKind::Move && IsTopLevelDeclaration(edit.NewNode()))
                    {
                        Assert(!IsTopLevelDeclaration(edit.OldNode()));
                        m_functionEdits.Add(Edit(EditKind::Insert, nullptr, edit.NewNode()));
                    }
                }
                break;
            }
        });
    }

    void SemanticChangeAnalyzer::MapOldTreeFunctionNodes()
    {
        EditAllocator* alloc = m_functionIdToNode.GetAllocator();

        // TODO: A better way to walk all function nodes
        FunctionTreeComparer<EditAllocator> comparer(alloc);
        comparer.MapTree(m_diff.OldTree().GetParseTree(), [&](ParseNodePtr x)
        {
            if (x->nop == knopFncDecl)
            {
                m_functionIdToNode.Add(x->sxFnc.functionId, x);
            }
        });
    }

    void SemanticChangeAnalyzer::Analyze(SemanticChange::SemanticChangeList* semanticChanges)
    {
        ProcessEdits();
        MapOldTreeFunctionNodes();

        m_nextAddFunctionEditIndex = 0;
        if (AreSameClosure(OldRoot(), m_diff.NewRoot())) // No top-level block scope difference
        {
            Analyze(semanticChanges, OldRoot());
        }

        CheckAddFunctionEdits(semanticChanges, m_functionEdits.Count()); // Finish Insert Function changes
    }

    //
    // Analyze changes in a "root" functionBody and its nested descendents. Called only when "root" and all its
    // ancestor functions have no closure changes.
    //
    // A child function can only be updated if its after-change closure environment is completely compatible with
    // existing captured closure environment in the runtime. We can walk up the tree and examine each parent/ancestor
    // function. If no closure change exists in the path, we can safely update this child function.
    //
    // Alternatively we can do this check top-down. "glob" function has no closure so all top-level functions can
    // be updated. When update a function, check if it has closure change. If yes, skipping its descendent functions
    // which can't be updated due to incompatible closure; If no closure change, recursively check its nested functions.
    //
    void SemanticChangeAnalyzer::Analyze(SemanticChange::SemanticChangeList* semanticChanges, FunctionBody* root)
    {
        MapNestedFunctions(root, [&](FunctionBody* top)
        {
            LocalFunctionId functionId = top->GetLocalFunctionId() + m_functionIdDelta;
            int editIndex = m_functionIdToEditIndex.Lookup(functionId, /*default*/-1);
            if (editIndex < 0)
            {
                // No edit found for this function. Move it and and all its nested functions to new source.
                MoveTree(semanticChanges, top);
            }
            else
            {
                // Check m_functionEdits and emit InsertFunction changes up to this editIndex. Try to emit changes in order.
                CheckAddFunctionEdits(semanticChanges, editIndex);

                // Examine corresponding function Edit.
                const Edit& edit = m_functionEdits.Item(editIndex);
                switch (edit.Kind())
                {
                case EditKind::Delete:
                case EditKind::Move:
                    // CONSIDER: Maybe skip this, we are not doing anything on it.
                    semanticChanges->Add(SemanticChange::DeleteFunction(edit.OldNode(), top));
                    break;

                case EditKind::Update:
                    AnalyzeUpdateTree(semanticChanges, top, edit);
                    break;

                case EditKind::Reorder:
                    MoveTree(semanticChanges, top);
                    break;

                default:
                    Assert(false);
                    break;
                }
            }
        });
    }

    //
    // Generate Move semantic changes to move a function and all its nested functions to new source.
    //
    void SemanticChangeAnalyzer::MoveTree(SemanticChange::SemanticChangeList* semanticChanges, FunctionBody* top)
    {
        MapTree(top, [=](FunctionBody* body)
        {
            LocalFunctionId functionId = body->GetLocalFunctionId() + m_functionIdDelta;
            ParseNodePtr oldNode = m_functionIdToNode.Item(functionId);
            ParseNodePtr newNode;
            if (m_diff.Match().TryGetPartnerInTree2(oldNode, &newNode))
            {
                semanticChanges->Add(SemanticChange::MoveFunction(oldNode, newNode, body));
            }
            else
            {
                AssertMsg(false, "Can't find function declaration match");
            }
        });
    }

    void SemanticChangeAnalyzer::AnalyzeUpdateTree(SemanticChange::SemanticChangeList* semanticChanges, FunctionBody* top, const Edit& edit)
    {
        semanticChanges->Add(SemanticChange::UpdateFunction(edit.OldNode(), edit.NewNode(), top));

        // Compare oldFunctionBody and newFunctionBody to detect closure differences.
        // If they have same closure, recursively check/move/update nested functions.
        // Otherwise no nested functions can update. (We might improve this in future. There can be nested functions that don't use closure and not affected.)
        //
        FunctionBody* newBody = m_diff.NewTree().GetUtf8SourceInfo()->FindFunction(edit.NewNode()->sxFnc.functionId);
        Assert(newBody);
        if (AreSameClosure(top, newBody))
        {
            Analyze(semanticChanges, top);
        }
    }

    bool SemanticChangeAnalyzer::AreSameClosure(FunctionBody* before, FunctionBody* after)
    {
        if (before->HasObjectRegister() != after->HasObjectRegister()   // Both use ScopeSlots, or both use ScopeObject?
            || before->scopeSlotArraySize != after->scopeSlotArraySize) // Same content in scopeSlotArray (apply to both ScopeSlot/ScopeObject)?
        {
            return false;
        }

        if (memcmp(before->GetPropertyIdsForScopeSlotArray(), after->GetPropertyIdsForScopeSlotArray(),
            sizeof(PropertyId) * before->scopeSlotArraySize) != 0)
        {
            return false;
        }

        return AreEquivalent(before->GetScopeObjectChain(), after->GetScopeObjectChain());
    }

    bool SemanticChangeAnalyzer::AreEquivalent(const ScopeObjectChain* before, const ScopeObjectChain* after)
    {
        if (before == nullptr || after == nullptr)
        {
            return before == after; // both nullptr?
        }

        if (before->pScopeChain->Count() != after->pScopeChain->Count())
        {
            return false;
        }

        for (int i = 0; i < before->pScopeChain->Count(); i++)
        {
            if (!AreEquivalent(before->pScopeChain->Item(i), after->pScopeChain->Item(i)))
            {
                return false;
            }
        }

        return true;
    }

    bool SemanticChangeAnalyzer::AreEquivalent(const DebuggerScope* before, const DebuggerScope* after)
    {
        if (before->scopeType != after->scopeType)
        {
            return false;
        }

        if (before->scopeProperties == nullptr || after->scopeProperties == nullptr)
        {
            return before->scopeProperties == after->scopeProperties; // both nullptr?
        }

        if (before->scopeProperties->Count() != after->scopeProperties->Count())
        {
            return false;
        }

        for (int k = 0; k < before->scopeProperties->Count(); k++)
        {
            const DebuggerScopeProperty& a = before->scopeProperties->Item(k);
            const DebuggerScopeProperty& b = after->scopeProperties->Item(k);

            if (a.propId != b.propId || a.flags != b.flags)
            {
                return false;
            }
        }

        return true;
    }

    //
    // Check that a FncDecl node represents a top-level (not nested, not in block-scope) function declaration (not function expression).
    //
    bool SemanticChangeAnalyzer::IsTopLevelDeclaration(ParseNodePtr node)
    {
        Assert(node->nop == knopFncDecl);
        return node->parent->nop == knopProg && node->sxFnc.IsDeclaration();
    }

    //
    // Check and emit any InsertFunction edits in m_functionEdits up to maxEditIndex.
    //
    void SemanticChangeAnalyzer::CheckAddFunctionEdits(SemanticChange::SemanticChangeList* semanticChanges, uint maxEditIndex)
    {
        while (m_nextAddFunctionEditIndex < maxEditIndex)
        {
            const Edit& edit = m_functionEdits.Item(m_nextAddFunctionEditIndex++);
            if (edit.Kind() == EditKind::Insert && IsTopLevelDeclaration(edit.NewNode())) // Only emit top-level InsertFunction
            {
                semanticChanges->Add(SemanticChange::InsertFunction(edit.NewNode()));
            }
        }
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void SemanticChange::Trace(const ScriptDiff& diff) const
    {
        // WARNING: Following names must be in sync with enum SemanticChangeKind
        static PCWSTR s_changeKinds[] = { L"None", L"InsertFunction", L"DeleteFunction", L"UpdateFunction", L"MoveFunction" };

        PCWSTR label = s_changeKinds[static_cast<int>(Kind())];
        LPCUTF8 oldSource = diff.OldTree().GetUtf8Source();
        LPCUTF8 newSource = diff.NewTree().GetUtf8Source();

        if (Kind() == SemanticChangeKind::InsertFunction)
        {
            diff.Trace(label, NewNode(), newSource);
        }
        else
        {
            diff.Trace(label, OldNode(), oldSource, NewNode(), newSource);
        }

        if (OldFunctionBody())
        {
            OUTPUT_TRACE(Phase::ENCPhase, L"    Old functionBody: %s\n", OldFunctionBody()->GetDisplayName());
        }
    }
#endif // ENABLE_DEBUG_CONFIG_OPTIONS

} // namespace Js

#endif
