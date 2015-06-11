// Copyright (C) Microsoft. All rights reserved. 

#pragma once

inline bool 
Int32Math::Add(int32 left, int32 right, int32 *pResult)
{
    if(sizeof(void *) == 4)
    {
        // Overflow occurs when the result has a different sign from both the left and right operands
        *pResult = left + right;
        return ((left ^ *pResult) & (right ^ *pResult)) < 0;
    }

    Assert(sizeof(void *) == 8);
    int64 result64 = (int64)left + (int64)right;
    *pResult = (int32)result64;
    return result64 != (int64)(*pResult);
}

inline bool
Int32Math::Mul(int32 left, int32 right, int32 *pResult)
{
    bool fOverflow;
#if _M_IX86
    __asm
    {
        mov eax, left
        imul eax, right
        seto fOverflow
        mov ecx, pResult
        mov [ecx], eax
    }
#else
    int64 result64 = (int64)left * (int64)right;

    *pResult = (int32)result64;

    fOverflow = (result64 != (int64)(*pResult));
#endif

    return fOverflow;
}

inline bool
Int32Math::Shl(int32 left, int32 right, int32 *pResult)
{
    *pResult = left << (right & 0x1F);
    return (left != (int32)((uint32)*pResult >> right));
}
