//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    const double NumberConstants::MAX_VALUE = *(double*)(&NumberConstants::k_PosMax);
    const double NumberConstants::MIN_VALUE = *(double*)(&NumberConstants::k_PosMin);
    const double NumberConstants::NaN = *(double*)(&NumberConstants::k_Nan);
    const double NumberConstants::NEGATIVE_INFINITY= *(double*)(&NumberConstants::k_NegInf);
    const double NumberConstants::POSITIVE_INFINITY= *(double*)(&NumberConstants::k_PosInf );
    const double NumberConstants::NEG_ZERO= *(double*)(&NumberConstants::k_NegZero );    

    // These are used in 128-bit operations in the JIT and inline asm
    __declspec(align(16)) const BYTE NumberConstants::AbsDoubleCst[] =
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F };

    __declspec(align(16)) const BYTE NumberConstants::AbsFloatCst[] =
    { 0xFF, 0xFF, 0xFF, 0x7F,
      0xFF, 0xFF, 0xFF, 0x7F,
      0xFF, 0xFF, 0xFF, 0x7F,
      0xFF, 0xFF, 0xFF, 0x7F };

    __declspec(align(16)) double const NumberConstants::UIntConvertConst[2] = { 0, 4294967296.000000 };
    __declspec(align(16)) float const NumberConstants::MaskNegFloat[] = { -0.0f, -0.0f, -0.0f, -0.0f };
    __declspec(align(16)) double const NumberConstants::MaskNegDouble[] = { -0.0, -0.0 };

    int NumberUtilities::CbitZeroLeft(ulong lu)
    {
        int cbit = 0;

        if (0 == (lu & 0xFFFF0000))
        {
            cbit += 16;
            lu <<= 16;
        }
        if (0 == (lu & 0xFF000000))
        {
            cbit += 8;
            lu <<= 8;
        }
        if (0 == (lu & 0xF0000000))
        {
            cbit += 4;
            lu <<= 4;
        }
        if (0 == (lu & 0xC0000000))
        {
            cbit += 2;
            lu <<= 2;
        }
        if (0 == (lu & 0x80000000))
        {
            cbit += 1;
            lu <<= 1;
        }
        Assert(lu & 0x80000000);

        return cbit;
    }

    charcount_t NumberUtilities::UInt16ToString(uint16 integer, __out __ecount(outBufferSize) WCHAR* outBuffer, charcount_t outBufferSize, char widthForPaddingZerosInsteadSpaces)
    {
        // inlined here
        WORD digit;
        charcount_t cchWritten = 0;

        Assert(cchWritten < outBufferSize);

        // word is 0 to 65,535 -- 5 digits max
        if (cchWritten < outBufferSize)
        {
            if (integer >= 10000)
            {
                digit = integer / 10000;
                integer %= 10000;
                *outBuffer = digit + L'0';
                outBuffer++;
                cchWritten++;
            }
            else if( widthForPaddingZerosInsteadSpaces > 4 )
            {
                *outBuffer = L'0';
                outBuffer++;
                cchWritten++;
            }
        }

        Assert(cchWritten < outBufferSize);
        if (cchWritten < outBufferSize)
        {
            if (integer >= 1000)
            {
                digit = integer / 1000;
                integer %= 1000;
                *outBuffer = digit + L'0';
                outBuffer++;
                cchWritten++;
            }
            else if( widthForPaddingZerosInsteadSpaces > 3 )
            {
                *outBuffer = L'0';
                outBuffer++;
                cchWritten++;
            }
        }

        Assert(cchWritten < outBufferSize);
        if (cchWritten < outBufferSize)
        {
            if (integer >= 100)
            {
                digit = integer / 100;
                integer %= 100;
                *outBuffer = digit + L'0';
                outBuffer++;
                cchWritten++;
            }
            else if( widthForPaddingZerosInsteadSpaces > 2 )
            {
                *outBuffer = L'0';
                outBuffer++;
                cchWritten++;
            }
        }

        Assert(cchWritten < outBufferSize);
        if (cchWritten < outBufferSize)
        {
            if (integer >= 10)
            {
                digit = integer / 10;
                integer %= 10;
                *outBuffer = digit + L'0';
                outBuffer++;
                cchWritten++;
            }
            else if( widthForPaddingZerosInsteadSpaces > 1 )
            {
                *outBuffer = L'0';
                outBuffer++;
                cchWritten++;
            }
        }

        Assert(cchWritten < outBufferSize);
        if (cchWritten < outBufferSize)
        {
            *outBuffer = integer + L'0';
            outBuffer++;
            cchWritten++;
        }

        Assert(cchWritten < outBufferSize);
        if (cchWritten < outBufferSize)
        {
            // cchWritten doesn't include the terminating char, like swprintf_s
            *outBuffer = 0;
        }

        return cchWritten;
    }

    BOOL NumberUtilities::TryConvertToUInt32(const wchar_t* str, int length, uint32* intVal)
    {
        if (length <= 0 || length > 10)
        {
            return false;
        }
        if (length == 1)
        {
            if (str[0] >= L'0' && str[0] <= L'9')
            {
                *intVal = (uint32)(str[0] - L'0');
                return true;
            }
            else
            {
                return false;
            }
        }
        if (str[0] < L'1' || str[0] > L'9')
        {
            return false;
        }
        uint32 val = (uint32)(str[0] - L'0');
        int calcLen = min(length, 9);
        for (int i = 1; i < calcLen; i++)
        {
            if ((str[i] < L'0')|| (str[i] > L'9'))
            {
                return false;
            }
            val = (val * 10) + (uint32)(str[i] - L'0');
        }
        if (length == 10)
        {
            // check for overflow 4294967295
            if (str[9] < L'0' || str[9] > L'9' ||
                UInt32Math::Mul(val, 10, &val) ||
                UInt32Math::Add(val, (uint32)(str[9] - L'0'), &val))
            {
                return false;
            }
        }
        *intVal = val;
        return true;
    }

    double NumberUtilities::Modulus(double dblLeft, double dblRight)
    {
        double value = 0;

        if (!Js::NumberUtilities::IsFinite(dblRight))
        {
            if (NumberUtilities::IsNan(dblRight) || !Js::NumberUtilities::IsFinite(dblLeft))
            {
                value = NumberConstants::NaN;
            }
            else
            {
                value =  dblLeft;
            }
        }
        else if (0 == dblRight || NumberUtilities::IsNan(dblLeft))
        {
            value =  NumberConstants::NaN;
        }
        else if (0 == dblLeft)
        {
            value =  dblLeft;
        }
        else
        {
            value = fmod(dblLeft, dblRight);
        }

        return value;
    }
}
