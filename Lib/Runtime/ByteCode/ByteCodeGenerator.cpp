//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "BackEndAPI.h"

// TODO (yongqu) get rid of pragma warning
#pragma warning(disable:4242)
#pragma warning(disable:4244)

const wchar_t *SymbolTypeNames[]={L"Function",L"Variable",L"MemberName",L"Formal",L"Unknown"};


ParseNode* InvertLoop(ParseNode* outerLoop,ByteCodeGenerator* byteCodeGenerator,FuncInfo* funcInfo);
extern void PrintPnode(ParseNode*);
void PreVisitBlock(ParseNode *pnodeBlock, ByteCodeGenerator *byteCodeGenerator);
void PostVisitBlock(ParseNode *pnodeBlock, ByteCodeGenerator *byteCodeGenerator);
bool EmitUseBeforeDeclaration(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo);
void EmitUseBeforeDeclarationRuntimeError(ByteCodeGenerator *byteCodeGenerator, Js::RegSlot location, bool fLoadUndef = true);

#ifdef LANGUAGE_SERVICE
#define RECORD_LOAD(p) byteCodeGenerator->Writer()->RecordNodeLoad(p)
#else
#define RECORD_LOAD(p)
#endif

bool CallTargetIsArray(ParseNode *pnode)
{
    return pnode->nop == knopName && pnode->sxPid.PropertyIdFromNameNode() == Js::PropertyIds::Array;
}

BOOL MayHaveSideEffectOnNode(ParseNode *pnode, ParseNode *pnodeSE)
{
    // Try to determine whether pnodeSE may kill the named var represented by pnode.

    if (pnode->nop != knopName)
    {
        // Only investigating named vars here.
        return false;
    }

    uint fnop = ParseNode::Grfnop(pnodeSE->nop);
    if (fnop & fnopLeaf)
    {
        // pnodeSE is a leaf and can't kill anything.
        return false;
    }

    if (fnop & fnopAsg)
    {
        // pnodeSE is an assignment (=, ++, +=, etc.)
        // I found that trying to examine the LHS of pnodeSE caused small SS regressions (??),
        // maybe because of code layout or some other subtle effect.
        return true;
    }

    if (fnop & fnopUni)
    {
        // pnodeSE is a unary op, so recurse to the source (if present - e.g., [] may have no opnd).
        if (pnodeSE->nop==knopTempRef) {
            return false;
        }
        else {
            return pnodeSE->sxUni.pnode1 && MayHaveSideEffectOnNode(pnode, pnodeSE->sxUni.pnode1);
        }
    }
    else if (fnop & fnopBin)
    {
        // pnodeSE is a binary (or ternary) op, so recurse to the sources (if present).
        if (pnodeSE->nop == knopQmark)
        {
            return MayHaveSideEffectOnNode(pnode, pnodeSE->sxTri.pnode1) ||
                MayHaveSideEffectOnNode(pnode, pnodeSE->sxTri.pnode2) ||
                MayHaveSideEffectOnNode(pnode, pnodeSE->sxTri.pnode3);
        }
        else if (pnodeSE->nop == knopCall || pnodeSE->nop == knopNew)
        {
            return MayHaveSideEffectOnNode(pnode, pnodeSE->sxCall.pnodeTarget) ||
                (pnodeSE->sxCall.pnodeArgs && MayHaveSideEffectOnNode(pnode, pnodeSE->sxCall.pnodeArgs));
        }
        else
        {
            return MayHaveSideEffectOnNode(pnode, pnodeSE->sxBin.pnode1) ||
                (pnodeSE->sxBin.pnode2 && MayHaveSideEffectOnNode(pnode, pnodeSE->sxBin.pnode2));
        }
    }
    else if (pnodeSE->nop == knopList)
    {
        return true;
    }

    return false;
}

bool IsCallOfConstants(ParseNode *pnode)
{
    return pnode->sxCall.callOfConstants && pnode->sxCall.argCount > ByteCodeGenerator::MinArgumentsForCallOptimization;
}

Js::PropertyId Symbol::EnsurePosition(ByteCodeGenerator* byteCodeGenerator)
{
    // Guarantee that a symbol's name has a property ID mapping.
    if (this->position == Js::Constants::NoProperty)
    {
        this->position = this->EnsurePositionNoCheck(byteCodeGenerator->TopFuncInfo());
    }
    return this->position;
}

Js::PropertyId Symbol::EnsurePosition(FuncInfo *funcInfo)
{
    // Guarantee that a symbol's name has a property ID mapping.
    if (this->position == Js::Constants::NoProperty)
    {
        this->position = this->EnsurePositionNoCheck(funcInfo);
    }
    return this->position;
}

Js::PropertyId Symbol::EnsurePositionNoCheck(FuncInfo *funcInfo)
{
    return funcInfo->byteCodeFunction->GetOrAddPropertyIdTracked(this->GetName());
}

void Symbol::SaveToPropIdArray(Symbol *sym, Js::PropertyIdArray *propIds, ByteCodeGenerator *byteCodeGenerator, Js::PropertyId *pFirstSlot /* = null */)
{
    if (sym)
    {
        Js::PropertyId slot = sym->scopeSlot;
        if (slot != Js::Constants::NoProperty)
        {
            Assert((uint32)slot < propIds->count);
            propIds->elements[slot] = sym->EnsurePosition(byteCodeGenerator);
            if (pFirstSlot && !sym->GetIsArguments())
            {
                if (*pFirstSlot == Js::Constants::NoProperty ||
                    *pFirstSlot > slot)
                {
                    *pFirstSlot = slot;
                }
            }
        }
    }
}

template<class TContext>
ParseNode* UpdateListNode(ParseNode *pnode,ByteCodeGenerator* byteCodeGenerator,TContext* context,ParseNode* (*mutator)(ParseNode*,ByteCodeGenerator* byteCodeGenerator,TContext*)) {
    ParseNode* originalPnode=pnode;
    if ((pnode!=NULL)&&(pnode->nop==knopList)) {
        ParseNode* prev=NULL;
        while(pnode->nop==knopList) {
            pnode->sxBin.pnode1=UpdateNode<TContext>(pnode->sxBin.pnode1,byteCodeGenerator,context,mutator);
            prev=pnode;
            pnode = pnode->sxBin.pnode2;
        }
        Assert(prev!=NULL);
        prev->sxBin.pnode2=UpdateNode<TContext>(pnode,byteCodeGenerator,context,mutator);
    }
    return originalPnode;
}

// This is incomplete; for now, use only for trees that do not contain with or try statements
template<class TContext>
ParseNode* UpdateNode(ParseNode *pnode,ByteCodeGenerator* byteCodeGenerator,TContext* context,ParseNode* (*mutator)(ParseNode*,ByteCodeGenerator*,TContext*)) {
    if (pnode==NULL)
        return NULL;

    ThreadContext::ProbeCurrentStackNoDispose(Js::Constants::MinStackByteCodeVisitor, byteCodeGenerator->GetScriptContext());

    // change current node before its children

    pnode=mutator(pnode,byteCodeGenerator,context);

    switch (pnode->nop) {
    default:
        {
            uint flags=ParseNode::Grfnop(pnode->nop);
            if (flags&fnopUni) {
                pnode->sxUni.pnode1=UpdateNode(pnode->sxUni.pnode1,byteCodeGenerator,context,mutator);
            }
            else if (flags&fnopBin) {
                pnode->sxBin.pnode1=UpdateNode(pnode->sxBin.pnode1,byteCodeGenerator,context,mutator);
                pnode->sxBin.pnode2=UpdateNode(pnode->sxBin.pnode2,byteCodeGenerator,context,mutator);
            }
        }
        break;

    case knopCall:
        pnode->sxCall.pnodeTarget=UpdateNode(pnode->sxCall.pnodeTarget,byteCodeGenerator,context,mutator);
        pnode->sxCall.pnodeArgs=UpdateNode(pnode->sxCall.pnodeArgs,byteCodeGenerator,context,mutator);
        break;

    case knopNew:
        {
            pnode->sxCall.pnodeTarget=UpdateNode(pnode->sxCall.pnodeTarget,byteCodeGenerator,context,mutator);
            if (!IsCallOfConstants(pnode))
            {
                pnode->sxCall.pnodeArgs=UpdateNode(pnode->sxCall.pnodeArgs,byteCodeGenerator,context,mutator);
            }
            break;
        }

    case knopQmark:
        pnode->sxTri.pnode1=UpdateNode(pnode->sxTri.pnode1,byteCodeGenerator,context,mutator);
        pnode->sxTri.pnode2=UpdateNode(pnode->sxTri.pnode2,byteCodeGenerator,context,mutator);
        pnode->sxTri.pnode3=UpdateNode(pnode->sxTri.pnode3,byteCodeGenerator,context,mutator);
        break;
    case knopList:
        return UpdateListNode(pnode,byteCodeGenerator,context,mutator);
        //PTNODE(knopVarDecl    , "varDcl"    ,None    ,Var  ,fnopNone)
    case knopVarDecl:
    case knopConstDecl:
    case knopLetDecl:
        if (pnode->sxVar.pnodeInit!=NULL)
            pnode->sxVar.pnodeInit=UpdateNode(pnode->sxVar.pnodeInit,byteCodeGenerator,context,mutator);
        break;
        //PTNODE(knopFncDecl    , "fncDcl"    ,None    ,Fnc  ,fnopLeaf)
    case knopFncDecl:
        {
            // Inner function declarations are visited before anything else in the scope.
            // (See UpdateNodeFunctionsInScope.)
            ParseNode *pnodeNames = pnode->sxFnc.pnodeNames;
            if (pnodeNames)
            {
                while (pnodeNames->nop == knopList)
                {
                    if (pnodeNames->sxBin.pnode1->nop == knopScope ||
                        pnodeNames->sxBin.pnode1->nop == knopDot)
                    {
                        pnodeNames->sxBin.pnode1=UpdateNode(pnodeNames->sxBin.pnode1,byteCodeGenerator,context,mutator);
                    }
                    pnodeNames = pnodeNames->sxBin.pnode2;
                }
                if (pnodeNames->nop == knopScope ||
                    pnodeNames->nop == knopDot)
                {
                    pnode->sxFnc.pnodeNames=UpdateNode(pnode->sxFnc.pnodeNames,byteCodeGenerator,context,mutator);
                }
            }
            break;
        }
    case knopFor:
        pnode->sxFor.pnodeInit=UpdateNode(pnode->sxFor.pnodeInit,byteCodeGenerator,context,mutator);
        pnode->sxFor.pnodeCond=UpdateNode(pnode->sxFor.pnodeCond,byteCodeGenerator,context,mutator);
        pnode->sxFor.pnodeIncr=UpdateNode(pnode->sxFor.pnodeIncr,byteCodeGenerator,context,mutator);
        pnode->sxFor.pnodeBody=UpdateNode(pnode->sxFor.pnodeBody,byteCodeGenerator,context,mutator);
        break;
        //PTNODE(knopIf         , "if"        ,None    ,If   ,fnopNone)
    case knopIf:
        pnode->sxIf.pnodeCond=UpdateNode(pnode->sxIf.pnodeCond,byteCodeGenerator,context,mutator);
        pnode->sxIf.pnodeTrue=UpdateNode(pnode->sxIf.pnodeTrue,byteCodeGenerator,context,mutator);
        if (pnode->sxIf.pnodeFalse != NULL)
        {
            pnode->sxIf.pnodeFalse=UpdateNode(pnode->sxIf.pnodeFalse,byteCodeGenerator,context,mutator);
        }
        break;
        //PTNODE(knopWhile      , "while"        ,None    ,While,fnopBreak|fnopContinue)
        //PTNODE(knopDoWhile    , "do-while"    ,None    ,While,fnopBreak|fnopContinue)
    case knopDoWhile:
    case knopWhile:
        pnode->sxWhile.pnodeCond=UpdateNode(pnode->sxWhile.pnodeCond,byteCodeGenerator,context,mutator);
        pnode->sxWhile.pnodeBody=UpdateNode(pnode->sxWhile.pnodeBody,byteCodeGenerator,context,mutator);
        break;
        //PTNODE(knopForIn      , "for in"    ,None    ,ForIn,fnopBreak|fnopContinue|fnopCleanup)
    case knopForIn:
    case knopForOf:
        pnode->sxForInOrForOf.pnodeLval=UpdateNode(pnode->sxForInOrForOf.pnodeLval,byteCodeGenerator,context,mutator);
        pnode->sxForInOrForOf.pnodeObj=UpdateNode(pnode->sxForInOrForOf.pnodeObj,byteCodeGenerator,context,mutator);
        pnode->sxForInOrForOf.pnodeBody=UpdateNode(pnode->sxForInOrForOf.pnodeBody,byteCodeGenerator,context,mutator);
        break;
        //PTNODE(knopReturn     , "return"    ,None    ,Uni  ,fnopNone)
    case knopReturn:
        if (pnode->sxReturn.pnodeExpr!=NULL)
            pnode->sxReturn.pnodeExpr=UpdateNode(pnode->sxReturn.pnodeExpr,byteCodeGenerator,context,mutator);
        break;
        //PTNODE(knopBlock      , "{}"        ,None    ,Block,fnopNone)
    case knopBlock:
        pnode->sxBlock.pnodeStmt=UpdateNode(pnode->sxBlock.pnodeStmt,byteCodeGenerator,context,mutator);
        break;
        //PTNODE(knopBreak      , "break"        ,None    ,Jump ,fnopNone)
    case knopBreak:
        // TODO: some representation of target
        break;
        //PTNODE(knopContinue   , "continue"    ,None    ,Jump ,fnopNone)
    case knopContinue:
        // TODO: some representation of target
        break;
        //PTNODE(knopLabel      , "label"        ,None    ,Label,fnopNone)
    case knopLabel:
        // TODO: print labeled statement
        break;
    case knopTypeof:
        pnode->sxUni.pnode1=UpdateNode(pnode->sxUni.pnode1,byteCodeGenerator,context,mutator);
        break;
        //PTNODE(knopThrow      , "throw"     ,None    ,Uni  ,fnopNone)
    case knopThrow:
        pnode->sxUni.pnode1=UpdateNode(pnode->sxUni.pnode1,byteCodeGenerator,context,mutator);
        break;
    case knopArray:
        {
            bool arrayLitOpt = pnode->sxArrLit.arrayOfNumbers && pnode->sxArrLit.count > 1;

            if (!arrayLitOpt)
            {
                pnode->sxUni.pnode1=UpdateNode(pnode->sxUni.pnode1,byteCodeGenerator,context,mutator);
            }
            break;
        }
    case knopWith:
    case knopFinally:
    case knopTry:
    case knopTryCatch:
    case knopTryFinally:
        context->notImplemented=true;
        break;
    }
    return pnode;
}

template <class PrefixFn, class PostfixFn>
void Visit(ParseNode *pnode,ByteCodeGenerator* byteCodeGenerator, PrefixFn prefix, PostfixFn postfix, ParseNode * pnodeParent = NULL);

template<class TContext>
void VisitIndirect(ParseNode *pnode,ByteCodeGenerator* byteCodeGenerator,TContext* context,void (*prefix)(ParseNode*,ByteCodeGenerator* byteCodeGenerator,TContext*),
    void (*postfix)(ParseNode*,ByteCodeGenerator* byteCodeGenerator,TContext*), ParseNode *pnodeParent = NULL)
{
    byteCodeGenerator->EnterVisitIndirect();
    Visit(pnode, byteCodeGenerator,
        [context, prefix](ParseNode * pnode, ByteCodeGenerator * byteCodeGenerator)
        {
            prefix(pnode, byteCodeGenerator, context);
        },
        [context, postfix](ParseNode * pnode, ByteCodeGenerator * byteCodeGenerator)
        {
            if (postfix)
            {
                postfix(pnode, byteCodeGenerator, context);
            }
        }, pnodeParent);
    byteCodeGenerator->LeaveVisitIndirect();
}

template <class PrefixFn, class PostfixFn>
void VisitList(ParseNode *pnode,ByteCodeGenerator* byteCodeGenerator, PrefixFn prefix, PostfixFn postfix)
{
    Assert(pnode != null);
    Assert(pnode->nop == knopList);

    do
    {
        ParseNode * pnode1 = pnode->sxBin.pnode1;
        Visit(pnode1, byteCodeGenerator, prefix, postfix);
        pnode = pnode->sxBin.pnode2;
    }
    while(pnode->nop == knopList);
    Visit(pnode, byteCodeGenerator, prefix, postfix);
}

// Perform the given action once for each name in the pnodeNames list of a given funcdecl node,
// with/without funcInfo.

template <class Fn>
void VisitFncNames(
    ParseNode *pnodeFnc,
    Fn action)
{
    Assert(pnodeFnc && pnodeFnc->nop == knopFncDecl);
    ParseNode *pnodeNames = pnodeFnc->sxFnc.pnodeNames;
    if (pnodeNames)
    {
        while (pnodeNames->nop == knopList)
        {
            action(pnodeFnc, pnodeNames->sxBin.pnode1);
            pnodeNames = pnodeNames->sxBin.pnode2;
        }
    }
    action(pnodeFnc, pnodeNames);
}

template <class PrefixFn, class PostfixFn>
void VisitWithStmt(ParseNode *pnode, Js::RegSlot loc, ByteCodeGenerator* byteCodeGenerator,PrefixFn prefix, PostfixFn postfix,ParseNode *pnodeParent)
{
    // Note the fact that we're visiting the body of a with statement. This allows us to optimize register assignment
    // in the normal case of calls not requiring that their "this" objects be found dynamically.
    Scope *scope = pnode->sxWith.scope;

    byteCodeGenerator->PushScope(scope);
    Visit(pnode->sxWith.pnodeBody, byteCodeGenerator,prefix, postfix, pnodeParent);

    scope->SetIsObject();
    scope->SetMustInstantiate(true);

    byteCodeGenerator->PopScope();
}

bool BlockHasOwnScope(ParseNode* pnodeBlock, ByteCodeGenerator *byteCodeGenerator)
{
    Assert(pnodeBlock->nop == knopBlock);
    return pnodeBlock->sxBlock.scope != NULL && (!(pnodeBlock->grfpn & fpnSyntheticNode) || (pnodeBlock->sxBlock.blockType == PnodeBlockType::Global && byteCodeGenerator->IsEvalWithBlockScopingNoParentScopeInfo()));
}

void BeginVisitBlock(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    if (BlockHasOwnScope(pnode, byteCodeGenerator))
    {
        byteCodeGenerator->PushBlock(pnode);
        byteCodeGenerator->PushScope(pnode->sxBlock.scope);
    }
}

void EndVisitBlock(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    if (BlockHasOwnScope(pnode, byteCodeGenerator))
    {
        Assert(byteCodeGenerator->GetCurrentScope() == pnode->sxBlock.scope);
        byteCodeGenerator->PopScope();
        byteCodeGenerator->PopBlock();
    }
}

void BeginVisitCatch(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    byteCodeGenerator->PushScope(pnode->sxCatch.scope);
}

void EndVisitCatch(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    byteCodeGenerator->PopScope();
}

bool CreateNativeArrays(ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    Js::FunctionBody *functionBody = funcInfo ? funcInfo->GetParsedFunctionBody() : NULL;

    return
        !PHASE_OFF_OPTFUNC(Js::NativeArrayPhase, functionBody) &&
        !byteCodeGenerator->IsInDebugMode() &&
        (
            functionBody
                ? Js::DynamicProfileInfo::IsEnabled(Js::NativeArrayPhase, functionBody)
                : Js::DynamicProfileInfo::IsEnabledForAtLeastOneFunction(
                    Js::NativeArrayPhase,
                    byteCodeGenerator->GetScriptContext())
        );
}

bool EmitAsConstantArray(ParseNode *pnodeArr, ByteCodeGenerator *byteCodeGenerator)
{
    Assert(pnodeArr && pnodeArr->nop == knopArray);

    // TODO: We shouldn't have to handle an empty funcinfo stack here, but there seem to be issues
    // with the stack involved nested deferral. Remove this null check when those are resolved.
    if (CreateNativeArrays(byteCodeGenerator, byteCodeGenerator->TopFuncInfo()))
    {
        return pnodeArr->sxArrLit.arrayOfNumbers;
    }

    return pnodeArr->sxArrLit.arrayOfTaggedInts && pnodeArr->sxArrLit.count > 1;
}

template<class PrefixFn, class PostfixFn>
void Visit(ParseNode *pnode, ByteCodeGenerator* byteCodeGenerator, PrefixFn prefix, PostfixFn postfix, ParseNode *pnodeParent)
{
    if (pnode == NULL)
        return;

    ThreadContext::ProbeCurrentStackNoDispose(Js::Constants::MinStackByteCodeVisitor, byteCodeGenerator->GetScriptContext());

    prefix(pnode, byteCodeGenerator);
    switch (pnode->nop) {
    default:
    {
        uint flags = ParseNode::Grfnop(pnode->nop);
        if (flags&fnopUni) {
            Visit(pnode->sxUni.pnode1, byteCodeGenerator, prefix, postfix);
        }
        else if (flags&fnopBin) {
            Visit(pnode->sxBin.pnode1, byteCodeGenerator, prefix, postfix);
            Visit(pnode->sxBin.pnode2, byteCodeGenerator, prefix, postfix);
        }
    }
        break;

    case knopCall:
        Visit(pnode->sxCall.pnodeTarget, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxCall.pnodeArgs, byteCodeGenerator, prefix, postfix);
        break;

    case knopNew:
    {
        Visit(pnode->sxCall.pnodeTarget, byteCodeGenerator, prefix, postfix);
        if (!IsCallOfConstants(pnode))
        {
            Visit(pnode->sxCall.pnodeArgs, byteCodeGenerator, prefix, postfix);
        }
        break;
    }

    case knopAsg:
        if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsES6DestructuringEnabled()
            && pnode->sxBin.pnode1->nop == knopArray
            && byteCodeGenerator->InDestructuredArray() == false)
        {
            byteCodeGenerator->SetInDestructuredArray(true);
            Visit(pnode->sxBin.pnode1, byteCodeGenerator, prefix, postfix);
            byteCodeGenerator->SetInDestructuredArray(false);
        }
        else
        {
            Visit(pnode->sxBin.pnode1, byteCodeGenerator, prefix, postfix);
        }
        Visit(pnode->sxBin.pnode2, byteCodeGenerator, prefix, postfix);
        break;
    case knopQmark:
        Visit(pnode->sxTri.pnode1, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxTri.pnode2, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxTri.pnode3, byteCodeGenerator, prefix, postfix);
        break;
    case knopList:
        VisitList(pnode, byteCodeGenerator, prefix, postfix);
        break;
        //PTNODE(knopVarDecl    , "varDcl"    ,None    ,Var  ,fnopNone)
    case knopVarDecl:
    case knopConstDecl:
    case knopLetDecl:
        if (pnode->sxVar.pnodeInit != NULL)
            Visit(pnode->sxVar.pnodeInit, byteCodeGenerator, prefix, postfix);
        break;
        //PTNODE(knopFncDecl    , "fncDcl"    ,None    ,Fnc  ,fnopLeaf)
    case knopFncDecl:
    {
        // Inner function declarations are visited before anything else in the scope.
        // (See VisitFunctionsInScope.)
        ParseNode *pnodeNames = pnode->sxFnc.pnodeNames;
        if (pnodeNames)
        {
            while (pnodeNames->nop == knopList)
            {
                if (pnodeNames->sxBin.pnode1->nop == knopScope ||
                    pnodeNames->sxBin.pnode1->nop == knopDot)
                {
                    pnodeNames->sxBin.pnode1->sxBin.pnode2->grfpn |= fpnMemberReference;
                    Visit(pnodeNames->sxBin.pnode1, byteCodeGenerator, prefix, postfix);
                }
                pnodeNames = pnodeNames->sxBin.pnode2;
            }
            if (pnodeNames->nop == knopScope ||
                pnodeNames->nop == knopDot)
            {
                pnodeNames->sxBin.pnode2->grfpn |= fpnMemberReference;
                Visit(pnodeNames, byteCodeGenerator, prefix, postfix);
            }
        }
        break;
    }
    case knopClassDecl:
    {
        // Visit the extends expression first, since it's bound outside the scope containing the class name.
        Visit(pnode->sxClass.pnodeExtends, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxClass.pnodeDeclName, byteCodeGenerator, prefix, postfix);
        // Now visit the class name and methods.
        BeginVisitBlock(pnode->sxClass.pnodeBlock, byteCodeGenerator);
        Visit(pnode->sxClass.pnodeName, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxClass.pnodeStaticMembers, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxClass.pnodeConstructor, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxClass.pnodeMembers, byteCodeGenerator, prefix, postfix);
        EndVisitBlock(pnode->sxClass.pnodeBlock, byteCodeGenerator);
        break;
    }
    case knopStrTemplate:
    {
        // Visit the string node lists only if we do not have a tagged template
        // We never need to visit the raw strings as they are not used in non-tagged templates and 
        // tagged templates will register them as part of the callsite constant object.
        if (!pnode->sxStrTemplate.isTaggedTemplate)
        {
            Visit(pnode->sxStrTemplate.pnodeStringLiterals, byteCodeGenerator, prefix, postfix);
        }
        Visit(pnode->sxStrTemplate.pnodeSubstitutionExpressions, byteCodeGenerator, prefix, postfix);
        break;
    }
        //PTNODE(knopProg       , "program"    ,None    ,Fnc  ,fnopNone)
    case knopProg:
    {
        // We expect that the global statements have been generated (meaning that the pnodeFncs
        // field is a real pointer, not an enumeration).
        Assert(pnode->sxFnc.pnodeBody);

        uint i = 0;
        VisitNestedScopes(pnode->sxFnc.pnodeScopes, pnode, byteCodeGenerator, prefix, postfix, &i);
        // Visiting global code: track the last value statement.
        BeginVisitBlock(pnode->sxFnc.pnodeScopes, byteCodeGenerator);
        pnode->sxProg.pnodeLastValStmt = VisitBlock(pnode->sxFnc.pnodeBody, byteCodeGenerator, prefix, postfix);
        EndVisitBlock(pnode->sxFnc.pnodeScopes, byteCodeGenerator);

        break;
    }
    case knopFor:
        BeginVisitBlock(pnode->sxFor.pnodeBlock, byteCodeGenerator);
        Visit(pnode->sxFor.pnodeInit, byteCodeGenerator, prefix, postfix);
        byteCodeGenerator->EnterLoop();
        Visit(pnode->sxFor.pnodeCond, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxFor.pnodeIncr, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxFor.pnodeBody, byteCodeGenerator, prefix, postfix, pnode);
        byteCodeGenerator->ExitLoop();
        EndVisitBlock(pnode->sxFor.pnodeBlock, byteCodeGenerator);
        break;
        //PTNODE(knopIf         , "if"        ,None    ,If   ,fnopNone)
    case knopIf:
        Visit(pnode->sxIf.pnodeCond, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxIf.pnodeTrue, byteCodeGenerator, prefix, postfix, pnode);
        if (pnode->sxIf.pnodeFalse != NULL)
        {
            Visit(pnode->sxIf.pnodeFalse, byteCodeGenerator, prefix, postfix, pnode);
        }
        break;
        //PTNODE(knopWhile      , "while"        ,None    ,While,fnopBreak|fnopContinue)
        //PTNODE(knopDoWhile    , "do-while"    ,None    ,While,fnopBreak|fnopContinue)
    case knopDoWhile:
    case knopWhile:
        byteCodeGenerator->EnterLoop();
        Visit(pnode->sxWhile.pnodeCond, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxWhile.pnodeBody, byteCodeGenerator, prefix, postfix, pnode);
        byteCodeGenerator->ExitLoop();
        break;
        //PTNODE(knopForIn      , "for in"    ,None    ,ForIn,fnopBreak|fnopContinue|fnopCleanup)
    case knopForIn:
    case knopForOf:
        BeginVisitBlock(pnode->sxForInOrForOf.pnodeBlock, byteCodeGenerator);
        Visit(pnode->sxForInOrForOf.pnodeLval, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxForInOrForOf.pnodeObj, byteCodeGenerator, prefix, postfix);
        byteCodeGenerator->EnterLoop();
        Visit(pnode->sxForInOrForOf.pnodeBody, byteCodeGenerator, prefix, postfix, pnode);
        byteCodeGenerator->ExitLoop();
        EndVisitBlock(pnode->sxForInOrForOf.pnodeBlock, byteCodeGenerator);
        break;
        //PTNODE(knopReturn     , "return"    ,None    ,Uni  ,fnopNone)
    case knopReturn:
        if (pnode->sxReturn.pnodeExpr != NULL)
            Visit(pnode->sxReturn.pnodeExpr, byteCodeGenerator, prefix, postfix);
        break;
        //PTNODE(knopBlock      , "{}"        ,None    ,Block,fnopNone)
    case knopBlock:
    {
        if (pnode->sxBlock.pnodeStmt != NULL)
        {
            BeginVisitBlock(pnode, byteCodeGenerator);
            pnode->sxBlock.pnodeLastValStmt = VisitBlock(pnode->sxBlock.pnodeStmt, byteCodeGenerator, prefix, postfix, pnode);
            EndVisitBlock(pnode, byteCodeGenerator);
        }
        else
        {
            pnode->sxBlock.pnodeLastValStmt = NULL;
        }
        break;
    }
        //PTNODE(knopWith       , "with"        ,None    ,With ,fnopCleanup)
    case knopWith:
        Visit(pnode->sxWith.pnodeObj, byteCodeGenerator, prefix, postfix);
        VisitWithStmt(pnode, pnode->sxWith.pnodeObj->location, byteCodeGenerator, prefix, postfix, pnode);
        break;
        //PTNODE(knopBreak      , "break"        ,None    ,Jump ,fnopNone)
    case knopBreak:
        // TODO: some representation of target
        break;
        //PTNODE(knopContinue   , "continue"    ,None    ,Jump ,fnopNone)
    case knopContinue:
        // TODO: some representation of target
        break;
        //PTNODE(knopLabel      , "label"        ,None    ,Label,fnopNone)
    case knopLabel:
        // TODO: print labeled statement
        break;
        //PTNODE(knopSwitch     , "switch"    ,None    ,Switch,fnopBreak)
    case knopSwitch:
        Visit(pnode->sxSwitch.pnodeVal, byteCodeGenerator, prefix, postfix);
        BeginVisitBlock(pnode->sxSwitch.pnodeBlock, byteCodeGenerator);
        for (ParseNode *pnodeT = pnode->sxSwitch.pnodeCases; NULL != pnodeT; pnodeT = pnodeT->sxCase.pnodeNext)
        {
            Visit(pnodeT, byteCodeGenerator, prefix, postfix, pnode);
        }
        Visit(pnode->sxSwitch.pnodeBlock, byteCodeGenerator, prefix, postfix);
        EndVisitBlock(pnode->sxSwitch.pnodeBlock, byteCodeGenerator);
        break;
        //PTNODE(knopCase       , "case"        ,None    ,Case ,fnopNone)
    case knopCase:
        Visit(pnode->sxCase.pnodeExpr, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxCase.pnodeBody, byteCodeGenerator, prefix, postfix, pnode);
        break;
    case knopTypeof:
        Visit(pnode->sxUni.pnode1, byteCodeGenerator, prefix, postfix);
        break;
        //PTNODE(knopTryCatchFinally,"try-catch-finally",None,TryCatchFinally,fnopCleanup)
    case knopTryFinally:
        Visit(pnode->sxTryFinally.pnodeTry, byteCodeGenerator, prefix, postfix, pnode);
        Visit(pnode->sxTryFinally.pnodeFinally, byteCodeGenerator, prefix, postfix, pnode);
        break;
        //PTNODE(knopTryCatch      , "try-catch" ,None    ,TryCatch  ,fnopCleanup)
    case knopTryCatch:
        Visit(pnode->sxTryCatch.pnodeTry, byteCodeGenerator, prefix, postfix, pnode);
        Visit(pnode->sxTryCatch.pnodeCatch, byteCodeGenerator, prefix, postfix, pnode);
        break;
        //PTNODE(knopTry        , "try"       ,None    ,Try  ,fnopCleanup)
    case knopTry:
        Visit(pnode->sxTry.pnodeBody, byteCodeGenerator, prefix, postfix, pnode);
        break;
    case knopCatch:
        BeginVisitCatch(pnode, byteCodeGenerator);
        Visit(pnode->sxCatch.pnodeParam, byteCodeGenerator, prefix, postfix);
        Visit(pnode->sxCatch.pnodeBody, byteCodeGenerator, prefix, postfix, pnode);
        EndVisitCatch(pnode, byteCodeGenerator);
        break;
    case knopFinally:
        Visit(pnode->sxFinally.pnodeBody, byteCodeGenerator, prefix, postfix, pnode);
        break;
        //PTNODE(knopThrow      , "throw"     ,None    ,Uni  ,fnopNone)
    case knopThrow:
        Visit(pnode->sxUni.pnode1, byteCodeGenerator, prefix, postfix);
        break;
    case knopArray:
    {
        bool arrayLitOpt = EmitAsConstantArray(pnode, byteCodeGenerator);
        if (!arrayLitOpt)
        {
            Visit(pnode->sxUni.pnode1, byteCodeGenerator, prefix, postfix);
        }
        break;
    }
    case knopComma:
    {
        ParseNode *pnode1 = pnode->sxBin.pnode1;
        if (pnode1->nop == knopComma)
        {
            // Spot fix to avoid recursion on very large comma expressions.
            ArenaAllocator *alloc = byteCodeGenerator->GetAllocator();
            SList<ParseNode*> rhsStack(alloc);
            do {
                rhsStack.Push(pnode1->sxBin.pnode2);
                pnode1 = pnode1->sxBin.pnode1;
            } while (pnode1->nop == knopComma);
            Visit(pnode1, byteCodeGenerator, prefix, postfix);
            while (!rhsStack.Empty())
            {
                ParseNode *pnodeRhs = rhsStack.Pop();
                Visit(pnodeRhs, byteCodeGenerator, prefix, postfix);
            }
        }
        else
        {
            Visit(pnode1, byteCodeGenerator, prefix, postfix);
        }
        Visit(pnode->sxBin.pnode2, byteCodeGenerator, prefix, postfix);
    }
        break;
    }
    if (pnodeParent)
    {
        PropagateFlags(pnode, pnodeParent);
    }
    postfix(pnode, byteCodeGenerator);
}

bool IsDateConstructor(ParseNode *pnode) {
    return((pnode->nop==knopName)&&(wcscmp(reinterpret_cast<const wchar_t*>(pnode->sxPid.pid->Psz()),L"Date")==0));
}

bool IsArguments(ParseNode *pnode)
{
    for (;;)
    {
        switch (pnode->nop)
        {
        case knopName:
            {
                return pnode->sxPid.sym && pnode->sxPid.sym->GetIsArguments();
            }

        case knopCall:
        case knopNew:
            {
                if (IsArguments(pnode->sxCall.pnodeTarget))
                    return true;

                if (pnode->sxCall.pnodeArgs)
                {
                    ParseNode *pnodeArg = pnode->sxCall.pnodeArgs;
                    while (pnodeArg->nop == knopList)
                    {
                        if (IsArguments(pnodeArg->sxBin.pnode1))
                            return true;

                        pnodeArg = pnodeArg->sxBin.pnode2;
                    }

                    pnode = pnodeArg;
                    break;
                }
                return false;
            }

        case knopArray:
            {
                if (pnode->sxArrLit.arrayOfNumbers || pnode->sxArrLit.count == 0)
                {
                    return false;
                }

                pnode = pnode->sxUni.pnode1;
                break;
            }

        case knopQmark:
            {
                if (IsArguments(pnode->sxTri.pnode1) || IsArguments(pnode->sxTri.pnode2))
                    return true;

                pnode = pnode->sxTri.pnode3;
                break;
            }

            //
            // Cases where we don't check for "arguments" yet.
            // Assume that they might have it. Disable the optimization is such scenarios
            //
        case knopList:
        case knopObject:
        case knopVarDecl:
        case knopConstDecl:
        case knopLetDecl:
        case knopFncDecl:
        case knopClassDecl:
        case knopFor:
        case knopIf:
        case knopDoWhile:
        case knopWhile:
        case knopForIn:
        case knopForOf:
        case knopReturn:
        case knopBlock:
        case knopBreak:
        case knopContinue:
        case knopLabel:
        case knopTypeof:
        case knopThrow:
        case knopWith:
        case knopFinally:
        case knopTry:
        case knopTryCatch:
        case knopTryFinally:
            {
                return true;
            }

        default:
            {
                uint flags=ParseNode::Grfnop(pnode->nop);
                if (flags&fnopUni)
                {
                    Assert(pnode->sxUni.pnode1);

                    pnode = pnode->sxUni.pnode1;
                    break;
                }
                else if (flags&fnopBin)
                {
                    Assert(pnode->sxBin.pnode1 && pnode->sxBin.pnode2);

                    if (IsArguments(pnode->sxBin.pnode1))
                        return true;

                    pnode = pnode->sxBin.pnode2;
                    break;
                }
                return false;
            }

        }
    }
}

BOOL IsJump(ParseNode *pnode)
{
    switch (pnode->nop)
    {
    case knopBreak:
    case knopContinue:
    case knopThrow:
    case knopReturn:
        return TRUE;

    case knopBlock:
    case knopDoWhile:
    case knopWhile:
    case knopWith:
    case knopIf:
    case knopForIn:
    case knopForOf:
    case knopFor:
    case knopSwitch:
    case knopCase:
    case knopTryFinally:
    case knopTryCatch:
    case knopTry:
    case knopCatch:
    case knopFinally:
        return (pnode->sxStmt.grfnop & fnopJump) != 0;

    default:
        return FALSE;
    }
}

void PropagateFlags(ParseNode *pnodeChild, ParseNode *pnodeParent)
{
    if (IsJump(pnodeChild))
    {
        pnodeParent->sxStmt.grfnop |= fnopJump;
    }
}

struct NonLocalSymbolToIndex {
    Symbol *sym;
    int nonLocalIndex;
};

bool ApplyEnclosesArgs(ParseNode* fncDecl,ByteCodeGenerator* byteCodeGenerator);
void Bind(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator);
void BindReference(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator);
void AssignRegisters(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator);
void Emit(ParseNode *pnode,ByteCodeGenerator *byteCodeGenerator,FuncInfo *funcInfo, BOOL fReturnValue, bool isConstructorCall = false);
void EmitBinaryOpnds(ParseNode *pnode1, ParseNode *pnode2, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo);

bool
Scope::IsGlobalEvalBlockScope() const
{
    return this->scopeType == ScopeType_GlobalEvalBlock;
}

Scope *
FuncInfo::GetGlobalBlockScope() const
{
    Assert(this->IsGlobalFunction());
    Scope * scope = this->root->sxFnc.pnodeScopes->sxBlock.scope;
    Assert(scope == null || scope == this->GetBodyScope() || scope->GetEnclosingScope() == this->GetBodyScope());
    return scope;
}

Scope *
FuncInfo::GetGlobalEvalBlockScope() const
{
    Scope * globalEvalBlockScope = this->GetGlobalBlockScope();
    Assert(globalEvalBlockScope->GetEnclosingScope() == this->GetBodyScope());
    Assert(globalEvalBlockScope->GetScopeType() == ScopeType_GlobalEvalBlock);
    return globalEvalBlockScope;
}

uint
FuncInfo::FindOrAddReferencedPropertyId(Js::PropertyId propertyId)
{
    Assert(propertyId != Js::Constants::NoProperty);
    Assert(referencedPropertyIdToMapIndex != null);
    if (propertyId < TotalNumberOfBuiltInProperties)
    {
        return propertyId;
    }
    uint index;
    if (!referencedPropertyIdToMapIndex->TryGetValue(propertyId, &index))
    {
        index = this->NewReferencedPropertyId();
        referencedPropertyIdToMapIndex->Add(propertyId, index);
    }
    return index + TotalNumberOfBuiltInProperties;
}

uint
FuncInfo::FindOrAddRootObjectInlineCacheId(Js::PropertyId propertyId, bool isLoadMethod, bool isStore)
{
    Assert(propertyId != Js::Constants::NoProperty);
    Assert(!isLoadMethod || !isStore);
    uint cacheId;
    RootObjectInlineCacheIdMap * idMap = isStore ? rootObjectStoreInlineCacheMap : isLoadMethod ? rootObjectLoadMethodInlineCacheMap : rootObjectLoadInlineCacheMap;
    if (!idMap->TryGetValue(propertyId, &cacheId))
    {
        cacheId = isStore? this->NewRootObjectStoreInlineCache() : isLoadMethod ? this->NewRootObjectLoadMethodInlineCache() : this->NewRootObjectLoadInlineCache();
        idMap->Add(propertyId, cacheId);
    }
    return cacheId;
}
#if DBG_DUMP
void FuncInfo::Dump()
    {
        Output::Print(L"FuncInfo: CallsEval:%s ChildCallsEval:%s HasArguments:%s HasHeapArguments:%s\n",
            IsTrueOrFalse(this->GetCallsEval()),
            IsTrueOrFalse(this->GetChildCallsEval()),
            IsTrueOrFalse(this->GetHasArguments()),
            IsTrueOrFalse(this->GetHasHeapArguments()));
    }
#endif
Js::RegSlot FuncInfo::AcquireLoc(ParseNode *pnode)
{
    // Assign a new temp pseudo-register to this expression.
    if (pnode->location == Js::Constants::NoRegister)
    {
        pnode->location = this->AcquireTmpRegister();
    }
    return pnode->location;
}

Js::RegSlot FuncInfo::AcquireTmpRegister()
{
    Assert(this->firstTmpReg != Js::Constants::NoRegister);
    // Allocate a new temp pseudo-register, increasing the locals count if necessary.
    Assert(this->curTmpReg <= this->varRegsCount && this->curTmpReg >= this->firstTmpReg);
    Js::RegSlot tmpReg = this->curTmpReg;
    UInt32Math::Inc(this->curTmpReg);
    if (this->curTmpReg > this->varRegsCount)
    {
        this->varRegsCount = this->curTmpReg;
    }
    return tmpReg;
}

void FuncInfo::ReleaseLoc(ParseNode *pnode)
{
    // Release the temp assigned to this expression so it can be re-used.
    if (pnode && pnode->location != Js::Constants::NoRegister)
    {
        this->ReleaseTmpRegister(pnode->location);
    }
}

void FuncInfo::ReleaseLoad(ParseNode *pnode)
{
    // Release any temp register(s) acquired by an EmitLoad.
    switch (pnode->nop)
    {
    case knopDot:
    case knopScope:
    case knopIndex:
    case knopCall:
        this->ReleaseReference(pnode);
        break;
    }
    this->ReleaseLoc(pnode);
}

void FuncInfo::ReleaseReference(ParseNode *pnode)
{
    // Release any temp(s) assigned to this reference expression so they can be re-used.
    switch (pnode->nop)
    {
    case knopDot:
    case knopScope:
        this->ReleaseLoc(pnode->sxBin.pnode1);
        break;

    case knopIndex:
        this->ReleaseLoc(pnode->sxBin.pnode2);
        this->ReleaseLoc(pnode->sxBin.pnode1);
        break;

    case knopName:
        // Do nothing (see EmitReference)
        break;

    case knopCall:
    case knopNew:
        // For call/new, we have to release the ArgOut register(s) in reverse order,
        // but we have the args in a singly linked list.
        // Fortunately, we know that the set we have to release is sequential.
        // So find the endpoints of the list and release them in descending order.
        if (pnode->sxCall.pnodeArgs)
        {
            ParseNode *pnodeArg = pnode->sxCall.pnodeArgs;
            Js::RegSlot firstArg = Js::Constants::NoRegister;
            Js::RegSlot lastArg = Js::Constants::NoRegister;
            if (pnodeArg->nop == knopList)
            {
                do
                {
                    if (this->IsTmpReg(pnodeArg->sxBin.pnode1->location))
                    {
                        lastArg = pnodeArg->sxBin.pnode1->location;
                        if (firstArg == Js::Constants::NoRegister)
                        {
                            firstArg = lastArg;
                        }
                    }
                    pnodeArg = pnodeArg->sxBin.pnode2;
                }
                while (pnodeArg->nop == knopList);
            }
            if (this->IsTmpReg(pnodeArg->location))
            {
                lastArg = pnodeArg->location;
                if (firstArg == Js::Constants::NoRegister)
                {
                    // Just one: first and last point to the same node.
                    firstArg = lastArg;
                }
            }
            if(lastArg != Js::Constants::NoRegister)
            {
                Assert(firstArg != Js::Constants::NoRegister);
                Assert(lastArg >= firstArg);
                do
                {
                    // Walk down from last to first.
                    this->ReleaseTmpRegister(lastArg);
                } while(lastArg-- > firstArg); // these are unsigned, so (--lastArg >= firstArg) will cause an infinite loop if firstArg is 0 (although that shouldn't happen)
            }
        }
        // Now release the call target.
        switch (pnode->sxCall.pnodeTarget->nop)
        {
        case knopDot:
        case knopScope:
        case knopIndex:
            this->ReleaseReference(pnode->sxCall.pnodeTarget);
            this->ReleaseLoc(pnode->sxCall.pnodeTarget);
            break;
        default:
            this->ReleaseLoad(pnode->sxCall.pnodeTarget);
            break;
        }
        break;
    default:
        this->ReleaseLoc(pnode);
        break;
    }
}

void FuncInfo::ReleaseTmpRegister(Js::RegSlot tmpReg)
{
    // Put this reg back on top of the temp stack (if it's a temp).
    Assert(tmpReg != Js::Constants::NoRegister);
    if (this->IsTmpReg(tmpReg))
    {
        Assert(tmpReg == this->curTmpReg - 1);
        this->curTmpReg--;
    }
}

bool Scope::IsBlockScope(FuncInfo *funcInfo)
{
    return this != funcInfo->GetBodyScope() && this != funcInfo->GetParamScope();
}

void Scope::SetHasLocalInClosure(bool has)
{
    // (Note: if a catch var is closure-captured, we won't merge the catch scope with the function scope.
    // So don't mark the function scope "has local in closure".)
    if (has && (this == func->GetBodyScope() || this == func->GetParamScope()) || (GetCanMerge() && this->scopeType != ScopeType_Catch))
    {
        func->SetHasLocalInClosure(true);
    }
    else
    {
        if (hasCrossScopeFuncAssignment)
        {
            func->SetHasMaybeEscapedNestedFunc(DebugOnly(L"InstantiateScopeWithCrossScopeAssignment"));
        }
        SetMustInstantiate(true);
    }
}

int Scope::AddScopeSlot()
{
    int slot = scopeSlotCount++;
    if (scopeSlotCount == Js::ScopeSlots::MaxEncodedSlotCount)
    {
        this->GetEnclosingFunc()->SetHasMaybeEscapedNestedFunc(DebugOnly(L"TooManySlots"));
    }
    return slot;
}

bool Symbol::NeedsSlotAlloc(FuncInfo *funcInfo)
{
    return IsInSlot(funcInfo, true);
}

bool Symbol::IsInSlot(FuncInfo *funcInfo, bool ensureSlotAlloc)
{
    if(this->GetIsGlobal())
    {
        return false;
    }
    if (funcInfo->GetHasHeapArguments() && this->GetIsFormal() && ByteCodeGenerator::NeedScopeObjectForArguments(funcInfo, funcInfo->root))
    {
        // Rest is a special case - it will be in a register.
        if (funcInfo->root->sxFnc.pnodeRest != this->decl)
        {
            return true;
        }
    }
    if (this->GetIsGlobalCatch())
    {
        return true;
    }
    if (this->scope->GetCapturesAll())
    {
        return true;
    }
    return this->GetHasNonLocalReference() && (ensureSlotAlloc || this->GetIsCommittedToSlot());
}

bool Symbol::GetIsCommittedToSlot() const
{
    return isCommittedToSlot || this->scope->GetIsObject();
}

Js::PropertyId Symbol::EnsureScopeSlot(FuncInfo *funcInfo)
{
    if (this->NeedsSlotAlloc(funcInfo) && this->scopeSlot == Js::Constants::NoProperty)
    {
        this->scopeSlot = this->scope->AddScopeSlot();
    }
    return this->scopeSlot;
}

bool IsExpressionStatement(ParseNode* stmt, const Js::ScriptContext *const scriptContext)
{
    if (stmt->nop == knopFncDecl)
    {
        // 'knopFncDecl' is used for both function declarations and function expressions. In a program, a function expression
        // produces the function object that is created for the function expression as its value for the program. A function
        // declaration does not produce a value for the program.
        return !stmt->sxFnc.IsDeclaration();
    }
    if ((stmt->nop >= 0) && (stmt->nop<knopLim))
    {
        return (ParseNode::Grfnop(stmt->nop) & fnopNotExprStmt) == 0;
    }
    return false;
}

BOOL MustProduceValue(ParseNode *pnode, const Js::ScriptContext *const scriptContext)
{
    // Determine whether the current statement is guaranteed to produce a value.

    if (IsExpressionStatement(pnode, scriptContext))
    {
        // These are trivially true.
        return TRUE;
    }

    for (;;)
    {
        switch (pnode->nop)
        {
        case knopFor:
            // Check the common "for(;;)" case.
            if (pnode->sxFor.pnodeCond != NULL ||
                pnode->sxFor.pnodeBody == NULL)
            {
                return FALSE;
            }
            // Loop body is always executed. Look at the loop body next.
            pnode = pnode->sxFor.pnodeBody;
            break;

        case knopIf:
            // True only if both "if" and "else" exist, and both produce values.
            if (pnode->sxIf.pnodeTrue == NULL ||
                pnode->sxIf.pnodeFalse == NULL)
            {
                return FALSE;
            }
            if (!MustProduceValue(pnode->sxIf.pnodeFalse, scriptContext))
            {
                return FALSE;
            }
            pnode = pnode->sxIf.pnodeTrue;
            break;

        case knopWhile:
            // Check the common "while(1)" case.
            if (pnode->sxWhile.pnodeBody == NULL ||
                (pnode->sxWhile.pnodeCond &&
                (pnode->sxWhile.pnodeCond->nop != knopInt ||
                pnode->sxWhile.pnodeCond->sxInt.lw == 0)))
            {
                return FALSE;
            }
            // Loop body is always executed. Look at the loop body next.
            pnode = pnode->sxWhile.pnodeBody;
            break;

        case knopDoWhile:
            if (pnode->sxWhile.pnodeBody == NULL)
            {
                return FALSE;
            }
            // Loop body is always executed. Look at the loop body next.
            pnode = pnode->sxWhile.pnodeBody;
            break;

        case knopBlock:
            return pnode->sxBlock.pnodeLastValStmt != NULL;

        case knopWith:
            if (pnode->sxWith.pnodeBody == NULL)
            {
                return FALSE;
            }
            pnode = pnode->sxWith.pnodeBody;
            break;

        case knopSwitch:
            {
                // This is potentially the most inefficient case. We could consider adding a flag to the PnSwitch
                // struct and computing it when we visit the switch, but:
                // a. switch statements at global scope shouldn't be that common;
                // b. switch statements with many arms shouldn't be that common;
                // c. switches without default cases can be trivially skipped.
                if (pnode->sxSwitch.pnodeDefault == NULL)
                {
                    // Can't guarantee that any code is executed.
                    return FALSE;
                }
                ParseNode *pnodeCase;
                for (pnodeCase = pnode->sxSwitch.pnodeCases; pnodeCase; pnodeCase = pnodeCase->sxCase.pnodeNext)
                {
                    if (pnodeCase->sxCase.pnodeBody == NULL)
                    {
                        if (pnodeCase->sxCase.pnodeNext == NULL)
                        {
                            // Last case has no code to execute.
                            return FALSE;
                        }
                        // Fall through to the next case.
                    }
                    else
                    {
                        if (!MustProduceValue(pnodeCase->sxCase.pnodeBody, scriptContext))
                        {
                            return FALSE;
                        }
                    }
                }
                return TRUE;
            }

        case knopTryCatch:
            // True only if both try and catch produce a value.
            if (pnode->sxTryCatch.pnodeTry->sxTry.pnodeBody == NULL ||
                pnode->sxTryCatch.pnodeCatch->sxCatch.pnodeBody == NULL)
            {
                return FALSE;
            }
            if (!MustProduceValue(pnode->sxTryCatch.pnodeCatch->sxCatch.pnodeBody, scriptContext))
            {
                return FALSE;
            }
            pnode = pnode->sxTryCatch.pnodeTry->sxTry.pnodeBody;
            break;

        case knopTryFinally:
            if (pnode->sxTryFinally.pnodeFinally->sxFinally.pnodeBody == NULL)
            {
                // No finally body: look at the try body.
                if (pnode->sxTryFinally.pnodeTry->sxTry.pnodeBody == NULL)
                {
                    return FALSE;
                }
                pnode = pnode->sxTryFinally.pnodeTry->sxTry.pnodeBody;
                break;
            }
            // Skip the try body, since the finally body will always follow it.
            pnode = pnode->sxTryFinally.pnodeFinally->sxFinally.pnodeBody;
            break;

        default:
            return FALSE;
        }
    }
}

static const Js::OpCode nopToOp[knopLim] =
{
#define OP(x) Br##x##_A
#define PTNODE(nop,sn,pc,nk,grfnop,json,apnk) Js::OpCode::pc,
#include "ptlist.h"
};
static const Js::OpCode nopToCMOp[knopLim] =
{
#define OP(x) Cm##x##_A
#define PTNODE(nop,sn,pc,nk,grfnop,json,apnk) Js::OpCode::pc,
#include "ptlist.h"
};

SymbolName const ByteCodeGenerator::argumentsName(L"arguments", _countof(L"arguments") - 1);

ByteCodeGenerator::ByteCodeGenerator(Js::ScriptContext* scriptContext, Js::ScopeInfo* parentScopeInfo) :
    alloc(nullptr),
    scriptContext(scriptContext),
    flags(0),
    funcInfoStack(nullptr),
    pRootFunc(nullptr),
    pCurrentFunction(nullptr),
    globalScope(nullptr),
    currentScope(nullptr),
    parentScopeInfo(parentScopeInfo),
#if DBG
    executingGenerate(false),
#endif
    dynamicScopeCount(0),
    isBinding(false),
    propertyRecords(nullptr),
    inDestructuredArray(false)
{
    m_writer.Create();
}

bool ByteCodeGenerator::IsLanguageServiceEnabled() const
{
    return BinaryFeatureControl::LanguageService() && scriptContext->GetThreadContext()->Diagnostics->languageServiceEnabled;
}

bool ByteCodeGenerator::UseParserBindings() const
{
    return !IsLanguageServiceEnabled() && IsInNonDebugMode() && !PHASE_OFF1(Js::ParserBindPhase);
}

// ByteCodeGenerator debug mode means we are generating debug mode user-code. Library code is always in non-debug mode.
bool ByteCodeGenerator::IsInDebugMode() const
{
    return scriptContext->IsInDebugMode() && !m_utf8SourceInfo->GetIsLibraryCode();
}

// ByteCodeGenerator non-debug mode means we are not debugging, or we are generating library code which is always in non-debug mode.
bool ByteCodeGenerator::IsInNonDebugMode() const
{
    return scriptContext->IsInNonDebugMode() || m_utf8SourceInfo->GetIsLibraryCode();
}

bool ByteCodeGenerator::ShouldTrackDebuggerMetadata() const
{
    return (IsInDebugMode() && !IsLanguageServiceEnabled())
#if DBG_DUMP
        || (Js::Configuration::Global.flags.Debug)
#endif
        ;
}

// Tracks a register slot let/const property for the passed in debugger block scope.
// debuggerScope         - The scope to add the variable to.
// symbol                - The symbol that represents the register property.
// funcInfo              - The function info used to store the property into the tracked debugger register slot list.
// flags                 - The flags to assign to the property.
// isFunctionDeclaration - Whether or not the register is a function declaration, which requires that its byte code offset be updated immediately.
void ByteCodeGenerator::TrackRegisterPropertyForDebugger(
    Js::DebuggerScope *debuggerScope,
    Symbol *symbol,
    FuncInfo *funcInfo,
    Js::DebuggerScopePropertyFlags flags /*= Js::DebuggerScopePropertyFlags_None*/,
    bool isFunctionDeclaration /*= false*/)
{
    Assert(debuggerScope);
    Assert(symbol);
    Assert(funcInfo);

    Js::RegSlot location = symbol->GetLocation();

    Js::DebuggerScope *correctDebuggerScope = debuggerScope;
    if (debuggerScope->scopeType != Js::DiagExtraScopesType::DiagBlockScopeDirect)
    {
        // We have to get the appropiate scope and add property over there.
        // Make sure the scope is created whether we're in debug mode or not, because we
        // need the empty scopes present during reparsing for debug mode.
        correctDebuggerScope = debuggerScope->GetSiblingScope(location, Writer()->GetFunctionWrite());
    }

    if (this->ShouldTrackDebuggerMetadata() && !symbol->GetIsTrackedForDebugger())
    {
        // Only track the property if we're in debug mode since it's only needed by the debugger.
        Js::PropertyId propertyId = symbol->EnsurePosition(this);

        this->Writer()->AddPropertyToDebuggerScope(
            correctDebuggerScope,
            location,
            propertyId,
            /*shouldConsumeRegister*/ true,
            flags,
            isFunctionDeclaration);

        Js::FunctionBody *byteCodeFunction = funcInfo->GetParsedFunctionBody();
        byteCodeFunction->InsertSymbolToRegSlotList(location, propertyId, funcInfo->varRegsCount);

        symbol->SetIsTrackedForDebugger(true);
    }
}

void ByteCodeGenerator::TrackActivationObjectPropertyForDebugger(
    Js::DebuggerScope *debuggerScope,
    Symbol *symbol,
    Js::DebuggerScopePropertyFlags flags /*= Js::DebuggerScopePropertyFlags_None*/,
    bool isFunctionDeclaration /*= false*/)
{
    Assert(debuggerScope);
    Assert(symbol);

    // Only need to track activation object properties in debug mode.
    if (ShouldTrackDebuggerMetadata() && !symbol->GetIsTrackedForDebugger())
    {
        Js::RegSlot location = symbol->GetLocation();
        Js::PropertyId propertyId = symbol->EnsurePosition(this);

        this->Writer()->AddPropertyToDebuggerScope(
            debuggerScope,
            location,
            propertyId,
            /*shouldConsumeRegister*/ false,
            flags,
            isFunctionDeclaration);

        symbol->SetIsTrackedForDebugger(true);
    }
}

void ByteCodeGenerator::TrackSlotArrayPropertyForDebugger(
    Js::DebuggerScope *debuggerScope,
    Symbol* symbol,
    Js::PropertyId propertyId,
    Js::DebuggerScopePropertyFlags flags /*= Js::DebuggerScopePropertyFlags_None*/,
    bool isFunctionDeclaration /*= false*/)
{
    // Note: Slot array properties are tracked even in non-debug mode in order to support slot array serialization
    // of let/const variables between non-debug and debug mode (for example, when a slot array var escapes and is retrieved
    // after a debugger attach or for WWA apps).  They are also needed for heap enumeration.
    Assert(debuggerScope);
    Assert(symbol);

    if (!symbol->GetIsTrackedForDebugger())
    {
        Js::RegSlot location = symbol->GetScopeSlot();
        Assert(location != Js::Constants::NoRegister);
        Assert(propertyId != Js::Constants::NoProperty);

        this->Writer()->AddPropertyToDebuggerScope(
            debuggerScope,
            location,
            propertyId,
            /*shouldConsumeRegister*/ false,
            flags,
            isFunctionDeclaration);

        symbol->SetIsTrackedForDebugger(true);
    }
}

// Tracks a function declaration inside a block scope for the debugger metadata's current scope (let binding).
void ByteCodeGenerator::TrackFunctionDeclarationPropertyForDebugger(Symbol *functionDeclarationSymbol, FuncInfo *funcInfoParent)
{
    Assert(functionDeclarationSymbol);
    Assert(funcInfoParent);
    AssertMsg(functionDeclarationSymbol->GetIsBlockVar(), "We should only track inner function let bindings for the debugger.");

    // Note: we don't have to check symbol->GetIsTrackedForDebugger, as we are not doing actual work here,
    //       which is done in other Track* functions that we call.

    if (functionDeclarationSymbol->IsInSlot(funcInfoParent))
    {
        if (functionDeclarationSymbol->GetScope()->GetIsObject())
        {
            this->TrackActivationObjectPropertyForDebugger(
                this->Writer()->GetCurrentDebuggerScope(),
                functionDeclarationSymbol,
                Js::DebuggerScopePropertyFlags_None,
                true /*isFunctionDeclaration*/);
        }
        else
        {
            // Make sure the property has a slot. This will bump up the size of the slot array if necessary.
            // Note that slot array inner function bindings are tracked even in non-debug mode in order
            // to keep the lifetime of the closure binding that could escape around for heap enumeration.
            functionDeclarationSymbol->EnsureScopeSlot(funcInfoParent);
            functionDeclarationSymbol->EnsurePosition(this);
            this->TrackSlotArrayPropertyForDebugger(
                this->Writer()->GetCurrentDebuggerScope(),
                functionDeclarationSymbol,
                functionDeclarationSymbol->GetPosition(),
                Js::DebuggerScopePropertyFlags_None,
                true /*isFunctionDeclaration*/);
        }
    }
    else
    {
        this->TrackRegisterPropertyForDebugger(
            this->Writer()->GetCurrentDebuggerScope(),
            functionDeclarationSymbol,
            funcInfoParent,
            Js::DebuggerScopePropertyFlags_None,
            true /*isFunctionDeclaration*/);
    }
}

void ByteCodeGenerator::SetRootFuncInfo(FuncInfo* func)
{
    Assert(pRootFunc == NULL || pRootFunc == func->byteCodeFunction || !IsInNonDebugMode());

    if (pRootFunc)
    {
        return;
    }

    this->pRootFunc = func->byteCodeFunction->GetParseableFunctionInfo();

    if (this->flags & (fscrImplicitThis | fscrImplicitParents))
    {
        // Mark a top-level event handler, since it will need to construct the "this" pointer's
        // namespace hierarchy to access globals.
        Assert(!func->IsGlobalFunction());
        func->SetIsEventHandler(true);
    }
}

// Updates the byte code offset of the property with the passed in location and ID.
// Used to track let/const variables that are in the dead zone debugger side.
// location                 - The activation object, scope slot index, or register location for the property.
// propertyId               - The ID of the property to update.
// shouldConsumeRegister    - Whether or not the a register should be consumed (used for reg slot locations).
void ByteCodeGenerator::UpdateDebuggerPropertyInitializationOffset(Js::RegSlot location, Js::PropertyId propertyId, bool shouldConsumeRegister)
{
    Assert(this->Writer());
    Js::DebuggerScope* currentDebuggerScope = this->Writer()->GetCurrentDebuggerScope();
    Assert(currentDebuggerScope);
    if (currentDebuggerScope != nullptr)
    {
        this->Writer()->UpdateDebuggerPropertyInitializationOffset(
            currentDebuggerScope,
            location,
            propertyId,
            shouldConsumeRegister);
    }
}

Js::RegSlot ByteCodeGenerator::NextVarRegister()
{
    return funcInfoStack->Top()->NextVarRegister();
}

Js::RegSlot ByteCodeGenerator::NextConstRegister()
{
    return funcInfoStack->Top()->NextConstRegister();
}

FuncInfo * ByteCodeGenerator::TopFuncInfo() const
{
    return funcInfoStack->Empty() ? NULL : funcInfoStack->Top();
}

void ByteCodeGenerator::SetHasTry(bool has) {
    TopFuncInfo()->GetParsedFunctionBody()->SetHasTry(has);
}

void ByteCodeGenerator::SetHasFinally(bool has) {
    TopFuncInfo()->GetParsedFunctionBody()->SetHasFinally(has);
}

// TODO: per-function register assignment for env and global symbols
void ByteCodeGenerator::AssignRegister(Symbol *sym) {
    AssertMsg(sym->GetDecl() == NULL || sym->GetDecl()->nop != knopConstDecl || sym->GetDecl()->nop != knopLetDecl, "Const and let should get only temporary register, assigned during emit stage");
    if (sym->GetLocation()==Js::Constants::NoRegister)
    {
        sym->SetLocation(NextVarRegister());
    }
}

void ByteCodeGenerator::AddTargetStmt(ParseNode *pnodeStmt) {
    FuncInfo *top=funcInfoStack->Top();
    top->AddTargetStmt(pnodeStmt);
}

Js::RegSlot ByteCodeGenerator::AssignNullConstRegister()
{
    FuncInfo *top = funcInfoStack->Top();
    return top->AssignNullConstRegister();
}

Js::RegSlot ByteCodeGenerator::AssignUndefinedConstRegister()
{
    FuncInfo *top = funcInfoStack->Top();
    return top->AssignUndefinedConstRegister();
}

Js::RegSlot ByteCodeGenerator::AssignTrueConstRegister()
{
    FuncInfo *top = funcInfoStack->Top();
    return top->AssignTrueConstRegister();
}

Js::RegSlot ByteCodeGenerator::AssignFalseConstRegister()
{
    FuncInfo *top = funcInfoStack->Top();
    return top->AssignFalseConstRegister();
}

Js::RegSlot ByteCodeGenerator::AssignThisRegister()
{
    FuncInfo *top = funcInfoStack->Top();
    return top->AssignThisRegister();
}

void ByteCodeGenerator::SetNeedEnvRegister()
{
    FuncInfo *top = funcInfoStack->Top();
    top->SetNeedEnvRegister();
}

void ByteCodeGenerator::AssignFrameObjRegister()
{
    FuncInfo* top = funcInfoStack->Top();
    if (top->frameObjRegister == Js::Constants::NoRegister)
    {
        top->frameObjRegister = top->NextVarRegister();
    }
}

void ByteCodeGenerator::AssignFrameDisplayRegister()
{
    FuncInfo* top = funcInfoStack->Top();
    if (top->frameDisplayRegister==Js::Constants::NoRegister)
    {
        top->frameDisplayRegister = top->NextVarRegister();
    }
}

void ByteCodeGenerator::AssignFrameSlotsRegister()
{
    FuncInfo* top = funcInfoStack->Top();
    if (top->frameSlotsRegister == Js::Constants::NoRegister)
    {
        top->frameSlotsRegister = NextVarRegister();
    }
}

void ByteCodeGenerator::SetNumberOfInArgs(Js::ArgSlot argCount) {
    FuncInfo *top=funcInfoStack->Top();
    top->inArgsCount=argCount;
}

void ByteCodeGenerator::LoadHeapArguments(FuncInfo *funcInfo)
{
    if (funcInfo->GetHasCachedScope())
    {
        this->LoadCachedHeapArguments(funcInfo);
    }
    else
    {
        this->LoadUncachedHeapArguments(funcInfo);
    }
}

void GetFormalArgsArray(ByteCodeGenerator *byteCodeGenerator, FuncInfo * funcInfo, Js::PropertyIdArray *propIds)
{
    Assert(funcInfo);
    Assert(propIds);
    Assert(byteCodeGenerator);

    bool hadDuplicates = false;
    Js::ArgSlot i = 0;

    auto processArg = [&](ParseNode *pnode)
    {
        Assert(i < propIds->count);
        Symbol *sym = pnode->sxVar.sym;
        Assert(sym);
        Js::PropertyId symPos = sym->EnsurePosition(byteCodeGenerator);

        //
        // Check if the function has any same name parameters
        // For the same name paras, only the last one will be passed the correct propertyid
        // For remaining dup para names, pass Constants::NoProperty
        //
        for (Js::ArgSlot j=0; j<i; j++)
        {
            if (propIds->elements[j] == symPos)
            {
                // Found a dup parameter name
                propIds->elements[j] = Js::Constants::NoProperty;
                hadDuplicates = true;
                break;
            }
        }

        propIds->elements[i] = symPos;
        ++i;
    };
    MapFormals(funcInfo->root, processArg);

    propIds->hadDuplicates = hadDuplicates;
}

void ByteCodeGenerator::LoadUncachedHeapArguments(FuncInfo *funcInfo)
{
    Assert(funcInfo->GetHasHeapArguments());

    Scope *scope = funcInfo->GetBodyScope();
    Assert(scope);
    Symbol *argSym = funcInfo->GetArgumentsSymbol();
    Assert(argSym && argSym->GetIsArguments());
    Js::RegSlot argumentsLoc = argSym->GetLocation();
    Js::RegSlot propIdsLoc = funcInfo->nullConstantRegister;
    Js::RegSlot frameObjLoc = funcInfo->nullConstantRegister;

    Js::OpCode opcode = funcInfo->root->sxFnc.IsSimpleParameterList() ? Js::OpCode::LdHeapArguments : Js::OpCode::LdLetHeapArguments;
    bool hasRest = funcInfo->root->sxFnc.pnodeRest != nullptr;
    uint count = funcInfo->inArgsCount + (hasRest ? 1 : 0) - 1;
    if (count == 0)
    {
        // If no formals to function (only "this"), then no need to create the scope object.
        // Leave both the arguments location and the propertyIds location as null.
        Assert(funcInfo->root->sxFnc.pnodeArgs == nullptr && !hasRest);
    }
    else if (!NeedScopeObjectForArguments(funcInfo, funcInfo->root))
    {
        // We may not need a scope object for arguments, e.g. strict mode with no eval.
    }
    else if (funcInfo->frameObjRegister != Js::Constants::NoRegister)
    {
        // Pass the frame object and ID array to the runtime, and put the resulting Arguments object
        // at the expected location.
        propIdsLoc = argumentsLoc;

        frameObjLoc = funcInfo->frameObjRegister;

        Js::PropertyIdArray *propIds = AnewPlus(GetAllocator(), count * sizeof(Js::PropertyId), Js::PropertyIdArray, count);

        GetFormalArgsArray(this, funcInfo, propIds);

        // Generate the opcode with propIds
        Writer()->Auxiliary(
            Js::OpCode::LdPropIds,
            propIdsLoc,
            propIds,
            sizeof(Js::PropertyIdArray) + count * sizeof(Js::PropertyId),
            count);

        AdeletePlus(GetAllocator(), count * sizeof(Js::PropertyId), propIds);
    }

    this->m_writer.Reg3(opcode, argumentsLoc, frameObjLoc, propIdsLoc);
    EmitLocalPropInit(argSym->GetLocation(), argSym, funcInfo);
}

void ByteCodeGenerator::LoadCachedHeapArguments(FuncInfo *funcInfo)
{
    Assert(funcInfo->GetHasHeapArguments());

    Scope *scope = funcInfo->GetBodyScope();
    Assert(scope);
    Symbol *argSym = funcInfo->GetArgumentsSymbol();
    Assert(argSym && argSym->GetIsArguments());
    Js::RegSlot argumentsLoc = argSym->GetLocation();

    Js::OpCode op = funcInfo->root->sxFnc.IsSimpleParameterList() ? Js::OpCode::LdHeapArgsCached : Js::OpCode::LdLetHeapArgsCached;

    this->m_writer.Reg2(op, argumentsLoc, funcInfo->frameObjRegister);
    EmitLocalPropInit(argumentsLoc, argSym, funcInfo);
}

Js::RegSlot ByteCodeGenerator::EnregisterConstant(unsigned int constant)
{
    Js::RegSlot loc = Js::Constants::NoRegister;
    FuncInfo *top = funcInfoStack->Top();
    if (!top->constantToRegister.TryGetValue(constant, &loc))
    {
        loc = NextConstRegister();
        top->constantToRegister.Add(constant, loc);
    }
    return loc;
}

Js::RegSlot ByteCodeGenerator::EnregisterStringConstant(IdentPtr pid)
{
    Js::RegSlot loc = Js::Constants::NoRegister;
    FuncInfo *top = funcInfoStack->Top();
    if (!top->stringToRegister.TryGetValue(pid, &loc))
    {
        loc = NextConstRegister();
        top->stringToRegister.Add(pid, loc);
    }
    return loc;
}

Js::RegSlot ByteCodeGenerator::EnregisterDoubleConstant(double d)
{
    Js::RegSlot loc = Js::Constants::NoRegister;
    FuncInfo *top = funcInfoStack->Top();
    if (!top->TryGetDoubleLoc(d,&loc))
    {
        loc = NextConstRegister();
        top->AddDoubleConstant(d,loc);
    }
    return loc;
}

Js::JavascriptArray* ByteCodeGenerator::BuildArrayFromStringList(ParseNode* stringNodeList, uint arrayLength, Js::ScriptContext* scriptContext)
{
    Assert(stringNodeList);

    uint index = 0;
    Js::Var str;
    IdentPtr pid;
    Js::JavascriptArray* pArr = scriptContext->GetLibrary()->CreateArray(arrayLength);

    while (stringNodeList->nop == knopList)
    {
        Assert(stringNodeList->sxBin.pnode1->nop == knopStr);

        pid = stringNodeList->sxBin.pnode1->sxPid.pid;
        str = Js::JavascriptString::NewCopyBuffer(pid->Psz(), pid->Cch(), scriptContext);
        pArr->SetItemWithAttributes(index, str, PropertyEnumerable);

        stringNodeList = stringNodeList->sxBin.pnode2;
        index++;
    }

    Assert(stringNodeList->nop == knopStr);

    pid = stringNodeList->sxPid.pid;
    str = Js::JavascriptString::NewCopyBuffer(pid->Psz(), pid->Cch(), scriptContext);
    pArr->SetItemWithAttributes(index, str, PropertyEnumerable);

    return pArr;
}

Js::RegSlot ByteCodeGenerator::EnregisterStringTemplateCallsiteConstant(ParseNode* pnode)
{
    Assert(pnode->nop == knopStrTemplate);
    Assert(pnode->sxStrTemplate.isTaggedTemplate);

    Js::RegSlot loc = Js::Constants::NoRegister;
    FuncInfo* top = funcInfoStack->Top();

    if (!top->stringTemplateCallsiteRegisterMap.TryGetValue(pnode, &loc))
    {
        loc = NextConstRegister();

        top->stringTemplateCallsiteRegisterMap.Add(pnode, loc);
    }

    return loc;
}

//
// Restore all outer func scope info when reparsing a deferred func.
//
void ByteCodeGenerator::RestoreScopeInfo(Js::FunctionBody* functionBody)
{
    if (functionBody && functionBody->GetScopeInfo())
    {
        PROBE_STACK(scriptContext, Js::Constants::MinStackByteCodeVisitor);

        Js::ScopeInfo* scopeInfo = functionBody->GetScopeInfo();
        RestoreScopeInfo(scopeInfo->GetParent()); // Recursively restore outer func scope info

        Js::ScopeInfo* paramScopeInfo = scopeInfo->GetParamScopeInfo();
        Scope* paramScope = nullptr;
        if (paramScopeInfo != nullptr)
        {
            paramScope = paramScopeInfo->GetScope();
            Assert(paramScope || !UseParserBindings());
            if (paramScope == null || !UseParserBindings())
            {
                paramScope = Anew(alloc, Scope, alloc, ScopeType_Parameter, true);
            }
            // We need the funcInfo before continuing the restoration of the param scope, so wait for the funcInfo to be created.
        }

        Scope* bodyScope = scopeInfo->GetScope();

        Assert(bodyScope || !UseParserBindings());
        if (bodyScope == nullptr || !UseParserBindings())
        {
            if (scopeInfo->IsGlobalEval())
            {
                bodyScope = Anew(alloc, Scope, alloc, ScopeType_GlobalEvalBlock, true);
            }
            else
            {
                bodyScope = Anew(alloc, Scope, alloc, ScopeType_FunctionBody, true);
            }
        }
        FuncInfo* func = Anew(alloc, FuncInfo, functionBody->GetDisplayName(), alloc, paramScope, bodyScope, nullptr, functionBody);

        if (paramScope != nullptr)
        {
            paramScope->SetFunc(func);
            paramScopeInfo->GetScopeInfo(null, this, func, paramScope);
        }

        if (bodyScope->GetScopeType() == ScopeType_GlobalEvalBlock)
        {
            func->bodyScope = this->currentScope;
        }
        PushFuncInfo(L"RestoreScopeInfo", func);

        if (!functionBody->DoStackNestedFunc())
        {
            func->hasEscapedUseNestedFunc = true;
        }

        Js::ScopeInfo* funcExprScopeInfo = scopeInfo->GetFuncExprScopeInfo();
        if (funcExprScopeInfo)
        {
            Scope* funcExprScope = funcExprScopeInfo->GetScope();
            Assert(funcExprScope || !UseParserBindings());
            if (funcExprScope == null || !UseParserBindings())
            {
                funcExprScope = Anew(alloc, Scope, alloc, ScopeType_FuncExpr, true);
            }
            funcExprScope->SetFunc(func);
            func->SetFuncExprScope(funcExprScope);
            funcExprScopeInfo->GetScopeInfo(null, this, func, funcExprScope);
        }

        scopeInfo->GetScopeInfo(null, this, func, bodyScope);
    }
    else
    {
        Assert(this->TopFuncInfo() == null);
        // funcBody is glo
        currentScope = Anew(alloc, Scope, alloc, ScopeType_Global, !UseParserBindings());
        globalScope = currentScope;

        FuncInfo *func = Anew(alloc, FuncInfo, Js::Constants::GlobalFunction,
            alloc, nullptr, currentScope, nullptr, functionBody);
        PushFuncInfo(L"RestoreScopeInfo", func);
    }
}

FuncInfo * ByteCodeGenerator::StartBindGlobalStatements(ParseNode *pnode)
{
    if (parentScopeInfo)
    {
        Assert(CONFIG_FLAG(DeferNested));
        trackEnvDepth = true;
        RestoreScopeInfo(parentScopeInfo->GetParent());
        trackEnvDepth = false;
        // "currentScope" is the parentFunc scope. This ensures the deferred func declaration
        // symbol will bind to the func declaration symbol already available in parentFunc scope.
    }
    else
    {
        currentScope = pnode->sxProg.scope;
        Assert(currentScope);
        if (!currentScope || !UseParserBindings())
        {
            currentScope = Anew(alloc, Scope, alloc, ScopeType_Global, true);
            pnode->sxProg.scope = currentScope;
        }
        globalScope = currentScope;
    }

    Js::FunctionBody * byteCodeFunction;

    if (!IsInNonDebugMode() && this->pCurrentFunction != NULL && this->pCurrentFunction->GetIsGlobalFunc() && !this->pCurrentFunction->IsFakeGlobalFunc(flags))
    {
        // we will re-use the global functionbody which was created before deferred parse.
        byteCodeFunction = this->pCurrentFunction;
        byteCodeFunction->RemoveDeferParseAttribute();
        byteCodeFunction->ResetByteCodeGenVisitState();
        if (byteCodeFunction->GetBoundPropertyRecords() == NULL)
        {
            Assert(!IsInNonDebugMode());

            // This happens when we try to re-use the function body which was created due to serialized bytecode.
            byteCodeFunction->SetBoundPropertyRecords(EnsurePropertyRecordList());
        }
    }
    else
    {
        {
            DECLARE_STACK_PINNED(Js::PropertyRecordList, propertyRecordList);
            propertyRecordList = EnsurePropertyRecordList();

            byteCodeFunction = Js::FunctionBody::NewFromRecycler(scriptContext, Js::Constants::GlobalFunction, Js::Constants::GlobalFunctionLength, pnode->sxFnc.nestedCount, m_utf8SourceInfo,
                m_utf8SourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId, pnode->sxFnc.functionId, propertyRecordList
                , Js::FunctionInfo::Attributes::None
#ifdef PERF_COUNTERS
                , false /* is function from deferred deserialized proxy */
#endif
                );


            scriptContext->RegisterDynamicFunctionReference(byteCodeFunction);
        }

        // Mark this global function to required for register script event
        byteCodeFunction->SetIsTopLevel(true);

        byteCodeFunction->SetIsGlobalFunc(TRUE);
        if (pnode->sxFnc.GetStrictMode() != 0)
        {
            byteCodeFunction->SetIsStrictMode();
        }
    }
    if(byteCodeFunction->IsReparsed())
    {
        byteCodeFunction->RestoreState(pnode);
    }
    else
    {
        byteCodeFunction->SaveState(pnode);
    }

    FuncInfo *funcInfo=Anew(alloc, FuncInfo, Js::Constants::GlobalFunction,
        alloc, nullptr, globalScope, pnode, byteCodeFunction);

    long currentAstSize = pnode->sxFnc.astSize;
    if (currentAstSize > this->maxAstSize)
    {
        this->maxAstSize = currentAstSize;
    }
    PushFuncInfo(L"StartBindGlobalStatements", funcInfo);
    return funcInfo;
}

void ByteCodeGenerator::AssignPropertyId(IdentPtr pid)
{
    if (pid->GetPropertyId() == Js::Constants::NoProperty)
    {
        Js::PropertyId id = TopFuncInfo()->byteCodeFunction->GetOrAddPropertyIdTracked(SymbolName(pid->Psz(), pid->Cch()));
        pid->SetPropertyId(id);
    }
}

void ByteCodeGenerator::AssignPropertyId(Symbol *sym, Js::ParseableFunctionInfo* functionInfo) {
    sym->SetPosition(functionInfo->GetOrAddPropertyIdTracked(sym->GetName()));
}

// for now, this just assigns field ids for the current script
// later, we will combine this information with the global field id map
// this temporary code will not work if a global member is accessed both with and without a lhs
void ByteCodeGenerator::AssignPropertyIds(Js::ParseableFunctionInfo* functionInfo) {
    globalScope->ForEachSymbol([this, functionInfo](Symbol * sym)
    {
        this->AssignPropertyId(sym, functionInfo);
    }
    );
}

template<class Fn> void ByteCodeGenerator::IterateBlockScopedVariables(ParseNode *pnodeBlock, Fn fn)
{
    Assert(pnodeBlock->nop == knopBlock);
    for (auto lexvar = pnodeBlock->sxBlock.pnodeLexVars; lexvar; lexvar = lexvar->sxVar.pnodeNext)
    {
        fn(lexvar);
    }
}

void ByteCodeGenerator::InitBlockScopedContent(ParseNode *pnodeBlock, Js::DebuggerScope* debuggerScope, FuncInfo *funcInfo)
{
    Assert(pnodeBlock->nop == knopBlock);

    auto genBlockInit = [this, debuggerScope, funcInfo](ParseNode *pnode)
        {
            // Only check if the scope is valid when let/const vars are in the scope.  If there are no let/const vars,
            // the debugger scope will not be created.
            AssertMsg(debuggerScope, "Missing a case of scope tracking in BeginEmitBlock.");

            FuncInfo *funcInfo = this->TopFuncInfo();
            Symbol *sym = pnode->sxVar.sym;
            Scope *scope = sym->GetScope();

            if (sym->GetIsGlobal())
            {
                AssertMsg(!(this->flags & fscrEval), "Let/consts cannot be globals in eval");

                Js::PropertyId propertyId = sym->EnsurePosition(this);

                Js::OpCode op = (sym->GetDecl()->nop == knopConstDecl) ?
                    Js::OpCode::InitUndeclRootConstFld : Js::OpCode::InitUndeclRootLetFld;

                this->m_writer.ElementRootU(op, funcInfo->FindOrAddReferencedPropertyId(propertyId));
            }
            else if (sym->IsInSlot(funcInfo))
            {
                if (scope->GetIsObject())
                {
                    Js::RegSlot loc = scope->GetLocation();
                    Js::PropertyId propertyId = sym->EnsurePosition(this);

                    uint cacheId = funcInfo->FindOrAddInlineCacheId(loc, propertyId, false, true);

                    Js::OpCode op = (sym->GetDecl()->nop == knopConstDecl) ?
                        Js::OpCode::InitUndeclConstFld : Js::OpCode::InitUndeclLetFld;

                    // CLEANUP: Change layout use for InitUndeclConstFld and InitUndeclLetFld since they do not use the Value register stored in the layout (it is assigned the ReturnRegister here which is weird)
                    this->m_writer.PatchableProperty(op, ByteCodeGenerator::ReturnRegister, sym->GetScope()->GetLocation(), cacheId);

                    TrackActivationObjectPropertyForDebugger(debuggerScope, sym, pnode->nop == knopConstDecl ? Js::DebuggerScopePropertyFlags_Const : Js::DebuggerScopePropertyFlags_None);
                }
                else if (CONFIG_FLAG(TDZ))
                {
                    this->m_writer.Reg1Unsigned1(Js::OpCode::InitUndeclSlot, sym->GetScope()->GetLocation(), sym->GetScopeSlot() + Js::ScopeSlots::FirstSlotIndex);

                    // Slot array properties are tracked in non-debug mode as well because they need to stay
                    // around for heap enumeration and escaping during attach/detach.
                    TrackSlotArrayPropertyForDebugger(debuggerScope, sym, sym->EnsurePosition(this), pnode->nop == knopConstDecl ? Js::DebuggerScopePropertyFlags_Const : Js::DebuggerScopePropertyFlags_None);
                }
            }
            else
            {
                if (sym->GetDecl()->sxVar.isSwitchStmtDecl && CONFIG_FLAG(TDZ))
                {
                    // let/const declared in a switch is the only case of a variable that must be checked for
                    // use-before-declaration dynamically within its own function.
                    this->m_writer.Reg1(Js::OpCode::InitUndecl, sym->GetLocation());
                }
                // Syms that begin in register may be delay-captured. In debugger mode, such syms
                // will live only in slots, so tell the debugger to find them there.
                if (sym->NeedsSlotAlloc(funcInfo))
                {
                    TrackSlotArrayPropertyForDebugger(debuggerScope, sym, sym->EnsurePosition(this), pnode->nop == knopConstDecl ? Js::DebuggerScopePropertyFlags_Const : Js::DebuggerScopePropertyFlags_None);
                }
                else
                {
                    TrackRegisterPropertyForDebugger(debuggerScope, sym, funcInfo, pnode->nop == knopConstDecl ? Js::DebuggerScopePropertyFlags_Const : Js::DebuggerScopePropertyFlags_None);
                }
            }
        };

    IterateBlockScopedVariables(pnodeBlock, genBlockInit);
}

// Records the start of a debugger scope if the passed in node has any let/const variables (or is not a block node).
// If it has no let/const variables, nullptr will be returned as no scope will be created.
Js::DebuggerScope* ByteCodeGenerator::RecordStartScopeObject(ParseNode *pnodeBlock, Js::DiagExtraScopesType scopeType, Js::RegSlot scopeLocation /*= Js::Constants::NoRegister*/, int* index /*= nullptr*/)
{
    Assert(pnodeBlock);
    if (pnodeBlock->nop == knopBlock && !pnodeBlock->sxBlock.HasBlockScopedContent())
    {
        // In order to reduce allocations now that we track debugger scopes in non-debug mode,
        // don't add a block to the chain if it has no let/const variables at all.
        return nullptr;
    }

    return this->Writer()->RecordStartScopeObject(scopeType, scopeLocation, index);
}

// Records the end of the current scope, but only if the current block has block scoped content.
// Otherwise, a scope would not have been added (see ByteCodeGenerator::RecordStartScopeObject()).
void ByteCodeGenerator::RecordEndScopeObject(ParseNode *pnodeBlock)
{
    Assert(pnodeBlock);
    if (pnodeBlock->nop == knopBlock && !pnodeBlock->sxBlock.HasBlockScopedContent())
    {
        return;
    }

    this->Writer()->RecordEndScopeObject();
}

template <class PrefixFn, class PostfixFn>
ParseNode* VisitBlock(ParseNode *pnode, ByteCodeGenerator* byteCodeGenerator, PrefixFn prefix, PostfixFn postfix, ParseNode *pnodeParent = NULL)
{
    ParseNode *pnodeLastVal = NULL;
    if (pnode!=NULL)
    {
        BOOL fTrackVal = byteCodeGenerator->IsBinding() &&
            (byteCodeGenerator->GetFlags() & fscrReturnExpression) &&
            byteCodeGenerator->TopFuncInfo()->IsGlobalFunction();
        while(pnode->nop==knopList)
        {
            Visit(pnode->sxBin.pnode1, byteCodeGenerator, prefix, postfix, pnodeParent);
            if (fTrackVal)
            {
                // If we're tracking values, find the last statement (if any) in the block that is
                // guaranteed to produce a value.
                if (MustProduceValue(pnode->sxBin.pnode1, byteCodeGenerator->GetScriptContext()))
                {
                    pnodeLastVal = pnode->sxBin.pnode1;
                }
                if (IsJump(pnode->sxBin.pnode1))
                {
                    // This is a jump out of the current block. The remaining instructions (if any)
                    // will not be executed, so stop tracking them.
                    fTrackVal = false;
                }
            }
            pnode = pnode->sxBin.pnode2;
        }
        Visit(pnode, byteCodeGenerator, prefix, postfix, pnodeParent);
        if (fTrackVal)
        {
            if (MustProduceValue(pnode, byteCodeGenerator->GetScriptContext()))
            {
                pnodeLastVal = pnode;
            }
        }
    }
    return pnodeLastVal;
}

void BeginEmitBlock(ParseNode *pnodeBlock, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    Js::DebuggerScope* debuggerScope = NULL;

    if (BlockHasOwnScope(pnodeBlock, byteCodeGenerator))
    {
        Scope *scope = pnodeBlock->sxBlock.scope;
        byteCodeGenerator->PushScope(scope);

        Js::RegSlot scopeLocation = scope->GetLocation();
        if (scope->GetMustInstantiate())
        {
            if (scopeLocation == Js::Constants::NoRegister)
            {
                scopeLocation = funcInfo->AcquireTmpRegister();
                scope->SetLocation(scopeLocation);
            }

            if (scope->GetIsObject())
            {
                debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnodeBlock, Js::DiagExtraScopesType::DiagBlockScopeInObject, scopeLocation);

                byteCodeGenerator->Writer()->Reg1(Js::OpCode::NewBlockScope, scopeLocation);
            }
            else
            {
                int scopeIndex = Js::DebuggerScope::InvalidScopeIndex;
                debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnodeBlock, Js::DiagExtraScopesType::DiagBlockScopeInSlot, scopeLocation, &scopeIndex);

                // TODO: Handle heap enumeration
                int scopeSlotCount = scope->GetScopeSlotCount();
                byteCodeGenerator->Writer()->Reg1Int2(Js::OpCode::NewScopeSlotsWithoutPropIds, scopeLocation, scopeSlotCount + Js::ScopeSlots::FirstSlotIndex, scopeIndex);
            }
        }
        else
        {
            // In the direct register access case, there is no block scope emitted but we can still track
            // the start and end offset of the block.  The location registers for let/const variables will still be
            // captured along with this range in InitBlockScopedContent().
            debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnodeBlock, Js::DiagExtraScopesType::DiagBlockScopeDirect);
        }

        bool const isGlobalEvalBlockScope = scope->IsGlobalEvalBlockScope();
        Js::RegSlot frameDisplayLoc = Js::Constants::NoRegister;
        Js::RegSlot tmpEnvReg = Js::Constants::NoRegister;
        Js::RegSlot tmpInnerEnvReg = Js::Constants::NoRegister;
        if (funcInfo->GetParsedFunctionBody()->DoStackFrameDisplay() && !byteCodeGenerator->InPrologue())
        {
            // The parent function allocated a frame display on the stack, but this declaration happens after
            // the frame display may have been boxed. So re-load it into a temp reg.
            // Do this here, because we're going to want to re-use the result for all funcs declared in this block.
            tmpEnvReg = funcInfo->AcquireTmpRegister();
            byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdLocalFrameDisplay, tmpEnvReg);
        }

        ParseNodePtr pnodeScope;
        for (pnodeScope = pnodeBlock->sxBlock.pnodeScopes; pnodeScope;)
        {
            switch(pnodeScope->nop)
            {
            case knopFncDecl:
                if (pnodeScope->sxFnc.IsDeclaration())
                {
                    // The frameDisplayLoc register's lifetime has to be controlled by this function. We can't let
                    // it be released by DefineOneFunction, because further iterations of this loop can allocate
                    // temps, and we can't let frameDisplayLoc be repurposed until this loop completes.
                    // So we'll supply a temp that we allocate and release here.
                    if (frameDisplayLoc == Js::Constants::NoRegister)
                    {
                        if (funcInfo->frameDisplayRegister != Js::Constants::NoRegister)
                        {
                            frameDisplayLoc = funcInfo->frameDisplayRegister;
                        }
                        else
                        {
                            frameDisplayLoc = funcInfo->GetEnvRegister();
                        }
                        tmpInnerEnvReg = funcInfo->AcquireTmpRegister();
                        frameDisplayLoc = byteCodeGenerator->PrependLocalScopes(frameDisplayLoc, tmpInnerEnvReg, funcInfo);
                    }
                    byteCodeGenerator->DefineOneFunction(pnodeScope, funcInfo, true, frameDisplayLoc, tmpEnvReg);
                }

                // If this is the global eval block scope, the function is actually assigned to the global
                // So we don't need to keep the registers
                if (isGlobalEvalBlockScope)
                {
                    funcInfo->ReleaseLoc(pnodeScope);
                    pnodeScope->location = Js::Constants::NoRegister;
                }
                pnodeScope = pnodeScope->sxFnc.pnodeNext;
                break;

            case knopBlock:
                pnodeScope = pnodeScope->sxBlock.pnodeNext;
                break;

            case knopCatch:
                pnodeScope = pnodeScope->sxCatch.pnodeNext;
                break;

            case knopWith:
                pnodeScope = pnodeScope->sxWith.pnodeNext;
                break;
            }
        }

        if (tmpInnerEnvReg != Js::Constants::NoRegister)
        {
            funcInfo->ReleaseTmpRegister(tmpInnerEnvReg);
        }
        if (tmpEnvReg != Js::Constants::NoRegister)
        {
            funcInfo->ReleaseTmpRegister(tmpEnvReg);
        }

        if (pnodeBlock->sxBlock.scope->IsGlobalEvalBlockScope() && funcInfo->thisScopeSlot != Js::Constants::NoRegister)
        {
            Scope* scope = funcInfo->GetGlobalEvalBlockScope();
            byteCodeGenerator->EmitInitCapturedThis(funcInfo, scope);
        }
    }
    else
    {
        Scope *scope = pnodeBlock->sxBlock.scope;
        if (scope)
        {
            if (scope->GetMustInstantiate())
            {
                debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnodeBlock, Js::DiagExtraScopesType::DiagBlockScopeInObject);
            }
            else
            {
                debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnodeBlock, Js::DiagExtraScopesType::DiagBlockScopeDirect);
            }
        }
        else
        {
            debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnodeBlock, Js::DiagExtraScopesType::DiagBlockScopeInSlot);
        }
    }

    byteCodeGenerator->InitBlockScopedContent(pnodeBlock, debuggerScope, funcInfo);
}

void EndEmitBlock(ParseNode *pnodeBlock, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    if (BlockHasOwnScope(pnodeBlock, byteCodeGenerator))
    {
        Scope *scope = pnodeBlock->sxBlock.scope;
        Assert(scope);
        Assert(scope == byteCodeGenerator->GetCurrentScope());
        if (scope->GetMustInstantiate())
        {
            funcInfo->ReleaseTmpRegister(scope->GetLocation());
        }

        byteCodeGenerator->PopScope();
    }

    byteCodeGenerator->RecordEndScopeObject(pnodeBlock);
}

void EmitBlock(ParseNode *pnodeBlock, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, BOOL fReturnValue)
{
    Assert(pnodeBlock->nop == knopBlock);
    ParseNode *pnode = pnodeBlock->sxBlock.pnodeStmt;
    if (pnode == NULL)
    {
        return;
    }

    BeginEmitBlock(pnodeBlock, byteCodeGenerator, funcInfo);

    ParseNode *pnodeLastValStmt = pnodeBlock->sxBlock.pnodeLastValStmt;

    while(pnode->nop == knopList)
    {
        ParseNode* stmt = pnode->sxBin.pnode1;
        if (stmt == pnodeLastValStmt)
        {
            // This is the last guaranteed return value, so any potential return values have to be
            // copied to the return register from this point forward.
            pnodeLastValStmt = NULL;
        }
        byteCodeGenerator->EmitTopLevelStatement(stmt, funcInfo, fReturnValue && (pnodeLastValStmt == NULL));
        pnode = pnode->sxBin.pnode2;
    }
    if (pnode == pnodeLastValStmt)
    {
        pnodeLastValStmt = NULL;
    }
    byteCodeGenerator->EmitTopLevelStatement(pnode, funcInfo, fReturnValue && (pnodeLastValStmt == NULL));

    EndEmitBlock(pnodeBlock, byteCodeGenerator, funcInfo);
}

FuncInfo * ByteCodeGenerator::StartBindFunction(const wchar_t *name, int nameLength, bool* pfuncExprWithName, ParseNode *pnode)
{
    bool funcExprWithName;
    union
    {
        Js::ParseableFunctionInfo* parseableFunctionInfo;
        Js::FunctionBody* parsedFunctionBody;
    };
    bool isDeferParsed = false;

    if (this->pCurrentFunction &&
        this->pCurrentFunction->IsFunctionParsed())
    {
        Assert(this->pCurrentFunction->StartInDocument()  == pnode->ichMin);
        Assert(this->pCurrentFunction->LengthInChars() == pnode->LengthInCodepoints());

        // This is the root function for the current AST subtree, and it already has a FunctionBody
        // (created by a deferred parse) which we're now filling in.
        parsedFunctionBody = this->pCurrentFunction;
        parsedFunctionBody->RemoveDeferParseAttribute();

        if (parsedFunctionBody->GetBoundPropertyRecords() == NULL)
        {
            Assert(!IsInNonDebugMode());

            // This happens when we try to re-use the function body which was created due to serialized bytecode.
            parsedFunctionBody->SetBoundPropertyRecords(EnsurePropertyRecordList());
        }

        Assert(!parsedFunctionBody->IsDeferredParseFunction() || parsedFunctionBody->IsReparsed());

        pnode->sxFnc.SetDeclaration(parsedFunctionBody->GetIsDeclaration());
        funcExprWithName =
            !(parsedFunctionBody->GetIsDeclaration() || pnode->sxFnc.IsMethod()) &&
            pnode->sxFnc.pnodeNames != NULL &&
            pnode->sxFnc.pnodeNames->nop == knopVarDecl;
        *pfuncExprWithName = funcExprWithName;

        Assert(parsedFunctionBody->GetLocalFunctionId() == pnode->sxFnc.functionId || !IsInNonDebugMode());

        // Some state may be tracked on the function body during the visit pass. Since the previous visit pass may have failed,
        // we need to reset the state on the function body.
        parsedFunctionBody->ResetByteCodeGenVisitState();

        if (parsedFunctionBody->GetScopeInfo())
        {
            // Propagate flags from the (real) parent function.
            Js::ParseableFunctionInfo *parent = parsedFunctionBody->GetScopeInfo()->GetParent();
            if (parent)
            {
                Assert(parent->GetFunctionBody());
                if (parent->GetFunctionBody()->GetHasOrParentHasArguments())
                {
                    parsedFunctionBody->SetHasOrParentHasArguments(true);
                }
            }
        }
    }
    else
    {
        funcExprWithName = *pfuncExprWithName;
        Js::LocalFunctionId functionId = pnode->sxFnc.functionId;

        isDeferParsed = (pnode->sxFnc.pnodeBody == null);

        // Create a function body if:
        //  1. The parse node is not defer parsed
        //  2. Or creating function proxies is disallowed
        bool createFunctionBody = !isDeferParsed;
        if (!CONFIG_FLAG(CreateFunctionProxy)) createFunctionBody = true;

        Js::FunctionInfo::Attributes attributes = Js::FunctionInfo::Attributes::None;
        if (pnode->sxFnc.IsLambda())
        {
            attributes = (Js::FunctionInfo::Attributes)(attributes | Js::FunctionInfo::Attributes::ErrorOnNew | Js::FunctionInfo::Attributes::Lambda);
        }
        if (pnode->sxFnc.HasSuperReference())
        {
            attributes = (Js::FunctionInfo::Attributes)(attributes | Js::FunctionInfo::Attributes::HasSuperReference);
        }
        if (pnode->sxFnc.IsClassConstructor() && pnode->sxFnc.IsGeneratedDefault())
        {
            attributes = (Js::FunctionInfo::Attributes)(attributes | Js::FunctionInfo::Attributes::IsDefaultConstructor);
        }
        if (pnode->sxFnc.IsGenerator())
        {
            attributes = (Js::FunctionInfo::Attributes)(attributes | Js::FunctionInfo::Attributes::Generator);
        }
        if (pnode->sxFnc.IsAccessor())
        {
            attributes = (Js::FunctionInfo::Attributes)(attributes | Js::FunctionInfo::Attributes::ErrorOnNew);
        }

        if (createFunctionBody)
        {
            DECLARE_STACK_PINNED(Js::PropertyRecordList, propertyRecordList);
            propertyRecordList = EnsurePropertyRecordList();
            parsedFunctionBody = Js::FunctionBody::NewFromRecycler(scriptContext, name, nameLength, pnode->sxFnc.nestedCount, m_utf8SourceInfo,
                m_utf8SourceInfo->GetSrcInfo()->sourceContextInfo->sourceContextId, functionId, propertyRecordList
                , attributes
#ifdef PERF_COUNTERS
                , false /* is function from deferred deserialized proxy */
#endif
            );
        }
        else
        {
            DECLARE_STACK_PINNED(Js::PropertyRecordList, propertyRecordList);
            propertyRecordList = null;

            if (funcExprWithName)
            {
                propertyRecordList = EnsurePropertyRecordList();
            }

            parseableFunctionInfo = Js::ParseableFunctionInfo::New(scriptContext, pnode->sxFnc.nestedCount, functionId, m_utf8SourceInfo, name, nameLength, propertyRecordList, attributes);
        }

        // In either case register the function reference
        scriptContext->RegisterDynamicFunctionReference(parseableFunctionInfo);

#if DBG
        parseableFunctionInfo->deferredParseNextFunctionId = pnode->sxFnc.deferredParseNextFunctionId;
#endif
        parseableFunctionInfo->SetIsDeclaration(pnode->sxFnc.IsDeclaration() != 0);
        parseableFunctionInfo->SetIsAccessor(pnode->sxFnc.IsAccessor() != 0);
        if (pnode->sxFnc.IsAccessor())
        {
            scriptContext->optimizationOverrides.SetSideEffects(Js::SideEffects_Accessor);
        }
#if LANGUAGE_SERVICE
        parseableFunctionInfo->SetIsClassMember(pnode->sxFnc.IsClassMember() != 0);
#endif
    }

    Scope *funcExprScope = null;
    if (funcExprWithName)
    {
        if (!UseParserBindings())
        {
            funcExprScope = Anew(alloc, Scope, alloc, ScopeType_FuncExpr, true);
            pnode->sxFnc.scope = funcExprScope;
        }
        else
        {
            funcExprScope = pnode->sxFnc.scope;
            Assert(funcExprScope);
        }
        PushScope(funcExprScope);
        Symbol *sym = AddSymbolToScope(funcExprScope, name, nameLength, pnode->sxFnc.pnodeNames, STFunction);

        sym->SetFuncExpr(true);

        sym->SetPosition(parsedFunctionBody->GetOrAddPropertyIdTracked(sym->GetName()));

        pnode->sxFnc.SetFuncSymbol(sym);
    }

    Scope *paramScope = pnode->sxFnc.pnodeScopes ? pnode->sxFnc.pnodeScopes->sxBlock.scope : nullptr;
    Scope *bodyScope = pnode->sxFnc.pnodeBodyScope ? pnode->sxFnc.pnodeBodyScope->sxBlock.scope : nullptr;
    Assert(paramScope != nullptr|| !pnode->sxFnc.pnodeScopes || !UseParserBindings());
    if (paramScope == null || !UseParserBindings())
    {
        paramScope = Anew(alloc, Scope, alloc, ScopeType_Parameter, true);
        if (pnode->sxFnc.pnodeScopes)
        {
            pnode->sxFnc.pnodeScopes->sxBlock.scope = paramScope;
        }
    }
    if (bodyScope == null || !UseParserBindings())
    {
        bodyScope = Anew(alloc, Scope, alloc, ScopeType_FunctionBody, true);
        if (pnode->sxFnc.pnodeBodyScope)
        {
            pnode->sxFnc.pnodeBodyScope->sxBlock.scope = bodyScope;
        }
    }

    AssertMsg(pnode->nop == knopFncDecl, "Non function declaration trying to create function body");

    parseableFunctionInfo->SetIsGlobalFunc(FALSE);
    if (pnode->sxFnc.GetStrictMode() != 0)
    {
        parseableFunctionInfo->SetIsStrictMode();
    }

    FuncInfo *funcInfo = Anew(alloc, FuncInfo, name, alloc, paramScope, bodyScope, pnode, parseableFunctionInfo);

    if (pnode->sxFnc.GetArgumentsObjectEscapes())
    {
        // If the parser detected that the arguments object escapes, then the function scope escapes
        // and cannot be cached.
        this->FuncEscapes(bodyScope);
        funcInfo->SetHasMaybeEscapedNestedFunc(DebugOnly(L"ArgumentsObjectEscapes"));
    }

    if(!isDeferParsed)
    {
        if(parsedFunctionBody->IsReparsed())
        {
            parsedFunctionBody->RestoreState(pnode);
        }
        else
        {
            parsedFunctionBody->SaveState(pnode);
        }
    }

    funcInfo->SetChildCallsEval(!!pnode->sxFnc.ChildCallsEval());

    if (pnode->sxFnc.CallsEval())
    {
        funcInfo->SetCallsEval(true);
        bodyScope->SetIsDynamic(true);
        bodyScope->SetIsObject();
        bodyScope->SetCapturesAll(true);
        bodyScope->SetMustInstantiate(true);
        paramScope->SetIsObject();
        paramScope->SetMustInstantiate(true);
        paramScope->SetCapturesAll(true);
    }
    else if (BinaryFeatureControl::LanguageService() && IsLanguageServiceEnabled())
    {
        paramScope->SetIsObject();
        bodyScope->SetIsObject();
    }

    PushFuncInfo(L"StartBindFunction", funcInfo);
    PushScope(paramScope);
    PushScope(bodyScope);

    if (funcExprScope)
    {
        funcExprScope->SetFunc(funcInfo);
        funcInfo->funcExprScope = funcExprScope;
    }

    long currentAstSize = pnode->sxFnc.astSize;
    if (currentAstSize > this->maxAstSize)
    {
        this->maxAstSize = currentAstSize;
    }

    return funcInfo;
}

void ByteCodeGenerator::EndBindFunction(bool funcExprWithName) {
    bool isGlobalScope = currentScope->GetScopeType() == ScopeType_Global;

    Assert(currentScope->GetScopeType() == ScopeType_FunctionBody || isGlobalScope);
    PopScope(); // function body

    if (isGlobalScope)
    {
        Assert(currentScope == nullptr);
    }
    else
    {
        Assert(currentScope->GetScopeType() == ScopeType_Parameter);
        PopScope(); // parameter scope
    }

    if (funcExprWithName)
    {
        Assert(currentScope->GetScopeType() == ScopeType_FuncExpr);
        PopScope();
    }

    funcInfoStack->Pop();
}

void ByteCodeGenerator::StartBindCatch(ParseNode *pnode)
{
    Scope *scope = pnode->sxCatch.scope;
    Assert(scope);
    if (scope == null || !UseParserBindings())
    {
        scope = Anew(alloc, Scope, alloc, ScopeType_Catch, true);
        pnode->sxCatch.scope = scope;
    }
    Assert(currentScope);
    scope->SetFunc(currentScope->GetFunc());
    PushScope(scope);
}

void ByteCodeGenerator::EndBindCatch() {
    PopScope();
}

void ByteCodeGenerator::PushScope(Scope *innerScope) {
    Assert(innerScope != nullptr);

    if (isBinding
        && currentScope != nullptr
        && currentScope->GetScopeType() == ScopeType_FunctionBody
        && innerScope->GetMustInstantiate())
    {
        // If the current scope is a function body, we don't expect another function body
        // without going through a function expression or parameter scope first. This may
        // not be the case in the emit phase, where we can merge the two scopes. This also
        // does not apply to incoming scopes marked as !mustInstantiate.
        Assert(innerScope->GetScopeType() != ScopeType_FunctionBody);
    }

    innerScope->SetEnclosingScope(currentScope);

    currentScope = innerScope;

    if (currentScope->GetIsDynamic())
    {
        this->dynamicScopeCount++;
    }

    if (this->trackEnvDepth && currentScope->GetMustInstantiate())
    {
        this->envDepth++;
        if (this->envDepth == 0)
        {
            Js::Throw::OutOfMemory();
        }
    }
}

void ByteCodeGenerator::PopScope()
{
    Assert(currentScope != nullptr);
    if (this->trackEnvDepth && currentScope->GetMustInstantiate())
    {
        this->envDepth--;
        Assert(this->envDepth != (uint16)-1);
    }
    if (currentScope->GetIsDynamic())
    {
        this->dynamicScopeCount--;
    }
    currentScope=currentScope->GetEnclosingScope();
}

void ByteCodeGenerator::PushBlock(ParseNode *pnode)
{
    pnode->sxBlock.SetEnclosingBlock(currentBlock);
    currentBlock = pnode;
}

void ByteCodeGenerator::PopBlock()
{
    currentBlock = currentBlock->sxBlock.GetEnclosingBlock();
}

Symbol * ByteCodeGenerator::FindSymbol(Symbol **symRef, IdentPtr pid, bool forReference)
{
    const wchar_t *key = null;
    int keyLength;

    Symbol *sym = null;
    if (!UseParserBindings())
    {
        key = reinterpret_cast<const wchar_t*>(pid->Psz());
        keyLength = pid->Cch();
        sym = currentScope->FindSymbol(SymbolName(key, keyLength), STUnknown);
        if (symRef)
        {
            *symRef = sym;
        }
    }
    else
    {
        Assert(symRef);
        if (*symRef)
        {
            sym = *symRef;
        }
        else
        {
            this->AssignPropertyId(pid);
            return null;
        }
    }

    Scope *symScope = sym->GetScope();
    Assert(symScope);

    if (this->Trace())
    {
        if (sym!=NULL)
        {
            Output::Print(L"resolved %s to symbol of type %s: \n", key,sym->GetSymbolTypeName());
        }
        else Output::Print(L"did not resolve %s\n",key);
    }

    if (!(sym->GetIsGlobal()))
    {
        FuncInfo *top=funcInfoStack->Top();

        BOOL nonLocalRef = symScope->GetFunc() != top;
        if (forReference)
        {
            Js::PropertyId i;
            Scope *scope = FindScopeForSym(symScope, null, &i, top);
            // If we have a reference to a local within a with, we want to generate a closure represented by an object.
            if (scope != symScope && scope->GetIsDynamic())
            {
                nonLocalRef = true;
                symScope->SetIsObject();
            }
        }

        if (nonLocalRef)
        {
            // Symbol referenced through a closure. Mark it as such and give it a property ID.
            sym->SetHasNonLocalReference(true, this);
            sym->SetPosition(top->byteCodeFunction->GetOrAddPropertyIdTracked(sym->GetName()));
            // If this is var is local to a function (meaning that it belongs to the function's scope
            // *or* to scope that need not be instantiated, like a function expression scope, which we'll
            // merge with the function scope, then indicate that fact.
            symScope->SetHasLocalInClosure(true);
            if (symScope->GetFunc()->GetHasArguments() && sym->GetIsFormal())
            {
                // A formal is referenced non-locally. We need to allocate it on the heap, so
                // do the same for the whole arguments object.

                // formal is referenced. So count of formals to function > 0.
                // So no need to check for inParams here

                symScope->GetFunc()->SetHasHeapArguments(true);
            }
            if (symScope->GetFunc() != top)
            {
                top->SetHasClosureReference(true);
            }
        }
        else if (sym->GetHasNonLocalReference() && !sym->GetIsCommittedToSlot() && !sym->HasVisitedCapturingFunc())
        {
            sym->SetHasNonCommittedReference(true);
        }

        if (sym->GetFuncExpr())
        {
            symScope->GetFunc()->SetFuncExprNameReference(true);
            // If the func expr is captured by a closure, and if it's not getting its own scope,
            // we have to make the function's scope a dynamic object. That's because we have to init
            // the property as non-writable, and if we put it directly in the slots we can't protect it from being
            // overwritten.
            if (nonLocalRef && !symScope->GetFunc()->GetCallsEval() && !symScope->GetFunc()->GetChildCallsEval())
            {
                symScope->GetFunc()->GetParamScope()->SetIsObject();
                symScope->GetFunc()->GetBodyScope()->SetIsObject();
            }
        }
    }
    return sym;
}

void Symbol::SetHasNonLocalReference(bool b, ByteCodeGenerator *byteCodeGenerator)
{
    this->hasNonLocalReference = b;

    // The symbol's home function will tell us which child function we're currently processing.
    // This is the one that captures the symbol, from the declaring function's perspective.
    // So based on that information note either that, a. the symbol is committed to the heap from its
    // inception, b. the symbol must be committed when the capturing function is instantiated.

    FuncInfo *funcHome = this->scope->GetFunc();
    FuncInfo *funcChild = funcHome->GetCurrentChildFunction();

    // If this is not a local property, or not all its references can be tracked, or
    // it's not scoped to the function, or we're in debug mode, disable the delayed capture optimization.
    if (funcHome->IsGlobalFunction() ||
        funcHome->GetCallsEval() ||
        funcHome->GetChildCallsEval() ||
        funcChild == nullptr ||
        this->GetScope() != funcHome->GetBodyScope() ||
        byteCodeGenerator->IsInDebugMode() ||
        PHASE_OFF(Js::DelayCapturePhase, funcHome->byteCodeFunction))
    {
        this->SetIsCommittedToSlot();
    }

    if (this->isCommittedToSlot)
    {
        return;
    }

    ParseNode *pnodeChild = funcChild->root;

    Assert(pnodeChild && pnodeChild->nop == knopFncDecl);

    if (pnodeChild->sxFnc.IsDeclaration())
    {
        // The capturing function is a declaration but may still be limited to an inner scope.
        Scope *scopeChild = funcHome->GetCurrentChildScope();
        if (scopeChild == this->scope || scopeChild->GetScopeType() == ScopeType_FunctionBody)
        {
            // The symbol is captured on entry to the scope in which it's declared.
            // (Check the scope type separately so that we get the special parameter list and
            // named function expression cases as well.)
            this->SetIsCommittedToSlot();
            return;
        }
    }

    // There is a chance we can limit the region in which the symbol lives on the heap.
    // Note which function captures the symbol.
    funcChild->AddCapturedSym(this);
}

void FuncInfo::AddCapturedSym(Symbol *sym)
{
    if (this->capturedSyms == nullptr)
    {
        this->capturedSyms = Anew(alloc, SymbolTable, alloc);
    }
    this->capturedSyms->AddNew(sym);
}

Symbol * ByteCodeGenerator::FindLocalSymbol(SymbolName const& key)
{
    Symbol *sym = currentScope->FindLocalSymbol(key);
    if(this->Trace())
    {
        if(sym!=NULL)
        {
            Output::Print(L"resolved %s to symbol of type %s\n", key.GetBuffer(), sym->GetSymbolTypeName());
        }
        else
        {
            Output::Print(L"did not resolve %s\n", key.GetBuffer());
        }
    }

    if(sym && sym->GetFuncExpr())
    {
        currentScope->GetFunc()->SetFuncExprNameReference(true);
    }

    return sym;
}

Symbol * ByteCodeGenerator::AddSymbolToScope(Scope *scope, const wchar_t *key, int keyLength, ParseNode *varDecl, SymbolType symbolType)
{
    Symbol *sym = nullptr;

    if (!UseParserBindings())
    {
        SymbolName const symName(key, keyLength);

        if (scope->GetScopeType() == ScopeType_FunctionBody)
        {
            sym = scope->GetFunc()->GetParamScope()->FindLocalSymbol(symName);
        }

        if (sym == nullptr)
        {
            sym = scope->FindLocalSymbol(symName);
        }

        if (sym == nullptr)
        {
            sym = Anew(alloc, Symbol, symName, varDecl, symbolType);

            scope->AddNewSymbol(sym);
            if (this->Trace())
            {
                Output::Print(L"added symbol %s of type %s to scope %x\n",key,sym->GetSymbolTypeName(), scope);
            }
        }
    }
    else
    {
        switch (varDecl->nop)
        {
        case knopConstDecl:
        case knopLetDecl:
        case knopVarDecl:
            sym = varDecl->sxVar.sym/*New*/;
            break;
        case knopName:
            Assert(varDecl->sxPid.symRef);
            sym = *varDecl->sxPid.symRef;
            break;
        default:
            Assert(0);
            sym = null;
            break;
        }

        if (sym->GetScope() != scope && sym->GetScope()->GetScopeType() != ScopeType_Parameter)
        {
            // Nasty. This can happen when we have a function declared at global eval scope, and it has
            // references in deferred function bodies inside the eval. The BCG creates a new global scope
            // on such compiles, so we essentially have to migrate the symbol to the new scope.
            // We check fscrEvalCode, not fscrEval, because the same thing can happen in indirect eval,
            // when fscrEval is not set.
            Assert((this->flags & fscrEvalCode) && sym->GetIsGlobal() && sym->GetSymbolType() == STFunction);
            Assert(scope->GetScopeType() == ScopeType_Global);
            scope->AddNewSymbol(sym);
        }
    }

    Assert(sym && sym->GetScope() && (sym->GetScope() == scope || sym->GetScope()->GetScopeType() == ScopeType_Parameter));

    return sym;
}

Symbol * ByteCodeGenerator::AddSymbolToFunctionScope(const wchar_t *key, int keyLength, ParseNode *varDecl, SymbolType symbolType)
{
    Scope* scope = currentScope->GetFunc()->GetBodyScope();
    return this->AddSymbolToScope(scope, key, keyLength, varDecl, symbolType);
}

void ClearTmpRegs(ParseNode* pnode,ByteCodeGenerator* byteCodeGenerator,FuncInfo* emitFunc) {
    if (emitFunc->IsTmpReg(pnode->location)) {
        pnode->location=Js::Constants::NoRegister;
    }
}

void ByteCodeGenerator::EmitTopLevelStatement(ParseNode *stmt, FuncInfo *funcInfo, BOOL fReturnValue)
{
    if (stmt->nop == knopFncDecl && stmt->sxFnc.IsDeclaration())
    {
        // Function declarations (not function-declaration RHS's) are already fully processed.
        // Skip them here so the temp registers don't get messed up.
        return;
    }

    if (stmt->nop == knopName || stmt->nop == knopDot)
    {
        // Generating span for top level names are mostly useful in debugging mode, because user can debug it even though no side-effect expected.
        // But the name can have runtime error, eg, foo.bar; // where foo is not defined.
        // At this time we need to throw proper line number and offset. so recording on all modes will be useful.
        StartStatement(stmt);
        Writer()->Empty(Js::OpCode::Nop);
        EndStatement(stmt);
    }

    Emit(stmt, this, funcInfo, fReturnValue);
    if (funcInfo->IsTmpReg(stmt->location))
    {
        if (!stmt->isUsed && !fReturnValue)
        {
            m_writer.Reg1(Js::OpCode::Unused, stmt->location);
        }
        funcInfo->ReleaseLoc(stmt);
    }
}

// ByteCodeGenerator::DefineFunctions
//
// Emit byte code for scope-wide function definitions before any calls in the scope, regardless of lexical
// order. Note that stores to the closure array are not emitted until we see the knopFncDecl in the tree
// to make sure that sources of the stores have been defined.
//
// This returns true if there were userVar dependent functions that couldn't be defined at the point DefineFunctions
// was called. If true is returned, DefineFunctions must be called again once userVars have been defined.
bool ByteCodeGenerator::DefineFunctions(FuncInfo *funcInfoParent, bool userVarsDeclared)
{
    // DefineCachedFunctions doesn't depend on whether the user vars are declared or not, so
    // we'll just overload this variable to mean that the functions getting called again and we don't need to do anything
    if (funcInfoParent->GetHasCachedScope())
    {
        this->DefineCachedFunctions(funcInfoParent);
        return false;
    }
    else
    {
        return this->DefineUncachedFunctions(funcInfoParent, userVarsDeclared);
    }
}

// Iterate over all child functions in a function's parameter and body scopes.
template<typename Fn>
void MapContainerScopeFunctions(ParseNode* pnodeScope, Fn fn)
{
    auto mapFncDeclsInScopeList = [&](ParseNode *pnodeHead)
    {
        for (ParseNode *pnode = pnodeHead; pnode != nullptr;)
        {
            switch (pnode->nop)
            {
            case knopFncDecl:
                fn(pnode);
                pnode = pnode->sxFnc.pnodeNext;
                break;

            case knopBlock:
                pnode = pnode->sxBlock.pnodeNext;
                break;

            case knopCatch:
                pnode = pnode->sxCatch.pnodeNext;
                break;

            case knopWith:
                pnode = pnode->sxWith.pnodeNext;
                break;

            default:
                AssertMsg(false, "Unexpected opcode in tree of scopes");
                return;
            }
        }
    };
    pnodeScope->sxFnc.MapContainerScopes(mapFncDeclsInScopeList);
}

void ByteCodeGenerator::DefineCachedFunctions(FuncInfo *funcInfoParent)
{
    ParseNode *pnodeParent=funcInfoParent->root;
    uint slotCount = 0;

    auto countFncSlots = [&](ParseNode *pnodeFnc)
    {
        if (pnodeFnc->sxFnc.GetFuncSymbol() != nullptr && pnodeFnc->sxFnc.IsDeclaration())
        {
            slotCount++;
        }
    };
    MapContainerScopeFunctions(pnodeParent, countFncSlots);

    if (slotCount == 0)
    {
        return;
    }

    int extraBytes = slotCount * sizeof(Js::FuncInfoEntry);
    Js::FuncInfoArray *info = AnewPlus(alloc, extraBytes, Js::FuncInfoArray, slotCount);

    slotCount = 0;

    auto fillEntries = [&](ParseNode *pnodeFnc)
    {
        Symbol *sym = pnodeFnc->sxFnc.GetFuncSymbol();
        if (sym != nullptr && (pnodeFnc->sxFnc.IsDeclaration()))
        {
            AssertMsg(!pnodeFnc->sxFnc.IsGenerator(), "Generator functions are not supported by InitCachedFuncs but since they always escape they should disable function caching");
            Js::FuncInfoEntry *entry = &info->elements[slotCount];
            entry->nestedIndex = pnodeFnc->sxFnc.nestedIndex;
            entry->scopeSlot = sym->GetScopeSlot();
            slotCount++;
        }
    };
    MapContainerScopeFunctions(pnodeParent, fillEntries);

    m_writer.Reg2Aux(Js::OpCode::InitCachedFuncs,
                     funcInfoParent->frameObjRegister,
                     funcInfoParent->frameDisplayRegister,
                     info,
                     sizeof(Js::FuncInfoArray) + extraBytes,
                     sizeof(Js::FuncInfoArray) + extraBytes);

    slotCount = 0;
    auto defineOrGetCachedFunc = [&](ParseNode *pnodeFnc)
    {
        Symbol *sym = pnodeFnc->sxFnc.GetFuncSymbol();
        if (pnodeFnc->sxFnc.IsDeclaration())
        {
            // Do we need to define the function here (i.e., is it not one of our cached locals)?
            // Only happens if the sym is null (e.g., function x.y(){}).
            if (sym == nullptr)
            {
                this->DefineOneFunction(pnodeFnc, funcInfoParent);
            }
            else if (!sym->IsInSlot(funcInfoParent) && sym->GetLocation() != Js::Constants::NoRegister)
            {
                // If it was defined by InitCachedFuncs, do we need to put it in a register rather than a slot?
                m_writer.Reg2Int1(Js::OpCode::GetCachedFunc, sym->GetLocation(), funcInfoParent->frameObjRegister, slotCount);
            }
            // The "x = function() {...}" case is being generated on the fly, during emission,
            // so the caller expects to be able to release this register.
            funcInfoParent->ReleaseLoc(pnodeFnc);
            pnodeFnc->location = Js::Constants::NoRegister;
            slotCount++;
        }
    };
    MapContainerScopeFunctions(pnodeParent, defineOrGetCachedFunc);

    AdeletePlus(alloc, extraBytes, info);
}

//
// This method checks if a parse node has either a dot or a scope operator
//
inline bool IsUserVarDependentParseNode(ParseNode* pNode, bool clearHasInit)
{
    if (pNode != NULL && (pNode->nop == knopDot || pNode->nop == knopScope)) {
        if (clearHasInit)
        {
            ParseNode* pNodeTarget = pNode->sxBin.pnode1;
            while (pNodeTarget->nop == knopDot)
            {
                pNodeTarget = pNodeTarget->sxBin.pnode1;
            }
            if (pNodeTarget->nop == knopName && pNodeTarget->sxPid.sym)
            {
                pNodeTarget->sxPid.sym->SetHasInit(false);
            }
        }

        return true;
    }

    return false;
}
//
// This method indicates whether a function declaration ParseNode is dependent on the user vars being declared
// The heuristic is to check if the function declaration is of the form x::y or x.y- here method x depends on y
// clearHasInit is an optional flag. If its set, we clear the hasInit flag of the x's associated
// symbol iff pFuncNode is a user var dependent function
inline bool IsUserVarDependentFunctionDeclaration(ParseNode* pFuncNode, bool clearHasInit = false)
{
    ParseNodePtr pNode = pFuncNode->sxFnc.pnodeNames;
    bool retVal = false;

    if (pNode != NULL) {
        while(pNode->nop == knopList) {
            retVal |= IsUserVarDependentParseNode(pNode->sxBin.pnode1, clearHasInit);
            pNode = pNode->sxBin.pnode2;
        }
        retVal |= IsUserVarDependentParseNode(pNode, clearHasInit);
    }

    return retVal;
}
bool ByteCodeGenerator::DefineUncachedFunctions(FuncInfo *funcInfoParent, bool userVarsDeclared)
{
    ParseNode *pnodeParent = funcInfoParent->root;
    bool callFunctionAgain = false;

    auto defineCheck = [&](ParseNode *pnodeFnc)
    {
        Assert(pnodeFnc->nop == knopFncDecl);
        //
        // Don't define the function upfront in following cases
        // 1. x = function() {...};
        //    Don't define the function for all modes
        //    Such a function can only be accessed via the LHS, so we define it at the assignment point
        //    rather than the scope entry to save a register (and possibly save the whole definition).
        //
        // 2. x = function f() {...};            
        //    In ES5 mode, f is not visible in the enclosing scope
        //    Such function expressions should be emitted only at the assignment point, as can be used only
        //    after the assignment. Might save register
        //

        // We pass in true for clearHasInit if the user vars haven't been declared so that the target will be initialized to undefined
        // in DefineUserVars.
        bool isUserVarDependentFunctionDeclaration = IsUserVarDependentFunctionDeclaration(pnodeFnc, !userVarsDeclared);

        if (pnodeFnc->sxFnc.IsDeclaration())
        {
            // * If userVars are not declared, we'll define *only* functions
            // not dependent on the user vars being declared
            // * If userVars are declared, we'll define *only* functions
            // dependent on the user vars being declared, since the rest
            // would have been defined earlier
            if (isUserVarDependentFunctionDeclaration == userVarsDeclared) {
                this->DefineOneFunction(pnodeFnc, funcInfoParent);
                // The "x = function() {...}" case is being generated on the fly, during emission,
                // so the caller expects to be able to release this register.
                funcInfoParent->ReleaseLoc(pnodeFnc);
                pnodeFnc->location = Js::Constants::NoRegister;
            }
            else {
                callFunctionAgain = true;
            }
        }
    };
    MapContainerScopeFunctions(pnodeParent, defineCheck);

    return callFunctionAgain;
}

void EmitAssignmentToFuncName(ParseNode *pnodeFnc, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfoParent)
{
    VisitFncNames(pnodeFnc, [byteCodeGenerator, funcInfoParent](ParseNode *pnodeFnc, ParseNode *pnodeName)
    {
        // Assign the location holding the func object reference to the given name.
        Symbol *sym;
        switch (pnodeName->nop)
        {
        case knopVarDecl:
            sym = pnodeName->sxVar.sym;
            if (sym != NULL && !sym->GetFuncExpr())
            {
                if (sym->GetIsGlobal())
                {
                    Js::PropertyId propertyId = sym->GetPosition();
                    byteCodeGenerator->EmitGlobalFncDeclInit(pnodeFnc->location, propertyId, funcInfoParent);
                    if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsBlockScopeEnabled() &&
                        byteCodeGenerator->GetFlags() & fscrEval && !funcInfoParent->GetIsStrictMode())
                    {
                        byteCodeGenerator->EmitPropStore(pnodeFnc->location, sym, null, funcInfoParent);
                    }
                }
                else
                {
                    if (sym->NeedsSlotAlloc(funcInfoParent))
                    {
                        if (!sym->GetHasNonCommittedReference() ||
                            (funcInfoParent->GetParsedFunctionBody()->DoStackNestedFunc()))
                        {
                            // No point in trying to optimize if there are no references before we have to commit to slot.
                            // And not safe to delay putting a stack function in the slot, since we may miss boxing.
                            sym->SetIsCommittedToSlot();
                        }
                    }

                    byteCodeGenerator->EmitLocalPropInit(pnodeFnc->location, sym, funcInfoParent);
                    Symbol * fncScopeSym = sym->GetFuncScopeVarSym();

                    if (fncScopeSym)
                    {
                        Assert(byteCodeGenerator->GetScriptContext()->GetConfig()->IsBlockScopeEnabled());

                        if (fncScopeSym->GetIsGlobal() && byteCodeGenerator->GetFlags() & fscrEval)
                        {
                            Js::PropertyId propertyId = fncScopeSym->GetPosition();
                            byteCodeGenerator->EmitGlobalFncDeclInit(pnodeFnc->location, propertyId, funcInfoParent);
                        }
                        else
                        {
                            byteCodeGenerator->EmitPropStore(pnodeFnc->location, fncScopeSym, null, funcInfoParent, false, false, /* isFncDeclVar */true);
                        }
                    }
                }
            }
            break;

        case knopScope:
        case knopDot:
            EmitReference(pnodeName, byteCodeGenerator, funcInfoParent);
            EmitAssignment(NULL, pnodeName, pnodeFnc->location, byteCodeGenerator, funcInfoParent);
            funcInfoParent->ReleaseReference(pnodeName);
            break;

        default:
            AssertMsg(0, "Unexpected opcode on function name");
            break;
        }
    });
}

Js::RegSlot ByteCodeGenerator::DefineOneFunctionHandleBoxedFD(
    ParseNode *pnodeFnc, FuncInfo *funcInfo, bool generateAssignment, Js::RegSlot regEnv)
{
    Js::RegSlot tmpEnvReg = Js::Constants::NoRegister;
    if (funcInfo->GetParsedFunctionBody()->DoStackFrameDisplay())
    {
        // The parent function allocated a frame display on the stack, but this declaration happens after
        // the frame display may have been boxed. So re-load it into a temp reg.
        // Do this here, because we're going to want to re-use the result for all funcs declared in this block.
        funcInfo->AcquireLoc(pnodeFnc);
        tmpEnvReg = funcInfo->AcquireTmpRegister();
        m_writer.Reg1(Js::OpCode::LdLocalFrameDisplay, tmpEnvReg);
    }
    regEnv = this->DefineOneFunction(pnodeFnc, funcInfo, generateAssignment, regEnv, tmpEnvReg);
    if (tmpEnvReg != Js::Constants::NoRegister)
    {
        funcInfo->ReleaseTmpRegister(tmpEnvReg);
    }

    return regEnv;
}

Js::RegSlot ByteCodeGenerator::DefineOneFunction(ParseNode *pnodeFnc, FuncInfo *funcInfoParent, bool generateAssignment, Js::RegSlot regEnv, Js::RegSlot frameDisplayTemp)
{
    Assert(pnodeFnc->nop == knopFncDecl);

    funcInfoParent->AcquireLoc(pnodeFnc);

    if (regEnv == Js::Constants::NoRegister)
    {
        // If the child needs a closure, find a heap-allocated frame to pass to it.
        if (frameDisplayTemp != Js::Constants::NoRegister)
        {
            // We allocated a temp to hold a local frame display value. Use that.
            // It's likely that the FD is on the stack, and we used the temp to load it back.
            regEnv = frameDisplayTemp;
        }
        else if (funcInfoParent->frameDisplayRegister != Js::Constants::NoRegister)
        {
            // This function has built a frame display, so pass it down.
            regEnv = funcInfoParent->frameDisplayRegister;
        }
        else
        {
            // This function has no captured locals but inherits a closure environment, so pass it down.
            regEnv = funcInfoParent->GetEnvRegister();
        }

        regEnv = this->PrependLocalScopes(regEnv, Js::Constants::NoRegister, funcInfoParent);
    }

    /*
    AssertMsg(funcInfo->nonLocalSymbols == 0 || regEnv != funcInfoParent->nullConstantRegister,
    "We need a closure for the nested function");
    */

    m_writer.NewFunction(pnodeFnc->location, pnodeFnc->sxFnc.nestedIndex, regEnv, pnodeFnc->sxFnc.IsGenerator());

    if (funcInfoParent->IsGlobalFunction() && (this->flags & fscrEval))
    {
        // A function declared at global scope in eval is untrackable,
        // so make sure the caller's cached scope is invalidated.
        this->funcEscapes = true;
    }
    else
    {
        if (pnodeFnc->sxFnc.IsDeclaration())
        {
            Symbol * funcSymbol = pnodeFnc->sxFnc.GetFuncSymbol();
            if (funcSymbol)
            {
                // In the case where a let/const declaration is the same symbol name
                // as the function declaration (shadowing case), the let/const var and
                // the function declaration symbol are the same and share the same flags
                // (particularly, sym->GetIsBlockVar() for this code path).
                //
                // For example:
                // let a = 0;       // <-- sym->GetIsBlockVar() = true
                // function b(){}   // <-- sym2->GetIsBlockVar() = false
                //
                // let x = 0;       // <-- sym3->GetIsBlockVar() = true
                // function x(){}   // <-- sym3->GetIsBlockVar() = true
                //
                // In order to tell if the function is actually part
                // of a block scope, we compare against the function scope here.
                // Note that having a function with the same name as a let/const declaration
                // is a redeclaration error, but we're pushing the fix for this out since it's
                // a bit involved.
                // TODO: Once the redeclaration error is in place, this boolean can be replaced
                // by funcSymbol->GetIsBlockVar().
                // UPDATE: Relying on a redeclaration error here is not possible because the
                // language service can skip the redeclaration error and cause failures.
                Assert(funcInfoParent->GetBodyScope() != nullptr && funcSymbol->GetScope() != nullptr);
                bool isFunctionDeclarationInBlock = funcSymbol->GetScope() != funcInfoParent->GetBodyScope();

                // Track all vars/lets/consts register slot function declarations.
                if (ShouldTrackDebuggerMetadata()
                 // If this is a let binding function declaration at global level, we want to
                 // be sure to track the register location as well.
                 && !(funcInfoParent->IsGlobalFunction() && !isFunctionDeclarationInBlock))
                {
                    if (!funcSymbol->IsInSlot(funcInfoParent))
                    {
                        funcInfoParent->byteCodeFunction->GetFunctionBody()->InsertSymbolToRegSlotList(funcSymbol->GetName(), pnodeFnc->location, funcInfoParent->varRegsCount);
                    }
                }

                if (isFunctionDeclarationInBlock)
                {
                    // We only track inner let bindings for the debugger side.
                    this->TrackFunctionDeclarationPropertyForDebugger(funcSymbol, funcInfoParent);
                }
            }
        }
    }

    if (pnodeFnc->sxFnc.pnodeNames == NULL || !generateAssignment)
    {
        return regEnv;
    }

    EmitAssignmentToFuncName(pnodeFnc, this, funcInfoParent);

    return regEnv;
}

void ByteCodeGenerator::DefineUserVars(FuncInfo *funcInfo)
{
    // Initialize scope-wide variables on entry to the scope. TODO: optimize by detecting uses that are always reached
    // by an existing initialization.

    BOOL fGlobal = funcInfo->IsGlobalFunction();
    ParseNode *pnode;
    Js::FunctionBody *byteCodeFunction = funcInfo->GetParsedFunctionBody();
    // Global declarations need a temp register to hold the init value, but the node shouldn't get a register.
    // Just assign one on the fly and re-use it for all initializations.
    Js::RegSlot tmpReg = fGlobal ? funcInfo->AcquireTmpRegister() : Js::Constants::NoRegister;

    for (pnode = funcInfo->root->sxFnc.pnodeVars; pnode; pnode = pnode->sxVar.pnodeNext)
    {
        Symbol* sym = pnode->sxVar.sym;

        if (sym != null && !(pnode->sxVar.isBlockScopeFncDeclVar && sym->GetIsBlockVar()))
        {
            if (sym->GetIsCatch() || (pnode->nop == knopVarDecl && sym->GetIsBlockVar()))
            {
                // The init node was bound to the catch object, because it's inside a catch and has the
                // same name as the catch object. But we want to define a user var at function scope,
                // so find the right symbol. (We'll still assign the RHS value to the catch object symbol.)
                // This also applies to a var declaration in the same scope as a let declaration.
#if DBG
                if (!sym->GetIsCatch())
                {
                    Assert(BinaryFeatureControl::LanguageService() || funcInfo->bodyScope != sym->GetScope() || !this->scriptContext->GetConfig()->IsBlockScopeEnabled());  // catch cannot be at function scope and let and var at function scope is redeclaration error
                }
#endif                
                sym = funcInfo->bodyScope->FindLocalSymbol(sym->GetName());
                Assert(BinaryFeatureControl::LanguageService() || sym && !sym->GetIsCatch() && !sym->GetIsBlockVar());
            }

            if (sym->GetSymbolType() == STVariable)
            {
                if (fGlobal)
                {
                    Js::PropertyId propertyId = sym->EnsurePosition(this);
                    // We do need to initialize some globals to avoid JS errors on loading undefined variables.
                    // But we first need to make sure we're not trashing built-ins.

                    if (this->flags & fscrEval)
                    {
                        if (funcInfo->byteCodeFunction->GetIsStrictMode())
                        {
                            // Check/Init the property of the frame object
                            this->m_writer.ElementU(Js::OpCode::LdElemUndef, funcInfo->frameObjRegister,
                                funcInfo->FindOrAddReferencedPropertyId(propertyId));
                        }
                        else
                        {
                            // The check and the init involve the first element in the scope chain.
                            this->m_writer.ElementU(
                                Js::OpCode::LdElemUndefScoped, funcInfo->GetEnvRegister(),
                                funcInfo->FindOrAddReferencedPropertyId(propertyId));
                        }
                    }
                    else
                    {
                        this->m_writer.ElementU(Js::OpCode::LdElemUndef, ByteCodeGenerator::RootObjectRegister,
                            funcInfo->FindOrAddReferencedPropertyId(propertyId));
                    }
                }
                else if (!sym->GetIsArguments())
                {
                    if (sym->NeedsSlotAlloc(funcInfo))
                    {
                        if (!sym->GetHasNonCommittedReference() ||
                            (sym->GetHasFuncAssignment() && funcInfo->GetParsedFunctionBody()->DoStackNestedFunc()))
                        {
                            // No point in trying to optimize if there are no references before we have to
                            // commit to slot.
                            // And not safe to delay putting a stack function in the slot, since we may miss boxing.
                            sym->SetIsCommittedToSlot();
                        }
                    }

                    if ((!sym->GetHasInit() && !sym->IsInSlot(funcInfo)) ||
                        (funcInfo->bodyScope->GetIsObject() && !funcInfo->GetHasCachedScope()))
                    {
                        Js::RegSlot reg = sym->GetLocation();
                        if (reg == Js::Constants::NoRegister)
                        {
                            Assert(sym->IsInSlot(funcInfo));
                            reg = funcInfo->AcquireTmpRegister();
                        }
                        this->m_writer.Reg1(Js::OpCode::LdUndef, reg);
                        this->EmitLocalPropInit(reg, sym, funcInfo);

                        if (ShouldTrackDebuggerMetadata() && !sym->GetHasInit() && !sym->IsInSlot(funcInfo))
                        {
                            byteCodeFunction->InsertSymbolToRegSlotList(sym->GetName(), reg, funcInfo->varRegsCount);
                        }

                        funcInfo->ReleaseTmpRegister(reg);
                    }
                }
                else if (ShouldTrackDebuggerMetadata())
                {
                    if (!sym->GetHasInit() && !sym->IsInSlot(funcInfo))
                    {
                        Js::RegSlot reg = sym->GetLocation();
                        if (reg != Js::Constants::NoRegister)
                        {
                            byteCodeFunction->InsertSymbolToRegSlotList(sym->GetName(), reg, funcInfo->varRegsCount);
                        }
                    }
                }
                sym->SetHasInit(TRUE);
            }
        }

    }
    if (tmpReg != Js::Constants::NoRegister)
    {
        funcInfo->ReleaseTmpRegister(tmpReg);
    }

    for(int i = 0; i < funcInfo->nonUserNonTempRegistersToInitialize.Count(); ++i)
    {
        m_writer.Reg1(Js::OpCode::LdUndef, funcInfo->nonUserNonTempRegistersToInitialize.Item(i));
    }

    this->InitBlockScopedNonTemps(funcInfo->root->sxFnc.pnodeScopes, funcInfo);
}

void ByteCodeGenerator::InitBlockScopedNonTemps(ParseNode *pnode, FuncInfo *funcInfo)
{
    // Initialize all non-temp register variables on entry to the enclosing func - in particular,
    // those with lifetimes that begin after the start of user code and may not be initialized normally.
    // This protects us from, for instance, trying to restore garbage on bailout.
    // It was originally done in debugger mode only, but we do it always to avoid issues with boxing
    // garbage on exit from jitted loop bodies.
    while (pnode)
    {
        switch (pnode->nop)
        {
            case knopFncDecl:
            {
                if (this->scriptContext->GetConfig()->IsBlockScopeEnabled())
                {
                    // If this is a block-scoped function, initialize it.
                    ParseNode *pnodeName = pnode->sxFnc.pnodeNames;
                    if (!pnode->sxFnc.IsMethod() && pnodeName && pnodeName->nop == knopVarDecl)
                    {
                        Symbol *sym = pnodeName->sxVar.sym;
                        Assert(sym);
                        if (sym->GetLocation() != Js::Constants::NoRegister &&
                            sym->GetScope()->IsBlockScope(funcInfo) &&
                            sym->GetScope()->GetFunc() == funcInfo)
                        {
                            this->m_writer.Reg1(Js::OpCode::LdUndef, sym->GetLocation());
                        }
                    }
                }
                // No need to recurse to the nested scopes, as they belong to a nested function.
                pnode = pnode->sxFnc.pnodeNext;
                break;
            }
            case knopBlock:
            {
                Scope *scope = pnode->sxBlock.scope;
                if (scope)
                {
                    if (scope->IsBlockScope(funcInfo))
                    {
                        Js::RegSlot scopeLoc = scope->GetLocation();
                        if (scopeLoc != Js::Constants::NoRegister && !funcInfo->IsTmpReg(scopeLoc))
                        {
                            this->m_writer.Reg1(Js::OpCode::LdUndef, scopeLoc);
                        }
                    }
                    auto fnInit = [this, funcInfo](ParseNode *pnode)
                        {
                            Symbol *sym = pnode->sxVar.sym;
                            if (!sym->IsInSlot(funcInfo) && !sym->GetIsGlobal())
                            {
                                this->m_writer.Reg1(Js::OpCode::InitUndecl, pnode->sxVar.sym->GetLocation());
                            }
                        };
                    IterateBlockScopedVariables(pnode, fnInit);
                }
                InitBlockScopedNonTemps(pnode->sxBlock.pnodeScopes, funcInfo);
                pnode = pnode->sxBlock.pnodeNext;
                break;
            }
            case knopCatch:
                InitBlockScopedNonTemps(pnode->sxCatch.pnodeScopes, funcInfo);
                pnode = pnode->sxCatch.pnodeNext;
                break;
            case knopWith:
            {
                Js::RegSlot withLoc = pnode->location;
                AssertMsg(withLoc != Js::Constants::NoRegister && !funcInfo->IsTmpReg(withLoc),
                          "We should put with objects at known stack locations in debug mode");
                this->m_writer.Reg1(Js::OpCode::LdUndef, withLoc);
                InitBlockScopedNonTemps(pnode->sxWith.pnodeScopes, funcInfo);
                pnode = pnode->sxWith.pnodeNext;
                break;
            }
            default:
                Assert(false);
                return;
        }
    }
}

void ByteCodeGenerator::EmitScopeObjectInit(FuncInfo *funcInfo)
{
    Assert(!funcInfo->byteCodeFunction->GetFunctionBody()->DoStackNestedFunc());

    if (!funcInfo->GetHasCachedScope() /* || forcing scope/inner func caching */)
    {
        m_writer.Reg1(Js::OpCode::NewScopeObject, funcInfo->frameObjRegister);
        return;
    }

    uint slotCount = funcInfo->bodyScope->GetScopeSlotCount();
    uint cachedFuncCount = 0;
    Js::PropertyId firstFuncSlot = Js::Constants::NoProperty;
    Js::PropertyId firstVarSlot = Js::Constants::NoProperty;
    uint extraAlloc = (slotCount + 3) * sizeof(Js::PropertyId);

    // Create and fill the array of local property ID's.
    // They all have slots assigned to them already (if they need them): see StartEmitFunction.

    Js::PropertyIdArray *propIds = AnewPlus(alloc, extraAlloc, Js::PropertyIdArray, slotCount);

    ParseNode *pnodeFnc = funcInfo->root;
    ParseNode *pnode;
    Symbol *sym;

    if (funcInfo->GetFuncExprNameReference() && pnodeFnc->sxFnc.GetFuncSymbol()->GetScope() == funcInfo->GetBodyScope())
    {
        Symbol::SaveToPropIdArray(pnodeFnc->sxFnc.GetFuncSymbol(), propIds, this);
    }

    if (funcInfo->GetHasArguments())
    {
        // Because the arguments object can access all instances of same-named formals ("function(x,x){...}"),
        // be sure we initialize any duplicate appearances of a formal parameter to "NoProperty".
        Js::PropertyId slot = 0;
        auto initArg = [&](ParseNode *pnode)
        {
            Symbol *sym = pnode->sxVar.sym;
            Assert(sym);
            if (sym->GetScopeSlot() == slot)
            {
                // This is the last appearance of the formal, so record the ID.
                Symbol::SaveToPropIdArray(sym, propIds, this);
            }
            else
            {
                // This is an earlier duplicate appearance of the formal, so use NoProperty as a placeholder
                // since this slot can't be accessed by name.
                Assert(sym->GetScopeSlot() != Js::Constants::NoProperty && sym->GetScopeSlot() > slot);
                propIds->elements[slot] = Js::Constants::NoProperty;
            }
            slot++;
        };
        MapFormalsWithoutRest(pnodeFnc, initArg);

        // initArg assumes the sym is in a slot, but this may not be true for the rest parameter.
        if (pnodeFnc->sxFnc.pnodeRest != nullptr && pnodeFnc->sxFnc.pnodeRest->sxVar.sym->IsInSlot(funcInfo))
        {
            initArg(pnodeFnc->sxFnc.pnodeRest);
        }
    }
    else
    {
        MapFormals(pnodeFnc, [&](ParseNode *pnode) { Symbol::SaveToPropIdArray(pnode->sxVar.sym, propIds, this); });
    }

    auto saveFunctionVarsToPropIdArray = [&](ParseNode *pnodeFunction)
    {
        if (pnodeFunction->sxFnc.IsDeclaration())
        {
            ParseNode *pnodeName = pnodeFunction->sxFnc.pnodeNames;
            if (pnodeName != nullptr)
            {
                while (pnodeName->nop == knopList)
                {
                    if (pnodeName->sxBin.pnode1->nop == knopVarDecl)
                    {
                        sym = pnodeName->sxBin.pnode1->sxVar.sym;
                        if (sym)
                        {
                            Symbol::SaveToPropIdArray(sym, propIds, this, &firstFuncSlot);
                        }
                    }
                    pnodeName = pnodeName->sxBin.pnode2;
                }
                if (pnodeName->nop == knopVarDecl)
                {
                    sym = pnodeName->sxVar.sym;
                    if (sym)
                    {
                        Symbol::SaveToPropIdArray(sym, propIds, this, &firstFuncSlot);
                        cachedFuncCount++;
                    }
                }
            }
        }
    };
    MapContainerScopeFunctions(pnodeFnc, saveFunctionVarsToPropIdArray);

    for (pnode = pnodeFnc->sxFnc.pnodeVars; pnode; pnode = pnode->sxVar.pnodeNext)
    {
        sym = pnode->sxVar.sym;
        if (!(pnode->sxVar.isBlockScopeFncDeclVar && sym->GetIsBlockVar()))
        {
            if (sym->GetIsCatch() || (pnode->nop == knopVarDecl && sym->GetIsBlockVar()))
            {
                sym = funcInfo->bodyScope->FindLocalSymbol(sym->GetName());
            }
            Symbol::SaveToPropIdArray(sym, propIds, this, &firstVarSlot);
        }
    }

    ParseNode *pnodeBlock = pnodeFnc->sxFnc.pnodeScopes;
    for (pnode = pnodeBlock->sxBlock.pnodeLexVars; pnode; pnode = pnode->sxVar.pnodeNext)
    {
        sym = pnode->sxVar.sym;
        Symbol::SaveToPropIdArray(sym, propIds, this, &firstVarSlot);
    }

    pnodeBlock = pnodeFnc->sxFnc.pnodeBodyScope;
    for (pnode = pnodeBlock->sxBlock.pnodeLexVars; pnode; pnode = pnode->sxVar.pnodeNext)
    {
        sym = pnode->sxVar.sym;
        Symbol::SaveToPropIdArray(sym, propIds, this, &firstVarSlot);
    }

    // If this is an outer function, then we'll also need to emit the first func slot and first var slot
    // in the auxiliary data.

    // Load the function object from the stack. It has the cache for the scope.
    // Put it in the register where the CommitScope will look for it.
    this->m_writer.Reg1(Js::OpCode::LdFuncExpr, funcInfo->funcObjRegister);

    // Write the first func slot and first var slot into the auxiliary data
    Js::PropertyId *slots = propIds->elements + slotCount;
    slots[0] = cachedFuncCount;
    slots[1] = firstFuncSlot;
    slots[2] = firstVarSlot;

    uint32 literalObjectId = funcInfo->GetParsedFunctionBody()->NewObjectLiteral();

    // Pass both the function object and the auxiliary data in the byte code
    funcInfo->localPropIdOffset =
        m_writer.Reg2Aux(
            funcInfo->root->sxFnc.IsSimpleParameterList() ? Js::OpCode::InitCachedScope : Js::OpCode::InitLetCachedScope,
            funcInfo->frameObjRegister,
            funcInfo->funcObjRegister,
            propIds,
            sizeof(Js::PropertyIdArray) + extraAlloc,
            literalObjectId);

    AdeletePlus(alloc, extraAlloc, propIds);
}

void ByteCodeGenerator::FinalizeRegisters(FuncInfo * funcInfo, Js::FunctionBody * byteCodeFunction)
{
    if (funcInfo->NeedEnvRegister())
    {
        bool constReg = !funcInfo->GetIsEventHandler() && funcInfo->IsGlobalFunction() && !(this->flags & fscrEval);
        funcInfo->AssignEnvRegister(constReg);
    }

    // Set the function body's constant count before emitting anything so that the byte code writer
    // can distinguish constants from variables.
    byteCodeFunction->SetConstantCount(funcInfo->constRegsCount);

    if (byteCodeFunction->DoStackFrameDisplay() && funcInfo->frameDisplayRegister != Js::Constants::NoRegister)
    {
        funcInfo->AssignStackClosureRegs();
    }

    if (byteCodeFunction->IsGenerator())
    {
        funcInfo->AssignYieldRegister();
    }

    Js::RegSlot firstTmpReg = funcInfo->varRegsCount;
    funcInfo->SetFirstTmpReg(firstTmpReg);
    byteCodeFunction->SetFirstTmpReg(funcInfo->RegCount());
}

void ByteCodeGenerator::InitScopeSlotArray(FuncInfo * funcInfo)
{
    // Record slots info for ScopeSlots/ScopeObject.
    uint scopeSlotCount = funcInfo->bodyScope->GetScopeSlotCount();
    if (scopeSlotCount == 0)
    {
        return;
    }

    Js::FunctionBody *byteCodeFunction = funcInfo->GetParsedFunctionBody();
    Js::PropertyId *propertyIdsForScopeSlotArray = RecyclerNewArrayLeafZ(scriptContext->GetRecycler(), Js::PropertyId, scopeSlotCount);
    AssertMsg(!byteCodeFunction->IsReparsed() || byteCodeFunction->m_wasEverAsmjsMode || byteCodeFunction->scopeSlotArraySize == scopeSlotCount,
              "The slot array size is different betweeen debug and non-debug mode");
    byteCodeFunction->SetPropertyIdsForScopeSlotArray(propertyIdsForScopeSlotArray, scopeSlotCount);
#if DEBUG
    for (UINT i = 0; i < scopeSlotCount; i++)
    {
        propertyIdsForScopeSlotArray[i] = Js::Constants::NoProperty;
    }
#endif

    auto setPropIdsForScopeSlotArray = [funcInfo, propertyIdsForScopeSlotArray](Symbol *const sym)
    {
        if (sym->NeedsSlotAlloc(funcInfo))
        {
            // All properties should get correct propertyId here.
            Assert(sym->HasScopeSlot()); // We can't allocate scope slot now. Any symbol needing scope slot must have allocated it before this point.
            propertyIdsForScopeSlotArray[sym->GetScopeSlot()] = sym->EnsurePosition(funcInfo);
        }
    };
    if (funcInfo->GetParamScope() != nullptr)
    {
        funcInfo->GetParamScope()->ForEachSymbol(setPropIdsForScopeSlotArray);
    }
    funcInfo->GetBodyScope()->ForEachSymbol(setPropIdsForScopeSlotArray);

    if (funcInfo->thisScopeSlot != Js::Constants::NoRegister)
    {
        propertyIdsForScopeSlotArray[funcInfo->thisScopeSlot] = Js::PropertyIds::_lexicalThisSlotSymbol;
    }

    if (funcInfo->superScopeSlot != Js::Constants::NoRegister)
    {
        propertyIdsForScopeSlotArray[funcInfo->superScopeSlot] = Js::PropertyIds::_superReferenceSymbol;
    }

#if DEBUG
    for (UINT i = 0; i < scopeSlotCount; i++)
    {
        Assert(propertyIdsForScopeSlotArray[i] != Js::Constants::NoProperty
            || funcInfo->frameObjRegister != Js::Constants::NoRegister); // ScopeObject may have unassigned entries, e.g. for same-named parameters
    }
#endif
}

// temporarily load all constants and special registers in a single block
void ByteCodeGenerator::LoadAllConstants(FuncInfo *funcInfo)
{
    Symbol *sym;

    Js::FunctionBody *byteCodeFunction = funcInfo->GetParsedFunctionBody();
    byteCodeFunction->CreateConstantTable();

    if (funcInfo->nullConstantRegister != Js::Constants::NoRegister)
    {
        byteCodeFunction->RecordNullObject(byteCodeFunction->MapRegSlot(funcInfo->nullConstantRegister));
    }

    if (funcInfo->undefinedConstantRegister != Js::Constants::NoRegister)
    {
        byteCodeFunction->RecordUndefinedObject(byteCodeFunction->MapRegSlot(funcInfo->undefinedConstantRegister));
    }

    if (funcInfo->trueConstantRegister != Js::Constants::NoRegister)
    {
        byteCodeFunction->RecordTrueObject(byteCodeFunction->MapRegSlot(funcInfo->trueConstantRegister));
    }

    if (funcInfo->falseConstantRegister != Js::Constants::NoRegister)
    {
        byteCodeFunction->RecordFalseObject(byteCodeFunction->MapRegSlot(funcInfo->falseConstantRegister));
    }

    if (funcInfo->frameObjRegister != Js::Constants::NoRegister)
    {
        m_writer.RecordObjectRegister(funcInfo->frameObjRegister);
        if (!funcInfo->GetApplyEnclosesArgs())
        {
            this->EmitScopeObjectInit(funcInfo);
        }
#if DBG
        uint count = 0;
        funcInfo->GetBodyScope()->ForEachSymbol([&](Symbol *const sym)
        {
            if (sym->IsInSlot(funcInfo))
            {
                // All properties should get correct propertyId here.
                count++;
            }
        });

        if (funcInfo->GetParamScope() != nullptr)
        {
            funcInfo->GetParamScope()->ForEachSymbol([&](Symbol *const sym)
            {
                if (sym->IsInSlot(funcInfo))
                {
                    // All properties should get correct propertyId here.
                    count++;
                }
            });
        }

        // A reparse should result in the same size of the activation object.
        // Exclude functions which were created from the bytecodecache.
        AssertMsg(!byteCodeFunction->IsReparsed() || byteCodeFunction->HasGeneratedFromByteCodeCache() || byteCodeFunction->scopeObjectSize == count, "The activation object size is different betweeen debug and non-debug mode");
        byteCodeFunction->scopeObjectSize = count;

#endif

    }
    else if (funcInfo->frameSlotsRegister != Js::Constants::NoRegister)
    {
        int scopeSlotCount = funcInfo->bodyScope->GetScopeSlotCount();
        if (scopeSlotCount != 0)
        {
            if (byteCodeFunction->DoStackScopeSlots())
            {
                m_writer.Reg1(Js::OpCode::NewStackScopeSlots, funcInfo->frameSlotsRegister);
            }
            else
            {
                m_writer.Reg1Unsigned1(Js::OpCode::NewScopeSlots, funcInfo->frameSlotsRegister, scopeSlotCount + Js::ScopeSlots::FirstSlotIndex);
            }
        }
        else
        {
            AssertMsg(funcInfo->frameDisplayRegister != Js::Constants::NoRegister, "Why do we need scope slots?");
            m_writer.Reg1(Js::OpCode::LdC_A_Null, funcInfo->frameSlotsRegister);
        }
    }

    if (funcInfo->funcExprScope && funcInfo->funcExprScope->GetIsObject())
    {
        m_writer.Reg1(Js::OpCode::NewPseudoScope, funcInfo->funcExprScope->GetLocation());
    }

    if (funcInfo->GetIsEventHandler())
    {
        bool thisLoadedFromParams = false;

        // The environment is the namespace hierarchy starting with "this".
        if (funcInfo->NeedEnvRegister())
        {
            Assert(!funcInfo->RegIsConst(funcInfo->GetEnvRegister()));
            thisLoadedFromParams = true;

            // load "this" from parameters
            this->m_writer.ArgIn0(funcInfo->thisPointerRegister);
            this->m_writer.Reg2(Js::OpCode::LdHandlerScope, funcInfo->GetEnvRegister(), funcInfo->thisPointerRegister);
            this->InvalidateCachedOuterScopes(funcInfo);
        }

        // Top-level HTML event handler.
        // "this" is required to load the environment
        if (funcInfo->thisPointerRegister!=Js::Constants::NoRegister)
        {
            this->LoadThisObject(funcInfo, thisLoadedFromParams);
        }
    }
    else if (funcInfo->IsGlobalFunction() && !(this->flags & fscrEval))
    {
        if (funcInfo->NeedEnvRegister())
        {
            Assert(funcInfo->RegIsConst(funcInfo->GetEnvRegister()));

            if (funcInfo->GetIsStrictMode())
            {
                byteCodeFunction->RecordStrictNullDisplayConstant(byteCodeFunction->MapRegSlot(funcInfo->GetEnvRegister()));
            }
            else
            {
                byteCodeFunction->RecordNullDisplayConstant(byteCodeFunction->MapRegSlot(funcInfo->GetEnvRegister()));
            }
        }
        if (funcInfo->thisPointerRegister!=Js::Constants::NoRegister)
        {
            this->LoadThisObject(funcInfo);
        }
    }
    else
    {
        // environment may be required to load "this"
        if (funcInfo->NeedEnvRegister())
        {
            Assert(!funcInfo->RegIsConst(funcInfo->GetEnvRegister()));
            m_writer.Reg1(Js::OpCode::LdEnv, funcInfo->GetEnvRegister());
            this->InvalidateCachedOuterScopes(funcInfo);
        }
        if (funcInfo->thisPointerRegister!=Js::Constants::NoRegister)
        {
            this->LoadThisObject(funcInfo);
        }
    }

    if (funcInfo->frameDisplayRegister != Js::Constants::NoRegister)
    {
        m_writer.RecordFrameDisplayRegister(funcInfo->frameDisplayRegister);
        this->LoadFrameDisplay(funcInfo);
    }


    this->RecordAllIntConstants(funcInfo);
    this->RecordAllStrConstants(funcInfo);
    this->RecordAllStringTemplateCallsiteConstants(funcInfo);

    funcInfo->doubleConstantToRegister.Map([byteCodeFunction](double d, Js::RegSlot location)
    {
        byteCodeFunction->RecordFloatConstant(byteCodeFunction->MapRegSlot(location), d);
    });

    if (funcInfo->GetHasArguments())
    {
        sym = funcInfo->GetArgumentsSymbol();
        Assert(sym);
        Assert(funcInfo->GetHasHeapArguments());

        if (funcInfo->GetCallsEval()||(!funcInfo->GetApplyEnclosesArgs()))
        {
            this->LoadHeapArguments(funcInfo);
        }

    }
    else if (!funcInfo->IsGlobalFunction() && !IsInNonDebugMode())
    {
        uint count = funcInfo->inArgsCount + (funcInfo->root->sxFnc.pnodeRest != nullptr ? 1 : 0)- 1;
        if (count != 0)
        {
            Js::PropertyIdArray *propIds = RecyclerNewPlus(scriptContext->GetRecycler(), count * sizeof(Js::PropertyId), Js::PropertyIdArray, count);

            GetFormalArgsArray(this, funcInfo, propIds);
            byteCodeFunction->SetPropertyIdsOfFormals(propIds);
        }
    }


    //
    // If the function is a function expression with a name, in ES5 mode
    // In that case, load the function object at runtime to it's activation object
    //
    sym = funcInfo->root->sxFnc.GetFuncSymbol();
    bool funcExprWithName = !funcInfo->IsGlobalFunction() && sym && sym->GetFuncExpr();

    if ( funcExprWithName )
    {
        if (funcInfo->GetFuncExprNameReference() ||
            (funcInfo->funcExprScope && funcInfo->funcExprScope->GetIsObject()))
        {
            //
            // x = function f(...) { ... }
            // A named function expression's name (Symbol:f) belongs to the enclosing scope much like a function
            // declaration in IE8 compat mode. Thus there are no uses of 'f' within the scope of the
            // function (as references to 'f' are looked up in the closure). So, we can't use f's
            // register as it is from the enclosing scope's register namespace. So use a tmp register.
            // In ES5 mode though 'f' is *not* a part of the enclosing scope. So we always assign 'f' a register
            // from it's register namespace, which LdFuncExpr can use.
            //
            Js::RegSlot ldFuncExprDst = sym->GetLocation();
            this->m_writer.Reg1(Js::OpCode::LdFuncExpr, ldFuncExprDst);

            if (sym->IsInSlot(funcInfo))
            {
                Assert(!this->TopFuncInfo()->GetParsedFunctionBody()->DoStackNestedFunc());
                Js::RegSlot scopeLocation;
                if (funcInfo->funcExprScope->GetIsObject())
                {
                    scopeLocation = funcInfo->funcExprScope->GetLocation();
                }
                else
                {
                    scopeLocation = funcInfo->frameObjRegister;
                }
                this->m_writer.Property(Js::OpCode::StFuncExpr, sym->GetLocation(), scopeLocation,
                    funcInfo->FindOrAddReferencedPropertyId(sym->GetPosition()));
            }
            else if (ShouldTrackDebuggerMetadata())
            {
                funcInfo->byteCodeFunction->GetFunctionBody()->InsertSymbolToRegSlotList(sym->GetName(), sym->GetLocation(), funcInfo->varRegsCount);
            }
        }
    }
}

void ByteCodeGenerator::InvalidateCachedOuterScopes(FuncInfo *funcInfo)
{
    Assert(funcInfo->GetEnvRegister() != Js::Constants::NoRegister);

    // Walk the scope stack, from funcInfo outward, looking for scopes that have been cached.

    Scope *scope = funcInfo->GetBodyScope()->GetEnclosingScope();
    uint32 envIndex = 0;

    while (scope && scope->GetFunc() == funcInfo)
    {
        // Skip over FuncExpr Scope and parameter scope for current funcInfo to get to the first enclosing scope of the outer function
        scope = scope->GetEnclosingScope();
    }

    for (;
         scope;
         scope = scope->GetEnclosingScope())
    {
        FuncInfo *func = scope->GetFunc();
        if (scope == func->GetBodyScope())
        {
            if (func->Escapes() && func->GetHasCachedScope())
            {
                Assert(scope->GetIsObject());
                this->m_writer.Reg1Unsigned1(Js::OpCode::InvalCachedScope, funcInfo->GetEnvRegister(), envIndex);
            }
        }
        if (scope->GetMustInstantiate())
        {
            envIndex++;
        }
    }
}

void ByteCodeGenerator::LoadThisObject(FuncInfo *funcInfo, bool thisLoadedFromParams)
{
    if (!funcInfo->IsGlobalFunction() || (this->flags & fscrEval))
    {
        //
        // thisLoadedFromParams would be true for the event Handler case,
        // "this" would have been loaded from parameters to put in the environment
        //
        if (!thisLoadedFromParams && !funcInfo->IsLambda())
        {
            m_writer.ArgIn0(funcInfo->thisPointerRegister);
        }
        if (!(this->flags & fscrEval) || !funcInfo->IsGlobalFunction())
        {
            // we don't want to emit 'this' for eval, because 'this' value in eval is equal to 'this' value of caller
            // and does not depend on "use strict" inside of eval.
            // so we pass 'this' dirrectly in GlobalObject::EntryEval()
            EmitThis(funcInfo, funcInfo->thisPointerRegister);
        }
    }
    else
    {
        Assert(funcInfo->IsGlobalFunction());
        Js::RegSlot root = funcInfo->nullConstantRegister;
        EmitThis(funcInfo, root);
    }
}

void ByteCodeGenerator::EmitInternalScopedSlotLoad(FuncInfo *funcInfo, Scope *scope, Js::PropertyId envIndex, Js::RegSlot slot, Js::RegSlot symbolRegister)
{
    Assert(slot != Js::Constants::NoProperty);
    Js::RegSlot scopeLocation = funcInfo->AcquireTmpRegister();
    this->m_writer.Slot(Js::OpCode::LdSlotArr, scopeLocation, funcInfo->GetEnvRegister(), envIndex + Js::FrameDisplay::GetOffsetOfScopes()/sizeof(Js::Var));
    funcInfo->ReleaseTmpRegister(scopeLocation);
    Js::ProfileId profileId = funcInfo->FindOrAddSlotProfileId(scope, symbolRegister);
    Js::OpCode opcode;

    if (scope->GetIsObject())
    {
        opcode = (scope->IsBlockScope(funcInfo)) ? Js::OpCode::LdObjSlot : Js::OpCode::LdSlot;
    }
    else
    {
        opcode = Js::OpCode::LdSlot;
        slot += Js::ScopeSlots::FirstSlotIndex;
    }

    this->m_writer.Slot(opcode, symbolRegister, scopeLocation, slot, profileId);
}

void ByteCodeGenerator::GetEnclosingNonLambdaScope(FuncInfo *funcInfo, Scope * &scope, Js::PropertyId &envIndex)
{
    envIndex = -1;
    for (scope = funcInfo->GetBodyScope()->GetEnclosingScope(); scope; scope = scope->GetEnclosingScope())
    {
        if (scope->GetMustInstantiate())
        {
            envIndex++;
        }
        if ((scope == scope->GetFunc()->GetBodyScope() && !scope->GetFunc()->IsLambda()) || scope->IsGlobalEvalBlockScope())
        {
            break;
        }
    }
}

void ByteCodeGenerator::EmitThis(FuncInfo *funcInfo, Js::RegSlot fromRegister)
{
    if (funcInfo->IsLambda())
    {
        Scope *scope;
        Js::PropertyId envIndex = -1;
        GetEnclosingNonLambdaScope(funcInfo, scope, envIndex);
        if (scope->GetFunc()->IsGlobalFunction())
        {
            if (this->flags & fscrEval)
            {
                scope = scope->GetFunc()->GetGlobalEvalBlockScope();
                Js::PropertyId slot = scope->GetFunc()->thisScopeSlot;
                EmitInternalScopedSlotLoad(funcInfo, scope, envIndex, slot, funcInfo->thisPointerRegister);
            }
            else
            {
                // Always load global object via LdThis of null to get the possibly protected via secureHostObject global object
                this->m_writer.Reg2Int1(Js::OpCode::LdThis, funcInfo->thisPointerRegister, funcInfo->nullConstantRegister, this->GetModuleID());
            }
        }
        else
        {
            Js::PropertyId slot = scope->GetFunc()->thisScopeSlot;
            EmitInternalScopedSlotLoad(funcInfo, scope, envIndex, slot, funcInfo->thisPointerRegister);
        }
    }
    else if (funcInfo->byteCodeFunction->GetIsStrictMode() && (!funcInfo->IsGlobalFunction() || this->flags & fscrEval))
    {
        m_writer.Reg2(Js::OpCode::StrictLdThis, funcInfo->thisPointerRegister, fromRegister);
    }
    else
    {
        m_writer.Reg2Int1(Js::OpCode::LdThis, funcInfo->thisPointerRegister, fromRegister, this->GetModuleID());
    }
}

void ByteCodeGenerator::LoadFrameDisplay(FuncInfo *funcInfo)
{
    // Build a frame display for the current function.

    Assert(funcInfo->frameDisplayRegister != Js::Constants::NoRegister);

    Js::RegSlot regEnv = funcInfo->GetEnvRegister();
    Assert(!funcInfo->IsGlobalFunction() || (funcInfo->byteCodeFunction->GetIsStrictMode() && (this->flags & fscrEval)));

    // In a normal function, the display consists of the local frame followed
    // by any outer frames passed in as a closure environment.
    Scope *scope = funcInfo->bodyScope;
    Js::RegSlot regFrame = scope->GetIsObject() ? funcInfo->frameObjRegister : funcInfo->frameSlotsRegister;

    if (funcInfo->funcExprScope && funcInfo->funcExprScope->GetIsObject())
    {
        //
        // A new activation object is created for the function expression
        // That also should be part of the scope chain
        // FrameDisplay = [ FrameObject, FuncExpression, Environment ]
        //
        // FrameDisplay = [ FuncExpression, Environment ]
        this->m_writer.Reg3(Js::OpCode::LdFrameDisplay, funcInfo->frameDisplayRegister, funcInfo->funcExprScope->GetLocation(), regEnv);

        // Environment = FrameDisplay
        regEnv = funcInfo->frameDisplayRegister;

        // TODO: Handle this case in frame display fast path
        funcInfo->GetParsedFunctionBody()->SetEnvDepth((uint16)-1);
    }

    Assert(regFrame != Js::Constants::NoRegister);
    Assert(regEnv != Js::Constants::NoRegister);

    // FrameDisplay = [ FrameObject, Environment ]
    Js::OpCode op;
    Js::FunctionBody *functionBody = funcInfo->GetParsedFunctionBody();
    if (funcInfo->GetParsedFunctionBody()->GetEnvDepth() == 0)
    {
        op = functionBody->DoStackFrameDisplay() ? Js::OpCode::NewStackFrameDisplayNoParent : Js::OpCode::LdFrameDisplayNoParent;
        this->m_writer.Reg2(op, funcInfo->frameDisplayRegister, regFrame);
    }
    else
    {
        op = functionBody->DoStackFrameDisplay() ? Js::OpCode::NewStackFrameDisplay : Js::OpCode::LdFrameDisplay;
        this->m_writer.Reg3(op, funcInfo->frameDisplayRegister, regFrame, regEnv);
    }
}

void ByteCodeGenerator::EmitLoadFormalIntoRegister(ParseNode *pnodeFormal, Js::ArgSlot pos, FuncInfo *funcInfo)
{
    // Get the param from its argument position into its assigned register.
    // The position should match the location, otherwise, it has been shadowed by parameter with the same name
    Symbol *formal = pnodeFormal->sxVar.sym;
    if (formal->GetLocation() + 1 == pos)
    {
            // Transfer to the frame object, etc., if necessary.
            this->EmitLocalPropInit(formal->GetLocation(), formal, funcInfo);
    }

    if (ShouldTrackDebuggerMetadata() && !formal->IsInSlot(funcInfo))
    {
        Assert(!formal->GetHasInit());
        funcInfo->GetParsedFunctionBody()->InsertSymbolToRegSlotList(formal->GetName(), formal->GetLocation(), funcInfo->varRegsCount);
    }
}

void ByteCodeGenerator::HomeArguments(FuncInfo *funcInfo) {
    // Transfer formal parameters to their home locations on the local frame.
    if (funcInfo->GetHasArguments())
    {
        if (funcInfo->root->sxFnc.pnodeRest != nullptr)
        {
            // Since we don't have to iterate over arguments here, we'll trust the location to be correct.
            EmitLoadFormalIntoRegister(funcInfo->root->sxFnc.pnodeRest, funcInfo->root->sxFnc.pnodeRest->sxVar.sym->GetLocation() + 1, funcInfo);
        }

        // The arguments object creation helper does this work for us.
        return;
    }

    Js::ArgSlot pos = 1;
    auto loadFormal = [&](ParseNode *pnodeFormal)
    {
        EmitLoadFormalIntoRegister(pnodeFormal, pos, funcInfo);
        pos++;
    };
    MapFormals(funcInfo->root, loadFormal);
}

void ByteCodeGenerator::DefineLabels(FuncInfo *funcInfo) {
    funcInfo->singleExit=m_writer.DefineLabel();
    SList<ParseNode *>::Iterator iter(&funcInfo->targetStatements);
    while (iter.Next())
    {
        ParseNode * node = iter.Data();
        node->sxStmt.breakLabel=m_writer.DefineLabel();
        node->sxStmt.continueLabel=m_writer.DefineLabel();
        node->emitLabels=true;
    }
}

void CheckDeadLoop(ParseNode *pnode,ByteCodeGenerator *byteCodeGenerator,void* ignored)
{
    ParseNode* nameNode;

    if ((pnode==NULL) || (!byteCodeGenerator->GetDeadLoopPossible())) {
        return;
    }
    switch (pnode->nop) {
    default:
        byteCodeGenerator->SetDeadLoopPossible(false);
        break;
    case knopFor:
        if (pnode!=byteCodeGenerator->GetOuterLoop()) {
            byteCodeGenerator->SetDeadLoopPossible(false);
        }
        break;
    case knopInt:
    case knopFlt:
    case knopLt:
    case knopGt:
    case knopLe:
    case knopGe:
    case knopEq:
    case knopNe:
    case knopEqv:
    case knopNEqv:
    case knopList:
    case knopIf:
    case knopAdd:
    case knopMul:
    case knopDiv:
    case knopMod:
    case knopNeg:
    case knopNot:
    case knopPos:
    case knopLogNot:
    case knopEllipsis:
    case knopLsh:
    case knopRsh:
    case knopRs2:
    case knopSub:
    case knopOr:
    case knopXor:
    case knopAnd:
    case knopIndex:
    case knopTrue:
    case knopFalse:
    case knopNull:
    case knopBlock:
        break;

    case knopName: {
        if ((pnode->sxPid.sym==NULL)||(pnode->sxPid.sym->GetDecl()==NULL)) {
            byteCodeGenerator->SetDeadLoopPossible(false);
        }
        break;
                   }
    case knopIncPost:
    case knopIncPre:
    case knopDecPost:
    case knopDecPre:
        nameNode = pnode->sxUni.pnode1;
        goto checkIsNotGlobal;

    case knopAsgAnd:
    case knopAsgXor:
    case knopAsgOr:
    case knopAsgLsh:
    case knopAsgRsh:
    case knopAsgRs2:
    case knopAsgSub:
    case knopAsgAdd:
    case knopAsgMul:
    case knopAsgDiv:
    case knopAsgMod:
    case knopAsg: {

        nameNode = pnode->sxBin.pnode1;

checkIsNotGlobal:
        if ((nameNode->nop!=knopName)||(nameNode->sxPid.sym==NULL)||(nameNode->sxPid.sym->GetIsGlobal())) {
            byteCodeGenerator->SetDeadLoopPossible(false);
        }
        break;
                  }
    case knopVarDecl:
    case knopConstDecl:
    case knopLetDecl:
        break;
    }
}

bool ByteCodeGenerator::IsDeadLoop(ParseNode* pnode,FuncInfo *funcInfo) {
#pragma prefast(suppress:6235, "Non-Zero Constant in Condition")
    if (!PHASE_ON(Js::DeadLoopDetectionPhase, funcInfo->byteCodeFunction))
    {
        return false;
    }
    if (IsInDebugMode()
        || funcInfo->GetHasLocalInClosure()
        || funcInfo->GetChildCallsEval()
        || funcInfo->GetHasClosureReference()
        || funcInfo->GetCallsEval()
        || (scriptContext->optimizationOverrides.GetSideEffects() != Js::SideEffects_None))
    {
        return false;
    }

    ParseNode *stmt = pnode->sxBin.pnode1;
    if (!callsConstructor && stmt->nop == knopFor && pnode->sxBin.pnode2->nop == knopEndCode) {
        deadLoopPossible=true;
        outerLoop=stmt;
        VisitIndirect<void>(stmt,this,NULL,&CheckDeadLoop,NULL);
        return deadLoopPossible;
    }
    return false;
}

void ByteCodeGenerator::EmitGlobalBody(FuncInfo *funcInfo)
{
    // Emit global code (global scope or eval), fixing up the return register with the implicit
    // return value.
    ParseNode *pnode = funcInfo->root->sxFnc.pnodeBody;
    ParseNode *pnodeLastVal = funcInfo->root->sxProg.pnodeLastValStmt;
    if (pnodeLastVal == NULL)
    {
        // We're not guaranteed to compute any values, so fix up the return register at the top
        // in case.
        this->m_writer.Reg1(Js::OpCode::LdUndef, ReturnRegister);
    }

    while(pnode->nop==knopList)
    {
        ParseNode *stmt=pnode->sxBin.pnode1;
        if (stmt == pnodeLastVal)
        {
            pnodeLastVal = NULL;
        }
        if (pnodeLastVal == NULL && (this->flags & fscrReturnExpression))
        {
            EmitTopLevelStatement(stmt, funcInfo, true);
        }
        else
        {
            // Haven't hit the post-dominating return value yet,
            // so don't bother with the return register.
            if (!IsDeadLoop(pnode, funcInfo))
            {
                EmitTopLevelStatement(stmt, funcInfo, false);
            }
            else
            {
                Js::ByteCodeLabel afterStatement = this->m_writer.DefineLabel();
                // Emit branch around.
                this->m_writer.BrS(Js::OpCode::BrNotHasSideEffects, afterStatement, Js::SideEffects_Any);
                EmitTopLevelStatement(stmt, funcInfo, false);
                this->m_writer.MarkLabel(afterStatement);
            }
        }
        pnode = pnode->sxBin.pnode2;
    }
    EmitTopLevelStatement(pnode, funcInfo, false);
}

bool ByteCodeGenerator::CanStackNestedFunc(FuncInfo * funcInfo, bool trace)
{
#if ENABLE_DEBUG_CONFIG_OPTIONS
    wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif
    Assert(!funcInfo->IsGlobalFunction());
    bool const doStackNestedFunc = !funcInfo->HasMaybeEscapedNestedFunc() && !IsInDebugMode() && !(this->flags & fscrIsNativeCode);

    if (!doStackNestedFunc)
    {
        return false;
    }

    bool callsEval = funcInfo->GetCallsEval() || funcInfo->GetChildCallsEval();
    if (callsEval)
    {
        if (trace)
        {
            PHASE_PRINT_TESTTRACE(Js::StackFuncPhase, funcInfo->byteCodeFunction,
                L"HasMaybeEscapedNestedFunc (Eval): %s (function %s)\n",
                funcInfo->byteCodeFunction->GetDisplayName(),
                funcInfo->byteCodeFunction->GetDebugNumberSet(debugStringBuffer));
        }
        return false;
    }

    if (funcInfo->GetBodyScope()->GetIsObject() || funcInfo->GetParamScope()->GetIsObject())
    {
        if (trace)
        {
            PHASE_PRINT_TESTTRACE(Js::StackFuncPhase, funcInfo->byteCodeFunction,
                L"HasMaybeEscapedNestedFunc (ObjectScope): %s (function %s)\n",
                funcInfo->byteCodeFunction->GetDisplayName(),
                funcInfo->byteCodeFunction->GetDebugNumberSet(debugStringBuffer));
        }
        return false;
    }

    if (trace && funcInfo->byteCodeFunction->GetNestedCount())
    {
        // Only print function that actually have nested function, although we will still mark
        // function that doesn't have nested child func as DoStackNestedFunc
        PHASE_PRINT_TESTTRACE(Js::StackFuncPhase, funcInfo->byteCodeFunction,
            L"DoStackNestedFunc: %s (function %s)\n",
            funcInfo->byteCodeFunction->GetDisplayName(),
            funcInfo->byteCodeFunction->GetDebugNumberSet(debugStringBuffer));
    }

    return !PHASE_OFF(Js::StackFuncPhase, funcInfo->byteCodeFunction);
}

void ByteCodeGenerator::EmitFunctionBody(FuncInfo *funcInfo)
{
    // Emit a function body. Only explicit returns and the implicit "undef" at the bottom
    // get copied to the return register.
    ParseNode *pnodeBody = funcInfo->root->sxFnc.pnodeBody;
    ParseNode *pnode = pnodeBody;
    while(pnode->nop==knopList)
    {
        ParseNode *stmt = pnode->sxBin.pnode1;
        if (stmt->CapturesSyms())
        {
            CapturedSymMap *map = funcInfo->EnsureCapturedSymMap();
            SList<Symbol*> *list = map->Item(stmt);
            FOREACH_SLIST_ENTRY(Symbol*, sym, list)
            {
                if (!sym->GetIsCommittedToSlot())
                {
                    Assert(sym->GetLocation() != Js::Constants::NoProperty);
                    sym->SetIsCommittedToSlot();
                    ParseNode *decl = sym->GetDecl();
                    Assert(decl);
                    if(PHASE_TRACE(Js::DelayCapturePhase, funcInfo->byteCodeFunction))
                    {
                        Output::Print(L"--- DelayCapture: Committed symbol '%s' to slot.\n", sym->GetName());
                        Output::Flush();
                    }
                    this->EmitPropStore(sym->GetLocation(), sym, sym->GetPid(), funcInfo, decl->nop == knopLetDecl, decl->nop == knopConstDecl);
                }
            }
            NEXT_SLIST_ENTRY;
        }
        if (!IsDeadLoop(pnode, funcInfo))
        {
            EmitTopLevelStatement(stmt, funcInfo, false);
        }
        else
        {
            Js::ByteCodeLabel afterStatement = this->m_writer.DefineLabel();
            // Emit branch around.
            this->m_writer.BrS(Js::OpCode::BrNotHasSideEffects, afterStatement, Js::SideEffects_Any);
            EmitTopLevelStatement(stmt, funcInfo, false);
            this->m_writer.MarkLabel(afterStatement);
        }
        pnode = pnode->sxBin.pnode2;
    }
    Assert(!pnode->CapturesSyms());
    EmitTopLevelStatement(pnode, funcInfo, false);
}

void ByteCodeGenerator::EmitProgram(ParseNode *pnodeProg)
{
    // Indicate that the binding phase is over.
    this->isBinding = false;
    this->trackEnvDepth = true;
    AssignPropertyIds(pnodeProg->sxFnc.funcInfo->byteCodeFunction);

    long initSize = this->maxAstSize / AstBytecodeRatioEstimate;

    // Use the temp allocator in bytecode write temp buffer.
    m_writer.InitData(this->alloc, initSize);

#ifdef LOG_BYTECODE_AST_RATIO
    // log the max Ast size
    Output::Print(L"Max Ast size: %d", initSize);
#endif

    Assert(pnodeProg && pnodeProg->nop == knopProg);
    if (this->parentScopeInfo)
    {
        // Scope stack is already set up the way we want it, so don't visit the global scope.
        // Start emitting with the nested scope (i.e., the deferred function).
        this->EmitScopeList(pnodeProg->sxProg.pnodeScopes);
    }
    else
    {
        this->EmitScopeList(pnodeProg);
    }
}

void ByteCodeGenerator::EmitInitCapturedThis(FuncInfo* funcInfo, Scope* scope)
{
    if (scope->GetIsObject())
    {
        // Ensure space for the this slot
        uint cacheId = funcInfo->FindOrAddInlineCacheId(scope->GetLocation(), Js::PropertyIds::_lexicalThisSlotSymbol, false, true);
        this->m_writer.PatchableProperty(Js::OpCode::InitFld, funcInfo->thisPointerRegister, scope->GetLocation(), cacheId);
    }
    else
    {
        Js::OpCode op = scope->IsBlockScope(funcInfo) ? Js::OpCode::StObjSlot : Js::OpCode::StSlot;
        this->m_writer.Slot(op, funcInfo->thisPointerRegister, scope->GetLocation(), funcInfo->thisScopeSlot + Js::ScopeSlots::FirstSlotIndex);
    }
};

void ByteCodeGenerator::EmitDefaultArgs(FuncInfo *funcInfo, ParseNode *pnode)
{
    auto emitDefaultArg = [&](ParseNode *pnodeArg)
    {
        Js::RegSlot location = pnodeArg->sxVar.sym->GetLocation();

        if (pnodeArg->sxVar.pnodeInit == nullptr)
        {
            // Since the formal hasn't been initialized in LdLetHeapArguments, we'll initialize it here.
            EmitPropStore(location, pnodeArg->sxVar.sym, pnodeArg->sxVar.pid, funcInfo, true);

            pnodeArg->sxVar.sym->SetNeedDeclaration(false);
            return;
        }

        // Load the default argument if we got undefined, skip RHS evaluation otherwise.
        Js::ByteCodeLabel noDefaultLabel = this->m_writer.DefineLabel();
        Js::ByteCodeLabel endLabel = this->m_writer.DefineLabel();
        this->StartStatement(pnodeArg);
        m_writer.BrReg2(Js::OpCode::BrNeq_A, noDefaultLabel, location, funcInfo->undefinedConstantRegister);

        this->StartStatement(pnodeArg->sxVar.pnodeInit);
        Emit(pnodeArg->sxVar.pnodeInit, this, funcInfo, false);
        pnodeArg->sxVar.sym->SetNeedDeclaration(false); // After emit to prevent foo(a = a)

        if (funcInfo->GetHasArguments() && pnodeArg->sxVar.sym->IsInSlot(funcInfo))
        {
            EmitPropStore(pnodeArg->sxVar.pnodeInit->location, pnodeArg->sxVar.sym, pnodeArg->sxVar.pid, funcInfo, true);

            m_writer.Br(endLabel);
        }
        else
        {
            EmitAssignment(nullptr, pnodeArg, pnodeArg->sxVar.pnodeInit->location, this, funcInfo);
        }

        funcInfo->ReleaseLoc(pnodeArg->sxVar.pnodeInit);
        this->EndStatement(pnodeArg->sxVar.pnodeInit);

        m_writer.MarkLabel(noDefaultLabel);

        if (funcInfo->GetHasArguments() && pnodeArg->sxVar.sym->IsInSlot(funcInfo))
        {
            EmitPropStore(location, pnodeArg->sxVar.sym, pnodeArg->sxVar.pid, funcInfo, true);

            m_writer.MarkLabel(endLabel);
        }

        this->EndStatement(pnodeArg);
    };

    // Rest cannot have a default argument, so we ignore it.
    MapFormalsWithoutRest(pnode, emitDefaultArg);
}

void ByteCodeGenerator::EmitOneFunction(ParseNode *pnode)
{
    Assert(pnode && (pnode->nop == knopProg || pnode->nop == knopFncDecl));
    FuncInfo *funcInfo = pnode->sxFnc.funcInfo;
    Assert(funcInfo != NULL);

    if (funcInfo->IsFakeGlobalFunction(this->flags))
    {
        return;
    }

    Js::ParseableFunctionInfo* deferParseFunction = funcInfo->byteCodeFunction;
    deferParseFunction->SetGrfscr(deferParseFunction->GetGrfscr() | (this->flags & ~fscrDeferredFncExpression));
    deferParseFunction->SetSourceInfo(this->GetCurrentSourceIndex(),
        funcInfo->root,
        !!(this->flags & fscrEvalCode),
        ((this->flags & fscrDynamicCode) && !(this->flags & fscrEvalCode)));

    deferParseFunction->SetInParamsCount(funcInfo->inArgsCount);
    if (pnode->sxFnc.HasDefaultArguments())
    {
        deferParseFunction->SetReportedInParamsCount(pnode->sxFnc.firstDefaultArg + 1);
    }
    else
    {
        deferParseFunction->SetReportedInParamsCount(funcInfo->inArgsCount);
    }

    if (funcInfo->root->sxFnc.pnodeBody == NULL)
    {
        if (scriptContext->GetConfig()->BindDeferredPidRefs() &&
            !PHASE_OFF1(Js::SkipNestedDeferredPhase))
        {
            deferParseFunction->BuildDeferredStubs(funcInfo->root);
        }
        return;
    }

    Js::FunctionBody* byteCodeFunction = funcInfo->GetParsedFunctionBody();
    // We've now done a full parse of this function, so we no longer need to remember the extents
    // and attributes of the top-level nested functions. (The above code has run for all of those,
    // so they have pointers to the stub sub-trees they need.)
    byteCodeFunction->SetDeferredStubs(nullptr);

    try
    {
        // Bug : 301517
        // In the debug mode the hasOnlyThis optimization needs to be disabled, since user can break in this function
        // and do operation on 'this' and its property, which may not be defined yet.
        if (funcInfo->root->sxFnc.HasOnlyThisStmts() && !IsInDebugMode())
        {
            byteCodeFunction->SetHasOnlyThisStmts(true);
        }

        if (byteCodeFunction->IsInlineApplyDisabled() || this->scriptContext->GetConfig()->IsNoNative())
        {
            if ((pnode->nop==knopFncDecl)&&(funcInfo->GetHasHeapArguments())&&(!funcInfo->GetCallsEval())&&ApplyEnclosesArgs(pnode,this))
            {
                funcInfo->SetApplyEnclosesArgs(true);
            }
        }

        if (!funcInfo->IsGlobalFunction())
        {
            if (CanStackNestedFunc(funcInfo, true))
            {
#if DBG
                byteCodeFunction->SetCanDoStackNestedFunc();
#endif
                if (funcInfo->root->sxFnc.astSize <= PnFnc::MaxStackClosureAST)
                {
                    byteCodeFunction->SetStackNestedFunc(true);
                }
            }
        }

        InitScopeSlotArray(funcInfo);
        FinalizeRegisters(funcInfo, byteCodeFunction);
        DebugOnly(Js::RegSlot firstTmpReg = funcInfo->varRegsCount);

        funcInfo->inlineCacheMap = Anew(alloc, FuncInfo::InlineCacheMap,
            alloc,
            funcInfo->RegCount()   // Pass the actual register count. TODO: Check if we can reduce this count
            );
        funcInfo->rootObjectLoadInlineCacheMap = Anew(alloc, FuncInfo::RootObjectInlineCacheIdMap,
            alloc,
            10);
        funcInfo->rootObjectLoadMethodInlineCacheMap = Anew(alloc, FuncInfo::RootObjectInlineCacheIdMap,
            alloc,
            10);
        funcInfo->rootObjectStoreInlineCacheMap = Anew(alloc, FuncInfo::RootObjectInlineCacheIdMap,
            alloc,
            10);
        funcInfo->referencedPropertyIdToMapIndex = Anew(alloc, FuncInfo::RootObjectInlineCacheIdMap,
            alloc,
            10);

        byteCodeFunction->AllocateLiteralRegexArray();
        m_callSiteId = 0;
        m_writer.Begin(this, byteCodeFunction, alloc, this->DoJitLoopBodies(funcInfo), funcInfo->hasLoop);
        this->PushFuncInfo(L"EmitOneFunction", funcInfo);

        this->inPrologue = true;

        // for now, emit all constant loads at top of function (should instead put in
        // closest dominator of uses)
        LoadAllConstants(funcInfo);
        HomeArguments(funcInfo);

        if (funcInfo->root->sxFnc.pnodeRest != nullptr)
        {
            byteCodeFunction->SetHasRestParameter();
        }

        if (funcInfo->thisScopeSlot != Js::Constants::NoRegister && !(funcInfo->IsLambda() || (funcInfo->IsGlobalFunction() && this->flags & fscrEval)))
        {
            EmitInitCapturedThis(funcInfo, funcInfo->bodyScope);
        }

        // TODO: (suwc) To implement Function.prototype.toMethod(), remove pre-emptive LdSuper or remove BadSuperReference throwing from Op_LdSuper
        // Any function with a super reference or an eval call inside a class method needs to load super
        if ((funcInfo->HasSuperReference() || (funcInfo->GetCallsEval() && funcInfo->root->sxFnc.IsClassMember()))
            // unless we are already inside the 'global' scope inside an eval (in which case 'ScopedLdSuper' is emitted at every 'super' reference)
            && !((GetFlags() & fscrEval) && funcInfo->IsGlobalFunction()))
        {
            if (funcInfo->IsLambda())
            {
                Scope *scope;
                Js::PropertyId envIndex = -1;
                GetEnclosingNonLambdaScope(funcInfo, scope, envIndex);

                FuncInfo* parent = scope->GetFunc();

                if (!parent->IsGlobalFunction())
                {
                    // lambda in non-global scope (eval and non-eval)
                    Js::PropertyId slot = parent->superScopeSlot;
                    EmitInternalScopedSlotLoad(funcInfo, scope, envIndex, slot, funcInfo->superRegister);
                }
                else if (!(GetFlags() & fscrEval))
                {
                    // lambda in non-eval global scope
                    m_writer.Reg1(Js::OpCode::LdUndef, funcInfo->superRegister);
                }
                // lambda in eval global scope: ScopedLdSuper will handle error throwing
            }
            else
            {
                m_writer.Reg1(Js::OpCode::LdSuper, funcInfo->superRegister);
                if (!funcInfo->IsGlobalFunction())
                {
                    if (funcInfo->bodyScope->GetIsObject() && funcInfo->bodyScope->GetLocation() != Js::Constants::NoRegister)
                    {
                        // Stash the super reference incase something inside the eval or lambda references it.
                        uint cacheId = funcInfo->FindOrAddInlineCacheId(funcInfo->bodyScope->GetLocation(), Js::PropertyIds::_superReferenceSymbol, false, true);
                        m_writer.PatchableProperty(Js::OpCode::InitFld, funcInfo->superRegister, funcInfo->bodyScope->GetLocation(), cacheId);
                    }
                    else if (funcInfo->superScopeSlot == Js::Constants::NoProperty)
                    {
                        // While the diag locals walker will pick up super from scoped slots or an activation object,
                        // it will not pick it up when it is only in a register.
                        byteCodeFunction->InsertSymbolToRegSlotList(funcInfo->superRegister, Js::PropertyIds::_superReferenceSymbol, funcInfo->varRegsCount);
                    }
                }
            }
        }

        // We don't want to load super if we are already in an eval. ScopedLdSuper will take care of loading super in that case.
        if (funcInfo->superScopeSlot != Js::Constants::NoRegister && !(GetFlags() & fscrEval) && !funcInfo->bodyScope->GetIsObject())
        {
            this->m_writer.Slot(Js::OpCode::StSlot, funcInfo->superRegister, funcInfo->GetBodyScope()->GetLocation(), funcInfo->superScopeSlot + Js::ScopeSlots::FirstSlotIndex);
        }


        if (byteCodeFunction->DoStackNestedFunc())
        {
            uint nestedCount = byteCodeFunction->GetNestedCount();
            for (uint i = 0; i < nestedCount; i++)
            {
                Js::FunctionProxy * nested = byteCodeFunction->GetNestedFunc(i);
                if (nested->IsFunctionBody())
                {
                    nested->GetFunctionBody()->SetStackNestedFuncParent(byteCodeFunction);
                }
            }
        }

        if (scriptContext->GetConfig()->IsLetAndConstEnabled() && funcInfo->IsGlobalFunction())
        {
            EnsureNoRedeclarations(pnode->sxFnc.pnodeScopes, funcInfo);
        }

        // Emit all scope-wide function definitions before emitting function bodies
        // so that calls may reference functions they precede lexically.
        // Note, global eval scope is a fake local scope and is handled as if it were
        // a lexical block instead of a true global scope, so do not define the functions
        // here. The will be defined during BeginEmitBlock.
        bool fFunctionsUndefined = false;
        if (!(funcInfo->IsGlobalFunction() && this->IsEvalWithBlockScopingNoParentScopeInfo()))
        {
            fFunctionsUndefined = DefineFunctions(funcInfo, false);
        }
        DefineUserVars(funcInfo);

        ::BeginEmitBlock(pnode->sxFnc.pnodeScopes, this, funcInfo);

        if (pnode->sxFnc.pnodeBodyScope != nullptr)
        {
            ::BeginEmitBlock(pnode->sxFnc.pnodeBodyScope, this, funcInfo);
        }

        // If the local frame has a scope object, but we can still access its slots directly,
        // then load a pointer to the slots so we can use it in the body of the function.
        // We do this after everything has been initialized on the frame object, including
        // both parameter and function body scopes.
        if (funcInfo->frameObjRegister != Js::Constants::NoRegister &&
            funcInfo->frameSlotsRegister != Js::Constants::NoRegister)
        {
            Assert(!byteCodeFunction->DoStackScopeSlots());
            if (!funcInfo->GetApplyEnclosesArgs()) {
                m_writer.Slot(Js::OpCode::LdSlotArr, funcInfo->frameSlotsRegister, funcInfo->frameObjRegister,
                    Js::DynamicObject::GetOffsetOfAuxSlots() / sizeof(Js::Var));
                // From now on, accesses to this scope go through the slots directly.
                funcInfo->GetBodyScope()->SetLocation(funcInfo->frameSlotsRegister);
            }
        }

        if (!pnode->sxFnc.IsSimpleParameterList())
        {
            EmitDefaultArgs(funcInfo, pnode);
        }
        else if (funcInfo->GetHasArguments() && !NeedScopeObjectForArguments(funcInfo, pnode))
        {
            // If we didn't create a scope object and didn't have default args, we still need to transfer the formals to their slots.
            MapFormalsWithoutRest(pnode, [&](ParseNode *pnodeArg) { EmitPropStore(pnodeArg->sxVar.sym->GetLocation(), pnodeArg->sxVar.sym, pnodeArg->sxVar.pid, funcInfo); });
        }

        // Rest needs to trigger use before declaration until all default args have been processed.
        if (pnode->sxFnc.pnodeRest != nullptr)
        {
            pnode->sxFnc.pnodeRest->sxVar.sym->SetNeedDeclaration(false);
        }

        if (fFunctionsUndefined)
        {
            DefineFunctions(funcInfo, true);
        }
        DefineLabels(funcInfo);

        this->inPrologue = false;

        if (funcInfo->IsGlobalFunction())
        {
            EmitGlobalBody(funcInfo);
        }
        else
        {
            EmitFunctionBody(funcInfo);
        }

        if (pnode->sxFnc.pnodeBodyScope != nullptr)
        {
            ::EndEmitBlock(pnode->sxFnc.pnodeBodyScope, this, funcInfo);
        }
        ::EndEmitBlock(pnode->sxFnc.pnodeScopes, this, funcInfo);

        Assert(funcInfo->firstTmpReg == firstTmpReg);
        Assert(funcInfo->curTmpReg == firstTmpReg);
        Assert(byteCodeFunction->GetFirstTmpReg() == firstTmpReg + byteCodeFunction->GetConstantCount());

        byteCodeFunction->SetVarCount(funcInfo->varRegsCount);
        byteCodeFunction->SetOutParamDepth(funcInfo->outArgsMaxDepth);

        // Do a uint32 add just to verify that we haven't overflowed the reg slot type.
        UInt32Math::Add(funcInfo->varRegsCount, funcInfo->constRegsCount);

#if DBG_DUMP
        if (PHASE_STATS1(Js::ByteCodePhase))
        {
            Output::Print(L" BCode: %-10d, Aux: %-10d, AuxC: %-10d Total: %-10d,  %s\n",
                m_writer.ByteCodeDataSize(),
                m_writer.AuxiliaryDataSize(),
                m_writer.AuxiliaryContextDataSize(),
                m_writer.ByteCodeDataSize() + m_writer.AuxiliaryDataSize() + m_writer.AuxiliaryContextDataSize(),
                funcInfo->name);

            this->scriptContext->byteCodeDataSize += m_writer.ByteCodeDataSize();
            this->scriptContext->byteCodeAuxiliaryDataSize += m_writer.AuxiliaryDataSize();
            this->scriptContext->byteCodeAuxiliaryContextDataSize += m_writer.AuxiliaryContextDataSize();
        }
#endif

        this->MapCacheIdsToPropertyIds(funcInfo);
        this->MapReferencedPropertyIds(funcInfo);

        Assert(this->TopFuncInfo() == funcInfo);
        PopFuncInfo(L"EmitOneFunction");
        m_writer.SetCallSiteCount(m_callSiteId);
#ifdef LOG_BYTECODE_AST_RATIO
        m_writer.End(funcInfo->root->sxFnc.astSize, this->maxAstSize);
#else
        m_writer.End();
#endif
    }
    catch(...)
    {
        // Failed to generate byte-code for this function body (likely OOM or stack overflow). Notify the function body so that
        // it can revert intermediate state changes that may have taken place during byte code generation before the failure.
        byteCodeFunction->ResetByteCodeGenState();
        m_writer.Reset();
        throw;
    }

#ifdef PERF_HINT
    if (PHASE_TRACE1(Js::PerfHintPhase) && !byteCodeFunction->GetIsGlobalFunc())
    {
        if (byteCodeFunction->GetHasTry())
        {
            WritePerfHint(PerfHints::HasTryBlock_Verbose, byteCodeFunction);
        }

        if (funcInfo->GetCallsEval())
        {
            WritePerfHint(PerfHints::CallsEval_Verbose, byteCodeFunction);
        }
        else if (funcInfo->GetChildCallsEval())
        {
            WritePerfHint(PerfHints::ChildCallsEval, byteCodeFunction);
        }
    }
#endif


    byteCodeFunction->SetInitialDefaultEntryPoint();

    byteCodeFunction->SetIsByteCodeDebugMode(this->IsInDebugMode());

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    if (byteCodeFunction->IsByteCodeDebugMode() != scriptContext->IsInDebugMode()) // debug mode mismatch
    {
        if (m_utf8SourceInfo->GetIsLibraryCode())
        {
            Assert(!byteCodeFunction->IsByteCodeDebugMode()); // Library script byteCode is never in debug mode
        }
        else
        {
            Js::Throw::FatalInternalError();
        }
    }
#endif

#if DBG_DUMP
    if(PHASE_DUMP(Js::ByteCodePhase, funcInfo->byteCodeFunction) && Js::Configuration::Global.flags.Verbose)
    {
        pnode->Dump();
    }
    if (this->Trace() || PHASE_DUMP(Js::ByteCodePhase, funcInfo->byteCodeFunction))
    {
        Js::ByteCodeDumper::Dump(byteCodeFunction);
    }
    if(PHASE_DUMP(Js::DebuggerScopePhase, funcInfo->byteCodeFunction))
    {
        byteCodeFunction->DumpScopes();
    }
#endif
#ifdef ENABLE_NATIVE_CODEGEN
    if ((!PHASE_OFF(Js::BackEndPhase, funcInfo->byteCodeFunction))
        && !this->forceNoNative
        && !this->scriptContext->GetConfig()->IsNoNative()
        && ((this->flags & fscrIsNativeCode) == 0))
    {
        GenerateFunction(this->scriptContext->GetNativeCodeGenerator(), byteCodeFunction);
    }
#endif
}

void ByteCodeGenerator::MapCacheIdsToPropertyIds(FuncInfo *funcInfo)
{
    Js::FunctionBody *functionBody = funcInfo->GetParsedFunctionBody();
    uint rootObjectLoadInlineCacheStart = funcInfo->GetInlineCacheCount();
    uint rootObjectLoadMethodInlineCacheStart = rootObjectLoadInlineCacheStart + funcInfo->GetRootObjectLoadInlineCacheCount();
    uint rootObjectStoreInlineCacheStart = rootObjectLoadMethodInlineCacheStart + funcInfo->GetRootObjectLoadMethodInlineCacheCount();
    uint totalFieldAccessInlineCacheCount = rootObjectStoreInlineCacheStart + funcInfo->GetRootObjectStoreInlineCacheCount();

    functionBody->CreateCacheIdToPropertyIdMap(rootObjectLoadInlineCacheStart, rootObjectLoadMethodInlineCacheStart,
        rootObjectStoreInlineCacheStart, totalFieldAccessInlineCacheCount, funcInfo->GetIsInstInlineCacheCount());

    if (totalFieldAccessInlineCacheCount == 0)
    {
        return;
    }

    funcInfo->inlineCacheMap->Map([functionBody](Js::RegSlot regSlot, FuncInfo::InlineCacheIdMap *inlineCacheIdMap)
    {
        inlineCacheIdMap->Map([functionBody](Js::PropertyId propertyId, FuncInfo::InlineCacheList* inlineCacheList)
        {
            if (inlineCacheList)
            {
                inlineCacheList->Iterate([functionBody, propertyId](InlineCacheUnit cacheUnit)
                {
                    CompileAssert(offsetof(InlineCacheUnit, cacheId) == offsetof(InlineCacheUnit, loadCacheId));
                    if (cacheUnit.loadCacheId != -1)
                    {
                        functionBody->SetPropertyIdForCacheId(cacheUnit.loadCacheId, propertyId);
                    }
                    if (cacheUnit.loadMethodCacheId != -1)
                    {
                        functionBody->SetPropertyIdForCacheId(cacheUnit.loadMethodCacheId, propertyId);
                    }
                    if (cacheUnit.storeCacheId != -1)
                    {
                        functionBody->SetPropertyIdForCacheId(cacheUnit.storeCacheId, propertyId);
                    }
                });
            }
        });
    });

    funcInfo->rootObjectLoadInlineCacheMap->Map([functionBody, rootObjectLoadInlineCacheStart](Js::PropertyId propertyId, uint cacheId)
    {
        functionBody->SetPropertyIdForCacheId(cacheId + rootObjectLoadInlineCacheStart, propertyId);
    });
    funcInfo->rootObjectLoadMethodInlineCacheMap->Map([functionBody, rootObjectLoadMethodInlineCacheStart](Js::PropertyId propertyId, uint cacheId)
    {
        functionBody->SetPropertyIdForCacheId(cacheId + rootObjectLoadMethodInlineCacheStart, propertyId);
    });
    funcInfo->rootObjectStoreInlineCacheMap->Map([functionBody, rootObjectStoreInlineCacheStart](Js::PropertyId propertyId, uint cacheId)
    {
        functionBody->SetPropertyIdForCacheId(cacheId + rootObjectStoreInlineCacheStart, propertyId);
    });

    SListBase<uint>::Iterator valueOfIter(&funcInfo->valueOfStoreCacheIds);
    while (valueOfIter.Next())
    {
        functionBody->SetPropertyIdForCacheId(valueOfIter.Data(), Js::PropertyIds::valueOf);
    }

    SListBase<uint>::Iterator toStringIter(&funcInfo->toStringStoreCacheIds);
    while (toStringIter.Next())
    {
        functionBody->SetPropertyIdForCacheId(toStringIter.Data(), Js::PropertyIds::toString);
    }

#if DBG
    functionBody->VerifyCacheIdToPropertyIdMap();
#endif
}

void
ByteCodeGenerator::MapReferencedPropertyIds(FuncInfo * funcInfo)
{
    Js::FunctionBody *functionBody = funcInfo->GetParsedFunctionBody();
    uint referencedPropertyIdCount = funcInfo->GetReferencedPropertyIdCount();
    functionBody->CreateReferencedPropertyIdMap(referencedPropertyIdCount);

    funcInfo->referencedPropertyIdToMapIndex->Map([functionBody](Js::PropertyId propertyId, uint mapIndex)
    {
        functionBody->SetReferencedPropertyIdWithMapIndex(mapIndex, propertyId);
    });

#if DBG
    functionBody->VerifyReferencedPropertyIdMap();
#endif
}

void ByteCodeGenerator::EmitScopeList(ParseNode *pnode)
{
    while (pnode)
    {
        switch (pnode->nop)
        {
        case knopFncDecl:
            if (pnode->sxFnc.GetAsmjsMode())
            {
                Js::ExclusiveContext context(this, GetScriptContext());
                if (Js::AsmJSCompiler::Compile(&context, pnode, pnode->sxFnc.pnodeArgs))
                {
                    pnode = pnode->sxFnc.pnodeNext;
                    break;
                }
                else if (CONFIG_FLAG(AsmJsStopOnError))
                {
                    exit(JSERR_AsmJsCompileError);
                }
            }
            //FALLTHROUGH
        case knopProg:
            if (pnode->sxFnc.funcInfo)
            {
                this->StartEmitFunction(pnode);
                this->EmitScopeList(pnode->sxFnc.pnodeScopes);
                this->EmitOneFunction(pnode);
                this->EndEmitFunction(pnode);
            }
            pnode = pnode->sxFnc.pnodeNext;
            break;

        case knopBlock:
            this->StartEmitBlock(pnode);
            this->EmitScopeList(pnode->sxBlock.pnodeScopes);
            this->EndEmitBlock(pnode);
            pnode = pnode->sxBlock.pnodeNext;
            break;

        case knopCatch:
            this->StartEmitCatch(pnode);
            this->EmitScopeList(pnode->sxCatch.pnodeScopes);
            this->EndEmitCatch(pnode);
            pnode = pnode->sxCatch.pnodeNext;
            break;

        case knopWith:
            this->StartEmitWith(pnode);
            this->EmitScopeList(pnode->sxWith.pnodeScopes);
            this->EndEmitWith(pnode);
            pnode = pnode->sxWith.pnodeNext;
            break;

        default:
            AssertMsg(false, "Unexpected opcode in tree of scopes");
            break;
        }
    }
}

void
EnsureFncDeclScopeSlot(ParseNode *pnodeFnc, FuncInfo *funcInfo)
{
    VisitFncNames(pnodeFnc, [funcInfo](ParseNode *pnodeFnc, ParseNode *pnodeName)
    {
        if (pnodeName && pnodeName->nop == knopVarDecl)
        {
            Symbol *sym = pnodeName->sxVar.sym;
            if (sym)
            {
                sym->EnsureScopeSlot(funcInfo);
            }
        }
    });
}

// Similar to EnsureFncScopeSlot visitor function, but verifies that a slot is needed before assigning it.
void
CheckFncDeclScopeSlot(ParseNode *pnodeFnc, FuncInfo *funcInfo)
{
    VisitFncNames(pnodeFnc, [funcInfo](ParseNode *pnodeFnc, ParseNode *pnodeName)
    {
        if (pnodeName && pnodeName->nop == knopVarDecl)
        {
            Symbol *sym = pnodeName->sxVar.sym;
            if (sym && sym->NeedsSlotAlloc(funcInfo))
            {
                sym->EnsureScopeSlot(funcInfo);
            }
        }
    });
}

bool ByteCodeGenerator::NeedObjectAsFunctionScope(FuncInfo * funcInfo, ParseNode * pnodeFnc) const
{
    return funcInfo->GetCallsEval()
        || funcInfo->GetChildCallsEval()
        || NeedScopeObjectForArguments(funcInfo, pnodeFnc)
        || (this->flags & (fscrEval | fscrImplicitThis | fscrImplicitParents));
}

void ByteCodeGenerator::StartEmitFunction(ParseNode *pnodeFnc)
{
    Assert(pnodeFnc->nop == knopFncDecl || pnodeFnc->nop == knopProg);

    FuncInfo *funcInfo = pnodeFnc->sxFnc.funcInfo;

    if (funcInfo->byteCodeFunction->IsFunctionParsed() &&
        !(flags & (fscrEval | fscrImplicitThis | fscrImplicitParents)))
    {
        // Only set the environment depth if it's truly known (i.e., not in eval or event handler).
        funcInfo->GetParsedFunctionBody()->SetEnvDepth(this->envDepth);
    }

    if (funcInfo->GetCallsEval())
    {
        funcInfo->byteCodeFunction->SetDontInline(true);
    }

    Scope * const funcExprScope = funcInfo->funcExprScope;
    if (funcExprScope)
    {
        if (funcInfo->GetCallsEval())
        {
            Assert(funcExprScope->GetIsObject());
        }

        if (funcExprScope->GetIsObject())
        {
            funcExprScope->SetCapturesAll(true);
            funcExprScope->SetMustInstantiate(true);
            PushScope(funcExprScope);
        }
        else
        {
            Symbol *sym = funcInfo->root->sxFnc.GetFuncSymbol();
            funcInfo->bodyScope->AddSymbol(sym);
        }
    }

    Scope * const bodyScope = funcInfo->bodyScope;

    if (pnodeFnc->nop != knopProg)
    {
        if (!bodyScope->GetIsObject() && NeedObjectAsFunctionScope(funcInfo, pnodeFnc))
        {
            Assert(bodyScope->GetIsObject());
        }

        if (bodyScope->GetIsObject())
        {
            bodyScope->SetLocation(funcInfo->frameObjRegister);
        }
        else
        {
            bodyScope->SetLocation(funcInfo->frameSlotsRegister);
        }

        bodyScope->SetMustInstantiate(funcInfo->frameObjRegister != Js::Constants::NoRegister || funcInfo->frameSlotsRegister != Js::Constants::NoRegister);

        if (bodyScope->GetIsObject())
        {
            // Win8 908700: Disable under F12 debugger because there are too many cached scopes holding onto locals.
            funcInfo->SetHasCachedScope(
                !PHASE_OFF(Js::CachedScopePhase, funcInfo->byteCodeFunction) &&
                !funcInfo->Escapes() &&
                funcInfo->frameObjRegister != Js::Constants::NoRegister &&
                !ApplyEnclosesArgs(pnodeFnc, this) &&
                (PHASE_FORCE(Js::CachedScopePhase, funcInfo->byteCodeFunction) || !IsInDebugMode()));

            if (funcInfo->GetHasCachedScope())
            {
                Assert(funcInfo->funcObjRegister == Js::Constants::NoRegister);
                Symbol *funcSym = funcInfo->root->sxFnc.GetFuncSymbol();
                if (funcSym && funcSym->GetFuncExpr())
                {
                    if (funcSym->GetLocation() == Js::Constants::NoRegister)
                    {
                        funcInfo->funcObjRegister = funcInfo->NextVarRegister();
                    }
                    else
                    {
                        funcInfo->funcObjRegister = funcSym->GetLocation();
                    }
                }
                else
                {
                    funcInfo->funcObjRegister = funcInfo->NextVarRegister();
                }
                Assert(funcInfo->funcObjRegister != Js::Constants::NoRegister);
            }

            ParseNode *pnode;
            Symbol *sym;

            PushScope(bodyScope);

            // Turns on capturesAll temporarily if func has deferred child, so that the following EnsureScopeSlot
            // will allocate scope slots no matter if symbol hasNonLocalRefence or not.
            Js::ScopeInfo::AutoCapturesAllScope autoCapturesAllScope(bodyScope, funcInfo->HasDeferredChild());

            if (funcInfo->GetHasArguments())
            {
                // Process function's formal parameters
                MapFormals(pnodeFnc, [&](ParseNode *pnode) { pnode->sxVar.sym->EnsureScopeSlot(funcInfo); });

                // Only allocate scope slot for "arguments" when really necessary. "hasDeferredChild"
                // doesn't require scope slot for "arguments" because inner functions can't access
                // outer function's arguments directly.
                sym = funcInfo->GetArgumentsSymbol();
                Assert(sym);
                if (sym->GetHasNonLocalReference() || autoCapturesAllScope.OldCapturesAll())
                {
                    sym->EnsureScopeSlot(funcInfo);
                }
            }

            sym = funcInfo->root->sxFnc.GetFuncSymbol();

            if (sym && sym->NeedsSlotAlloc(funcInfo))
            {
                if (funcInfo->funcExprScope && funcInfo->funcExprScope->GetIsObject())
                {
                    sym->SetScopeSlot(0);
                }
                else if (funcInfo->GetFuncExprNameReference())
                {
                    sym->EnsureScopeSlot(funcInfo);
                }
            }

            if (!funcInfo->GetHasArguments())
            {
                Symbol *formal;
                Js::ArgSlot pos = 1;
                auto moveArgToReg = [&](ParseNode *pnode)
                {
                    formal = pnode->sxVar.sym;
                    // Get the param from its argument position into its assigned register.
                    // The position should match the location, otherwise, it has been shadowed by parameter with the same name
                    if (formal->GetLocation() + 1 == pos)
                    {
                        pnode->sxVar.sym->EnsureScopeSlot(funcInfo);
                    }
                    pos++;
                };
                MapFormals(pnodeFnc, moveArgToReg);
            }

            if (funcInfo->isThisLexicallyCaptured)
            {
                funcInfo->EnsureThisScopeSlot();
            }
            if (((!funcInfo->IsLambda() && funcInfo->GetCallsEval())
                || funcInfo->isSuperLexicallyCaptured)
                && funcInfo->superRegister != Js::Constants::NoRegister)
            {
                funcInfo->EnsureSuperScopeSlot();
            }

            auto ensureFncDeclScopeSlots = [&](ParseNode *pnodeScope) {
                for (pnode = pnodeScope; pnode;)
                {
                    switch (pnode->nop)
                    {
                    case knopFncDecl:
                        if (pnode->sxFnc.IsDeclaration())
                        {
                            EnsureFncDeclScopeSlot(pnode, funcInfo);
                        }
                        pnode = pnode->sxFnc.pnodeNext;
                        break;
                    case knopBlock:
                        pnode = pnode->sxBlock.pnodeNext;
                        break;
                    case knopCatch:
                        pnode = pnode->sxCatch.pnodeNext;
                        break;
                    case knopWith:
                        pnode = pnode->sxWith.pnodeNext;
                        break;
                    }
                }
            };
            pnodeFnc->sxFnc.MapContainerScopes(ensureFncDeclScopeSlots);

            for (pnode = pnodeFnc->sxFnc.pnodeVars; pnode; pnode = pnode->sxVar.pnodeNext)
            {
                sym = pnode->sxVar.sym;
                if (!(pnode->sxVar.isBlockScopeFncDeclVar && sym->GetIsBlockVar()))
                {
                    if (sym->GetIsCatch() || (pnode->nop == knopVarDecl && sym->GetIsBlockVar()))
                    {
                        sym = funcInfo->bodyScope->FindLocalSymbol(sym->GetName());
                    }
                    if (sym->GetSymbolType() == STVariable && !sym->GetIsArguments())
                    {
                        sym->EnsureScopeSlot(funcInfo);
                    }
                }
            }

            if (pnodeFnc->sxFnc.pnodeBody)
            {
                Assert(pnodeFnc->sxFnc.pnodeScopes->nop == knopBlock);
                this->EnsureLetConstScopeSlots(pnodeFnc->sxFnc.pnodeBodyScope, funcInfo);
            }
        }
        else
        {
            ParseNode *pnode;
            Symbol *sym;

            // Don't rely on the Emit() pass to assign scope slots where needed, because peeps/shortcuts
            // may cause some expressions not to be emitted. Assign the slots we need before we start
            // emitting the prolog.
            // TODO: Investigate moving detection of non-local references to Emit() so we don't assign
            // slots to symbols that are never referenced in emitted code.

            if (funcInfo->isThisLexicallyCaptured)
            {
                funcInfo->EnsureThisScopeSlot();
            }
            if (funcInfo->isSuperLexicallyCaptured)
            {
                funcInfo->EnsureSuperScopeSlot();
            }

            pnodeFnc->sxFnc.MapContainerScopes([&](ParseNode *pnodeScope) { this->EnsureFncScopeSlots(pnodeScope, funcInfo); });

            PushScope(bodyScope);

            for (pnode = pnodeFnc->sxFnc.pnodeVars; pnode; pnode = pnode->sxVar.pnodeNext)
            {
                sym = pnode->sxVar.sym;
                if (!(pnode->sxVar.isBlockScopeFncDeclVar && sym->GetIsBlockVar()))
                {
                    if (sym->GetIsCatch() || (pnode->nop == knopVarDecl && sym->GetIsBlockVar()))
                    {
                        sym = funcInfo->bodyScope->FindLocalSymbol(sym->GetName());
                    }
                    if (sym->GetSymbolType() == STVariable && sym->NeedsSlotAlloc(funcInfo) && !sym->GetIsArguments())
                    {
                        sym->EnsureScopeSlot(funcInfo);
                    }
                }
            }

            auto ensureScopeSlot = [&](ParseNode *pnode)
            {
                sym = pnode->sxVar.sym;
                if (sym->GetSymbolType() == STFormal && sym->NeedsSlotAlloc(funcInfo))
                {
                    sym->EnsureScopeSlot(funcInfo);
                }
            };
            // Process function's formal parameters
            MapFormals(pnodeFnc, ensureScopeSlot);

            if (pnodeFnc->sxFnc.pnodeBody)
            {
                this->EnsureLetConstScopeSlots(pnodeFnc->sxFnc.pnodeScopes, funcInfo);
                this->EnsureLetConstScopeSlots(pnodeFnc->sxFnc.pnodeBodyScope, funcInfo);
            }
        }
    }
    else
    {
        bool newScopeForEval = (funcInfo->byteCodeFunction->GetIsStrictMode() && (this->GetFlags() & fscrEval));

        if (newScopeForEval)
        {
            Assert(bodyScope->GetIsObject());
        }

        PushScope(bodyScope);
    }

    // Persist outer func scope info if nested func is deferred
    if (CONFIG_FLAG(DeferNested))
    {
        FuncInfo* parentFunc = TopFuncInfo();
        Js::ScopeInfo::SaveScopeInfoForDeferParse(this, parentFunc, funcInfo);
        PushFuncInfo(L"StartEmitFunction", funcInfo);
    }
}

void ByteCodeGenerator::EnsureLetConstScopeSlots(ParseNode *pnodeBlock, FuncInfo *funcInfo)
{
    bool hasNonLocalReference = pnodeBlock->sxBlock.GetCallsEval() || pnodeBlock->sxBlock.GetChildCallsEval();
    auto ensureLetConstSlots = ([this, pnodeBlock, funcInfo, hasNonLocalReference](ParseNode *pnode)
        {
            Symbol *sym = pnode->sxVar.sym;
            sym->EnsureScopeSlot(funcInfo);
            if (hasNonLocalReference)
            {
                sym->SetHasNonLocalReference(true, this);
            }
        });
    IterateBlockScopedVariables(pnodeBlock, ensureLetConstSlots);
}

void ByteCodeGenerator::EnsureFncScopeSlots(ParseNode *pnode, FuncInfo *funcInfo)
{
    while (pnode)
    {
        switch (pnode->nop)
        {
        case knopFncDecl:
            if (pnode->sxFnc.IsDeclaration())
            {
                CheckFncDeclScopeSlot(pnode, funcInfo);
            }
            pnode = pnode->sxFnc.pnodeNext;
            break;
        case knopBlock:
            pnode = pnode->sxBlock.pnodeNext;
            break;
        case knopCatch:
            pnode = pnode->sxCatch.pnodeNext;
            break;
        case knopWith:
            pnode = pnode->sxWith.pnodeNext;
            break;
        }
    }
}

void ByteCodeGenerator::EndEmitFunction(ParseNode *pnodeFnc)
{
    Assert(pnodeFnc->nop == knopFncDecl || pnodeFnc->nop == knopProg);
    Assert(pnodeFnc->nop == knopFncDecl && currentScope->GetEnclosingScope() != nullptr || pnodeFnc->nop == knopProg);

    PopScope(); // function body

    FuncInfo *funcInfo = pnodeFnc->sxFnc.funcInfo;

    Scope *scope = funcInfo->funcExprScope;
    if (scope && scope->GetMustInstantiate())
    {
        Assert(currentScope == scope);
        PopScope();
    }

    if (CONFIG_FLAG(DeferNested))
    {
        Assert(funcInfo == this->TopFuncInfo());
        PopFuncInfo(L"EndEmitFunction");
    }
}

void ByteCodeGenerator::StartEmitCatch(ParseNode *pnodeCatch)
{
    Assert(pnodeCatch->nop == knopCatch);

    Scope *scope = pnodeCatch->sxCatch.scope;
    Symbol *sym = pnodeCatch->sxCatch.pnodeParam->sxPid.sym;
    FuncInfo *funcInfo = scope->GetFunc();

    // Catch scope is a dynamic object if it can be passed to a scoped lookup helper (i.e., eval is present or we're in an event handler).
    if (funcInfo->GetCallsEval() || funcInfo->GetChildCallsEval() || (this->flags & (fscrEval | fscrImplicitThis | fscrImplicitParents)))
    {
        scope->SetIsObject();
    }
    // Catch object is stored in the catch scope if there may be an ambiguous lookup or a var declaration that hides it.
    scope->SetCapturesAll(funcInfo->GetCallsEval() || funcInfo->GetChildCallsEval() || sym->GetHasNonLocalReference());
    scope->SetMustInstantiate(scope->GetCapturesAll() || funcInfo->IsGlobalFunction() || currentScope != funcInfo->GetBodyScope());

    if (scope->GetMustInstantiate())
    {
        // Give slot 0 (the only one) to the catch symbol.
        sym->SetScopeSlot(0);
        PushScope(scope);
    }
    else
    {
        // Add it to the parent function's scope and treat it like any other local.
        // We can only do this if we don't need to get the symbol from a slot, though, because adding it to the
        // parent's scope object on entry to the catch could re-size the slot array.
        funcInfo->bodyScope->AddSymbol(sym);
    }
}

void ByteCodeGenerator::EndEmitCatch(ParseNode *pnodeCatch)
{
    Assert(pnodeCatch->nop == knopCatch);
    if (pnodeCatch->sxCatch.scope->GetMustInstantiate())
    {
        Assert(currentScope == pnodeCatch->sxCatch.scope);
        PopScope();
    }
}

void ByteCodeGenerator::StartEmitBlock(ParseNode *pnodeBlock)
{
    if (!BlockHasOwnScope(pnodeBlock, this))
    {
        return;
    }

    Assert(pnodeBlock->nop == knopBlock);

    PushBlock(pnodeBlock);

    Scope *scope = pnodeBlock->sxBlock.scope;
    if (pnodeBlock->sxBlock.GetCallsEval() || pnodeBlock->sxBlock.GetChildCallsEval() || (this->flags & (fscrEval | fscrImplicitThis | fscrImplicitParents)))
    {
        Assert(scope->GetIsObject());
    }

    // TODO: Consider nested deferred parsing.
    if (scope->GetMustInstantiate())
    {
        FuncInfo *funcInfo = scope->GetFunc();
        if (scope->IsGlobalEvalBlockScope() && funcInfo->isThisLexicallyCaptured)
        {
            funcInfo->EnsureThisScopeSlot();
        }
        this->EnsureFncScopeSlots(pnodeBlock->sxBlock.pnodeScopes, funcInfo);
        this->EnsureLetConstScopeSlots(pnodeBlock, funcInfo);
        PushScope(scope);
    }
}

void ByteCodeGenerator::EndEmitBlock(ParseNode *pnodeBlock)
{
    if (!BlockHasOwnScope(pnodeBlock, this))
    {
        return;
    }

    Assert(pnodeBlock->nop == knopBlock);

    Scope *scope = pnodeBlock->sxBlock.scope;
    if (scope && scope->GetMustInstantiate())
    {
        Assert(currentScope == pnodeBlock->sxBlock.scope);
        PopScope();
    }

    PopBlock();
}

void ByteCodeGenerator::StartEmitWith(ParseNode *pnodeWith)
{
    Assert(pnodeWith->nop == knopWith);

    Scope *scope = pnodeWith->sxWith.scope;

    Assert(scope->GetIsObject());

    PushScope(scope);
}

void ByteCodeGenerator::EndEmitWith(ParseNode *pnodeWith)
{
    Assert(pnodeWith->nop == knopWith);
    Assert(currentScope == pnodeWith->sxWith.scope);

    PopScope();
}

Js::RegSlot ByteCodeGenerator::PrependLocalScopes(Js::RegSlot evalEnv, Js::RegSlot tempLoc, FuncInfo *funcInfo)
{
    Scope *currScope = this->currentScope;
    Scope *funcScope = funcInfo->GetBodyScope();

    if (currScope == funcScope)
    {
        return evalEnv;
    }

    bool acquireTempLoc = tempLoc == Js::Constants::NoRegister;
    if (acquireTempLoc)
    {
        tempLoc = funcInfo->AcquireTmpRegister();
    }

    // This is a bit of an ugly case. The with/catch objects must be prepended to the environment we pass
    // to eval() or to a func declared inside with, but the list must first be reversed so that innermost
    // scopes appear first in the list.
    while (currScope != funcScope)
    {
        Scope *tmp;
        for (tmp = currScope; tmp->GetEnclosingScope() != funcScope; tmp = tmp->GetEnclosingScope())
            ;
        if (tmp->GetMustInstantiate())
        {
            this->m_writer.Reg3(Js::OpCode::LdFrameDisplay, tempLoc, tmp->GetLocation(), evalEnv);
            evalEnv = tempLoc;
        }
        funcScope = tmp;
    }

    if (acquireTempLoc)
    {
        funcInfo->ReleaseTmpRegister(tempLoc);
    }
    return evalEnv;
}

Scope * ByteCodeGenerator::FindScopeForSym(Scope *symScope, Scope *scope, Js::PropertyId *envIndex, FuncInfo *funcInfo) const
{
    for (scope = scope ? scope->GetEnclosingScope() : currentScope; scope; scope = scope->GetEnclosingScope())
    {
        if (scope->GetFunc() != funcInfo && scope->GetMustInstantiate() && scope != this->globalScope)
        {
            (*envIndex)++;
        }
        if (scope == symScope || scope->GetIsDynamic())
        {
            break;
        }
    }

    Assert(scope);
    return scope;
}

void ByteCodeGenerator::EmitLoadInstance(Symbol *sym, IdentPtr pid, Js::RegSlot *pThisLocation, Js::RegSlot *pInstLocation, FuncInfo *funcInfo)
{
    Js::ByteCodeLabel doneLabel = 0;
    bool fLabelDefined = false;
    Js::RegSlot scopeLocation = Js::Constants::NoRegister;
    Js::RegSlot thisLocation = *pThisLocation;
    Js::RegSlot instLocation = *pInstLocation;
    Js::PropertyId envIndex = -1;
    Scope *scope = null;
    Scope *symScope = sym ? sym->GetScope() : this->globalScope;
    Assert(symScope);

    for (;;)
    {
        scope = this->FindScopeForSym(symScope, scope, &envIndex, funcInfo);
        if (scope == this->globalScope)
        {
            break;
        }

        if (scope != symScope)
        {
            // We're not sure where the function is (eval/with/etc).
            // So we're going to need registers to hold the instance where we (dynamically) find
            // the function, and possibly to hold the "this" pointer we will pass to it.
            // Assign them here so that they can't overlap with the scopeLocation assigned below.
            // Otherwise we wind up with temp lifetime confusion in the IRBuilder. (Win8 281689.)
            if (instLocation == Js::Constants::NoRegister)
            {
                instLocation = funcInfo->AcquireTmpRegister();
                // The "this" pointer will not be the same as the instance, so give it its
                // own register.
                thisLocation = funcInfo->AcquireTmpRegister();
            }
        }

        if (envIndex != -1)
        {
            // The scope is in the closure environment, so retrieve it.
            if (scope != symScope || instLocation != Js::Constants::NoRegister)
            {
                scopeLocation = funcInfo->AcquireTmpRegister();
                this->m_writer.Slot(Js::OpCode::LdSlotArr, scopeLocation, funcInfo->GetEnvRegister(), envIndex + Js::FrameDisplay::GetOffsetOfScopes()/sizeof(Js::Var));
                funcInfo->ReleaseTmpRegister(scopeLocation);
            }
        }
        else
        {
            Assert(funcInfo == scope->GetFunc());
            scopeLocation = scope->GetLocation();
        }

        if (scope == symScope)
        {
            break;
        }

        // Found a scope to which the property may have been added.
        Assert(scope && scope->GetIsDynamic());

        if (!fLabelDefined)
        {
            fLabelDefined = true;
            doneLabel = this->m_writer.DefineLabel();
        }

        Js::ByteCodeLabel nextLabel = this->m_writer.DefineLabel();
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        this->m_writer.BrProperty(Js::OpCode::BrOnNoProperty, nextLabel, scopeLocation,
            funcInfo->FindOrAddReferencedPropertyId(propertyId));

        this->m_writer.Reg2(Js::OpCode::Ld_A, instLocation, scopeLocation);

        if (thisLocation != Js::Constants::NoRegister)
        {
            if (scope->GetScopeType() == ScopeType_With && scriptContext->GetConfig()->IsES6UnscopablesEnabled())
            {
                this->m_writer.Reg2(Js::OpCode::Ld_UnwrapWithObj, thisLocation, scopeLocation);
            }
            else
            {
                this->m_writer.Reg2(Js::OpCode::Ld_A, thisLocation, scopeLocation);
            }
        }

        this->m_writer.Br(doneLabel);
        this->m_writer.MarkLabel(nextLabel);
    }

    if (sym == null || sym->GetIsGlobal())
    {
        if (this->flags & (fscrEval | fscrImplicitThis | fscrImplicitParents))
        {
            // Load of a symbol with unknown scope from within eval.
            // Get it from the closure environment.
            if (instLocation == Js::Constants::NoRegister)
            {
                instLocation = funcInfo->AcquireTmpRegister();
            }

            Js::RegSlot ldFromReg = funcInfo->GetEnvRegister();
            if (funcInfo->GetIsStrictMode() && funcInfo->IsGlobalFunction())
            {
                ldFromReg = funcInfo->frameDisplayRegister;
            }

            // TODO: It should be possible to avoid this double call to ScopedLdInst by having it return both
            // results at once. The reason for the uncertainty here is that we don't know whether the callee
            // belongs to a "with" object. If it does, we have to pass the "with" object as "this"; in all other
            // cases, we pass "undefined". I'm not investing time on it now because I don't see any significant
            // benchmark losses.
            Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();

            if (thisLocation == Js::Constants::NoRegister)
            {
                thisLocation = funcInfo->AcquireTmpRegister();
            }
            this->m_writer.Property2(Js::OpCode::ScopedLdInst, instLocation, ldFromReg,
                funcInfo->FindOrAddReferencedPropertyId(propertyId), thisLocation);
        }
        else
        {
            if (instLocation == Js::Constants::NoRegister)
            {
                instLocation = ByteCodeGenerator::RootObjectRegister;
            }
            else
            {
                this->m_writer.Reg2(Js::OpCode::Ld_A, instLocation, ByteCodeGenerator::RootObjectRegister);
            }

            if (thisLocation == Js::Constants::NoRegister)
            {
                thisLocation = funcInfo->undefinedConstantRegister;
            }
            else
            {
                this->m_writer.Reg2(Js::OpCode::Ld_A, thisLocation, funcInfo->undefinedConstantRegister);
            }
        }
    }
    else if (instLocation != Js::Constants::NoRegister)
    {
        if (symScope != funcInfo->GetBodyScope())
        {
            this->m_writer.Reg2(Js::OpCode::Ld_A, instLocation, scopeLocation);
        }
        else
        {
            this->m_writer.Reg2(Js::OpCode::Ld_A, instLocation, funcInfo->frameObjRegister);
        }

        if (thisLocation != Js::Constants::NoRegister)
        {
            this->m_writer.Reg2(Js::OpCode::Ld_A, thisLocation, funcInfo->undefinedConstantRegister);
        }
        else
        {
            thisLocation = funcInfo->undefinedConstantRegister;
        }
    }

    *pThisLocation = thisLocation;
    *pInstLocation = instLocation;

    if (fLabelDefined)
    {
        this->m_writer.MarkLabel(doneLabel);
    }
}

void ByteCodeGenerator::EmitGlobalFncDeclInit(Js::RegSlot rhsLocation, Js::PropertyId propertyId, FuncInfo * funcInfo)
{
    // Note: declared variables and assignments in the global function go to the root object directly.
    if (this->flags & fscrEval)
    {
        // Func decl's always get their init values before any use, so we don't pre-initialize the property to undef.
        // That means that we have to use ScopedInitFld so that we initialize the property on the right instance
        // even if the instance doesn't have the property yet (i.e., collapse the init-to-undef and the store
        // into one operation). See WOOB 1121763 and 1120973.
        if (funcInfo->GetIsStrictMode())
        {
            this->m_writer.Property(Js::OpCode::ScopedInitFunc, rhsLocation, funcInfo->frameDisplayRegister,
                funcInfo->FindOrAddReferencedPropertyId(propertyId));
        }
        else
        {
            // Distinguish block-scope and function-scope cases.
            this->m_writer.Property(Js::OpCode::ScopedInitFunc, rhsLocation, funcInfo->GetEnvRegister(),
                funcInfo->FindOrAddReferencedPropertyId(propertyId));
        }
    }
    else
    {
        this->EmitPatchableRootProperty(Js::OpCode::InitRootFld, rhsLocation, propertyId, false, true, funcInfo);
    }
}

void
ByteCodeGenerator::EmitPatchableRootProperty(Js::OpCode opcode,
    Js::RegSlot regSlot, Js::PropertyId propertyId, bool isLoadMethod, bool isStore, FuncInfo * funcInfo)
{
    uint cacheId = funcInfo->FindOrAddRootObjectInlineCacheId(propertyId, isLoadMethod, isStore);
    this->m_writer.PatchableRootProperty(opcode, regSlot, cacheId, isLoadMethod, isStore);
}

void ByteCodeGenerator::EmitLocalPropInit(Js::RegSlot rhsLocation, Symbol *sym, FuncInfo *funcInfo)
{
    Scope *scope = sym->GetScope();

    // Check consistency of sym->IsInSlot.
    Assert(sym->NeedsSlotAlloc(funcInfo) || sym->GetScopeSlot() == Js::Constants::NoProperty);

    // Arrived at the scope in which the property was defined.
    if (sym->IsInSlot(funcInfo))
    {
        // The property is in memory rather than register. We'll have to load it from the slots.
        if (scope->GetIsObject())
        {
            Assert(!this->TopFuncInfo()->GetParsedFunctionBody()->DoStackNestedFunc());
            Js::PropertyId propertyId = sym->EnsurePosition(this);
            uint cacheId = funcInfo->FindOrAddInlineCacheId(scope->GetLocation(), propertyId, false, true);
            this->m_writer.PatchableProperty(sym->GetIsNonSimpleParameter() ? Js::OpCode::InitLetFld : Js::OpCode::InitFld, rhsLocation, scope->GetLocation(), cacheId);
        }
        else
        {
            // Make sure the property has a slot. This will bump up the size of the slot array if necessary.
            Js::PropertyId slot = sym->EnsureScopeSlot(funcInfo);

            Js::RegSlot slotReg = scope->GetCanMerge() ? funcInfo->frameSlotsRegister : scope->GetLocation();
            // Now store the property to its slot.
            this->m_writer.Slot(Js::OpCode::StSlot, rhsLocation, slotReg, slot + Js::ScopeSlots::FirstSlotIndex);
        }
    }
    else if (rhsLocation != sym->GetLocation())
    {
        this->m_writer.Reg2(Js::OpCode::Ld_A, sym->GetLocation(), rhsLocation);
    }
}

Js::PropertyId CheckTranslateSlotIndex(Symbol * sym, Js::PropertyId slot)
{
    return slot + (sym->GetScope()->GetIsObject()? 0 : Js::ScopeSlots::FirstSlotIndex);
}

void ByteCodeGenerator::EmitPropStore(Js::RegSlot rhsLocation, Symbol *sym, IdentPtr pid, FuncInfo *funcInfo, bool isLetDecl, bool isConstDecl, bool isFncDeclVar)
{
    Js::ByteCodeLabel doneLabel = 0;
    bool fLabelDefined = false;
    Js::PropertyId envIndex = -1;
    Scope *symScope = sym == null || sym->GetIsGlobal() ? this->globalScope : sym->GetScope();
    Assert(symScope);
    // isFncDeclVar denotes that the symbol being stored to here is the var
    // binding of a function declaration and we know we want to store directly
    // to it, skipping over any dynamic scopes that may lie in between.
    Scope *scope = isFncDeclVar ? symScope : null;
    Js::RegSlot scopeLocation = isFncDeclVar ? scope->GetLocation() : Js::Constants::NoRegister;
    bool scopeAcquired = false;

    for (;!isFncDeclVar;)
    {
        scope = this->FindScopeForSym(symScope, scope, &envIndex, funcInfo);
        if (scope == this->globalScope)
        {
            break;
        }
        if (envIndex != -1)
        {
            // The scope is in the closure environment, so retrieve it.
            if (!scopeAcquired)
            {
                scopeAcquired = true;
                scopeLocation = funcInfo->AcquireTmpRegister();
            }
            this->m_writer.Slot(Js::OpCode::LdSlotArr, scopeLocation, funcInfo->GetEnvRegister(), envIndex + Js::FrameDisplay::GetOffsetOfScopes()/sizeof(Js::Var));
        }
        else
        {
            Assert(funcInfo == scope->GetFunc());
            scopeLocation = scope->GetLocation();
        }

        if (scope == symScope)
        {
            break;
        }

        // Found a scope to which the property may have been added.
        Assert(scope && scope->GetIsDynamic());

        if (!fLabelDefined)
        {
            fLabelDefined = true;
            doneLabel = this->m_writer.DefineLabel();
        }
        Js::ByteCodeLabel nextLabel = this->m_writer.DefineLabel();
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        this->m_writer.BrProperty(Js::OpCode::BrOnNoProperty, nextLabel, scopeLocation,
            funcInfo->FindOrAddReferencedPropertyId(propertyId));

        uint cacheId = funcInfo->FindOrAddInlineCacheId(scopeLocation, propertyId, false, true);
        // REVIEW: isLetDecl and isConstDecl can never be true here now that eval's do not leak them, correct?
        this->m_writer.PatchableProperty(GetStFldOpCode(funcInfo, false, isLetDecl, isConstDecl), rhsLocation, scopeLocation, cacheId);

        this->m_writer.Br(doneLabel);
        this->m_writer.MarkLabel(nextLabel);
    }

    // Arrived at the scope in which the property was defined.
    if (sym == null || sym->GetIsGlobal())
    {
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        if (this->flags & fscrEval)
        {
            if (funcInfo->byteCodeFunction->GetIsStrictMode() && funcInfo->IsGlobalFunction())
            {
                uint cacheId = funcInfo->FindOrAddInlineCacheId(funcInfo->frameDisplayRegister, propertyId, false, true);
                this->m_writer.PatchableProperty(GetScopedStFldOpCode(funcInfo), rhsLocation, funcInfo->frameDisplayRegister, cacheId);
            }
            else
            {
                uint cacheId = funcInfo->FindOrAddInlineCacheId(funcInfo->GetEnvRegister(), propertyId, false, true);

                // In "eval", store to a symbol with unknown scope goes through the closure environment.
                this->m_writer.PatchableProperty(GetScopedStFldOpCode(funcInfo), rhsLocation, funcInfo->GetEnvRegister(), cacheId);
            }
        }
        else if (this->flags & (fscrImplicitThis | fscrImplicitParents))
        {
            uint cacheId = funcInfo->FindOrAddInlineCacheId(funcInfo->GetEnvRegister(), propertyId, false, true);

            // In "eval", store to a symbol with unknown scope goes through the closure environment.
            this->m_writer.PatchableProperty(GetScopedStFldOpCode(funcInfo), rhsLocation, funcInfo->GetEnvRegister(), cacheId);
        }
        else
        {
            this->EmitPatchableRootProperty(GetStFldOpCode(funcInfo, true, isLetDecl, isConstDecl), rhsLocation, propertyId, false, true, funcInfo);
        }
    }
    else if (sym->GetFuncExpr())
    {
        // Store to function expr variable.

        // strict mode, we need to to throw type error
        if (funcInfo->byteCodeFunction->GetIsStrictMode())
        {
            // Note that in this case the sym's location belongs to the parent function, so we can't use it.
            // It doesn't matter which register we use, as long as it's valid for this function.
            this->m_writer.W1(Js::OpCode::RuntimeTypeError, SCODE_CODE(JSERR_CantAssignToReadOnly));
        }
    }
    else if (sym->IsInSlot(funcInfo) || envIndex != -1)
    {
        if (!isConstDecl && sym->GetDecl() && sym->GetDecl()->nop == knopConstDecl)
        {
            // This is a case where const reassignment can't be proven statically (e.g., eval, with) so
            // we have to catch it at runtime.
            this->m_writer.W1(
                Js::OpCode::RuntimeReferenceError, SCODE_CODE(ERRAssignmentToConst));
        }
        // Make sure the property has a slot. This will bump up the size of the slot array if necessary.
        Js::PropertyId slot = sym->EnsureScopeSlot(funcInfo);
        bool chkBlockVar = !isLetDecl && !isConstDecl && NeedCheckBlockVar(sym, scope, funcInfo);

        // The property is in memory rather than register. We'll have to load it from the slots.
        if (scope->GetIsDynamic() || (scope->GetIsObject() && scope->IsBlockScope(funcInfo)))
        {
            // The scope is an object whose slots are subject to change. First load them from the object.
            Js::OpCode op = chkBlockVar ? Js::OpCode::StObjSlotChkUndecl : Js::OpCode::StObjSlot;
            this->m_writer.Slot(op, rhsLocation, scopeLocation, slot);
        }
        else
        {
            if (scopeLocation == funcInfo->frameSlotsRegister &&
                funcInfo->GetParsedFunctionBody()->DoStackScopeSlots() &&
                !this->inPrologue)
            {
                // The function allocated local slots on the stack, but they may since have been boxed, so
                // re-load them before accessing them.
                scopeLocation = funcInfo->AcquireTmpRegister();
                this->m_writer.Reg1(Js::OpCode::LdLocalScopeSlots, scopeLocation);
                funcInfo->ReleaseTmpRegister(scopeLocation);
            }

            // Now store the property to its slot.
            Js::OpCode op = chkBlockVar ? Js::OpCode::StSlotChkUndecl : Js::OpCode::StSlot;
            this->m_writer.Slot(op, rhsLocation, scopeLocation,
                slot + (sym->GetScope()->GetIsObject()? 0 : Js::ScopeSlots::FirstSlotIndex));
        }

        if (this->ShouldTrackDebuggerMetadata() && (isLetDecl || isConstDecl))
        {
            Js::PropertyId location = scope->GetIsObject() ? sym->GetLocation() : slot;
            this->UpdateDebuggerPropertyInitializationOffset(location, sym->GetPosition(), false);
        }
    }
    else if (isConstDecl)
    {
        this->m_writer.Reg2(Js::OpCode::InitConst, sym->GetLocation(), rhsLocation);

        if (this->ShouldTrackDebuggerMetadata())
        {
            this->UpdateDebuggerPropertyInitializationOffset(sym->GetLocation(), sym->GetPosition());
        }
    }
    else
    {
        if (!isConstDecl && sym->GetDecl() && sym->GetDecl()->nop == knopConstDecl)
        {
            // This is a case where const reassignment can't be proven statically (e.g., eval, with) so
            // we have to catch it at runtime.
            this->m_writer.W1(Js::OpCode::RuntimeReferenceError, SCODE_CODE(ERRAssignmentToConst));
        }
        if (rhsLocation != sym->GetLocation())
        {
            this->m_writer.Reg2(Js::OpCode::Ld_A, sym->GetLocation(), rhsLocation);

            if (this->ShouldTrackDebuggerMetadata() && isLetDecl)
            {
                this->UpdateDebuggerPropertyInitializationOffset(sym->GetLocation(), sym->GetPosition());
            }
        }
    }
    if (fLabelDefined)
    {
        this->m_writer.MarkLabel(doneLabel);
    }

    if (scopeAcquired)
    {
        funcInfo->ReleaseTmpRegister(scopeLocation);
    }
}

void ByteCodeGenerator::EmitPropLoad(Js::RegSlot lhsLocation, Symbol *sym, IdentPtr pid, FuncInfo *funcInfo)
{
    // If sym belongs to a parent frame, get it from the closure environment.
    // If it belongs to this func, but there's a non-local reference, get it from the heap-allocated frame.
    // (TODO: optimize this by getting the sym from its normal location if there are no non-local defs.)
    // Otherwise, just copy the value to the lhsLocation.

    Js::ByteCodeLabel doneLabel = 0;
    bool fLabelDefined = false;
    Js::RegSlot scopeLocation = Js::Constants::NoRegister;
    Js::PropertyId envIndex = -1;
    Scope *scope = null;
    Scope *symScope = sym ? sym->GetScope() : this->globalScope;
    Assert(symScope);

    for (;;)
    {
        scope = this->FindScopeForSym(symScope, scope, &envIndex, funcInfo);
        if (scope == this->globalScope)
        {
            break;
        }

        if (envIndex != -1)
        {
            // The scope is in the closure environment, so retrieve it.
            scopeLocation = funcInfo->AcquireTmpRegister();
            this->m_writer.Slot(Js::OpCode::LdSlotArr, scopeLocation, funcInfo->GetEnvRegister(), envIndex + Js::FrameDisplay::GetOffsetOfScopes()/sizeof(Js::Var));
            funcInfo->ReleaseTmpRegister(scopeLocation);
        }
        else
        {
            Assert(funcInfo == scope->GetFunc());
            scopeLocation = scope->GetLocation();
        }

        if (scope == symScope)
        {
            break;
        }

        // Found a scope to which the property may have been added.
        Assert(scope && scope->GetIsDynamic());

        if (!fLabelDefined)
        {
            fLabelDefined = true;
            doneLabel = this->m_writer.DefineLabel();
        }

        Js::ByteCodeLabel nextLabel = this->m_writer.DefineLabel();
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        this->m_writer.BrProperty(Js::OpCode::BrOnNoProperty, nextLabel, scopeLocation,
            funcInfo->FindOrAddReferencedPropertyId(propertyId));

        uint cacheId = funcInfo->FindOrAddInlineCacheId(scopeLocation, propertyId, false, false);
        this->m_writer.PatchableProperty(Js::OpCode::LdFld, lhsLocation, scopeLocation, cacheId);

        this->m_writer.Br(doneLabel);
        this->m_writer.MarkLabel(nextLabel);
    }

    // Arrived at the scope in which the property was defined.
    if (sym && sym->GetNeedDeclaration() && scope->GetFunc() == funcInfo)
    {
        // Ensure this symbol has a slot if it needs one.
        if(sym->IsInSlot(funcInfo))
        {
            Js::PropertyId slot = sym->EnsureScopeSlot(funcInfo);
            funcInfo->FindOrAddSlotProfileId(scope, slot);
        }

        EmitUseBeforeDeclarationRuntimeError(this, lhsLocation);
    }
    else if (sym == null || sym->GetIsGlobal())
    {
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        if (this->flags & fscrEval)
        {
            if (funcInfo->byteCodeFunction->GetIsStrictMode() && funcInfo->IsGlobalFunction())
            {
                uint cacheId = funcInfo->FindOrAddInlineCacheId(funcInfo->frameDisplayRegister, propertyId, false, false);
                this->m_writer.PatchableProperty(Js::OpCode::ScopedLdFld, lhsLocation, funcInfo->frameDisplayRegister, cacheId);
            }
            else
            {
                uint cacheId = funcInfo->FindOrAddInlineCacheId(funcInfo->GetEnvRegister(), propertyId, false, false);

                // Load of a symbol with unknown scope from within eval
                // Get it from the closure environment.
                this->m_writer.PatchableProperty(Js::OpCode::ScopedLdFld, lhsLocation, funcInfo->GetEnvRegister(), cacheId);
            }
        }
        else if (this->flags & (fscrImplicitThis | fscrImplicitParents))
        {
            uint cacheId = funcInfo->FindOrAddInlineCacheId(funcInfo->GetEnvRegister(), propertyId, false, false);

            // Load of a symbol with unknown scope from within eval or event handler.
            // Get it from the closure environment.
            this->m_writer.PatchableProperty(Js::OpCode::ScopedLdFld, lhsLocation, funcInfo->GetEnvRegister(), cacheId);
        }
        else
        {
            // Special case non-writable built ins
            // TODO: support non-writable global property in general by detecting what attribute the property have current?
            // But can't be done if we are byte code serialized, because the attribute might be different for use fields
            // next time we run. May want to catch that in the JIT
            Js::OpCode opcode = Js::OpCode::LdRootFld;

            // These properties are non-writable in IE9 mode or above only
            switch (propertyId)
            {
            case Js::PropertyIds::NaN:
                opcode = Js::OpCode::LdNaN;
                break;
            case Js::PropertyIds::Infinity:
                opcode = Js::OpCode::LdInfinity;
                break;
            case Js::PropertyIds::undefined:
                opcode = Js::OpCode::LdUndef;
                break;
            };
            if (opcode == Js::OpCode::LdRootFld)
            {
                this->EmitPatchableRootProperty(Js::OpCode::LdRootFld, lhsLocation, propertyId, false, false, funcInfo);
            }
            else
            {
                this->Writer()->Reg1(opcode, lhsLocation);
            }
        }
    }
    else if (sym->IsInSlot(funcInfo) || envIndex != -1)
    {
        // Make sure the property has a slot. This will bump up the size of the slot array if necessary.
        Js::PropertyId slot = sym->EnsureScopeSlot(funcInfo);
        Js::ProfileId profileId = funcInfo->FindOrAddSlotProfileId(scope, slot);
        bool chkBlockVar = NeedCheckBlockVar(sym, scope, funcInfo);
        Js::OpCode op;

        // The property is in memory rather than register. We'll have to load it from the slots.
        if (scope->GetIsDynamic() || (scope->GetIsObject() && scope->IsBlockScope(funcInfo)))
        {
            // The scope is an object whose slots are subject to change. First load them from the object.
            op = chkBlockVar ? Js::OpCode::LdObjSlotChkUndecl : Js::OpCode::LdObjSlot;
        }
        else
        {
            // Now get the property from its slot.
            op = chkBlockVar ? Js::OpCode::LdSlotChkUndecl : Js::OpCode::LdSlot;
            slot = slot + (sym->GetScope()->GetIsObject()? 0 : Js::ScopeSlots::FirstSlotIndex);

            if (scopeLocation == funcInfo->frameSlotsRegister &&
                funcInfo->GetParsedFunctionBody()->DoStackScopeSlots() &&
                !this->inPrologue)
            {
                // The function allocated local slots on the stack, but they may since have been boxed, so
                // re-load them before accessing them.
                scopeLocation = funcInfo->AcquireTmpRegister();
                this->m_writer.Reg1(Js::OpCode::LdLocalScopeSlots, scopeLocation);
                funcInfo->ReleaseTmpRegister(scopeLocation);
            }
        }
        this->m_writer.Slot(op, lhsLocation, scopeLocation, slot, profileId);
    }
    else
    {
        if (lhsLocation != sym->GetLocation())
        {
            this->m_writer.Reg2(Js::OpCode::Ld_A, lhsLocation, sym->GetLocation());
        }
        if (sym->GetIsBlockVar() && ((sym->GetDecl()->nop == knopLetDecl || sym->GetDecl()->nop == knopConstDecl) && sym->GetDecl()->sxVar.isSwitchStmtDecl) && CONFIG_FLAG(TDZ))
        {
            this->m_writer.Reg1(Js::OpCode::ChkUndecl, lhsLocation);
        }
    }

    if (fLabelDefined)
    {
        this->m_writer.MarkLabel(doneLabel);
    }
}

bool ByteCodeGenerator::NeedCheckBlockVar(Symbol* sym, Scope* scope, FuncInfo* funcInfo) const
{
    bool tdz = sym->GetIsBlockVar()
        && (scope->GetFunc() != funcInfo || ((sym->GetDecl()->nop == knopLetDecl || sym->GetDecl()->nop == knopConstDecl) && sym->GetDecl()->sxVar.isSwitchStmtDecl))
        && CONFIG_FLAG(TDZ);

    return tdz || sym->GetIsNonSimpleParameter();
}

void ByteCodeGenerator::EmitPropDelete(Js::RegSlot lhsLocation, Symbol *sym, IdentPtr pid, FuncInfo *funcInfo)
{
    // If sym belongs to a parent frame, delete it from the closure environment.
    // If it belongs to this func, but there's a non-local reference, get it from the heap-allocated frame.
    // (TODO: optimize this by getting the sym from its normal location if there are no non-local defs.)
    // Otherwise, just return false

    Js::ByteCodeLabel doneLabel = 0;
    bool fLabelDefined = false;
    Js::RegSlot scopeLocation;
    Js::PropertyId envIndex = -1;
    Scope *scope = null;
    Scope *symScope = sym ? sym->GetScope() : this->globalScope;
    Assert(symScope);

    for (;;)
    {
        scope = this->FindScopeForSym(symScope, scope, &envIndex, funcInfo);
        if (scope == this->globalScope)
        {
            scopeLocation = ByteCodeGenerator::RootObjectRegister;
        }
        else if (envIndex != -1)
        {
            // The scope is in the closure environment, so retrieve it.
            scopeLocation = funcInfo->AcquireTmpRegister();
            this->m_writer.Slot(Js::OpCode::LdSlotArr, scopeLocation, funcInfo->GetEnvRegister(), envIndex + Js::FrameDisplay::GetOffsetOfScopes()/sizeof(Js::Var));
            funcInfo->ReleaseTmpRegister(scopeLocation);
        }
        else
        {
            Assert(funcInfo == scope->GetFunc());
            scopeLocation = scope->GetLocation();
        }

        if (scope == symScope)
        {
            break;
        }

        // Found a scope to which the property may have been added.
        Assert(scope && scope->GetIsDynamic());

        if (!fLabelDefined)
        {
            fLabelDefined = true;
            doneLabel = this->m_writer.DefineLabel();
        }

        Js::ByteCodeLabel nextLabel = this->m_writer.DefineLabel();
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        this->m_writer.BrProperty(Js::OpCode::BrOnNoProperty, nextLabel, scopeLocation,
            funcInfo->FindOrAddReferencedPropertyId(propertyId));

        this->m_writer.Property(Js::OpCode::DeleteFld, lhsLocation, scopeLocation,
            funcInfo->FindOrAddReferencedPropertyId(propertyId));

        this->m_writer.Br(doneLabel);
        this->m_writer.MarkLabel(nextLabel);
    }

    // Arrived at the scope in which the property was defined.
    if (sym == null || sym->GetIsGlobal())
    {
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        if (this->flags & (fscrEval | fscrImplicitThis | fscrImplicitParents))
        {
            this->m_writer.Property(Js::OpCode::ScopedDeleteFld, lhsLocation, funcInfo->GetEnvRegister(),
                funcInfo->FindOrAddReferencedPropertyId(propertyId));
        }
        else
        {
            this->m_writer.Property(Js::OpCode::DeleteRootFld, lhsLocation, ByteCodeGenerator::RootObjectRegister,
                funcInfo->FindOrAddReferencedPropertyId(propertyId));
        }
    }
    else
    {
        // A minor hack to make sure that this sym gets properly put in a closure.
        // The delete will look like a non-local reference, so make sure a slot is reserved.
        // (A cleverer fix that avoids the closure altogether is possible but not worth it
        // for such a degenerate case.)
        sym->EnsureScopeSlot(funcInfo);
        this->m_writer.Reg1(Js::OpCode::LdFalse, lhsLocation);
    }

    if (fLabelDefined)
    {
        this->m_writer.MarkLabel(doneLabel);
    }
}

void ByteCodeGenerator::EmitTypeOfFld(FuncInfo * funcInfo, Js::PropertyId propertyId, Js::RegSlot value, Js::RegSlot instance, Js::OpCode ldFldOp)
{
 
    uint cacheId;
    Js::RegSlot tmpReg = funcInfo->AcquireTmpRegister();
    if (ldFldOp == Js::OpCode::LdRootFldForTypeOf)
    {
        cacheId = funcInfo->FindOrAddRootObjectInlineCacheId(propertyId, false, false);
        this->Writer()->PatchableRootProperty(ldFldOp, tmpReg, cacheId, false, false);
    }
    else
    {
        cacheId = funcInfo->FindOrAddInlineCacheId(instance, propertyId, false, false);
        this->Writer()->PatchableProperty(ldFldOp, tmpReg, instance, cacheId);
    }
    this->Writer()->Reg2(
        Js::OpCode::Typeof, value, tmpReg);
    funcInfo->ReleaseTmpRegister(tmpReg);
}

void ByteCodeGenerator::EmitPropTypeof(Js::RegSlot lhsLocation, Symbol *sym, IdentPtr pid, FuncInfo *funcInfo)
{
    // If sym belongs to a parent frame, delete it from the closure environment.
    // If it belongs to this func, but there's a non-local reference, get it from the heap-allocated frame.
    // (TODO: optimize this by getting the sym from its normal location if there are no non-local defs.)
    // Otherwise, just return false

    Js::ByteCodeLabel doneLabel = 0;
    bool fLabelDefined = false;
    Js::RegSlot scopeLocation;
    Js::PropertyId envIndex = -1;
    Scope *scope = null;
    Scope *symScope = sym ? sym->GetScope() : this->globalScope;
    Assert(symScope);

    for (;;)
    {
        scope = this->FindScopeForSym(symScope, scope, &envIndex, funcInfo);
        if (scope == this->globalScope)
        {
            scopeLocation = ByteCodeGenerator::RootObjectRegister;
        }
        else if (envIndex != -1)
        {
            // The scope is in the closure environment, so retrieve it.
            scopeLocation = funcInfo->AcquireTmpRegister();
            this->m_writer.Slot(Js::OpCode::LdSlotArr, scopeLocation, funcInfo->GetEnvRegister(), envIndex + Js::FrameDisplay::GetOffsetOfScopes()/sizeof(Js::Var));
            funcInfo->ReleaseTmpRegister(scopeLocation);
        }
        else
        {
            Assert(funcInfo == scope->GetFunc());
            scopeLocation = scope->GetLocation();
        }

        if (scope == symScope)
        {
            break;
        }

        // Found a scope to which the property may have been added.
        Assert(scope && scope->GetIsDynamic());

        if (!fLabelDefined)
        {
            fLabelDefined = true;
            doneLabel = this->m_writer.DefineLabel();
        }

        Js::ByteCodeLabel nextLabel = this->m_writer.DefineLabel();
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        this->m_writer.BrProperty(Js::OpCode::BrOnNoProperty, nextLabel, scopeLocation,
            funcInfo->FindOrAddReferencedPropertyId(propertyId));

        this->EmitTypeOfFld(funcInfo, propertyId, lhsLocation, scopeLocation, Js::OpCode::LdFldForTypeOf);

        this->m_writer.Br(doneLabel);
        this->m_writer.MarkLabel(nextLabel);
    }

    // Arrived at the scope in which the property was defined.
    if (sym && sym->GetNeedDeclaration() && scope->GetFunc() == funcInfo)
    {
        // Ensure this symbol has a slot if it needs one.
        if(sym->IsInSlot(funcInfo))
        {
            Js::PropertyId slot = sym->EnsureScopeSlot(funcInfo);
            funcInfo->FindOrAddSlotProfileId(scope, slot);
        }

        EmitUseBeforeDeclarationRuntimeError(this, lhsLocation);
    }
    else if (sym == null || sym->GetIsGlobal())
    {
        Js::PropertyId propertyId = sym ? sym->EnsurePosition(this) : pid->GetPropertyId();
        if (this->flags & fscrEval)
        {
            if (funcInfo->byteCodeFunction->GetIsStrictMode() && funcInfo->IsGlobalFunction())
            {
                this->EmitTypeOfFld(funcInfo, propertyId, lhsLocation, funcInfo->frameDisplayRegister, Js::OpCode::ScopedLdFldForTypeOf);
            }
            else
            {
                this->EmitTypeOfFld(funcInfo, propertyId, lhsLocation, funcInfo->GetEnvRegister(), Js::OpCode::ScopedLdFldForTypeOf);
            }
        }
        else if (this->flags & (fscrImplicitThis | fscrImplicitParents))
        {
            this->EmitTypeOfFld(funcInfo, propertyId, lhsLocation, funcInfo->GetEnvRegister(), Js::OpCode::ScopedLdFldForTypeOf);
        }
        else
        {
            this->EmitTypeOfFld(funcInfo, propertyId, lhsLocation, ByteCodeGenerator::RootObjectRegister, Js::OpCode::LdRootFldForTypeOf);
        }
    }
    else if (sym->IsInSlot(funcInfo) || envIndex != -1)
    {
        // Make sure the property has a slot. This will bump up the size of the slot array if necessary.
        Js::PropertyId slot = sym->EnsureScopeSlot(funcInfo);
        Js::ProfileId profileId = funcInfo->FindOrAddSlotProfileId(scope, slot);
        Js::RegSlot tmpLocation = funcInfo->AcquireTmpRegister();
        bool chkBlockVar = NeedCheckBlockVar(sym, scope, funcInfo);
        Js::OpCode op;

        // The property is in memory rather than register. We'll have to load it from the slots.
        if (scope->GetIsDynamic() || (scope->GetIsObject() && scope->IsBlockScope(funcInfo)))
        {
            // The scope is an object whose slots are subject to change. First load them from the object.
            op = chkBlockVar ? Js::OpCode::LdObjSlotChkUndecl : Js::OpCode::LdObjSlot;
        }
        else
        {
            // Now get the property from its slot.
            op = chkBlockVar ? Js::OpCode::LdSlotChkUndecl : Js::OpCode::LdSlot;
            slot = slot + (sym->GetScope()->GetIsObject()? 0 : Js::ScopeSlots::FirstSlotIndex);

            if (scopeLocation == funcInfo->frameSlotsRegister &&
                funcInfo->GetParsedFunctionBody()->DoStackScopeSlots() &&
                !this->inPrologue)
            {
                // The function allocated local slots on the stack, but they may since have been boxed, so
                // re-load them before accessing them.
                scopeLocation = funcInfo->AcquireTmpRegister();
                this->m_writer.Reg1(Js::OpCode::LdLocalScopeSlots, scopeLocation);
                funcInfo->ReleaseTmpRegister(scopeLocation);
            }
        }
        this->m_writer.Slot(op, tmpLocation, scopeLocation, slot, profileId);

        this->m_writer.Reg2(Js::OpCode::Typeof, lhsLocation, tmpLocation);
        funcInfo->ReleaseTmpRegister(tmpLocation);
    }
    else
    {
        this->m_writer.Reg2(Js::OpCode::Typeof, lhsLocation, sym->GetLocation());
    }

    if (fLabelDefined)
    {
        this->m_writer.MarkLabel(doneLabel);
    }
}

void ByteCodeGenerator::Generate( __in ParseNode *pnode, ulong grfscr, __in ByteCodeGenerator* byteCodeGenerator,
    __inout Js::ParseableFunctionInfo ** ppRootFunc, __in uint sourceIndex,
    __in bool forceNoNative, __in Parser* parser, Js::ScriptFunction **functionRef)
{
#if DBG
    AssertMsg(!byteCodeGenerator->executingGenerate, "ByteCodeGenerator::Generate is not reentrant safe");
    AutoBooleanToggle restoreExecutingGenerate(&byteCodeGenerator->executingGenerate);
#endif

    Js::ScriptContext * scriptContext = byteCodeGenerator->scriptContext;

#ifdef PROFILE_EXEC
    scriptContext->ProfileBegin(Js::ByteCodePhase);
#endif
#ifdef LANGUAGE_SERVICE
    if (scriptContext->authoringData && scriptContext->authoringData->Callbacks())
        scriptContext->authoringData->Callbacks()->GeneratingByteCode();
#endif
    JSETW(EventWriteJSCRIPT_BYTECODEGEN_START(scriptContext,0));

    ThreadContext * threadContext = scriptContext->GetThreadContext();
    Js::Utf8SourceInfo * utf8SourceInfo = scriptContext->GetSource(sourceIndex);
    byteCodeGenerator->m_utf8SourceInfo = utf8SourceInfo;

    // For dynamic code, just provide a small number since that source info should have very few functions
    // For static code, the nextLocalFunctionId is a good guess of the initial size of the array to minimize reallocs
    SourceContextInfo * sourceContextInfo = utf8SourceInfo->GetSrcInfo()->sourceContextInfo;
    utf8SourceInfo->EnsureInitialized((grfscr & fscrDynamicCode) ? 4 : (sourceContextInfo->nextLocalFunctionId - pnode->sxFnc.functionId));
    sourceContextInfo->EnsureInitialized();

    ArenaAllocator localAlloc(L"ByteCode", threadContext->GetPageAllocator(), Js::Throw::OutOfMemory);
    byteCodeGenerator->parser=parser;
    byteCodeGenerator->SetCurrentSourceIndex(sourceIndex);
    byteCodeGenerator->Begin(&localAlloc, grfscr, *ppRootFunc);
    byteCodeGenerator->callsConstructor=false;
    byteCodeGenerator->functionRef = functionRef;
    Visit(pnode, byteCodeGenerator, Bind, AssignRegisters);
#if 0
    // TODO: Does anyone use this info? If so, restore it.
    if (byteCodeGenerator->Trace()) {
        byteCodeGenerator->PrintFuncInfo();
    }
#endif

#if defined(ENABLE_DEBUG_CONFIG_OPTIONS)
    Output::Flush();
#endif

    byteCodeGenerator->forceNoNative = forceNoNative;
    byteCodeGenerator->EmitProgram(pnode);

    if (byteCodeGenerator->flags & fscrEval)
    {
        // The eval caller's frame always escapes if eval refers to the caller's arguments.
        byteCodeGenerator->GetRootFunc()->GetFunctionBody()->SetFuncEscapes(
            byteCodeGenerator->funcEscapes || pnode->sxProg.m_UsesArgumentsAtGlobal);
    }

#ifdef IR_VIEWER
    if (grfscr & fscrIrDumpEnable)
    {
        byteCodeGenerator->GetRootFunc()->GetFunctionBody()->SetIRDumpEnabled(true);
    }
#endif /* IR_VIEWER */

    byteCodeGenerator->CheckDeferParseHasMaybeEscapedNestedFunc();

#ifdef PROFILE_EXEC
    scriptContext->ProfileEnd(Js::ByteCodePhase);
#endif
    JSETW(EventWriteJSCRIPT_BYTECODEGEN_STOP(scriptContext,0));

#if defined(ENABLE_NATIVE_CODEGEN) && defined(ENABLE_PREJIT)
    if (!byteCodeGenerator->forceNoNative && !scriptContext->GetConfig()->IsNoNative()
        && Js::Configuration::Global.flags.Prejit
        && (grfscr & fscrNoPreJit)==0)
    {
        GenerateAllFunctions(scriptContext->GetNativeCodeGenerator(), byteCodeGenerator->GetRootFunc()->GetFunctionBody());
    }
#endif

    if (ppRootFunc)
    {
        *ppRootFunc = byteCodeGenerator->GetRootFunc();
    }

#ifdef PERF_COUNTERS
    PHASE_PRINT_TESTTRACE1(Js::DeferParsePhase, L"TestTrace: deferparse - # of func: %d # deferparsed: %d\n", PerfCounter::CodeCounterSet::GetTotalFunctionCounter().GetValue(), PerfCounter::CodeCounterSet::GetDeferedFunctionCounter().GetValue());
#endif
}

void ByteCodeGenerator::CheckDeferParseHasMaybeEscapedNestedFunc()
{
    if (!this->parentScopeInfo)
    {
        return;
    }

    Assert(CONFIG_FLAG(DeferNested));

    Assert(this->funcInfoStack && !this->funcInfoStack->Empty());

    // Box the stack nested function if we detected new may be escaped use function
    SList<FuncInfo *>::Iterator i(this->funcInfoStack);
    bool succeed = i.Next();
    Assert(succeed);
    Assert(i.Data()->IsGlobalFunction());        /// We always leaves a glo on type when defer parsing
    Assert(!i.Data()->IsRestored());
    succeed = i.Next();
    FuncInfo * top = i.Data();

    Assert(!top->IsGlobalFunction());
    Assert(top->IsRestored());
    Js::FunctionBody * rootFuncBody = this->GetRootFunc()->GetFunctionBody();
    if (!rootFuncBody->DoStackNestedFunc())
    {
        top->SetHasMaybeEscapedNestedFunc(DebugOnly(L"DeferredChild"));
    }
    else
    {
        // We have to wait until it is parse before we populate the stack nested func parent
        Js::FunctionBody * parentFunctionBody = null;
        FuncInfo * parentFunc = top->GetBodyScope()->GetEnclosingFunc();
        if (!parentFunc->IsGlobalFunction())
        {
            parentFunctionBody = parentFunc->GetParsedFunctionBody();
            Assert(parentFunctionBody != rootFuncBody);
            if (parentFunctionBody->DoStackNestedFunc())
            {
                rootFuncBody->SetStackNestedFuncParent(parentFunctionBody);
            }
        }
    }

    do
    {
        FuncInfo * funcInfo = i.Data();
        Assert(funcInfo->IsRestored());
        Js::FunctionBody * functionBody = funcInfo->GetParsedFunctionBody();
        bool didStackNestedFunc = functionBody->DoStackNestedFunc();
        if (!didStackNestedFunc)
        {
            return;
        }
        if (funcInfo->HasMaybeEscapedNestedFunc())
        {
            // This should box the rest of the parent functions
            if (PHASE_TESTTRACE(Js::StackFuncPhase, this->pCurrentFunction))
            {
                wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

                Output::Print(L"DeferParse: box and disable stack function: %s (function %s)\n",
                    functionBody->GetDisplayName(), functionBody->GetDebugNumberSet(debugStringBuffer));
                Output::Flush();
            }
            Js::StackScriptFunction::Box(functionBody, functionRef);
            return;
        }
    }
    while (i.Next());
}

void ByteCodeGenerator::EnsureNoRedeclarations(ParseNode *pnodeBlock, FuncInfo *funcInfo)
{
    // Emit dynamic runtime checks for variable redeclarations. Only necessary for global functions (script or eval).
    // In eval only var declarations can cause redeclaration, and only in non-strict mode, because let/const variables
    // remain local to the eval code.

    Assert(pnodeBlock->nop == knopBlock);
    Assert(pnodeBlock->sxBlock.blockType == PnodeBlockType::Global || pnodeBlock->sxBlock.scope->GetScopeType() == ScopeType_GlobalEvalBlock);

    if (!(this->flags & fscrEvalCode))
    {
        IterateBlockScopedVariables(pnodeBlock, [this](ParseNode *pnode)
        {
            FuncInfo *funcInfo = this->TopFuncInfo();
            Symbol *sym = pnode->sxVar.sym;

            Assert(sym->GetIsGlobal());

            Js::PropertyId propertyId = sym->EnsurePosition(this);

            this->m_writer.ElementRootU(Js::OpCode::EnsureNoRootFld, funcInfo->FindOrAddReferencedPropertyId(propertyId));
        });
    }

    for (ParseNode *pnode = funcInfo->root->sxFnc.pnodeVars; pnode; pnode = pnode->sxVar.pnodeNext)
    {
        Symbol* sym = pnode->sxVar.sym;

        if (sym == null || pnode->sxVar.isBlockScopeFncDeclVar)
            continue;

        if (sym->GetIsCatch() || (pnode->nop == knopVarDecl && sym->GetIsBlockVar()))
        {
            // The init node was bound to the catch object, because it's inside a catch and has the
            // same name as the catch object. But we want to define a user var at function scope,
            // so find the right symbol. (We'll still assign the RHS value to the catch object symbol.)
            // This also applies to a var declaration in the same scope as a let declaration.
            Assert(sym->GetIsCatch() || funcInfo->bodyScope != sym->GetScope() || !this->scriptContext->GetConfig()->IsBlockScopeEnabled());  // catch cannot be at function scope and let and var at function scope is redeclaration error
            sym = funcInfo->bodyScope->FindLocalSymbol(sym->GetName());
            Assert(sym && !sym->GetIsCatch() && !sym->GetIsBlockVar());
        }

        Assert(sym->GetIsGlobal());

        if (sym->GetSymbolType() == STVariable)
        {
            Js::PropertyId propertyId = sym->EnsurePosition(this);

            if (this->flags & fscrEval)
            {
                if (!funcInfo->byteCodeFunction->GetIsStrictMode())
                {
                    this->m_writer.Property(Js::OpCode::ScopedEnsureNoRedeclFld, ByteCodeGenerator::RootObjectRegister, funcInfo->GetEnvRegister(),
                        funcInfo->FindOrAddReferencedPropertyId(propertyId));
                }
            }
            else
            {
                this->m_writer.ElementRootU(Js::OpCode::EnsureNoRootRedeclFld, funcInfo->FindOrAddReferencedPropertyId(propertyId));
            }
        }
    }
}

// Emitting
void ByteCodeGenerator::Begin(
    __in ArenaAllocator *alloc,
    __in ulong grfscr,
    __in Js::ParseableFunctionInfo* pRootFunc)
{
    this->alloc = alloc;
    this->flags = grfscr;
    this->pRootFunc = pRootFunc;
    this->pCurrentFunction = pRootFunc? pRootFunc->GetFunctionBody() : null;
    if (this->pCurrentFunction && this->pCurrentFunction->GetIsGlobalFunc() && IsInNonDebugMode())
    {
        // This is the deferred parse case (not due to debug mode), in which case the global function will not be marked to compiled again.
        this->pCurrentFunction = NULL;
    }

    this->globalScope = null;
    this->currentScope = null;
    this->currentBlock = null;
    this->isBinding = true;
    this->inPrologue = false;
    this->funcEscapes = false;
    this->maxAstSize = 0;
    this->loopDepth = 0;
    this->envDepth = 0;
    this->trackEnvDepth = false;
    this->visitIndirectDepth = 0;

    this->funcInfoStack = Anew(alloc, SList<FuncInfo*>, alloc);

    // If pRootFunc is not null, this is a deferred parse function
    // so reuse the property record list bound there since some of the symbols could have
    // been bound. If it's null, we need to create a new property record list
    if (pRootFunc != null)
    {
        this->propertyRecords = pRootFunc->GetBoundPropertyRecords();
    }
    else
    {
        this->propertyRecords = null;
    }
}

void ByteCodeGenerator::RecordAllIntConstants(FuncInfo * funcInfo)
{
    Js::FunctionBody *byteCodeFunction = this->TopFuncInfo()->GetParsedFunctionBody();
    funcInfo->constantToRegister.Map([byteCodeFunction](unsigned int val, Js::RegSlot location)
    {
        byteCodeFunction->RecordIntConstant(byteCodeFunction->MapRegSlot(location), val);
    });
}

void ByteCodeGenerator::RecordAllStrConstants(FuncInfo * funcInfo)
{
    Js::FunctionBody *byteCodeFunction = this->TopFuncInfo()->GetParsedFunctionBody();
    funcInfo->stringToRegister.Map([byteCodeFunction](IdentPtr pid, Js::RegSlot location)
    {
        byteCodeFunction->RecordStrConstant(byteCodeFunction->MapRegSlot(location), pid->Psz(), pid->Cch());
    });
}

void ByteCodeGenerator::RecordAllStringTemplateCallsiteConstants(FuncInfo* funcInfo)
{
    Js::FunctionBody *byteCodeFunction = this->TopFuncInfo()->GetParsedFunctionBody();
    funcInfo->stringTemplateCallsiteRegisterMap.Map([byteCodeFunction](ParseNodePtr pnode, Js::RegSlot location)
    {
        Js::ScriptContext* scriptContext = byteCodeFunction->GetScriptContext();
        Js::JavascriptLibrary* library = scriptContext->GetLibrary();
        Js::RecyclableObject* callsiteObject = library->TryGetStringTemplateCallsiteObject(pnode);

        if (callsiteObject == nullptr)
        {
            Js::RecyclableObject* rawArray = ByteCodeGenerator::BuildArrayFromStringList(pnode->sxStrTemplate.pnodeStringRawLiterals, pnode->sxStrTemplate.countStringLiterals, scriptContext);
            rawArray->Freeze();

            callsiteObject = ByteCodeGenerator::BuildArrayFromStringList(pnode->sxStrTemplate.pnodeStringLiterals, pnode->sxStrTemplate.countStringLiterals, scriptContext);
            callsiteObject->SetPropertyWithAttributes(Js::PropertyIds::raw, rawArray, PropertyNone, nullptr);
            callsiteObject->Freeze();

            library->AddStringTemplateCallsiteObject(callsiteObject);
        }

        byteCodeFunction->RecordConstant(byteCodeFunction->MapRegSlot(location), callsiteObject);
    });
}

extern unsigned __int64 count;

HRESULT GenerateByteCode(__in ParseNode *pnode, __in ulong grfscr, __in Js::ScriptContext* scriptContext, __inout Js::ParseableFunctionInfo ** ppRootFunc,
                         __in uint sourceIndex, __in bool forceNoNative, __in Parser* parser, __in CompileScriptException *pse, Js::ScopeInfo* parentScopeInfo,
                        Js::ScriptFunction ** functionRef)
{
    HRESULT hr = S_OK;
    ByteCodeGenerator byteCodeGenerator(scriptContext, parentScopeInfo);
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT_NESTED
    {
        // Main code.
        ByteCodeGenerator::Generate(pnode, grfscr, &byteCodeGenerator, ppRootFunc, sourceIndex, forceNoNative, parser, functionRef);
    }
    END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);

    if (FAILED(hr))
    {
        hr = pse->ProcessError(NULL, hr, NULL);
    }

    return hr;
}

void BindInstAndMember(ParseNode *pnode,ByteCodeGenerator *byteCodeGenerator)
{
    Assert(pnode->nop == knopDot || pnode->nop == knopScope);

    BindReference(pnode, byteCodeGenerator);

    ParseNode *right=pnode->sxBin.pnode2;
    Assert(right->nop==knopName);
    byteCodeGenerator->AssignPropertyId(right->sxPid.pid);
    right->sxPid.sym = NULL;
    right->sxPid.symRef = NULL;
    right->grfpn |= fpnMemberReference;
}

void BindReference(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    // Do special reference-op binding so that we can, for instance, handle call from inside "with"
    // where the "this" instance must be found dynamically.

    bool isCallNode = false;
    switch (pnode->nop)
    {
    case knopCall:
        isCallNode = true;
        pnode = pnode->sxCall.pnodeTarget;
        break;
    case knopDelete:
    case knopTypeof:
        pnode = pnode->sxUni.pnode1;
        break;
    case knopAsg:
    case knopDot:
    case knopIndex:
    case knopScope:
        pnode = pnode->sxBin.pnode1;
        break;
    default:
        AssertMsg(0, "Unexpected opcode in BindReference");
        return;
    }

    if (pnode->nop == knopName)
    {
        pnode->sxPid.sym = byteCodeGenerator->FindSymbol(pnode->sxPid.symRef, pnode->sxPid.pid, isCallNode);
    }
}

void MarkFormal(ByteCodeGenerator *byteCodeGenerator, Symbol *formal, bool assignLocation, bool needDeclaration)
{
    if (assignLocation)
    {
        formal->SetLocation(byteCodeGenerator->NextVarRegister());
    }
    if (needDeclaration)
    {
        formal->SetNeedDeclaration(true);
    }
}

void AddArgsToScope(ParseNodePtr pnode, ByteCodeGenerator *byteCodeGenerator, bool assignLocation, bool isSimpleParameterList = false)
{
    Assert(byteCodeGenerator->TopFuncInfo()->varRegsCount == 0);
    Js::ArgSlot pos = 1;
    auto addArgToScope = [&](ParseNode *vars)
    {
        Symbol *formal = byteCodeGenerator->AddSymbolToScope(byteCodeGenerator->TopFuncInfo()->GetParamScope(),
                                                             reinterpret_cast<const wchar_t*>(vars->sxVar.pid->Psz()),
                                                             vars->sxVar.pid->Cch(),
                                                             vars,
                                                             STFormal);
        if (byteCodeGenerator->Trace())
        {
            Output::Print(L"current context has declared arg %s of type %s at position %d\n", vars->sxVar.pid->Psz(), formal->GetSymbolTypeName(), pos);
        }

        if (!pnode->sxFnc.IsSimpleParameterList())
        {
            formal->SetIsNonSimpleParameter(true);
        }

        vars->sxVar.sym = formal;
        MarkFormal(byteCodeGenerator, formal, assignLocation || !pnode->sxFnc.IsSimpleParameterList(), !pnode->sxFnc.IsSimpleParameterList());

        UInt16Math::Inc(pos);
    };

    // We process rest separately because the number of in args needs to exclude rest.
    MapFormalsWithoutRest(pnode, addArgToScope);

    byteCodeGenerator->SetNumberOfInArgs(pos);

    if (pnode->sxFnc.pnodeRest != nullptr)
    {
        // The rest parameter will always be in a register, regardless of whether it is in a scope slot.
        // We save the assignLocation value for the assert condition below.
        bool assignLocationSave = assignLocation;
        assignLocation = true;

        addArgToScope(pnode->sxFnc.pnodeRest);

        assignLocation = assignLocationSave;
    }

    Assert(!assignLocation || byteCodeGenerator->TopFuncInfo()->varRegsCount + 1 == pos);
}

void AddVarsToScope(ParseNode *vars, ByteCodeGenerator *byteCodeGenerator) {
    while (vars != nullptr)
    {
        Symbol *sym = nullptr;

        if (!byteCodeGenerator->UseParserBindings()
            && byteCodeGenerator->GetCurrentScope()->GetFunc()->GetParamScope() != nullptr)
        {
            SymbolName const symName(reinterpret_cast<const wchar_t*>(vars->sxVar.pid->Psz()), vars->sxVar.pid->Cch());
            // If we are not using parser bindings, we need to check that the sym is not in the parameter scope before adding it.
            // Fetch the sym we just added from the param scope.
            sym = byteCodeGenerator->GetCurrentScope()->GetFunc()->GetParamScope()->FindLocalSymbol(symName);

            // Arguments needs to be created at parameter scope.
            if (sym == nullptr && vars->grfpn & PNodeFlags::fpnArguments)
            {
                Scope* scope = byteCodeGenerator->GetCurrentScope()->GetFunc()->GetParamScope();
                sym = byteCodeGenerator->AddSymbolToScope(scope, reinterpret_cast<const wchar_t*>(vars->sxVar.pid->Psz()), vars->sxVar.pid->Cch(), vars, STVariable);
            }
        }

        if (sym == nullptr)
        {
            sym = byteCodeGenerator->AddSymbolToFunctionScope(reinterpret_cast<const wchar_t*>(vars->sxVar.pid->Psz()), vars->sxVar.pid->Cch(), vars, STVariable);
        }

        if (sym->GetSymbolType() == STVariable && byteCodeGenerator->Trace()) {
            Output::Print(L"current context has declared var %s of type %s\n",
                vars->sxVar.pid->Psz(), sym->GetSymbolTypeName());
        }

        if (sym->GetIsArguments() || vars->sxVar.pnodeInit == nullptr)
        {
            // LHS's of var decls are usually bound to symbols later, during the Visit/Bind pass,
            // so that things like catch scopes can be taken into account.
            // The exception is "arguments", which always binds to the local scope.
            // We can also cheat and bind to the function scope symbol now if there's no init value
            // to assign.
            vars->sxVar.sym = sym;
            if (sym->GetIsArguments())
            {
                byteCodeGenerator->TopFuncInfo()->SetArgumentsSymbol(sym);
            }
        }
        else
        {
            vars->sxVar.sym = nullptr;
        }
        vars = vars->sxVar.pnodeNext;
    }
}

template <class Fn>
void VisitFncDecls(ParseNode *fns, Fn action)
{
    while (fns != NULL)
    {
        switch (fns->nop)
        {
        case knopFncDecl:
            // This would add symbol for this func node. Which gets added to parent scope in following condition
            // Function has a name and version mode is compat version 8 OR
            // Function has a name, version is ES5 and function is function declaration
            VisitFncNames(fns, action);
            fns = fns->sxFnc.pnodeNext;
            break;

        case knopBlock:
            fns = fns->sxBlock.pnodeNext;
            break;

        case knopCatch:
            fns = fns->sxCatch.pnodeNext;
            break;

        case knopWith:
            fns = fns->sxWith.pnodeNext;
            break;

        default:
            AssertMsg(false, "Unexpected opcode in tree of scopes");
            return;
        }
    }
}

FuncInfo* PreVisitFunction(ParseNode* pnode, ByteCodeGenerator* byteCodeGenerator)
{
    // Do binding of function name(s), initialize function scope, propagate function-wide properties from
    // the parent (if any).
    FuncInfo* parentFunc = byteCodeGenerator->TopFuncInfo();

    // fIsRoot indicates that this is the root function to be returned to a ParseProcedureText/AddScriptLet/etc. call.
    // In such cases, the global function is just a wrapper around the root function's declaration.
    // We used to assert that this was the only top-level function body, but it's possible to trick
    // "new Function" into compiling more than one function (see WOOB 1121759).
    BOOL fIsRoot = (!(byteCodeGenerator->GetFlags() & fscrGlobalCode) &&
        parentFunc->IsGlobalFunction() &&
        parentFunc->root->sxFnc.GetTopLevelScope() == pnode);

    const wchar_t *funcName = Js::Constants::AnonymousFunction;
    int funcNameLength = 18;
    bool funcExprWithName = false;

    if (pnode->sxFnc.hint != NULL)
    {
        funcName = reinterpret_cast<const wchar_t*>(pnode->sxFnc.hint);
        funcNameLength = pnode->sxFnc.hintLength;
        Assert(funcNameLength != 0 || funcNameLength == (int)wcslen(funcName));
    }
    if (pnode->sxFnc.IsDeclaration() || pnode->sxFnc.IsMethod())
    {
        if (pnode->sxFnc.pnodeNames != NULL &&
            (pnode->sxFnc.pnodeNames->nop == knopDot ||
            pnode->sxFnc.pnodeNames->nop == knopScope))
        {
            // TODO: ES5 doesn't allow this, need to version this.
            // Assert(byteCodeGenerator->GetScriptContext()->GetConfig()->IsCompatVersion8());
        }
        // Class members have the fully qualified name stored in 'hint', no need to replace it.
        else if (pnode->sxFnc.pid && !pnode->sxFnc.IsClassMember())
        {
            funcName=reinterpret_cast<const wchar_t*>(pnode->sxFnc.pid->Psz());
            funcNameLength = pnode->sxFnc.pid->Cch();
        }
    }
    else if ((pnode->sxFnc.pnodeNames!=NULL) &&
             (pnode->sxFnc.pnodeNames->nop==knopVarDecl))
    {
        funcName=reinterpret_cast<const wchar_t*>(pnode->sxFnc.pnodeNames->sxVar.pid->Psz());
        funcNameLength = pnode->sxFnc.pnodeNames->sxVar.pid->Cch();

        //
        // create the new scope for Function expression only in ES5 mode
        //
        funcExprWithName = true;
    }

    if (byteCodeGenerator->Trace()) {
        Output::Print(L"function start %s\n",funcName);
    }

    Assert(byteCodeGenerator->IsLanguageServiceEnabled() || pnode->sxFnc.funcInfo == nullptr);

    FuncInfo* funcInfo = pnode->sxFnc.funcInfo = byteCodeGenerator->StartBindFunction(funcName, funcNameLength, &funcExprWithName, pnode);
    funcInfo->byteCodeFunction->SetIsNamedFunctionExpression(funcExprWithName);
    funcInfo->byteCodeFunction->SetIsNameIdentifierRef (pnode->sxFnc.isNameIdentifierRef);
    if (fIsRoot)
    {
        byteCodeGenerator->SetRootFuncInfo(funcInfo);
    }

    if (pnode->sxFnc.pnodeBody == NULL)
    {
        // This is a deferred byte code gen, so we're done.
        // Process the formal arguments, even if there's no AST for the body, to support Function.length.
        if (byteCodeGenerator->UseParserBindings())
        {
            Js::ArgSlot pos = 1;
            // We skip the rest parameter here because it is not counted towards the in arg count.
            MapFormalsWithoutRest(pnode, [&](ParseNode *pnode) { UInt16Math::Inc(pos); });
            byteCodeGenerator->SetNumberOfInArgs(pos);
        }
        else
        {
            AddArgsToScope(pnode, byteCodeGenerator, false, pnode->sxFnc.IsSimpleParameterList());
        }
        return funcInfo;
    }

    if(pnode->sxFnc.HasReferenceableBuiltInArguments())
    {
        // The parser identified that there is a way to reference the built-in 'arguments' variable from this function. So, we
        // need to determine whether we need to create the variable or not. We need to create the variable iff:
        if (pnode->sxFnc.CallsEval() || byteCodeGenerator->IsLanguageServiceEnabled())
        {
            // 1. eval is called.
            // 2. when the debugging or language service is enabled, since user can seek arguments during breakpoint.
            funcInfo->SetHasArguments(true);
            funcInfo->SetHasHeapArguments(true);
            if (funcInfo->inArgsCount == 0)
            {
                // If no formals to function, no need to create the propertyid array
                byteCodeGenerator->AssignNullConstRegister();
            }
        }
        else if (pnode->sxFnc.UsesArguments())
        {
            // 3. the function directly references an 'arguments' identifier
            funcInfo->SetHasArguments(true);
            if (pnode->sxFnc.HasHeapArguments())
            {
                funcInfo->SetHasHeapArguments(true, true /*= Optimize arguments in backend*/);
                if (funcInfo->inArgsCount == 0)
                {
                    // If no formals to function, no need to create the propertyid array
                    byteCodeGenerator->AssignNullConstRegister();
                }
            }
        }
    }

    Js::FunctionBody* parentFunctionBody = parentFunc->GetParsedFunctionBody();
    if (funcInfo->GetHasArguments() ||
        parentFunctionBody->GetHasOrParentHasArguments())
    {
        // The JIT uses this info, for instance, to narrow kills of array operations
        funcInfo->GetParsedFunctionBody()->SetHasOrParentHasArguments(true);
    }
    PreVisitBlock(pnode->sxFnc.pnodeScopes, byteCodeGenerator);
    // If we have arguments, we are going to need locations if the function is in strict mode or we have a non-simple parameter list. This is because we will not create a scope object.
    bool assignLocationForFormals = !(funcInfo->GetHasHeapArguments() && ByteCodeGenerator::NeedScopeObjectForArguments(funcInfo, funcInfo->root));
    AddArgsToScope(pnode, byteCodeGenerator, assignLocationForFormals, pnode->sxFnc.IsSimpleParameterList());
    PreVisitBlock(pnode->sxFnc.pnodeBodyScope, byteCodeGenerator);
    AddVarsToScope(pnode->sxFnc.pnodeVars, byteCodeGenerator);

    return funcInfo;
}

void AssignFuncSymRegister(ParseNode * pnode, ByteCodeGenerator * byteCodeGenerator, FuncInfo * callee)
{
    // register to hold the allocated function (in enclosing sequence of global statements)
    // TODO: Make the parser identify uses of function decls as RHS's of expressions.
    // Currently they're all marked as used, so they all get permanent (non-temp) registers.
    VisitFncNames(pnode, [byteCodeGenerator, callee](ParseNode *pnode, ParseNode *pnodeName)
    {
        if (pnodeName == NULL || pnodeName->nop != knopVarDecl)
        {
            return;
        }
        Symbol *sym = pnodeName->sxVar.sym;
        if (sym)
        {
            if (!sym->GetIsGlobal() && !(callee->funcExprScope && callee->funcExprScope->GetIsObject()))
            {
                // If the func decl is used, we have to give the expression a register to protect against:
                // x.x = function f() {...};
                // x.y = function f() {...};
                // If we let the value reside in the local slot for f, then both assignments will get the
                // second definition.
                if (!pnode->sxFnc.IsDeclaration())
                {
                    // A named function expression's name belongs to the enclosing scope much like a function
                    // declaration in IE8 compat mode. In ES5 mode, it is visible only inside the inner function.
                    // Allocate a register for the 'name' symbol from an appropriate register namespace.
                    if (callee->GetFuncExprNameReference())
                    {
                        // This is a function expression with a name, but probably doesn't have a use within
                        // the function. If that is the case then allocate a register for LdFuncExpr inside the function
                        // we just finished post-visiting.
                        if (sym->GetLocation() == Js::Constants::NoRegister)
                        {
                            sym->SetLocation(callee->NextVarRegister());
                        }
                    }
                }
                else
                {
                    // Function declaration
                    byteCodeGenerator->AssignRegister(sym);
                    pnode->location = sym->GetLocation();

                    Assert(byteCodeGenerator->GetCurrentScope()->GetFunc() == sym->GetScope()->GetFunc());
                    Symbol * functionScopeVarSym = sym->GetFuncScopeVarSym();
                    if (functionScopeVarSym &&
                        !functionScopeVarSym->GetIsGlobal() &&
                        !functionScopeVarSym->IsInSlot(sym->GetScope()->GetFunc()))
                    {
                        Assert(byteCodeGenerator->GetScriptContext()->GetConfig()->IsBlockScopeEnabled());
                        byteCodeGenerator->AssignRegister(functionScopeVarSym);
                    }
                }
            }
            else if (!pnode->sxFnc.IsDeclaration())
            {
                if (sym->GetLocation() == Js::Constants::NoRegister)
                {
                    // Here, we are assigning a register for the LdFuncExpr instruction inside the function we just finished
                    // post-visiting. The symbol is given
                    // a register from the register pool for the function we just finished post-visiting, rather than from the
                    // parent function's register pool.
                    sym->SetLocation(callee->NextVarRegister());
                }
            }
        }
    });
}

FuncInfo* PostVisitFunction(ParseNode* pnode, ByteCodeGenerator* byteCodeGenerator)
{
    // Assign function-wide registers such as local frame object, closure environment, etc., based on
    // observed attributes. Propagate attributes to the parent function (if any).
    FuncInfo *top = byteCodeGenerator->TopFuncInfo();
    Symbol *sym = pnode->sxFnc.GetFuncSymbol();
    bool funcExprWithName = !top->IsGlobalFunction() && sym && sym->GetFuncExpr();

    if (top->IsLambda())
    {
        if (byteCodeGenerator->FindEnclosingNonLambda()->isThisLexicallyCaptured)
        {
            top->byteCodeFunction->SetCapturesThis();
        }
    }

    // If this is a named function expression and has deferred child, mark has non-local reference.
    if (funcExprWithName)
    {
        // If we are reparsing this fuction due to being in debug mode - we should restore the state of this from the earlier parse
        if(top->byteCodeFunction->IsFunctionParsed() && top->GetParsedFunctionBody()->HasFuncExprNameReference())
        {
            top->SetFuncExprNameReference(true);
            sym->SetHasNonLocalReference(true, byteCodeGenerator);
            top->SetHasLocalInClosure(true);
        }
        else if(top->HasDeferredChild())
        {
            // Before doing this, though, make sure there's no local symbol that hides the function name
            // from the nested functions. If a lookup starting at the current local scope finds some symbol
            // other than the func expr, then it's hidden. (See Win8 393618.)
            Assert(CONFIG_FLAG(DeferNested));
            sym->SetHasNonLocalReference(true, byteCodeGenerator);
            top->SetHasLocalInClosure(true);
            if (pnode->sxFnc.pnodeBody)
            {
                top->GetParsedFunctionBody()->SetAllNonLocalReferenced(true);
            }

            if (!top->root->sxFnc.NameIsHidden())
            {
                top->SetFuncExprNameReference(true);
                if (pnode->sxFnc.pnodeBody)
                {
                    top->GetParsedFunctionBody()->SetFuncExprNameReference(true);
                }

                top->GetBodyScope()->SetIsObject();
                top->GetParamScope()->SetIsObject();
                if (pnode->sxFnc.pnodeBody)
                {
                    top->GetParsedFunctionBody()->SetHasSetIsObject(true);
                }
            }
        }
    }

    if (pnode->nop != knopProg
        && !top->bodyScope->GetIsObject()
        && byteCodeGenerator->NeedObjectAsFunctionScope(top, pnode))
    {
        // Even if it wasn't determined during visiting this function that we need a scope object, we still have a few conditions that may require one.
        top->bodyScope->SetIsObject();
    }

    if (pnode->nop == knopProg
        && top->byteCodeFunction->GetIsStrictMode()
        && (byteCodeGenerator->GetFlags() & fscrEval))
    {
        // At global scope inside a strict mode eval, vars will not leak out and require a scope object (along with its parent.)
        top->bodyScope->SetIsObject();
    }

    if (pnode->sxFnc.pnodeBody)
    {
        if (!top->IsGlobalFunction())
        {
            PostVisitBlock(pnode->sxFnc.pnodeBodyScope, byteCodeGenerator);
            PostVisitBlock(pnode->sxFnc.pnodeScopes, byteCodeGenerator);
        }

#if FALSE // Disable Collection match optimization.
#if defined(_M_IX86)
        if (!(top->GetCallsEval()||top->GetHasClosureReference() || top->GetHasHeapArguments())) {
            CollectionMatchContext matchContext;
            matchContext.notImplemented=false;
            if (IsCollectionMatch(pnode->sxFnc.pnodeBody,byteCodeGenerator,&matchContext)) {
                ReplaceCollectionMatchBod(pnode->sxFnc.pnodeBody,byteCodeGenerator,&matchContext);
            }
        }
#endif
#endif
        if ((byteCodeGenerator->GetFlags() & fscrEvalCode) && top->GetCallsEval())
        {
            // Must establish "this" in case nested eval refers to it.
            top->GetParsedFunctionBody()->SetHasThis(true);
        }

        // This function refers to the closure environment if:
        // 1. it has a child function (we'll pass the environment to the constructor when the child is created -
        //      even if it's not needed, it's as cheap as loading "null" from the library);
        // 2. it calls eval (and will use the environment to construct the scope chain to pass to eval);
        // 3. it refers to a local defined in a parent function;
        // 4. it refers to a global and some parent calls eval (which might declare the "global" locally);
        // 5. it refers to a global and we're in an event handler;
        // 6. it refers to a global and the function was declared inside a "with";
        // 7. it refers to a global and we're in an eval expression.
        if (pnode->sxFnc.nestedCount != 0 ||
            top->GetCallsEval() ||
            top->GetHasClosureReference() ||
            ((top->GetHasGlobalRef() &&
            (byteCodeGenerator->InDynamicScope() ||
            (byteCodeGenerator->GetFlags() & (fscrImplicitThis | fscrImplicitParents | fscrEval))))))
        {
            byteCodeGenerator->SetNeedEnvRegister();
            if (top->GetIsEventHandler())
            {
                byteCodeGenerator->AssignThisRegister();
            }
        }

        // This function needs to construct a local frame on the heap if it is not the global function (even in eval) and:
        // 1. it calls eval, which may refer to or declare any locals in this frame;
        // 2. a child calls eval (which may refer to locals through a closure);
        // 3. it uses non-strict mode "arguments", so the arguments have to be put in a closure;
        // 4. it defines a local that is used by a child function (read from a closure).
        // 5. it is a main function that's wrapped in a function expression scope but has locals used through
        //    a closure (used in forReference function call cases in a with for example).
        if (!top->IsGlobalFunction())
        {
            if (top->GetCallsEval() ||
                top->GetChildCallsEval() ||
                (top->GetHasArguments() && ByteCodeGenerator::NeedScopeObjectForArguments(top, pnode) && pnode->sxFnc.pnodeArgs != nullptr) ||
                top->GetHasLocalInClosure() ||
                top->funcExprScope && top->funcExprScope->GetMustInstantiate())
            {
                if (!top->GetCallsEval())
                {
                    byteCodeGenerator->AssignFrameSlotsRegister();
                }

                if (byteCodeGenerator->NeedObjectAsFunctionScope(top, top->root)
                    || top->bodyScope->GetIsObject()
                    || top->paramScope->GetIsObject())
                {
                    byteCodeGenerator->AssignFrameObjRegister();
                }

                // The function also needs to construct a frame display if:
                // 1. it calls eval;
                // 2. it has a child function.
                // 3. When has arguments and in debug mode. So that frame display be there along with frameobject register.
                if (top->GetCallsEval() ||
                    pnode->sxFnc.nestedCount != 0
                    || (top->GetHasArguments()
                        && (pnode->sxFnc.pnodeArgs != nullptr)
                        && byteCodeGenerator->IsInDebugMode()))
                {
                    byteCodeGenerator->SetNeedEnvRegister();          // This to ensure that Env should be there when the FrameDisplay register is there.
                    byteCodeGenerator->AssignFrameDisplayRegister();

                    if (top->GetIsEventHandler())
                    {
                        byteCodeGenerator->AssignThisRegister();
                    }
                }
            }

            if (top->GetHasArguments())
            {
                Symbol *argSym = top->GetArgumentsSymbol();
                Assert(argSym);
                if(argSym)
                {
                    Assert(top->bodyScope->GetScopeSlotCount() == 0);
                    Assert(top->sameNameArgsPlaceHolderSlotCount == 0);
                    byteCodeGenerator->AssignRegister(argSym);
                    uint i = 0;
                    auto setArgScopeSlot = [&](ParseNode *pnodeArg)
                    {
                        Symbol* sym = pnodeArg->sxVar.sym;
                        if (sym->GetScopeSlot() != Js::Constants::NoProperty)
                        {
                            top->sameNameArgsPlaceHolderSlotCount++; // Same name args appeared before
                        }
                        sym->SetScopeSlot(i);
                        i++;
                    };

                    // We don't need to process the rest parameter here because it may not need a scope slot.
                    if (ByteCodeGenerator::NeedScopeObjectForArguments(top, pnode))
                    {
                        MapFormalsWithoutRest(pnode, setArgScopeSlot);
                    }

                    top->paramScope->SetScopeSlotCount(i);

                    Assert(top->GetHasHeapArguments());
                    if (ByteCodeGenerator::NeedScopeObjectForArguments(top, pnode)
                        && pnode->sxFnc.IsSimpleParameterList())
                    {
                        top->byteCodeFunction->SetHasImplicitArgIns(false);
                    }
                }
            }
        }
        else
        {
            Assert(top->IsGlobalFunction());
            // eval is called in strict mode
            bool newScopeForEval = (top->byteCodeFunction->GetIsStrictMode() && (byteCodeGenerator->GetFlags() & fscrEval));

            if (newScopeForEval)
            {
                byteCodeGenerator->SetNeedEnvRegister();
                byteCodeGenerator->AssignFrameObjRegister();
                byteCodeGenerator->AssignFrameDisplayRegister();
            }
        }

        Assert(!funcExprWithName || sym);
        if (funcExprWithName)
        {
            Assert(top->funcExprScope);
            // If the func expr may be accessed via eval, force the func expr scope into an object.
            if (top->GetCallsEval() || top->GetChildCallsEval())
            {
                top->funcExprScope->SetIsObject();
            }
            if (top->funcExprScope->GetIsObject())
            {
                top->funcExprScope->SetLocation(byteCodeGenerator->NextVarRegister());
            }
        }
    }

    byteCodeGenerator->EndBindFunction(funcExprWithName);

    // If the "child" is the global function, we're done.
    if (top->IsGlobalFunction())
    {
        return top;
    }

    FuncInfo* const parentFunc = byteCodeGenerator->TopFuncInfo();

    Js::FunctionBody * parentFunctionBody =  parentFunc->byteCodeFunction->GetFunctionBody();
    Assert(parentFunctionBody != NULL);
    bool const hasAnyDeferredChild = top->HasDeferredChild() || top->IsDeferred();
    bool setHasNonLocalReference = parentFunctionBody->HasAllNonLocalReferenced();

    // If we have any deferred child, we need to instantiate the fake global block scope if it is not empty
    if (parentFunc->IsGlobalFunction())
    {
        if (hasAnyDeferredChild && byteCodeGenerator->IsEvalWithBlockScopingNoParentScopeInfo())
        {
            Scope * globalEvalBlockScope = parentFunc->GetGlobalEvalBlockScope();
            if (globalEvalBlockScope->Count() != 0 || parentFunc->isThisLexicallyCaptured)
            {
                // We must instantiate the eval block scope if we have any deferred child
                // and force all symbol to have non local reference so it will be captured
                // by the scope.
                globalEvalBlockScope->SetMustInstantiate(true);
                globalEvalBlockScope->ForceAllSymbolNonLocalReference(byteCodeGenerator);
                parentFunc->SetHasDeferredChild();
            }
        }
    }
    else
    {
        if (setHasNonLocalReference)
        {
            // All locals are already marked as non-locals-referenced. Mark the parent as well.
            if (parentFunctionBody->HasSetIsObject())
            {
                // Updated the current function, as per the previous stored info.
                parentFunc->GetBodyScope()->SetIsObject();
                parentFunc->GetParamScope()->SetIsObject();
            }
        }

        // Propagate "hasDeferredChild" attribute back to parent.
        if (hasAnyDeferredChild)
        {
            Assert(CONFIG_FLAG(DeferNested));
            parentFunc->SetHasDeferredChild();

            // Anything in parent may have non-local reference from deferredChild.
            setHasNonLocalReference = true;
            parentFunctionBody->SetAllNonLocalReferenced(true);

            // If a deferred child has with, parent scopes may contain symbols called inside the with.
            // Current implementation needs the symScope isObject.
            if (top->ChildHasWith() || pnode->sxFnc.HasWithStmt())
            {
                parentFunc->SetChildHasWith();
                parentFunc->GetBodyScope()->SetIsObject();
                parentFunc->GetParamScope()->SetIsObject();

                // Record this for future use in the no-refresh debugging.
                parentFunctionBody->SetHasSetIsObject(true);
            }
        }

        // Progagate HasMaybeEscapedNestedFunc
        if (!byteCodeGenerator->CanStackNestedFunc(top, false) ||
            byteCodeGenerator->NeedObjectAsFunctionScope(top, pnode))
        {
            parentFunc->SetHasMaybeEscapedNestedFunc(DebugOnly(L"Child"));
        }
    }

    if (top->GetCallsEval() || top->GetChildCallsEval())
    {
        parentFunc->SetChildCallsEval(true);
        ParseNode *currentBlock = byteCodeGenerator->GetCurrentBlock();
        if (currentBlock)
        {
            Assert(currentBlock->nop == knopBlock);
            currentBlock->sxBlock.SetChildCallsEval(true);
        }
        parentFunc->SetHasHeapArguments(true);
        setHasNonLocalReference = true;
        parentFunctionBody->SetAllNonLocalReferenced(true);

        Scope * const funcExprScope = top->funcExprScope;
        if (funcExprScope)
        {
            // If we have the body scope as an object, the outer function expression scope also needs to be an object to propagate the name.
            funcExprScope->SetIsObject();
        }

        if (parentFunc->inArgsCount == 1)
        {
            // If no formals to function, no need to create the propertyid array
            byteCodeGenerator->AssignNullConstRegister();
        }
    }

    if (setHasNonLocalReference && !parentFunctionBody->HasDoneAllNonLocalReferenced())
    {
        parentFunc->GetBodyScope()->ForceAllSymbolNonLocalReference(byteCodeGenerator);
        if (!parentFunc->IsGlobalFunction())
        {
            parentFunc->GetParamScope()->ForceAllSymbolNonLocalReference(byteCodeGenerator);
        }
        parentFunctionBody->SetHasDoneAllNonLocalReferenced(true);
    }

    if (top->HasSuperReference())
    {
        // Language service may remove all of the knopSuper ParseNodes. If this does happen, the function will still be marked as having a super reference.
        // Since we will emit the LdSuper opcode regardless, assign the super register now.
        top->AssignSuperRegister();
    }

    AssignFuncSymRegister(pnode, byteCodeGenerator, top);

    return top;
}

void Scope::ForceAllSymbolNonLocalReference(ByteCodeGenerator *byteCodeGenerator)
{
    this->ForEachSymbol([this, byteCodeGenerator](Symbol *const sym)
    {
        if(!sym->GetIsArguments())
        {
            sym->SetHasNonLocalReference(true, byteCodeGenerator);
            this->GetFunc()->SetHasLocalInClosure(true);
        }
    });
}

bool Scope::IsEmpty() const
{
    if (GetFunc()->bodyScope == this || (GetFunc()->IsGlobalFunction() && this->IsGlobalEvalBlockScope()))
    {
        return Count() == 0 && !GetFunc()->isThisLexicallyCaptured;
    }
    else
    {
        return Count() == 0;
    }
}

void Scope::SetIsObject()
{
    if (this->isObject)
    {
        return;
    }

    this->isObject = true;

    // We might set the scope to be object after we have process the symbol
    // (e.g. "With" scope referencing a symbol in an outter scope)
    // If we have func assignment, we need to mark the function to not do stack nested function
    // as these are now assigned to a scope object.
    FuncInfo * funcInfo = this->GetFunc();
    if (funcInfo && !funcInfo->HasMaybeEscapedNestedFunc())
    {
        this->ForEachSymbolUntil([funcInfo](Symbol * const sym)
        {
            if (sym->GetHasFuncAssignment())
            {
                funcInfo->SetHasMaybeEscapedNestedFunc(DebugOnly(L"DelayedObjectScopeAssignment"));
                return true;
            }
            return false;
        });
    }
}

void MarkInit(ParseNode* pnode)
{
    if (pnode->nop == knopList)
    {
        do
        {
            MarkInit(pnode->sxBin.pnode1);
            pnode = pnode->sxBin.pnode2;
        }
        while (pnode->nop == knopList);
        MarkInit(pnode);
    }
    else
    {
        Symbol *sym = nullptr;
        ParseNode *pnodeInit = nullptr;
        if (pnode->nop == knopVarDecl)
        {
            sym = pnode->sxVar.sym;
            pnodeInit = pnode->sxVar.pnodeInit;
        }
        else if (pnode->nop == knopAsg && pnode->sxBin.pnode1->nop == knopName)
        {
            sym = pnode->sxBin.pnode1->sxPid.sym;
            pnodeInit = pnode->sxBin.pnode2;
        }
        if (sym && !sym->GetIsUsed() && pnodeInit)
        {
            sym->SetHasInit(TRUE);
            if (sym->HasVisitedCapturingFunc())
            {
                sym->SetHasNonCommittedReference(false);
            }
        }
    }
}

void AddFunctionsToScope(ParseNodePtr scope, ByteCodeGenerator * byteCodeGenerator)
{
    VisitFncDecls(scope, [byteCodeGenerator](ParseNode *fn, ParseNode *pnodeName)
    {
        if (pnodeName && pnodeName->nop == knopVarDecl && fn->sxFnc.IsDeclaration())
        {
            const wchar_t *fnName = pnodeName->sxVar.pid->Psz();
            if (byteCodeGenerator->Trace())
            {
                Output::Print(L"current context has declared function %s\n", fnName);
            }
            // In ES6, functions are scoped to the block, which will be the current scope.
            // Pre-ES6, function declarations are scoped to the function body, so get that scope.
            Symbol *sym;
            if (!byteCodeGenerator->GetCurrentScope()->IsGlobalEvalBlockScope())
            {
                sym = byteCodeGenerator->AddSymbolToScope(byteCodeGenerator->GetCurrentScope(), fnName, pnodeName->sxVar.pid->Cch(), pnodeName, STFunction);
            }
            else
            {
                sym = byteCodeGenerator->AddSymbolToFunctionScope(fnName, pnodeName->sxVar.pid->Cch(), pnodeName, STFunction);
            }
            pnodeName->sxVar.sym = sym;

            if (sym->GetIsGlobal())
            {
                FuncInfo* func = byteCodeGenerator->TopFuncInfo();
                func->SetHasGlobalRef(true);
            }

            if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsBlockScopeEnabled()
                && sym->GetScope() != sym->GetScope()->GetFunc()->GetBodyScope()
                && sym->GetScope() != sym->GetScope()->GetFunc()->GetParamScope())
            {
                sym->SetIsBlockVar(true);
            }
        }
    });
}

void BindComplexFncDecl(ParseNodePtr scope, ByteCodeGenerator * byteCodeGenerator)
{
    VisitFncDecls(scope, [byteCodeGenerator](ParseNode *fn, ParseNode *pnodeName)
    {
        // This binds the LHS of the :: operator for the event binding function declaration
        // syntax. e.g. function x::y() {} will cause x to be bound here.
        if (pnodeName && (pnodeName->nop == knopScope || pnodeName->nop == knopDot))
        {
            BindInstAndMember(pnodeName, byteCodeGenerator);
        }
    });
}

void Scope::MergeParamAndBodyScopes(ParseNode *pnodeScope, ByteCodeGenerator * byteCodeGenerator)
{
    Assert(pnodeScope->sxFnc.funcInfo);
    Scope *paramScope = pnodeScope->sxFnc.pnodeScopes->sxBlock.scope;
    Scope *bodyScope = pnodeScope->sxFnc.pnodeBodyScope->sxBlock.scope;

    Assert(paramScope->m_symList == nullptr || paramScope->symbolTable == nullptr);
    Assert(bodyScope->m_symList == nullptr || bodyScope->symbolTable == nullptr);

    if (paramScope->Count() == 0)
    {
        // Once the scopes are merged, there's no reason to instantiate the param scope.
        paramScope->SetMustInstantiate(false);

        // Scopes are already merged or we don't have an arguments object. Go ahead and
        // remove the param scope from the scope chain.
        bodyScope->SetEnclosingScope(paramScope->GetEnclosingScope());
        return;
    }

    bodyScope->ForEachSymbol([&](Symbol * sym)
    {
        // Duplicate 'arguments' - param scope arguments wins.
        if (byteCodeGenerator->UseParserBindings()
            && sym->GetDecl()->sxVar.pid == byteCodeGenerator->GetParser()->names()->arguments)
        {
            return;
        }

#ifdef LANGUAGE_SERVICE
        // The language service can have duplicate syms that are not 'arguments' in the symbol table.
        // This can happen if we have error nodes in both parameter and body scope. In this case, the
        // body scoped error node needs to win.
        if (paramScope->symbolTable != nullptr && paramScope->symbolTable->ContainsKey(sym->GetName()))
        {
            Assert(sym->GetDecl()->sxVar.pid == byteCodeGenerator->GetParser()->GetErrorPid()
                || sym->GetDecl()->sxVar.pid == byteCodeGenerator->GetParser()->GetDeclErrorPid());
            paramScope->symbolTable->RemoveKey(sym->GetName());
        }
#endif

        Assert(paramScope->m_symList == nullptr || paramScope->FindLocalSymbol(sym->GetName()) == nullptr);
        paramScope->AddNewSymbol(sym);
    });

    // Reassign non-formal slot positions. Formals need to keep their slot positions to ensure
    // the argument object works properly. Other symbols need to be reassigned slot positions.
    paramScope->ForEachSymbol([&](Symbol * sym)
    {
        if (sym->GetSymbolType() != STFormal && sym->GetScopeSlot() != Js::Constants::NoProperty)
        {
            sym->SetScopeSlot(Js::Constants::NoProperty);
            sym->EnsureScopeSlot(pnodeScope->sxFnc.funcInfo);
        }
        sym->SetScope(bodyScope);
    });

    bodyScope->m_count = paramScope->m_count;
    bodyScope->m_symList = paramScope->m_symList;
    bodyScope->scopeSlotCount = paramScope->scopeSlotCount;
    if (bodyScope->symbolTable != nullptr)
    {
        Adelete(byteCodeGenerator->GetAllocator(), bodyScope->symbolTable);
        bodyScope->symbolTable = nullptr;
    }
    bodyScope->symbolTable = paramScope->symbolTable;
    if (paramScope->GetIsObject())
    {
        bodyScope->SetIsObject();
    }
    if (paramScope->GetMustInstantiate())
    {
        bodyScope->SetMustInstantiate(true);
    }

    // Once the scopes are merged, there's no reason to instantiate the param scope.
    paramScope->SetMustInstantiate(false);

    paramScope->m_count = 0;
    paramScope->scopeSlotCount = 0;
    paramScope->m_symList = nullptr;
    paramScope->symbolTable = nullptr;

    // Remove the parameter scope from the scope chain.
    bodyScope->SetEnclosingScope(paramScope->GetEnclosingScope());
}

template <class PrefixFn, class PostfixFn>
void VisitNestedScopes(ParseNode* pnodeScopeList, ParseNode* pnodeParent, ByteCodeGenerator* byteCodeGenerator,
    PrefixFn prefix, PostfixFn postfix, uint *pIndex)
{
    // Visit all scopes nested in this scope before visiting this function's statements. This way we have all the
    // attributes of all the inner functions before we assign registers within this function.
    // All the attributes we need to propagate downward should already be recorded by the parser.
    // - call to "eval()"
    // - nested in "with"
    Js::ParseableFunctionInfo* parentFunc = pnodeParent->sxFnc.funcInfo->byteCodeFunction;
    ParseNode* pnodeScope;
    uint i = 0;

    // Cache to restore it back once we come out of current function.
    Js::FunctionBody * pLastReuseFunc = byteCodeGenerator->pCurrentFunction;

    for (pnodeScope = pnodeScopeList; pnodeScope;)
    {
        switch (pnodeScope->nop)
        {
        case knopFncDecl:
            if (pLastReuseFunc)
            {
                if (!byteCodeGenerator->IsInNonDebugMode())
                {
                    // Here we are trying to match the inner sub-tree as well with already created inner function.

                    if ((pLastReuseFunc->GetIsGlobalFunc() && parentFunc->GetIsGlobalFunc())
                        || (!pLastReuseFunc->GetIsGlobalFunc() && !parentFunc->GetIsGlobalFunc()))
                    {
                        Assert(pLastReuseFunc->StartInDocument() == pnodeParent->ichMin);
                        Assert(pLastReuseFunc->LengthInChars() == pnodeParent->LengthInCodepoints());
                        Assert(pLastReuseFunc->GetNestedCount() == parentFunc->GetNestedCount());

                        // If the current function is not parse yet, its function body is not generated yet. Reset pCurrentFunction to null so that it will not be able re-use anything.
                        Js::FunctionProxy* proxy = pLastReuseFunc->GetNestedFunc((*pIndex));
                        if (proxy && proxy->IsFunctionBody())
                        {
                            byteCodeGenerator->pCurrentFunction = proxy->GetFunctionBody();
                        }
                        else
                        {
                            byteCodeGenerator->pCurrentFunction = NULL;
                        }
                    }
                }
                else if (!parentFunc->GetIsGlobalFunc())
                {
                    // In the deferred parsing mode, we will be reusing the only one function (which is asked when on ::Begin) all inner function will be created.
                    byteCodeGenerator->pCurrentFunction = NULL;
                }
            }
            PreVisitFunction(pnodeScope,byteCodeGenerator);

            pnodeParent->sxFnc.funcInfo->OnStartVisitFunction(pnodeScope);

            if (pnodeScope->sxFnc.pnodeBody)
            {
                if (!byteCodeGenerator->IsInNonDebugMode() && pLastReuseFunc != NULL && byteCodeGenerator->pCurrentFunction == NULL)
                {
                    // Patch current non-parsed function's functionbodyimpl with the new generated function body.
                    // So  that the function object (pointing to the old function body) can able to get to the new one.

                    Js::FunctionProxy* proxy = pLastReuseFunc->GetNestedFunc((*pIndex));
                    if (proxy && !proxy->IsFunctionBody())
                    {
                        proxy->UpdateFunctionBodyImpl(pnodeScope->sxFnc.funcInfo->byteCodeFunction->GetFunctionBody());
                    }
                }

                BindComplexFncDecl(pnodeScope->sxFnc.pnodeScopes->sxBlock.pnodeScopes, byteCodeGenerator);

                BeginVisitBlock(pnodeScope->sxFnc.pnodeScopes, byteCodeGenerator);
                i = 0;
                ParseNodePtr containerScope = pnodeScope->sxFnc.pnodeScopes;

                VisitNestedScopes(containerScope, pnodeScope, byteCodeGenerator, prefix, postfix, &i);

                MapFormals(pnodeScope, [&](ParseNode *argNode){ Visit(argNode, byteCodeGenerator, prefix, postfix); });

                if (!pnodeScope->sxFnc.IsSimpleParameterList())
                {
                    byteCodeGenerator->AssignUndefinedConstRegister();
                }

                BindComplexFncDecl(pnodeScope->sxFnc.pnodeBodyScope->sxBlock.pnodeScopes, byteCodeGenerator);

                BeginVisitBlock(pnodeScope->sxFnc.pnodeBodyScope, byteCodeGenerator);

                ParseNode* pnode = pnodeScope->sxFnc.pnodeBody;
                while (pnode->nop == knopList)
                {
                    // Check to see whether initializations of locals to "undef" can be skipped.
                    // The logic to do this is cheap and dirty - omit the init if we see an init
                    // with a value on the RHS at the top statement level (i.e., not inside a block,
                    // try, loop, etc.) before we see a use.
                    // The motivation is to help identify single-def locals in the BE.
                    // Note that this can't be done for globals, which must
                    byteCodeGenerator->SetCurrentTopStatement(pnode->sxBin.pnode1);
                    Visit(pnode->sxBin.pnode1,byteCodeGenerator, prefix, postfix);
                    if (!pnodeScope->sxFnc.funcInfo->GetCallsEval() &&
                        !pnodeScope->sxFnc.funcInfo->GetChildCallsEval() &&
                        // So that it will not be marked as init thus it will be added to the diagnostics symbols container.
                        !(byteCodeGenerator->ShouldTrackDebuggerMetadata()))
                    {
                        MarkInit(pnode->sxBin.pnode1);
                    }
                    pnode = pnode->sxBin.pnode2;
                }
                byteCodeGenerator->SetCurrentTopStatement(pnode);
                Visit(pnode,byteCodeGenerator, prefix, postfix);

                EndVisitBlock(pnodeScope->sxFnc.pnodeBodyScope, byteCodeGenerator);
                EndVisitBlock(pnodeScope->sxFnc.pnodeScopes, byteCodeGenerator);
            }
            else if (pnodeScope->sxFnc.nestedCount)
            {
                // The nested function is deferred but has its own nested functions.
                // Make sure we at least zero-initialize its array in case, for instance, we get cloned
                // before the function is called and the array filled in.
#ifdef RECYCLER_WRITE_BARRIER
                WriteBarrierPtr<Js::FunctionProxy>::ClearArray(pnodeScope->sxFnc.funcInfo->byteCodeFunction->GetNestedFuncArray(), pnodeScope->sxFnc.nestedCount);
#else
                memset(pnodeScope->sxFnc.funcInfo->byteCodeFunction->GetNestedFuncArray(), 0, pnodeScope->sxFnc.nestedCount * sizeof(Js::FunctionBody*));
#endif
            }
            pnodeScope->sxFnc.nestedIndex = *pIndex;
            parentFunc->SetNestedFunc(pnodeScope->sxFnc.funcInfo->byteCodeFunction, (*pIndex)++, byteCodeGenerator->GetFlags());

            Assert(parentFunc);

            pnodeParent->sxFnc.funcInfo->OnEndVisitFunction(pnodeScope);

            PostVisitFunction(pnodeScope,byteCodeGenerator);

            // Merge parameter and body scopes, unless we are deferring the function.
            // If we are deferring the function, we will need both scopes to do the proper binding when
            // the function is undeferred. After the function is undeferred, it is safe to merge the scopes.
            if (pnodeScope->sxFnc.funcInfo->paramScope != nullptr && pnodeScope->sxFnc.pnodeBody != nullptr)
            {
                Scope::MergeParamAndBodyScopes(pnodeScope, byteCodeGenerator);
            }

            pnodeScope = pnodeScope->sxFnc.pnodeNext;

            byteCodeGenerator->pCurrentFunction = pLastReuseFunc;
            break;

        case knopBlock:
            PreVisitBlock(pnodeScope, byteCodeGenerator);
            pnodeParent->sxFnc.funcInfo->OnStartVisitScope(pnodeScope->sxBlock.scope);
            VisitNestedScopes(pnodeScope->sxBlock.pnodeScopes, pnodeParent, byteCodeGenerator,  prefix, postfix, pIndex);
            pnodeParent->sxFnc.funcInfo->OnEndVisitScope(pnodeScope->sxBlock.scope);
            PostVisitBlock(pnodeScope, byteCodeGenerator);

            pnodeScope = pnodeScope->sxBlock.pnodeNext;
            break;

        case knopCatch:
            PreVisitCatch(pnodeScope, byteCodeGenerator);

            Visit(pnodeScope->sxCatch.pnodeParam, byteCodeGenerator, prefix, postfix);
            pnodeParent->sxFnc.funcInfo->OnStartVisitScope(pnodeScope->sxCatch.scope);
            VisitNestedScopes(pnodeScope->sxCatch.pnodeScopes, pnodeParent, byteCodeGenerator, prefix, postfix, pIndex);

            pnodeParent->sxFnc.funcInfo->OnEndVisitScope(pnodeScope->sxCatch.scope);
            PostVisitCatch(pnodeScope, byteCodeGenerator);

            pnodeScope = pnodeScope->sxCatch.pnodeNext;
            break;

        case knopWith:
            PreVisitWith(pnodeScope,byteCodeGenerator);
            pnodeParent->sxFnc.funcInfo->OnStartVisitScope(pnodeScope->sxWith.scope);
            VisitNestedScopes(pnodeScope->sxWith.pnodeScopes, pnodeParent,byteCodeGenerator, prefix, postfix, pIndex);
            pnodeParent->sxFnc.funcInfo->OnEndVisitScope(pnodeScope->sxWith.scope);
            PostVisitWith(pnodeScope,byteCodeGenerator);
            pnodeScope = pnodeScope->sxWith.pnodeNext;
            break;


        default:
            AssertMsg(false, "Unexpected opcode in tree of scopes");
            return;
        }
    }
}

void FuncInfo::OnStartVisitFunction(ParseNode *pnodeFnc)
{
    Assert(pnodeFnc->nop == knopFncDecl);
    Assert(this->GetCurrentChildFunction() == nullptr);

    this->SetCurrentChildFunction(pnodeFnc->sxFnc.funcInfo);
    pnodeFnc->sxFnc.funcInfo->SetCurrentChildScope(pnodeFnc->sxFnc.funcInfo->bodyScope);
}

void FuncInfo::OnEndVisitFunction(ParseNode *pnodeFnc)
{
    Assert(pnodeFnc->nop == knopFncDecl);
    Assert(this->GetCurrentChildFunction() == pnodeFnc->sxFnc.funcInfo);

    pnodeFnc->sxFnc.funcInfo->SetCurrentChildScope(nullptr);
    this->SetCurrentChildFunction(nullptr);
}

void FuncInfo::OnStartVisitScope(Scope *scope)
{
    if (scope == nullptr)
    {
        return;
    }

    if (scope->GetScopeType() == ScopeType_Parameter)
    {
        // If the scopes are unmerged and we are visiting the parameter scope, the child scope will be the function body scope.
        Assert(this->GetCurrentChildScope()->GetEnclosingScope() == scope || this->GetCurrentChildScope() == nullptr);
    }
    else
    {
        Assert(this->GetCurrentChildScope() == scope->GetEnclosingScope() || this->GetCurrentChildScope() == nullptr);
    }

    this->SetCurrentChildScope(scope);
}

void FuncInfo::OnEndVisitScope(Scope *scope)
{
    if (scope == nullptr)
    {
        return;
    }
    Assert(this->GetCurrentChildScope() == scope);

    this->SetCurrentChildScope(scope->GetEnclosingScope());
}

void PreVisitBlock(ParseNode *pnodeBlock, ByteCodeGenerator *byteCodeGenerator)
{
    if (!pnodeBlock->sxBlock.scope &&
        !pnodeBlock->sxBlock.HasBlockScopedContent() &&
        !pnodeBlock->sxBlock.GetCallsEval())
    {
        // Do nothing here if the block doesn't declare anything or call eval (which may declare something).
        return;
    }

    bool isGlobalEvalBlockScope = false;
    FuncInfo *func = byteCodeGenerator->TopFuncInfo();
    if (func->IsGlobalFunction() &&
        func->root->sxFnc.pnodeScopes == pnodeBlock &&
        byteCodeGenerator->IsEvalWithBlockScopingNoParentScopeInfo())
    {
        isGlobalEvalBlockScope = true;
    }
    Assert(!byteCodeGenerator->UseParserBindings() ||
           !pnodeBlock->sxBlock.scope ||
           isGlobalEvalBlockScope == (pnodeBlock->sxBlock.scope->GetScopeType() == ScopeType_GlobalEvalBlock));

    ArenaAllocator *alloc = byteCodeGenerator->GetAllocator();
    Scope *scope;

    if ((pnodeBlock->sxBlock.blockType == PnodeBlockType::Global && !byteCodeGenerator->IsEvalWithBlockScopingNoParentScopeInfo()) || pnodeBlock->sxBlock.blockType == PnodeBlockType::Function)
    {
        scope = byteCodeGenerator->GetCurrentScope();

        if (pnodeBlock->sxBlock.blockType == PnodeBlockType::Function
            && pnodeBlock->sxBlock.scope->GetScopeType() == ScopeType_Parameter
            && scope->GetScopeType() == ScopeType_FunctionBody)
        {
            scope = scope->GetEnclosingScope();
        }

        pnodeBlock->sxBlock.scope = scope;
    }
    else if (!(pnodeBlock->grfpn & fpnSyntheticNode) || isGlobalEvalBlockScope)
    {
        Assert(byteCodeGenerator->GetScriptContext()->GetConfig()->IsBlockScopeEnabled());
        scope = pnodeBlock->sxBlock.scope;
        if (!scope || !byteCodeGenerator->UseParserBindings())
        {
            scope = Anew(alloc, Scope, alloc,
                         isGlobalEvalBlockScope? ScopeType_GlobalEvalBlock : ScopeType_Block, true);
            pnodeBlock->sxBlock.scope = scope;
        }
        scope->SetFunc(byteCodeGenerator->TopFuncInfo());
        // For now, prevent block scope from being merged with enclosing function scope.
        // Consider optimizing this.
        scope->SetCanMerge(false);

        if (isGlobalEvalBlockScope ||
            (BinaryFeatureControl::LanguageService() && byteCodeGenerator->IsLanguageServiceEnabled()))
        {
            scope->SetIsObject();
        }

        byteCodeGenerator->PushScope(scope);
        byteCodeGenerator->PushBlock(pnodeBlock);
    }
    else
    {
        return;
    }

    Assert(scope && scope == pnodeBlock->sxBlock.scope);

    bool isGlobalScope = (scope->GetEnclosingScope() == null);
    Assert(!isGlobalScope || (pnodeBlock->grfpn & fpnSyntheticNode));

    // if it is  the global eval block scope, we don't what function decl to be assigned in the block scope.
    // they should already declared in the global function's scope
    if (!isGlobalEvalBlockScope && !isGlobalScope)
    {
        AddFunctionsToScope(pnodeBlock->sxBlock.pnodeScopes, byteCodeGenerator);
    }

    auto addSymbolToScope = [scope, byteCodeGenerator, isGlobalScope](ParseNode *pnode)
        {
            Symbol *sym = byteCodeGenerator->AddSymbolToScope(scope, reinterpret_cast<const wchar_t*>(pnode->sxVar.pid->Psz()), pnode->sxVar.pid->Cch(), pnode, STVariable);
            if (sym->GetSymbolType() == STVariable && byteCodeGenerator->Trace()) {
                Output::Print(L"current context has declared const %s of type %s\n",
                    pnode->sxVar.pid->Psz(), sym->GetSymbolTypeName());
            }
            sym->SetIsGlobal(isGlobalScope);
            sym->SetIsBlockVar(true);
            sym->SetNeedDeclaration(true);
            pnode->sxVar.sym = sym;
        };

    byteCodeGenerator->IterateBlockScopedVariables(pnodeBlock, addSymbolToScope);
}

void PostVisitBlock(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    if (!BlockHasOwnScope(pnode, byteCodeGenerator))
    {
        return;
    }

    if (pnode->sxBlock.GetCallsEval() || pnode->sxBlock.GetChildCallsEval() || (byteCodeGenerator->GetFlags() & (fscrEval | fscrImplicitThis | fscrImplicitParents)))
    {
        Scope *scope = pnode->sxBlock.scope;
        bool scopeIsEmpty = scope->IsEmpty();
        scope->SetIsObject();
        scope->SetCapturesAll(true);
        scope->SetMustInstantiate(!scopeIsEmpty);
    }

    byteCodeGenerator->PopScope();
    byteCodeGenerator->PopBlock();

    ParseNode *currentBlock = byteCodeGenerator->GetCurrentBlock();
    if (currentBlock && (pnode->sxBlock.GetCallsEval() || pnode->sxBlock.GetChildCallsEval()))
    {
        currentBlock->sxBlock.SetChildCallsEval(true);
    }
}

void PreVisitCatch(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    // Push the catch scope and add the catch expression to it.
    byteCodeGenerator->StartBindCatch(pnode);
    Symbol *sym;
    if (!byteCodeGenerator->UseParserBindings())
    {
        sym = byteCodeGenerator->AddSymbolToScope(pnode->sxCatch.scope, reinterpret_cast<const wchar_t*>(pnode->sxCatch.pnodeParam->sxPid.pid->Psz()), pnode->sxCatch.pnodeParam->sxPid.pid->Cch(), pnode->sxCatch.pnodeParam, STVariable);
    }
    else
    {
        sym = *pnode->sxCatch.pnodeParam->sxPid.symRef;
    }
    Assert(sym->GetScope() == pnode->sxCatch.scope);
    if (byteCodeGenerator->Trace())
    {
        Output::Print(L"current context has declared catch var %s of type %s\n",
            pnode->sxCatch.pnodeParam->sxPid.pid->Psz(), sym->GetSymbolTypeName());
    }
    sym->SetIsCatch(true);
    pnode->sxCatch.pnodeParam->sxPid.sym = sym;
    // This call will actually add the nested function symbols to the enclosing function scope (which is what we want).
    AddFunctionsToScope(pnode->sxCatch.pnodeScopes, byteCodeGenerator);
}

void PostVisitCatch(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    byteCodeGenerator->EndBindCatch();
}

void PreVisitWith(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    ArenaAllocator *alloc = byteCodeGenerator->GetAllocator();
    Scope *scope = Anew(alloc, Scope, alloc, ScopeType_With);
    scope->SetFunc(byteCodeGenerator->TopFuncInfo());
    scope->SetIsDynamic(true);
    pnode->sxWith.scope = scope;

    byteCodeGenerator->PushScope(scope);
}

void PostVisitWith(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    byteCodeGenerator->PopScope();
}

void BindFuncSymbol(ParseNode *pnodeFnc, ByteCodeGenerator *byteCodeGenerator)
{
    VisitFncNames(pnodeFnc, [byteCodeGenerator](ParseNode *pnodeFnc, ParseNode *pnodeName)
    {
        if (pnodeName && pnodeName->nop == knopVarDecl)
        {
            Symbol *sym = pnodeName->sxVar.sym;
            FuncInfo* func = byteCodeGenerator->TopFuncInfo();
            if (sym == null || sym->GetIsGlobal())
            {
                func->SetHasGlobalRef(true);
            }
            else
            {
                if (!func->IsGlobalFunction() && func->GetChildCallsEval())
                {
                    // (How can the global function refer to a non-global function symbol? One way
                    //  is if the symbol is a named function expression, and we're in ES5 mode.)
                    if (!sym->GetFuncExpr())
                    {
                        sym->GetScope()->SetHasLocalInClosure(true);
                        sym->SetHasNonLocalReference(true, byteCodeGenerator);
                    }
                }
            }
        }
    });
}

// expand using hash table for library function names

bool IsLibraryFunction(ParseNode* expr,Js::ScriptContext* scriptContext) {
    if (expr && expr->nop==knopDot) {
        ParseNode* lhs=expr->sxBin.pnode1;
        ParseNode* rhs=expr->sxBin.pnode2;
        if ((lhs!=NULL)&&(rhs!=NULL)&&(lhs->nop==knopName)&&(rhs->nop==knopName)) {
            Symbol* lsym=lhs->sxPid.sym;
            if ((lsym == NULL || lsym->GetIsGlobal()) && lhs->sxPid.PropertyIdFromNameNode() == Js::PropertyIds::Math)
            {
                return Js::IsMathLibraryId(rhs->sxPid.PropertyIdFromNameNode());
            }
        }
    }
    return false;
}

struct ApplyCheck {
    bool matches;
    bool insideApplyCall;
    bool sawApply;
};



bool IsApplyArgs(ParseNode* callNode) {
    ParseNode* target=callNode->sxCall.pnodeTarget;
    ParseNode* args=callNode->sxCall.pnodeArgs;
    if ((target!=NULL)&&(target->nop==knopDot)) {
        ParseNode* lhsNode=target->sxBin.pnode1;
        if ((lhsNode!=NULL)&&((lhsNode->nop==knopDot)||(lhsNode->nop==knopName)) && !IsArguments(lhsNode)) {
            ParseNode* nameNode=target->sxBin.pnode2;
            if (nameNode!=NULL) {
                bool nameIsApply = nameNode->sxPid.PropertyIdFromNameNode() == Js::PropertyIds::apply;
                if (nameIsApply && args != NULL && args->nop == knopList)
                {
                    ParseNode* arg1=args->sxBin.pnode1;
                    ParseNode* arg2=args->sxBin.pnode2;
                    if ((arg1!=NULL)&&(arg1->nop==knopThis)&&(arg2!=NULL)&&(arg2->nop==knopName)&&(arg2->sxPid.sym!=NULL)) {
                        return arg2->sxPid.sym->GetIsArguments();
                    }
                }
            }
        }
    }
    return false;
}

void PostCheckApplyEnclosesArgs(ParseNode* pnode,ByteCodeGenerator* byteCodeGenerator,ApplyCheck* applyCheck) {
    if ((pnode==NULL)||(!applyCheck->matches)) {
        return;
    }
    if (pnode->nop==knopCall) {
        if ((!pnode->isUsed)&&IsApplyArgs(pnode)) {
            if (!applyCheck->insideApplyCall) {
                applyCheck->matches=false;
            }
            applyCheck->insideApplyCall=false;
        }
    }
}

void CheckApplyEnclosesArgs(ParseNode* pnode,ByteCodeGenerator* byteCodeGenerator,ApplyCheck* applyCheck) {
    if ((pnode==NULL)||(!applyCheck->matches)) {
        return;
    }
    switch (pnode->nop) {
    case knopName:
    {
        Symbol* sym=pnode->sxPid.sym;
        if (sym!=NULL) {
            if (sym->GetIsArguments()) {
                if (!applyCheck->insideApplyCall) {
                    applyCheck->matches=false;
                }
            }
        }
    }
    break;
    case knopCall:
    {
        if ((!pnode->isUsed)&&IsApplyArgs(pnode)) {
            // no nested apply calls
            if (applyCheck->insideApplyCall) {
                applyCheck->matches=false;
            }
            else {
                applyCheck->insideApplyCall=true;
                applyCheck->sawApply=true;
                pnode->sxCall.isApplyCall=true;
            }
        }
    }
    break;
    }
}

bool ApplyEnclosesArgs(ParseNode* fncDecl,ByteCodeGenerator* byteCodeGenerator) {
    if (byteCodeGenerator->IsInDebugMode())
    {
        // Inspection of the arguments object will be messed up if we do ApplyArgs.
        return false;
    }
    if (!fncDecl->HasVarArguments()
        && fncDecl->sxFnc.pnodeArgs == nullptr
        && fncDecl->sxFnc.pnodeRest == nullptr
        && fncDecl->sxFnc.nestedCount == 0) {
        ApplyCheck applyCheck;
        applyCheck.matches=true;
        applyCheck.sawApply=false;
        applyCheck.insideApplyCall=false;
        VisitIndirect<ApplyCheck>(fncDecl->sxFnc.pnodeBody,byteCodeGenerator,&applyCheck,&CheckApplyEnclosesArgs,&PostCheckApplyEnclosesArgs);
        return applyCheck.matches&&applyCheck.sawApply;
    }
    return false;
}

void CheckInvertableExpr(ParseNode* pnode,ByteCodeGenerator* byteCodeGenerator,SymCheck* symCheck) {
    if (symCheck->result) {
        switch (pnode->nop) {
        case knopName:
            if (symCheck->MatchSymbol(pnode->sxPid.sym)) {
                symCheck->result=false;
            }
            break;
        case knopCall: {
            ParseNode* callTarget=pnode->sxBin.pnode1;
            if (callTarget!=NULL) {
                if (callTarget->nop==knopName) {
                    Symbol* sym=callTarget->sxPid.sym;
                    if (sym && sym->SingleDef()) {
                        ParseNode* decl=sym->GetDecl();
                        if ((decl!=NULL)&&(decl->nop==knopVarDecl)&&
                            IsLibraryFunction(decl->sxVar.pnodeInit,byteCodeGenerator->GetScriptContext())) {
                        }
                        else {
                            symCheck->result=false;
                        }
                    }
                    else {
                        symCheck->result=false;
                    }
                }
                else if (callTarget->nop==knopDot) {
                    if (IsLibraryFunction(callTarget,byteCodeGenerator->GetScriptContext())) {
                    }
                    else {
                        symCheck->result=false;
                    }
                }
                else {
                    symCheck->result=false;
                }
            }
            break;
                       }
        case knopDot:
            if (!IsLibraryFunction(pnode,byteCodeGenerator->GetScriptContext())) {
                symCheck->result=false;
            }
            break;
        case knopTrue:
        case knopFalse:
        case knopAdd:
        case knopSub:
        case knopDiv:
        case knopMul:
        case knopMod:
        case knopNeg:
        case knopInt:
        case knopFlt:
        case knopLt:
        case knopGt:
        case knopLe:
        case knopGe:
        case knopEq:
        case knopNe:
            break;
        default:
            symCheck->result=false;
            break;
        }
    }
}

bool InvertableExpr(SymCheck* symCheck,ParseNode* expr,ByteCodeGenerator* byteCodeGenerator) {
    symCheck->result=true;
    symCheck->cond=false;
    symCheck->permittedSym=NULL;
    VisitIndirect<SymCheck>(expr,byteCodeGenerator,symCheck,&CheckInvertableExpr,NULL);
    return symCheck->result;
}

bool InvertableExprPlus(SymCheck* symCheck,ParseNode* expr,ByteCodeGenerator* byteCodeGenerator,Symbol* permittedSym) {
    symCheck->result=true;
    symCheck->cond=true;
    symCheck->permittedSym=permittedSym;
    VisitIndirect<SymCheck>(expr,byteCodeGenerator,symCheck,&CheckInvertableExpr,NULL);
    return symCheck->result;
}

void CheckLocalVarDef(ParseNode *pnode,ByteCodeGenerator *byteCodeGenerator)
{
    Assert(pnode->nop == knopAsg);
    if (pnode->sxBin.pnode1!=NULL)
    {
        ParseNode *lhs=pnode->sxBin.pnode1;
        if (lhs->nop == knopName)
        {
            Symbol *sym=lhs->sxPid.sym;
            if (sym != NULL)
            {
                sym->RecordDef();
            }
        }
    }
}

ParseNode* ConstructInvertedStatement(ParseNode* stmt,ByteCodeGenerator* byteCodeGenerator,FuncInfo* funcInfo,
    ParseNode** outerStmtRef) {
        if (stmt==NULL) {
            return NULL;
        }
        ParseNode* cStmt;
        if ((stmt->nop==knopAsg)||(stmt->nop==knopVarDecl)) {
            ParseNode* rhs=NULL;
            ParseNode* lhs=NULL;

            if (stmt->nop==knopAsg) {
                rhs=stmt->sxBin.pnode2;
                lhs=stmt->sxBin.pnode1;
            }
            else if (stmt->nop==knopVarDecl) {
                rhs=stmt->sxVar.pnodeInit;
            }
            ArenaAllocator* alloc=byteCodeGenerator->GetAllocator();
            ParseNode* loopInvar=byteCodeGenerator->GetParser()->CreateTempNode(rhs);
            loopInvar->location=funcInfo->NextVarRegister();

            // Can't use a temp register here because the inversion happens at the parse tree level without generating
            // any byte-code yet. All local non-temp registers need to be initialized for jitted loop bodies, and since this is
            // not a user variable, track this register separately to have it be initialized at the top of the function.
            funcInfo->nonUserNonTempRegistersToInitialize.Add(loopInvar->location);

            // add temp node to list of initializers for new outer loop
            if ((*outerStmtRef)->sxBin.pnode1==NULL) {
                (*outerStmtRef)->sxBin.pnode1=loopInvar;
            }
            else {
                ParseNode* listNode=Parser::StaticCreateBinNode(knopList, NULL,NULL,alloc);
                (*outerStmtRef)->sxBin.pnode2=listNode;
                listNode->sxBin.pnode1=loopInvar;
                *outerStmtRef=listNode;
            }

            ParseNode* tempName=byteCodeGenerator->GetParser()->CreateTempRef(loopInvar);

            if (lhs!=NULL) {
                cStmt=Parser::StaticCreateBinNode(knopAsg, lhs,tempName,alloc);
            }
            else {
                // Use AddVarDeclNode to add the var to the function.
                // Do not use CreateVarDeclNode which is meant to be used while parsing. It assumes that
                // parser's internal data structures (m_ppnodeVar in particular) is at the "current" location.
                cStmt=byteCodeGenerator->GetParser()->AddVarDeclNode(stmt->sxVar.pid, funcInfo->root);
                cStmt->sxVar.pnodeInit=tempName;
                cStmt->sxVar.sym=stmt->sxVar.sym;
            }
        }
        else {
            cStmt=byteCodeGenerator->GetParser()->CopyPnode(stmt);
        }
        return cStmt;
}

ParseNode* ConstructInvertedLoop(ParseNode* innerLoop,ParseNode* outerLoop,ByteCodeGenerator* byteCodeGenerator,FuncInfo* funcInfo) {
    ArenaAllocator* alloc=byteCodeGenerator->GetAllocator();
    ParseNode* outerLoopC=Parser::StaticCreateNodeT<knopFor>(alloc);
    outerLoopC->sxFor.pnodeInit=innerLoop->sxFor.pnodeInit;
    outerLoopC->sxFor.pnodeCond=innerLoop->sxFor.pnodeCond;
    outerLoopC->sxFor.pnodeIncr=innerLoop->sxFor.pnodeIncr;
    outerLoopC->sxFor.pnodeBlock = innerLoop->sxFor.pnodeBlock;
    outerLoopC->sxFor.pnodeInverted=NULL;

    ParseNode* innerLoopC=Parser::StaticCreateNodeT<knopFor>(alloc);
    innerLoopC->sxFor.pnodeInit=outerLoop->sxFor.pnodeInit;
    innerLoopC->sxFor.pnodeCond=outerLoop->sxFor.pnodeCond;
    innerLoopC->sxFor.pnodeIncr=outerLoop->sxFor.pnodeIncr;
    innerLoopC->sxFor.pnodeBlock = outerLoop->sxFor.pnodeBlock;
    innerLoopC->sxFor.pnodeInverted=NULL;

    ParseNode* innerBod=Parser::StaticCreateBlockNode(alloc);
    innerLoopC->sxFor.pnodeBody=innerBod;
    innerBod->sxBlock.scope = innerLoop->sxFor.pnodeBody->sxBlock.scope;

    ParseNode* outerBod=Parser::StaticCreateBlockNode(alloc);
    outerLoopC->sxFor.pnodeBody=outerBod;
    outerBod->sxBlock.scope = outerLoop->sxFor.pnodeBody->sxBlock.scope;

    ParseNode* listNode=Parser::StaticCreateBinNode(knopList,NULL,NULL,alloc);
    outerBod->sxBlock.pnodeStmt=listNode;

    ParseNode* innerBodOriginal=innerLoop->sxFor.pnodeBody;
    ParseNode* origStmt=innerBodOriginal->sxBlock.pnodeStmt;
    if (origStmt->nop==knopList) {
        ParseNode* invertedStmt=NULL;
        while (origStmt->nop==knopList) {
            ParseNode* invertedItem=ConstructInvertedStatement(origStmt->sxBin.pnode1,byteCodeGenerator,funcInfo,&listNode);
            if (invertedStmt!=NULL) {
                invertedStmt = invertedStmt->sxBin.pnode2 = byteCodeGenerator->GetParser()->CreateBinNode(knopList, invertedItem,NULL);
            }
            else {
                invertedStmt = innerBod->sxBlock.pnodeStmt = byteCodeGenerator->GetParser()->CreateBinNode(knopList, invertedItem,NULL);
            }
            origStmt=origStmt->sxBin.pnode2;
        }
        Assert(invertedStmt!=NULL);
        invertedStmt->sxBin.pnode2 = ConstructInvertedStatement(origStmt,byteCodeGenerator,funcInfo,&listNode);
    }
    else {
        innerBod->sxBlock.pnodeStmt=ConstructInvertedStatement(origStmt,byteCodeGenerator,funcInfo,&listNode);
    }
    if (listNode->sxBin.pnode1 == NULL)
    {
        listNode->sxBin.pnode1 = byteCodeGenerator->GetParser()->CreateTempNode(NULL);
    }
    listNode->sxBin.pnode2=innerLoopC;
    return outerLoopC;
}

bool InvertableStmt(ParseNode* stmt,Symbol* outerVar,ParseNode* innerLoop,ParseNode* outerLoop,ByteCodeGenerator* byteCodeGenerator,SymCheck* symCheck) {
    if (stmt!=NULL) {
        ParseNode* lhs=NULL;
        ParseNode* rhs=NULL;
        if (stmt->nop==knopAsg) {
            lhs=stmt->sxBin.pnode1;
            rhs=stmt->sxBin.pnode2;
        }
        else if (stmt->nop==knopVarDecl) {
            rhs=stmt->sxVar.pnodeInit;
        }
        if (lhs!=NULL) {
            if (lhs->nop==knopDot) {
                return false;
            }
            if (lhs->nop==knopName) {
                if ((lhs->sxPid.sym!=NULL)&&(lhs->sxPid.sym->GetIsGlobal())) {
                    return false;
                }
            }
            else if (lhs->nop==knopIndex) {
                ParseNode* indexed=lhs->sxBin.pnode1;
                ParseNode* index=lhs->sxBin.pnode2;

                if ((index==NULL)||(indexed==NULL)) {
                    return false;
                }

                if ((indexed->nop!=knopName)||(indexed->sxPid.sym==NULL)) {
                    return false;
                }

                if (!InvertableExprPlus(symCheck,index,byteCodeGenerator,outerVar)) {
                    return false;
                }
            }
        }
        if (rhs!=NULL) {
            if (!InvertableExpr(symCheck,rhs,byteCodeGenerator)) {
                return false;
            }
        }
        else {
            if (!InvertableExpr(symCheck,stmt,byteCodeGenerator)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool GatherInversionSyms(ParseNode* stmt,Symbol* outerVar,ParseNode* innerLoop,ByteCodeGenerator* byteCodeGenerator,SymCheck* symCheck) {
    if (stmt!=NULL) {
        ParseNode* lhs=NULL;
        Symbol* auxSym=NULL;

        if (stmt->nop==knopAsg) {
            lhs=stmt->sxBin.pnode1;
        }
        else if (stmt->nop==knopVarDecl) {
            auxSym=stmt->sxVar.sym;
        }
        if (lhs!=NULL) {
            if (lhs->nop==knopDot) {
                return false;
            }
            if (lhs->nop==knopName) {
                if ((lhs->sxPid.sym == NULL) || (lhs->sxPid.sym->GetIsGlobal())) {
                    return false;
                }
                else {
                    auxSym=lhs->sxPid.sym;
                }
            }
        }
        if (auxSym!=NULL) {
            return symCheck->AddSymbol(auxSym);
        }
    }
    return true;
}

bool InvertableBlock(ParseNode* block,Symbol* outerVar,ParseNode* innerLoop,ParseNode* outerLoop,ByteCodeGenerator* byteCodeGenerator,
    SymCheck* symCheck) {
        if (block==NULL) {
            return false;
        }
        if (!symCheck->AddSymbol(outerVar)) {
            return false;
        }
        if (innerLoop->sxFor.pnodeBody->nop == knopBlock && innerLoop->sxFor.pnodeBody->sxBlock.HasBlockScopedContent()
            || outerLoop->sxFor.pnodeBody->nop == knopBlock && outerLoop->sxFor.pnodeBody->sxBlock.HasBlockScopedContent())
        {
            // we can not invert loops if there are block scoped declarations inside
            return false;
        }
        if ((block!=NULL)&&(block->nop==knopBlock)) {
            ParseNode* stmt=block->sxBlock.pnodeStmt;
            while ((stmt!=NULL)&&(stmt->nop==knopList)) {
                if (!GatherInversionSyms(stmt->sxBin.pnode1,outerVar,innerLoop,byteCodeGenerator,symCheck)) {
                    return false;
                }
                stmt=stmt->sxBin.pnode2;
            }
            if (!GatherInversionSyms(stmt,outerVar,innerLoop,byteCodeGenerator,symCheck)) {
                return false;
            }
            stmt=block->sxBlock.pnodeStmt;
            while ((stmt!=NULL)&&(stmt->nop==knopList)) {
                if (!InvertableStmt(stmt->sxBin.pnode1,outerVar,innerLoop,outerLoop,byteCodeGenerator,symCheck)) {
                    return false;
                }
                stmt=stmt->sxBin.pnode2;
            }
            if (!InvertableStmt(stmt,outerVar,innerLoop,outerLoop,byteCodeGenerator,symCheck)) {
                return false;
            }
            return (InvertableExprPlus(symCheck,innerLoop->sxFor.pnodeCond,byteCodeGenerator,NULL)&&
                InvertableExprPlus(symCheck,outerLoop->sxFor.pnodeCond,byteCodeGenerator,outerVar));
        }
        else {
            return false;
        }
}

// start of invert loop optimization
// for now, find simple cases (only for loops around single assignment)
// returns new AST for inverted loop; also returns in out param
// side effects level, if any that guards the new AST (old AST will be
// used if guard fails).
// should only be called with loopNode representing top-level statement

ParseNode* InvertLoop(ParseNode* outerLoop,ByteCodeGenerator* byteCodeGenerator,FuncInfo* funcInfo) {
    if (byteCodeGenerator->GetScriptContext()->optimizationOverrides.GetSideEffects() != Js::SideEffects_None) {
        return NULL;
    }
    SymCheck symCheck;
    symCheck.Init();


    if (outerLoop->nop==knopFor) {
        ParseNode* innerLoop=outerLoop->sxFor.pnodeBody;
        if ((innerLoop==NULL)||(innerLoop->nop!=knopBlock)) {
            return NULL;
        }
        else {
            innerLoop=innerLoop->sxBlock.pnodeStmt;
        }
        if ((innerLoop!=NULL)&&(innerLoop->nop==knopFor)) {
            if ((outerLoop->sxFor.pnodeInit!=NULL)&&
                (outerLoop->sxFor.pnodeInit->nop==knopVarDecl)&&
                (outerLoop->sxFor.pnodeInit->sxVar.pnodeInit!=NULL)&&
                (outerLoop->sxFor.pnodeInit->sxVar.pnodeInit->nop==knopInt)&&
                (outerLoop->sxFor.pnodeIncr!=NULL)&&
                ((outerLoop->sxFor.pnodeIncr->nop==knopIncPre)||(outerLoop->sxFor.pnodeIncr->nop==knopIncPost))&&
                (outerLoop->sxFor.pnodeIncr->sxUni.pnode1->nop == knopName) &&
                (outerLoop->sxFor.pnodeInit->sxVar.pid == outerLoop->sxFor.pnodeIncr->sxUni.pnode1->sxPid.pid) &&
                (innerLoop->sxFor.pnodeIncr!=NULL)&&
                ((innerLoop->sxFor.pnodeIncr->nop==knopIncPre)||(innerLoop->sxFor.pnodeIncr->nop==knopIncPost))&&
                (innerLoop->sxFor.pnodeInit!=NULL) &&
                (innerLoop->sxFor.pnodeInit->nop==knopVarDecl)&&
                (innerLoop->sxFor.pnodeInit->sxVar.pnodeInit!=NULL)&&
                (innerLoop->sxFor.pnodeInit->sxVar.pnodeInit->nop==knopInt)&&
                (innerLoop->sxFor.pnodeIncr->sxUni.pnode1->nop == knopName) &&
                (innerLoop->sxFor.pnodeInit->sxVar.pid == innerLoop->sxFor.pnodeIncr->sxUni.pnode1->sxPid.pid)) {
                    Symbol* outerVar=outerLoop->sxFor.pnodeInit->sxVar.sym;
                    Symbol* innerVar=innerLoop->sxFor.pnodeInit->sxVar.sym;
                    if ((outerVar!=NULL)&&(innerVar!=NULL)) {
                        ParseNode* block=innerLoop->sxFor.pnodeBody;
                        if (InvertableBlock(block,outerVar,innerLoop,outerLoop,byteCodeGenerator,&symCheck)) {
                            return ConstructInvertedLoop(innerLoop,outerLoop,byteCodeGenerator,funcInfo);
                        }
                    }

            }
        }
    }
    return NULL;
}

void SetAdditionalBindInfoForVariables(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator)
{
    Symbol *sym = pnode->sxVar.sym;
    if (sym == NULL)
    {
        return;
    }

    FuncInfo* func = byteCodeGenerator->TopFuncInfo();
    if (func->IsGlobalFunction())
    {
        func->SetHasGlobalRef(true);
    }
    if (!sym->GetIsGlobal() && !sym->GetIsArguments() &&
        (sym->GetScope() == func->GetBodyScope() || sym->GetScope() == func->GetParamScope() || sym->GetScope()->GetCanMerge()))
    {
        if (func->GetChildCallsEval())
        {
            func->SetHasLocalInClosure(true);
        }
        else
        {
            sym->RecordDef();
        }
    }

    // If this decl does an assignment inside a loop body, then there's a chance
    // that a jitted loop body will expect us to begin with a valid value in this var.
    // So mark the sym as used so that we guarantee the var will at least get "undefined".
    if (byteCodeGenerator->IsInLoop() &&
        pnode->sxVar.pnodeInit)
    {
        sym->SetIsUsed(TRUE);
    }
}

// bind references to definitions (prefix pass)
void Bind(ParseNode *pnode,ByteCodeGenerator *byteCodeGenerator)
{
    if (pnode==NULL)
        return;
    switch (pnode->nop) {
    case knopBreak:
    case knopContinue:
        byteCodeGenerator->AddTargetStmt(pnode->sxJump.pnodeTarget);
        break;
    case knopProg:
        {
            //Output::Print(L"program start\n");
            FuncInfo* globFuncInfo = byteCodeGenerator->StartBindGlobalStatements(pnode);
            pnode->sxFnc.funcInfo = globFuncInfo;
            AddFunctionsToScope(pnode->sxFnc.GetTopLevelScope(), byteCodeGenerator);
            AddVarsToScope(pnode->sxFnc.pnodeVars,byteCodeGenerator);
            BindComplexFncDecl(pnode->sxFnc.GetTopLevelScope(), byteCodeGenerator);
            // There are no args to add, but "eval" gets a this pointer.
            byteCodeGenerator->SetNumberOfInArgs(!!(byteCodeGenerator->GetFlags() & fscrEvalCode));
            if (!globFuncInfo->IsFakeGlobalFunction(byteCodeGenerator->GetFlags()))
            {
                // Global code: the root function is the global function.
                byteCodeGenerator->SetRootFuncInfo(globFuncInfo);
            }
            else if (globFuncInfo->byteCodeFunction)
            {
                // If the current global code wasn't marked to be treated as global code (eg from deferred parsing),we don't need to send a register script event for it.
                globFuncInfo->byteCodeFunction->SetIsTopLevel(false);
            }
            if (pnode->sxFnc.CallsEval())
            {
                globFuncInfo->SetCallsEval(true);
            }
            break;
        }
    case knopFncDecl:
        // VisitFunctionsInScope has already done binding within the declared function. Here, just record the fact
        // that the parent function has a local/global declaration in it.
        BindFuncSymbol(pnode, byteCodeGenerator);
        if (pnode->sxFnc.IsGenerator())
        {
            // Always assume generator functions escape since tracking them requires tracking
            // the resulting generators in addition to the function.
            byteCodeGenerator->FuncEscapes(byteCodeGenerator->TopFuncInfo()->GetBodyScope());
        }
        if (!pnode->sxFnc.IsDeclaration())
        {
            FuncInfo *funcInfo = byteCodeGenerator->TopFuncInfo();
            if (!funcInfo->IsGlobalFunction() || (byteCodeGenerator->GetFlags() & fscrEval))
            {
                // In the case of a nested function expression, assumes that it escapes.
                // We could try to analyze what it touches to be more precise.
                byteCodeGenerator->FuncEscapes(funcInfo->GetBodyScope());
            }
            byteCodeGenerator->ProcessCapturedSyms(pnode);
        }
        else if (byteCodeGenerator->IsInLoop())
        {
            Symbol *funcSym = pnode->sxFnc.GetFuncSymbol();
            if (funcSym)
            {
                Symbol *funcVarSym = funcSym->GetFuncScopeVarSym();
                if (funcVarSym)
                {
                    // We're going to write to the funcVarSym when we do the function instantiation,
                    // so treat the funcVarSym as used. That way, we know it will get undef-initialized at the
                    // top of the function, so a jitted loop body won't have any issue with boxing if
                    // the function instantiation isn't executed.
                    Assert(funcVarSym != funcSym);
                    funcVarSym->SetIsUsed(true);
                }
            }
        }
        break;
    case knopThis:
    case knopSuper:
    {
        FuncInfo *top = byteCodeGenerator->TopFuncInfo();
        if (top->IsGlobalFunction() && !(byteCodeGenerator->GetFlags() & fscrEval))
        {
            top->SetHasGlobalRef(true);
        }
        else if (top->IsLambda())
        {
            byteCodeGenerator->MarkThisUsedInLambda();
        }

        // "this" should be loaded for both global and non global functions
        byteCodeGenerator->TopFuncInfo()->GetParsedFunctionBody()->SetHasThis(true);
        break;
    }
    case knopName:
        if (pnode->sxPid.sym == null)
        {
            if (pnode->grfpn & fpnMemberReference)
            {
                // This is a member name. No binding.
                break;
            }
            Symbol *sym = byteCodeGenerator->FindSymbol(pnode->sxPid.symRef, pnode->sxPid.pid);
            if (sym)
            {
                // This is a named load, not just a reference, so if it's a nested function note that all
                // the nested scopes escape.
                Assert(byteCodeGenerator->IsLanguageServiceEnabled() ||
                       !sym->GetDecl() || (pnode->sxPid.symRef && *pnode->sxPid.symRef));
                Assert(byteCodeGenerator->IsLanguageServiceEnabled() ||
                       !sym->GetDecl() || ((*pnode->sxPid.symRef)->GetDecl() == sym->GetDecl()));

                pnode->sxPid.sym = sym;
                if (sym->GetSymbolType() == STFunction &&
                    (!sym->GetIsGlobal() || (byteCodeGenerator->GetFlags() & fscrEval)))
                {
                    byteCodeGenerator->FuncEscapes(sym->GetScope());
                }
            }
        }
        {
            FuncInfo *top = byteCodeGenerator->TopFuncInfo();
            if (pnode->sxPid.sym == null || pnode->sxPid.sym->GetIsGlobal())
            {
                top->SetHasGlobalRef(true);
            }
            if (pnode->sxPid.sym)
            {
                pnode->sxPid.sym->SetIsUsed(TRUE);
            }
        }
        break;
    case knopMember:
        if (pnode->sxBin.pnode1->nop == knopComputedName)
        {
            // Computed property name - cannot bind yet
            break;
        }
        // fall through
    case knopGetMember:
    case knopSetMember:
        {
            // lhs is knopStr, rhs is expr
            ParseNode *id=pnode->sxBin.pnode1;
            if (id->nop == knopStr || id->nop == knopName)
            {
                byteCodeGenerator->AssignPropertyId(id->sxPid.pid);
                id->sxPid.sym = NULL;
                id->sxPid.symRef = NULL;
                id->grfpn |= fpnMemberReference;
            }
            break;
        }
        // TODO: convert index over string to Get/Put Value
    case knopIndex:
        BindReference(pnode, byteCodeGenerator);
        break;
    case knopDot:
        BindInstAndMember(pnode,byteCodeGenerator);
        break;
    case knopTryFinally:
        byteCodeGenerator->SetHasFinally(true);
    case knopTryCatch:
        byteCodeGenerator->SetHasTry(true);
        byteCodeGenerator->TopFuncInfo()->byteCodeFunction->SetDontInline(true);
        byteCodeGenerator->AddTargetStmt(pnode);
        break;
    case knopAsg:
        BindReference(pnode, byteCodeGenerator);
        CheckLocalVarDef(pnode,byteCodeGenerator);
        break;
    case knopVarDecl:
        // "arguments" symbol or decl w/o rhs may have been bound already; otherwise, do the binding here.
        if (pnode->sxVar.sym == NULL)
        {
            pnode->sxVar.sym = byteCodeGenerator->FindSymbol(pnode->sxVar.symRef, pnode->sxVar.pid);
        }
        SetAdditionalBindInfoForVariables(pnode, byteCodeGenerator);
        break;
    case knopConstDecl:
    case knopLetDecl:
        // "arguments" symbol or decl w/o rhs may have been bound already; otherwise, do the binding here.
        if (!pnode->sxVar.sym)
        {
            AssertMsg(pnode->sxVar.symRef && *pnode->sxVar.symRef, "'const' and 'let' should be binded when we bind block");
            pnode->sxVar.sym = *pnode->sxVar.symRef;
        }
        SetAdditionalBindInfoForVariables(pnode, byteCodeGenerator);
        break;
    case knopCall:
        if (pnode->sxCall.isEvalCall && byteCodeGenerator->TopFuncInfo()->IsLambda())
        {
            byteCodeGenerator->MarkThisUsedInLambda();
        }
        // fallthrough
    case knopTypeof:
    case knopDelete:
        BindReference(pnode, byteCodeGenerator);
        break;
    case knopRegExp:
        pnode->sxPid.regexPatternIndex = byteCodeGenerator->TopFuncInfo()->GetParsedFunctionBody()->NewLiteralRegex();
        break;

    case knopComma:
        pnode->sxBin.pnode1->SetNotEscapedUse();
        break;

    case knopBlock:
    {
        for (ParseNode *pnodeScope = pnode->sxBlock.pnodeScopes; pnodeScope;)
        {
            switch (pnodeScope->nop)
            {
            case knopFncDecl:
                if (pnodeScope->sxFnc.IsDeclaration())
                {
                    byteCodeGenerator->ProcessCapturedSyms(pnodeScope);
                }
                pnodeScope = pnodeScope->sxFnc.pnodeNext;
                break;

            case knopBlock:
                pnodeScope = pnodeScope->sxBlock.pnodeNext;
                break;

            case knopCatch:
                pnodeScope = pnodeScope->sxCatch.pnodeNext;
                break;

            case knopWith:
                pnodeScope = pnodeScope->sxWith.pnodeNext;
                break;
            }
        }
        break;
    }

    }
}

void ByteCodeGenerator::ProcessCapturedSyms(ParseNode *pnode)
{
    SymbolTable *capturedSyms = pnode->sxFnc.funcInfo->GetCapturedSyms();
    if (capturedSyms)
    {
        FuncInfo *funcInfo = this->TopFuncInfo();
        CapturedSymMap *capturedSymMap = funcInfo->EnsureCapturedSymMap();
        ParseNode *pnodeStmt = this->GetCurrentTopStatement();

        SList<Symbol*> *capturedSymList;
        if (!pnodeStmt->CapturesSyms())
        {
            capturedSymList = Anew(this->alloc, SList<Symbol*>, this->alloc);
            capturedSymMap->Add(pnodeStmt, capturedSymList);
            pnodeStmt->SetCapturesSyms();
        }
        else
        {
            capturedSymList = capturedSymMap->Item(pnodeStmt);
        }

        capturedSyms->Map([&](Symbol *sym) {
            if (!sym->GetIsCommittedToSlot() && !sym->HasVisitedCapturingFunc())
            {
                capturedSymList->Prepend(sym);
                sym->SetHasVisitedCapturingFunc();
            }
        });
    }
}

CapturedSymMap *FuncInfo::EnsureCapturedSymMap()
{
    if (this->capturedSymMap == nullptr)
    {
        this->capturedSymMap = Anew(alloc, CapturedSymMap, alloc);
    }
    return this->capturedSymMap;
}

void ByteCodeGenerator::MarkThisUsedInLambda()
{
    // This is a lambda that refers to "this".
    // Find the enclosing "normal" function and indicate that the lambda captures the enclosing function's "this".
    FuncInfo *parent = this->FindEnclosingNonLambda();
    parent->GetParsedFunctionBody()->SetHasThis(true);
    if (!parent->IsGlobalFunction() || this->GetFlags() & fscrEval)
    {
        // If the enclosing function is non-global or eval global, it will put "this" in a closure slot.
        parent->SetIsThisLexicallyCaptured();
        Scope* scope = parent->IsGlobalFunction() ? parent->GetGlobalEvalBlockScope() : parent->GetBodyScope();
        scope->SetHasLocalInClosure(true);
    }

    if (!parent->IsGlobalFunction() || this->flags & fscrEval)
    {
        this->TopFuncInfo()->SetHasClosureReference(true);
    }

    this->TopFuncInfo()->SetHasCapturedThis();
}

void ByteCodeGenerator::FuncEscapes(Scope *scope)
{
    while (scope)
    {
        Assert(scope->GetFunc());
        scope->GetFunc()->SetEscapes(true);
        scope = scope->GetEnclosingScope();
    }

    if (this->flags & fscrEval)
    {
        // If a function declared inside eval escapes, we'll need
        // to invalidate the caller's cached scope.
        this->funcEscapes = true;
    }
}

unsigned int CountArguments(ParseNode *pnode, BOOL *pSideEffect = NULL)
{
    // If the caller passed us a pSideEffect, it wants to know whether there are potential
    // side-effects in the argument list. We need to know this so that the call target
    // operands can be preserved if necessary.
    // For now, treat any non-leaf op as a potential side-effect. This causes no detectable slowdowns,
    // but we can be more precise if we need to be.
    if (pSideEffect)
    {
        *pSideEffect = FALSE;
    }
    unsigned int argCount=1;
    if (pnode!=NULL)
    {
        while(pnode->nop==knopList)
        {
            argCount++;
            if (pSideEffect && !(ParseNode::Grfnop(pnode->sxBin.pnode1->nop) & fnopLeaf))
            {
                *pSideEffect = TRUE;
            }
            pnode = pnode->sxBin.pnode2;
        }
        argCount++;
        if (pSideEffect && !(ParseNode::Grfnop(pnode->nop) & fnopLeaf))
        {
            *pSideEffect = TRUE;
        }
    }
    return argCount;
}

void SaveOpndValue(ParseNode *pnode, FuncInfo *funcInfo)
{
    // Save a local name to a register other than its home location.
    // This guards against side-effects in cases like x.foo(x = bar()).
    if (pnode->nop != knopName)
    {
        return;
    }

    Symbol *sym = pnode->sxPid.sym;
    if (sym == null)
    {
        return;
    }

    // If the target is a local being kept in its home location,
    // protect the target's value in the event the home location is overwritten.
    if (pnode->location != Js::Constants::NoRegister &&
        sym->GetScope()->GetFunc() == funcInfo &&
        pnode->location == sym->GetLocation())
    {
        pnode->location = funcInfo->AcquireTmpRegister();
    }
}

void ByteCodeGenerator::StartStatement(ParseNode* node)
{
    Assert(TopFuncInfo() != NULL);
    m_writer.StartStatement(node, TopFuncInfo()->curTmpReg - TopFuncInfo()->firstTmpReg);
}

void ByteCodeGenerator::EndStatement(ParseNode* node)
{
    m_writer.EndStatement(node);
}

void ByteCodeGenerator::StartSubexpression(ParseNode* node)
{
    Assert(TopFuncInfo() != NULL);
    m_writer.StartSubexpression(node);
}

void ByteCodeGenerator::EndSubexpression(ParseNode* node)
{
    m_writer.EndSubexpression(node);
}

void Symbol::SetHasMaybeEscapedUse(ByteCodeGenerator * byteCodeGenerator)
{
    Assert(!this->GetIsMember());
    if (!hasMaybeEscapedUse)
    {
        SetHasMaybeEscapedUseInternal(byteCodeGenerator);
    }
}

void Symbol::SetHasMaybeEscapedUseInternal(ByteCodeGenerator * byteCodeGenerator)
{
    Assert(!hasMaybeEscapedUse);
    Assert(!this->GetIsFormal());
    hasMaybeEscapedUse = true;
    if (PHASE_TESTTRACE(Js::StackFuncPhase, byteCodeGenerator->TopFuncInfo()->byteCodeFunction))
    {
        Output::Print(L"HasMaybeEscapedUse: %s\n", this->GetName().GetBuffer());
        Output::Flush();
    }
    if (this->GetHasFuncAssignment())
    {
        this->GetScope()->GetFunc()->SetHasMaybeEscapedNestedFunc(
            DebugOnly(this->symbolType == STFunction? L"MaybeEscapedUseFuncDecl" : L"MaybeEscapedUse"));
    }
}

void Symbol::SetHasFuncAssignment(ByteCodeGenerator * byteCodeGenerator)
{
    Assert(!this->GetIsMember());
    if (!hasFuncAssignment)
    {
        SetHasFuncAssignmentInternal(byteCodeGenerator);
    }
}

void Symbol::SetHasFuncAssignmentInternal(ByteCodeGenerator * byteCodeGenerator)
{
    Assert(!hasFuncAssignment);
    hasFuncAssignment = true;
    FuncInfo * top = byteCodeGenerator->TopFuncInfo();
    if (PHASE_TESTTRACE(Js::StackFuncPhase, top->byteCodeFunction))
    {
        Output::Print(L"HasFuncAssignment: %s\n", this->GetName().GetBuffer());
        Output::Flush();
    }

    if (this->GetHasMaybeEscapedUse() || this->GetScope()->GetIsObject())
    {
        byteCodeGenerator->TopFuncInfo()->SetHasMaybeEscapedNestedFunc(DebugOnly(
            this->GetIsFormal()? L"FormalAssignment" :
            this->GetScope()->GetIsObject()? L"ObjectScopeAssignment" :
            L"MaybeEscapedUse"));
    }
}

bool ByteCodeGenerator::HasInterleavingDynamicScope(Symbol * sym) const
{
    Js::PropertyId unused;
    return this->InDynamicScope() &&
        sym->GetScope() != this->FindScopeForSym(sym->GetScope(), NULL, &unused, this->TopFuncInfo());
}

void Symbol::RestoreHasFuncAssignment()
{
    Assert(hasFuncAssignment == (this->symbolType == STFunction));
    Assert(this->GetIsFormal() || !this->GetHasMaybeEscapedUse());
    hasFuncAssignment = true;
    if (PHASE_TESTTRACE1(Js::StackFuncPhase))
    {
        Output::Print(L"RestoreHasFuncAssignment: %s\n", this->GetName().GetBuffer());
        Output::Flush();
    }
}

Symbol * Symbol::GetFuncScopeVarSym() const
{
    if (!this->GetIsBlockVar())
    {
        return null;
    }
    FuncInfo * parentFuncInfo = this->GetScope()->GetFunc();
    if (parentFuncInfo->GetIsStrictMode())
    {
        return null;
    }
    Symbol *fncScopeSym = parentFuncInfo->GetBodyScope()->FindLocalSymbol(this->GetName());
    if (fncScopeSym == nullptr && parentFuncInfo->GetParamScope() != nullptr)
    {
        // We couldn't find the sym in the body scope, try finding it in the parameter scope.
        fncScopeSym = parentFuncInfo->GetParamScope()->FindLocalSymbol(this->GetName());
    }
    Assert(BinaryFeatureControl::LanguageService() || fncScopeSym);
    // Parser should have added a fake var decl node for block scoped functions in non-strict mode
    // IsBlockVar() indicates a user let declared variable at function scope which
    // shadows the function's var binding, thus only emit the var binding init if
    // we do not have a block var symbol.
    if (!fncScopeSym || fncScopeSym->GetIsBlockVar())
    {
        return nullptr;
    }
    return fncScopeSym;

}

void FuncInfo::SetHasMaybeEscapedNestedFunc(DebugOnly(wchar_t const * reason))
{
    if (PHASE_TESTTRACE(Js::StackFuncPhase, this->byteCodeFunction) && !hasEscapedUseNestedFunc)
    {
        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
        wchar_t const * r = L"";

        DebugOnly(r = reason);
        Output::Print(L"HasMaybeEscapedNestedFunc (%s): %s (function %s)\n",
            r,
            this->byteCodeFunction->GetDisplayName(),
            this->byteCodeFunction->GetDebugNumberSet(debugStringBuffer));
        Output::Flush();
    }
    hasEscapedUseNestedFunc = true;
}

void CheckMaybeEscapedUse(ParseNode * pnode, ByteCodeGenerator * byteCodeGenerator, bool isCall = false)
{
    if (pnode == null)
    {
        return;
    }

    FuncInfo * topFunc = byteCodeGenerator->TopFuncInfo();
    if (topFunc->IsGlobalFunction())
    {
        return;
    }

    switch(pnode->nop)
    {
    case knopAsg:
        if (pnode->sxBin.pnode1->nop != knopName)
        {
            break;
        }
        // use of an assignemnt assignment (e.g.  (y = function() {}) + "1"), just make y an escaped use.
        pnode = pnode->sxBin.pnode1;
        isCall = false;
        // fall thru
    case knopName:
        if (!isCall)
        {
            // Mark the name has having escaped use
            if (pnode->sxPid.sym)
            {
                pnode->sxPid.sym->SetHasMaybeEscapedUse(byteCodeGenerator);
            }
        }
        break;

    case knopFncDecl:
        // A function declaration has an unknown use (not assignment nor call),
        // mark the function as having childe escaped
        topFunc->SetHasMaybeEscapedNestedFunc(DebugOnly(L"UnknownUse"));
        break;
    }
}

void CheckFuncAssignment(Symbol * sym, ParseNode * pnode2, ByteCodeGenerator * byteCodeGenerator)
{
    if (pnode2 == null)
    {
        return;
    }

    switch (pnode2->nop)
    {
    default:
        CheckMaybeEscapedUse(pnode2, byteCodeGenerator);
        break;
    case knopFncDecl:
        {
            FuncInfo * topFunc = byteCodeGenerator->TopFuncInfo();
            if (topFunc->IsGlobalFunction())
            {
                return;
            }
            // Use not as an assignemnt or assignment to an outer function's sym, or assigned to a formal
            // or assigned to multiple names

            if (sym == null
                || sym->GetScope()->GetFunc() != topFunc)
            {
                topFunc->SetHasMaybeEscapedNestedFunc(DebugOnly(
                    sym == null? L"UnknownAssignment" :
                    (sym->GetScope()->GetFunc() != topFunc)? L"CrossFuncAssignment" :
                    L"SomethingIsWrong!")
                    );
            }
            else
            {
                // TODO-STACK-NESTED-FUNC: Since we only support single def functions, we can still put the
                // nested function on the stack and reuse even if the function go out of the block scope.
                // However, we cannot allocate frame display or slots on the stack if the function is
                // declared in a loop, because there might be multiple function referencing different
                // iteration of the scope.
                // For now, just disable everything.

                Scope * funcParentScope = pnode2->sxFnc.funcInfo->GetBodyScope()->GetEnclosingScope();
                while (sym->GetScope() != funcParentScope)
                {
                    if (funcParentScope->GetMustInstantiate())
                    {
                        topFunc->SetHasMaybeEscapedNestedFunc(DebugOnly(L"CrossScopeAssignment"));
                        break;
                    }
                    funcParentScope->SetHasCrossScopeFuncAssignment();
                    funcParentScope = funcParentScope->GetEnclosingScope();
                }

                // Need to always detect interleaving dynamic scope ('with')  for assignments
                // as those may end up escaping into the 'with' scope
                // TODO: the with scope is marked as MustInstaniate late during byte code emit
                // We could detect this using the loop above as well, by marking the with
                // scope as must instantiate early, this is just less risky of a fix for RTM

                if (byteCodeGenerator->HasInterleavingDynamicScope(sym))
                {
                     byteCodeGenerator->TopFuncInfo()->SetHasMaybeEscapedNestedFunc(DebugOnly(L"InterleavingDynamicScope"));
                }

                sym->SetHasFuncAssignment(byteCodeGenerator);
            }
        }
        break;
    };
}

inline bool ContainsSuperCallReference(ParseNodePtr pnode)
{
    return pnode->sxCall.pnodeTarget->nop == knopSuper // super()
           || (pnode->sxCall.pnodeTarget->nop == knopDot && pnode->sxCall.pnodeTarget->sxBin.pnode1->nop == knopSuper) // super.prop()
           || (pnode->sxCall.pnodeTarget->nop == knopIndex && pnode->sxCall.pnodeTarget->sxBin.pnode1->nop == knopSuper); // super[prop]()
}

// Assign permanent (non-temp) registers for the function.
// These include constants (null, 3.7, this) and locals that use registers as their home locations.
// Assign the location fields of parse nodes whose values are constants/locals with permanent/known registers.
// Re-usable expression temps are assigned during the final Emit pass.
void AssignRegisters(ParseNode *pnode,ByteCodeGenerator *byteCodeGenerator)
{
    if (pnode==NULL)
        return;

    Symbol *sym;
    OpCode nop = pnode->nop;
    switch (nop) {
    default:
        {
            uint flags = ParseNode::Grfnop(nop);
            if (flags & fnopUni)
            {
                CheckMaybeEscapedUse(pnode->sxUni.pnode1, byteCodeGenerator);
            }
            else if (flags & fnopBin)
            {
                CheckMaybeEscapedUse(pnode->sxBin.pnode1, byteCodeGenerator);
                CheckMaybeEscapedUse(pnode->sxBin.pnode2, byteCodeGenerator);
            }
        }
        break;
    case knopDot:
    case knopScope:
        CheckMaybeEscapedUse(pnode->sxBin.pnode1, byteCodeGenerator);
        break;
    case knopMember:
    case knopGetMember:
    case knopSetMember:
        CheckMaybeEscapedUse(pnode->sxBin.pnode2, byteCodeGenerator);
        break;
    case knopAsg:
        {
            Symbol * sym = pnode->sxBin.pnode1->nop == knopName? pnode->sxBin.pnode1->sxPid.sym : null;
            CheckFuncAssignment(sym, pnode->sxBin.pnode2, byteCodeGenerator);

            if (pnode->IsInList())
            {
                // Assignment in array literal
                CheckMaybeEscapedUse(pnode->sxBin.pnode1, byteCodeGenerator);
            }

            if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsES6DestructuringEnabled() && pnode->sxBin.pnode1->nop == knopArray)
            {
                // Destructured arrays may have default values and need undefined.
                byteCodeGenerator->AssignUndefinedConstRegister();

                // Any rest parameter in a destructured array will need a 0 constant.
                byteCodeGenerator->EnregisterConstant(0);
            }
        }
        break;
    case knopEllipsis:
        if (byteCodeGenerator->InDestructuredArray())
        {
            // Get a register for the rest array counter.
            pnode->location = byteCodeGenerator->NextVarRegister();
        }
        CheckMaybeEscapedUse(pnode->sxUni.pnode1, byteCodeGenerator);
        break;

    case knopQmark:
        CheckMaybeEscapedUse(pnode->sxTri.pnode1, byteCodeGenerator);
        CheckMaybeEscapedUse(pnode->sxTri.pnode2, byteCodeGenerator);
        CheckMaybeEscapedUse(pnode->sxTri.pnode3, byteCodeGenerator);
        break;
    case knopWith:
        pnode->location = byteCodeGenerator->NextVarRegister();
        CheckMaybeEscapedUse(pnode->sxWith.pnodeObj, byteCodeGenerator);
        break;
    case knopComma:
        if (!pnode->IsNotEscapedUse())
        {
            // only the last expr in comma expr escape.  mark it if it is escapable
            CheckMaybeEscapedUse(pnode->sxBin.pnode2, byteCodeGenerator);
        }
        break;
    case knopFncDecl:
        if (!byteCodeGenerator->TopFuncInfo()->IsGlobalFunction())
        {
            if (pnode->sxFnc.IsGenerator())
            {
                // Assume generators always escape; otherwise need to analyze if
                // the return value of calls to generator function, the generator
                // objects, escape.
                FuncInfo* funcInfo = byteCodeGenerator->TopFuncInfo();
                funcInfo->SetHasMaybeEscapedNestedFunc(DebugOnly(L"Generator"));
            }

            if (pnode->IsInList() && !pnode->IsNotEscapedUse())
            {
                byteCodeGenerator->TopFuncInfo()->SetHasMaybeEscapedNestedFunc(DebugOnly(L"InList"));
            }

            ParseNodePtr pnodeNames = pnode->sxFnc.pnodeNames;
            if (pnodeNames != null)
            {
                // There is a weird case in compat v8 mode where we may not have a sym assigned to a fnc decl's
                // name node if it is a named function declare inside 'with' that also assigned to something else
                // as well. Instead, We generate two knopFncDecl node one for parent function and one for the assignment
                // Only the top one get a sym, not the inner one.  The assignment in the 'with' will be using the inner
                // one.  Also we will detect that the assignment to a variable is an escape inside a with
                // Since we need the sym in the fnc decl's name, we just detect the escape here as "WithScopeFuncName"

                if (pnodeNames->nop == knopVarDecl && pnodeNames->sxVar.sym != null)
                {
                    // Unlike in CheckFuncAssignemnt, we don't have check if there is a interleaving
                    // dynamic scope ('with') here, because we also generate direct assignment for
                    // function decl's names

                    pnodeNames->sxVar.sym->SetHasFuncAssignment(byteCodeGenerator);

                    // Function decoration in block scope and non-strict mode has a
                    // corresponding var sym that we assign to as well.  Need to
                    // mark that symbol as has func assignment as well.
                    Symbol * functionScopeVarSym = pnodeNames->sxVar.sym->GetFuncScopeVarSym();
                    if (functionScopeVarSym)
                    {
                        functionScopeVarSym->SetHasFuncAssignment(byteCodeGenerator);
                    }
                }
                else
                {
                    // the function has multiple names, or assign to o.x or o::x
                    byteCodeGenerator->TopFuncInfo()->SetHasMaybeEscapedNestedFunc(DebugOnly(
                        pnodeNames->nop == knopList? L"MultipleFuncName" :
                        pnodeNames->nop == knopDot? L"PropFuncName" :
                        pnodeNames->nop == knopScope? L"ScopeFuncName" :
                        pnodeNames->nop == knopVarDecl && pnodeNames->sxVar.sym == null? L"WithScopeFuncName" :
                        L"WeirdFuncName"
                    ));
                }
            }
        }

        break;
    case knopNew:
        CheckMaybeEscapedUse(pnode->sxCall.pnodeTarget, byteCodeGenerator);
        CheckMaybeEscapedUse(pnode->sxCall.pnodeArgs, byteCodeGenerator);
        break;
    case knopThrow:
        CheckMaybeEscapedUse(pnode->sxUni.pnode1, byteCodeGenerator);
        break;

    // REVIEW: Technically, switch expr or case expr doesn't really escape as strict equal
    // doesn't cause the function to escape.  But we just don't care about these cases
    case knopSwitch:
        CheckMaybeEscapedUse(pnode->sxSwitch.pnodeVal, byteCodeGenerator);
        break;
    case knopCase:
        CheckMaybeEscapedUse(pnode->sxCase.pnodeExpr, byteCodeGenerator);
        break;
    // REVIEW: Technically, the object for GetForInEnumerator doesn't escape, except when cached.
    // which we can made to work.  But we don't really care about this corner case
    case knopForIn:
        CheckMaybeEscapedUse(pnode->sxForInOrForOf.pnodeObj, byteCodeGenerator);
        break;

    case knopForOf:
        byteCodeGenerator->AssignNullConstRegister();
        byteCodeGenerator->AssignUndefinedConstRegister();
        CheckMaybeEscapedUse(pnode->sxForInOrForOf.pnodeObj, byteCodeGenerator);
        break;

    case knopTrue:
        pnode->location = byteCodeGenerator->AssignTrueConstRegister();
        break;

    case knopFalse:
        pnode->location = byteCodeGenerator->AssignFalseConstRegister();
        break;

    case knopDecPost:
    case knopIncPost:
    case knopDecPre:
    case knopIncPre:
        byteCodeGenerator->EnregisterConstant(1);
        CheckMaybeEscapedUse(pnode->sxUni.pnode1, byteCodeGenerator);
        break;
    case knopObject:
        byteCodeGenerator->AssignNullConstRegister();
        break;
    case knopClassDecl:
        {
            FuncInfo * topFunc = byteCodeGenerator->TopFuncInfo();
            topFunc->SetHasMaybeEscapedNestedFunc(DebugOnly(L"Class"));

            // We may need undefined for the 'this', e.g. calling a class expression
            byteCodeGenerator->AssignUndefinedConstRegister();
        }
        break;
    case knopNull:
        pnode->location = byteCodeGenerator->AssignNullConstRegister();
        break;
    case knopThis:
        {
            FuncInfo* func = byteCodeGenerator->TopFuncInfo();
            pnode->location = func->AssignThisRegister();
            if (func->IsLambda())
            {
                func = byteCodeGenerator->FindEnclosingNonLambda();
                func->AssignThisRegister();

                if (func->IsGlobalFunction() && !(byteCodeGenerator->GetFlags() & fscrEval))
                {
                    byteCodeGenerator->AssignNullConstRegister();
                }
            }
            // "this" should be loaded for both global and non global functions
            if (func->IsGlobalFunction() && !(byteCodeGenerator->GetFlags() & fscrEval))
            {
                // We'll pass "null" to LdThis, to simulate "null" passed as "this" to the
                // global function.
                func->AssignNullConstRegister();
            }
            break;
        }
    case knopSuper:
    {
        FuncInfo* func = byteCodeGenerator->TopFuncInfo();
        pnode->location = func->AssignSuperRegister();
        func->AssignThisRegister();

        FuncInfo* nonLambdaFunc = func;
        if (func->IsLambda())
        {
            // If this is a lambda inside a class member, the class member will need to load super.
            nonLambdaFunc = byteCodeGenerator->FindEnclosingNonLambda();

            nonLambdaFunc->root->sxFnc.SetHasSuperReference();
            nonLambdaFunc->AssignSuperRegister();
            nonLambdaFunc->AssignThisRegister();
            nonLambdaFunc->SetIsSuperLexicallyCaptured();
            nonLambdaFunc->GetBodyScope()->SetHasLocalInClosure(true);
            func->SetHasClosureReference(true);
        }

        if (nonLambdaFunc->IsGlobalFunction())
        {
            if (!(byteCodeGenerator->GetFlags() & fscrEval))
            {
                // Enable LdSuper for global function to support subsequent emission of call, dot, prop, etc. related to super
                func->AssignNullConstRegister();
                nonLambdaFunc->AssignNullConstRegister();
            }
        }
        else if (!func->IsClassMember())
        {
            func->AssignUndefinedConstRegister();
        }
        break;
    }
    case knopCall:
    {
        if (pnode->sxCall.pnodeTarget->nop != knopIndex &&
            pnode->sxCall.pnodeTarget->nop != knopDot &&
            pnode->sxCall.pnodeTarget->nop != knopScope)
        {
            byteCodeGenerator->AssignUndefinedConstRegister();
        }
        FuncInfo *funcInfo = byteCodeGenerator->TopFuncInfo();
        if (ContainsSuperCallReference(pnode))
        {
            // A super call requires 'this' to be available.
            byteCodeGenerator->SetNeedEnvRegister();
            byteCodeGenerator->AssignThisRegister();
            if (funcInfo->IsLambda())
            {
                // If this is a lambda inside a class member, the class member will need to load super.
                FuncInfo *parent = byteCodeGenerator->FindEnclosingNonLambda();
                if (parent->root->sxFnc.IsClassMember())
                {
                    // Set up super reference
                    parent->root->sxFnc.SetHasSuperReference();
                    parent->AssignSuperRegister();
                    parent->SetIsSuperLexicallyCaptured();

                    parent->GetBodyScope()->SetHasLocalInClosure(true);
                    funcInfo->SetHasClosureReference(true);
                }

                parent->AssignThisRegister();
                byteCodeGenerator->MarkThisUsedInLambda();
            }
        }
        if (pnode->sxCall.isEvalCall)
        {
            if(!funcInfo->GetParsedFunctionBody()->IsReparsed())
            {
                Assert(funcInfo->IsGlobalFunction() || funcInfo->GetCallsEval());
                funcInfo->SetCallsEval(true);
                funcInfo->GetParsedFunctionBody()->SetCallsEval(true);
            }
            else
            {
                // On reparsing - load the state from function Body - instead of using the state on the parse node
                // as they might be different
                pnode->sxCall.isEvalCall = funcInfo->GetParsedFunctionBody()->GetCallsEval();
            }

            if (funcInfo->IsLambda() && pnode->sxCall.isEvalCall)
            {
                FuncInfo* nonLambdaParent = byteCodeGenerator->FindEnclosingNonLambda();
                if (!nonLambdaParent->IsGlobalFunction() || (byteCodeGenerator->GetFlags() & fscrEval))
                {
                    nonLambdaParent->AssignThisRegister();
                }
            }

            // An eval call in a class member needs to load super.
            if (funcInfo->root->sxFnc.IsClassMember())
            {
                funcInfo->AssignSuperRegister();
            }
            else if (funcInfo->IsLambda())
            {
                // If this is a lambda inside a class member, the class member will need to load super.
                FuncInfo *parent = byteCodeGenerator->FindEnclosingNonLambda();
                if (parent->root->sxFnc.IsClassMember())
                {
                    parent->root->sxFnc.SetHasSuperReference();
                    parent->AssignSuperRegister();
                }
            }
        }
        // Don't need to check pnode->sxCall.pnodeTarget even if it is a knopFncDecl,
        // (e.g. (function(){})();
        // It is only used as a call, so don't count as an escape.
        // Although not assigned to a slot, we will still able to box it by boxing
        // all the stack function on the interpreter frame or the stack function link list
        // on a jitted frame
        break;
    }

    case knopInt:
        pnode->location = byteCodeGenerator->EnregisterConstant(pnode->sxInt.lw);
        break;
    case knopFlt:{
        pnode->location = byteCodeGenerator->EnregisterDoubleConstant(pnode->sxFlt.dbl);
        break;
    }
    case knopStr:
        pnode->location = byteCodeGenerator->EnregisterStringConstant(pnode->sxPid.pid);
        break;
    case knopVarDecl:
    case knopConstDecl:
    case knopLetDecl:
        {
            sym = pnode->sxVar.sym;
            Assert(sym->GetScope()->GetEnclosingFunc() == byteCodeGenerator->TopFuncInfo());
            Assert(sym!=NULL);

            if (pnode->sxVar.isBlockScopeFncDeclVar && sym->GetIsBlockVar())
            {
                break;
            }

            if (!sym->GetIsGlobal())
            {
                FuncInfo *funcInfo = byteCodeGenerator->TopFuncInfo();

                // Check the function assignment for the sym that we have, even if we we remap it to function level sym below
                // as we are going assign to the original sym
                CheckFuncAssignment(sym, pnode->sxVar.pnodeInit, byteCodeGenerator);

                if (sym->GetIsCatch() || (pnode->nop == knopVarDecl && sym->GetIsBlockVar() && !pnode->sxVar.isBlockScopeFncDeclVar))
                {
                    // The lhs of the var decl really binds to the local symbol, not the catch or let symbol.
                    // But the assignment will go to the catch or let symbol. Just assign a register to the local
                    // so that it can get initialized to undefined.
#if DBG
                    if (!sym->GetIsCatch())
                    {
                        Assert(BinaryFeatureControl::LanguageService() || funcInfo->bodyScope != sym->GetScope() || !byteCodeGenerator->GetScriptContext()->GetConfig()->IsBlockScopeEnabled());  // catch cannot be at function scope and let and var at function scope is redeclaration error
                    }
#endif                    
                    auto symName = sym->GetName();
                    sym = funcInfo->bodyScope->FindLocalSymbol(symName);
                    if (sym == nullptr)
                    {
                        sym = funcInfo->paramScope->FindLocalSymbol(symName);
                    }
                    Assert(BinaryFeatureControl::LanguageService() || (sym && !sym->GetIsCatch() && !sym->GetIsBlockVar()));
                }
                // Don't give the declared var a register if it's in a closure, because the closure slot
                // is its true "home". (Need to check "is global" again as the sym may have changed above.)
                if (!sym->GetIsGlobal() && !sym->IsInSlot(funcInfo))
                {
                    if(PHASE_TRACE(Js::DelayCapturePhase, funcInfo->byteCodeFunction))
                    {
                        if(sym->NeedsSlotAlloc(byteCodeGenerator->TopFuncInfo()))
                        {
                            Output::Print(L"--- DelayCapture: Delayed capturing symbol '%s' during initialization.\n", sym->GetName());
                            Output::Flush();
                        }
                    }
                    byteCodeGenerator->AssignRegister(sym);
                }
            }
            else
            {
                Assert(byteCodeGenerator->TopFuncInfo()->IsGlobalFunction());
            }

            break;
        }
    case knopFor:
        if ((pnode->sxFor.pnodeBody!=NULL)&&(pnode->sxFor.pnodeBody->nop==knopBlock)&&
            (pnode->sxFor.pnodeBody->sxBlock.pnodeStmt!=NULL)&&
            (pnode->sxFor.pnodeBody->sxBlock.pnodeStmt->nop==knopFor)&&
            (!byteCodeGenerator->IsInDebugMode())) {
                FuncInfo *funcInfo = byteCodeGenerator->TopFuncInfo();
                pnode->sxFor.pnodeInverted=InvertLoop(pnode,byteCodeGenerator,funcInfo);
        }
        else {
            pnode->sxFor.pnodeInverted=NULL;
        }
        break;

    case knopName:
        sym = pnode->sxPid.sym;
        if (sym == NULL)
        {
            Assert(pnode->sxPid.pid->GetPropertyId() != Js::Constants::NoProperty);
        }
        else
        {
            // Note: don't give a register to a local if it's in a closure, because then the closure
            // is its true home.
            if (!sym->GetIsGlobal() &&
                !sym->GetIsMember() &&
                byteCodeGenerator->TopFuncInfo() == sym->GetScope()->GetEnclosingFunc() &&
                !sym->IsInSlot(byteCodeGenerator->TopFuncInfo()) &&
                !sym->HasVisitedCapturingFunc())
            {
                if(PHASE_TRACE(Js::DelayCapturePhase, byteCodeGenerator->TopFuncInfo()->byteCodeFunction))
                {
                    if(sym->NeedsSlotAlloc(byteCodeGenerator->TopFuncInfo()))
                    {
                        Output::Print(L"--- DelayCapture: Delayed capturing symbol '%s'.\n", sym->GetName());
                        Output::Flush();
                    }
                }

                // Local symbol being accessed in its own frame. Even if "with" or event
                // handler semantics make the binding ambiguous, it has a home location,
                // so assign it.
                byteCodeGenerator->AssignRegister(sym);

                // If we're in something like a "with" we'll need a scratch register to hold
                // the multiple possible values of the property.
                if (!byteCodeGenerator->HasInterleavingDynamicScope(sym))
                {
                    // We're not in a dynamic scope, or our home scope is nested within the dynamic scope, so we
                    // don't have to do dynamic binding. Just use the home location for this reference.
                    pnode->location = sym->GetLocation();
                }
            }
        }
        if (pnode->IsInList() && !pnode->IsNotEscapedUse())
        {
            // A node that is in a list is assumed to be escape, unless marked otherwise.
            // This includes array literal list/object literal list
            CheckMaybeEscapedUse(pnode, byteCodeGenerator);
        }
        break;

    case knopProg:
        if (!byteCodeGenerator->HasParentScopeInfo())
        {
            // If we're compiling a nested deferred function, don't pop the scope stack,
            // because we just want to leave it as is for the emit pass.
            PostVisitFunction(pnode, byteCodeGenerator);
        }
        break;
    case knopReturn:
        {
            ParseNode *pnodeExpr = pnode->sxReturn.pnodeExpr;
            CheckMaybeEscapedUse(pnodeExpr, byteCodeGenerator);
            break;
        }
    case knopBlock:
        {
            if (pnode->sxBlock.scope && pnode->sxBlock.scope->GetMustInstantiate() && byteCodeGenerator->IsInDebugMode())
            {
                pnode->sxBlock.scope->SetLocation(byteCodeGenerator->NextVarRegister());
            }
            break;
        }
    case knopStrTemplate:
        {
            ParseNode* pnodeExprs = pnode->sxStrTemplate.pnodeSubstitutionExpressions;
            if (pnodeExprs != nullptr)
            {
                while (pnodeExprs->nop == knopList)
                {
                    Assert(pnodeExprs->sxBin.pnode1 != nullptr);
                    Assert(pnodeExprs->sxBin.pnode2 != nullptr);

                    CheckMaybeEscapedUse(pnodeExprs->sxBin.pnode1, byteCodeGenerator);
                    pnodeExprs = pnodeExprs->sxBin.pnode2;
                }

                // Also check the final element in the list
                CheckMaybeEscapedUse(pnodeExprs, byteCodeGenerator);
            }

            if (pnode->sxStrTemplate.isTaggedTemplate)
            {
                pnode->location = byteCodeGenerator->EnregisterStringTemplateCallsiteConstant(pnode);
            }
            break;
        }
    case knopYieldLeaf:
        byteCodeGenerator->AssignUndefinedConstRegister();
        break;
    case knopYield:
        CheckMaybeEscapedUse(pnode->sxUni.pnode1, byteCodeGenerator);
        break;
    case knopYieldStar:
        byteCodeGenerator->AssignNullConstRegister();
        byteCodeGenerator->AssignUndefinedConstRegister();
        CheckMaybeEscapedUse(pnode->sxUni.pnode1, byteCodeGenerator);
        break;
    }

}

void EmitReference(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    // Generate code for the LHS of an assignment.
    switch (pnode->nop)
    {
    case knopDot:
    case knopScope:
        Emit(pnode->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
        break;

    case knopIndex:
        Emit(pnode->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
        Emit(pnode->sxBin.pnode2, byteCodeGenerator, funcInfo, false);
        break;

    case knopName:
        break;

    case knopArray:
        if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsES6DestructuringEnabled())
        {
            break;
        }
        Emit(pnode, byteCodeGenerator, funcInfo, false);
        break;

    case knopCall:
    case knopNew:
        // Emit the operands of a call that will be used as a LHS.
        // These have to be emitted before the RHS, but they have to persist until
        // the end of the expression.
        // Emit the call target operands first.
        switch (pnode->sxCall.pnodeTarget->nop)
        {
        case knopDot:
        case knopScope:
        case knopIndex:
            funcInfo->AcquireLoc(pnode->sxCall.pnodeTarget);
            EmitReference(pnode->sxCall.pnodeTarget, byteCodeGenerator, funcInfo);
            break;
        case knopName:
            // If the call is being made to a call target defined in slots
            // we will have to load it from the slot here.
        {
            Symbol *sym = pnode->sxCall.pnodeTarget->sxPid.sym;
            if (!sym || sym->GetLocation() == Js::Constants::NoRegister)
            {
                funcInfo->AcquireLoc(pnode->sxCall.pnodeTarget);
            }
            if (sym && (sym->IsInSlot(funcInfo) || sym->GetScope()->GetFunc() != funcInfo))
            {
                // TODO: Why is EmitLoad needed? It seems like the value loaded is never used in this path, which is part of a call.
                EmitLoad(pnode->sxCall.pnodeTarget, byteCodeGenerator, funcInfo);
            }
            else
            {
                EmitReference(pnode->sxCall.pnodeTarget, byteCodeGenerator, funcInfo);
            }
            break;
        }
        default:
            EmitLoad(pnode->sxCall.pnodeTarget, byteCodeGenerator, funcInfo);
        }

        // Now the arg list. We evaluate everything now and emit the ArgOut's later.
        if (pnode->sxCall.pnodeArgs)
        {
            ParseNode *pnodeArg = pnode->sxCall.pnodeArgs;
            while (pnodeArg->nop == knopList)
            {
                Emit(pnodeArg->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
                pnodeArg = pnodeArg->sxBin.pnode2;
            }
            Emit(pnodeArg, byteCodeGenerator, funcInfo, false);
        }
        break;

    default:
        Emit(pnode, byteCodeGenerator, funcInfo, false);
        break;
    }
}

void EmitGetIterator(Js::RegSlot iteratorLocation, Js::RegSlot iterableLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo);
void EmitIteratorNext(Js::RegSlot itemLocation, Js::RegSlot iteratorLocation, Js::RegSlot nextInputLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo);
void EmitIteratorComplete(Js::RegSlot doneLocation, Js::RegSlot iteratorResultLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo);
void EmitIteratorValue(Js::RegSlot valueLocation, Js::RegSlot iteratorResultLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo);

void EmitDestructuredElement(ParseNode *elem, Js::RegSlot sourceLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo *funcInfo)
{
    switch (elem->nop)
    {
    case knopVarDecl:
    case knopLetDecl:
    case knopConstDecl:
        // We manually need to set NeedDeclaration since the node won't be visited.
        elem->sxVar.sym->SetNeedDeclaration(false);
        break;

    default:
        EmitReference(elem, byteCodeGenerator, funcInfo);
    }

    EmitAssignment(nullptr, elem, sourceLocation, byteCodeGenerator, funcInfo);
    funcInfo->ReleaseReference(elem);
}

void EmitDestructuredRestArray(ParseNode *elem, Js::RegSlot iteratorLocation, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    Js::RegSlot restArrayLocation = funcInfo->AcquireTmpRegister();
    byteCodeGenerator->Writer()->Reg1Unsigned1(
        Js::OpCode::NewScArray,
        restArrayLocation,
        ByteCodeGenerator::DefaultArraySize);

    // BytecodeGen can't convey to IRBuilder that some of the temporaries used here are live. When we
    // have a rest parameter, a counter is used in a loop for the array index, but there is no way to
    // convey this is live on the back edge.
    // As a workaround, we have a persistent var reg that is used for the loop counter
    Js::RegSlot counterLocation = elem->location;
    Js::RegSlot zeroConstantReg = byteCodeGenerator->EnregisterConstant(0);
    byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, counterLocation, zeroConstantReg);

    // loopTop:
    Js::ByteCodeLabel loopTop = byteCodeGenerator->Writer()->DefineLabel();
    byteCodeGenerator->Writer()->MarkLabel(loopTop);

    Js::RegSlot itemLocation = funcInfo->AcquireTmpRegister();

    EmitIteratorNext(itemLocation, iteratorLocation, Js::Constants::NoRegister, byteCodeGenerator, funcInfo);

    Js::RegSlot doneLocation = funcInfo->AcquireTmpRegister();
    EmitIteratorComplete(doneLocation, itemLocation, byteCodeGenerator, funcInfo);

    Js::ByteCodeLabel iteratorDone = byteCodeGenerator->Writer()->DefineLabel();
    byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrTrue_A, iteratorDone, doneLocation);

    Js::RegSlot valueLocation = funcInfo->AcquireTmpRegister();
    EmitIteratorValue(valueLocation, itemLocation, byteCodeGenerator, funcInfo);

    byteCodeGenerator->Writer()->Element(
        ByteCodeGenerator::GetStElemIOpCode(funcInfo),
        valueLocation, restArrayLocation, counterLocation);
    funcInfo->ReleaseTmpRegister(valueLocation);
    funcInfo->ReleaseTmpRegister(doneLocation);
    funcInfo->ReleaseTmpRegister(itemLocation);

    byteCodeGenerator->Writer()->Reg2(Js::OpCode::Incr_A, counterLocation, counterLocation);
    byteCodeGenerator->Writer()->Br(loopTop);

    // iteratorDone:
    byteCodeGenerator->Writer()->MarkLabel(iteratorDone);

    ParseNode *restElem = elem->sxUni.pnode1;
    EmitDestructuredElement(restElem, restArrayLocation, byteCodeGenerator, funcInfo);

    funcInfo->ReleaseTmpRegister(restArrayLocation);
}

/*
  EmitDestructuredArray(lhsArray, rhs):
  iterator = rhs[@@iterator]

  if lhsArray empty
    return

  for each element in lhsArray except rest
    value = iterator.next()
    if element is a nested destructured array
      EmitDestructuredArray(element, value)
    else
      if value is undefined and there is an initializer
        evaluate initializer
        evaluate element reference
        element = initializer
      else
        element = value

  if lhsArray has a rest element
    rest = []
    while iterator is not done
      value = iterator.next()
      rest.append(value)
*/
void EmitDestructuredArray(
    ParseNode *lhs,
    Js::RegSlot rhsLocation,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo)
{
    byteCodeGenerator->StartStatement(lhs);
    Js::RegSlot iteratorLocation = funcInfo->AcquireTmpRegister();

    EmitGetIterator(iteratorLocation, rhsLocation, byteCodeGenerator, funcInfo);

    Assert(lhs->nop == knopArray);
    ParseNode *list = lhs->sxArrLit.pnode1;

    if (list == nullptr)
    {
        // No elements to bind or assign.
        funcInfo->ReleaseTmpRegister(iteratorLocation);
        return;
    }

    ParseNode *elem = nullptr;
    while (list != nullptr)
    {
        ParseNode *init = nullptr;

        if (list->nop == knopList)
        {
            elem = list->sxBin.pnode1;
        }
        else
        {
            elem = list;
        }

        if (elem->nop == knopEllipsis)
        {
            // Rest must be the last argument - no need to continue.
            break;
        }

        switch (elem->nop)
        {
        case knopAsg:
            // An assignment node will always have an initializer
            init = elem->sxBin.pnode2;
            elem = elem->sxBin.pnode1;
            break;

        case knopVarDecl:
        case knopLetDecl:
        case knopConstDecl:
            init = elem->sxVar.pnodeInit;
            break;

        default:
            break;
        }

        byteCodeGenerator->StartStatement(elem);

        Js::RegSlot itemLocation = funcInfo->AcquireTmpRegister();

        EmitIteratorNext(itemLocation, iteratorLocation, Js::Constants::NoRegister, byteCodeGenerator, funcInfo);

        if (elem->nop == knopEmpty)
        {
            // Missing elements only require a next call.
            funcInfo->ReleaseTmpRegister(itemLocation);
            if (list->nop == knopList)
            {
                list = list->sxBin.pnode2;
                continue;
            }
            else
            {
                break;
            }
        }

        Js::RegSlot doneLocation = funcInfo->AcquireTmpRegister();
        EmitIteratorComplete(doneLocation, itemLocation, byteCodeGenerator, funcInfo);

        // If the iterator hasn't completed, skip assigning undefined
        Js::ByteCodeLabel iteratorAlreadyDone = byteCodeGenerator->Writer()->DefineLabel();
        byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrTrue_A, iteratorAlreadyDone, doneLocation);
        funcInfo->ReleaseTmpRegister(doneLocation);

        // We're not done with the iterator, so assign the .next() value.
        Js::RegSlot valueLocation = funcInfo->AcquireTmpRegister();
        EmitIteratorValue(valueLocation, itemLocation, byteCodeGenerator, funcInfo);
        Js::ByteCodeLabel beforeDefaultAssign = byteCodeGenerator->Writer()->DefineLabel();
        byteCodeGenerator->Writer()->Br(beforeDefaultAssign);

        // iteratorAlreadyDone:
        byteCodeGenerator->Writer()->MarkLabel(iteratorAlreadyDone);
        byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, valueLocation, funcInfo->undefinedConstantRegister);

        // beforeDefaultAssign:
        byteCodeGenerator->Writer()->MarkLabel(beforeDefaultAssign);

        if (elem->nop == knopArray)
        {

            // If we get an undefined value and have an initializer, use it in place of undefined.
            if (init != nullptr)
            {
                /*
                  the IR builder uses two symbols for a temp register in the if else path
                  R9 <- R3
                  if(...)
                  R9 <- R2
                  R10 = R9.<property>  // error -> IR creates a new lifetime for the if path, and the direct path dest is not referenced
                  hence we have to create a new temp

                  TEMP REG USED TO FIX THIS PRODUCES THIS
                  R9 <- R3
                  if(BrEq_A R9, R3)
                  R10 <- R2                               :
                  else
                  R10 <- R9               : skipdefault
                  ...  = R10[@@iterator]     : loadIter
                */

                // Temp Register
                Js::RegSlot valueLocationTmp = funcInfo->AcquireTmpRegister();
                byteCodeGenerator->StartStatement(init);

                Js::ByteCodeLabel skipDefault = byteCodeGenerator->Writer()->DefineLabel();
                Js::ByteCodeLabel loadIter = byteCodeGenerator->Writer()->DefineLabel();

                // check value is undefined
                byteCodeGenerator->Writer()->BrReg2(Js::OpCode::BrNeq_A, skipDefault, valueLocation, funcInfo->undefinedConstantRegister);

                // Evaluate the default expression and assign it.
                Emit(init, byteCodeGenerator, funcInfo, false);
                byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, valueLocationTmp, init->location);
                funcInfo->ReleaseLoc(init);

                //jmp to loadIter
                byteCodeGenerator->Writer()->Br(loadIter);

                //skipDefault:
                byteCodeGenerator->Writer()->MarkLabel(skipDefault);
                byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, valueLocationTmp, valueLocation);

                //loadIter:
                // @@iterator
                byteCodeGenerator->Writer()->MarkLabel(loadIter);
                byteCodeGenerator->EndStatement(init);

                // Recursively emit a destructured array using the current .next() as the RHS.
                EmitDestructuredArray(elem, valueLocationTmp, byteCodeGenerator, funcInfo);
                funcInfo->ReleaseTmpRegister(valueLocationTmp);
            }
            else
            {
                // Recursively emit a destructured array using the current .next() as the RHS.
                EmitDestructuredArray(elem, valueLocation, byteCodeGenerator, funcInfo);
            }
        }
        else
        {
            Js::ByteCodeLabel useDefault = -1;
            Js::ByteCodeLabel end = -1;

            // Without an initializer, we can just emit the assignment. If there is an initializer, we need to also have a branch for
            // the undefined value case instead of overwriting the value due to assumptions about the use of temporaries.
            if (init != nullptr) {
                useDefault = byteCodeGenerator->Writer()->DefineLabel();
                end = byteCodeGenerator->Writer()->DefineLabel();

                byteCodeGenerator->Writer()->BrReg2(Js::OpCode::BrEq_A, useDefault, valueLocation, funcInfo->undefinedConstantRegister);
            }

            EmitDestructuredElement(elem, valueLocation, byteCodeGenerator, funcInfo);

            // Emit the initializer. Because we have to evaluate the initializer before the element reference,
            // we have to emit the element reference again for this branch before we use it for assignment.
            if (init != nullptr)
            {
                // Br $end
                byteCodeGenerator->Writer()->Br(end);

                byteCodeGenerator->Writer()->MarkLabel(useDefault);

                // Evaluate the default expression and assign it.
                byteCodeGenerator->StartStatement(init);
                Emit(init, byteCodeGenerator, funcInfo, false);
                byteCodeGenerator->EndStatement(init);

                EmitDestructuredElement(elem, init->location, byteCodeGenerator, funcInfo);

                funcInfo->ReleaseLoc(init);
                byteCodeGenerator->Writer()->MarkLabel(end);
            }
        }
        funcInfo->ReleaseTmpRegister(valueLocation);
        funcInfo->ReleaseTmpRegister(itemLocation);

        byteCodeGenerator->EndStatement(elem);

        if (list->nop == knopList)
        {
            list = list->sxBin.pnode2;
        }
        else
        {
            break;
        }
    }

    // If we saw a rest element, emit the rest array.
    if (elem != nullptr && elem->nop == knopEllipsis)
    {
        EmitDestructuredRestArray(elem, iteratorLocation, byteCodeGenerator, funcInfo);
    }
    funcInfo->ReleaseTmpRegister(iteratorLocation);

    byteCodeGenerator->EndStatement(lhs);
}

void EmitAssignment(
    ParseNode *asgnNode,
    ParseNode *lhs,
    Js::RegSlot rhsLocation,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo)
{
    switch (lhs->nop)
    {
        // assignment to a local or global variable
    case knopVarDecl:
    case knopLetDecl:
    case knopConstDecl:
        {
            Symbol *sym=lhs->sxVar.sym;
            Assert(sym!=NULL);
            byteCodeGenerator->EmitPropStore(rhsLocation, sym, null, funcInfo, lhs->nop == knopLetDecl, lhs->nop == knopConstDecl);
            break;
        }

    case knopName:
        {
            byteCodeGenerator->EmitPropStore(rhsLocation, lhs->sxPid.sym, lhs->sxPid.pid, funcInfo);
            break;
        }

        // x.y =
    case knopDot:
        {
            // PutValue(x,"y",rhs)
            Js::PropertyId propertyId = lhs->sxBin.pnode2->sxPid.PropertyIdFromNameNode();

            uint cacheId = funcInfo->FindOrAddInlineCacheId(lhs->sxBin.pnode1->location, propertyId, false, true);
            byteCodeGenerator->Writer()->PatchableProperty(
                ByteCodeGenerator::GetStFldOpCode(funcInfo, false, false, false), rhsLocation, lhs->sxBin.pnode1->location, cacheId);
            break;
        }
    case knopScope:
        {
            // BindEvt(x,"y",rhs)
            Js::PropertyId propertyId = lhs->sxBin.pnode2->sxPid.PropertyIdFromNameNode();
            byteCodeGenerator->Writer()->Property(Js::OpCode::BindEvt,rhsLocation,lhs->sxBin.pnode1->location,
                funcInfo->FindOrAddReferencedPropertyId(propertyId));
            break;
        }
    case knopIndex:
        {
            byteCodeGenerator->Writer()->Element(
                ByteCodeGenerator::GetStElemIOpCode(funcInfo),
                rhsLocation, lhs->sxBin.pnode1->location, lhs->sxBin.pnode2->location);
            break;
        }
    case knopCall:
        EmitCall(lhs, rhsLocation, byteCodeGenerator, funcInfo, false, false);
        break;

    case knopArray:
        if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsES6DestructuringEnabled())
        {
            // Copy the rhs value to be the result of the assignment if needed.
            if (asgnNode != nullptr)
            {
                byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, asgnNode->location, rhsLocation);
            }
            return EmitDestructuredArray(lhs, rhsLocation, byteCodeGenerator, funcInfo);
        }
        // fall through
    default:
        byteCodeGenerator->Writer()->W1(Js::OpCode::RuntimeReferenceError, SCODE_CODE(JSERR_CantAssignTo));
        break;

    }

    if (asgnNode != NULL)
    {
        // We leave it up to the caller to pass this node only if the assignment expression is used.
        if (asgnNode->location != rhsLocation)
        {
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, asgnNode->location, rhsLocation);
        }
    }
}

void EmitLoad(
    ParseNode *lhs,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo)
{
    // Emit the instructions to load the value into the LHS location. Do not assign/free any temps
    // in the process.
    // We usually get here as part of an op-equiv expression: x.y += z;
    // In such a case, x has to be emitted first, then the value of x.y loaded (by this function), then z emitted.
    switch (lhs->nop)
    {
        // load of a local or global variable
    case knopName:
        {
            funcInfo->AcquireLoc(lhs);
            byteCodeGenerator->EmitPropLoad(lhs->location, lhs->sxPid.sym, lhs->sxPid.pid, funcInfo);
            break;
        }
        //  = x.y
    case knopDot:
        {
            // get field id for "y"
            Js::PropertyId propertyId = lhs->sxBin.pnode2->sxPid.PropertyIdFromNameNode();
            funcInfo->AcquireLoc(lhs);
            EmitReference(lhs, byteCodeGenerator, funcInfo);
            uint cacheId = funcInfo->FindOrAddInlineCacheId(lhs->sxBin.pnode1->location, propertyId, false, false);
            byteCodeGenerator->Writer()->PatchableProperty(Js::OpCode::LdFld, lhs->location, lhs->sxBin.pnode1->location, cacheId);
            break;
        }
    case knopIndex:
        funcInfo->AcquireLoc(lhs);
        EmitReference(lhs, byteCodeGenerator, funcInfo);
        byteCodeGenerator->Writer()->Element(
            Js::OpCode::LdElemI_A, lhs->location, lhs->sxBin.pnode1->location, lhs->sxBin.pnode2->location);
        break;
        // f(x) +=
    case knopCall:
        funcInfo->AcquireLoc(lhs);
        EmitReference(lhs, byteCodeGenerator, funcInfo);
        EmitCall(lhs, /*rhs=*/ Js::Constants::NoRegister,  byteCodeGenerator, funcInfo, /*fReturnValue=*/ false, /*fAssignRegs=*/ false);
        break;
    default:
        funcInfo->AcquireLoc(lhs);
        Emit(lhs, byteCodeGenerator, funcInfo, false);
        break;
    }
}


void EmitList(ParseNode *pnode,ByteCodeGenerator *byteCodeGenerator,FuncInfo *funcInfo)
{
    if (pnode!=NULL)
    {
        while(pnode->nop==knopList)
        {
            byteCodeGenerator->EmitTopLevelStatement(pnode->sxBin.pnode1, funcInfo, false);
            pnode = pnode->sxBin.pnode2;
        }
        byteCodeGenerator->EmitTopLevelStatement(pnode, funcInfo, false);
    }
}

void EmitSpreadArgToListBytecodeInstr(ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, Js::RegSlot argLoc, Js::ProfileId callSiteId, size_t &argIndex)
{
    Js::RegSlot regVal = funcInfo->AcquireTmpRegister();
    byteCodeGenerator->Writer()->Reg2(Js::OpCode::LdCustomSpreadIteratorList, regVal, argLoc);
    byteCodeGenerator->Writer()->ArgOut<true>(++argIndex, regVal, callSiteId);
    funcInfo->ReleaseTmpRegister(regVal);
}

size_t EmitArgs(
    ParseNode *pnode,
    BOOL fAssignRegs,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    Js::ProfileId callSiteId,
    Js::AuxArray<uint32> *spreadIndices = nullptr
    )
{
    size_t argIndex = 0;
    size_t spreadIndex = 0;

    if (pnode != nullptr)
    {
        while (pnode->nop == knopList)
        {
            // If this is a put, the arguments have already been evaluated (see EmitReference).
            // We just need to emit the ArgOut instructions.
            if (fAssignRegs)
            {
                Emit(pnode->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
            }

            if (pnode->sxBin.pnode1->nop == knopEllipsis)
            {
                Assert(spreadIndices != nullptr);
                spreadIndices->elements[spreadIndex++] = argIndex + 1; // account for 'this'
                EmitSpreadArgToListBytecodeInstr(byteCodeGenerator, funcInfo, pnode->sxBin.pnode1->location, callSiteId, argIndex);
            }
            else
            {
                byteCodeGenerator->Writer()->ArgOut<true>(++argIndex, pnode->sxBin.pnode1->location, callSiteId);
            }
            if (fAssignRegs)
            {
                funcInfo->ReleaseLoc(pnode->sxBin.pnode1);
            }

            pnode = pnode->sxBin.pnode2;
        }
        // If this is a put, the call target has already been evaluated (see EmitReference).
        if (fAssignRegs)
        {
            Emit(pnode, byteCodeGenerator, funcInfo, false);
        }

        if (pnode->nop == knopEllipsis)
        {
            Assert(spreadIndices != nullptr);
            spreadIndices->elements[spreadIndex++] = argIndex + 1; // account for 'this'
            EmitSpreadArgToListBytecodeInstr(byteCodeGenerator, funcInfo, pnode->location, callSiteId, argIndex);
        }
        else
        {
            byteCodeGenerator->Writer()->ArgOut<true>(++argIndex, pnode->location, callSiteId);
        }
        if (fAssignRegs)
        {
            funcInfo->ReleaseLoc(pnode);
        }
    }

    return argIndex;
}

void EmitArgListStart(
    Js::RegSlot thisLocation,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    Js::ProfileId callSiteId)
{
    if (thisLocation != Js::Constants::NoRegister)
    {
        // Emit the "this" object.
        byteCodeGenerator->Writer()->ArgOut<true>(0, thisLocation, callSiteId);
    }
}

Js::ArgSlot EmitArgListEnd(
    ParseNode *pnode,
    Js::RegSlot rhsLocation,
    Js::RegSlot thisLocation,
    Js::RegSlot evalLocation,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    size_t argIndex,
    Js::ProfileId callSiteId)
{
    BOOL fEvalInModule = false;
    BOOL fIsPut = (rhsLocation != Js::Constants::NoRegister);
    BOOL fIsEval = (evalLocation != Js::Constants::NoRegister);

    size_t evalIndex;

    if (fIsPut)
    {
        // Emit the assigned value as an additional operand. Note that the value has already been evaluated.
        // We just need to emit the ArgOut instruction.
        argIndex++;
        byteCodeGenerator->Writer()->ArgOut<true>(argIndex, rhsLocation, callSiteId);
    }
    if (fIsEval && argIndex > 0)
    {
        // Pass the frame display as an extra argument to "eval".
        // Do this only if eval is called with some args
        Js::RegSlot evalEnv;
        if (funcInfo->IsGlobalFunction() && !(funcInfo->GetIsStrictMode() && byteCodeGenerator->GetFlags() & fscrEval))
        {
            // Use current environment as the environment for the function being called when:
            // - this is the root global function (not an eval's global function)
            // - this is an eval's global function that is not in strict mode (see else block)
            evalEnv = funcInfo->GetEnvRegister();
        }
        else
        {
            // Use the frame display as the environment for the function being called when:
            // - this is not a global function and thus it will have its own scope
            // - this is is an eval's global function that is in strict mode, since in strict mode the eval's global function
            //   has its own scope
            evalEnv = funcInfo->frameDisplayRegister;
        }

        evalEnv = byteCodeGenerator->PrependLocalScopes(evalEnv, evalLocation, funcInfo);

        Js::ModuleID moduleID = byteCodeGenerator->GetModuleID();
        if (moduleID != kmodGlobal)
        {
            // Pass both the module root and the environment.
            fEvalInModule = true;
            byteCodeGenerator->Writer()->ArgOut<true>(argIndex + 1, ByteCodeGenerator::RootObjectRegister, callSiteId);
            evalIndex = argIndex + 2;
        }
        else
        {
            // Just pass the environment.
            evalIndex = argIndex + 1;
        }

        byteCodeGenerator->Writer()->ArgOut<false>(evalIndex, evalEnv, callSiteId);
    }

    size_t argIntCount = argIndex + 1 + fIsEval + fEvalInModule;

    Js::ArgSlot argumentsCount = (Js::ArgSlot)argIntCount;

    // Check for integer overflow
    if ((argIntCount + 1) != (Js::ArgSlot)(argumentsCount + 1))
    {
        Js::Throw::OutOfMemory();
    }

    // eval and no args passed, return 1 as argument count
    if (fIsEval && pnode == NULL)
    {
        return 1;
    }
    return argumentsCount;
}

Js::ArgSlot EmitArgList(
    ParseNode *pnode,
    Js::RegSlot rhsLocation,
    Js::RegSlot thisLocation,
    BOOL fIsEval,
    BOOL fAssignRegs,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    Js::ProfileId callSiteId,
    uint16 spreadArgCount = 0,
    Js::AuxArray<uint32> **spreadIndices = nullptr)
{
    // This function emits the arguments for a call.
    // ArgOut's with uses immediately following defs

    EmitArgListStart(thisLocation, byteCodeGenerator, funcInfo, callSiteId);

    Js::RegSlot evalLocation = Js::Constants::NoRegister;

    //
    // If Emiting arguments for eval and assigning registers, get a tmpLocation for eval
    // This would be used while generating frameDisplay in EmitArgListEnd
    //
    if (fIsEval)
    {
        evalLocation = funcInfo->AcquireTmpRegister();
    }


    if (spreadArgCount > 0)
    {
        const size_t extraAlloc = spreadArgCount * sizeof(uint32);
        Assert(spreadIndices != nullptr);
        *spreadIndices = AnewPlus(byteCodeGenerator->GetAllocator(), extraAlloc, Js::AuxArray<uint32>, spreadArgCount);
    }

    size_t argIndex = EmitArgs(pnode, fAssignRegs, byteCodeGenerator, funcInfo, callSiteId, spreadIndices == nullptr ? nullptr : *spreadIndices);

    Js::ArgSlot argumentsCount = EmitArgListEnd(pnode, rhsLocation, thisLocation, evalLocation, byteCodeGenerator, funcInfo, argIndex, callSiteId);

    if (fIsEval)
    {
        funcInfo->ReleaseTmpRegister(evalLocation);
    }

    return argumentsCount;
}

void EmitConstantArgsToVarArray(ByteCodeGenerator *byteCodeGenerator,__out_ecount(argCount) Js::Var *vars, ParseNode *args, uint argCount)
{
    uint index = 0;
    while (args->nop == knopList && index < argCount)
    {
        if (args->sxBin.pnode1->nop == knopInt)
        {
            int value = args->sxBin.pnode1->sxInt.lw;
            vars[index++] = Js::TaggedInt::ToVarUnchecked(value);
        }
        else if (args->sxBin.pnode1->nop == knopFlt)
        {
            Js::Var number = Js::JavascriptNumber::New(args->sxBin.pnode1->sxFlt.dbl, byteCodeGenerator->GetScriptContext());
#if ! FLOATVAR
            byteCodeGenerator->GetScriptContext()->BindReference(number);
#endif
            vars[index++] = number;
        }
        else
        {
            Assert(false);
        }
        args = args->sxBin.pnode2;
    }

    if (index == argCount)
    {
        Assert(false);
        Js::Throw::InternalError();
        return;
    }

    if (args->nop == knopInt)
    {
        int value = args->sxInt.lw;
        vars[index++] = Js::TaggedInt::ToVarUnchecked(value);
    }
    else if (args->nop == knopFlt)
    {
        Js::Var number = Js::JavascriptNumber::New(args->sxFlt.dbl, byteCodeGenerator->GetScriptContext());
#if ! FLOATVAR
        byteCodeGenerator->GetScriptContext()->BindReference(number);
#endif
        vars[index++] = number;
    }
    else
    {
        Assert(false);
    }
}

void EmitConstantArgsToIntArray(ByteCodeGenerator *byteCodeGenerator,__out_ecount(argCount) int32 *vars, ParseNode *args, uint argCount)
{
    uint index = 0;
    while (args->nop == knopList && index < argCount)
    {
        Assert(args->sxBin.pnode1->nop == knopInt);
        vars[index++] = args->sxBin.pnode1->sxInt.lw;
        args = args->sxBin.pnode2;
    }

    if (index == argCount)
    {
        Assert(false);
        Js::Throw::InternalError();
        return;
    }

    Assert(args->nop == knopInt);
    vars[index++] = args->sxInt.lw;

    Assert(index == argCount);
}

void EmitConstantArgsToFltArray(ByteCodeGenerator *byteCodeGenerator,__out_ecount(argCount) double *vars, ParseNode *args, uint argCount)
{
    uint index = 0;
    while (args->nop == knopList && index < argCount)
    {
        OpCode nop = args->sxBin.pnode1->nop;
        if (nop == knopInt)
        {
            vars[index++] = (double)args->sxBin.pnode1->sxInt.lw;
        }
        else
        {
            Assert(nop == knopFlt);
            vars[index++] = args->sxBin.pnode1->sxFlt.dbl;
        }
        args = args->sxBin.pnode2;
    }

    if (index == argCount)
    {
        Assert(false);
        Js::Throw::InternalError();
        return;
    }

    if (args->nop == knopInt)
    {
        vars[index++] = (double)args->sxInt.lw;
    }
    else
    {
        Assert(args->nop == knopFlt);
        vars[index++] = args->sxFlt.dbl;
    }

    Assert(index == argCount);
}

//
// Called when we have new Ctr(constant, constant...)
//
Js::ArgSlot EmitNewObjectOfConstants(
    ParseNode *pnode,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    unsigned int argCount)
{
    EmitArgListStart(Js::Constants::NoRegister, byteCodeGenerator, funcInfo, Js::Constants::NoProfileId);

    // Create the vars array
    Js::VarArrayVarCount *vars = AnewPlus(byteCodeGenerator->GetAllocator(), (argCount-1) * sizeof(Js::Var), Js::VarArrayVarCount, Js::TaggedInt::ToVarUnchecked(argCount-1));

    // emit all constants to the vars array
    EmitConstantArgsToVarArray(byteCodeGenerator, vars->elements, pnode->sxCall.pnodeArgs, argCount-1);

    // finish the arg list
    unsigned int actualArgCount = EmitArgListEnd(
        pnode->sxCall.pnodeArgs,
        Js::Constants::NoRegister,
        Js::Constants::NoRegister,
        Js::Constants::NoRegister,
        byteCodeGenerator,
        funcInfo,
        argCount-1,
        Js::Constants::NoProfileId);

    // Make sure the cacheId to regSlot map in the ByteCodeWriter is left in a consistent state after writing NewScObject_A
    byteCodeGenerator->Writer()->RemoveEntryForRegSlotFromCacheIdMap(pnode->sxCall.pnodeTarget->location);

    // Generate the opcode with vars
    byteCodeGenerator->Writer()->AuxiliaryContext(
        Js::OpCode::NewScObject_A,
        funcInfo->AcquireLoc(pnode),
        vars,
        sizeof(Js::VarArray) + (argCount-1) * sizeof(Js::Var),
        pnode->sxCall.pnodeTarget->location);

    AdeletePlus(byteCodeGenerator->GetAllocator(), (argCount-1) * sizeof(Js::VarArrayVarCount), vars);

    return actualArgCount;
}

void EmitMethodFld(bool isRoot, bool isScoped, Js::RegSlot location, Js::RegSlot callObjLocation, Js::PropertyId propertyId, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, bool registerCacheIdForCall = true)
{
    Js::OpCode opcode = !isRoot ? Js::OpCode::LdMethodFld :
                        isScoped ? Js::OpCode::ScopedLdMethodFld : Js::OpCode::LdRootMethodFld;

    if (isScoped || !isRoot)
    {
        Assert(isScoped || !isRoot || callObjLocation == ByteCodeGenerator::RootObjectRegister);
        uint cacheId = funcInfo->FindOrAddInlineCacheId(callObjLocation, propertyId, true, false);
        byteCodeGenerator->Writer()->PatchableProperty(opcode, location, callObjLocation, cacheId, false /*isCtor*/, registerCacheIdForCall);
    }
    else
    {
        uint cacheId = funcInfo->FindOrAddRootObjectInlineCacheId(propertyId, true, false);
        byteCodeGenerator->Writer()->PatchableRootProperty(opcode, location, cacheId, true, false, registerCacheIdForCall);
    }
}

void EmitMethodFld(ParseNode *pnode, Js::RegSlot callObjLocation, Js::PropertyId propertyId, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, bool registerCacheIdForCall = true)
{
    // Load a call target of the form x.y(). (Call target may be a plain knopName if we're getting it from
    // the global object, etc.)
    bool isRoot   = pnode->nop == knopName && (pnode->sxPid.sym == null || pnode->sxPid.sym->GetIsGlobal());
    bool isScoped = (byteCodeGenerator->GetFlags() & fscrEval) != 0 ||
                    (isRoot && callObjLocation != ByteCodeGenerator::RootObjectRegister);

    EmitMethodFld(isRoot, isScoped, pnode->location, callObjLocation, propertyId, byteCodeGenerator, funcInfo, registerCacheIdForCall);
}

void VisitClearTmpRegs(ParseNode * pnode, ByteCodeGenerator * byteCodeGenerator, FuncInfo * funcInfo)
{
#if 1
    VisitIndirect<FuncInfo>(pnode,byteCodeGenerator,funcInfo,&ClearTmpRegs,NULL);
#else
    Visit(pnode, byteCodeGenerator,
        [funcInfo](ParseNode * pnode, ByteCodeGenerator * byteCodeGenerator)
        {
            ClearTmpRegs(pnode, byteCodeGenerator, funcInfo);
        },
        [](ParseNode * pnode, ByteCodeGenerator * byteCodeGenerator){}
        );
#endif
}

// lhs.apply(this,arguments);

void EmitApplyCall(ParseNode* pnode, Js::RegSlot rhsLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo, BOOL fReturnValue, BOOL fAssignRegs)
{
    ParseNode* applyNode = pnode->sxCall.pnodeTarget;
    ParseNode* thisNode=pnode->sxCall.pnodeArgs->sxBin.pnode1;
    Assert(applyNode->nop==knopDot);

    ParseNode* funcNode = applyNode->sxBin.pnode1;
    Js::ByteCodeLabel slowPath=byteCodeGenerator->Writer()->DefineLabel();
    Js::ByteCodeLabel afterSlowPath=byteCodeGenerator->Writer()->DefineLabel();
    Js::ByteCodeLabel argsAlreadyCreated=byteCodeGenerator->Writer()->DefineLabel();

    Assert(applyNode->nop ==  knopDot);

    Emit(funcNode, byteCodeGenerator, funcInfo, false);

    funcInfo->AcquireLoc(applyNode);
    Js::PropertyId propertyId = applyNode->sxBin.pnode2->sxPid.PropertyIdFromNameNode();

    // As we won't be emitting a call instruction for apply, no need to register the cacheId for apply load to be associated with the call.
    // This is also required, as in the absence of a corresponding call for apply, we won't remove the entry for "apply" cacheId from ByteCodeWriter::callRegToLdFldCacheIndexMap,
    // which is contrary to our assumption that we would have removed an entry from a map upon seeing its corresponding call.
    EmitMethodFld(applyNode, funcNode->location, propertyId, byteCodeGenerator, funcInfo, false /*registerCacheIdForCall*/);

    Symbol *argSym = funcInfo->GetArgumentsSymbol();
    Assert(argSym && argSym->GetIsArguments());
    Js::RegSlot argumentsLoc = argSym->GetLocation();

    byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdArgumentsFromFrame, argumentsLoc);
    byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrNotNull_A, argsAlreadyCreated, argumentsLoc);

    // If apply is overriden, bail to slow path
    byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrFncNeqApply, slowPath, applyNode->location);

    // Note: acquire and release a temp register for this stack arg pointer instead of trying to stash it
    // in funcInfo->stackArgReg. Otherwise, we'll needlessly load and store it in jitted loop bodies and
    // may crash if we try to unbox it on the store.
    Js::RegSlot stackArgReg = funcInfo->AcquireTmpRegister();
    byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdStackArgPtr, stackArgReg);

    Js::RegSlot argCountLocation=funcInfo->AcquireTmpRegister();

    byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdArgCnt,argCountLocation);
    byteCodeGenerator->Writer()->Reg5(Js::OpCode::ApplyArgs, funcNode->location, funcNode->location, thisNode->location, stackArgReg, argCountLocation);

    funcInfo->ReleaseTmpRegister(argCountLocation);
    funcInfo->ReleaseTmpRegister(stackArgReg);
    funcInfo->ReleaseLoc(applyNode);
    funcInfo->ReleaseLoc(funcNode);

    // Clear these nodes as they are going to be used to re-generate the slow path.
    VisitClearTmpRegs(applyNode, byteCodeGenerator, funcInfo);
    VisitClearTmpRegs(funcNode, byteCodeGenerator, funcInfo);

    byteCodeGenerator->Writer()->Br(afterSlowPath);

    // slow path
    byteCodeGenerator->Writer()->MarkLabel(slowPath);
    if (funcInfo->frameObjRegister != Js::Constants::NoRegister)
    {
        byteCodeGenerator->EmitScopeObjectInit(funcInfo);
    }
    byteCodeGenerator->LoadHeapArguments(funcInfo);

    byteCodeGenerator->Writer()->MarkLabel(argsAlreadyCreated);
    EmitCall(pnode,rhsLocation,byteCodeGenerator,funcInfo,fReturnValue,fAssignRegs);
    byteCodeGenerator->Writer()->MarkLabel(afterSlowPath);
}

void EmitMethodElem(ParseNode *pnode, Js::RegSlot callObjLocation, Js::RegSlot indexLocation, ByteCodeGenerator *byteCodeGenerator)
{
    // Load a call target of the form x[y]().
    byteCodeGenerator->Writer()->Element(Js::OpCode::LdMethodElem, pnode->location, callObjLocation, indexLocation);
}

void EmitCallTargetCompat(
    ParseNode *pnodeTarget,
    BOOL fEvaluateComponents,
    BOOL fSideEffectArgs,
    Js::RegSlot *thisLocation,
    Js::RegSlot *callObjLocation,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo)
{
    // In compat mode, we first get a reference to the call target, then evaluate the arguments, then
    // evaluate the call target.

    // If fEvaluateComponents is false, it means that all the evaluations have been done already and shouldn't
    // be repeated. (This happens in put-call cases like f(x) = y and f(x) += y.)

    switch (pnodeTarget->nop)
    {
    case knopDot:
        if (fEvaluateComponents)
        {
            // Assign the call target operand(s), putting them into expression temps if necessary to protect
            // them from side-effects.
            if (fSideEffectArgs)
            {
                SaveOpndValue(pnodeTarget->sxBin.pnode1, funcInfo);
            }
            Emit(pnodeTarget->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
        }
        *thisLocation = pnodeTarget->sxBin.pnode1->location;
        *callObjLocation = pnodeTarget->sxBin.pnode1->location;
        break;

    case knopIndex:
        if (fEvaluateComponents)
        {
            // Assign the call target operand(s), putting them into expression temps if necessary to protect
            // them from side-effects.
            if (fSideEffectArgs || !(ParseNode::Grfnop(pnodeTarget->sxBin.pnode2->nop) & fnopLeaf))
            {
                SaveOpndValue(pnodeTarget->sxBin.pnode1, funcInfo);
            }
            Emit(pnodeTarget->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
            if (fSideEffectArgs)
            {
                SaveOpndValue(pnodeTarget->sxBin.pnode2, funcInfo);
            }
            Emit(pnodeTarget->sxBin.pnode2, byteCodeGenerator, funcInfo, false);
        }
        *thisLocation = pnodeTarget->sxBin.pnode1->location;
        *callObjLocation = pnodeTarget->sxBin.pnode1->location;
        break;

    case knopName:
        // If the call target is a name, do some extra work to get its instance and the "this" pointer.
        byteCodeGenerator->EmitLoadInstance(pnodeTarget->sxPid.sym, pnodeTarget->sxPid.pid, thisLocation, callObjLocation, funcInfo);
        if (*thisLocation == Js::Constants::NoRegister)
        {
            *thisLocation = funcInfo->undefinedConstantRegister;
        }

        if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsLetAndConstEnabled())
        {
            EmitUseBeforeDeclaration(pnodeTarget, byteCodeGenerator, funcInfo);
        }
        break;

    default:
        if (fEvaluateComponents)
        {
            // Assign the call target operand(s), putting them into expression temps if necessary to protect
            // them from side-effects.
            Emit(pnodeTarget, byteCodeGenerator, funcInfo, false);
        }
        *thisLocation = funcInfo->undefinedConstantRegister;
        break;
    }
}

void EmitCallTargetES5(
    ParseNode *pnodeTarget,
    BOOL fSideEffectArgs,
    Js::RegSlot *thisLocation,
    Js::RegSlot *callObjLocation,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo)
{
    // In ES5 mode, the call target is fully evaluated before the argument list. Note that we're not handling
    // put-call cases here currently, as such cases lie outside ES5 (only apply to host objects) and are very
    // unlikely to behave differently depending on the order of evaluation.

    switch (pnodeTarget->nop)
    {
    case knopDot:
        {
            funcInfo->AcquireLoc(pnodeTarget);
            // Assign the call target operand(s), putting them into expression temps if necessary to protect
            // them from side-effects.
            if (fSideEffectArgs)
            {
                // Though we're done with target evaluation after this point, still protect opnd1 from
                // arg side-effects as it's the "this" pointer.
                SaveOpndValue(pnodeTarget->sxBin.pnode1, funcInfo);
            }

            if ((pnodeTarget->sxBin.pnode2->nop == knopName) && ((pnodeTarget->sxBin.pnode2->sxPid.PropertyIdFromNameNode() == Js::PropertyIds::apply) || (pnodeTarget->sxBin.pnode2->sxPid.PropertyIdFromNameNode() == Js::PropertyIds::call)))
            {
                pnodeTarget->sxBin.pnode1->SetIsCallApplyTargetLoad();
            }
            Emit(pnodeTarget->sxBin.pnode1, byteCodeGenerator, funcInfo, false);

            Js::PropertyId propertyId = pnodeTarget->sxBin.pnode2->sxPid.PropertyIdFromNameNode();
            Js::RegSlot callObjLocation = pnodeTarget->sxBin.pnode1->location;
            EmitMethodFld(pnodeTarget, callObjLocation, propertyId, byteCodeGenerator, funcInfo);

            // Function calls on the 'super' object should maintain current 'this' pointer
            *thisLocation = (pnodeTarget->sxBin.pnode1->nop == knopSuper) ? funcInfo->thisPointerRegister : pnodeTarget->sxBin.pnode1->location;
            break;
        }

    case knopIndex:
        {
            funcInfo->AcquireLoc(pnodeTarget);
            // Assign the call target operand(s), putting them into expression temps if necessary to protect
            // them from side-effects.
            if (fSideEffectArgs || !(ParseNode::Grfnop(pnodeTarget->sxBin.pnode2->nop) & fnopLeaf))
            {
                // Though we're done with target evaluation after this point, still protect opnd1 from
                // arg or opnd2 side-effects as it's the "this" pointer.
                SaveOpndValue(pnodeTarget->sxBin.pnode1, funcInfo);
            }
            Emit(pnodeTarget->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
            Emit(pnodeTarget->sxBin.pnode2, byteCodeGenerator, funcInfo, false);
            funcInfo->ReleaseLoc(pnodeTarget->sxBin.pnode2);

            Js::RegSlot callObjLocation = pnodeTarget->sxBin.pnode1->location;
            Js::RegSlot indexLocation = pnodeTarget->sxBin.pnode2->location;
            EmitMethodElem(pnodeTarget, callObjLocation, indexLocation, byteCodeGenerator);

            *thisLocation = pnodeTarget->sxBin.pnode1->location;
            break;
        }

    case knopClassDecl:
    {
        Emit(pnodeTarget, byteCodeGenerator, funcInfo, false);
        // We won't always have an assigned this register (e.g. class expression calls.) We need undefined in this case.
        *thisLocation = funcInfo->thisPointerRegister == Js::Constants::NoRegister ? funcInfo->undefinedConstantRegister : funcInfo->thisPointerRegister;
        break;
    }

    case knopSuper:
    {
        Emit(pnodeTarget, byteCodeGenerator, funcInfo, false);
        *thisLocation = funcInfo->thisPointerRegister;
        break;
    }

    case knopName:
        {
            funcInfo->AcquireLoc(pnodeTarget);
            // Assign the call target operand(s), putting them into expression temps if necessary to protect
            // them from side-effects.
            if (fSideEffectArgs)
            {
                SaveOpndValue(pnodeTarget, funcInfo);
            }
            byteCodeGenerator->EmitLoadInstance(pnodeTarget->sxPid.sym, pnodeTarget->sxPid.pid, thisLocation, callObjLocation, funcInfo);
            if (*callObjLocation != Js::Constants::NoRegister)
            {
                // Load the call target as a property of the instance.
                Js::PropertyId propertyId = pnodeTarget->sxPid.PropertyIdFromNameNode();
                EmitMethodFld(pnodeTarget, *callObjLocation, propertyId, byteCodeGenerator, funcInfo);
                break;
            }

            // FALL THROUGH to evaluate call target.
        }

    default:
        // Assign the call target operand(s), putting them into expression temps if necessary to protect
        // them from side-effects.
        Emit(pnodeTarget, byteCodeGenerator, funcInfo, false);
        *thisLocation = funcInfo->undefinedConstantRegister;
        break;
    }

    // "This" pointer should have been assigned by the above.
    Assert(*thisLocation != Js::Constants::NoRegister);
}

void EmitCallI(
    ParseNode *pnode,
    BOOL fEvaluateComponents,
    BOOL fIsPut,
    BOOL fIsEval,
    uint32 actualArgCount,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    Js::ProfileId callSiteId,
    Js::AuxArray<uint32> *spreadIndices = nullptr)
{
    // Emit a call where the target is in a register, because it's either a local name or an expression we've
    // already evaluated.

    ParseNode *pnodeTarget = pnode->sxBin.pnode1;
    Js::OpCode op;
    size_t spreadExtraAlloc = 0;

    if (fIsPut)
    {
        if (pnode->sxCall.spreadArgCount > 0)
        {
            // TODO(tcare): We are disallowing spread with CallIPut for the moment. See DEVDIV2: 876387
            //              When CallIPut is migrated to the CallIExtended layout, this can be removed.
            byteCodeGenerator->Writer()->W1(Js::OpCode::RuntimeReferenceError, SCODE_CODE(JSERR_CantAsgCall));
        }
        // Grab a tmp register for the call result.
        Js::RegSlot tmpReg = funcInfo->AcquireTmpRegister();
        byteCodeGenerator->Writer()->CallI(Js::OpCode::CallIPut, tmpReg, pnodeTarget->location, actualArgCount, callSiteId);
        funcInfo->ReleaseTmpRegister(tmpReg);
    }
    else
    {
        if (fEvaluateComponents)
        {
            // Release the call target operands we assigned above. If we didn't assign them here,
            // we'll need them later, so we can't re-use them for the result of the call.
            funcInfo->ReleaseLoc(pnodeTarget);
        }
        // Grab a register for the call result.
        if (pnode->isUsed)
        {
            funcInfo->AcquireLoc(pnode);
        }
        if (fIsEval)
        {
            op = Js::OpCode::CallIEval;
        }
        else if (pnode->sxCall.spreadArgCount > 0)
        {
            op = Js::OpCode::CallIExtended;
        }
        else
        {
            op = Js::OpCode::CallI;
        }

        if (op == Js::OpCode::CallI)
        {
            byteCodeGenerator->Writer()->CallI(op, pnode->location, pnodeTarget->location, actualArgCount, callSiteId);
        }
        else
        {
            size_t spreadIndicesSize = 0;
            Js::CallIExtendedOptions options = Js::CallIExtended_None;

            if (pnode->sxCall.spreadArgCount > 0)
            {
                Assert(spreadIndices != nullptr);
                spreadExtraAlloc = spreadIndices->count * sizeof(uint32);
                spreadIndicesSize = sizeof(*spreadIndices) + spreadExtraAlloc;
                options = Js::CallIExtended_SpreadArgs;
            }

            byteCodeGenerator->Writer()->CallIExtended(op, pnode->location, pnodeTarget->location, actualArgCount, options, spreadIndices, spreadIndicesSize, callSiteId);
        }

        if (pnode->sxCall.spreadArgCount > 0)
        {
            Assert(spreadExtraAlloc != 0);
            AdeletePlus(byteCodeGenerator->GetAllocator(), spreadExtraAlloc, spreadIndices);
        }
    }
}

void EmitCallInstrCompat(
    ParseNode *pnode,
    BOOL fEvaluateComponents,
    BOOL fIsPut,
    BOOL fIsEval,
    Js::RegSlot thisLocation,
    Js::RegSlot callObjLocation,
    uint32 actualArgCount,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    Js::ProfileId callSiteId,
    Js::AuxArray<uint32> *spreadIndices = nullptr)
{
    // Emit the call instruction. In compat mode, the call target is a reference at this point, and we evaluate
    // it as part of doing the actual call.

    ParseNode *pnodeTarget = pnode->sxBin.pnode1;

    switch (pnodeTarget->nop)
    {
    case knopDot:
        {
            Assert(pnodeTarget->sxBin.pnode2->nop == knopName);
            Js::PropertyId propertyId = pnodeTarget->sxBin.pnode2->sxPid.PropertyIdFromNameNode();

            if (fEvaluateComponents)
            {
                // Release the call target operand we assigned above. If we didn't assign it here,
                // we'll need it later, so we can't re-use it for the result of the call.
                funcInfo->ReleaseLoc(pnodeTarget->sxBin.pnode1);
                funcInfo->AcquireLoc(pnodeTarget);
            }
            EmitMethodFld(pnodeTarget, callObjLocation, propertyId, byteCodeGenerator, funcInfo);
            EmitCallI(pnode, fEvaluateComponents, fIsPut, fIsEval, actualArgCount, byteCodeGenerator, funcInfo, callSiteId, spreadIndices);
        }
        break;

    case knopIndex:
        {
            if (fEvaluateComponents)
            {
                // Release the call target operands we assigned above. If we didn't assign them here,
                // we'll need them later, so we can't re-use them for the result of the call.
                funcInfo->ReleaseLoc(pnodeTarget->sxBin.pnode2);
                funcInfo->ReleaseLoc(pnodeTarget->sxBin.pnode1);
                funcInfo->AcquireLoc(pnodeTarget);
            }
            EmitMethodElem(pnodeTarget, pnodeTarget->sxBin.pnode1->location, pnodeTarget->sxBin.pnode2->location, byteCodeGenerator);
            EmitCallI(pnode, fEvaluateComponents, fIsPut, fIsEval, actualArgCount, byteCodeGenerator, funcInfo, callSiteId, spreadIndices);
        }
        break;

    case knopName:
        {
            if (callObjLocation != Js::Constants::NoRegister)
            {
                // We still have to get the property from its instance, so emit CallFld.
                if (thisLocation != callObjLocation)
                {
                    funcInfo->ReleaseTmpRegister(thisLocation);
                }
                funcInfo->ReleaseTmpRegister(callObjLocation);
                if (fEvaluateComponents)
                {
                    funcInfo->AcquireLoc(pnodeTarget);
                }
                Js::PropertyId propertyId = pnodeTarget->sxPid.PropertyIdFromNameNode();
                EmitMethodFld(pnodeTarget, callObjLocation, propertyId, byteCodeGenerator, funcInfo);
                EmitCallI(pnode, fEvaluateComponents, fIsPut, fIsEval, actualArgCount, byteCodeGenerator, funcInfo, callSiteId, spreadIndices);
                break;
            }

            // Call target is in a register, so fall through and CallI.
            if (fEvaluateComponents)
            {
                // The target may, e.g., be in a closure slot, so get it now, after side-effects from evaluating args.
                Emit(pnodeTarget, byteCodeGenerator, funcInfo, false);
            }
        }
        // FALL THROUGH

    default:
        EmitCallI(pnode, fEvaluateComponents, fIsPut, fIsEval, actualArgCount, byteCodeGenerator, funcInfo, callSiteId, spreadIndices);
        break;
    }
}

void EmitCallInstrES5(
    ParseNode *pnode,
    BOOL fIsPut,
    BOOL fIsEval,
    Js::RegSlot thisLocation,
    Js::RegSlot callObjLocation,
    uint32 actualArgCount,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    Js::ProfileId callSiteId,
    Js::AuxArray<uint32> *spreadIndices = nullptr)
{
    // Emit a call instruction. In ES5 mode, the call target has been fully evaluated already, so we always
    // emit a CallI through the register that holds the target value.
    // Note that we don't handle !fEvaluateComponents cases in ES5 mode at this point.
    if (thisLocation != Js::Constants::NoRegister)
    {
        funcInfo->ReleaseTmpRegister(thisLocation);
    }
    if (callObjLocation != Js::Constants::NoRegister &&
        callObjLocation != thisLocation)
    {
        funcInfo->ReleaseTmpRegister(callObjLocation);
    }
    EmitCallI(pnode, TRUE, fIsPut, fIsEval, actualArgCount, byteCodeGenerator, funcInfo, callSiteId, spreadIndices);
}

void EmitCall(
    ParseNode* pnode,
    Js::RegSlot rhsLocation,
    ByteCodeGenerator* byteCodeGenerator,
    FuncInfo* funcInfo,
    BOOL fReturnValue,
    BOOL fEvaluateComponents)
{
    BOOL fIsPut = (rhsLocation != Js::Constants::NoRegister);
    // If the call returns a float, we'll note this in the byte code.
    Js::RegSlot thisLocation = Js::Constants::NoRegister;
    Js::RegSlot callObjLocation = Js::Constants::NoRegister;
    BOOL fSideEffectArgs = FALSE;
    ParseNode *pnodeTarget = pnode->sxCall.pnodeTarget;
    ParseNode *pnodeArgs = pnode->sxCall.pnodeArgs;
    uint16 spreadArgCount = pnode->sxCall.spreadArgCount;

    unsigned int argCount = CountArguments(pnode->sxCall.pnodeArgs, &fSideEffectArgs) + fIsPut;

    BOOL fIsEval = !fIsPut && pnode->sxCall.isEvalCall;

    if (fIsEval)
    {
        //
        // "eval" takes the closure environment as an extra argument
        // Pass the closure env only if some argument is passed
        // For just eval(), don't pass the closure environment
        //
        if (argCount > 1)
        {
            // Check the module ID as well. If it's not the global (default) module,
            // we need to pass the root to eval so it can do the right global lookups.
            // (Passing the module root is the least disruptive way to get the module ID
            // to the helper, given the current set of byte codes. Once we have a full set
            // of byte code ops taking immediate opnds, passing the ID is more intuitive.)
            Js::ModuleID moduleID = byteCodeGenerator->GetModuleID();
            if (moduleID == kmodGlobal)
            {
                argCount++;
            }
            else
            {
                // Module ID must be passed
                argCount += 2;
            }
        }
    }

    if (argCount != (Js::ArgSlot)argCount)
    {
        Js::Throw::OutOfMemory();
    }

    if (fReturnValue)
    {
        pnode->isUsed = true;
    }

    //
    // Set up the call.
    //

    // Compat:
    // - emit reference to target
    //    - copy instance to scratch reg if nec'y.
    //    - assign this
    //    - assign instance for dynamic/global name
    // - emit args
    // - do call (CallFld/Elem/I)

    // ES5:
    // - emit target
    //    - assign this
    // - emit args
    // - do call

    // Send !fEvaluateComponents down the compat mode path at this point for the sake of stability.

    if (!fEvaluateComponents)
    {
        EmitCallTargetCompat(pnodeTarget, fEvaluateComponents, fSideEffectArgs, &thisLocation, &callObjLocation, byteCodeGenerator, funcInfo);
    }
    else
    {
        EmitCallTargetES5(pnodeTarget, fSideEffectArgs, &thisLocation, &callObjLocation, byteCodeGenerator, funcInfo);
    }

    // Evaluate the arguments (nothing mode-specific here).
    // Start call, allocate out param space
    funcInfo->StartRecordingOutArgs(argCount);

    Js::ProfileId callSiteId = byteCodeGenerator->GetNextCallSiteId(Js::OpCode::CallI);
    
	byteCodeGenerator->Writer()->StartCall(Js::OpCode::StartCall, argCount);
    Js::AuxArray<uint32> *spreadIndices;
    uint32 actualArgCount = EmitArgList(pnodeArgs, rhsLocation, thisLocation, fIsEval, fEvaluateComponents, byteCodeGenerator, funcInfo, callSiteId, spreadArgCount, &spreadIndices);
    Assert(argCount == actualArgCount);

    if (!fEvaluateComponents)
    {
        EmitCallInstrCompat(pnode, fEvaluateComponents, fIsPut, fIsEval, thisLocation, callObjLocation, actualArgCount, byteCodeGenerator, funcInfo, callSiteId, spreadIndices);
    }
    else
    {
        EmitCallInstrES5(pnode, fIsPut, fIsEval, thisLocation, callObjLocation, actualArgCount, byteCodeGenerator, funcInfo, callSiteId, spreadIndices);
    }

    // End call, pop param space
    funcInfo->EndRecordingOutArgs(argCount);
}

void EmitInvoke(
    Js::RegSlot location,
    Js::RegSlot callObjLocation,
    Js::PropertyId propertyId,
    ByteCodeGenerator* byteCodeGenerator,
    FuncInfo* funcInfo
)
{
    EmitMethodFld(false, false, location, callObjLocation, propertyId, byteCodeGenerator, funcInfo);

    funcInfo->StartRecordingOutArgs(1);

    Js::ProfileId callSiteId = byteCodeGenerator->GetNextCallSiteId(Js::OpCode::CallI);
    
	byteCodeGenerator->Writer()->StartCall(Js::OpCode::StartCall, 1);
    EmitArgListStart(callObjLocation, byteCodeGenerator, funcInfo, callSiteId);

    byteCodeGenerator->Writer()->CallI(Js::OpCode::CallI, location, location, 1, callSiteId);
}

void EmitInvoke(
    Js::RegSlot location,
    Js::RegSlot callObjLocation,
    Js::PropertyId propertyId,
    ByteCodeGenerator* byteCodeGenerator,
    FuncInfo* funcInfo,
    Js::RegSlot arg1Location
)
{
    EmitMethodFld(false, false, location, callObjLocation, propertyId, byteCodeGenerator, funcInfo);

    funcInfo->StartRecordingOutArgs(2);

    Js::ProfileId callSiteId = byteCodeGenerator->GetNextCallSiteId(Js::OpCode::CallI);

    byteCodeGenerator->Writer()->StartCall(Js::OpCode::StartCall, 2);
    EmitArgListStart(callObjLocation, byteCodeGenerator, funcInfo, callSiteId);
    byteCodeGenerator->Writer()->ArgOut<true>(1, arg1Location, callSiteId);

    byteCodeGenerator->Writer()->CallI(Js::OpCode::CallI, location, location, 2, callSiteId);
}


void EmitMemberNode(ParseNode *memberNode, Js::RegSlot objectLocation, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, ParseNode* parentNode, bool useStore = false) {
    ParseNode *nameNode=memberNode->sxBin.pnode1;
    ParseNode *exprNode=memberNode->sxBin.pnode2;

    if (nameNode->nop == knopComputedName)
    {
        // Computed property name
        Emit(exprNode, byteCodeGenerator, funcInfo, false);
        Emit(nameNode, byteCodeGenerator, funcInfo, false);
        if (memberNode->nop == knopGetMember)
        {
            byteCodeGenerator->Writer()->Element(
                Js::OpCode::InitGetElemI, exprNode->location, objectLocation, nameNode->location, true);
        }
        else if (memberNode->nop == knopSetMember)
        {
            byteCodeGenerator->Writer()->Element(
                Js::OpCode::InitSetElemI, exprNode->location, objectLocation, nameNode->location, true);
        }
        else
        {
            Assert(memberNode->nop == knopMember);
            byteCodeGenerator->Writer()->Element(
                Js::OpCode::InitComputedProperty,
                exprNode->location, objectLocation, nameNode->location, true);
        }
        if (exprNode->nop == knopFncDecl && (exprNode->sxFnc.pnodeNames == nullptr || exprNode->sxFnc.pnodeNames->nop != knopVarDecl))
        {
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::SetComputedNameVar, exprNode->location, nameNode->location);
        }

        // Class members need a reference back to the class.
        if (exprNode->nop == knopFncDecl && exprNode->sxFnc.IsClassMember())
        {
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::SetHomeObj, exprNode->location, objectLocation);
        }

        funcInfo->ReleaseLoc(nameNode);
        funcInfo->ReleaseLoc(exprNode);

        return;
    }

    Js::OpCode stFldOpCode = (Js::OpCode)0;
    if (useStore)
    {
        stFldOpCode = ByteCodeGenerator::GetStFldOpCode(funcInfo, false, false, false);
    }

    Emit(exprNode, byteCodeGenerator, funcInfo, false);
    Js::PropertyId propertyId = nameNode->sxPid.PropertyIdFromNameNode();

    if (Js::PropertyIds::name == propertyId
        && exprNode->nop == knopFncDecl
        && exprNode->sxFnc.IsStaticMember()
        && parentNode != nullptr && parentNode->nop == knopClassDecl
        && parentNode->sxClass.pnodeConstructor != nullptr)
    {
        Js::ParseableFunctionInfo* nameFunc = parentNode->sxClass.pnodeConstructor->sxFnc.funcInfo->byteCodeFunction->GetParseableFunctionInfo();
        nameFunc->SetIsStaticNameFunction(true); // TODO this code needs to be duplicated for computed properties
    }

    if (memberNode->nop == knopMember)
    {
        if (!useStore && propertyId == Js::PropertyIds::__proto__)
        {
            byteCodeGenerator->Writer()->Property(Js::OpCode::InitProto, exprNode->location, objectLocation,
                funcInfo->FindOrAddReferencedPropertyId(propertyId));
        }
        else
        {
            uint cacheId = funcInfo->FindOrAddInlineCacheId(objectLocation, propertyId, false, true);
            byteCodeGenerator->Writer()->PatchableProperty(useStore ? stFldOpCode : Js::OpCode::InitFld, exprNode->location, objectLocation, cacheId);
        }
    }
    else if (memberNode->nop == knopGetMember)
    {
        byteCodeGenerator->Writer()->Property(Js::OpCode::InitGetFld,exprNode->location,objectLocation,
            funcInfo->FindOrAddReferencedPropertyId(propertyId));
    }
    else // if (memberNode->nop == knopSetMember)
    {
        byteCodeGenerator->Writer()->Property(Js::OpCode::InitSetFld,exprNode->location,objectLocation,
            funcInfo->FindOrAddReferencedPropertyId(propertyId));
    }

    // Class members need a reference back to the class.
    if (exprNode->nop == knopFncDecl && exprNode->sxFnc.IsClassMember())
    {
        byteCodeGenerator->Writer()->Reg2(Js::OpCode::SetHomeObj, exprNode->location, objectLocation);
    }

    funcInfo->ReleaseLoc(exprNode);

    if (propertyId == Js::PropertyIds::valueOf)
    {
        byteCodeGenerator->GetScriptContext()->optimizationOverrides.SetSideEffects(Js::SideEffects_ValueOf);
    }
    else if (propertyId == Js::PropertyIds::toString)
    {
        byteCodeGenerator->GetScriptContext()->optimizationOverrides.SetSideEffects(Js::SideEffects_ToString);
    }
}

void EmitClassInitializers(ParseNode *memberList, Js::RegSlot objectLocation, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, ParseNode* parentNode)
{
    if (memberList != NULL)
    {
        while (memberList->nop == knopList)
        {
            ParseNode *memberNode = memberList->sxBin.pnode1;
            EmitMemberNode(memberNode, objectLocation, byteCodeGenerator, funcInfo, parentNode);
            memberList = memberList->sxBin.pnode2;
        }
        EmitMemberNode(memberList, objectLocation, byteCodeGenerator, funcInfo, parentNode);
    }
}

void EmitObjectInitializers(ParseNode *memberList, Js::RegSlot objectLocation,ByteCodeGenerator *byteCodeGenerator,FuncInfo *funcInfo)
{
    ParseNode *pmemberList = memberList;
    unsigned int argCount = 0;
    uint32  value;
    Js::PropertyId propertyId;

    //
    // 1. Add all non-int property ids to a dictionary propertyIds with value true
    // 2. Get the count of propertyIds
    // 3. Create a propertyId array of size count
    // 4. Put the propIds in the auxiliary area
    // 5. Get the objectLiteralCacheId
    // 6. Generate propId inits with values
    //

    // Handle propertyId collision
    typedef JsUtil::BaseHashSet<Js::PropertyId, ArenaAllocator, PowerOf2SizePolicy> PropertyIdSet;
    PropertyIdSet* propertyIds = Anew(byteCodeGenerator->GetAllocator(), PropertyIdSet, byteCodeGenerator->GetAllocator(), 17);

    bool hasComputedName = false;
    if (memberList != nullptr)
    {
        while (memberList->nop == knopList)
        {
            if (memberList->sxBin.pnode1->sxBin.pnode1->nop == knopComputedName)
            {
                hasComputedName = true;
                break;
            }

            propertyId = memberList->sxBin.pnode1->sxBin.pnode1->sxPid.PropertyIdFromNameNode();
            if (!byteCodeGenerator->GetScriptContext()->IsNumericPropertyId(propertyId, &value))
            {
                propertyIds->Item(propertyId);
            }
            memberList = memberList->sxBin.pnode2;
        }

        if (memberList->sxBin.pnode1->nop != knopComputedName && !hasComputedName)
        {
            propertyId = memberList->sxBin.pnode1->sxPid.PropertyIdFromNameNode();
            if (!byteCodeGenerator->GetScriptContext()->IsNumericPropertyId(propertyId, &value))
            {
                propertyIds->Item(propertyId);
            }
        }
    }

    argCount = propertyIds->Count();

    memberList = pmemberList;
    if ( (memberList == nullptr) || (argCount == 0) )
    {
        // Empty literal or numeric property only object literal
        byteCodeGenerator->Writer()->Reg1(Js::OpCode::NewScObjectSimple, objectLocation);
    }
    else
    {
        Js::PropertyIdArray *propIds = AnewPlus(byteCodeGenerator->GetAllocator(), argCount * sizeof(Js::PropertyId), Js::PropertyIdArray, argCount);

        if (propertyIds->ContainsKey(Js::PropertyIds::__proto__))
        {
            // Always record whether the initializer contains __proto__ no matter if current environment has it enabled
            // or not, in case the bytecode is later run with __proto__ enabled.
            propIds->has__proto__ = true;
        }

        unsigned int argIndex = 0;
        while (memberList->nop == knopList)
        {
            if (memberList->sxBin.pnode1->sxBin.pnode1->nop == knopComputedName)
            {
                break;
            }
            propertyId = memberList->sxBin.pnode1->sxBin.pnode1->sxPid.PropertyIdFromNameNode();
            if (!byteCodeGenerator->GetScriptContext()->IsNumericPropertyId(propertyId, &value) && propertyIds->Remove(propertyId))
            {
                propIds->elements[argIndex] = propertyId;
                argIndex++;
            }
            memberList = memberList->sxBin.pnode2;
        }

        if (memberList->sxBin.pnode1->nop != knopComputedName && !hasComputedName)
        {
            propertyId = memberList->sxBin.pnode1->sxPid.PropertyIdFromNameNode();
            if (!byteCodeGenerator->GetScriptContext()->IsNumericPropertyId(propertyId, &value) && propertyIds->Remove(propertyId))
            {
                propIds->elements[argIndex] = propertyId;
                argIndex++;
            }
        }

        uint32 literalObjectId = funcInfo->GetParsedFunctionBody()->NewObjectLiteral();

        // Generate the opcode with propIds and cacheId
        byteCodeGenerator->Writer()->Auxiliary(Js::OpCode::NewScObjectLiteral, objectLocation, propIds, sizeof(Js::PropertyIdArray) + argCount * sizeof(Js::PropertyId), literalObjectId);

        Adelete(byteCodeGenerator->GetAllocator(), propertyIds);

        AdeletePlus(byteCodeGenerator->GetAllocator(), argCount * sizeof(Js::PropertyId), propIds);
    }

    memberList = pmemberList;

    bool useStore = false;
    // Generate the actual assignment to those properties
    if (memberList != nullptr)
    {
        while (memberList->nop == knopList)
        {
            ParseNode *memberNode=memberList->sxBin.pnode1;

            if (memberNode->sxBin.pnode1->nop == knopComputedName)
            {
                useStore = true;
            }

            byteCodeGenerator->StartSubexpression(memberNode);
            EmitMemberNode(memberNode, objectLocation, byteCodeGenerator, funcInfo, nullptr, useStore);
            byteCodeGenerator->EndSubexpression(memberNode);
            memberList = memberList->sxBin.pnode2;
        }
        byteCodeGenerator->StartSubexpression(memberList);
        EmitMemberNode(memberList, objectLocation, byteCodeGenerator, funcInfo, nullptr, useStore);
        byteCodeGenerator->EndSubexpression(memberList);
    }
}

void EmitStringTemplate(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    Assert(pnode->sxStrTemplate.pnodeStringLiterals);

    // For a tagged string template, we will create the callsite constant object as part of the FunctionBody constants table.
    // We only need to emit code for non-tagged string templates here.
    if (!pnode->sxStrTemplate.isTaggedTemplate)
    {
        // If we have no substitutions and this is not a tagged template, we can emit just the single cooked string.
        if (pnode->sxStrTemplate.pnodeSubstitutionExpressions == nullptr)
        {
            Assert(pnode->sxStrTemplate.pnodeStringLiterals->nop != knopList);

            funcInfo->AcquireLoc(pnode);
            Emit(pnode->sxStrTemplate.pnodeStringLiterals, byteCodeGenerator, funcInfo, false);

            Assert(pnode->location != pnode->sxStrTemplate.pnodeStringLiterals->location);

            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxStrTemplate.pnodeStringLiterals->location);
            funcInfo->ReleaseLoc(pnode->sxStrTemplate.pnodeStringLiterals);
        }
        else
        {
            // If we have substitutions but no tag function, we can skip the callSite object construction (and also ignore raw strings)
            funcInfo->AcquireLoc(pnode);

            // First string must be a list node since we have substitutions
            AssertMsg(pnode->sxStrTemplate.pnodeStringLiterals->nop == knopList, "First string in the list must be a knopList node.");

            ParseNode* stringNodeList = pnode->sxStrTemplate.pnodeStringLiterals;

            // Emit the first string and load that into the pnode location
            Emit(stringNodeList->sxBin.pnode1, byteCodeGenerator, funcInfo, false);

            Assert(pnode->location != stringNodeList->sxBin.pnode1->location);

            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, stringNodeList->sxBin.pnode1->location);
            funcInfo->ReleaseLoc(stringNodeList->sxBin.pnode1);

            ParseNode* expressionNodeList = pnode->sxStrTemplate.pnodeSubstitutionExpressions;
            ParseNode* stringNode;
            ParseNode* expressionNode;

            // Now append the substitution expressions and remaining string constants via normal add operator
            // We will always have one more string constant than substitution expression
            // `strcon1 ${expr1} strcon2 ${expr2} strcon3` = strcon1 + expr1 + strcon2 + expr2 + strcon3
            //
            // strcon1 --- step 1 (above)
            // expr1   \__ step 2
            // strcon2 /
            // expr2   \__ step 3
            // strcon3 /
            while (stringNodeList->nop == knopList)
            {
                // If the current head of the expression list is a list, fetch the node and walk the list
                if (expressionNodeList->nop == knopList)
                {
                    expressionNode = expressionNodeList->sxBin.pnode1;
                    expressionNodeList = expressionNodeList->sxBin.pnode2;
                }
                else
                {
                    // This is the last element of the expression list
                    expressionNode = expressionNodeList;
                }

                // Emit the expression and append it to the string we're building
                Emit(expressionNode, byteCodeGenerator, funcInfo, false);
                byteCodeGenerator->Writer()->Reg3(Js::OpCode::Add_A, pnode->location, pnode->location, expressionNode->location);
                funcInfo->ReleaseLoc(expressionNode);

                // Move to the next string in the list - we already got ahead of the expressions in the first string literal above
                stringNodeList = stringNodeList->sxBin.pnode2;

                // If the current head of the string literal list is also a list node, need to fetch the actual string literal node
                if (stringNodeList->nop == knopList)
                {
                    stringNode = stringNodeList->sxBin.pnode1;
                }
                else
                {
                    // This is the last element of the string literal list
                    stringNode = stringNodeList;
                }

                // Emit the string node following the previous expression and append it to the string
                // This is either just some string in the list or it is the last string
                Emit(stringNode, byteCodeGenerator, funcInfo, false);
                byteCodeGenerator->Writer()->Reg3(Js::OpCode::Add_A, pnode->location, pnode->location, stringNode->location);
                funcInfo->ReleaseLoc(stringNode);
            }
        }
    }
}

void SetNewArrayElements(ParseNode *pnode, Js::RegSlot arrayLocation, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    ParseNode *args = pnode->sxUni.pnode1;
    uint argCount = pnode->sxArrLit.count;
    uint spreadCount = pnode->sxArrLit.spreadCount;
    bool nativeArrays = CreateNativeArrays(byteCodeGenerator, funcInfo);

    bool arrayIntOpt = nativeArrays && pnode->sxArrLit.arrayOfInts;
    if (arrayIntOpt)
    {
        int extraAlloc = argCount * sizeof(int32);
        Js::AuxArray<int> *ints = AnewPlus(byteCodeGenerator->GetAllocator(), extraAlloc, Js::AuxArray<int32>, argCount);
        EmitConstantArgsToIntArray(byteCodeGenerator, ints->elements, args, argCount);
        Assert(!pnode->sxArrLit.hasMissingValues);
        byteCodeGenerator->Writer()->Auxiliary(
            Js::OpCode::NewScIntArray,
            pnode->location,
            ints,
            sizeof(Js::AuxArray<int>) + extraAlloc,
            argCount);
        AdeletePlus(byteCodeGenerator->GetAllocator(), extraAlloc, ints);
        return;
    }

    bool arrayNumOpt = nativeArrays && pnode->sxArrLit.arrayOfNumbers;
    if (arrayNumOpt)
    {
        int extraAlloc = argCount * sizeof(double);
        Js::AuxArray<double> *doubles = AnewPlus(byteCodeGenerator->GetAllocator(), extraAlloc, Js::AuxArray<double>, argCount);
        EmitConstantArgsToFltArray(byteCodeGenerator, doubles->elements, args, argCount);
        Assert(!pnode->sxArrLit.hasMissingValues);
        byteCodeGenerator->Writer()->Auxiliary(
            Js::OpCode::NewScFltArray,
            pnode->location,
            doubles,
            sizeof(Js::AuxArray<double>) + extraAlloc,
            argCount);
        AdeletePlus(byteCodeGenerator->GetAllocator(), extraAlloc, doubles);
        return;
    }

    bool arrayLitOpt = pnode->sxArrLit.arrayOfTaggedInts && pnode->sxArrLit.count > 1;
    Assert(!arrayLitOpt || !nativeArrays);

    Js::RegSlot spreadArrLoc = arrayLocation;
    Js::AuxArray<uint32> *spreadIndices = nullptr;
    const size_t extraAlloc = spreadCount * sizeof(uint32);
    if (pnode->sxArrLit.spreadCount > 0)
    {
        arrayLocation = funcInfo->AcquireTmpRegister();
        spreadIndices = AnewPlus(byteCodeGenerator->GetAllocator(), extraAlloc, Js::AuxArray<uint32>, spreadCount);
    }

    byteCodeGenerator->Writer()->Reg1Unsigned1(
        pnode->sxArrLit.hasMissingValues ? Js::OpCode::NewScArrayWithMissingValues : Js::OpCode::NewScArray,
        arrayLocation,
        argCount);

    if (args!=NULL)
    {
        Js::OpCode opcode;
        Js::RegSlot arrLoc;
        if (argCount == 1 && !byteCodeGenerator->Writer()->DoProfileNewScArrayOp(Js::OpCode::NewScArray))
        {
            opcode = Js::OpCode::StArrItemC_CI4;
            arrLoc = arrayLocation;
        }
        else if (arrayLitOpt)
        {
            opcode = Js::OpCode::StArrSegItem_A;
            arrLoc = funcInfo->AcquireTmpRegister();
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::LdArrHead, arrLoc, arrayLocation);
        }
        else if (Js::JavascriptArray::HasInlineHeadSegment(argCount))
        {
            // The head segment will be allocated inline as an interior pointer. To keep the array alive, the set operation
            // should be done relative to the array header to keep it alive (instead of the array segment)
            opcode = Js::OpCode::StArrInlineItem_CI4;
            arrLoc = arrayLocation;
        }
        else if (argCount <= Js::JavascriptArray::MaxInitialDenseLength)
        {
            opcode = Js::OpCode::StArrSegItem_CI4;
            arrLoc = funcInfo->AcquireTmpRegister();
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::LdArrHead, arrLoc, arrayLocation);
        }
        else
        {
            opcode = Js::OpCode::StArrItemI_CI4;
            arrLoc = arrayLocation;
        }

        if (arrayLitOpt)
        {
            Js::VarArray *vars = AnewPlus(byteCodeGenerator->GetAllocator(), argCount * sizeof(Js::Var), Js::VarArray, argCount);

            EmitConstantArgsToVarArray(byteCodeGenerator, vars->elements, args, argCount);

            // Generate the opcode with vars
            byteCodeGenerator->Writer()->Auxiliary(Js::OpCode::StArrSegItem_A, arrLoc, vars, sizeof(Js::VarArray) + argCount * sizeof(Js::Var), argCount);

            AdeletePlus(byteCodeGenerator->GetAllocator(), argCount * sizeof(Js::Var), vars);
        }
        else
        {
            uint i = 0;
            unsigned spreadIndex = 0;
            Js::RegSlot rhsLocation;
            while (args->nop == knopList)
            {
                if (args->sxBin.pnode1->nop != knopEmpty)
                {
                    Emit(args->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
                    rhsLocation = args->sxBin.pnode1->location;
                    Js::RegSlot regVal = rhsLocation;
                    if (args->sxBin.pnode1->nop == knopEllipsis)
                    {
                        regVal = funcInfo->AcquireTmpRegister();
                        byteCodeGenerator->Writer()->Reg2(Js::OpCode::LdCustomSpreadIteratorList, regVal, rhsLocation);
                        spreadIndices->elements[spreadIndex++] = i;
                    }


                    byteCodeGenerator->Writer()->ElementUnsigned1(opcode, regVal, arrLoc, i);

                    if (args->sxBin.pnode1->nop == knopEllipsis)
                    {
                        funcInfo->ReleaseTmpRegister(regVal);
                    }

                    funcInfo->ReleaseLoc(args->sxBin.pnode1);
                }

                args = args->sxBin.pnode2;
                i++;
            }

            if (args->nop != knopEmpty)
            {
                Emit(args, byteCodeGenerator, funcInfo, false);
                rhsLocation = args->location;
                Js::RegSlot regVal = rhsLocation;
                if (args->nop == knopEllipsis)
                {
                    regVal = funcInfo->AcquireTmpRegister();
                    byteCodeGenerator->Writer()->Reg2(Js::OpCode::LdCustomSpreadIteratorList, regVal, rhsLocation);
                    spreadIndices->elements[spreadIndex] = i;
                }
                
                byteCodeGenerator->Writer()->ElementUnsigned1(opcode, regVal, arrLoc, i);
                
                if (args->nop == knopEllipsis)
                {
                    funcInfo->ReleaseTmpRegister(regVal);
                }

                funcInfo->ReleaseLoc(args);
                i++;
            }
            Assert(i <= argCount);
        }

        if (arrLoc != arrayLocation)
        {
            funcInfo->ReleaseTmpRegister(arrLoc);
        }
    }

    if (pnode->sxArrLit.spreadCount > 0)
    {
        byteCodeGenerator->Writer()->Reg2Aux(Js::OpCode::SpreadArrayLiteral, spreadArrLoc, arrayLocation, spreadIndices, sizeof(Js::AuxArray<uint32>) + extraAlloc, extraAlloc);
        AdeletePlus(byteCodeGenerator->GetAllocator(), extraAlloc, spreadIndices);
        funcInfo->ReleaseTmpRegister(arrayLocation);
    }
}



// FIX: TODO: mixed-mode expressions (arithmetic expressions mixed with boolean expressions); current solution
// will not short-circuit in some cases and is not complete (for example: var i=(x==y))
// This uses Aho and Ullman style double-branch generation (p. 494 ASU); we will need to peephole optimize or replace
// with special case for single-branch style.
void EmitBooleanExpression(ParseNode *expr,Js::ByteCodeLabel trueLabel, Js::ByteCodeLabel falseLabel,ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo)
{
    switch (expr->nop) {
    case knopLogOr: {
        byteCodeGenerator->StartStatement(expr);
        Js::ByteCodeLabel leftFalse=byteCodeGenerator->Writer()->DefineLabel();
        EmitBooleanExpression(expr->sxBin.pnode1,trueLabel,leftFalse,byteCodeGenerator,funcInfo);
        funcInfo->ReleaseLoc(expr->sxBin.pnode1);
        byteCodeGenerator->Writer()->MarkLabel(leftFalse);
        EmitBooleanExpression(expr->sxBin.pnode2,trueLabel,falseLabel,byteCodeGenerator,funcInfo);
        funcInfo->ReleaseLoc(expr->sxBin.pnode2);
        byteCodeGenerator->EndStatement(expr);
        break;
                    }
    case knopLogAnd: {
        byteCodeGenerator->StartStatement(expr);
        Js::ByteCodeLabel leftTrue=byteCodeGenerator->Writer()->DefineLabel();
        EmitBooleanExpression(expr->sxBin.pnode1,leftTrue,falseLabel,byteCodeGenerator,funcInfo);
        funcInfo->ReleaseLoc(expr->sxBin.pnode1);
        byteCodeGenerator->Writer()->MarkLabel(leftTrue);
        EmitBooleanExpression(expr->sxBin.pnode2,trueLabel,falseLabel,byteCodeGenerator,funcInfo);
        funcInfo->ReleaseLoc(expr->sxBin.pnode2);
        byteCodeGenerator->EndStatement(expr);
        break;
                     }
    case knopLogNot:
        byteCodeGenerator->StartStatement(expr);
        EmitBooleanExpression(expr->sxUni.pnode1,falseLabel,trueLabel,byteCodeGenerator,funcInfo);
        funcInfo->ReleaseLoc(expr->sxUni.pnode1);
        byteCodeGenerator->EndStatement(expr);
        break;

    case knopEq:
    case knopEqv:
    case knopNEqv:
    case knopNe:
    case knopLt:
    case knopLe:
    case knopGe:
    case knopGt:
        byteCodeGenerator->StartStatement(expr);
        EmitBinaryOpnds(expr->sxBin.pnode1, expr->sxBin.pnode2, byteCodeGenerator, funcInfo);
        funcInfo->ReleaseLoc(expr->sxBin.pnode2);
        funcInfo->ReleaseLoc(expr->sxBin.pnode1);
        byteCodeGenerator->Writer()->BrReg2(nopToOp[expr->nop],trueLabel,expr->sxBin.pnode1->location,
            expr->sxBin.pnode2->location);
        byteCodeGenerator->Writer()->Br(falseLabel);
        byteCodeGenerator->EndStatement(expr);
        break;
    case knopTrue:
        byteCodeGenerator->StartStatement(expr);
        byteCodeGenerator->Writer()->Br(trueLabel);
        byteCodeGenerator->EndStatement(expr);
        break;
    case knopFalse:
        byteCodeGenerator->StartStatement(expr);
        byteCodeGenerator->Writer()->Br(falseLabel);
        byteCodeGenerator->EndStatement(expr);
        break;
    default:
        // Note: we usually release the temp assigned to a node after we Emit it.
        // But in this case, EmitBooleanExpression is just a wrapper around a normal Emit call,
        // and the caller of EmitBooleanExpression expects to be able to release this register.

        // for diagnostics purpose, register the name and dot to the statement list
        if (expr->nop == knopName || expr->nop == knopDot)
        {
            byteCodeGenerator->StartStatement(expr);
            Emit(expr, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrTrue_A,trueLabel,expr->location);
            byteCodeGenerator->Writer()->Br(falseLabel);
            byteCodeGenerator->EndStatement(expr);
        }
        else
        {
            Emit(expr, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrTrue_A,trueLabel,expr->location);
            byteCodeGenerator->Writer()->Br(falseLabel);
        }
        break;
    }
}

// used by while and for loops
void EmitLoop(
    ParseNode *loopNode,
    ParseNode *cond,
    ParseNode *body,
    ParseNode *incr,
    ByteCodeGenerator *byteCodeGenerator,
    FuncInfo *funcInfo,
    BOOL fReturnValue,
    BOOL doWhile = false)
{
    // Need to increment loop count whether we are going to profile or not for HasLoop()

    Js::ByteCodeLabel loopEntrance = byteCodeGenerator->Writer()->DefineLabel();
    Js::ByteCodeLabel continuePastLoop = byteCodeGenerator->Writer()->DefineLabel();

    uint loopId = byteCodeGenerator->Writer()->EnterLoop(loopEntrance);
    loopNode->sxLoop.loopId = loopId;

    if (doWhile)
    {
        Emit(body, byteCodeGenerator, funcInfo, fReturnValue);
        funcInfo->ReleaseLoc(body);
        if (loopNode->emitLabels)
        {
            byteCodeGenerator->Writer()->MarkLabel(loopNode->sxStmt.continueLabel);
        }
        if (!ByteCodeGenerator::IsFalse(cond) ||
            byteCodeGenerator->IsInDebugMode())
        {
            EmitBooleanExpression(cond, loopEntrance, continuePastLoop, byteCodeGenerator, funcInfo);
        }
        funcInfo->ReleaseLoc(cond);
    }
    else
    {
        if (cond)
        {
            if (!(cond->nop == knopInt &&
                cond->sxInt.lw != 0))
            {
                Js::ByteCodeLabel trueLabel = byteCodeGenerator->Writer()->DefineLabel();
                EmitBooleanExpression(cond, trueLabel, continuePastLoop, byteCodeGenerator, funcInfo);
                byteCodeGenerator->Writer()->MarkLabel(trueLabel);
            }
            funcInfo->ReleaseLoc(cond);
        }
        Emit(body, byteCodeGenerator, funcInfo, fReturnValue);
        funcInfo->ReleaseLoc(body);

        if (loopNode->emitLabels)
        {
            byteCodeGenerator->Writer()->MarkLabel(loopNode->sxStmt.continueLabel);
        }
        if (incr!=NULL)
        {
            Emit(incr, byteCodeGenerator, funcInfo, false);
            funcInfo->ReleaseLoc(incr);
        }
        byteCodeGenerator->Writer()->Br(loopEntrance);
    }
    byteCodeGenerator->Writer()->MarkLabel(continuePastLoop);
    if (loopNode->emitLabels)
    {
        byteCodeGenerator->Writer()->MarkLabel(loopNode->sxStmt.breakLabel);
    }

    byteCodeGenerator->Writer()->ExitLoop(loopId);
}

void ByteCodeGenerator::EmitInvertedLoop(ParseNode* outerLoop,ParseNode* invertedLoop,FuncInfo* funcInfo) {
    Js::ByteCodeLabel invertedLoopLabel=this->m_writer.DefineLabel();
    Js::ByteCodeLabel afterInvertedLoop=this->m_writer.DefineLabel();
    // emit branch around original

    Emit(outerLoop->sxFor.pnodeInit, this, funcInfo, false);
    funcInfo->ReleaseLoc(outerLoop->sxFor.pnodeInit);
    this->m_writer.BrS(Js::OpCode::BrNotHasSideEffects,invertedLoopLabel,
        Js::SideEffects_Any);

    // emit original

    EmitLoop(outerLoop,outerLoop->sxFor.pnodeCond,outerLoop->sxFor.pnodeBody,
        outerLoop->sxFor.pnodeIncr,this,funcInfo,false);

    // clear temporary registers since inverted loop may share nodes with
    // emitted original loop
    VisitClearTmpRegs(outerLoop, this, funcInfo);

    // emit branch around inverted
    this->m_writer.Br(afterInvertedLoop);
    this->m_writer.MarkLabel(invertedLoopLabel);

    // Emit a zero trip test for the original outerloop
    Js::ByteCodeLabel zeroTrip = this->m_writer.DefineLabel();
    ParseNode* testNode = this->GetParser()->CopyPnode(outerLoop->sxFor.pnodeCond);
    EmitBooleanExpression(testNode,zeroTrip,afterInvertedLoop,this,funcInfo);
    this->m_writer.MarkLabel(zeroTrip);
    funcInfo->ReleaseLoc(testNode);

    // emit inverted
    Emit(invertedLoop->sxFor.pnodeInit, this, funcInfo, false);
    funcInfo->ReleaseLoc(invertedLoop->sxFor.pnodeInit);
    EmitLoop(invertedLoop,invertedLoop->sxFor.pnodeCond,invertedLoop->sxFor.pnodeBody,
        invertedLoop->sxFor.pnodeIncr,this,funcInfo,false);
    this->m_writer.MarkLabel(afterInvertedLoop);
}

void EmitGetIterator(Js::RegSlot iteratorLocation, Js::RegSlot iterableLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo)
{
    // get iterator object from the iterable
    EmitInvoke(iteratorLocation, iterableLocation, Js::PropertyIds::_symbolIterator, byteCodeGenerator, funcInfo);

    // throw TypeError if the result is not an object
    Js::ByteCodeLabel skipThrow = byteCodeGenerator->Writer()->DefineLabel();
    byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrOnObject_A, skipThrow, iteratorLocation);
    byteCodeGenerator->Writer()->W1(Js::OpCode::RuntimeTypeError, SCODE_CODE(JSERR_NeedObject));
    byteCodeGenerator->Writer()->MarkLabel(skipThrow);
}

void EmitIteratorNext(Js::RegSlot itemLocation, Js::RegSlot iteratorLocation, Js::RegSlot nextInputLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo)
{
    // invoke next() on the iterator
    if (nextInputLocation == Js::Constants::NoRegister)
    {
        EmitInvoke(itemLocation, iteratorLocation, Js::PropertyIds::next, byteCodeGenerator, funcInfo);
    }
    else
    {
        EmitInvoke(itemLocation, iteratorLocation, Js::PropertyIds::next, byteCodeGenerator, funcInfo, nextInputLocation);
    }

    // throw TypeError if the result is not an object
    Js::ByteCodeLabel skipThrow = byteCodeGenerator->Writer()->DefineLabel();
    byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrOnObject_A, skipThrow, itemLocation);
    byteCodeGenerator->Writer()->W1(Js::OpCode::RuntimeTypeError, SCODE_CODE(JSERR_NeedObject));
    byteCodeGenerator->Writer()->MarkLabel(skipThrow);
}

void EmitIteratorComplete(Js::RegSlot doneLocation, Js::RegSlot iteratorResultLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo)
{
    // get the iterator result's "done" property
    uint cacheId = funcInfo->FindOrAddInlineCacheId(iteratorResultLocation, Js::PropertyIds::done, false, false);
    byteCodeGenerator->Writer()->PatchableProperty(Js::OpCode::LdFld, doneLocation, iteratorResultLocation, cacheId);

    // Do not need to do ToBoolean explicitly with current uses of EmitIteratorComplete since BrTrue_A does this.
    // Add a ToBoolean controlled by template flag if needed for new uses later on.
}

void EmitIteratorValue(Js::RegSlot valueLocation, Js::RegSlot iteratorResultLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo)
{
    // get the iterator result's "value" property
    uint cacheId = funcInfo->FindOrAddInlineCacheId(iteratorResultLocation, Js::PropertyIds::value, false, false);
    byteCodeGenerator->Writer()->PatchableProperty(Js::OpCode::LdFld, valueLocation, iteratorResultLocation, cacheId);
}

void EmitForInOrForOf(ParseNode *loopNode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, BOOL fReturnValue)
{
    bool isForIn = (loopNode->nop == knopForIn);
    Assert(isForIn || loopNode->nop == knopForOf);

    byteCodeGenerator->StartStatement(loopNode);
    funcInfo->AcquireLoc(loopNode);

    BeginEmitBlock(loopNode->sxForInOrForOf.pnodeBlock, byteCodeGenerator, funcInfo);

    // Record the branch bytecode offset.
    // This is used for "ignore exception" and "set next stmt" scenarios. See ProbeContainer::GetNextUserStatementOffsetForAdvance:
    // if there is a branch recorded between current offset and next stmt offset, we'll use offset of the branch recorded,
    // otherwise use offset of next stmt.
    // The idea here is that when we bail out after ignore exception, we need to bail out to the beginning of the ForIn,
    // but currently ForIn stmt starts at the condition part, which is needed for correct handling of break point on ForIn
    // (break every time on the loop back edge) and correct display of current statement under debugger.
    // See WinBlue 231880 for details.
    byteCodeGenerator->Writer()->RecordStatementAdjustment(Js::FunctionBody::SAT_All);
    Js::ByteCodeLabel loopEntrance = byteCodeGenerator->Writer()->DefineLabel();
    Js::ByteCodeLabel continuePastLoop = byteCodeGenerator->Writer()->DefineLabel();
    Js::ByteCodeLabel skipPastLoop = byteCodeGenerator->Writer()->DefineLabel();

    if (loopNode->sxForInOrForOf.pnodeLval->nop == knopVarDecl)
    {
        EmitReference(loopNode->sxForInOrForOf.pnodeLval, byteCodeGenerator, funcInfo);
    }

    Emit(loopNode->sxForInOrForOf.pnodeObj, byteCodeGenerator, funcInfo, false); // evaluate collection expression
    funcInfo->ReleaseLoc(loopNode->sxForInOrForOf.pnodeObj);

    // Grab registers for the enumerator and for the current enumerated item.
    // The enumerator register will be released after this call returns.
    loopNode->sxForInOrForOf.itemLocation = funcInfo->AcquireTmpRegister();

    if (isForIn)
    {
        // get enumerator from the collection
        byteCodeGenerator->Writer()->Reg2(Js::OpCode::GetForInEnumerator, loopNode->location, loopNode->sxForInOrForOf.pnodeObj->location);
    }
    else
    {
        // We want call profile information on the @@iterator call, so instead of adding a GetForOfIterator bytecode op
        // to do all the following work in a helper do it explicitly in bytecode so that the @@iterator call is exposed
        // to the profiler and JIT.

        // if collection is null or undefined don't enter the loop
        byteCodeGenerator->Writer()->BrReg2(Js::OpCode::BrSrEq_A, skipPastLoop, loopNode->sxForInOrForOf.pnodeObj->location, funcInfo->nullConstantRegister);
        byteCodeGenerator->Writer()->BrReg2(Js::OpCode::BrSrEq_A, skipPastLoop, loopNode->sxForInOrForOf.pnodeObj->location, funcInfo->undefinedConstantRegister);

        // do a ToObject on the collection
        Js::RegSlot tmpObj = funcInfo->AcquireTmpRegister();
        byteCodeGenerator->Writer()->Reg2(Js::OpCode::Conv_Obj, tmpObj, loopNode->sxForInOrForOf.pnodeObj->location);

        EmitGetIterator(loopNode->location, tmpObj, byteCodeGenerator, funcInfo);
        funcInfo->ReleaseTmpRegister(tmpObj);
    }
    byteCodeGenerator->EndStatement(loopNode);

    // Need to increment loop count whether we are gointo profile or not for HasLoop()
    uint loopId = byteCodeGenerator->Writer()->EnterLoop(loopEntrance);
    loopNode->sxForInOrForOf.loopId = loopId;

    byteCodeGenerator->StartStatement(loopNode->sxForInOrForOf.pnodeLval);

    if (isForIn)
    {
        // branch past loop when GetCurrentAndMoveNext returns NULL
        byteCodeGenerator->Writer()->BrReg2(Js::OpCode::BrOnEmpty, continuePastLoop, loopNode->sxForInOrForOf.itemLocation, loopNode->location);
    }
    else
    {
        EmitIteratorNext(loopNode->sxForInOrForOf.itemLocation, loopNode->location, Js::Constants::NoRegister, byteCodeGenerator, funcInfo);

        Js::RegSlot doneLocation = funcInfo->AcquireTmpRegister();
        EmitIteratorComplete(doneLocation, loopNode->sxForInOrForOf.itemLocation, byteCodeGenerator, funcInfo);

        // branch past loop if the result's done property is truthy
        byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrTrue_A, continuePastLoop, doneLocation);
        funcInfo->ReleaseTmpRegister(doneLocation);

        // otherwise put result's value property in itemLocation
        EmitIteratorValue(loopNode->sxForInOrForOf.itemLocation, loopNode->sxForInOrForOf.itemLocation, byteCodeGenerator, funcInfo);
    }
    byteCodeGenerator->EndStatement(loopNode->sxForInOrForOf.pnodeLval);


    if (loopNode->sxForInOrForOf.pnodeLval->nop != knopVarDecl &&
        loopNode->sxForInOrForOf.pnodeLval->nop != knopLetDecl)
    {
        EmitReference(loopNode->sxForInOrForOf.pnodeLval, byteCodeGenerator, funcInfo);
    }
    else
    {
        Symbol * sym = loopNode->sxForInOrForOf.pnodeLval->sxVar.sym;
        sym->SetNeedDeclaration(false);
    }
    EmitAssignment(NULL, loopNode->sxForInOrForOf.pnodeLval, loopNode->sxForInOrForOf.itemLocation, byteCodeGenerator, funcInfo);
    funcInfo->ReleaseReference(loopNode->sxForInOrForOf.pnodeLval);

    Emit(loopNode->sxForInOrForOf.pnodeBody, byteCodeGenerator, funcInfo, fReturnValue);
    funcInfo->ReleaseLoc(loopNode->sxForInOrForOf.pnodeBody);
    funcInfo->ReleaseTmpRegister(loopNode->sxForInOrForOf.itemLocation);
    if (loopNode->emitLabels)
    {
        byteCodeGenerator->Writer()->MarkLabel(loopNode->sxForInOrForOf.continueLabel);
    }
    byteCodeGenerator->Writer()->Br(loopEntrance);
    byteCodeGenerator->Writer()->MarkLabel(continuePastLoop);
    if (loopNode->emitLabels)
    {
        byteCodeGenerator->Writer()->MarkLabel(loopNode->sxForInOrForOf.breakLabel);
    }
    byteCodeGenerator->Writer()->ExitLoop(loopId);

    if (isForIn)
    {
        byteCodeGenerator->Writer()->Reg1(Js::OpCode::ReleaseForInEnumerator, loopNode->location);
    }
    else
    {
        byteCodeGenerator->Writer()->MarkLabel(skipPastLoop);
    }

    EndEmitBlock(loopNode->sxForInOrForOf.pnodeBlock, byteCodeGenerator, funcInfo);
}

void EmitArrayLiteral(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    funcInfo->AcquireLoc(pnode);
    ParseNode *args=pnode->sxUni.pnode1;
    if (args == NULL)
    {
        byteCodeGenerator->Writer()->Reg1Unsigned1(
            pnode->sxArrLit.hasMissingValues ? Js::OpCode::NewScArrayWithMissingValues : Js::OpCode::NewScArray,
            pnode->location,
            ByteCodeGenerator::DefaultArraySize);
    }
    else
    {
        SetNewArrayElements(pnode, pnode->location, byteCodeGenerator, funcInfo);
    }
}

void EmitJumpCleanup(ParseNode *pnode, ParseNode *pnodeTarget, ByteCodeGenerator *byteCodeGenerator, FuncInfo * funcInfo) {
    for (; pnode != pnodeTarget; pnode = pnode->sxStmt.pnodeOuter)
    {
        switch(pnode->nop) {
        case knopTry:
        case knopCatch:
        case knopFinally:
            // We insert OpCode::Leave when there is a 'return' inside try/catch/finally.
            // This is for flow control and does not participate in identifying boundaries of try/catch blocks,
            // thus we shouldn't call RecordCrossFrameEntryExitRecord() here.
            byteCodeGenerator->Writer()->Empty(Js::OpCode::Leave);
            break;
        case knopWhile:
        case knopDoWhile:
        case knopFor:
        case knopForIn:
        case knopForOf:
            if (Js::DynamicProfileInfo::EnableImplicitCallFlags(funcInfo->GetParsedFunctionBody()))
            {
                byteCodeGenerator->Writer()->Unsigned1(Js::OpCode::ProfiledLoopEnd, pnode->sxLoop.loopId);
            }
        }
    }
}

void EmitBinaryOpnds(ParseNode *pnode1, ParseNode *pnode2, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    // If opnd2 can overwrite opnd1, make sure the value of opnd1 is stashed away.
    if (MayHaveSideEffectOnNode(pnode1, pnode2))
    {
        SaveOpndValue(pnode1, funcInfo);
    }
    Emit(pnode1, byteCodeGenerator, funcInfo, false);
    Emit(pnode2, byteCodeGenerator, funcInfo, false);
}

void EmitBinaryReference(ParseNode *pnode1, ParseNode *pnode2, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, BOOL fLoadLhs)
{
    // Make sure that the RHS of an assignment doesn't kill the opnd's of the expression on the LHS.
    switch (pnode1->nop)
    {
    case knopName:
        if (fLoadLhs && MayHaveSideEffectOnNode(pnode1, pnode2))
        {
            // Given x op y, y may kill x, so stash x.
            // Note that this only matters if we're loading x prior to the op.
            SaveOpndValue(pnode1, funcInfo);
        }
        break;
    case knopDot:
        if (fLoadLhs)
        {
            // We're loading the value of the LHS before the RHS, so make sure the LHS gets a register first.
            funcInfo->AcquireLoc(pnode1);
        }
        if (MayHaveSideEffectOnNode(pnode1->sxBin.pnode1, pnode2))
        {
            // Given x.y op z, z may kill x, so stash x away.
            SaveOpndValue(pnode1->sxBin.pnode1, funcInfo);
        }
        break;
    case knopIndex:
        if (fLoadLhs)
        {
            // We're loading the value of the LHS before the RHS, so make sure the LHS gets a register first.
            funcInfo->AcquireLoc(pnode1);
        }
        if (MayHaveSideEffectOnNode(pnode1->sxBin.pnode1, pnode2) ||
            MayHaveSideEffectOnNode(pnode1->sxBin.pnode1, pnode1->sxBin.pnode2))
        {
            // Given x[y] op z, y or z may kill x, so stash x away.
            SaveOpndValue(pnode1->sxBin.pnode1, funcInfo);
        }
        if (MayHaveSideEffectOnNode(pnode1->sxBin.pnode2, pnode2))
        {
            // Given x[y] op z, z may kill y, so stash y away.
            // But make sure that x gets a register before y.
            funcInfo->AcquireLoc(pnode1->sxBin.pnode1);
            SaveOpndValue(pnode1->sxBin.pnode2, funcInfo);
        }
        break;
    }
    if (fLoadLhs)
    {
        // Emit code to load the value of the LHS.
        EmitLoad(pnode1, byteCodeGenerator, funcInfo);
    }
    else
    {
        // Emit code to evaluate the LHS opnds, but don't load the LHS's value.
        EmitReference(pnode1, byteCodeGenerator, funcInfo);
    }
    // Evaluate the RHS.
    Emit(pnode2, byteCodeGenerator, funcInfo, false);
}

extern void printNop(int);

void EmitUseBeforeDeclarationRuntimeError(ByteCodeGenerator * byteCodeGenerator, Js::RegSlot location, bool fLoadUndef)
{
    byteCodeGenerator->Writer()->W1(Js::OpCode::RuntimeReferenceError, SCODE_CODE(JSERR_UseBeforeDeclaration));
    // load something into register in order to do not confuse IRBuilder. This value will never be used.
    if (fLoadUndef)
    {
        byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdUndef, location);
    }
}

bool EmitUseBeforeDeclaration(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    Symbol *sym = NULL;
    bool fAcquireLoc = true;
    if (pnode->nop == knopName)
    {
        sym = pnode->sxPid.sym;
    }
    else if (pnode->nop == knopVarDecl)
    {
        fAcquireLoc = false;
        sym = pnode->sxVar.sym;
    }
    else if ((ParseNode::Grfnop(pnode->nop) & fnopAsg) != 0)
    {
        if ((ParseNode::Grfnop(pnode->nop) & fnopBin) != 0 && pnode->sxBin.pnode1->nop == knopName)
        {
            sym = pnode->sxBin.pnode1->sxPid.sym;
        }
        if ((ParseNode::Grfnop(pnode->nop) & fnopUni) != 0 && pnode->sxUni.pnode1->nop == knopName)
        {
            sym = pnode->sxUni.pnode1->sxPid.sym;
        }
    }
    // Don't emit static use-before-declaration error in a closure or dynamic scope case. We detect such cases with dynamic checks,
    // if necessary.
    if (sym != NULL && sym->GetNeedDeclaration() && byteCodeGenerator->GetCurrentScope()->HasStaticPathToAncestor(sym->GetScope()) && sym->GetScope()->GetFunc() == funcInfo)
    {
        if (fAcquireLoc)
        {
            funcInfo->AcquireLoc(pnode);
        }
        EmitUseBeforeDeclarationRuntimeError(byteCodeGenerator, pnode->location, fAcquireLoc);
        return true;
    }
    return false;
}

void EmitBinary(Js::OpCode opcode, ParseNode * pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    byteCodeGenerator->StartStatement(pnode);
    EmitBinaryOpnds(pnode->sxBin.pnode1, pnode->sxBin.pnode2, byteCodeGenerator, funcInfo);
    funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
    funcInfo->ReleaseLoc(pnode->sxBin.pnode1);
    funcInfo->AcquireLoc(pnode);
    byteCodeGenerator->Writer()->Reg3(opcode,
        pnode->location,
        pnode->sxBin.pnode1->location,
        pnode->sxBin.pnode2->location);
    byteCodeGenerator->EndStatement(pnode);
    RECORD_LOAD(pnode);
}


bool CollectConcat(ParseNode * pnodeAdd, DListCounted<ParseNode *, ArenaAllocator>& concatOpnds,
                   ArenaAllocator * arenaAllocator)
{
    Assert(pnodeAdd->nop == knopAdd);
    Assert(pnodeAdd->CanFlattenConcatExpr());

    bool doConcatString = false;
    DList<ParseNode*, ArenaAllocator> pnodeStack(arenaAllocator);
    pnodeStack.Prepend(pnodeAdd->sxBin.pnode2);
    ParseNode * pnode = pnodeAdd->sxBin.pnode1;
    while (true)
    {
        if (!pnode->CanFlattenConcatExpr())
        {
            concatOpnds.Append(pnode);
        }
        else if (pnode->nop == knopStr)
        {
            concatOpnds.Append(pnode);

            // Detect if there are any string larger then the append size limit
            // If there are, we can do concat, other wise, still use add so we will not lose the AddLeftDead oppurtunities
            doConcatString = doConcatString || !Js::CompoundString::ShouldAppendChars(pnode->sxPid.pid->Cch());
        }
        else
        {
            Assert(pnode->nop == knopAdd);
            pnodeStack.Prepend(pnode->sxBin.pnode2);
            pnode = pnode->sxBin.pnode1;
            continue;
        }

        if (pnodeStack.Empty())
        {
            break;
        }
        pnode = pnodeStack.Head();
        pnodeStack.RemoveHead();
    }
    return doConcatString;
}

void EmitConcat3(ParseNode * pnode, ParseNode * pnode1, ParseNode * pnode2, ParseNode * pnode3, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    byteCodeGenerator->StartStatement(pnode);
    if (MayHaveSideEffectOnNode(pnode1, pnode2) || MayHaveSideEffectOnNode(pnode1, pnode3))
    {
        SaveOpndValue(pnode1, funcInfo);
    }
    if (MayHaveSideEffectOnNode(pnode2, pnode3))
    {
        SaveOpndValue(pnode2, funcInfo);
    }

    Emit(pnode1, byteCodeGenerator, funcInfo, false);
    Emit(pnode2, byteCodeGenerator, funcInfo, false);
    Emit(pnode3, byteCodeGenerator, funcInfo, false);
    funcInfo->ReleaseLoc(pnode3);
    funcInfo->ReleaseLoc(pnode2);
    funcInfo->ReleaseLoc(pnode1);
    funcInfo->AcquireLoc(pnode);
    byteCodeGenerator->Writer()->Reg4(Js::OpCode::Concat3,
        pnode->location,
        pnode1->location,
        pnode2->location,
        pnode3->location);
    byteCodeGenerator->EndStatement(pnode);
    RECORD_LOAD(pnode);
}

void EmitNewConcatStrMulti(ParseNode * pnode, uint8 count, ParseNode * pnode1, ParseNode * pnode2,
                    ByteCodeGenerator * byteCodeGenerator, FuncInfo * funcInfo)
{
    EmitBinaryOpnds(pnode1, pnode2, byteCodeGenerator, funcInfo);
    funcInfo->ReleaseLoc(pnode2);
    funcInfo->ReleaseLoc(pnode1);
    funcInfo->AcquireLoc(pnode);
    byteCodeGenerator->Writer()->Reg3B1(Js::OpCode::NewConcatStrMulti,
        pnode->location,
        pnode1->location,
        pnode2->location,
        count);
}

void EmitAdd(ParseNode * pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo)
{
    Assert(pnode->nop == knopAdd);

    // Don't do the optimization on compat version 8, as concat may call ToPrimitive that swallow exception
    // We just didn't bother to implementat that for IE8 doc mode
    if (pnode->CanFlattenConcatExpr())
    {
        // We should only have a string concat if the feature is on.
        Assert(!PHASE_OFF1(Js::ByteCodeConcatExprOptPhase));
        DListCounted<ParseNode*, ArenaAllocator> concatOpnds(byteCodeGenerator->GetAllocator());
        bool doConcatString = CollectConcat(pnode, concatOpnds, byteCodeGenerator->GetAllocator());
        if (doConcatString)
        {
            uint concatCount = concatOpnds.Count();
            Assert(concatCount >= 2);

            // Don't do concatN if the number is too high
            // CONSIDER:: although we could have done multiple ConcatNs
            if (concatCount > 2 && concatCount <= UINT8_MAX)
            {
#if DBG
                wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
#endif
                ParseNode * pnode1 = concatOpnds.Head();
                concatOpnds.RemoveHead();
                ParseNode * pnode2 = concatOpnds.Head();
                concatOpnds.RemoveHead();
                if (concatCount == 3)
                {
                    OUTPUT_TRACE_DEBUGONLY(Js::ByteCodeConcatExprOptPhase, L"%s(%s) offset:#%d : Concat3\n",
                        funcInfo->GetParsedFunctionBody()->GetDisplayName(),
                        funcInfo->GetParsedFunctionBody()->GetDebugNumberSet(debugStringBuffer),
                        byteCodeGenerator->Writer()->ByteCodeDataSize());
                    EmitConcat3(pnode, pnode1, pnode2, concatOpnds.Head(), byteCodeGenerator, funcInfo);
                    return;
                }


                OUTPUT_TRACE_DEBUGONLY(Js::ByteCodeConcatExprOptPhase, L"%s(%s) offset:#%d: ConcatMulti %d\n",
                    funcInfo->GetParsedFunctionBody()->GetDisplayName(),
                    funcInfo->GetParsedFunctionBody()->GetDebugNumberSet(debugStringBuffer),
                    byteCodeGenerator->Writer()->ByteCodeDataSize(), concatCount);
                byteCodeGenerator->StartStatement(pnode);
                funcInfo->AcquireLoc(pnode);

                // CONSIDER: this may cause the backend not able CSE repeating pattern within the concat
                EmitNewConcatStrMulti(pnode, concatCount, pnode1, pnode2, byteCodeGenerator, funcInfo);

                uint i = 2;
                do
                {
                    ParseNode * currNode = concatOpnds.Head();
                    concatOpnds.RemoveHead();
                    ParseNode * currNode2 = concatOpnds.Head();
                    concatOpnds.RemoveHead();

                    EmitBinaryOpnds(currNode, currNode2, byteCodeGenerator, funcInfo);
                    funcInfo->ReleaseLoc(currNode2);
                    funcInfo->ReleaseLoc(currNode);
                    byteCodeGenerator->Writer()->Reg3B1(
                        Js::OpCode::SetConcatStrMultiItem2, pnode->location, currNode->location, currNode2->location, i);
                    i += 2;
                }
                while (concatOpnds.Count() > 1);

                if (!concatOpnds.Empty())
                {
                    ParseNode * currNode = concatOpnds.Head();
                    Emit(currNode, byteCodeGenerator, funcInfo, false);
                    funcInfo->ReleaseLoc(currNode);
                    byteCodeGenerator->Writer()->Reg2B1(
                        Js::OpCode::SetConcatStrMultiItem, pnode->location, currNode->location, i);
                    i++;
                }

                Assert(concatCount == i);
                byteCodeGenerator->EndStatement(pnode);
                return;
            }
        }

        // Since we collected all the node already, let's just emit them instead of doing it recursively
        byteCodeGenerator->StartStatement(pnode);
        ParseNode * currNode = concatOpnds.Head();
        concatOpnds.RemoveHead();
        ParseNode * currNode2 = concatOpnds.Head();
        concatOpnds.RemoveHead();

        EmitBinaryOpnds(currNode, currNode2, byteCodeGenerator, funcInfo);
        funcInfo->ReleaseLoc(currNode2);
        funcInfo->ReleaseLoc(currNode);
        Js::RegSlot dstReg = funcInfo->AcquireLoc(pnode);
        byteCodeGenerator->Writer()->Reg3(
            Js::OpCode::Add_A, dstReg, currNode->location, currNode2->location);
        while (!concatOpnds.Empty())
        {
            currNode = concatOpnds.Head();
            concatOpnds.RemoveHead();
            Emit(currNode, byteCodeGenerator, funcInfo, false);
            funcInfo->ReleaseLoc(currNode);
            byteCodeGenerator->Writer()->Reg3(
                Js::OpCode::Add_A, dstReg, dstReg, currNode->location);
        }
        byteCodeGenerator->EndStatement(pnode);
    }
    else
    {
        EmitBinary(Js::OpCode::Add_A, pnode, byteCodeGenerator, funcInfo);
    }
}

void EmitSuperFieldPatch(FuncInfo* funcInfo, ParseNode* pnode, ByteCodeGenerator* byteCodeGenerator)
{
    ParseNodePtr propFuncNode = funcInfo->root;

    if (byteCodeGenerator->GetFlags() & fscrEval)
    {
        // If we are inside an eval, ScopedLdSuper will take care of the patch.
        return;
    }

    if (funcInfo->IsLambda())
    {
        FuncInfo *parent = byteCodeGenerator->FindEnclosingNonLambda();
        propFuncNode = parent->root;
    }

    // No need to emit a LdFld for the constructor.
    if (propFuncNode->sxFnc.IsClassConstructor())
    {
        return;
    }

    if (!propFuncNode->sxFnc.IsClassMember() || propFuncNode->sxFnc.pid == nullptr)
    {
        // Non-methods will fail lookup.
        return;
    }
    if (propFuncNode->sxFnc.pid->GetPropertyId() == Js::Constants::NoProperty)
    {
        byteCodeGenerator->AssignPropertyId(propFuncNode->sxFnc.pid);
    }

    // Load the current method's property ID from super instead of using super directly.
    Js::RegSlot superLoc = funcInfo->superRegister;
    pnode->sxCall.pnodeTarget->location = Js::Constants::NoRegister;
    Js::RegSlot superPropLoc = funcInfo->AcquireLoc(pnode->sxCall.pnodeTarget);
    Js::PropertyId propertyId = propFuncNode->sxFnc.pid->GetPropertyId();
    uint cacheId = funcInfo->FindOrAddInlineCacheId(superLoc, propertyId, true, false);
    byteCodeGenerator->Writer()->PatchableProperty(Js::OpCode::LdMethodFld, superPropLoc, superLoc, cacheId);

    propFuncNode->sxFnc.pnodeNames =  nullptr;
}

struct ByteCodeGenerator::TryScopeRecord : public JsUtil::DoublyLinkedListElement<TryScopeRecord>
{
    Js::OpCode op;
    Js::ByteCodeLabel label;
    Js::RegSlot reg1;
    Js::RegSlot reg2;

    TryScopeRecord(Js::OpCode op, Js::ByteCodeLabel label) : op(op), label(label), reg1(Js::Constants::NoRegister), reg2(Js::Constants::NoRegister) { }
    TryScopeRecord(Js::OpCode op, Js::ByteCodeLabel label, Js::RegSlot r1, Js::RegSlot r2) : op(op), label(label), reg1(r1), reg2(r2) { }
};

void ByteCodeGenerator::EmitLeaveOpCodesBeforeYield()
{
    for (TryScopeRecord* node = this->tryScopeRecordsList.Tail(); node != nullptr; node = node->Previous())
    {
        switch (node->op)
        {
        case Js::OpCode::TryFinallyWithYield:
            this->Writer()->Empty(Js::OpCode::LeaveNull);
            break;
        case Js::OpCode::TryCatch:
        case Js::OpCode::ResumeFinally:
        case Js::OpCode::ResumeCatch:
            this->Writer()->Empty(Js::OpCode::Leave);
            break;
        default:
            AssertMsg(false, "Unexpected OpCode before Yield in the Try-Catch-Finally cache for generator!");
            break;
        }
    }
}

void ByteCodeGenerator::EmitTryBlockHeadersAfterYield()
{
    for (TryScopeRecord* node = this->tryScopeRecordsList.Head(); node != nullptr; node = node->Next())
    {
        switch (node->op)
        {
        case Js::OpCode::TryCatch:
            this->Writer()->Br(node->op, node->label);
            break;
        case Js::OpCode::TryFinallyWithYield:
        case Js::OpCode::ResumeFinally:
            this->Writer()->BrReg2(node->op, node->label, node->reg1, node->reg2);
            break;
        case Js::OpCode::ResumeCatch:
            this->Writer()->Empty(node->op);
            break;
        default:
            AssertMsg(false, "Unexpected OpCode after yield in the Try-Catch-Finally cache for generator!");
            break;
        }
    }
}

void EmitYield(Js::RegSlot inputLocation, Js::RegSlot resultLocation, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo, Js::RegSlot yieldStarIterator = Js::Constants::NoRegister)
{
    byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, funcInfo->yieldRegister, inputLocation);

    byteCodeGenerator->EmitLeaveOpCodesBeforeYield();
    byteCodeGenerator->Writer()->Reg2(Js::OpCode::Yield, funcInfo->yieldRegister, funcInfo->yieldRegister);
    byteCodeGenerator->EmitTryBlockHeadersAfterYield();

    if (yieldStarIterator == Js::Constants::NoRegister)
    {
        byteCodeGenerator->Writer()->Reg2(Js::OpCode::ResumeYield, resultLocation, funcInfo->yieldRegister);
    }
    else
    {
        byteCodeGenerator->Writer()->Reg3(Js::OpCode::ResumeYieldStar, resultLocation, funcInfo->yieldRegister, yieldStarIterator);
    }
}

void EmitYieldStar(ParseNode* yieldStarNode, ByteCodeGenerator* byteCodeGenerator, FuncInfo* funcInfo)
{
    funcInfo->AcquireLoc(yieldStarNode);

    Js::ByteCodeLabel loopEntrance = byteCodeGenerator->Writer()->DefineLabel();
    Js::ByteCodeLabel continuePastLoop = byteCodeGenerator->Writer()->DefineLabel();

    Js::RegSlot iteratorLocation = funcInfo->AcquireTmpRegister();

    // Evaluate operand
    Emit(yieldStarNode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
    funcInfo->ReleaseLoc(yieldStarNode->sxUni.pnode1);

    EmitGetIterator(iteratorLocation, yieldStarNode->sxUni.pnode1->location, byteCodeGenerator, funcInfo);

    uint loopId = byteCodeGenerator->Writer()->EnterLoop(loopEntrance);
    // since a yield* doesn't have a user defined body, we cannot return from this loop
    // which means we don't need to support EmitJumpCleanup() and there do not need to
    // remember the loopId like the loop statements do.

    // Get a temporary to hold the input for the next() calls.  Initialize it to undefined for the first call.
    Js::RegSlot nextInputLocation = funcInfo->AcquireTmpRegister();
    byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, nextInputLocation, funcInfo->undefinedConstantRegister);

    // Call the iterator's next()
    EmitIteratorNext(yieldStarNode->location, iteratorLocation, nextInputLocation, byteCodeGenerator, funcInfo);

    Js::RegSlot doneLocation = funcInfo->AcquireTmpRegister();
    EmitIteratorComplete(doneLocation, yieldStarNode->location, byteCodeGenerator, funcInfo);

    // Put the iterator result's value in yieldStarNode->location regardless of the done property value.  The value will
    // be used as the result value of the yield* operator expression in the case when done is true.
    EmitIteratorValue(yieldStarNode->location, yieldStarNode->location, byteCodeGenerator, funcInfo);

    // branch past the loop if the done property is truthy
    byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrTrue_A, continuePastLoop, doneLocation);
    funcInfo->ReleaseTmpRegister(doneLocation);

    EmitYield(yieldStarNode->location, nextInputLocation, byteCodeGenerator, funcInfo, iteratorLocation);

    funcInfo->ReleaseTmpRegister(nextInputLocation);
    funcInfo->ReleaseTmpRegister(iteratorLocation);

    byteCodeGenerator->Writer()->Br(loopEntrance);
    byteCodeGenerator->Writer()->MarkLabel(continuePastLoop);
    byteCodeGenerator->Writer()->ExitLoop(loopId);
}

void TrackIntConstantsOnGlobalUserObject(ByteCodeGenerator *byteCodeGenerator, bool isSymGlobalAndSingleAssignment, Js::PropertyId propertyId)
{
    if (isSymGlobalAndSingleAssignment)
    {
        byteCodeGenerator->GetScriptContext()->TrackIntConstPropertyOnGlobalUserObject(propertyId);
    }
}

void TrackIntConstantsOnGlobalObject(ByteCodeGenerator *byteCodeGenerator, bool isSymGlobalAndSingleAssignment, Js::PropertyId propertyId)
{
    if (isSymGlobalAndSingleAssignment)
    {
        byteCodeGenerator->GetScriptContext()->TrackIntConstPropertyOnGlobalObject(propertyId);
    }
}

void TrackIntConstantsOnGlobalObject(ByteCodeGenerator *byteCodeGenerator, Symbol *sym)
{
    if (sym && sym->GetIsGlobal() && sym->IsAssignedOnce())
    {
        Js::PropertyId propertyId = sym->EnsurePosition(byteCodeGenerator);
        byteCodeGenerator->GetScriptContext()->TrackIntConstPropertyOnGlobalObject(propertyId);
    }
}

void TrackMemberNodesInObjectForIntConstants(ByteCodeGenerator *byteCodeGenerator, ParseNodePtr objNode)
{
    Assert(objNode->nop == knopObject);
    Assert(ParseNode::Grfnop(objNode->nop) & fnopUni);

    ParseNodePtr memberList = objNode->sxUni.pnode1;

    if (memberList != nullptr)
    {
        //Iterate through all the member nodes
        while (memberList->nop == knopList)
        {
            ParseNodePtr memberNode = memberList->sxBin.pnode1;
            ParseNodePtr memberNameNode = memberNode->sxBin.pnode1;
            ParseNodePtr memberValNode = memberNode->sxBin.pnode2;

            if (memberNameNode->nop != knopComputedName && memberValNode->nop == knopInt)
            {
                Js::PropertyId propertyId = memberNameNode->sxPid.PropertyIdFromNameNode();
                TrackIntConstantsOnGlobalUserObject(byteCodeGenerator, true, propertyId);
            }
            memberList = memberList->sxBin.pnode2;
        }

        ParseNode *memberNameNode = memberList->sxBin.pnode1;
        ParseNode *memberValNode = memberList->sxBin.pnode2;
        
        if (memberNameNode->nop != knopComputedName && memberValNode->nop == knopInt)
        {
            Js::PropertyId propertyId = memberNameNode->sxPid.PropertyIdFromNameNode();
            TrackIntConstantsOnGlobalUserObject(byteCodeGenerator, true, propertyId);
        }
    }
}

void TrackGlobalIntAssignmentsForknopDotProps(ParseNodePtr knopDotNode, ByteCodeGenerator * byteCodeGenerator)
{
    Assert(knopDotNode->nop == knopDot);

    ParseNodePtr objectNode = knopDotNode->sxBin.pnode1;
    ParseNodePtr propertyNode = knopDotNode->sxBin.pnode2;
    bool isSymGlobalAndSingleAssignment = false;

    if (objectNode->nop == knopName)
    {
        Symbol * sym = objectNode->sxVar.sym;
        isSymGlobalAndSingleAssignment = sym && sym->GetIsGlobal() && sym->IsAssignedOnce() && propertyNode->sxPid.pid->IsSingleAssignment();
        Js::PropertyId propertyId = propertyNode->sxPid.PropertyIdFromNameNode();
        TrackIntConstantsOnGlobalUserObject(byteCodeGenerator, isSymGlobalAndSingleAssignment, propertyId);
    }
    else if (objectNode->nop == knopThis)
    {
        //Assume knopThis always refer to GlobalObject
        // Cases like "this.a = "
        isSymGlobalAndSingleAssignment = propertyNode->sxPid.pid->IsSingleAssignment();
        Js::PropertyId propertyId = propertyNode->sxPid.PropertyIdFromNameNode();
        TrackIntConstantsOnGlobalObject(byteCodeGenerator, isSymGlobalAndSingleAssignment, propertyId);
    }
}

/*
* Track the Global Int Constant properties' assignments here
*/

void TrackGlobalIntAssignments(ParseNodePtr pnode, ByteCodeGenerator * byteCodeGenerator)
{
    //Track the Global Int Constant properties' assignments here
    uint nodeType = ParseNode::Grfnop(pnode->nop);
    if (nodeType & fnopAsg)
    {
        if (nodeType & fnopBin)
        {
            ParseNodePtr lhs = pnode->sxBin.pnode1;
            ParseNodePtr rhs = pnode->sxBin.pnode2;

            Assert(lhs && rhs);

            //Don't track other than integers and objects with member nodes.
            if (rhs->nop == knopObject && (ParseNode::Grfnop(rhs->nop) & fnopUni))
            {
                TrackMemberNodesInObjectForIntConstants(byteCodeGenerator, rhs);
            }
            else if (rhs->nop != knopInt && 
                ((rhs->nop != knopLsh && rhs->nop != knopRsh) || (rhs->sxBin.pnode1->nop != knopInt || rhs->sxBin.pnode2->nop != knopInt)))
            {
                return;
            }

            if (lhs->nop == knopName)
            {
                //Handle "a = <Integer>" cases here
                Symbol * sym = lhs->sxVar.sym;
                TrackIntConstantsOnGlobalObject(byteCodeGenerator, sym);
            }
            else if (lhs->nop == knopDot && lhs->sxBin.pnode2->nop == knopName)
            {
                //Cases like "obj.a = <Integer>"
                TrackGlobalIntAssignmentsForknopDotProps(lhs, byteCodeGenerator);
            }
        }
        else if (nodeType & fnopUni)
        {
            ParseNodePtr lhs = pnode->sxUni.pnode1;

            if (lhs->nop == knopName)
            {
                //Cases like "a++"
                Symbol * sym = lhs->sxVar.sym;
                TrackIntConstantsOnGlobalObject(byteCodeGenerator, sym);
            }
            else if (lhs->nop == knopDot && lhs->sxBin.pnode2->nop == knopName)
            {
                //Cases like "obj.a++"
                TrackGlobalIntAssignmentsForknopDotProps(lhs, byteCodeGenerator);
            }
        }
    }
}

void Emit(ParseNode *pnode, ByteCodeGenerator *byteCodeGenerator, FuncInfo *funcInfo, BOOL fReturnValue, bool isConstructorCall)
{
    if (pnode==NULL)
        return;

    if (EmitUseBeforeDeclaration(pnode, byteCodeGenerator, funcInfo))
    {
        if (fReturnValue && IsExpressionStatement(pnode, byteCodeGenerator->GetScriptContext()))
        {
            // If this statement may produce the global function's return value, make sure we load something to the
            // return register for the JIT.

            // fReturnValue implies global function, which implies that "return" is a parse error.
            Assert(funcInfo->IsGlobalFunction());
            Assert(pnode->nop != knopReturn);
            byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdUndef, ByteCodeGenerator::ReturnRegister);
        }
        return;
    }
    ThreadContext::ProbeCurrentStackNoDispose(Js::Constants::MinStackByteCodeVisitor, byteCodeGenerator->GetScriptContext());

    TrackGlobalIntAssignments(pnode, byteCodeGenerator);

    //    printNop(pnode->nop);
    switch (pnode->nop) {
    case knopList:
        EmitList(pnode,byteCodeGenerator,funcInfo);
        break;
    case knopInt:
        // currently, these are loaded at the top
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopFlt        , "flt const"    ,None    ,Flt  ,fnopLeaf|fnopConst)
    case knopFlt:
        // currently, these are loaded at the top
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopStr        , "str const"    ,None    ,Pid  ,fnopLeaf|fnopConst)
    case knopStr:
        // TODO: protocol for combining string constants
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopRegExp     , "reg expr"    ,None    ,Pid  ,fnopLeaf|fnopConst)
    case knopRegExp:
        funcInfo->GetParsedFunctionBody()->SetLiteralRegex(pnode->sxPid.regexPatternIndex, pnode->sxPid.regexPattern);
        byteCodeGenerator->Writer()->Reg1Unsigned1(Js::OpCode::NewRegEx, funcInfo->AcquireLoc(pnode), pnode->sxPid.regexPatternIndex);
        RECORD_LOAD(pnode);
        break;          //PTNODE(knopThis       , "this"        ,None    ,None ,fnopLeaf)
    case knopThis:
        // enregistered
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopSuper      , "super"       ,None    , None        , fnopLeaf)
    case knopSuper:
        if (!funcInfo->IsClassMember())
        {
            FuncInfo* nonLambdaFunc = funcInfo;
            if (funcInfo->IsLambda())
            {
                nonLambdaFunc = byteCodeGenerator->FindEnclosingNonLambda();
            }

            if (nonLambdaFunc->IsGlobalFunction())
            {
                if ((byteCodeGenerator->GetFlags() & fscrEval))
                {
                    byteCodeGenerator->Writer()->Reg1(Js::OpCode::ScopedLdSuper, funcInfo->AcquireLoc(pnode));
                }
                else
                {
                    byteCodeGenerator->Writer()->W1(Js::OpCode::RuntimeReferenceError, SCODE_CODE(JSERR_BadSuperReference));
                }
            }
            else
            {
                Js::ByteCodeLabel errLabel = byteCodeGenerator->Writer()->DefineLabel();
                Js::ByteCodeLabel skipLabel = byteCodeGenerator->Writer()->DefineLabel();
                byteCodeGenerator->Writer()->BrReg2(Js::OpCode::BrEq_A, errLabel, funcInfo->superRegister, funcInfo->undefinedConstantRegister);
                byteCodeGenerator->Writer()->Br(Js::OpCode::Br, skipLabel);
                byteCodeGenerator->Writer()->MarkLabel(errLabel);
                byteCodeGenerator->Writer()->W1(Js::OpCode::RuntimeReferenceError, SCODE_CODE(JSERR_BadSuperReference));
                byteCodeGenerator->Writer()->MarkLabel(skipLabel);
            }
        }
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopNull       , "null"        ,Null    ,None ,fnopLeaf)
    case knopNull:
        // enregistered
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopFalse      , "false"        ,False   ,None ,fnopLeaf)
    case knopFalse:      
        // enregistered
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopTrue       , "true"        ,True    ,None ,fnopLeaf)
    case knopTrue:
        // enregistered
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopEmpty      , "empty"        ,Empty   ,None ,fnopLeaf)
    case knopEmpty:
        break;
        // Unary operators.
        //PTNODE(knopNot        , "~"            ,BitNot  ,Uni  ,fnopUni)
    case knopNot:
        Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
        funcInfo->ReleaseLoc(pnode->sxUni.pnode1);
        byteCodeGenerator->Writer()->Reg2(
            Js::OpCode::Not_A, funcInfo->AcquireLoc(pnode), pnode->sxUni.pnode1->location);
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopNeg        , "unary -"    ,Neg     ,Uni  ,fnopUni)
    case knopNeg:
        Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
        funcInfo->ReleaseLoc(pnode->sxUni.pnode1);
        funcInfo->AcquireLoc(pnode);
        byteCodeGenerator->Writer()->Reg2(
            Js::OpCode::Neg_A, pnode->location, pnode->sxUni.pnode1->location);
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopPos        , "unary +"    ,Pos     ,Uni  ,fnopUni)
    case knopPos:
        Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
        funcInfo->ReleaseLoc(pnode->sxUni.pnode1);
        byteCodeGenerator->Writer()->Reg2(
            Js::OpCode::Conv_Num, funcInfo->AcquireLoc(pnode), pnode->sxUni.pnode1->location);
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopLogNot     , "!"            ,LogNot  ,Uni  ,fnopUni)
    case knopLogNot:
        {
            Js::ByteCodeLabel doneLabel = byteCodeGenerator->Writer()->DefineLabel();
            // For boolean expressions that compute a result, we have to burn a register for the result
            // so that the back end can identify it cheaply as a single temp lifetime. Revisit this if we do
            // full-on renaming in the back end.
            funcInfo->AcquireLoc(pnode);
            if (pnode->sxUni.pnode1->nop == knopInt)
            {
                long value = pnode->sxUni.pnode1->sxInt.lw;
                Js::OpCode op = value ? Js::OpCode::LdFalse : Js::OpCode::LdTrue;
                byteCodeGenerator->Writer()->Reg1(op, pnode->location);
            }
            else
            {
                Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
                byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdFalse, pnode->location);
                byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrTrue_A, doneLabel, pnode->sxUni.pnode1->location);
                byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdTrue, pnode->location);
                byteCodeGenerator->Writer()->MarkLabel(doneLabel);
            }
            funcInfo->ReleaseLoc(pnode->sxUni.pnode1);
            RECORD_LOAD(pnode);
            break;
        }
        //PTNODE(knopEllipsis     , "..."       ,Spread  ,Uni         , fnopUni)
    case knopEllipsis:
        {
            Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
            // Transparently pass the location of the array
            pnode->location = pnode->sxUni.pnode1->location;
            break;
        }
        //PTNODE(knopIncPost    , "post++"    ,Inc     ,Uni  ,fnopUni|fnopAsg)
    case knopIncPost:
    case knopDecPost:
        // FALL THROUGH to the faster pre-inc/dec case if the result of the expression is not needed.
        if (pnode->isUsed || fReturnValue)
        {
            byteCodeGenerator->StartStatement(pnode);
            Js::OpCode op=Js::OpCode::Add_A;
            if (pnode->nop==knopDecPost)
            {
                op=Js::OpCode::Sub_A;
            }
            // Grab a register for the expression result.
            funcInfo->AcquireLoc(pnode);

            // Load the initial value, convert it (this is the expression result), and increment it.
            EmitLoad(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo);
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Conv_Num, pnode->location, pnode->sxUni.pnode1->location);

            Js::RegSlot incDecResult = pnode->sxUni.pnode1->location;
            if (funcInfo->RegIsConst(incDecResult))
            {
                // Avoid letting the add/sub overwrite a constant reg, as this may actually change the
                // contents of the constant table.
                incDecResult = funcInfo->AcquireTmpRegister();
            }

            byteCodeGenerator->Writer()->Reg3(op, incDecResult, pnode->location, funcInfo->constantToRegister.Lookup(1));
            // Store the incremented value.
            EmitAssignment(NULL, pnode->sxUni.pnode1, incDecResult, byteCodeGenerator, funcInfo);

            // Release the incremented value and the l-value.
            if (incDecResult != pnode->sxUni.pnode1->location)
            {
                funcInfo->ReleaseTmpRegister(incDecResult);
            }
            funcInfo->ReleaseLoad(pnode->sxUni.pnode1);
            byteCodeGenerator->EndStatement(pnode);
            RECORD_LOAD(pnode);
            break;
        }
        else
        {
            pnode->nop = (pnode->nop == knopIncPost) ? knopIncPre : knopDecPre;
        }
        // FALL THROUGH to the fast pre-inc/dec case if the result of the expression is not needed.

        //PTNODE(knopIncPre     , "++ pre"    ,Inc     ,Uni  ,fnopUni|fnopAsg)
    case knopIncPre:
    case knopDecPre: {
        byteCodeGenerator->StartStatement(pnode);
        Js::OpCode op=Js::OpCode::Incr_A;
        if (pnode->nop==knopDecPre)
        {
            op=Js::OpCode::Decr_A;
        }

        // Assign a register for the result only if the result is used or the operand can't be assigned to
        // (i.e., is a constant).
        if (pnode->isUsed || fReturnValue)
        {
            funcInfo->AcquireLoc(pnode);

            // Load the initial value and increment it (this is the expression result).
            EmitLoad(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo);
            byteCodeGenerator->Writer()->Reg2(op, pnode->location, pnode->sxUni.pnode1->location);

            // Store the incremented value and release the l-value.
            EmitAssignment(NULL, pnode->sxUni.pnode1, pnode->location, byteCodeGenerator, funcInfo);
            funcInfo->ReleaseLoad(pnode->sxUni.pnode1);
        }
        else
        {
            // Load the initial value and increment it (this is the expression result).
            EmitLoad(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo);

            Js::RegSlot incDecResult = pnode->sxUni.pnode1->location;
            if (funcInfo->RegIsConst(incDecResult))
            {
                // Avoid letting the add/sub overwrite a constant reg, as this may actually change the
                // contents of the constant table.
                incDecResult = funcInfo->AcquireTmpRegister();
            }

            byteCodeGenerator->Writer()->Reg2(op, incDecResult, pnode->sxUni.pnode1->location);

            // Store the incremented value and release the l-value.
            EmitAssignment(NULL, pnode->sxUni.pnode1, incDecResult, byteCodeGenerator, funcInfo);
            if (incDecResult != pnode->sxUni.pnode1->location)
            {
                funcInfo->ReleaseTmpRegister(incDecResult);
            }
            funcInfo->ReleaseLoad(pnode->sxUni.pnode1);
        }

        byteCodeGenerator->EndStatement(pnode);
        RECORD_LOAD(pnode);
        break;
                     }
                     //PTNODE(knopTypeof     , "typeof"    ,None    ,Uni  ,fnopUni)
    case knopTypeof:
        {
            ParseNode* pnodeOpnd = pnode->sxUni.pnode1;
            switch (pnodeOpnd->nop)
            {
            case knopDot:
                {
                    Emit(pnodeOpnd->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
                    Js::PropertyId propertyId = pnodeOpnd->sxBin.pnode2->sxPid.PropertyIdFromNameNode();
                    Assert(pnodeOpnd->sxBin.pnode2->nop == knopName);
                    funcInfo->ReleaseLoc(pnodeOpnd->sxBin.pnode1);
                    funcInfo->AcquireLoc(pnode);

                    byteCodeGenerator->EmitTypeOfFld(funcInfo, propertyId, pnode->location, pnodeOpnd->sxBin.pnode1->location, Js::OpCode::LdFldForTypeOf);
                    break;
                }

            case knopIndex:
                {
                    EmitBinaryOpnds(pnodeOpnd->sxBin.pnode1, pnodeOpnd->sxBin.pnode2, byteCodeGenerator, funcInfo);
                    funcInfo->ReleaseLoc(pnodeOpnd->sxBin.pnode2);
                    funcInfo->ReleaseLoc(pnodeOpnd->sxBin.pnode1);
                    funcInfo->AcquireLoc(pnode);
                    byteCodeGenerator->Writer()->Element(Js::OpCode::TypeofElem, pnode->location, pnodeOpnd->sxBin.pnode1->location, pnodeOpnd->sxBin.pnode2->location);
                    break;
                }
            case knopName:
                {
                    funcInfo->AcquireLoc(pnode);
                    byteCodeGenerator->EmitPropTypeof(pnode->location, pnodeOpnd->sxPid.sym, pnodeOpnd->sxPid.pid, funcInfo);
                    break;
                }

            default:
                Emit(pnodeOpnd, byteCodeGenerator, funcInfo, false);
                funcInfo->ReleaseLoc(pnodeOpnd);
                byteCodeGenerator->Writer()->Reg2(
                    Js::OpCode::Typeof, funcInfo->AcquireLoc(pnode), pnodeOpnd->location);
                break;
            }
            RECORD_LOAD(pnode);
            break;
        }
        //PTNODE(knopVoid       , "void"        ,Void    ,Uni  ,fnopUni)
    case knopVoid:
        Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
        funcInfo->ReleaseLoc(pnode->sxUni.pnode1);
        byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdUndef, funcInfo->AcquireLoc(pnode));
        break;
        //PTNODE(knopArray      , "arr cnst"    ,None    ,Uni  ,fnopUni)
    case knopArray:
        EmitArrayLiteral(pnode,byteCodeGenerator,funcInfo);
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopObject     , "obj cnst"    ,None    ,Uni  ,fnopUni)
    case knopObject: {
        funcInfo->AcquireLoc(pnode);
        EmitObjectInitializers(pnode->sxUni.pnode1,pnode->location,byteCodeGenerator,funcInfo);
        RECORD_LOAD(pnode);
        break;
        //PTNODE(knopComputedName, "[name]"      ,None    ,Uni  ,fnopUni)
    case knopComputedName:
        {
            Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
            // Transparently pass the name expr
            pnode->location = pnode->sxUni.pnode1->location;
            break;
        }
                     }
                     // Binary and Ternary Operators
    case knopAdd:
        EmitAdd(pnode, byteCodeGenerator, funcInfo);
        break;
    case knopSub:
    case knopMul:
    case knopDiv:
    case knopMod:
    case knopOr:
    case knopXor:
    case knopAnd:
    case knopLsh:
    case knopRsh:
    case knopRs2:
    case knopIn:
        EmitBinary(nopToOp[pnode->nop], pnode, byteCodeGenerator, funcInfo);
        break;
    case knopInstOf:
         {
            byteCodeGenerator->StartStatement(pnode);
            EmitBinaryOpnds(pnode->sxBin.pnode1, pnode->sxBin.pnode2, byteCodeGenerator, funcInfo);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode1);
            funcInfo->AcquireLoc(pnode);
            uint cacheId = funcInfo->NewIsInstInlineCache();
            byteCodeGenerator->Writer()->Reg3C(nopToOp[pnode->nop], pnode->location, pnode->sxBin.pnode1->location,
                pnode->sxBin.pnode2->location, cacheId);
            byteCodeGenerator->EndStatement(pnode);
            RECORD_LOAD(pnode);
        }
        break;
    case knopEq:
    case knopEqv:
    case knopNEqv:
    case knopNe:
    case knopLt:
    case knopLe:
    case knopGe:
    case knopGt:
        byteCodeGenerator->StartStatement(pnode);
        EmitBinaryOpnds(pnode->sxBin.pnode1, pnode->sxBin.pnode2, byteCodeGenerator, funcInfo);
        funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
        funcInfo->ReleaseLoc(pnode->sxBin.pnode1);
        funcInfo->AcquireLoc(pnode);
        byteCodeGenerator->Writer()->Reg3(nopToCMOp[pnode->nop],pnode->location,pnode->sxBin.pnode1->location,
            pnode->sxBin.pnode2->location);
        byteCodeGenerator->EndStatement(pnode);
        RECORD_LOAD(pnode);
        break;
    case knopNew:
        {
            unsigned int argCount = pnode->sxCall.argCount;
            // For "this"
            argCount++;

            BOOL fSideEffectArgs = FALSE;
            unsigned int tmpCount = CountArguments(pnode->sxCall.pnodeArgs, &fSideEffectArgs);
            Assert(argCount == tmpCount);

            if (argCount != (Js::ArgSlot)argCount)
            {
                Js::Throw::OutOfMemory();
            }

            byteCodeGenerator->StartStatement(pnode);

            // Start call, allocate out param space
            funcInfo->StartRecordingOutArgs(argCount);

            // Assign the call target operand(s), putting them into expression temps if necessary to protect
            // them from side-effects.
            if (fSideEffectArgs)
            {
                SaveOpndValue(pnode->sxCall.pnodeTarget, funcInfo);
            }

            if (pnode->sxCall.pnodeTarget->nop == knopSuper)
            {
                EmitSuperFieldPatch(funcInfo, pnode, byteCodeGenerator);
            }

            Emit(pnode->sxCall.pnodeTarget, byteCodeGenerator, funcInfo, false, true);

            if (pnode->sxCall.pnodeArgs == NULL)
            {
                funcInfo->ReleaseLoc(pnode->sxCall.pnodeTarget);
                Js::OpCode op = (CreateNativeArrays(byteCodeGenerator, funcInfo)
                                 && CallTargetIsArray(pnode->sxCall.pnodeTarget))
                                    ? Js::OpCode::NewScObjArray : Js::OpCode::NewScObject;
                Assert(argCount == 1);
				
                Js::ProfileId callSiteId = byteCodeGenerator->GetNextCallSiteId(op);
                byteCodeGenerator->Writer()->StartCall(Js::OpCode::StartCall, argCount);
                byteCodeGenerator->Writer()->CallI(op, funcInfo->AcquireLoc(pnode),
                    pnode->sxCall.pnodeTarget->location, argCount, callSiteId);
            }
            else
            {
                byteCodeGenerator->Writer()->StartCall(Js::OpCode::StartCall, argCount);
                uint32 actualArgCount = 0;

                if (IsCallOfConstants(pnode))
                {
                    funcInfo->ReleaseLoc(pnode->sxCall.pnodeTarget);
                    actualArgCount = EmitNewObjectOfConstants(pnode, byteCodeGenerator, funcInfo, argCount);
                }
                else
                {
                    Js::OpCode op;
                    if ((CreateNativeArrays(byteCodeGenerator, funcInfo) && CallTargetIsArray(pnode->sxCall.pnodeTarget)))
                    {
	                    op = pnode->sxCall.spreadArgCount > 0 ? Js::OpCode::NewScObjArraySpread : Js::OpCode::NewScObjArray;
                    }
                    else
                    {
	                    op = pnode->sxCall.spreadArgCount > 0 ? Js::OpCode::NewScObjectSpread : Js::OpCode::NewScObject;
                    }

                    Js::ProfileId callSiteId = byteCodeGenerator->GetNextCallSiteId(op);


                    Js::AuxArray<uint32> *spreadIndices = nullptr;
                    actualArgCount = EmitArgList(pnode->sxCall.pnodeArgs, Js::Constants::NoRegister, Js::Constants::NoRegister, false,
                                                    true, byteCodeGenerator, funcInfo, callSiteId, pnode->sxCall.spreadArgCount, &spreadIndices);
                    funcInfo->ReleaseLoc(pnode->sxCall.pnodeTarget);


                    if (pnode->sxCall.spreadArgCount > 0)
                    {
                        Assert(spreadIndices != nullptr);
                        size_t spreadExtraAlloc = spreadIndices->count * sizeof(uint32);
                        size_t spreadIndicesSize = sizeof(*spreadIndices) + spreadExtraAlloc;
                        byteCodeGenerator->Writer()->CallIExtended(op, funcInfo->AcquireLoc(pnode), pnode->sxCall.pnodeTarget->location,
                                                                   (uint16)actualArgCount, Js::CallIExtended_SpreadArgs,
                                                                   spreadIndices, spreadIndicesSize, callSiteId);
                    }
                    else
                    {
                        byteCodeGenerator->Writer()->CallI(op, funcInfo->AcquireLoc(pnode), pnode->sxCall.pnodeTarget->location,
                                                           (uint16)actualArgCount, callSiteId);
                    }
                }

                Assert(argCount == actualArgCount);
            }

            // End call, pop param space
            funcInfo->EndRecordingOutArgs(argCount);

            byteCodeGenerator->EndStatement(pnode);
            RECORD_LOAD(pnode);

            break;
        }
    case knopDelete:
        {
            ParseNode *pexpr=pnode->sxUni.pnode1;
            byteCodeGenerator->StartStatement(pnode);
            switch (pexpr->nop)
            {
            case knopName:
                {
                    funcInfo->AcquireLoc(pnode);
                    byteCodeGenerator->EmitPropDelete(pnode->location, pexpr->sxPid.sym, pexpr->sxPid.pid, funcInfo);
                    break;
                }
            case knopDot:
                {
                    Emit(pexpr->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
                    Js::PropertyId propertyId = pexpr->sxBin.pnode2->sxPid.PropertyIdFromNameNode();
                    funcInfo->ReleaseLoc(pexpr->sxBin.pnode1);
                    funcInfo->AcquireLoc(pnode);
                    byteCodeGenerator->Writer()->Property(Js::OpCode::DeleteFld, pnode->location,pexpr->sxBin.pnode1->location,
                        funcInfo->FindOrAddReferencedPropertyId(propertyId));
                    break;
                }
            case knopIndex:
                {
                    EmitBinaryOpnds(pexpr->sxBin.pnode1, pexpr->sxBin.pnode2, byteCodeGenerator, funcInfo);
                    funcInfo->ReleaseLoc(pexpr->sxBin.pnode2);
                    funcInfo->ReleaseLoc(pexpr->sxBin.pnode1);
                    funcInfo->AcquireLoc(pnode);
                    byteCodeGenerator->Writer()->Element(Js::OpCode::DeleteElemI_A,pnode->location,pexpr->sxBin.pnode1->location, pexpr->sxBin.pnode2->location);
                    break;
                }
            case knopThis:
                {
                    funcInfo->AcquireLoc(pnode);
                    byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdTrue, pnode->location);
                    break;
                }
            default:
                {
                    Emit(pexpr, byteCodeGenerator, funcInfo, false);
                    funcInfo->ReleaseLoc(pexpr);
                    byteCodeGenerator->Writer()->Reg2(
                        Js::OpCode::Delete_A, funcInfo->AcquireLoc(pnode), pexpr->location);
                    break;
                }
            }
            byteCodeGenerator->EndStatement(pnode);
            break;
        }
    case knopCall:
        byteCodeGenerator->StartStatement(pnode);
        if (pnode->sxCall.pnodeTarget->nop == knopSuper)
        {
            EmitSuperFieldPatch(funcInfo, pnode, byteCodeGenerator);
        }
        if (pnode->sxCall.isApplyCall && funcInfo->GetApplyEnclosesArgs())
        {
            EmitApplyCall(pnode, Js::Constants::NoRegister, byteCodeGenerator, funcInfo, fReturnValue, true);
        }
        else
        {
            EmitCall(pnode, Js::Constants::NoRegister, byteCodeGenerator, funcInfo, fReturnValue, true);
        }
        byteCodeGenerator->EndStatement(pnode);
        RECORD_LOAD(pnode);
        break;
    case knopIndex:
        byteCodeGenerator->StartStatement(pnode);
        EmitBinaryOpnds(pnode->sxBin.pnode1, pnode->sxBin.pnode2, byteCodeGenerator, funcInfo);
        funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
        funcInfo->ReleaseLoc(pnode->sxBin.pnode1);
        funcInfo->AcquireLoc(pnode);
        byteCodeGenerator->Writer()->Element(
            Js::OpCode::LdElemI_A,pnode->location,pnode->sxBin.pnode1->location, pnode->sxBin.pnode2->location);
        byteCodeGenerator->EndStatement(pnode);
        RECORD_LOAD(pnode);
        break;
         // this is MemberExpression as rvalue
    case knopDot:
    case knopScope:
        {
            Emit(pnode->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode1);
            funcInfo->AcquireLoc(pnode);
            Js::PropertyId propertyId = pnode->sxBin.pnode2->sxPid.PropertyIdFromNameNode();
            if (propertyId == Js::PropertyIds::length)
            {
                byteCodeGenerator->Writer()->Reg2(Js::OpCode::LdLen_A, pnode->location, pnode->sxBin.pnode1->location);
            }
            else
            {
                uint cacheId = funcInfo->FindOrAddInlineCacheId(pnode->sxBin.pnode1->location, propertyId, false, false);
                if(pnode->IsCallApplyTargetLoad())
                {
                    byteCodeGenerator->Writer()->PatchableProperty(Js::OpCode::LdFldForCallApplyTarget, pnode->location, pnode->sxBin.pnode1->location, cacheId);
                }
                else
                {
                    if (pnode->sxBin.pnode1->nop == knopSuper)
                    {
                        byteCodeGenerator->Writer()->PatchablePropertyWithThisPtr(Js::OpCode::LdSuperFld, pnode->location, pnode->sxBin.pnode1->location, funcInfo->thisPointerRegister, cacheId, isConstructorCall);
                    }
                    else
                    {
                        byteCodeGenerator->Writer()->PatchableProperty(Js::OpCode::LdFld, pnode->location, pnode->sxBin.pnode1->location, cacheId, isConstructorCall);
                    }
                }
            }
            RECORD_LOAD(pnode);
            break;
        }

        //PTNODE(knopAsg        , "="            ,None    ,Bin  ,fnopBin|fnopAsg)
    case knopAsg:
        {
            ParseNode *lhs=pnode->sxBin.pnode1;
            ParseNode *rhs=pnode->sxBin.pnode2;
            byteCodeGenerator->StartStatement(pnode);
            if (pnode->isUsed || fReturnValue)
            {
                // If the assignment result is used, grab a register to hold it and pass it to EmitAssignment,
                // which will copy the assigned value there.
                funcInfo->AcquireLoc(pnode);
                EmitBinaryReference(lhs, rhs, byteCodeGenerator, funcInfo, false);
                EmitAssignment(pnode, lhs, rhs->location, byteCodeGenerator, funcInfo);
            }
            else
            {
                EmitBinaryReference(lhs, rhs, byteCodeGenerator, funcInfo, false);
                EmitAssignment(NULL, lhs, rhs->location, byteCodeGenerator, funcInfo);
            }
            funcInfo->ReleaseLoc(rhs);
            if (!(byteCodeGenerator->GetScriptContext()->GetConfig()->IsES6DestructuringEnabled() && lhs->nop == knopArray))
            {
                funcInfo->ReleaseReference(lhs);
            }
            byteCodeGenerator->EndStatement(pnode);
            RECORD_LOAD(pnode);
            break;
        }

    case knopName:
        funcInfo->AcquireLoc(pnode);
        byteCodeGenerator->EmitPropLoad(pnode->location, pnode->sxPid.sym, pnode->sxPid.pid, funcInfo);
        RECORD_LOAD(pnode);
        break;

    case knopComma:
        {
            // The parser marks binary opnd pnodes as used, but value of the first opnd of a comma is not used.
            // Easier to correct this here than to check every binary op in the parser.
            ParseNode *pnode1 = pnode->sxBin.pnode1;
            pnode1->isUsed = false;
            if (pnode1->nop == knopComma)
            {
                // Spot fix for giant comma expressions that send us into OOS if we use a simple recursive
                // algorithm. Instead of recursing on comma LHS's, iterate over them, pushing the RHS's onto
                // a stack. (This suggests a model for removing recursion from Emit altogether...)
                ArenaAllocator *alloc = byteCodeGenerator->GetAllocator();
                SList<ParseNode*> rhsStack(alloc);
                do {
                    rhsStack.Push(pnode1->sxBin.pnode2);
                    pnode1 = pnode1->sxBin.pnode1;
                    pnode1->isUsed = false;
                } while (pnode1->nop == knopComma);
                Emit(pnode1, byteCodeGenerator, funcInfo, false);
                if (funcInfo->IsTmpReg(pnode1->location))
                {
                    byteCodeGenerator->Writer()->Reg1(Js::OpCode::Unused, pnode1->location);
                }
                while (!rhsStack.Empty())
                {
                    ParseNode *pnodeRhs = rhsStack.Pop();
                    pnodeRhs->isUsed = false;
                    Emit(pnodeRhs, byteCodeGenerator, funcInfo, false);
                    if (funcInfo->IsTmpReg(pnodeRhs->location))
                    {
                        byteCodeGenerator->Writer()->Reg1(Js::OpCode::Unused, pnodeRhs->location);
                    }
                    funcInfo->ReleaseLoc(pnodeRhs);
                }
            }
            else
            {
                Emit(pnode1, byteCodeGenerator, funcInfo, false);
                if (funcInfo->IsTmpReg(pnode1->location))
                {
                    byteCodeGenerator->Writer()->Reg1(Js::OpCode::Unused, pnode1->location);
                }
            }
            funcInfo->ReleaseLoc(pnode1);

            pnode->sxBin.pnode2->isUsed = pnode->isUsed || fReturnValue;
            Emit(pnode->sxBin.pnode2, byteCodeGenerator, funcInfo, false);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
            funcInfo->AcquireLoc(pnode);
            if (pnode->sxBin.pnode2->isUsed && pnode->location != pnode->sxBin.pnode2->location)
            {
                byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxBin.pnode2->location);
            }
            RECORD_LOAD(pnode);
        }
        break;

        // The binary logical ops && and || resolve to the value of the left-hand expression if its
        // boolean value short-circuits the operation, and to the value of the right-hand expression
        // otherwise. (In other words, the "truth" of the right-hand expression is never tested.)
        //PTNODE(knopLogOr      , "||"        ,None    ,Bin  ,fnopBin)
    case knopLogOr:
        {
            Js::ByteCodeLabel doneLabel = byteCodeGenerator->Writer()->DefineLabel();
            // For boolean expressions that compute a result, we have to burn a register for the result
            // so that the back end can identify it cheaply as a single temp lifetime. Revisit this if we do
            // full-on renaming in the back end.
            funcInfo->AcquireLoc(pnode);

            Emit(pnode->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxBin.pnode1->location);
            byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrTrue_A, doneLabel, pnode->sxBin.pnode1->location);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode1);

            Emit(pnode->sxBin.pnode2, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxBin.pnode2->location);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
            byteCodeGenerator->Writer()->MarkLabel(doneLabel);
            RECORD_LOAD(pnode);
            break;
        }
        //PTNODE(knopLogAnd     , "&&"        ,None    ,Bin  ,fnopBin)
    case knopLogAnd:
        {
            Js::ByteCodeLabel doneLabel = byteCodeGenerator->Writer()->DefineLabel();
            // For boolean expressions that compute a result, we have to burn a register for the result
            // so that the back end can identify it cheaply as a single temp lifetime. Revisit this if we do
            // full-on renaming in the back end.
            funcInfo->AcquireLoc(pnode);

            Emit(pnode->sxBin.pnode1, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxBin.pnode1->location);
            byteCodeGenerator->Writer()->BrReg1(Js::OpCode::BrFalse_A, doneLabel, pnode->sxBin.pnode1->location);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode1);

            Emit(pnode->sxBin.pnode2, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxBin.pnode2->location);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
            byteCodeGenerator->Writer()->MarkLabel(doneLabel);
            RECORD_LOAD(pnode);
            break;
        }
        //PTNODE(knopQmark      , "?"            ,None    ,Tri  ,fnopBin)
    case knopQmark:
        {
            Js::ByteCodeLabel trueLabel = byteCodeGenerator->Writer()->DefineLabel();
            Js::ByteCodeLabel falseLabel = byteCodeGenerator->Writer()->DefineLabel();
            Js::ByteCodeLabel skipLabel = byteCodeGenerator->Writer()->DefineLabel();
            EmitBooleanExpression(pnode->sxTri.pnode1, trueLabel, falseLabel, byteCodeGenerator, funcInfo);
            byteCodeGenerator->Writer()->MarkLabel(trueLabel);
            funcInfo->ReleaseLoc(pnode->sxTri.pnode1);

            // For boolean expressions that compute a result, we have to burn a register for the result
            // so that the back end can identify it cheaply as a single temp lifetime. Revisit this if we do
            // full-on renaming in the back end.
            funcInfo->AcquireLoc(pnode);

            Emit(pnode->sxTri.pnode2, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxTri.pnode2->location);
            funcInfo->ReleaseLoc(pnode->sxTri.pnode2);

            // Record the branch bytecode offset
            byteCodeGenerator->Writer()->RecordStatementAdjustment(Js::FunctionBody::SAT_FromCurrentToNext);

            byteCodeGenerator->Writer()->Br(skipLabel);

            byteCodeGenerator->Writer()->MarkLabel(falseLabel);
            Emit(pnode->sxTri.pnode3, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxTri.pnode3->location);
            funcInfo->ReleaseLoc(pnode->sxTri.pnode3);

            byteCodeGenerator->Writer()->MarkLabel(skipLabel);
            RECORD_LOAD(pnode);
            break;
        }
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
        byteCodeGenerator->StartStatement(pnode);
        // Assign a register for the result only if the result is used or the LHS can't be assigned to
        // (i.e., is a constant).
        if (pnode->isUsed || fReturnValue || funcInfo->RegIsConst(pnode->sxBin.pnode1->location))
        {
            // If the assign-op result is used, grab a register to hold it.
            funcInfo->AcquireLoc(pnode);

            // Grab a register for the initial value and load it.
            EmitBinaryReference(pnode->sxBin.pnode1, pnode->sxBin.pnode2, byteCodeGenerator, funcInfo, true);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
            // Do the arithmetic, store the result, and release the l-value.
            byteCodeGenerator->Writer()->Reg3(nopToOp[pnode->nop], pnode->location, pnode->sxBin.pnode1->location,
                pnode->sxBin.pnode2->location);

            EmitAssignment(pnode, pnode->sxBin.pnode1, pnode->location, byteCodeGenerator, funcInfo);
        }
        else
        {
            // Grab a register for the initial value and load it.
            EmitBinaryReference(pnode->sxBin.pnode1, pnode->sxBin.pnode2, byteCodeGenerator, funcInfo, true);
            funcInfo->ReleaseLoc(pnode->sxBin.pnode2);
            // Do the arithmetic, store the result, and release the l-value.
            byteCodeGenerator->Writer()->Reg3(nopToOp[pnode->nop], pnode->sxBin.pnode1->location, pnode->sxBin.pnode1->location,
                pnode->sxBin.pnode2->location);
            EmitAssignment(NULL, pnode->sxBin.pnode1, pnode->sxBin.pnode1->location, byteCodeGenerator, funcInfo);
        }
        funcInfo->ReleaseLoad(pnode->sxBin.pnode1);

        byteCodeGenerator->EndStatement(pnode);
        RECORD_LOAD(pnode);
        break;
        // General nodes.

        //PTNODE(knopTempRef      , "temp ref"  ,None   ,Uni ,fnopUni)
    case knopTempRef:
        // TODO: check whether mov is necessary
        funcInfo->AcquireLoc(pnode);
        byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxUni.pnode1->location);
        break;
        //PTNODE(knopTemp      , "temp"        ,None   ,None ,fnopLeaf)
    case knopTemp:
        // Emit initialization code
        if (pnode->sxVar.pnodeInit!=NULL)
        {
            byteCodeGenerator->StartStatement(pnode);
            Emit(pnode->sxVar.pnodeInit, byteCodeGenerator, funcInfo, false);
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, pnode->location, pnode->sxVar.pnodeInit->location);
            funcInfo->ReleaseLoc(pnode->sxVar.pnodeInit);
            byteCodeGenerator->EndStatement(pnode);
        }
        break;
        //PTNODE(knopVarDecl    , "varDcl"    ,None    ,Var  ,fnopNone)
    case knopVarDecl:
        {
            // Emit initialization code
            ParseNodePtr initNode = pnode->sxVar.pnodeInit;

            if (initNode != NULL)
            {
                byteCodeGenerator->StartStatement(pnode);
                Emit(pnode->sxVar.pnodeInit, byteCodeGenerator, funcInfo, false);
                EmitAssignment(NULL, pnode, pnode->sxVar.pnodeInit->location, byteCodeGenerator, funcInfo);
                funcInfo->ReleaseLoc(pnode->sxVar.pnodeInit);
                byteCodeGenerator->EndStatement(pnode);

                Symbol *sym = pnode->sxVar.sym;
                if (pnode->sxVar.pnodeInit->nop == knopObject && (ParseNode::Grfnop(pnode->sxVar.pnodeInit->nop) & fnopUni))
                {
                    TrackMemberNodesInObjectForIntConstants(byteCodeGenerator, pnode->sxVar.pnodeInit);
                }
                else if (initNode->nop == knopInt)
                {
                    TrackIntConstantsOnGlobalObject(byteCodeGenerator, sym);
                }
            }
            break;
        }
    case knopConstDecl:
        {
            ParseNodePtr initNode = pnode->sxVar.pnodeInit;
            AssertMsg(pnode->sxVar.pnodeInit, "Const must be initialized");
            byteCodeGenerator->StartStatement(pnode);

            Symbol * sym = pnode->sxVar.sym;
            Emit(pnode->sxVar.pnodeInit, byteCodeGenerator, funcInfo, false);
            // Set NeedDeclaration to false AFTER emitting the initializer, otherwise we won't have Use Before Declaration
            // errors reported in the initializer for code like this:
            // { const a = a; }
            sym->SetNeedDeclaration(false);
            byteCodeGenerator->EmitPropStore(pnode->sxVar.pnodeInit->location, sym, null, funcInfo, false, true);
            funcInfo->ReleaseLoc(pnode->sxVar.pnodeInit);

            byteCodeGenerator->EndStatement(pnode);

            if (initNode->nop == knopInt)
            {
                TrackIntConstantsOnGlobalObject(byteCodeGenerator, sym);
            }
        }
        break;
    case knopLetDecl:
        {
            ParseNodePtr initNode = pnode->sxVar.pnodeInit;
            byteCodeGenerator->StartStatement(pnode);

            Symbol * sym = pnode->sxVar.sym;
            Js::RegSlot rhsLocation;
            if (pnode->sxVar.pnodeInit != NULL)
            {
                Emit(pnode->sxVar.pnodeInit, byteCodeGenerator, funcInfo, false);
                rhsLocation = pnode->sxVar.pnodeInit->location;
                if (initNode->nop == knopInt)
                {
                    TrackIntConstantsOnGlobalObject(byteCodeGenerator, sym);
                }
            }
            else
            {
                rhsLocation = funcInfo->AcquireTmpRegister();
                byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdUndef, rhsLocation);
            }

            // Set NeedDeclaration to false AFTER emitting the initializer, otherwise we won't have Use Before Declaration
            // errors reported in the initializer for code like this:
            // { let a = a; }
            sym->SetNeedDeclaration(false);

            byteCodeGenerator->EmitPropStore(rhsLocation, sym, null, funcInfo, true, false);

            funcInfo->ReleaseTmpRegister(rhsLocation);
            byteCodeGenerator->EndStatement(pnode);

        }
        break;
    //PTNODE(knopFncDecl    , "fncDcl"    ,None    ,Fnc  ,fnopLeaf)
    case knopFncDecl:
        // The "function declarations" were emitted in DefineFunctions()
        if (!pnode->sxFnc.IsDeclaration())
        {
            //
            // - For "function expressions", in some scenarios (see comments in DefineFunctions()), they were emitted there also.
            //   That caters to following scenario
            //     - Only in IE8 compat mode.
            //     - In ES5 mode, "second" f doesn't affect the enclosing scope
            //
            //   function f() { write("first"); }
            //   f();       // ----------------------------> Should print "second" in IE8 mode
            //              // ----------------------------> Should print "first"  in ES5 mode
            //   var x = function f() { write("second"); }
            //
            // - But function expressions should be emitted on the fly as well, as each invocation results in a new function object
            //
            //   var a={}; for (i=0;i<2;i++) { g=function f(){...}; a[i] = f; }
            //   a[0] is different from a[1]
            //

            byteCodeGenerator->DefineOneFunctionHandleBoxedFD(pnode, funcInfo, false);
            RECORD_LOAD(pnode);
        }
        break;
        //PTNODE(knopEndCode    , "<endcode>"    ,None    ,None ,fnopNone)
    case knopClassDecl:
        {
            funcInfo->AcquireLoc(pnode);

            // Extends
            if (pnode->sxClass.pnodeExtends)
            {
                Emit(pnode->sxClass.pnodeExtends, byteCodeGenerator, funcInfo, false);
            }

            Assert(pnode->sxClass.pnodeConstructor);
            pnode->sxClass.pnodeConstructor->location = pnode->location;

            BeginEmitBlock(pnode->sxClass.pnodeBlock, byteCodeGenerator, funcInfo);

            // Constructor
            Emit(pnode->sxClass.pnodeConstructor, byteCodeGenerator, funcInfo, false);
           
            if (pnode->sxClass.pnodeExtends)
            {
                byteCodeGenerator->Writer()->InitClass(pnode->location, pnode->sxClass.pnodeExtends->location);
            }
            else
            {
                byteCodeGenerator->Writer()->InitClass(pnode->location);
            }

            // Static Methods
            EmitClassInitializers(pnode->sxClass.pnodeStaticMembers, pnode->location, byteCodeGenerator, funcInfo, pnode);

            // Instance Methods
            Js::RegSlot protoLoc = funcInfo->AcquireTmpRegister();
            int cacheId = funcInfo->FindOrAddInlineCacheId(pnode->location, Js::PropertyIds::prototype, false, false);
            byteCodeGenerator->Writer()->PatchableProperty(Js::OpCode::LdFld, protoLoc, pnode->location, cacheId);

            EmitClassInitializers(pnode->sxClass.pnodeMembers, protoLoc, byteCodeGenerator, funcInfo, pnode);

            funcInfo->ReleaseTmpRegister(protoLoc);

            // Emit name binding.
            if (pnode->sxClass.pnodeName)
            {
                Symbol * sym = pnode->sxClass.pnodeName->sxVar.sym;
                sym->SetNeedDeclaration(false);
                byteCodeGenerator->EmitPropStore(pnode->location, sym, null, funcInfo, false, true);
            }

            EndEmitBlock(pnode->sxClass.pnodeBlock, byteCodeGenerator, funcInfo);

            if (pnode->sxClass.pnodeExtends)
            {
                funcInfo->ReleaseLoc(pnode->sxClass.pnodeExtends);
            }

            if (pnode->sxClass.pnodeDeclName)
            {
                Symbol * sym = pnode->sxClass.pnodeDeclName->sxVar.sym;
                sym->SetNeedDeclaration(false);
                byteCodeGenerator->EmitPropStore(pnode->location, sym, null, funcInfo, true, false);
            }

            break;
        }
    case knopStrTemplate:
        EmitStringTemplate(pnode, byteCodeGenerator, funcInfo);
        break;
    case knopEndCode:
        byteCodeGenerator->Writer()->RecordStatementAdjustment(Js::FunctionBody::SAT_All);

        // load undefined for the fallthrough case:
        if (!funcInfo->IsGlobalFunction())
        {
            // In the global function, implicit return values are copied to the return register, and if
            // necessary the return register is initialized at the top. Don't clobber the value here.
            byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdUndef, ByteCodeGenerator::ReturnRegister);
        }

        // Label for non-fall through ret
        byteCodeGenerator->Writer()->MarkLabel(funcInfo->singleExit);

        if (funcInfo->GetHasCachedScope())
        {
            byteCodeGenerator->Writer()->Auxiliary(
                Js::OpCode::CommitScope,
                funcInfo->frameObjRegister,
                funcInfo->localPropIdOffset,
                sizeof(Js::PropertyIdArray) + ((funcInfo->GetBodyScope()->GetScopeSlotCount() + 3) * sizeof(Js::PropertyId)));
        }
        byteCodeGenerator->StartStatement(pnode);
        byteCodeGenerator->Writer()->Empty(Js::OpCode::Ret);
        byteCodeGenerator->EndStatement(pnode);
        break;
        //PTNODE(knopDebugger   , "debugger"    ,None    ,None ,fnopNone)
    case knopDebugger:
        byteCodeGenerator->StartStatement(pnode);
        byteCodeGenerator->Writer()->Empty(Js::OpCode::Break);
        byteCodeGenerator->EndStatement(pnode);
        break;
        //PTNODE(knopFor        , "for"        ,None    ,For  ,fnopBreak|fnopContinue)
    case knopFor:
        if (pnode->sxFor.pnodeInverted!=NULL) {
            byteCodeGenerator->EmitInvertedLoop(pnode,pnode->sxFor.pnodeInverted,funcInfo);
        }
        else {
            BeginEmitBlock(pnode->sxFor.pnodeBlock, byteCodeGenerator, funcInfo);
            Emit(pnode->sxFor.pnodeInit, byteCodeGenerator, funcInfo, false);
            funcInfo->ReleaseLoc(pnode->sxFor.pnodeInit);
            EmitLoop(pnode,
                pnode->sxFor.pnodeCond,
                pnode->sxFor.pnodeBody,
                pnode->sxFor.pnodeIncr,
                byteCodeGenerator,
                funcInfo,
                fReturnValue);
            EndEmitBlock(pnode->sxFor.pnodeBlock, byteCodeGenerator, funcInfo);
        }
        break;
        //PTNODE(knopIf         , "if"        ,None    ,If   ,fnopNone)
    case knopIf: {
        byteCodeGenerator->StartStatement(pnode);

        Js::ByteCodeLabel trueLabel=byteCodeGenerator->Writer()->DefineLabel();
        Js::ByteCodeLabel falseLabel=byteCodeGenerator->Writer()->DefineLabel();
        EmitBooleanExpression(pnode->sxIf.pnodeCond,trueLabel,falseLabel,byteCodeGenerator,funcInfo);
        funcInfo->ReleaseLoc(pnode->sxIf.pnodeCond);

        byteCodeGenerator->EndStatement(pnode);

        byteCodeGenerator->Writer()->MarkLabel(trueLabel);
        Emit(pnode->sxIf.pnodeTrue, byteCodeGenerator, funcInfo, fReturnValue);
        funcInfo->ReleaseLoc(pnode->sxIf.pnodeTrue);
        if (pnode->sxIf.pnodeFalse!=NULL) {
            // has else clause
            Js::ByteCodeLabel skipLabel=byteCodeGenerator->Writer()->DefineLabel();

            // Record the branch bytecode offset
            byteCodeGenerator->Writer()->RecordStatementAdjustment(Js::FunctionBody::SAT_FromCurrentToNext);

            // then clause skips else clause
            byteCodeGenerator->Writer()->Br(skipLabel);
            // generate code for else clause
            byteCodeGenerator->Writer()->MarkLabel(falseLabel);
            Emit(pnode->sxIf.pnodeFalse, byteCodeGenerator, funcInfo, fReturnValue);
            funcInfo->ReleaseLoc(pnode->sxIf.pnodeFalse);
            byteCodeGenerator->Writer()->MarkLabel(skipLabel);
        }
        else
        {
            byteCodeGenerator->Writer()->MarkLabel(falseLabel);
        }
        if (pnode->emitLabels)
        {
            byteCodeGenerator->Writer()->MarkLabel(pnode->sxStmt.breakLabel);
        }
        break;
                 }
    case knopWhile:
        EmitLoop(pnode,
            pnode->sxWhile.pnodeCond,
            pnode->sxWhile.pnodeBody,
            NULL,
            byteCodeGenerator,
            funcInfo,
            fReturnValue);
        break;
        //PTNODE(knopDoWhile    , "do-while"    ,None    ,While,fnopBreak|fnopContinue)
    case knopDoWhile:
        EmitLoop(pnode,
            pnode->sxWhile.pnodeCond,
            pnode->sxWhile.pnodeBody,
            NULL,
            byteCodeGenerator,
            funcInfo,
            fReturnValue,
            true);
        break;
        //PTNODE(knopForIn      , "for in"    ,None    ,ForIn,fnopBreak|fnopContinue|fnopCleanup)
    case knopForIn:
        EmitForInOrForOf(pnode, byteCodeGenerator, funcInfo, fReturnValue);
        break;
    case knopForOf:
        EmitForInOrForOf(pnode, byteCodeGenerator, funcInfo, fReturnValue);
        break;
        //PTNODE(knopReturn     , "return"    ,None    ,Uni  ,fnopNone)
    case knopReturn:
        byteCodeGenerator->StartStatement(pnode);
        if (pnode->sxReturn.pnodeExpr!=NULL)
        {
            if (pnode->sxReturn.pnodeExpr->location == Js::Constants::NoRegister)
            {
                // No need to burn a register for the return value. If we need a temp, use R0 directly.
                pnode->sxReturn.pnodeExpr->location = ByteCodeGenerator::ReturnRegister;
            }
            Emit(pnode->sxReturn.pnodeExpr, byteCodeGenerator, funcInfo, fReturnValue);
            if (pnode->sxReturn.pnodeExpr->location != ByteCodeGenerator::ReturnRegister)
            {
                byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, ByteCodeGenerator::ReturnRegister, pnode->sxReturn.pnodeExpr->location);
            }
            funcInfo->GetParsedFunctionBody()->SetHasNoExplicitReturnValue(false);
        }
        else
        {
            byteCodeGenerator->Writer()->Reg1(Js::OpCode::LdUndef, ByteCodeGenerator::ReturnRegister);
        }
        if (pnode->sxStmt.grfnop & fnopCleanup)
        {
            EmitJumpCleanup(pnode, null, byteCodeGenerator, funcInfo);
        }

        byteCodeGenerator->Writer()->Br(funcInfo->singleExit);
        byteCodeGenerator->EndStatement(pnode);
        break;
    case knopLabel:
        break;
        //PTNODE(knopBlock      , "{}"        ,None    ,Block,fnopNone)
    case knopBlock:
        if (pnode->sxBlock.pnodeStmt!=NULL)
        {
            EmitBlock(pnode, byteCodeGenerator, funcInfo, fReturnValue);
            if (pnode->emitLabels)
            {
                byteCodeGenerator->Writer()->MarkLabel(pnode->sxStmt.breakLabel);
            }
        }
        break;
        //PTNODE(knopWith       , "with"        ,None    ,With ,fnopCleanup)
    case knopWith:
    {
        Assert(pnode->sxWith.pnodeObj != NULL);
        byteCodeGenerator->StartStatement(pnode);
        // Copy the with object to a temp register (the location assigned to pnode) so that if the with object
        // is overwritten in the body, the lookups are not affected.
        funcInfo->AcquireLoc(pnode);
        Emit(pnode->sxWith.pnodeObj, byteCodeGenerator, funcInfo, false);

        Js::RegSlot regVal = (byteCodeGenerator->GetScriptContext()->GetConfig()->IsES6UnscopablesEnabled()) ? funcInfo->AcquireTmpRegister() : pnode->location;
        byteCodeGenerator->Writer()->Reg2(Js::OpCode::Conv_Obj, regVal, pnode->sxWith.pnodeObj->location);
        if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsES6UnscopablesEnabled())
        {
            byteCodeGenerator->Writer()->Reg2(Js::OpCode::NewWithObject, pnode->location, regVal);
        }
        byteCodeGenerator->EndStatement(pnode);

#ifdef PERF_HINT
        if (PHASE_TRACE1(Js::PerfHintPhase))
        {
            WritePerfHint(PerfHints::HasWithBlock, funcInfo->byteCodeFunction->GetFunctionBody(), byteCodeGenerator->Writer()->GetCurrentOffset() - 1);
        }
#endif
        if (pnode->sxWith.pnodeBody != NULL)
        {
            Scope *scope = pnode->sxWith.scope;
            scope->SetLocation(pnode->location);
            byteCodeGenerator->PushScope(scope);

            Js::DebuggerScope *debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnode, Js::DiagExtraScopesType::DiagWithScope, regVal);

            if (byteCodeGenerator->ShouldTrackDebuggerMetadata())
            {
                byteCodeGenerator->Writer()->AddPropertyToDebuggerScope(debuggerScope, regVal, Js::Constants::NoProperty, /*shouldConsumeRegister*/ true, Js::DebuggerScopePropertyFlags_WithObject);
            }

            Emit(pnode->sxWith.pnodeBody, byteCodeGenerator, funcInfo, fReturnValue);
            funcInfo->ReleaseLoc(pnode->sxWith.pnodeBody);
            byteCodeGenerator->PopScope();

            byteCodeGenerator->RecordEndScopeObject(pnode);
        }
        if (pnode->emitLabels)
        {
            byteCodeGenerator->Writer()->MarkLabel(pnode->sxStmt.breakLabel);
        }
        if (byteCodeGenerator->GetScriptContext()->GetConfig()->IsES6UnscopablesEnabled())
        {
            funcInfo->ReleaseTmpRegister(regVal);
        }
        funcInfo->ReleaseLoc(pnode->sxWith.pnodeObj);
        break;
    }
        //PTNODE(knopBreak      , "break"        ,None    ,Jump ,fnopNone)
    case knopBreak:
        Assert(pnode->sxJump.pnodeTarget->emitLabels);
        byteCodeGenerator->StartStatement(pnode);
        if (pnode->sxStmt.grfnop & fnopCleanup)
        {
            EmitJumpCleanup(pnode, pnode->sxJump.pnodeTarget, byteCodeGenerator, funcInfo);
        }
        byteCodeGenerator->Writer()->Br(pnode->sxJump.pnodeTarget->sxStmt.breakLabel);
        if (pnode->emitLabels)
        {
            byteCodeGenerator->Writer()->MarkLabel(pnode->sxStmt.breakLabel);
        }
        byteCodeGenerator->EndStatement(pnode);
        break;
    case knopContinue:
        Assert(pnode->sxJump.pnodeTarget->emitLabels);
        byteCodeGenerator->StartStatement(pnode);
        if (pnode->sxStmt.grfnop & fnopCleanup)
        {
            EmitJumpCleanup(pnode, pnode->sxJump.pnodeTarget, byteCodeGenerator, funcInfo);
        }
        byteCodeGenerator->Writer()->Br(pnode->sxJump.pnodeTarget->sxStmt.continueLabel);
        byteCodeGenerator->EndStatement(pnode);
        break;
        //PTNODE(knopContinue   , "continue"    ,None    ,Jump ,fnopNone)
    case knopSwitch:
        {
            BOOL fHasDefault = false;
            Assert(pnode->sxSwitch.pnodeVal != NULL);
            byteCodeGenerator->StartStatement(pnode);
            Emit(pnode->sxSwitch.pnodeVal, byteCodeGenerator, funcInfo, false);

            Js::RegSlot regVal = funcInfo->AcquireTmpRegister();

            byteCodeGenerator->Writer()->Reg2(Js::OpCode::BeginSwitch, regVal, pnode->sxSwitch.pnodeVal->location);

            BeginEmitBlock(pnode->sxSwitch.pnodeBlock, byteCodeGenerator, funcInfo);

            byteCodeGenerator->EndStatement(pnode);

            // TODO: if all cases are compile-time constants, emit a switch statement in the byte
            // code so the BE can optimize it.

            ParseNode *pnodeCase;
            for (pnodeCase = pnode->sxSwitch.pnodeCases; pnodeCase; pnodeCase = pnodeCase->sxCase.pnodeNext)
            {
                // Jump to the first case body if this one doesn't match. Make sure any side-effects of the case
                // expression take place regardless.
                pnodeCase->sxCase.labelCase = byteCodeGenerator->Writer()->DefineLabel();
                if (pnodeCase == pnode->sxSwitch.pnodeDefault)
                {
                    fHasDefault = true;
                    continue;
                }
                Emit(pnodeCase->sxCase.pnodeExpr, byteCodeGenerator, funcInfo, false);
                byteCodeGenerator->Writer()->BrReg2(
                    Js::OpCode::Case, pnodeCase->sxCase.labelCase, regVal, pnodeCase->sxCase.pnodeExpr->location);
                funcInfo->ReleaseLoc(pnodeCase->sxCase.pnodeExpr);
            }

            // No explicit case value matches. Jump to the default arm (if any) or break out altogether.
            if (fHasDefault)
            {
                byteCodeGenerator->Writer()->Br(Js::OpCode::EndSwitch,pnode->sxSwitch.pnodeDefault->sxCase.labelCase);
            }
            else
            {
                if (!pnode->emitLabels)
                {
                    pnode->sxStmt.breakLabel = byteCodeGenerator->Writer()->DefineLabel();
                }
                byteCodeGenerator->Writer()->Br(Js::OpCode::EndSwitch,pnode->sxStmt.breakLabel);
            }
            // Now emit the case arms to which we jump on matching a case value.
            for (pnodeCase = pnode->sxSwitch.pnodeCases; pnodeCase; pnodeCase = pnodeCase->sxCase.pnodeNext)
            {
                byteCodeGenerator->Writer()->MarkLabel(pnodeCase->sxCase.labelCase);
                Emit(pnodeCase->sxCase.pnodeBody, byteCodeGenerator, funcInfo, fReturnValue);
                funcInfo->ReleaseLoc(pnodeCase->sxCase.pnodeBody);
            }

            EndEmitBlock(pnode->sxSwitch.pnodeBlock, byteCodeGenerator, funcInfo);
            funcInfo->ReleaseTmpRegister(regVal);
            funcInfo->ReleaseLoc(pnode->sxSwitch.pnodeVal);

            if (!fHasDefault || pnode->emitLabels)
            {
                byteCodeGenerator->Writer()->MarkLabel(pnode->sxStmt.breakLabel);
            }
            break;
        }

    case knopTryCatch:
        {
            Js::ByteCodeLabel catchLabel = (Js::ByteCodeLabel)-1;

            ParseNode *pnodeTry = pnode->sxTryCatch.pnodeTry;
            Assert(pnodeTry);
            ParseNode *pnodeCatch = pnode->sxTryCatch.pnodeCatch;
            Assert(pnodeCatch);

            catchLabel = byteCodeGenerator->Writer()->DefineLabel();

            // Note: try uses OpCode::Leave which causes a return to parent interpreter thunk,
            // same for catch block. Thus record cross interpreter frame entry/exit records for them.
            byteCodeGenerator->Writer()->RecordCrossFrameEntryExitRecord(/* isEnterBlock = */ true);

            byteCodeGenerator->Writer()->Br(Js::OpCode::TryCatch, catchLabel);

            ByteCodeGenerator::TryScopeRecord tryRecForTry(Js::OpCode::TryCatch, catchLabel);
            if (funcInfo->byteCodeFunction->IsGenerator())
            {
                byteCodeGenerator->tryScopeRecordsList.LinkToEnd(&tryRecForTry);
            }

            Emit(pnodeTry->sxTry.pnodeBody, byteCodeGenerator, funcInfo, fReturnValue);
            funcInfo->ReleaseLoc(pnodeTry->sxTry.pnodeBody);

            if (funcInfo->byteCodeFunction->IsGenerator())
            {
                byteCodeGenerator->tryScopeRecordsList.UnlinkFromEnd();
            }

            byteCodeGenerator->Writer()->RecordCrossFrameEntryExitRecord(/* isEnterBlock = */ false);

            byteCodeGenerator->Writer()->Empty(Js::OpCode::Leave);
            byteCodeGenerator->Writer()->Br(pnode->sxStmt.breakLabel);
            byteCodeGenerator->Writer()->MarkLabel(catchLabel);
            Assert(pnodeCatch->sxCatch.pnodeParam);
            ParseNode *pnodeObj = pnodeCatch->sxCatch.pnodeParam;
            Js::RegSlot location;
            Js::RegSlot scopeLocation = Js::Constants::NoRegister;

            bool fNeedsReg = false;

            Js::DiagExtraScopesType diagScopeType = Js::DiagCatchScopeInObject;

            Js::DebuggerScopePropertyFlags debuggerPropertyFlags = Js::DebuggerScopePropertyFlags_CatchObject;


            location = pnodeObj->sxPid.sym->GetLocation();
            if (location == Js::Constants::NoRegister)
            {
                location = funcInfo->AcquireLoc(pnodeObj);
                fNeedsReg = true;
            }
            byteCodeGenerator->Writer()->Reg1(Js::OpCode::Catch, location);

            Scope *scope = pnodeCatch->sxCatch.scope;
            Symbol *sym = pnodeObj->sxPid.sym;

            if (scope->GetMustInstantiate())
            {
                byteCodeGenerator->PushScope(scope);
                scopeLocation = funcInfo->AcquireTmpRegister();
                scope->SetLocation(scopeLocation);
                if (scope->GetIsObject())
                {
                    diagScopeType = Js::DiagCatchScopeInObject;
                    Js::DebuggerScope *debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnode, diagScopeType, scopeLocation);

                    if (byteCodeGenerator->ShouldTrackDebuggerMetadata())
                    {
                        byteCodeGenerator->Writer()->AddPropertyToDebuggerScope(debuggerScope, scopeLocation, sym->EnsurePosition(byteCodeGenerator), /*shouldConsumeRegister*/ true, debuggerPropertyFlags);
                    }

                    byteCodeGenerator->Writer()->Reg1(Js::OpCode::NewPseudoScope, scopeLocation);
                    Js::PropertyId propertyId = sym->EnsurePosition(byteCodeGenerator);
                    uint cacheId = funcInfo->FindOrAddInlineCacheId(scopeLocation, propertyId, false, true);
                    byteCodeGenerator->Writer()->PatchableProperty(Js::OpCode::InitFld, location, scopeLocation, cacheId);
                }
                else
                {
                    int index = Js::DebuggerScope::InvalidScopeIndex;

                    diagScopeType = Js::DiagCatchScopeInSlot;
                    Js::DebuggerScope *debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnode, diagScopeType, scopeLocation, &index);
                    byteCodeGenerator->TrackSlotArrayPropertyForDebugger(debuggerScope, sym, sym->EnsurePosition(byteCodeGenerator), debuggerPropertyFlags);

                    byteCodeGenerator->Writer()->Reg1Int2(Js::OpCode::NewScopeSlotsWithoutPropIds, scopeLocation, 1 + Js::ScopeSlots::FirstSlotIndex, index);
                    byteCodeGenerator->Writer()->Slot(Js::OpCode::StSlot, location, scopeLocation, Js::ScopeSlots::FirstSlotIndex);
                }
                sym->SetIsGlobalCatch(true);
            }
            else
            {
                diagScopeType = Js::DiagCatchScopeDirect;
                Js::DebuggerScope *debuggerScope = byteCodeGenerator->RecordStartScopeObject(pnode, diagScopeType, location);

                if (byteCodeGenerator->ShouldTrackDebuggerMetadata())
                {
                    byteCodeGenerator->Writer()->AddPropertyToDebuggerScope(debuggerScope, location, sym->EnsurePosition(byteCodeGenerator), /*shouldConsumeRegister*/ true, debuggerPropertyFlags);
                }

                byteCodeGenerator->EmitLocalPropInit(location, sym, funcInfo);
            }

            byteCodeGenerator->Writer()->RecordCrossFrameEntryExitRecord(true);

            // Allow a debugger to stop on the 'catch(e)'
            byteCodeGenerator->StartStatement(pnodeCatch);
            byteCodeGenerator->Writer()->Empty(Js::OpCode::Nop);
            byteCodeGenerator->EndStatement(pnodeCatch);

            ByteCodeGenerator::TryScopeRecord tryRecForCatch(Js::OpCode::ResumeCatch, catchLabel);
            if (funcInfo->byteCodeFunction->IsGenerator())
            {
                byteCodeGenerator->tryScopeRecordsList.LinkToEnd(&tryRecForCatch);
            }

            Emit(pnodeCatch->sxCatch.pnodeBody, byteCodeGenerator, funcInfo, fReturnValue);

            if (funcInfo->byteCodeFunction->IsGenerator())
            {
                byteCodeGenerator->tryScopeRecordsList.UnlinkFromEnd();
            }


            if (scope->GetMustInstantiate())
            {
                byteCodeGenerator->PopScope();
                funcInfo->ReleaseTmpRegister(scopeLocation);
            }

            byteCodeGenerator->RecordEndScopeObject(pnode);

            funcInfo->ReleaseLoc(pnodeCatch->sxCatch.pnodeBody);

            if (fNeedsReg)
            {
                funcInfo->ReleaseLoc(pnodeObj);
            }

            byteCodeGenerator->Writer()->RecordCrossFrameEntryExitRecord(false);

            byteCodeGenerator->Writer()->Empty(Js::OpCode::Leave);
            byteCodeGenerator->Writer()->MarkLabel(pnode->sxStmt.breakLabel);
            break;
        }

    case knopTryFinally:
        {
            Js::ByteCodeLabel finallyLabel = (Js::ByteCodeLabel)-1;

            ParseNode *pnodeTry = pnode->sxTryFinally.pnodeTry;
            Assert(pnodeTry);
            ParseNode *pnodeFinally = pnode->sxTryFinally.pnodeFinally;
            Assert(pnodeFinally);

            // If we yield from the finally block after an exception, we have to store the exception object for the future next call
            // When we yield from the Try-Finally the offset to the end of the Try block is needed for the branch instruction
            Js::RegSlot regException = Js::Constants::NoRegister;
            Js::RegSlot regOffset = Js::Constants::NoRegister;

            finallyLabel = byteCodeGenerator->Writer()->DefineLabel();
            byteCodeGenerator->Writer()->RecordCrossFrameEntryExitRecord(true);

            // [CONSIDER][aneeshd] Ideally the TryFinallyWithYield opcode needs to be used only if there is a yield expression
            // For now, if the function is generator we are using the TryFinallyWithYield.
            ByteCodeGenerator::TryScopeRecord tryRecForTry(Js::OpCode::TryFinallyWithYield, finallyLabel);
            if (funcInfo->byteCodeFunction->IsGenerator())
            {
                regException = funcInfo->AcquireTmpRegister();
                regOffset = funcInfo->AcquireTmpRegister();
                byteCodeGenerator->Writer()->BrReg2(Js::OpCode::TryFinallyWithYield, finallyLabel, regException, regOffset);
                tryRecForTry.reg1 = regException;
                tryRecForTry.reg2 = regOffset;
                byteCodeGenerator->tryScopeRecordsList.LinkToEnd(&tryRecForTry);
            }
            else
            {
                byteCodeGenerator->Writer()->Br(Js::OpCode::TryFinally, finallyLabel);
            }

            Emit(pnodeTry->sxTry.pnodeBody, byteCodeGenerator, funcInfo, fReturnValue);
            funcInfo->ReleaseLoc(pnodeTry->sxTry.pnodeBody);

            if (funcInfo->byteCodeFunction->IsGenerator())
            {
                byteCodeGenerator->tryScopeRecordsList.UnlinkFromEnd();
            }

            byteCodeGenerator->Writer()->Empty(Js::OpCode::Leave);
            byteCodeGenerator->Writer()->RecordCrossFrameEntryExitRecord(false);

            // Note: although we don't use OpCode::Leave for finally block,
            // OpCode::LeaveNull causes a return to parent interpreter thunk.
            // This has to be on offset prior to offset of 1st statement of finally.
            byteCodeGenerator->Writer()->RecordCrossFrameEntryExitRecord(true);

            byteCodeGenerator->Writer()->Br(pnode->sxStmt.breakLabel);
            byteCodeGenerator->Writer()->MarkLabel(finallyLabel);

            ByteCodeGenerator::TryScopeRecord tryRecForFinally(Js::OpCode::ResumeFinally, finallyLabel, regException, regOffset);
            if (funcInfo->byteCodeFunction->IsGenerator())
            {
                byteCodeGenerator->tryScopeRecordsList.LinkToEnd(&tryRecForFinally);
            }

            Emit(pnodeFinally->sxFinally.pnodeBody, byteCodeGenerator, funcInfo, fReturnValue);
            funcInfo->ReleaseLoc(pnodeFinally->sxFinally.pnodeBody);

            if (funcInfo->byteCodeFunction->IsGenerator())
            {
                byteCodeGenerator->tryScopeRecordsList.UnlinkFromEnd();
                funcInfo->ReleaseTmpRegister(regOffset);
                funcInfo->ReleaseTmpRegister(regException);
            }

            byteCodeGenerator->Writer()->RecordCrossFrameEntryExitRecord(false);

            byteCodeGenerator->Writer()->Empty(Js::OpCode::LeaveNull);

            byteCodeGenerator->Writer()->MarkLabel(pnode->sxStmt.breakLabel);
            break;
        }
    case knopThrow:
        byteCodeGenerator->StartStatement(pnode);
        Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
        byteCodeGenerator->Writer()->Reg1(Js::OpCode::Throw, pnode->sxUni.pnode1->location);
        funcInfo->ReleaseLoc(pnode->sxUni.pnode1);
        byteCodeGenerator->EndStatement(pnode);
        break;
    case knopYieldLeaf:
        byteCodeGenerator->StartStatement(pnode);
        funcInfo->AcquireLoc(pnode);
        EmitYield(funcInfo->undefinedConstantRegister, pnode->location, byteCodeGenerator, funcInfo);
        byteCodeGenerator->EndStatement(pnode);
        break;
    case knopYield:
        byteCodeGenerator->StartStatement(pnode);
        funcInfo->AcquireLoc(pnode);
        Emit(pnode->sxUni.pnode1, byteCodeGenerator, funcInfo, false);
        EmitYield(pnode->sxUni.pnode1->location, pnode->location, byteCodeGenerator, funcInfo);
        funcInfo->ReleaseLoc(pnode->sxUni.pnode1);
        byteCodeGenerator->EndStatement(pnode);
        break;
    case knopYieldStar:
        byteCodeGenerator->StartStatement(pnode);
        EmitYieldStar(pnode, byteCodeGenerator, funcInfo);
        byteCodeGenerator->EndStatement(pnode);
        break;
    default:
        AssertMsg(0, "emit unhandled pnode op");
        break;
    }

    if (fReturnValue && IsExpressionStatement(pnode, byteCodeGenerator->GetScriptContext()))
    {
        // If this statement may produce the global function's return value, copy its result to the return register.
        // fReturnValue implies global function, which implies that "return" is a parse error.
        Assert(funcInfo->IsGlobalFunction());
        Assert(pnode->nop != knopReturn);
        byteCodeGenerator->Writer()->Reg2(Js::OpCode::Ld_A, ByteCodeGenerator::ReturnRegister, pnode->location);
    }
}
