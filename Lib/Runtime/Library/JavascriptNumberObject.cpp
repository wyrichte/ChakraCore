//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptNumberObject::JavascriptNumberObject(DynamicType * type)
        : DynamicObject(type), value(Js::TaggedInt::ToVarUnchecked(0))
    {
        Assert(type->GetTypeId() == TypeIds_NumberObject);
    }

    JavascriptNumberObject::JavascriptNumberObject(Var value, DynamicType * type) 
        : DynamicObject(type), value(value)
    {
        Assert(type->GetTypeId() == TypeIds_NumberObject);
        Assert(TaggedInt::Is(value) || JavascriptNumber::Is(value));
        Assert(TaggedInt::Is(value) || !ThreadContext::IsOnStack(value));
    }

    bool JavascriptNumberObject::Is(Var aValue)
    {        
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_NumberObject;
    }

    JavascriptNumberObject* JavascriptNumberObject::FromVar(Var aValue)                            // Any var: Int, Object,
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptNumber'");

        return static_cast<JavascriptNumberObject *>(RecyclableObject::FromVar(aValue));
    }

    Var JavascriptNumberObject::Unwrap() const
    {        
        return value;
    }

    double JavascriptNumberObject::GetValue() const
    {
        if (TaggedInt::Is(value)) 
        {
            return TaggedInt::ToDouble(value);
        }
        Assert(JavascriptNumber::Is(value));
        return JavascriptNumber::GetValue(value);
    }

    void JavascriptNumberObject::SetValue(Var value)
    {
        Assert(TaggedInt::Is(value) || JavascriptNumber::Is(value));
        Assert(TaggedInt::Is(value) || !ThreadContext::IsOnStack(value));

        this->value = value;
    }

    JavascriptNumberObject* JavascriptNumberObject::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        DynamicType *type = scriptContext->GetLibrary()->CreateObjectTypeNoCache(RecyclableObject::FromVar(scriptContext->CopyOnWrite(this->GetPrototype())), this->GetTypeId());
        auto result = CopyOnWriteObject<JavascriptNumberObject>::New(recycler, type, this, scriptContext);
        result->value = scriptContext->CopyOnWrite(value);
        return result;
    }

} 
