//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace Js
{
    BOOL ArgumentsObject::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics, bool enumSymbols)
    {        
        this->GetTypeHandler()->EnsureObjectReady(this);
        *enumerator = RecyclerNew(GetScriptContext()->GetRecycler(), ArgumentsObjectEnumerator, this, requestContext, enumNonEnumerable, enumSymbols);
        return true;
    }

    BOOL ArgumentsObject::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"{...}");
        return TRUE;
    }

    BOOL ArgumentsObject::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Object, (Arguments)");
        return TRUE;
    }


    //---------------------- HeapArgumentsObjectType -------------------------------   

    BOOL HeapArgumentsObject::HasProperty(PropertyId id)
    {
        ScriptContext *scriptContext = GetScriptContext();

        // Try to get a numbered property that maps to an actual argument.
        uint32 index;
        if (scriptContext->IsNumericPropertyId(id, &index) && index < this->HeapArgumentsObject::GetNumberOfArguments())
        {
            return HeapArgumentsObject::HasItem(index);
        }
        
        return DynamicObject::HasProperty(id);
    }

    BOOL HeapArgumentsObject::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {        
        ScriptContext *scriptContext = GetScriptContext();

        // Try to get a numbered property that maps to an actual argument.
        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index) && index < this->HeapArgumentsObject::GetNumberOfArguments())
        {
            if (this->GetItemAt(index, value, requestContext))
            {
                return true;
            }
        }

        if (DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext))
        {
            // Property has been pre-set and not deleted. Use it.
            return true;
        }

        return false;
    }

    BOOL HeapArgumentsObject::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {        

        AssertMsg(!PropertyRecord::IsPropertyNameNumeric(propertyNameString->GetString(), propertyNameString->GetLength()),
            "Numeric property names should have been converted to uint or PropertyRecord*");

        if (DynamicObject::GetProperty(originalInstance, propertyNameString, value, info, requestContext))
        {
            // Property has been pre-set and not deleted. Use it.
            return true;
        }
        
        return false;
    }
    
    BOOL HeapArgumentsObject::GetPropertyReference(Var originalInstance, PropertyId id, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return HeapArgumentsObject::GetProperty(originalInstance, id, value, info, requestContext);
    }

    BOOL HeapArgumentsObject::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        // Try to set a numbered property that maps to an actual argument.
        ScriptContext *scriptContext = GetScriptContext();
        
        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index) && index < this->HeapArgumentsObject::GetNumberOfArguments())
        {
            if (this->SetItemAt(index, value))
            {
                return true;
            }
        }
        
        // TODO: In strict mode, "callee" and "caller" cannot be set.

        // length is also set in the dynamic object
        return DynamicObject::SetProperty(propertyId, value, flags, info);
    }

    BOOL HeapArgumentsObject::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        AssertMsg(!PropertyRecord::IsPropertyNameNumeric(propertyNameString->GetSz(), propertyNameString->GetLength()),
            "Numeric property names should have been converted to uint or PropertyRecord*");
        
        // TODO: In strict mode, "callee" and "caller" cannot be set.

        // length is also set in the dynamic object
        return DynamicObject::SetProperty(propertyNameString, value, flags, info);
    }    

    BOOL HeapArgumentsObject::HasItem(uint32 index)
    {
        if (this->HasItemAt(index))
        {
            return true;
        }
        return DynamicObject::HasItem(index);
    }

    BOOL HeapArgumentsObject::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {        
        if (this->GetItemAt(index, value, requestContext))
        {
            return true;
        }
        return DynamicObject::GetItem(originalInstance, index, value, requestContext);
    }

    BOOL HeapArgumentsObject::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        return HeapArgumentsObject::GetItem(originalInstance, index, value, requestContext);
    }

    BOOL HeapArgumentsObject::SetItem(uint32 index, Var value, PropertyOperationFlags flags)
    {        
        if (this->SetItemAt(index, value))
        {
            return true;
        }
        return DynamicObject::SetItem(index, value, flags);
    }

    BOOL HeapArgumentsObject::DeleteItem(uint32 index, PropertyOperationFlags flags)
    {        
        if (this->DeleteItemAt(index))
        {
            return true;
        }
        return DynamicObject::DeleteItem(index, flags);
    }

    BOOL HeapArgumentsObject::SetConfigurable(PropertyId propertyId, BOOL value)
    {
        // Try to set a numbered property that maps to an actual argument.
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            // From now on, make sure that frame object is ES5HeapArgumentsObject.
            return this->ConvertToES5HeapArgumentsObject()->SetConfigurableForFormal(index, propertyId, value);
        }
        
        // Use 'this' dynamic object.
        // This will use type handler and convert its objectArray to ES5Array is not already converted.
        return __super::SetConfigurable(propertyId, value);
    }

    BOOL HeapArgumentsObject::SetEnumerable(PropertyId propertyId, BOOL value)
    {
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return this->ConvertToES5HeapArgumentsObject()->SetEnumerableForFormal(index, propertyId, value);
        }
        return __super::SetEnumerable(propertyId, value);
    }

    BOOL HeapArgumentsObject::SetWritable(PropertyId propertyId, BOOL value)
    {
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return this->ConvertToES5HeapArgumentsObject()->SetWritableForFormal(index, propertyId, value);
        }
        return __super::SetWritable(propertyId, value);
    }

    BOOL HeapArgumentsObject::SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return this->ConvertToES5HeapArgumentsObject()->SetAccessorsForFormal(index, propertyId, getter, setter, flags);
        }
        return __super::SetAccessors(propertyId, getter, setter, flags);
    }

    BOOL HeapArgumentsObject::SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        // This is called by defineProperty in order to change the value in objectArray.
        // We have to intercept this call because
        // in case when we are connected (objectArray item is not used) we need to update the slot value (which is actually used).
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return this->ConvertToES5HeapArgumentsObject()->SetPropertyWithAttributesForFormal(
                index, propertyId, value, attributes, info, flags, possibleSideEffects);
        }
        return __super::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
    }

    // This disables adding new properties to the object.
    BOOL HeapArgumentsObject::PreventExtensions()
    {
        // It's possible that after call to preventExtensions, the user can change the attributes;
        // In this case if we don't covert to ES5 version now, later we would not be able to add items to objectArray,
        // which would cause not being able to change attributes.
        // So, convert to ES5 now which will make sure the items are there.
        return this->ConvertToES5HeapArgumentsObject()->PreventExtensions();
    }
    
    // This is equivalent to .preventExtensions semantics with addition of setting configurable to false for all properties.
    BOOL HeapArgumentsObject::Seal()
    {
        // Same idea as with PreventExtensions: we have to make sure that items in objectArray for formals 
        // are there before seal, otherwise we will not be able to add them later.
        return this->ConvertToES5HeapArgumentsObject()->Seal();
    }

    // This is equivalent to .seal semantics with addition of setting writable to false for all properties.
    BOOL HeapArgumentsObject::Freeze()
    {
        // Same idea as with PreventExtensions: we have to make sure that items in objectArray for formals 
        // are there before seal, otherwise we will not be able to add them later.
        return this->ConvertToES5HeapArgumentsObject()->Freeze();
    }

    //---------------------- ES5HeapArgumentsObject -------------------------------   

    BOOL ES5HeapArgumentsObject::SetConfigurable(PropertyId propertyId, BOOL value)
    {
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return this->SetConfigurableForFormal(index, propertyId, value);
        }
        return this->DynamicObject::SetConfigurable(propertyId, value);
    }

    BOOL ES5HeapArgumentsObject::SetEnumerable(PropertyId propertyId, BOOL value)
    {
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return this->SetEnumerableForFormal(index, propertyId, value);
        }
        return this->DynamicObject::SetEnumerable(propertyId, value);
    }

    BOOL ES5HeapArgumentsObject::SetWritable(PropertyId propertyId, BOOL value)
    {
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return this->SetWritableForFormal(index, propertyId, value);
        }
        return this->DynamicObject::SetWritable(propertyId, value);
    }

    BOOL ES5HeapArgumentsObject::SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return SetAccessorsForFormal(index, propertyId, getter, setter);
        }
        return this->DynamicObject::SetAccessors(propertyId, getter, setter, flags);
    }

    BOOL ES5HeapArgumentsObject::SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        // This is called by defineProperty in order to change the value in objectArray.
        // We have to intercept this call because
        // in case when we are connected (objectArray item is not used) we need to update the slot value (which is actually used).
        uint32 index;
        if (this->IsFormalArgument(propertyId, &index))
        {
            return this->SetPropertyWithAttributesForFormal(index, propertyId, value, attributes, info, flags, possibleSideEffects);
        }

        return this->DynamicObject::SetPropertyWithAttributes(propertyId, value, attributes, info, flags, possibleSideEffects);
    }

    BOOL ES5HeapArgumentsObject::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics, bool enumSymbols)
    {        
        this->GetTypeHandler()->EnsureObjectReady(this);
        *enumerator = RecyclerNew(GetScriptContext()->GetRecycler(), ES5ArgumentsObjectEnumerator, this, requestContext, enumNonEnumerable, enumSymbols);
        return true;
    }

    BOOL ES5HeapArgumentsObject::PreventExtensions()
    {
        return this->DynamicObject::PreventExtensions();
    }

    BOOL ES5HeapArgumentsObject::Seal()
    {
        return this->DynamicObject::Seal();
    }

    BOOL ES5HeapArgumentsObject::Freeze()
    {
        return this->DynamicObject::Freeze();
    }
};
