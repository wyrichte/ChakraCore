//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline BOOL JavascriptOperators::GreaterEqual(Var aLeft, Var aRight, ScriptContext* scriptContext)
    {
        if (TaggedInt::Is(aLeft))
        {
            if (TaggedInt::Is(aRight))
            {
                return (int)aLeft >= (int)aRight;
            }
            if (JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return TaggedInt::ToDouble(aLeft) >= JavascriptNumber::GetValue(aRight);
            }
        }
        else if (TaggedInt::Is(aRight))
        {
            if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft))
            {
                return JavascriptNumber::GetValue(aLeft) >= TaggedInt::ToDouble(aRight);
            }
        }
        else
        {
            if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft) && JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return JavascriptNumber::GetValue(aLeft) >= JavascriptNumber::GetValue(aRight);
            }
        }

        return !RelationalComparsionHelper(aLeft, aRight, scriptContext, true, true);
    }

    inline BOOL JavascriptOperators::LessEqual(Var aLeft, Var aRight, ScriptContext* scriptContext)
    {
        if (TaggedInt::Is(aLeft))
        {
            if (TaggedInt::Is(aRight))
            {
                return (int)aLeft <= (int)aRight;
            }
            
            if (JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return TaggedInt::ToDouble(aLeft) <= JavascriptNumber::GetValue(aRight);
            }
        }
        else if (TaggedInt::Is(aRight))
        {
            if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft))
            {
                return JavascriptNumber::GetValue(aLeft) <= TaggedInt::ToDouble(aRight);
            }
        }
        else
        {
            if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft) && JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return JavascriptNumber::GetValue(aLeft) <= JavascriptNumber::GetValue(aRight);
            }
        }

        return !RelationalComparsionHelper(aRight, aLeft, scriptContext, false, true);
    }

    inline BOOL JavascriptOperators::NotEqual(Var aLeft, Var aRight, ScriptContext* scriptContext)
    {
        //
        // TODO: Change to use Abstract Equality Comparison Algorithm (ES3.0: S11.9.3):
        // - Evaluate left, then right, operands to preserve correct evaluation order.
        // - Call algorithm, potentially reversing arguments.
        //

        return !Equal(aLeft, aRight,scriptContext);
    }


    // NotStrictEqual() returns whether the two vars have strict equality, as
    // described in (ES3.0: S11.9.5, S11.9.6).

    inline BOOL JavascriptOperators::NotStrictEqual(Var aLeft, Var aRight, ScriptContext* scriptContext)
    {
        return !StrictEqual(aLeft, aRight,scriptContext);
    }


    inline bool JavascriptOperators::CheckIfObjectAndPrototypeChainHasOnlyWritableDataProperties(RecyclableObject* object)
    {
        Assert(object);
        if(object->GetType()->HasSpecialPrototype())
        {
            TypeId typeId = object->GetTypeId();
            if (typeId == TypeIds_Null)
            {
                return true;
            }
            if (typeId == TypeIds_Proxy)
            {
                return false;
            }
        }
        if(!object->HasOnlyWritableDataProperties())
        {
            return false;
        }
        return CheckIfPrototypeChainHasOnlyWritableDataProperties(object->GetPrototype());
    }

    inline bool JavascriptOperators::CheckIfPrototypeChainHasOnlyWritableDataProperties(RecyclableObject* prototype)
    {
        Assert(prototype);

        if(prototype->GetType()->AreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties())
        {
            Assert(DoCheckIfPrototypeChainHasOnlyWritableDataProperties(prototype));
            return true;
        }
        return DoCheckIfPrototypeChainHasOnlyWritableDataProperties(prototype);
    }

    // Does a quick check to see if the specified object (which should be a prototype object) and all objects in its prototype
    // chain have only writable data properties (i.e. no accessors or non-writable properties).
    inline bool JavascriptOperators::DoCheckIfPrototypeChainHasOnlyWritableDataProperties(RecyclableObject* prototype)
    {
        Assert(prototype);

        Type *const originalType = prototype->GetType();
        ScriptContext *const scriptContext = prototype->GetScriptContext();
        bool onlyOneScriptContext = true;
        TypeId typeId;
        for(; (typeId = prototype->GetTypeId()) != TypeIds_Null; prototype = prototype->GetPrototype())
        {
            if (typeId == TypeIds_Proxy)
            {
                return false;
            }
            if (!prototype->HasOnlyWritableDataProperties())
            {
                return false;
            }
            if(prototype->GetScriptContext() != scriptContext)
            {
                onlyOneScriptContext = false;
            }
        }

        if(onlyOneScriptContext)
        {
            // See JavascriptLibrary::typesEnsuredToHaveOnlyWritableDataPropertiesInItAndPrototypeChain for a description of
            // this cache. Technically, we could register all prototypes in the chain but this is good enough for now.
            originalType->SetAreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties(true);
        }
        
        return true;
    }


    __inline BOOL JavascriptOperators::Equal(Var aLeft, Var aRight, ScriptContext* scriptContext) 
    {
        if (aLeft == aRight)
        {
            if (TaggedInt::Is(aLeft) || JavascriptObject::Is(aLeft))
            {
                return true;
            }
            else
            {
                return Equal_Full(aLeft, aRight, scriptContext);
            }
        }

        if (JavascriptString::Is(aLeft) && JavascriptString::Is(aRight))
        {
            JavascriptString* left = (JavascriptString*)aLeft;
            JavascriptString* right = (JavascriptString*)aRight;

            if (left->GetLength() == right->GetLength())
            {
                if (left->UnsafeGetBuffer() != NULL && right->UnsafeGetBuffer() != NULL)
                {
                    if(left->GetLength() == 1)
                    {
                        return left->UnsafeGetBuffer()[0] == right->UnsafeGetBuffer()[0];
                    }
                    return memcmp(left->UnsafeGetBuffer(), right->UnsafeGetBuffer(), left->GetLength() * sizeof(left->UnsafeGetBuffer()[0])) == 0;
                }
                // fall through to Equal_Full
            }
            else
            {
                return false;
            }
        }

        return Equal_Full(aLeft, aRight, scriptContext);
    }

    __inline BOOL JavascriptOperators::Greater(Var aLeft, Var aRight, ScriptContext* scriptContext)
    {
        if (TaggedInt::Is(aLeft))
        {
            if (TaggedInt::Is(aRight))
            {
                return (int)aLeft > (int)aRight;
            }
            if (JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return TaggedInt::ToDouble(aLeft) > JavascriptNumber::GetValue(aRight);
            }
        }
        else if (TaggedInt::Is(aRight))
        {
            if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft))
            {
                return JavascriptNumber::GetValue(aLeft) > TaggedInt::ToDouble(aRight);
            }
        }
        else
        {
            if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft) && JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return JavascriptNumber::GetValue(aLeft) > JavascriptNumber::GetValue(aRight);
            }
        }

        return Greater_Full(aLeft, aRight,scriptContext);
    }


    __inline BOOL JavascriptOperators::Less(Var aLeft, Var aRight, ScriptContext* scriptContext)
    {
        if (TaggedInt::Is(aLeft))
        {
            if (TaggedInt::Is(aRight))
            {
                return (int)aLeft < (int)aRight;
            }
            if (JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return TaggedInt::ToDouble(aLeft) < JavascriptNumber::GetValue(aRight);
            }
        }
        else if (TaggedInt::Is(aRight))
        {
            if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft))
            {
                return JavascriptNumber::GetValue(aLeft) < TaggedInt::ToDouble(aRight);
            }
        }
        else
        {
            if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft) && JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return JavascriptNumber::GetValue(aLeft) < JavascriptNumber::GetValue(aRight);
            }
        }

        return Less_Full(aLeft, aRight, scriptContext);
    }

    __inline void * JavascriptOperators::Op_SwitchStringLookUp(JavascriptString* str, Js::BranchDictionaryWrapper<JavascriptString*>* branchTargets)
    {
        void* defaultTarget = branchTargets->defaultTarget;
        Js::BranchDictionaryWrapper<JavascriptString*>::BranchDictionary& stringDictionary = branchTargets->dictionary;
        return stringDictionary.Lookup(str, defaultTarget);
    }

    __inline Var JavascriptOperators::ToObject(Var aRight, ScriptContext* scriptContext)
    {
        RecyclableObject* object = null;
        if (FALSE == JavascriptConversion::ToObject(aRight, scriptContext, &object))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject /* TODO-ERROR: get arg name - aValue */);
        }

        return object;
    }

    __inline Var JavascriptOperators::ToWithObject(Var aRight, ScriptContext* scriptContext)
    {
        RecyclableObject* object = RecyclableObject::FromVar(aRight);

        WithScopeObject* withWrapper = RecyclerNew(scriptContext->GetRecycler(), WithScopeObject, object, scriptContext->GetLibrary()->GetWithType());
        return withWrapper;
    }

    __inline Var JavascriptOperators::ToNumber(Var aRight, ScriptContext* scriptContext)
    {
        if (TaggedInt::Is(aRight) || (JavascriptNumber::Is_NoTaggedIntCheck(aRight)))
        {
            return aRight;
        }

        return JavascriptNumber::ToVarNoCheck(JavascriptConversion::ToNumber_Full(aRight, scriptContext), scriptContext);
    }

    // ToDo: only force inline for some special code path
    __forceinline TypeId JavascriptOperators::GetTypeId(const Var aValue)
    {
        AssertMsg(aValue != null, "GetTypeId aValue is null");

        if (TaggedInt::Is(aValue))
        {
            return TypeIds_Integer;
        }
#if FLOATVAR
        else if (JavascriptNumber::Is_NoTaggedIntCheck(aValue))
        {
            return TypeIds_Number;
        }
#endif
        else
        {
            auto typeId = RecyclableObject::FromVar(aValue)->GetTypeId();
#if DBG
            auto isExternal = RecyclableObject::FromVar(aValue)->CanHaveInterceptors();
            AssertMsg(typeId < TypeIds_Limit || isExternal, "GetTypeId aValue has invalid TypeId");
#endif
            return typeId;
        }
    }

    __inline BOOL JavascriptOperators::IsObject(Var aValue)
    {
        return GetTypeId(aValue) > TypeIds_LastJavascriptPrimitiveType;       
    }

    __inline BOOL JavascriptOperators::IsObjectType(TypeId typeId)
    {
        return typeId > TypeIds_LastJavascriptPrimitiveType;
    }

    __inline BOOL JavascriptOperators::IsExposedType(TypeId typeId)
    {
        return typeId <= TypeIds_LastTrueJavascriptObjectType && typeId != TypeIds_HostDispatch;
    }

    __inline BOOL JavascriptOperators::IsObjectOrNull(Var instance)
    {
        TypeId typeId = GetTypeId(instance);
        return IsObjectType(typeId) || typeId == TypeIds_Null;
    }

    __inline BOOL JavascriptOperators::IsUndefinedOrNullType(TypeId typeId)
    {
        return typeId <= TypeIds_UndefinedOrNull;       
    }

    __inline BOOL JavascriptOperators::IsSpecialObjectType(TypeId typeId)
    {
        return typeId > TypeIds_LastTrueJavascriptObjectType;       
    }

    __inline BOOL JavascriptOperators::IsUndefinedObject(Var instance)
    {
        return JavascriptOperators::GetTypeId(instance) == TypeIds_Undefined;
    }

    __inline BOOL JavascriptOperators::IsUndefinedObject(Var instance, RecyclableObject *libraryUndefined)
    {
        Assert(JavascriptOperators::IsUndefinedObject(libraryUndefined));

        return instance == libraryUndefined || (BinaryFeatureControl::LanguageService() && instance && JavascriptOperators::IsUndefinedObject(instance));
    }
        
    __inline BOOL JavascriptOperators::IsUndefinedObject(Var instance, ScriptContext *scriptContext)
    {
        return JavascriptOperators::IsUndefinedObject(instance, scriptContext->GetLibrary()->GetUndefined());
    }
        
    __inline BOOL JavascriptOperators::IsUndefinedObject(Var instance, JavascriptLibrary* library)
    {
        return JavascriptOperators::IsUndefinedObject(instance, library->GetUndefined());
    }

    __inline BOOL JavascriptOperators::IsAnyNumberValue(Var instance)
    {
        TypeId typeId = GetTypeId(instance);
        return TypeIds_FirstNumberType <= typeId && typeId <= TypeIds_LastNumberType;
    }

    __inline BOOL JavascriptOperators::IsIterable(RecyclableObject* instance)
    {
        return JavascriptOperators::HasProperty(instance, PropertyIds::_symbolIterator);
    }

    // GetIterator as described in ES6.0 (draft 22) Section 7.4.1
    inline RecyclableObject* JavascriptOperators::GetIterator(Var iterable, ScriptContext* scriptContext)
    {
        RecyclableObject* iterableObj = RecyclableObject::FromVar(JavascriptOperators::ToObject(iterable, scriptContext));
        return JavascriptOperators::GetIterator(iterableObj, scriptContext);
    }

    inline RecyclableObject* JavascriptOperators::GetIteratorFunction(Var iterable, ScriptContext* scriptContext)
    {
        RecyclableObject* iterableObj = RecyclableObject::FromVar(JavascriptOperators::ToObject(iterable, scriptContext));
        return JavascriptOperators::GetIteratorFunction(iterableObj, scriptContext);
    }

    inline RecyclableObject* JavascriptOperators::GetIteratorFunction(RecyclableObject* instance, ScriptContext * scriptContext)
    {
        Var func = JavascriptOperators::GetProperty(instance, PropertyIds::_symbolIterator, scriptContext);

        if (!JavascriptConversion::IsCallable(func))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
        }

        RecyclableObject* function = RecyclableObject::FromVar(func);
        return function;
    }

    inline RecyclableObject* JavascriptOperators::GetIterator(RecyclableObject* instance, ScriptContext * scriptContext)
    {
        RecyclableObject* function = GetIteratorFunction(instance, scriptContext);
        Var iterator = function->GetEntryPoint()(function, CallInfo(Js::CallFlags_Value, 1), instance);

        if (!JavascriptOperators::IsObject(iterator))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
        }

        return RecyclableObject::FromVar(iterator);
    }

    // IteratorNext as described in ES6.0 (draft 22) Section 7.4.2
    inline RecyclableObject* JavascriptOperators::IteratorNext(RecyclableObject* iterator, ScriptContext* scriptContext, Var value)
    {
        Var func = JavascriptOperators::GetProperty(iterator, PropertyIds::next, scriptContext);

        if (!JavascriptConversion::IsCallable(func))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
        }

        RecyclableObject* callable = RecyclableObject::FromVar(func);
        Js::Var args[] = { iterator, value };
        Js::CallInfo callInfo(Js::CallFlags_Value, _countof(args) + (value == nullptr ? - 1 : 0));
        Var result = JavascriptFunction::CallFunction<true>(callable, callable->GetEntryPoint(), Js::Arguments(callInfo, args));

        if (!JavascriptOperators::IsObject(result))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
        }

        return RecyclableObject::FromVar(result);
    }

    // IteratorComplete as described in ES6.0 (draft 22) Section 7.4.3
    inline bool JavascriptOperators::IteratorComplete(RecyclableObject* iterResult, ScriptContext* scriptContext)
    {
        Var done = JavascriptOperators::GetProperty(iterResult, Js::PropertyIds::done, scriptContext);

        return JavascriptConversion::ToBool(done, scriptContext);
    }

    // IteratorValue as described in ES6.0 (draft 22) Section 7.4.4
    inline Var JavascriptOperators::IteratorValue(RecyclableObject* iterResult, ScriptContext* scriptContext)
    {
        return JavascriptOperators::GetProperty(iterResult, Js::PropertyIds::value, scriptContext);
    }

    // IteratorStep as described in ES6.0 (draft 22) Section 7.4.5
    inline bool JavascriptOperators::IteratorStep(RecyclableObject* iterator, ScriptContext* scriptContext, RecyclableObject** result)
    {
        Assert(result);

        *result = JavascriptOperators::IteratorNext(iterator, scriptContext);
        return !JavascriptOperators::IteratorComplete(*result, scriptContext);
    }

    inline bool JavascriptOperators::IteratorStepAndValue(RecyclableObject* iterator, ScriptContext* scriptContext, Var* resultValue)
    {
        RecyclableObject* result = JavascriptOperators::IteratorNext(iterator, scriptContext);

        if (!JavascriptOperators::IteratorComplete(result, scriptContext))
        {
            *resultValue = JavascriptOperators::IteratorValue(result, scriptContext);
            return true;
        }

        return false;
    }

    inline RecyclableObject* JavascriptOperators::OrdinaryCreateFromConstructor(RecyclableObject* constructor, RecyclableObject* obj, DynamicObject* intrinsicProto, ScriptContext* scriptContext)
    {
        // There isn't a good way for us to add internal properties to objects in Chakra.
        // Thus, caller should take care to create obj with the correct internal properties.

        Var proto = JavascriptOperators::GetProperty(constructor, Js::PropertyIds::prototype, scriptContext);

        // If constructor.prototype is an object, we should use that as the [[Prototype]] for our obj.
        // Else, we set the [[Prototype]] internal slot of obj to %intrinsicProto% - which should be the default.
        if (JavascriptOperators::IsObjectType(JavascriptOperators::GetTypeId(proto)) &&
            DynamicObject::FromVar(proto) != intrinsicProto)
        {
            JavascriptObject::ChangePrototype(obj, RecyclableObject::FromVar(proto), /*validate*/true, scriptContext);
        }

        return obj;
    }



    inline Var JavascriptOperators::GetProperty(RecyclableObject* instance, PropertyId propertyId, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        return JavascriptOperators::GetProperty(instance, instance, propertyId, requestContext, info);
    }

    inline BOOL JavascriptOperators::GetProperty(RecyclableObject* instance, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        return JavascriptOperators::GetProperty(instance, instance, propertyId, value, requestContext, info);
    }

    inline Var JavascriptOperators::GetProperty(Var instance, RecyclableObject* propertyObject, PropertyId propertyId, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        Var value;
        if (JavascriptOperators::GetProperty(instance, propertyObject, propertyId, &value, requestContext, info))
        {
            return value;
        }
        return requestContext->GetMissingPropertyResult(propertyObject, propertyId);
    }

    inline Var JavascriptOperators::GetRootProperty(RecyclableObject* instance, PropertyId propertyId, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        Var value;
        if (JavascriptOperators::GetRootProperty(instance, propertyId, &value, requestContext, info))
        {
            return value;
        }
        return requestContext->GetMissingPropertyResult(instance, propertyId);
    }

    inline BOOL JavascriptOperators::GetPropertyReference(RecyclableObject *instance, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        return JavascriptOperators::GetPropertyReference(instance, instance, propertyId, value, requestContext, info);
    }

    inline BOOL JavascriptOperators::GetItem(RecyclableObject* instance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return JavascriptOperators::GetItem(instance, instance, index, value, requestContext);
    }

    inline BOOL JavascriptOperators::GetItemReference(RecyclableObject* instance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return GetItemReference(instance, instance, index, value, requestContext);
    }
  
    inline BOOL JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(RecyclableObject* instance, PropertyId propertyId, Var* setterValue, DescriptorFlags* flags, PropertyValueInfo* info, ScriptContext* scriptContext)
    {
        if (propertyId == Js::PropertyIds::__proto__)
        {
            return CheckPrototypesForAccessorOrNonWritablePropertyCore<PropertyId, false, false>(instance, propertyId, setterValue, flags, info, scriptContext);
        }
        else
        {
            return CheckPrototypesForAccessorOrNonWritablePropertyCore<PropertyId, true, false>(instance, propertyId, setterValue, flags, info, scriptContext);
        }
    }

    inline BOOL JavascriptOperators::CheckPrototypesForAccessorOrNonWritableRootProperty(RecyclableObject* instance, PropertyId propertyId, Var* setterValue, DescriptorFlags* flags, PropertyValueInfo* info, ScriptContext* scriptContext)
    {
        if (propertyId == Js::PropertyIds::__proto__)
        {
            return CheckPrototypesForAccessorOrNonWritablePropertyCore<PropertyId, false, true>(instance, propertyId, setterValue, flags, info, scriptContext);
        }
        else
        {
            return CheckPrototypesForAccessorOrNonWritablePropertyCore<PropertyId, true, true>(instance, propertyId, setterValue, flags, info, scriptContext);
        }
    }

    inline BOOL JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(RecyclableObject* instance, JavascriptString* propertyNameString, Var* setterValue, DescriptorFlags* flags, PropertyValueInfo* info, ScriptContext* scriptContext)
    {
        JsUtil::CharacterBuffer<WCHAR> propertyName(propertyNameString->GetString(), propertyNameString->GetLength());
        if (Js::BuiltInPropertyRecords::__proto__.Equals(propertyName))
        {
            return CheckPrototypesForAccessorOrNonWritablePropertyCore<JavascriptString*, false, false>(instance, propertyNameString, setterValue, flags, info, scriptContext);
        }
        else
        {
            return CheckPrototypesForAccessorOrNonWritablePropertyCore<JavascriptString*, true, false>(instance, propertyNameString, setterValue, flags, info, scriptContext);
        }
    }

    template<typename PropertyKeyType>
    BOOL JavascriptOperators::CheckPrototypesForAccessorOrNonWritablePropertySlow(RecyclableObject* instance, PropertyKeyType propertyKey, Var* setterValue, DescriptorFlags* flags, bool isRoot, ScriptContext* scriptContext)
    {
        // This is used in debug verification, do not doFastProtoChainCheck to avoid side effect (doFastProtoChainCheck may update HasWritableDataOnly flags).
        if (isRoot)
        {
            return CheckPrototypesForAccessorOrNonWritablePropertyCore<PropertyKeyType, /*doFastProtoChainCheck*/false, true>(instance, propertyKey, setterValue, flags, nullptr, scriptContext);
        }
        else
        {
            return CheckPrototypesForAccessorOrNonWritablePropertyCore<PropertyKeyType, /*doFastProtoChainCheck*/false, false>(instance, propertyKey, setterValue, flags, nullptr, scriptContext);
        }
    }

    inline BOOL JavascriptOperators::SetProperty(Var instance, RecyclableObject* object, PropertyId propertyId, Var newValue, ScriptContext* requestContext, PropertyOperationFlags propertyOperationFlags)
    {
        PropertyValueInfo info;
        return JavascriptOperators::SetProperty(instance, object, propertyId, newValue, &info, requestContext, propertyOperationFlags);
    }

    inline BOOL JavascriptOperators::TryConvertToUInt32(const wchar_t* str, int length, uint32* intVal)
    {
        return NumberUtilities::TryConvertToUInt32(str, length, intVal);
    }

    template <typename TPropertyKey>
    inline DescriptorFlags JavascriptOperators::GetRootSetter(RecyclableObject* instance, TPropertyKey propertyKey, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        // This is provided only so that CheckPrototypesForAccessorOrNonWritablePropertyCore will compile.
        // It will never be called.
        Throw::FatalInternalError();
    }

    template <>
    inline DescriptorFlags JavascriptOperators::GetRootSetter(RecyclableObject* instance, PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        AssertMsg(JavascriptOperators::GetTypeId(instance) == TypeIds_GlobalObject
            || JavascriptOperators::GetTypeId(instance) == TypeIds_ModuleRoot,
            "Root must be a global object!");

        RootObjectBase* rootObject = static_cast<RootObjectBase*>(instance);
        return rootObject->GetRootSetter(propertyId, setterValue, info, requestContext);
    }

    template <BOOL stopAtProxy, class Func>
    void JavascriptOperators::MapObjectAndPrototypes(RecyclableObject* object, Func func)
    {
        MapObjectAndPrototypesUntil<stopAtProxy>(object, [=](RecyclableObject* obj)
        {
            func(obj);
            return false; // this will map whole prototype chain
        });
    }

    template <BOOL stopAtProxy, class Func>
    bool JavascriptOperators::MapObjectAndPrototypesUntil(RecyclableObject* object, Func func)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(object);
        while (typeId != TypeIds_Null && (!stopAtProxy || typeId != TypeIds_Proxy))
        {
            if (func(object))
            {
                return true;
            }

            object = object->GetPrototype();
            typeId = JavascriptOperators::GetTypeId(object);
        }

        return false;
    }
}
