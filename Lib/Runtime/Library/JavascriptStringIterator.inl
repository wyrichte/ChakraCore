//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline bool JavascriptStringIterator::Is(Var aValue)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(aValue);
        return typeId == TypeIds_StringIterator;
    }

    inline JavascriptStringIterator* JavascriptStringIterator::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptStringIterator'");
        
        return static_cast<JavascriptStringIterator *>(RecyclableObject::FromVar(aValue));
    }
} // namespace Js