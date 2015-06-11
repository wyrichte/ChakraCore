//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    BOOL JavascriptBoolean::Equals(Var other, BOOL* value, ScriptContext * requestContext)
    {
        return JavascriptBoolean::Equals(this, other, value, requestContext);
    }

    BOOL JavascriptBoolean::Equals(JavascriptBoolean* left, Var right, BOOL* value, ScriptContext * requestContext)
    {
        switch (JavascriptOperators::GetTypeId(right))
        {
        case TypeIds_Integer:
            *value = left->GetValue() ? TaggedInt::ToInt32(right) == 1 : TaggedInt::ToInt32(right) == 0;
            break;
        case TypeIds_Number:
            *value = left->GetValue() ? JavascriptNumber::GetValue(right) == 1.0 : JavascriptNumber::GetValue(right) == 0.0;
            break;
        case TypeIds_Int64Number:
            *value = left->GetValue() ? JavascriptInt64Number::FromVar(right)->GetValue() == 1 : JavascriptInt64Number::FromVar(right)->GetValue() == 0;
            break;
        case TypeIds_UInt64Number:
            *value = left->GetValue() ? JavascriptUInt64Number::FromVar(right)->GetValue() == 1 : JavascriptUInt64Number::FromVar(right)->GetValue() == 0;
            break;
        case TypeIds_Boolean:
            *value = left->GetValue() == JavascriptBoolean::FromVar(right)->GetValue();
            break;
        case TypeIds_String:
            *value = left->GetValue() ? JavascriptConversion::ToNumber(right, requestContext) == 1.0 : JavascriptConversion::ToNumber(right, requestContext) == 0.0;
            break;
        case TypeIds_Symbol:
            *value = FALSE;
            break;
        case TypeIds_VariantDate:
            // == on a variant always returns false.  Putting this in a 
            // switch in each .Equals to prevent a perf hit by adding an
            // if/branch to JavascriptOperators::Equal_Full
            *value = FALSE; 
            break;
        case TypeIds_Undefined:
        case TypeIds_Null:
        default:
            *value = JavascriptOperators::Equal_Full(left->GetValue() ? TaggedInt::ToVarUnchecked(1) : TaggedInt::ToVarUnchecked(0), right, requestContext);
            break;
        }
        return true;
    }

    BOOL JavascriptBoolean::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        if (this->GetValue())
        {
            JavascriptString* trueDisplayString = GetLibrary()->GetTrueDisplayString();
            stringBuilder->Append(trueDisplayString->GetString(), trueDisplayString->GetLength());
        }
        else
        {
            JavascriptString* falseDisplayString = GetLibrary()->GetFalseDisplayString();
            stringBuilder->Append(falseDisplayString->GetString(), falseDisplayString->GetLength());
        }
        return TRUE;
    }

    BOOL JavascriptBoolean::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Boolean");
        return TRUE;
    }

    RecyclableObject* JavascriptBoolean::ToObject(ScriptContext * requestContext)
    {
        return requestContext->GetLibrary()->CreateBooleanObject(this->GetValue() ? true : false);
    }

    Var JavascriptBoolean::GetTypeOfString(ScriptContext * requestContext)
    {
        return requestContext->GetLibrary()->GetBooleanTypeDisplayString();
    }
} // namespace Js
