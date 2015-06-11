//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    TYPE_STATS(RefCountedScriptContext, L"RefCountedScriptContext");

    bool IsInCallParenthesis(charcount_t offset, ParseNode* callNode, bool includeLParen, LanguageServiceExtension* extensions)
    {
        Assert(callNode);
        Assert(callNode->nop == knopNew || callNode->nop == knopCall);
        Assert(extensions);
        auto ichLParen = extensions->LParen(callNode);
        if (!ichLParen)
            return false;
        auto ichRParen = extensions->RParen(callNode);
        return InRange(offset, ichLParen + (includeLParen ? 0 : 1), ichRParen ? ichRParen : callNode->ichLim);
    }

    wchar_t ExtensionsObjectName[] = L"intellisense";

    bool IsExtensionObjectName(LPCWSTR name, charcount_t length)
    {
        return (length == (sizeof(ExtensionsObjectName) / sizeof(wchar_t) - 1)) && (wcscmp(name, ExtensionsObjectName) == 0);
    }

    struct ChildMinPolicy : WalkerPolicyBase<ParseNode *, long *>
    {
        virtual ParseNode *WalkChild(ParseNode *pnode, Context context) 
        { 
            if (pnode)
            {
                int pnodeMin = ActualMin(pnode);
                if (pnodeMin < *context) *context = pnodeMin;
            }
            return NULL; 
        }
    };

    long ActualMin(ParseNode *pnode)
    {
        long result = pnode->ichMin;
        switch (pnode->nop)
        {
        case knopProg:
            result = 0;
            break;
        case knopBlock:
        case knopTry:
        case knopTryCatch:
        case knopTryFinally:
        case knopDoWhile:
            ParseNodeWalker<ChildMinPolicy> walker;
            walker.Walk(pnode, &result);
            break;
        }
        return result;
    }

    struct ChildLimPolicy : WalkerPolicyBase<ParseNode *, long *>
    {
        virtual ParseNode *WalkChild(ParseNode *pnode, Context context) 
        { 
            if (pnode)
            {
                int pnodeLim = ActualLim(pnode);
                if (pnodeLim > *context) *context = pnodeLim;
            }
            return NULL; 
        }
    };

    long ActualLim(ParseNode *pnode)
    {
        long result = pnode->ichLim;
        switch (pnode->nop)
        {
        case knopBlock:
        case knopFor:
        case knopForIn:
        case knopForOf:
        case knopSwitch:
        case knopCase:
        case knopDefaultCase:
        case knopDoWhile:
        case knopWhile:
        case knopIf:
        case knopTry:
        case knopWith:
        case knopList:
        case knopVarDeclList:
        case knopTryCatch:
        case knopTryFinally:
        case knopCatch:
        case knopFinally:
        case knopIndex:
        case knopClassDecl:
        case knopVarDecl:
        case knopStrTemplate:
            ParseNodeWalker<ChildLimPolicy> walker;
            walker.Walk(pnode, &result);
            break;
        }
        return result;
    }

    ParseNodePtr *FindEndCodeRef(ParseNodePtr pnodeFnc)
    {
        Assert(pnodeFnc && (pnodeFnc->nop == knopFncDecl || pnodeFnc->nop == knopProg));
    
        ParseNodePtr current = pnodeFnc->sxFnc.pnodeBody;
        if (current && current->nop == knopEndCode) return &pnodeFnc->sxFnc.pnodeBody;
        while (current && current->nop == knopList) 
        {
            ParseNodePtr right = current->sxBin.pnode2;
            if (right && right->nop == knopEndCode) return &current->sxBin.pnode2;
            current = right;
        }

        return nullptr;
    }

    int CountNestedFunctions(ParseNodePtr scope)
    {
        int count = 0;
        while (scope != NULL) 
        {
            switch (scope->nop)
            {
            case knopFncDecl:
                scope = scope->sxFnc.pnodeNext;
                count++;
                break;

            case knopBlock:
                scope = scope->sxBlock.pnodeNext;
                break;

            case knopCatch:
                scope = scope->sxCatch.pnodeNext;
                break;

            case knopWith:
                scope = scope->sxWith.pnodeNext;
                break;

            default:
                AssertMsg(false, "Unexpected opcode in tree of scopes");
                __assume(false);
                break;
            }
        }
        return count;
    }

    struct ApplyLocationPolicy: VisitorPolicyBase<charcount_t>
    {
        bool Preorder(ParseNode* pnode, charcount_t location)
        {
            pnode->ichMin = location;
            pnode->ichLim = location;
            return true;
        }

    };

    // Calling ApplyLocation::Apply() will initialize the ichMin and ichMin of
    // node and all child nodes to the given location
    void ApplyLocation(ParseNodePtr pnode, charcount_t location)
    {
        ParseNodeVisitor<ApplyLocationPolicy> visitor;
        visitor.Visit(pnode, location);
    }

    // Used in FindNodeReference to find the location pointing to the given node.
    class ReferenceFinderPolicy
    {
    public:
        typedef bool ResultType;
        struct ReferenceContext
        {
            ParseNodePtr node;
            ParseNodePtr *reference;
            bool fullScope;
        };
        typedef ReferenceContext *Context;

        inline bool ContinueWalk(bool value) { return value; }
        inline bool DefaultResult() { return true; }
        inline bool WalkNode(ParseNode *node, Context context) { return true; }
        inline bool WalkListNode(ParseNode *node, Context context) { return true; }
        inline bool WalkChild(ParseNode *&node, Context context)
        { 
            if (node)
            {
                if (node == context->node) 
                {
                    context->reference = &node; 
                    return false;
                }
                if (context->fullScope && node->nop != knopFncDecl)
                {
                    ParseNodePtr *found = FindNodeReference(node, context->node, true);
                    if (found)
                    {
                        context->reference = found;
                        return false;
                    }
                }
            }
            return true;
        }
        inline bool WalkFirstChild(ParseNode *&node, Context context) { return WalkChild(node, context); }
        inline bool WalkSecondChild(ParseNode *&node, Context context) { return WalkChild(node, context);  }
        inline bool WalkNthChild(ParseNode* pnodeParent, ParseNode *&node, Context context) { return WalkChild(node, context); }
        inline void WalkReference(ParseNode **ppnode, Context context) { }
    };

    ParseNodePtr *FindNodeReference(ParseNodePtr parent, ParseNodePtr node, bool fullScope)
    {
        ParseNodeWalker<ReferenceFinderPolicy> walker;
        ReferenceFinderPolicy::ReferenceContext context;
        context.reference = NULL;
        context.node = node;
        context.fullScope = fullScope;
        walker.Walk(parent, &context);
        return context.reference;
    }

    struct FindPrevContext
    {
        ParseNode* node;
        ParseNode* result;
        FindPrevContext(ParseNode* node) : node(node), result(nullptr) { }
    };

    class FindPrevPolicy : public VisitorPolicyBase<FindPrevContext*>
    {
    protected:
        bool Preorder(ParseNode *pnode, FindPrevContext* context)
        {
            if (pnode && pnode->ichMin != pnode->ichLim)
            {
                if(pnode->nop != knopList)
                {
                    if (pnode->ichLim < context->node->ichMin)
                    {
                        if (context->result == nullptr)
                        {
                            context->result = pnode;
                        }
                        else if (pnode->ichLim > context->result->ichLim)
                        {
                            context->result = pnode;
                        }
                    }
                }
            }
            return true; 
        }
    };

    ParseNode* GetPreviousNode(ParseNode* parent, ParseNode* node)
    {
        FindPrevContext context(node);
        ParseNodeVisitor<FindPrevPolicy> visitor;
        visitor.Visit(parent, &context);
        return context.result != parent ? context.result : nullptr;
    }

    bool InternalName(LPCWSTR name) 
    { 
        return name && ((name[0] == '_' && name[1] == '$') || (name[0] == '?' && !name[1])); 
    }

    bool InternalName(IdentPtr name) 
    { 
        return name && name->Cch() >= 1 && InternalName(name->Psz()); 
    }

    JsUtil::List<RefCountedScriptContext::CallbackEntry, HeapAllocator, true, Js::CopyRemovePolicy, RefCountedScriptContext::CallbackComparer> RefCountedScriptContext::callbackEntries(&HeapAllocator::Instance);

