//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "BackEnd.h"
#include "SCCLiveness.h"

extern "C" IRType RegTypes[RegNumCount];

LinearScanMD::LinearScanMD(Func *func)
    : helperSpillSlots(null),
      maxOpHelperSpilledLiveranges(0),
      func(func)
{
    this->byteableRegsBv.ClearAll();
    
    FOREACH_REG(reg)
    {
        if (LinearScan::GetRegAttribs(reg) & RA_BYTEABLE)
        {
            this->byteableRegsBv.Set(reg);
        }
    } NEXT_REG;

    memset(this->xmmSymTable64, 0, sizeof(this->xmmSymTable64));
    memset(this->xmmSymTable32, 0, sizeof(this->xmmSymTable32));
}

BitVector
LinearScanMD::FilterRegIntSizeConstraints(BitVector regsBv, BitVector sizeUsageBv) const
{
    // Requires byte-able reg?
    if (sizeUsageBv.Test(1))
    {
        regsBv.And(this->byteableRegsBv);
    }

    return regsBv;
}

bool
LinearScanMD::FitRegIntSizeConstraints(RegNum reg, BitVector sizeUsageBv) const
{
    // Requires byte-able reg?
    return !sizeUsageBv.Test(1) || this->byteableRegsBv.Test(reg);
}

bool
LinearScanMD::FitRegIntSizeConstraints(RegNum reg, IRType type) const
{
    // Requires byte-able reg?
    return TySize[type] != 1 || this->byteableRegsBv.Test(reg);
}

StackSym *
LinearScanMD::EnsureSpillSymForXmmReg(RegNum reg, Func *func, IRType type)
{
    Assert(REGNUM_ISXMMXREG(reg));

    __analysis_assume(reg - FIRST_XMM_REG < XMM_REGCOUNT);
    StackSym *sym;
    if (type == TyFloat32)
    {
        sym = this->xmmSymTable32[reg - FIRST_XMM_REG];
    }
    else
    {
        sym = this->xmmSymTable64[reg - FIRST_XMM_REG];
    }

    if (sym == NULL)
    {
        sym = StackSym::New(type, func);
        func->StackAllocate(sym, TySize[type]);

        __analysis_assume(reg - FIRST_XMM_REG < XMM_REGCOUNT);

        if (type == TyFloat32)
        {
            this->xmmSymTable32[reg - FIRST_XMM_REG] = sym;
        }
        else
        {
            this->xmmSymTable64[reg - FIRST_XMM_REG] = sym;
        }
    }

    return sym;
}

void
LinearScanMD::LegalizeConstantUse(IR::Instr * instr, IR::Opnd * opnd)
{    
    Assert(opnd->IsAddrOpnd() || opnd->IsIntConstOpnd());
    intptr value = opnd->IsAddrOpnd() ? (intptr)opnd->AsAddrOpnd()->m_address : opnd->AsIntConstOpnd()->m_value;
    if (value == 0 
        && instr->m_opcode == Js::OpCode::MOV 
        && !instr->GetDst()->IsRegOpnd()
        && TySize[opnd->GetType()] >= 4)
    {        
        Assert(this->linearScan->instrUseRegs.IsEmpty());

        // MOV doesn't have a imm8 encoding for 32-bit/64-bit assignment, so if we have a register available,
        // we should hoist it and generate xor reg, reg and MOV dst, reg
        BitVector regsBv;
        regsBv.Copy(this->linearScan->activeRegs);
        regsBv.ComplimentAll();
        regsBv.And(this->linearScan->int32Regs);
        regsBv.Minus(this->linearScan->tempRegs);       // Avoid tempRegs
        BVIndex regIndex = regsBv.GetNextBit();
        if (regIndex != BVInvalidIndex)
        {            
            instr->HoistSrc1(Js::OpCode::MOV, (RegNum)regIndex);
            this->linearScan->instrUseRegs.Set(regIndex);
            this->func->m_regsUsed.Set(regIndex);

            // If we are in a loop, we need to mark the register being used by the loop so that
            // reload to that register will not be hoisted out of the loop         
            this->linearScan->RecordLoopUse(nullptr, (RegNum)regIndex);
        }
    }   
}

void
LinearScanMD::InsertOpHelperSpillAndRestores(SList<OpHelperBlock> *opHelperBlockList)
{
    if (maxOpHelperSpilledLiveranges)
    {
        Assert(!helperSpillSlots);
        helperSpillSlots = AnewArrayZ(linearScan->GetTempAlloc(), StackSym *, maxOpHelperSpilledLiveranges);
    }

    FOREACH_SLIST_ENTRY(OpHelperBlock, opHelperBlock, opHelperBlockList)
    {
        InsertOpHelperSpillsAndRestores(opHelperBlock);
    }
    NEXT_SLIST_ENTRY;
}

