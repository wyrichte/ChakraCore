//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{

    BOOL JavascriptVariantDate::ToPrimitive(JavascriptHint hint, Var* result, ScriptContext * requestContext)
    {
        if (hint == JavascriptHint::HintString)
        {
            JavascriptString* resultString = this->GetValueString(requestContext);
            if (resultString != NULL)
            {
                (*result) = resultString;
                return TRUE;
            }
            Assert(false);
        }
        else if (hint == JavascriptHint::HintNumber)
        {
            *result = JavascriptNumber::ToVarNoCheck(DateImplementation::JsUtcTimeFromVarDate(value, requestContext), requestContext);
            return TRUE;
        }
        else
        {
            Assert(hint == JavascriptHint::None);
            *result = this;
            return TRUE;
        }
        return FALSE;
    }

    BOOL JavascriptVariantDate::Equals(Var other, BOOL *value, ScriptContext * requestContext)
    {
        // Calling .Equals on a VT_DATE variant at least gives the "[property name] is null or not An object error"
        *value = FALSE;
        return true;
    }
}