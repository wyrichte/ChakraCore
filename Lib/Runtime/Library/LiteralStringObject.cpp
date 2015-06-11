//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptStringObject::JavascriptStringObject(DynamicType * type)
        : DynamicObject(type), value(nullptr)
    {
        Assert(type->GetTypeId() == TypeIds_StringObject);

        this->GetTypeHandler()->ClearHasOnlyWritableDataProperties(); // length is non-writable
        if(GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsPrototypeFlag)
        {
            // No need to invalidate store field caches for non-writable properties here.  Since this type is just being created, it cannot represent 
            // an object that is already a prototype.  If it becomes a prototype and then we attempt to add a property to an object dervied from this 
            // object, then we will check if this property is writable, and only if it is will we do the fast path for add property.
            GetLibrary()->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();
        }
    }

    JavascriptStringObject::JavascriptStringObject(JavascriptString* value, DynamicType * type)
        : DynamicObject(type), value(value)
    {
        Assert(type->GetTypeId() == TypeIds_StringObject);

        this->GetTypeHandler()->ClearHasOnlyWritableDataProperties(); // length is non-writable
        if(GetTypeHandler()->GetFlags() & DynamicTypeHandler::IsPrototypeFlag)
        {
            // No need to invalidate store field caches for non-writable properties here.  Since this type is just being created, it cannot represent 
            // an object that is already a prototype.  If it becomes a prototype and then we attempt to add a property to an object dervied from this 
            // object, then we will check if this property is writable, and only if it is will we do the fast path for add property.
            GetLibrary()->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();
        }
    }

    bool JavascriptStringObject::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_StringObject;
    }

    JavascriptStringObject* JavascriptStringObject::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptString'");

        return static_cast<JavascriptStringObject *>(RecyclableObject::FromVar(aValue));
    }

    void JavascriptStringObject::Initialize(JavascriptString* value)
    {
        Assert(this->value == nullptr);

        this->value = value;
    }

    JavascriptString* JavascriptStringObject::InternalUnwrap()
    {
        if (value == nullptr)
        {
            ScriptContext* scriptContext = GetScriptContext();
            value = scriptContext->GetLibrary()->GetEmptyString();
        }

        return value;
    }

    JavascriptStringObject* JavascriptStringObject::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        DynamicType *type = scriptContext->GetLibrary()->GetStringTypeDynamic();
        JavascriptStringObject* result = RecyclerNew(recycler, CopyOnWriteObject<JavascriptStringObject>, type, this, scriptContext);
        result->value = value;
        return result;
    }

} // namespace Js
