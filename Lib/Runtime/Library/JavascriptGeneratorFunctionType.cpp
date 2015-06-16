//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
namespace Js
{
    BOOL JavascriptGeneratorFunction::HasProperty(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return true;
        }

        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::HasProperty(propertyId);
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

        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext);
        }

        return JavascriptFunction::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptGeneratorFunction::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != nullptr)
        {
            BOOL result;
            if (GetPropertyBuiltIns(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext, &result))
            {
                return result;
            }

            if (propertyRecord->GetPropertyId() == PropertyIds::caller || propertyRecord->GetPropertyId() == PropertyIds::arguments)
            {
                // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
                return DynamicObject::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
            }
        }

        return JavascriptFunction::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
    }

    bool JavascriptGeneratorFunction::GetPropertyBuiltIns(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext, BOOL* result)
    {
        if (propertyId == PropertyIds::length)
        {   
            // Cannot just call the base GetProperty for `length` because we need
            // to get the length from our private ScriptFunction instead of ourself.
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

        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::SetProperty(propertyId, value, flags, info);
        }

        return JavascriptFunction::SetProperty(propertyId, value, flags, info);
    }

    BOOL JavascriptGeneratorFunction::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != nullptr)
        {
            BOOL result;
            if (SetPropertyBuiltIns(propertyRecord->GetPropertyId(), value, flags, info, &result))
            {
                return result;
            }

            if (propertyRecord->GetPropertyId() == PropertyIds::caller || propertyRecord->GetPropertyId() == PropertyIds::arguments)
            {
                // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
                return DynamicObject::SetProperty(propertyNameString, value, flags, info);
            }
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
        
        return false;
    }

    BOOL JavascriptGeneratorFunction::GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext)
    {
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::GetAccessors(propertyId, getter, setter, requestContext);
        }

        return JavascriptFunction::GetAccessors(propertyId, getter, setter, requestContext);
    }

    DescriptorFlags JavascriptGeneratorFunction::GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::GetSetter(propertyId, setterValue, info, requestContext);
        }

        return JavascriptFunction::GetSetter(propertyId, setterValue, info, requestContext);
    }

    DescriptorFlags JavascriptGeneratorFunction::GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != nullptr && (propertyRecord->GetPropertyId() == PropertyIds::caller || propertyRecord->GetPropertyId() == PropertyIds::arguments))
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::GetSetter(propertyNameString, setterValue, info, requestContext);
        }

        return JavascriptFunction::GetSetter(propertyNameString, setterValue, info, requestContext);
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

        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::DeleteProperty(propertyId, flags);
        }

        return JavascriptFunction::DeleteProperty(propertyId, flags);
    }

    BOOL JavascriptGeneratorFunction::IsWritable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }

        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::IsWritable(propertyId);
        }

        return JavascriptFunction::IsWritable(propertyId);
    }

    BOOL JavascriptGeneratorFunction::IsEnumerable(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return false;
        }

        if (propertyId == PropertyIds::caller || propertyId == PropertyIds::arguments)
        {
            // JavascriptFunction has special case for caller and arguments; call DynamicObject:: virtual directly to skip that.
            return DynamicObject::IsEnumerable(propertyId);
        }

        return JavascriptFunction::IsEnumerable(propertyId);
    }

    BOOL JavascriptGeneratorFunction::HasInstance(Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache)
    {
        return this->scriptFunction->HasInstance(instance, scriptContext, inlineCache);
    }
} // namespace Js