//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace Js
{   
    DEFINE_RECYCLER_TRACKER_WEAKREF_PERF_COUNTER(Type);

    InternalString Type::ObjectTypeNameString    = InternalString(L"object", 6);
    InternalString Type::UndefinedTypeNameString = InternalString(L"undefined", 9);
    InternalString Type::BooleanTypeNameString   = InternalString(L"boolean", 7);
    InternalString Type::StringTypeNameString    = InternalString(L"string", 6);
    InternalString Type::NumberTypeNameString    = InternalString(L"number", 6);
    InternalString Type::FunctionTypeNameString  = InternalString(L"function", 8);

    Type::Type(ScriptContext* scriptContext, TypeId typeId, RecyclableObject* prototype, JavascriptMethod entryPoint) :
        javascriptLibrary(scriptContext->GetLibrary()),
        typeId(typeId),
        prototype(prototype),
        propertyCache(null),
        flags(TypeFlagMask_None)
    {        
#ifdef PROFILE_TYPES
        if (typeId < sizeof(scriptContext->typeCount)/sizeof(int))
        {
            scriptContext->typeCount[typeId]++;
        }
#endif
        this->entryPoint = entryPoint != null ? entryPoint : RecyclableObject::DefaultEntryPoint;
        if (prototype)
        {
            Assert(! CrossSite::NeedMarshalVar(prototype,scriptContext));
            prototype->SetIsPrototype();
        }
    }

    Type::Type(Type * type) :
        typeId(type->typeId),
        javascriptLibrary(type->javascriptLibrary),
        prototype(type->prototype),
        entryPoint(type->entryPoint),
        flags(type->flags),
        propertyCache(null)
    {
#ifdef PROFILE_TYPES
        if (typeId < sizeof(javascriptLibrary->GetScriptContext()->typeCount)/sizeof(int))
        {
            javascriptLibrary->GetScriptContext()->typeCount[typeId]++;
        }
#endif
        Assert(! (prototype && CrossSite::NeedMarshalVar(prototype, javascriptLibrary->GetScriptContext())));

        // If the type property cache is copied over to this new type, then if a property ID caused the type to be changed for
        // the purpose of invalidating caches due to the property being deleted or its attributes being changed, then the cache
        // for that property ID must be cleared on this new type after the type property cache is copied. Also, types are not
        // changed consistently to use this copy constructor, so those would need to be fixed as well.

        if(type->AreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties())
        {
            SetAreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties(true);
        }
        if(type->IsFalsy())
        {
            SetIsFalsy(true);
        }
    }

    ScriptContext *
    Type::GetScriptContext() const
    {
        return GetLibrary()->GetScriptContext();
    }

    Recycler *
    Type::GetRecycler() const
    {
        return GetLibrary()->GetRecycler();
    }

    TypePropertyCache *Type::GetPropertyCache()
    {
        return propertyCache;
    }

    TypePropertyCache *Type::CreatePropertyCache()
    {
        Assert(!propertyCache);

        propertyCache = RecyclerNew(GetRecycler(), TypePropertyCache);
        return propertyCache;
    }

    void Type::SetAreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties(const bool truth)
    {
        if (truth)
        {
            if (GetScriptContext()->IsClosed())
            {
                // The cache is disabled after the script context is closed, to avoid issues between being closed and being deleted,
                // where the cache of these types in JavascriptLibrary may be reclaimed at any point
                return;
            }

            flags |= TypeFlagMask_AreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties;
            javascriptLibrary->TypeAndPrototypesAreEnsuredToHaveOnlyWritableDataProperties(this);
        }
        else
        {
            flags &= ~TypeFlagMask_AreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties;
        }
    }

    BOOL Type::AreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties() const 
    { 
        return flags & TypeFlagMask_AreThisAndPrototypesEnsuredToHaveOnlyWritableDataProperties;
    }

    BOOL Type::IsFalsy() const
    {
        return flags & TypeFlagMask_IsFalsy; 
    }

    void Type::SetIsFalsy(const bool truth)
    {
        if (truth)
        {
            Assert(this->GetScriptContext()->GetThreadContext()->CanBeFalsy(this->GetTypeId()));
            flags |= TypeFlagMask_IsFalsy;
        }
        else
        {
            flags &= ~TypeFlagMask_IsFalsy;
        }
    }

    void Type::SetHasSpecialPrototype(const bool truth)
    {
        if (truth)
        {
            flags |= TypeFlagMask_HasSpecialPrototype;
        }
        else
        {
            flags &= ~TypeFlagMask_HasSpecialPrototype;
        }
    }

    uint32 Type::GetOffsetOfTypeId()
    {
        return offsetof(Type, typeId);
    }

    uint32 Type::GetOffsetOfFlags()
    {
        return offsetof(Type, flags);
    }

    uint32 Type::GetOffsetOfEntryPoint()
    {
        return offsetof(Type, entryPoint);
    }
    
    uint32 Type::GetOffsetOfPrototype()
    {
        return offsetof(Type, prototype);
    }

    // In order to avoid a branch, every object has an entry point if it gets called like a 
    // function - however, if it can't be called like a function, it's set to DefaultEntryPoint
    // which will emit an error.  
    Var RecyclableObject::DefaultEntryPoint(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ARGUMENTS(args, callInfo);        
        TypeId typeId = function->GetTypeId();
        rtErrors err = typeId == TypeIds_Undefined || typeId == TypeIds_Null ? JSERR_NeedObject : JSERR_NeedFunction;
        JavascriptError::ThrowTypeError(function->GetScriptContext(), err
            /* TODO-ERROR: args.Info.Count > 0? args[0] : null); */);
    }

    BOOL RecyclableObject::HasProperty(PropertyId propertyId)
    {
        return false;
    }

    BOOL RecyclableObject::HasOwnProperty(PropertyId propertyId)
    {
        return false;
    }

    BOOL RecyclableObject::HasOwnPropertyNoHostObject(PropertyId propertyId)
    {
        return HasOwnProperty(propertyId);
    }

    BOOL RecyclableObject::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return false;
    }

    BOOL RecyclableObject::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return false;
    }

    BOOL RecyclableObject::GetInternalProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return false;
    }

    BOOL RecyclableObject::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return false;
    }

    BOOL RecyclableObject::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return false;
    }

    BOOL RecyclableObject::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return false;
    }

    BOOL RecyclableObject::SetInternalProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return false;
    }

    BOOL RecyclableObject::InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return false;
    }

    BOOL RecyclableObject::InitPropertyScoped(PropertyId propertyId, Var value)
    {
        return false;
    }

    BOOL RecyclableObject::InitFuncScoped(PropertyId propertyId, Var value)
    {
        return false;
    }

    BOOL RecyclableObject::EnsureProperty(PropertyId propertyId)
    {
        return false;
    }

    BOOL RecyclableObject::EnsureNoRedeclProperty(PropertyId propertyId)
    {
        return false;
    }

    BOOL RecyclableObject::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        return true;
    }

    BOOL RecyclableObject::IsFixedProperty(PropertyId propertyId)
    {
        return false;
    }

    BOOL RecyclableObject::HasItem(uint32 index)
    {
        return false;
    }

    BOOL RecyclableObject::HasOwnItem(uint32 index)
    {
        return false;
    }

    BOOL RecyclableObject::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext)
    {
        return false;
    }

    BOOL RecyclableObject::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext)
    {
        return false;
    }

    BOOL RecyclableObject::SetItem(uint32 index, Var value, PropertyOperationFlags flags)
    {
        return false;
    }

    BOOL RecyclableObject::DeleteItem(uint32 index, PropertyOperationFlags flags)
    {
        return true;
    }

    BOOL RecyclableObject::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics, bool enumSymbols)
    {
        return false;        
    }

    BOOL RecyclableObject::ToPrimitive(JavascriptHint hint, Var* value, ScriptContext * scriptContext)
    {
        *value = NULL;
        return false;
    }

    BOOL RecyclableObject::SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        return false;
    }

    BOOL RecyclableObject::GetAccessors(PropertyId propertyId, Var* getter, Var* setter, ScriptContext * requestContext)
    {
        return false;
    }
    
    BOOL RecyclableObject::StrictEquals(Var aRight, BOOL* value, ScriptContext * requestContext)
    {
        //StrictEquals is handled in JavascriptOperators::StrictEqual
        Throw::InternalError();
    }

