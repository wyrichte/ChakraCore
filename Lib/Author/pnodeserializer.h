//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

typedef enum ParseNodeEdge
{
#define PNELABEL(label,name,apne)  label,
#include "PNELABEL.h"
#undef PNELABEL
} ParseNodeEdge;

template <class Context>
struct SerializerPolicyBase
{
    typedef Context Context;

protected:
    inline void PreSerializeNodeListItem(ParseNodePtr pnode, bool isFirst, Context* context) {}
    inline bool PreSerializeCases(ParseNodePtr pnodeCases, ParseNodePtr pnodeDefault, Context* context) { return true;}
    inline void PostSerializeCases(ParseNodePtr pnodeCases, ParseNodePtr pnodeDefault, Context* context) {}
    inline bool PreSerializeArguments(ParseNodePtr pnodeArguments, Context* context) { return true; }
    inline void PostSerializeArguments(ParseNodePtr pnodeArguments, Context* context){}
    inline bool PreSerializeNode(ParseNodePtr pnode, ParseNodeEdge edgeLabel, Context* context) { return true; }
    inline void PostSerializeNode(ParseNodePtr pnode, ParseNodeEdge edgeLabel, Context* context) {}
    inline void SerializeInt(ParseNodePtr pnode, Context* context) {}
    inline void SerializeFlt(ParseNodePtr pnode, Context* context) {}
    inline void SerializeStr(ParseNodePtr pnode, Context* context) {}
    inline void SerializeRegExp(ParseNodePtr pnode, Context* context) {}
    inline void SerializeLeafNode(ParseNodePtr pnode, Context* context) {}
    inline void SerializeTargetLabel(ParseNodePtr pnodeTarget, Context* context) { }
    inline void SerializeName(IdentPtr id, Context* context) {}
    inline void SerializeNameNode(ParseNodePtr pnode, Context* context) {}
    inline void SerializeEmptyNode (ParseNodeEdge edgeLabel, Context* context) { }
};

class ParseNodeSerializerHelpers
{
public:
    static OpCode ActualNop(ParseNodePtr pnode)
    {
        Assert(pnode);

        OpCode actualNop =  pnode->nop;
        if (pnode->nop == knopDot && pnode->grfpn & PNodeFlags::fpnIndexOperator)
        {
            // dot operator is an index operator, the nop got changed as an optimization
            actualNop = knopIndex;
        }

        if (pnode->nop == knopList && pnode->grfpn & PNodeFlags::fpnDclList)
        {
            // introduce a new node kind for var, let, and const declaration lists
            if (pnode->sxBin.pnode1)
            {
                switch (pnode->sxBin.pnode1->nop)
                {
                case knopConstDecl:
                    actualNop = knopConstDeclList;
                    break;
                case knopLetDecl:
                    actualNop = knopLetDeclList;
                    break;
                default:
                    actualNop = knopVarDeclList;
                    break;
                }
            }
            else 
                actualNop = knopVarDeclList;
        }

        return actualNop;
    }

    static bool IsDeclList(OpCode nop)
    {
        switch (nop)
        {
        case knopConstDeclList:
        case knopLetDeclList:
        case knopVarDeclList:
            return true;
        }
        return false;
    }
};

template<class SerializerPolicy>
class ParseNodeSerializer : SerializerPolicy
{
    typedef typename SerializerPolicy::Context SerializerContext;

    void SerializeNodeList(ParseNodePtr pnodeList, SerializerContext* context)
    {
        Assert(pnodeList);
        Assert(context);

        OpCode listActualNop = ParseNodeSerializerHelpers::ActualNop(pnodeList);

        for (ParseNode *current = pnodeList; current; current = current->sxBin.pnode2)
        {
            OpCode actualNop = ParseNodeSerializerHelpers::ActualNop(current);
            if (actualNop == knopEndCode) 
            {
                break;
            }

            PreSerializeNodeListItem(current, current == pnodeList, context);

            // Skip list nodes and nested VarDeclList nodes
            if (actualNop == knopList || (ParseNodeSerializerHelpers::IsDeclList(actualNop) &&
                ParseNodeSerializerHelpers::IsDeclList(listActualNop)))
            {
                SerializeNode(current->sxBin.pnode1, ParseNodeEdge::pneListItem, context);
            }
            else
            {
                SerializeNode(current, ParseNodeEdge::pneListItem, context);
                break;
            }
        }
    }

