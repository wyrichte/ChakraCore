//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class ParseNodeCursor
    {
    private:
        JsUtil::List<ParseNode *, ArenaAllocator> *m_parents;
        ParseNode *m_root;
        charcount_t m_offset;
        LanguageServiceExtension* m_lsExtension;
        bool m_includeListNodes;
    public:
        ParseNodeCursor(ArenaAllocator *alloc, ParseNodeTree *nodeTree, bool includeListNodes = false);
        ParseNodeCursor(ArenaAllocator *alloc, ParseNodePtr root, bool includeListNodes = false);

        ParseNode *Current() { return m_parents->Count() > 0 ? m_parents->Item(m_parents->Count() - 1) : NULL; }
        ParseNode *Parent() { return m_parents->Count() > 1 ? m_parents->Item(m_parents->Count() - 2): NULL;}
        ParseNode *NthParent(int parentIndex) { return (parentIndex > 0 && m_parents->Count() > parentIndex) ? m_parents->Item(m_parents->Count() - (parentIndex + 1)): NULL;}
        charcount_t Offset() { return m_offset; }

        ParseNode *SeekToOffset(charcount_t offset, bool excludeEndOffset = false);
        HRESULT Child (AuthorParseNodeEdge childLabel, long index, ParseNodePtr* result);
        HRESULT MoveToChild (AuthorParseNodeEdge childLabel, long index);
        ParseNode *Up();

        template <class TAccept>
        ParseNode* Up(TAccept accept)
        {
            return Up(accept, NeverStop);
        }

        template <class TAccept, class TStop>
        ParseNode* Up(TAccept accept, TStop stop)
        {
            ParseNode *current = Current();
            while((current != null) && !accept(current))
            {
                if(stop(current))
                {
                    return nullptr;
                }

                current = Up();
            }

            return current;
        }

        bool InALoop();
        int SwitchLevel();
        bool InAFunction();
        bool IsNumericLiteral(ParseNodePtr pnode);
        bool RightOfDot();
        bool IsCallOrAssignmentTarget();
        LPCWSTR LeftOfDotIdentifier();
        bool InAJumpStatement();
        bool IsErrorNode(ParseNode* node);

    private:
        static bool NeverStop(ParseNode* node)
        {
            return false;
        }
        static bool IsJumpNode(ParseNode* node);
    };

}