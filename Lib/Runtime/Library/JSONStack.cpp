//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace JSON
{   
    bool StrictEqualsObjectComparer::Equals(Js::Var x, Js::Var y)
    {
        return JSONStack::Equals(x,y);
    }
}// namespace JSON