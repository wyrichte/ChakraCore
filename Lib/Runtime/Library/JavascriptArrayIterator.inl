//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline bool JavascriptArrayIterator::Is(Var aValue)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(aValue);
        return typeId == TypeIds_ArrayIterator;
    }

    inline JavascriptArrayIterator* JavascriptArrayIterator::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptArrayIterator'");
        
        return static_cast<JavascriptArrayIterator *>(RecyclableObject::FromVar(aValue));
    }
} // namespace Js