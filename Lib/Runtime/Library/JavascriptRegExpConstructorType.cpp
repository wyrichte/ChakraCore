//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
namespace Js
{
    /*static*/
    PropertyId JavascriptRegExpConstructor::specialPropertyIds[] = 
    {
        PropertyIds::$_,
        PropertyIds::$Ampersand,
        PropertyIds::$Plus,
        PropertyIds::$BackTick,
        PropertyIds::$Tick,
        PropertyIds::index,
    };

    PropertyId JavascriptRegExpConstructor::specialEnumPropertyIds[] =
    {
        PropertyIds::$1,
        PropertyIds::$2,
        PropertyIds::$3,
        PropertyIds::$4,
        PropertyIds::$5,
        PropertyIds::$6,
        PropertyIds::$7,
        PropertyIds::$8,
        PropertyIds::$9,
        PropertyIds::input,
        PropertyIds::rightContext,
        PropertyIds::leftContext,
        PropertyIds::lastParen,
        PropertyIds::lastMatch,
    };

    PropertyId JavascriptRegExpConstructor::specialnonEnumPropertyIds[] =
    {
        PropertyIds::$_,
        PropertyIds::$Ampersand,
        PropertyIds::$Plus,
        PropertyIds::$BackTick,
        PropertyIds::$Tick,
        PropertyIds::index,
    };

    BOOL JavascriptRegExpConstructor::HasProperty(PropertyId propertyId)
    {
        switch (propertyId)
        {
        case PropertyIds::lastMatch:
        case PropertyIds::$Ampersand:
        case PropertyIds::lastParen:
        case PropertyIds::$Plus:
        case PropertyIds::leftContext:
        case PropertyIds::$BackTick:
        case PropertyIds::rightContext:
        case PropertyIds::$Tick:
        case PropertyIds::index:
        case PropertyIds::input:
        case PropertyIds::$_:
        case PropertyIds::$1:
        case PropertyIds::$2:
        case PropertyIds::$3:
        case PropertyIds::$4:
        case PropertyIds::$5:
        case PropertyIds::$6:
        case PropertyIds::$7:
        case PropertyIds::$8:
        case PropertyIds::$9:
            return true;
        default:
            return JavascriptFunction::HasProperty(propertyId);
        }
    }

    BOOL JavascriptRegExpConstructor::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return JavascriptRegExpConstructor::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptRegExpConstructor::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result;
        if (GetPropertyBuiltIns(propertyId, value, &result))
        {
            return result;
        }

