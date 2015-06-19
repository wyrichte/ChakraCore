//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////
// NumberUtilitiesBase.h is used by static lib shared between trident and chakra. We need to keep
// the size consistent and try not to change its size. We need to have matching mshtml.dll
// if the size changed here. 
/////////////////////////////////////////////////////////
#pragma once
namespace Js
{
    class NumberConstantsBase
    {
    public:
        static const UINT64 k_Nan = 0xFFF8000000000000ull;
    };

    class NumberUtilitiesBase
    {
    protected:
        static const INT64 Pos_InvalidInt64 = 0x8000000000000000ll;  // Used for positive infinity/overflow.
        static const INT64 Neg_InvalidInt64 = 0x7fffffffffffffffll;  // Used for negative infinity/overflow.
    };
}