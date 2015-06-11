//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "math.h"

// Mark as volatile to minimize MT issue.
// Note that Rand() isn't MT safe, which shouldn't matter as long
// as we still return a random number.
volatile UINT_PTR Math::RandSeed = 0;

bool
Math::FitsInDWord(size_t value)
{
    return ((size_t)(signed int)(value & 0xFFFFFFFF) == value);
}

uint32
Math::NextPowerOf2(uint32 n)
{
    n = n - 1;
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    n = n | (n >> 8);
    n = n | (n >> 16);
    n++;
    return n;
}

UINT_PTR
Math::Rand()
{
    UINT_PTR newRand;

    newRand = Math::RandSeed;

    if (newRand == 0)
    {
        // Assume this is first call
        LARGE_INTEGER perfctr;
        QueryPerformanceCounter(&perfctr);
        newRand = perfctr.LowPart;
        newRand ^= perfctr.HighPart;

        srand((uint)newRand);
        newRand ^= rand();

        newRand = (UINT_PTR)EncodeSystemPointer((PVOID)newRand);
        
        Math::RandSeed = newRand;

        return newRand;
    }

    newRand ^= GetTickCount();
#ifdef _WIN64
    newRand = _rotl64(newRand, 13);
#else
    newRand = _rotl(newRand, 13);
#endif
    newRand ^= rand();

    Math::RandSeed = newRand;

    return newRand;
}

__declspec(noreturn) void Math::DefaultOverflowPolicy()
{
    Js::Throw::OutOfMemory();
}
