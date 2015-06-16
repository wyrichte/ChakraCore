//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

unsigned long __stdcall GetUserLocale()
{
    return GetDefaultLocale();
}

namespace Js
{
    DEFINE_RECYCLER_TRACKER_ARRAY_PERF_COUNTER(Var);
    DEFINE_RECYCLER_TRACKER_PERF_COUNTER(FrameDisplay);

    enum IndexType
    {
        IndexType_Number,
        IndexType_PropertyId,
        IndexType_JavascriptString
    };

    IndexType GetIndexTypeFromString(wchar_t const * propertyName, charcount_t propertyLength, ScriptContext* scriptContext, uint32* index, PropertyRecord const** propertyRecord, bool createIfNotFound)
    {
        if (JavascriptOperators::TryConvertToUInt32(propertyName, propertyLength, index) &&
            (*index != JavascriptArray::InvalidIndex))
        {
            return IndexType_Number;
        }
        else
        {
            if (createIfNotFound)
            {
                scriptContext->GetOrAddPropertyRecord(propertyName, propertyLength, propertyRecord);
            }
            else
            {
                scriptContext->FindPropertyRecord(propertyName, propertyLength, propertyRecord);
            }
            return IndexType_PropertyId;
        }
    }

    IndexType GetIndexType(Var indexVar, ScriptContext* scriptContext, uint32* index, PropertyRecord const ** propertyRecord, JavascriptString ** propertyNameString, bool createIfNotFound, bool preferJavascriptStringOverPropertyRecord)
    {
        // TODO: At time of this comment, only OP_SetElementI and OP_GetElementI use and take advantage of the
        // IndexType_JavascriptString result. Consider modifying other callers of GetIndexType to take
        // advantage of non-interned property strings where appropriate.
        if (TaggedInt::Is(indexVar))
        {
            int indexInt = TaggedInt::ToInt32(indexVar);
            if (indexInt >= 0)
            {
                *index = (uint)indexInt;
                return IndexType_Number;
            }
            else
            {
                wchar_t buffer[20];
                ::_itow_s(indexInt, buffer, sizeof(buffer)/sizeof(wchar_t), 10);
                charcount_t length = JavascriptString::GetBufferLength(buffer);
                if (createIfNotFound || preferJavascriptStringOverPropertyRecord)
                {
                    // When preferring JavascriptString objects, just return a PropertyRecord instead
                    // of creating temporary JavascriptString objects for every negative integer that
                    // comes through here.
                    scriptContext->GetOrAddPropertyRecord(buffer, length, propertyRecord);
                }
                else
                {
                    scriptContext->FindPropertyRecord(buffer, length, propertyRecord);
                }
                return IndexType_PropertyId;
            }
        }
        else if (JavascriptSymbol::Is(indexVar))
        {
            JavascriptSymbol* symbol = JavascriptSymbol::FromVar(indexVar);

            // JavascriptSymbols cannot add a new PropertyRecord - they correspond to one and only one existing PropertyRecord.
            // We already know what the PropertyRecord is since it is stored in the JavascriptSymbol itself so just return it.

            *propertyRecord = symbol->GetValue();

            return IndexType_PropertyId;
        }
        else if (JavascriptSymbolObject::Is(indexVar))
        {
            JavascriptSymbolObject* symbolObject = JavascriptSymbolObject::FromVar(indexVar);

            *propertyRecord = symbolObject->GetValue();

            return IndexType_PropertyId;
        }
        else
        {
            JavascriptString* indexStr = JavascriptConversion::ToString(indexVar, scriptContext);
            wchar_t const * propertyName = indexStr->GetString();
            charcount_t const propertyLength = indexStr->GetLength();

            if (!createIfNotFound && preferJavascriptStringOverPropertyRecord)
            {
                if (JavascriptOperators::TryConvertToUInt32(propertyName, propertyLength, index) &&
                    (*index != JavascriptArray::InvalidIndex))
                {
                    return IndexType_Number;
                }

                *propertyNameString = indexStr;
                return IndexType_JavascriptString;
            }
            return GetIndexTypeFromString(propertyName, propertyLength, scriptContext, index, propertyRecord, createIfNotFound);
        }
    }

    IndexType GetIndexType(Var indexVar, ScriptContext* scriptContext, uint32* index, PropertyRecord const ** propertyRecord, bool createIfNotFound)
    {
        return GetIndexType(indexVar, scriptContext, index, propertyRecord, null, createIfNotFound, false);
    }

    BOOL FEqualDbl(double dbl1, double dbl2)
    {
        // If the low ulongs don't match, they can't be equal.
        if (Js::NumberUtilities::LuLoDbl(dbl1) != Js::NumberUtilities::LuLoDbl(dbl2))
            return FALSE;

        // If the high ulongs don't match, they can be equal iff one is -0 and
        // the other is +0.
        if (Js::NumberUtilities::LuHiDbl(dbl1) != Js::NumberUtilities::LuHiDbl(dbl2))
        {
            return 0x80000000 == (Js::NumberUtilities::LuHiDbl(dbl1) | Js::NumberUtilities::LuHiDbl(dbl2)) &&
                0 == Js::NumberUtilities::LuLoDbl(dbl1);
        }

        // The bit patterns match. They are equal iff they are not Nan.
        return !Js::NumberUtilities::IsNan(dbl1);
    }

    Var JavascriptOperators::OP_ApplyArgs(Var func, Var instance, __in_xcount(8) void** stackPtr, CallInfo callInfo, ScriptContext* scriptContext)
    {
        int argCount=callInfo.Count;
        ///
        /// Check func has internal [[Call]] property
        /// If not, throw TypeError
        ///
        if (!JavascriptConversion::IsCallable(func)) {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction);
        }

        // Fix callInfo: expect result/value, and none of other flags are currently applicable.
        //   OP_ApplyArgs expects a result. Neither of {jit, intereted} mode sends correct callFlags:
        //   LdArgCnt -- jit sends whatever was passed to current function, interpreter always sends 0.
        //   See Win8 bug 490489.
        callInfo.Flags = CallFlags_Value;

        RecyclableObject *funcPtr = RecyclableObject::FromVar(func);
        PROBE_STACK(scriptContext, Js::Constants::MinStackDefault+argCount*4);

        JavascriptMethod entryPoint=funcPtr->GetEntryPoint();
        Var ret;

        switch (argCount) {
        case 0:
            Assert(false);
            ret=entryPoint(funcPtr,callInfo);
            break;
        case 1:
            ret=entryPoint(funcPtr,callInfo,instance);
            break;
        case 2:
            ret=entryPoint(funcPtr,callInfo,instance,stackPtr[0]);
            break;
        case 3:
            ret=entryPoint(funcPtr,callInfo,instance,stackPtr[0],stackPtr[1]);
            break;
        case 4:
            ret=entryPoint(funcPtr,callInfo,instance,stackPtr[0],stackPtr[1],stackPtr[2]);
            break;
        case 5:
            ret=entryPoint(funcPtr,callInfo,instance,stackPtr[0],stackPtr[1],stackPtr[2],stackPtr[3]);
            break;
        case 6:
            ret=entryPoint(funcPtr,callInfo,instance,stackPtr[0],stackPtr[1],stackPtr[2],stackPtr[3],stackPtr[4]);
            break;
        case 7:
            ret=entryPoint(funcPtr,callInfo,instance,stackPtr[0],stackPtr[1],stackPtr[2],stackPtr[3],stackPtr[4],stackPtr[5]);
            break;
        default: {
                // Don't need stack probe here- we just did so above
                Arguments args(callInfo,stackPtr-1);
                ret=JavascriptFunction::CallFunction<false>(funcPtr,entryPoint,args);
            }
            break;
        }
        return ret;
    }

#ifdef _M_IX86
    // Alias for overloaded JavascriptNumber::ToVar so it can be called unambiguously from native code
    Var JavascriptOperators::Int32ToVar(int32 value, ScriptContext* scriptContext)
    {
        return JavascriptNumber::ToVar(value, scriptContext);
    }

    // Alias for overloaded JavascriptNumber::ToVar so it can be called unambiguously from native code
    Var JavascriptOperators::Int32ToVarInPlace(int32 value, ScriptContext* scriptContext, __out JavascriptNumber* result)
    {
        return JavascriptNumber::ToVarInPlace(value, scriptContext, result);
    }

    // Alias for overloaded JavascriptNumber::ToVar so it can be called unambiguously from native code
    Var JavascriptOperators::UInt32ToVar(uint32 value, ScriptContext* scriptContext)
    {
        return JavascriptNumber::ToVar(value, scriptContext);
    }

    // Alias for overloaded JavascriptNumber::ToVar so it can be called unambiguously from native code
    Var JavascriptOperators::UInt32ToVarInPlace(uint32 value, ScriptContext* scriptContext, __out JavascriptNumber* result)
    {
        return JavascriptNumber::ToVarInPlace(value, scriptContext, result);
    }
#endif

    Var JavascriptOperators::OP_FinishOddDivBy2(uint32 value, ScriptContext *scriptContext)
    {
        return JavascriptNumber::New((double)(value + 0.5), scriptContext);
    }

    Var JavascriptOperators::ToNumberInPlace(Var aRight, ScriptContext* scriptContext, __out JavascriptNumber* result)
    {
        if (TaggedInt::Is(aRight) || JavascriptNumber::Is_NoTaggedIntCheck(aRight))
        {
            return aRight;
        }

        return JavascriptNumber::ToVarInPlace(JavascriptConversion::ToNumber(aRight, scriptContext), scriptContext, result);
    }

    Var JavascriptOperators::Typeof(Var var, ScriptContext* scriptContext)
    {
        switch (JavascriptOperators::GetTypeId(var))
        {
        case TypeIds_Undefined:
            return scriptContext->GetLibrary()->GetUndefinedDisplayString();

        case TypeIds_Null:
            //null
            return scriptContext->GetLibrary()->GetObjectTypeDisplayString();

        case TypeIds_Integer:
        case TypeIds_Number:
        case TypeIds_Int64Number:
        case TypeIds_UInt64Number:
            return scriptContext->GetLibrary()->GetNumberTypeDisplayString();

#ifdef SIMD_JS_ENABLED
        case TypeIds_SIMDFloat32x4:
            if (Js::Configuration::Global.flags.Simdjs)
            {
                return scriptContext->GetLibrary()->GetSIMDFloat32x4DisplayString();
            }
        case TypeIds_SIMDFloat64x2:
            if (Js::Configuration::Global.flags.Simdjs)
            {
                return scriptContext->GetLibrary()->GetSIMDFloat64x2DisplayString();
            }
        case TypeIds_SIMDInt32x4:
            if (Js::Configuration::Global.flags.Simdjs)
            {
                return scriptContext->GetLibrary()->GetSIMDInt32x4DisplayString();
            }
#endif

        default:

            // Falsy objects are typeof 'undefined'.
            if (RecyclableObject::FromVar(var)->GetType()->IsFalsy())
            {
                return scriptContext->GetLibrary()->GetUndefinedDisplayString();
            }
            else
            {
                return RecyclableObject::FromVar(var)->GetTypeOfString(scriptContext);
            }
        }
    }

    Var JavascriptOperators::TypeofFld(Var instance, PropertyId propertyId, ScriptContext* scriptContext)
    {
        return TypeofFld_Internal(instance, false, propertyId, scriptContext);
    }

    Var JavascriptOperators::TypeofRootFld(Var instance, PropertyId propertyId, ScriptContext* scriptContext)
    {
        return TypeofFld_Internal(instance, true, propertyId, scriptContext);
    }

    Var JavascriptOperators::TypeofFld_Internal(Var instance, const bool isRoot, PropertyId propertyId, ScriptContext* scriptContext)
    {
        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined , scriptContext->GetPropertyName(propertyId)->GetBuffer());
        }

        Var value;
        try
        {
            Js::JavascriptExceptionOperators::AutoCatchHandlerExists autoCatchHandlerExists(scriptContext);

            // In edge mode, spec compat is more important than backward compat. Use spec/web behavior here
            if (isRoot
                    ? !JavascriptOperators::GetRootProperty(instance, propertyId, &value, scriptContext)
                    : !JavascriptOperators::GetProperty(instance, object, propertyId, &value, scriptContext))
            {
                return scriptContext->GetLibrary()->GetUndefinedDisplayString();
            }
            if (!scriptContext->IsUndeclBlockVar(value))
            {
                return JavascriptOperators::Typeof(value, scriptContext);
            }
        }
        catch(Js::JavascriptExceptionObject * )
        {
            return scriptContext->GetLibrary()->GetUndefinedDisplayString();
        }

        Assert(scriptContext->IsUndeclBlockVar(value));
        Assert(scriptContext->GetConfig()->IsLetAndConstEnabled());
        JavascriptError::ThrowReferenceError(scriptContext, JSERR_UseBeforeDeclaration);
    }


    Var JavascriptOperators::TypeofElem_UInt32(Var instance, uint32 index, ScriptContext* scriptContext)
    {
        if (JavascriptOperators::IsNumberFromNativeArray(instance, index, scriptContext))
            return scriptContext->GetLibrary()->GetNumberTypeDisplayString();

#if FLOATVAR
        return TypeofElem(instance, Js::JavascriptNumber::ToVar(index, scriptContext), scriptContext);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return TypeofElem(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext);
#endif
    }

    Var JavascriptOperators::TypeofElem_Int32(Var instance, int32 index, ScriptContext* scriptContext)
    {
        if (JavascriptOperators::IsNumberFromNativeArray(instance, index, scriptContext))
            return scriptContext->GetLibrary()->GetNumberTypeDisplayString();

#if FLOATVAR
        return TypeofElem(instance, Js::JavascriptNumber::ToVar(index, scriptContext), scriptContext);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return TypeofElem(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext);
#endif

    }

    Js::JavascriptString* GetPropertyDisplayNameForError(Var prop, ScriptContext* scriptContext)
    {
        JavascriptString* str;

        if (JavascriptSymbol::Is(prop))
        {
            str = JavascriptSymbol::ToString(JavascriptSymbol::FromVar(prop)->GetValue(), scriptContext);
        }
        else
        {
            str = JavascriptConversion::ToString(prop, scriptContext);
        }

        return str;
    }

    Var JavascriptOperators::TypeofElem(Var instance, Var index, ScriptContext* scriptContext)
    {
        RecyclableObject* object = nullptr;

        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined, GetPropertyDisplayNameForError(index, scriptContext));
        }

        Var member;
        uint32 indexVal;
        PropertyRecord const * propertyRecord = nullptr;

        ThreadContext* threadContext = scriptContext->GetThreadContext();
        ImplicitCallFlags savedImplicitCallFlags = threadContext->GetImplicitCallFlags();
        threadContext->ClearImplicitCallFlags();

        try
        {
            Js::JavascriptExceptionOperators::AutoCatchHandlerExists autoCatchHandlerExists(scriptContext);

            // TODO: Perf optimization: If IsStringTypeHandler is true, convert indexVal to string instead of PropertyRecord and let
            // type handler decide whether to turn it into a string or not.  Perf is gained if PropertyRecords are avoided altogether
            // when dealing with a string based type handler.

            // For JS Objects, don't create the propertyId if not already added
            bool createIfNotFound = !IsJsNativeObject(object) ||
                (DynamicType::Is(object->GetTypeId()) && static_cast<DynamicObject*>(object)->GetTypeHandler()->IsStringTypeHandler());
            if (GetIndexType(index, scriptContext, &indexVal, &propertyRecord, createIfNotFound) == IndexType_Number)
            {
                // In edge mode, we don't need to worry about the special "unknown" behavior. If the item is not available from Get,
                // just return undefined.
                if (!JavascriptOperators::GetItem(instance, object, indexVal, &member, scriptContext))
                {
                    // If the instance doesn't have the item, typeof result is "undefined".
                    threadContext->CheckAndResetImplicitCallAccessorFlag();
                    threadContext->AddImplicitCallFlags(savedImplicitCallFlags);
                    return scriptContext->GetLibrary()->GetUndefinedDisplayString();
                }
            }
            else if (propertyRecord == nullptr)
            {
                Assert(IsJsNativeObject(object));

#if DBG
                JavascriptString* indexStr = JavascriptConversion::ToString(index, scriptContext);
                PropertyRecord const * debugPropertyRecord;
                scriptContext->GetOrAddPropertyRecord(indexStr->GetString(), indexStr->GetLength(), &debugPropertyRecord);
                AssertMsg(!JavascriptOperators::GetProperty(instance, object, debugPropertyRecord->GetPropertyId(), &member, scriptContext), "how did this property come");
#endif

                // If the instance doesn't have the property, typeof result is "undefined".
                threadContext->CheckAndResetImplicitCallAccessorFlag();
                threadContext->AddImplicitCallFlags(savedImplicitCallFlags);
                return scriptContext->GetLibrary()->GetUndefinedDisplayString();
            }
            else
            {
                if (!JavascriptOperators::GetProperty(instance, object, propertyRecord->GetPropertyId(), &member, scriptContext))
                {
                    // If the instance doesn't have the property, typeof result is "undefined".
                    threadContext->CheckAndResetImplicitCallAccessorFlag();
                    threadContext->AddImplicitCallFlags(savedImplicitCallFlags);
                    return scriptContext->GetLibrary()->GetUndefinedDisplayString();
                }
            }
            threadContext->CheckAndResetImplicitCallAccessorFlag();
            threadContext->AddImplicitCallFlags(savedImplicitCallFlags);
            return JavascriptOperators::Typeof(member, scriptContext);
        }
        catch(Js::JavascriptExceptionObject * )
        {
            threadContext->CheckAndResetImplicitCallAccessorFlag();
            threadContext->AddImplicitCallFlags(savedImplicitCallFlags);
            return scriptContext->GetLibrary()->GetUndefinedDisplayString();
        }
    }

    //
    // Delete the given Var
    // TODO: Make the TypeId generic and call the method here.
    // "Cannot delete" should be a generic message
    //
    Var JavascriptOperators::Delete(Var var, ScriptContext* scriptContext)
    {
        return scriptContext->GetLibrary()->GetTrue();
    }

    BOOL JavascriptOperators::Equal_Full(Var aLeft, Var aRight, ScriptContext* requestContext)
    {
        //
        // Fast-path SmInts and paired Number combinations.
        //

        if (aLeft == aRight)
        {
            if (JavascriptNumber::Is(aLeft) && JavascriptNumber::IsNan(JavascriptNumber::GetValue(aLeft)))
            {
                return false;
            }
            else if (JavascriptVariantDate::Is(aLeft) == false) // only need to check on aLeft - since they are the same var, aRight would do the same
            {
                return true;
            }
            else
            {
                //In ES5 mode strict equals (===) on same instance of object type VariantDate succeeds.
                //Hence equals needs to succeed.
                return true;
            }
        }

        BOOL result = false;

        if (TaggedInt::Is(aLeft))
        {
            if (TaggedInt::Is(aRight))
            {
                // If aLeft == aRight, we would already have returned true above.
                return false;
            }
            else if (JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return TaggedInt::ToDouble(aLeft) == JavascriptNumber::GetValue(aRight);
            }
            else
            {
                BOOL res = RecyclableObject::FromVar(aRight)->Equals(aLeft, &result, requestContext);
                AssertMsg(res, "Should have handled this");
                return result;
            }
        }
        else if (JavascriptNumber::Is_NoTaggedIntCheck(aLeft))
        {
            if (TaggedInt::Is(aRight))
            {
                return TaggedInt::ToDouble(aRight) == JavascriptNumber::GetValue(aLeft);
            }
            else if(JavascriptNumber::Is_NoTaggedIntCheck(aRight))
            {
                return JavascriptNumber::GetValue(aLeft) == JavascriptNumber::GetValue(aRight);
            }
            else
            {
                BOOL res = RecyclableObject::FromVar(aRight)->Equals(aLeft, &result, requestContext);
                AssertMsg(res, "Should have handled this");
                return result;
            }
        }

        if (RecyclableObject::FromVar(aLeft)->Equals(aRight, &result, requestContext))
        {
            return result;
        }
        else
        {
            return false;
        }
    }

    BOOL JavascriptOperators::Greater_Full(Var aLeft,Var aRight,ScriptContext* scriptContext)
    {
        return RelationalComparsionHelper(aRight, aLeft, scriptContext, false, false);
    }

    BOOL JavascriptOperators::Less_Full(Var aLeft, Var aRight, ScriptContext* scriptContext)
    {
        return RelationalComparsionHelper(aLeft, aRight, scriptContext, true, false);
    }

    BOOL JavascriptOperators::RelationalComparsionHelper(Var aLeft, Var aRight, ScriptContext* scriptContext, bool leftFirst, bool undefinedAs)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(aLeft);

        if (typeId == TypeIds_Null)
        {
            aLeft=TaggedInt::ToVarUnchecked(0);
        }
        else if (typeId == TypeIds_Undefined)
        {
            aLeft=scriptContext->GetLibrary()->GetNaN();
        }

        typeId = JavascriptOperators::GetTypeId(aRight);

        if (typeId == TypeIds_Null)
        {
            aRight=TaggedInt::ToVarUnchecked(0);
        }
        else if (typeId == TypeIds_Undefined)
        {
            aRight=scriptContext->GetLibrary()->GetNaN();
        }

        double dblLeft, dblRight;

Redo:
        TypeId leftType = JavascriptOperators::GetTypeId(aLeft);
        TypeId rightType = JavascriptOperators::GetTypeId(aRight);

        switch (leftType)
        {
        case TypeIds_Integer:
            dblLeft = TaggedInt::ToDouble(aLeft);
            switch (rightType)
            {
            case TypeIds_Integer:
                dblRight = TaggedInt::ToDouble(aRight);
                break;
            case TypeIds_Number:
                dblRight = JavascriptNumber::GetValue(aRight);
                break;
            default:
                dblRight = JavascriptConversion::ToNumber(aRight, scriptContext);
                break;
            }
            break;
        case TypeIds_Number:
            dblLeft = JavascriptNumber::GetValue(aLeft);
            switch (rightType)
            {
            case TypeIds_Integer:
                dblRight = TaggedInt::ToDouble(aRight);
                break;
            case TypeIds_Number:
                dblRight = JavascriptNumber::GetValue(aRight);
                break;
            default:
                dblRight = JavascriptConversion::ToNumber(aRight, scriptContext);
                break;
            }
            break;
        case TypeIds_Int64Number:
            {
                switch (rightType)
                {
                case TypeIds_Int64Number:
                    {
                        __int64 leftValue = JavascriptInt64Number::FromVar(aLeft)->GetValue();
                        __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                        return leftValue < rightValue;
                    }
                    break;
                case TypeIds_UInt64Number:
                    {
                        __int64 leftValue = JavascriptInt64Number::FromVar(aLeft)->GetValue();
                        unsigned __int64 rightValue = JavascriptUInt64Number::FromVar(aRight)->GetValue();
                        if (rightValue <= INT_MAX && leftValue >= 0)
                        {
                            return leftValue < (__int64)rightValue;
                        }
                    }
                    break;
                }
                dblLeft = (double)JavascriptInt64Number::FromVar(aLeft)->GetValue();
                dblRight = JavascriptConversion::ToNumber(aRight, scriptContext);
            }
            break;

        // we cannot do double conversion between 2 int64 numbers as we can get wrong result after conversion
        // ie, two different numbers become the same after losing precision. We'll continue dbl comparision
        // if either number is not an int64 number.
        case TypeIds_UInt64Number:
            {
                switch (rightType)
                {
                case TypeIds_Int64Number:
                    {
                        unsigned __int64 leftValue = JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                        __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                        if (leftValue < INT_MAX && rightValue >= 0)
                        {
                            return (__int64)leftValue < rightValue;
                        }
                    }
                    break;
                case TypeIds_UInt64Number:
                    {
                        unsigned __int64 leftValue = JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                        unsigned __int64 rightValue = JavascriptUInt64Number::FromVar(aRight)->GetValue();
                        return leftValue < rightValue;
                    }
                    break;
                }
                dblLeft = (double)JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                dblRight = JavascriptConversion::ToNumber(aRight, scriptContext);
            }
            break;
        case TypeIds_String:
            switch (rightType)
            {
            case TypeIds_Integer:
            case TypeIds_Number:
            case TypeIds_Boolean:
                break;
            default:
                aRight = JavascriptConversion::ToPrimitive(aRight, JavascriptHint::HintNumber, scriptContext);
                rightType = JavascriptOperators::GetTypeId(aRight);
                if (rightType != TypeIds_String)
                {
                    dblRight = JavascriptConversion::ToNumber(aRight, scriptContext);
                    break;
                }
            case TypeIds_String:
                return JavascriptString::LessThan(aLeft, aRight);
            }
            dblLeft = JavascriptConversion::ToNumber(aLeft, scriptContext);
            dblRight = JavascriptConversion::ToNumber(aRight, scriptContext);
            break;
        case TypeIds_Boolean:
            dblLeft = JavascriptConversion::ToNumber(aLeft, scriptContext);
            dblRight = JavascriptConversion::ToNumber(aRight, scriptContext);
            break;
        default:
            // TODO: does order really matter?
            if (leftFirst)
            {
                aLeft = JavascriptConversion::ToPrimitive(aLeft, JavascriptHint::HintNumber, scriptContext);
                aRight = JavascriptConversion::ToPrimitive(aRight, JavascriptHint::HintNumber, scriptContext);
            }
            else
            {
                aRight = JavascriptConversion::ToPrimitive(aRight, JavascriptHint::HintNumber, scriptContext);
                aLeft = JavascriptConversion::ToPrimitive(aLeft, JavascriptHint::HintNumber, scriptContext);
            }
            goto Redo;
        }

        //
        // And +0,-0 that is not implemented fully
        //

        if (JavascriptNumber::IsNan(dblLeft) || JavascriptNumber::IsNan(dblRight))
        {
            return undefinedAs;
        }

        // this will succeed for -0.0 == 0.0 case as well
        if (dblLeft == dblRight)
        {
            return false;
        }

        return dblLeft < dblRight;
    }

    BOOL JavascriptOperators::StrictEqualString(Var aLeft, Var aRight)
    {
        Assert(JavascriptOperators::GetTypeId(aRight) == TypeIds_String);

        if (JavascriptOperators::GetTypeId(aLeft) != TypeIds_String)
            return false;

        return JavascriptString::Equals(aLeft, aRight);
    }

    BOOL JavascriptOperators::StrictEqualEmptyString(Var aLeft)
    {
        TypeId leftType = JavascriptOperators::GetTypeId(aLeft);
        if (leftType != TypeIds_String)
            return false;

        return JavascriptString::FromVar(aLeft)->GetLength() == 0;
    }

    BOOL JavascriptOperators::StrictEqual(Var aLeft, Var aRight, ScriptContext* requestContext)
    {
        //
        // TODO: Need to implement the correct "Strict Equality Comparison Algorithm", as described
        // in (ES3.0: S11.9.6).
        //

        double dblLeft, dblRight;
        TypeId leftType = JavascriptOperators::GetTypeId(aLeft);
        TypeId rightType = JavascriptOperators::GetTypeId(aRight);

        switch (leftType)
        {
        case TypeIds_String:
            switch (rightType)
            {
            case TypeIds_String:
                return JavascriptString::Equals(aLeft, aRight);
            }
            return FALSE;
        case TypeIds_Integer:
            switch (rightType)
            {
            case TypeIds_Integer:
                return aLeft == aRight;
            // we don't need to worry about int64: it cannot equal as we create
            // JavascriptInt64Number only in overflow scenarios.
            case TypeIds_Number:
                dblLeft     = TaggedInt::ToDouble(aLeft);
                dblRight    = JavascriptNumber::GetValue(aRight);
                goto CommonNumber;
            }
            return FALSE;
        case TypeIds_Int64Number:
            switch (rightType)
            {
            case TypeIds_Int64Number:
                {
                    __int64 leftValue = JavascriptInt64Number::FromVar(aLeft)->GetValue();
                    __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    return leftValue == rightValue;
                }
            case TypeIds_UInt64Number:
                {
                    __int64 leftValue = JavascriptInt64Number::FromVar(aLeft)->GetValue();
                    unsigned __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    // TODO: yongqu to review whether we need to check for neg value
                    return (/*leftValue >= 0 && */(unsigned __int64)leftValue == rightValue);
                }
            case TypeIds_Number:
                dblLeft     = (double)JavascriptInt64Number::FromVar(aLeft)->GetValue();
                dblRight    = JavascriptNumber::GetValue(aRight);
                goto CommonNumber;
            }
            return FALSE;
        case TypeIds_UInt64Number:
            switch (rightType)
            {
            case TypeIds_Int64Number:
                {
                    unsigned __int64 leftValue = JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                    __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    // TODO: yongqu to review whether we need to check for neg value
                    return (/* rightValue >= 0 && */leftValue == (unsigned __int64)rightValue);
                }
            case TypeIds_UInt64Number:
                {
                    unsigned __int64 leftValue = JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                    unsigned __int64 rightValue = JavascriptInt64Number::FromVar(aRight)->GetValue();
                    return leftValue == rightValue;
                }
            case TypeIds_Number:
                dblLeft     = (double)JavascriptUInt64Number::FromVar(aLeft)->GetValue();
                dblRight    = JavascriptNumber::GetValue(aRight);
                goto CommonNumber;
            }
            return FALSE;

        case TypeIds_Number:
            switch (rightType)
            {
            case TypeIds_Integer:
                dblLeft     = JavascriptNumber::GetValue(aLeft);
                dblRight    = TaggedInt::ToDouble(aRight);
                goto CommonNumber;
            case TypeIds_Int64Number:
                dblLeft     = JavascriptNumber::GetValue(aLeft);
                dblRight = (double)JavascriptInt64Number::FromVar(aRight)->GetValue();
                goto CommonNumber;
            case TypeIds_UInt64Number:
                dblLeft     = JavascriptNumber::GetValue(aLeft);
                dblRight = (double)JavascriptUInt64Number::FromVar(aRight)->GetValue();
                goto CommonNumber;
            case TypeIds_Number:
                dblLeft     = JavascriptNumber::GetValue(aLeft);
                dblRight    = JavascriptNumber::GetValue(aRight);
CommonNumber:
                return FEqualDbl(dblLeft, dblRight);
            }
            return FALSE;
        case TypeIds_Boolean:
            switch (rightType)
            {
            case TypeIds_Boolean:
                return aLeft == aRight;
            }
            return FALSE;

        case TypeIds_Undefined:
            return rightType == TypeIds_Undefined;

        case TypeIds_Null:
            return rightType == TypeIds_Null;

        case TypeIds_Array:
            return (rightType == TypeIds_Array && aLeft == aRight);

        case TypeIds_Symbol:
            switch (rightType)
            {
            case TypeIds_Symbol:
                {
                    const PropertyRecord* leftValue = JavascriptSymbol::FromVar(aLeft)->GetValue();
                    const PropertyRecord* rightValue = JavascriptSymbol::FromVar(aRight)->GetValue();
                    return leftValue == rightValue;
                }
            }
            return false;

        case TypeIds_GlobalObject:
        case TypeIds_HostDispatch:
            switch (rightType)
            {
                case TypeIds_HostDispatch:
                case TypeIds_GlobalObject:
                {
                    BOOL result;
                    if(RecyclableObject::FromVar(aLeft)->StrictEquals(aRight, &result, requestContext))
                    {
                        return result;
                    }
                    return false;
                }
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
                    return true;
                }
            }
            break;
        }

        if (RecyclableObject::FromVar(aLeft)->CanHaveInterceptors())
        {
            BOOL result;
            if (RecyclableObject::FromVar(aLeft)->StrictEquals(aRight, &result, requestContext))
            {
                if (result)
                {
                    return TRUE;
                }
            }
        }

        if (!TaggedNumber::Is(aRight) && RecyclableObject::FromVar(aRight)->CanHaveInterceptors())
        {
            BOOL result;
            if (RecyclableObject::FromVar(aRight)->StrictEquals(aLeft, &result, requestContext))
            {
                if (result)
                {
                    return TRUE;
                }
            }
        }

        return aLeft == aRight;
    }

    BOOL JavascriptOperators::HasOwnProperty(Var instance, PropertyId propertyId, ScriptContext *requestContext)
    {
        BOOL result;
        if (TaggedNumber::Is(instance))
        {
            result = false;
        }
        else
        {
            RecyclableObject* object = RecyclableObject::FromVar(instance);

            if (JavascriptProxy::Is(instance))
            {
                PropertyDescriptor desc;
                return GetOwnPropertyDescriptor(object, propertyId, requestContext, &desc);
            }
            else
            {
                return object && object->HasOwnProperty(propertyId);
            }
        }
        return result;
    }

    BOOL JavascriptOperators::GetOwnAccessors(Var instance, PropertyId propertyId, Var* getter, Var* setter, ScriptContext * requestContext)
    {
        BOOL result;
        if (TaggedNumber::Is(instance))
        {
            result = false;
        }
        else
        {
            RecyclableObject* object = RecyclableObject::FromVar(instance);
            result = object && object->GetAccessors(propertyId, getter, setter, requestContext);
        }
        return result;
    }

    Var JavascriptOperators::GetOwnPropertyNames(Var instance, ScriptContext *scriptContext)
    {
        RecyclableObject *object = RecyclableObject::FromVar(ToObject(instance, scriptContext));

        if (JavascriptProxy::Is(instance))
        {
            JavascriptProxy* proxy = JavascriptProxy::FromVar(instance);
            return proxy->PropertyKeysTrap(JavascriptProxy::KeysTrapKind::GetOwnPropertyNamesKind);
        }

        return JavascriptObject::CreateOwnStringPropertiesHelper(object, scriptContext);
    }

    Var JavascriptOperators::GetOwnPropertySymbols(Var instance, ScriptContext *scriptContext)
    {
        RecyclableObject *object = RecyclableObject::FromVar(ToObject(instance, scriptContext));
        CHAKRATEL_LANGSTATS_INC_BUILTINCOUNT(GetOwnPropertySymbolsCount);

        if (JavascriptProxy::Is(instance))
        {
            JavascriptProxy* proxy = JavascriptProxy::FromVar(instance);
            return proxy->PropertyKeysTrap(JavascriptProxy::KeysTrapKind::GetOwnPropertySymbolKind);
        }

        return JavascriptObject::CreateOwnSymbolPropertiesHelper(object, scriptContext);
    }

    Var JavascriptOperators::GetOwnPropertyKeys(Var instance, ScriptContext* scriptContext)
    {
        RecyclableObject *object = RecyclableObject::FromVar(ToObject(instance, scriptContext));

        if (JavascriptProxy::Is(instance))
        {
            JavascriptProxy* proxy = JavascriptProxy::FromVar(instance);
            return proxy->PropertyKeysTrap(JavascriptProxy::KeysTrapKind::KeysKind);
        }

        return JavascriptObject::CreateOwnStringSymbolPropertiesHelper(object, scriptContext);
    }

    Var JavascriptOperators::GetOwnEnumerablePropertyNames(Var instance, ScriptContext* scriptContext)
    {
        RecyclableObject *object = RecyclableObject::FromVar(ToObject(instance, scriptContext));

        if (JavascriptProxy::Is(instance))
        {
            JavascriptProxy* proxy = JavascriptProxy::FromVar(instance);
            Var result = proxy->PropertyKeysTrap(JavascriptProxy::KeysTrapKind::GetOwnPropertyNamesKind);

            AssertMsg(JavascriptArray::Is(result), "PropertyKeysTrap should return JavascriptArray.");
            JavascriptArray* proxyResult;
            JavascriptArray* proxyResultToReturn = scriptContext->GetLibrary()->CreateArray(0);
            if (JavascriptArray::Is(result))
            {
                proxyResult = JavascriptArray::FromVar(result);
            }
            else
            {
                return proxyResultToReturn;
            }

            // filter enumerable keys
            uint32 resultLength = proxyResult->GetLength();
            Var element;
            const Js::PropertyRecord *propertyRecord;
            uint32 index = 0;
            for (uint32 i = 0; i < resultLength; i++)
            {
                element = proxyResult->DirectGetItem(i);

                // skip symbols
                if (JavascriptSymbol::Is(element))
                {
                    continue;
                }

                PropertyDescriptor propertyDescriptor;
                JavascriptObject::GetPropertyRecordFromVar<false>(element, scriptContext, &propertyRecord);
                if (JavascriptOperators::GetOwnPropertyDescriptor(RecyclableObject::FromVar(instance), propertyRecord->GetPropertyId(), scriptContext, &propertyDescriptor))
                {
                    if (propertyDescriptor.IsEnumerable())
                    {
                        proxyResultToReturn->DirectSetItemAt(index++, element);
                    }
                }
            }
            return proxyResultToReturn;
        }
        return JavascriptObject::CreateOwnEnumerableStringPropertiesHelper(object, scriptContext);
    }

    Var JavascriptOperators::GetOwnEnumerablePropertyNamesSymbols(Var instance, ScriptContext* scriptContext)
    {
        RecyclableObject *object = RecyclableObject::FromVar(ToObject(instance, scriptContext));

        if (JavascriptProxy::Is(instance))
        {
            JavascriptProxy* proxy = JavascriptProxy::FromVar(instance);
            return proxy->PropertyKeysTrap(JavascriptProxy::KeysTrapKind::KeysKind);
        }
        return JavascriptObject::CreateOwnEnumerableStringSymbolPropertiesHelper(object, scriptContext);
    }

    BOOL JavascriptOperators::GetOwnProperty(Var instance, PropertyId propertyId, Var* value, ScriptContext* requestContext)
    {
        BOOL result;
        if (TaggedNumber::Is(instance))
        {
            result = false;
        }
        else
        {
            RecyclableObject* object = RecyclableObject::FromVar(instance);
            result = object && object->GetProperty(object, propertyId, value, NULL, requestContext);
        }
        return result;
    }

    // ES5's [[GetOwnProperty]].
    // Return value:
    //   FALSE means "undefined" PD.
    //   TRUE means success. The propertyDescriptor parameter gets the descriptor.
    //
    BOOL JavascriptOperators::GetOwnPropertyDescriptor(RecyclableObject* obj, PropertyId propertyId, ScriptContext* scriptContext, PropertyDescriptor* propertyDescriptor)
    {
        Assert(obj);
        Assert(scriptContext);
        Assert(propertyDescriptor);

        // TODO: this relies on the passed descriptor, while the spec says that we should create a new descriptor.
        if (JavascriptProxy::Is(obj))
        {
            return JavascriptProxy::GetOwnPropertyDescriptor(obj, propertyId, scriptContext, propertyDescriptor);
        }
        Var getter, setter;
        if (false == JavascriptOperators::GetOwnAccessors(obj, propertyId, &getter, &setter, scriptContext))
        {
            Var value;
            if (false == JavascriptOperators::GetOwnProperty(obj, propertyId, &value, scriptContext))
            {
                return FALSE;
            }
            if (null != value)
            {
                propertyDescriptor->SetValue(value);
            }

            //TODO (abchatra): Its expensive to query for each flag from type system. Combine this with the GetOwnProperty to get all the flags
            //at once. This will require a new API from type system and override in all the types which overrides IsEnumerable etc.
            //Currently there is no performance tuning for ES5. This should be ok.
            propertyDescriptor->SetWritable(FALSE != obj->IsWritable(propertyId));
        }
        else
        {
            if (nullptr == getter)
            {
                getter = scriptContext->GetLibrary()->GetUndefined();
            }
            propertyDescriptor->SetGetter(getter);

            if (nullptr == setter)
            {
                setter = scriptContext->GetLibrary()->GetUndefined();
            }
            propertyDescriptor->SetSetter(setter);
        }

        propertyDescriptor->SetConfigurable(FALSE != obj->IsConfigurable(propertyId));
        propertyDescriptor->SetEnumerable(FALSE != obj->IsEnumerable(propertyId));
        return TRUE;
    }

    __inline RecyclableObject* JavascriptOperators::GetPrototypeNoTrap(RecyclableObject* instance)
    {
        Type* type = instance->GetType();
        if (type->HasSpecialPrototype())
        {
            if (type->GetTypeId() == TypeIds_Proxy)
            {
                // get back null
                Assert(type->GetPrototype() == instance->GetScriptContext()->GetLibrary()->GetNull());
                return type->GetPrototype();
            }
            else
            {
                return instance->GetPrototypeSpecial();
            }
        }
        return type->GetPrototype();
    }

    BOOL JavascriptOperators::IsArray(Var instanceVar)
    {
        if (!RecyclableObject::Is(instanceVar))
        {
            return FALSE;
        }
        RecyclableObject* instance = RecyclableObject::FromVar(instanceVar);
        if (DynamicObject::IsAnyArray(instance))
        {
            return TRUE;
        }
        if (JavascriptProxy::Is(instanceVar))
        {
            JavascriptProxy* proxy = JavascriptProxy::FromVar(instanceVar);
            return IsArray(proxy->GetTarget());
        }
        TypeId remoteTypeId = TypeIds_Limit;
        if (JavascriptOperators::GetRemoteTypeId(instanceVar, &remoteTypeId) &&
            DynamicObject::IsAnyArrayTypeId(remoteTypeId))
        {
            return TRUE;
        }
        return FALSE;
    }

    BOOL JavascriptOperators::IsConcatSpreadable(Var instanceVar)
    {
        // an object is spreadable under two condition, either it is a JsArray
        // or you define an isconcatSpreadable flag on it.
        if (!JavascriptOperators::IsObject(instanceVar))
        {
            return false;
        }

        RecyclableObject* instance = RecyclableObject::FromVar(instanceVar);
        ScriptContext* scriptContext = instance->GetScriptContext();
        Var spreadable = JavascriptOperators::GetProperty(instance, PropertyIds::_symbolIsConcatSpreadable, scriptContext);
        if (spreadable != scriptContext->GetLibrary()->GetUndefined())
        {
            return JavascriptConversion::ToBoolean(spreadable, scriptContext);
        }
        if (JavascriptOperators::IsArray(instance))
        {
            return true;
        }
        return false;

    }

    Var JavascriptOperators::OP_LdCustomSpreadIteratorList(Var aRight, ScriptContext* scriptContext)
    {
        RecyclableObject* function = GetIteratorFunction(aRight, scriptContext);
        JavascriptMethod method = function->GetEntryPoint();
        if (((JavascriptArray::Is(aRight) || ArgumentsObject::Is(aRight)) && method == (&JavascriptArray::EntryInfo::Values)->GetOriginalEntryPoint()) ||
            (TypedArrayBase::Is(aRight) && method == (&TypedArrayBase::EntryInfo::Values)->GetOriginalEntryPoint())
            )
        {
            return aRight;
        }

        Var iteratorVar = function->GetEntryPoint()(function, CallInfo(Js::CallFlags_Value, 1), aRight);

        if (!JavascriptOperators::IsObject(iteratorVar))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
        }

        RecyclableObject* iterator = RecyclableObject::FromVar(iteratorVar);
        return RecyclerNew(scriptContext->GetRecycler(), SpreadArgument, aRight, iterator, scriptContext->GetLibrary()->GetSpreadArgumentType());
    }

    BOOL JavascriptOperators::IsPropertyUnscopable (Var instanceVar, JavascriptString *propertyString)
    {
        // This never gets called.
        Throw::InternalError();

    }
    BOOL JavascriptOperators::IsPropertyUnscopable (Var instanceVar, PropertyId propertyId)
    {
        RecyclableObject* instance = RecyclableObject::FromVar(instanceVar);

        Var propertyValue = JavascriptOperators::GetProperty(instance, PropertyIds::_symbolUnscopables, instance->GetScriptContext());
        if (DynamicObject::Is(propertyValue))
        {
            DynamicObject *blackList = DynamicObject::FromVar(propertyValue);
            Var value;
            //8.1.1.2.1.9.c If blocked is not undefined
            return blackList->GetProperty(propertyValue, propertyId, &value, nullptr, blackList->GetScriptContext());
        }
        return false;
    }

    template <bool unscopables>
    BOOL JavascriptOperators::HasProperty_Impl(RecyclableObject* instance, PropertyId propertyId)
    {
        while (JavascriptOperators::GetTypeId(instance) != TypeIds_Null)
        {

            if(unscopables && IsPropertyUnscopable (instance, propertyId))
            {
                return false;
            }
            if(instance->HasProperty(propertyId))
            {
                return true;
            }
            instance = JavascriptOperators::GetPrototypeNoTrap(instance);
        }
        return false;
    }

    BOOL JavascriptOperators::HasPropertyUnscopables(RecyclableObject* instance, PropertyId propertyId)
    {
        return HasProperty_Impl<true>(instance, propertyId);
    }

    BOOL JavascriptOperators::HasProperty(RecyclableObject* instance, PropertyId propertyId)
    {
        return HasProperty_Impl<false>(instance, propertyId);
    }

    BOOL JavascriptOperators::HasRootProperty(RecyclableObject* instance, PropertyId propertyId)
    {
        Assert(RootObjectBase::Is(instance));

        RootObjectBase* rootObject = static_cast<RootObjectBase*>(instance);
        if (rootObject->HasRootProperty(propertyId))
        {
            return true;
        }
        instance = instance->GetPrototype();

        return HasProperty(instance, propertyId);
    }

    BOOL JavascriptOperators::HasProxyOrPrototypeInlineCacheProperty(RecyclableObject* instance, PropertyId propertyId)
    {
        TypeId typeId;
        typeId = JavascriptOperators::GetTypeId(instance);
        if (typeId == Js::TypeIds_Proxy)
        {
            // let's be more aggressive to disable inline prototype cache when proxy is presented in the prototypechain
            return true;
        }
        do
        {
            instance = instance->GetPrototype();
            typeId = JavascriptOperators::GetTypeId(instance);
            if (typeId == Js::TypeIds_Proxy)
            {
                // let's be more aggressive to disable inline prototype cache when proxy is presented in the prototypechain
                return true;
            }
            if (typeId == TypeIds_Null)
            {
                break;
            }
            /* We can rule out object with deferred type handler, because they would have expanded if they are in the cache */
            if (!instance->HasDeferredTypeHandler() && instance->HasProperty(propertyId)) { return true; }
        } while (typeId != TypeIds_Null);
        return false;
    }

    BOOL JavascriptOperators::OP_HasProperty(Var instance, PropertyId propertyId, ScriptContext* scriptContext)
    {
        RecyclableObject* object = TaggedNumber::Is(instance) ?
            scriptContext->GetLibrary()->GetNumberPrototype() :
            RecyclableObject::FromVar(instance);
        BOOL result = HasProperty(object, propertyId);
        return result;
    }

    BOOL JavascriptOperators::OP_HasOwnProperty(Var instance, PropertyId propertyId, ScriptContext* scriptContext)
    {
        RecyclableObject* object = TaggedNumber::Is(instance) ?
            scriptContext->GetLibrary()->GetNumberPrototype() :
            RecyclableObject::FromVar(instance);
        BOOL result = HasOwnProperty(object, propertyId, scriptContext);
        return result;
    }

    // TODO: change this to similar code in HasOwnDataPropetyNoHostObject?
    BOOL JavascriptOperators::HasOwnPropertyNoHostObject(Var instance, PropertyId propertyId)
    {
        AssertMsg(!TaggedNumber::Is(instance), "HasOwnPropertyNoHostObject int passed");

        RecyclableObject* object = RecyclableObject::FromVar(instance);
        return object && object->HasOwnPropertyNoHostObject(propertyId);
    }

    // TODO: Remove HasOwnPropertyNoHostObjectForHeapEnum and use GetOwnPropertyNoHostObjectForHeapEnum in its place by changing it
    // to return BOOL, true or false with whether the property exists or not, and return the value if not getter/setter as an out param.
    BOOL JavascriptOperators::HasOwnPropertyNoHostObjectForHeapEnum(Var instance, PropertyId propertyId, ScriptContext* requestContext, Var& getter, Var& setter)
    {
        AssertMsg(!TaggedNumber::Is(instance), "HasOwnDataPropertyNoHostObject int passed");

        RecyclableObject * object = RecyclableObject::FromVar(instance);
        if (StaticType::Is(object->GetTypeId()))
        {
            return FALSE;
        }
        getter = setter = NULL;
        DynamicObject* dynamicObject = DynamicObject::FromVar(instance);
        Assert(dynamicObject->GetScriptContext()->IsHeapEnumInProgress());
        if (dynamicObject->UseDynamicObjectForNoHostObjectAccess())
        {
            if (!dynamicObject->DynamicObject::GetAccessors(propertyId, &getter, &setter, requestContext))
            {
                Var value;
                if (!dynamicObject->DynamicObject::GetProperty(instance, propertyId, &value, NULL, requestContext) ||
                    (requestContext->IsUndeclBlockVar(value) && (ActivationObject::Is(instance) || RootObjectBase::Is(instance))))
                {
                    return FALSE;
                }
            }
        }
        else
        {
            if (!object->GetAccessors(propertyId, &getter, &setter, requestContext))
            {
                Var value;
                if (!object->GetProperty(instance, propertyId, &value, NULL, requestContext) ||
                    (requestContext->IsUndeclBlockVar(value) && (ActivationObject::Is(instance) || RootObjectBase::Is(instance))))
                {
                    return FALSE;
                }
            }
        }
        return TRUE;
    }

    Var JavascriptOperators::GetOwnPropertyNoHostObjectForHeapEnum(Var instance, PropertyId propertyId, ScriptContext* requestContext, Var& getter, Var& setter)
    {
        AssertMsg(!TaggedNumber::Is(instance), "GetDataPropertyNoHostObject int passed");
        Assert(HasOwnPropertyNoHostObjectForHeapEnum(instance, propertyId, requestContext, getter, setter) || getter || setter);
        DynamicObject* dynamicObject = DynamicObject::FromVar(instance);
        getter = setter = NULL;
        if (NULL == dynamicObject)
        {
            return requestContext->GetLibrary()->GetUndefined();
        }
        Var returnVar = requestContext->GetLibrary()->GetUndefined();
        BOOL result = FALSE;
        if (dynamicObject->UseDynamicObjectForNoHostObjectAccess())
        {
            if (! dynamicObject->DynamicObject::GetAccessors(propertyId, &getter, &setter, requestContext))
            {
                result = dynamicObject->DynamicObject::GetProperty(instance, propertyId, &returnVar, NULL, requestContext);
            }
        }
        else
        {
            if (! dynamicObject->GetAccessors(propertyId, &getter, &setter, requestContext))
            {
                result = dynamicObject->GetProperty(instance, propertyId, &returnVar, NULL, requestContext);
            }
        }

        if (result)
        {
            return returnVar;
        }
        return requestContext->GetLibrary()->GetUndefined();
    }


    BOOL JavascriptOperators::OP_HasOwnPropScoped(Var scope, PropertyId propertyId, Var defaultInstance, ScriptContext* scriptContext)
    {
        AssertMsg(scope == scriptContext->GetLibrary()->GetNull() || JavascriptArray::Is(scope),
                  "Invalid scope chain pointer passed - should be null or an array");
        if (JavascriptArray::Is(scope))
        {
            JavascriptArray* arrScope = JavascriptArray::FromVar(scope);
            Var instance = arrScope->DirectGetItem(0);
            return JavascriptOperators::OP_HasOwnProperty(instance, propertyId, scriptContext);
        }
        return JavascriptOperators::OP_HasOwnProperty(defaultInstance, propertyId, scriptContext);
    }

    inline bool GetMissingPropertyValue(ScriptContext *scriptContext, RecyclableObject *instance, Var &value, PropertyId propertyId)
    {
        // If this is library undefined or null then ask the langauge service what undefined value to use. The language service
        // uses  instances with the type TypeIds_Undefined that are not the same value as Library.GetUndefined() to track what type
        // the value would have been had it been defined (derived from meta-data declarations).
        if (value == scriptContext->GetLibrary()->GetUndefined())
        {
            value = scriptContext->GetMissingPropertyResult(instance, propertyId);
            return true;
        }
        if (value == scriptContext->GetLibrary()->GetNull())
        {
            value = scriptContext->GetNullPropertyResult(instance, propertyId);
            return true;
        }
        return false;
    }

    inline bool GetMissingItemValue(ScriptContext *scriptContext, RecyclableObject *instance, Var &value, uint32 index)
    {
        // If this is library undefined or null then ask the langauge service what undefined value to use. The language service
        // uses  instances with the type TypeIds_Undefined that are not the same value as Library.GetUndefined() to track what type
        // the value would have been had it been defined (derived from meta-data declarations).
        if (value == scriptContext->GetLibrary()->GetUndefined())
        {
            value = scriptContext->GetMissingItemResult(instance, index);
            return true;
        }
        if (value == scriptContext->GetLibrary()->GetNull())
        {
            value = scriptContext->GetNullItemResult(instance, index);
            return true;
        }

        return false;
    }

    BOOL JavascriptOperators::GetPropertyUnscopable(Var instance, RecyclableObject* propertyObject, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        return GetProperty_Internal<true>(instance, propertyObject, false, propertyId, value, requestContext, info);
    }

    BOOL JavascriptOperators::GetProperty(Var instance, RecyclableObject* propertyObject, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        return GetProperty_Internal<false>(instance, propertyObject, false, propertyId, value, requestContext, info);
    }

    BOOL JavascriptOperators::GetRootProperty(Var instance, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        return GetProperty_Internal<false>(instance, RecyclableObject::FromVar(instance), true, propertyId, value, requestContext, info);
    }

    template <bool unscopables>
    BOOL JavascriptOperators::GetProperty_Internal(Var instance, RecyclableObject* propertyObject, const bool isRoot, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        if (TaggedNumber::Is(instance))
        {
            PropertyValueInfo::ClearCacheInfo(info);
        }
        RecyclableObject* object = propertyObject;
        BOOL foundProperty = FALSE;
        if (isRoot)
        {
            Assert(RootObjectBase::Is(object));

            RootObjectBase* rootObject = static_cast<RootObjectBase*>(object);
            foundProperty = rootObject->GetRootProperty(instance, propertyId, value, info, requestContext);
        }
        while (!foundProperty && JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {

            if (unscopables && IsPropertyUnscopable(object, propertyId))
            {
                break;
            }
            else
            {
                if (object->GetProperty(instance, propertyId, value, info, requestContext))
                {
                    foundProperty = true;
                    break;
                }
            }

            if (object->SkipsPrototype())
            {
                break;
            }

            object = JavascriptOperators::GetPrototypeNoTrap(object);
        }

        if (foundProperty)
        {
#if DBG
            if (DynamicObject::Is(object))
            {
                DynamicObject* dynamicObject = (DynamicObject*)object;
                DynamicTypeHandler* dynamicTypeHandler = dynamicObject->GetDynamicType()->GetTypeHandler();
                Var property;
                if (dynamicTypeHandler->CheckFixedProperty(requestContext->GetPropertyName(propertyId), &property, requestContext))
                {
                    Assert(value == null || *value == property);
                }
            }
#endif
            if (BinaryFeatureControl::LanguageService() && value && !requestContext->IsUndeclBlockVar(*value) && (GetMissingPropertyValue(requestContext, propertyObject, *value, propertyId) || IsUndefinedOrNullType(GetTypeId(*value))))
            {
                // We cannot update the inline cache as it might mask the correct undefined in an instance of the same type.
                return TRUE; // Avoid updating the cache for properties with the value undefined if in the language service.
            }
            // Don't cache the information if the value is undecl block var
            // REVIEW: We might want to only check this if we need to (For LdRootFld or ScopedLdFld)
            //         Also we might want to throw here instead of checking it again in the caller
            if (value && !requestContext->IsUndeclBlockVar(*value) && !WithScopeObject::Is(object))
            {
                CacheOperators::CachePropertyRead(instance, object, isRoot, propertyId, false, info, requestContext);
            }

            return TRUE;
        }
        else
        {
#ifdef MISSING_PROPERTY_STATS
            if (PHASE_STATS1(MissingPropertyCachePhase))
            {
                requestContext->RecordMissingPropertyMiss();
            }
#endif
            if (PHASE_TRACE1(MissingPropertyCachePhase))
            {
                Output::Print(L"MissingPropertyCaching: Missing property %d on slow path.\n", propertyId);
            }

            // Only cache missing property lookups for non-root field loads on objects that have PathTypeHandlers, because only these objects guarantee a type change when the property is added,
            // which obviates the need to explicitly invalidate missing property inline caches.
            if (!PHASE_OFF1(MissingPropertyCachePhase) && !isRoot && !BinaryFeatureControl::LanguageService() && DynamicObject::Is(instance) && ((DynamicObject*)instance)->GetDynamicType()->GetTypeHandler()->IsPathTypeHandler())
            {
#ifdef MISSING_PROPERTY_STATS
                if (PHASE_STATS1(MissingPropertyCachePhase))
                {
                    requestContext->RecordMissingPropertyCacheAttempt();
                }
#endif
                if (PHASE_TRACE1(MissingPropertyCachePhase))
                {
                    Output::Print(L"MissingPropertyCache: Caching missing property for property %d.\n", propertyId);
                }

                PropertyValueInfo::Set(info, requestContext->GetLibrary()->GetMissingPropertyHolder(), 0);
                CacheOperators::CachePropertyRead(instance, requestContext->GetLibrary()->GetMissingPropertyHolder(), isRoot, propertyId, true, info, requestContext);
            }

            return FALSE;
        }
    }

    template<typename PropertyKeyType>
    BOOL JavascriptOperators::GetPropertyWPCache(Var instance, RecyclableObject* propertyObject, PropertyKeyType propertyKey, Var* value, ScriptContext* requestContext, PropertyString * propertyString)
    {
        if (TaggedNumber::Is(instance))
        {
            propertyString = NULL;
        }
        PropertyValueInfo info;
        RecyclableObject* object = propertyObject;
        while (JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {
            if (object->GetProperty(instance, propertyKey, value, &info, requestContext))
            {
                if (propertyString != NULL)
                {
                    uint16 slotIndex = info.GetPropertyIndex();
                    if (slotIndex != Constants::NoSlot &&
                        info.GetInstance() == object &&
                        info.IsWritable() && !object->CanHaveInterceptors() &&
                        requestContext == object->GetScriptContext() &&
                        ((info.GetFlags() & (InlineCacheGetterFlag | InlineCacheSetterFlag)) == 0))
                    {
                        uint16 inlineOrAuxSlotIndex;
                        bool isInlineSlot;
                        DynamicObject::FromVar(info.GetInstance())->GetTypeHandler()->PropertyIndexToInlineOrAuxSlotIndex(slotIndex, &inlineOrAuxSlotIndex, &isInlineSlot);
                        propertyString->UpdateCache(info.GetInstance()->GetType(), inlineOrAuxSlotIndex, isInlineSlot, info.IsStoreFieldCacheEnabled());
                    }
                }
                return TRUE;
            }
            if (object->SkipsPrototype())
            {
                break;
            }
            object = JavascriptOperators::GetPrototypeNoTrap(object);
        }
        return FALSE;
    }

    BOOL JavascriptOperators::GetPropertyObject(Var instance, ScriptContext * scriptContext, RecyclableObject** propertyObject)
    {
        Assert(propertyObject);
        if (TaggedNumber::Is(instance))
        {
            *propertyObject = scriptContext->GetLibrary()->GetNumberPrototype();
            return TRUE;
        }
        RecyclableObject* object = RecyclableObject::FromVar(instance);
        TypeId typeId = object->GetTypeId();
        *propertyObject = object;
        if (typeId == TypeIds_Null || typeId == TypeIds_Undefined)
        {
            if (BinaryFeatureControl::LanguageService())
            {
                auto trackingValue = scriptContext->GetTrackingValue(object);
                if(trackingValue)
                {
                    typeId = JavascriptOperators::GetTypeId(trackingValue);
                    if (typeId != TypeIds_Null && typeId != TypeIds_Undefined)
                    {
                        return GetPropertyObject(trackingValue, scriptContext, propertyObject);
                    }
                }
            }
            return FALSE;
        }
        return TRUE;
    }

#if DBG
    BOOL JavascriptOperators::IsPropertyObject(RecyclableObject * instance)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(instance);
        return (typeId != TypeIds_Integer && typeId != TypeIds_Null && typeId != TypeIds_Undefined);
    }
#endif

    Var JavascriptOperators::OP_GetProperty(Var instance, PropertyId propertyId, ScriptContext* scriptContext)
    {
        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined, scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        Var result = JavascriptOperators::GetProperty(instance, object, propertyId, scriptContext);
        AssertMsg(result != null, "result null in OP_GetProperty");
        return result;
    }

    Var JavascriptOperators::OP_GetRootProperty(Var instance, PropertyId propertyId, PropertyValueInfo * info, ScriptContext* scriptContext)
    {
        AssertMsg(RootObjectBase::Is(instance), "Root must be an object!");

        Var value;
        if (JavascriptOperators::GetRootProperty(RecyclableObject::FromVar(instance), propertyId, &value, scriptContext, info))
        {
            if (scriptContext->IsUndeclBlockVar(value))
            {
                JavascriptError::ThrowReferenceError(scriptContext, JSERR_UseBeforeDeclaration);
            }
            return value;
        }

        if (BinaryFeatureControl::LanguageService())
        {
            auto authoringData = scriptContext->authoringData;
            if (authoringData && authoringData->Callbacks())
            {
                return authoringData->Callbacks()->GetMissingPropertyResult(scriptContext, RecyclableObject::FromVar(instance), propertyId, Js::TypeIds_GlobalObject);
            }
        }

        const wchar_t* propertyName = scriptContext->GetPropertyName(propertyId)->GetBuffer();

        JavascriptFunction * caller = NULL;
        if (JavascriptStackWalker::GetCaller(&caller, scriptContext))
        {
            FunctionBody * callerBody = caller->GetFunctionBody();
            if (callerBody && callerBody->GetUtf8SourceInfo()->GetIsXDomain())
            {
                propertyName = NULL;
            }
        }

        // Don't error if we disabled implicit calls
        if (scriptContext->GetThreadContext()->RecordImplicitException())
        {
            JavascriptError::ThrowReferenceError(scriptContext, JSERR_UndefVariable, propertyName);
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var JavascriptOperators::OP_GetThisScoped(FrameDisplay *pScope, Var defaultInstance, ScriptContext* scriptContext)
    {
        // NOTE: If changes are made to this logic be sure to update the debuggers as well
        int length = pScope->GetLength();

        for (int i = 0; i < length; i += 1)
        {
            Var value;
            DynamicObject *obj = DynamicObject::FromVar(pScope->GetItem(i));
            if (JavascriptOperators::GetProperty(obj, Js::PropertyIds::_lexicalThisSlotSymbol, &value, scriptContext))
            {
                return value;
            }
        }

        return defaultInstance;
    }

    Var JavascriptOperators::OP_Ld_UnwrapWithObj(Var aValue)
    {
        return RecyclableObject::FromVar(aValue)->GetThisObjectOrUnWrap();
    }
    Var JavascriptOperators::OP_GetInstanceScoped(FrameDisplay *pScope, PropertyId propertyId, Var rootObject, Var* thisVar, ScriptContext* scriptContext)
    {
        // Similar to GetPropertyScoped, but instead of returning the property value, we return the instance that
        // owns it, or the global object if no instance is found.

        int i;
        int length = pScope->GetLength();

        for (i = 0; i < length; i++)
        {
            RecyclableObject *obj = (RecyclableObject*)pScope->GetItem(i);


            if (JavascriptOperators::HasProperty(obj, propertyId))
            {
                // HasProperty will call WithObjects HasProperty which will do the filtering
                // All we have to do here is unwrap the object hence the api call

                *thisVar = obj->GetThisObjectOrUnWrap();
                return *thisVar;
            }
        }

        *thisVar = scriptContext->GetLibrary()->GetUndefined();
        if (rootObject != scriptContext->GetGlobalObject())
        {
            if (JavascriptOperators::OP_HasProperty(rootObject, propertyId, scriptContext))
            {
                return rootObject;
            }
        }

        return scriptContext->GetGlobalObject();
    }

    Var JavascriptOperators::GetPropertyReference(RecyclableObject *instance, PropertyId propertyId, ScriptContext* requestContext)
    {
        Var value = null;
        PropertyValueInfo info;
        if (JavascriptOperators::GetPropertyReference(instance, propertyId, &value, requestContext, &info))
        {
            Assert(value != null);
            return value;
        }
        return requestContext->GetLibrary()->GetUndefined();
    }

    BOOL JavascriptOperators::GetPropertyReference(Var instance, RecyclableObject* propertyObject, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        return GetPropertyReference_Internal(instance, propertyObject, false, propertyId, value, requestContext, info);
    }

    BOOL JavascriptOperators::GetRootPropertyReference(RecyclableObject* instance, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        return GetPropertyReference_Internal(instance, instance, true, propertyId, value, requestContext, info);
    }

    BOOL JavascriptOperators::PropertyReferenceWalkUnscopable(Var instance, RecyclableObject** propertyObject, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return PropertyReferenceWalk_Impl<true>(instance, propertyObject, propertyId, value, info, requestContext);
    }

    BOOL JavascriptOperators::PropertyReferenceWalk(Var instance, RecyclableObject** propertyObject, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return PropertyReferenceWalk_Impl<false>(instance, propertyObject, propertyId, value, info, requestContext);
    }

    template <bool unscopables>
    BOOL JavascriptOperators::PropertyReferenceWalk_Impl(Var instance, RecyclableObject** propertyObject, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL foundProperty = false;
        RecyclableObject* object = *propertyObject;
        while (!foundProperty && JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {
            if (unscopables && JavascriptOperators::IsPropertyUnscopable(object, propertyId))
            {
                break;
            }
            else
            {
                if (object->GetPropertyReference(instance, propertyId, value, info, requestContext))
                {
                    foundProperty = true;
                    break;
                }
            }

            if (object->SkipsPrototype())
            {
                break; // will return false
            }

            object = JavascriptOperators::GetPrototypeNoTrap(object);

        }
        *propertyObject = object;
        return foundProperty;
    }

    BOOL JavascriptOperators::GetPropertyReference_Internal(Var instance, RecyclableObject* propertyObject, const bool isRoot, PropertyId propertyId, Var* value, ScriptContext* requestContext, PropertyValueInfo* info)
    {
        if (TaggedNumber::Is(instance))
        {
            PropertyValueInfo::ClearCacheInfo(info);
        }
        BOOL foundProperty = FALSE;
        RecyclableObject* object = propertyObject;

        if (isRoot)
        {
            foundProperty = RootObjectBase::FromVar(object)->GetRootPropertyReference(instance, propertyId, value, info, requestContext);
        }
        if (!foundProperty)
        {
            foundProperty = PropertyReferenceWalk(instance, &object, propertyId, value, info, requestContext);
        }

        if (!foundProperty)
        {
            return foundProperty;
        }

        if (requestContext->IsUndeclBlockVar(*value))
        {
            JavascriptError::ThrowReferenceError(requestContext, JSERR_UseBeforeDeclaration);
        }
#if DBG
        if (DynamicObject::Is(object))
        {
            DynamicObject* dynamicObject = (DynamicObject*)object;
            DynamicTypeHandler* dynamicTypeHandler = dynamicObject->GetDynamicType()->GetTypeHandler();
            Var property;
            if (dynamicTypeHandler->CheckFixedProperty(requestContext->GetPropertyName(propertyId), &property, requestContext))
            {
                Assert(value == null || *value == property);
            }
        }
#endif
        if (BinaryFeatureControl::LanguageService() && value && GetMissingPropertyValue(requestContext, propertyObject, *value, propertyId))
        {
            return TRUE; // Avoid updating the cache for properties with the value undefined if in the language service.
        }
        CacheOperators::CachePropertyRead(instance, object, isRoot, propertyId, false, info, requestContext);
        return TRUE;
    }

    template <typename PropertyKeyType, bool unscopable>
    DescriptorFlags JavascriptOperators::GetterSetter_Impl(RecyclableObject* instance, PropertyKeyType propertyKey, Var* setterValue, PropertyValueInfo* info, ScriptContext* scriptContext)
    {
        DescriptorFlags flags = None;
        RecyclableObject* object = instance;
        while (flags == None && JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {

            if (unscopable && IsPropertyUnscopable(object, propertyKey))
            {
                break;
            }
            else
            {

                flags = object->GetSetter(propertyKey, setterValue, info, scriptContext);
                if (flags != None)
                {
                    break;
                }
            }
            // TODO: Consider if we should add SkipsPrototype support. DOM has no ES 5 concepts built in that aren't
            // already part of our prototype objects which are Eze objects.
            object = object->GetPrototype();
        }
        return flags;
    }

    DescriptorFlags JavascriptOperators::GetterSetterUnscopable(RecyclableObject* instance, PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* scriptContext)
    {
        return GetterSetter_Impl<PropertyId, true>(instance, propertyId, setterValue, info, scriptContext);
    }

    DescriptorFlags JavascriptOperators::GetterSetter(RecyclableObject* instance, PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* scriptContext)
    {
        return GetterSetter_Impl<PropertyId, false>(instance, propertyId, setterValue, info, scriptContext);
    }

    DescriptorFlags JavascriptOperators::GetterSetter(RecyclableObject* instance, JavascriptString * propertyName, Var* setterValue, PropertyValueInfo* info, ScriptContext* scriptContext)
    {
        return GetterSetter_Impl<JavascriptString*, false>(instance, propertyName, setterValue, info, scriptContext);
    }

    // Checks to see if any object in the prototype chain has a property descriptor for the given property
    // that specifies either an accessor or a non-writable attribute.
    // If TRUE, check flags for details.
    template<typename PropertyKeyType, bool doFastProtoChainCheck, bool isRoot>
    BOOL JavascriptOperators::CheckPrototypesForAccessorOrNonWritablePropertyCore(RecyclableObject* instance,
        PropertyKeyType propertyKey, Var* setterValue, DescriptorFlags* flags, PropertyValueInfo* info, ScriptContext* scriptContext)
    {
        Assert(setterValue);
        Assert(flags);

        // Do a quick check to see if all objects in the prototype chain are known to have only
        // writable data properties (i.e. no accessors or non-writable properties).
        if (doFastProtoChainCheck && CheckIfObjectAndPrototypeChainHasOnlyWritableDataProperties(instance))
        {
            return FALSE;
        }

        if (isRoot)
        {
            *flags = JavascriptOperators::GetRootSetter(instance, propertyKey, setterValue, info, scriptContext);
        }
        if (*flags == None)
        {
            *flags = JavascriptOperators::GetterSetter(instance, propertyKey, setterValue, info, scriptContext);
        }



        return ((*flags & Accessor) == Accessor) || ((*flags & Proxy) == Proxy)|| ((*flags & Data) == Data && (*flags & Writable) == None);
    }

    void JavascriptOperators::OP_InvalidateProtoCaches(PropertyId propertyId, ScriptContext *scriptContext)
    {
        scriptContext->InvalidateProtoCaches(propertyId);
    }

    // Checks to see if any object in the prototype chain has a property descriptor for the given index
    // that specifies either an accessor or a non-writable attribute.
    // If TRUE, check flags for details.
    BOOL JavascriptOperators::CheckPrototypesForAccessorOrNonWritableItem(RecyclableObject* instance, uint32 index,
        Var* setterValue, DescriptorFlags *flags, ScriptContext* scriptContext, BOOL skipPrototypeCheck /* = FALSE */)
    {
        Assert(setterValue);
        Assert(flags);

        // Do a quick walk up the prototype chain to see if any of the prototypes has ever had ANY setter or non-writable property.
        if (CheckIfObjectAndPrototypeChainHasOnlyWritableDataProperties(instance))
        {
            return FALSE;
        }

        RecyclableObject* object = instance;
        while (JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {
            *flags = object->GetItemSetter(index, setterValue, scriptContext);
            if (*flags != None || skipPrototypeCheck)
            {
                break;
            }
            object = object->GetPrototype();
        }

        return ((*flags & Accessor) == Accessor) || ((*flags & Proxy) == Proxy) || ((*flags & Data) == Data && (*flags & Writable) == None);
    }

    BOOL JavascriptOperators::SetGlobalPropertyNoHost(wchar_t const * propertyName, charcount_t propertyLength, Var value, ScriptContext * scriptContext)
    {
        GlobalObject * globalObject = scriptContext->GetGlobalObject();
        uint32 index;
        PropertyRecord const * propertyRecord;
        IndexType indexType = GetIndexTypeFromString(propertyName, propertyLength, scriptContext, &index, &propertyRecord, true);

        if (indexType == IndexType_Number)
        {
            return globalObject->DynamicObject::SetItem(index, value, PropertyOperation_None);
        }
        return globalObject->DynamicObject::SetProperty(propertyRecord->GetPropertyId(), value, PropertyOperation_None, NULL);
    }

    template<typename PropertyKeyType>
    BOOL JavascriptOperators::SetPropertyWPCache(Var receiver, RecyclableObject* object, PropertyKeyType propertyKey, Var newValue, ScriptContext* requestContext, PropertyString * propertyString, PropertyOperationFlags propertyOperationFlags)
    {
        if (receiver)
        {
            Assert(!TaggedNumber::Is(receiver));
            Var setterValueOrProxy = null;
            DescriptorFlags flags = None;
            if (JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(object, propertyKey, &setterValueOrProxy, &flags, NULL, requestContext))
            {
                if ((flags & Accessor) == Accessor)
                {
                    if (JavascriptError::ThrowIfStrictModeUndefinedSetter(propertyOperationFlags, setterValueOrProxy, requestContext))
                    {
                        return TRUE;
                    }
                    if (setterValueOrProxy)
                    {
                        receiver = (RecyclableObject::FromVar(receiver))->GetThisObjectOrUnWrap();
                        RecyclableObject* func = RecyclableObject::FromVar(setterValueOrProxy);
                        JavascriptOperators::CallSetter(func, receiver, newValue, requestContext);
                    }
                    return TRUE;
                }
                else if ((flags & Proxy) == Proxy)
                {
                    Assert(JavascriptProxy::Is(setterValueOrProxy));
                    JavascriptProxy* proxy = JavascriptProxy::FromVar(setterValueOrProxy);
                    auto fn = [&](RecyclableObject* target) -> BOOL {
                        return JavascriptOperators::SetPropertyWPCache(receiver, target, propertyKey, newValue, requestContext, propertyString, propertyOperationFlags);
                    };
                    return proxy->SetPropertyTrap(receiver, JavascriptProxy::SetPropertyTrapKind::SetPropertyWPCacheKind, propertyKey, newValue, requestContext);
                }
                else
                {
                    Assert((flags & Data) == Data && (flags & Writable) == None);

                    JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, requestContext);
                    requestContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
                    return FALSE;
                }
            }
            else if (!JavascriptOperators::IsObject(receiver))
            {
                JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, requestContext);
                return FALSE;
            }

            PropertyValueInfo info;
            // in 9.1.9, step 5, we should return false if receiver is not object, and that will happen in default RecyclableObject operation anyhow.
            RecyclableObject* receiverObject = RecyclableObject::FromVar(receiver);
            if (receiverObject->SetProperty(propertyKey, newValue, propertyOperationFlags, &info))
            {
                if (propertyString != NULL)
                {
                    uint16 slotIndex = info.GetPropertyIndex();
                    if (slotIndex != Constants::NoSlot &&
                        info.GetInstance() == receiverObject &&
                        !object->CanHaveInterceptors() &&
                        requestContext == receiverObject->GetScriptContext() &&
                        (info.GetFlags() != InlineCacheSetterFlag))
                    {
                        uint16 inlineOrAuxSlotIndex;
                        bool isInlineSlot;
                        DynamicObject::FromVar(info.GetInstance())->GetTypeHandler()->PropertyIndexToInlineOrAuxSlotIndex(info.GetPropertyIndex(), &inlineOrAuxSlotIndex, &isInlineSlot);
                        propertyString->UpdateCache(info.GetInstance()->GetType(), inlineOrAuxSlotIndex, isInlineSlot, info.IsStoreFieldCacheEnabled());
                    }
                }
                return TRUE;
            }
        }

        return FALSE;
    }

    BOOL JavascriptOperators::SetItemOnTaggedNumber(Var receiver, RecyclableObject* object, uint32 index, Var newValue, ScriptContext* requestContext,
        PropertyOperationFlags propertyOperationFlags)
    {
        Assert(TaggedNumber::Is(receiver));

        if (requestContext->optimizationOverrides.GetSideEffects() & SideEffects_Accessor)
        {
            Var setterValueOrProxy = null;
            DescriptorFlags flags = None;
            if (object == nullptr)
            {
                GetPropertyObject(receiver, requestContext, &object);
            }
            if (JavascriptOperators::CheckPrototypesForAccessorOrNonWritableItem(object, index, &setterValueOrProxy, &flags, requestContext))
            {
                if ((flags & Accessor) == Accessor)
                {
                    if (JavascriptError::ThrowIfStrictModeUndefinedSetter(propertyOperationFlags, setterValueOrProxy, requestContext))
                    {
                        return TRUE;
                    }
                    if (setterValueOrProxy)
                    {
                        RecyclableObject* func = RecyclableObject::FromVar(setterValueOrProxy);
                        JavascriptOperators::CallSetter(func, receiver, newValue, requestContext);
                        return TRUE;
                    }
                }
                else if ((flags & Proxy) == Proxy)
                {
                    Assert(JavascriptProxy::Is(setterValueOrProxy));
                    JavascriptProxy* proxy = JavascriptProxy::FromVar(setterValueOrProxy);
                    const PropertyRecord* propertyRecord;
                    proxy->PropertyIdFromInt(index, &propertyRecord);
                    return proxy->SetPropertyTrap(receiver, JavascriptProxy::SetPropertyTrapKind::SetItemOnTaggedNumberKind, propertyRecord->GetPropertyId(), newValue, requestContext);
                }
                else
                {
                    Assert((flags & Data) == Data && (flags & Writable) == None);
                    JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, requestContext);
                }
            }
        }

        JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, requestContext);
        return FALSE;
    }

    BOOL JavascriptOperators::SetPropertyOnTaggedNumber(Var receiver, RecyclableObject* object, PropertyId propertyId, Var newValue, ScriptContext* requestContext,
        PropertyOperationFlags propertyOperationFlags)
    {
        Assert (TaggedNumber::Is(receiver));

        if (requestContext->optimizationOverrides.GetSideEffects() & SideEffects_Accessor)
        {
            Var setterValueOrProxy = null;
            PropertyValueInfo info;
            DescriptorFlags flags = None;
            if (object == nullptr)
            {
                GetPropertyObject(receiver, requestContext, &object);
            }
            if (JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(object, propertyId, &setterValueOrProxy, &flags, &info, requestContext))
            {
                if ((flags & Accessor) == Accessor)
                {
                    if (JavascriptError::ThrowIfStrictModeUndefinedSetter(propertyOperationFlags, setterValueOrProxy, requestContext))
                    {
                        return TRUE;
                    }
                    if (setterValueOrProxy)
                    {
                        RecyclableObject* func = RecyclableObject::FromVar(setterValueOrProxy);
                        Assert(info.GetFlags() == InlineCacheSetterFlag || info.GetPropertyIndex() == Constants::NoSlot);
                        JavascriptOperators::CallSetter(func, receiver, newValue, requestContext);
                        return TRUE;
                    }
                }
                else if ((flags & Proxy) == Proxy)
                {
                    Assert(JavascriptProxy::Is(setterValueOrProxy));
                    JavascriptProxy* proxy = JavascriptProxy::FromVar(setterValueOrProxy);
                    return proxy->SetPropertyTrap(receiver, JavascriptProxy::SetPropertyTrapKind::SetPropertyOnTaggedNumberKind, propertyId, newValue, requestContext);
                }
                else //TODO: This won't enter, because no SideEffects_NonWritable
                {
                    Assert((flags & Data) == Data && (flags & Writable) == None);
                    JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, requestContext);
                }
            }
        }

        // Add implicit call flags, to bail out if field copy prop may propagate the wrong value.
        requestContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
        JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, requestContext);
        return FALSE;
    }

    BOOL JavascriptOperators::SetPropertyUnscopable(Var instance, RecyclableObject* receiver, PropertyId propertyId, Var newValue, PropertyValueInfo * info, ScriptContext* requestContext, PropertyOperationFlags propertyOperationFlags)
    {
        return SetProperty_Internal<true>(instance, receiver, false, propertyId, newValue, info, requestContext, propertyOperationFlags);
    }

    BOOL JavascriptOperators::SetProperty(Var receiver, RecyclableObject* object, PropertyId propertyId, Var newValue, PropertyValueInfo * info, ScriptContext* requestContext, PropertyOperationFlags propertyOperationFlags)
    {
        return SetProperty_Internal<false>(receiver, object, false, propertyId, newValue, info, requestContext, propertyOperationFlags);
    }

    BOOL JavascriptOperators::SetRootProperty(RecyclableObject* instance, PropertyId propertyId, Var newValue, PropertyValueInfo * info, ScriptContext* requestContext, PropertyOperationFlags propertyOperationFlags)
    {
        return SetProperty_Internal<false>(instance, instance, true, propertyId, newValue, info, requestContext, propertyOperationFlags);
    }

    template <bool unscopables>
    BOOL JavascriptOperators::SetProperty_Internal(Var receiver, RecyclableObject* object, const bool isRoot, PropertyId propertyId, Var newValue, PropertyValueInfo * info, ScriptContext* requestContext, PropertyOperationFlags propertyOperationFlags)
    {
        if (receiver)
        {
            Assert(!TaggedNumber::Is(receiver));
            Var setterValueOrProxy = null;
            DescriptorFlags flags = None;
            if ((isRoot && JavascriptOperators::CheckPrototypesForAccessorOrNonWritableRootProperty(object, propertyId, &setterValueOrProxy, &flags, info, requestContext)) ||
                (!isRoot && JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(object, propertyId, &setterValueOrProxy, &flags, info, requestContext)))
            {
                if ((flags & Accessor) == Accessor)
                {
                    if (JavascriptError::ThrowIfStrictModeUndefinedSetter(propertyOperationFlags, setterValueOrProxy, requestContext) ||
                        JavascriptError::ThrowIfNotExtensibleUndefinedSetter(propertyOperationFlags, setterValueOrProxy, requestContext))
                    {
                        return TRUE;
                    }
                    if (setterValueOrProxy)
                    {
                        RecyclableObject* func = RecyclableObject::FromVar(setterValueOrProxy);
                        Assert(!info || info->GetFlags() == InlineCacheSetterFlag || info->GetPropertyIndex() == Constants::NoSlot);

                        if (WithScopeObject::Is(receiver))
                        {
                            receiver = (RecyclableObject::FromVar(receiver))->GetThisObjectOrUnWrap();
                        }
                        else
                        {
                            CacheOperators::CachePropertyWrite(RecyclableObject::FromVar(receiver), isRoot, object->GetType(), propertyId, info, requestContext);
                        }

                        if (MutationBreakpoint::IsFeatureEnabled(requestContext))
                        {
                            MutationBreakpoint::HandleSetProperty(requestContext, object, propertyId, newValue);
                        }
                        JavascriptOperators::CallSetter(func, receiver, newValue, requestContext);
                    }
                    return TRUE;
                }
                else if ((flags & Proxy) == Proxy)
                {
                    Assert(JavascriptProxy::Is(setterValueOrProxy));
                    JavascriptProxy* proxy = JavascriptProxy::FromVar(setterValueOrProxy);
                    // We can't cache the property at this time. both target and handler can be changed outside of the proxy, so the inline cache needs to be
                    // invalidate when target, handler, or handler prototype has changed. We don't have a way to achieve this yet.
                    PropertyValueInfo::SetNoCache(info, proxy);
                    PropertyValueInfo::DisablePrototypeCache(info, proxy); // We can't cache prototype property either

                    // TODO Add unscopeables check here when es6 spec is clarified
                    return proxy->SetPropertyTrap(receiver, JavascriptProxy::SetPropertyTrapKind::SetPropertyKind, propertyId, newValue, requestContext);
                }
                else
                {
                    Assert((flags & Data) == Data && (flags & Writable) == None);
                    if (flags & Const)
                    {
                        JavascriptError::ThrowReferenceError(requestContext, ERRAssignmentToConst);
                    }
                    JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, requestContext);
                    return FALSE;
                }
            } 
            else if (!JavascriptOperators::IsObject(receiver))
            {
                JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, requestContext);
                return FALSE;
            }

            // Break on mutation if needed
            bool doNotUpdateCacheForMbp = MutationBreakpoint::IsFeatureEnabled(requestContext) ?
                MutationBreakpoint::HandleSetProperty(requestContext, object, propertyId, newValue) : false;

            // Get the original type before setting the property
            Type *typeWithoutProperty = object->GetType();
            BOOL didSetProperty = false;
            if (isRoot)
            {
                AssertMsg(JavascriptOperators::GetTypeId(receiver) == TypeIds_GlobalObject
                    || JavascriptOperators::GetTypeId(receiver) == TypeIds_ModuleRoot,
                    "Root must be a global object!");

                RootObjectBase* rootObject = static_cast<RootObjectBase*>(receiver);
                didSetProperty = rootObject->SetRootProperty(propertyId, newValue, propertyOperationFlags, info);
            }
            else
            {
                RecyclableObject* instanceObject = RecyclableObject::FromVar(receiver);
                while (JavascriptOperators::GetTypeId(instanceObject) != TypeIds_Null)
                {
                    if (unscopables && JavascriptOperators::IsPropertyUnscopable(instanceObject, propertyId))
                    {
                        break;
                    }
                    else
                    {
                        didSetProperty = instanceObject->SetProperty(propertyId, newValue, propertyOperationFlags, info);
                        if (didSetProperty || !unscopables)
                        {
                            break;
                        }
                    }
                    instanceObject = JavascriptOperators::GetPrototypeNoTrap(instanceObject);
                }
            }

            if (didSetProperty)
            {
                bool updateCache = true;
                if (BinaryFeatureControl::LanguageService())
                {
                    TypeId typeId = JavascriptOperators::GetTypeId(newValue);
                    if (typeId == TypeIds_Null || typeId == TypeIds_Undefined)
                    {
                        // Get might replace null and undefined with tracking values. Avoid updating
                        // the cache to allow GetProperty() to trigger.
                        updateCache = false;
                    }
                }
                updateCache = updateCache && !doNotUpdateCacheForMbp;

                if (updateCache)
                {
                    if (!JavascriptProxy::Is(receiver))
                    {
                        CacheOperators::CachePropertyWrite(RecyclableObject::FromVar(receiver), isRoot, typeWithoutProperty, propertyId, info, requestContext);
                    }
                }
                return TRUE;
            }
        }

        return FALSE;
    }

    BOOL JavascriptOperators::IsNumberFromNativeArray(Var instance, uint32 index, ScriptContext* scriptContext)
    {
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(instance);
        Js::TypeId instanceType = JavascriptOperators::GetTypeId(instance);
        // Fast path for native and typed arrays.
        if ( (instanceType == TypeIds_NativeIntArray || instanceType == TypeIds_NativeFloatArray) || (instanceType >= TypeIds_Int8Array && instanceType <= TypeIds_Uint64Array) )
        {
            RecyclableObject* object = RecyclableObject::FromVar(instance);
            Var member;

            // If the item is found in the array own body, then it is a number
            if (JavascriptOperators::GetOwnItem(object, index, &member, scriptContext))
                return TRUE;
        }
        return FALSE;
    }

    BOOL JavascriptOperators::GetAccessors(RecyclableObject* instance, PropertyId propertyId, ScriptContext* requestContext, Var* getter, Var* setter)
    {
        RecyclableObject* object = instance;

        while (JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {
            if (object->GetAccessors(propertyId, getter, setter, requestContext))
            {
                *getter = JavascriptOperators::CanonicalizeAccessor(*getter, requestContext);
                *setter = JavascriptOperators::CanonicalizeAccessor(*setter, requestContext);
                return TRUE;
            }

            if (object->SkipsPrototype())
            {
                break;
            }
            object = JavascriptOperators::GetPrototypeNoTrap(object);
        }
        return FALSE;
    }

    BOOL JavascriptOperators::SetAccessors(RecyclableObject* instance, PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        BOOL result = instance && instance->SetAccessors(propertyId, getter, setter, flags);
        return result;
    }

    BOOL JavascriptOperators::OP_SetProperty(Var instance, PropertyId propertyId, Var newValue, ScriptContext* scriptContext, PropertyValueInfo * info, PropertyOperationFlags flags)
    {
        // TODO:  version for ES5.
        // The call into ToObject(dynamicObject) is avoided here by checking for null and undefined and doing nothing when dynamicObject is a primitive value.
        // In ES5 this optimization has to be reviewed because the propertyid could be a setter with side effect in 'primitive'.prototype
        TypeId typeId = JavascriptOperators::GetTypeId(instance);

        if (typeId == TypeIds_Null || typeId == TypeIds_Undefined)
        {
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotSet_NullOrUndefined, scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            return TRUE;
        }
        else if (typeId == TypeIds_VariantDate)
        {
            // REVIEW: Why do we have this extra check for VariantData here while JavascriptVariantDate::SetProperty already throws?

            // In IE8 jscript, at least VT_DATE variants error when trying to set properties instead of failing silently
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_VarDate, scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            return TRUE;
        }

        if (!TaggedNumber::Is(instance))
        {
            return JavascriptOperators::SetProperty(RecyclableObject::FromVar(instance), RecyclableObject::FromVar(instance), propertyId, newValue, info, scriptContext, flags);
        }

        JavascriptError::ThrowCantAssignIfStrictMode(flags, scriptContext);
        return false;
    }

    BOOL JavascriptOperators::OP_StFunctionExpression(Var obj, PropertyId propertyId, Var newValue)
    {
        RecyclableObject* instance = RecyclableObject::FromVar(obj);

        instance->SetProperty(propertyId, newValue, PropertyOperation_None, NULL);
        instance->SetWritable(propertyId, FALSE);
        instance->SetConfigurable(propertyId, FALSE);

        return TRUE;
    }

    BOOL JavascriptOperators::OP_InitLetProperty(Var obj, PropertyId propertyId, Var newValue)
    {
        RecyclableObject* instance = RecyclableObject::FromVar(obj);

        PropertyOperationFlags flags = instance->GetScriptContext()->IsUndeclBlockVar(newValue) ? PropertyOperation_SpecialValue : PropertyOperation_None;
        PropertyAttributes attributes = PropertyLetDefaults;

        if (RootObjectBase::Is(instance))
        {
            attributes |= PropertyLetConstGlobal;
        }

        instance->SetPropertyWithAttributes(propertyId, newValue, attributes, NULL, (PropertyOperationFlags)(flags | PropertyOperation_AllowUndecl));

        return TRUE;
    }

    BOOL JavascriptOperators::OP_InitConstProperty(Var obj, PropertyId propertyId, Var newValue)
    {
        RecyclableObject* instance = RecyclableObject::FromVar(obj);

        PropertyOperationFlags flags = instance->GetScriptContext()->IsUndeclBlockVar(newValue) ? PropertyOperation_SpecialValue : PropertyOperation_None;
        PropertyAttributes attributes = PropertyConstDefaults;

        if (RootObjectBase::Is(instance))
        {
            attributes |= PropertyLetConstGlobal;
        }

        instance->SetPropertyWithAttributes(propertyId, newValue, attributes, NULL, (PropertyOperationFlags)(flags | PropertyOperation_AllowUndecl));

        return TRUE;
    }

    BOOL JavascriptOperators::OP_InitUndeclRootLetProperty(Var obj, PropertyId propertyId)
    {
        RecyclableObject* instance = RecyclableObject::FromVar(obj);

        PropertyOperationFlags flags = static_cast<PropertyOperationFlags>(PropertyOperation_SpecialValue | PropertyOperation_AllowUndecl);
        PropertyAttributes attributes = PropertyLetDefaults | PropertyLetConstGlobal;

        instance->SetPropertyWithAttributes(propertyId, instance->GetLibrary()->GetUndeclBlockVar(), attributes, NULL, flags);

        return TRUE;
    }

    BOOL JavascriptOperators::OP_InitUndeclRootConstProperty(Var obj, PropertyId propertyId)
    {
        RecyclableObject* instance = RecyclableObject::FromVar(obj);

        PropertyOperationFlags flags = static_cast<PropertyOperationFlags>(PropertyOperation_SpecialValue | PropertyOperation_AllowUndecl);
        PropertyAttributes attributes = PropertyConstDefaults | PropertyLetConstGlobal;

        instance->SetPropertyWithAttributes(propertyId, instance->GetLibrary()->GetUndeclBlockVar(), attributes, NULL, flags);

        return TRUE;
    }

    BOOL JavascriptOperators::InitProperty(RecyclableObject* instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags)
    {
        return instance && instance->InitProperty(propertyId, newValue, flags);
    }

    BOOL JavascriptOperators::OP_InitProperty(Var instance, PropertyId propertyId, Var newValue)
    {
        if(TaggedNumber::Is(instance)) { return false; }
        return JavascriptOperators::InitProperty(RecyclableObject::FromVar(instance), propertyId, newValue);
    }

    BOOL JavascriptOperators::DeleteProperty(RecyclableObject* instance, PropertyId propertyId, PropertyOperationFlags propertyOperationFlags)
    {
        return DeleteProperty_Impl<false>(instance, propertyId, propertyOperationFlags);
    }
    BOOL JavascriptOperators::DeletePropertyUnscopables(RecyclableObject* instance, PropertyId propertyId, PropertyOperationFlags propertyOperationFlags)
    {
        return DeleteProperty_Impl<true>(instance, propertyId, propertyOperationFlags);
    }
    template<bool unscopables>
    BOOL JavascriptOperators::DeleteProperty_Impl(RecyclableObject* instance, PropertyId propertyId, PropertyOperationFlags propertyOperationFlags)
    {
        while (JavascriptOperators::GetTypeId(instance) != TypeIds_Null)
        {
            if (unscopables && JavascriptOperators::IsPropertyUnscopable(instance, propertyId))
            {
                break;
            }
            else
            {
                ScriptContext *scriptContext = instance->GetScriptContext();
                if (MutationBreakpoint::IsFeatureEnabled(scriptContext)
                    && scriptContext->HasMutationBreakpoints())
                {
                    MutationBreakpoint::HandleDeleteProperty(scriptContext, instance, propertyId);
                }

                // !unscopables will hit the return statement on the first iteration
                return instance->DeleteProperty(propertyId, propertyOperationFlags);
            }
            instance = JavascriptOperators::GetPrototypeNoTrap(instance);
        }
        return false;
    }

    Var JavascriptOperators::OP_DeleteProperty(Var instance, PropertyId propertyId, ScriptContext* scriptContext, PropertyOperationFlags propertyOperationFlags)
    {
        if(TaggedNumber::Is(instance))
        {
            return scriptContext->GetLibrary()->GetTrue();
        }

        TypeId typeId = JavascriptOperators::GetTypeId(instance);
        if (typeId == TypeIds_Null || typeId == TypeIds_Undefined)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotDelete_NullOrUndefined,
                scriptContext->GetPropertyName(propertyId)->GetBuffer());
        }

        RecyclableObject *recyclableObject = RecyclableObject::FromVar(instance);
        if (ScriptFunction::Is(instance) && ScriptFunction::FromVar(instance)->GetHomeObj() == instance)
        {
            // Only do a prop lookup on a class constructor.
            if (Var prop = JavascriptOperators::GetProperty(recyclableObject, propertyId, scriptContext))
            {
                if (ScriptFunction::Is(prop))
                {
                    // Properties that reference super cannot be deleted.
                    ScriptFunction *scriptFunction = ScriptFunction::FromVar(prop);
                    if (scriptFunction->HasSuperReference())
                    {
                        JavascriptError::ThrowReferenceError(scriptContext, JSERR_DeletePropertyWithSuper,
                            scriptContext->GetPropertyName(propertyId)->GetBuffer());
                    }
                }
            }
        }

        return scriptContext->GetLibrary()->CreateBoolean(
            JavascriptOperators::DeleteProperty(recyclableObject, propertyId, propertyOperationFlags));
    }

    Var JavascriptOperators::OP_DeleteRootProperty(Var instance, PropertyId propertyId, ScriptContext* scriptContext, PropertyOperationFlags propertyOperationFlags)
    {
        AssertMsg(RootObjectBase::Is(instance), "Root must be a global object!");
        RootObjectBase* rootObject = static_cast<RootObjectBase*>(instance);

        return scriptContext->GetLibrary()->CreateBoolean(
            rootObject->DeleteRootProperty(propertyId, propertyOperationFlags));
    }

    template <bool IsFromFullJit, class TInlineCache>
    __inline void JavascriptOperators::PatchSetPropertyScoped(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var newValue, Var defaultInstance, PropertyOperationFlags propertyOperationFlags)
    {
        // Set the property using a scope stack rather than an individual instance.
        // Walk the stack until we find an instance that has the property and store
        // the new value there.

        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        uint16 length = pDisplay->GetLength();
        DynamicObject *object;

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        for (uint16 i = 0; i < length; i++)
        {
            object = (DynamicObject*)pDisplay->GetItem(i);
            Type* type = object->GetType();
            if (CacheOperators::TrySetProperty<true, true, true, true, true, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                    object, false, propertyId, newValue, scriptContext, propertyOperationFlags, nullptr, &info))
            {
                return;
            }

            // In scoped set property, we need to set the property when it is available; it could be a setter
            // or normal property. we need to check setter first, and if no setter is available, but HasProperty
            // is true, this must be a normal property.
            // TODO: merge OP_HasProperty and GetSetter in one pass if there is perf problem. In fastDOM we have quite
            // a lot of setters so separating the two might be actually faster.
            Var setterValueOrProxy = null;
            DescriptorFlags flags = None;
            if (JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(object, propertyId, &setterValueOrProxy, &flags, &info, scriptContext))
            {
                if ((flags & Accessor) == Accessor)
                {
                    if (setterValueOrProxy)
                    {
                        JavascriptFunction* func = (JavascriptFunction*)setterValueOrProxy;
                        Assert(info.GetFlags() == InlineCacheSetterFlag || info.GetPropertyIndex() == Constants::NoSlot);
                        CacheOperators::CachePropertyWrite(object, false, type, propertyId, &info, scriptContext);
                        JavascriptOperators::CallSetter(func, object, newValue, scriptContext);
                    }
                    return;
                }
                else if ((flags & Proxy) == Proxy)
                {
                    Assert(JavascriptProxy::Is(setterValueOrProxy));
                    JavascriptProxy* proxy = JavascriptProxy::FromVar(setterValueOrProxy);
                    auto fn = [&](RecyclableObject* target) -> BOOL {
                        return JavascriptOperators::SetProperty(object, target, propertyId, newValue, scriptContext, propertyOperationFlags);
                    };
                    // We can't cache the property at this time. both target and handler can be changed outside of the proxy, so the inline cache needs to be
                    // invalidate when target, handler, or handler prototype has changed. We don't have a way to achieve this yet.
                    PropertyValueInfo::SetNoCache(&info, proxy);
                    PropertyValueInfo::DisablePrototypeCache(&info, proxy); // We can't cache prototype property either
                    proxy->SetPropertyTrap(object, JavascriptProxy::SetPropertyTrapKind::SetPropertyKind, propertyId, newValue, scriptContext);
                }
                else
                {
                    Assert((flags & Data) == Data && (flags & Writable) == None);
                    if (flags & Const)
                    {
                        JavascriptError::ThrowReferenceError(scriptContext, ERRAssignmentToConst);
                    }
                    return;
                }
            }
            else if (!JavascriptOperators::IsObject(object))
            {
                JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, scriptContext);
            }

            // Need to do a "get" of the current value (if any) to make sure that we're not writing to
            // let/const before declaration, but we need to disable implicit calls around the "get",
            // so we need to do a "has" first to make sure the "get" is valid (e.g., "get" on a HostDispatch
            // with implicit calls disabled will always "succeed").
            if (JavascriptOperators::HasProperty(object, propertyId))
            {
                if (scriptContext->GetConfig()->IsLetAndConstEnabled())
                {
                    DisableImplicitFlags disableImplicitFlags =
                        scriptContext->GetThreadContext()->GetDisableImplicitFlags();
                    scriptContext->GetThreadContext()->SetDisableImplicitFlags(DisableImplicitCallAndExceptionFlag);

                    Var value;
                    BOOL result = JavascriptOperators::GetProperty(object, propertyId, &value, scriptContext, null);

                    scriptContext->GetThreadContext()->SetDisableImplicitFlags(disableImplicitFlags);

                    if (result && scriptContext->IsUndeclBlockVar(value))
                    {
                        JavascriptError::ThrowReferenceError(scriptContext, JSERR_UseBeforeDeclaration);
                    }
                }
                PropertyValueInfo info;
                PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
                object->SetProperty(propertyId, newValue, PropertyOperation_None, &info);

#if DBG_DUMP
                if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
                {
                    CacheOperators::TraceCache(inlineCache, L"PatchSetPropertyScoped", propertyId, scriptContext, object);
                }
#endif
                if (!JavascriptProxy::Is(object))
                {
                    CacheOperators::CachePropertyWrite(object, false, type, propertyId, &info, scriptContext);
                }
                return;
            }
        }

        // No one in the scope stack has the property, so add it to the default instance provided by the caller.
        AssertMsg(!TaggedNumber::Is(defaultInstance), "Root object is an int or tagged float?");
        Assert(defaultInstance != null);
        RecyclableObject* obj = RecyclableObject::FromVar(defaultInstance);
        {
            //SetPropertyScoped does not use inline cache for default instance
            PropertyValueInfo info;
            JavascriptOperators::SetRootProperty(obj, propertyId, newValue, &info, scriptContext, (PropertyOperationFlags)(propertyOperationFlags | PropertyOperation_Root));
        }
    }
    template void JavascriptOperators::PatchSetPropertyScoped<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var newValue, Var defaultInstance, PropertyOperationFlags propertyOperationFlags);
    template void JavascriptOperators::PatchSetPropertyScoped<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var newValue, Var defaultInstance, PropertyOperationFlags propertyOperationFlags);
    template void JavascriptOperators::PatchSetPropertyScoped<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var newValue, Var defaultInstance, PropertyOperationFlags propertyOperationFlags);
    template void JavascriptOperators::PatchSetPropertyScoped<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var newValue, Var defaultInstance, PropertyOperationFlags propertyOperationFlags);

    BOOL JavascriptOperators::OP_InitFuncScoped(FrameDisplay *pScope, PropertyId propertyId, Var newValue, Var defaultInstance, ScriptContext* scriptContext)
    {
        int i;
        int length = pScope->GetLength();
        DynamicObject *obj;

        for (i = 0; i < length; i++)
        {
            obj = (DynamicObject*)pScope->GetItem(i);

            if (obj->InitFuncScoped(propertyId, newValue))
            {
                return TRUE;
            }
        }

        AssertMsg(!TaggedNumber::Is(defaultInstance), "Root object is an int or tagged float?");
        return RecyclableObject::FromVar(defaultInstance)->InitFuncScoped(propertyId, newValue);
    }

    BOOL JavascriptOperators::OP_InitPropertyScoped(FrameDisplay *pScope, PropertyId propertyId, Var newValue, Var defaultInstance, ScriptContext* scriptContext)
    {
        int i;
        int length = pScope->GetLength();
        DynamicObject *obj;

        for (i = 0; i < length; i++)
        {
            obj = (DynamicObject*)pScope->GetItem(i);
            if (obj->InitPropertyScoped(propertyId, newValue))
            {
                return TRUE;
            }
        }

        AssertMsg(!TaggedNumber::Is(defaultInstance), "Root object is an int or tagged float?");
        return RecyclableObject::FromVar(defaultInstance)->InitPropertyScoped(propertyId, newValue);
    }

    Var JavascriptOperators::OP_DeletePropertyScoped(
        FrameDisplay *pScope,
        PropertyId propertyId,
        Var defaultInstance,
        ScriptContext* scriptContext,
        PropertyOperationFlags propertyOperationFlags)
    {
        int i;
        int length = pScope->GetLength();

        for (i = 0; i < length; i++)
        {
            DynamicObject *obj = (DynamicObject*)pScope->GetItem(i);
            if (JavascriptOperators::HasProperty(obj, propertyId))
            {
                return scriptContext->GetLibrary()->CreateBoolean(JavascriptOperators::DeleteProperty(obj, propertyId, propertyOperationFlags));
            }
        }

        return JavascriptOperators::OP_DeleteRootProperty(RecyclableObject::FromVar(defaultInstance), propertyId, scriptContext, propertyOperationFlags);
    }

    Var JavascriptOperators::OP_TypeofPropertyScoped(FrameDisplay *pScope, PropertyId propertyId, Var defaultInstance, ScriptContext* scriptContext)
    {
        int i;
        int length = pScope->GetLength();

        for (i = 0; i < length; i++)
        {
            DynamicObject *obj = (DynamicObject*)pScope->GetItem(i);
            if (JavascriptOperators::HasProperty(obj, propertyId))
            {
                return JavascriptOperators::TypeofFld(obj, propertyId, scriptContext);
            }
        }

        return JavascriptOperators::TypeofRootFld(RecyclableObject::FromVar(defaultInstance), propertyId, scriptContext);
    }

    BOOL JavascriptOperators::HasOwnItem(RecyclableObject* object, uint32 index)
    {
        return object->HasOwnItem(index);
    }

    BOOL JavascriptOperators::HasItem(RecyclableObject* object, uint32 index)
    {
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(object);
        while (JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {
            if (object->HasItem(index))
            {
                return true;
            }
            // CONSIDER: Numeric property values shouldn't be on the prototype for now but if this changes
            // we should add SkipsPrototype support here as well
            object = JavascriptOperators::GetPrototypeNoTrap(object);
        }
        return false;
    }

    BOOL JavascriptOperators::GetOwnItem(RecyclableObject* object, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return object->GetItem(object, index, value, requestContext);
    }

    BOOL JavascriptOperators::GetItem(Var instance, RecyclableObject* propertyObject, uint32 index, Var* value, ScriptContext* requestContext)
    {
        RecyclableObject* object = propertyObject;
        while (JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {
            if (object->GetItem(instance, index, value, requestContext))
            {
                if (BinaryFeatureControl::LanguageService() && value)
                    GetMissingItemValue(requestContext, propertyObject, *value, index);
                return true;
            }
            if (object->SkipsPrototype())
            {
                break;
            }
            object = JavascriptOperators::GetPrototypeNoTrap(object);
        }
        return false;
    }

    BOOL JavascriptOperators::GetItemReference(Var instance, RecyclableObject* propertyObject, uint32 index, Var* value, ScriptContext* requestContext)
    {
        RecyclableObject* object = propertyObject;
        while (JavascriptOperators::GetTypeId(object) != TypeIds_Null)
        {
            if (object->GetItemReference(instance, index, value, requestContext))
            {
                if (BinaryFeatureControl::LanguageService() && value)
                    GetMissingItemValue(requestContext, propertyObject, *value, index);
                return true;
            }
            if (object->SkipsPrototype())
            {
                break;
            }
            object = JavascriptOperators::GetPrototypeNoTrap(object);
        }
        return false;
    }

    BOOL JavascriptOperators::SetItem(Var receiver, RecyclableObject* object, uint32 index, Var value, ScriptContext* scriptContext, PropertyOperationFlags propertyOperationFlags, BOOL skipPrototypeCheck /* = FALSE */)
    {
        Var setterValueOrProxy = null;
        DescriptorFlags flags = None;
        Assert(!TaggedNumber::Is(receiver));
        if (JavascriptOperators::CheckPrototypesForAccessorOrNonWritableItem(object, index, &setterValueOrProxy, &flags, scriptContext, skipPrototypeCheck))
        {
            scriptContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
            if ((flags & Accessor) == Accessor)
            {
                if (JavascriptError::ThrowIfStrictModeUndefinedSetter(propertyOperationFlags, setterValueOrProxy, scriptContext) ||
                    JavascriptError::ThrowIfNotExtensibleUndefinedSetter(propertyOperationFlags, setterValueOrProxy, scriptContext))
                {
                    return TRUE;
                }
                if (setterValueOrProxy)
                {
                    RecyclableObject* func = RecyclableObject::FromVar(setterValueOrProxy);
                    JavascriptOperators::CallSetter(func, receiver, value, scriptContext);
                }
                return TRUE;
            }
            else if ((flags & Proxy) == Proxy)
            {
                Assert(JavascriptProxy::Is(setterValueOrProxy));
                JavascriptProxy* proxy = JavascriptProxy::FromVar(setterValueOrProxy);
                const PropertyRecord* propertyRecord;
                proxy->PropertyIdFromInt(index, &propertyRecord);
                return proxy->SetPropertyTrap(receiver, JavascriptProxy::SetPropertyTrapKind::SetItemKind, propertyRecord->GetPropertyId(), value, scriptContext, skipPrototypeCheck);
            }
            else
            {
                Assert((flags & Data) == Data && (flags & Writable) == None);
                if ((propertyOperationFlags & PropertyOperationFlags::PropertyOperation_ThrowIfNotExtensible) == PropertyOperationFlags::PropertyOperation_ThrowIfNotExtensible)
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_NonExtensibleObject);
                }

                JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, scriptContext);
                return FALSE;
            }
        }
        else if (!JavascriptOperators::IsObject(receiver))
        {
            JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, scriptContext);
            return FALSE;
        }

        return (RecyclableObject::FromVar(receiver))->SetItem(index, value, propertyOperationFlags);
    }

    BOOL JavascriptOperators::DeleteItem(RecyclableObject* object, uint32 index, PropertyOperationFlags propertyOperationFlags)
    {
        return object->DeleteItem(index, propertyOperationFlags);
    }

    BOOL JavascriptOperators::OP_HasItem(Var instance, Var index, ScriptContext* scriptContext)
    {
        RecyclableObject* object = TaggedNumber::Is(instance) ?
            scriptContext->GetLibrary()->GetNumberPrototype() :
            RecyclableObject::FromVar(instance);

        uint32 indexVal;
        PropertyRecord const * propertyRecord;
        // TODO: Perf optimization: If IsStringTypeHandler is true, convert indexVal to string instead of PropertyRecord and let
        // type handler decide whether to turn it into a string or not.  Perf is gained if PropertyRecords are avoided altogether
        // when dealing with a string based type handler.
        bool createIfNotFound = DynamicType::Is(object->GetTypeId()) && static_cast<DynamicObject*>(object)->GetTypeHandler()->IsStringTypeHandler();
        if (GetIndexType(index, scriptContext, &indexVal, &propertyRecord, createIfNotFound) == IndexType_Number)
        {
            return HasItem(object, indexVal);
        }
        else
        {
            return propertyRecord && HasProperty(object, propertyRecord->GetPropertyId());
        }
    }

    void JavascriptOperators::UpdateNativeArrayProfileInfoToCreateVarArray(Var instance, const bool expectingNativeFloatArray, const bool expectingVarArray)
    {
        Assert(instance);
        Assert(expectingNativeFloatArray ^ expectingVarArray);

        if (!JavascriptNativeArray::Is(instance))
        {
            return;
        }

        ArrayCallSiteInfo *const arrayCallSiteInfo = JavascriptNativeArray::FromVar(instance)->GetArrayCallSiteInfo();
        if (!arrayCallSiteInfo)
        {
            return;
        }

        if (expectingNativeFloatArray)
        {
            // Profile data is expecting a native float array. Ensure that at the array's creation site, that a native int array
            // is not created, such that the profiled array type would be correct.
            arrayCallSiteInfo->SetIsNotNativeIntArray();
        }
        else
        {
            // Profile data is expecting a var array. Ensure that at the array's creation site, that a native array is not
            // created, such that the profiled array type would be correct.
            Assert(expectingVarArray);
            arrayCallSiteInfo->SetIsNotNativeArray();
        }
    }

    bool JavascriptOperators::SetElementMayHaveImplicitCalls(ScriptContext *const scriptContext)
    {
        return
            scriptContext->optimizationOverrides.GetArraySetElementFastPathVtable() ==
                ScriptContextOptimizationOverrideInfo::InvalidVtable;
    }

    RecyclableObject *JavascriptOperators::GetCallableObjectOrThrow(const Var callee, ScriptContext *const scriptContext)
    {
        Assert(callee);
        Assert(scriptContext);

        if (TaggedNumber::Is(callee))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction /* TODO-ERROR: get arg name - aFunc */);
        }
        return RecyclableObject::FromVar(callee);
    }

    Var JavascriptOperators::OP_GetElementI_JIT(Var instance, Var index, ScriptContext *scriptContext)
    {
        Assert(Js::JavascriptStackWalker::ValidateTopJitFrame(scriptContext));

        return OP_GetElementI(instance, index, scriptContext);
    }

    Var JavascriptOperators::OP_GetElementI_JIT_ExpectingNativeFloatArray(Var instance, Var index, ScriptContext *scriptContext)
    {
        Assert(Js::JavascriptStackWalker::ValidateTopJitFrame(scriptContext));

        UpdateNativeArrayProfileInfoToCreateVarArray(instance, true, false);
        return OP_GetElementI_JIT(instance, index, scriptContext);
    }

    Var JavascriptOperators::OP_GetElementI_JIT_ExpectingVarArray(Var instance, Var index, ScriptContext *scriptContext)
    {
        Assert(Js::JavascriptStackWalker::ValidateTopJitFrame(scriptContext));


        UpdateNativeArrayProfileInfoToCreateVarArray(instance, false, true);
        return OP_GetElementI_JIT(instance, index, scriptContext);
    }

    Var JavascriptOperators::OP_GetElementI_UInt32(Var instance, uint32 index, ScriptContext* scriptContext)
    {
#if FLOATVAR
        return OP_GetElementI_JIT(instance, Js::JavascriptNumber::ToVar(index, scriptContext), scriptContext);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_GetElementI_JIT(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext);
#endif
    }

    Var JavascriptOperators::OP_GetElementI_UInt32_ExpectingNativeFloatArray(Var instance, uint32 index, ScriptContext* scriptContext)
    {
        UpdateNativeArrayProfileInfoToCreateVarArray(instance, true, false);
        return OP_GetElementI_UInt32(instance, index, scriptContext);
    }

    Var JavascriptOperators::OP_GetElementI_UInt32_ExpectingVarArray(Var instance, uint32 index, ScriptContext* scriptContext)
    {
        UpdateNativeArrayProfileInfoToCreateVarArray(instance, false, true);
        return OP_GetElementI_UInt32(instance, index, scriptContext);
    }

    Var JavascriptOperators::OP_GetElementI_Int32(Var instance, int32 index, ScriptContext* scriptContext)
    {
#if FLOATVAR
        return OP_GetElementI_JIT(instance, Js::JavascriptNumber::ToVar(index, scriptContext), scriptContext);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_GetElementI_JIT(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext);
#endif
    }

    Var JavascriptOperators::OP_GetElementI_Int32_ExpectingNativeFloatArray(Var instance, int32 index, ScriptContext* scriptContext)
    {
        UpdateNativeArrayProfileInfoToCreateVarArray(instance, true, false);
        return OP_GetElementI_Int32(instance, index, scriptContext);
    }

    Var JavascriptOperators::OP_GetElementI_Int32_ExpectingVarArray(Var instance, int32 index, ScriptContext* scriptContext)
    {
        UpdateNativeArrayProfileInfoToCreateVarArray(instance, false, true);
        return OP_GetElementI_Int32(instance, index, scriptContext);
    }

    BOOL JavascriptOperators::GetItemFromArrayPrototype(JavascriptArray * arr, int32 indexInt, Var * result, ScriptContext * scriptContext)
    {
        // try get from Array prototype
        RecyclableObject* prototype = arr->GetPrototype();
        if (JavascriptOperators::GetTypeId(prototype) != TypeIds_Array) //This can be TypeIds_ES5Array (or any other object changed through __proto__).
        {
            return false;
        }

        JavascriptArray* arrayPrototype = JavascriptArray::FromVar(prototype); //Prototype must be Array.prototype (unless changed through __proto__)
        AssertMsg(scriptContext->GetConfig()->Is__proto__Enabled()
            || arrayPrototype->GetScriptContext()->GetLibrary()->GetArrayPrototype() == arrayPrototype, "This function is supported only for [[class]] Array");
        if (arrayPrototype->GetLength() && arrayPrototype->GetItem(arrayPrototype, (uint32)indexInt, result, scriptContext))
        {
            return true;
        }

        prototype = arrayPrototype->GetPrototype(); //Its prototype must be Object.prototype (unless changed through __proto__)
        AssertMsg(scriptContext->GetConfig()->Is__proto__Enabled()
            || prototype->GetScriptContext()->GetLibrary()->GetObjectPrototype() == prototype, "This function is supported only for [[class]] Array");
        if (prototype->GetScriptContext()->GetLibrary()->GetObjectPrototype() != prototype)
        {
            return false;
        }

        if (DynamicObject::FromVar(prototype)->HasNonEmptyObjectArray())
        {
            if (prototype->GetItem(arr, (uint32)indexInt, result, scriptContext))
            {
                return true;
            }
        }

        *result = scriptContext->GetMissingItemResult(arr, indexInt);
        return true;
    }

    template <typename T>
    BOOL JavascriptOperators::OP_GetElementI_ArrayFastPath(T * arr, int indexInt, Var * result, ScriptContext * scriptContext)
    {
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(arr);
        if (indexInt >= 0)
        {
            if (!CrossSite::IsCrossSiteObjectTyped(arr))
            {
                if (arr->T::DirectGetVarItemAt((uint32)indexInt, result, scriptContext))
                {
                    return true;
                }
            }
            else
            {
                if (arr->GetItem(arr, (uint32)indexInt, result, scriptContext))
                {
                    return true;
                }
            }
            return GetItemFromArrayPrototype(arr, indexInt, result, scriptContext);
        }
        return false;
    }

    Var JavascriptOperators::OP_GetElementI(Var instance, Var index, ScriptContext* scriptContext)
    {
#ifdef ARRLOG
        rawGet++;
#endif
        JavascriptString *temp = NULL;
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(instance);

        if (TaggedInt::Is(index))
        {
        TaggedIntIndex:
            switch (JavascriptOperators::GetTypeId(instance))
            {
            case TypeIds_Array: //fast path for array
            {
                Var result;
                if (OP_GetElementI_ArrayFastPath(JavascriptArray::FromVar(instance), TaggedInt::ToInt32(index), &result, scriptContext))
                {
                    return result;
                }
                break;
            }
            case TypeIds_NativeIntArray:
            {
                Var result;
                if (OP_GetElementI_ArrayFastPath(JavascriptNativeIntArray::FromVar(instance), TaggedInt::ToInt32(index), &result, scriptContext))
                {
                    return result;
                }
                break;
            }
            case TypeIds_NativeFloatArray:
            {
                Var result;
                if (OP_GetElementI_ArrayFastPath(JavascriptNativeFloatArray::FromVar(instance), TaggedInt::ToInt32(index), &result, scriptContext))
                {
                    return result;
                }
                break;
            }

            case TypeIds_String: // fast path for string [string-base64 FTW]
            {
                charcount_t indexInt = TaggedInt::ToUInt32(index);
                JavascriptString* string = JavascriptString::FromVar(instance);
                Var result;
                if (string->JavascriptString::GetItem(instance, indexInt, &result, scriptContext))
                {
                    return result;
                }
                break;
            }

            case TypeIds_Int8Array:
            {
                // The typed array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);
                if (VirtualTableInfo<Int8VirtualArray>::HasVirtualTable(instance))
                {
                    Int8VirtualArray* int8Array = Int8VirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(int8Array) && indexInt >= 0)
                    {
                        return int8Array->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Int8Array* int8Array = Int8Array::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(int8Array) && indexInt >= 0)
                    {
                        return int8Array->DirectGetItem(indexInt);
                    }
                }
                break;
            }

            case TypeIds_Uint8Array:
            {
                // The typed array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);
                if (VirtualTableInfo<Uint8VirtualArray>::HasVirtualTable(instance))
                {
                    Uint8VirtualArray* uint8Array = Uint8VirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(uint8Array) && indexInt >= 0)
                    {
                        return uint8Array->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Uint8Array* uint8Array = Uint8Array::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(uint8Array) && indexInt >= 0)
                    {
                        return uint8Array->DirectGetItem(indexInt);
                    }
                }
                break;
            }

            case TypeIds_Uint8ClampedArray:
            {
                // The typed array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);
                if (VirtualTableInfo<Uint8ClampedVirtualArray>::HasVirtualTable(instance))
                {
                    Uint8ClampedVirtualArray* uint8ClampedArray = Uint8ClampedVirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(uint8ClampedArray) && indexInt >= 0)
                    {
                        return uint8ClampedArray->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Uint8ClampedArray* uint8ClampedArray = Uint8ClampedArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(uint8ClampedArray) && indexInt >= 0)
                    {
                        return uint8ClampedArray->DirectGetItem(indexInt);
                    }
                }
                break;
            }

            case TypeIds_Int16Array:
            {
                // The type array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);

                if (VirtualTableInfo<Int16VirtualArray>::HasVirtualTable(instance))
                {
                    Int16VirtualArray* int16Array = Int16VirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(int16Array) && indexInt >= 0)
                    {
                        return int16Array->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Int16Array* int16Array = Int16Array::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(int16Array) && indexInt >= 0)
                    {
                        return int16Array->DirectGetItem(indexInt);
                    }
                }
                break;
            }

            case TypeIds_Uint16Array:
            {
                // The type array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);

                if (VirtualTableInfo<Uint16VirtualArray>::HasVirtualTable(instance))
                {
                    Uint16VirtualArray* uint16Array = Uint16VirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(uint16Array) && indexInt >= 0)
                    {
                        return uint16Array->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Uint16Array* uint16Array = Uint16Array::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(uint16Array) && indexInt >= 0)
                    {
                        return uint16Array->DirectGetItem(indexInt);
                    }
                }
                break;
            }
            case TypeIds_Int32Array:
            {
                // The type array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);
                if (VirtualTableInfo<Int32VirtualArray>::HasVirtualTable(instance))
                {
                    Int32VirtualArray* int32Array = Int32VirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(int32Array) && indexInt >= 0)
                    {
                        return int32Array->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Int32Array* int32Array = Int32Array::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(int32Array) && indexInt >= 0)
                    {
                        return int32Array->DirectGetItem(indexInt);
                    }
                }
                break;

            }
            case TypeIds_Uint32Array:
            {
                // The type array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);
                if (VirtualTableInfo<Uint32VirtualArray>::HasVirtualTable(instance))
                {
                    Uint32VirtualArray* uint32Array = Uint32VirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(uint32Array) && indexInt >= 0)
                    {
                        return uint32Array->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Uint32Array* uint32Array = Uint32Array::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(uint32Array) && indexInt >= 0)
                    {
                        return uint32Array->DirectGetItem(indexInt);
                    }
                }
                break;
            }
            case TypeIds_Float32Array:
            {
                // The type array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);

                if (VirtualTableInfo<Float32VirtualArray>::HasVirtualTable(instance))
                {
                    Float32VirtualArray* float32Array = Float32VirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(float32Array) && indexInt >= 0)
                    {
                        return float32Array->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Float32Array* float32Array = Float32Array::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(float32Array) && indexInt >= 0)
                    {
                        return float32Array->DirectGetItem(indexInt);
                    }
                }
                break;
            }
            case TypeIds_Float64Array:
            {
                // The type array will deal with all possible values for the index
                int32 indexInt = TaggedInt::ToInt32(index);
                if (VirtualTableInfo<Float64VirtualArray>::HasVirtualTable(instance))
                {
                    Float64VirtualArray* float64Array = Float64VirtualArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(float64Array) && indexInt >= 0)
                    {
                        return float64Array->DirectGetItem(indexInt);
                    }
                }
                else
                {
                    Float64Array* float64Array = Float64Array::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(float64Array) && indexInt >= 0)
                    {
                        return float64Array->DirectGetItem(indexInt);
                    }
                }
                break;
            }

            case TypeIds_PixelArray: // fast path for pixel array
            {
                // The pixel array will deal with all possible values for the index
                uint32 indexInt = TaggedInt::ToUInt32(index);
                JavascriptPixelArray* pixelArray = JavascriptPixelArray::FromVar(instance);
                if (!CrossSite::IsCrossSiteObjectTyped(pixelArray))
                {
                    return pixelArray->DirectGetItem(indexInt);
                }
                break;
            }

            default:
                break;
            }
        }
        else if (JavascriptNumber::Is_NoTaggedIntCheck(index))
        {
            uint32 uint32Index = JavascriptConversion::ToUInt32(index, scriptContext);

            if ((double)uint32Index == JavascriptNumber::GetValue(index) && !TaggedInt::IsOverflow(uint32Index))
            {
                index = TaggedInt::ToVarUnchecked(uint32Index);
                goto TaggedIntIndex;
            }
        }
        else if (JavascriptString::Is(index)) // fastpath for PropertyStrings
        {
            temp = JavascriptString::FromVar(index);
            Assert(temp->GetScriptContext() == scriptContext);

            if (VirtualTableInfo<Js::PropertyString>::HasVirtualTable(temp))
            {
                PropertyString * propertyString = (PropertyString*)temp;
                PropertyCache const *cache = propertyString->GetPropertyCache();
                RecyclableObject* object = null;
                if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined,
                        JavascriptString::FromVar(index)->GetSz());
                }
                if (object->GetType() == cache->type)
                {
#if DBG_DUMP
                    scriptContext->forinCache++;
#endif
                    Assert(object->GetScriptContext() == scriptContext);
                    Var value;
                    if (cache->isInlineSlot)
                    {
                        value = DynamicObject::FromVar(object)->GetInlineSlot(cache->dataSlotIndex);
                    }
                    else
                    {
                        value = DynamicObject::FromVar(object)->GetAuxSlot(cache->dataSlotIndex);
                    }
                    Assert(!CrossSite::NeedMarshalVar(value, scriptContext));
                    Assert((BinaryFeatureControl::LanguageService() && JavascriptOperators::IsUndefinedOrNullType(JavascriptOperators::GetTypeId(value)))
                        || value == JavascriptOperators::GetProperty(object, propertyString->GetPropertyRecord()->GetPropertyId(), scriptContext)
                        || value == JavascriptOperators::GetRootProperty(object, propertyString->GetPropertyRecord()->GetPropertyId(), scriptContext));
                    return value;
                }
#if DBG_DUMP
                scriptContext->forinNoCache++;
#endif
                PropertyRecord const * propertyRecord = propertyString->GetPropertyRecord();
                Var value;
                if (propertyRecord->IsNumeric())
                {
                    if (JavascriptOperators::GetItem(instance, object, propertyRecord->GetNumericValue(), &value, scriptContext))
                    {
                        return value;
                    }
                }
                else
                {
                    if (JavascriptOperators::GetPropertyWPCache(instance, object, propertyRecord->GetPropertyId(), &value, scriptContext, propertyString))
                    {
                        return value;
                    }
                }
                return scriptContext->GetLibrary()->GetUndefined();
            }
#if DBG_DUMP
            scriptContext->forinNoCache++;
#endif
        }

        return JavascriptOperators::GetElementIHelper(instance, index, instance, scriptContext);
    }

    Var JavascriptOperators::GetElementIHelper(Var instance, Var index, Var receiver, ScriptContext* scriptContext)
    {
        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            if(scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined, GetPropertyDisplayNameForError(index, scriptContext));
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        uint32 indexVal;
        PropertyRecord const * propertyRecord;
        JavascriptString * propertyNameString;
        Var value;

        bool createIfNotFound = !IsJsNativeObject(object);

        IndexType indexType = GetIndexType(index, scriptContext, &indexVal, &propertyRecord, &propertyNameString, createIfNotFound, true);

        if (indexType == IndexType_Number)
        {
#ifdef ARRLOG
            rawTypeGet++;
#endif
            if (JavascriptOperators::GetItem(receiver, object, indexVal, &value, scriptContext))
            {
                return value;
            }
        }
        else if (indexType == IndexType_JavascriptString)
        {
            if (JavascriptOperators::GetPropertyWPCache(receiver, object, propertyNameString, &value, scriptContext, null))
            {
                return value;
            }
        }
        else
        {
            // We called GetIndexType with preferJavascriptString as true, so we mush have a propertyRecord
            Assert(indexType == IndexType_PropertyId);

            Assert(propertyRecord);

            if (JavascriptOperators::GetPropertyWPCache(receiver, object, propertyRecord->GetPropertyId(), &value, scriptContext, null))
            {
                return value;
            }
        }

        return scriptContext->GetMissingItemResult(object, indexVal);
    }

    int32 JavascriptOperators::OP_GetNativeIntElementI(Var instance, Var index)
    {
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(instance);
        if (TaggedInt::Is(index))
        {
            int32 indexInt = TaggedInt::ToInt32(index);
            if (indexInt < 0)
            {
                return JavascriptNativeIntArray::MissingItem;
            }
            JavascriptArray * arr = JavascriptArray::FromVar(instance);
            int32 result;
            if (arr->DirectGetItemAt((uint32)indexInt, &result))
            {
                return result;
            }
        }
        else if (JavascriptNumber::Is_NoTaggedIntCheck(index))
        {
            int32 indexInt;
            bool isInt32;
            double dIndex = JavascriptNumber::GetValue(index);
            if (JavascriptNumber::TryGetInt32OrUInt32Value(dIndex, &indexInt, &isInt32))
            {
                if (isInt32 && indexInt < 0)
                {
                    return JavascriptNativeIntArray::MissingItem;
                }
                JavascriptArray * arr = JavascriptArray::FromVar(instance);
                int32 result;
                if (arr->DirectGetItemAt((uint32)indexInt, &result))
                {
                    return result;
                }
            }
        }
        else
        {
            AssertMsg(0, "Non-numerical index in this helper?");
        }

        return JavascriptNativeIntArray::MissingItem;
    }

    int32 JavascriptOperators::OP_GetNativeIntElementI_UInt32(Var instance, uint32 index, ScriptContext* scriptContext)
    {
#if FLOATVAR
        return OP_GetNativeIntElementI(instance, Js::JavascriptNumber::ToVar(index, scriptContext));
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_GetNativeIntElementI(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer));
#endif
    }

    int32 JavascriptOperators::OP_GetNativeIntElementI_Int32(Var instance, int32 index, ScriptContext* scriptContext)
    {
#if FLOATVAR
        return OP_GetNativeIntElementI(instance, Js::JavascriptNumber::ToVar(index, scriptContext));
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_GetNativeIntElementI(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer));
#endif
    }

    double JavascriptOperators::OP_GetNativeFloatElementI(Var instance, Var index)
    {
        double result = 0;

        if (TaggedInt::Is(index))
        {
            int32 indexInt = TaggedInt::ToInt32(index);
            if (indexInt < 0)
            {
                result = JavascriptNativeFloatArray::MissingItem;
            }
            else
            {
                JavascriptArray * arr = JavascriptArray::FromVar(instance);
                if (!arr->DirectGetItemAt((uint32)indexInt, &result))
                {
                    result = JavascriptNativeFloatArray::MissingItem;
                }
            }
        }
        else if (JavascriptNumber::Is_NoTaggedIntCheck(index))
        {
            int32 indexInt;
            bool isInt32;
            double dIndex = JavascriptNumber::GetValue(index);
            if (JavascriptNumber::TryGetInt32OrUInt32Value(dIndex, &indexInt, &isInt32))
            {
                if (isInt32 && indexInt < 0)
                {
                    result = JavascriptNativeFloatArray::MissingItem;
                }
                else
                {
                    JavascriptArray * arr = JavascriptArray::FromVar(instance);
                    if (!arr->DirectGetItemAt((uint32)indexInt, &result))
                    {
                        result = JavascriptNativeFloatArray::MissingItem;
                    }
                }
            }
        }
        else
        {
            AssertMsg(0, "Non-numerical index in this helper?");
        }

        return result;
    }

    double JavascriptOperators::OP_GetNativeFloatElementI_UInt32(Var instance, uint32 index, ScriptContext* scriptContext)
    {
#if FLOATVAR
        return OP_GetNativeFloatElementI(instance, Js::JavascriptNumber::ToVar(index, scriptContext));
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_GetNativeFloatElementI(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer));
#endif
    }

    double JavascriptOperators::OP_GetNativeFloatElementI_Int32(Var instance, int32 index, ScriptContext* scriptContext)
    {
#if FLOATVAR
        return OP_GetNativeFloatElementI(instance, Js::JavascriptNumber::ToVar(index, scriptContext));
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_GetNativeFloatElementI(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer));
#endif
    }

    Var JavascriptOperators::OP_GetMethodElement_UInt32(Var instance, uint32 index, ScriptContext* scriptContext)
    {
#if FLOATVAR
        return OP_GetMethodElement(instance, Js::JavascriptNumber::ToVar(index, scriptContext), scriptContext);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_GetMethodElement(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext);
#endif
    }

    Var JavascriptOperators::OP_GetMethodElement_Int32(Var instance, int32 index, ScriptContext* scriptContext)
    {
#if FLOATVAR
        return OP_GetElementI(instance, Js::JavascriptNumber::ToVar(index, scriptContext), scriptContext);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_GetMethodElement(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext);
#endif
    }

    Var JavascriptOperators::OP_GetMethodElement(Var instance, Var index, ScriptContext* scriptContext)
    {
        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined, GetPropertyDisplayNameForError(index, scriptContext));
        }

        ThreadContext* threadContext = scriptContext->GetThreadContext();
        ImplicitCallFlags savedImplicitCallFlags = threadContext->GetImplicitCallFlags();
        threadContext->ClearImplicitCallFlags();

        uint32 indexVal;
        PropertyRecord const * propertyRecord;
        Var value = NULL;
        BOOL hasProperty = FALSE;
        // TODO: Perf optimization: If IsStringTypeHandler is true, convert indexVal to string instead of PropertyRecord and let
        // type handler decide whether to turn it into a string or not.  Perf is gained if PropertyRecords are avoided altogether
        // when dealing with a string based type handler.
        bool createIfNotFound = !IsJsNativeObject(object) ||
            (DynamicType::Is(object->GetTypeId()) && static_cast<DynamicObject*>(object)->GetTypeHandler()->IsStringTypeHandler());

        if (GetIndexType(index, scriptContext, &indexVal, &propertyRecord, createIfNotFound) == IndexType_Number)
        {
            hasProperty = JavascriptOperators::GetItemReference(instance, object, indexVal, &value, scriptContext);
        }
        else
        {
            if (propertyRecord != null)
            {
                hasProperty = JavascriptOperators::GetPropertyReference(instance, object, propertyRecord->GetPropertyId(), &value, scriptContext, NULL);
            }
#if DBG
            else
            {
                Assert(IsJsNativeObject(object));
                JavascriptString* indexStr = JavascriptConversion::ToString(index, scriptContext);
                PropertyRecord const * debugPropertyRecord;
                scriptContext->GetOrAddPropertyRecord(indexStr->GetString(), indexStr->GetLength(), &debugPropertyRecord);
                AssertMsg(!JavascriptOperators::GetPropertyReference(instance, object, debugPropertyRecord->GetPropertyId(), &value, scriptContext, NULL),
                          "how did this property come?");
            }
#endif
        }

        if (!hasProperty)
        {
            JavascriptString* varName = JavascriptConversion::ToString(index, scriptContext);

            // ES5 11.2.3 #2: We evaluate the call target but don't throw yet if target member is missing. We need to evaluate argList
            // first (#3). Postpone throwing error to invoke time.
            value = ThrowErrorObject::CreateThrowTypeErrorObject(scriptContext, VBSERR_OLENoPropOrMethod, varName);
        }
        else if(!JavascriptConversion::IsCallable(value))
        {
            // ES5 11.2.3 #2: We evaluate the call target but don't throw yet if target member is missing. We need to evaluate argList
            // first (#3). Postpone throwing error to invoke time.
            JavascriptString* varName = JavascriptConversion::ToString(index, scriptContext);
            value = ThrowErrorObject::CreateThrowTypeErrorObject(scriptContext, JSERR_Property_NeedFunction, varName);
        }

        threadContext->CheckAndResetImplicitCallAccessorFlag();
        threadContext->AddImplicitCallFlags(savedImplicitCallFlags);
        return value;
    }

    BOOL JavascriptOperators::OP_SetElementI_UInt32(Var instance, uint32 index, Var value, ScriptContext* scriptContext, PropertyOperationFlags flags)
    {
#if FLOATVAR
        return OP_SetElementI_JIT(instance, Js::JavascriptNumber::ToVar(index, scriptContext), value, scriptContext, flags);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_SetElementI_JIT(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), value, scriptContext, flags);
#endif
    }

    BOOL JavascriptOperators::OP_SetElementI_Int32(Var instance, int32 index, Var value, ScriptContext* scriptContext, PropertyOperationFlags flags)
    {
#if FLOATVAR
        return OP_SetElementI_JIT(instance, Js::JavascriptNumber::ToVar(index, scriptContext), value, scriptContext, flags);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_SetElementI_JIT(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), value, scriptContext, flags);
#endif
    }

    BOOL JavascriptOperators::OP_SetElementI_JIT(Var instance, Var index, Var value, ScriptContext* scriptContext, PropertyOperationFlags flags)
    {
        if (TaggedNumber::Is(instance))
        {
            return OP_SetElementI(instance, index, value, scriptContext, flags);
        }

        INT_PTR vt = VirtualTableInfoBase::GetVirtualTable(instance);
        OP_SetElementI(instance, index, value, scriptContext, flags);
        return vt != VirtualTableInfoBase::GetVirtualTable(instance);
    }

    BOOL JavascriptOperators::OP_SetElementI(Var instance, Var index, Var value, ScriptContext* scriptContext, PropertyOperationFlags flags)
    {
#ifdef ARRLOG
        rawSet++;
#endif
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(instance);

        TypeId instanceType = JavascriptOperators::GetTypeId(instance);

        bool isTypedArray = (instanceType >= TypeIds_Int8Array && instanceType <= TypeIds_Float64Array);

        if (isTypedArray)
        {
            if (TaggedInt::Is(index) || JavascriptNumber::Is_NoTaggedIntCheck(index) || JavascriptString::Is(index))
            {
                BOOL returnValue = FALSE;
                bool isNumericIndex = false;
                switch (instanceType)
                {
                case TypeIds_Int8Array:
                {
                    // The typed array will deal with all possible values for the index

                    if (VirtualTableInfo<Int8VirtualArray>::HasVirtualTable(instance))
                    {
                        Int8VirtualArray* int8Array = Int8VirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(int8Array))
                        {
                            returnValue = int8Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Int8Array* int8Array = Int8Array::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(int8Array))
                        {
                            returnValue = int8Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }

                case TypeIds_Uint8Array:
                {
                    // The typed array will deal with all possible values for the index
                    if (VirtualTableInfo<Uint8VirtualArray>::HasVirtualTable(instance))
                    {
                        Uint8VirtualArray* uint8Array = Uint8VirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(uint8Array))
                        {
                            returnValue = uint8Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Uint8Array* uint8Array = Uint8Array::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(uint8Array))
                        {
                            returnValue = uint8Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }

                case TypeIds_Uint8ClampedArray:
                {
                    // The typed array will deal with all possible values for the index
                    if (VirtualTableInfo<Uint8ClampedVirtualArray>::HasVirtualTable(instance))
                    {
                        Uint8ClampedVirtualArray* uint8ClampedArray = Uint8ClampedVirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(uint8ClampedArray))
                        {
                            returnValue = uint8ClampedArray->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Uint8ClampedArray* uint8ClampedArray = Uint8ClampedArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(uint8ClampedArray))
                        {
                            returnValue = uint8ClampedArray->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }

                case TypeIds_Int16Array:
                {
                    // The type array will deal with all possible values for the index
                    if (VirtualTableInfo<Int16VirtualArray>::HasVirtualTable(instance))
                    {
                        Int16VirtualArray* int16Array = Int16VirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(int16Array))
                        {
                            returnValue = int16Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Int16Array* int16Array = Int16Array::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(int16Array))
                        {
                            returnValue = int16Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }

                case TypeIds_Uint16Array:
                {
                    // The type array will deal with all possible values for the index

                    if (VirtualTableInfo<Uint16VirtualArray>::HasVirtualTable(instance))
                    {
                        Uint16VirtualArray* uint16Array = Uint16VirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(uint16Array))
                        {
                            returnValue = uint16Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Uint16Array* uint16Array = Uint16Array::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(uint16Array))
                        {
                            returnValue = uint16Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }
                case TypeIds_Int32Array:
                {
                    // The type array will deal with all possible values for the index
                    if (VirtualTableInfo<Int32VirtualArray>::HasVirtualTable(instance))
                    {
                        Int32VirtualArray* int32Array = Int32VirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(int32Array))
                        {
                            returnValue = int32Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Int32Array* int32Array = Int32Array::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(int32Array))
                        {
                            returnValue = int32Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }
                case TypeIds_Uint32Array:
                {
                    // The type array will deal with all possible values for the index

                    if (VirtualTableInfo<Uint32VirtualArray>::HasVirtualTable(instance))
                    {
                        Uint32VirtualArray* uint32Array = Uint32VirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(uint32Array))
                        {
                            returnValue = uint32Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Uint32Array* uint32Array = Uint32Array::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(uint32Array))
                        {
                            returnValue = uint32Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }
                case TypeIds_Float32Array:
                {
                    // The type array will deal with all possible values for the index
                    if (VirtualTableInfo<Float32VirtualArray>::HasVirtualTable(instance))
                    {
                        Float32VirtualArray* float32Array = Float32VirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(float32Array))
                        {
                            returnValue = float32Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Float32Array* float32Array = Float32Array::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(float32Array))
                        {
                            returnValue = float32Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }
                case TypeIds_Float64Array:
                {
                    // The type array will deal with all possible values for the index

                    if (VirtualTableInfo<Float64VirtualArray>::HasVirtualTable(instance))
                    {
                        Float64VirtualArray* float64Array = Float64VirtualArray::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(float64Array))
                        {
                            returnValue = float64Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    else
                    {
                        Float64Array* float64Array = Float64Array::FromVar(instance);
                        if (!CrossSite::IsCrossSiteObjectTyped(float64Array))
                        {
                            returnValue = float64Array->ValidateIndexAndDirectSetItem(index, value, &isNumericIndex);
                        }
                    }
                    break;
                }
                }

                // if this was numeric index, return operation status else
                // Return the result of calling the default ordinary object [[Set]] internal method (9.1.8) on O passing P, V, and Receiver as arguments.
                if (isNumericIndex)
                    return returnValue;
            }
        }
        else
        {
            if (TaggedInt::Is(index))
            {
            TaggedIntIndex:
                switch (instanceType)
                {
                case TypeIds_NativeIntArray:
                case TypeIds_NativeFloatArray:
                case TypeIds_Array: // fast path for array
                {
                    int indexInt = TaggedInt::ToInt32(index);
                    if (indexInt >= 0 && scriptContext->optimizationOverrides.IsEnabledArraySetElementFastPath())
                    {
                        JavascriptArray::FromVar(instance)->SetItem((uint32)indexInt, value, flags);
                        return true;
                    }
                    break;
                }
                case TypeIds_PixelArray: // fast path for pixel array
                {
                    // The pixel array will deal with all possible values for the index
                    uint32 indexInt = TaggedInt::ToUInt32(index);
                    JavascriptPixelArray* pixelArray = JavascriptPixelArray::FromVar(instance);
                    if (!CrossSite::IsCrossSiteObjectTyped(pixelArray))
                    {
                        pixelArray->DirectSetItem(indexInt, value);
                        return true;
                    }
                    break;
                }
                }
            }
            else if (JavascriptNumber::Is_NoTaggedIntCheck(index))
            {
                double dIndexValue = JavascriptNumber::GetValue(index);
                uint32 uint32Index = JavascriptConversion::ToUInt32(index, scriptContext);

                if ((double)uint32Index == dIndexValue && !TaggedInt::IsOverflow(uint32Index))
                {
                    index = TaggedInt::ToVarUnchecked(uint32Index);
                    goto TaggedIntIndex;
                }
            }
        }

        RecyclableObject* object;
        BOOL isNullOrUndefined = !GetPropertyObject(instance, scriptContext, &object);
        if (!BinaryFeatureControl::LanguageService())
        {
            // In the LS mode, we may get the different object (the one which tracked undefined has given to us)
            Assert(object == instance || TaggedNumber::Is(instance));
        }

        if (isNullOrUndefined)
        {
            if (!scriptContext->GetThreadContext()->RecordImplicitException())
            {
                return FALSE;
            }

            JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotSet_NullOrUndefined, GetPropertyDisplayNameForError(index, scriptContext));
        } 

        return JavascriptOperators::SetElementIHelper(instance, object, index, value, scriptContext, flags);
    }

    BOOL JavascriptOperators::SetElementIHelper(Var receiver, RecyclableObject* object, Var index, Var value, ScriptContext* scriptContext, PropertyOperationFlags flags)
    {
        PropertyString * propertyString = nullptr;
        Js::IndexType indexType;
        uint32 indexVal = 0;
        PropertyRecord const * propertyRecord = nullptr;
        JavascriptString * propertyNameString = nullptr;

        if (TaggedNumber::Is(receiver))
        {
            indexType = GetIndexType(index, scriptContext, &indexVal, &propertyRecord, true);
            if (indexType == IndexType_Number)
            {
                return  JavascriptOperators::SetItemOnTaggedNumber(receiver, object, indexVal, value, scriptContext, flags);
            }
            else
            {
                if (!propertyRecord)
                {
                    //Lets get the propertyRecord from the propertyString
                    scriptContext->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
                }
                return  JavascriptOperators::SetPropertyOnTaggedNumber(receiver, object, propertyRecord->GetPropertyId(), value, scriptContext, flags);
            }
        }

        // REVIEW: Disable property string cache for language service, as the copy on write code
        // doesn't marshal the property string to the correct script context
        if (!BinaryFeatureControl::LanguageService() &&
            !TaggedInt::Is(index) && JavascriptString::Is(index) &&
            VirtualTableInfo<Js::PropertyString>::HasVirtualTable(JavascriptString::FromVar(index))) // fastpath for PropertyStrings
        {
            propertyString = (PropertyString *)JavascriptString::FromVar(index);

            Assert(propertyString->GetScriptContext() == scriptContext);

            PropertyCache const * cache = propertyString->GetPropertyCache();
            if (object->GetType() == cache->type && cache->isStoreFieldEnabled)
            {
#if DBG
                propertyRecord = propertyString->GetPropertyRecord();
#endif
#if DBG_DUMP
                scriptContext->forinCache++;
#endif
        //wprintf(L"set cache hit with %x[%s]\n",instance,temp->GetSz());
                Assert(object->GetScriptContext() == scriptContext);
                Assert(!CrossSite::NeedMarshalVar(value, scriptContext));
                if (cache->isInlineSlot)
                {
                    DynamicObject::FromVar(object)->SetInlineSlot(SetSlotArguments(propertyRecord->GetPropertyId(), cache->dataSlotIndex, value));
                }
                else
                {
                    DynamicObject::FromVar(object)->SetAuxSlot(SetSlotArguments(propertyRecord->GetPropertyId(), cache->dataSlotIndex, value));
                }
                return true;
            }

            propertyRecord = propertyString->GetPropertyRecord();
            if (propertyRecord->IsNumeric())
            {
                indexType = IndexType_Number;
                indexVal = propertyRecord->GetNumericValue();
            }
            else
            {
                indexType = IndexType_PropertyId;
            }

#if DBG_DUMP
            scriptContext->forinNoCache++;
#endif
        }
        else
        {
#if DBG_DUMP
            scriptContext->forinNoCache += (!TaggedInt::Is(index) && JavascriptString::Is(index));
#endif
            indexType = GetIndexType(index, scriptContext, &indexVal, &propertyRecord, &propertyNameString, false, true);
            if (scriptContext->GetThreadContext()->IsDisableImplicitCall() &&
                scriptContext->GetThreadContext()->GetImplicitCallFlags() != ImplicitCall_None)
            {
                // We hit an implicit call trying to convert the index, and implicit calls are disabled, so
                // quit before we try to store the element.
                return FALSE;
            }
        }

        if (indexType == IndexType_Number)
        {
            return JavascriptOperators::SetItem(receiver, object, indexVal, value, scriptContext, flags);
        }
        else if (indexType == IndexType_JavascriptString)
        {
            Assert(propertyNameString);
            JsUtil::CharacterBuffer<WCHAR> propertyName(propertyNameString->GetString(), propertyNameString->GetLength());

            if (BuiltInPropertyRecords::NaN.Equals(propertyName))
            {
                // Follow SetProperty convention for NaN
                return JavascriptOperators::SetProperty(receiver, object, PropertyIds::NaN, value, scriptContext, flags);
            }
            else if (BuiltInPropertyRecords::Infinity.Equals(propertyName))
            {
                // Follow SetProperty convention for Infinity
                return JavascriptOperators::SetProperty(receiver, object, PropertyIds::Infinity, value, scriptContext, flags);
            }

            return JavascriptOperators::SetPropertyWPCache(receiver, object, propertyNameString, value, scriptContext, nullptr, flags);
        }
        else if (indexType == IndexType_PropertyId)
        {
            Assert(propertyRecord);
            PropertyId propId = propertyRecord->GetPropertyId();
            if (propId == PropertyIds::NaN || propId == PropertyIds::Infinity)
            {
                // As we no longer convert o[x] into o.x for NaN and Infinity, we need to follow SetProperty convention for these,
                // which would check for read-only properties, strict mode, etc.
                // Note that "-Infinity" does not qualify as property name, so we don't have to take care of it.
                return JavascriptOperators::SetProperty(receiver, object, propId, value, scriptContext, flags);
            }
        }

        return JavascriptOperators::SetPropertyWPCache(receiver, object, propertyRecord->GetPropertyId(), value, scriptContext, propertyString, flags);
    }

    BOOL JavascriptOperators::OP_SetNativeIntElementI(
        Var instance,
        Var aElementIndex,
        int32 iValue,
        ScriptContext* scriptContext,
        PropertyOperationFlags flags)
    {
        if (TaggedInt::Is(aElementIndex))
        {
            int32 indexInt = TaggedInt::ToInt32(aElementIndex);
            if (indexInt >= 0 && scriptContext->optimizationOverrides.IsEnabledArraySetElementFastPath())
            {
                JavascriptNativeIntArray *arr = JavascriptNativeIntArray::FromVar(instance);
                if (!(arr->TryGrowHeadSegmentAndSetItem<int32, JavascriptNativeIntArray>((uint32)indexInt, iValue)))
                {
                    arr->SetItem(indexInt, iValue);
                }
                return TRUE;
            }
        }

        return JavascriptOperators::OP_SetElementI(instance, aElementIndex, JavascriptNumber::ToVar(iValue, scriptContext), scriptContext, flags);
    }

    BOOL JavascriptOperators::OP_SetNativeIntElementI_UInt32(
        Var instance,
        uint32 aElementIndex,
        int32 iValue,
        ScriptContext* scriptContext,
        PropertyOperationFlags flags)
    {
#if FLOATVAR
        return OP_SetNativeIntElementI(instance, Js::JavascriptNumber::ToVar(aElementIndex, scriptContext), iValue, scriptContext, flags);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_SetNativeIntElementI(instance, Js::JavascriptNumber::ToVarInPlace(aElementIndex, scriptContext,
            (Js::JavascriptNumber *)buffer), iValue, scriptContext, flags);
#endif
    }

    BOOL JavascriptOperators::OP_SetNativeIntElementI_Int32(
        Var instance,
        int aElementIndex,
        int32 iValue,
        ScriptContext* scriptContext,
        PropertyOperationFlags flags)
    {
#if FLOATVAR
        return OP_SetNativeIntElementI(instance, Js::JavascriptNumber::ToVar(aElementIndex, scriptContext), iValue, scriptContext, flags);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_SetNativeIntElementI(instance, Js::JavascriptNumber::ToVarInPlace(aElementIndex, scriptContext,
            (Js::JavascriptNumber *)buffer), iValue, scriptContext, flags);
#endif
    }

    BOOL JavascriptOperators::OP_SetNativeFloatElementI(
        Var instance,
        Var aElementIndex,
        ScriptContext* scriptContext,
        PropertyOperationFlags flags,
        double dValue)
    {
        if (TaggedInt::Is(aElementIndex))
        {
            int32 indexInt = TaggedInt::ToInt32(aElementIndex);
            if (indexInt >= 0 && scriptContext->optimizationOverrides.IsEnabledArraySetElementFastPath())
            {
                JavascriptNativeFloatArray *arr = JavascriptNativeFloatArray::FromVar(instance);
                if (!(arr->TryGrowHeadSegmentAndSetItem<double, JavascriptNativeFloatArray>((uint32)indexInt, dValue)))
                {
                    arr->SetItem(indexInt, dValue);
                }
                return TRUE;
            }
        }

        return JavascriptOperators::OP_SetElementI(instance, aElementIndex, JavascriptNumber::ToVarWithCheck(dValue, scriptContext), scriptContext, flags);
    }

    BOOL JavascriptOperators::OP_SetNativeFloatElementI_UInt32(
        Var instance, uint32
        aElementIndex,
        ScriptContext* scriptContext,
        PropertyOperationFlags flags,
        double dValue)
    {
#if FLOATVAR
        return OP_SetNativeFloatElementI(instance, JavascriptNumber::ToVar(aElementIndex, scriptContext), scriptContext, flags, dValue);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_SetNativeFloatElementI(instance, JavascriptNumber::ToVarInPlace(aElementIndex, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext, flags, dValue);
#endif
    }

    BOOL JavascriptOperators::OP_SetNativeFloatElementI_Int32(
        Var instance,
        int aElementIndex,
        ScriptContext* scriptContext,
        PropertyOperationFlags flags,
        double dValue)
    {
#if FLOATVAR
        return OP_SetNativeFloatElementI(instance, JavascriptNumber::ToVar(aElementIndex, scriptContext), scriptContext, flags, dValue);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_SetNativeFloatElementI(instance, JavascriptNumber::ToVarInPlace(aElementIndex, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext, flags, dValue);
#endif
    }

    Var JavascriptOperators::OP_DeleteElementI_UInt32(Var instance, uint32 index, ScriptContext* scriptContext, PropertyOperationFlags propertyOperationFlags)
    {
#if FLOATVAR
        return OP_DeleteElementI(instance, Js::JavascriptNumber::ToVar(index, scriptContext), scriptContext, propertyOperationFlags);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_DeleteElementI(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext, propertyOperationFlags);
#endif
    }

    Var JavascriptOperators::OP_DeleteElementI_Int32(Var instance, int32 index, ScriptContext* scriptContext, PropertyOperationFlags propertyOperationFlags)
    {
#if FLOATVAR
        return OP_DeleteElementI(instance, Js::JavascriptNumber::ToVar(index, scriptContext), scriptContext, propertyOperationFlags);
#else
        char buffer[sizeof(Js::JavascriptNumber)];
        return OP_DeleteElementI(instance, Js::JavascriptNumber::ToVarInPlace(index, scriptContext,
            (Js::JavascriptNumber *)buffer), scriptContext, propertyOperationFlags);
#endif
    }

    Var JavascriptOperators::OP_DeleteElementI(Var instance, Var index, ScriptContext* scriptContext, PropertyOperationFlags propertyOperationFlags)
    {
        if(TaggedNumber::Is(instance))
        {
            return scriptContext->GetLibrary()->GetTrue();
        }

        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(instance);
        TypeId typeId = JavascriptOperators::GetTypeId(instance);
        if (typeId == TypeIds_Null || typeId == TypeIds_Undefined)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotDelete_NullOrUndefined, GetPropertyDisplayNameForError(index, scriptContext));
        }

        RecyclableObject* object = RecyclableObject::FromVar(instance);

        uint32 indexVal;
        PropertyRecord const * propertyRecord;
        BOOL result = TRUE;

        // TODO: Perf optimization: If IsStringTypeHandler is true, convert indexVal to string instead of PropertyRecord and let
        // type handler decide whether to turn it into a string or not.  Perf is gained if PropertyRecords are avoided altogether
        // when dealing with a string based type handler.
        bool createIfNotFound = !IsJsNativeObject(object) ||
            (DynamicType::Is(object->GetTypeId()) && static_cast<DynamicObject*>(object)->GetTypeHandler()->IsStringTypeHandler());

        if (GetIndexType(index, scriptContext, &indexVal, &propertyRecord, createIfNotFound) == IndexType_Number)
        {
            result = JavascriptOperators::DeleteItem(object, indexVal, propertyOperationFlags);
        }
        else
        {
            if (propertyRecord)
            {
                result = JavascriptOperators::DeleteProperty(object, propertyRecord->GetPropertyId(), propertyOperationFlags);
            }
#if DBG
            else
            {
                Assert(IsJsNativeObject(object));
                JavascriptString* indexStr = JavascriptConversion::ToString(index, scriptContext);
                PropertyRecord const * debugPropertyRecord;
                scriptContext->GetOrAddPropertyRecord(indexStr->GetString(), indexStr->GetLength(), &debugPropertyRecord);
                AssertMsg(JavascriptOperators::DeleteProperty(object, debugPropertyRecord->GetPropertyId(), propertyOperationFlags), "delete should have been true");
            }
#endif
        }

        return scriptContext->GetLibrary()->CreateBoolean(result);
    }

    Var JavascriptOperators::OP_GetLength(Var instance, ScriptContext* scriptContext)
    {
        return JavascriptOperators::OP_GetProperty(instance, PropertyIds::length, scriptContext);
    }

    __inline Var JavascriptOperators::GetThisFromModuleRoot(Var thisVar)
    {
        ModuleRoot *moduleRoot = static_cast<ModuleRoot*>(thisVar);
        RecyclableObject* hostObject = moduleRoot->GetHostObject();

        //
        // if the module root has the host object, use that as "this"
        //
        if (hostObject)
        {
            thisVar = hostObject->GetHostDispatchVar();
        }
        return thisVar;
    }

    __inline void JavascriptOperators::TryLoadRoot(Var& thisVar, TypeId typeId, int moduleID, ScriptContext* scriptContext)
    {
        bool loadRoot = false;
        if (JavascriptOperators::IsUndefinedOrNullType(typeId) || typeId == TypeIds_ActivationObject)
        {
            loadRoot = true;
        }
        else if (typeId == TypeIds_HostDispatch)
        {
            TypeId remoteTypeId;
            if (RecyclableObject::FromVar(thisVar)->GetRemoteTypeId(&remoteTypeId))
            {
                if (remoteTypeId == TypeIds_Null || remoteTypeId == TypeIds_Undefined || remoteTypeId == TypeIds_ActivationObject)
                {
                    loadRoot = true;
                }
            }
        }

        if (loadRoot)
        {
            if (moduleID == 0)
            {
                thisVar = JavascriptOperators::OP_LdRoot(scriptContext)->ToThis();
            }
            else
            {
                Js::ModuleRoot * moduleRoot = JavascriptOperators::GetModuleRoot(moduleID, scriptContext);
                if (moduleRoot == null)
                {
                    Assert(false);
                    thisVar = scriptContext->GetLibrary()->GetUndefined();
                }
                else
                {
                    thisVar = GetThisFromModuleRoot(moduleRoot);
                }
            }
        }
    }

    Var JavascriptOperators::OP_GetThis(Var thisVar, int moduleID, ScriptContext* scriptContext)
    {
        //
        // if "this" is null or undefined
        //   Pass the global object
        // Else
        //   Pass ToObject(this)
        //
        // Be very careful how you modify this function. It is on critical path for V8:DeltaBlue, V8:Richards & SS:string-tagcloud
        // Even a couple of extra comparison checks can make a big difference. DeltaBlue, for example, calls it 100M times.
        //

        TypeId typeId = JavascriptOperators::GetTypeId(thisVar);

        Assert(!JavascriptOperators::IsThisSelf(typeId));

        return JavascriptOperators::GetThisHelper(thisVar, typeId, moduleID, scriptContext);
    }

    Var JavascriptOperators::OP_GetThisNoFastPath(Var thisVar, int moduleID, ScriptContext* scriptContext)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(thisVar);

        if (JavascriptOperators::IsThisSelf(typeId))
        {
            Assert(typeId != TypeIds_GlobalObject || ((Js::GlobalObject*)thisVar)->ToThis() == thisVar);
            Assert(typeId != TypeIds_ModuleRoot || JavascriptOperators::GetThisFromModuleRoot(thisVar) == thisVar);

            return thisVar;
        }

        return JavascriptOperators::GetThisHelper(thisVar, typeId, moduleID, scriptContext);
    }

    bool JavascriptOperators::IsThisSelf(TypeId typeId)
    {
        return (JavascriptOperators::IsObjectType(typeId) && ! JavascriptOperators::IsSpecialObjectType(typeId));
    }

    Var JavascriptOperators::GetThisHelper(Var thisVar, TypeId typeId, int moduleID, ScriptContext *scriptContext)
    {
        if (! JavascriptOperators::IsObjectType(typeId) && ! JavascriptOperators::IsUndefinedOrNullType(typeId))
        {
#if !FLOATVAR
            // We allowed stack number to be used as the "this" for getter and setter activation of
            // n.x and n[prop], where n is the Javascript Number
            return JavascriptOperators::ToObject(
                JavascriptNumber::BoxStackNumber(thisVar, scriptContext), scriptContext);
#else
            return JavascriptOperators::ToObject(thisVar, scriptContext);
#endif

        }
        else
        {
            TryLoadRoot(thisVar, typeId, moduleID, scriptContext);
            return thisVar;
        }
    }

    Var JavascriptOperators::OP_StrictGetThis(Var thisVar, ScriptContext* scriptContext)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(thisVar);

        if (typeId == TypeIds_ActivationObject)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        return thisVar;
    }

    BOOL JavascriptOperators::GetRemoteTypeId(Var aValue, TypeId* typeId)
    {
        if (GetTypeId(aValue) != TypeIds_HostDispatch)
        {
            return FALSE;
        }
        return RecyclableObject::FromVar(aValue)->GetRemoteTypeId(typeId);
    }

    BOOL JavascriptOperators::IsJsNativeObject(Var aValue)
    {
        switch(GetTypeId(aValue))
        {
            case TypeIds_Object:
            case TypeIds_Function:
            case TypeIds_Array:
            case TypeIds_NativeIntArray:
            case TypeIds_CopyOnAccessNativeIntArray:
            case TypeIds_NativeFloatArray:
            case TypeIds_ES5Array:
            case TypeIds_Date:
            case TypeIds_WinRTDate:
            case TypeIds_RegEx:
            case TypeIds_Error:
            case TypeIds_BooleanObject:
            case TypeIds_NumberObject:
            case TypeIds_StringObject:
            case TypeIds_Symbol:
            case TypeIds_SymbolObject:
            //case TypeIds_GlobalObject:
            case TypeIds_ExtensionEnumerator:
            //case TypeIds_ModuleRoot:
            //case TypeIds_HostObject:
            case TypeIds_Arguments:
            case TypeIds_ActivationObject:
            case TypeIds_Map:
            case TypeIds_Set:
            case TypeIds_WeakMap:
            case TypeIds_WeakSet:
            case TypeIds_ArrayIterator:
            case TypeIds_MapIterator:
            case TypeIds_SetIterator:
            case TypeIds_StringIterator:
            case TypeIds_Generator:
            case TypeIds_Promise:
            case TypeIds_Proxy:
                return true;
            default:
                return false;
        }
    }

    RecyclableObject* JavascriptOperators::GetPrototype(RecyclableObject* instance)
    {
        if (JavascriptOperators::GetTypeId(instance) == TypeIds_Null)
        {
            return instance;
        }
        return instance->GetPrototype();
    }

    RecyclableObject* JavascriptOperators::OP_GetPrototype(Var instance, ScriptContext* scriptContext)
    {
        if (TaggedNumber::Is(instance))
        {
            return scriptContext->GetLibrary()->GetNumberPrototype();
        }
        else if (JavascriptOperators::GetTypeId(instance) != TypeIds_Null)
        {
            return JavascriptOperators::GetPrototype(RecyclableObject::FromVar(instance));
        }
        else
        {
            return scriptContext->GetLibrary()->GetNull();
        }
    }

     BOOL JavascriptOperators::OP_BrFncEqApply(Var instance, ScriptContext *scriptContext)
     {
         // JavascriptFunction && !HostDispatch
         if (JavascriptOperators::GetTypeId(instance) == TypeIds_Function)
         {
             FunctionProxy *bod= ((JavascriptFunction*)instance)->GetFunctionProxy();
             if (bod != null)
             {
                 return bod->GetDirectEntryPoint(bod->GetDefaultEntryPointInfo()) == &Js::JavascriptFunction::EntryApply;
             }
             else
             {
                 FunctionInfo* info = ((JavascriptFunction *)instance)->GetFunctionInfo();
                 if (info != null)
                 {
                     return &Js::JavascriptFunction::EntryApply == info->GetOriginalEntryPoint();
                 }
                 else
                 {
                     return false;
                 }
             }
         }

         return false;
     }

     BOOL JavascriptOperators::OP_BrFncNeqApply(Var instance, ScriptContext *scriptContext)
     {
         // JavascriptFunction and !HostDispatch
         if (JavascriptOperators::GetTypeId(instance) == TypeIds_Function)
         {
             FunctionProxy *bod = ((JavascriptFunction *)instance)->GetFunctionProxy();
             if (bod != null)
             {
                 return bod->GetDirectEntryPoint(bod->GetDefaultEntryPointInfo()) != &Js::JavascriptFunction::EntryApply;
             }
             else
             {
                 FunctionInfo* info = ((JavascriptFunction *)instance)->GetFunctionInfo();
                 if (info != null)
                 {
                     return &Js::JavascriptFunction::EntryApply != info->GetOriginalEntryPoint();
                 }
                 else
                 {
                     return true;
                 }
             }
         }

         return true;
     }

    BOOL JavascriptOperators::OP_BrHasSideEffects(int se, ScriptContext* scriptContext)
    {
        return (scriptContext->optimizationOverrides.GetSideEffects() & se) != SideEffects_None;
    }

    BOOL JavascriptOperators::OP_BrNotHasSideEffects(int se, ScriptContext* scriptContext)
    {
        return (scriptContext->optimizationOverrides.GetSideEffects() & se) == SideEffects_None;
    }


    // returns NULL if there is no more elements to enumerate.
    Var JavascriptOperators::OP_BrOnEmpty(ForInObjectEnumerator * aEnumerator)
    {
        PropertyId id;
        return aEnumerator->GetCurrentAndMoveNext(id);
    }

    ForInObjectEnumerator * JavascriptOperators::OP_GetForInEnumerator(Var enumerable, ScriptContext* scriptContext)
    {
        RecyclableObject* enumerableObject;
        bool isCrossSite;
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(enumerable);
        if (GetPropertyObject(enumerable, scriptContext, &enumerableObject))
        {
            isCrossSite = enumerableObject->GetScriptContext() != scriptContext;
        }
        else
        {
            enumerableObject = null;
            isCrossSite = false;
        }
        if (!isCrossSite)
        {
            ForInObjectEnumerator * enumerator  = scriptContext->GetLibrary()->GetAndClearForInEnumeratorCache();
            if(enumerator != NULL)
            {
                enumerator->Initialize(enumerableObject, scriptContext);
                return enumerator;
            }
        }
        return RecyclerNew(scriptContext->GetRecycler(), ForInObjectEnumerator, enumerableObject, scriptContext);
    }

    void JavascriptOperators::OP_ReleaseForInEnumerator(ForInObjectEnumerator * enumerator, ScriptContext* scriptContext)
    {
        // Debugger SetNextStatement may skip OP_GetForInEnumerator and result in NULL ForInObjectEnumerator here. See Win8 391556
        if (enumerator && enumerator->CanBeReused())
        {
            enumerator->Clear();
            scriptContext->GetLibrary()->SetForInEnumeratorCache(enumerator);
        }
    }

    Js::Var JavascriptOperators::OP_CmEq_A(Var a, Var b, ScriptContext* scriptContext)
    {
       return JavascriptBoolean::ToVar(JavascriptOperators::Equal(a, b, scriptContext), scriptContext);
    }

    Var JavascriptOperators::OP_CmNeq_A(Var a, Var b, ScriptContext* scriptContext)
    {
        return JavascriptBoolean::ToVar(JavascriptOperators::NotEqual(a,b,scriptContext), scriptContext);
    }

    Var JavascriptOperators::OP_CmSrEq_A(Var a, Var b, ScriptContext* scriptContext)
    {
       return JavascriptBoolean::ToVar(JavascriptOperators::StrictEqual(a, b, scriptContext), scriptContext);
    }

    Var JavascriptOperators::OP_CmSrEq_String(Var a, Var b, ScriptContext *scriptContext)
    {
        return JavascriptBoolean::ToVar(JavascriptOperators::StrictEqualString(a, b), scriptContext);
    }

    Var JavascriptOperators::OP_CmSrEq_EmptyString(Var a, ScriptContext *scriptContext)
    {
        return JavascriptBoolean::ToVar(JavascriptOperators::StrictEqualEmptyString(a), scriptContext);
    }

    Var JavascriptOperators::OP_CmSrNeq_A(Var a, Var b, ScriptContext* scriptContext)
    {
        return JavascriptBoolean::ToVar(JavascriptOperators::NotStrictEqual(a, b, scriptContext), scriptContext);
    }

    Var JavascriptOperators::OP_CmLt_A(Var a, Var b, ScriptContext* scriptContext)
    {
        return JavascriptBoolean::ToVar(JavascriptOperators::Less(a, b, scriptContext), scriptContext);
    }

    Var JavascriptOperators::OP_CmLe_A(Var a, Var b, ScriptContext* scriptContext)
    {
        return JavascriptBoolean::ToVar(JavascriptOperators::LessEqual(a, b, scriptContext), scriptContext);
    }

    Var JavascriptOperators::OP_CmGt_A(Var a, Var b, ScriptContext* scriptContext)
    {
        return JavascriptBoolean::ToVar(JavascriptOperators::Greater(a, b, scriptContext), scriptContext);
    }

    Var JavascriptOperators::OP_CmGe_A(Var a, Var b, ScriptContext* scriptContext)
    {
        return JavascriptBoolean::ToVar(JavascriptOperators::GreaterEqual(a, b, scriptContext), scriptContext);
    }

    DetachedStateBase* JavascriptOperators::DetachVarAndGetState(Var var)
    {
        switch (GetTypeId(var))
        {
        case TypeIds_ArrayBuffer:
            return Js::ArrayBuffer::FromVar(var)->DetachAndGetState();
        default:
            if (!Js::RecyclableObject::FromVar(var)->IsExternal())
            {
                AssertMsg(false, "We should explicitly have a case statement for each non-external object that can be detached.");
            }
            return nullptr;
        }
    }

    bool JavascriptOperators::IsObjectDetached(Var var)
    {
        switch (GetTypeId(var))
        {
        case TypeIds_ArrayBuffer:
            return Js::ArrayBuffer::FromVar(var)->IsDetached();
        default:
            return false;
        }
    }

    Var JavascriptOperators::NewVarFromDetachedState(DetachedStateBase* state, JavascriptLibrary *library)
    {
        switch (state->GetTypeId())
        {
        case TypeIds_ArrayBuffer:
            return Js::ArrayBuffer::NewFromDetachedState(state, library);
            break;
        default:
            AssertMsg(false, "We should explicitly have a case statement for each object which has detached state.");
            return nullptr;
        }
    }

    DynamicType *
    JavascriptOperators::EnsureObjectLiteralType(ScriptContext* scriptContext, const Js::PropertyIdArray *propIds, DynamicType ** literalType)
    {
        DynamicType * newType = *literalType;
        if (newType != null)
        {
            if (!newType->GetIsShared())
            {
                newType->ShareType();
            }
        }
        else
        {
            DynamicType* objectType =
                FunctionBody::DoObjectHeaderInliningForObjectLiteral(propIds, scriptContext)
                    ?   scriptContext->GetLibrary()->GetObjectHeaderInlinedLiteralType((uint16)propIds->count)
                    :   scriptContext->GetLibrary()->GetObjectLiteralType(
                            static_cast<PropertyIndex>(
                                min(propIds->count, static_cast<uint32>(MaxPreInitializedObjectTypeInlineSlotCount))));
            newType = PathTypeHandlerBase::CreateTypeForNewScObject(scriptContext, objectType, propIds, false);
            *literalType = newType;
        }

        Assert(GetLiteralInlineSlotCapacity(propIds, scriptContext) == newType->GetTypeHandler()->GetInlineSlotCapacity());
        Assert(newType->GetTypeHandler()->GetSlotCapacity() >= 0);
        Assert(GetLiteralSlotCapacity(propIds, scriptContext) == (uint)newType->GetTypeHandler()->GetSlotCapacity());
        return newType;
    }

    Var JavascriptOperators::NewScObjectLiteral(ScriptContext* scriptContext, const Js::PropertyIdArray *propIds, DynamicType ** literalType)
    {
        Assert(propIds->count != 0);
        Assert(!propIds->hadDuplicates);        // duplicates are removed by parser

#ifdef PROFILE_OBJECT_LITERALS
        // Empty objects not counted in the object literal counts
        scriptContext->objectLiteralInstanceCount++;
        if (propIds->count > scriptContext->objectLiteralMaxLength)
        {
            scriptContext->objectLiteralMaxLength = propIds->count;
        }
#endif

        DynamicType* newType = EnsureObjectLiteralType(scriptContext, propIds, literalType);
        DynamicObject* instance = DynamicObject::New(scriptContext->GetRecycler(), newType);

        if (!newType->GetIsShared())
        {
            newType->GetTypeHandler()->SetSingletonInstanceIfNeeded(instance);
        }
#ifdef PROFILE_OBJECT_LITERALS
        else
        {
            scriptContext->objectLiteralCacheCount++;
        }
#endif
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(instance));
        // can't autoproxy here as object literal is not exactly "new" object and cannot be intercepted as proxy.
        return instance;
    }

    uint JavascriptOperators::GetLiteralSlotCapacity(Js::PropertyIdArray const * propIds, ScriptContext *const scriptContext)
    {
        const uint inlineSlotCapacity = GetLiteralInlineSlotCapacity(propIds, scriptContext);
        return DynamicTypeHandler::RoundUpSlotCapacity(propIds->count, static_cast<PropertyIndex>(inlineSlotCapacity));
    }

    uint JavascriptOperators::GetLiteralInlineSlotCapacity(
        Js::PropertyIdArray const * propIds,
        ScriptContext *const scriptContext)
    {
        if (propIds->hadDuplicates)
        {
            return 0;
        }

        return
            FunctionBody::DoObjectHeaderInliningForObjectLiteral(propIds, scriptContext)
                ?   DynamicTypeHandler::RoundUpObjectHeaderInlinedInlineSlotCapacity(static_cast<PropertyIndex>(propIds->count))
                :   DynamicTypeHandler::RoundUpInlineSlotCapacity(
                        static_cast<PropertyIndex>(
                            min(propIds->count, static_cast<uint32>(MaxPreInitializedObjectTypeInlineSlotCount))));
    }

    Var JavascriptOperators::OP_InitCachedScope(Var varFunc, const Js::PropertyIdArray *propIds, DynamicType ** literalType, bool formalsAreLetDecls, ScriptContext *scriptContext)
    {
        ScriptFunction *func = JavascriptGeneratorFunction::Is(varFunc) ?
            JavascriptGeneratorFunction::FromVar(varFunc)->GetGeneratorVirtualScriptFunction() :
            ScriptFunction::FromVar(varFunc);

#ifdef PROFILE_OBJECT_LITERALS
        // Empty objects not counted in the object literal counts
        scriptContext->objectLiteralInstanceCount++;
        if (propIds->count > scriptContext->objectLiteralMaxLength)
        {
            scriptContext->objectLiteralMaxLength = propIds->count;
        }
#endif

        PropertyId cachedFuncCount = ActivationObjectEx::GetCachedFuncCount(propIds);
        PropertyId firstFuncSlot = ActivationObjectEx::GetFirstFuncSlot(propIds);
        PropertyId firstVarSlot = ActivationObjectEx::GetFirstVarSlot(propIds);
        PropertyId lastFuncSlot = Constants::NoProperty;

        if (firstFuncSlot != Constants::NoProperty)
        {
            if (firstVarSlot == Constants::NoProperty)
            {
                lastFuncSlot = propIds->count - 1;
            }
            else
            {
                lastFuncSlot = firstVarSlot - 1;
            }
        }

        DynamicType *type = *literalType;
        if (type != null)
        {
#ifdef PROFILE_OBJECT_LITERALS
            scriptContext->objectLiteralCacheCount++;
#endif
        }
        else
        {
            type = scriptContext->GetLibrary()->GetActivationObjectType();
            if (formalsAreLetDecls)
            {
                uint formalsSlotLimit = (firstFuncSlot != Constants::NoProperty) ? (uint)firstFuncSlot :
                                        (firstVarSlot != Constants::NoProperty) ? (uint)firstVarSlot :
                                        propIds->count;
                type = PathTypeHandlerBase::CreateNewScopeObject(scriptContext, type, propIds, PropertyLet, formalsSlotLimit);
            }
            else
            {
                type = PathTypeHandlerBase::CreateNewScopeObject(scriptContext, type, propIds);
            }
            *literalType = type;
        }
        Var undef = scriptContext->GetLibrary()->GetUndefined();

        ActivationObjectEx *scopeObjEx = func->GetCachedScope();
        if (scopeObjEx && scopeObjEx->IsCommitted())
        {
            scopeObjEx->ReplaceType(type);
            scopeObjEx->SetCommit(false);
#if DBG
            for (uint i = firstVarSlot; i < propIds->count; i++)
            {
                AssertMsg(scopeObjEx->GetSlot(i) == undef, "Var attached to cached scope");
            }
#endif
        }
        else
        {
            ActivationObjectEx *tmp = RecyclerNewPlus(scriptContext->GetRecycler(), (cachedFuncCount == 0 ? 0 : cachedFuncCount - 1) * sizeof(FuncCacheEntry), ActivationObjectEx, type, func, cachedFuncCount, firstFuncSlot, lastFuncSlot);
            if (!scopeObjEx)
            {
                func->SetCachedScope(tmp);
            }
            scopeObjEx = tmp;

            for (uint i = firstVarSlot; i < propIds->count; i++)
            {
                scopeObjEx->SetSlot(SetSlotArguments(propIds->elements[i], i, undef));
            }
        }

        return scopeObjEx;
    }

    void JavascriptOperators::OP_InvalidateCachedScope(void* varEnv, int32 envIndex)
    {
        FrameDisplay *disp = (FrameDisplay*)varEnv;
        RecyclableObject *objScope = RecyclableObject::FromVar(disp->GetItem(envIndex));
        objScope->InvalidateCachedScope();
    }

    void JavascriptOperators::OP_InitCachedFuncs(Var varScope, FrameDisplay *pDisplay, const FuncInfoArray *info, ScriptContext *scriptContext)
    {
        ActivationObjectEx *scopeObj = (ActivationObjectEx*)ActivationObjectEx::FromVar(varScope);
        Assert(scopeObj->GetTypeHandler()->GetInlineSlotCapacity() == 0);
        ScriptFunction *func;
        FuncCacheEntry *entry;
        FunctionProxy  *proxy;
        uint scopeSlot;
        uint funcCount = info->count;

        if (funcCount == 0)
        {
            // Degenerate case: no nested funcs at all
            return;
        }

        if (scopeObj->HasCachedFuncs())
        {
            for (uint i = 0; i < funcCount; i++)
            {
                entry = scopeObj->GetFuncCacheEntry(i);
                func = entry->func;

                proxy = func->GetFunctionProxy();
                if (proxy != proxy->GetFunctionProxy())
                {
                    // The FunctionProxy has changed since the object was cached, e.g., due to execution
                    // of a deferred function through a different object.
                    proxy = proxy->GetFunctionProxy();
                    func->SetFunctionInfo(proxy);
                }

                // Reset the function's type to the default type with no properties
                // Use the cached type on the function proxy rather than the type in the func cache entry
                // TODO: Stop caching the function types in the scope object
                func->ReplaceType(proxy->EnsureDeferredPrototypeType());
                func->ResetConstructorCacheToDefault();

                scopeSlot = info->elements[i].scopeSlot;
                if (scopeSlot != Constants::NoProperty)
                {
                    // Consider: Store property IDs in FuncInfoArray in debug builds so we can properly assert in SetAuxSlot
                    scopeObj->SetAuxSlot(SetSlotArguments(Constants::NoProperty, scopeSlot, entry->func));
                }
            }
            return;
        }

        // No cached functions, so create them and cache them.
        JavascriptFunction *funcParent = scopeObj->GetParentFunc();
        for (uint i = 0; i < funcCount; i++)
        {
            const FuncInfoEntry *entry = &info->elements[i];
            uint nestedIndex = entry->nestedIndex;
            scopeSlot = entry->scopeSlot;

            proxy = funcParent->GetFunctionBody()->GetNestedFunc(nestedIndex);

            func = scriptContext->GetLibrary()->CreateScriptFunction(proxy);

            func->SetEnvironment(pDisplay);
            JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_FUNCTION(func, EtwTrace::GetFunctionId(proxy)));

            scopeObj->SetCachedFunc(i, func);
            if (scopeSlot != Constants::NoProperty)
            {
                // Consider: Store property IDs in FuncInfoArray in debug builds so we can properly assert in SetAuxSlot
                scopeObj->SetAuxSlot(SetSlotArguments(Constants::NoProperty, scopeSlot, func));
            }
        }
    }

    Var JavascriptOperators::AddVarsToArraySegment(SparseArraySegment<Var> * segment, const Js::VarArray *vars)
    {
        uint32 count = vars->count;

        Assert(segment->left == 0);
        Assert(count <= segment->size);

        if(count > segment->length)
        {
            segment->length = count;
        }
        js_memcpy_s(segment->elements, sizeof(Var) * segment->length, vars->elements, sizeof(Var) * count);

        return segment;
    }

    void JavascriptOperators::AddIntsToArraySegment(SparseArraySegment<int32> * segment, const Js::AuxArray<int32> *ints)
    {
        uint32 count = ints->count;

        Assert(segment->left == 0);
        Assert(count <= segment->size);

        if(count > segment->length)
        {
            segment->length = count;
        }
        js_memcpy_s(segment->elements, sizeof(int32) * segment->length, ints->elements, sizeof(int32) * count);
    }

    void JavascriptOperators::AddFloatsToArraySegment(SparseArraySegment<double> * segment, const Js::AuxArray<double> *doubles)
    {
        uint32 count = doubles->count;

        Assert(segment->left == 0);
        Assert(count <= segment->size);

        if(count > segment->length)
        {
            segment->length = count;
        }
        js_memcpy_s(segment->elements, sizeof(double) * segment->length, doubles->elements, sizeof(double) * count);
    }

    RecyclableObject * JavascriptOperators::GetPrototypeObject(RecyclableObject * constructorFunction, ScriptContext * scriptContext)
    {
        Var prototypeProperty = JavascriptOperators::GetProperty(constructorFunction, PropertyIds::prototype, scriptContext);
        RecyclableObject* prototypeObject;
        PrototypeObject(prototypeProperty, constructorFunction, scriptContext, &prototypeObject);
        return prototypeObject;
    }

    RecyclableObject * JavascriptOperators::GetPrototypeObjectForConstructorCache(RecyclableObject * constructor, ScriptContext* requestContext, bool& canBeCached)
    {
        PropertyValueInfo info;
        Var prototypeValue;
        RecyclableObject* prototypeObject;

        canBeCached = false;

        // Do a local property lookup.  Since a function's prototype property is a non-configurable data property, we don't need to worry
        // about the prototype being an accessor property, whose getter returns different values every time it's called.
        if (constructor->GetProperty(constructor, PropertyIds::prototype, &prototypeValue, &info, requestContext))
        {
            if (!JavascriptOperators::PrototypeObject(prototypeValue, constructor, requestContext, &prototypeObject))
            {
                // The value returned by the property lookup is not a valid prototype object, default to object prototype.
                Assert(prototypeObject == constructor->GetLibrary()->GetObjectPrototype());
            }

            // For these scenarios, we do not want to populate the cache.
            if (constructor->GetScriptContext() != requestContext || info.GetInstance() != constructor)
            {
                return prototypeObject;
            }
        }
        else
        {
            // It's ok to cache Object.prototype, because Object.prototype cannot be overwritten.
            prototypeObject = constructor->GetLibrary()->GetObjectPrototype();
        }

        canBeCached = true;
        return prototypeObject;
    }

    bool JavascriptOperators::PrototypeObject(Var prototypeProperty, RecyclableObject * constructorFunction, ScriptContext * scriptContext, RecyclableObject** prototypeObject)
    {
        TypeId prototypeType = JavascriptOperators::GetTypeId(prototypeProperty);

        if (JavascriptOperators::IsObjectType(prototypeType))
        {
            *prototypeObject = RecyclableObject::FromVar(prototypeProperty);
            return true;
        }
        *prototypeObject = constructorFunction->GetLibrary()->GetObjectPrototype();
        return false;
    }

    FunctionInfo* JavascriptOperators::GetConstructorFunctionInfo(Var instance, ScriptContext * scriptContext)
    {
        TypeId typeId = JavascriptOperators::GetTypeId(instance);
        if (typeId == TypeIds_Function)
        {
            JavascriptFunction * function =  JavascriptFunction::FromVar(instance);
            return function->GetFunctionInfo();
        }
        if (typeId != TypeIds_HostDispatch && typeId != TypeIds_Proxy)
        {
            if (typeId == TypeIds_Null)
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
            }

            JavascriptError::ThrowTypeError(scriptContext, VBSERR_ActionNotSupported);
        }
        return nullptr;
    }

    Var JavascriptOperators::NewJavascriptObjectNoArg(ScriptContext* requestContext)
    {
        DynamicObject * newObject = requestContext->GetLibrary()->CreateObject(true);
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(newObject));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            newObject = DynamicObject::FromVar(JavascriptProxy::AutoProxyWrapper(newObject));
        }
#endif
        return newObject;
    }

    Var JavascriptOperators::NewJavascriptArrayNoArg(ScriptContext* requestContext)
    {
        JavascriptArray * newArray = requestContext->GetLibrary()->CreateArray();
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(newArray));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            newArray = static_cast<JavascriptArray*>(JavascriptProxy::AutoProxyWrapper(newArray));
        }
#endif
        return newArray;
    }

    Var JavascriptOperators::NewScObjectNoArgNoCtor(Var instance, ScriptContext * requestContext)
    {
        RecyclableObject * object = RecyclableObject::FromVar(instance);
        FunctionInfo* functionInfo = JavascriptOperators::GetConstructorFunctionInfo(instance, requestContext);
        Assert(functionInfo != &JavascriptObject::EntryInfo::NewInstance); // built-ins are not inlined
        Assert(functionInfo != &JavascriptArray::EntryInfo::NewInstance); // built-ins are not inlined

        return functionInfo != null ?
            JavascriptOperators::NewScObjectCommon(object, functionInfo, requestContext) :
            JavascriptOperators::NewScObjectHostDispatchOrProxy(object, requestContext);
    }

    Var JavascriptOperators::NewScObjectNoArg(Var instance, ScriptContext * requestContext)
    {
        if (JavascriptProxy::Is(instance))
        {
            Arguments args(CallInfo(CallFlags_New, 1), &instance);
            JavascriptProxy* proxy = JavascriptProxy::FromVar(instance);
            return proxy->ConstructorTrap(args, requestContext, 0);
        }

        FunctionInfo* functionInfo = JavascriptOperators::GetConstructorFunctionInfo(instance, requestContext);
        RecyclableObject * object = RecyclableObject::FromVar(instance);

        if (functionInfo == &JavascriptObject::EntryInfo::NewInstance)
        {
            // Fast path for new Object()
            Assert((functionInfo->GetAttributes() & FunctionInfo::ErrorOnNew) == 0);
            JavascriptLibrary* library = object->GetLibrary();

            DynamicObject * newObject = library->CreateObject(true);
            JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(newObject));
#if ENABLE_DEBUG_CONFIG_OPTIONS
            if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
            {
                newObject = DynamicObject::FromVar(JavascriptProxy::AutoProxyWrapper(newObject));
            }
#endif

#if DBG
            DynamicType* newObjectType = newObject->GetDynamicType();
            Assert(newObjectType->GetIsShared());

            JavascriptFunction* constructor = JavascriptFunction::FromVar(instance);
            // TODO (jedmiad): Create Library::GetBuiltInConstructorCache
            //Assert(constructor->GetConstructorCache() == constructor->GetLibrary()->GetBuiltInConstructorCache());
            Assert(!constructor->GetConstructorCache()->NeedsUpdateAfterCtor());
#endif

            ScriptContext * scriptContext = library->GetScriptContext();
            if (scriptContext != requestContext)
            {
                CrossSite::MarshalDynamicObjectAndPrototype(requestContext, newObject);
            }

            return newObject;
        }
        else if (functionInfo == &JavascriptArray::EntryInfo::NewInstance)
        {
            Assert((functionInfo->GetAttributes() & FunctionInfo::ErrorOnNew) == 0);
            JavascriptLibrary* library = object->GetLibrary();

            JavascriptArray * newArray = library->CreateArray();
            JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(newArray));
#if ENABLE_DEBUG_CONFIG_OPTIONS
            if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
            {
                newArray = static_cast<JavascriptArray*>(JavascriptProxy::AutoProxyWrapper(newArray));
            }
#endif

#if DBG
            DynamicType* newArrayType = newArray->GetDynamicType();
            Assert(newArrayType->GetIsShared());

            JavascriptFunction* constructor = JavascriptFunction::FromVar(instance);
            // TODO (jedmiad): Create Library::GetBuiltInConstructorCache
            //Assert(constructor->GetConstructorCache() == constructor->GetLibrary()->GetBuiltInConstructorCache());
            Assert(!constructor->GetConstructorCache()->NeedsUpdateAfterCtor());
#endif

            ScriptContext * scriptContext = library->GetScriptContext();
            if (scriptContext != requestContext)
            {
                CrossSite::MarshalDynamicObjectAndPrototype(requestContext, newArray);
            }
            return newArray;
        }

        Var newObject = functionInfo != null ?
            JavascriptOperators::NewScObjectCommon(object, functionInfo, requestContext) :
            JavascriptOperators::NewScObjectHostDispatchOrProxy(object, requestContext);

        Var returnVar = object->GetEntryPoint()(object, CallInfo(CallFlags_New, 1), newObject);
        if (JavascriptOperators::IsObject(returnVar))
        {
            newObject = returnVar;
        }

        ConstructorCache * constructorCache = null;
        if (JavascriptFunction::Is(instance))
        {
            constructorCache = JavascriptFunction::FromVar(instance)->GetConstructorCache();
        }

        if (constructorCache != null && constructorCache->NeedsUpdateAfterCtor())
        {
            JavascriptOperators::UpdateNewScObjectCache(object, newObject, requestContext);
        }

#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            newObject = DynamicObject::FromVar(JavascriptProxy::AutoProxyWrapper(newObject));
            // this might come from a different scriptcontext.
            newObject = CrossSite::MarshalVar(requestContext, newObject);
        }
#endif

        return newObject;
    }

    Var JavascriptOperators::NewScObjectNoCtor(Var instance, ScriptContext * requestContext)
    {
        FunctionInfo* functionInfo = JavascriptOperators::GetConstructorFunctionInfo(instance, requestContext);
        return functionInfo != null ?
            JavascriptOperators::NewScObjectCommon(RecyclableObject::FromVar(instance), functionInfo, requestContext) :
            JavascriptOperators::NewScObjectHostDispatchOrProxy(RecyclableObject::FromVar(instance), requestContext);
    }

    Var JavascriptOperators::NewScObjectHostDispatchOrProxy(RecyclableObject * function, ScriptContext * requestContext)
    {
        ScriptContext* functionScriptContext = function->GetScriptContext();

        if (JavascriptProxy::Is(function))
        {
            // We can still call into NewScObjectNoCtor variations in JIT code for performance; however for proxy we dont
            // really need the new object as the trap will handle the "this" pointer separately. pass back nullptr to ensure
            // failure in invalid case.
            return  nullptr;
        }
        RecyclableObject * prototype = JavascriptOperators::GetPrototypeObject(function, functionScriptContext);
        prototype = RecyclableObject::FromVar(CrossSite::MarshalVar(requestContext, prototype));
        Var object = requestContext->GetLibrary()->CreateObject(prototype);
        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(object));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            object = DynamicObject::FromVar(JavascriptProxy::AutoProxyWrapper(object));
        }
#endif
        return object;
    }

    Var JavascriptOperators::NewScObjectCommon(RecyclableObject * function, FunctionInfo* functionInfo, ScriptContext * requestContext)
    {
        // TODO (jedmiad): Allow for the cache to be repopulated if the type got collected, and a new one got populated with
        // the same number of inlined slots. This requires that the JIT-ed code actually load the type from the cache
        // (instead of hard-coding it), but it can (and must) keep the hard-coded number of inline slots.
        // TODO (jedmiad): Consider also not pinning the type in the cache.  This can be done by using a registration based
        // weak reference (we need to control the memory address), which we don't yet have, or by allocating the cache from
        // the inline cache arena to allow it to be zeroed, but retain a recycler-allocated portion to hold on to the size of
        // inlined slots.

        JavascriptFunction* constructor = JavascriptFunction::FromVar(function);

        if (BinaryFeatureControl::LanguageService() && constructor->IsScriptFunction() && functionInfo->IsDeferred())
        {
            ((ScriptFunction *)constructor)->EnsureCopyFunction();
        }

        ConstructorCache* constructorCache = constructor->GetConstructorCache();
        AssertMsg(constructorCache->GetScriptContext() == null || constructorCache->GetScriptContext() == constructor->GetScriptContext(),
            "Why did we populate a constructor cache with a mismatched script context?");

        Assert(constructorCache != null);
        DynamicType* type = constructorCache->GetGuardValueAsType();
        if (type != null && constructorCache->GetScriptContext() == requestContext)
        {
#if DBG
            bool cachedProtoCanBeCached;
            Assert(type->GetPrototype() == JavascriptOperators::GetPrototypeObjectForConstructorCache(constructor, requestContext, cachedProtoCanBeCached));
            Assert(cachedProtoCanBeCached);
            Assert(type->GetIsShared());
#endif

#if DBG_DUMP
            TraceUseConstructorCache(constructorCache, constructor, true);
#endif
            Var object = DynamicObject::New(requestContext->GetRecycler(), type);
            JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(object));
#if ENABLE_DEBUG_CONFIG_OPTIONS
            if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
            {
                object = DynamicObject::FromVar(JavascriptProxy::AutoProxyWrapper(object));
            }
#endif
            return object;
        }

        if (constructorCache->SkipDefaultNewObject())
        {
            Assert(!constructorCache->NeedsUpdateAfterCtor());

#if DBG_DUMP
            TraceUseConstructorCache(constructorCache, constructor, true);
#endif

            return null;
        }

#if DBG_DUMP
        TraceUseConstructorCache(constructorCache, constructor, false);
#endif

        ScriptContext* constructorScriptContext = function->GetScriptContext();
        // we shouldn't try to call the constructor if it's closed already.
        Assert(!constructorScriptContext->GetThreadContext()->IsDisableImplicitException());
        constructorScriptContext->VerifyAlive(TRUE, requestContext);

        FunctionInfo::Attributes attributes = functionInfo->GetAttributes();
        if (attributes & FunctionInfo::ErrorOnNew)
        {
            JavascriptError::ThrowTypeError(requestContext, JSERR_ErrorOnNew);
        }

        // Slow path
        FunctionProxy * ctorProxy = constructor->GetFunctionProxy();
        FunctionBody * functionBody = ctorProxy != null ? ctorProxy->EnsureDeserialized()->Parse() : null;

        if (attributes & FunctionInfo::SkipDefaultNewObject)
        {
            // The constructor doesn't use the default new object.
            // TODO (jedmiad): Add tracing.
            #pragma prefast(suppress:__WARNING_LOGICALORNONZERO, "DevDiv bug 830883. False positive when PHASE_OFF is #defined as '(false)'.")
            if (!PHASE_OFF1(ConstructorCachePhase) && (functionBody == null || !PHASE_OFF(ConstructorCachePhase, functionBody)))
            {
                constructorCache = constructor->EnsureValidConstructorCache();
                constructorCache->PopulateForSkipDefaultNewObject(constructorScriptContext);

#if DBG_DUMP
                if ((functionBody != null && PHASE_TRACE(Js::ConstructorCachePhase, functionBody)) || (functionBody == null && PHASE_TRACE1(Js::ConstructorCachePhase)))
                {
                    const wchar_t* ctorName = functionBody != null ? functionBody->GetDisplayName() : L"<unknown>";
                    wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

                    Output::Print(L"CtorCache: populated cache (0x%p) for ctor %s (%s): ", constructorCache, ctorName,
                        functionBody ? functionBody->GetDebugNumberSet(debugStringBuffer) : L"(null)");
                    constructorCache->Dump();
                    Output::Print(L"\n");
                    Output::Flush();
                }
#endif

            }

            Assert(!constructorCache->NeedsUpdateAfterCtor());
            return null;
        }

        // TODO (jedmiad): Create some form of PatchGetProtoObjForCtorCache, which actually caches the prototype object in the constructor cache.
        // Make sure that it does NOT populate the guard field.  On the slow path (the only path for cross-context calls) we can do a faster lookup
        // after we fail the guard check.  When invalidating the cache for proto change, make sure we zap the prototype field of the cache in
        // addition to the guard value.
        bool prototypeCanBeCached;
        RecyclableObject* prototype = JavascriptOperators::GetPrototypeObjectForConstructorCache(function, constructorScriptContext, prototypeCanBeCached);
        prototype = RecyclableObject::FromVar(CrossSite::MarshalVar(requestContext, prototype));

        DynamicObject* newObject = requestContext->GetLibrary()->CreateObject(prototype, 8);

        JSETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(newObject));
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.IsEnabled(Js::autoProxyFlag))
        {
            newObject = DynamicObject::FromVar(JavascriptProxy::AutoProxyWrapper(newObject));
        }
#endif

        Assert(newObject->GetTypeHandler()->GetPropertyCount() == 0);

        if (prototypeCanBeCached && functionBody != null && requestContext == constructorScriptContext &&
            !Js::JavascriptProxy::Is(newObject) &&
            !PHASE_OFF1(ConstructorCachePhase) && !PHASE_OFF(ConstructorCachePhase, functionBody))
        {
            DynamicType* newObjectType = newObject->GetDynamicType();
            // Initial type (without any properties) should always be shared up-front.  This allows us to populate the cache right away.
            Assert(newObjectType->GetIsShared());

            // Populate the cache here and set the updateAfterCtor flag.  This way, if the ctor is called recursively the
            // recursive calls will hit the cache and use the initial type.  On the unwind path, we will update the cache
            // after the innermost ctor and clear the flag.  After subsequent ctors we won't attempt an update anymore.
            // As long as the updateAfterCtor flag is set it is safe to update the cache, because it would not have been
            // hard-coded in the JIT-ed code.
            constructorCache = constructor->EnsureValidConstructorCache();
            constructorCache->Populate(newObjectType, constructorScriptContext, functionBody->GetHasNoExplicitReturnValue(), true);
            Assert(constructorCache->IsConsistent());

#if DBG_DUMP
            if ((functionBody != null && PHASE_TRACE(Js::ConstructorCachePhase, functionBody)) || (functionBody == null && PHASE_TRACE1(Js::ConstructorCachePhase)))
            {
                const wchar_t* ctorName = functionBody != null ? functionBody->GetDisplayName() : L"<unknown>";
                wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

                Output::Print(L"CtorCache: populated cache (0x%p) for ctor %s (%s): ", constructorCache, ctorName,
                    functionBody ? functionBody->GetDebugNumberSet(debugStringBuffer) : L"(null)");
                constructorCache->Dump();
                Output::Print(L"\n");
                Output::Flush();
            }
#endif
        }
        else
        {
#if DBG_DUMP
            if ((functionBody != null && PHASE_TRACE(Js::ConstructorCachePhase, functionBody)) || (functionBody == null && PHASE_TRACE1(Js::ConstructorCachePhase)))
            {
                const wchar_t* ctorName = functionBody != null ? functionBody->GetDisplayName() : L"<unknown>";
                wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

                Output::Print(L"CtorCache: did not populate cache (0x%p) for ctor %s (%s), because %s: prototype = 0x%p, functionBody = 0x%p, ctor context = 0x%p, request context = 0x%p",
                    constructorCache, ctorName, functionBody ? functionBody->GetDebugNumberSet(debugStringBuffer) : L"(null)",
                    !prototypeCanBeCached ? L"prototype cannot be cached" :
                    functionBody == null ? L"function has no body" :
                    requestContext != constructorScriptContext ? L"of cross-context call" : L"constructor cache phase is off",
                    prototype, functionBody, constructorScriptContext, requestContext);
                Output::Print(L"\n");
                Output::Flush();
            }
#endif
        }

        return newObject;
    }

    void JavascriptOperators::UpdateNewScObjectCache(Var function, Var instance, ScriptContext* requestContext)
    {
        JavascriptFunction* constructor = JavascriptFunction::FromVar(function);
        if(constructor->GetScriptContext() != requestContext)
        {
            // The cache is populated only when the constructor function's context is the same as the calling context. However,
            // the cached type is not finalized yet and may not be until multiple calls to the constructor have been made (see
            // flag ConstructorCallsRequiredToFinalizeCachedType). A subsequent call to the constructor may be made from a
            // different context, so ignore those cross-context calls and wait for the constructor to be called from its own
            // context again to finalize the cached type.
            return;
        }

        // Review (jedmiad): What happens if the cache got invalidated between NewScObject and here?
        // Should we allocate new?  Should we mark it as polymorphic?
        ConstructorCache* constructorCache = constructor->GetConstructorCache();
        Assert(constructorCache->IsConsistent());
        Assert(!ConstructorCache::IsDefault(constructorCache));
        AssertMsg(constructorCache->GetScriptContext() == constructor->GetScriptContext(), "Why did we populate a constructor cache with a mismatched script context?");
        AssertMsg(constructorCache->IsPopulated(), "Why are we updating a constructor cache that hasn't been populated?");

        // The presence of the updateAfterCtor flag guarantees that this cache hasn't been used in JIT-ed fast path.  Even, if the
        // cache is invalidated, this flag is not changed.
        AssertMsg(constructorCache->NeedsUpdateAfterCtor(), "Why are we updating a constructor cache that doesn't need to be updated?");

        const bool finalizeCachedType =
            constructorCache->CallCount() >= CONFIG_FLAG(ConstructorCallsRequiredToFinalizeCachedType);
        if(!finalizeCachedType)
        {
            constructorCache->IncCallCount();
        }
        else
        {
            constructorCache->ClearUpdateAfterCtor();
        }

        FunctionBody* constructorBody = constructor->GetFunctionBody();
        AssertMsg(constructorBody != null, "Constructor function doesn't have a function body.");
        Assert(RecyclableObject::Is(instance));

        // The cache might have been invalidated between NewScObjectCommon and UpdateNewScObjectCache.  This could occur, for example, if
        // the constructor updates its own prototype property.  If that happens we don't want to re-populate it here.  A new cache will
        // be created when the constructor is called again.
        if (constructorCache->IsInvalidated())
        {
#if DBG_DUMP
            TraceUpdateConstructorCache(constructorCache, constructorBody, false, L"because cache is invalidated");
#endif
            return;
        }

        Assert(constructorCache->GetGuardValueAsType() != null);

        if (DynamicType::Is(RecyclableObject::FromVar(instance)->GetTypeId()))
        {
            DynamicObject *object = DynamicObject::FromVar(instance);
            DynamicType* type = object->GetDynamicType();
            DynamicTypeHandler* typeHandler = type->GetTypeHandler();

            if (constructorBody->GetHasOnlyThisStmts())
            {
                if (typeHandler->IsSharable())
                {
#if DBG
                    bool cachedProtoCanBeCached;
                    Assert(type->GetPrototype() == JavascriptOperators::GetPrototypeObjectForConstructorCache(constructor, requestContext, cachedProtoCanBeCached));
                    Assert(cachedProtoCanBeCached);
                    Assert(type->GetScriptContext() == constructorCache->GetScriptContext());
                    Assert(type->GetPrototype() == constructorCache->GetType()->GetPrototype());
#endif

                    typeHandler->SetMayBecomeShared();
                    // TODO (jedmiad): Remove only this for delayed type sharing.
                    type->ShareType();

                    DynamicProfileInfo* profileInfo = constructorBody->HasDynamicProfileInfo() ? constructorBody->GetAnyDynamicProfileInfo() : null;
                    if ((profileInfo != null && profileInfo->GetImplicitCallFlags() <= ImplicitCall_None) ||
                        CheckIfPrototypeChainHasOnlyWritableDataProperties(type->GetPrototype()))
                    {
                        Assert(typeHandler->GetPropertyCount() < Js::PropertyIndexRanges<PropertyIndex>::MaxValue);

                        for (PropertyIndex pi = 0; pi < typeHandler->GetPropertyCount(); pi++)
                        {
                            requestContext->RegisterConstructorCache(typeHandler->GetPropertyId(requestContext, pi), constructorCache);
                        }

                        Assert(constructorBody->GetUtf8SourceInfo()->GetIsLibraryCode() || !constructor->GetScriptContext()->IsInDebugMode());

                        if (constructorCache->TryUpdateAfterConstructor(type, constructor->GetScriptContext()))
                        {
#if DBG_DUMP
                            TraceUpdateConstructorCache(constructorCache, constructorBody, true, L"");
#endif
                        }
                        else
                        {
#if DBG_DUMP
                            TraceUpdateConstructorCache(constructorCache, constructorBody, false, L"because number of slots > MaxCachedSlotCount");
#endif
                        }
                    }
#if DBG_DUMP
                    else
                    {
                        if (profileInfo &&
                            ((profileInfo->GetImplicitCallFlags() & ~(Js::ImplicitCall_External | Js::ImplicitCall_Accessor)) == 0) &&
                            profileInfo != nullptr && CheckIfPrototypeChainHasOnlyWritableDataProperties(type->GetPrototype()) &&
                            Js::Configuration::Global.flags.Trace.IsEnabled(Js::HostOptPhase))
                        {
                            const wchar_t* ctorName = constructorBody->GetDisplayName();
                            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
                            Output::Print(L"CtorCache: %s cache (0x%p) for ctor %s (#%u) did not update because external call",
                                constructorCache, constructorBody, ctorName, constructorBody ? constructorBody->GetDebugNumberSet(debugStringBuffer) : L"(null)");
                            Output::Print(L"\n");
                            Output::Flush();
                        }
                    }
#endif
                }
                else
                {
                    // Dynamic type created is not sharable.
                    // So in future don't try to check for "this assignment optimization".
                    constructorBody->SetHasOnlyThisStmts(false);
#if DBG_DUMP
                    TraceUpdateConstructorCache(constructorCache, constructorBody, false, L"because final type is not shareable");
#endif
                }
            }
            else
            {
#if DBG_DUMP
                TraceUpdateConstructorCache(constructorCache, constructorBody, false, L"because ctor has not only this statements");
#endif
            }
        }
        else
        {
            // Even though this constructor apparently returned something other than the default object we created,
            // it still makes sense to cache the parameters of the default object, since we must create it every time, anyway.
#if DBG_DUMP
            TraceUpdateConstructorCache(constructorCache, constructorBody, false, L"because ctor return a non-object value");
#endif
            return;
        }

        // Whatever the constructor returned, if we're caching a type we want to be sure we shrink its inline slot capacity.
        if (finalizeCachedType && constructorCache->IsEnabled())
        {
            DynamicType* cachedType = constructorCache->NeedsTypeUpdate() ? constructorCache->GetPendingType() : constructorCache->GetType();
            DynamicTypeHandler* cachedTypeHandler = cachedType->GetTypeHandler();

            // Consider: We could delay inline slot capacity shrinking until the second time this constructor is invoked.  In some cases
            // this might permit more properties to remain inlined if the objects grow after constructor.  This would require flagging
            // the cache as special (already possible) and forcing the shrinking during work item creation if we happen to JIT this
            // constructor while the cache is in this special state.
            if (cachedTypeHandler->GetInlineSlotCapacity())
            {
#if DBG_DUMP
                int inlineSlotCapacityBeforeShrink = cachedTypeHandler->GetInlineSlotCapacity();
#endif

                // Note that after the cache has been updated and might have been used in the JIT-ed code, it is no longer legal to
                // shrink the inline slot capacity of the type.  That's because we allocate memory for a fixed number of inlined properties
                // and if that number changed on the type, this update wouldn't get reflected in JIT-ed code and we would allocate objects
                // of a wrong size.  This could conceivably happen if the original object got collected, and with it some of the successor
                // types also.  If then another constructor has the same prototype and needs to populate its own cache, it would attempt to
                // shrink inlined slots again.  If all surviving type handlers have smaller inline slot capacity, we would shrink it further.
                // To address this problem the type handler has a bit indicating its inline slots have been shrunk already.  If that bit is
                // set ShrinkSlotAndInlineSlotCapacity does nothing.
                cachedTypeHandler->ShrinkSlotAndInlineSlotCapacity();
                constructorCache->UpdateInlineSlotCount();

#if DBG_DUMP
                Assert(inlineSlotCapacityBeforeShrink >= cachedTypeHandler->GetInlineSlotCapacity());
                if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::InlineSlotsPhase))
                {
                    if (inlineSlotCapacityBeforeShrink != cachedTypeHandler->GetInlineSlotCapacity())
                    {
                        wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

                        Output::Print(L"Inline slot capacity shrunk: Function:%04s Before:%d After:%d\n",
                            constructorBody->GetDebugNumberSet(debugStringBuffer), inlineSlotCapacityBeforeShrink, cachedTypeHandler->GetInlineSlotCapacity());
                    }
                }
#endif
            }
        }
    }

    void JavascriptOperators::TraceUseConstructorCache(const ConstructorCache* ctorCache, const JavascriptFunction* ctor, bool isHit)
    {
#if DBG_DUMP
        // We are under debug, so we can incurr the extra check here.
        FunctionProxy* ctorBody = ctor->GetFunctionProxy();
        if (ctorBody != null && !ctorBody->GetScriptContext()->IsClosed())
        {
            ctorBody = ctorBody->EnsureDeserialized();
        }
        if ((ctorBody != null && PHASE_TRACE(Js::ConstructorCachePhase, ctorBody)) || (ctorBody == null && PHASE_TRACE1(Js::ConstructorCachePhase)))
        {
            const wchar_t* ctorName = ctorBody != null ? ctorBody->GetDisplayName() : L"<unknown>";
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

            Output::Print(L"CtorCache: %s cache (0x%p) for ctor %s (%s): ", isHit ? L"hit" : L"missed", ctorCache, ctorName,
                ctorBody ? ctorBody->GetDebugNumberSet(debugStringBuffer) : L"(null)");
            ctorCache->Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
    }

    void JavascriptOperators::TraceUpdateConstructorCache(const ConstructorCache* ctorCache, const FunctionBody* ctorBody, bool updated, const wchar_t* reason)
    {
#if DBG_DUMP
        if (PHASE_TRACE(Js::ConstructorCachePhase, ctorBody))
        {
            const wchar_t* ctorName = ctorBody->GetDisplayName();
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];

            Output::Print(L"CtorCache: %s cache (0x%p) for ctor %s (%s)%s %s: ",
                updated ? L"updated" : L"did not update", ctorBody, ctorName,
                ctorBody ? const_cast<Js::FunctionBody *>(ctorBody)->GetDebugNumberSet(debugStringBuffer) : L"(null)",
                updated ? L"" : L", because" , reason);
            ctorCache->Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
    }

    Var JavascriptOperators::NewScObject(const Var callee, const Arguments args, ScriptContext *const scriptContext, const Js::AuxArray<uint32> *spreadIndices)
    {
        Assert(callee);
        Assert(args.Info.Count != 0);
        Assert(scriptContext);

        // Always save and restore implicit call flags when calling out
        // REVIEW: Can we avoid it if we don't collect dynamic profile info?
        ThreadContext *const threadContext = scriptContext->GetThreadContext();
        const ImplicitCallFlags savedImplicitCallFlags = threadContext->GetImplicitCallFlags();

        const Var newVarInstance = JavascriptFunction::CallAsConstructor(callee, args, scriptContext, spreadIndices);

        threadContext->SetImplicitCallFlags(savedImplicitCallFlags);
        return newVarInstance;
    }

    Js::GlobalObject * JavascriptOperators::OP_LdRoot(ScriptContext* scriptContext)
    {
        return scriptContext->GetGlobalObject();
    }

    Js::ModuleRoot * JavascriptOperators::GetModuleRoot(int moduleID, ScriptContext* scriptContext)
    {
        Assert(moduleID != kmodGlobal);
        JavascriptLibrary* library = scriptContext->GetLibrary();
        HostObjectBase *hostObject = library->GetGlobalObject()->GetHostObject();
        if (hostObject)
        {
            Js::ModuleRoot * moduleRoot = hostObject->GetModuleRoot(moduleID);
            Assert(!CrossSite::NeedMarshalVar(moduleRoot, scriptContext));
            return moduleRoot;
        }
        HostScriptContext *hostScriptContext = scriptContext->GetHostScriptContext();
        if (hostScriptContext)
        {
            Js::ModuleRoot * moduleRoot = hostScriptContext->GetModuleRoot(moduleID);
            Assert(!CrossSite::NeedMarshalVar(moduleRoot, scriptContext));
            return moduleRoot;
        }
        Assert(FALSE);
        return null;
    }

    Var JavascriptOperators::OP_LoadModuleRoot(int moduleID, ScriptContext* scriptContext)
    {
        Js::ModuleRoot * moduleRoot = GetModuleRoot(moduleID, scriptContext);
        if (moduleRoot)
        {
            return moduleRoot;
        }
        Assert(false);
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var JavascriptOperators::OP_LdNull(ScriptContext* scriptContext)
    {
        return scriptContext->GetLibrary()->GetNull();
    }

    Var JavascriptOperators::OP_LdUndef(ScriptContext* scriptContext)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var JavascriptOperators::OP_LdNaN(ScriptContext* scriptContext)
    {
        return scriptContext->GetLibrary()->GetNaN();
    }

    Var JavascriptOperators::OP_LdInfinity(ScriptContext* scriptContext)
    {
        return scriptContext->GetLibrary()->GetPositiveInfinite();
    }

    void JavascriptOperators::BuildHandlerScope(Var argThis, RecyclableObject * hostObject, FrameDisplay * pDisplay, ScriptContext * scriptContext)
    {
        Assert(argThis != null);
        pDisplay->SetItem(0, TaggedNumber::Is(argThis) ? scriptContext->GetLibrary()->CreateNumberObject(argThis) : argThis);
        uint16 i = 1;

        Var aChild = argThis;
        uint16 length = pDisplay->GetLength();

        // Now add any parent scopes
        // We need to support the namespace parent lookup in both fastDOM on and off scenario.
        while (aChild != NULL)
        {
            Var aParent = hostObject->GetNamespaceParent(aChild);

            if (aParent == null)
            {
                break;
            }
            aParent = CrossSite::MarshalVar(scriptContext, aParent);
            if (i == length)
            {
                length += 8;
                FrameDisplay * tmp = RecyclerNewPlus(scriptContext->GetRecycler(), length * sizeof(void*), FrameDisplay, length);
                js_memcpy_s((char*)tmp + tmp->GetOffsetOfScopes(), tmp->GetLength() * sizeof(void *), (char*)pDisplay + pDisplay->GetOffsetOfScopes(), pDisplay->GetLength() * sizeof(void*));
                pDisplay = tmp;
            }
            pDisplay->SetItem(i, aParent);
            aChild = aParent;
            i++;
        }

        Assert(i <= pDisplay->GetLength());
        pDisplay->SetLength(i);
    }
    FrameDisplay * JavascriptOperators::OP_LdHandlerScope(Var argThis, ScriptContext* scriptContext)
    {

        // The idea here is to build a stack of nested scopes in the form of a JS array.
        //
        // The scope stack for an event handler looks like this:
        //
        // implicit "this"
        // implicit namespace parent scopes

        // Put the implicit "this"
        if (argThis != NULL)
        {
            RecyclableObject* hostObject = scriptContext->GetGlobalObject()->GetHostObject();
            if (hostObject == null)
            {
                hostObject = scriptContext->GetGlobalObject()->GetDirectHostObject();
            }
            if (hostObject != null)
            {
                uint16 length = 7;
                FrameDisplay *pDisplay =
                    RecyclerNewPlus(scriptContext->GetRecycler(), length * sizeof(void*), FrameDisplay, length);
                BuildHandlerScope(argThis, hostObject, pDisplay, scriptContext);
                return pDisplay;
            }
        }

        return const_cast<FrameDisplay *>(&Js::NullFrameDisplay);
    }

    FrameDisplay* JavascriptOperators::OP_LdFrameDisplay(void *argHead, void *argEnv, ScriptContext* scriptContext)
    {
        // Build a display of nested frame objects.
        // argHead is the current scope; argEnv is either the lone trailing scope or an array of scopes
        // which we append to the new display.

        // Note that there are cases in which a function with no local frame must construct a display to pass
        // to the function(s) nested within it. In such a case, argHead will be a null object, and it's not
        // strictly necessary to include it. But such cases are rare and not perf critical, so it's not
        // worth the extra complexity to notify the nested functions that they can "skip" this slot in the
        // frame display when they're loading scopes nested outside it.

        FrameDisplay *pDisplay = null;
        FrameDisplay *envDisplay = (FrameDisplay*)argEnv;
        uint16 length = envDisplay->GetLength() + 1;

        pDisplay = RecyclerNewPlus(scriptContext->GetRecycler(), length * sizeof(void*), FrameDisplay, length);
        for (int j = 0; j < length - 1; j++)
        {
            pDisplay->SetItem(j + 1, envDisplay->GetItem(j));
        }

        pDisplay->SetItem(0, argHead);

        return pDisplay;
    }

    FrameDisplay* JavascriptOperators::OP_LdFrameDisplayNoParent(void *argHead, ScriptContext* scriptContext)
    {
        return OP_LdFrameDisplay(argHead, (void*)&NullFrameDisplay, scriptContext);
    }

    FrameDisplay* JavascriptOperators::OP_LdStrictFrameDisplay(void *argHead, void *argEnv, ScriptContext* scriptContext)
    {
        FrameDisplay * pDisplay = OP_LdFrameDisplay(argHead, argEnv, scriptContext);
        pDisplay->SetStrictMode(true);

        return pDisplay;
    }

    FrameDisplay* JavascriptOperators::OP_LdStrictFrameDisplayNoParent(void *argHead, ScriptContext* scriptContext)
    {
        return OP_LdStrictFrameDisplay(argHead, (void*)&StrictNullFrameDisplay, scriptContext);
    }

    void JavascriptOperators::OP_InitSetter(Var object, PropertyId propertyId, Var setter)
    {
        AssertMsg(!TaggedNumber::Is(object), "SetMember on a non-object?");
        RecyclableObject::FromVar(object)->SetAccessors(propertyId, nullptr, setter);
    }

    void JavascriptOperators::OP_InitElemSetter(Var object, Var elementName, Var setter, ScriptContext* scriptContext, PropertyOperationFlags flags)
    {
        AssertMsg(!TaggedNumber::Is(object), "SetMember on a non-object?");

        JavascriptString * indexStr = JavascriptConversion::ToString(elementName, scriptContext);
        PropertyRecord const * propertyRecord = nullptr;
        scriptContext->GetOrAddPropertyRecord(indexStr->GetString(), indexStr->GetLength(), &propertyRecord);

        RecyclableObject::FromVar(object)->SetAccessors(propertyRecord->GetPropertyId(), nullptr, setter);
    }

    void JavascriptOperators::OP_InitGetter(Var object, PropertyId propertyId, Var getter)
    {
        AssertMsg(!TaggedNumber::Is(object), "GetMember on a non-object?");
        RecyclableObject::FromVar(object)->SetAccessors(propertyId, getter, nullptr);
    }

    void JavascriptOperators::OP_InitElemGetter(Var object, Var elementName, Var getter, ScriptContext* scriptContext, PropertyOperationFlags flags)
    {
        AssertMsg(!TaggedNumber::Is(object), "GetMember on a non-object?");

        JavascriptString * indexStr = JavascriptConversion::ToString(elementName, scriptContext);
        PropertyRecord const * propertyRecord = nullptr;
        scriptContext->GetOrAddPropertyRecord(indexStr->GetString(), indexStr->GetLength(), &propertyRecord);

        RecyclableObject::FromVar(object)->SetAccessors(propertyRecord->GetPropertyId(), getter, nullptr);
    }

    void JavascriptOperators::OP_InitComputedProperty(Var object, Var elementName, Var value, ScriptContext* scriptContext, PropertyOperationFlags flags)
    {
        PropertyRecord const * propertyRecord = nullptr;
        if (JavascriptSymbol::Is(elementName))
        {
            propertyRecord = JavascriptSymbol::FromVar(elementName)->GetValue();
        }
        else if (JavascriptSymbolObject::Is(elementName))
        {
            propertyRecord = JavascriptSymbolObject::FromVar(elementName)->GetValue();
        }
        else
        {
            JavascriptString * indexStr = JavascriptConversion::ToString(elementName, scriptContext);
            scriptContext->GetOrAddPropertyRecord(indexStr->GetString(), indexStr->GetLength(), &propertyRecord);
        }

        RecyclableObject::FromVar(object)->InitProperty(propertyRecord->GetPropertyId(), value, flags);
    }

    //
    // Used by object literal {..., __proto__: ..., }.
    // When __proto__ is enabled, it is effectively same as StFld. However when __proto__ is disabled, it functions same as InitFld.
    //
    void JavascriptOperators::OP_InitProto(Var instance, PropertyId propertyId, Var value)
    {
        AssertMsg(RecyclableObject::Is(instance), "__proto__ member on a non-object?");
        Assert(propertyId == PropertyIds::__proto__);

        RecyclableObject* object = RecyclableObject::FromVar(instance);
        ScriptContext* scriptContext = object->GetScriptContext();

        if (scriptContext->GetConfig()->Is__proto__Enabled())
        {
            // B.3.1    __proto___ Property Names in Object Initialisers
            //6.If propKey is the string value "__proto__" and if isComputedPropertyName(propKey) is false, then
            //    a.If Type(v) is either Object or Null, then
            //        i.Return the result of calling the [[SetInheritance]] internal method of object with argument propValue.
            //    b.Return NormalCompletion(empty).
            if (JavascriptOperators::IsObjectOrNull(value))
            {
                JavascriptObject::ChangePrototype(object, RecyclableObject::FromVar(value), /*validate*/false, scriptContext);
            }
        }
        else
        {
            object->InitProperty(propertyId, value);
        }
    }

    void JavascriptOperators::OP_BindEvent(Var object, PropertyId propertyId, Var eventHandler)
    {
        if (TaggedNumber::Is(object))
        {
            JavascriptError::ThrowTypeError(RecyclableObject::FromVar(eventHandler)->GetScriptContext(), JSERR_NeedObject);
        }
        RecyclableObject::FromVar(object)->BindEvent(eventHandler, propertyId);
    }

    Var JavascriptOperators::LoadHeapArguments(JavascriptFunction *funcCallee, uint32 paramCount, Var *paramAddr, Var frameObj, Var vArray, ScriptContext* scriptContext, bool formalsAreLetDecls)
    {
        AssertMsg(paramCount != (unsigned int)-1, "Loading the arguments object in the global function?");

        // Create and initialize the Arguments object.

        uint32 formalsCount = 0;
        Js::PropertyIdArray *propIds = null;
        if (vArray != scriptContext->GetLibrary()->GetNull())
        {
            propIds = (Js::PropertyIdArray *)vArray;
            formalsCount = propIds->count;
        }

        HeapArgumentsObject *argsObj = JavascriptOperators::CreateHeapArguments(funcCallee, paramCount, formalsCount, frameObj, scriptContext);

        // Transfer formal arguments (that were actually passed) from their ArgIn slots to the local frame object.
        uint32 i;

        Var *tmpAddr = paramAddr;

        if (propIds != null)
        {
            ActivationObject* frameObject = (ActivationObject*)frameObj;
            // No fixed fields for formal parameters of the arguments object.  Also, mark all fields as inititalized up-front, because
            // we will set them directly using SetSlot below, so the type handler will not have a chance to mark them as intitialized later.
            // TODO (jedmiad): When we delay type sharing until the second instance is created, pass an argument indicating we want the types
            // and handlers created here to be marked as shared up-front. This is to ensure we don't get any fixed fields and that the handler
            // is ready for storing values directly to slots.
            DynamicType* newType = PathTypeHandlerBase::CreateNewScopeObject(scriptContext, frameObject->GetDynamicType(), propIds, formalsAreLetDecls ? PropertyLetDefaults : PropertyNone);

            int oldSlotCapacity = frameObject->GetDynamicType()->GetTypeHandler()->GetSlotCapacity();
            int newSlotCapacity = newType->GetTypeHandler()->GetSlotCapacity();
            __analysis_assume((uint32)newSlotCapacity >= formalsCount);

            frameObject->EnsureSlots(oldSlotCapacity, newSlotCapacity, scriptContext, newType->GetTypeHandler());
            frameObject->ReplaceType(newType);

            for (i = 0; i < formalsCount && i < paramCount; i++, tmpAddr++)
            {
                if (formalsAreLetDecls)
                {
                    // This is a default value. We need to delay the initialization until we have processed the initializer. This enforces TDZ in situations with eval().
                    frameObject->SetSlot(SetSlotArguments(propIds->elements[i], i, scriptContext->GetLibrary()->GetUndeclBlockVar()));
                }
                else
                {
                    frameObject->SetSlot(SetSlotArguments(propIds->elements[i], i, *tmpAddr));
                }
            }

            if (i < formalsCount)
            {
                // The formals that weren't passed still need to be put in the frame object so that
                // their names will be found. Initialize them to "undefined".
                Var undef = scriptContext->GetLibrary()->GetUndefined();
                for (; i < formalsCount; i++)
                {
                    if (formalsAreLetDecls)
                    {
                        // This is a default value. We need to delay the initialization until we hav processed the initializer. This enforces TDZ in situations with eval().
                        frameObject->SetSlot(SetSlotArguments(propIds->elements[i], i, scriptContext->GetLibrary()->GetUndeclBlockVar()));
                    }
                    else if (BinaryFeatureControl::LanguageService())
                    {
                        // In the language service, determine if there is a tracking undefined value that can be used instead
                        // of the library undefined.
                        frameObject->SetSlot(SetSlotArguments(propIds->elements[i], i, scriptContext->GetMissingParameterValue(funcCallee, i)));
                    }
                    else
                    {
                        frameObject->SetSlot(SetSlotArguments(propIds->elements[i], i, undef));
                    }
                }
            }
        }

        // Transfer the unnamed actual arguments, if any, to the Arguments object itself.
        for (i = formalsCount, tmpAddr = paramAddr + i; i < paramCount; i++, tmpAddr++)
        {
            // ES5 10.6.11: use [[DefineOwnProperty]] semantics (instead of [[Put]]):
            // do not check whether property is non-writable/etc in the prototype.
            // ES3 semantics is same.
            JavascriptOperators::SetItem(argsObj, argsObj, i, *tmpAddr, scriptContext, PropertyOperation_None, /* skipPrototypeCheck = */ TRUE);
        }

        if (funcCallee->IsStrictMode())
        {
            // If the formals are let decls, then we just overwrote the frame object slots with
            // Undecl sentinels, and we can use the original arguments that were passed to the HeapArgumentsObject.
            return argsObj->ConvertToStrictModeArgumentsObject(!formalsAreLetDecls);
        }

        return argsObj;
    }

    Var JavascriptOperators::LoadHeapArgsCached(JavascriptFunction *funcCallee, uint32 actualsCount, uint32 formalsCount, Var *paramAddr, Var frameObj, ScriptContext* scriptContext, bool formalsAreLetDecls)
    {
        // Disregard the "this" param.
        AssertMsg(actualsCount != (uint32)-1 && formalsCount != (uint32)-1,
                  "Loading the arguments object in the global function?");

        // Create and initialize the Arguments object.
        HeapArgumentsObject *argsObj = JavascriptOperators::CreateHeapArguments(funcCallee, actualsCount, formalsCount, frameObj, scriptContext);

        // Transfer formal arguments (that were actually passed) from their ArgIn slots to the local frame object.
        uint32 i;

        Var *tmpAddr = paramAddr;

        if (formalsCount != 0)
        {
            DynamicObject* frameObject = DynamicObject::FromVar(frameObj);
            __analysis_assume((uint32)frameObject->GetDynamicType()->GetTypeHandler()->GetSlotCapacity() >= formalsCount);

            for (i = 0; i < formalsCount && i < actualsCount; i++, tmpAddr++)
            {
                if (formalsAreLetDecls)
                {
                    // This is a default value. We need to delay the initialization until we have processed the initializer. This enforces TDZ in situations with eval().
                    frameObject->SetSlot(SetSlotArguments(Constants::NoProperty, i, scriptContext->GetLibrary()->GetUndeclBlockVar()));
                }
                else
                {
                    // We don't know the propertyId at this point.
                    frameObject->SetSlot(SetSlotArguments(Constants::NoProperty, i, *tmpAddr));
                }
            }

            if (i < formalsCount)
            {
                // The formals that weren't passed still need to be put in the frame object so that
                // their names will be found. Initialize them to "undefined".
                Var undef = scriptContext->GetLibrary()->GetUndefined();
                for (; i < formalsCount; i++)
                {
                    if (formalsAreLetDecls)
                    {
                        // This is a default value. We need to delay the initialization until we hav processed the initializer. This enforces TDZ in situations with eval().
                        frameObject->SetSlot(SetSlotArguments(Constants::NoProperty, i, scriptContext->GetLibrary()->GetUndeclBlockVar()));
                    }
                    else
                    {
                        // We don't know the propertyId at this point.
                        frameObject->SetSlot(SetSlotArguments(Constants::NoProperty, i, undef));
                    }
                }
            }
        }

        // Transfer the unnamed actual arguments, if any, to the Arguments object itself.
        for (i = formalsCount, tmpAddr = paramAddr + i; i < actualsCount; i++, tmpAddr++)
        {
            // ES5 10.6.11: use [[DefineOwnProperty]] semantics (instead of [[Put]]):
            // do not check whether property is non-writable/etc in the prototype.
            // ES3 semantics is same.
            JavascriptOperators::SetItem(argsObj, argsObj, i, *tmpAddr, scriptContext, PropertyOperation_None, /* skipPrototypeCheck = */ TRUE);
        }

        if (funcCallee->IsStrictMode())
        {
            // If the formals are let decls, then we just overwrote the frame object slots with
            // Undecl sentinels, and we can use the original arguments that were passed to the HeapArgumentsObject.
            return argsObj->ConvertToStrictModeArgumentsObject(!formalsAreLetDecls);
        }

        return argsObj;
    }

    HeapArgumentsObject *JavascriptOperators::CreateHeapArguments(JavascriptFunction *funcCallee, uint32 actualsCount, uint32 formalsCount, Var frameObj, ScriptContext* scriptContext)
    {
        JavascriptLibrary *library = scriptContext->GetLibrary();
        HeapArgumentsObject *argsObj = library->CreateHeapArguments(frameObj, formalsCount);

        //
        // Set the number of arguments of Arguments Object
        //
        argsObj->SetNumberOfArguments(actualsCount);

        JavascriptOperators::SetProperty(argsObj, argsObj, PropertyIds::length, JavascriptNumber::ToVar(actualsCount, scriptContext), scriptContext);
        if (scriptContext->GetConfig()->IsES6IteratorsEnabled())
        {
            JavascriptOperators::SetProperty(argsObj, argsObj, PropertyIds::_symbolIterator, library->GetArrayPrototypeValuesFunction(), scriptContext);
        }
        if (funcCallee->IsStrictMode() && !BinaryFeatureControl::LanguageService())
        {
            PropertyDescriptor propertyDescriptorCaller;
            JavascriptFunction* callerAccessor = library->GetThrowTypeErrorCallerAccessorFunction();

            propertyDescriptorCaller.SetGetter(callerAccessor);
            propertyDescriptorCaller.SetSetter(callerAccessor);
            propertyDescriptorCaller.SetEnumerable(false);
            propertyDescriptorCaller.SetConfigurable(false);

            argsObj->SetAccessors(PropertyIds::caller, callerAccessor, callerAccessor, PropertyOperation_NonFixedValue);
            JavascriptOperators::SetAttributes(argsObj, PropertyIds::caller, propertyDescriptorCaller, false);

            PropertyDescriptor propertyDescriptorCallee;
            JavascriptFunction* calleeAccessor = library->GetThrowTypeErrorCalleeAccessorFunction();

            propertyDescriptorCallee.SetGetter(calleeAccessor);
            propertyDescriptorCallee.SetSetter(calleeAccessor);
            propertyDescriptorCallee.SetEnumerable(false);
            propertyDescriptorCallee.SetConfigurable(false);

            argsObj->SetAccessors(PropertyIds::callee, calleeAccessor, calleeAccessor, PropertyOperation_NonFixedValue);
            JavascriptOperators::SetAttributes(argsObj, PropertyIds::callee, propertyDescriptorCallee, false);
        }
        else
        {
            JavascriptOperators::SetProperty(argsObj, argsObj, PropertyIds::callee,
                StackScriptFunction::EnsureBoxed(BOX_PARAM(funcCallee, null, L"callee")), scriptContext);
        }

        return argsObj;
    }

    Var JavascriptOperators::OP_NewScopeObject(ScriptContext*scriptContext)
    {
        return scriptContext->GetLibrary()->CreateActivationObject();
    }

    Var* JavascriptOperators::OP_NewScopeSlots(unsigned int size, ScriptContext *scriptContext, Var scope)
    {
        Assert(size > ScopeSlots::FirstSlotIndex); // Should never see empty slot array
        Var* slotArray = RecyclerNewArray(scriptContext->GetRecycler(), Var, size); // last initialized slot contains reference to array of propertyIds, correspondent to objects in previous slots
        uint count = size - ScopeSlots::FirstSlotIndex;
        ScopeSlots slots(slotArray);
        slots.SetCount(count);
        slots.SetScope(scope);
        Var undef = scriptContext->GetLibrary()->GetUndefined();
        for (unsigned int i = 0; i < count; i++)
        {
            slots.Set(i, undef);
        }

        return slotArray;
    }

    Var* JavascriptOperators::OP_NewScopeSlotsWithoutPropIds(unsigned int count, int scopeIndex, ScriptContext *scriptContext, FunctionBody *functionBody)
    {
        DebuggerScope* scope = Constants::FunctionBodyUnavailable;
        if(scopeIndex != DebuggerScope::InvalidScopeIndex)
        {
            AssertMsg(functionBody->GetScopeObjectChain(), "A scope chain should always be created when there are new scope slots for blocks.");
            scope = functionBody->GetScopeObjectChain()->pScopeChain->Item(scopeIndex);
        }
        return OP_NewScopeSlots(count, scriptContext, scope);
    }

    Var JavascriptOperators::OP_NewPseudoScope(ScriptContext *scriptContext)
    {
        return scriptContext->GetLibrary()->CreatePseudoActivationObject();
    }

    Var JavascriptOperators::OP_NewBlockScope(ScriptContext *scriptContext)
    {
        return scriptContext->GetLibrary()->CreateBlockActivationObject();
    }

    Var JavascriptOperators::OP_IsInst(Var instance, Var aClass, ScriptContext* scriptContext, IsInstInlineCache* inlineCache)
    {
        if (!RecyclableObject::Is(aClass))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_Operand_Invalid_NeedFunction, L"instanceof");
        }

        RecyclableObject* constructor = RecyclableObject::FromVar(aClass);
        return JavascriptBoolean::ToVar(constructor->HasInstance(instance, scriptContext, inlineCache), scriptContext);
    }

    void JavascriptOperators::OP_InitClass(Var constructor, Var extends, ScriptContext * scriptContext)
    {
        if (JavascriptOperators::GetTypeId(constructor) != Js::TypeId::TypeIds_Function)
        {
             JavascriptError::ThrowTypeError(scriptContext, JSERR_Operand_Invalid_NeedFunction, L"class");
        }

        RecyclableObject * ctor = RecyclableObject::FromVar(constructor);

        // This is a circular reference to the constructor, it associate the constructor with the class and also allows us to check if a
        // function is a construcor by comparing the homeObj to the this pointer. see ScriptFunction::IsClassConstructor() for implementation
        JavascriptOperators::OP_SetHomeObj(constructor, constructor);

        if (extends)
        {
            switch (JavascriptOperators::GetTypeId(extends))
            {
                case Js::TypeId::TypeIds_Null:
                {
                    Var ctorProto = JavascriptOperators::GetProperty(constructor, ctor, Js::PropertyIds::prototype, scriptContext);
                    RecyclableObject * ctorProtoObj = RecyclableObject::FromVar(ctorProto);

                    ctorProtoObj->SetPrototype(RecyclableObject::FromVar(extends));

                    ctorProtoObj->EnsureProperty(Js::PropertyIds::constructor);
                    ctorProtoObj->SetEnumerable(Js::PropertyIds::constructor, FALSE);
                    break;
                }

                default:
                {
                    if (!RecyclableObject::Is(extends))
                    {
                        JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidPrototype, L"extends");
                    }
                    RecyclableObject * extendsObj = RecyclableObject::FromVar(extends);
                    if (!extendsObj->HasProperty(Js::PropertyIds::prototype))
                    {
                        JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidPrototype);
                    }

                    Var extendsProto = JavascriptOperators::GetProperty(extends, extendsObj, Js::PropertyIds::prototype, scriptContext);
                    uint extendsProtoTypeId = JavascriptOperators::GetTypeId(extendsProto);
                    if (extendsProtoTypeId <= Js::TypeId::TypeIds_LastJavascriptPrimitiveType && extendsProtoTypeId != Js::TypeId::TypeIds_Null)
                    {
                        JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidPrototype);
                    }

                    Var ctorProto = JavascriptOperators::GetProperty(constructor, ctor, Js::PropertyIds::prototype, scriptContext);
                    RecyclableObject * ctorProtoObj = RecyclableObject::FromVar(ctorProto);

                    ctorProtoObj->SetPrototype(RecyclableObject::FromVar(extendsProto));

                    ctorProtoObj->EnsureProperty(Js::PropertyIds::constructor);
                    ctorProtoObj->SetEnumerable(Js::PropertyIds::constructor, FALSE);

                    Var protoCtor = JavascriptOperators::GetProperty(ctorProto, ctorProtoObj, Js::PropertyIds::constructor, scriptContext);
                    RecyclableObject * protoCtorObj = RecyclableObject::FromVar(protoCtor);
                    protoCtorObj->SetPrototype(extendsObj);

                    break;
                }
            }
        }

        ctor->SetWritable(Js::PropertyIds::prototype, FALSE); //TODO SetWritable undefers the function, do we really want to do it this way?
    }

    void JavascriptOperators::OP_LoadUndefinedToElement(Var instance, PropertyId propertyId)
    {
        AssertMsg(!TaggedNumber::Is(instance), "Invalid scope/root object");
        JavascriptOperators::EnsureProperty(instance, propertyId);
    }

    void JavascriptOperators::OP_LoadUndefinedToElementScoped(FrameDisplay *pScope, PropertyId propertyId, Var defaultInstance, ScriptContext* scriptContext)
    {
        int i;
        int length = pScope->GetLength();
        Var argInstance;
        for (i = 0; i < length; i++)
        {
            argInstance = pScope->GetItem(i);
            if (JavascriptOperators::EnsureProperty(argInstance, propertyId))
            {
                return;
            }
        }

        if (!JavascriptOperators::HasOwnPropertyNoHostObject(defaultInstance, propertyId))
        {
            // TODO (jedmiad): Consider adding pre-initialization support to activation objects.
            JavascriptOperators::OP_InitPropertyScoped(pScope, propertyId, scriptContext->GetLibrary()->GetUndefined(), defaultInstance, scriptContext);
        }
    }

    void JavascriptOperators::OP_LoadUndefinedToElementDynamic(Var instance, PropertyId propertyId, ScriptContext *scriptContext)
    {
        if (!JavascriptOperators::HasOwnPropertyNoHostObject(instance, propertyId))
        {
            RecyclableObject::FromVar(instance)->InitPropertyScoped(propertyId, scriptContext->GetLibrary()->GetUndefined());
        }
    }

    BOOL JavascriptOperators::EnsureProperty(Var instance, PropertyId propertyId)
    {
        RecyclableObject *obj = RecyclableObject::FromVar(instance);
        return (obj && obj->EnsureProperty(propertyId));
    }

    void JavascriptOperators::OP_EnsureNoRootProperty(Var instance, PropertyId propertyId)
    {
        Assert(RootObjectBase::Is(instance));
        RootObjectBase *obj = RootObjectBase::FromVar(instance);
        obj->EnsureNoProperty(propertyId);
    }

    void JavascriptOperators::OP_EnsureNoRootRedeclProperty(Var instance, PropertyId propertyId)
    {
        Assert(RootObjectBase::Is(instance));
        RecyclableObject *obj = RecyclableObject::FromVar(instance);
        obj->EnsureNoRedeclProperty(propertyId);
    }

    void JavascriptOperators::OP_ScopedEnsureNoRedeclProperty(FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance)
    {
        int i;
        int length = pDisplay->GetLength();
        RecyclableObject *object;

        for (i = 0; i < length; i++)
        {
            object = RecyclableObject::FromVar(pDisplay->GetItem(i));
            if (object->EnsureNoRedeclProperty(propertyId))
            {
                return;
            }
        }

        object = RecyclableObject::FromVar(defaultInstance);
        object->EnsureNoRedeclProperty(propertyId);
    }

    Var JavascriptOperators::IsIn(Var argProperty, Var instance, ScriptContext* scriptContext)
    {
        // Note that the fact that we haven't seen a given name before doesn't mean that the instance doesn't

        if (!IsObject(instance))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_Operand_Invalid_NeedObject, L"in");
        }

        PropertyRecord const * propertyRecord;
        uint32 index;
        IndexType indexType = GetIndexType(argProperty, scriptContext, &index, &propertyRecord, true);

        RecyclableObject* object = RecyclableObject::FromVar(instance);

        BOOL result;
        if( indexType == Js::IndexType_Number )
        {
            result = JavascriptOperators::HasItem( object, index );
        }
        else
        {
            PropertyId propertyId = propertyRecord->GetPropertyId();
            result = JavascriptOperators::HasProperty( object, propertyId );

#ifdef TELEMETRY
            {
                Assert(indexType != Js::IndexType_JavascriptString);
                if( indexType == Js::IndexType_PropertyId )
                {
                    scriptContext->GetTelemetry().GetOpcodeTelemetry().IsIn( instance, propertyId, result != 0 );
                }
            }
#endif
        }
        return JavascriptBoolean::ToVar(result, scriptContext);
    }

    template <bool IsFromFullJit, class TInlineCache>
    __inline Var JavascriptOperators::PatchGetValue(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId)
    {
        return PatchGetValueWithThisPtr<IsFromFullJit, TInlineCache>(functionBody, inlineCache, inlineCacheIndex, instance, propertyId, instance);
    }


    template <bool IsFromFullJit, class TInlineCache>
    __forceinline Var JavascriptOperators::PatchGetValueWithThisPtr(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var thisInstance)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        Assert(Js::JavascriptStackWalker::ValidateTopJitFrame(scriptContext));

        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined,
                    scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        Var value;
        if (CacheOperators::TryGetProperty<true, true, true, true, true, true, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                thisInstance, false, object, propertyId, &value, scriptContext, nullptr, &info))
        {
            return value;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchGetValue", propertyId, scriptContext, object);
        }
#endif

        return JavascriptOperators::GetProperty(thisInstance, object, propertyId, scriptContext, &info);
    }


    template Var JavascriptOperators::PatchGetValue<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetValue<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetValue<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetValue<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);

    template Var JavascriptOperators::PatchGetValueWithThisPtr<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var thisInstance);
    template Var JavascriptOperators::PatchGetValueWithThisPtr<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var thisInstance);
    template Var JavascriptOperators::PatchGetValueWithThisPtr<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var thisInstance);
    template Var JavascriptOperators::PatchGetValueWithThisPtr<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var thisInstance);

    template <bool IsFromFullJit, class TInlineCache>
    Var JavascriptOperators::PatchGetValueForTypeOf(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        Assert(Js::JavascriptStackWalker::ValidateTopJitFrame(scriptContext));

        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined,
                    scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        Var value;
        if (CacheOperators::TryGetProperty<true, true, true, true, true, true, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
            instance, false, object, propertyId, &value, scriptContext, nullptr, &info))
        {
            return value;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchGetValueForTypeOf", propertyId, scriptContext, object);
        }
#endif
        Var prop = nullptr;

        BEGIN_TYPEOF_ERROR_HANDLER(scriptContext);
        prop = JavascriptOperators::GetProperty(instance, object, propertyId, scriptContext, &info);
        END_TYPEOF_ERROR_HANDLER(scriptContext, prop);

        return prop;
    }
    template Var JavascriptOperators::PatchGetValueForTypeOf<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetValueForTypeOf<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetValueForTypeOf<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetValueForTypeOf<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);


    Var JavascriptOperators::PatchGetValueUsingSpecifiedInlineCache(InlineCache * inlineCache, Var instance, RecyclableObject * object, PropertyId propertyId, ScriptContext* scriptContext)
    {
        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, inlineCache);
        Var value;
        if (CacheOperators::TryGetProperty<true, true, true, true, false, true, !InlineCache::IsPolymorphic, InlineCache::IsPolymorphic, false>(
                instance, false, object, propertyId, &value, scriptContext, nullptr, &info))
        {
            return value;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchGetValue", propertyId, scriptContext, object);
        }
#endif

        return JavascriptOperators::GetProperty(instance, object, propertyId, scriptContext, &info);
    }

    Var JavascriptOperators::PatchGetValueNoFastPath(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId)
    {
        return PatchGetValueWithThisPtrNoFastPath(functionBody, inlineCache, inlineCacheIndex, instance, propertyId, instance);
    }

    Var JavascriptOperators::PatchGetValueWithThisPtrNoFastPath(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var thisInstance)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined,
                    scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, true);
        return JavascriptOperators::GetProperty(thisInstance, object, propertyId, scriptContext, &info);
    }

    template <bool IsFromFullJit, class TInlineCache>
    __inline Var JavascriptOperators::PatchGetRootValue(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId)
    {
        AssertMsg(RootObjectBase::Is(object), "Root must be a global object!");

        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        Var value;
        if (CacheOperators::TryGetProperty<true, true, true, false, true, false, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                object, true, object, propertyId, &value, scriptContext, nullptr, &info))
        {
            return value;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchGetRootValue", propertyId, scriptContext, object);
        }
#endif

        return JavascriptOperators::OP_GetRootProperty(object, propertyId, &info, scriptContext);
    }
    template Var JavascriptOperators::PatchGetRootValue<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootValue<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootValue<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootValue<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId);

    template <bool IsFromFullJit, class TInlineCache>
    Var JavascriptOperators::PatchGetRootValueForTypeOf(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId)
    {
        AssertMsg(RootObjectBase::Is(object), "Root must be a global object!");

        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        Var value = nullptr;
        if (CacheOperators::TryGetProperty<true, true, true, false, true, false, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
            object, true, object, propertyId, &value, scriptContext, nullptr, &info))
        {
            return value;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchGetRootValueForTypeOf", propertyId, scriptContext, object);
        }
#endif
        value = nullptr;
        BEGIN_TYPEOF_ERROR_HANDLER(scriptContext);
        if (JavascriptOperators::GetRootProperty(RecyclableObject::FromVar(object), propertyId, &value, scriptContext, &info))
        {
            if (scriptContext->IsUndeclBlockVar(value))
            {
                JavascriptError::ThrowReferenceError(scriptContext, JSERR_UseBeforeDeclaration);
            }
            return value;
        }
        END_TYPEOF_ERROR_HANDLER(scriptContext, value);

        value = scriptContext->GetLibrary()->GetUndefined();
        return value;
    }
    template Var JavascriptOperators::PatchGetRootValueForTypeOf<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootValueForTypeOf<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootValueForTypeOf<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootValueForTypeOf<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject * object, PropertyId propertyId);


    Var JavascriptOperators::PatchGetRootValueNoFastPath_Var(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId)
    {
        return
            PatchGetRootValueNoFastPath(
                functionBody,
                inlineCache,
                inlineCacheIndex,
                DynamicObject::FromVar(instance),
                propertyId);
    }

    Var JavascriptOperators::PatchGetRootValueNoFastPath(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject* object, PropertyId propertyId)
    {
        AssertMsg(RootObjectBase::Is(object), "Root must be a global object!");

        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, true);
        return JavascriptOperators::OP_GetRootProperty(object, propertyId, &info, scriptContext);
    }

    template <bool IsFromFullJit, class TInlineCache>
    __inline Var JavascriptOperators::PatchGetPropertyScoped(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance)
    {
        // Get the property, using a scope stack rather than an individual instance.
        // Walk the stack until we find an instance that has the property.

        ScriptContext *const scriptContext = functionBody->GetScriptContext();
        uint16 length = pDisplay->GetLength();

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        for (uint16 i = 0; i < length; i++)
        {
            DynamicObject* object = (DynamicObject*)pDisplay->GetItem(i);
            Var value;
            if (CacheOperators::TryGetProperty<true, true, true, false, true, true, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                    object, false, object, propertyId, &value, scriptContext, nullptr, &info))
            {
                return value;
            }

#if DBG_DUMP
            if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
            {
                CacheOperators::TraceCache(inlineCache, L"PatchGetPropertyScoped", propertyId, scriptContext, object);
            }
#endif
            if (JavascriptOperators::GetProperty(object, propertyId, &value, scriptContext, &info))
            {
                if (scriptContext->IsUndeclBlockVar(value))
                {
                    JavascriptError::ThrowReferenceError(scriptContext, JSERR_UseBeforeDeclaration);
                }
                return value;
            }
        }

        // No one in the scope stack has the property, so get it from the default instance provided by the caller.
        Var value = JavascriptOperators::PatchGetRootValue<IsFromFullJit>(functionBody, inlineCache, inlineCacheIndex, DynamicObject::FromVar(defaultInstance), propertyId);
        if (scriptContext->IsUndeclBlockVar(value))
        {
            JavascriptError::ThrowReferenceError(scriptContext, JSERR_UseBeforeDeclaration);
        }
        return value;
    }
    template Var JavascriptOperators::PatchGetPropertyScoped<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance);
    template Var JavascriptOperators::PatchGetPropertyScoped<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance);
    template Var JavascriptOperators::PatchGetPropertyScoped<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance);
    template Var JavascriptOperators::PatchGetPropertyScoped<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance);

    template <bool IsFromFullJit, class TInlineCache>
    Var JavascriptOperators::PatchGetPropertyForTypeOfScoped(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance)
    {
        Var value = nullptr;
        ScriptContext *scriptContext = functionBody->GetScriptContext();

        BEGIN_TYPEOF_ERROR_HANDLER(scriptContext);
        value = JavascriptOperators::PatchGetPropertyScoped<IsFromFullJit, TInlineCache>(functionBody, inlineCache, inlineCacheIndex, pDisplay, propertyId, defaultInstance);
        END_TYPEOF_ERROR_HANDLER(scriptContext, value)

        return value;
    }
    template Var JavascriptOperators::PatchGetPropertyForTypeOfScoped<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance);
    template Var JavascriptOperators::PatchGetPropertyForTypeOfScoped<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance);
    template Var JavascriptOperators::PatchGetPropertyForTypeOfScoped<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance);
    template Var JavascriptOperators::PatchGetPropertyForTypeOfScoped<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, FrameDisplay *pDisplay, PropertyId propertyId, Var defaultInstance);


    template <bool IsFromFullJit, class TInlineCache>
    __inline Var JavascriptOperators::PatchGetMethod(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId)
    {
        Assert(inlineCache != null);

        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        RecyclableObject* object = null;
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(instance);
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            // Don't error if we disabled implicit calls
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined,
                    scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        Var value;
        if (CacheOperators::TryGetProperty<true, true, true, false, true, true, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                instance, false, object, propertyId, &value, scriptContext, nullptr, &info))
        {
            return value;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchGetMethod", propertyId, scriptContext, object);
        }
#endif

        return Js::JavascriptOperators::PatchGetMethodFromObject(instance, object, propertyId, &info, scriptContext, false);
    }
    template Var JavascriptOperators::PatchGetMethod<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetMethod<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetMethod<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetMethod<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);

    template <bool IsFromFullJit, class TInlineCache>
    __inline Var JavascriptOperators::PatchGetRootMethod(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject* object, PropertyId propertyId)
    {
        Assert(inlineCache != null);

        AssertMsg(RootObjectBase::Is(object), "Root must be a global object!");

        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        Var value;
        if (CacheOperators::TryGetProperty<true, true, true, false, true, false, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                object, true, object, propertyId, &value, scriptContext, nullptr, &info))
        {
            return value;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchGetRootMethod", propertyId, scriptContext, object);
        }
#endif

        return Js::JavascriptOperators::PatchGetMethodFromObject(object, object, propertyId, &info, scriptContext, true);
    }
    template Var JavascriptOperators::PatchGetRootMethod<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject* object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootMethod<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject* object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootMethod<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject* object, PropertyId propertyId);
    template Var JavascriptOperators::PatchGetRootMethod<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject* object, PropertyId propertyId);

    template <bool IsFromFullJit, class TInlineCache>
    __inline Var JavascriptOperators::PatchScopedGetMethod(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId)
    {
        Assert(inlineCache != null);

        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            // Don't error if we disabled implicit calls
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined,
                    scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        const bool isRoot = RootObjectBase::Is(object);
        Var value;
        if (CacheOperators::TryGetProperty<true, true, true, false, true, false, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                instance, isRoot, object, propertyId, &value, scriptContext, nullptr, &info))
        {
            return value;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchGetMethod", propertyId, scriptContext, object);
        }
#endif

        return Js::JavascriptOperators::PatchGetMethodFromObject(instance, object, propertyId, &info, scriptContext, isRoot);
    }
    template Var JavascriptOperators::PatchScopedGetMethod<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchScopedGetMethod<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchScopedGetMethod<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);
    template Var JavascriptOperators::PatchScopedGetMethod<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId);

    Var JavascriptOperators::PatchGetMethodNoFastPath(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        RecyclableObject* object = null;
        if (FALSE == JavascriptOperators::GetPropertyObject(instance, scriptContext, &object))
        {
            // Don't error if we disabled implicit calls
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotGet_NullOrUndefined,
                    scriptContext->GetPropertyName(propertyId)->GetBuffer());
            }
            else
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, true);
        return Js::JavascriptOperators::PatchGetMethodFromObject(instance, object, propertyId, &info, scriptContext, false);
    }

    Var JavascriptOperators::PatchGetRootMethodNoFastPath_Var(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId)
    {
        return
            PatchGetRootMethodNoFastPath(
                functionBody,
                inlineCache,
                inlineCacheIndex,
                DynamicObject::FromVar(instance),
                propertyId);
    }

    Var JavascriptOperators::PatchGetRootMethodNoFastPath(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, DynamicObject* object, PropertyId propertyId)
    {
        AssertMsg(RootObjectBase::Is(object), "Root must be a global object!");

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, true);
        return Js::JavascriptOperators::PatchGetMethodFromObject(object, object, propertyId, &info, functionBody->GetScriptContext(), true);
    }

    Var JavascriptOperators::PatchGetMethodFromObject(Var instance, RecyclableObject* propertyObject, PropertyId propertyId, PropertyValueInfo * info, ScriptContext* scriptContext, bool isRootLd)
    {
        Assert(IsPropertyObject(propertyObject));

        Var value = null;
        BOOL foundValue = FALSE;

        if (isRootLd)
        {
            RootObjectBase* rootObject = RootObjectBase::FromVar(instance);
            foundValue = JavascriptOperators::GetRootPropertyReference(rootObject, propertyId, &value, scriptContext, info);
        }
        else
        {
            foundValue = JavascriptOperators::GetPropertyReference(instance, propertyObject, propertyId, &value, scriptContext, info);
        }

        if (!foundValue)
        {
            // Don't error if we disabled implicit calls
            if (scriptContext->GetThreadContext()->RecordImplicitException())
            {
                const wchar_t* propertyName = scriptContext->GetPropertyName(propertyId)->GetBuffer();

                value = scriptContext->GetLibrary()->GetUndefined();
                JavascriptFunction * caller = NULL;
                if (JavascriptStackWalker::GetCaller(&caller, scriptContext))
                {
                    FunctionBody * callerBody = caller->GetFunctionBody();
                    if (callerBody && callerBody->GetUtf8SourceInfo()->GetIsXDomain())
                    {
                        propertyName = NULL;
                    }
                }

                // Prior to version 12 we had mistakenly immediately thrown an error for property reference method calls
                // (i.e. <expr>.foo() form) when the target object is the global object.  The spec says that a GetValue
                // on a reference should throw if the reference is unresolved, of which a property reference can never be,
                // however it can be unresolved in the case of an identifier expression, e.g. foo() with no qualification.
                // Such a case would come down to the global object if foo was undefined, hence the check for root object,
                // except that it should have been a check for isRootLd to be correct.
                //
                //   // (at global scope)
                //   foo(x());
                //
                // should throw an error before evaluating x() if foo is not defined, but
                //
                //   // (at global scope)
                //   this.foo(x());
                //
                // should evaluate x() before throwing an error if foo is not a property on the global object.
                // Maintain old behavior prior to version 12.
                bool isPropertyReference = !isRootLd;
                if (!isPropertyReference)
                {
                    JavascriptError::ThrowReferenceError(scriptContext, JSERR_UndefVariable, propertyName);
                }
                else
                {
                    // ES5 11.2.3 #2: We evaluate the call target but don't throw yet if target member is missing. We need to evaluate argList
                    // first (#3). Postpone throwing error to invoke time.
                    value = ThrowErrorObject::CreateThrowTypeErrorObject(scriptContext, VBSERR_OLENoPropOrMethod, propertyName);
                }
            }
        }
        return value;
    }

    template <bool IsFromFullJit, class TInlineCache>
    __inline void JavascriptOperators::PatchPutValue(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        if (TaggedNumber::Is(instance))
        {
            JavascriptOperators::SetPropertyOnTaggedNumber(instance, nullptr, propertyId, newValue, scriptContext, flags);
            return;
        }

        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(instance);
        RecyclableObject* object = RecyclableObject::FromVar(instance);
        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        if (CacheOperators::TrySetProperty<true, true, true, true, true, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                object, false, propertyId, newValue, scriptContext, flags, nullptr, &info))
        {
            return;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchPutValue", propertyId, scriptContext, object);
        }
#endif

        ImplicitCallFlags prevImplicitCallFlags = ImplicitCall_None;
        ImplicitCallFlags currImplicitCallFlags = ImplicitCall_None;
        bool hasThisOnlyStatements = functionBody->GetHasOnlyThisStmts();
        if (hasThisOnlyStatements)
        {
            prevImplicitCallFlags = CacheAndClearImplicitBit(scriptContext);
        }
        if (!JavascriptOperators::OP_SetProperty(object, propertyId, newValue, scriptContext, &info, flags))
        {
            // Add implicit call flags, to bail out if field copy prop may propagate the wrong value.
            scriptContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
        }
        if (hasThisOnlyStatements)
        {
            currImplicitCallFlags = CheckAndUpdateFunctionBodyWithImplicitFlag(functionBody);
            RestoreImplicitFlag(scriptContext, prevImplicitCallFlags, currImplicitCallFlags);
        }

    }
    template void JavascriptOperators::PatchPutValue<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutValue<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutValue<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutValue<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);

    template <bool IsFromFullJit, class TInlineCache>
    __inline void JavascriptOperators::PatchPutRootValue(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        RecyclableObject* object = RecyclableObject::FromVar(instance);
        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        if (CacheOperators::TrySetProperty<true, true, true, true, false, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                object, true, propertyId, newValue, scriptContext, flags, nullptr, &info))
        {
            return;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchPutRootValue", propertyId, scriptContext, object);
        }
#endif

        ImplicitCallFlags prevImplicitCallFlags = ImplicitCall_None;
        ImplicitCallFlags currImplicitCallFlags = ImplicitCall_None;
        bool hasThisOnlyStatements = functionBody->GetHasOnlyThisStmts();
        if (hasThisOnlyStatements)
        {
            prevImplicitCallFlags = CacheAndClearImplicitBit(scriptContext);
        }
        if (!JavascriptOperators::SetRootProperty(object, propertyId, newValue, &info, scriptContext, flags))
        {
            // Add implicit call flags, to bail out if field copy prop may propagate the wrong value.
            scriptContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
        }
        if (hasThisOnlyStatements)
        {
            currImplicitCallFlags = CheckAndUpdateFunctionBodyWithImplicitFlag(functionBody);
            RestoreImplicitFlag(scriptContext, prevImplicitCallFlags, currImplicitCallFlags);
        }
    }
    template void JavascriptOperators::PatchPutRootValue<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutRootValue<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutRootValue<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutRootValue<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);

    template <bool IsFromFullJit, class TInlineCache>
    __inline void JavascriptOperators::PatchPutValueNoLocalFastPath(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        if (TaggedNumber::Is(instance))
        {
            JavascriptOperators::SetPropertyOnTaggedNumber(instance,
                                        nullptr,
                                        propertyId,
                                        newValue,
                                        scriptContext,
                                        flags);
             return;
        }
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(instance);
        RecyclableObject *object = RecyclableObject::FromVar(instance);

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        if (CacheOperators::TrySetProperty<!TInlineCache::IsPolymorphic, true, true, true, true, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                object, false, propertyId, newValue, scriptContext, flags, nullptr, &info))
        {
            return;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchPutValueNoLocalFastPath", propertyId, scriptContext, object);
        }
#endif

        ImplicitCallFlags prevImplicitCallFlags = ImplicitCall_None;
        ImplicitCallFlags currImplicitCallFlags = ImplicitCall_None;
        bool hasThisOnlyStatements = functionBody->GetHasOnlyThisStmts();
        if (hasThisOnlyStatements)
        {
            prevImplicitCallFlags = CacheAndClearImplicitBit(scriptContext);
        }
        if (!JavascriptOperators::OP_SetProperty(instance, propertyId, newValue, scriptContext, &info, flags))
        {
            // Add implicit call flags, to bail out if field copy prop may propagate the wrong value.
            scriptContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
        }
        if (hasThisOnlyStatements)
        {
            currImplicitCallFlags = CheckAndUpdateFunctionBodyWithImplicitFlag(functionBody);
            RestoreImplicitFlag(scriptContext, prevImplicitCallFlags, currImplicitCallFlags);
        }
    }
    template void JavascriptOperators::PatchPutValueNoLocalFastPath<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutValueNoLocalFastPath<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutValueNoLocalFastPath<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutValueNoLocalFastPath<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);

    template <bool IsFromFullJit, class TInlineCache>
    __inline void JavascriptOperators::PatchPutRootValueNoLocalFastPath(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        RecyclableObject *object = RecyclableObject::FromVar(instance);

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        if (CacheOperators::TrySetProperty<!TInlineCache::IsPolymorphic, true, true, true, false, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                object, true, propertyId, newValue, scriptContext, flags, nullptr, &info))
        {
            return;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchPutRootValueNoLocalFastPath", propertyId, scriptContext, object);
        }
#endif

        ImplicitCallFlags prevImplicitCallFlags = ImplicitCall_None;
        ImplicitCallFlags currImplicitCallFlags = ImplicitCall_None;
        bool hasThisOnlyStatements = functionBody->GetHasOnlyThisStmts();
        if (hasThisOnlyStatements)
        {
            prevImplicitCallFlags = CacheAndClearImplicitBit(scriptContext);
        }
        if (!JavascriptOperators::SetRootProperty(object, propertyId, newValue, &info, scriptContext, flags))
        {
            // Add implicit call flags, to bail out if field copy prop may propagate the wrong value.
            scriptContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
        }
        if (hasThisOnlyStatements)
        {
            currImplicitCallFlags = CheckAndUpdateFunctionBodyWithImplicitFlag(functionBody);
            RestoreImplicitFlag(scriptContext, prevImplicitCallFlags, currImplicitCallFlags);
        }
    }
    template void JavascriptOperators::PatchPutRootValueNoLocalFastPath<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutRootValueNoLocalFastPath<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutRootValueNoLocalFastPath<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);
    template void JavascriptOperators::PatchPutRootValueNoLocalFastPath<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags);

    void JavascriptOperators::PatchPutValueNoFastPath(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        if (TaggedNumber::Is(instance))
        {
            JavascriptOperators::SetPropertyOnTaggedNumber(instance, nullptr, propertyId, newValue, scriptContext, flags);
            return;
        }
        RecyclableObject* object = RecyclableObject::FromVar(instance);

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, true);
        if (!JavascriptOperators::OP_SetProperty(object, propertyId, newValue, scriptContext, &info, flags))
        {
            // Add implicit call flags, to bail out if field copy prop may propagate the wrong value.
            scriptContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
        }
    }

    void JavascriptOperators::PatchPutRootValueNoFastPath(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, Var instance, PropertyId propertyId, Var newValue, PropertyOperationFlags flags)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        RecyclableObject* object = RecyclableObject::FromVar(instance);

        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, true);
        if (!JavascriptOperators::SetRootProperty(object, propertyId, newValue, &info, scriptContext, flags))
        {
            // Add implicit call flags, to bail out if field copy prop may propagate the wrong value.
            scriptContext->GetThreadContext()->AddImplicitCallFlags(ImplicitCall_NoOpSet);
        }
    }

    template <bool IsFromFullJit, class TInlineCache>
    __inline void JavascriptOperators::PatchInitValue(FunctionBody *const functionBody, TInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, RecyclableObject* object, PropertyId propertyId, Var newValue)
    {
        ScriptContext *const scriptContext = functionBody->GetScriptContext();

        const PropertyOperationFlags flags = newValue == NULL ? PropertyOperation_SpecialValue : PropertyOperation_None;
        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, !IsFromFullJit);
        if (CacheOperators::TrySetProperty<true, true, false, true, true, !TInlineCache::IsPolymorphic, TInlineCache::IsPolymorphic, false>(
                object, false, propertyId, newValue, scriptContext, flags, nullptr, &info))
        {
            return;
        }

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            CacheOperators::TraceCache(inlineCache, L"PatchInitValue", propertyId, scriptContext, object);
        }
#endif

        Type *typeWithoutProperty = object->GetType();
        if (BinaryFeatureControl::LanguageService() && scriptContext->GetLibrary()->GetUndeclBlockVar() == newValue)
        {
            newValue = scriptContext->GetLibrary()->GetUndefined();
        }

        // This is a hack.  Ideally the lowerer would emit a call to the right flavor of PatchInitValue, so that we can ensure that we only
        // ever initialize to NULL in the right cases.  But the backend uses the StFld opcode for initialization, and threading the different
        // helper calls all the way down would be a pain.
        if (object->InitProperty(propertyId, newValue, flags, &info))
        {
            CacheOperators::CachePropertyWrite(object, false, typeWithoutProperty, propertyId, &info, scriptContext);
        }
    }
    template void JavascriptOperators::PatchInitValue<false, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, RecyclableObject* object, PropertyId propertyId, Var newValue);
    template void JavascriptOperators::PatchInitValue<true, InlineCache>(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, RecyclableObject* object, PropertyId propertyId, Var newValue);
    template void JavascriptOperators::PatchInitValue<false, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, RecyclableObject* object, PropertyId propertyId, Var newValue);
    template void JavascriptOperators::PatchInitValue<true, PolymorphicInlineCache>(FunctionBody *const functionBody, PolymorphicInlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, RecyclableObject* object, PropertyId propertyId, Var newValue);

    void JavascriptOperators::PatchInitValueNoFastPath(FunctionBody *const functionBody, InlineCache *const inlineCache, const InlineCacheIndex inlineCacheIndex, RecyclableObject* object, PropertyId propertyId, Var newValue)
    {
        PropertyValueInfo info;
        PropertyValueInfo::SetCacheInfo(&info, functionBody, inlineCache, inlineCacheIndex, true);
        Type *typeWithoutProperty = object->GetType();
        if (object->InitProperty(propertyId, newValue, PropertyOperation_None, &info))
        {
            CacheOperators::CachePropertyWrite(object, false, typeWithoutProperty, propertyId, &info, functionBody->GetScriptContext());
        }
    }

#if ENABLE_DEBUG_CONFIG_OPTIONS
    void JavascriptOperators::TracePropertyEquivalenceCheck(const JitEquivalentTypeGuard* guard, const Type* type, const Type* refType, bool isEquivalent, uint failedPropertyIndex)
    {
        if (PHASE_TRACE1(Js::EquivObjTypeSpecPhase))
        {
            uint propertyCount = guard->GetCache()->record.propertyCount;

            Output::Print(L"EquivObjTypeSpec: checking %u properties on operation %u, (type = 0x%p, ref type = 0x%p):\n",
                propertyCount, guard->GetObjTypeSpecFldId(), type, refType);

            const Js::TypeEquivalenceRecord& record = guard->GetCache()->record;
            ScriptContext* scriptContext = type->GetScriptContext();
            if (isEquivalent)
            {
                if (Js::Configuration::Global.flags.Verbose)
                {
                    Output::Print(L"    <start>, ");
                    for (uint pi = 0; pi < propertyCount; pi++)
                    {
                        const EquivalentPropertyEntry* refInfo = &record.properties[pi];
                        const PropertyRecord* propertyRecord = scriptContext->GetPropertyName(refInfo->propertyId);
                        Output::Print(L"%s(#%d)@%ua%dw%d, ", propertyRecord->GetBuffer(), propertyRecord->GetPropertyId(), refInfo->slotIndex, refInfo->isAuxSlot, refInfo->mustBeWritable);
                    }
                    Output::Print(L"<end>\n");
                }
            }
            else
            {
                const EquivalentPropertyEntry* refInfo = &record.properties[failedPropertyIndex];
                Js::PropertyEquivalenceInfo info(Constants::NoSlot, false, false);
                const PropertyRecord* propertyRecord = scriptContext->GetPropertyName(refInfo->propertyId);
                if (DynamicType::Is(type->GetTypeId()))
                {
                    Js::DynamicTypeHandler* typeHandler = (static_cast<const DynamicType*>(type))->GetTypeHandler();
                    typeHandler->GetPropertyEquivalenceInfo(propertyRecord, info);
                }

                Output::Print(L"EquivObjTypeSpec: check failed for %s (#%d) on operation %u:\n",
                    propertyRecord->GetBuffer(), propertyRecord->GetPropertyId(), guard->GetObjTypeSpecFldId());
                Output::Print(L"    type = 0x%p, ref type = 0x%p, slot = 0x%u (%d), ref slot = 0x%u (%d), is writable = %d, required writable = %d\n",
                    type, refType, info.slotIndex, refInfo->slotIndex, info.isAuxSlot, refInfo->isAuxSlot, info.isWritable, refInfo->mustBeWritable);
            }

            Output::Flush();
        }
    }
#endif

    bool JavascriptOperators::IsStaticTypeObjTypeSpecEquivalent(const TypeEquivalenceRecord& equivalenceRecord, uint& failedIndex)
    {
        uint propertyCount = equivalenceRecord.propertyCount;
        Js::EquivalentPropertyEntry* properties = equivalenceRecord.properties;
        for (uint pi = 0; pi < propertyCount; pi++)
        {
            const EquivalentPropertyEntry* refInfo = &properties[pi];
            if (!IsStaticTypeObjTypeSpecEquivalent(refInfo))
            {
                failedIndex = pi;
                return false;
            }
        }
        return true;
    }

    bool JavascriptOperators::IsStaticTypeObjTypeSpecEquivalent(const EquivalentPropertyEntry *entry)
    {
        // Objects of static types have no local properties, but they may load fields from their prototypes.
        return entry->slotIndex == Constants::NoSlot && !entry->mustBeWritable;
    }

    bool JavascriptOperators::CheckIfTypeIsEquivalent(Type* type, JitEquivalentTypeGuard* guard)
    {
        if (guard->GetValue() == 0)
        {
            return false;
        }

        if (guard->GetType()->GetScriptContext() != type->GetScriptContext())
        {
            // Can't cache cross-context objects
            return false;
        }

        // TODO (EquivObjTypeSpec): Consider keeping stats on how often the cache hits, and simply force bailout if
        // the efficacy is too low.

        EquivalentTypeCache* cache = guard->GetCache();

        // TODO (EquivObjTypeSpec): Consider emitting o.type == equivTypes[hash(o.type)] in machine code before calling
        // this helper, particularly if we want to handle polymorphism with frequently changing types.
        Assert(EQUIVALENT_TYPE_CACHE_SIZE == 8);
        Type** equivTypes = cache->types;
        if (type == equivTypes[0] || type == equivTypes[1] || type == equivTypes[2] || type == equivTypes[3] ||
            type == equivTypes[4] || type == equivTypes[5] || type == equivTypes[6] || type == equivTypes[7])
        {
            guard->SetType(type);
            return true;
        }

        // If we didn't find the type in the cache, let's check if it's equivalent the slow way, by comparing
        // each of its relevant property slots to its equivalent in one of the cached types.
        // We are making a few assumption that simplify the process:
        // 1. If two types have the same prototype, any properties loaded from a prototype must come from the same slot.
        //    If any of the prototypes in the chain was altered such that this is no longer true, the corresponding
        //    property guard would have been invalidated and we would bail out at the guard check (either on this
        //    type check or downstream, but before the property load is attempted).
        // 2. For polymorphic field loads fixed fields are only supported on prototypes.  Hence, if two types have the
        //    same prototype, any of the equivalent fixed properties will match. If any has been overwritten, the
        //    corresponding guard would have been invalidated and we would bail out (as above).

        Type* refType = equivTypes[0];
        if (refType == nullptr)
        {
            return false;
        }

        if (cache->IsLoadedFromProto() && type->GetPrototype() != refType->GetPrototype())
        {
            if (PHASE_TRACE1(Js::EquivObjTypeSpecPhase))
            {
                Output::Print(L"EquivObjTypeSpec: failed check on operation %u (type = 0x%x, ref type = 0x%x, proto = 0x%x, ref proto = 0x%x) \n",
                    guard->GetObjTypeSpecFldId(), type, refType, type->GetPrototype(), refType->GetPrototype());
                Output::Flush();
            }

            return false;
        }

        if (type->GetTypeId() != refType->GetTypeId())
        {
            if (PHASE_TRACE1(Js::EquivObjTypeSpecPhase))
            {
                Output::Print(L"EquivObjTypeSpec: failed check on operation %u (type = 0x%x, ref type = 0x%x, proto = 0x%x, ref proto = 0x%x) \n",
                    guard->GetObjTypeSpecFldId(), type, refType, type->GetPrototype(), refType->GetPrototype());
                Output::Flush();
            }

            return false;
        }

        // Review (jedmiad): This is quite slow.  We could make it somewhat faster, by keeping slot indexes instead
        // of property IDs, but that would mean we would need to look up property IDs from slot indexes when installing
        // property guards, or maintain a whole separate list of equivalent slot indexes.
        Assert(cache->record.propertyCount > 0);

        // TODO (EquivObjTypeSpec): Consider imposing a limit on the number of properties guarded by an equivalent type check.
        // The trick is where in the glob opt to make the cut off. Perhaps in the forward pass we could track the number of
        // field operations protected by a type check (keep a counter on the type's value info), and if that counter exceeds
        // some threshold, simply stop optimizing any further instructions.

        bool isEquivalent;
        uint failedPropertyIndex;
        if (DynamicType::Is(type->GetTypeId()))
        {
            Js::DynamicTypeHandler* typeHandler = (static_cast<DynamicType*>(type))->GetTypeHandler();
            isEquivalent = typeHandler->IsObjTypeSpecEquivalent(type, cache->record, failedPropertyIndex);
        }
        else
        {
            Assert(StaticType::Is(type->GetTypeId()));
            isEquivalent = IsStaticTypeObjTypeSpecEquivalent(cache->record, failedPropertyIndex);
        }

#if ENABLE_DEBUG_CONFIG_OPTIONS
        TracePropertyEquivalenceCheck(guard, type, refType, isEquivalent, failedPropertyIndex);
#endif

        if (!isEquivalent)
        {
            return false;
        }

        // TODO (EquivObjTypeSpec): Invent some form of least recently used eviction scheme.
        uint index = (reinterpret_cast<uint>(type) >> 4) & (EQUIVALENT_TYPE_CACHE_SIZE - 1);
        if (cache->nextEvictionVictim == EQUIVALENT_TYPE_CACHE_SIZE)
        {
            __analysis_assume(index < EQUIVALENT_TYPE_CACHE_SIZE);
            if (equivTypes[index] != null)
            {
                uint initialIndex = index;
                index = (initialIndex + 1) & (EQUIVALENT_TYPE_CACHE_SIZE - 1);
                for (; index != initialIndex; index = (index + 1) & (EQUIVALENT_TYPE_CACHE_SIZE - 1))
                {
                    if (equivTypes[index] == null) break;
                }
            }
            __analysis_assume(index < EQUIVALENT_TYPE_CACHE_SIZE);
            if (equivTypes[index] != null)
            {
                cache->nextEvictionVictim = 0;
            }
        }
        else
        {
            Assert(cache->nextEvictionVictim < EQUIVALENT_TYPE_CACHE_SIZE);
            __analysis_assume(cache->nextEvictionVictim < EQUIVALENT_TYPE_CACHE_SIZE);
            equivTypes[cache->nextEvictionVictim] = equivTypes[index];
            cache->nextEvictionVictim = (cache->nextEvictionVictim + 1) & (EQUIVALENT_TYPE_CACHE_SIZE - 1);
        }

        Assert(index < EQUIVALENT_TYPE_CACHE_SIZE);
        __analysis_assume(index < EQUIVALENT_TYPE_CACHE_SIZE);
        equivTypes[index] = type;

        if (cache->HasFixedValue())
        {
            // Fixed field checks allow us to assume a specific type ID, but the assumption is only
            // valid if we lock the type. Otherwise, the type ID may change out from under us without
            // evolving the type.
            if (DynamicType::Is(type->GetTypeId()))
            {
                DynamicType *dynamicType = static_cast<DynamicType*>(type);
                if (!dynamicType->GetIsLocked())
                {
                    dynamicType->LockType();
                }
            }
        }

        guard->SetType(type);
        return true;
    }

    void JavascriptOperators::GetPropertyIdForInt(uint64 value, ScriptContext* scriptContext, PropertyRecord const ** propertyRecord)
    {
        wchar_t buffer[20];
        ::_ui64tow_s(value, buffer, sizeof(buffer)/sizeof(wchar_t), 10);
        scriptContext->GetOrAddPropertyRecord(buffer, JavascriptString::GetBufferLength(buffer), propertyRecord);
    }

    void JavascriptOperators::GetPropertyIdForInt(uint32 value, ScriptContext* scriptContext, PropertyRecord const ** propertyRecord)
    {
        GetPropertyIdForInt(static_cast<uint64>(value), scriptContext, propertyRecord);
    }

    Var JavascriptOperators::FromPropertyDescriptor(PropertyDescriptor descriptor, ScriptContext* scriptContext)
    {
        DynamicObject* object = scriptContext->GetLibrary()->CreateObject();

        // ES5 Section 8.10.4 specifies the order for adding these properties.
        if (descriptor.IsDataDescriptor())
        {
            if (descriptor.ValueSpecified())
            {
                JavascriptOperators::InitProperty(object, PropertyIds::value, descriptor.GetValue());
            }
            JavascriptOperators::InitProperty(object, PropertyIds::writable, JavascriptBoolean::ToVar(descriptor.IsWritable(),scriptContext));
        }
        else if (descriptor.IsAccessorDescriptor())
        {
            JavascriptOperators::InitProperty(object, PropertyIds::get, JavascriptOperators::CanonicalizeAccessor(descriptor.GetGetter(), scriptContext));
            JavascriptOperators::InitProperty(object, PropertyIds::set, JavascriptOperators::CanonicalizeAccessor(descriptor.GetSetter(), scriptContext));
        }

        if (descriptor.EnumerableSpecified())
        {
            JavascriptOperators::InitProperty(object, PropertyIds::enumerable, JavascriptBoolean::ToVar(descriptor.IsEnumerable(), scriptContext));
        }
        if (descriptor.ConfigurableSpecified())
        {
            JavascriptOperators::InitProperty(object, PropertyIds::configurable, JavascriptBoolean::ToVar(descriptor.IsConfigurable(), scriptContext));
        }
        return object;
    }

    // ES5 8.12.9 [[DefineOwnProperty]].
    // Return value:
    // - TRUE = success.
    // - FALSE (can throw depending on throwOnError parameter) = unsuccessful.
    BOOL JavascriptOperators::DefineOwnPropertyDescriptor(RecyclableObject* obj, PropertyId propId, const PropertyDescriptor& descriptor, bool throwOnError, ScriptContext* scriptContext)
    {
        Assert(obj);
        Assert(scriptContext);

        if (JavascriptProxy::Is(obj))
        {
            return JavascriptProxy::DefineOwnPropertyDescriptor(obj, propId, descriptor, throwOnError, scriptContext);
        }

        PropertyDescriptor currentDescriptor;
        BOOL isCurrentDescriptorDefined = JavascriptOperators::GetOwnPropertyDescriptor(obj, propId, scriptContext, &currentDescriptor);

        bool isExtensible = !!obj->IsExtensible();
        return ValidateAndApplyPropertyDescriptor<true>(obj, propId, descriptor, isCurrentDescriptorDefined ? &currentDescriptor : nullptr, isExtensible, throwOnError, scriptContext);
    }

    BOOL JavascriptOperators::IsCompatiblePropertyDescriptor(const PropertyDescriptor& descriptor, PropertyDescriptor* currentDescriptor, bool isExtensible, bool throwOnError, ScriptContext* scriptContext)
    {
        return ValidateAndApplyPropertyDescriptor<false>(nullptr, Constants::NoProperty, descriptor, currentDescriptor, isExtensible, throwOnError, scriptContext);
    }

    template<bool needToSetProperty>
    BOOL JavascriptOperators::ValidateAndApplyPropertyDescriptor(RecyclableObject* obj, PropertyId propId, const PropertyDescriptor& descriptor,
        PropertyDescriptor* currentDescriptor, bool isExtensible, bool throwOnError, ScriptContext* scriptContext)
    {
        Var defaultDataValue = scriptContext->GetLibrary()->GetUndefined();
        Var defaultAccessorValue = scriptContext->GetLibrary()->GetDefaultAccessorFunction();

        if (currentDescriptor == nullptr)
        {
            if (!isExtensible) // ES5 8.12.9.3.
            {
                return Reject(throwOnError, scriptContext, JSERR_DefineProperty_NotExtensible, propId);
            }
            else // ES5 8.12.9.4.
            {
                if (needToSetProperty)
                {
                    if (descriptor.IsGenericDescriptor() || descriptor.IsDataDescriptor())
                    {
                        // ES5 8.12.9.4a: Create an own data property named P of object O whose [[Value]], [[Writable]],
                        // [[Enumerable]] and [[Configurable]]  attribute values are described by Desc.
                        // If the value of an attribute field of Desc is absent, the attribute of the newly created property
                        // is set to its default value.
                        PropertyDescriptor filledDescriptor = FillMissingPropertyDescriptorFields<false>(descriptor, scriptContext);

                        BOOL tempResult = obj->SetPropertyWithAttributes(propId, filledDescriptor.GetValue(), filledDescriptor.GetAttributes(), nullptr);
                        Assert(tempResult || obj->IsExternal());
                    }
                    else
                    {
                        // ES5 8.12.9.4b: Create an own accessor property named P of object O whose [[Get]], [[Set]], [[Enumerable]]
                        // and [[Configurable]] attribute values are described by Desc. If the value of an attribute field of Desc is absent,
                        // the attribute of the newly created property is set to its default value.
                        Assert(descriptor.IsAccessorDescriptor());
                        PropertyDescriptor filledDescriptor = FillMissingPropertyDescriptorFields<true>(descriptor, scriptContext);

                        BOOL isSetAccessorsSuccess = obj->SetAccessors(propId, filledDescriptor.GetGetter(), filledDescriptor.GetSetter());

                        // It is valid for some objects to not-support getters and setters, specifically, for projection of an ABI method
                        // (CustomExternalObject => MapWithStringKey) which SetAccessors returns VBSErr_ActionNotSupported.
                        // But for non-extenal objects SetAccessors should succeed.
                        Assert(isSetAccessorsSuccess || obj->CanHaveInterceptors());

                        // If SetAccessors failed, the property wasn't created, so no need to change the attributes.
                        if (isSetAccessorsSuccess)
                        {
                            JavascriptOperators::SetAttributes(obj, propId, filledDescriptor, true);   // use 'force' as default attributes in type system are different from ES5.
                        }
                    }
                }
                return TRUE;
            }
        }

        // ES5 8.12.9.5: Return true, if every field in Desc is absent.
        if (!descriptor.ConfigurableSpecified() && !descriptor.EnumerableSpecified() && !descriptor.WritableSpecified() &&
            !descriptor.ValueSpecified() && !descriptor.GetterSpecified() && !descriptor.SetterSpecified())
        {
            return TRUE;
        }

        // ES5 8.12.9.6: Return true, if every field in Desc also occurs in current and the value of every field in Desc is the same value
        // as the corresponding field in current when compared using the SameValue algorithm (9.12).
        PropertyDescriptor filledDescriptor = descriptor.IsAccessorDescriptor() ? FillMissingPropertyDescriptorFields<true>(descriptor, scriptContext)
            : FillMissingPropertyDescriptorFields<false>(descriptor, scriptContext);
        if (JavascriptOperators::AreSamePropertyDescriptors(&filledDescriptor, currentDescriptor, scriptContext))
        {
            return TRUE;
        }

        if (!currentDescriptor->IsConfigurable()) // ES5 8.12.9.7.
        {
            if (descriptor.ConfigurableSpecified() && descriptor.IsConfigurable())
            {
                return Reject(throwOnError, scriptContext, JSERR_DefineProperty_NotConfigurable, propId);
            }
            if (descriptor.EnumerableSpecified() && descriptor.IsEnumerable() != currentDescriptor->IsEnumerable())
            {
                return Reject(throwOnError, scriptContext, JSERR_DefineProperty_NotConfigurable, propId);
            }
        }

        // Whether to merge attrbiutes from tempDescriptor into descriptor to keep original values
        // of some attributes from the object/use tempDescriptor for SetAttributes, or just use descriptor.
        // This is optimization to avoid 2 calls to SetAttributes.
        bool mergeDescriptors = false;

        // Whether to call SetAttributes with 'force' flag which forces setting all attrbiutes
        // rather than only specified or which have true values.
        // This is to make sure that the object has correct attrbiutes, as default values in the object are not for ES5.
        bool forceSetAttributes = false;

        PropertyDescriptor tempDescriptor;

        // ES5 8.12.9.8: If IsGenericDescriptor(Desc) is true, then no further validation is required.
        if (!descriptor.IsGenericDescriptor())
        {
            if (currentDescriptor->IsDataDescriptor() != descriptor.IsDataDescriptor())
            {
                // ES5 8.12.9.9: Else, if IsDataDescriptor(current) and IsDataDescriptor(Desc) have different results...
                if (!currentDescriptor->IsConfigurable())
                {
                    return Reject(throwOnError, scriptContext, JSERR_DefineProperty_NotConfigurable, propId);
                }

                if (needToSetProperty)
                {
                    if (currentDescriptor->IsDataDescriptor())
                    {
                        // ES5 8.12.9.9.b: Convert the property named P of object O from a data property to an accessor property.
                        // Preserve the existing values of the converted property's [[Configurable]] and [[Enumerable]] attributes
                        // and set the rest of the property's attributes to their default values.
                        PropertyAttributes preserveFromObject = currentDescriptor->GetAttributes() & (PropertyConfigurable | PropertyEnumerable);

                        BOOL isSetAccessorsSuccess = obj->SetAccessors(propId, defaultAccessorValue, defaultAccessorValue);

                        // It is valid for some objects to not-support getters and setters, specifically, for projection of an ABI method
                        // (CustomExternalObject => MapWithStringKey) which SetAccessors returns VBSErr_ActionNotSupported.
                        // But for non-extenal objects SetAccessors should succeed.
                        Assert(isSetAccessorsSuccess || obj->CanHaveInterceptors());

                        if (isSetAccessorsSuccess)
                        {
                            tempDescriptor.SetAttributes(preserveFromObject, PropertyConfigurable | PropertyEnumerable);
                            forceSetAttributes = true;  // use SetAttrbiutes with 'force' as default attributes in type system are different from ES5.
                            mergeDescriptors = true;
                        }
                    }
                    else
                    {
                        // ES5 8.12.9.9.c: Convert the property named P of object O from an accessor property to a data property.
                        // Preserve the existing values of the converted property's [[Configurable]] and [[Enumerable]] attributes
                        // and set the rest of the property's attributes to their default values.
                        // Note: avoid using SetProperty/SetPropertyWithAttributes here because they has undesired side-effects:
                        //       it calls previous setter and in some cases of attrbute values throws.
                        //       To walk around, call DeleteProperty and then AddProperty.
                        PropertyAttributes preserveFromObject = currentDescriptor->GetAttributes() & (PropertyConfigurable | PropertyEnumerable);

                        tempDescriptor.SetAttributes(preserveFromObject, PropertyConfigurable | PropertyEnumerable);
                        tempDescriptor.MergeFrom(descriptor);   // Update only fields specified in 'descriptor'.
                        Var descriptorValue = descriptor.ValueSpecified() ? descriptor.GetValue() : defaultDataValue;

                        // Note: HostDispath'es implementation of DeleteProperty currently throws E_NOTIMPL.
                        obj->DeleteProperty(propId, PropertyOperation_None);
                        BOOL tempResult = obj->SetPropertyWithAttributes(propId, descriptorValue, tempDescriptor.GetAttributes(), NULL, PropertyOperation_Force);
                        Assert(tempResult);

                        // At this time we already set value and attributes to desired values,
                        // thus we can skip step ES5 8.12.9.12 and simply return true.
                        return TRUE;
                    }
                }
            }
            else if (currentDescriptor->IsDataDescriptor() && descriptor.IsDataDescriptor())
            {
                // ES5 8.12.9.10: Else, if IsDataDescriptor(current) and IsDataDescriptor(Desc) are both true...
                if (!currentDescriptor->IsConfigurable())
                {
                    if (!currentDescriptor->IsWritable())
                    {
                        if ((descriptor.WritableSpecified() && descriptor.IsWritable()) ||  // ES5 8.12.9.10.a.i
                            (descriptor.ValueSpecified() &&
                                !JavascriptConversion::SameValue(descriptor.GetValue(), currentDescriptor->GetValue(), scriptContext))) // ES5 8.12.9.10.a.ii
                        {
                            return Reject(throwOnError, scriptContext, JSERR_DefineProperty_NotWritable, propId);
                        }
                    }
                }
                // ES5 8.12.9.10.b: else, the [[Configurable]] field of current is true, so any change is acceptable.
            }
            else
            {
                // ES5 8.12.9.11: Else, IsAccessorDescriptor(current) and IsAccessorDescriptor(Desc) are both true, so...
                Assert(currentDescriptor->IsAccessorDescriptor() && descriptor.IsAccessorDescriptor());
                if (!currentDescriptor->IsConfigurable())
                {
                    if ((descriptor.SetterSpecified() &&
                            !JavascriptConversion::SameValue(
                            JavascriptOperators::CanonicalizeAccessor(descriptor.GetSetter(), scriptContext),
                                JavascriptOperators::CanonicalizeAccessor(currentDescriptor->GetSetter(), scriptContext),
                                scriptContext)) ||
                        (descriptor.GetterSpecified() &&
                            !JavascriptConversion::SameValue(
                            JavascriptOperators::CanonicalizeAccessor(descriptor.GetGetter(), scriptContext),
                                JavascriptOperators::CanonicalizeAccessor(currentDescriptor->GetGetter(), scriptContext),
                                scriptContext)))
                    {
                        return Reject(throwOnError, scriptContext, JSERR_DefineProperty_NotConfigurable, propId);
                    }
                }
            }

            // This part is only for non-generic descriptors:
            //   ES5 8.12.9.12: For each attribute field of Desc that is present,
            //   set the correspondingly named attribute of the property named P of object O to the value of the field.
            if (descriptor.IsDataDescriptor())
            {
                if (descriptor.ValueSpecified() && needToSetProperty)
                {
                    // Set just the value by passing the current attributes of the property.
                    // If the property's attributes are also changing (perhaps becoming non-writable),
                    // this will be taken care of in the call to JavascriptOperators::SetAttributes below.
                    // Built-in Function.prototype properties 'length', 'arguments', and 'caller' are special cases.
                    BOOL tempResult = obj->SetPropertyWithAttributes(propId, descriptor.GetValue(), currentDescriptor->GetAttributes(), nullptr);
                    AssertMsg(tempResult || JavascriptFunction::IsBuiltinProperty(obj, propId), "If you hit this assert, most likely there is something wrong with the object/type.");
                }
            }
            else if (descriptor.IsAccessorDescriptor() && needToSetProperty)
            {
                Assert(descriptor.GetterSpecified() || descriptor.SetterSpecified());
                Var oldGetter = defaultAccessorValue, oldSetter = defaultAccessorValue;
                if (!descriptor.GetterSpecified() || !descriptor.SetterSpecified())
                {
                    // Unless both getter and setter are specified, make sure we don't overwrite old accessor.
                    obj->GetAccessors(propId, &oldGetter, &oldSetter, scriptContext);
                }

                Var getter = descriptor.GetterSpecified() ? descriptor.GetGetter() : oldGetter;
                Var setter = descriptor.SetterSpecified() ? descriptor.GetSetter() : oldSetter;

                obj->SetAccessors(propId, getter, setter);
            }
        } // if (!descriptor.IsGenericDescriptor())

        // Continue for all descriptors including generic:
        //   ES5 8.12.9.12: For each attribute field of Desc that is present,
        //   set the correspondingly named attribute of the property named P of object O to the value of the field.
        if (needToSetProperty)
        {
            if (mergeDescriptors)
            {
                tempDescriptor.MergeFrom(descriptor);
                JavascriptOperators::SetAttributes(obj, propId, tempDescriptor, forceSetAttributes);
            }
            else
            {
                JavascriptOperators::SetAttributes(obj, propId, descriptor, forceSetAttributes);
            }
        }
        return TRUE;
    }

    template <bool isAccessor>
    PropertyDescriptor JavascriptOperators::FillMissingPropertyDescriptorFields(PropertyDescriptor descriptor, ScriptContext* scriptContext)
    {
        PropertyDescriptor newDescriptor;
        const PropertyDescriptor* defaultDescriptor = scriptContext->GetLibrary()->GetDefaultPropertyDescriptor();
        if (isAccessor)
        {
            newDescriptor.SetGetter(descriptor.GetterSpecified() ? descriptor.GetGetter() : defaultDescriptor->GetGetter());
            newDescriptor.SetSetter(descriptor.SetterSpecified() ? descriptor.GetSetter() : defaultDescriptor->GetSetter());
        }
        else
        {
            newDescriptor.SetValue(descriptor.ValueSpecified() ? descriptor.GetValue() : defaultDescriptor->GetValue());
            newDescriptor.SetWritable(descriptor.WritableSpecified() ? descriptor.IsWritable() : defaultDescriptor->IsWritable());
        }
        newDescriptor.SetConfigurable(descriptor.ConfigurableSpecified() ? descriptor.IsConfigurable() : defaultDescriptor->IsConfigurable());
        newDescriptor.SetEnumerable(descriptor.EnumerableSpecified() ? descriptor.IsEnumerable() : defaultDescriptor->IsEnumerable());
        return newDescriptor;
    }
    // ES5: 15.4.5.1
    BOOL JavascriptOperators::DefineOwnPropertyForArray(JavascriptArray* arr, PropertyId propId, const PropertyDescriptor& descriptor, bool throwOnError, ScriptContext* scriptContext)
    {
        if (propId == PropertyIds::length)
        {
            if (!descriptor.ValueSpecified())
            {
                return DefineOwnPropertyDescriptor(arr, PropertyIds::length, descriptor, throwOnError, scriptContext);
            }

            PropertyDescriptor newLenDesc = descriptor;
            uint32 newLen = ES5Array::ToLengthValue(descriptor.GetValue(), scriptContext);
            newLenDesc.SetValue(JavascriptNumber::ToVar(newLen, scriptContext));

            uint32 oldLen = arr->GetLength();
            if (newLen >= oldLen)
            {
                return DefineOwnPropertyDescriptor(arr, PropertyIds::length, newLenDesc, throwOnError, scriptContext);
            }

            BOOL oldLenWritable = arr->IsWritable(PropertyIds::length);
            if (!oldLenWritable)
            {
                return Reject(throwOnError, scriptContext, JSERR_DefineProperty_NotWritable, propId);
            }

            bool newWritable = (!newLenDesc.WritableSpecified() || newLenDesc.IsWritable());
            if (!newWritable)
            {
                // Need to defer setting writable to false in case any elements cannot be deleted
                newLenDesc.SetWritable(true);
            }

            BOOL succeeded = DefineOwnPropertyDescriptor(arr, PropertyIds::length, newLenDesc, throwOnError, scriptContext);
            //
            // Our SetProperty(length) is also responsible to trim elements. When succeeded is
            //
            //  false:
            //      * length attributes rejected
            //      * elements not touched
            //  true:
            //      * length attributes are set successfully
            //      * elements trimming may be either completed or incompleted, length value is correct
            //
            //      * Strict mode TODO: Currently SetProperty(length) does not throw. If that throws, we need
            //        to update here to set correct newWritable even on exception.
            //
            if (!succeeded)
            {
                return false;
            }

            if (!newWritable) // Now set requested newWritable.
            {
                PropertyDescriptor newWritableDesc;
                newWritableDesc.SetWritable(false);
                DefineOwnPropertyDescriptor(arr, PropertyIds::length, newWritableDesc, false, scriptContext);
            }

            if (arr->GetLength() > newLen) // Delete incompleted
            {
                // Since SetProperty(length) not throwing, we'll reject here
                return Reject(throwOnError, scriptContext, JSERR_DefineProperty_Default, propId);
            }

            return true;
        }

        uint32 index;
        if (scriptContext->IsNumericPropertyId(propId, &index))
        {
            if (index >= arr->GetLength() && !arr->IsWritable(PropertyIds::length))
            {
                return Reject(throwOnError, scriptContext, JSERR_DefineProperty_LengthNotWritable, propId);
            }

            BOOL succeeded = DefineOwnPropertyDescriptor(arr, propId, descriptor, false, scriptContext);
            if (!succeeded)
            {
                return Reject(throwOnError, scriptContext, JSERR_DefineProperty_Default, propId);
            }

            // Out SetItem takes care of growing "length". we are done.
            return true;
        }

        return DefineOwnPropertyDescriptor(arr, propId, descriptor, throwOnError, scriptContext);
    }

    BOOL JavascriptOperators::SetPropertyDescriptor(RecyclableObject* object, PropertyId propId, PropertyDescriptor descriptor)
    {
        if (descriptor.ValueSpecified())
        {
            ScriptContext* requestContext = object->GetScriptContext(); // Real requestContext?
            JavascriptOperators::SetProperty(object, object, propId, descriptor.GetValue(), requestContext);
        }
        else if (descriptor.GetterSpecified() || descriptor.SetterSpecified())
        {
            JavascriptOperators::SetAccessors(object, propId, descriptor.GetGetter(), descriptor.GetSetter());
        }

        if (descriptor.EnumerableSpecified())
        {
            object->SetEnumerable(propId, descriptor.IsEnumerable());
        }
        if (descriptor.ConfigurableSpecified())
        {
            object->SetConfigurable(propId, descriptor.IsConfigurable());
        }
        if (descriptor.WritableSpecified())
        {
            object->SetWritable(propId, descriptor.IsWritable());
        }

        return true;
    }
    BOOL JavascriptOperators::ToPropertyDescriptorForProxyObjects(Var propertySpec, PropertyDescriptor* descriptor, ScriptContext* scriptContext)
    {
        if (!JavascriptOperators::IsObject(propertySpec))
        {
            return FALSE;
        }

        Var value;
        RecyclableObject* propertySpecObj = RecyclableObject::FromVar(propertySpec);

        if (JavascriptOperators::HasProperty(propertySpecObj, PropertyIds::writable) == TRUE)
        {
            if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::writable, &value, scriptContext))
            {
                descriptor->SetWritable(JavascriptConversion::ToBoolean(value, scriptContext) ? true : false);
            }
            else
            {
                AssertMsg(FALSE, "Proxy : HasProperty and GetProperty's result don't match for 'writable'.");
            }
        }

        if (JavascriptOperators::HasProperty(propertySpecObj, PropertyIds::enumerable) == TRUE)
        {
            if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::enumerable, &value, scriptContext))
            {
                descriptor->SetEnumerable(JavascriptConversion::ToBoolean(value, scriptContext) ? true : false);
            }
            else
            {
                AssertMsg(FALSE, "Proxy : HasProperty and GetProperty's result don't match for 'enumerable'.");
            }
        }

        if (JavascriptOperators::HasProperty(propertySpecObj, PropertyIds::configurable) == TRUE)
        {
            if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::configurable, &value, scriptContext))
            {
                descriptor->SetConfigurable(JavascriptConversion::ToBoolean(value, scriptContext) ? true : false);
            }
            else
            {
                AssertMsg(FALSE, "Proxy : HasProperty and GetProperty's result don't match for 'configurable'.");
            }
        }

        if (JavascriptOperators::HasProperty(propertySpecObj, PropertyIds::value) == TRUE)
        {
            if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::value, &value, scriptContext))
            {
                descriptor->SetValue(value);
            }
            else
            {
                AssertMsg(FALSE, "Proxy : HasProperty and GetProperty's result don't match for 'value'.");
            }
        }

        if (JavascriptOperators::HasProperty(propertySpecObj, PropertyIds::get) == TRUE)
        {
            if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::get, &value, scriptContext))
            {
                if (JavascriptOperators::GetTypeId(value) != TypeIds_Undefined && (false == JavascriptConversion::IsCallable(value)))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_NeedFunction, scriptContext->GetPropertyName(PropertyIds::get)->GetBuffer());
                }
                descriptor->SetGetter(value);
            }
            else
            {
                AssertMsg(FALSE, "Proxy : HasProperty and GetProperty's result don't match for 'get'.");
            }
        }

        if (JavascriptOperators::HasProperty(propertySpecObj, PropertyIds::set) == TRUE)
        {
            if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::set, &value, scriptContext))
            {
                if (JavascriptOperators::GetTypeId(value) != TypeIds_Undefined && (false == JavascriptConversion::IsCallable(value)))
                {
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_NeedFunction, scriptContext->GetPropertyName(PropertyIds::set)->GetBuffer());
                }
                descriptor->SetSetter(value);
            }
            else
            {
                AssertMsg(FALSE, "Proxy : HasProperty and GetProperty's result don't match for 'set'.");
            }
        }
        return TRUE;
    }

    BOOL JavascriptOperators::ToPropertyDescriptorForGenericObjects(Var propertySpec, PropertyDescriptor* descriptor, ScriptContext* scriptContext)
    {
        if (!JavascriptOperators::IsObject(propertySpec))
        {
            return FALSE;
        }

        Var value;
        RecyclableObject* propertySpecObj = RecyclableObject::FromVar(propertySpec);

        if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::writable, &value, scriptContext))
        {
            descriptor->SetWritable(JavascriptConversion::ToBoolean(value, scriptContext) ? true : false);
        }

        if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::enumerable, &value, scriptContext))
        {
            descriptor->SetEnumerable(JavascriptConversion::ToBoolean(value, scriptContext) ? true : false);
        }

        if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::configurable, &value, scriptContext))
        {
            descriptor->SetConfigurable(JavascriptConversion::ToBoolean(value, scriptContext) ? true : false);
        }

        if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::value, &value, scriptContext))
        {
            descriptor->SetValue(value);
        }

        if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::get, &value, scriptContext))
        {
            if (JavascriptOperators::GetTypeId(value) != TypeIds_Undefined && (false == JavascriptConversion::IsCallable(value)))
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_NeedFunction, scriptContext->GetPropertyName(PropertyIds::get)->GetBuffer());
            }
            descriptor->SetGetter(value);
        }

        if (JavascriptOperators::GetProperty(propertySpecObj, PropertyIds::set, &value, scriptContext))
        {
            if (JavascriptOperators::GetTypeId(value) != TypeIds_Undefined && (false == JavascriptConversion::IsCallable(value)))
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_NeedFunction, scriptContext->GetPropertyName(PropertyIds::set)->GetBuffer());
            }
            descriptor->SetSetter(value);
        }

        return TRUE;
    }

    BOOL JavascriptOperators::ToPropertyDescriptor(Var propertySpec, PropertyDescriptor* descriptor, ScriptContext* scriptContext)
    {
        if (JavascriptProxy::Is(propertySpec))
        {
            if (ToPropertyDescriptorForProxyObjects(propertySpec, descriptor, scriptContext) == FALSE)
            {
                return FALSE;
            }
        }
        else
        {
            if (ToPropertyDescriptorForGenericObjects(propertySpec, descriptor, scriptContext) == FALSE)
            {
                return FALSE;
            }
        }

        if (descriptor->GetterSpecified() || descriptor->SetterSpecified())
        {
            if (descriptor->ValueSpecified())
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_CannotHaveAccessorsAndValue);
            }
            if (descriptor->WritableSpecified())
            {
                long hCode = descriptor->IsWritable() ? JSERR_InvalidAttributeTrue : JSERR_InvalidAttributeFalse;
                JavascriptError::ThrowTypeError(scriptContext, hCode, L"writable");
            }
        }

        descriptor->SetOriginal(propertySpec);

        return TRUE;
    }

    void JavascriptOperators::CompletePropertyDescriptor(PropertyDescriptor* resultDescriptor, PropertyDescriptor* likeDescriptor, ScriptContext* requestContext)
    {
        const PropertyDescriptor* likePropertyDescriptor = likeDescriptor;
        //    1. Assert: LikeDesc is either a Property Descriptor or undefined.
        //    2. ReturnIfAbrupt(Desc).
        //    3. Assert : Desc is a Property Descriptor
        //    4. If LikeDesc is undefined, then set LikeDesc to Record{ [[Value]]: undefined, [[Writable]] : false, [[Get]] : undefined, [[Set]] : undefined, [[Enumerable]] : false, [[Configurable]] : false }.
        if (likePropertyDescriptor == nullptr)
        {
            likePropertyDescriptor = requestContext->GetLibrary()->GetDefaultPropertyDescriptor();
        }
        //    5. If either IsGenericDescriptor(Desc) or IsDataDescriptor(Desc) is true, then
        if (resultDescriptor->IsDataDescriptor() || resultDescriptor->IsGenericDescriptor())
        {
            //    a.If Desc does not have a[[Value]] field, then set Desc.[[Value]] to LikeDesc.[[Value]].
            //    b.If Desc does not have a[[Writable]] field, then set Desc.[[Writable]] to LikeDesc.[[Writable]].
            if (!resultDescriptor->ValueSpecified())
            {
                resultDescriptor->SetValue(likePropertyDescriptor->GetValue());
            }
            if (!resultDescriptor->WritableSpecified())
            {
                resultDescriptor->SetWritable(likePropertyDescriptor->IsWritable());
            }
        }
        else
        {
            //    6. Else,
            //    a.If Desc does not have a[[Get]] field, then set Desc.[[Get]] to LikeDesc.[[Get]].
            //    b.If Desc does not have a[[Set]] field, then set Desc.[[Set]] to LikeDesc.[[Set]].
            if (!resultDescriptor->GetterSpecified())
            {
                resultDescriptor->SetGetter(likePropertyDescriptor->GetGetter());
            }
            if (!resultDescriptor->SetterSpecified())
            {
                resultDescriptor->SetSetter(likePropertyDescriptor->GetSetter());
            }
        }
        //    7. If Desc does not have an[[Enumerable]] field, then set Desc.[[Enumerable]] to LikeDesc.[[Enumerable]].
        //    8. If Desc does not have a[[Configurable]] field, then set Desc.[[Configurable]] to LikeDesc.[[Configurable]].
        //    9. Return Desc.
        if (!resultDescriptor->EnumerableSpecified())
        {
            resultDescriptor->SetEnumerable(likePropertyDescriptor->IsEnumerable());
        }
        if (!resultDescriptor->ConfigurableSpecified())
        {
            resultDescriptor->SetConfigurable(likePropertyDescriptor->IsConfigurable());
        }
    }

    Var JavascriptOperators::OP_InvokePut(Js::ScriptContext *scriptContext, Var instance, CallInfo callInfo, ...)
    {
        // Handle a store to a call result: x(y) = z.
        // This is not strictly permitted in JScript, but some scripts expect to be able to use
        // the syntax to set properties of ActiveX objects.
        // We handle this by deferring to a virtual method of type. This incurs an extra level of
        // indirection but seems preferable to adding the "put" method as a member of every type
        // and using the normal JScript calling mechanism.

        RUNTIME_ARGUMENTS(args, callInfo);
        AssertMsg(args.Info.Count > 0, "Missing this argument in InvokePut");

        if (TaggedNumber::Is(instance))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction /* TODO-ERROR: get arg name - aFunc */);
        }
        RecyclableObject* function = RecyclableObject::FromVar(instance);
        return function->InvokePut(args);
    }

    // Conformance to: ES5 8.6.1.
    // Set attributes on the object as provided by property descriptor.
    // If force parameter is true, we force SetAttributes call even if none of the attributes are defined by the descriptor.
    // NOTE: does not set [[Get]], [Set]], [[Value]]
    void JavascriptOperators::SetAttributes(RecyclableObject* object, PropertyId propId, const PropertyDescriptor& descriptor, bool force)
    {
        Assert(object);

        BOOL isWritable = FALSE;
        if (descriptor.IsDataDescriptor())
        {
            isWritable = descriptor.WritableSpecified() ? descriptor.IsWritable() : FALSE;
        }
        else if (descriptor.IsAccessorDescriptor())
        {
            // HACK: internally we keep writable for accessors as true.
            // The reason is that JavascriptOperators::OP_SetProperty checks for RecyclableObject::FromVar(instance)->IsWritableOrAccessor(propertyId),
            // which should in fact check for 'is writable or accessor' but since there is no GetAttributes, we can't do that efficiently.
            isWritable = TRUE;
        }

        // TODO: call object->SetAttributes which is much more efficient as that's 1 call instead of 3.
        //       Can't do that now as object->SetAttributes doesn't provide a way which attributes to modify and which not.
        if (force || descriptor.ConfigurableSpecified())
        {
            object->SetConfigurable(propId, descriptor.ConfigurableSpecified() ? descriptor.IsConfigurable() : FALSE);
        }
        if (force || descriptor.EnumerableSpecified())
        {
            object->SetEnumerable(propId, descriptor.EnumerableSpecified() ? descriptor.IsEnumerable() : FALSE);
        }
        if (force || descriptor.WritableSpecified() || isWritable)
        {
            object->SetWritable(propId, isWritable);
        }
    }

    void JavascriptOperators::OP_ClearAttributes(Var instance, PropertyId propertyId)
    {
        Assert(instance);

        if (RecyclableObject::Is(instance))
        {
            RecyclableObject* obj = RecyclableObject::FromVar(instance);
            obj->SetAttributes(propertyId, PropertyNone);
        }
    }

    void JavascriptOperators::OP_Freeze(Var instance)
    {
        Assert(instance);

        if (RecyclableObject::Is(instance))
        {
            RecyclableObject* obj = RecyclableObject::FromVar(instance);
            obj->Freeze();
        }
    }

    BOOL JavascriptOperators::Reject(bool throwOnError, ScriptContext* scriptContext, long errorCode, PropertyId propertyId)
    {
        Assert(scriptContext);

        if (throwOnError)
        {
            JavascriptError::ThrowTypeError(scriptContext, errorCode, scriptContext->GetThreadContext()->GetPropertyName(propertyId)->GetBuffer());
        }
        return FALSE;
    }

    bool JavascriptOperators::AreSamePropertyDescriptors(const PropertyDescriptor* x, const PropertyDescriptor* y, ScriptContext* scriptContext)
    {
        Assert(scriptContext);

        if (x->ConfigurableSpecified() != y->ConfigurableSpecified() || x->IsConfigurable() != y->IsConfigurable() ||
            x->EnumerableSpecified() != y->EnumerableSpecified() || x->IsEnumerable() != y->IsEnumerable())
        {
            return false;
        }

        if (x->IsDataDescriptor())
        {
            if (!y->IsDataDescriptor() || x->WritableSpecified() != y->WritableSpecified() || x->IsWritable() != y->IsWritable())
            {
                return false;
            }

            if (x->ValueSpecified())
            {
                if (!y->ValueSpecified() || !JavascriptConversion::SameValue(x->GetValue(), y->GetValue(), scriptContext))
                {
                    return false;
                }
            }
        }
        else if (x->IsAccessorDescriptor())
        {
            if (!y->IsAccessorDescriptor())
            {
                return false;
            }

            if (x->GetterSpecified())
            {
                if (!y->GetterSpecified() || !JavascriptConversion::SameValue(
                    JavascriptOperators::CanonicalizeAccessor(x->GetGetter(), scriptContext),
                    JavascriptOperators::CanonicalizeAccessor(y->GetGetter(), scriptContext), scriptContext))
                {
                    return false;
                }
            }

            if (x->SetterSpecified())
            {
                if (!y->SetterSpecified() || !JavascriptConversion::SameValue(
                    JavascriptOperators::CanonicalizeAccessor(x->GetSetter(), scriptContext),
                    JavascriptOperators::CanonicalizeAccessor(y->GetSetter(), scriptContext), scriptContext))
                {
                    return false;
                }
            }
        }

        return true;
    }

    // Check if an accessor is undefined (null or defaultAccessor)
    bool JavascriptOperators::IsUndefinedAccessor(Var accessor, ScriptContext* scriptContext)
    {
        return null == accessor || scriptContext->GetLibrary()->GetDefaultAccessorFunction() == accessor;
    }

    // Converts default accessor to undefined.
    // Can be used when comparing accessors.
    Var JavascriptOperators::CanonicalizeAccessor(Var accessor, ScriptContext* scriptContext)
    {
        Assert(scriptContext);

        if (IsUndefinedAccessor(accessor, scriptContext))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
        return accessor;
    }

    Var JavascriptOperators::DefaultAccessor(RecyclableObject* function, CallInfo callInfo, ...)
    {
        return function->GetLibrary()->GetUndefined();
    }

    void FrameDisplay::SetItem(uint index, void* item)
    {
        AssertMsg(index < this->length, "Invalid frame display access");

        scopes[index] = item;
    }

    void *FrameDisplay::GetItem(uint index)
    {
        AssertMsg(index < this->length, "Invalid frame display access");

        return scopes[index];
    }

    // Grab the "this" pointer, mapping a root object to its associated host object.
    Var JavascriptOperators::RootToThisObject(const Var object, ScriptContext* scriptContext)
    {
        Js::Var thisVar = object;
        TypeId typeId = Js::JavascriptOperators::GetTypeId(thisVar);

        switch (typeId)
        {
        case Js::TypeIds_GlobalObject:
            return ((Js::GlobalObject*)thisVar)->ToThis();

        case Js::TypeIds_ModuleRoot:
            return Js::JavascriptOperators::GetThisFromModuleRoot(thisVar);

        default:
            if (typeId == scriptContext->GetDirectHostTypeId())
            {
                return ((RecyclableObject*)thisVar)->GetLibrary()->GetGlobalObject()->ToThis();
            }

        }

        return thisVar;
    }


    Var JavascriptOperators::CallGetter(RecyclableObject * const function, Var const object, ScriptContext * requestContext)
    {
        ScriptContext * scriptContext = function->GetScriptContext();
        ThreadContext * threadContext = scriptContext->GetThreadContext();
        return threadContext->ExecuteImplicitCall(function, ImplicitCall_Accessor, [=]() -> Js::Var
        {
            // Stack object should have a pre-op bail on implicit call.  We shouldn't see them here.
            // Stack numbers are ok, as we will call ToObject to wrap it in a number object anyway
            // See JavascriptOperators::GetThisHelper
            Assert(JavascriptOperators::GetTypeId(object) == TypeIds_Integer ||
                JavascriptOperators::GetTypeId(object) == TypeIds_Number || !ThreadContext::IsOnStack(object));

            // Verify that the scriptcontext is alive before firing getter/setter
            if (!scriptContext->VerifyAlive(!function->IsExternal(), requestContext))
            {
                return nullptr;
            }
            CallFlags flags = CallFlags_Value;

            Var thisVar = RootToThisObject(object, scriptContext);

            RecyclableObject* marshalledFunction = RecyclableObject::FromVar(CrossSite::MarshalVar(requestContext, function));
            Var result = marshalledFunction->GetEntryPoint()(function, CallInfo(flags, 1), thisVar);

            result = CrossSite::MarshalVar(requestContext, result);

            return result;
        });
    }

    void JavascriptOperators::CallSetter(RecyclableObject * const function, Var const  object, Var const value, ScriptContext * requestContext)
    {
        ScriptContext * scriptContext = function->GetScriptContext();
        ThreadContext * threadContext = scriptContext->GetThreadContext();
        threadContext->ExecuteImplicitCall(function, ImplicitCall_Accessor, [=]() -> Js::Var
        {
            // Stack object should have a pre-op bail on implicit call.  We shouldn't see them here.
            // Stack numbers are ok, as we will call ToObject to wrap it in a number object anyway
            // See JavascriptOperators::GetThisHelper
            Assert(JavascriptOperators::GetTypeId(object) == TypeIds_Integer ||
                JavascriptOperators::GetTypeId(object) == TypeIds_Number || !ThreadContext::IsOnStack(object));

            // Verify that the scriptcontext is alive before firing getter/setter
            if (!scriptContext->VerifyAlive(!function->IsExternal(), requestContext))
            {
                return nullptr;
            }

            CallFlags flags = CallFlags_Value;
            Var putValue = value;

            // TODO: we should have requestContext everywhere, even in the setproperty related codepath. It's too big a change at this time. Let me
            // fix this for now and have the separate change going in parallel.
            if (requestContext)
            {
                putValue = CrossSite::MarshalVar(requestContext, value);
            }

            Var thisVar = RootToThisObject(object, scriptContext);

            RecyclableObject* marshalledFunction = function;
            if (requestContext)
            {
                marshalledFunction = RecyclableObject::FromVar(CrossSite::MarshalVar(requestContext, function));
            }

            Var result = marshalledFunction->GetEntryPoint()(function, CallInfo(flags, 2), thisVar, putValue);
            Assert(result);
            return null;
        });
    }

    void * JavascriptOperators::AllocMemForVarArray(size_t size, Recycler* recycler)
    {
        TRACK_ALLOC_INFO(recycler, Js::Var, Recycler, 0, (size_t)(size / sizeof(Js::Var)));
        return recycler->AllocZero(size);
    }

    void * JavascriptOperators::AllocUninitializedNumber(Js::RecyclerJavascriptNumberAllocator * allocator)
    {
        TRACK_ALLOC_INFO(allocator->GetRecycler(), Js::JavascriptNumber, Recycler, 0, (size_t)-1);
        return allocator->Alloc(sizeof(Js::JavascriptNumber));
    }

    void JavascriptOperators::ScriptAbort()
    {
        throw ScriptAbortException();
    }

    void PolymorphicInlineCache::Finalize(bool isShutdown)
    {
        if (size == 0)
        {
            // Already finalized
            Assert(!inlineCaches && !prev && !next);
            return;
        }

        Assert(inlineCaches && size > 0);

        // If we're not shutting down (as in closing the script context), we need to remove our inline caches from
        // thread context's invalidation lists, and release memory back to the arena.  During script context shutdown,
        // we leave everything in place, because the inline cache arena will stay alive until script context is destroyed
        // (as in destructor has been called) and thus the invadation lists are safe to keep references to caches from this
        // script context.  We will, however, zero all inline caches so that we don't have to process them on subsequent
        // collections, which may still happen from other script contexts.
        if (isShutdown)
        {
            memset(inlineCaches, 0, size * sizeof(InlineCache));
        }
        else
        {
            for (int i = 0; i < size; i++)
            {
                inlineCaches[i].RemoveFromInvalidationList();
            }

            AllocatorDeleteArray(InlineCacheAllocator, functionBody->GetScriptContext()->GetInlineCacheAllocator(), size, inlineCaches);
#ifdef POLY_INLINE_CACHE_SIZE_STATS
            functionBody->GetScriptContext()->GetInlineCacheAllocator()->LogPolyCacheFree(size * sizeof(InlineCache));
#endif
        }

        // Remove this PolymorphicInlineCache from the list
        if (this == functionBody->GetPolymorphicInlineCachesHead())
        {
            Assert(!prev);
            if (next)
            {
                Assert(next->prev == this);
                next->prev = null;
            }
            functionBody->SetPolymorphicInlineCachesHead(next);
        }
        else
        {
            if (prev)
            {
                Assert(prev->next == this);
                prev->next = next;
            }
            if (next)
            {
                Assert(next->prev == this);
                next->prev = prev;
            }
        }
        prev = next = null;
        inlineCaches = null;
        size = 0;
    }

    JavascriptString * JavascriptOperators::Concat3(Var aLeft, Var aCenter, Var aRight, ScriptContext * scriptContext)
    {
        // Make sure we do the concversion in order from left to right
        JavascriptString * strLeft = JavascriptConversion::ToPrimitiveString(aLeft, scriptContext);
        JavascriptString * strCenter = JavascriptConversion::ToPrimitiveString(aCenter, scriptContext);
        JavascriptString * strRight = JavascriptConversion::ToPrimitiveString(aRight, scriptContext);
        return JavascriptString::Concat3(strLeft, strCenter, strRight);
    }

    JavascriptString *
    JavascriptOperators::NewConcatStrMulti(Var a1, Var a2, uint count, ScriptContext * scriptContext)
    {
        // Make sure we do the concversion in order
        JavascriptString * str1 = JavascriptConversion::ToPrimitiveString(a1, scriptContext);
        JavascriptString * str2 = JavascriptConversion::ToPrimitiveString(a2, scriptContext);
        return ConcatStringMulti::New(count, str1, str2, scriptContext);
    }

    void
    JavascriptOperators::SetConcatStrMultiItem(Var concatStr, Var str, uint index, ScriptContext * scriptContext)
    {
        if (BinaryFeatureControl::LanguageService() && !ConcatStringMulti::Is(concatStr))
        {
            // This is possible in the LS, as NewConcatStrMulti could fail and we eat exception.
            return;
        }

        ConcatStringMulti::FromVar(concatStr)->SetItem(index,
            JavascriptConversion::ToPrimitiveString(str, scriptContext));
    }

    void
    JavascriptOperators::SetConcatStrMultiItem2(Var concatStr, Var str1, Var str2, uint index, ScriptContext * scriptContext)
    {
        if (BinaryFeatureControl::LanguageService() && !ConcatStringMulti::Is(concatStr))
        {
            // This is possible in the LS, as NewConcatStrMulti could fail and we eat exception.
            return;
        }

        ConcatStringMulti * cs = ConcatStringMulti::FromVar(concatStr);
        cs->SetItem(index, JavascriptConversion::ToPrimitiveString(str1, scriptContext));
        cs->SetItem(index + 1, JavascriptConversion::ToPrimitiveString(str2, scriptContext));
    }

    void JavascriptOperators::OP_SetComputedNameVar(Var method, Var computedNameVar)
    {
        ScriptFunctionBase *scriptFunction = ScriptFunctionBase::FromVar(method);
        scriptFunction->SetComputedNameVar(computedNameVar);
    }

    void JavascriptOperators::OP_SetHomeObj(Var method, Var homeObj)
    {
        ScriptFunctionBase *scriptFunction = ScriptFunctionBase::FromVar(method);
        scriptFunction->SetHomeObj(homeObj);
    }

    Var JavascriptOperators::OP_LdSuper(Var scriptFunction, ScriptContext * scriptContext)
    {
        // Ensure this is not a stack ScriptFunction
        if (!ScriptFunction::Is(scriptFunction) || ThreadContext::IsOnStack(scriptFunction))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        ScriptFunction *instance = ScriptFunction::FromVar(scriptFunction);

        // We keep a reference to the current class rather than its super prototype
        // since the prototype could change.
        Var homeObj = instance->GetHomeObj();

        if (homeObj == nullptr || !RecyclableObject::Is(homeObj))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        RecyclableObject *thisObjPrototype = RecyclableObject::FromVar(homeObj);

        Assert(thisObjPrototype != nullptr);

        RecyclableObject *superClass = thisObjPrototype->GetPrototype();

        if (superClass == nullptr || !RecyclableObject::Is(superClass))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        return superClass;
    }

    Var JavascriptOperators::OP_ScopedLdSuper(Var scriptFunction, ScriptContext * scriptContext)
    {
        ScriptFunction *instance = ScriptFunction::FromVar(scriptFunction);
        Var superRef = nullptr;

        FrameDisplay *frameDisplay = instance->GetEnvironment();

        if (frameDisplay->GetLength() == 0)
        {
            // Globally scoped evals are a syntax error
            JavascriptError::ThrowSyntaxError(scriptContext, ERRSuperInGlobalEval, L"super");
        }

        // Iterate over the scopes in the FrameDisplay, looking for the super property.
        for (unsigned i = 0; i < frameDisplay->GetLength(); ++i)
        {
            void *currScope = frameDisplay->GetItem(i);
            if (RecyclableObject::Is(currScope))
            {
                if (BlockActivationObject::Is(currScope))
                {
                    // We won't find super in a block scope.
                    continue;
                }

                RecyclableObject *recyclableObject = RecyclableObject::FromVar(currScope);
                if (GetProperty(recyclableObject, Js::PropertyIds::_superReferenceSymbol, &superRef, scriptContext))
                {
                    return superRef;
                }

                if (HasProperty(recyclableObject, Js::PropertyIds::_lexicalThisSlotSymbol))
                {
                    // If we reach 'this' and haven't found the super reference, we don't need to look any further.
                    JavascriptError::ThrowReferenceError(scriptContext, JSERR_BadSuperReference, L"super");
                }
            }
        }

        if (superRef == nullptr)
        {
            // We didn't find a super reference. Emit a reference error.
            JavascriptError::ThrowReferenceError(scriptContext, JSERR_BadSuperReference, L"super");
        }

        return superRef;
    }

    Var JavascriptOperators::OP_ResumeYield(ResumeYieldData* yieldData, RecyclableObject* iterator)
    {
        // CONSIDER: Fast path this early out return path in JITed code before helper call to avoid the helper call overhead in the common case e.g. next() calls.
        if (yieldData->exceptionObj == nullptr)
        {
            return yieldData->data;
        }

        ScriptContext* scriptContext = yieldData->exceptionObj->GetScriptContext();
        bool isReturn = yieldData->exceptionObj->IsGeneratorReturnException();

        if (iterator != nullptr)
        {
            PropertyId propertyId = isReturn ? PropertyIds::return_ : PropertyIds::throw_;
            Var prop = nullptr;

            if (JavascriptOperators::GetProperty(iterator, iterator, propertyId, &prop, iterator->GetScriptContext()))
            {
                Var args [] = { iterator, yieldData->data };
                CallInfo callInfo(CallFlags_Value, _countof(args));
                RecyclableObject* method = RecyclableObject::FromVar(prop);

                Var result = JavascriptFunction::CallFunction<true>(method, method->GetEntryPoint(), Arguments(callInfo, args));

                if (isReturn)
                {
                    if (!JavascriptOperators::IsObject(result))
                    {
                        JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
                    }

                    Var value = JavascriptOperators::GetProperty(RecyclableObject::FromVar(result), PropertyIds::value, scriptContext);
                    // CONSIDER: Using an exception to carry the return value and force finally code to execute is a bit of a janky
                    // solution since we have to override the value here in the case of yield* expressions.  It works but is there
                    // a more elegant way?
                    //
                    // Instead what if ResumeYield was a "set Dst then optionally branch" opcode, that could also throw? Then we could
                    // avoid using a special exception entirely with byte code something like this:
                    //
                    // ;; Ry is the yieldData
                    //
                    // ResumeYield Rx Ry $returnPathLabel
                    // ... code like normal
                    // $returnPathLabel:
                    // Ld_A R0 Rx
                    // Br $exitFinallyAndReturn
                    //
                    // This would probably give better performance for the common case of calling next() on generators since we wouldn't
                    // have to wrap the call to the generator code in a try catch.
                    yieldData->exceptionObj->SetThrownObject(value);
                }
            }
        }

        if (!isReturn)
        {
            // Use ThrowExceptionObject() to get debugger support for breaking on throw
            JavascriptExceptionOperators::ThrowExceptionObject(yieldData->exceptionObj, scriptContext, true);
        }

        // Do not use ThrowExceptionObject for return() API exceptions since these exceptions are not real exceptions
        throw yieldData->exceptionObj;
    }

    Js::Var
    JavascriptOperators::BoxStackInstance(Js::Var instance, ScriptContext * scriptContext, bool allowStackFunction)
    {
        if (!ThreadContext::IsOnStack(instance) || (allowStackFunction && !TaggedNumber::Is(instance) && (*(int*)instance & 1)))
        {
            return instance;
        }

        TypeId typeId = JavascriptOperators::GetTypeId(instance);
        switch (typeId)
        {
        case Js::TypeIds_Number:
#if !FLOATVAR
            return JavascriptNumber::BoxStackInstance(instance, scriptContext);
#endif
            // Fall thru for FLOATVAR
        case Js::TypeIds_Integer:
            return instance;
        case Js::TypeIds_RegEx:
            return JavascriptRegExp::BoxStackInstance(JavascriptRegExp::FromVar(instance));
        case Js::TypeIds_Object:
            return DynamicObject::BoxStackInstance(DynamicObject::FromVar(instance));
        case Js::TypeIds_Array:
            return JavascriptArray::BoxStackInstance(JavascriptArray::FromVar(instance));
        case Js::TypeIds_NativeIntArray:
            return JavascriptNativeIntArray::BoxStackInstance(JavascriptNativeIntArray::FromVar(instance));
        case Js::TypeIds_NativeFloatArray:
            return JavascriptNativeFloatArray::BoxStackInstance(JavascriptNativeFloatArray::FromVar(instance));
        case Js::TypeIds_Function:
            Assert(allowStackFunction);
            // Stack functions are deal with not mar mark them, but by nested function escape analysis
            // in the front end.  No need to box here.
            return instance;
        case Js::TypeIds_CopyOnAccessNativeIntArray:
            Assert(false);
            // fall-through
        default:
            Assert(false);
            return instance;
        };
    }
    ImplicitCallFlags
    JavascriptOperators::CacheAndClearImplicitBit(ScriptContext* scriptContext)
    {
        ImplicitCallFlags prevImplicitCallFlags = scriptContext->GetThreadContext()->GetImplicitCallFlags();
        scriptContext->GetThreadContext()->ClearImplicitCallFlags();
        return prevImplicitCallFlags;
    }
    ImplicitCallFlags
    JavascriptOperators::CheckAndUpdateFunctionBodyWithImplicitFlag(FunctionBody* functionBody)
    {
        ScriptContext* scriptContext = functionBody->GetScriptContext();
        ImplicitCallFlags currImplicitCallFlags = scriptContext->GetThreadContext()->GetImplicitCallFlags();
        if ((currImplicitCallFlags > ImplicitCall_None))
        {
            functionBody->SetHasOnlyThisStmts(false);
        }
        return currImplicitCallFlags;
    }
    void
    JavascriptOperators::RestoreImplicitFlag(ScriptContext* scriptContext, ImplicitCallFlags prevImplicitCallFlags, ImplicitCallFlags currImplicitCallFlags)
    {
        scriptContext->GetThreadContext()->SetImplicitCallFlags((ImplicitCallFlags)(prevImplicitCallFlags | currImplicitCallFlags));
    }

    FunctionProxy*
    JavascriptOperators::GetDeferredDeserializedFunctionProxy(JavascriptFunction* func)
    {
        FunctionProxy* proxy = func->GetFunctionProxy();
        if (proxy->GetFunctionProxy() != proxy)
        {
            proxy = proxy->GetFunctionProxy();
        }
        return proxy;
    }

    template <>
    Js::Var JavascriptOperators::GetElementAtIndex(Js::JavascriptArray* arrayObject, UINT index, Js::ScriptContext* scriptContext)
    {
        Js::Var result;
        if (Js::JavascriptOperators::OP_GetElementI_ArrayFastPath(arrayObject, index, &result, scriptContext))
        {
            return result;
        }
        return scriptContext->GetMissingItemResult(arrayObject, index);
    }

    template<>
    Js::Var JavascriptOperators::GetElementAtIndex(Js::JavascriptNativeIntArray* arrayObject, UINT index, Js::ScriptContext* scriptContext)
    {
        Js::Var result;
        if (Js::JavascriptOperators::OP_GetElementI_ArrayFastPath(arrayObject, index, &result, scriptContext))
        {
            return result;
        }
        return scriptContext->GetMissingItemResult(arrayObject, index);
    }

    template<>
    Js::Var JavascriptOperators::GetElementAtIndex(Js::JavascriptNativeFloatArray* arrayObject, UINT index, Js::ScriptContext* scriptContext)
    {
        Js::Var result;
        if (Js::JavascriptOperators::OP_GetElementI_ArrayFastPath(arrayObject, index, &result, scriptContext))
        {
            return result;
        }
        return scriptContext->GetMissingItemResult(arrayObject, index);
    }

    template<>
    Js::Var JavascriptOperators::GetElementAtIndex(Js::Var* arrayObject, UINT index, Js::ScriptContext* scriptContext)
    {
        return Js::JavascriptOperators::OP_GetElementI_Int32(*arrayObject, index, scriptContext);
    }

    template<typename T>
    void JavascriptOperators::ObjectToNativeArray(T* arrayObject,
        JsNativeValueType valueType,
        __in UINT length,
        __in UINT elementSize,
        __out_xcount(length*elementSize) byte* buffer,
        Js::ScriptContext* scriptContext)
    {
        Var element;
#if DBG
        uint64 allocSize = length * elementSize;
#endif
        // TODO:further fast path the call for things like IntArray convert to int, floatarray convert to float etc.
        // such that we don't need boxing.
        switch (valueType)
        {
        case JsInt8Type:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((int8*)buffer)[i] = Js::JavascriptConversion::ToInt8(element, scriptContext);
                Assert(i*sizeof(int8) < allocSize);
            }
            break;
        case JsUint8Type:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((uint8*)buffer)[i] = Js::JavascriptConversion::ToUInt8(element, scriptContext);
                Assert(i*sizeof(uint8) < allocSize);
            }
            break;
        case JsInt16Type:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((int16*)buffer)[i] = Js::JavascriptConversion::ToInt16(element, scriptContext);
                Assert(i*sizeof(int16) < allocSize);
            }
            break;
        case JsUint16Type:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((uint16*)buffer)[i] = Js::JavascriptConversion::ToUInt16(element, scriptContext);
                Assert(i*sizeof(int16) < allocSize);
            }
            break;
        case JsInt32Type:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((int32*)buffer)[i] = Js::JavascriptConversion::ToInt32(element, scriptContext);
                Assert(i*sizeof(int32) < allocSize);
            }
            break;
        case JsUint32Type:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((uint32*)buffer)[i] = Js::JavascriptConversion::ToUInt32(element, scriptContext);
                Assert(i*sizeof(uint32) < allocSize);
            }
            break;
        case JsInt64Type:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((int64*)buffer)[i] = Js::JavascriptConversion::ToInt64(element, scriptContext);
                Assert(i*sizeof(int64) < allocSize);
            }
            break;
        case JsUint64Type:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((uint64*)buffer)[i] = Js::JavascriptConversion::ToUInt64(element, scriptContext);
                Assert(i*sizeof(uint64) < allocSize);
            }
            break;
        case JsFloatType:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((float*)buffer)[i] = Js::JavascriptConversion::ToFloat(element, scriptContext);
                Assert(i*sizeof(float) < allocSize);
            }
            break;
        case JsDoubleType:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                ((double*)buffer)[i] = Js::JavascriptConversion::ToNumber(element, scriptContext);
                Assert(i*sizeof(double) < allocSize);
            }
            break;
        case JsNativeStringType:
            for (UINT i = 0; i < length; i++)
            {
                element = GetElementAtIndex(arrayObject, i, scriptContext);
                Js::JavascriptString* string = Js::JavascriptConversion::ToString(element, scriptContext);
                (((JsNativeString*)buffer)[i]).str = string->GetSz();
                (((JsNativeString*)buffer)[i]).length = string->GetLength();
                Assert(i*sizeof(double) < allocSize);
            }
            break;
        default:
            Assert(FALSE);
        }
    }

    void JavascriptOperators::VarToNativeArray(Var arrayObject,
        JsNativeValueType valueType,
        __in UINT length,
        __in UINT elementSize,
        __out_bcount(length*elementSize) byte* buffer,
        Js::ScriptContext* scriptContext)
    {
        Js::DynamicObject* dynamicObject = DynamicObject::FromVar(arrayObject);
        if (dynamicObject->IsCrossSiteObject() || Js::TaggedInt::IsOverflow(length))
        {
            Js::JavascriptOperators::ObjectToNativeArray(&arrayObject, valueType, length, elementSize, buffer, scriptContext);
        }
        else
        {
            JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(arrayObject);
            switch (Js::JavascriptOperators::GetTypeId(arrayObject))
            {
            case TypeIds_Array:
                Js::JavascriptOperators::ObjectToNativeArray(Js::JavascriptArray::FromVar(arrayObject), valueType, length, elementSize, buffer, scriptContext);
                break;
            case TypeIds_NativeFloatArray:
                Js::JavascriptOperators::ObjectToNativeArray(Js::JavascriptNativeFloatArray::FromVar(arrayObject), valueType, length, elementSize, buffer, scriptContext);
                break;
            case TypeIds_NativeIntArray:
                Js::JavascriptOperators::ObjectToNativeArray(Js::JavascriptNativeIntArray::FromVar(arrayObject), valueType, length, elementSize, buffer, scriptContext);
                break;
                // We can have more specialized template if needed.
            default:
                Js::JavascriptOperators::ObjectToNativeArray(&arrayObject, valueType, length, elementSize, buffer, scriptContext);
            }
        }
    }
} // namespace Js
