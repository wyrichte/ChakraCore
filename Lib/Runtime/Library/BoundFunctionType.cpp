//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
namespace Js
{
    BOOL BoundFunction::HasProperty(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length ||
            propertyId == PropertyIds::caller || 
            propertyId == PropertyIds::arguments)
        {
            return true;
        }

        return JavascriptFunction::HasProperty(propertyId);
    }

    BOOL BoundFunction::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result;
        if (GetPropertyBuiltIns(originalInstance, propertyId, value, info, requestContext, &result))
        {
            return result;
        }

        return JavascriptFunction::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL BoundFunction::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result;
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && GetPropertyBuiltIns(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext, &result))
        {
            return result;
        }

        return JavascriptFunction::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
    }

    bool BoundFunction::GetPropertyBuiltIns(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext, BOOL* result)
    {
        if (propertyId == PropertyIds::length)
        {   
            // Get the "length" property of the underlying target function
            int len = 0;
            Var varLength;
            if (targetFunction->GetProperty(targetFunction, PropertyIds::length, &varLength, NULL, requestContext))
            {
                len = JavascriptConversion::ToInt32(varLength, requestContext);
            }

            // Reduce by number of bound args
            len = len - this->count;
            len = max(len, 0);

            *value = JavascriptNumber::ToVar(len, requestContext);
            *result = true;
            return true;
        }

        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            JavascriptError::ThrowTypeError(this->GetScriptContext(), VBSERR_ActionNotSupported);
        }

        return false;
    }

    BOOL BoundFunction::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) 
    {
        return BoundFunction::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL BoundFunction::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        BOOL result;
        if (SetPropertyBuiltIns(propertyId, value, flags, info, &result))
        {
            return result;
        }

        return JavascriptFunction::SetProperty(propertyId, value, flags, info);
    }

    BOOL BoundFunction::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        BOOL result;
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && SetPropertyBuiltIns(propertyRecord->GetPropertyId(), value, flags, info, &result))
        {
            return result;
        }

        return JavascriptFunction::SetProperty(propertyNameString, value, flags, info);
    }

    bool BoundFunction::SetPropertyBuiltIns(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info, BOOL* result)
    {
        if (propertyId == PropertyIds::length)
        {
            JavascriptError::ThrowCantAssignIfStrictMode(flags, this->GetScriptContext());

            *result = false;
            return true;
        }
        
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            JavascriptError::ThrowTypeError(this->GetScriptContext(), VBSERR_ActionNotSupported);           
        }

        return false;
    }

    BOOL BoundFunction::GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext)
    {
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            *setter = *getter = requestContext->GetLibrary()->GetThrowTypeErrorAccessorFunction();
            return true;
        }

        // JavascriptFunction has special case for caller and arguments
        // TODO: Remove this, once that is removed
        return DynamicObject::GetAccessors(propertyId, getter, setter, requestContext);
    }

    DescriptorFlags BoundFunction::GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            *setterValue = requestContext->GetLibrary()->GetThrowTypeErrorAccessorFunction();
            return DescriptorFlags::Accessor;
        }

        return DynamicObject::GetSetter(propertyId, setterValue, info, requestContext);
    }

    DescriptorFlags BoundFunction::GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null)
        {
            PropertyId propertyId = propertyRecord->GetPropertyId();

            if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
            {
                *setterValue = requestContext->GetLibrary()->GetThrowTypeErrorAccessorFunction();
                return DescriptorFlags::Accessor;
            }
        }

        return DynamicObject::GetSetter(propertyNameString, setterValue, info, requestContext);
    }

    BOOL BoundFunction::InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return SetProperty(propertyId, value, PropertyOperation_None, info);
    }

    BOOL BoundFunction::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }

        // JavascriptFunction has special case for caller and arguments
        // TODO: Remove this, once that is removed
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            return false;
        }

        return JavascriptFunction::DeleteProperty(propertyId, flags);
    }

    BOOL BoundFunction::IsWritable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }

        // JavascriptFunction has special case for caller and arguments
        // TODO: Remove this, once that is removed
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            return false;
        }

        return JavascriptFunction::IsWritable(propertyId);
    }

    BOOL BoundFunction::IsConfigurable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }

        // JavascriptFunction has special case for caller and arguments
        // TODO: Remove this, once that is removed
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            return false;
        }

        return JavascriptFunction::IsConfigurable(propertyId);
    }

    BOOL BoundFunction::IsEnumerable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }

        // JavascriptFunction has special case for caller and arguments
        // TODO: Remove this, once that is removed
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            return false;
        }

        return JavascriptFunction::IsEnumerable(propertyId);
    }

    BOOL BoundFunction::HasInstance(Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache)
    {
        return this->targetFunction->HasInstance(instance, scriptContext, inlineCache);
    }
} // namespace Js
