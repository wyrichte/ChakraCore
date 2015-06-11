//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    const ushort CallInfo::ksizeofCount =  24;
    const ushort CallInfo::ksizeofCallFlags = 8;
    const uint CallInfo::kMaxCountArgs = (1 << ksizeofCount) - 1 ;
}
