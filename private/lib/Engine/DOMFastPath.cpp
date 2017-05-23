//------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#include "EnginePch.h"

#ifdef ENABLE_DOM_FAST_PATH
#include "DOMFastPath.h"
#include "DOMFastPathInfo.h"
#include "JavascriptTypedObjectSlotAccessorFunction.h"
#include "ActiveScriptExternalLibrary.h"

DECLARE_OBJECT_SIMPLEACCESS_INFO
DECLARE_TYPE_SIMPLEACCESS_INFO

Js::FunctionInfo DOMFastPathInfo::objectGetterTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attributeGetter, attributeSetter) funcInfoGetter,
#include "DOMFastPathInfolist.h"
    OBJECT_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
};

Js::FunctionInfo DOMFastPathInfo::objectSetterTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) funcInfoSetter,
#include "DOMFastPathInfolist.h"
    OBJECT_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
};

IR::JnHelperMethod const DOMFastPathInfo::objectGetterHelperIDTable [] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) IR::JnHelperMethod::Helper##nameGetter,
#include "DOMFastPathInfolist.h"
    OBJECT_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
};

IR::JnHelperMethod const DOMFastPathInfo::objectSetterHelperIDTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) IR::JnHelperMethod::Helper##nameSetter,
#include "DOMFastPathInfolist.h"
    OBJECT_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
};

C_ASSERT(_countof(DOMFastPathInfo::objectSetterTable) == ActiveScriptExternalLibrary::DOM_BUILTIN_MAX_OBJECT_SLOT_COUNT);
C_ASSERT(_countof(DOMFastPathInfo::objectGetterTable) == ActiveScriptExternalLibrary::DOM_BUILTIN_MAX_OBJECT_SLOT_COUNT);

Js::FunctionInfo DOMFastPathInfo::typeGetterTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attributeGetter, attributeSetter) funcInfoGetter,
#include "DOMFastPathInfolist.h"
    TYPE_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
};

Js::FunctionInfo DOMFastPathInfo::typeSetterTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) funcInfoSetter,
#include "DOMFastPathInfolist.h"
    TYPE_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
};

IR::JnHelperMethod const DOMFastPathInfo::typeGetterHelperIDTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) IR::JnHelperMethod::Helper##nameGetter,
#include "DOMFastPathInfolist.h"
    TYPE_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
};

IR::JnHelperMethod const DOMFastPathInfo::typeSetterHelperIDTable[] = {
#undef _ONE_SIMPLESLOT_RECORD
#define _ONE_SIMPLESLOT_RECORD(nameGetter, nameSetter, funcInfoGetter, funcInfoSetter, entryGetter, entrySetter, attribute, attributeSetter) IR::JnHelperMethod::Helper##nameSetter,
#include "DOMFastPathInfolist.h"
    TYPE_SIMPLESLOT_RECORDS
#undef _ONE_SIMPLESLOT_RECORD
};

C_ASSERT(_countof(DOMFastPathInfo::typeSetterTable) == ActiveScriptExternalLibrary::DOM_BUILTIN_MAX_TYPE_SLOT_COUNT);
C_ASSERT(_countof(DOMFastPathInfo::typeGetterTable) == ActiveScriptExternalLibrary::DOM_BUILTIN_MAX_TYPE_SLOT_COUNT);

