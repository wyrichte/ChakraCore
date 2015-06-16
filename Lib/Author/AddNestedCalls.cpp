//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    namespace Names
    {
        const wchar_t arguments[] = L"arguments";
        const wchar_t callee[] = L"callee";
        const wchar_t _called[] = L"_$called";
        const wchar_t callClassMethod[] = L"_$callClassMethod";
        const wchar_t callGetterSetter[] = L"_$callGetterSetter";
        const wchar_t callNested[] = L"_$callNested";
        const wchar_t fnc[] = L"_$$fnc$$";
        const wchar_t prototype[] = L"prototype";
        const wchar_t prototypeCall[] = L"_$prototypeCall";
        const wchar_t savedThis[] = L"_$$this$$";
        const wchar_t tmpmember[] = L"_$$tmpmember$$";
    }

    ParseNodePtr StrNode(Parser *parser, IdentPtr ptr)
    {
        auto result = parser->CreateStrNode(ptr);
        result->sxPid.sym = nullptr;
        return result;
    }

    ParseNodePtr GetCallNestedCall(Parser* parser, IdentPtr functionIdent, IdentPtr functionName, bool isGenerator = false, IdentPtr thisObject = nullptr, bool useNew = false)
    {
        // Create a call to _$callNested helper function
        // function _$callNested(f, name, isGenerator, thisObject, forceNew)
        return parser->CreateCallNode(knopCall, parser->CreateNameNode(CreatePidFromLiteral(parser, Names::callNested)),
                            parser->CreateBinNode(knopList,
                                parser->CreateNameNode(functionIdent),
                                parser->CreateBinNode(knopList,
                                    StrNode(parser, functionName ? functionName : functionIdent),
                                    parser->CreateBinNode(knopList,
                                        parser->CreateNode(isGenerator ? knopTrue : knopFalse),
                                        parser->CreateBinNode(knopList,
                                            thisObject ? parser->CreateNameNode(thisObject) : parser->CreateNode(knopNull),
                                            parser->CreateNode(useNew ? knopTrue : knopFalse))))));
    }

    ParseNodePtr GetPrototypeCallCall(Parser* parser, IdentPtr functionIdent, IdentPtr ctorObject)
    {
        // Create a call to _$prototypeCall helper function
        // function _$prototypeCall(f, ctor)

        Assert(ctorObject);
        return parser->CreateCallNode(knopCall,
            parser->CreateNameNode(CreatePidFromLiteral(parser, Names::prototypeCall)),
            parser->CreateBinNode(knopList,
                parser->CreateNameNode(functionIdent),
                parser->CreateNameNode(ctorObject)));

    }

    ParseNodePtr GetCallClassMethodCall(Parser* parser, IdentPtr ctorObj, IdentPtr memberName, bool isStaticMember)
    {
        // Create a call to _$callClassMethod helper function
        // function _$callClassMethod(ctor, name, isStatic)
        return parser->CreateCallNode(knopCall, parser->CreateNameNode(CreatePidFromLiteral(parser, Names::callClassMethod)),
            parser->CreateBinNode(knopList,
                parser->CreateNameNode(ctorObj),
                parser->CreateBinNode(knopList,
                    StrNode(parser, memberName),
                    parser->CreateNode(isStaticMember ? knopTrue : knopFalse))));
    }

    ParseNodePtr GetCallGetterSetterCall(Parser* parser, IdentPtr obj, IdentPtr memberName, bool isGetter, bool protoCall)
    {
        // Create a call to _$callGetterSetter helper function
        // function _$callGetterSetter(obj, prop, isGetter, protoCall)
        return parser->CreateCallNode(knopCall, parser->CreateNameNode(CreatePidFromLiteral(parser, Names::callGetterSetter)),
            parser->CreateBinNode(knopList,
                parser->CreateNameNode(obj),
                parser->CreateBinNode(knopList,
                    StrNode(parser, memberName),
                    parser->CreateBinNode(knopList,
                        parser->CreateNode(isGetter ? knopTrue : knopFalse),
                        parser->CreateNode(protoCall ? knopTrue : knopFalse)))));
    }

    bool IsIdentifierNode(ParseNodePtr pnode, IdentPtr ident)
    {
        return pnode && pnode->nop == knopName && pnode->sxPid.pid == ident;
    }

    ParseNodePtr First(ParseNodePtr pnode)
    {
        return !pnode || pnode->nop != knopList ? pnode : pnode->sxBin.pnode1;
    }

    ParseNodePtr FindActualLiteral(Parser* parser, ParseNodePtr pnode)
    {
        if (pnode && pnode->nop == knopObject)
            return pnode;

        // Otherwise, try to unwrap this from a potential doc rewrite which will look like,
        // Object.defineProperties(<actual literal>
        if (pnode && pnode->nop == knopCall && pnode->sxCall.pnodeTarget && pnode->sxCall.pnodeTarget->nop == knopDot &&
            IsIdentifierNode(pnode->sxCall.pnodeTarget->sxBin.pnode1, CreatePidFromLiteral(parser, Names::Object)) &&
            IsIdentifierNode(pnode->sxCall.pnodeTarget->sxBin.pnode2, CreatePidFromLiteral(parser, Names::defineProperties)))
        {
            auto param = First(pnode->sxCall.pnodeArgs);
            if (param && param->nop == knopObject)
                return param;
        }

        return nullptr;
    }

    template< size_t N >
    IdentPtr SaveExprToTemporary(Parser *parser, ParseNodePtr pnodeParent, ParseNodePtr pnodeContainer, ParseNodePtr pnode, 
        const wchar_t(&tempName)[N])
    {
        ParseNodePtr* ppnode = NULL;
        IdentPtr tempIdent = NULL;

        ppnode = FindNodeReference(pnodeContainer, pnode);
        if (ppnode)
        {
            tempIdent = CreatePidFromLiteral(parser, tempName);
            *ppnode = RewriteTreeHelpers::RememberNodeInto(parser, pnodeParent, pnode, tempIdent);
        }

        return tempIdent;
    }

    template< size_t N >
    IdentPtr SaveDeclarationToTemporary(Parser* parser, ParseNodePtr pnodeParent, ParseNodePtr pnodeDecl, IdentPtr declName, const wchar_t(&tempName)[N])
    {
        Assert(pnodeDecl);

        IdentPtr tempIdent = CreatePidFromLiteral(parser, tempName);

        // To ensure we remember this declaration even if there is a variable with the same name that overrides it,
        // create a var declaration to hold the declared name and insert just after the declaration node
        Assert(pnodeParent && (pnodeParent->nop == knopProg || pnodeParent->nop == knopFncDecl));
        ASTBuilder<> ast(parser, pnodeParent);
        ast.SetExtent(ActualLim(pnodeDecl));
        auto var = ast.Var(tempIdent, ast.Name(declName));
        ast.SetExtent(ActualMin(pnodeDecl), ActualLim(pnodeDecl));
        ast.Replace(pnodeDecl, ast.List(pnodeDecl, var));

        return tempIdent;
    }

    void AddNestedCallToParentFunction(Parser* parser, ParseNodePtr call, ParseNodePtr pnodeParent, ParseNodePtr pnodeFnc)
    {
        Assert(call && pnodeParent);
        ApplyLocation(call, pnodeParent->ichLim);
        RewriteTreeHelpers::EnsureExecutionOf(parser, pnodeParent, call);

        // Prepend: 
        //    arguments.callee._$called = true
        // to have the function mark itself as being called. This ensures that, if the function is called
        // by normal evaluation, we will not try to call it again as a nested function potentially adding
        // a nested function instance to the list of function to be called with undefined values as the 
        // parent's parameter values instead of the values of the original call.
        Assert(pnodeFnc && pnodeFnc->nop == knopFncDecl);
        ASTBuilder<> ast(parser, pnodeFnc);
        ast.SetExtent(pnodeFnc->sxFnc.pnodeBody->ichMin);
        ast.Prepend(ast.Assign(ast.Dot(ast.Dot(ast.Name(Names::arguments), ast.Name(Names::callee)), ast.Name(Names::_called)), ast.True()));
    }

    bool AddNestedCalls::Preorder(ParseNodePtr pnode, Context context)
    {
        if (pnode && (pnode->nop == knopFncDecl || pnode->nop == knopProg) && pnode->ichMin <= context->offset && pnode->ichLim >= context->offset)
        {
            // This is a function that contains the offset.

            // Check if the offset is in a nested function.
            ParseNodeFinder finder(pnode, context->offset);
            ParseNodePtr pnodeThisContainer = NULL;
            ParseNodePtr pnodeThisExpr = NULL;
            bool thisExprAsCtor = false;
            ParseNodePtr pnodePrototypeLiteral = NULL;
            ParseNodePtr pnodeFnc = NULL;
            ParseNodePtr pnodeFncContainer = NULL;
            ParseNodePtr pnodeAccessorMember = NULL;
            ParseNodePtr pnodeClassMember = NULL;
            ParseNodePtr pnodeCall = NULL;
            IdentPtr pidPrototype = CreatePidFromLiteral(context->parser, Names::prototype);

            finder.Start();
            for (ParseNodePtr previous = pnode, current = finder.Next(); current; previous = current, current = finder.Next())
            {
                switch (current->nop)
                {
                case knopFncDecl:
                    pnodeFnc = current;
                    pnodeFncContainer = previous;
                    break;

                case knopAsg:
                    // If function is in an assignment statement to a dotted expression then the left side of the dot should be remembered 
                    // as a candidate for the this parameter.
                    if (current->sxBin.pnode1 && current->sxBin.pnode1->nop == knopDot && current->sxBin.pnode2 && current->sxBin.pnode2->nop == knopFncDecl &&
                        context->offset >= current->sxBin.pnode1->ichLim)
                    {
                        pnodeThisExpr = current->sxBin.pnode1->sxBin.pnode1;
                        pnodeThisContainer = current->sxBin.pnode1;
                        thisExprAsCtor = false;

                        // If the this expression is a prototype assignment, such as,
                        //   MyObj.prototype.myFunc = function () ...
                        // we should remember the function identifier to call to create the this instance instead.
                        if (pnodeThisExpr->nop == knopDot && 
                                pnodeThisExpr->sxBin.pnode2->nop == knopName && pnodeThisExpr->sxBin.pnode2->sxPid.pid == pidPrototype &&
                                pnodeThisExpr->sxBin.pnode1->nop == knopName)
                        {
                            pnodeThisContainer = pnodeThisExpr;
                            pnodeThisExpr = pnodeThisExpr->sxBin.pnode1;
                            thisExprAsCtor = true;
                        }
                    }
                    // Check for the ...prototype = { someFunc: function () } cases.
                    else if (current->sxBin.pnode1 && current->sxBin.pnode1->nop == knopDot &&
                        IsIdentifierNode(current->sxBin.pnode1->sxBin.pnode2, pidPrototype) &&
                        context->offset >= current->sxBin.pnode1->ichLim)
                    {
                        pnodePrototypeLiteral = FindActualLiteral(context->parser, current->sxBin.pnode2);
                        if (pnodePrototypeLiteral)
                        {
                            // Instead of remembering the literal, remember the prototype function.
                            pnodeThisExpr = current->sxBin.pnode1->sxBin.pnode1;
                            pnodeThisContainer = current->sxBin.pnode1;
                            thisExprAsCtor = true;
                        }
                    }
                    continue;

                case knopObject:
                    if (current != pnodePrototypeLiteral)
                    {
                        // A function expression in an object literal should assume the object literal as the this parameter.
                        pnodeThisExpr = current; 
                        pnodeThisContainer = previous;
                        thisExprAsCtor = false;
                    }
                    continue;

                case knopClassDecl:
                    pnodeThisExpr = current;
                    pnodeThisContainer = previous;
                    thisExprAsCtor = true;
                    continue;

                case knopMember:
                case knopMemberShort:
                case knopSetMember:
                case knopGetMember:
                    // If this is a member of a class decl, the class decl should be used to obtain a candidate this parameter.
                    if (previous->nop == knopClassDecl)
                    {
                        Assert(pnodeThisExpr && pnodeThisExpr->nop == knopClassDecl);

                        // Do not used the stored this as a constructor, if this is a static member
                        thisExprAsCtor = (current->sxBin.pnode2->sxFnc.IsStaticMember()) ? false : true;
                        pnodeClassMember = current;
                        break;
                    }

                    if (current->nop == knopSetMember || current->nop == knopGetMember)
                    {
                        pnodeAccessorMember = current;
                        break;
                    }
                    continue;

                case knopCall:
                    pnodeCall = current;
                    continue;

                default:
                    // Keep looking.
                    continue;
                }

                break;
            }

            if (pnodeCall && pnodeFnc && pnodeCall->sxCall.pnodeTarget == pnodeFnc)
            {   
                // Nothing to do, the call will already for the function target to be called, no need to force it again
            }
            else if (pnodeFnc)
            {
                // pnodeFnc is a nested function that contains offset so this function should be rewritten to 
                // ensure that pnodeFnc gets called at least once. This is done by wrapping the body of the 
                // function in a try-finally and injecting the call to the nested function in the manufactured
                // finally.

                Parser *parser = context->parser;
                IdentPtr functionIdent = NULL;
                IdentPtr thisIdent = NULL;
                IdentPtr functionName = NULL;

                if (pnodeFnc->sxFnc.IsDeclaration() && !IsFunctionRedefined(pnodeFnc) && !(pnodeFnc->sxFnc.IsClassMember() && pnodeFnc->sxFnc.IsGeneratedDefault()))
                {
                    // Store the function name to figure out if the function should be called as a constructor
                    functionName = pnodeFnc->sxFnc.pid;

                    functionIdent = SaveDeclarationToTemporary(parser, pnode, pnodeFnc, functionName, Names::fnc);

                    if (pnodeFnc->sxFnc.pnodeNames && pnodeFnc->sxFnc.pnodeNames->nop == knopScope)
                    {
                        // Ignore the event handling syntax by replacing it with normal name node
                        pnodeFnc->sxFnc.pnodeNames = parser->CreateNode(knopVarDecl, pnodeFnc->sxFnc.pnodeNames->ichMin, pnodeFnc->sxFnc.pnodeNames->ichLim);
                        pnodeFnc->sxFnc.pnodeNames->sxVar.InitDeclNode(functionName, nullptr);
                    }
                }
                else
                {
                    // If the function was redefined, set the function to be an expression to allow the use in assignment statement
                    if (pnodeFnc->sxFnc.IsDeclaration())
                        pnodeFnc->sxFnc.SetDeclaration(false);

                    // The function is a function expression. We need to save the value of the function into a temporary 
                    // variable then use that temporary to call it.
                    functionIdent = SaveExprToTemporary(parser, pnode, pnodeFncContainer, pnodeFnc, Names::fnc);

                    // If we found a candidate this parameter to use, remember that as well.
                    if (pnodeThisExpr && !thisExprAsCtor)
                    {
                        thisIdent = SaveExprToTemporary(parser, pnode, pnodeThisContainer, pnodeThisExpr, Names::savedThis);
                    }
                }

                // If we do not yet have a this candidate and the this expr is a constructor, save it.
                if (!thisIdent && pnodeThisExpr && thisExprAsCtor)
                {
                    // Save the this expression to a temporary identifier.
                    thisIdent = SaveExprToTemporary(parser, pnode, pnodeThisContainer, pnodeThisExpr, Names::savedThis);
                }

                Assert(functionIdent);
                if (functionIdent)
                {
                    // TODO: This is where adding parameters of the right type would be useful.
                    ParseNodePtr call = nullptr;

                    if (thisIdent)
                    {
                        call = thisExprAsCtor ? 
                            // We found a potential this constructor. Call the this constructor using _$prototypeCall.
                            GetPrototypeCallCall(parser, functionIdent, thisIdent) :
                            // Otherwise, schedule the function to be called with the candidate this parameter
                            GetCallNestedCall(parser, functionIdent, functionName, pnodeFnc->sxFnc.IsGenerator(), thisIdent);
                    }

                    if (!call)
                    {
                        // Failing to find a good candidate for a this parameter, schedule the function to be called.
                        // If the function is potentially a constructor (e.g. has a "this dot" assignment) then
                        // we should call it as a constructor, 
                        //   e.g. new _$$fnc$$();
                        // otherwise we use a helper to call the method,
                        //   _$callNested(fnc, "fnc");
                        if (pnodeFnc->sxFnc.HasThisStmt())
                            call = GetCallNestedCall(parser, functionIdent, functionName, pnodeFnc->sxFnc.IsGenerator(), /* thisObject = */ nullptr, /* useNew = */ true);
                        else
                            call = GetCallNestedCall(parser, functionIdent, functionName, pnodeFnc->sxFnc.IsGenerator());
                    }

                    AddNestedCallToParentFunction(parser, call, pnode, pnodeFnc);
                }
            }
            else if (pnodeClassMember)
            {
                Assert(pnodeClassMember && (pnodeClassMember->nop == knopGetMember
                    || pnodeClassMember->nop == knopSetMember || pnodeClassMember->nop == knopMember)
                    && pnodeClassMember->sxBin.pnode2->sxFnc.IsClassMember());

                Parser *parser = context->parser;
                IdentPtr thisIdent = NULL;
                ParseNodePtr call = NULL;

                bool isStatic = pnodeClassMember->sxBin.pnode2->sxFnc.IsStaticMember();

                Assert(isStatic == !thisExprAsCtor);

                // The cursor is in a class member. Ensure the 'this' candidate is a class.
                Assert(pnodeThisExpr && pnodeThisContainer && pnodeThisExpr->nop == knopClassDecl);

                if (pnodeThisExpr->sxClass.isDeclaration)
                {
                    IdentPtr className = pnodeThisExpr->sxClass.pnodeName->sxVar.pid;
                    thisIdent = SaveDeclarationToTemporary(parser, pnode, pnodeThisExpr, className, Names::savedThis);
                }
                else
                {
                    // Remember the class expression containing the member
                    thisIdent = SaveExprToTemporary(parser, pnode, pnodeThisContainer, pnodeThisExpr, Names::savedThis);
                }

                Assert(thisIdent);
                if (IsClassMemberRedefined(pnodeThisExpr, pnodeClassMember))
                {
                    pnodeClassMember->sxBin.pnode1->sxPid.pid = CreatePidFromLiteral(parser, Names::tmpmember);
                }

                IdentPtr memberName = nullptr;
                if (pnodeClassMember->sxBin.pnode1->nop == knopStr)
                {
                    memberName = pnodeClassMember->sxBin.pnode1->sxPid.pid;
                }
                else
                {
                    // TODO : Need to handle this one correctly as work-item, for now avoiding crash.
                    Assert(pnodeClassMember->sxBin.pnode1->nop == knopComputedName);
                    memberName = CreatePidFromLiteral(parser, L"");
                }

                call = (pnodeClassMember->nop == knopGetMember || pnodeClassMember->nop == knopSetMember) ?
                    GetCallGetterSetterCall(parser, thisIdent, memberName,
                        (pnodeClassMember->nop == knopGetMember), !isStatic) :
                    GetCallClassMethodCall(parser, thisIdent, memberName, isStatic);

                Assert(call);
                AddNestedCallToParentFunction(parser, call, pnode, pnodeClassMember->sxBin.pnode2);
            }
            else if (pnodeAccessorMember)
            {
                // The cursor is in a object literal get or set member, enusre it is called.

                Assert(pnodeThisExpr);
                Assert(thisExprAsCtor ? (pnodePrototypeLiteral != NULL) : (pnodeThisExpr->nop == knopObject));
                Parser *parser = context->parser;

                // Remember the object containing the member.
                IdentPtr thisIdent = SaveExprToTemporary(parser, pnode, pnodeThisContainer, pnodeThisExpr, Names::savedThis);

                Assert(thisIdent);
                // If the offset is in a redefined getter or setter change the name to make sure it gets executed
                if (IsAccessorRedefined(thisExprAsCtor ? pnodePrototypeLiteral : pnodeThisExpr, pnodeAccessorMember))
                {
                    pnodeAccessorMember->sxBin.pnode1->sxPid.pid = CreatePidFromLiteral(parser, Names::tmpmember);
                }
                
                IdentPtr memberName = nullptr;
                if (pnodeAccessorMember->sxBin.pnode1->nop == knopStr)
                {
                    memberName = pnodeAccessorMember->sxBin.pnode1->sxPid.pid;
                }
                else
                {
                    // TODO : Need to handle this one correctly as work-item, for now avoiding crash.
                    Assert(pnodeAccessorMember->sxBin.pnode1->nop == knopComputedName);
                    memberName = CreatePidFromLiteral(parser, L"");
                }

                // Create the call node
                ParseNodePtr call = GetCallGetterSetterCall(parser, thisIdent, memberName,
                    (pnodeAccessorMember->nop == knopGetMember), thisExprAsCtor);

                Assert(call);
                AddNestedCallToParentFunction(parser, call, pnode, pnodeAccessorMember->sxBin.pnode2);
            }
        }

        return true;
    }

    bool AddNestedCalls::IsFunctionRedefined(ParseNode* pnodeFnc)
    {
        Assert(pnodeFnc);
        Assert(pnodeFnc->nop == knopFncDecl);

        bool isRedefined = false;
        if (pnodeFnc->sxFnc.IsDeclaration() && pnodeFnc->sxFnc.pid) 
        {
            // Look in the scopes following this function and see if there is another declaration with the same name
            ASTHelpers::Scope::ForEach(&(pnodeFnc->sxFnc.pnodeNext), [&] (ParseNodePtr* ppnodeNext){
                if ((*ppnodeNext)->nop == knopFncDecl && (*ppnodeNext)->sxFnc.IsDeclaration() && (*ppnodeNext)->sxFnc.pid == pnodeFnc->sxFnc.pid)
                {
                    isRedefined = true;
                }
            });
        }
        return isRedefined;
    }

    bool AddNestedCalls::IsAccessorRedefined(ParseNode* pnodeObj, ParseNode* pnodeAccessor)
    {
        Assert(pnodeObj && pnodeObj->nop == knopObject);
        Assert(pnodeAccessor && (pnodeAccessor->nop == knopGetMember || pnodeAccessor->nop == knopSetMember));
        Assert(pnodeAccessor->sxBin.pnode1 &&
                (pnodeAccessor->sxBin.pnode1->nop == knopStr || pnodeAccessor->sxBin.pnode1->nop == knopComputedName));

        bool isRedefined = false;
        bool found = false;
        // First find the definition of pnodeAccessor, then look in the following definitions for another accessor with the same name
        ASTHelpers::List::ForEach(pnodeObj->sxUni.pnode1, [&] (ParseNode* pnodeMember) {
            if (found)
            {
                OpCode memberOpCode = pnodeMember->sxBin.pnode1->nop;
                Assert(pnodeMember->sxBin.pnode1 &&
                    (memberOpCode == knopStr || memberOpCode == knopComputedName));

                if (pnodeMember->nop == pnodeAccessor->nop && 
                    ((memberOpCode == knopStr && pnodeMember->sxBin.pnode1->sxPid.pid == pnodeAccessor->sxBin.pnode1->sxPid.pid)
                    || (memberOpCode == knopComputedName && pnodeMember->sxBin.pnode1->sxUni.pnode1 == pnodeAccessor->sxBin.pnode1->sxUni.pnode1)))
                {
                    isRedefined = true;
                }
            }
            else
            {
                found = pnodeMember == pnodeAccessor;
            }
        
        });
        return isRedefined;
    }

    bool AddNestedCalls::IsClassMemberRedefined(ParseNode* pnodeObj, ParseNode* pnodeMember)
    {
        Assert(pnodeObj && pnodeObj->nop == knopClassDecl);
        Assert(pnodeMember && (pnodeMember->nop == knopGetMember || pnodeMember->nop == knopSetMember || pnodeMember->nop == knopMember) 
            && pnodeMember->sxBin.pnode2->sxFnc.IsClassMember());

        Assert(pnodeMember->sxBin.pnode1 &&
            (pnodeMember->sxBin.pnode1->nop == knopStr || pnodeMember->sxBin.pnode1->nop == knopComputedName));

        bool isStatic = pnodeMember->sxBin.pnode2->sxFnc.IsStaticMember();

        bool isRedefined = false;
#if DBG
        bool found = false;
#endif
        // First find the definition of pnodeMember, then look in the following definitions for another class member of a conflicting type with the same name
        ASTHelpers::List::ForEach((isStatic ? pnodeObj->sxClass.pnodeStaticMembers : pnodeObj->sxClass.pnodeMembers), [&](ParseNode* pnodeCurrent) {
            OpCode currentOpCode = pnodeCurrent->sxBin.pnode1->nop;

            Assert(pnodeCurrent->sxBin.pnode1 &&
                (currentOpCode == knopStr || currentOpCode == knopComputedName));
#if DBG
                found = found || (pnodeCurrent == pnodeMember);
#endif
                if ((pnodeCurrent != pnodeMember) && 
                    // Non-accessor members conflict with any other member of the same name. 
                    // Accessor members only conflict with each other if they are of the same type.
                    (pnodeMember->nop == knopMember || pnodeCurrent->nop == knopMember || pnodeCurrent->nop == pnodeMember->nop) &&
                    ((currentOpCode == knopStr && pnodeCurrent->sxBin.pnode1->sxPid.pid == pnodeMember->sxBin.pnode1->sxPid.pid)
                    || (currentOpCode == knopComputedName && pnodeCurrent->sxBin.pnode1->sxUni.pnode1 == pnodeMember->sxBin.pnode1->sxUni.pnode1)))
                {
                    isRedefined = true;
                }
        });
        AssertMsg(found, "The target member should always be present in the appropriate member list.");
        return isRedefined;

    }
}
