//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2009 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    /*static*/
    PropertyId JavascriptPixelArray::specialPropertyIds[] = 
    {
        PropertyIds::length
    };

    BOOL JavascriptPixelArray::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }
        return DynamicObject::DeleteProperty(propertyId, flags);
    }

    BOOL JavascriptPixelArray::HasProperty(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return true;
        }
        
        uint32 index = 0;  
        if (GetScriptContext()->IsNumericPropertyId(propertyId, &index) && (index < this->GetBufferLength()))
        {
            // There is a value in every valid array slot
            return true;
        }
        return DynamicObject::HasProperty(propertyId);
    }

    BOOL JavascriptPixelArray::IsEnumerable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }
                
        return DynamicObject::IsEnumerable(propertyId);
    }

    BOOL JavascriptPixelArray::IsConfigurable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }
        return DynamicObject::IsConfigurable(propertyId);
    }

    BOOL JavascriptPixelArray::GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext)
    {
        if (index == 0)
        {
            *propertyName = requestContext->GetPropertyString(PropertyIds::length);
            return true;
        }
        return false;
    }

    // Returns the number of special non-enumerable properties this type has.
    uint JavascriptPixelArray::GetSpecialPropertyCount() const
    {
        return _countof(specialPropertyIds);
    }

    // Returns the list of special non-enumerable properties for the type.
    PropertyId* JavascriptPixelArray::GetSpecialPropertyIds() const
    {
        return specialPropertyIds;
    }

    BOOL JavascriptPixelArray::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result;
        if (GetPropertyBuiltIns(propertyId, value, &result))
        {
            return result;
        }
        
        uint32 index = 0;
        if (GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {
            *value = this->DirectGetItem(index);
            return true;
        }

        return DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptPixelArray::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        AssertMsg(!PropertyRecord::IsPropertyNameNumeric(propertyNameString->GetString(), propertyNameString->GetLength()),
            "Numeric property names should have been converted to uint or PropertyRecord*");

        BOOL result;
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && GetPropertyBuiltIns(propertyRecord->GetPropertyId(), value, &result))
        {
            return result;
        }

        return DynamicObject::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
    }

    bool JavascriptPixelArray::GetPropertyBuiltIns(PropertyId propertyId, Var* value, BOOL* result)
    {
        //
        // length being accessed. Return array length
        //        
        if (propertyId == PropertyIds::length)
        {
            *value = JavascriptNumber::ToVar(this->GetBufferLength(), GetScriptContext());
            *result = true;
            return true;
        }

        return false;
    }

    BOOL JavascriptPixelArray::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) 
    {
        return JavascriptPixelArray::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptPixelArray::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        *value = DirectGetItem(index);
        return true;
    }   
    
    BOOL JavascriptPixelArray::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        *value = DirectGetItem(index);
        return true;
    }

    BOOL JavascriptPixelArray::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        BOOL result;
        if (SetPropertyBuiltIns(propertyId, &result))
        {
            return result;
        }

        uint32 index;
        if (GetScriptContext()->IsNumericPropertyId(propertyId, &index))
        {            
            this->DirectSetItem((int)index, value);
            return true;
        }

        return DynamicObject::SetProperty(propertyId, value, flags, info);
    }

    BOOL JavascriptPixelArray::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        AssertMsg(!PropertyRecord::IsPropertyNameNumeric(propertyNameString->GetString(), propertyNameString->GetLength()),
            "Numeric property names should have been converted to uint or PropertyRecord*");

        BOOL result;
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && SetPropertyBuiltIns(propertyRecord->GetPropertyId(), &result))
        {            
            return result;
        }

        return DynamicObject::SetProperty(propertyNameString, value, flags, info);
    }

    bool JavascriptPixelArray::SetPropertyBuiltIns(PropertyId propertyId, BOOL* result)
    {
        if (propertyId == PropertyIds::length)
        {            
            // Pixel array is fixed-sized
            *result = false;
            return true;
        }

        return false;
    }

    BOOL JavascriptPixelArray::SetItem(uint32 index, Var value, PropertyOperationFlags flags)
    {
        DirectSetItem(index, value);
        return true;
    }
    
    BOOL JavascriptPixelArray::DeleteItem(uint32 index, PropertyOperationFlags flags)
    {
        // It doesn't make sense to delete items from a BYTE array
        return false;
    }

    BOOL JavascriptPixelArray::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext* scriptContxt, bool preferSnapshotSemantics, bool enumSymbols)
    {
        // Pixel array don't support snapshot semantics

        *enumerator = RecyclerNew(GetScriptContext()->GetRecycler(), JavascriptPixelArrayEnumerator, this);
        return true;
    }

    BOOL JavascriptPixelArray::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"[...]");
        return TRUE;
    }

    BOOL JavascriptPixelArray::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Object, (PixelArray)");
        return TRUE;
    }
}
