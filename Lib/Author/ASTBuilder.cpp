//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

// Use these lines to enable/disable running verification on every ASTBuilder object destructor
#define ASTBUILDER_DEBUG 0

namespace Authoring
{
    //
    //  ASTBuilderBase
    //
    ASTBuilderBase::ASTBuilderBase(Parser* parser, ParseNodePtr fncNode, ParseNodePtr blockNode)
        : _parser(parser), _blockNode(blockNode)
    {
        Assert(parser);
        SetFunction(fncNode);
    }

    ASTBuilderBase::~ASTBuilderBase()
    {
#if ASTBUILDER_DEBUG
        ValidateTree();
#endif
    }

    void ASTBuilderBase::SetFunction(ParseNodePtr fncNode)
    {
        Assert(fncNode);
        _fncNode = fncNode;
        _currentScope = _fncNode;
        SetExtent(Lim());
    }

     void ASTBuilderBase::SetScope(ParseNodePtr scope)
    {
        Assert(scope);
        _currentScope = scope;
    }

    void ASTBuilderBase::SetExtent(charcount_t min, charcount_t lim)
    {
        Assert(ContainsOffset(_fncNode, min));
        _min = min;
        if(lim)
        {
            Assert(ContainsOffset(_fncNode, lim));
            _lim = lim;
        }
        else
        {
            _lim = min;
        }
    }

    void ASTBuilderBase::SetExtentAs(ParseNodePtr node)
    {
        Assert(node);
        SetExtent(node->ichMin, node->ichLim);
    }

    charcount_t ASTBuilderBase::Min()
    {
        return _fncNode->sxFnc.pnodeBody && _fncNode->sxFnc.pnodeBody->nop != knopEndCode ? _fncNode->sxFnc.pnodeBody->ichMin : _fncNode->ichMin;
    }

    charcount_t ASTBuilderBase::Lim()
    {
        return _fncNode->sxFnc.pnodeBody && _fncNode->sxFnc.pnodeBody->nop != knopEndCode ? _fncNode->sxFnc.pnodeBody->ichLim : _fncNode->ichLim;
    }

    IdentPtr ASTBuilderBase::Pid(LPCWSTR value)
    {
        return !String::IsNullOrEmpty(value) ? _parser->CreatePid(value, wcslen(value)) : nullptr;
    }

    ParseNodePtr ASTBuilderBase::Undefined() 
    {
        return Name(GetPidCache()->undefined());
    }

    ParseNodePtr ASTBuilderBase::Assign(ParseNodePtr target, ParseNodePtr expr)
    {
        Assert(target);
        return _parser->CreateBinNode(knopAsg, target, expr, _min, _lim); 
    }

    ParseNodePtr ASTBuilderBase::Block(ParseNodePtr body)
    {
        return _parser->CreateBlockNode(_min, _lim);
    }

    ParseNodePtr ASTBuilderBase::Name(LPCWSTR name) 
    {
        return Name(Pid(name));
    }

    ParseNodePtr ASTBuilderBase::Name(IdentPtr name) 
    {
        return name ? _parser->CreateNameNode(name, _min, _lim) : Undefined();
    }

    ParseNodePtr ASTBuilderBase::String(LPCWSTR value) 
    {
        return String(Pid(value));
    }

    ParseNodePtr ASTBuilderBase::String(IdentPtr value) 
    {
        if(!value) return Undefined();
        auto node = _parser->CreateStrNode(value);
        node->ichMin = _min;
        node->ichLim = _lim;
        return node;
    }

    ParseNodePtr ASTBuilderBase::Int(int value)
    {
        auto node = _parser->CreateIntNode(value);
        node->ichMin = _min;
        node->ichLim = _lim;
        return node;
    }

    ParseNodePtr ASTBuilderBase::Array(ParseNodePtr items)
    {
        auto arr = _parser->CreateNode(knopArray, _min, _lim);
        arr->sxArrLit.pnode1 = items;
        arr->sxArrLit.arrayOfTaggedInts = false;
        arr->sxArrLit.arrayOfInts = false;
        arr->sxArrLit.arrayOfNumbers = false;
        arr->sxArrLit.hasMissingValues = false;
        arr->sxArrLit.count = ASTHelpers::List::Count(items);
        arr->sxArrLit.spreadCount = 0;
        return arr;
    }

