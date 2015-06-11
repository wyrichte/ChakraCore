//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------- 

#include "StdAfx.h"
#include "SIMDInt32x4Operation.h"

#if defined(_M_ARM32_OR_ARM64)
#ifdef SIMD_JS_ENABLED

namespace Js
{
    SIMDValue SIMDInt32x4Operation::OpInt32x4(int x, int y, int z, int w)
    {
        SIMDValue result;

        result.i32[SIMD_X] = x;
        result.i32[SIMD_Y] = y;
        result.i32[SIMD_Z] = z;
        result.i32[SIMD_W] = w;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpInt32x4(const SIMDValue& v)
    {// overload function with input paramter as SIMDValue for completeness, may not need
        SIMDValue result;

        result = v;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpZero()
    {
        SIMDValue result;

        result.i32[SIMD_X] = result.i32[SIMD_Y] = result.i32[SIMD_Z] = result.i32[SIMD_W] = 0;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpSplat(int x)
    {
        SIMDValue result;

        result.i32[SIMD_X] = result.i32[SIMD_Y] = result.i32[SIMD_Z] = result.i32[SIMD_W] = x;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpSplat(const SIMDValue& v)
    {
        SIMDValue result;

        result.i32[SIMD_X] = result.i32[SIMD_Y] = result.i32[SIMD_Z] = result.i32[SIMD_W] = v.i32[SIMD_X];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpBool(int x, int y, int z, int w)
    {
        SIMDValue result;

        int nX = x ? -1 : 0x0;
        int nY = y ? -1 : 0x0;
        int nZ = z ? -1 : 0x0;
        int nW = w ? -1 : 0x0;

        result.i32[SIMD_X] = nX;
        result.i32[SIMD_Y] = nY;
        result.i32[SIMD_Z] = nZ;
        result.i32[SIMD_W] = nW;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpBool(const SIMDValue& v)
    {
        SIMDValue result;

        // incoming 4 signed integers has to be 0 or -1
        Assert(v.i32[SIMD_X] == 0 || v.i32[SIMD_X] == -1);
        Assert(v.i32[SIMD_Y] == 0 || v.i32[SIMD_Y] == -1);
        Assert(v.i32[SIMD_Z] == 0 || v.i32[SIMD_Z] == -1);
        Assert(v.i32[SIMD_W] == 0 || v.i32[SIMD_W] == -1);
        
        result = v;
        return result;
    }

    // Conversions
    SIMDValue SIMDInt32x4Operation::OpFromBool(const SIMDValue& v)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (v.i32[SIMD_X]) ? 0xFFFFFFFF : 0x0;
        result.i32[SIMD_Y] = (v.i32[SIMD_Y]) ? 0xFFFFFFFF : 0x0;
        result.i32[SIMD_Z] = (v.i32[SIMD_Z]) ? 0xFFFFFFFF : 0x0;
        result.i32[SIMD_W] = (v.i32[SIMD_W]) ? 0xFFFFFFFF : 0x0;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpFromFloat32x4(const SIMDValue& v)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (int)(v.f32[SIMD_X]);
        result.i32[SIMD_Y] = (int)(v.f32[SIMD_Y]);
        result.i32[SIMD_Z] = (int)(v.f32[SIMD_Z]);
        result.i32[SIMD_W] = (int)(v.f32[SIMD_W]);

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpFromFloat64x2(const SIMDValue& v)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (int)(v.f64[SIMD_X]);
        result.i32[SIMD_Y] = (int)(v.f64[SIMD_Y]);
        result.i32[SIMD_Z] = result.i32[SIMD_W] = 0;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpFromFloat32x4Bits(const SIMDValue& v)
    {
        SIMDValue result;

        result.f64[SIMD_X] = v.f64[SIMD_X];
        result.f64[SIMD_Y] = v.f64[SIMD_Y];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpFromFloat64x2Bits(const SIMDValue& v)
    {
        return OpFromFloat32x4Bits(v);
    }

    // Unary Ops
    SIMDValue SIMDInt32x4Operation::OpAbs(const SIMDValue& value)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (value.i32[SIMD_X] < 0) ? -1 * value.i32[SIMD_X] : value.i32[SIMD_X];
        result.i32[SIMD_Y] = (value.i32[SIMD_Y] < 0) ? -1 * value.i32[SIMD_Y] : value.i32[SIMD_Y];
        result.i32[SIMD_Z] = (value.i32[SIMD_Z] < 0) ? -1 * value.i32[SIMD_Z] : value.i32[SIMD_Z];
        result.i32[SIMD_W] = (value.i32[SIMD_W] < 0) ? -1 * value.i32[SIMD_W] : value.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpNeg(const SIMDValue& value)
    {
        SIMDValue result;

        result.i32[SIMD_X] = -1 * value.i32[SIMD_X];
        result.i32[SIMD_Y] = -1 * value.i32[SIMD_Y];
        result.i32[SIMD_Z] = -1 * value.i32[SIMD_Z];
        result.i32[SIMD_W] = -1 * value.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpNot(const SIMDValue& value)
    {
        SIMDValue result;

        result.i32[SIMD_X] = ~(value.i32[SIMD_X]);
        result.i32[SIMD_Y] = ~(value.i32[SIMD_Y]);
        result.i32[SIMD_Z] = ~(value.i32[SIMD_Z]);
        result.i32[SIMD_W] = ~(value.i32[SIMD_W]);

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpAdd(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = aValue.i32[SIMD_X] + bValue.i32[SIMD_X];
        result.i32[SIMD_Y] = aValue.i32[SIMD_Y] + bValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = aValue.i32[SIMD_Z] + bValue.i32[SIMD_Z];
        result.i32[SIMD_W] = aValue.i32[SIMD_W] + bValue.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpSub(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = aValue.i32[SIMD_X] - bValue.i32[SIMD_X];
        result.i32[SIMD_Y] = aValue.i32[SIMD_Y] - bValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = aValue.i32[SIMD_Z] - bValue.i32[SIMD_Z];
        result.i32[SIMD_W] = aValue.i32[SIMD_W] - bValue.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpMul(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = aValue.i32[SIMD_X] * bValue.i32[SIMD_X];
        result.i32[SIMD_Y] = aValue.i32[SIMD_Y] * bValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = aValue.i32[SIMD_Z] * bValue.i32[SIMD_Z];
        result.i32[SIMD_W] = aValue.i32[SIMD_W] * bValue.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpAnd(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = aValue.i32[SIMD_X] & bValue.i32[SIMD_X];
        result.i32[SIMD_Y] = aValue.i32[SIMD_Y] & bValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = aValue.i32[SIMD_Z] & bValue.i32[SIMD_Z];
        result.i32[SIMD_W] = aValue.i32[SIMD_W] & bValue.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpOr(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = aValue.i32[SIMD_X] | bValue.i32[SIMD_X];
        result.i32[SIMD_Y] = aValue.i32[SIMD_Y] | bValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = aValue.i32[SIMD_Z] | bValue.i32[SIMD_Z];
        result.i32[SIMD_W] = aValue.i32[SIMD_W] | bValue.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpXor(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = aValue.i32[SIMD_X] ^ bValue.i32[SIMD_X];
        result.i32[SIMD_Y] = aValue.i32[SIMD_Y] ^ bValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = aValue.i32[SIMD_Z] ^ bValue.i32[SIMD_Z];
        result.i32[SIMD_W] = aValue.i32[SIMD_W] ^ bValue.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpMin(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (aValue.i32[SIMD_X] < bValue.i32[SIMD_X]) ? aValue.i32[SIMD_X] : bValue.i32[SIMD_X];
        result.i32[SIMD_Y] = (aValue.i32[SIMD_Y] < bValue.i32[SIMD_Y]) ? aValue.i32[SIMD_Y] : bValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = (aValue.i32[SIMD_Z] < bValue.i32[SIMD_Z]) ? aValue.i32[SIMD_Z] : bValue.i32[SIMD_Z];
        result.i32[SIMD_W] = (aValue.i32[SIMD_W] < bValue.i32[SIMD_W]) ? aValue.i32[SIMD_W] : bValue.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpMax(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (aValue.i32[SIMD_X] > bValue.i32[SIMD_X]) ? aValue.i32[SIMD_X] : bValue.i32[SIMD_X];
        result.i32[SIMD_Y] = (aValue.i32[SIMD_Y] > bValue.i32[SIMD_Y]) ? aValue.i32[SIMD_Y] : bValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = (aValue.i32[SIMD_Z] > bValue.i32[SIMD_Z]) ? aValue.i32[SIMD_Z] : bValue.i32[SIMD_Z];
        result.i32[SIMD_W] = (aValue.i32[SIMD_W] > bValue.i32[SIMD_W]) ? aValue.i32[SIMD_W] : bValue.i32[SIMD_W];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpLessThan(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (aValue.i32[SIMD_X] < bValue.i32[SIMD_X]) ? 0xffffffff : 0x0;
        result.i32[SIMD_Y] = (aValue.i32[SIMD_Y] < bValue.i32[SIMD_Y]) ? 0xffffffff : 0x0;
        result.i32[SIMD_Z] = (aValue.i32[SIMD_Z] < bValue.i32[SIMD_Z]) ? 0xffffffff : 0x0;
        result.i32[SIMD_W] = (aValue.i32[SIMD_W] < bValue.i32[SIMD_W]) ? 0xffffffff : 0x0;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpEqual(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (aValue.i32[SIMD_X] == bValue.i32[SIMD_X]) ? 0xffffffff : 0x0;
        result.i32[SIMD_Y] = (aValue.i32[SIMD_Y] == bValue.i32[SIMD_Y]) ? 0xffffffff : 0x0;
        result.i32[SIMD_Z] = (aValue.i32[SIMD_Z] == bValue.i32[SIMD_Z]) ? 0xffffffff : 0x0;
        result.i32[SIMD_W] = (aValue.i32[SIMD_W] == bValue.i32[SIMD_W]) ? 0xffffffff : 0x0;

        return result;
    }


    SIMDValue SIMDInt32x4Operation::OpGreaterThan(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i32[SIMD_X] = (aValue.i32[SIMD_X] > bValue.i32[SIMD_X]) ? 0xffffffff : 0x0;
        result.i32[SIMD_Y] = (aValue.i32[SIMD_Y] > bValue.i32[SIMD_Y]) ? 0xffffffff : 0x0;
        result.i32[SIMD_Z] = (aValue.i32[SIMD_Z] > bValue.i32[SIMD_Z]) ? 0xffffffff : 0x0;
        result.i32[SIMD_W] = (aValue.i32[SIMD_W] > bValue.i32[SIMD_W]) ? 0xffffffff : 0x0;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpShuffle(const SIMDValue& value, int mask)
    {
        SIMDValue result;

        int x = mask        & 0x03;
        int y = (mask >> 2) & 0x03;
        int z = (mask >> 4) & 0x03;
        int w = (mask >> 6) & 0x03;

        Assert(x >= 0 && x < 4);
        Assert(y >= 0 && y < 4);
        Assert(z >= 0 && z < 4);
        Assert(w >= 0 && w < 4);

        int storage[4];
        storage[0] = value.i32[SIMD_X];
        storage[1] = value.i32[SIMD_Y];
        storage[2] = value.i32[SIMD_Z];
        storage[3] = value.i32[SIMD_W];

        result.i32[SIMD_X] = storage[x];
        result.i32[SIMD_Y] = storage[y];
        result.i32[SIMD_Z] = storage[z];
        result.i32[SIMD_W] = storage[w];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpShuffleMix(const SIMDValue& aValue, const SIMDValue& bValue, int mask)
    {
        SIMDValue result;

        int x = mask & 0x03;
        int y = (mask >> 2) & 0x03;
        int z = (mask >> 4) & 0x03;
        int w = (mask >> 6) & 0x03;

        Assert(x >= 0 && x < 4);
        Assert(y >= 0 && y < 4);
        Assert(z >= 0 && z < 4);
        Assert(w >= 0 && w < 4);

        int storage[8];
        storage[0] = aValue.i32[SIMD_X];
        storage[1] = aValue.i32[SIMD_Y];
        storage[2] = aValue.i32[SIMD_Z];
        storage[3] = aValue.i32[SIMD_W];
        storage[4] = bValue.i32[SIMD_X];
        storage[5] = bValue.i32[SIMD_Y];
        storage[6] = bValue.i32[SIMD_Z];
        storage[7] = bValue.i32[SIMD_W];

        result.i32[SIMD_X] = storage[0 + x];
        result.i32[SIMD_Y] = storage[0 + y];
        result.i32[SIMD_Z] = storage[4 + z];
        result.i32[SIMD_W] = storage[4 + w];

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpShiftLeft(const SIMDValue& value, int count)
    {
        SIMDValue result;

        result.i32[SIMD_X] = value.i32[SIMD_X] << count;
        result.i32[SIMD_Y] = value.i32[SIMD_Y] << count;
        result.i32[SIMD_Z] = value.i32[SIMD_Z] << count;
        result.i32[SIMD_W] = value.i32[SIMD_W] << count;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpShiftRightLogical(const SIMDValue& value, int count)
    {
        SIMDValue result;

        int nIntMin = INT_MIN; // INT_MIN = -2147483648 = 0x80000000
        int mask = ~((nIntMin >> count) << 1); // now first count bits are 0
        // right shift count bits and shift in with 0
        result.i32[SIMD_X] = (value.i32[SIMD_X] >> count) & mask;
        result.i32[SIMD_Y] = (value.i32[SIMD_Y] >> count) & mask;
        result.i32[SIMD_Z] = (value.i32[SIMD_Z] >> count) & mask;
        result.i32[SIMD_W] = (value.i32[SIMD_W] >> count) & mask;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpShiftRightArithmetic(const SIMDValue& value, int count)
    {
        SIMDValue result;

        result.i32[SIMD_X] = value.i32[SIMD_X] >> count;
        result.i32[SIMD_Y] = value.i32[SIMD_Y] >> count;
        result.i32[SIMD_Z] = value.i32[SIMD_Z] >> count;
        result.i32[SIMD_W] = value.i32[SIMD_W] >> count;

        return result;
    }

    SIMDValue SIMDInt32x4Operation::OpSelect(const SIMDValue& mV, const SIMDValue& tV, const SIMDValue& fV)
    {
        SIMDValue result;

        SIMDValue trueResult  = SIMDInt32x4Operation::OpAnd(mV, tV);
        SIMDValue notValue    = SIMDInt32x4Operation::OpNot(mV);
        SIMDValue falseResult = SIMDInt32x4Operation::OpAnd(notValue, fV);

        result = SIMDInt32x4Operation::OpOr(trueResult, falseResult);

        return result;
    }

    // Get SignMask
    int SIMDInt32x4Operation::OpGetSignMask(const SIMDValue& v)
    {
        int result;

        // shift right 31 bits while shiftting in with zero 
        SIMDValue value = SIMDInt32x4Operation::OpShiftRightLogical(v, 31);

        // extract sign bit from each lane
        int mx = value.i32[SIMD_X];
        int my = value.i32[SIMD_Y];
        int mz = value.i32[SIMD_Z];
        int mw = value.i32[SIMD_W];

        result = mx | my << 1 | mz << 2 | mw << 3;

        return result;
    }
}

#endif
#endif