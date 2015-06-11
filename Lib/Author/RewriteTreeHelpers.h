//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{

    class RewriteTreeHelpers
    {
    public:
        static ParseNodePtr CreateStrNode(Parser *parser, LPCWSTR value, charcount_t min, charcount_t lim);
        static ParseNodePtr CreateNameNode(Parser *parser, LPCWSTR name, charcount_t min, charcount_t lim);
        static ParseNodePtr CreateIntNode(Parser *parser, int value, charcount_t min, charcount_t lim);
        static void AddMember(Parser *parser, ParseNodePtr pnodeObject, LPCWSTR memberName, ParseNodePtr memberValue);

        static ParseNodePtr CreateVarDeclNode(Parser *parser, ParseNodePtr pnodeParentFnc, IdentPtr pid, charcount_t ichMin, charcount_t ichLim);
        static ParseNodePtr RememberNodeInto(Parser *parser, ParseNodePtr pnodeParentFnc, ParseNodePtr pnode, IdentPtr ident);

        static void FixupOuters(ParseNodePtr pnode, ParseNodePtr outer);
        static ParseNodePtr RemoveEndCode(ParseNodePtr pnode);
        static ParseNodePtr AppendEndCode(Parser *parser, ParseNodePtr pnode, charcount_t ichMin, charcount_t ichLim);
        static void EnsureExecutionOf(Parser *parser, ParseNodePtr pnodeFnc, ParseNodePtr pnodeStmt);
    };

}