    ParseNodePtr ASTBuilderBase::Object(ParseNodePtr membersList)
    {
        return _parser->CreateUniNode(knopObject, membersList, _min, _lim);
    }

    ParseNodePtr ASTBuilderBase::Member(LPCWSTR name, ParseNodePtr value)
    {
        return _parser->CreateBinNode(knopMember, String(name), value, _min, _lim);
    }

    ParseNodePtr ASTBuilderBase::Member(IdentPtr name, ParseNodePtr value)
    {
        return _parser->CreateBinNode(knopMember, String(name), value, _min, _lim);
    }

    ParseNodePtr ASTBuilderBase::Dot(ParseNodePtr left, ParseNodePtr right)
    {
        Assert(left);
        return _parser->CreateBinNode(knopDot, left, right, _min, _lim);
    }

    ParseNodePtr ASTBuilderBase::CallNode(OpCode nop, ParseNodePtr target, ParseNodePtr args)
    {
        Assert(target);
        Assert(nop == knopCall || nop == knopNew);

        auto call = _parser->CreateCallNode(nop, target, args, _min, _lim);
        call->sxCall.argCount = (uint16)ASTHelpers::List::Count(args);
        // Detect calls to 'eval' and set the necessary flags
        if(target->nop == knopName && target->sxPid.pid && target->sxPid.pid->GetIsEval())
        {
            call->sxCall.isEvalCall = TRUE;
            _fncNode->sxFnc.SetCallsEval(TRUE);
        }

        return call;
    }

    ParseNodePtr ASTBuilderBase::Call(ParseNodePtr target, ParseNodePtr args)
    {
        return CallNode(knopCall, target, args);
    }

    ParseNodePtr ASTBuilderBase::New(ParseNodePtr target, ParseNodePtr args)
    {
        return CallNode(knopNew, target, args);
    }

    ParseNodePtr ASTBuilderBase::Var(IdentPtr name, ParseNodePtr initExpr)
    {
        Assert(name);
        auto varNode = _parser->CreateNode(knopVarDecl, _min, _lim);
        varNode->sxVar.InitDeclNode(name, initExpr);

        if(!_fncNode->sxFnc.pnodeVars)
        {
            _fncNode->sxFnc.pnodeVars = varNode;
        }
        else
        {
            auto lastVar = _fncNode->sxFnc.pnodeVars;
            while(lastVar->sxVar.pnodeNext)
            {
                lastVar = lastVar->sxVar.pnodeNext;
            }
            lastVar->sxVar.pnodeNext = varNode;
        }

        return varNode;
    }

    ParseNodePtr ASTBuilderBase::Let(IdentPtr name, ParseNodePtr initExpr)
    {
        Assert(name && _blockNode && _blockNode->nop == knopBlock);
        auto letNode = _parser->CreateNode(knopLetDecl, _min, _lim);
        letNode->sxVar.InitDeclNode(name, initExpr);

        name->SetIsLetOrConst();

        if(!_blockNode->sxBlock.pnodeLexVars)
        {
            _blockNode->sxBlock.pnodeLexVars = letNode;
        }
        else
        {
            auto lastLexVar = _blockNode->sxBlock.pnodeLexVars;
            while(lastLexVar->sxVar.pnodeNext)
            {
                lastLexVar = lastLexVar->sxVar.pnodeNext;
            }
            lastLexVar->sxVar.pnodeNext = letNode;
        }

        return letNode;
    }

    ParseNodePtr ASTBuilderBase::If(ParseNodePtr cond, ParseNodePtr ifTrue, ParseNodePtr ifFalse)
    {
        auto ifNode = _parser->CreateNode(knopIf, _min, _lim);
        ifNode->sxIf.pnodeCond = cond;
        ifNode->sxIf.pnodeTrue = ifTrue;
        ifNode->sxIf.pnodeFalse = ifFalse;
        return ifNode;
    }

