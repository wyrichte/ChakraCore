//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    struct AddNestedCallsContext
    {
        Parser *parser;
        charcount_t offset;
    };

    struct AddNestedCalls : VisitorPolicyBase<AddNestedCallsContext *>
    {
    public:
        bool Preorder(ParseNode* pnode, Context context);
    private:
        static bool IsFunctionRedefined(ParseNode* pnodeFnc);
        static bool IsAccessorRedefined(ParseNode* pnodeObj, ParseNode* pnodeAccessor);
        static bool IsClassMemberRedefined(ParseNode* pnodeObj, ParseNode* pnodeMember);
    };
}

