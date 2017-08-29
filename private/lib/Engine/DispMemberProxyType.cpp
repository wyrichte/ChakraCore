/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"

Js::PropertyQueryFlags DispMemberProxy::HasPropertyQuery(Js::PropertyId propertyId)
{
    AssertMsg(false, "DispMemberProxy::HasPropertyQuery should never be called");
    return Js::PropertyQueryFlags::Property_NotFound;
}

Js::PropertyQueryFlags DispMemberProxy::GetPropertyQuery(Js::Var originalInstance,Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{
    AssertMsg(false, "DispMemberProxy::GetPropertyQuery should never be called");
    return Js::PropertyQueryFlags::Property_NotFound;
}

Js::PropertyQueryFlags DispMemberProxy::GetPropertyQuery(Js::Var originalInstance,Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{
    AssertMsg(false, "DispMemberProxy::GetPropertyQuery should never be called");
    return Js::PropertyQueryFlags::Property_NotFound;
}

Js::PropertyQueryFlags DispMemberProxy::GetPropertyReferenceQuery(Js::Var originalInstance,Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{
    AssertMsg(false, "DispMemberProxy::GetRefProperty should never be called");
    return Js::PropertyQueryFlags::Property_NotFound;
}

BOOL DispMemberProxy::SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    AssertMsg(false, "DispMemberProxy::SetProperty should never be called");
    return false;
}

BOOL DispMemberProxy::SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    AssertMsg(false, "DispMemberProxy::SetProperty should never be called");
    return false;
}

BOOL DispMemberProxy::SetAccessor(Js::PropertyId propertyId, Js::Var setter, Js::Var getter)
{
    AssertMsg(false, "DispMemberProxy::SetAccessor should never be called");
    return false;
}

BOOL DispMemberProxy::InitProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info)
{
    AssertMsg(false, "DispMemberProxy::InitProperty should never be called");
    return false;
}

BOOL DispMemberProxy::DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags)
{
    AssertMsg(false, "DispMemberProxy::DeleteProperty should never be called");
    return false;
}

BOOL DispMemberProxy::DeleteProperty(Js::JavascriptString *propertyNameString, Js::PropertyOperationFlags flags)
{
    AssertMsg(false, "DispMemberProxy::DeleteProperty should never be called");
    return false;
}

// Have to override this, otherwise we get 'function'.
Var DispMemberProxy::GetTypeOfString(Js::ScriptContext * requestContext)
{
    return requestContext->GetLibrary()->GetObjectTypeDisplayString();
}