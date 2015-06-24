//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    Var JavascriptVariantDate::GetTypeOfString(ScriptContext* requestContext)
    {
        return requestContext->GetLibrary()->CreateStringFromCppLiteral(L"date");
    }

    JavascriptString* JavascriptVariantDate::GetValueString(ScriptContext* scriptContext)
    {
        return DateImplementation::ConvertVariantDateToString(this->value, scriptContext);
    }

    BOOL JavascriptVariantDate::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        BOOL ret;
        
        ENTER_PINNED_SCOPE(JavascriptString, resultString);
        resultString = DateImplementation::ConvertVariantDateToString(this->value, GetScriptContext());
        if (resultString != NULL)
        {
            stringBuilder->Append(resultString->GetString(), resultString->GetLength()); 
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }

        LEAVE_PINNED_SCOPE();
        
        return ret;
    }

    BOOL JavascriptVariantDate::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Date"); // For whatever reason in IE8 jscript, typeof returns "date"
                                                  // while the debugger displays "Date" for the type
        return TRUE;
    }

    RecyclableObject * JavascriptVariantDate::CloneToScriptContext(ScriptContext* requestContext)
    {
        return requestContext->GetLibrary()->CreateVariantDate(value);
    }

    RecyclableObject* JavascriptVariantDate::ToObject(ScriptContext* requestContext)
    {
        // WOOB 1124298: Just return a new object when converting to object.
        return requestContext->GetLibrary()->CreateObject(true);
    }    

    BOOL JavascriptVariantDate::GetProperty(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, PropertyValueInfo* info, Js::ScriptContext* requestContext) 
    {
        if (requestContext->GetThreadContext()->RecordImplicitException())
        {
            JavascriptError::ThrowTypeError(requestContext, JSERR_Property_VarDate, requestContext->GetPropertyName(propertyId)->GetBuffer());
        }
        *value = null;
        return true;
    };

    BOOL JavascriptVariantDate::GetProperty(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, PropertyValueInfo* info, Js::ScriptContext* requestContext) 
    {
        if (requestContext->GetThreadContext()->RecordImplicitException())
        {
            JavascriptError::ThrowTypeError(requestContext, JSERR_Property_VarDate, propertyNameString);
        }
        *value = null;
        return true;
    };

    BOOL JavascriptVariantDate::GetPropertyReference(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, PropertyValueInfo* info, Js::ScriptContext* requestContext)
    {
        if (requestContext->GetThreadContext()->RecordImplicitException())
        {
            JavascriptError::ThrowTypeError(requestContext, JSERR_Property_VarDate, requestContext->GetPropertyName(propertyId)->GetBuffer());
        }
        *value = null;
        return true;
    };

    BOOL JavascriptVariantDate::SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_VarDate, scriptContext->GetPropertyName(propertyId)->GetBuffer());
    };

    BOOL JavascriptVariantDate::SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_VarDate, propertyNameString->GetSz());
    };

    BOOL JavascriptVariantDate::InitProperty(Js::PropertyId propertyId, Js::Var value, PropertyOperationFlags flags, Js::PropertyValueInfo* info)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_VarDate, scriptContext->GetPropertyName(propertyId)->GetBuffer());
    };

    BOOL JavascriptVariantDate::DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_VarDate, scriptContext->GetPropertyName(propertyId)->GetBuffer());
    };

    BOOL JavascriptVariantDate::GetItemReference(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, __in Js::ScriptContext * scriptContext)
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_VarDate, JavascriptNumber::ToStringRadix10(index, scriptContext)->GetSz());
    };

    BOOL JavascriptVariantDate::GetItem(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, __in Js::ScriptContext * scriptContext) 
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_VarDate, JavascriptNumber::ToStringRadix10(index, scriptContext)->GetSz());
    };

    BOOL JavascriptVariantDate::SetItem(__in uint32 index, __in Js::Var value, __in Js::PropertyOperationFlags flags) 
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        JavascriptError::ThrowTypeError(scriptContext, JSERR_Property_VarDate, JavascriptNumber::ToStringRadix10(index, scriptContext)->GetSz());
    };
}

