//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    bool NumberUtilities::IsDigit(int ch)
    {
        return ch >= '0' && ch <= '9';
    }

    /***************************************************************************
    Multiply two unsigned longs. Return the low ulong and fill *pluHi with
    the high ulong.
    ***************************************************************************/
    // Turn off warning that there is no return value
#pragma warning(disable:4035)  // re-enable below
    ulong NumberUtilities::MulLu(ulong lu1, ulong lu2, ulong *pluHi)
    {
#if _WIN32 || _WIN64

#if I386_ASM
        __asm
        {
            mov eax,lu1
                mul lu2
                mov ebx,pluHi
                mov DWORD PTR [ebx],edx
        }
#else //!I386_ASM
        DWORDLONG llu = UInt32x32To64(lu1, lu2);

        *pluHi = (ulong)(llu >> 32);
        return (ulong)llu;
#endif //!I386_ASM

#else
#error Neither _WIN32, nor _WIN64 is defined
#endif
    }
#pragma warning(default:4035)

    /***************************************************************************
    Add two unsigned longs and return the carry bit.
    ***************************************************************************/
    int NumberUtilities::AddLu(ulong *plu1, ulong lu2)
    {
        *plu1 += lu2;
        return *plu1 < lu2;
    }

    ulong &NumberUtilities::LuHiDbl(double &dbl)
    {
#ifdef BIG_ENDIAN
        return ((ulong *)&dbl)[0];
#else //!BIG_ENDIAN
        return ((ulong *)&dbl)[1];
#endif //!BIG_ENDIAN
    }

    ulong &NumberUtilities::LuLoDbl(double &dbl)
    {
#ifdef BIG_ENDIAN
        return ((ulong *)&dbl)[1];
#else //!BIG_ENDIAN
        return ((ulong *)&dbl)[0];
#endif //!BIG_ENDIAN
    }

#if defined(_M_X64)
    __inline INT64 NumberUtilities::TryToInt64(double T1)
    {
        // _mm_cvttsd_si64x will result in 0x8000000000000000 if the value is NaN Inf or Zero, or overflows int64
        __m128d a;
        a = _mm_load_sd(&T1);
        return _mm_cvttsd_si64x(a);
    }
#else
    __inline INT64 NumberUtilities::TryToInt64(double T1)
   {
        INT64 T4_64;
#if defined(_M_IX86)
        // If SSE3 is available use FISTPP.  VC (dev10) generates a FISTP, but needs to 
        // first change the FPU rounding, which is very slow...
        if (AutoSystemInfo::Data.SSE3Available())
        {
            // FISTTP will result in 0x8000000000000000 in T4_64 if the value is NaN Inf or Zero, or overflows int64
            _asm {
                FLD T1
                FISTTP T4_64
            }
        }
        else
#endif
#if defined(_M_ARM32_OR_ARM64)
        // Win8 286065: ARM: casts to int64 from double for NaNs, infinity, overflow:
        // - non-infinity NaNs -> 0
        // - infinity NaNs: -1.#INF -> 0x8000000000000000, 1.#INF  -> 0x7FFFFFFFFFFFFFFF.
        // - overflow: negative     -> 0x8000000000000000, positive-> 0x7FFFFFFFFFFFFFFF.
        // We have to take care of non-infinite NaNs to make sure the result is not a valid int64 rather than 0.
        if (IsNan(T1))
        {
            return Pos_InvalidInt64;
        }
        else if (T1 < -9223372036854775808.0) // -9223372036854775808 is double value corresponsing to Neg_InvalidInt64.
        {
            // TODO: Remove this temp workaround.
            // This is to walk around CRT issue (Win8 404170): there is a band of values near/less than negative overflow 
            // for which cast to int64 results in positive number (bug), then going further down in negative direction it turns 
            // back to negative overflow value (as it should).
            return Pos_InvalidInt64;
        }
        else
#endif
        {
            // The cast will result in 0x8000000000000000 in T4_64 if the value is NaN Inf or Zero, or overflows int64
            T4_64 = static_cast<INT64>(T1);
        }

#if defined(_M_ARM32_OR_ARM64)
        if (T4_64 == Neg_InvalidInt64)
        {
            // Win8 391983: what happens in 64bit overflow is not spec'ed. On ARM T4_64 would be 0x7F..FF but if we extend
            // ToInt32 to 64bit, because of ES5_9.5.5 the result would be 0x80..00. On Intel all overflows result in 0x80..00.
            // So, be consistent with Intel.
            return Pos_InvalidInt64;
        }
#endif

        return T4_64;
    }
#endif

    // Returns true <=> TryToInt64() call resulted in a valid value.
    __inline bool NumberUtilities::IsValidTryToInt64(__int64 value)
    {
#if defined(_M_ARM32_OR_ARM64)
        return value != Pos_InvalidInt64 && value != Neg_InvalidInt64;
#else
        return value != Pos_InvalidInt64;
#endif
    }

    __inline bool NumberUtilities::IsFinite(double value)
    {
#if defined(_M_X64_OR_ARM64)
        return 0 != (~(ToSpecial(value)) & 0x7FF0000000000000ull);
#else
        return 0 != (~Js::NumberUtilities::LuHiDbl(value) & 0x7FF00000);
#endif
    }

    __inline bool NumberUtilities::IsNan(double value) 
    {
#if defined(_M_X64_OR_ARM64)
        // NaN is a range of values; all bits on the exponent are 1's and some nonzero significant.
        // no distinction on signed NaN's
        uint64 nCompare = ToSpecial(value);
        bool isNan = ( 0 == (~nCompare & 0x7FF0000000000000ull) &&
                0 != ( nCompare & 0x000FFFFFFFFFFFFFull) );
        return isNan;
#else
        return 0 == (~Js::NumberUtilities::LuHiDbl(value) & 0x7FF00000) &&
            (0 != Js::NumberUtilities::LuLoDbl(value) || 0 != (Js::NumberUtilities::LuHiDbl(value) & 0x000FFFFF));
#endif
    }

    __inline bool NumberUtilities::IsSpecial(double value,uint64 nSpecial)
    {
        // Perform a bitwise comparison using uint64 instead of a double comparison, since that
        // would trigger FPU exceptions, etc.
        uint64 nCompare = ToSpecial(value);
        return nCompare == nSpecial;
    }

    __inline uint64 NumberUtilities::ToSpecial(double value)
    {
        return  *(reinterpret_cast<uint64 *>(&value));
    }
}
