//----------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
// 
//  Implements simple, fast property getter/setter for DOM object.
//----------------------------------------------------------------------------
#pragma once

#define DECLARE_SIMPLEGETTER_INFO(x) Js::FunctionInfo DOMFastPath<x>::EntryInfo::SimpleSlotGetter(DOMFastPath<x>::EntrySimpleSlotGetter,  \
        (Js::FunctionInfo::Attributes)(Js::FunctionInfo::NeedCrossSiteSecurityCheck | Js::FunctionInfo::HasNoSideEffect | Js::FunctionInfo::CanBeHoisted), \
        Js::JavascriptBuiltInFunction::DOMFastPathGetter);

#define DECLARE_SIMPLESETTER_INFO(x) Js::FunctionInfo DOMFastPath<x>::EntryInfo::SimpleSlotSetter(DOMFastPath<x>::EntrySimpleSlotSetter,  \
        Js::FunctionInfo::NeedCrossSiteSecurityCheck, Js::JavascriptBuiltInFunction::DOMFastPathSetter);

#define DECLARE_SIMPLEACCESSOR_INFO(x) \
    DECLARE_SIMPLESETTER_INFO(x) \
    DECLARE_SIMPLEGETTER_INFO(x) \

#define DECLARE_TEN_SIMPLEACCESS_INFO(x) \
    DECLARE_SIMPLEACCESSOR_INFO(x##0) \
    DECLARE_SIMPLEACCESSOR_INFO(x##1) \
    DECLARE_SIMPLEACCESSOR_INFO(x##2) \
    DECLARE_SIMPLEACCESSOR_INFO(x##3) \
    DECLARE_SIMPLEACCESSOR_INFO(x##4) \
    DECLARE_SIMPLEACCESSOR_INFO(x##5) \
    DECLARE_SIMPLEACCESSOR_INFO(x##6) \
    DECLARE_SIMPLEACCESSOR_INFO(x##7) \
    DECLARE_SIMPLEACCESSOR_INFO(x##8) \
    DECLARE_SIMPLEACCESSOR_INFO(x##9) \

template <unsigned int slotIndex>
class DOMFastPath
{
public:
    class EntryInfo
    {
    public:
        static Js::FunctionInfo SimpleSlotSetter;
        static Js::FunctionInfo SimpleSlotGetter;
    };

    static Js::Var EntrySimpleSlotGetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & Js::CallFlags_New)); 

        if (args.Info.Count == 0)
        {
            Js::ScriptContext* scriptContext = function->GetScriptContext();
            // Don't error if we disabled implicit calls
            if(scriptContext->GetThreadContext()->RecordImplicitException())
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedObject);
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        Js::JavascriptTypedObjectSlotAccessorFunction* typedObjectSlotAccessorFunction = Js::JavascriptTypedObjectSlotAccessorFunction::FromVar(function);
        typedObjectSlotAccessorFunction->ValidateThisInstance(args[0]);

        Js::ExternalObject* obj = Js::ExternalObject::FromVar(args[0]);
#if DBG_EXTRAFIELD
        Assert(DOMFastPathInfo::VerifyObjectSize(obj, sizeof(Js::ExternalObject) + (slotIndex+1) * sizeof(PVOID)));
#endif

        Js::Var* externalVars = (Js::Var*)((byte*)obj + sizeof(Js::ExternalObject));
        Js::Var retVal = externalVars[slotIndex];
        Assert(retVal != nullptr);
        return retVal;
    }

    static Js::Var EntrySimpleSlotSetter(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);
        if (args.Info.Count == 0)
        {
            Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_NeedObject);
        }

        if (!Js::ExternalObject::Is(args[0]))
        {
            Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_NeedObject, L"DOM object");
        }
        if (args.Info.Count == 1)
        {
            Js::JavascriptError::ThrowTypeError(function->GetScriptContext(), JSERR_FunctionArgument_Invalid);
        }

        Js::JavascriptTypedObjectSlotAccessorFunction* typedObjectSlotAccessorFunction = Js::JavascriptTypedObjectSlotAccessorFunction::FromVar(function);
        typedObjectSlotAccessorFunction->ValidateThisInstance(args[0]);
        Js::ExternalObject* obj = Js::ExternalObject::FromVar(args[0]);
#if DBG_EXTRAFIELD
        Assert(DOMFastPathInfo::VerifyObjectSize(obj, sizeof(Js::ExternalObject) + (slotIndex+1) * sizeof(PVOID)));
#endif
        Js::Var* externalVars = (Js::Var*)((byte*)obj + sizeof(Js::ExternalObject));
        externalVars[slotIndex] = args[1];
        return nullptr;
    }
};