void
LinearScanMD::InsertOpHelperSpillsAndRestores(const OpHelperBlock& opHelperBlock)
{
    uint32 index = 0;

    FOREACH_SLIST_ENTRY(OpHelperSpilledLifetime, opHelperSpilledLifetime, &opHelperBlock.spilledLifetime)
    {
        // Use the original sym as spill slot if this is an inlinee arg
        StackSym* sym = nullptr;
        if (opHelperSpilledLifetime.spillAsArg)
        {
            sym = opHelperSpilledLifetime.lifetime->sym;
            Assert(sym->IsAllocated());
        }

        if (RegTypes[opHelperSpilledLifetime.reg] == TyFloat64)
        {
            IRType type = opHelperSpilledLifetime.lifetime->sym->GetType();
            IR::RegOpnd *regOpnd = IR::RegOpnd::New(NULL, opHelperSpilledLifetime.reg, type, this->func);
            
            if (!sym)
            {
                sym = EnsureSpillSymForXmmReg(regOpnd->GetReg(), this->func, type);
            }
            
            IR::Instr   *pushInstr = IR::Instr::New(LowererMDArch::GetAssignOp(type), IR::SymOpnd::New(sym, type, this->func), regOpnd, this->func);
            opHelperBlock.opHelperLabel->InsertAfter(pushInstr);
            pushInstr->CopyNumber(opHelperBlock.opHelperLabel);
            if (opHelperSpilledLifetime.reload)
            {
                IR::Instr   *popInstr = IR::Instr::New(LowererMDArch::GetAssignOp(type), regOpnd, IR::SymOpnd::New(sym, type, this->func), this->func);
                opHelperBlock.opHelperEndInstr->InsertBefore(popInstr);
                popInstr->CopyNumber(opHelperBlock.opHelperEndInstr);
            }
        }
        else
        {
            Assert(helperSpillSlots);
            Assert(index < maxOpHelperSpilledLiveranges);

            if (!sym)
            {
                // Lazily allocate only as many slots as we really need.
                if (!helperSpillSlots[index])
                {
                    helperSpillSlots[index] = StackSym::New(TyMachReg, func);
                }

                sym = helperSpillSlots[index];
                index++;

                Assert(sym);
                func->StackAllocate(sym, MachRegInt);
            }
            IR::RegOpnd * regOpnd = IR::RegOpnd::New(NULL, opHelperSpilledLifetime.reg, TyMachReg, func);
            LowererMD::CreateAssign(IR::SymOpnd::New(sym, TyMachReg, func), regOpnd, opHelperBlock.opHelperLabel->m_next);
            if (opHelperSpilledLifetime.reload)
            {
                LowererMD::CreateAssign(regOpnd, IR::SymOpnd::New(sym, TyMachReg, func), opHelperBlock.opHelperEndInstr);
            }
        }
    }
    NEXT_SLIST_ENTRY;
}

void
LinearScanMD::EndOfHelperBlock(uint32 helperSpilledLiveranges)
{
    if (helperSpilledLiveranges > maxOpHelperSpilledLiveranges)
    {
        maxOpHelperSpilledLiveranges = helperSpilledLiveranges;
    }
}

void 
LinearScanMD::GenerateBailOut(IR::Instr * instr, __in_ecount(registerSaveSymsCount) StackSym ** registerSaveSyms, uint registerSaveSymsCount)
{
    Func *const func = instr->m_func;
    BailOutInfo *const bailOutInfo = instr->GetBailOutInfo();
    IR::Instr *firstInstr = instr->m_prev;

    // Save registers used for parameters, and rax, if necessary, into the shadow space allocated for register parameters:
    //     mov  [rsp + 16], rdx
    //     mov  [rsp + 8], rcx
    //     mov  [rsp], rax
    for(RegNum reg = bailOutInfo->branchConditionOpnd ? RegRDX : RegRCX;
        reg != RegNOREG;
        reg = static_cast<RegNum>(reg - 1))
    {
        StackSym *const stackSym = registerSaveSyms[reg - 1];
        if(!stackSym)
        {
            continue;
        }

        const IRType regType = RegTypes[reg];
        Lowerer::InsertMove(
            IR::SymOpnd::New(func->m_symTable->GetArgSlotSym(static_cast<Js::ArgSlot>(reg)), regType, func),
            IR::RegOpnd::New(stackSym, reg, regType, func),
            instr);
    }

    if(bailOutInfo->branchConditionOpnd)
    {
        // Pass in the branch condition
        //     mov  rdx, condition
        IR::Instr *const newInstr =
            Lowerer::InsertMove(
                IR::RegOpnd::New(null, RegRDX, bailOutInfo->branchConditionOpnd->GetType(), func),
                bailOutInfo->branchConditionOpnd,
                instr);
        linearScan->SetSrcRegs(newInstr);
    }

    // Pass in the bailout record
    //     mov  rcx, bailOutRecord
    Lowerer::InsertMove(
        IR::RegOpnd::New(null, RegRCX, TyMachPtr, func),
        // RELOCJIT: Bailouts not supported.
        IR::AddrOpnd::New(bailOutInfo->bailOutRecord, IR::AddrOpndKindDynamicBailOutRecord, func, true),
        instr);

    firstInstr = firstInstr->m_next;
    for(uint i = 0; i < registerSaveSymsCount; i++)
    {
        StackSym *const stackSym = registerSaveSyms[i];
        if(!stackSym)
        {
            continue;
        }

        // Record the use on the lifetime in case it spilled afterwards. Spill loads will be inserted before 'firstInstr', that
        // is, before the register saves are done.
        this->linearScan->RecordUse(stackSym->scratch.linearScan.lifetime, firstInstr, null, true);
    }

    // Load the bailout target into rax
    //     mov  rax, BailOut
    //     call rax
    Assert(instr->GetSrc1()->IsHelperCallOpnd());
    Lowerer::InsertMove(IR::RegOpnd::New(null, RegRAX, TyMachPtr, func), instr->GetSrc1(), instr);
    instr->ReplaceSrc1(IR::RegOpnd::New(null, RegRAX, TyMachPtr, func));
}

IR::Instr *
LinearScanMD::GenerateBailInForGeneratorYield(IR::Instr * resumeLabelInstr, BailOutInfo * bailOutInfo)
{
    Js::Throw::NotImplemented();
}
