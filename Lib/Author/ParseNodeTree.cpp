//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    ParseNodeTree::~ParseNodeTree() 
    { 
        ForgetTree(); 
        ReleaseActiveCursors(); 
    }

    void ParseNodeTree::SetMutated(MutationKind mutation)
    {
        m_mutations = (MutationKind)(m_mutations | mutation);
        this->UpdateVersion();
    }

    void ParseNodeTree::SetByteCodeGenerated()
    {
        m_treeWasGenerated = true;
        this->UpdateVersion();
    }

    void ParseNodeTree::SetCleaned()
    {
        m_treeWasGenerated = false;
        this->UpdateVersion();
    }

    void ParseNodeTree::Initialize(Parser* parser, ParseNodePtr tree, AuthoringFileText* text, ArenaAllocator *alloc)
    {
        Assert(parser);
        Assert(tree);
        Assert(text);

        // deactivate any active cursors
        this->UpdateVersion();

        if (m_parser)
        {
            // If there is still an old copy, forget it.
            ForgetTree();
        }

        // initialize the new tree. "!parser || alloc === parser implies alloc" In other words, if a parser is supplied so must an alloc.
        Assert(!parser || alloc);
        m_parser = parser;
        m_parserAlloc = alloc;
        m_nodeTree = tree;
        m_text = text;
        m_parserLSExtension = parser->GetLanguageServiceExtension();
        m_treeWasGenerated = false;
        m_mutations = mutNone;
    }

    // Release a parser not owned by this tree.
    void ParseNodeTree::ReleaseParser()
    {
        m_parser = nullptr;
    }

    void ParseNodeTree::ForgetTree()
    {
        // deactivate any active cursors
        this->UpdateVersion();
        m_nodeTree = nullptr;
        m_parserLSExtension = nullptr;
        m_treeWasGenerated = false;
        m_mutations = mutNone;
        if (m_parser)
        {
            Assert(m_parserAlloc);

            // The script context for the parser might already have gone away because it was invalidated because something in
            // the script context path changed.
            // Ensure the parser doesn't try to use an invalid script context.
            m_parser->ClearScriptContext(); 
            ADeletePtr(m_parserAlloc, m_parser);
        }
    }

    void ParseNodeTree::AddCursor(AuthorParseNodeCursor* pCursor)
    {
        m_activeCursors.Add(pCursor);
    }

    void ParseNodeTree::RemoveCursor(AuthorParseNodeCursor* pCursor)
    {
        if (m_activeCursors.Contains(pCursor))
        {
            m_activeCursors.Remove(pCursor);
        }
    }

    ParseNode* ParseNodeTree::FindParseNode(ArenaAllocator* alloc, charcount_t min, OpCode nop)
    {
        return FindParseNode(alloc, min, [&](ParseNode* pnode) { return ContainsOffset(pnode, min) && pnode->nop == nop; });
    }

    // A node defines a symbol if it defines a scope that includes that symbol.
    bool NodeDefines(ParseNodePtr node, IdentPtr symbol)
    {
        Assert(node);
        Assert(symbol);

        switch (node->nop)
        {
        case knopFncDecl:
            // Check the paramters
            for (auto arg = node->sxFnc.pnodeArgs; arg; arg = arg->sxVar.pnodeNext)
            {
                if (arg->sxVar.pid == symbol)
                {
                    return true;
                }
            }
            if (node->sxFnc.pnodeRest != nullptr && node->sxFnc.pnodeRest->sxVar.pid == symbol)
            {
                return true;
            }

            // Check the var declarations
            for (auto var = node->sxFnc.pnodeVars; var; var = var->sxVar.pnodeNext)
            {
                if (var->sxVar.pid == symbol) return true;
            }

            // Check the block scope if it has one.
            if (node->sxFnc.pnodeScopes) return NodeDefines(node->sxFnc.pnodeScopes, symbol);
            break;

        case knopBlock:
            if (node->sxBlock.HasBlockScopedContent())
            {
                for (auto lexvar = node->sxBlock.pnodeLexVars; lexvar; lexvar = lexvar->sxVar.pnodeNext)
                {
                    if (lexvar->sxVar.pid == symbol) return true;
                }
            }
            break;

        case knopSwitch:
            if (node->sxSwitch.pnodeBlock) return NodeDefines(node->sxSwitch.pnodeBlock, symbol);
            break;

        case knopFor:
            // TODO: Enable when sxFor gets a block scope.
            // if (node->sxFor.pnodeBlock) return NodeDefineds(node->sxFor.pnodeBlock, symbol);
            break;

        case knopForIn:
        case knopForOf:
            // TODO: Enable when sxForInOrForOf gets a block scope.
            // if (node->sxForInOrForOf.pnodeBlock) return NodeDefineds(node->sxForInOrForOf.pnodeBlock, symbol);
            break;

        case knopCatch:
            return node->sxCatch.pnodeParam && node->sxCatch.pnodeParam->nop == knopName && node->sxCatch.pnodeParam->sxPid.pid == symbol;
        }

        return false;
    }

    struct Scope
    {
        Scope *parent;
        ParseNodePtr node;
        
        Scope(Scope *parent, ParseNodePtr node): parent(parent), node(node), lastSymbol(nullptr), definesSymbol(false) { }

        Scope *Find(IdentPtr symbol)
        {
            for (auto current = this; current; current = current->parent)
            {
                if (current->Defines(symbol)) return current;
            }
            return nullptr;
        }

        bool ScopeInDoubt()
        {
            for (auto current = this; current; current = current->parent)
            {
                switch (current->node->nop)
                {
                case knopWith:
                    return true;
                case knopFncDecl:
                    if (current->node->sxFnc.CallsEval())
                        return true;
                    break;
                }
            }

            return false;
        }
    private:
        IdentPtr lastSymbol;
        bool definesSymbol;

        bool Defines(IdentPtr symbol)
        {
            if (symbol != lastSymbol) 
            {
                definesSymbol = NodeDefines(node, symbol);
                lastSymbol = symbol;
            }
                 
            return definesSymbol;
        }
    };


    bool ParseNodeTree::CollectReferences(charcount_t offset, ReferenceList *list, IdentPtr &symbol)
    {
        auto pageAllocator = GetParser()->GetScriptContext()->GetThreadContext()->GetPageAllocator();
        ArenaAllocator localArena(L"ls:CollectReferences", pageAllocator, Js::Throw::OutOfMemory);
       
        symbol = nullptr;

        ParseNodeCursor cursor(&localArena, this);

        cursor.SeekToOffset(offset);
        
        auto identMin = [&](ParseNodePtr node) -> charcount_t {
            auto min = LanguageServiceExtension()->IdentMin(node);
            if (min == 0) return ActualMin(node);
            return min;
        };

        auto identLim = [&](ParseNodePtr node) -> charcount_t {
            auto lim = LanguageServiceExtension()->IdentLim(node);
            if (lim == 0) return ActualLim(node);
            return lim;
        };

        auto inIdent = [&](ParseNodePtr node) {
            auto min = identMin(node);
            auto lim = identLim(node);
            return offset >= min && offset <= lim;
        };

        auto current = cursor.Current();
        if (current)
        {

            AuthorSymbolReferenceFlags flags = asrfLocalReference;

            // Determine the symbol and if it is a member reference.
            switch (current->nop)
            {
            case knopName:
                {
                    auto parent = cursor.Parent();

                    // If this appears as the right side of a dot expression or the left side of a literal member, it is a member reference.
                    if ((parent->nop == knopDot && parent->sxBin.pnode2 == current) || (parent->nop == knopMember && parent->sxBin.pnode1 == current))
                    {
                        flags = asrfMemberReference;
                    }

                }
                symbol = current->sxPid.pid;
                break;

            case knopStr:
                {
                    auto parent = cursor.Parent();
                    if (parent->nop == knopMember && parent->sxBin.pnode1 == current)
                    {
                        flags = asrfMemberReference;
                        symbol = current->sxPid.pid;
                    }
                }
                break;

            case knopConstDecl:
            case knopLetDecl:
            case knopVarDecl:
                if (inIdent(current))
                    symbol = current->sxVar.pid;
                break;

            case knopFncDecl:
                if (inIdent(current))
                    symbol = current->sxFnc.pid;
                break;
            }

            if (symbol && symbol != m_parser->GetErrorPid() && symbol != m_parser->GetDeclErrorPid())
            {
                auto isAssignmentNode = [&](ParseNodePtr node) {
                    if (node) 
                    {
                        switch (node->nop)
                        {
                        case knopAsg:
                        case knopAsgAdd:
                        case knopAsgAnd:
                        case knopAsgDiv:
                        case knopAsgLsh:
                        case knopAsgMod:
                        case knopAsgMul:
                        case knopAsgOr:
                        case knopAsgRs2:
                        case knopAsgRsh:
                        case knopAsgSub:
                        case knopAsgXor:
                            return true;
                        }
                    }
                    return false;
                };

                JsUtil::Stack<ParseNodePtr> parents(&localArena);
                ParseNodePtr parent = nullptr;
                Scope global(nullptr, TreeRoot());
                Scope *scope = &global;

                auto enterReference = [&](ParseNodePtr node, AuthorSymbolReferenceFlags flags)
                {
                    AuthorSymbolReference reference;
                    
                    // If the caller didn't determine the LValue vs. RValue, do it now.
                    if ((flags & (asrfLValue | asrfRValue)) == 0)
                        if (isAssignmentNode(parent) && parent->sxBin.pnode1 == node)
                            flags = (AuthorSymbolReferenceFlags)(flags | asrfLValue);
                        else
                            flags = (AuthorSymbolReferenceFlags)(flags | asrfRValue);

                    if (scope->ScopeInDoubt())
                    {
                        // Record that the scope is in doubt and that the reference is suggestive.
                        flags = (AuthorSymbolReferenceFlags)( (flags & ~asrfAuthoritative) | (asrfScopeInDoubt | asrfSuggestive));
                    }
                    reference.flags = flags;
                    if (node->nop == knopFncDecl && node->sxFnc.IsClassConstructor())
                    {
                        if (node->sxFnc.IsGeneratedDefault())
                        {
                            // empty constructor - need to discard this.
                            return;
                        }
                        // For class constructor we want to highlight only the constructor part.
                        reference.position = node->ichMin;
                        reference.length = 11; //"constructor"'s length
                    }
                    else
                    {
                        reference.position = identMin(node);
                        reference.length = identLim(node) - reference.position;
                    }

                    list->Add(reference);
                };

                if ((flags & asrfMemberReference) != 0)
                {
                    flags = (AuthorSymbolReferenceFlags)(asrfMemberReference | asrfSuggestive);
                    // For member references collect all identifiers in a member position in the AST.
                    ASTHelpers::Visit(TreeRoot(), [&](ParseNodePtr node) {
                        ParseNodePtr referenceCandidate = nullptr;
                        int candidateFlags = flags;
                        switch (node->nop)
                        {
                        case knopDot:
                            referenceCandidate = node->sxBin.pnode2;
                            break;

                        case knopGetMember:
                        case knopSetMember:
                        case knopMember:
                            referenceCandidate = node->sxBin.pnode1;
                            candidateFlags = candidateFlags | asrfLValue;
                            break;
                        }
                        if (referenceCandidate)
                        {
                            switch (referenceCandidate->nop)
                            {
                            case knopName:
                            case knopStr:
                                if (referenceCandidate->sxPid.pid == symbol)
                                    enterReference(referenceCandidate, (AuthorSymbolReferenceFlags)candidateFlags);
                                break;
                            }
                        }
                        parents.Push(parent);
                        parent = node;
                        return true;
                    }, [&](ParseNodePtr node) {
                        parent = parents.Pop();
                    });
                    return true;
                }
                else
                {
                    // For other references, find all references that are defined by the same scope.
                    
                    // Find the defining scope (or nullptr for global).
                    ParseNodePtr node = cursor.Up();
                    for (; node != nullptr; node = cursor.Up())
                    {
                        if (NodeDefines(node, symbol)) break;
                    }

                    auto pushScope = [&](ParseNodePtr node) {
                        scope = Anew(&localArena, Scope, scope, node);
                    };

                    auto popScope = [&]() {
                        auto oldScope = scope;
                        scope = scope->parent;
                        Adelete(&localArena, oldScope);
                    };

                    Scope* definingScope = nullptr;
                    if (node) 
                    {
                        // This is a local reference. Find the local references to this node.    
                        pushScope(node);
                        definingScope = scope;
                        flags = asrfLocalReference;
                    }
                    else
                    {
                        node = TreeRoot();
                        flags = asrfGlobalReference;
                    }

                    ASTHelpers::Visit(node, [&](ParseNodePtr node) {
                        bool candidate = false;
                        bool shouldPushScope = false;
                        int candidateFlags = flags;
                        ParseNodePtr candidateNode = node;
                        switch (node->nop)
                        {
                        case knopClassDecl:
                        {
                            // We are changing the candidate node to be it's declaration name.
                            candidateFlags = candidateFlags | (asrfLValue | asrfAuthoritative);
                            if (node->sxClass.pnodeDeclName && node->sxClass.pnodeDeclName->sxVar.pid == symbol)
                            {
                                candidateNode = node->sxClass.pnodeDeclName;
                                candidate = true;
                            }
                            else if (node->sxClass.pnodeName && node->sxClass.pnodeName->sxVar.pid == symbol)
                            {
                                candidateNode = node->sxClass.pnodeName;
                                candidate = true;
                            }
                            shouldPushScope = true;
                        }
                            break;

                        case knopFncDecl:
                            candidateFlags = candidateFlags | (asrfLValue | asrfAuthoritative);
                            candidate = node->sxFnc.pid == symbol;
                            __fallthrough;

                        case knopWith:
                        case knopBlock:
                        case knopForIn:
                        case knopForOf:
                        case knopFor:
                        case knopSwitch:
                        case knopCatch:
                            shouldPushScope = true;
                            break;

                        case knopDot:
                            {
                                auto member = node->sxBin.pnode2;
                                if (member->nop == knopName || member->nop == knopStr)
                                    member->grfpn |= fpnMemberReference;
                            }
                            break;

                        case knopName:
                            if (!(node->grfpn & fpnMemberReference))
                            {
                                candidate = node->sxPid.pid == symbol;
                            }
                            break;

                        case knopVarDecl:
                        case knopConstDecl:
                        case knopLetDecl:
                            candidate = node->sxVar.pid == symbol;
                            candidateFlags = candidateFlags | (asrfLValue | asrfAuthoritative);
                            break;
                        }

                        // If the identifier matches and it is defined in the same scope, enter the reference.
                        if (candidate)
                        {
                            auto symbolScope = scope->Find(symbol); 
                            
                            if (symbolScope == definingScope || (symbolScope && definingScope && symbolScope->node == definingScope->node))
                            {
                                // If the symbol scope is global, it is a suggestive reference, otherwise it is an authoritative reference.
                                if ((candidateFlags & (asrfAuthoritative | asrfSuggestive)) == 0)
                                    if (!symbolScope || symbolScope->node == this->TreeRoot())
                                        candidateFlags = candidateFlags | asrfSuggestive;
                                    else
                                        candidateFlags = candidateFlags | asrfAuthoritative;

                                enterReference(candidateNode, (AuthorSymbolReferenceFlags)candidateFlags);
                            }
                        }

                        if (shouldPushScope)
                            pushScope(node);

                        parents.Push(parent);
                        parent = node;
                        return true;
                    }, [&](ParseNodePtr node) {
                        parent = parents.Pop();
                        if (scope->node == node)
                            popScope();
                        if (node->nop == knopDot)
                        {
                            auto member = node->sxBin.pnode2;
                            if (member->nop == knopName || member->nop == knopStr)
                                member->grfpn &= ~fpnMemberReference;
                        }
                    });

                    return true;
                }
            }
        }

        return false;
    }

    bool ParseNodeTree::ContainsName(Js::InternalString *name)
    {
        return m_parser->KnownIdent(name->GetBuffer(), name->GetLength());       
    }

}