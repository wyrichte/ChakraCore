//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class Peeps;

class PeepsMD
{
private:
    Func *      func;
    Peeps *     peeps;
public:
    PeepsMD(Func *func) : func(func) {}

    void        Init(Peeps *peeps);
    void        ProcessImplicitRegs(IR::Instr *instr);
    void        PeepAssign(IR::Instr *instr);
};


