//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#ifdef SIMD_JS_ENABLED

namespace Js {
    

    class SIMDFloat32x4Lib
    {
    public:
        class EntryInfo
        {
        public:
            // Float32x4
            static FunctionInfo Float32x4;
            static FunctionInfo Zero;
            static FunctionInfo Splat;

            static FunctionInfo FromFloat64x2;
            static FunctionInfo FromFloat64x2Bits;
            static FunctionInfo FromInt32x4;
            static FunctionInfo FromInt32x4Bits;

            // UnaryOps
            static FunctionInfo Abs;
            static FunctionInfo Neg;
            static FunctionInfo Not;
            static FunctionInfo Reciprocal;
            static FunctionInfo ReciprocalSqrt;
            static FunctionInfo Sqrt;
            // BinaryOps
            static FunctionInfo Add;
            static FunctionInfo Sub;
            static FunctionInfo Mul;
            static FunctionInfo Div;
            static FunctionInfo And;
            static FunctionInfo Or;
            static FunctionInfo Xor;
            static FunctionInfo Min;
            static FunctionInfo Max;
            static FunctionInfo Scale;
            // CompareOps
            static FunctionInfo LessThan;
            static FunctionInfo LessThanOrEqual;
            static FunctionInfo Equal;
            static FunctionInfo NotEqual;
            static FunctionInfo GreaterThan;
            static FunctionInfo GreaterThanOrEqual;

            static FunctionInfo Shuffle;
            static FunctionInfo ShuffleMix;
            static FunctionInfo Clamp;
            static FunctionInfo Select;
            // WithX/Y/Z/W
            static FunctionInfo WithX;
            static FunctionInfo WithY;
            static FunctionInfo WithZ;
            static FunctionInfo WithW;
        };

        // Entry points to library
        // constructor
        static Var EntryFloat32x4(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryZero(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySplat(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryFromFloat64x2(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFromFloat64x2Bits(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFromInt32x4(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFromInt32x4Bits(RecyclableObject* function, CallInfo callInfo, ...);
        // WithX/Y/Z/W
        static Var EntryWithX(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithY(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithZ(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithW(RecyclableObject* function, CallInfo callInfo, ...);
        // UnaryOps
        static Var EntryAbs(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryNeg(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryNot(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryReciprocal(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryReciprocalSqrt(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySqrt(RecyclableObject* function, CallInfo callInfo, ...);
        // BinaryOps    
        static Var EntryAdd(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySub(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryMul(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryDiv(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryAnd(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryOr(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryXor(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryMin(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryMax(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryScale(RecyclableObject* function, CallInfo callInfo, ...);
        // CompareOps
        static Var EntryLessThan(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryLessThanOrEqual(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryEqual(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryNotEqual(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGreaterThan(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGreaterThanOrEqual(RecyclableObject* function, CallInfo callInfo, ...);
        // Others
        static Var EntryCompareOp(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryShuffle(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryShuffleMix(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryClamp(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySelect(RecyclableObject* function, CallInfo callInfo, ...);
        // End entry points
    };
} // namespace Js

#endif