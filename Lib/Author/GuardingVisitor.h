//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    extern int uniqueNumber;

    // This is a utility class for use in creating visitors that declare guard variables
    // in global scope. It takes care of ensuring the guard variables are initialzied in
    // global scope to 0. Uses of this class just need to call NewGuardName() to retrieve
    // a new name that will be initialized to 0. All subclasses should call the constructor
    // with a unique letter which is used to construct the unique guard variable name.
    template <class T>
    struct GuardingVisitor : VisitorPolicyBase<T>
    {
    private:
        wchar_t m_guardName[7];
        int m_firstGuard;
        int m_lastGuard;

        IdentPtr GuardName(Parser *parser, int index)
        {
            m_guardName[4] = (wchar_t)index;
            m_guardName[5] = (wchar_t)((index >> 16) | 0x4000);
            return parser->CreatePid(m_guardName, 6);
        }

    protected:
        IdentPtr NewGuardName(Parser *parser)
        {
            auto index = ++uniqueNumber;
            if (m_firstGuard < 0) m_firstGuard = index;
            m_lastGuard = index;
            return GuardName(parser, index);
        }

        GuardingVisitor(wchar_t guardChar): m_firstGuard(-1), m_lastGuard(-1)
        {
            m_guardName[0] = '_';
            m_guardName[1] = '$';
            m_guardName[2] = guardChar;
            m_guardName[3] = 'g';
            m_guardName[4] = 0;
            m_guardName[5] = 0;
            m_guardName[6] = 0;
        }

        void Postorder(ParseNode* pnode, Parser *parser)
        {
            switch (pnode->nop)        
            {
            case knopProg:
                if (m_firstGuard > 0)
                {
                    ParseNode *current = pnode->sxProg.pnodeBody;
                    charcount_t min = pnode->sxProg.pnodeBody ? pnode->sxProg.pnodeBody->ichMin : pnode->ichMin;
                    charcount_t lim = pnode->sxProg.pnodeBody ? pnode->sxProg.pnodeBody->ichLim : pnode->ichLim;
                    // Add the loop guard variables to the global scope.
                    for (int i = m_firstGuard; i <= m_lastGuard; i++)
                    {
                        ParseNodePtr varNode = parser->CreateNode(knopVarDecl, min, min);
                        varNode->sxVar.InitDeclNode(NULL, NULL);
                        varNode->sxVar.pid = GuardName(parser, i);
                        varNode->sxVar.pnodeInit = parser->CreateIntNode(0);
                        varNode->sxVar.pnodeInit->ichMin = min;
                        varNode->sxVar.pnodeInit->ichLim = min;
                        ParseNode *link = parser->CreateNode(knopList, min, lim);
                        link->sxBin.pnode1 = varNode;
                        link->sxBin.pnode2 = current;
                        current = link;
                    }
                    pnode->sxProg.pnodeBody = current;
                }
                break;
            }
        }
    };

}