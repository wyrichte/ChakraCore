//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    struct DirectExecutionContext 
    {
        ParseNode* target;
        Parser* parser;
        ParseNode* pnodeFnc;
        JsUtil::Stack<ParseNode *> *stack;
    };

    // Applying this transformation directs execution towards the offset in the context
    // by adding an "|| true" to all conditions leading to offset. 
    struct DirectExecution : VisitorPolicyBase<DirectExecutionContext *>
    {
    private:
        bool OffsetIn(ParseNode *pnode, charcount_t offset)
        {
            return pnode && (charcount_t)pnode->ichMin <= offset && offset < (charcount_t)ActualLim(pnode);
        }

        bool TargetIn(ParseNode *pnode, ParseNode *target);

        ParseNode *ForcedExpr(Parser *parser, ParseNode *expr, OpCode forcedTo)
        {
            Assert(expr && (forcedTo == knopTrue || forcedTo == knopFalse));
            return parser->CreateBinNode(forcedTo == knopTrue ? knopLogOr : knopLogAnd, expr, parser->CreateNode(forcedTo, expr->ichLim, expr->ichLim));
        }

        bool OffsetInScopes(ParseNode* current, charcount_t offset);
        bool OffsetInNestedFnc(Context context);
        void ForceCatch(Parser* parser, ParseNode* pnodeTryCatch, ParseNode* pnodeFnc);

    public:
        bool Preorder(ParseNode* pnode, Context context);
        void Postorder(ParseNode* pnode, Context context);
    };
}