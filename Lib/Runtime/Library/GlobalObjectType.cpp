//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    BOOL GlobalObject::HasProperty(PropertyId propertyId)
    {
        return DynamicObject::HasProperty(propertyId) ||
            (this->directHostObject && JavascriptOperators::HasProperty(this->directHostObject, propertyId)) ||
            (this->hostObject && JavascriptOperators::HasProperty(this->hostObject, propertyId));
    }

    BOOL GlobalObject::HasRootProperty(PropertyId propertyId)
    {
        return __super::HasRootProperty(propertyId) ||
            (this->directHostObject && JavascriptOperators::HasProperty(this->directHostObject, propertyId)) ||
            (this->hostObject && JavascriptOperators::HasProperty(this->hostObject, propertyId));
    }

    BOOL GlobalObject::HasOwnProperty(PropertyId propertyId)
    {
        return DynamicObject::HasProperty(propertyId) ||
            (this->directHostObject && this->directHostObject->HasProperty(propertyId));
    }

    BOOL GlobalObject::HasOwnPropertyNoHostObject(PropertyId propertyId)
    {
        return DynamicObject::HasProperty(propertyId);
    }

    BOOL GlobalObject::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext))
        {
            return TRUE;
        }
        return (this->directHostObject && JavascriptOperators::GetProperty(this->directHostObject, propertyId, value, requestContext, info)) ||
            (this->hostObject && JavascriptOperators::GetProperty(this->hostObject, propertyId, value, requestContext, info));
    }

    BOOL GlobalObject::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return GlobalObject::GetProperty(originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext);
    }

    BOOL GlobalObject::GetRootProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (__super::GetRootProperty(originalInstance, propertyId, value, info, requestContext))
        {
            return TRUE;
        }
        return (this->directHostObject && JavascriptOperators::GetProperty(this->directHostObject, propertyId, value, requestContext, info)) ||
            (this->hostObject && JavascriptOperators::GetProperty(this->hostObject, propertyId, value, requestContext, info));
    }

    BOOL GlobalObject::GetAccessors(PropertyId propertyId, Var* getter, Var* setter, ScriptContext * requestContext)
    {
        if (DynamicObject::GetAccessors(propertyId, getter, setter, requestContext))
        {
            return TRUE;
        }
        if (this->directHostObject)
        {
            return this->directHostObject->GetAccessors(propertyId, getter, setter, requestContext);
        }
        else if (this->hostObject)
        {
            return this->hostObject->GetAccessors(propertyId, getter, setter, requestContext);
        }
        return FALSE;
    }

    BOOL GlobalObject::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info,
        ScriptContext* requestContext)
    {
        if (DynamicObject::GetPropertyReference(originalInstance, propertyId, value, info, requestContext))
        {
            return true;
        }
        return (this->directHostObject && JavascriptOperators::GetPropertyReference(this->directHostObject, propertyId, value, requestContext, info)) ||
            (this->hostObject && JavascriptOperators::GetPropertyReference(this->hostObject, propertyId, value, requestContext, info));
    }

    BOOL GlobalObject::GetRootPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info,
        ScriptContext* requestContext)
    {
        if (__super::GetRootPropertyReference(originalInstance, propertyId, value, info, requestContext))
        {
            return true;
        }
        return (this->directHostObject && JavascriptOperators::GetPropertyReference(this->directHostObject, propertyId, value, requestContext, info)) ||
            (this->hostObject && JavascriptOperators::GetPropertyReference(this->hostObject, propertyId, value, requestContext, info));
    }

    BOOL GlobalObject::InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        // var x = 10; variables declared with "var" at global scope
        // These cannot be deleted
        // In ES5 they are enumerable.

        PropertyAttributes attributes = PropertyWritable | PropertyEnumerable | PropertyDeclaredGlobal;
        flags = static_cast<PropertyOperationFlags>(flags | PropertyOperation_ThrowIfNotExtensible);
        return DynamicObject::SetPropertyWithAttributes(propertyId, value, attributes, info, flags);
    }

    BOOL GlobalObject::InitPropertyScoped(PropertyId propertyId, Var value)
    {
        // var x = 10; variables declared with "var" inside "eval"
        // These CAN be deleted
        // in ES5 they are enumerable.
        //
        PropertyAttributes attributes = PropertyDynamicTypeDefaults | PropertyDeclaredGlobal;
        return DynamicObject::SetPropertyWithAttributes(propertyId, value, attributes, NULL, PropertyOperation_ThrowIfNotExtensible);
    }

    BOOL GlobalObject::InitFuncScoped(PropertyId propertyId, Var value)
    {
        // Var binding of functions declared in eval are elided when conflicting
        // with global scope let/const variables, so do not actually set the
        // property if it exists and is a let/const variable.
        bool noRedecl = false;
        if (!GetTypeHandler()->HasRootProperty(this, propertyId, &noRedecl) || !noRedecl)
        {
            //
            // var x = 10; variables declared with "var" inside "eval"
            // These CAN be deleted
            // in ES5 they are enumerable.

            PropertyAttributes attributes= PropertyDynamicTypeDefaults;

            DynamicObject::SetPropertyWithAttributes(propertyId, value, attributes, NULL, PropertyOperation_ThrowIfNotExtensible);
        }
        return true;
    }

    BOOL GlobalObject::SetExistingProperty(PropertyId propertyId, Var value, PropertyValueInfo* info, BOOL *setAttempted)
    {
        BOOL hasOwnProperty = DynamicObject::HasProperty(propertyId);
        BOOL hasProperty = JavascriptOperators::HasProperty(this->GetPrototype(), propertyId);
        *setAttempted = TRUE;

        if (this->directHostObject &&
            !hasOwnProperty &&
            !hasProperty &&
            this->directHostObject->HasProperty(propertyId))
        {
            // directHostObject->HasProperty returns true for things in global collections, however, they are not able to be set.
            // When we call SetProperty we'll return FALSE which means fall back to GlobalObject::SetProperty and shadow our collection
            // object rather than failing the set altogether. See bug Windows Out Of Band Releases 1144780 and linked bugs for details.
            if ( this->directHostObject->SetProperty(propertyId, value, PropertyOperation_None, info) )
            {
                return TRUE;
            }
        }
        else
        if (this->hostObject &&
            // Consider to revert to the commented line and ignore the prototype chain when direct host is on by default in IE9 mode
            !hasOwnProperty &&
            !hasProperty &&
            this->hostObject->HasProperty(propertyId))
        {
            return this->hostObject->SetProperty(propertyId, value, PropertyOperation_None, NULL);
        }

        if (hasOwnProperty || hasProperty)
        {
            return DynamicObject::SetProperty(propertyId, value, PropertyOperation_None, info);
        }

        *setAttempted = FALSE;
        return FALSE;
    }

    BOOL GlobalObject::SetExistingRootProperty(PropertyId propertyId, Var value, PropertyValueInfo* info, BOOL *setAttempted)
    {
        BOOL hasOwnProperty = __super::HasRootProperty(propertyId);
        BOOL hasProperty = JavascriptOperators::HasProperty(this->GetPrototype(), propertyId);
        *setAttempted = TRUE;

        if (this->directHostObject &&
            !hasOwnProperty &&
            !hasProperty &&
            this->directHostObject->HasProperty(propertyId))
        {
            // directHostObject->HasProperty returns true for things in global collections, however, they are not able to be set.
            // When we call SetProperty we'll return FALSE which means fall back to GlobalObject::SetProperty and shadow our collection
            // object rather than failing the set altogether. See bug Windows Out Of Band Releases 1144780 and linked bugs for details.
            if ( this->directHostObject->SetProperty(propertyId, value, PropertyOperation_None, info) )
            {
                return TRUE;
            }
        }
        else
        if (this->hostObject &&
            // Consider to revert to the commented line and ignore the prototype chain when direct host is on by default in IE9 mode
            !hasOwnProperty &&
            !hasProperty &&
            this->hostObject->HasProperty(propertyId))
        {
            return this->hostObject->SetProperty(propertyId, value, PropertyOperation_None, NULL);
        }

        if (hasOwnProperty || hasProperty)
        {
            return __super::SetRootProperty(propertyId, value, PropertyOperation_None, info);
        }

        *setAttempted = FALSE;
        return FALSE;
    }

    BOOL GlobalObject::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        Assert(!(flags & PropertyOperation_Root));

        BOOL setAttempted = TRUE;
        if (this->SetExistingProperty(propertyId, value, info, &setAttempted))
        {
            return TRUE;
        }

        //
        // Set was attempted. But the set operation returned false.
        // This happens, when the property is read only.
        // In those scenarios, we should be setting the property with default attributes
        //
        if (setAttempted)
        {
            return FALSE;
        }

        // Windows 8 430092: When we set a new property on globalObject there may be stale inline caches holding onto directHostObject->prototype
        // properties of the same name. So we need to clear proto caches in this sceanrio. But check for same property in directHostObject->prototype
        // chain is expensive (call to DOM) compared to just invalidating the cache.
        // If this blind invalidation is expensive in any scenario then we need to revisit this.
        // Another solution proposed was not to cache any of the properties of window->directHostObject->prototype.
        // if ((this->directHostObject && JavascriptOperators::HasProperty(this->directHostObject->GetPrototype(), propertyId)) ||
        //    (this->hostObject && JavascriptOperators::HasProperty(this->hostObject->GetPrototype(), propertyId)))

        this->GetScriptContext()->InvalidateProtoCaches(propertyId);

        //
        // x = 10; implicit/phantom variable at global scope
        // These CAN be deleted
        // In ES5 they are enumerable.

        PropertyAttributes attributes = PropertyDynamicTypeDefaults;
        return DynamicObject::SetPropertyWithAttributes(propertyId, value, attributes, info);
    }

    BOOL GlobalObject::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return GlobalObject::SetProperty(propertyRecord->GetPropertyId(), value, flags, info);
    }

    BOOL GlobalObject::SetRootProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        Assert(flags & PropertyOperation_Root);

        BOOL setAttempted = TRUE;
        if (this->SetExistingRootProperty(propertyId, value, info, &setAttempted))
        {
            return TRUE;
        }

        //
        // Set was attempted. But the set operation returned false.
        // This happens, when the property is read only.
        // In those scenarios, we should be setting the property with default attributes
        //
        if (setAttempted)
        {
            return FALSE;
        }

        if (flags & PropertyOperation_StrictMode)
        {
            if (!GetScriptContext()->GetThreadContext()->RecordImplicitException())
            {
                return FALSE;
            }
            JavascriptError::ThrowReferenceError(GetScriptContext(), JSERR_RefErrorUndefVariable);
        }

        // Windows 8 430092: When we set a new property on globalObject there may be stale inline caches holding onto directHostObject->prototype
        // properties of the same name. So we need to clear proto caches in this sceanrio. But check for same property in directHostObject->prototype
        // chain is expensive (call to DOM) compared to just invalidating the cache.
        // If this blind invalidation is expensive in any scenario then we need to revisit this.
        // Another solution proposed was not to cache any of the properties of window->directHostObject->prototype.
        // if ((this->directHostObject && JavascriptOperators::HasProperty(this->directHostObject->GetPrototype(), propertyId)) ||
        //    (this->hostObject && JavascriptOperators::HasProperty(this->hostObject->GetPrototype(), propertyId)))

        this->GetScriptContext()->InvalidateProtoCaches(propertyId);

        return __super::SetRootProperty(propertyId, value, flags, info);
    }

    BOOL GlobalObject::SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
        if (DynamicObject::SetAccessors(propertyId, getter, setter, flags))
        {
            return TRUE;
        }
        if (this->directHostObject)
        {
            return this->directHostObject->SetAccessors(propertyId, getter, setter, flags);
        }
        if (this->hostObject)
        {
            return this->hostObject->SetAccessors(propertyId, getter, setter, flags);
        }
        return TRUE;
    }

    DescriptorFlags GlobalObject::GetSetter(PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        DescriptorFlags flags = DynamicObject::GetSetter(propertyId, setterValue, info, requestContext);
        if (flags == None)
        {
            if (this->directHostObject)
            {
                // We need to look up the prototype chain here.
                JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(this->directHostObject, propertyId, setterValue, &flags, info, requestContext);
            }
            else if (this->hostObject)
            {
                JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(this->hostObject, propertyId, setterValue, &flags, info, requestContext);
            }
        }

        return flags;
    }

    DescriptorFlags GlobalObject::GetSetter(JavascriptString* propertyNameString, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return GlobalObject::GetSetter(propertyRecord->GetPropertyId(), setterValue, info, requestContext);
    }

    DescriptorFlags GlobalObject::GetRootSetter(PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        DescriptorFlags flags = __super::GetRootSetter(propertyId, setterValue, info, requestContext);
        if (flags == None)
        {
            if (this->directHostObject)
            {
                // We need to look up the prototype chain here.
                JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(this->directHostObject, propertyId, setterValue, &flags, info, requestContext);
            }
            else if (this->hostObject)
            {
                JavascriptOperators::CheckPrototypesForAccessorOrNonWritableProperty(this->hostObject, propertyId, setterValue, &flags, info, requestContext);
            }
        }

        return flags;
    }

    DescriptorFlags GlobalObject::GetItemSetter(uint32 index, Var* setterValue, ScriptContext* requestContext)
    {
        DescriptorFlags flags = DynamicObject::GetItemSetter(index, setterValue, requestContext);
        if (flags == None)
        {
            if (this->directHostObject)
            {
                // We need to look up the prototype chain here.
                JavascriptOperators::CheckPrototypesForAccessorOrNonWritableItem(this->directHostObject, index, setterValue, &flags, requestContext);
            }
            else if (this->hostObject)
            {
                JavascriptOperators::CheckPrototypesForAccessorOrNonWritableItem(this->hostObject, index, setterValue, &flags, requestContext);
            }
        }

        return flags;
    }

    BOOL GlobalObject::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        if (__super::HasProperty(propertyId))
        {
            return __super::DeleteProperty(propertyId, flags);
        }
        else if (this->directHostObject && this->directHostObject->HasProperty( propertyId))
        {
            return this->directHostObject->DeleteProperty(propertyId, flags);
        }
        else if (this->hostObject && this->hostObject->HasProperty(propertyId))
        {
            return this->hostObject->DeleteProperty(propertyId, flags);
        }

        // Non existant property
        return TRUE;
    }

    BOOL GlobalObject::DeleteRootProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        if (__super::HasRootProperty(propertyId))
        {
            return __super::DeleteRootProperty(propertyId, flags);
        }
        else if (this->directHostObject && this->directHostObject->HasProperty( propertyId))
        {
            return this->directHostObject->DeleteProperty(propertyId, flags);
        }
        else if (this->hostObject && this->hostObject->HasProperty(propertyId))
        {
            return this->hostObject->DeleteProperty(propertyId, flags);
        }

        // Non existant property
        return TRUE;
    }

    BOOL GlobalObject::HasItem(uint32 index)
    {
        return DynamicObject::HasItem(index)
            || (this->directHostObject && JavascriptOperators::HasItem(this->directHostObject, index))
            || (this->hostObject && JavascriptOperators::HasItem(this->hostObject, index));
    }

    BOOL GlobalObject::HasOwnItem(uint32 index)
    {
        return DynamicObject::HasItem(index)
            ||  (this->directHostObject && this->directHostObject->HasItem(index));
    }

    BOOL GlobalObject::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        if (DynamicObject::GetItemReference(originalInstance, index, value, requestContext))
        {
            return TRUE;
        }
        return (this->directHostObject && this->directHostObject->GetItemReference(originalInstance, index, value, requestContext)) ||
            (this->hostObject && this->hostObject->GetItemReference(originalInstance, index, value, requestContext));
    }

    BOOL GlobalObject::SetItem(uint32 index, Var value, PropertyOperationFlags flags)
    {
        BOOL result = DynamicObject::SetItem(index, value, flags);
        return result;
    }

    BOOL GlobalObject::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        if (DynamicObject::GetItem(originalInstance, index, value, requestContext))
        {
            return TRUE;
        }

        return (this->directHostObject && this->directHostObject->GetItem(originalInstance, index, value, requestContext)) ||
            (this->hostObject && this->hostObject->GetItem(originalInstance, index, value, requestContext));
    }

    BOOL GlobalObject::DeleteItem(uint32 index, PropertyOperationFlags flags)
    {
        if (DynamicObject::DeleteItem(index, flags))
        {
            return TRUE;
        }

        if (this->directHostObject)
        {
            return this->directHostObject->DeleteItem(index, flags);
        }

        if (this->hostObject)
        {
            return this->hostObject->DeleteItem(index, flags);
        }
        return FALSE;
    }

    BOOL GlobalObject::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"{...}");
        return TRUE;
    }

    BOOL GlobalObject::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Object, (Global)");
        return TRUE;
    }

    BOOL GlobalObject::StrictEquals(Js::Var other, BOOL* value, ScriptContext * requestContext)
    {
        if (this == other)
        {
            *value = true;
            return TRUE;
        }
        else if(this->directHostObject)
        {
            return this->directHostObject->StrictEquals(other, value, requestContext);
        }
        else if(this->hostObject)
        {
            return this->hostObject->StrictEquals(other, value, requestContext);
        }
        return FALSE;
    }

    BOOL GlobalObject::Equals(Js::Var other, BOOL* value, ScriptContext * requestContext)
    {
        if (this == other)
        {
            *value = true;
            return TRUE;
        }
        else if(this->directHostObject)
        {
            return this->directHostObject->Equals(other, value, requestContext);
        }
        else if(this->hostObject)
        {
            return this->hostObject->Equals(other, value, requestContext);
        }

        *value = false;
        return TRUE;
    }
}
