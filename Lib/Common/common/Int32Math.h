//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

class Int32Math 
{
public:
    static bool Add(int32 left, int32 right, int32 *pResult);
    static bool Sub(int32 left, int32 right, int32 *pResult);
    static bool Mul(int32 left, int32 right, int32 *pResult);
    static bool Div(int32 left, int32 right, int32 *pResult);
    static bool Mod(int32 left, int32 right, int32 *pResult);
    static bool Shl(int32 left, int32 right, int32 *pResult);
    static bool Shr(int32 left, int32 right, int32 *pResult);
    static bool ShrU(int32 left, int32 right, int32 *pResult);
    static bool And(int32 left, int32 right, int32 *pResult);
    static bool  Or(int32 left, int32 right, int32 *pResult);
    static bool Xor(int32 left, int32 right, int32 *pResult);

    static bool Neg(int32 val, int32 *pResult);
    static bool Not(int32 val, int32 *pResult);
    static bool Inc(int32 val, int32 *pResult);
    static bool Dec(int32 val, int32 *pResult);

    static int32 NearestInRangeTo(const int32 value, const int32 minimum, const int32 maximum); // inclusive
};