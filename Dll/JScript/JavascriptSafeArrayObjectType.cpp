//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    BOOL JavascriptSafeArrayObject::GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {        
        stringBuilder->AppendCppLiteral(JS_DIAG_VALUE_JavascriptSafeArrayObject);
        return  TRUE;
    }

    BOOL JavascriptSafeArrayObject::GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext)
    {
        stringBuilder->AppendCppLiteral(JS_DIAG_TYPE_JavascriptSafeArrayObject);
        return TRUE;
    }
} 