    ParseNodePtr ASTBuilderBase::Var(LPCWSTR name, ParseNodePtr initExpr)
    {
        return Var(Pid(name), initExpr);
    }

    ParseNodePtr ASTBuilderBase::Let(LPCWSTR name, ParseNodePtr initExpr)
    {
        return Let(Pid(name), initExpr);
    }

    ParseNodePtr ASTBuilderBase::Return(ParseNodePtr expr)
    {
        ParseNodePtr returnNode = _parser->CreateNode(knopReturn, _min, _lim);
        returnNode->sxReturn.pnodeExpr = expr;
        returnNode->sxReturn.pnodeOuter = nullptr;
        return returnNode;
    }

    ParseNodePtr ASTBuilderBase::True()
    {
        return _parser->CreateNode(knopTrue, _min, _lim);
    }

    ParseNodePtr ASTBuilderBase::False()
    {
        return _parser->CreateNode(knopFalse, _min, _lim);
    }

    ParseNodePtr ASTBuilderBase::Boolean(bool value)
    {
        return value ? True() : False();
    }

    ParseNodePtr ASTBuilderBase::This() 
    {
        return _parser->CreateNode(knopThis, _min, _lim);
    }

    ParseNodePtr ASTBuilderBase::Optional(ParseNodePtr node)
    {
        return node ? node : Undefined();
    }