        return JavascriptFunction::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptRegExpConstructor::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        BOOL result;
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && GetPropertyBuiltIns(propertyRecord->GetPropertyId(), value, &result))
        {
            return result;
        }

        return JavascriptFunction::GetProperty(originalInstance, propertyNameString, value, info, requestContext);
    }

    bool JavascriptRegExpConstructor::GetPropertyBuiltIns(PropertyId propertyId, Var* value, BOOL* result)
    {
        switch (propertyId)
        {
        case PropertyIds::input:
        case PropertyIds::$_:
            this->EnsureValues();
            *value = this->lastInput;
            *result = true;
            return true;
        case PropertyIds::lastMatch:
        case PropertyIds::$Ampersand:
            this->EnsureValues();
            *value = this->captures[0];
            *result = true;
            return true;
        case PropertyIds::lastParen:
        case PropertyIds::$Plus:
            this->EnsureValues();
            *value = this->lastParen;
            *result = true;
            return true;
        case PropertyIds::leftContext:
        case PropertyIds::$BackTick:
            this->EnsureValues();
            *value = this->leftContext;
            *result = true;
            return true;
        case PropertyIds::rightContext:
        case PropertyIds::$Tick:
            this->EnsureValues();
            *value = this->rightContext;
            *result = true;
            return true;
        case PropertyIds::$1:
            this->EnsureValues();
            *value = this->captures[1];
            *result = true;
            return true;
        case PropertyIds::$2:
            this->EnsureValues();
            *value = this->captures[2];
            *result = true;
            return true;
        case PropertyIds::$3:
            this->EnsureValues();
            *value = this->captures[3];
            *result = true;
            return true;
        case PropertyIds::$4:
            this->EnsureValues();
            *value = this->captures[4];
            *result = true;
            return true;
        case PropertyIds::$5:
            this->EnsureValues();
            *value = this->captures[5];
            *result = true;
            return true;
        case PropertyIds::$6:
            this->EnsureValues();
            *value = this->captures[6];
            *result = true;
            return true;
        case PropertyIds::$7:
            this->EnsureValues();
            *value = this->captures[7];
            *result = true;
            return true;
        case PropertyIds::$8:
            this->EnsureValues();
            *value = this->captures[8];
            *result = true;
            return true;
        case PropertyIds::$9:
            this->EnsureValues();
            *value = this->captures[9];
            *result = true;
            return true;
        case PropertyIds::index:
            this->EnsureValues();
            *value = this->index;
            *result = true;
            return true;
        default:
            return false;
        }
    }

    BOOL JavascriptRegExpConstructor::SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {        
        BOOL result;
        if (SetPropertyBuiltIns(propertyId, value, &result))
        {
            return result;
        }

        return JavascriptFunction::SetProperty(propertyId, value, flags, info);
    }

    BOOL JavascriptRegExpConstructor::SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {        
        BOOL result;
        PropertyRecord const * propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && SetPropertyBuiltIns(propertyRecord->GetPropertyId(), value, &result))
        {
            return result;
        }

        return JavascriptFunction::SetProperty(propertyNameString, value, flags, info);
    }

    bool JavascriptRegExpConstructor::SetPropertyBuiltIns(PropertyId propertyId, Var value, BOOL* result)
    {
        switch (propertyId)
        {
        case PropertyIds::input:
        case PropertyIds::$_:
            //TODO: review: although the 'input' property is marked as readonly, it has a set on V5.8. There is no spec on this. 
            EnsureValues(); // The last match info relies on the last input. Use it before it is changed.
            this->lastInput = JavascriptConversion::ToString(value, this->GetScriptContext());
            *result = true;
            return true;
        case PropertyIds::lastMatch:
        case PropertyIds::$Ampersand:
        case PropertyIds::lastParen:
        case PropertyIds::$Plus:
        case PropertyIds::leftContext:
        case PropertyIds::$BackTick:
        case PropertyIds::rightContext:
        case PropertyIds::$Tick:
        case PropertyIds::$1:
        case PropertyIds::$2:
        case PropertyIds::$3:
        case PropertyIds::$4:
        case PropertyIds::$5:
        case PropertyIds::$6:
        case PropertyIds::$7:
        case PropertyIds::$8:
        case PropertyIds::$9:
        case PropertyIds::index:
            *result = false;
            return true;
        default:
            return false;
        }
    }

    BOOL JavascriptRegExpConstructor::InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return SetProperty(propertyId, value, flags, info);
    }

    BOOL JavascriptRegExpConstructor::DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags)
    {
        switch (propertyId)
        {
            // all globals are 'fNoDelete' in V5.8
        case PropertyIds::input:
        case PropertyIds::$_:
        case PropertyIds::lastMatch:
        case PropertyIds::$Ampersand:
        case PropertyIds::lastParen:
        case PropertyIds::$Plus:
        case PropertyIds::leftContext:
        case PropertyIds::$BackTick:
        case PropertyIds::rightContext:
        case PropertyIds::$Tick:
        case PropertyIds::$1:
        case PropertyIds::$2:
        case PropertyIds::$3:
        case PropertyIds::$4:
        case PropertyIds::$5:
        case PropertyIds::$6:
        case PropertyIds::$7:
        case PropertyIds::$8:
        case PropertyIds::$9:
        case PropertyIds::index:
            JavascriptError::ThrowCantDeleteIfStrictMode(flags, GetScriptContext(), GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());            
            return false;

        default:
            return JavascriptFunction::DeleteProperty(propertyId, flags);
        }
    }

    BOOL JavascriptRegExpConstructor::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(JS_DIAG_VALUE_JavascriptRegExpConstructor);
        return TRUE;
    }

    BOOL JavascriptRegExpConstructor::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(JS_DIAG_TYPE_JavascriptRegExpConstructor);
        return TRUE;
    }

    BOOL JavascriptRegExpConstructor::IsEnumerable(PropertyId propertyId)
    {
        switch (propertyId)
        {
        case PropertyIds::input:
        case PropertyIds::$1:
        case PropertyIds::$2:
        case PropertyIds::$3:
        case PropertyIds::$4:
        case PropertyIds::$5:
        case PropertyIds::$6:
        case PropertyIds::$7:
        case PropertyIds::$8:
        case PropertyIds::$9:
        case PropertyIds::leftContext:
        case PropertyIds::rightContext:
        case PropertyIds::lastMatch:
        case PropertyIds::lastParen:
            return true;
        case PropertyIds::$_:
        case PropertyIds::$Ampersand:
        case PropertyIds::$Plus:
        case PropertyIds::$BackTick:
        case PropertyIds::$Tick:
        case PropertyIds::index:
            return false;
        default:
            return JavascriptFunction::IsEnumerable(propertyId);
        }
    }

    BOOL JavascriptRegExpConstructor::IsConfigurable(PropertyId propertyId)
    {
        switch (propertyId)
        {
        case PropertyIds::input:
        case PropertyIds::$_:

        case PropertyIds::lastMatch:
        case PropertyIds::$Ampersand:

        case PropertyIds::lastParen:
        case PropertyIds::$Plus:

        case PropertyIds::leftContext:
        case PropertyIds::$BackTick:

        case PropertyIds::rightContext:
        case PropertyIds::$Tick:

        case PropertyIds::$1:
        case PropertyIds::$2:
        case PropertyIds::$3:
        case PropertyIds::$4:
        case PropertyIds::$5:
        case PropertyIds::$6:
        case PropertyIds::$7:
        case PropertyIds::$8:
        case PropertyIds::$9:
        case PropertyIds::index:
            return false;
        default:
            return JavascriptFunction::IsConfigurable(propertyId);
        }
    }

    BOOL JavascriptRegExpConstructor::GetSpecialNonEnumerablePropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext)
    {
        uint length = GetSpecialNonEnumerablePropertyCount();
        if (index < length)
        {
            *propertyName = requestContext->GetPropertyString(specialnonEnumPropertyIds[index]);
            return true;
        }
        return false;
    }

    // Returns the number of special non-enumerable properties this type has.
    uint JavascriptRegExpConstructor::GetSpecialNonEnumerablePropertyCount() const
    {
        return _countof(specialnonEnumPropertyIds);
    }

    // Returns the list of special properties for the type.
    PropertyId* JavascriptRegExpConstructor::GetSpecialNonEnumerablePropertyIds() const
    {
        return specialnonEnumPropertyIds;
    }


    BOOL JavascriptRegExpConstructor::GetSpecialEnumerablePropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext)
    {
        uint length = GetSpecialEnumerablePropertyCount();
        if (index < length)
        {
            *propertyName = requestContext->GetPropertyString(specialEnumPropertyIds[index]);
            return true;
        }
        return false;
    }

    PropertyId* JavascriptRegExpConstructor::GetSpecialEnumerablePropertyIds() const
    {
        return specialEnumPropertyIds;
    }

    // Returns the number of special non-enumerable properties this type has.
    uint JavascriptRegExpConstructor::GetSpecialEnumerablePropertyCount() const
    {
        return _countof(specialEnumPropertyIds);
    }

    // Returns the list of special properties for the type.
    PropertyId* JavascriptRegExpConstructor::GetSpecialPropertyIds() const
    {
        return specialPropertyIds;
    }

    uint JavascriptRegExpConstructor::GetSpecialPropertyCount() const
    {
        return _countof(specialPropertyIds);
    }

    BOOL JavascriptRegExpConstructor::GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext)
    {
        uint length = GetSpecialPropertyCount();
        if (index < length)
        {
            *propertyName = requestContext->GetPropertyString(specialPropertyIds[index]);
            return true;
        }
        return false;
    }


} // namespace Js
