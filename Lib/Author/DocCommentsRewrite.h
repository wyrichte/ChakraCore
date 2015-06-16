//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class Pids;

    struct RewriteContext
    {
        ArenaAllocator* alloc;
        ParseNodeTree* tree;
        AuthoringFileHandle* handle;
        int fileId;
        int memberCount;
        long cursorPos;
        ParseNode **lastReferenceToNode;
        JsUtil::Stack<ParseNode *> nodeStack;
        JsUtil::Stack<int> memberCountStack;

        size_t level;
        size_t blockScopeLevel;
        Pids* pids;

        ParseNodePtr previousNode;  // Used by RewriteVarDecl

#if DEBUG
        int nestedCount;
#endif

        RewriteContext(ArenaAllocator* alloc, ParseNodeTree* tree, AuthoringFileHandle* handle, int fileId, long cursorPos = -1);

        template <typename THandler>
        ParseNodePtr FindParent(int index, THandler accept)
        {
            int parentIndex = index;
            while (parentIndex < this->nodeStack.Count())
            {
                auto currentNode = this->nodeStack.Peek(parentIndex);
                Assert (currentNode);

                if (accept(currentNode))
                    return currentNode;

                parentIndex++;
            }
            return nullptr;
        }

        ParseNodePtr GetParent();
        ParseNodePtr GetFunction();
        ParseNodePtr GetBlockFromGlobal();
    };

    class DocCommentsRewrite : public VisitorPolicyBase<RewriteContext*>
    {
    protected:
        bool Preorder(ParseNode *pnode, RewriteContext *context);
        void Postorder(ParseNode *pnode, RewriteContext *context);
        void PassReference(ParseNode **ppnode, RewriteContext *context);
    };

    bool HasRelevantTags(LPCWSTR text);
    bool IsSafeTypeExpression(LPCWSTR typeExpression);
}
