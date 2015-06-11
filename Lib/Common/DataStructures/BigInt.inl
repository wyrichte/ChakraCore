//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    BigInt & BigInt::operator= (BigInt &bi)
    {
        AssertMsg(false, "can't assign BigInts");
        return *this;
    }

#if DBG
    void BigInt::AssertValid(bool fCheckVal)
    {
        Assert(m_cluMax >= kcluMaxInit);
        Assert(m_prglu != 0);
        Assert(m_clu >= 0 && m_clu <= m_cluMax);
        Assert(!fCheckVal || 0 == m_clu || 0 != m_prglu[m_clu - 1]);
        Assert((m_prglu == m_rgluInit) == (m_cluMax == kcluMaxInit));
    }
#endif

    BigInt::BigInt(void)
    {
        m_cluMax = kcluMaxInit;
        m_clu = 0;
        m_prglu = m_rgluInit;
        AssertBi(this);
    }

    BigInt::~BigInt(void)
    {
        if (m_prglu != m_rgluInit)
            free(m_prglu);
    }

    long BigInt::Clu(void)
    {
        return m_clu;
    }

    ulong BigInt::Lu(long ilu)
    {
        AssertBi(this);
        Assert(ilu < m_clu);
        return m_prglu[ilu];
    }
}
