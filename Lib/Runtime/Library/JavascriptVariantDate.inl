//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline bool JavascriptVariantDate::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_VariantDate;
    }

    inline JavascriptVariantDate* JavascriptVariantDate::FromVar(Js::Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptVariantDate'");
        
        return static_cast<JavascriptVariantDate *>(RecyclableObject::FromVar(aValue));
    }
} // namespace Js
