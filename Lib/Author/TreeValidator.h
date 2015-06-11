//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{

#ifdef DEBUG
    struct TreeValidator : VisitorPolicyBase<NullType>
    {
        bool m_validateCleanTree;

        TreeValidator()
            :m_validateCleanTree(true) 
        {
        }

        void DisableCleanTreeValidation() { m_validateCleanTree = false; }
        void EnableCleanTreeValidateion() { m_validateCleanTree = true; }

        struct ChildLocationValidator: WalkerPolicyBase<bool, ParseNodePtr>
        {
            typedef WalkerPolicyBase<bool, ParseNodePtr> base;

            inline bool ContinueWalk(ResultType value) { return value; }
            inline bool DefaultResult() { return true; }

            virtual ResultType WalkChild(ParseNode *pnode, Context context) override;
        };

        bool Preorder(ParseNode *pnode, Context context);
    };
#endif

}