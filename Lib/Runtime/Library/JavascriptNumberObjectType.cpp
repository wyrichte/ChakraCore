//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{

    BOOL JavascriptNumberObject::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        ENTER_PINNED_SCOPE(JavascriptString, valueStr);
        valueStr = JavascriptNumber::ToStringRadix10(this->GetValue(), GetScriptContext());
        stringBuilder->Append(valueStr->GetString(), valueStr->GetLength());
        LEAVE_PINNED_SCOPE();
        return TRUE;
    }

    BOOL JavascriptNumberObject::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Number, (Object)");
        return TRUE;
    }
} 