#if DEBUG
    int InstructionsExecuted(Js::ScriptContext *scriptContext)
    {
        int result = 0;
#if DBG_DUMP
        for (int i = 0; i < sizeof(scriptContext->byteCodeHistogram)/sizeof(scriptContext->byteCodeHistogram[0]); i++)
        {
            result += scriptContext->byteCodeHistogram[i];
        }
#endif
        return result;
    }
#endif

#if DEBUG
    void DumpIdOfName(Js::ScriptContext *scriptContext, LPCWSTR name)
    {
        auto len = (int)wcslen(name);
        Js::PropertyRecord const *propertyRecord = nullptr;
        scriptContext->GetOrAddPropertyRecord(name, len, &propertyRecord);
        if (propertyRecord)
            Output::Print(L"ID: %d\n", propertyRecord->GetPropertyId());
        else
            Output::Print(L"* no id found *\n");
    }

    Js::Var DebugGetPropertyOf(Js::DynamicObject *obj, LPCWSTR propertyName)
    {
        auto len = (int)wcslen(propertyName);
        Js::PropertyRecord const *propertyRecord = nullptr;
        auto scriptContext = obj->GetScriptContext();
        scriptContext->GetOrAddPropertyRecord(propertyName, len, &propertyRecord);
        return Js::JavascriptOperators::GetProperty(obj, propertyRecord->GetPropertyId(), scriptContext, nullptr);
    }
#endif
}
