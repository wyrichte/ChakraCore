//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    namespace Names
    {
        const wchar_t guardForInOrForOfTarget[] = L"_$guardForInOrForOfTarget";
    }

    int uniqueNumber = 0;

    ParseNode *AddLoopGuards::GuardExpression(Parser *parser, ParseNode *pnodeParent, ParseNode *expr)
    {
        Assert(pnodeParent);

        ParseNode *guard =
            parser->CreateBinNode(knopLogOr,
                parser->CreateNameNode(CreatePidFromLiteral(parser, LOOP_GUARD_DISABLED_FLAG)),
                parser->CreateBinNode(knopLt, 
                    parser->CreateUniNode(knopIncPre, parser->CreateNameNode(NewGuardName(parser))),
                    parser->CreateNameNode(CreatePidFromLiteral(parser, LOOP_GUARD_LIMIT))));

        ApplyLocation(guard, expr ? expr->ichLim : pnodeParent->ichMin);
        if (expr)
            guard = parser->CreateBinNode(knopLogAnd, expr, guard);
        return guard;
    }

    ParseNode *AddLoopGuards::ForInOrForOfGuard(Parser *parser, ParseNode *pnodeParent, ParseNode *expr)
    {
        IdentPtr guard = NewGuardName(parser);
        ParseNodePtr guardName = parser->CreateStrNode(guard);
        guardName->ichMin = expr->ichLim;
        guardName->ichLim = expr->ichLim;
        return parser->CreateCallNode(knopCall, 
            parser->CreateNameNode(CreatePidFromLiteral(parser, Names::guardForInOrForOfTarget), expr->ichMin, expr->ichMin),
            parser->CreateBinNode(knopList, expr, guardName));
    }

    bool AddLoopGuards::Preorder(ParseNode* pnode, Parser *parser)
    {
        switch (pnode->nop)
        {
        case knopWhile:
        case knopDoWhile:
            pnode->sxWhile.pnodeCond = GuardExpression(parser, pnode, pnode->sxWhile.pnodeCond);
            break;
        case knopFor:
            pnode->sxFor.pnodeCond = GuardExpression(parser, pnode, pnode->sxFor.pnodeCond);
            break;
        case knopForIn:
        case knopForOf:
            pnode->sxForInOrForOf.pnodeObj = ForInOrForOfGuard(parser, pnode, pnode->sxForInOrForOf.pnodeObj);
        }
        return true;
    }

}
