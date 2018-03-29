//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#pragma once

namespace Js
{
        enum class BuiltInFacet : size_t
        {
            _None,

#define ENTRY_BUILTIN(esVersion, typeName, location, propertyName) typeName ## _ ## location ## _ ## propertyName ,
#include "ESBuiltIns.h"
#undef ENTRY_BUILTIN

            _Max
        };


}