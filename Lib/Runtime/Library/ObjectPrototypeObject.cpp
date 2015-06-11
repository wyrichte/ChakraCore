//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#include "StdAfx.h"

namespace Js
{
    ObjectPrototypeObject::ObjectPrototypeObject(DynamicType* type) : DynamicObject(type)
    {
        __proto__Enabled = GetScriptContext()->GetConfig()->Is__proto__Enabled();
    }

    ObjectPrototypeObject * ObjectPrototypeObject::New(Recycler * recycler, DynamicType * type)
    {
        return NewObject<ObjectPrototypeObject>(recycler, type);
    }

    Var ObjectPrototypeObject::Entry__proto__getter(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));
        ScriptContext* scriptContext = function->GetScriptContext();

#if !FLOATVAR
        // Mark temp number will stack allocate number that is used as the object ptr.
        // So we should box it before call ToObject on it.
        Var arg0 = JavascriptNumber::BoxStackNumber(args[0], scriptContext);
#else
        Var arg0 = args[0];
#endif
        // B.2.2.1
        // get Object.prototype.__proto__ 
        // The value of the [[Get]] attribute is a built-in function that requires no arguments. It performs the following steps:
        // 1.   Let O be the this value.
        // 2.   If Type(O) is not Object, then throw a TypeError exception.                
        RecyclableObject* object;
        if (args.Info.Count < 1 || !JavascriptConversion::ToObject(arg0, scriptContext, &object)) // NOTE: ToObject for compat, in es-discuss
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Object.prototype.__proto__");
        }

        // 3.   Return the result of calling the [[GetInheritance]] internal method of O.
        return JavascriptObject::GetPrototypeOf(object, scriptContext);
    }

    Var ObjectPrototypeObject::Entry__proto__setter(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));
        ScriptContext* scriptContext = function->GetScriptContext();

        // Note the recent spec adjustment to conform to existing implementations:
        // The value of the [[Set]] attribute is a built-in function that takes an argument proto. It performs the following steps:
        //-1.Let O be the this value.
        //-2.If Type(O) is not Object, then throw a TypeError exception.
        //-3.If Type(proto) is neither Object or Null, then throw a TypeError exception
        //+1.Let O be CheckObjectCoercible(this value).
        //+2.ReturnIfAbrupt(O).
        //+3.If Type(proto) is neither Object or Null, then return proto.
        //+4.If Type(O) is not Object, then return proto.

#if !FLOATVAR
        // Mark temp number will stack allocate number that is used as the object ptr.
        // So we should box it before call ToObject on it.
        Var arg0 = JavascriptNumber::BoxStackNumber(args[0], scriptContext);
#else
        Var arg0 = args[0];
#endif

        RecyclableObject* object = nullptr;
        if (args.Info.Count < 1 || !JavascriptConversion::ToObject(arg0, scriptContext, &object)) // NOTE: ToObject for compat, in es-discuss
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedObject, L"Object.prototype.__proto__");
        }
        else if (args.Info.Count < 2)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
        else if (!JavascriptOperators::IsObjectOrNull(args[1]))
        {
            return args[1];
        }

        RecyclableObject* newPrototype = RecyclableObject::FromVar(args[1]);

        // 4.   Let status be the result of calling the [[SetInheritance]] internal method of O with argument proto.
        // 5.   ReturnIfAbrupt(status).
        // 6.   If status is false, then throw a TypeError exception.
        JavascriptObject::ChangePrototype(object, newPrototype, /*validate*/true, scriptContext);

        // 7.   Return proto.
        return newPrototype;
    }

    BOOL ObjectPrototypeObject::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        const BOOL result = __super::DeleteProperty(propertyId, flags);
        if (result && propertyId == PropertyIds::__proto__)
        {
            this->__proto__Enabled = false;
        }

        return result;
    }

    ObjectPrototypeObject* ObjectPrototypeObject::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        Recycler *recycler = scriptContext->GetRecycler();
        DynamicType *type = scriptContext->GetLibrary()->CreateObjectTypeNoCache(RecyclableObject::FromVar(scriptContext->CopyOnWrite(this->GetPrototype())), this->GetTypeId());
        auto cow = CopyOnWriteObject<ObjectPrototypeObject>::New(recycler, type, this, scriptContext);
        cow->__proto__Enabled = this->__proto__Enabled;
        return cow;
    }

    void ObjectPrototypeObject::PostDefineOwnProperty__proto__(RecyclableObject* obj)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        if (obj == this && scriptContext->GetConfig()->Is__proto__Enabled())
        {
            Var getter, setter;

            // __proto__Enabled is now only used by diagnostics to decide displaying __proto__ or [prototype].
            // We consider __proto__ is enabled when original getter and setter are both in place.
            this->__proto__Enabled =
                this->GetAccessors(PropertyIds::__proto__, &getter, &setter, scriptContext)
                && getter == scriptContext->GetLibrary()->Get__proto__getterFunction()
                && setter == scriptContext->GetLibrary()->Get__proto__setterFunction();
        }
    }
}
