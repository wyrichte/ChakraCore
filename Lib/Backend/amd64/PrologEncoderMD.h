//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class PrologEncoderMD
{
public:
    static unsigned __int8 GetRequiredNodeCountForAlloca(size_t size);
    static unsigned __int8 GetOp(IR::Instr *instr);
    static unsigned __int8 GetNonVolRegToSave(IR::Instr *instr);
    static unsigned __int8 GetXmmRegToSave(IR::Instr *instr, unsigned __int16 *scaledOffset);
    static size_t          GetAllocaSize(IR::Instr *instr);
    static unsigned __int8 GetFPReg();
};
