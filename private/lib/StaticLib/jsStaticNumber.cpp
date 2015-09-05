
//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StaticLibPch.h"

namespace JsStaticAPI
{
    BOOL JavascriptNumber::Is(Var value)
    {
        return Js::JavascriptNumber::Is(value);
    }

    double GetValueUncheck(Var value)
    {
        return Js::JavascriptNumber::GetValue(value);
    }
}