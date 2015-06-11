//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptBooleanObject::JavascriptBooleanObject(JavascriptBoolean* value, DynamicType * type) 
        : DynamicObject(type), value(value)
    {        
        Assert(type->GetTypeId() == TypeIds_BooleanObject);
    }

    bool JavascriptBooleanObject::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_BooleanObject;
    }

    JavascriptBooleanObject* JavascriptBooleanObject::FromVar(Js::Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptBooleanObject'");

        return static_cast<JavascriptBooleanObject *>(RecyclableObject::FromVar(aValue));
    }

    BOOL JavascriptBooleanObject::GetValue() const
    {
        if (this->value == nullptr)
        {
            return false;
        }
        return this->value->GetValue();
    }

    void JavascriptBooleanObject::Initialize(JavascriptBoolean* value)
    {
        Assert(this->value == nullptr);

        this->value = value;
    }

} // namespace Js
