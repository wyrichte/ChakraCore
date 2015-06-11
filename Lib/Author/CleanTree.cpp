//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{

    void CleanTree::CleanNames(ParseNode *pnode)
    {
        if (!pnode) return;
        pnode->location = NoRegister;
        switch (pnode->nop)
        {
        case knopList:
        case knopScope:
            CleanNames(pnode->sxBin.pnode1);
            CleanNames(pnode->sxBin.pnode2);
            break;
        case knopVarDecl:
            pnode->sxVar.sym = NULL;
            // NEXT : we need a better way to know/assert on which field we need to reset when we re-use the parse-tree for the next execution.
            pnode->sxVar.pid->SetPropertyId(Js::Constants::NoProperty);
            break;
        }
    }

    void CleanBlock(ParseNode *pnodeBlock)
    {
        if (pnodeBlock && pnodeBlock->nop == knopBlock)
            pnodeBlock->sxBlock.scope = nullptr;
    }

    bool CleanTree::Preorder(ParseNode *pnode, Context context)
    {
        pnode->location = NoRegister;
        pnode->emitLabels = false;
        switch (pnode->nop)
        {
        case knopStr:
        case knopName:
            pnode->sxPid.sym = NULL;
            pnode->sxPid.pid->SetPropertyId(Js::Constants::NoProperty);
            break;
        case knopVarDecl:
        case knopLetDecl:
        case knopConstDecl:
            pnode->sxVar.sym = NULL;
            pnode->sxVar.pid->SetPropertyId(Js::Constants::NoProperty);
            break;
        case knopFncDecl:
            //// Restore nestedCount. This was bashed by code-gen.
            pnode->sxFnc.nestedCount = context->LSExtension->NestedCount(pnode);
            if (pnode->sxFnc.pnodeNames)
                CleanNames(pnode->sxFnc.pnodeNames);
            // Intentional fall-through
            __fallthrough;
        case knopProg:            
            // The first block in the scopes of a function (or program) is a synthetic block
            // inserted in the tree just to hold the scope. It will not be seen in the traversal
            // of the tree so process it now. We use sxFnc because sxProg starts with an PnFnc so,
            // for anything in sxProg that is common with sxFnc you can use sxFnc.
            CleanBlock(pnode->sxFnc.pnodeScopes);
            break;
        case knopBlock:
            CleanBlock(pnode);
            break;

        // The following have a similar synthetic block to the function case above function. 
        // Explicitly clean them.
        case knopFor:
            CleanBlock(pnode->sxFor.pnodeBlock);
            break;
        case knopForIn:
        case knopForOf:
            CleanBlock(pnode->sxForInOrForOf.pnodeBlock);
            break;
        case knopSwitch:
            CleanBlock(pnode->sxSwitch.pnodeBlock);
            break;
        }
        return true;
    }

}