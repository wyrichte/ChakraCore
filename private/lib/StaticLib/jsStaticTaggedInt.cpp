//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StaticLibPch.h"

namespace JsStaticAPI
{
    BOOL TaggedInt::Is(Var value)
    {
        return Js::TaggedInt::Is(value);
    }

    Var ToVarUnchecked(int value)
    {
        return Js::TaggedInt::ToVarUnchecked(value);
    }
};