#pragma fenv_access (on)
    BOOL RecyclableObject::Equals(Var aRight, BOOL* value, ScriptContext * requestContext)
    {
        Var aLeft = this;
        if (aLeft == aRight) 
        {
            //In ES5 mode strict equals (===) on same instance of object type VariantDate succeeds.
            //Hence equals needs to succeed.
            goto ReturnTrue;            
        }

        double dblLeft, dblRight;
        TypeId leftType = this->GetTypeId();
        TypeId rightType = JavascriptOperators::GetTypeId(aRight);
        int redoCount = 0;

Redo:
        if (redoCount == 2)
        {
            goto ReturnFalse;
        }

        switch (leftType)
        {
        case TypeIds_Undefined:
        case TypeIds_Null:
            switch (rightType)
            {
            case TypeIds_Integer:
            case TypeIds_Number:
            case TypeIds_Symbol:
                goto ReturnFalse;
            case TypeIds_Undefined:
            case TypeIds_Null:
                goto ReturnTrue;
            default:
                // Falsy objects are == null and == undefined.
                *value = RecyclableObject::FromVar(aRight)->GetType()->IsFalsy();
                return TRUE;
            }
        case TypeIds_Integer:
            switch (rightType)
            {
            case TypeIds_Undefined:
            case TypeIds_Null:
            case TypeIds_Symbol:
                goto ReturnFalse;
            case TypeIds_Integer:
                // We already did a check to see if aLeft == aRight above, but we need to check again in case there was a redo.
                *value = aLeft == aRight;
                return TRUE;
            case TypeIds_Int64Number:
                {
                    int leftValue = TaggedInt::ToInt32(aLeft);
                    __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    *value = leftValue == rightValue;
                    Assert(!(*value));  // currently it cannot be true. more for future extension if we allow arithmatic calculation
                    return TRUE;
                }
            case TypeIds_UInt64Number:
                {
                    __int64 leftValue = TaggedInt::ToInt32(aLeft);
                    unsigned __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    // TODO: yongqu to review whether we need to check for neg value
                    *value = (/*leftValue >= 0 && */(unsigned __int64)leftValue == rightValue);
                    Assert(!(*value));  // currently it cannot be true. more for future extension if we allow arithmatic calculation
                    return TRUE;
                }
            case TypeIds_Number:
                dblLeft = TaggedInt::ToDouble(aLeft);
                dblRight = JavascriptNumber::GetValue(aRight);
                goto CompareDoubles;
            case TypeIds_Boolean:
            case TypeIds_String:
                dblLeft = TaggedInt::ToDouble(aLeft);
                dblRight = JavascriptConversion::ToNumber(aRight, requestContext);
                goto CompareDoubles;
            default:
                goto RedoRight;
            }
            break;
        case TypeIds_Int64Number:
            switch (rightType)
            {
            case TypeIds_Integer:
                {
                    __int64 leftValue = JavascriptInt64Number::FromVar(aLeft)->GetValue();
                    int rightValue = TaggedInt::ToInt32(aRight);
                    *value = leftValue == rightValue;
                    Assert(!(*value));  // currently it cannot be true. more for future extension if we allow arithmatic calculation
                    return TRUE;
                }
            case TypeIds_Number:
                    dblLeft     = (double)JavascriptInt64Number::FromVar(aLeft)->GetValue();
                    dblRight    = JavascriptNumber::GetValue(aRight);
                    goto CompareDoubles;
            case TypeIds_Int64Number:
                {
                    __int64 leftValue = JavascriptInt64Number::FromVar(aLeft)->GetValue();
                    __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    *value = leftValue == rightValue;
                    return TRUE;
                }
            case TypeIds_UInt64Number:
                {
                    __int64 leftValue = JavascriptInt64Number::FromVar(aLeft)->GetValue();
                    unsigned __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    // TODO: yongqu to review whether we need to check for neg value
                    *value = (/* leftValue >= 0 && */(unsigned __int64)leftValue == rightValue);
                    return TRUE;
                }
            }
            break;
        case TypeIds_UInt64Number:
            switch (rightType)
            {
            case TypeIds_Integer:
                {
                    unsigned __int64 leftValue = JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                    __int64 rightValue = TaggedInt::ToInt32(aRight);
                    // TODO: yongqu to review whether we need to check for neg value
                    *value = rightValue >= 0 && leftValue == (unsigned __int64)rightValue;
                    Assert(!(*value));  // currently it cannot be true. more for future extension if we allow arithmatic calculation
                    return TRUE;
                }
            case TypeIds_Number:
                dblLeft     = (double)JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                dblRight    = JavascriptNumber::GetValue(aRight);
                goto CompareDoubles;
            case TypeIds_Int64Number:
                {
                    unsigned __int64 leftValue = JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                    __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    // TODO: yongqu to review whether we need to check for neg value
                    *value = (/* rightValue >= 0 && */leftValue == (unsigned __int64)rightValue);
                    return TRUE;
                }
            case TypeIds_UInt64Number:
                {
                    unsigned __int64 leftValue = JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                    unsigned __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    *value = leftValue == rightValue;
                    return TRUE;
                }
            }
            break;
        case TypeIds_Number:
            switch (rightType)
            {
            case TypeIds_Undefined:
            case TypeIds_Null:
            case TypeIds_Symbol:
                goto ReturnFalse;
            case TypeIds_Integer:
                dblLeft = JavascriptNumber::GetValue(aLeft);
                dblRight    = TaggedInt::ToDouble(aRight);
                goto CompareDoubles;
            case TypeIds_Number:
                dblLeft = JavascriptNumber::GetValue(aLeft);
                dblRight = JavascriptNumber::GetValue(aRight);
                goto CompareDoubles;
            case TypeIds_Boolean:
            case TypeIds_String:
                dblLeft = JavascriptNumber::GetValue(aLeft);
                dblRight = JavascriptConversion::ToNumber(aRight, requestContext);
                goto CompareDoubles;
            default:
                goto RedoRight;
            }
            break;
        case TypeIds_String:
            switch (rightType)
            {
            case TypeIds_Undefined:
            case TypeIds_Null:
            case TypeIds_Symbol:
                goto ReturnFalse;
            case TypeIds_String:
                goto CompareStrings;
            case TypeIds_Number:
            case TypeIds_Integer:
            case TypeIds_Boolean:
                dblLeft = JavascriptConversion::ToNumber(aLeft, requestContext);
                dblRight = JavascriptConversion::ToNumber(aRight, requestContext);
                goto CompareDoubles;
            default:
                goto RedoRight;
            }
        case TypeIds_Boolean:
            switch (rightType)
            {
            case TypeIds_Undefined:
            case TypeIds_Null:
            case TypeIds_Symbol:
                goto ReturnFalse;
            case TypeIds_Boolean:
                *value = JavascriptBoolean::FromVar(aLeft)->GetValue() == JavascriptBoolean::FromVar(aRight)->GetValue();
                return TRUE;
            case TypeIds_Number:
            case TypeIds_Integer:
            case TypeIds_String:
                dblLeft = JavascriptConversion::ToNumber(aLeft, requestContext);
                dblRight = JavascriptConversion::ToNumber(aRight, requestContext);
                goto CompareDoubles;
            default:
                goto RedoRight;
            }
            break;

        case TypeIds_Symbol:
            switch (rightType)
            {
            case TypeIds_Undefined:
            case TypeIds_Null:
            case TypeIds_Number:
            case TypeIds_Integer:
            case TypeIds_String:
            case TypeIds_Boolean:
                goto ReturnFalse;
            case TypeIds_Symbol:
                *value = JavascriptSymbol::FromVar(aLeft)->GetValue() == JavascriptSymbol::FromVar(aRight)->GetValue();
                return TRUE;
            case TypeIds_SymbolObject:
                *value = JavascriptSymbol::FromVar(aLeft)->GetValue() == JavascriptSymbolObject::FromVar(aRight)->GetValue();
                return TRUE;
            default:
                goto RedoRight;
            }
            break;

        case TypeIds_Function:
            if (rightType == TypeIds_Function)
            {
                // In ES5 in certain cases (ES5 10.6.14(strict), 13.2.19(strict), 15.3.4.5.20-21) we return a function that throws type error.
                // For different scenarios we return different instances of the function, which differ by exception/error message.
                // According to ES5, this is the same [[ThrowTypeError]] (thrower) internal function, thus they should be equal.
                if (JavascriptFunction::IsThrowTypeErrorFunction(JavascriptFunction::FromVar(aLeft), requestContext) && 
                    JavascriptFunction::IsThrowTypeErrorFunction(JavascriptFunction::FromVar(aRight), requestContext))
                {
                    goto ReturnTrue;
                }
                goto ReturnFalse;
            }
            // Fall through to do normal object comparison on function object.
        default:
            switch (rightType)
            {
            case TypeIds_Undefined:
            case TypeIds_Null:
                // Falsy objects are == null and == undefined.
                *value = this->type->IsFalsy();
                return TRUE;
            case TypeIds_Boolean:
            case TypeIds_Integer:
            case TypeIds_Number:
            case TypeIds_String:
            case TypeIds_Symbol:
                goto RedoLeft;
            default:
                goto ReturnFalse;
            }
        }

RedoLeft:
        aLeft = JavascriptConversion::ToPrimitive(aLeft, JavascriptHint::None, requestContext);
        leftType = JavascriptOperators::GetTypeId(aLeft);
        redoCount++;
        goto Redo;
RedoRight:
        aRight = JavascriptConversion::ToPrimitive(aRight, JavascriptHint::None, requestContext);
        rightType = JavascriptOperators::GetTypeId(aRight);
        redoCount++;
        goto Redo;
CompareStrings:
        *value = JavascriptString::Equals(aLeft, aRight);
        return TRUE;
CompareDoubles:
        *value = dblLeft == dblRight;
        return TRUE;
ReturnFalse:
        *value = FALSE;
        return TRUE;
ReturnTrue:
        *value = TRUE;
        return TRUE;
    }
    
    RecyclableObject* RecyclableObject::ToObject(ScriptContext * requestContext)
    {
        AssertMsg(JavascriptOperators::IsObject(this), "bad type object in conversion ToObject");
        Assert(!CrossSite::NeedMarshalVar(this, requestContext));
        return this;
    }

    Var RecyclableObject::GetTypeOfString(ScriptContext * requestContext)
    {
        return requestContext->GetLibrary()->GetUnknownDisplayString();
    }

    Var RecyclableObject::InvokePut(Arguments args)
    {
        // Handle x(y) = z.
        // Native jscript object behavior: throw an error in all such cases.
        JavascriptError::ThrowReferenceError(GetScriptContext(), JSERR_CantAsgCall);
        return null;
    }

    BOOL RecyclableObject::GetRemoteTypeId(TypeId * typeId)
    {
        return FALSE;
    }

    DynamicObject* RecyclableObject::GetRemoteObject()
    {
        return NULL;
    }

    Var RecyclableObject::GetHostDispatchVar()
    {
        Assert(FALSE);
        return this->GetLibrary()->GetUndefined();
    }

    JavascriptString* RecyclableObject::GetClassName(ScriptContext * requestContext)
    {
        // we don't need this when not handling fastDOM.
        Assert(0);
        return NULL;
    }

    void RecyclableObject::BindEvent(Var eventHandler, PropertyId propertyId)
    {
        JavascriptError::ThrowTypeError(GetScriptContext(), JSERR_NeedObject /* TODO-ERROR: get arg name - instance */);
    }

    BOOL RecyclableObject::HasInstance(Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache)
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_Operand_Invalid_NeedFunction, L"instanceof" /* TODO-ERROR: get arg name - aClass */);
    }

#if defined(PROFILE_RECYCLER_ALLOC) && defined(RECYCLER_DUMP_OBJECT_GRAPH)    
    bool Type::DumpObjectFunction(type_info const * typeinfo, bool isArray, void * objectAddress)
    {
        if (isArray)
        {
            // Don't deal with array
            return false;
        }

        Output::Print(L"%S{%x} %p", typeinfo->name(), ((Type *)objectAddress)->GetTypeId(), objectAddress);
        return true;
    }
#endif
}
