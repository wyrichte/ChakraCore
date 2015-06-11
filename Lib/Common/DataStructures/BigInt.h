//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    /***************************************************************************
        Big non-negative integer class.
    ***************************************************************************/
    class BigInt
    {
    private:
        // Make this big enough that we rarely have to call malloc.
        enum { kcluMaxInit = 30 };

        long m_cluMax;
        long m_clu;
        ulong *m_prglu;
        ulong m_rgluInit[kcluMaxInit];

        inline BigInt & operator= (BigInt &bi);
        bool FResize(long clu);

#if DBG
        #define AssertBi(pbi) (pbi)->AssertValid(true);
        #define AssertBiNoVal(pbi) (pbi)->AssertValid(false);
        inline void AssertValid(bool fCheckVal);
#else //!DBG
        #define AssertBi(pbi)
        #define AssertBiNoVal(pbi)
#endif //!DBG

    public:
        inline BigInt(void);
        inline ~BigInt(void);

        bool FInitFromRglu(ulong *prglu, long clu);
        bool FInitFromBigint(BigInt *pbiSrc);
        template <typename EncodedChar>
        bool FInitFromDigits(const EncodedChar *prgch, long cch, long *pcchDec);
        bool FMulAdd(ulong luMul, ulong luAdd);
        bool FMulPow5(long c5);
        bool FShiftLeft(long cbit);
        void ShiftLusRight(long clu);
        void ShiftRight(long cbit);
        int Compare(BigInt *pbi);
        bool FAdd(BigInt *pbi);
        void Subtract(BigInt *pbi);
        int DivRem(BigInt *pbi);

        inline long Clu(void);
        inline ulong Lu(long ilu);
        double GetDbl(void);
    };
}
