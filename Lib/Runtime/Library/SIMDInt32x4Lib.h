//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#ifdef SIMD_JS_ENABLED

namespace Js {

    class SIMDInt32x4Lib
    {
    public:
        class EntryInfo
        {
        public:
            static FunctionInfo Int32x4;
            static FunctionInfo Zero;
            static FunctionInfo Splat;
            static FunctionInfo Bool;
            // Conversions
            static FunctionInfo FromBool;
            static FunctionInfo FromFloat64x2;
            static FunctionInfo FromFloat64x2Bits;
            static FunctionInfo FromFloat32x4;
            static FunctionInfo FromFloat32x4Bits;
            // UnaryOps
            static FunctionInfo Abs;
            static FunctionInfo Neg;
            static FunctionInfo Not;
            // BinaryOps
            static FunctionInfo Add;
            static FunctionInfo Sub;
            static FunctionInfo Mul;
            static FunctionInfo And;
            static FunctionInfo Or;
            static FunctionInfo Xor;
            static FunctionInfo Min;
            static FunctionInfo Max;
            // CompareOps
            static FunctionInfo LessThan;
            static FunctionInfo Equal;
            static FunctionInfo GreaterThan;
            // WithX/Y/Z/W
            static FunctionInfo WithX;
            static FunctionInfo WithY;
            static FunctionInfo WithZ;
            static FunctionInfo WithW;
            // WithFlagX/Y/Z/W
            static FunctionInfo WithFlagX;
            static FunctionInfo WithFlagY;
            static FunctionInfo WithFlagZ;
            static FunctionInfo WithFlagW;
            // ShiftOps
            static FunctionInfo ShiftLeft;
            static FunctionInfo ShiftRightLogical;
            static FunctionInfo ShiftRightArithmetic;
            // Others
            static FunctionInfo Shuffle;
            static FunctionInfo ShuffleMix;
            static FunctionInfo Select;
        };

        // Entry points to library
        // constructor
        static Var EntryInt32x4(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryZero(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySplat(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryBool(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryFromFloat64x2(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFromFloat64x2Bits(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFromFloat32x4(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFromFloat32x4Bits(RecyclableObject* function, CallInfo callInfo, ...);

        // UnaryOps
        static Var EntryAbs(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryNeg(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryNot(RecyclableObject* function, CallInfo callInfo, ...);
        // BinaryOps
        static Var EntryAdd(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySub(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryMul(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryAnd(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryOr(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryXor(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryMin(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryMax(RecyclableObject* function, CallInfo callInfo, ...);
        // CompareOps
        static Var EntryLessThan(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryEqual(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGreaterThan(RecyclableObject* function, CallInfo callInfo, ...);
        // WithX/Y/Z/W
        static Var EntryWithX(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithY(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithZ(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithW(RecyclableObject* function, CallInfo callInfo, ...);
        // WithFlagX/Y/Z/W
        static Var EntryWithFlagX(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithFlagY(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithFlagZ(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryWithFlagW(RecyclableObject* function, CallInfo callInfo, ...);
        // ShiftOps
        static Var EntryShiftLeft(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryShiftRightLogical(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryShiftRightArithmetic(RecyclableObject* function, CallInfo callInfo, ...);
        // Others
        static Var EntryShuffle(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryShuffleMix(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySelect(RecyclableObject* function, CallInfo callInfo, ...);
        // End entry points
    };

} // namespace Js

#endif