//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline bool JavascriptMapIterator::Is(Var aValue)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(aValue);
        return typeId == TypeIds_MapIterator;
    }

    inline JavascriptMapIterator* JavascriptMapIterator::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptMapIterator'");
        
        return static_cast<JavascriptMapIterator *>(RecyclableObject::FromVar(aValue));
    }
} // namespace Js