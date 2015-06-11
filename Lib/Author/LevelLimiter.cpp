#include "stdafx.h"

namespace Authoring
{
    // Maximum tree depth that can be supported by the Code Generator
#ifdef DEBUG
    // In debug, more space is required
    static const int KB_FOR_1000_LEVELS = 400;
#else
    static const int KB_FOR_1000_LEVELS = 150;
#endif
    // The calculation: KB_FOR_1000_LEVELS for every 1000 levels.
    static const int MAX_TREE_DEPTH = (LS_MAX_STACK_SIZE_KB * 1000) / KB_FOR_1000_LEVELS;

    bool LevelLimiter::Preorder(ParseNode *pnode, LevelLimiterContext *context)
    {
        (context->level)++;
        if (context->level >= MAX_TREE_DEPTH)
        {
            switch (pnode->nop)
            {
            case knopName:
            case knopFncDecl:
            case knopWith:
            case knopCatch:
                // Ignore nodes that can contribute to scope. They are threaded in the
                // tree and cannot be removed simply by changning their nop.
                break;

                // Ignore other blocks that are conditionally scopes.
            case knopFor:
                if (pnode->sxFor.pnodeBlock)
                    break;
                goto LDefault;

            case knopForIn:
            case knopForOf:
                if (pnode->sxForInOrForOf.pnodeBlock)
                    break;
                goto LDefault;

            case knopSwitch:
                if (pnode->sxSwitch.pnodeBlock)
                    break;
                goto LDefault;

            case knopBlock:
                if (pnode->sxBlock.HasBlockScopedContent())
                    break;
                goto LDefault;

            default:
LDefault:
                pnode->nop = knopNull;
                context->treeTruncated = true;
                break;
            }
        }
        return true;
    }

    void LevelLimiter::Postorder(ParseNode *pnode, LevelLimiterContext* context)
    {
        (context->level)--;
    }
}