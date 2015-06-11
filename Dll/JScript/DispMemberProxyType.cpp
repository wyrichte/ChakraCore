/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

BOOL DispMemberProxy::HasProperty(Js::PropertyId propertyId)
{
    AssertMsg(false, "DispMemberProxy::HasProperty should never be called");
    return false;
}

BOOL DispMemberProxy::GetProperty(Js::Var originalInstance,Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{
    AssertMsg(false, "DispMemberProxy::GetProperty should never be called");
    return false;
}

BOOL DispMemberProxy::GetProperty(Js::Var originalInstance,Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{
    AssertMsg(false, "DispMemberProxy::GetProperty should never be called");
    return false;
}

BOOL DispMemberProxy::GetPropertyReference(Js::Var originalInstance,Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info,
    Js::ScriptContext* requestContext)
{
    AssertMsg(false, "DispMemberProxy::GetRefProperty should never be called");
    return false;
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

// Have to override this, otherwise we get 'function'.
Var DispMemberProxy::GetTypeOfString(Js::ScriptContext * requestContext)
{
    return requestContext->GetLibrary()->GetObjectTypeDisplayString();
}

Var DispMemberProxy::InvokePut(Js::Arguments args)
{    
    return this->InvokeByDispId(args, this->m_id, true);
}
