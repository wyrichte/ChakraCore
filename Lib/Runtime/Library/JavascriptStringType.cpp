//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{

    BOOL JavascriptString::HasProperty(PropertyId propertyId)
    {
        if (propertyId == PropertyIds::length)
        {
            return true;
        }
        ScriptContext* scriptContext = GetScriptContext();
        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            if (index < (uint32)this->GetLength())
            {
                return true;
            }
        }
        return false;
    }

    BOOL JavascriptString::IsEnumerable(PropertyId propertyId)
    {
        ScriptContext* scriptContext = GetScriptContext();
        uint32 index;
        if (scriptContext->IsNumericPropertyId(propertyId, &index))
        {
            if (index < (uint32)this->GetLength())
            {
                return true;
            }
        }
        return false;
    }

    BOOL JavascriptString::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return GetPropertyBuiltIns(propertyId, value);
    }
    BOOL JavascriptString::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != null && GetPropertyBuiltIns(propertyRecord->GetPropertyId(), value))
        {
            return true;
        }
        return false;
    }
    bool JavascriptString::GetPropertyBuiltIns(PropertyId propertyId, Var* value)
    {
        if (propertyId == PropertyIds::length)
        {
            *value = JavascriptNumber::ToVar(this->GetLength(), this->GetScriptContext());
            return true;
        }

        return false;
    }
    BOOL JavascriptString::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return JavascriptString::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL JavascriptString::SetItem(uint32 index, Var value, PropertyOperationFlags propertyOperationFlags)
    {
        if (this->HasItemAt(index))
        {
            JavascriptError::ThrowCantAssignIfStrictMode(propertyOperationFlags, this->GetScriptContext());

            return FALSE;
        }

        return __super::SetItem(index, value, propertyOperationFlags);
    }

    BOOL JavascriptString::DeleteItem(uint32 index, PropertyOperationFlags propertyOperationFlags)
    {
        if (this->HasItemAt(index))
        {
            JavascriptError::ThrowCantDeleteIfStrictMode(propertyOperationFlags, this->GetScriptContext(), TaggedInt::ToString(index, this->GetScriptContext())->GetString());

            return FALSE;
        }

        return __super::DeleteItem(index, propertyOperationFlags);
    }

    BOOL JavascriptString::HasItem(uint32 index)
    {
        return this->HasItemAt(index);
    }

    BOOL JavascriptString::GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        // String should always be marshalled to the current context
        Assert(requestContext == this->GetScriptContext());
        return this->GetItemAt(index, value);
    }

    BOOL JavascriptString::GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext)
    {
        // String should always be marshalled to the current context
        return this->GetItemAt(index, value);
    }

    BOOL JavascriptString::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics, bool enumSymbols)
    {        
        *enumerator = RecyclerNew(GetScriptContext()->GetRecycler(), JavascriptStringEnumerator, this, requestContext);
        return true;
    }

    BOOL JavascriptString::DeleteProperty(PropertyId propertyId, PropertyOperationFlags propertyOperationFlags)
    {
        if (propertyId == PropertyIds::length)
        { 
            JavascriptError::ThrowCantDeleteIfStrictMode(propertyOperationFlags, this->GetScriptContext(), this->GetScriptContext()->GetPropertyName(propertyId)->GetBuffer());

            return FALSE; 
        }
        return __super::DeleteProperty(propertyId, propertyOperationFlags);
    }

    BOOL JavascriptString::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {        
        stringBuilder->Append(L'"');
        stringBuilder->Append(this->GetString(), this->GetLength());
        stringBuilder->Append(L'"');
        return TRUE;
    }

    BOOL JavascriptString:: GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"String");
        return TRUE;
    }
    
    RecyclableObject* JavascriptString::ToObject(ScriptContext * requestContext)
    {        
        return requestContext->GetLibrary()->CreateStringObject(this);           
    }

    Var JavascriptString::GetTypeOfString(ScriptContext * requestContext)
    {
        return requestContext->GetLibrary()->GetStringTypeDisplayString();
    }
}
