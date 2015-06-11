//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

template <
    class T,
    class TStack = JsUtil::Stack<T, ArenaAllocator>>
class AutoStackItem
{
private:
    TStack * stack;

public:
    AutoStackItem(TStack* stack, T value) throw() : stack(stack)
    {
        Assert(this->stack);
        if (this->stack)
        {
            this->stack->Push(value);
        }
    }

    ~AutoStackItem() throw()
    {
        Assert(this->stack);
        if (this->stack)
        {
            this->stack->Pop();
        }
    }
};
