//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    ParseNodePtr RewriteTreeHelpers::CreateStrNode(Parser *parser, LPCWSTR value, charcount_t min, charcount_t lim)
    {
        auto node = parser->CreateStrNode(parser->CreatePid(value, wcslen(value)));
        node->ichMin = min;
        node->ichLim = lim;
        return node;
    }

    ParseNodePtr RewriteTreeHelpers::CreateNameNode(Parser *parser, LPCWSTR name, charcount_t min, charcount_t lim)
    {
        return parser->CreateNameNode(parser->CreatePid(name, wcslen(name)), min, lim);
    }

    ParseNodePtr RewriteTreeHelpers::CreateIntNode(Parser *parser, int value, charcount_t min, charcount_t lim)
    {
        auto node = parser->CreateIntNode(value);
        node->ichMin = min;
        node->ichLim = lim;
        return node;
    }

    void RewriteTreeHelpers::AddMember(Parser *parser, ParseNodePtr pnodeObject, LPCWSTR memberName, ParseNodePtr memberValue)
    {
        Assert(pnodeObject);
        Assert(!String::IsNullOrEmpty(memberName));
        Assert(memberValue);
        auto min = memberValue->ichMin;
        auto lim = memberValue->ichLim;
        auto nameNode = RewriteTreeHelpers::CreateStrNode(parser, memberName, min, lim);
        auto memberNode = parser->CreateBinNode(knopMember, nameNode, memberValue, min, lim);
        if(pnodeObject->sxUni.pnode1 == nullptr)
        {
            pnodeObject->sxUni.pnode1 = memberNode;
        }
        else
        {
            auto prevRoot = pnodeObject->sxUni.pnode1;
            auto listNode = parser->CreateBinNode(knopList, memberNode, prevRoot, prevRoot->ichMin, prevRoot->ichLim);
            pnodeObject->sxUni.pnode1 = listNode;
        }
    }

    ParseNodePtr RewriteTreeHelpers::CreateVarDeclNode(Parser *parser, ParseNodePtr pnodeParentFnc, IdentPtr pid, charcount_t ichMin, charcount_t ichLim)
    {
        Assert(parser);
        Assert(pnodeParentFnc);
        Assert(pid);

        ParseNodePtr pnodeVarDecl = parser->CreateNode(knopVarDecl, ichMin, ichLim);
        pnodeVarDecl->sxVar.InitDeclNode(NULL, NULL);
        pnodeVarDecl->sxVar.pid = pid;

        // link the new var in the parent function
        pnodeVarDecl->sxVar.pnodeNext = pnodeParentFnc->sxFnc.pnodeVars;
        pnodeParentFnc->sxFnc.pnodeVars = pnodeVarDecl;

        return pnodeVarDecl;
    }

    ParseNodePtr RewriteTreeHelpers::RememberNodeInto(Parser *parser, ParseNodePtr pnodeParent, ParseNodePtr pnode, IdentPtr ident)
    {
        Assert(parser);
        Assert(pnodeParent);
        Assert(pnode);
        Assert(ident);

        ParseNodePtr pnodeName = parser->CreateNameNode(ident, pnode->ichMin, pnode->ichMin);
        ParseNodePtr pnodeAsg = parser->CreateBinNode(knopAsg, pnodeName, pnode, ActualMin(pnode), ActualLim(pnode));

        Assert(pnodeParent->nop == knopFncDecl || pnodeParent->nop == knopProg);
        CreateVarDeclNode(parser, pnodeParent, ident, pnode->ichMin, pnode->ichLim);

        return pnodeAsg;
    }

    struct PropagateCleanup : VisitorPolicyBase<NullType>
    {
        bool Preorder(ParseNodePtr pnode, Context context)
        {
            if (pnode)
            {
                switch (pnode->nop)
                {
                case knopBreak:
                case knopContinue:
                case knopReturn:
                    pnode->sxStmt.grfnop |= fnopCleanup;
                    break;
                }
            }

            return true;
        }

        static void Apply(ParseNodePtr pnode)
        {
            ParseNodeVisitor<PropagateCleanup> visitor;
            visitor.Visit(pnode);
        }
    };

    void RewriteTreeHelpers::FixupOuters(ParseNodePtr pnode, ParseNodePtr outer)
    {
        if (pnode)
        {
            // Fix-up the pnodeOuter of a nested statement nodes.
            switch(pnode->nop)
            {
            case knopBlock:
            case knopBreak:
            case knopContinue:
            case knopWhile:
            case knopWith:
            case knopIf:
            case knopForIn:
            case knopForOf:
            case knopFor:
            case knopSwitch:
            case knopReturn:
            case knopTryFinally:
            case knopTryCatch:
                pnode->sxStmt.pnodeOuter = outer;
                break;
            case knopList:
                FixupOuters(pnode->sxBin.pnode1, outer);
                FixupOuters(pnode->sxBin.pnode2, outer);
                break;
            }
        }
    }

    ParseNodePtr RewriteTreeHelpers::RemoveEndCode(ParseNodePtr pnode)
    {
        if (pnode)
        {
            switch (pnode->nop)
            {
            case knopEndCode:
                pnode = nullptr;
                break;
            case knopList:
                ParseNodePtr right = pnode->sxBin.pnode2;
                if (right && right->nop == knopEndCode)
                    pnode = pnode->sxBin.pnode1;
                else
                    pnode->sxBin.pnode2 = RemoveEndCode(right);
                break;
            }
        }
        return pnode;
    }

    ParseNodePtr RewriteTreeHelpers::AppendEndCode(Parser *parser, ParseNodePtr pnode, charcount_t ichMin, charcount_t ichLim)
    {
        Assert(parser);

        if (pnode)
        {
            switch (pnode->nop)
            {
            case knopList:
                pnode->sxBin.pnode2 = AppendEndCode(parser, pnode->sxBin.pnode2, ichMin, ichLim);
                break;
            default:
                {
                    ParseNodePtr endCode = parser->CreateNode(knopEndCode, ichMin, ichLim);
                    ParseNodePtr pnodeT = parser->CreateBinNode(knopList, pnode, endCode, ichMin, ichLim);
                    pnode = pnodeT;
                }
                break;
            }
        }
        else
            pnode = parser->CreateNode(knopEndCode, ichMin, ichLim);
        return pnode;
    }

    void RewriteTreeHelpers::EnsureExecutionOf(Parser *parser, ParseNodePtr pnodeFnc, ParseNodePtr pnodeStmt)
    {
        Assert(parser);
        Assert(pnodeFnc);
        Assert(pnodeFnc->nop == knopFncDecl || pnodeFnc->nop == knopProg);
        // The resulting try should look something like:
        // {Block {TryFinally {Try pnode->sxFnc.pnodeBody} {Finally pnodeStmt}}}
        // The surrounding block is necessary because that is what the parser would have generated.
        ParseNodePtr block = parser->CreateBlockNode();
        block->sxBlock.pnodeOuter = NULL;
        ParseNodePtr tryFinally = parser->CreateNode(knopTryFinally);
        tryFinally->sxTryFinally.pnodeOuter = block;
        block->sxBlock.pnodeStmt = tryFinally;
        ParseNodePtr tryNode = parser->CreateNode(knopTry);
        tryNode->sxTry.pnodeOuter = tryFinally;
        tryFinally->sxTryFinally.pnodeTry = tryNode;
        ParseNodePtr body = RemoveEndCode(pnodeFnc->sxFnc.pnodeBody);
        tryNode->sxTry.pnodeBody = body;
        PropagateCleanup::Apply(body);
        if (body) 
        {
            auto min = body->ichMin; 
            if(pnodeFnc->nop == knopProg) 
            {
                // Adjust Prog to start from 0 offset to ensure we're not loosing any comments.
                pnodeFnc->ichMin = min = 0; 
            }
            else
            {
                auto LCurly = parser->GetLanguageServiceExtension()->LCurly(pnodeFnc);
                // LCurly might be missing in case of error correction.
                min = LCurly ? LCurly : min;
            }
            auto lim = body->ichLim;
            tryNode->ichMin = min;
            tryNode->ichLim = lim;
            tryFinally->ichMin = min;
            tryFinally->ichLim = lim;
            block->ichMin = min;
            block->ichLim = lim;
        }
        FixupOuters(body, tryNode);
        ParseNodePtr finallyNode = parser->CreateNode(knopFinally);
        finallyNode->ichMin = pnodeStmt->ichMin;
        finallyNode->ichLim = pnodeStmt->ichLim;
        finallyNode->sxFinally.pnodeOuter = tryFinally;
        tryFinally->sxTryFinally.pnodeFinally = finallyNode;
        finallyNode->sxFinally.pnodeBody = pnodeStmt;
        pnodeFnc->sxFnc.pnodeBody = AppendEndCode(parser, block, block->ichMin, block->ichLim);
    }

}
