//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

#define LOOP_GUARD_DISABLED_FLAG L"_$lg_disabled"
#define LOOP_GUARD_LIMIT L"_$lg_limit"
#define LOOP_EXCEPTION_LIMIT 10

namespace Authoring
{
    // Applying this tranformation ensures all loops only execute up to N times.
    struct AddLoopGuards : GuardingVisitor<Parser *>
    {
    private:
        // Return a guarded version of the given boolean expression
        ParseNode *GuardExpression(Parser *parser, ParseNode *pnodeParent, ParseNode *expr);
        ParseNode *ForInOrForOfGuard(Parser *parser, ParseNode *pnodeParent, ParseNode *expr);

    protected:
        AddLoopGuards(): GuardingVisitor('l') { }

        bool Preorder(ParseNode* pnode, Parser *parser);
    };

    class LoopGuardsHelpers
    {
    public:
        static void EnableLoopGuards(Js::ScriptContext *scriptContext)
        {
            Assert(scriptContext);

            // Enable the loop guards. This allows context files to always include the guards but they are only enabled if the loop guard flag is enabled.
            JsHelpers::SetField(scriptContext->GetGlobalObject(), LOOP_GUARD_DISABLED_FLAG, false, scriptContext);
        }

        static void DisableLoopGuards(Js::ScriptContext *scriptContext)
        {
            Assert(scriptContext);

            // Disable the loop guards. This allows context files to always include the guards but they are only enabled if the loop guard flag is enabled.
            JsHelpers::SetField(scriptContext->GetGlobalObject(), LOOP_GUARD_DISABLED_FLAG, true, scriptContext);
        }

        static bool LoopGuardsEnabled(Js::ScriptContext *scriptContext)
        {
            Assert(scriptContext);

            auto gaurdValue = JsHelpers::GetProperty<Js::RecyclableObject*>(scriptContext->GetGlobalObject(), LOOP_GUARD_DISABLED_FLAG, nullptr, scriptContext);

            // guardValue will be undefined on the first context file (helpers)
            if (gaurdValue == scriptContext->GetLibrary()->GetUndefined())
            {
                return false;
            }

            return gaurdValue == scriptContext->GetLibrary()->GetFalse();
        }
    };

}