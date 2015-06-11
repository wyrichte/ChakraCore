//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
#ifdef DEBUG
    bool TreeValidator::ChildLocationValidator::WalkChild(ParseNode *pnode, Context context) 
    { 
        if (pnode && pnode->nop != knopEndCode)
        {
            auto min = ActualMin(context);
            auto lim = ActualLim(context);

            if (ActualMin(pnode) >= min && ActualLim(pnode) <= lim)
            {
                return true;
            }

            return false;
        }
        return true;
    } 

    bool TreeValidator::Preorder(ParseNode *pnode, Context context)
    {
        if (pnode)
        {
            // Validate that the children of the node are within the parent.
            ParseNodeWalker<ChildLocationValidator> walker;
            Assert(walker.Walk(pnode, pnode));

            // Validate nestedCount
            if (pnode->nop == knopProg || pnode->nop == knopFncDecl)
            {
                // Function must be deferred (e.g. pnodeBody == nullptr) or the nested count must be correct.
                // Parameter scope and function body scope will both be walked by the GetNestedCount call.
                Assert(pnode->sxFnc.pnodeBody == nullptr || pnode->sxFnc.nestedCount == ASTHelpers::Scope::GetNestedCount(pnode->sxFnc.GetTopLevelScope()));
            }

            if (m_validateCleanTree)
            {
                // Validate that the work done in CleanTree was performed and wasn't 
                // corrupted.
                switch (pnode->nop)
                {
                case knopStr:
                case knopName:
                    Assert(pnode->sxPid.pid);
                    Assert(!pnode->sxPid.sym);
                    break;
                case knopVarDecl:
                case knopLetDecl:
                case knopConstDecl:
                    Assert(pnode->sxVar.pid);
                    Assert(!pnode->sxVar.sym);
                    break;
                }
            }
        }
        return true;
    }

#endif

}