Js::Var __cdecl DOMFastPathInfo::CrossSiteSimpleSlotAccessorThunk(Js::RecyclableObject* recyclableObject, Js::CallInfo callInfo, ...)
{
    RUNTIME_ARGUMENTS(args, callInfo);

    Js::DynamicObject * dynamicObject = Js::DynamicObject::FromVar(recyclableObject);
    Js::ScriptContext * targetScriptContext = dynamicObject->GetScriptContext();
    HostScriptContext* requestHostContext = ThreadContext::GetContextForCurrentThread()->GetPreviousHostScriptContext();
    Js::ScriptContext* requestContext = requestHostContext->GetScriptContext();
    if (!Js::JavascriptFunction::Is(dynamicObject))
    {
        Js::JavascriptError::TryThrowTypeError(targetScriptContext, requestContext, JSERR_NeedFunction);
        return nullptr;
    }

    Assert(VirtualTableInfo<Js::CrossSiteObject<Js::JavascriptTypedObjectSlotAccessorFunction>>::HasVirtualTable(recyclableObject));
    Assert(args.Info.Count > 0);

    Js::JavascriptTypedObjectSlotAccessorFunction* simpleAccessorFunction = Js::JavascriptTypedObjectSlotAccessorFunction::FromVar(dynamicObject);
    Js::FunctionInfo* funcInfo = simpleAccessorFunction->GetFunctionInfo();
    Assert((funcInfo->GetAttributes() & Js::FunctionInfo::Attributes::NeedCrossSiteSecurityCheck) != 0);
    targetScriptContext->VerifyAliveWithHostContext(!dynamicObject->IsExternal(), requestHostContext);
    if (!simpleAccessorFunction->ValidateThisInstance(args[0]))
    {
        return nullptr;
    }

    HRESULT hr = requestHostContext->VerifyDOMSecurity(targetScriptContext, args.Values[0]);
    if (FAILED(hr))
    {
        if (targetScriptContext->GetThreadContext()->RecordImplicitException())
        {
            Js::JavascriptError::MapAndThrowError(requestContext, hr);
        }
        return nullptr;
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
#endif

template <unsigned int slotIndex>
Js::Var DOMFastPath<slotIndex>::EntrySimpleObjectSlotGetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Assert(!(callInfo.Flags & Js::CallFlags_New));

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    if (args.Info.Count == 0)
    {
        // Don't error if we disabled implicit calls
        if (scriptContext->GetThreadContext()->RecordImplicitException())
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedObject);
        }
        else
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
    }

    AssertOrFailFast(Js::JavascriptTypedObjectSlotAccessorFunction::Is(function));
    Js::JavascriptTypedObjectSlotAccessorFunction* typedObjectSlotAccessorFunction = Js::JavascriptTypedObjectSlotAccessorFunction::FromVar(function);
    typedObjectSlotAccessorFunction->ValidateThisInstance(args[0]);

    Js::ExternalObject* obj = Js::ExternalObject::FromVar(args[0]);
#if DBG_EXTRAFIELD
    Assert(DOMFastPathInfo::VerifyObjectSize(obj, sizeof(Js::ExternalObject) + (slotIndex + 1) * sizeof(PVOID)));
#endif

    Js::Var* externalVars = (Js::Var*)((byte*)obj + sizeof(Js::ExternalObject));

    Js::Var retVal = externalVars[slotIndex];

    // It's possible that the slot is null if the DOM decided to lazily initialize it.
    // In this case we need to use their trampoline.
    ScriptMethod fallBackTrampoline = typedObjectSlotAccessorFunction->GetFallBackTrampoline();
    if (externalVars[slotIndex] == nullptr && fallBackTrampoline != nullptr)
    {
        CallInfo scriptMethodCallInfo;
        scriptMethodCallInfo.Count = callInfo.Count;
        scriptMethodCallInfo.Flags = callInfo.Flags;

        Var result = nullptr;
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            result = fallBackTrampoline(static_cast<Js::Var>(function), scriptMethodCallInfo, args.Values);
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        return result;
    }

    return Js::CrossSite::MarshalVar(function->GetScriptContext(), retVal);
}

template <unsigned int slotIndex>
Js::Var DOMFastPath<slotIndex>::EntrySimpleObjectSlotSetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    if (args.Info.Count == 0)
    {
        Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_NeedObject);
    }

    if (!Js::ExternalObject::Is(args[0]))
    {
        Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_NeedObject, _u("DOM object"));
    }
    if (args.Info.Count == 1)
    {
        Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_Invalid);
    }

    AssertOrFailFast(Js::JavascriptTypedObjectSlotAccessorFunction::Is(function));
    Js::JavascriptTypedObjectSlotAccessorFunction* typedObjectSlotAccessorFunction = Js::JavascriptTypedObjectSlotAccessorFunction::FromVar(function);
    typedObjectSlotAccessorFunction->ValidateThisInstance(args[0]);

    Js::ExternalObject* obj = Js::ExternalObject::FromVar(args[0]);
#if DBG_EXTRAFIELD
    Assert(DOMFastPathInfo::VerifyObjectSize(obj, sizeof(Js::ExternalObject) + (slotIndex + 1) * sizeof(PVOID)));
#endif
    Js::Var* externalVars = (Js::Var*)((byte*)obj + sizeof(Js::ExternalObject));

    // It's possible that the slot is null if the DOM decided to lazily initialize it.
    // In this case we need to use their trampoline.
    ScriptMethod fallBackTrampoline = typedObjectSlotAccessorFunction->GetFallBackTrampoline();
    if (externalVars[slotIndex] == nullptr && fallBackTrampoline != nullptr)
    {
        CallInfo scriptMethodCallInfo;
        scriptMethodCallInfo.Count = callInfo.Count;
        scriptMethodCallInfo.Flags = callInfo.Flags;

        Js::ScriptContext* scriptContext = function->GetScriptContext();
        Var result = nullptr;
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            result = fallBackTrampoline(static_cast<Js::Var>(function), scriptMethodCallInfo, args.Values);
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        return result;
    }

    externalVars[slotIndex] = Js::CrossSite::MarshalVar(obj->GetScriptContext(), args[1]);
    return nullptr;
}

