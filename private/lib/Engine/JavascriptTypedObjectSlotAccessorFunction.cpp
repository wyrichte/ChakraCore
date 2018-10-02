//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#include "EnginePch.h"

#ifdef ENABLE_DOM_FAST_PATH
#include "DOMFastPath.h"
#include "DOMFastPathInfo.h"
#include "JavascriptTypedObjectSlotAccessorFunction.h"

namespace Js
{
    JavascriptTypedObjectSlotAccessorFunction::JavascriptTypedObjectSlotAccessorFunction(DynamicType* type, FunctionInfo* functionInfo, int allowedTypeId, PropertyId nameId, ScriptMethod fallBackTrampoline) :
        RuntimeFunction(type, functionInfo),
        allowedTypeId(allowedTypeId),
        fallBackTrampoline(fallBackTrampoline)
    {
        DebugOnly(VerifyEntryPoint());
        SetFunctionNameId(Js::TaggedInt::ToVarUnchecked(nameId));
    }

    template <> bool VarIsImpl<JavascriptTypedObjectSlotAccessorFunction>(RecyclableObject* object)
    {
        bool result = VirtualTableInfo<Js::JavascriptTypedObjectSlotAccessorFunction>::HasVirtualTable(object) ||
            VirtualTableInfo<Js::CrossSiteObject<Js::JavascriptTypedObjectSlotAccessorFunction>>::HasVirtualTable(object);
        Assert(!result || ((Js::VarTo<Js::JavascriptFunction>(object)->GetFunctionInfo()->GetAttributes() & Js::FunctionInfo::Attributes::NeedCrossSiteSecurityCheck) != 0));
        return result;
    }

    bool JavascriptTypedObjectSlotAccessorFunction::ValidateThisInstance(Js::Var thisObj)
    {
        if (!InstanceOf(thisObj))
        {
            ScriptContext* scriptContext = GetType()->GetScriptContext();
            Js::JavascriptError::TryThrowTypeError(scriptContext, scriptContext, JSERR_FunctionArgument_NeedObject, _u("DOM object"));
            return false;
        }
        return true;
    }

    bool JavascriptTypedObjectSlotAccessorFunction::InstanceOf(Var thisObj)
    {
        int allowedTypeId = GetAllowedTypeId();
        TypeId typeId = Js::JavascriptOperators::GetTypeId(thisObj);
        if (typeId == allowedTypeId)
        {
            return true;
        }

        if (!VarIs<RecyclableObject>(thisObj))
        {
            return false;
        }

        Type* type = VarTo<RecyclableObject>(thisObj)->GetType();
        if (ExternalTypeWithInheritedTypeIds::Is(type))
        {
            void* extension;
            // When we are in this code path, we know that the object is a DOM CEO and they are trying to use FTL.
            // FTL is only setup for object instance, but for prototypes (document.__proto__) they are using the same type, though
            // the object is not an instance with CBase associated. This is using the internal information about DOM's type system
            // but it is probably not worthy to do another interface/vtable call back to DOM just to call CBaseToVar which is basically
            // what we are doing here.
            // Note that if we are using a fallback trampoline for lazy FTL slots, it is OK to have a null slot. In this case the trampoline will be called.
            if (fallBackTrampoline == nullptr
                && (FAILED(JsVarToExtension(thisObj, &extension))
                    || (*(void**)extension == nullptr)))
            {
                return false;
            }
            return ((Js::ExternalTypeWithInheritedTypeIds*)type)->InstanceOf((TypeId)allowedTypeId);
        }
        return false;
    }
}
#endif
