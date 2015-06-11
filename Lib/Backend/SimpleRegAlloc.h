//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "SimpleRegAllocMD.h"

///---------------------------------------------------------------------------
///
/// class SimpleRegAlloc
///
///     Temporary dumb register allocator until we implement a better one.
///
///---------------------------------------------------------------------------

class SimpleRegAlloc
{
    friend class SimpleRegAllocMD;

public:
    SimpleRegAlloc(Func * func) : m_func(func), m_simpleRegAllocMD(func), m_lastSpilled(RegNOREG) {}

    static bool         IsCalleeSaved(RegNum reg);
    static bool         IsCallerSaved(RegNum reg);

    void                RegAssign();

private:
    IR::Instr *         AllocateDst(IR::Opnd *dst, IR::Instr *instr);
    IR::Instr *         AllocateRegDef(IR::RegOpnd *def, IR::Instr *instr);
    IR::Instr *         AllocateSrc(IR::Opnd *src, IR::Instr *instr);
    IR::Instr *         AllocateRegUse(IR::RegOpnd *use, IR::Instr *instr);
    bool                IsAllocatable(RegNum reg) const;
    void                Release(RegNum reg);
    RegNum              Allocate(IR::RegOpnd *regOpnd, IR::Instr * instr);
    void                SpillAll(IR::Instr *instr);

private:
    Func *              m_func;
    SimpleRegAllocMD    m_simpleRegAllocMD;
    RegNum              m_lastSpilled;
};