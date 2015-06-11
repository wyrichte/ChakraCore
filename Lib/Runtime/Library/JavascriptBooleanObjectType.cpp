//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    BOOL JavascriptBooleanObject::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        if(this->GetValue())
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

    BOOL JavascriptBooleanObject::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Boolean, (Object)");
        return TRUE;
    }
} // namespace Js
