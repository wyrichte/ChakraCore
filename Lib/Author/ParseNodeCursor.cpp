//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    ParseNodeCursor::ParseNodeCursor(ArenaAllocator *alloc, ParseNodeTree * nodeTree, bool includeListNodes)
    {
        Assert (NULL != nodeTree);
        m_root = nodeTree->TreeRoot();
        m_lsExtension = nodeTree->LanguageServiceExtension();
        m_offset = 0;
        m_parents = JsUtil::List<ParseNode *, ArenaAllocator>::New(alloc);
        m_parents->Add(m_root);
        m_includeListNodes = includeListNodes;
    }

    ParseNodeCursor::ParseNodeCursor(ArenaAllocator *alloc, ParseNodePtr root, bool includeListNodes)
    {
        m_root = root;
        m_lsExtension = nullptr;
        m_offset = 0;
        m_parents = JsUtil::List<ParseNode *, ArenaAllocator>::New(alloc);
        m_parents->Add(m_root);
        m_includeListNodes = includeListNodes;
    }

    ParseNode *ParseNodeCursor::SeekToOffset(charcount_t offset, bool excludeEndOffset)
    {
        m_offset = offset;
        m_parents->Clear();
        ParseNodeFinder finder(m_root, offset, m_includeListNodes, excludeEndOffset, true/*excludeListNodeOfClassAndStringTemplate*/);
        finder.Find([&](ParseNode *pnode){
            m_parents->Add(pnode);
        });

        return Current();
    }

    /*
    HRESULT Child(AuthorParseNodeEdge childLabel, long index, ParseNodePtr* result);

    HRESULT MoveToChild (AuthorParseNodeEdge childLabel, long index);
    */

    ParseNode *ParseNodeCursor::Up()
    {
        int count = m_parents->Count();
        if (count > 0)
            m_parents->RemoveAt(count - 1);
        return Current();
    }

    bool ParseNodeCursor::InALoop()
    {
        for (int i = m_parents->Count() - 1; i >= 0; --i)
        {
            switch (m_parents->Item(i)->nop)
            {
            case knopFor: 
            case knopForIn:
            case knopForOf:
            case knopWhile:
            case knopDoWhile:
                return true;
            }
        }

        return false;
    }

    int ParseNodeCursor::SwitchLevel()
    {
        int ignored = 0;
        for (int i = m_parents->Count() - 1; i >= 0; --i)
        {
            switch (m_parents->Item(i)->nop)
            {
            case knopBlock:
                // If the ichMin is past the offset this is an implied block
                // that was inserted into the tree automatically and doesn't
                // represent a block in the syntax. Ignore it.
                if (m_parents->Item(i)->ichMin > m_offset) ignored++;
                break;
            case knopSwitch: 
                return m_parents->Count() - 1 - i - ignored;
            }
        }

        return -1;
    }

    bool ParseNodeCursor::InAFunction()
    {
        for (int i = m_parents->Count() - 1; i >= 0; --i)
        {
            switch (m_parents->Item(i)->nop)
            {
            case knopFncDecl: 
                return true;
            }
        }

        return false;
    }

    bool ParseNodeCursor::IsNumericLiteral(ParseNodePtr pnode)
    {
        if (!m_lsExtension) return false;
        if (pnode)
        {
            switch (pnode->nop)
            {
            case knopInt:
            case knopFlt:
                // Check if it is a numeric literal or a numeric expression e.g. 1.0 vs. (1.0)
                return (m_lsExtension->GetParenthesesCount(pnode) == 0);
            }
        }
        return false;
    }

    bool ParseNodeCursor::RightOfDot()
    {
        int count = m_parents->Count();
        if (count > 1)
        {
            ParseNodePtr current = m_parents->Item(count - 1);
            ParseNodePtr parent = m_parents->Item(count - 2);
            return (current->nop == knopDot && !IsNumericLiteral(current->sxBin.pnode1)) || (parent->nop == knopDot && parent->sxBin.pnode2 == current && !IsNumericLiteral(parent->sxBin.pnode1));
        }
        return false;
    }

    LPCWSTR ParseNodeCursor::LeftOfDotIdentifier()
    {
        LPCWSTR identifier = nullptr;
        if(RightOfDot())
        {
            ParseNodePtr current = Current();
            if(current && current->nop != knopDot)
            {
                current = Parent();
            }
            if(current && current->nop == knopDot)
            {
                auto left = current->sxBin.pnode1;
                if(left)
                {
                    switch(left->nop)
                    {
                    case knopName: 
                        if(left->sxPid.pid)
                            identifier = left->sxPid.pid->Psz();
                        break;
                    case knopThis:
                        identifier = L"this";
                        break;
                    case knopSuper:
                        identifier = L"super";
                        break;
                    case knopDot:
                        auto parentRight = left->sxBin.pnode2;
                        if(parentRight && parentRight->nop == knopName && parentRight->sxPid.pid)
                            identifier = parentRight->sxPid.pid->Psz();
                    }
                }
            }
        }
        return identifier;
    }

    bool ParseNodeCursor::IsCallOrAssignmentTarget()
    {
        int count = m_parents->Count();
        if (count > 1)
        {
            ParseNodePtr current = m_parents->Item(count - 1);
            ParseNodePtr parent = m_parents->Item(count - 2);
            switch (parent->nop)
            {
            case knopCall:
                return parent->sxCall.pnodeTarget == current;
            case knopAsg:
            case knopAsgAdd:
            case knopAsgSub:
            case knopAsgMul:
            case knopAsgMod:
            case knopAsgAnd:
            case knopAsgOr:
            case knopAsgLsh:
            case knopAsgRsh:
            case knopAsgRs2:
                return parent->sxBin.pnode1 == current;
            }
        }
        return false;
    }

    bool ParseNodeCursor::IsErrorNode(ParseNode* node)
    {
        Assert(node != nullptr);
        if(node->nop == knopName && node->sxPid.pid && node->sxPid.pid->Cch() == 1 && node->sxPid.pid->Psz()[0] == '?')
        {
            return true;
        }

        return false;
    }

    bool ParseNodeCursor::IsJumpNode(ParseNode* node)
    {
        Assert(node);

        switch(node->nop)
        {
        case knopBreak:
        case knopContinue:
            return true;
        case knopEmpty:
            // check if the node was a jump node but changed by error recovery
            return ((node->grfpn & PNodeFlags::fpnJumbStatement) == PNodeFlags::fpnJumbStatement);
        }
        return false;
    }

    bool ParseNodeCursor::InAJumpStatement()
    {
        ParseNodePtr current = Current();
        ParseNodePtr parent = Parent();

        return (current && IsJumpNode(current) && current->ichMin < m_offset) || (parent && IsJumpNode(parent) && parent->ichMin < m_offset);
    }
}