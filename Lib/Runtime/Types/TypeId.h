//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

namespace Js
{

#include "EdgeJavascriptTypeId.h"
    // All WinRT dates are regular Javascript dates too
    inline bool IsDateTypeId(TypeId typeId) { return (typeId == TypeIds_Date || typeId == TypeIds_WinRTDate); }
}
