//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    bool IdentifierContext::Abort()
    {
        return this->fileAuthoring->IsHurryCalled();
    }

    void IdentifierContext::AddPid(IdentPtr pid, AuthorCompletionKind kind, AuthorCompletionFlags flags, HintInfo* hintInfo)
    {
        if (pid)
        {
            IdentInfo info(pid, kind, flags, hintInfo);
            Add(info);
        }
    }

    void IdentifierContext::Add(IdentInfo info) 
    {  
        completions->AddUnique(info.kind, info.group, info.ident, nullptr, info.hintInfo);
    }

    IdentPtr IdentifierPreorderAddPolicy::RightNameOf(ParseNode *pnode)
    {
        if (pnode)
        {
            switch (pnode->nop)
            {
            case knopName:
                return pnode->sxPid.pid;
            case knopDot:
                return RightNameOf(pnode->sxBin.pnode2);
            }
        }
        return NULL;
    }

    void IdentifierPreorderAddPolicy::AddStrs(ParseNode *pnode, AuthorCompletionKind kind, IdentifierContext* context)
    {
        if (pnode)
            switch (pnode->nop)
            {
            case knopStr:
                context->AddPid(pnode->sxPid.pid, kind, acfFileIdentifiersFilter, nullptr);
                break;
            case knopList:
                for (ParseNode *current = pnode; pnode; pnode = pnode->sxBin.pnode2)
                {
                    ParseNode *item = current->nop == knopList ? current->sxBin.pnode1 : current;

                    AddStrs(item, kind, context);

                    if (current->nop != knopList) break;
                }
            }
    }

    bool IdentifierPreorderAddPolicy::Preorder(ParseNode *pnode, IdentifierContext* context)
    {
        if (context->Abort()) 
            return false;

        if (pnode->ichMin > 0 || pnode->ichLim > pnode->ichMin)
            switch(pnode->nop)
            {
            case knopName:
                context->AddPid(pnode->sxPid.pid, ackIdentifier, acfFileIdentifiersFilter, nullptr);
                break;
            case knopVarDecl:
            case knopLetDecl:
            case knopConstDecl:
                {
                    auto hintInfo = Anew(context->Alloc(), HintInfo, ascopeUnknown, atUnknown, context->FileId(), pnode->ichMin);
                    context->AddPid(pnode->sxVar.pid, ackVariable, acfFileIdentifiersFilter, hintInfo);
                    break;
                }
            case knopFncDecl:
                {
                    auto hintInfo = Anew(context->Alloc(), HintInfo, ascopeUnknown, atFunction, context->FileId(), pnode->ichMin);
                    context->AddPid(pnode->sxFnc.pid, ackMethod, acfFileIdentifiersFilter, hintInfo);

                    // Ensure all the variables parameters are added with a parameter kind.
                    for (ParseNodePtr parameter = pnode->sxFnc.pnodeArgs; parameter; parameter = parameter->sxVar.pnodeNext)
                    {
                        context->AddPid(parameter->sxVar.pid, ackParameter, acfFileIdentifiersFilter, nullptr);
                    }
                    if (pnode->sxFnc.pnodeRest != nullptr)
                    {
                        context->AddPid(pnode->sxFnc.pnodeRest->sxVar.pid, ackParameter, acfFileIdentifiersFilter, nullptr);
                    }

                    // Ensure all vars are added before the any potential field access to ensure we avoid the case
                    // a = 1; var a = 2; 
                    // Introducing a as an identifier instead of variable.
                    for (ParseNodePtr variable = pnode->sxFnc.pnodeVars; variable; variable = variable->sxVar.pnodeNext)
                        this->Preorder(variable, context);

                    break;
                }
            case knopDot:
                if (pnode->sxBin.pnode2 && pnode->sxBin.pnode2->nop == knopName)
                    context->AddPid(pnode->sxBin.pnode2->sxPid.pid, ackField, acfFileIdentifiersFilter, nullptr);
                break;
            case knopCatch:
                {
                    if (pnode->sxCatch.pnodeParam && pnode->sxCatch.pnodeParam->nop == knopName)
                    {
                        auto hintInfo = Anew(context->Alloc(), HintInfo, ascopeUnknown, atUnknown, context->FileId(), pnode->sxCatch.pnodeParam->ichMin);
                        context->AddPid(pnode->sxCatch.pnodeParam->sxPid.pid, ackVariable, acfFileIdentifiersFilter, hintInfo);
                    }
                    break;
                }
            case knopCall:
                context->AddPid(RightNameOf(pnode->sxCall.pnodeTarget), ackMethod, acfFileIdentifiersFilter, nullptr);
                break;
            case knopMember:
                AddStrs(pnode->sxBin.pnode1, ackField, context);
                break;
            }
        return true;
    }
 
}