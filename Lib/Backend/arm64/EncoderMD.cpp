//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "BackEnd.h"
#include "ARMEncode.h"

bool     
EncoderMD::EncodeImmediate16(long constant, DWORD * result)
{
    if (constant > 0xFFFF)
    {
        return FALSE;
    }

    DWORD encode = (constant & 0xFFFF) << 5;

    *result |= encode;
    return TRUE;
}

ENCODE_32 
EncoderMD::BranchOffset_26(int64 x)
{
    Assert(IS_CONST_INT26(x >> 1));
    Assert((x & 0x3) == 0);
    x = x >> 2;
    return (ENCODE_32) x;
}

