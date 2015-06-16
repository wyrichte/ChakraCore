//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    /*static*/
    PropertyId JavascriptArray::specialPropertyIds[] = 
    {
        PropertyIds::length
    };

    BOOL JavascriptArray::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }
        return DynamicObject::DeleteProperty(propertyId, flags);
    }

    BOOL JavascriptArray::HasProperty(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return true;
        }

        ScriptContext* scriptContext = GetScriptContext();
        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return this->HasItem(index);
        }

        return DynamicObject::HasProperty(propertyId);
    }

    BOOL JavascriptArray::IsEnumerable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }
        return DynamicObject::IsEnumerable(propertyId);
    }

    BOOL JavascriptArray::IsConfigurable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }
        return DynamicObject::IsConfigurable(propertyId);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::SetEnumerable(PropertyId propertyId, BOOL value)
    {
        if (propertyId == PropertyIds::length)
        {
            Assert(!value); // Can't change array length enumerable
            return true;
        }

        ScriptContext* scriptContext = this->GetScriptContext();

        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)
                ->SetEnumerable(this, propertyId, value);
        }

        return __super::SetEnumerable(propertyId, value);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::SetWritable(PropertyId propertyId, BOOL value)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        uint32 index;

        bool setLengthNonWritable = (propertyId == PropertyIds::length && !value);
        if (setLengthNonWritable || scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)
                ->SetWritable(this, propertyId, value);
        }

        return __super::SetWritable(propertyId, value);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::SetConfigurable(PropertyId propertyId, BOOL value)
    {
        if (propertyId == PropertyIds::length)
        {
            Assert(!value); // Can't change array length configurable
            return true;
        }

        ScriptContext* scriptContext = this->GetScriptContext();

        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)
                ->SetConfigurable(this, propertyId, value);
        }

        return __super::SetConfigurable(propertyId, value);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::SetAttributes(PropertyId propertyId, PropertyAttributes attributes)
    {
        ScriptContext* scriptContext = this->GetScriptContext();

        // SetAttributes on "length" is not expected. DefineOwnProperty uses SetWritable. If this is
        // changed, we need to handle it here.
        Assert(propertyId != PropertyIds::length);

        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)
                ->SetItemAttributes(this, index, attributes);
        }

        return __super::SetAttributes(propertyId, attributes);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        ScriptContext* scriptContext = this->GetScriptContext();

        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)
                ->SetItemAccessors(this, index, getter, setter);
        }

        return __super::SetAccessors(propertyId, getter, setter, flags);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::SetItemWithAttributes(uint32 index, Var value, PropertyAttributes attributes)
    {
        return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)
            ->SetItemWithAttributes(this, index, value, attributes);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::SetItemAttributes(uint32 index, PropertyAttributes attributes)
    {
        return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)
            ->SetItemAttributes(this, index, attributes);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::SetItemAccessors(uint32 index, Var getter, Var setter)
    {
        return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)
            ->SetItemAccessors(this, index, getter, setter);
    }

    // Check if this objectArray isFrozen.
    BOOL JavascriptArray::IsObjectArrayFrozen()
    {
        // If this is still a JavascriptArray, it's not frozen.
        return false;
    }

    BOOL JavascriptArray::GetEnumerator(Var originalInstance, BOOL enumNonEnumerable, Var* enumerator, ScriptContext* requestContext, bool preferSnapshotSemantics, bool enumSymbols)
    {
        // JavascriptArray does not support accessors, discard originalInstance.
        return JavascriptArray::GetEnumerator(enumNonEnumerable, enumerator, requestContext, preferSnapshotSemantics, enumSymbols);
    }

    BOOL JavascriptArray::GetNonIndexEnumerator(Var* enumerator, ScriptContext* requestContext)
    {
        *enumerator = RecyclerNew(GetScriptContext()->GetRecycler(), JavascriptArrayNonIndexSnapshotEnumerator, this, requestContext, false);
        return true;
    }

    BOOL JavascriptArray::IsItemEnumerable(uint32 index)
    {
        return true;
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::PreventExtensions()
    {
        return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)->PreventExtensions(this);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::Seal()
    {
        return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)->Seal(this);
    }

    //
    // Evolve typeHandlers explicitly so that simple typeHandlers can skip array
    // handling and only check instance objectArray for numeric propertyIds.
    //
    BOOL JavascriptArray::Freeze()
    {
        return GetTypeHandler()->ConvertToTypeWithItemAttributes(this)->Freeze(this);
    }

    BOOL JavascriptArray::GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext)
    {
        if (index == 0)
        {
            *propertyName = requestContext->GetPropertyString(PropertyIds::length);
            return true;
        }
        return false;
    }

    // Returns the number of special non-enumerable properties this type has.
    uint JavascriptArray::GetSpecialPropertyCount() const
    {
        return _countof(specialPropertyIds);
    }

    // Returns the list of special non-enumerable properties for the type.
    PropertyId* JavascriptArray::GetSpecialPropertyIds() const
    {
        return specialPropertyIds;
    }
    
    BOOL JavascriptArray::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return JavascriptArray::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }
    
    BOOL JavascriptArray::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (GetPropertyBuiltIns(propertyId, value))
        {
            return true;
        }

        ScriptContext* scriptContext = GetScriptContext();
        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            return this->GetItem(this, index, value, scriptContext);
        }

        return DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptArray::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        AssertMsg(!PropertyRecord::IsPropertyNameNumeric(propertyNameString->GetString(), propertyNameString->GetLength()),
            "Numeric property names should have been converted to uint or PropertyRecord*");

        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && GetPropertyBuiltIns(propertyRecord->GetPropertyId(), value))
        {
            return true;
        }

        return DynamicObject::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
    }

    BOOL JavascriptArray::GetPropertyBuiltIns(PropertyId propertyId, Var* value)
    {
        //
        // length being accessed. Return array length
        //
        if (propertyId == PropertyIds::length)
        {
            *value = JavascriptNumber::ToVar(this->GetLength(), GetScriptContext());
            return true;
        }

        return false;
    }

    BOOL JavascriptArray::HasItem(uint32 index)
    {
        Var value;
        return this->DirectGetItemAt<Var>(index, &value);
    }

    BOOL JavascriptArray::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return this->DirectGetItemAt<Var>(index, value);
    }

    BOOL JavascriptArray::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return this->DirectGetItemAt<Var>(index, value);
    }

    BOOL JavascriptArray::DirectGetVarItemAt(uint32 index, Var *value, ScriptContext *requestContext)
    {
        return this->DirectGetItemAt<Var>(index, value);
    }

    BOOL JavascriptNativeIntArray::HasItem(uint32 index)
    {
        int32 value;
        return this->DirectGetItemAt<int32>(index, &value);
    }

    BOOL JavascriptNativeFloatArray::HasItem(uint32 index)
    {
        double dvalue;
        return this->DirectGetItemAt<double>(index, &dvalue);
    }

    BOOL JavascriptNativeIntArray::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(this);
        return JavascriptNativeIntArray::DirectGetVarItemAt(index, value, requestContext);
    }

    BOOL JavascriptNativeIntArray::DirectGetVarItemAt(uint32 index, Var *value, ScriptContext *requestContext)
    {
        int32 intvalue;
        if (!this->DirectGetItemAt<int32>(index, &intvalue))
        {
            return FALSE;
        }
        *value = JavascriptNumber::ToVar(intvalue, requestContext);
        return TRUE;
    }

    BOOL JavascriptNativeIntArray::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return JavascriptNativeIntArray::GetItem(originalInstance, index, value, requestContext);
    }

    BOOL JavascriptNativeFloatArray::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return JavascriptNativeFloatArray::DirectGetVarItemAt(index, value, requestContext);
    }

    BOOL JavascriptNativeFloatArray::DirectGetVarItemAt(uint32 index, Var *value, ScriptContext *requestContext)
    {
        double dvalue;
        int32 ivalue;
        if (!this->DirectGetItemAt<double>(index, &dvalue))
        {
            return FALSE;
        }
        if (*(uint64*)&dvalue == 0ull)
        {
            *value = TaggedInt::ToVarUnchecked(0);
        }
        else if (JavascriptNumber::TryGetInt32Value(dvalue, &ivalue) && !TaggedInt::IsOverflow(ivalue))
        {
            *value = TaggedInt::ToVarUnchecked(ivalue);
        }
        else
        {
            *value = JavascriptNumber::ToVarWithCheck(dvalue, requestContext);
        }
        return TRUE;
    }

    BOOL JavascriptNativeFloatArray::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return JavascriptNativeFloatArray::GetItem(originalInstance, index, value, requestContext);
    }

    BOOL JavascriptArray::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        uint32 indexValue;
        if (propertyId == PropertyIds::length)
        {
            return this->SetLength(value);
        }
        else if (GetScriptContext()->IsNumericPropertyId(propertyId, &indexValue))
        {
            // Call this or subclass method
            return SetItem(indexValue, value, flags);
        }
        else
        {
            return DynamicObject::SetProperty(propertyId, value, flags, info);
        }
    }

    BOOL JavascriptArray::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        AssertMsg(!PropertyRecord::IsPropertyNameNumeric(propertyNameString->GetString(), propertyNameString->GetLength()),
            "Numeric property names should have been converted to uint or PropertyRecord*");

        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && propertyRecord->GetPropertyId() == PropertyIds::length)
        {
            return this->SetLength(value);
        }

        return DynamicObject::SetProperty(propertyNameString, value, flags, info);
    }

    BOOL JavascriptArray::SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        ScriptContext* scriptContext = GetScriptContext();

        if (propertyId == PropertyIds::length)
        {
            Assert(attributes == PropertyWritable);
            Assert(IsWritable(propertyId) && !IsConfigurable(propertyId) && !IsEnumerable(propertyId));
            return this->SetLength(value);
        }

        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            // Call this or subclass method
            return SetItemWithAttributes(index, value, attributes);
        }

        return __super::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
    }

    BOOL JavascriptArray::SetItem(uint32 index, Var value, PropertyOperationFlags flags)
    {
        this->DirectSetItemAt(index, value);
        return true;
    }

    BOOL JavascriptNativeIntArray::SetItem(uint32 index, Var value, PropertyOperationFlags flags)
    {
        int32 iValue;
        double dValue;
        JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Var>(this);
        TypeId typeId = this->TrySetNativeIntArrayItem(value, &iValue, &dValue);
        if (typeId == TypeIds_NativeIntArray)
        {
            this->SetItem(index, iValue);
        }
        else if (typeId == TypeIds_NativeFloatArray)
        {
            reinterpret_cast<JavascriptNativeFloatArray*>(this)->DirectSetItemAt<double>(index, dValue);
        }
        else
        {
            this->DirectSetItemAt<Var>(index, value);
        }

        return TRUE;
    }

    TypeId JavascriptNativeIntArray::TrySetNativeIntArrayItem(Var value, int32 *iValue, double *dValue)
    {
        if (TaggedInt::Is(value))
        {
            int32 i = TaggedInt::ToInt32(value);
            if (i != JavascriptNativeIntArray::MissingItem)
            {
                *iValue = i; 
                return TypeIds_NativeIntArray;
            }
        }
        if (JavascriptNumber::Is_NoTaggedIntCheck(value))
        {
            bool isInt32;
            int32 i;
            double d = JavascriptNumber::GetValue(value);
            if (JavascriptNumber::TryGetInt32OrUInt32Value(d, &i, &isInt32))
            {
                if (isInt32 && i != JavascriptNativeIntArray::MissingItem)
                {
                    *iValue = i;
                    return TypeIds_NativeIntArray;
                }
            }
            else
            {
                *dValue = d;
                JavascriptNativeIntArray::ToNativeFloatArray(this);
                return TypeIds_NativeFloatArray;
            }
        }

        JavascriptNativeIntArray::ToVarArray(this);
        return TypeIds_Array;
    }

    BOOL JavascriptNativeIntArray::SetItem(uint32 index, int32 iValue)
    {
        if (iValue == JavascriptNativeIntArray::MissingItem)
        {
            JavascriptArray *varArr = JavascriptNativeIntArray::ToVarArray(this);
            varArr->DirectSetItemAt(index, JavascriptNumber::ToVar(iValue, GetScriptContext()));
            return TRUE;
        }

        this->DirectSetItemAt(index, iValue);
        return TRUE;
    }

    BOOL JavascriptNativeFloatArray::SetItem(uint32 index, Var value, PropertyOperationFlags flags)
    {
        double dValue;
        TypeId typeId = this->TrySetNativeFloatArrayItem(value, &dValue);
        if (typeId == TypeIds_NativeFloatArray)
        {
            this->SetItem(index, dValue);
        }
        else
        {
            this->DirectSetItemAt(index, value);
        }
        return TRUE;    
    }

    TypeId JavascriptNativeFloatArray::TrySetNativeFloatArrayItem(Var value, double *dValue)
    {
        if (TaggedInt::Is(value))
        {
            *dValue = (double)TaggedInt::ToInt32(value);
            return TypeIds_NativeFloatArray;
        }
        else if (JavascriptNumber::Is_NoTaggedIntCheck(value))
        {
            *dValue = JavascriptNumber::GetValue(value);
            return TypeIds_NativeFloatArray;
        }

        JavascriptNativeFloatArray::ToVarArray(this);
        return TypeIds_Array;
    }

    BOOL JavascriptNativeFloatArray::SetItem(uint32 index, double dValue)
    {
        if (*(uint64*)&dValue == *(uint64*)&JavascriptNativeFloatArray::MissingItem)
        {
            JavascriptArray *varArr = JavascriptNativeFloatArray::ToVarArray(this);
            varArr->DirectSetItemAt(index, JavascriptNumber::ToVarNoCheck(dValue, GetScriptContext()));
            return TRUE;
        }

        this->DirectSetItemAt<double>(index, dValue);
        return TRUE;
    }

    BOOL JavascriptArray::DeleteItem(uint32 index, PropertyOperationFlags flags)
    {
        return this->DirectDeleteItemAt<Var>(index);
    }

    BOOL JavascriptNativeIntArray::DeleteItem(uint32 index, PropertyOperationFlags flags)
    {
        return this->DirectDeleteItemAt<int32>(index);
    }

    BOOL JavascriptNativeFloatArray::DeleteItem(uint32 index, PropertyOperationFlags flags)
    {
        return this->DirectDeleteItemAt<double>(index);
    }

    BOOL JavascriptArray::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics, bool enumSymbols)
    {
        if (preferSnapshotSemantics)
        {
            *enumerator = RecyclerNew(GetRecycler(), JavascriptArraySnapshotEnumerator, this, requestContext, enumNonEnumerable, enumSymbols);
        }
        else
        {
            *enumerator = RecyclerNew(GetRecycler(), JavascriptArrayEnumerator, this, requestContext, enumNonEnumerable, enumSymbols);
        }

        return true;
    }

    BOOL JavascriptArray::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->Append(L'[');

        if (this->length < 10)
        {
            BEGIN_JS_RUNTIME_CALL(requestContext);
            {
                ENTER_PINNED_SCOPE(JavascriptString, valueStr);
                valueStr = JavascriptArray::JoinHelper(this, GetLibrary()->GetCommaDisplayString(), requestContext);
                stringBuilder->Append(valueStr->GetString(), valueStr->GetLength());
                LEAVE_PINNED_SCOPE();
            }
            END_JS_RUNTIME_CALL(requestContext);
        }
        else
        {
            stringBuilder->AppendCppLiteral(L"...");
        }

        stringBuilder->Append(L']');

        return TRUE;
    }

    BOOL JavascriptArray::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Object, (Array)");
        return TRUE;
    }
}
