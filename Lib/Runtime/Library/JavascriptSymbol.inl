//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline bool JavascriptSymbol::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_Symbol;
    }

    inline JavascriptSymbol* JavascriptSymbol::FromVar(Js::Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptSymbol'");

        return static_cast<JavascriptSymbol *>(RecyclableObject::FromVar(aValue));
    }
} // namespace Js
