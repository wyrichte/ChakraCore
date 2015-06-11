//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

DECLARE_SIMPLEACCESSOR_INFO(0)
DECLARE_SIMPLEACCESSOR_INFO(1)
DECLARE_SIMPLEACCESSOR_INFO(2)
DECLARE_SIMPLEACCESSOR_INFO(3)
DECLARE_SIMPLEACCESSOR_INFO(4)
DECLARE_SIMPLEACCESSOR_INFO(5)
DECLARE_SIMPLEACCESSOR_INFO(6)
DECLARE_SIMPLEACCESSOR_INFO(7)
DECLARE_SIMPLEACCESSOR_INFO(8)
DECLARE_SIMPLEACCESSOR_INFO(9)
DECLARE_TEN_SIMPLEACCESS_INFO(1)
DECLARE_TEN_SIMPLEACCESS_INFO(2)
DECLARE_TEN_SIMPLEACCESS_INFO(3)
DECLARE_TEN_SIMPLEACCESS_INFO(4)
DECLARE_TEN_SIMPLEACCESS_INFO(5)
DECLARE_TEN_SIMPLEACCESS_INFO(6)
DECLARE_TEN_SIMPLEACCESS_INFO(7)
DECLARE_TEN_SIMPLEACCESS_INFO(8)
DECLARE_TEN_SIMPLEACCESS_INFO(9)
// Turn off compiler optimizations for this function, as it causes the compiler to crash with the latest compiler we got after 3/9 FI
#pragma optimize("", off)
Js::FunctionInfo DOMFastPathInfo::getterTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attributeGetter, attributeSetter) funcInfoGetter,
#include "..\..\lib\runtime\types\DOMFastPathInfolist.h"
#undef _ONE_SIMPLESLOT_RECORD
};
//Turn the optimizations back on
#pragma optimize("", on)
Js::FunctionInfo DOMFastPathInfo::setterTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) funcInfoSetter,
#include "..\..\lib\runtime\types\DOMFastPathInfolist.h"
#undef _ONE_SIMPLESLOT_RECORD
};

IR::JnHelperMethod DOMFastPathInfo::getterHelperIDTable [] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) IR::JnHelperMethod::Helper##nameGetter,
#include "..\..\lib\runtime\types\DOMFastPathInfolist.h"
#undef _ONE_SIMPLESLOT_RECORD
};

IR::JnHelperMethod DOMFastPathInfo::setterHelperIDTable [] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) IR::JnHelperMethod::Helper##nameSetter,
#include "..\..\lib\runtime\types\DOMFastPathInfolist.h"
#undef _ONE_SIMPLESLOT_RECORD
};

C_ASSERT(_countof(DOMFastPathInfo::setterTable) == Js::JavascriptLibrary::DOM_BUILTIN_MAX_SLOT_COUNT);
C_ASSERT(_countof(DOMFastPathInfo::getterTable) == Js::JavascriptLibrary::DOM_BUILTIN_MAX_SLOT_COUNT);

Js::Var __cdecl DOMFastPathInfo::CrossSiteSimpleSlotAccessorThunk(Js::RecyclableObject* recyclableObject, Js::CallInfo callInfo, ...)
{
    RUNTIME_ARGUMENTS(args, callInfo);

    Js::DynamicObject * dynamicObject = Js::DynamicObject::FromVar(recyclableObject);
    Js::ScriptContext * targetScriptContext = dynamicObject->GetScriptContext();
    if (!Js::JavascriptFunction::Is(dynamicObject))
    {
        Js::JavascriptError::ThrowTypeError(targetScriptContext, JSERR_NeedFunction);
    }

    Assert(VirtualTableInfo<Js::CrossSiteObject<Js::JavascriptTypedObjectSlotAccessorFunction>>::HasVirtualTable(recyclableObject));
    Assert(args.Info.Count > 0);

    Js::JavascriptTypedObjectSlotAccessorFunction* simpleAccessorFunction = Js::JavascriptTypedObjectSlotAccessorFunction::FromVar(dynamicObject);
    Js::FunctionInfo* funcInfo = simpleAccessorFunction->GetFunctionInfo();
    Assert((funcInfo->GetAttributes() & Js::FunctionBody::Attributes::NeedCrossSiteSecurityCheck) != 0);
    HostScriptContext* requestContext = ThreadContext::GetContextForCurrentThread()->GetPreviousHostScriptContext();
    targetScriptContext->VerifyAliveWithHostContext(!dynamicObject->IsExternal(), requestContext);
    simpleAccessorFunction->ValidateThisInstance(args[0]);

    HRESULT hr = requestContext->VerifyDOMSecurity(targetScriptContext, args.Values[0]);
    if (FAILED(hr))
    {
        Js::JavascriptError::MapAndThrowError(targetScriptContext, hr);
    }

    return Js::CrossSite::CommonThunk(recyclableObject, funcInfo->GetOriginalEntryPoint(), args);
}

#if DBG_EXTRAFIELD
bool DOMFastPathInfo::VerifyObjectSize(Js::RecyclableObject* obj, size_t objectSize)
{
    Js::ScriptContext* scriptContext = obj->GetScriptContext();
    Js::CustomExternalObject* customExternalObject = Js::CustomExternalObject::FromVar(obj);
    if (sizeof(Js::CustomExternalObject) + customExternalObject->additionalByteCount >= objectSize)
    {
        return true;
    }
    AssertMsg(false, "getter/setter for slotIndex called on object without enough slot extension");
    return false;
}
#endif
