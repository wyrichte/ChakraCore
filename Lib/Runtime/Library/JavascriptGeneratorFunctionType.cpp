//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
namespace Js
{
    BOOL JavascriptGeneratorFunction::HasProperty(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length ||
            propertyId == PropertyIds::caller || 
            propertyId == PropertyIds::arguments)
        {
            return true;
        }

        return JavascriptFunction::HasProperty(propertyId);
    }

    BOOL JavascriptGeneratorFunction::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result;
        if (GetPropertyBuiltIns(originalInstance, propertyId, value, info, requestContext, &result))
        {
            return result;
        }

        return JavascriptFunction::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptGeneratorFunction::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
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

    bool JavascriptGeneratorFunction::GetPropertyBuiltIns(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext, BOOL* result)
    {
        if (propertyId == PropertyIds::length)
        {   
            // Get the "length" property of the private ScriptFunction
            int len = 0;
            Var varLength;
            if (scriptFunction->GetProperty(scriptFunction, PropertyIds::length, &varLength, NULL, requestContext))
            {
                len = JavascriptConversion::ToInt32(varLength, requestContext);
            }

            *value = JavascriptNumber::ToVar(len, requestContext);
            *result = true;
            return true;
        }

        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // TODO: Better error message? E.g. "'arguments' and 'caller' not accessible on generator functions"?
            // TODO: Different error message in strict mode?
            JavascriptError::ThrowTypeError(this->GetScriptContext(), VBSERR_ActionNotSupported);
            return true;
        }

        return false;
    }

    BOOL JavascriptGeneratorFunction::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) 
    {
        return JavascriptGeneratorFunction::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptGeneratorFunction::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        BOOL result;
        if (SetPropertyBuiltIns(propertyId, value, flags, info, &result))
        {
            return result;
        }

        return JavascriptFunction::SetProperty(propertyId, value, flags, info);
    }

    BOOL JavascriptGeneratorFunction::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
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

    bool JavascriptGeneratorFunction::SetPropertyBuiltIns(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info, BOOL* result)
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
            
            *result = false;
            return true;
        }

        return false;
    }

    BOOL JavascriptGeneratorFunction::GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext)
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

    DescriptorFlags JavascriptGeneratorFunction::GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            *setterValue = requestContext->GetLibrary()->GetThrowTypeErrorAccessorFunction();
            return DescriptorFlags::Accessor;
        }

        return DynamicObject::GetSetter(propertyId, setterValue, info, requestContext);
    }

    DescriptorFlags JavascriptGeneratorFunction::GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
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

    BOOL JavascriptGeneratorFunction::InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return SetProperty(propertyId, value, PropertyOperation_None, info);
    }

    BOOL JavascriptGeneratorFunction::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
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

    BOOL JavascriptGeneratorFunction::IsWritable(PropertyId propertyId)
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

    BOOL JavascriptGeneratorFunction::IsEnumerable(PropertyId propertyId)
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

    BOOL JavascriptGeneratorFunction::HasInstance(Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache)
    {
        return this->scriptFunction->HasInstance(instance, scriptContext, inlineCache);
    }
} // namespace Js