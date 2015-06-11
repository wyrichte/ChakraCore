//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#define b(x) (x * MachBits)

//      Uppercase  Base      Size      Align Size        EnReg OK?  Brief name
//        Name     Type      Bytes     log2  Bits          Flag     for dumps
//      --------- ---------- --------  ----- ----------  --------  ----------

IRTYPE(Illegal,   Illegal,   0,        0,   b(0),        0,        ill)
IRTYPE(Int8,      Int,       1,        0,   b(1),        1,        i8)
IRTYPE(Int16,     Int,       2,        1,   b(2),        1,        i16)
IRTYPE(Int32,     Int,       4,        2,   b(4),        1,        i32)
IRTYPE(Int64,     Int,       8,        3,   b(8),        1,        i64)
IRTYPE(Uint8,     Uint,      1,        0,   b(1),        1,        u8)
IRTYPE(Uint16,    Uint,      2,        1,   b(2),        1,        u16)
IRTYPE(Uint32,    Uint,      4,        2,   b(4),        1,        u32)
IRTYPE(Uint64,    Uint,      8,        3,   b(8),        1,        u64)
IRTYPE(Float32,   Float,     4,        2,   b(4),        1,        f32)
IRTYPE(Float64,   Float,     8,        3,   b(8),        1,        f64)

#ifdef SIMD_JS_ENABLED
IRTYPE(Simd128,   Simd,     16,        4,   b(16),       1,        simd128)
#endif
//
// review: MachPtr->Align is incorrect on AMD64. We dont use this value today (6/29/09) so its fine
// for now. make sure it is either fixed to removed.
//

IRTYPE(Var,       Var,       MachPtr,  2,   b(MachPtr),  1,        var)
IRTYPE(Condcode, Condcode,   0,        0,   4,           1,        cc)
IRTYPE(Misc,     Misc,       0,        0,   0,           0,        misc)
#undef b
