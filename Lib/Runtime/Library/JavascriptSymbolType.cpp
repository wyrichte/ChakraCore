//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    BOOL JavascriptSymbol::Equals(Var other, BOOL* value, ScriptContext * requestContext)
    {
        return JavascriptSymbol::Equals(this, other, value, requestContext);
    }

    BOOL JavascriptSymbol::Equals(JavascriptSymbol* left, Var right, BOOL* value, ScriptContext * requestContext)
    {
        switch (JavascriptOperators::GetTypeId(right))
        {
        case TypeIds_Symbol:
            *value = left->GetValue() == JavascriptSymbol::FromVar(right)->GetValue();
            break;
        case TypeIds_SymbolObject:
            *value = left->GetValue() == JavascriptSymbolObject::FromVar(right)->GetValue();
            break;
        default:
            *value = JavascriptOperators::Equal_Full(right, left, requestContext);
            break;
        }

        return TRUE;
    }

    BOOL JavascriptSymbol::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        if (this->GetValue())
        {
            stringBuilder->AppendCppLiteral(L"Symbol(");
            stringBuilder->Append(this->GetValue()->GetBuffer(), this->GetValue()->GetLength());
            stringBuilder->Append(L')');
        }
        return TRUE;
    }

    BOOL JavascriptSymbol::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Symbol");
        return TRUE;
    }

    RecyclableObject* JavascriptSymbol::ToObject(ScriptContext * requestContext)
    {
        return requestContext->GetLibrary()->CreateSymbolObject(this);
    }

    Var JavascriptSymbol::GetTypeOfString(ScriptContext * requestContext)
    {
        return requestContext->GetLibrary()->GetSymbolTypeDisplayString();
    }

    JavascriptString* JavascriptSymbol::ToString(ScriptContext * requestContext)
    {
        // Reject implicit call
        ThreadContext* threadContext = requestContext->GetThreadContext();
        if (threadContext->IsDisableImplicitCall())
        {
            threadContext->AddImplicitCallFlags(Js::ImplicitCall_External);
            return nullptr;
        }
        // This keeps getting revisted but as of ES6 spec revision 20, 
        // implicit string conversion of symbol primitives is supposed to throw a TypeError.
        JavascriptError::ThrowTypeError(requestContext, VBSERR_OLENoPropOrMethod, L"ToString");
    }

    JavascriptString* JavascriptSymbol::ToString(const PropertyRecord* propertyRecord, ScriptContext * requestContext)
    {
        const wchar_t* description = propertyRecord->GetBuffer();
        uint len = propertyRecord->GetLength();
        CompoundString* str = CompoundString::NewWithCharCapacity(len + _countof(L"Symbol()"), requestContext->GetLibrary());

        str->AppendChars(L"Symbol(", _countof(L"Symbol(") - 1);
        str->AppendChars(description, len);
        str->AppendChars(L')');

        return str;
    }
} // namespace Js
