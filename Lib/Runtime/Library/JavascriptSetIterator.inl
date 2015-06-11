//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline bool JavascriptSetIterator::Is(Var aValue)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(aValue);
        return typeId == TypeIds_SetIterator;
    }

    inline JavascriptSetIterator* JavascriptSetIterator::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptSetIterator'");
        
        return static_cast<JavascriptSetIterator *>(RecyclableObject::FromVar(aValue));
    }
} // namespace Js