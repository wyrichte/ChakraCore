//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    BOOL JavascriptSymbolObject::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        if (this->GetValue())
        {
            stringBuilder->Append(this->GetValue()->GetBuffer(), this->GetValue()->GetLength());
        }
        return TRUE;
    }

    BOOL JavascriptSymbolObject::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(L"Symbol, (Object)");
        return TRUE;
    }
} // namespace Js
