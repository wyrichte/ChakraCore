//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    namespace Names
    {
        const wchar_t callLss[] =L"_$callLss";
        const wchar_t Error[] = L"Error";
        const wchar_t wrapForInOrForOfTarget[] = L"_$wrapForInOrForOfTarget";
        const wchar_t nextMethod[] = L"next";
    }
    
    bool ContainsNode(ParseNode *node, ParseNode* target)
    {
        bool found = false;
        if (node)
        {
            ASTHelpers::Visit(node, [&] (ParseNodePtr current) -> bool 
            {
                if (current == target)
                {
                    found = true;
                    return false;
                }
                return !found;
            });
        }
        return found;
    }

    bool DirectExecution::TargetIn(ParseNode *pnode, ParseNode *target)
    {
        return OffsetIn(pnode, target->ichMin) || 
            // Manufactured nodes, such as the scope record, have zero width and OffsetIn reports them as
            // not being contained in their parent we need to traverse the tree target is at the
            // end of pnode.
            (pnode && target->ichMin == target->ichLim && ActualLim(pnode) == target->ichMin && ContainsNode(pnode, target));
    }

    bool DirectExecution::OffsetInScopes(ParseNode* current, charcount_t offset)
    {
        while (current)
        {
#if DEBUG
            ParseNode *startingValue = current;
#endif
            switch (current->nop)
            {
            case knopFncDecl:
                if (OffsetIn(current, offset) && current->sxFnc.IsDeclaration()) return true;
                current = current->sxFnc.pnodeNext;
                break;
            case knopBlock:
                if (OffsetIn(current, offset) && OffsetInScopes(current->sxBlock.pnodeScopes, offset)) return true;
                current = current->sxBlock.pnodeNext;
                break;
            case knopWith:
                if (OffsetIn(current, offset) && OffsetInScopes(current->sxWith.pnodeScopes, offset)) return true;
                current = current->sxWith.pnodeNext;
                break;
            case knopCatch:
                if (OffsetIn(current, offset) && OffsetInScopes(current->sxCatch.pnodeScopes, offset)) return true;
                current = current->sxCatch.pnodeNext;
                break;
            
            default:
                AssertMsg(false, "Unexpected opcode in tree of scopes");
                __assume(false);
            }
#if DEBUG
            AssertMsg(current != startingValue, "Loop didn't advance");
#endif
        }

        return false;
    }

    bool DirectExecution::OffsetInNestedFnc(Context context)
    {
        ParseNode *pnodeFnc = context->pnodeFnc;
        charcount_t offset = context->target->ichMin;
        if (!pnodeFnc) return true;
        Assert(pnodeFnc->nop == knopFncDecl || pnodeFnc->nop == knopProg);
        return OffsetInScopes(pnodeFnc->sxFnc.pnodeScopes, offset);
    }

    /// Force the execution of Catch statements, through rewriting the tree.
    /// We will force throwing a javascript exception - which will trigger the catch body execution
    ///
    /// A try/catch statement like this:
    ///     try {
    ///            sxTry.pnodeBody
    ///     }
    ///     catch (e) {
    ///            sxCatch.pnodeBody
    ///     }
    ///
    /// Should be transformed to:
    ///     try {
    ///              sxTry.pnodeBody
    ///              _$forceCatchException(new Error())
    ///     }
    ///     catch (e) {
    ///            sxCatch.pnodeBody
    ///     }
    ///
    /// by _$forceCatchException(new Error()) - we are trying to mimic 'throw new Error();' as Language service have suppressed all 'throw's 
    /// so we need to call helper function do that.

    void DirectExecution::ForceCatch(Parser* parser, ParseNode* pnodeTryCatch, ParseNode* pnodeParentFunction)
    {
        Assert(parser);
        Assert(pnodeTryCatch);
        Assert(pnodeParentFunction && (pnodeParentFunction->nop == knopFncDecl || pnodeParentFunction->nop == knopProg));

        ParseNode* pnodeTry = pnodeTryCatch->sxTryCatch.pnodeTry;

        Assert(pnodeTry != nullptr && pnodeTryCatch->sxTryCatch.pnodeCatch != nullptr);

        // locate the try body
        ParseNode** pnodeTryBody = nullptr;
        ParseNode* pnodeParentScope = nullptr;
        if (pnodeTry->sxTry.pnodeBody && pnodeTry->sxTry.pnodeBody->nop == knopBlock)
        {
            auto tryBlock = pnodeTry->sxTry.pnodeBody;
            pnodeTryBody = &tryBlock->sxBlock.pnodeStmt;

            pnodeParentScope = tryBlock; 
        }
        AssertMsg(pnodeTryBody, "ForceCatch couldn't find the try body");
        if (pnodeTryBody == nullptr)
        {
            // If we cannot find the try body it is better to return no results than crash.
            return;
        }

        charcount_t tryBodyIchLim = ActualLim(pnodeTry);

        ASTBuilder<> ast(parser, pnodeParentFunction);
        ast.SetExtent(tryBodyIchLim, tryBodyIchLim);

        // constructing - _$forceCatchException(new Error())
        auto pnodeCallNode = ast.Call(
            ast.Name(Names::forceCatchException),
            ast.New(ast.Name(Names::Error), nullptr));

        if (*pnodeTryBody != nullptr)
        {
            ast.SetExtent(pnodeTry->ichMin, tryBodyIchLim);
            *pnodeTryBody = ast.List(*pnodeTryBody, pnodeCallNode);
        }
        else
        {
            *pnodeTryBody = pnodeCallNode;
        }
    }

    bool DirectExecution::Preorder(ParseNode* pnode, Context context)
    {
        ParseNode* target = context->target;

        switch (pnode->nop)
        {
        case knopIf:
            if (TargetIn(pnode->sxIf.pnodeTrue, target) && !OffsetInNestedFnc(context))
            {
                pnode->sxIf.pnodeCond = ForcedExpr(context->parser, pnode->sxIf.pnodeCond, knopTrue);
            }
            else if (TargetIn(pnode->sxIf.pnodeFalse, target) && !OffsetInNestedFnc(context))
            {
                pnode->sxIf.pnodeCond = ForcedExpr(context->parser, pnode->sxIf.pnodeCond, knopFalse);
            }
            break;

        case knopQmark:
            if (TargetIn(pnode->sxTri.pnode2, target) && !OffsetInNestedFnc(context))
            {
                pnode->sxTri.pnode1 = ForcedExpr(context->parser, pnode->sxTri.pnode1, knopTrue);
            }
            else if (TargetIn(pnode->sxTri.pnode3, target) && !OffsetInNestedFnc(context))
            {
                pnode->sxTri.pnode1 = ForcedExpr(context->parser, pnode->sxTri.pnode1, knopFalse);
            }
            break;

        // case knopDoWhile: Do while will already execute at least once

        case knopWhile:
            if (TargetIn(pnode->sxWhile.pnodeBody, target) && !OffsetInNestedFnc(context))
            {
                pnode->sxWhile.pnodeCond = ForcedExpr(context->parser, pnode->sxWhile.pnodeCond, knopTrue);
            }
            break;

        case knopFor:
            if (pnode->sxFor.pnodeCond && TargetIn(pnode->sxFor.pnodeBody, target) && !OffsetInNestedFnc(context))
            {
                pnode->sxFor.pnodeCond = ForcedExpr(context->parser, pnode->sxFor.pnodeCond, knopTrue);
            }
            break;

        case knopForIn:
        case knopForOf:
            if (TargetIn(pnode->sxForInOrForOf.pnodeBody, target) && !OffsetInNestedFnc(context))
            {
                charcount_t min = pnode->sxForInOrForOf.pnodeObj->ichMin;
                charcount_t lim = pnode->sxForInOrForOf.pnodeObj->ichLim;
                pnode->sxForInOrForOf.pnodeObj = context->parser->CreateCallNode(
                    knopCall, 
                    context->parser->CreateNameNode(CreatePidFromLiteral(context->parser, Names::wrapForInOrForOfTarget), min, lim),
                    pnode->sxForInOrForOf.pnodeObj, min, lim);
            }
            break;

        case knopYield:
        case knopYieldLeaf:
            // For yield expressions we want to evaluate the expression but not return that value for the assignment expression.
            // So make it a void expression which is also a Unary node.
            if (pnode->sxUni.pnode1)
            {
                pnode->nop = knopVoid;
            }
            else
            {
                pnode->nop = knopEmpty;
            }
            break;
        case knopYieldStar:
            if (pnode->sxUni.pnode1)
            {
                // In case of yield* we have to make sure that the iterator is executed at least once, so insert a call node to call next() method on it.
                ParseNodePtr iterableNode = pnode->sxUni.pnode1;
                pnode->nop = knopVoid;
                pnode->sxUni.pnode1 = context->parser->CreateCallNode(knopCall,
                    context->parser->CreateBinNode(knopDot,
                        iterableNode,
                        context->parser->CreateNameNode(CreatePidFromLiteral(context->parser, Names::nextMethod))),
                        nullptr);
                pnode->ichLim = pnode->sxUni.pnode1->ichLim;
            }
            break;
        case knopReturn:
            // Ignore return statements that might terminate the function containing the offset early
            // unless the return statement itself contains the offset.
            if (!TargetIn(pnode, target) && TargetIn(context->pnodeFnc, target) && !OffsetInNestedFnc(context))
            {
                if (pnode->sxReturn.pnodeExpr)
                {
#if kcbPnReturn < kcbPnBin
#error This code assumes PnReturn is at least as large as PnBin
#endif
                    // Convert the node to a list with the expression as right and empty as left. This leaves
                    // the expression in the tree (as an expression statement) but removes the knopReturn
                    ParseNodePtr pnodeExpr = pnode->sxReturn.pnodeExpr;
                    pnode->nop = knopList;
                    pnode->sxBin.pnode1 = pnodeExpr;
                    auto empty = context->parser->CreateNode(knopEmpty);
                    ApplyLocation(empty, pnodeExpr->ichLim);
                    pnode->sxBin.pnode2 = empty;

                }
                else
                    pnode->nop = knopEmpty;
            }
            break;

        case knopBreak:
        case knopContinue:
            // Ignore break and continues that might terminate the loop early
            if (!TargetIn(pnode, target) && pnode->sxJump.pnodeTarget && TargetIn(pnode->sxJump.pnodeTarget, target) && !OffsetInNestedFnc(context))
            {
                pnode->nop = knopEmpty;
            }
            break;

        case knopLogAnd:
            if (TargetIn(pnode->sxBin.pnode2, target) && !OffsetInNestedFnc(context))
            {
                pnode->sxBin.pnode1 = ForcedExpr(context->parser, pnode->sxBin.pnode1, knopTrue);
            }
            break;

        case knopLogOr:
            if (TargetIn(pnode->sxBin.pnode2, target) && !OffsetInNestedFnc(context))
            {
                pnode->sxBin.pnode1 = ForcedExpr(context->parser, pnode->sxBin.pnode1, knopFalse);
            }
            break;

        case knopTryCatch:
            if (pnode->sxTryCatch.pnodeCatch && TargetIn(pnode->sxTryCatch.pnodeCatch, target))
            {
                ForceCatch(context->parser, pnode, context->pnodeFnc);
            }
            break;

        case knopSwitch:
            // If it is a switch statement, if the value of the switch contains the offset, do nothing.
            // Change the case expressions to ((<expr> && false) || {}) (forcing them to not match) 
            // unless the offset is in the case expresion then just stop, it will be executed.
            // Make the case the default, the expression will be ignored if it has one but we need
            // to keep the expression in the tree to avoid confusing the byte-code generator if the expression
            // contains a function expression.
            {
                charcount_t offset = target->ichMin;
                if (TargetIn(pnode, target) && (charcount_t)ActualLim(pnode->sxSwitch.pnodeVal) < offset)
                {
                    ParseNodePtr offsetCase = nullptr;
                    for (ParseNodePtr current = pnode->sxSwitch.pnodeCases; current; current = current->sxCase.pnodeNext)
                    {
                        Assert(current->nop == knopCase);

                        // Consider everything from the start of the case to the beginning of the next case as in the case, or
                        // if this is the last case, it must be the one we want.
                        if (!current->sxCase.pnodeNext || (charcount_t)ActualMin(current->sxCase.pnodeNext) >= offset)
                        {
                            offsetCase = current;
                            break;
                        }
                    }

                    if (offsetCase)
                    {
                        for (ParseNodePtr current = pnode->sxSwitch.pnodeCases; current; current = current->sxCase.pnodeNext)
                        {
                            Assert(current->nop == knopCase);
                            if (current == offsetCase)
                            {
                                if (current->sxCase.pnodeExpr && OffsetIn(current->sxCase.pnodeExpr, offset))
                                    goto LSwitchDone;
                            }
                            else
                            {
                                auto originalExpr = current->sxCase.pnodeExpr;
                                if (originalExpr)
                                {
                                    auto falseNode = context->parser->CreateNode(knopFalse, originalExpr->ichLim, originalExpr->ichLim);
                                    current->sxCase.pnodeExpr = 
                                        context->parser->CreateBinNode(knopLogOr,
                                            context->parser->CreateBinNode(knopLogAnd, originalExpr, falseNode, originalExpr->ichMin, originalExpr->ichLim), 
                                            context->parser->CreateUniNode(knopObject, nullptr, originalExpr->ichLim, originalExpr->ichLim), originalExpr->ichMin, originalExpr->ichLim);
                                }
                                else
                                    current->sxCase.pnodeExpr = context->parser->CreateUniNode(knopObject, nullptr, current->ichMin, current->ichMin);
                            }
                        }
                        pnode->sxSwitch.pnodeDefault = offsetCase;
                    }
                }
            }
LSwitchDone:
            break;

        case knopProg:
        case knopFncDecl:
            context->stack->Push(context->pnodeFnc);
            context->pnodeFnc = pnode;
            break;
        }

        return true;
    }

    void DirectExecution::Postorder(ParseNode* pnode, Context context)
    {
        switch (pnode->nop)
        {
        case knopProg:
            {
                ParseNodePtr *ppnodeEndCode = FindEndCodeRef(pnode);
                if (ppnodeEndCode)
                {
                    Parser *parser = context->parser;
                    IdentPtr name = CreatePidFromLiteral(parser, Names::callLss);
                    ParseNodePtr nameNode = parser->CreateNameNode(name, pnode->ichLim, pnode->ichLim);
                    ParseNodePtr pnodeCallLs = parser->CreateCallNode(knopCall, nameNode, nullptr, pnode->ichLim, pnode->ichLim);
                    ParseNodePtr link = parser->CreateBinNode(knopList, pnodeCallLs, *ppnodeEndCode);
                    *ppnodeEndCode = link;
                }
            }
            break;

        case knopFncDecl:
            context->pnodeFnc = context->stack->Pop();
            break;
        }
    }

}