//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class ParseNodeFinder
    {
    private:
        ParseNode *m_root;
        ParseNode *m_current;
        ParseNode *m_lastListNode;
        charcount_t m_location;
        bool m_includeListNodes;
        bool m_excludeEndOffset;
        // This is to exclude list node from Class and String template structure.
        bool m_excludeListNodeOfClassAndStringTemplate;

    public:
        ParseNodeFinder(ParseNode *root, charcount_t location, bool includeListNodes = false, bool excludeEndOffset = false, bool excludeClassAndStringTemplateListNode = false) 
            : m_root(root), m_current(nullptr), m_lastListNode(nullptr), m_location(location), m_includeListNodes(includeListNodes), m_excludeEndOffset(excludeEndOffset), m_excludeListNodeOfClassAndStringTemplate(excludeClassAndStringTemplateListNode)
        { 
            Assert(root != nullptr);
        }

        ParseNode *Start()
        {
            return m_current = (InRange(m_location, ActualMin(m_root), ActualLim(m_root), m_excludeEndOffset) ? m_root : NULL);
        }

        ParseNode *Next()
        {
            ParseNodeWalker<FindChildWalkerPolicy> walker;
            bool isCurrentClassOrStringTemplate = m_current && (m_current->nop == knopClassDecl || m_current->nop == knopStrTemplate);
            if (m_includeListNodes &&
                (!m_excludeListNodeOfClassAndStringTemplate || !isCurrentClassOrStringTemplate))
            {
                walker.IncludeListNodes(m_current);
            }
            walker.ExcludeEndOffset(m_excludeEndOffset);
            ParseNode *nextChild = walker.Walk(m_current, m_location);
            m_lastListNode = isCurrentClassOrStringTemplate ? walker.GetLastVisitedListNode() : nullptr;
            if (nextChild != null) m_current = nextChild;
            return nextChild;
        }

        template <class CallbackFn>
        void Find(CallbackFn fn)
        {
            for (ParseNode *node = Start(); node; node = Next())
            {
                if (m_lastListNode != nullptr && node != m_lastListNode)
                {
                    fn(m_lastListNode);
                }

                fn(node);
            }
        }
    };
}