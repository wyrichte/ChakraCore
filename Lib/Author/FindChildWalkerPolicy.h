//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring 
{
    struct FindChildWalkerPolicy
    {
    private:
        ParseNode* m_lastVisitedNode;
        ParseNode* m_lastVisitedListNode;
        bool m_includeListNodes;
        bool m_excludeEndOffset;

    public:
        typedef ParseNode *ResultType;
        typedef long Context;

        FindChildWalkerPolicy() : m_lastVisitedNode(nullptr), m_lastVisitedListNode(nullptr), m_includeListNodes(false), m_excludeEndOffset(false)
        {
        }

        ParseNode* GetLastVisitedListNode() const { return m_lastVisitedListNode; }

        inline void IncludeListNodes(ParseNode* lastVisitedNode) { m_includeListNodes = true; m_lastVisitedNode = lastVisitedNode; }

        inline void ExcludeEndOffset(bool excludeEndOffset) { m_excludeEndOffset = excludeEndOffset; }

        /*
         * The parse tree for tagged string template may mislead the children finding process as follow:
         *
         * Consider the simple JavaScript program as follow:
         * function firstName()
         * {
         *   return "Andrew";
         * }
         *
         * function processTemplate(strings, values)
         * {
         *   return strings[0] + values + strings[1];
         * }
         *
         * var s = processTemplate`Hello ${firstName()},`;
         *
         * WScript.Echo(s);
         *
         * The parse tree look like this
         *
         * [0, 217):     program
         *       Scopes: knopBlock synthetic (0-217)
         *         Scopes: knopFncDecl (0-48), knopFncDecl (52-145)
         * [0, 48):       fn decl 1 nested 0 name firstName (0-48)
         *         Scopes: knopBlock synthetic (18-48)
         * [29, 48):         List
         * [29, 44):           return
         * [36, 44):             "Andrew"
         * [47, 48):           <endcode>
         * [52, 145):       fn decl 1 nested 0 name processTemplate (52-145)
         *         Scopes: knopBlock synthetic (76-145)
         * [77, 84):         var strings
         * [102, 145):         List
         * [102, 141):           return
         * [109, 141):             +
         * [109, 128):               +
         * [109, 119):                 []
         * [109, 116):                   id: strings
         * [117, 118):                   0
         * [122, 128):                 id: values
         * [131, 141):               []
         * [131, 138):                 id: strings
         * [139, 140):                 1
         * [144, 145):           <endcode>
         * [150, 196):       var s
         * [158, 196):         Call
         * [158, 173):           id: processTemplate
         * [173, 196):           string template                                         <== The misleading string template node
         * [182, 193):           Call
         * [182, 191):             id: firstName                                         <== The node we wanted to find
         * [201, 216):       Call
         * [201, 213):         .
         * [201, 208):           id: WScript
         * [209, 213):           id: Echo
         * [214, 215):         id: s
         * [0, 0):       <endcode>
         *
         * Suppose we want to search for the firstName call id node (e.g. at offset 185) - the search will lead to the string template, and because there is no more children at the 
         * string template node - the search stops at the string template node and we never find the call node.
         *
         * Therefore we need to make sure the search continues if we find the string template node, there might be siblings that we need to look at.
         *
         */
        inline bool ContinueWalk(ParseNode *result) { return (result == nullptr || result->nop == knopStrTemplate && result->sxStrTemplate.isTaggedTemplate ); }

        inline ParseNode *DefaultResult() { return NULL; }

        inline ParseNode *WalkNode(ParseNode *pnode, Context context) { return NULL; }

        inline ParseNode *WalkListNode(ParseNode *pnode, Context context) 
        {
            m_lastVisitedListNode = pnode;
            return m_includeListNodes && pnode != m_lastVisitedNode ? WalkChild(pnode, context) : NULL;
        }

        inline ParseNode *WalkChild(ParseNode *pnode, Context context) 
        { 
            return pnode && 
                // Ignore generated nodes that might be generated at the locationw are looking for.
                // Generated nodes have a zero extent.
                (pnode->ichMin != pnode->ichLim || pnode->nop == knopDoWhile || pnode->nop == knopStr) && 
                // If the node is in range return it, else continue looking.
                InRange(context, ActualMin(pnode), ActualLim(pnode), m_excludeEndOffset) ? pnode : NULL; 
        }

        inline ParseNode *WalkFirstChild(ParseNode *pnode, Context context) { return WalkChild(pnode, context); }    

        inline ParseNode *WalkSecondChild(ParseNode *pnode, Context context) { return WalkChild(pnode, context); }    

        inline ParseNode *WalkNthChild(ParseNode *pparentnode, ParseNode *pnode, Context context) { return WalkChild(pnode, context); }
        inline void WalkReference(ParseNode **ppnode, Context context) { }
    };
}