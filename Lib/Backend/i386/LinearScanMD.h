//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class LinearScan;
class OpHelperBlock;
class BailOutRecord;
class BranchBailOutRecord;

class LinearScanMD : public LinearScanMDShared
{
private:
    BitVector   byteableRegsBv;
    StackSym   *xmmSymTable64[XMM_REGCOUNT];
    StackSym   *xmmSymTable32[XMM_REGCOUNT];
    Func       *func;

public:
    LinearScanMD(Func *func);

    bool        IsAllocatable(RegNum reg, Func *func) const { return true; }
    BitVector   FilterRegIntSizeConstraints(BitVector regsBv, BitVector sizeUsageBv) const;
    bool        FitRegIntSizeConstraints(RegNum reg, BitVector sizeUsageBv) const;
    bool        FitRegIntSizeConstraints(RegNum reg, IRType type) const;
    uint        UnAllocatableRegCount(Func *func) const { return 2; }
    StackSym   *EnsureSpillSymForXmmReg(RegNum reg, Func *func, IRType type);
    void        LegalizeDef(IR::Instr * instr) { /* This is a nop for x86 */ }
    void        LegalizeUse(IR::Instr * instr, IR::Opnd * opnd) { /* A nop for x86 */ }
    void        LegalizeConstantUse(IR::Instr * instr, IR::Opnd * opnd) { /* A nop for x86 */ }
    void        GenerateBailOut(IR::Instr * instr, __in_ecount(registerSaveSymsCount) StackSym ** registerSaveSyms, uint registerSaveSymsCount);
    IR::Instr  *GenerateBailInForGeneratorYield(IR::Instr * resumeLabelInstr, BailOutInfo * bailOutInfo);
    void        InsertOpHelperSpillAndRestores(SList<OpHelperBlock> *opHelperBlockList);
    void        EndOfHelperBlock(uint32 helperSpilledLiveranges) { /* NOP */ }

private:
    static void SaveAllRegisters(BailOutRecord *const bailOutRecord);
    static void SaveAllRegistersNoSse2(BailOutRecord *const bailOutRecord);
public:
    static void SaveAllRegistersAndBailOut(BailOutRecord *const bailOutRecord);
    static void SaveAllRegistersNoSse2AndBailOut(BailOutRecord *const bailOutRecord);
    static void SaveAllRegistersAndBranchBailOut(BranchBailOutRecord *const bailOutRecord, const BOOL condition);
    static void SaveAllRegistersNoSse2AndBranchBailOut(BranchBailOutRecord *const bailOutRecord, const BOOL condition);

public:
    static uint GetRegisterSaveSlotCount() { 
        return RegisterSaveSlotCount;  
    } 
    static uint GetRegisterSaveIndex(RegNum reg);
    static RegNum GetRegisterFromSaveIndex(uint offset);

    // All regs plus an extra slot for float64s
    static const uint RegisterSaveSlotCount = RegNumCount + XMM_REGCOUNT;

private:
    void        InsertOpHelperSpillsAndRestores(const OpHelperBlock& opHelperBlock);

};