template <unsigned int slotIndex>
Js::Var DOMFastPath<slotIndex>::EntrySimpleTypeSlotGetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Assert(!(callInfo.Flags & Js::CallFlags_New));

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    if (args.Info.Count == 0)
    {
        // Don't error if we disabled implicit calls
        if (scriptContext->GetThreadContext()->RecordImplicitException())
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedObject);
        }
        else
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
    }

    AssertOrFailFast(Js::JavascriptTypedObjectSlotAccessorFunction::Is(function));
    Js::JavascriptTypedObjectSlotAccessorFunction* typedObjectSlotAccessorFunction = Js::JavascriptTypedObjectSlotAccessorFunction::FromVar(function);
    typedObjectSlotAccessorFunction->ValidateThisInstance(args[0]);

    Js::ExternalObject* obj = Js::ExternalObject::FromVar(args[0]);
    Js::Type *type = obj->GetType();
    Assert(Js::CustomExternalType::Is(type));
    Js::CustomExternalType *externalType = static_cast<Js::CustomExternalType*>(type);

    Js::Var* externalVars = (Js::Var*)((byte*)externalType + sizeof(Js::CustomExternalType));

    Js::Var retVal = externalVars[slotIndex];

    // It's possible that the slot is null if the DOM decided to lazily initialize it.
    // In this case we need to use their trampoline.
    ScriptMethod fallBackTrampoline = typedObjectSlotAccessorFunction->GetFallBackTrampoline();
    if (retVal == nullptr && fallBackTrampoline != nullptr)
    {
        CallInfo scriptMethodCallInfo;
        scriptMethodCallInfo.Count = callInfo.Count;
        scriptMethodCallInfo.Flags = callInfo.Flags;

        Var result = nullptr;
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            result = fallBackTrampoline(static_cast<Js::Var>(function), scriptMethodCallInfo, args.Values);
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        return result;
    }

    return Js::CrossSite::MarshalVar(function->GetScriptContext(), retVal);
}

template <unsigned int slotIndex>
Js::Var DOMFastPath<slotIndex>::EntrySimpleTypeSlotSetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    if (args.Info.Count == 0)
    {
        Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_NeedObject);
    }

    if (!Js::ExternalObject::Is(args[0]))
    {
        Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_NeedObject, _u("DOM object"));
    }
    if (args.Info.Count == 1)
    {
        Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_Invalid);
    }

    AssertOrFailFast(Js::JavascriptTypedObjectSlotAccessorFunction::Is(function));
    Js::JavascriptTypedObjectSlotAccessorFunction* typedObjectSlotAccessorFunction = Js::JavascriptTypedObjectSlotAccessorFunction::FromVar(function);
    typedObjectSlotAccessorFunction->ValidateThisInstance(args[0]);

    Js::ExternalObject* obj = Js::ExternalObject::FromVar(args[0]);
    Js::Type *type = obj->GetType();
    Assert(Js::CustomExternalType::Is(type));
    Js::CustomExternalType *externalType = static_cast<Js::CustomExternalType*>(type);

    Js::Var* externalVars = (Js::Var*)((byte*)externalType + sizeof(Js::CustomExternalType));

    // It's possible that the slot is null if the DOM decided to lazily initialize it.
    // In this case we need to use their trampoline.
    ScriptMethod fallBackTrampoline = typedObjectSlotAccessorFunction->GetFallBackTrampoline();
    if (externalVars[slotIndex] == nullptr && fallBackTrampoline != nullptr)
    {
        CallInfo scriptMethodCallInfo;
        scriptMethodCallInfo.Count = callInfo.Count;
        scriptMethodCallInfo.Flags = callInfo.Flags;

        Js::ScriptContext* scriptContext = function->GetScriptContext();
        Var result = nullptr;
        BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        {
            result = fallBackTrampoline(static_cast<Js::Var>(function), scriptMethodCallInfo, args.Values);
        }
        END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext)
        return result;
    }

    externalVars[slotIndex] = Js::CrossSite::MarshalVar(obj->GetScriptContext(), args[1]);
    return externalVars[slotIndex];
}
