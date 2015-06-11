//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class OpHelperBlock;
class LinearScan;
class BailOutRecord;
class BranchBailOutRecord;

class LinearScanMD : public LinearScanMDShared
{
private:
    StackSym ** helperSpillSlots;
    Func      * func;
    uint32      maxOpHelperSpilledLiveranges;
    BitVector   byteableRegsBv;
    StackSym   *xmmSymTable64[XMM_REGCOUNT];
    StackSym   *xmmSymTable32[XMM_REGCOUNT];

public:
    LinearScanMD(Func *func);   
    
    StackSym   *EnsureSpillSymForXmmReg(RegNum reg, Func *func, IRType type);
    BitVector   FilterRegIntSizeConstraints(BitVector regsBv, BitVector sizeUsageBv) const;
    bool        FitRegIntSizeConstraints(RegNum reg, BitVector sizeUsageBv) const;
    bool        FitRegIntSizeConstraints(RegNum reg, IRType type) const;
    bool        IsAllocatable(RegNum reg, Func *func) const { return true; }
    uint        UnAllocatableRegCount(Func *func) const { return 2; /* RSP, RBP */ }
    void        LegalizeDef(IR::Instr * instr) { /* This is a nop for amd64 */ }
    void        LegalizeUse(IR::Instr * instr, IR::Opnd * opnd) { /* A nop for amd64 */ }
    void        LegalizeConstantUse(IR::Instr * instr, IR::Opnd * opnd);
    void        InsertOpHelperSpillAndRestores(SList<OpHelperBlock> *opHelperBlockList);
    void        EndOfHelperBlock(uint32 helperSpilledLiveranges);
    void        GenerateBailOut(IR::Instr * instr,
                                __in_ecount(registerSaveSymsCount) StackSym ** registerSaveSyms, 
                                uint registerSaveSymsCount);
    IR::Instr  *GenerateBailInForGeneratorYield(IR::Instr * resumeLabelInstr, BailOutInfo * bailOutInfo);

private:
    static void SaveAllRegisters(BailOutRecord *const bailOutRecord);
public:
    static void SaveAllRegistersAndBailOut(BailOutRecord *const bailOutRecord);
    static void SaveAllRegistersAndBranchBailOut(BranchBailOutRecord *const bailOutRecord, const BOOL condition);

    static uint GetRegisterSaveSlotCount() { 
        return RegisterSaveSlotCount;  
    } 

    static uint GetRegisterSaveIndex(RegNum reg) { 
        return reg;
    }
    static RegNum GetRegisterFromSaveIndex(uint offset)
    {
        return (RegNum)offset;
    }

    static const uint RegisterSaveSlotCount = RegNumCount;
private:
    void        InsertOpHelperSpillsAndRestores(const OpHelperBlock& opHelperBlock);
};