    ParseNodePtr ASTBuilderBase::List(
        ParseNodePtr arg0, 
        ParseNodePtr arg1, 
        ParseNodePtr arg2, 
        ParseNodePtr arg3, 
        ParseNodePtr arg4, 
        ParseNodePtr arg5,
        ParseNodePtr arg6,
        ParseNodePtr arg7)
    {
        ParseNodePtr result = nullptr;
        ParseNodePtr args[] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7 };
        for(int i = 0; i < sizeof(args) / sizeof(args[0]); i++)
        {
            if(!args[i]) break;
            Append(result, args[i]);
        }
        return result;
    }

    void ASTBuilderBase::Append(ParseNodePtr& node, ParseNodePtr nodeToAppend)
    {
        Assert(nodeToAppend);
        ParseNodePtr* lastPtr = &node;
        while ((*lastPtr) && (*lastPtr)->nop == knopList)
        {
            lastPtr = &(*lastPtr)->sxBin.pnode2;
        }
        auto last = (*lastPtr);
        if(last)
            *lastPtr = _parser->CreateBinNode(knopList, last, nodeToAppend, last->ichMin, nodeToAppend->ichLim);
        else *lastPtr = nodeToAppend;
    }

    void ASTBuilderBase::Prepend(ParseNodePtr nodeToPrepend)
    {
        Assert(nodeToPrepend);
        Prepend(_fncNode->sxFnc.pnodeBody, nodeToPrepend);
    }

    void ASTBuilderBase::Prepend(ParseNodePtr& node, ParseNodePtr nodeToPrepend)
    {
        Assert(nodeToPrepend);
        if(node)
        {
            Assert(nodeToPrepend->ichMin <= node->ichMin);
        }
        node = node ? ListNode(nodeToPrepend, node) : nodeToPrepend;
    }

    void ASTBuilderBase::Append(ParseNodePtr statement)
    {
        Assert(statement);
        auto endCodeRef = FindEndCodeRef(_fncNode);
        auto endCode = endCodeRef ? *endCodeRef : nullptr;
        if(endCode)
        {
            *endCodeRef = statement;
            Append(_fncNode->sxFnc.pnodeBody, endCode);
        }
        else
        {
            // No endCode found
            Assert(false);
        }
    }
    
    bool ASTBuilderBase::Replace(ParseNodePtr node, ParseNodePtr replaceWith)
    {
        // This checks knopList nodes as well, but scans only 1 level. 
        // Should work for most rewritings.
        ParseNodePtr* nodeRef = ASTHelpers::List::Find(_fncNode->sxFnc.pnodeBody, node);
        if(!nodeRef) 
            // This performs scan into children.
            nodeRef = FindNodeReference(_fncNode, node, true); 
        if(nodeRef)
        {
            *nodeRef = replaceWith;
            return true;
        }
        return false;
    }

    bool ASTBuilderBase::Insert(ParseNodePtr& listStart, ParseNodePtr after, ParseNodePtr nodeToInsert)
    {
        Assert(after);
        Assert(nodeToInsert);
        bool result = false;

        if(listStart == after)
        {
            // The 'after' is the only node in the list. 
            listStart = ListNode(after, nodeToInsert);
            result = true;
        }
        else
        {
            // Find the parent knopList node of the 'after' node
            ParseNodePtr parentListNode = nullptr;
            ASTHelpers::List::Traverse(listStart, [&] (ParseNodePtr node) -> bool
            {
                if(node->nop == knopList && (node->sxBin.pnode1 == after || node->sxBin.pnode2 == after))
                {
                    parentListNode = node;
                    return false;
                }
                return true;
            },
            true // Include list nodes
            );

            if(parentListNode)
            {
                if(parentListNode->sxBin.pnode1 == after)
                {
                    // The 'after' node is in the middle of the list
                    parentListNode->sxBin.pnode2 = ListNode(nodeToInsert, parentListNode->sxBin.pnode2);
                }
                else if(parentListNode->sxBin.pnode2 == after)
                {
                    // The 'after' node is the last in the list
                    parentListNode->sxBin.pnode2 = ListNode(after, nodeToInsert);
                } 
                else
                {
                    Assert(false);
                }
                result = true;
            }
        }
        return result;
    }

    ParseNodePtr ASTBuilderBase::Function(LPCWSTR name, ParseNodePtr body, ParseNodePtr scopes,  ParseNodePtr argList)
    {
        ParseNodePtr fncNode = _parser->CreateNode(knopFncDecl, _min, _lim);
        IdentPtr fncName = Pid(name);

        fncNode->sxFnc.ClearFlags();
        fncNode->sxFnc.cbMin = 0;
        fncNode->sxFnc.cbLim = 0;
        fncNode->sxFnc.astSize = 0;
        fncNode->sxFnc.pid = fncName;
        fncNode->sxFnc.hint = name;
        fncNode->sxFnc.pnodeVars = nullptr;
        fncNode->sxFnc.pnodeNames = nullptr;
        fncNode->sxFnc.pnodeNext = nullptr;
        fncNode->sxFnc.pnodeRest = nullptr;
        fncNode->sxFnc.pnodeArgs = argList;

        // Set the scopes.
        ParseNodePtr blockNode = _parser->CreateBlockNode(_min, _lim);
        blockNode->sxBlock.pnodeScopes = scopes;
        blockNode->sxBlock.blockType = PnodeBlockType::Function;
        blockNode->grfpn |= fpnSyntheticNode;
        fncNode->sxFnc.pnodeBodyScope = blockNode;

        ParseNodePtr paramBlockNode = _parser->CreateBlockNode(_min, _lim);
        paramBlockNode->sxBlock.pnodeScopes = blockNode;
        paramBlockNode->sxBlock.blockType = PnodeBlockType::Parameter;
        paramBlockNode->grfpn |= fpnSyntheticNode;
        fncNode->sxFnc.pnodeScopes = paramBlockNode;

        // Set the nestedCount.
        uint nestedFunctionCount = ASTHelpers::Scope::GetNestedCount(scopes);
        ASTHelpers::Scope::SetNestedCount(_parser, fncNode, nestedFunctionCount);

        // Increment the nestedCount of the parent.
        ASTHelpers::Scope::SetNestedCount(_parser, _fncNode, _fncNode->sxFnc.nestedCount + 1);

        // Set the body
        auto endCode = EndCode();
        endCode->ichMin = _lim;
        endCode->ichLim = _lim;
        fncNode->sxFnc.pnodeBody = body ? ListNode(body, endCode) : endCode;

        // Thread the function node into the current function.
        // Add the function into the parent scope.
        ASTHelpers::Scope::Append(_currentScope, fncNode);

        return fncNode;
    }

    ParseNodePtr ASTBuilderBase::Param(IdentPtr name)
    {
        Assert(name);
        auto paramNode = _parser->CreateNode(knopVarDecl, _min, _lim);
        paramNode->sxVar.InitDeclNode(name, nullptr);

        return paramNode;
    }

    ParseNodePtr ASTBuilderBase::EndCode() 
    {
        return _parser->CreateNode(knopEndCode, _min, _lim);
    }

    void ASTBuilderBase::ValidateTree()
    {
#ifdef DEBUG
        ParseNodeVisitor<TreeValidator> validator;
        validator.DisableCleanTreeValidation();
        validator.Visit(_fncNode);
#endif
    }

    ParseNodePtr ASTBuilderBase::ListNode(ParseNodePtr node1, ParseNodePtr node2)
    {
        Assert(node1);
        Assert(node2);
        return _parser->CreateBinNode(knopList, node1, node2, node1->ichMin, node2->ichLim ? node2->ichLim : node1->ichLim);
    }

    //
    //  ASTHelpers::List
    //
    uint ASTHelpers::List::Count(ParseNodePtr nodes)
    {
        uint count = 0;
        ForEach(nodes, [&] (ParseNodePtr)
        {
            count++;
        });
        return count;
    }

    ParseNodePtr* ASTHelpers::List::Find(ParseNodePtr& current, ParseNodePtr nodeToFind)
    {
        if(!current)
            return nullptr;

        if(current == nodeToFind)
            return &current;

        if (current->nop != knopList)
            return nullptr;

        if (current->sxBin.pnode1)
        {
            auto result = Find(current->sxBin.pnode1, nodeToFind);
            if(result) return result;
        }
        if (current->sxBin.pnode2)
            return Find(current->sxBin.pnode2, nodeToFind);

        return nullptr;
    }

    //
    //  ASTHelpers::Scope
    //
    bool ASTHelpers::Scope::IsScopeNode(ParseNodePtr node)
    {
        if (node)
        {
            switch (node->nop) 
            {
            case knopProg:
            case knopFncDecl: 
            case knopCatch: 
            case knopWith: 
            case knopBlock:
                return true;
            };
        }
        return false;
    }

    ParseNodePtr* ASTHelpers::Scope::GetScopesRef(ParseNodePtr scopeNode)
    {
        Assert(scopeNode && IsScopeNode(scopeNode));

        switch (scopeNode->nop) 
        {
        case knopProg:
            return scopeNode->sxFnc.GetParamScopeRef();
        case knopFncDecl:
            return scopeNode->sxFnc.GetBodyScopeRef();
        case knopBlock:
            return &(scopeNode->sxBlock.pnodeScopes);
        case knopCatch: 
            return &(scopeNode->sxCatch.pnodeScopes);
        case knopWith: 
            return &(scopeNode->sxWith.pnodeScopes);
        };

        return nullptr;
    }

    ParseNodePtr* ASTHelpers::Scope::GetNextRef(ParseNodePtr scopeNode)
    {
        Assert(scopeNode && IsScopeNode(scopeNode));

        switch (scopeNode->nop) 
        {
        case knopProg:
        case knopFncDecl: 
            return &(scopeNode->sxFnc.pnodeNext); 
        case knopBlock:
            return &(scopeNode->sxBlock.pnodeNext);
        case knopCatch: 
            return &(scopeNode->sxCatch.pnodeNext); 
        case knopWith: 
            return &(scopeNode->sxWith.pnodeNext); 
        };

        return nullptr;
    }

    uint ASTHelpers::Scope::GetNestedCount(ParseNodePtr scopeNode)
    {
        uint nestedFunctionCount = 0;

        ForEach(&scopeNode, [&](ParseNodePtr* ppnodeScope) {
            auto pnodeScope = *ppnodeScope;

            if (pnodeScope->nop == knopFncDecl)
            {
                nestedFunctionCount ++;
            }
            else
            {
                Assert(pnodeScope->nop != knopProg);
                nestedFunctionCount += GetNestedCount(*Scope::GetScopesRef(pnodeScope));
            }
        });

        return nestedFunctionCount;
    }

    void ASTHelpers::Scope::SetNestedCount(Parser* parser, ParseNodePtr fncNode, uint nestedCount)
    {
        Assert(parser);
        Assert(fncNode);
        Assert(fncNode->nop == knopFncDecl || fncNode->nop == knopProg);

        fncNode->sxFnc.nestedCount = nestedCount;

        // update the LS Extension nodes for the functions, as CleanTree will use it to overwrite the original value
        parser->GetLanguageServiceExtension()->SetNestedCount(fncNode, fncNode->sxFnc.nestedCount);

        // set the nested flag
        if (nestedCount > 0  && fncNode->nop == knopFncDecl)
        {
            fncNode->sxFnc.SetNested();
        }
    }

    void ASTHelpers::Scope::Append(ParseNodePtr scopeNode, ParseNodePtr scopeToAppend)
    {
        Assert(scopeNode && IsScopeNode(scopeNode));
        Assert(scopeToAppend && IsScopeNode(scopeToAppend));

        ParseNodePtr* ppnodeLastScope = GetScopesRef(scopeNode);
        Assert(ppnodeLastScope);

        // Skip to the end of the list
        ForEach(ppnodeLastScope, [&](ParseNodePtr* ppnodeScope) {
            ppnodeLastScope = ppnodeScope;
        });

        if (*ppnodeLastScope)
        {
            // Point to the next of the lastScope
            ppnodeLastScope = GetNextRef(*ppnodeLastScope);
        }
        Assert(*ppnodeLastScope == nullptr);

        // Set the new scope at the end of the list
        *ppnodeLastScope = scopeToAppend;
    }

    void ASTHelpers::Scope::Prepend(ParseNodePtr scopeNode, ParseNodePtr scopeToPrepend)
    {
        Assert(scopeNode && IsScopeNode(scopeNode));
        Assert(scopeToPrepend && IsScopeNode(scopeToPrepend));

        ParseNodePtr* ppnodeFirstScope = GetScopesRef(scopeNode);
        Assert(ppnodeFirstScope);

        ParseNodePtr* ppnodeNextScope = GetNextRef(scopeToPrepend);
        Assert(ppnodeFirstScope);

        // Prepend the new node before the first scope node
        *ppnodeNextScope = *ppnodeFirstScope;
        *ppnodeFirstScope = scopeToPrepend;
    }

    void ASTHelpers::Scope::Remove(ParseNodePtr scopeNode, ParseNodePtr scopeToRemove)
    {
        Assert(scopeNode && IsScopeNode(scopeNode));
        Assert(scopeToRemove && IsScopeNode(scopeToRemove));

        // Find a reference to the node to remove in the scope chain
        ParseNodePtr* pScopeToRemove = nullptr;
        ASTHelpers::Scope::ForEach(ASTHelpers::Scope::GetScopesRef(scopeNode), [&] (ParseNodePtr* ppnodeNextScope) {
            if (*ppnodeNextScope == scopeToRemove)
                pScopeToRemove = ppnodeNextScope;
        });

        Assert (pScopeToRemove && *pScopeToRemove == scopeToRemove);

        // Remove the scope from the list
        ParseNodePtr* ppnodeNextScope = GetNextRef(scopeToRemove);
        *pScopeToRemove = *ppnodeNextScope;
    }

    bool ASTHelpers::IsArgument(ParseNodePtr pnodeFnc, ParseNodePtr pnodeVar)
    {
        if (pnodeVar && pnodeVar->nop == knopVarDecl && pnodeFnc && pnodeFnc->nop == knopFncDecl)
        {
            for (auto current = pnodeFnc->sxFnc.pnodeArgs; current; current = current->sxVar.pnodeNext)
            {
                if (pnodeVar == current)
                {
                    return true;
                }
            }
            if (pnodeVar == pnodeFnc->sxFnc.pnodeRest)
            {
                return true;
            }
        }
        return false;
    }
}
