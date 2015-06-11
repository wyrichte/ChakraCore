/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "StdAfx.h"

namespace Js
{
    JavascriptTypedObjectSlotAccessorFunction::JavascriptTypedObjectSlotAccessorFunction(DynamicType* type, FunctionInfo* functionInfo, int allowedTypeId, PropertyId nameId) : 
        RuntimeFunction(type, functionInfo),
        allowedTypeId(allowedTypeId) 
    {
        DebugOnly(VerifyEntryPoint());
        SetFunctionNameId(Js::TaggedInt::ToVarUnchecked(nameId));
    }


    bool JavascriptTypedObjectSlotAccessorFunction::Is(Var instance)
    {
        if (VirtualTableInfo<Js::JavascriptTypedObjectSlotAccessorFunction>::HasVirtualTable(instance) ||
            VirtualTableInfo<Js::CrossSiteObject<Js::JavascriptTypedObjectSlotAccessorFunction>>::HasVirtualTable(instance) )
        {
            return true;
        }
        return false;
    }


    void JavascriptTypedObjectSlotAccessorFunction::ValidateThisInstance(Js::Var thisObj)
    {
        Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(thisObj);
        if (typeId != GetAllowedTypeId())
        {
            Js::JavascriptError::ThrowTypeError(GetType()->GetScriptContext(), JSERR_FunctionArgument_NeedObject, L"DOM object");
        }
    }

    JavascriptTypedObjectSlotAccessorFunction* JavascriptTypedObjectSlotAccessorFunction::FromVar(Var instance)
    {
        Assert(Js::JavascriptTypedObjectSlotAccessorFunction::Is(instance));
        Assert((Js::JavascriptFunction::FromVar(instance)->GetFunctionInfo()->GetAttributes() & Js::FunctionBody::Attributes::NeedCrossSiteSecurityCheck) != 0);
        return static_cast<JavascriptTypedObjectSlotAccessorFunction*>(instance);
    }

    void JavascriptTypedObjectSlotAccessorFunction::ValidateThis(Js::JavascriptTypedObjectSlotAccessorFunction* func, Var thisObject)
    {
        func->ValidateThisInstance(thisObject);
    }
}