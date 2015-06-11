//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace JSON 
{
    class JSONStack;
    class StrictEqualsObjectComparer
    {
    public:       
        static bool Equals(Js::Var x, Js::Var y);
    };

    class JSONStack
    {
    private:
        SList<Js::Var> jsObjectStack; //TODO: Consider key only dictionary here
        typedef JsUtil::List<Js::Var, ArenaAllocator, false, Js::CopyRemovePolicy, 
            SpecializedComparer<Js::Var, JSON::StrictEqualsObjectComparer>::TComparerType> DOMObjectStack;
        DOMObjectStack *domObjectStack;
        ArenaAllocator *alloc;
        Js::ScriptContext *scriptContext;

    public:
        JSONStack(ArenaAllocator *allocator, Js::ScriptContext *context): jsObjectStack(allocator), domObjectStack(null), alloc(allocator), scriptContext(context)
        {
        }

        static bool Equals(Js::Var x, Js::Var y)
        {
            return Js::JavascriptOperators::StrictEqual(x,y, ((Js::RecyclableObject *)x)->GetScriptContext()) == TRUE;
        }

        bool Has(Js::Var data, bool bJsObject = true) const
        {
            if (bJsObject)
            {
                return jsObjectStack.Has(data);
            }
            else if (domObjectStack)
            {
                return domObjectStack->Contains(data);
            }
            return false;
        }

        bool Push(Js::Var data, bool bJsObject = true)
        {
            if (bJsObject)
            {
                return jsObjectStack.Push(data);
            }
            EnsuresDomObjectStack();
            domObjectStack->Add(data);
            return true;
        }

        void Pop(bool bJsObject = true)
        {
            if (bJsObject)
            {
                jsObjectStack.Pop();
                return;
            }
            AssertMsg(domObjectStack != NULL, "Misaligned pop");
            domObjectStack->RemoveAtEnd();
        }

    private:
        void EnsuresDomObjectStack(void)
        {
            if (!domObjectStack)
            {
                domObjectStack = DOMObjectStack::New(alloc);
            }
        }

    };
} //namespace JSON