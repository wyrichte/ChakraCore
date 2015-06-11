//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
namespace Js
{
    RecyclableObject*  JavascriptSafeArray::ToObject(ScriptContext * requestContext)
    {        
        return ScriptSite::FromScriptContext(requestContext)->CreateJavascriptSafeArrayObject(this);
    }

    BOOL JavascriptSafeArray::Equals(Var other, BOOL* value, ScriptContext * requestContext)
    {
        *value = FALSE;
        if (TypeIds_SafeArray == JavascriptOperators::GetTypeId(other))
        {            
            JavascriptSafeArray* right = JavascriptSafeArray::FromVar(other);
            if(this->GetSafeArray() == right->GetSafeArray())
            {
                *value = TRUE;
            }
        }
        return TRUE;
    }

    // in V5.8 typeof(safe_array) returns "unknown", like the base. 
    // Consider to version this in V9 with a better string
    Var JavascriptSafeArray::GetTypeOfString(ScriptContext * requestContext)
    {
        return requestContext->GetLibrary()->GetUnknownDisplayString();
    }

}