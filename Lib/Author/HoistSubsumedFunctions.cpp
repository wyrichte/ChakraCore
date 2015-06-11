//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{

    static inline bool ContainsNode(ParseNodePtr node, ParseNodePtr searchNode)
    {
        return (node && searchNode && ContainsOffset(node, searchNode->ichMin) && ContainsOffset(node, searchNode->ichLim));
    }

    static ParseNodePtr* ChildrenListRef(ParseNodePtr node, ParseNodePtr targetChildNode)
    {
        Assert(node);
        switch(node->nop)
        {
        case knopFncDecl:
            return &node->sxFnc.pnodeBody;
        case knopWith:
            if (ContainsNode(node->sxWith.pnodeBody,targetChildNode))
                return &node->sxWith.pnodeBody;
            break;
        case knopCatch:
            if (ContainsNode(node->sxCatch.pnodeBody,targetChildNode))
                return &node->sxCatch.pnodeBody;
            break;
        case knopProg:
            return &node->sxProg.pnodeBody;
        case knopFor:
            if (ContainsNode(node->sxFor.pnodeBody,targetChildNode))
                return &node->sxFor.pnodeBody;
            break;
        case knopWhile:
        case knopDoWhile:
            if (ContainsNode(node->sxWhile.pnodeBody,targetChildNode))
                return &node->sxWhile.pnodeBody;
            break;
        case knopForIn:
        case knopForOf:
            if (ContainsNode(node->sxForInOrForOf.pnodeBody,targetChildNode))
                return &node->sxForInOrForOf.pnodeBody;
            break;
        case knopBlock:
            return &node->sxBlock.pnodeStmt;
        case knopCase:
        case knopDefaultCase:
            if (ContainsNode(node->sxCase.pnodeBody,targetChildNode))
                return &node->sxCase.pnodeBody;
            break;
        case knopFinally:
            return &node->sxFinally.pnodeBody;
        case knopTry:
            return &node->sxTry.pnodeBody;
        case knopIf:
            if (ContainsNode(node->sxIf.pnodeTrue,targetChildNode))
                return &node->sxIf.pnodeTrue;
            if (ContainsNode(node->sxIf.pnodeFalse,targetChildNode))
                return &node->sxIf.pnodeFalse;
            break;
        // Hoisting into those is not currently supported because they can have multiple edges:
        //case knopTryCatch:
        //case knopTryFinally:
        }
        return nullptr;
    }

    static void HoistFunction(ParseNodePtr parentFnc, ParseNodePtr containerNode, ParseNodePtr subsumedFnc, ArenaAllocator* alloc, ParseNodeTree* tree)
    {
        Assert(containerNode);
        Assert(parentFnc);
        Assert(parentFnc->nop == knopFncDecl || parentFnc->nop == knopProg);
        Assert(subsumedFnc);
        Assert(alloc);
        Assert(tree);

        ASTBuilder<> ast(tree->GetParser(), parentFnc);
        ParseNodePtr callNode = nullptr;

        ParseNodeCursor cursor(alloc, tree);
        if(cursor.SeekToOffset(subsumedFnc->ichMin))
        {
            // Find the parent call node
            callNode = cursor.Up(
                [&](ParseNodePtr node) { return node && node->nop == knopCall; }, 
                [&](ParseNodePtr node) { return node == containerNode; });
            if(callNode)
            {
                ParseNodePtr after = callNode;
                ParseNodePtr* to = nullptr;

                // Add 1 for RParen - required for parameter help
                charcount_t callNewLim = subsumedFnc->ichMin + 1;
                // Adjust the subsumed function min to start after the call.
                subsumedFnc->ichMin = callNewLim + 1;
                // Set the subsumed function lim to the endCode lim since its lim may be wrong due to error correction
                // and may actually overlap with nodes below causing us trouble to find them.
                ASTHelpers::List::ForEach(subsumedFnc->sxFnc.pnodeBody, [&] (ParseNodePtr current) 
                { 
                    if(current->nop == knopEndCode) 
                        subsumedFnc->ichLim = current->ichLim;
                });

                // Find the expression root node since we need to insert the hoisted function just after it.
                cursor.Up([&](ParseNodePtr node) -> bool 
                { 
                    bool stop = false;
                    OpCode nop = ParseNodeSerializerHelpers::ActualNop(node);
                    if (node && nop != knopList)
                    {
                        to = ChildrenListRef(node, after);

                        if(to)
                        {
                            // We can hoist into the node
                            stop = true;
                        }
                        else
                        {
                            after = node;
                        }
                    }
                    return stop;
                });
                Assert(to); // We were supposed to hit containerNode or one of its children

                // Defensive check to make sure the function insert will succeed
                if (to && *to)
                {
                    // Find the argument containing the function and replace it with undefined. 
                    // Make sure to set the extent so the 'undefined' name node min & lim would be within the callNode range.
                    ast.SetExtent(callNode->ichLim);
                    auto undefinedNode = ast.Undefined();
                    // Mark the new node to identify that it is added to the tree and does not represent user code
                    undefinedNode->grfpn |= PNodeFlags::fpnSyntheticNode; 
                    if (ast.Replace(subsumedFnc, undefinedNode))
                    {
                        // Insert the subsumed function just after the call (where it was supposed to be)
                        bool inserted = ast.Insert(*to, after, subsumedFnc);
                        Assert(inserted);

                        if (inserted)
                        {
                            // Trim the lim of all nodes before the function to make it not exceed callNewLim 
                            // so it won't overlap with the hoisted function range.
                            ASTHelpers::Visit(after, [&](ParseNodePtr pnode)->bool {
                                if (pnode->ichLim > callNewLim)
                                {
                                    if (pnode->ichMin > callNewLim)
                                        pnode->ichMin = callNewLim;
                                    pnode->ichLim = callNewLim;
                                }
                                return true;
                            });

                            // The parser treats function definitions and function expressions differently when it comes to scopes.
                            // The subsumed function will be assumed to be a function expression, and will be linked in the parent catch/with
                            // scope chain. Hoisting it requires moving it to the parent function scope as it becomes a function definition.
                            if (containerNode->nop == knopWith || containerNode->nop == knopCatch)
                            {
                                // Remove the function form the catch/with scope list
                                ASTHelpers::Scope::Remove(containerNode, subsumedFnc);
                                // Add the function to the parent function scope chain
                                ASTHelpers::Scope::Prepend(parentFnc, subsumedFnc);
                            }

                            // Make it a function declaration instead of expression
                            subsumedFnc->sxFnc.SetDeclaration(true);
                        }
                        else
                        {
                            // We could not insert the function in the expected location. Revert the original change, to avoid crashes
                            // later on in code generation
                            ast.Replace(undefinedNode, subsumedFnc);
                        }
                    }
                }
            }
        }
    }

    void HoistSubsumedFunctions::Apply(ArenaAllocator* alloc, ParseNodeTree* tree)
    {
        Assert(alloc);
        Assert(tree);

        ParseNodePtr pnodeProg = tree->TreeRoot();
        Assert(pnodeProg);
        Assert(pnodeProg->nop == knopProg);

        if (pnodeProg && pnodeProg->nop == knopProg)
        {
            ASTHelpers::Scope::TraverseRecursive(pnodeProg, nullptr, [&](ParseNodePtr* ppnodeCurrentScope, ParseNodePtr pnodeParentScope, ParseNodePtr pnodeParentFunction){
                ParseNodePtr current = *ppnodeCurrentScope;

                if (current && current->nop == knopFncDecl && current->sxFnc.IsSubsumed() && current->sxFnc.pnodeNames)
                {
                    // Hoist this function
                    HoistFunction(pnodeParentFunction, pnodeParentScope, current, alloc, tree);
                }
            });
        }
    }
 }