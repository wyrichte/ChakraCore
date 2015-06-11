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

    void        Init(Peeps *peeps) { __debugbreak(); }
    void        ProcessImplicitRegs(IR::Instr *instr) { __debugbreak(); }
    void        PeepAssign(IR::Instr *instr) { __debugbreak(); }
};