    void SerializeCases(ParseNodePtr pnodeCases, ParseNodePtr pnodeDefault, SerializerContext* context)
    {
        bool continueSerialziation = PreSerializeCases(pnodeCases, pnodeDefault, context);

        if (!continueSerialziation)
        {
            return;
        }

        for (ParseNode *current = pnodeCases; current; current = current->sxCase.pnodeNext)
        {
            PreSerializeNodeListItem(current, current == pnodeCases, context);

            if (current == pnodeDefault)
            {
                // serialize the default case
                TemporaryAssignment<OpCode> t(current->nop, knopDefaultCase);
                SerializeNode(pnodeDefault, ParseNodeEdge::pneDefaultCase, context);
            }
            else
            {
                SerializeNode(current, ParseNodeEdge::pneCase, context);
            }
        }
        PostSerializeCases(pnodeCases, pnodeDefault, context);
    }

    void SerializeArguments(ParseNode *pnodeFnc, SerializerContext* context)
    {
        ParseNode *pnodeArguments = pnodeFnc->sxFnc.pnodeArgs;
        bool continueSerialziation = PreSerializeArguments(pnodeArguments, context);

        if (!continueSerialziation)
        {
            return;
        }

        for (ParseNode *current = pnodeArguments; current; current = current->sxVar.pnodeNext)
        {
           PreSerializeNodeListItem(current, current == pnodeArguments, context);
           SerializeNode(current, ParseNodeEdge::pneArgument, context);
        }

        if (pnodeFnc->sxFnc.pnodeRest != nullptr)
        {
            if (pnodeArguments == nullptr)
            {
                // There were no formals apart from rest, so we have to handle rest like the first argument.
                PreSerializeNodeListItem(pnodeFnc->sxFnc.pnodeRest, true, context);
            }
            pnodeArguments = pnodeFnc->sxFnc.pnodeRest;
            SerializeNode(pnodeArguments, ParseNodeEdge::pneArgument, context);
        }

        PostSerializeArguments(pnodeArguments, context);
    }

public:
    void SerializeNode(ParseNodePtr pnode, ParseNodeEdge edgeLabel, SerializerContext* context)
    {
        if (!ThreadContext::IsCurrentStackAvailable(PNODEVISIRORSIZE))
           return;

        if (pnode == NULL)
        {
            SerializeEmptyNode(edgeLabel, context);
            return;
        }

        TemporaryAssignment<OpCode> t(pnode->nop, ParseNodeSerializerHelpers::ActualNop(pnode));

        bool continueSerialziation = PreSerializeNode(pnode, edgeLabel, context);

        if (!continueSerialziation)
        {
            return;
        }

        switch (pnode->nop)
        {
            case knopName:
                SerializeNameNode(pnode, context);
                break;
            case knopStr: 
                SerializeStr(pnode, context);
                break;
            case knopInt:
                SerializeInt(pnode, context);
                break;
            case knopFlt:
                SerializeFlt(pnode, context);
                break;
            case knopRegExp:
                SerializeRegExp(pnode, context);
                break;
            case knopThis:
            case knopNull:
            case knopFalse:
            case knopTrue:
            case knopEmpty:
            case knopEndCode:
            case knopDebugger:
                SerializeLeafNode(pnode, context);
                break;

            case knopBreak:
            case knopContinue:
                if (pnode->sxJump.hasExplicitTarget && pnode->sxJump.pnodeTarget)
                {
                    SerializeTargetLabel(pnode->sxJump.pnodeTarget, context);
                }
                break;

            case knopLabel:
                Assert(false);  // label nodes are not created by the parser and should not be encountered here
                break;

            case knopNot:
            case knopNeg:
            case knopPos:
            case knopLogNot:
            case knopIncPre:
            case knopDecPre:
            case knopTypeof:
            case knopVoid:
            case knopDelete:
            case knopIncPost:
            case knopDecPost:
                SerializeNode(pnode->sxUni.pnode1, ParseNodeEdge::pneOperand, context);
                break;

            case knopArray:
                SerializeNode(pnode->sxUni.pnode1, ParseNodeEdge::pneElements, context);
                break;

            case knopObject:
                SerializeNode(pnode->sxUni.pnode1, ParseNodeEdge::pneMembers, context);
                break;

            case knopGetMember:
            case knopSetMember:
                SerializeNode(pnode->sxBin.pnode1, ParseNodeEdge::pneTarget, context);
                SerializeNode(pnode->sxBin.pnode2, ParseNodeEdge::pneValue, context);
                break;

            case knopAdd:
            case knopSub:
            case knopMul:
            case knopDiv:
            case knopMod:
            case knopOr:
            case knopXor:
            case knopAnd:
            case knopEq:
            case knopNe:
            case knopLt:
            case knopLe:
            case knopGe:
            case knopGt:
            case knopDot:
            case knopAsg:
            case knopInstOf:
            case knopIn:
            case knopEqv:
            case knopNEqv:
            case knopComma:
            case knopLogOr:
            case knopLogAnd:
            case knopLsh:
            case knopRsh:
            case knopRs2:
            case knopAsgAdd:
            case knopAsgSub:
            case knopAsgMul:
            case knopAsgDiv:
            case knopAsgMod:
            case knopAsgAnd:
            case knopAsgXor:
            case knopAsgOr:
            case knopAsgLsh:
            case knopAsgRsh:
            case knopAsgRs2:
            case knopScope:
                SerializeNode(pnode->sxBin.pnode1, ParseNodeEdge::pneLeft, context);
                SerializeNode(pnode->sxBin.pnode2, ParseNodeEdge::pneRight, context);
                break;

            case knopMember:
            case knopMemberShort:
                SerializeNode(pnode->sxBin.pnode1, ParseNodeEdge::pneTarget, context);
                SerializeNode(pnode->sxBin.pnode2, ParseNodeEdge::pneMember, context);
                break;

            case knopCall:
            case knopNew:
                SerializeNode(pnode->sxCall.pnodeTarget, ParseNodeEdge::pneTarget, context);
                SerializeNode(pnode->sxCall.pnodeArgs, ParseNodeEdge::pneArguments, context);
                break;

            case knopIndex:
                SerializeNode(pnode->sxCall.pnodeTarget, ParseNodeEdge::pneTarget, context);
                SerializeNode(pnode->sxCall.pnodeArgs, ParseNodeEdge::pneValue, context);
                break;

           case knopQmark:
               SerializeNode(pnode->sxTri.pnode1, ParseNodeEdge::pneCondition, context);
               SerializeNode(pnode->sxTri.pnode2, ParseNodeEdge::pneThen, context);
               SerializeNode(pnode->sxTri.pnode3, ParseNodeEdge::pneElse, context);
               break;

            case knopConstDeclList:
            case knopLetDeclList:
            case knopList:
            case knopVarDeclList:
                SerializeNodeList(pnode, context);
                break;

            case knopConstDecl:
            case knopLetDecl:
            case knopVarDecl:
                SerializeName(pnode->sxVar.pid, context);
                SerializeNode(pnode->sxVar.pnodeInit, ParseNodeEdge::pneInitialization, context);
                break;

            case knopFncDecl:
                if (pnode->sxFnc.pid)
                {
                    SerializeName(pnode->sxFnc.pid, context);
                }
                if (pnode->sxFnc.pnodeArgs  != nullptr || pnode->sxFnc.pnodeRest != nullptr)
                {
                    SerializeArguments(pnode, context);
                }
                else
                {
                    SerializeEmptyNode(ParseNodeEdge::pneArguments, context);
                }
                if (pnode->sxFnc.pnodeBody && pnode->sxFnc.pnodeBody->nop != knopEndCode)
                {
                    SerializeNode(pnode->sxFnc.pnodeBody, ParseNodeEdge::pneBody, context);
                }
                else
                {
                    SerializeEmptyNode(ParseNodeEdge::pneBody, context);
                }
                break;

            case knopProg:
                if (pnode->sxProg.pnodeBody && pnode->sxProg.pnodeBody->nop != knopEndCode)
                {
                    SerializeNode(pnode->sxProg.pnodeBody, ParseNodeEdge::pneBody, context);
                }
                else
                {
                    SerializeEmptyNode(ParseNodeEdge::pneBody, context);
                }
                break;

            case knopFor:
                SerializeNode(pnode->sxFor.pnodeInit, ParseNodeEdge::pneInitialization, context);
                SerializeNode(pnode->sxFor.pnodeCond, ParseNodeEdge::pneCondition, context);
                SerializeNode(pnode->sxFor.pnodeIncr, ParseNodeEdge::pneIncrement, context);
                SerializeNode(pnode->sxFor.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopIf:
                SerializeNode(pnode->sxIf.pnodeCond, ParseNodeEdge::pneCondition, context);
                SerializeNode(pnode->sxIf.pnodeTrue, ParseNodeEdge::pneThen, context);
                SerializeNode(pnode->sxIf.pnodeFalse, ParseNodeEdge::pneElse, context);
                break;

            case knopWhile:
                SerializeNode(pnode->sxWhile.pnodeCond, ParseNodeEdge::pneCondition, context);
                SerializeNode(pnode->sxWhile.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopDoWhile:
                SerializeNode(pnode->sxWhile.pnodeBody, ParseNodeEdge::pneBody, context);
                SerializeNode(pnode->sxWhile.pnodeCond, ParseNodeEdge::pneCondition, context);
                break;

            case knopForIn:
            case knopForOf:
                SerializeNode(pnode->sxForInOrForOf.pnodeLval, ParseNodeEdge::pneVariable, context);
                SerializeNode(pnode->sxForInOrForOf.pnodeObj, ParseNodeEdge::pneObject, context);
                SerializeNode(pnode->sxForInOrForOf.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopReturn:
                SerializeNode(pnode->sxReturn.pnodeExpr, ParseNodeEdge::pneValue, context);
                break;

            case knopBlock:
                SerializeNode(pnode->sxBlock.pnodeStmt, ParseNodeEdge::pneBlockBody, context);
                break;

            case knopWith:
                SerializeNode(pnode->sxWith.pnodeObj, ParseNodeEdge::pneObject, context);
                SerializeNode(pnode->sxWith.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopSwitch:
                SerializeNode(pnode->sxSwitch.pnodeVal, ParseNodeEdge::pneValue, context);
                SerializeCases(pnode->sxSwitch.pnodeCases, pnode->sxSwitch.pnodeDefault, context);
                break;

            case knopCase:
                SerializeNode(pnode->sxCase.pnodeExpr, ParseNodeEdge::pneValue, context);
                SerializeNode(pnode->sxCase.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopDefaultCase:
                SerializeNode(pnode->sxCase.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopTryFinally:
                SerializeNode(pnode->sxTryFinally.pnodeTry, ParseNodeEdge::pneTry, context);
                SerializeNode(pnode->sxTryFinally.pnodeFinally, ParseNodeEdge::pneFinally, context);
                break;

            case knopFinally:
                SerializeNode(pnode->sxFinally.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopCatch:
                SerializeNode(pnode->sxCatch.pnodeParam, ParseNodeEdge::pneVariable, context);
                SerializeNode(pnode->sxCatch.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopTryCatch:
                SerializeNode(pnode->sxTryCatch.pnodeTry, ParseNodeEdge::pneTry, context);
                SerializeNode(pnode->sxTryCatch.pnodeCatch, ParseNodeEdge::pneCatch, context);
                break;

            case knopTry:
                SerializeNode(pnode->sxTry.pnodeBody, ParseNodeEdge::pneBody, context);
                break;

            case knopThrow:
                SerializeNode(pnode->sxUni.pnode1, ParseNodeEdge::pneValue, context);
                break;

            case knopClassDecl:
                if (pnode->sxClass.isDeclaration)
                {
                    Assert(pnode->sxClass.pnodeDeclName && pnode->sxClass.pnodeDeclName->nop == knopLetDecl);
                    SerializeName(pnode->sxClass.pnodeDeclName->sxVar.pid, context);
                }
                SerializeNode(pnode->sxClass.pnodeExtends, ParseNodeEdge::pneExtends, context);
                SerializeNode(pnode->sxClass.pnodeConstructor, ParseNodeEdge::pneCtor, context);
                SerializeNode(pnode->sxClass.pnodeStaticMembers, ParseNodeEdge::pneStaticMembers, context);
                SerializeNode(pnode->sxClass.pnodeMembers, ParseNodeEdge::pneMembers, context);
                break;

            case knopStrTemplate:
                SerializeNode(pnode->sxStrTemplate.pnodeStringLiterals, ParseNodeEdge::pneStringLiterals, context);
                SerializeNode(pnode->sxStrTemplate.pnodeSubstitutionExpressions, ParseNodeEdge::pneSubstitutionExpression, context);
                SerializeNode(pnode->sxStrTemplate.pnodeStringRawLiterals, ParseNodeEdge::pneStringRawLiterals, context);
                break;
        };

        PostSerializeNode(pnode, edgeLabel, context);
    }
};