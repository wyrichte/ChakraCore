//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptSymbolObject::JavascriptSymbolObject(JavascriptSymbol* value, DynamicType * type)
        : DynamicObject(type), value(value)
    {
        Assert(type->GetTypeId() == TypeIds_SymbolObject);
    }

    bool JavascriptSymbolObject::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_SymbolObject;
    }

    JavascriptSymbolObject* JavascriptSymbolObject::FromVar(Js::Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptSymbolObject'");

        return static_cast<JavascriptSymbolObject *>(RecyclableObject::FromVar(aValue));
    }
} // namespace Js
