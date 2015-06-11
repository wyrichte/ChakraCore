//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    struct DiagConstants
    {
        // Max length, in characters, of the function name string.
        static const int MaxFunctionNameLength = MAX_PATH;
        
        // Max length, in characters of the Uri string.
        static const int MaxUriLength = MAX_PATH;

        static const UINT InvalidIndexPropertyName = (UINT)-1;
    };
}
