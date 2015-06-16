//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    BOOL JavascriptDate::ToPrimitive(JavascriptHint hint, Var* result, ScriptContext * requestContext)
    {
        if (hint == JavascriptHint::None)
        {
            hint = JavascriptHint::HintString;
        }

        return DynamicObject::ToPrimitive(hint, result, requestContext);
    }

    BOOL JavascriptDate::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {        
        ENTER_PINNED_SCOPE(JavascriptString, valueStr);
        valueStr = this->m_date.GetString(DateImplementation::DateStringFormat::Default);
        stringBuilder->Append(valueStr->GetString(), valueStr->GetLength());
        LEAVE_PINNED_SCOPE();
        return TRUE;
    }

    BOOL JavascriptDate::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Object, (Date)");
        return TRUE;
    }

} // namespace Js
