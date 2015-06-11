//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring 
{
    struct CleanTreeContext 
    {
        LanguageServiceExtension* LSExtension;
    };

    // Applying this transformation removes all annotations added by ByteCodeGenerator and restores the
    // tree to a state the ByteCodeGenerator can use to generate code again.
    struct CleanTree : VisitorPolicyBase<CleanTreeContext*>
    {
        void CleanNames(ParseNode *pnodeNames);
        bool Preorder(ParseNode *pnode, Context context);
    };
}